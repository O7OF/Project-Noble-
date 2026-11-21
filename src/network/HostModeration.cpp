#include "HostModeration.hpp"

#include "common/Logger.hpp"
#include "network/AuthClient.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"

#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <nlohmann/json.hpp>
#include <set>
#include <unordered_map>

namespace noble::HostModeration {
namespace {

std::mutex s_mutex;
bool s_loaded = false;
std::vector<BanEntry> s_bans;
struct Cooldown {
  PlayerRef player;
  int64_t until = 0;
};
std::vector<Cooldown> s_cooldowns;
std::set<uint64_t> s_hostMuted;
bool s_filterOn = false; // off by default; first-start setup asks (Tyler 2026-11-20)
std::vector<std::string> s_customWords;
std::unordered_map<uint64_t, ULONGLONG> s_lastSent; // machine id -> last host_remove
std::set<uint64_t> s_chatMutedIds;      // local text mute
std::set<std::string> s_chatMutedNames; // local text mute, lower-case names
int s_afkMinutes = 0;                   // host setting
std::set<uint64_t> s_coHosts;           // host side
// Guest: the host's last host_rules.
int s_guestAfk = 0;
bool s_guestFilterOn = false;
std::vector<std::string> s_guestWords;
std::set<uint64_t> s_guestMuted;
std::set<uint64_t> s_guestCoHosts;
ULONGLONG s_rulesAt = 0;
// Lobby access (host): saved password; the mode, admitted players and invites
// live in host_access.json so a match relaunch keeps them.
std::string s_lobbyPassword;
JoinAccess s_access = JoinAccess::Open;
std::set<std::string> s_admitted;  // "a:<account>", "m:<machine>", "n:<name>"
std::set<std::string> s_invited;   // lower-case usernames
struct AuthSeen {
  bool ok = false;
  bool gavePassword = false;
};
std::unordered_map<uint64_t, AuthSeen> s_authByMachine;
std::unordered_map<uint64_t, AuthSeen> s_authByAccount;
std::unordered_map<uint64_t, ULONGLONG> s_firstSeen; // machine id -> first seen
// Lobby access (guest).
std::string s_joinPassword;
ULONGLONG s_joinedAt = 0;
bool s_guestAdmitted = false;

int64_t Now() { return static_cast<int64_t>(std::time(nullptr)); }

std::filesystem::path FilePath() {
  wchar_t exe[MAX_PATH] = {};
  GetModuleFileNameW(nullptr, exe, MAX_PATH);
  return std::filesystem::path(exe).parent_path() / "data" / "userdata" /
         "host_bans.json";
}

void SaveLocked() {
  nlohmann::json j;
  j["bans"] = nlohmann::json::array();
  for (const BanEntry &b : s_bans) {
    j["bans"].push_back({{"account", std::to_string(b.accountId)},
                         {"name", b.name},
                         {"reason", b.reason},
                         {"banned_at", b.bannedAt},
                         {"until", b.until}});
  }
  j["filter"] = {{"enabled", s_filterOn}, {"custom", s_customWords}};
  j["afk_minutes"] = s_afkMinutes;
  j["lobby_password"] = s_lobbyPassword;
  try {
    const std::filesystem::path path = FilePath();
    std::filesystem::create_directories(path.parent_path());
    const std::filesystem::path tmp = path.wstring() + L".tmp";
    {
      std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
      out << j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace);
    }
    MoveFileExW(tmp.c_str(), path.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
  } catch (const std::exception &e) {
    NOBLE_WARN("HostModeration: could not save the ban list: {}", e.what());
    (void)e;
  }
}

void LoadLocked() {
  if (s_loaded)
    return;
  s_loaded = true;
  try {
    std::ifstream in(FilePath(), std::ios::binary);
    if (!in)
      return;
    const nlohmann::json j = nlohmann::json::parse(in, nullptr, false);
    if (j.is_discarded())
      return;
    for (const auto &b : j.value("bans", nlohmann::json::array())) {
      BanEntry e;
      try {
        e.accountId = std::stoull(b.value("account", std::string("0")));
      } catch (...) {
      }
      e.name = b.value("name", std::string());
      e.reason = b.value("reason", std::string());
      e.bannedAt = b.value("banned_at", static_cast<int64_t>(0));
      e.until = b.value("until", static_cast<int64_t>(0));
      s_bans.push_back(e);
    }
    const auto f = j.value("filter", nlohmann::json::object());
    s_filterOn = f.value("enabled", false);
    s_customWords = f.value("custom", std::vector<std::string>());
    s_afkMinutes = j.value("afk_minutes", 0);
    s_lobbyPassword = j.value("lobby_password", std::string());
    NOBLE_INFO("HostModeration: {} ban(s), word filter {} ({} custom word(s))",
               s_bans.size(), s_filterOn ? "on" : "off", s_customWords.size());
  } catch (...) {
  }
}

void DropExpiredLocked() {
  const int64_t now = Now();
  const size_t before = s_bans.size();
  s_bans.erase(std::remove_if(s_bans.begin(), s_bans.end(),
                              [now](const BanEntry &b) {
                                return b.until != 0 && b.until <= now;
                              }),
               s_bans.end());
  if (s_bans.size() != before)
    SaveLocked();
  s_cooldowns.erase(std::remove_if(s_cooldowns.begin(), s_cooldowns.end(),
                                   [now](const Cooldown &c) {
                                     return c.until <= now;
                                   }),
                    s_cooldowns.end());
}

bool SameText(const std::string &a, const std::string &b) {
  return !a.empty() && a.size() == b.size() &&
         std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
           return std::tolower(static_cast<unsigned char>(x)) ==
                  std::tolower(static_cast<unsigned char>(y));
         });
}

// Account first (stable); else this session's machine id; else, for players
// without an account on either side, the name.
bool Matches(uint64_t accountId, uint64_t machineId, const std::string &name,
             const PlayerRef &p) {
  if (accountId > 1 && p.accountId > 1)
    return accountId == p.accountId;
  if (machineId != 0 && machineId == p.machineId)
    return true;
  // No account on the player we are checking (LAN, or the in-match scoreboard,
  // which has no account ids): fall back to the name.
  return p.accountId <= 1 && SameText(name, p.name);
}

int MinutesLeft(int64_t until) {
  if (until == 0)
    return 0;
  return static_cast<int>((std::max)(static_cast<int64_t>(1), (until - Now() + 59) / 60));
}

// ---- word filter ----
std::string Normalise(const std::string &word) {
  std::string out;
  out.reserve(word.size());
  for (unsigned char c : word) {
    switch (c) {
    case '0': out += 'o'; break;
    case '1': case '!': case '|': out += 'i'; break;
    case '3': out += 'e'; break;
    case '4': case '@': out += 'a'; break;
    case '5': case '$': out += 's'; break;
    case '7': out += 't'; break;
    default:
      if (std::isalpha(c))
        out += static_cast<char>(std::tolower(c));
    }
  }
  return out;
}
std::string Collapse(const std::string &w) {
  std::string out;
  for (char c : w)
    if (out.empty() || out.back() != c)
      out += c;
  return out;
}

// Built-in list (slurs), normalised. Matched as whole words, and also with
// letters repeated ("niiigger"): only a word that repeats a letter is compared
// in its collapsed form, so a real word like the country "Niger" never is.
const char *const kBuiltIn[] = {
    "nigger", "niggers", "nigga", "niggas", "niggaz", "nigguh", "sandnigger",
    "faggot", "faggots", "faggit", "fag", "fags", "kike", "kikes", "chink",
    "chinks", "spic", "spics", "wetback", "wetbacks", "gook", "gooks",
    "beaner", "beaners", "raghead", "ragheads", "towelhead", "towelheads",
    "tranny", "trannies", "retard", "retards", "coon", "coons", "dyke", "dykes"};
struct WordSets {
  std::set<std::string> exact, collapsed;
};
WordSets BuildSets(bool builtIn, const std::vector<std::string> &custom);
WordSets BuildSetsLocked() { return BuildSets(s_filterOn, s_customWords); }

WordSets BuildSets(bool builtIn, const std::vector<std::string> &custom) {
  WordSets s;
  auto add = [&](const std::string &raw) {
    const std::string n = Normalise(raw);
    if (n.size() < 2)
      return;
    s.exact.insert(n);
    const std::string c = Collapse(n);
    if (c.size() >= 4)
      s.collapsed.insert(c);
  };
  if (builtIn)
    for (const char *w : kBuiltIn)
      add(w);
  for (const std::string &w : custom)
    add(w);
  return s;
}

// Common swear words for server names (whole words only; the slur list is added
// too). Kept short on purpose: names like "Dick" or "Cockpit" are left alone.
const char *const kProfanity[] = {
    "fuck", "fucks", "fucked", "fucker", "fuckers", "fucking", "fuckin",
    "motherfucker", "motherfuckers", "motherfucking", "fuk", "fck", "fuq",
    "shit", "shits", "shitty", "shithead", "bullshit", "ass", "asses",
    "asshole", "assholes", "arsehole", "jackass", "dumbass", "badass", "bitch",
    "bitches", "bitchy", "cunt", "cunts", "dickhead", "dickheads", "pussy",
    "pussies", "whore", "whores", "slut", "sluts", "bastard", "bastards",
    "porn", "twat", "twats", "wanker", "wankers", "tits", "boobs", "penis",
    "vagina", "dildo", "nazi", "nazis", "hitler", "rape", "rapist"};

// Masks every listed word with '*'. Words = runs of letters, digits and the
// leet symbols Normalise maps; '!' and '|' count only between letters
// ("f!g"), not as punctuation. splitCaps: "BadAss" -> "Bad" + "Ass".
std::string MaskWords(const std::string &text, const WordSets &sets,
                      bool splitCaps) {
  if (sets.exact.empty())
    return text;
  std::string out = text;
  auto isWordChar = [&out](size_t at) {
    const unsigned char c = static_cast<unsigned char>(out[at]);
    if (std::isalnum(c) || c == '@' || c == '$')
      return true;
    if (c == '!' || c == '|')
      return at + 1 < out.size() &&
             std::isalnum(static_cast<unsigned char>(out[at + 1]));
    return false;
  };
  size_t i = 0;
  while (i < out.size()) {
    if (!isWordChar(i)) {
      ++i;
      continue;
    }
    size_t j = i;
    while (j < out.size() && isWordChar(j)) {
      ++j;
      if (splitCaps && j < out.size() &&
          std::isupper(static_cast<unsigned char>(out[j])) &&
          std::islower(static_cast<unsigned char>(out[j - 1])))
        break;
    }
    const std::string n = Normalise(out.substr(i, j - i));
    const std::string c = Collapse(n);
    if (!n.empty() &&
        (sets.exact.count(n) || (c != n && sets.collapsed.count(c)))) {
      for (size_t k = i; k < j; ++k)
        out[k] = '*';
    }
    i = j;
  }
  return out;
}

} // namespace

std::string BanLengthText(int minutes) {
  switch (minutes) {
  case 0: return "PERMANENT";
  case 30: return "30 MIN";
  case 60: return "1 HOUR";
  case 24 * 60: return "24 HOURS";
  default: return std::to_string(minutes) + " MIN";
  }
}

void Kick(const PlayerRef &player, const std::string &reason) {
  {
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    s_cooldowns.push_back({player, Now() + kKickCooldownSeconds});
    s_lastSent[player.machineId] = GetTickCount64();
  }
  NetworkTransport::Get().SendHostRemove(player.machineId, "kick", reason, 0);
  NOBLE_INFO("HostModeration: kicked '{}' (machine 0x{:X}, account {})",
             player.name, player.machineId, player.accountId);
}

void Ban(const PlayerRef &player, int minutes, const std::string &reason) {
  BanEntry e;
  e.accountId = player.accountId;
  e.machineId = player.machineId;
  e.name = player.name;
  e.reason = reason;
  e.bannedAt = Now();
  e.until = minutes > 0 ? e.bannedAt + static_cast<int64_t>(minutes) * 60 : 0;
  {
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    s_bans.push_back(e);
    SaveLocked();
    s_lastSent[player.machineId] = GetTickCount64();
  }
  NetworkTransport::Get().SendHostRemove(player.machineId, "ban", reason,
                                         minutes);
  NOBLE_INFO("HostModeration: banned '{}' ({}, account {})", player.name,
             BanLengthText(minutes), player.accountId);
}

void Unban(size_t index) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  DropExpiredLocked();
  if (index >= s_bans.size())
    return;
  NOBLE_INFO("HostModeration: lifted the ban on '{}'", s_bans[index].name);
  s_bans.erase(s_bans.begin() + static_cast<std::ptrdiff_t>(index));
  SaveLocked();
}

std::vector<BanEntry> Bans() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  DropExpiredLocked();
  return s_bans;
}

std::string BlockReason(const PlayerRef &player, int *minutesLeft) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  DropExpiredLocked();
  for (const BanEntry &b : s_bans) {
    if (Matches(b.accountId, b.machineId, b.name, player)) {
      if (minutesLeft)
        *minutesLeft = MinutesLeft(b.until);
      return "ban";
    }
  }
  for (const Cooldown &c : s_cooldowns) {
    if (Matches(c.player.accountId, c.player.machineId, c.player.name,
                player)) {
      if (minutesLeft)
        *minutesLeft = MinutesLeft(c.until);
      return "kick";
    }
  }
  return {};
}

void Tick(const std::vector<PlayerRef> &players) {
  static ULONGLONG s_last = 0, s_lastRules = 0;
  const ULONGLONG now = GetTickCount64();
  // Every 5 s: tell everyone the anti-AFK limit and who the co-hosts are.
  // (Tick only runs on the host; in the pre-game lobby this goes over the relay.)
  if (now - s_lastRules >= 5000) {
    s_lastRules = now;
    nlohmann::json rules = {{"type", "host_rules"}};
    std::vector<std::string> ids;
    {
      std::lock_guard<std::mutex> lock(s_mutex);
      LoadLocked();
      rules["afk"] = s_afkMinutes;
      for (uint64_t id : s_coHosts)
        ids.push_back(std::to_string(id));
      // The host's chat rules, so guests can apply them to chat that reaches
      // them over the relay without passing through the host.
      rules["filter"] = s_filterOn;
      rules["words"] = s_customWords;
      std::vector<std::string> muted;
      for (uint64_t id : s_hostMuted)
        muted.push_back(std::to_string(id));
      rules["muted"] = muted;
    }
    rules["cohosts"] = ids;
    NetworkTransport::Get().SendLobbyControlJson(rules.dump());
  }
  if (now - s_last < 1000)
    return;
  s_last = now;
  for (const PlayerRef &p : players) {
    if (p.machineId == 0)
      continue;
    int minutes = 0;
    const std::string kind = BlockReason(p, &minutes);
    if (kind.empty())
      continue;
    ULONGLONG &last = s_lastSent[p.machineId];
    if (last != 0 && now - last < 3000)
      continue;
    last = now;
    NetworkTransport::Get().SendHostRemove(
        p.machineId, kind,
        kind == "ban" ? "You are banned from this host's games."
                      : "You were removed by the host a moment ago.",
        minutes);
    NOBLE_INFO("HostModeration: '{}' is {} here; asked them to leave", p.name,
               kind == "ban" ? "banned" : "on a rejoin cooldown");
  }
}

void SetHostMuted(uint64_t machineId, bool muted) {
  std::lock_guard<std::mutex> lock(s_mutex);
  if (muted)
    s_hostMuted.insert(machineId);
  else
    s_hostMuted.erase(machineId);
}

bool IsHostMuted(uint64_t machineId) {
  std::lock_guard<std::mutex> lock(s_mutex);
  return s_hostMuted.count(machineId) > 0;
}

std::string FilterText(const std::string &text) {
  WordSets sets;
  {
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    sets = BuildSetsLocked();
  }
  return MaskWords(text, sets, false);
}

std::string FilterServerName(const std::string &name) {
  static const WordSets sets = [] {
    const std::vector<std::string> words(std::begin(kProfanity),
                                         std::end(kProfanity));
    return BuildSets(true, words);
  }();
  return MaskWords(name, sets, true);
}

bool ApplyChatRules(uint64_t senderMachine, std::string &text) {
  if (NetworkTransport::Get().IsHost()) {
    if (senderMachine != 0 && IsHostMuted(senderMachine))
      return false;
    text = FilterText(text);
    return true;
  }
  WordSets sets;
  {
    std::lock_guard<std::mutex> lock(s_mutex);
    // The host sends its rules every 5 s; stale rules are ignored.
    if (s_rulesAt == 0 || GetTickCount64() - s_rulesAt > 20000)
      return true;
    if (senderMachine != 0 && s_guestMuted.count(senderMachine))
      return false;
    sets = BuildSets(s_guestFilterOn, s_guestWords);
  }
  text = MaskWords(text, sets, false);
  return true;
}

bool FilterEnabled() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return s_filterOn;
}

void SetFilterEnabled(bool on) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  s_filterOn = on;
  SaveLocked();
}

std::vector<std::string> CustomWords() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return s_customWords;
}

void SetCustomWords(const std::vector<std::string> &words) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  s_customWords = words;
  SaveLocked();
}

void SetChatMuted(uint64_t machineId, const std::string &name, bool muted) {
  std::string lower = name;
  std::transform(lower.begin(), lower.end(), lower.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  std::lock_guard<std::mutex> lock(s_mutex);
  if (muted) {
    if (machineId != 0)
      s_chatMutedIds.insert(machineId);
    if (!lower.empty())
      s_chatMutedNames.insert(lower);
  } else {
    s_chatMutedIds.erase(machineId);
    s_chatMutedNames.erase(lower);
  }
}

bool IsChatMuted(uint64_t machineId, const std::string &name) {
  std::string lower = name;
  std::transform(lower.begin(), lower.end(), lower.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  std::lock_guard<std::mutex> lock(s_mutex);
  return (machineId != 0 && s_chatMutedIds.count(machineId)) ||
         (!lower.empty() && s_chatMutedNames.count(lower));
}

int AfkMinutes() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return s_afkMinutes;
}

void SetAfkMinutes(int minutes) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  s_afkMinutes = (std::max)(0, minutes);
  SaveLocked();
}

int GuestAfkMinutes() {
  std::lock_guard<std::mutex> lock(s_mutex);
  // A rule not refreshed for 20 s no longer applies (host gone, or no rules).
  if (s_rulesAt == 0 || GetTickCount64() - s_rulesAt > 20000)
    return 0;
  return s_guestAfk;
}

double LocalIdleSeconds() {
  // Keyboard and mouse: Windows' last input. Controllers don't count there, so
  // XInput packet numbers (they change on any button or stick change) too.
  using GetStateFn = DWORD(WINAPI *)(DWORD, void *);
  static const GetStateFn getState = []() -> GetStateFn {
    HMODULE m = LoadLibraryW(L"xinput1_4.dll");
    if (!m)
      m = LoadLibraryW(L"xinput9_1_0.dll");
    return m ? reinterpret_cast<GetStateFn>(GetProcAddress(m, "XInputGetState"))
             : nullptr;
  }();
  static DWORD s_packet[4] = {};
  static ULONGLONG s_padAt = 0;
  const ULONGLONG now = GetTickCount64();
  if (getState) {
    struct {
      DWORD packet;
      BYTE pad[12];
    } st{};
    for (DWORD i = 0; i < 4; ++i) {
      if (getState(i, &st) == 0 && st.packet != s_packet[i]) {
        s_packet[i] = st.packet;
        s_padAt = now;
      }
    }
  }
  LASTINPUTINFO li{};
  li.cbSize = sizeof(li);
  double kbm = 0.0;
  if (GetLastInputInfo(&li))
    kbm = static_cast<double>(GetTickCount() - li.dwTime) / 1000.0;
  const double pad =
      s_padAt == 0 ? 1e9 : static_cast<double>(now - s_padAt) / 1000.0;
  return (std::min)(kbm, pad);
}

void SetCoHost(uint64_t machineId, bool on) {
  if (machineId == 0)
    return;
  std::lock_guard<std::mutex> lock(s_mutex);
  if (on)
    s_coHosts.insert(machineId);
  else
    s_coHosts.erase(machineId);
  NOBLE_INFO("HostModeration: machine 0x{:X} is {} a co-host", machineId,
             on ? "now" : "no longer");
}

bool IsCoHost(uint64_t machineId) {
  std::lock_guard<std::mutex> lock(s_mutex);
  return s_coHosts.count(machineId) > 0;
}

bool LocalIsCoHost() {
  const uint64_t a = NetworkTransport::Get().GetLocalMachineId();
  const uint64_t b = NetworkTransport::Get().GetRelayMachineId();
  std::lock_guard<std::mutex> lock(s_mutex);
  if (s_rulesAt == 0 || GetTickCount64() - s_rulesAt > 20000)
    return false;
  return (a != 0 && s_guestCoHosts.count(a)) ||
         (b != 0 && s_guestCoHosts.count(b));
}

void RequestCoHostAction(const std::string &action, uint64_t targetMachine,
                         const std::string &targetName) {
  const nlohmann::json j = {
      {"type", "cohost_action"},
      {"action", action},
      {"target", std::to_string(targetMachine)},
      {"name", targetName},
      {"from", std::to_string(NetworkTransport::Get().GetLocalMachineId())}};
  NetworkTransport::Get().SendLobbyControlJson(
      j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
  NOBLE_INFO("HostModeration: asked the host to {} '{}'", action, targetName);
}

void OnHostRules(const std::string &json) {
  try {
    const nlohmann::json j = nlohmann::json::parse(json);
    std::set<uint64_t> ids;
    for (const auto &v : j.value("cohosts", nlohmann::json::array())) {
      try {
        ids.insert(std::stoull(v.get<std::string>()));
      } catch (...) {
      }
    }
    std::set<uint64_t> muted;
    for (const auto &v : j.value("muted", nlohmann::json::array())) {
      try {
        muted.insert(std::stoull(v.get<std::string>()));
      } catch (...) {
      }
    }
    std::vector<std::string> words;
    for (const auto &v : j.value("words", nlohmann::json::array()))
      if (v.is_string())
        words.push_back(v.get<std::string>());
    std::lock_guard<std::mutex> lock(s_mutex);
    s_guestAfk = (std::max)(0, j.value("afk", 0));
    s_guestCoHosts = std::move(ids);
    s_guestMuted = std::move(muted);
    s_guestFilterOn = j.value("filter", false);
    s_guestWords = std::move(words);
    s_rulesAt = GetTickCount64();
  } catch (...) {
  }
}

void OnCoHostAction(const std::string &json, uint64_t senderMachine) {
  try {
    const nlohmann::json j = nlohmann::json::parse(json);
    uint64_t sender = senderMachine;
    if (sender == 0) { // via the relay: the request names its sender
      try {
        sender = std::stoull(j.value("from", std::string("0")));
      } catch (...) {
      }
    }
    PlayerRef target;
    try {
      target.machineId = std::stoull(j.value("target", std::string("0")));
    } catch (...) {
    }
    target.name = j.value("name", std::string());
    const std::string action = j.value("action", std::string());
    if (!IsCoHost(sender)) {
      NOBLE_WARN("HostModeration: ignored a co-host request from 0x{:X} (not a "
                 "co-host)",
                 sender);
      return;
    }
    if (target.machineId == 0 ||
        target.machineId == NetworkTransport::kDefaultHostMachineId ||
        target.machineId == NetworkTransport::Get().GetLocalMachineId())
      return; // never the host
    NOBLE_INFO("HostModeration: co-host 0x{:X} asked to {} '{}'", sender,
               action, target.name);
    if (action == "kick")
      Kick(target, "A co-host removed you from the game.");
    else if (action == "mute")
      SetHostMuted(target.machineId, true);
    else if (action == "unmute")
      SetHostMuted(target.machineId, false);
  } catch (...) {
  }
}

// ---- notice shown after the host removed this player ----
namespace {
std::filesystem::path NoticePath() {
  return FilePath().parent_path() / "host_notice.txt";
}
} // namespace

void WriteRemovalNotice(const std::string &text) {
  try {
    std::ofstream out(NoticePath(), std::ios::binary | std::ios::trunc);
    out << text;
  } catch (...) {
  }
}

std::string TakeRemovalNotice() {
  std::string text;
  try {
    std::ifstream in(NoticePath(), std::ios::binary);
    if (!in)
      return {};
    text.assign(std::istreambuf_iterator<char>(in),
                std::istreambuf_iterator<char>());
    in.close();
    std::filesystem::remove(NoticePath());
  } catch (...) {
  }
  return text;
}


// ---------------- Lobby access ----------------
namespace {

std::filesystem::path AccessPath() {
  return FilePath().parent_path() / "host_access.json";
}

std::string Lower(std::string text) {
  for (char &c : text)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return text;
}

// Every key that identifies this player: the account when there is one
// (stable), else this session's machine id and the name (LAN players get a new
// machine id each launch, so the name is what finds them after a match).
std::vector<std::string> PlayerKeys(const PlayerRef &p) {
  std::vector<std::string> keys;
  if (p.accountId > 1) {
    keys.push_back("a:" + std::to_string(p.accountId));
    return keys;
  }
  if (p.machineId != 0)
    keys.push_back("m:" + std::to_string(p.machineId));
  if (!p.name.empty())
    keys.push_back("n:" + Lower(p.name));
  return keys;
}

void SaveAccessLocked() {
  nlohmann::json j;
  j["access"] = static_cast<int>(s_access);
  j["admitted"] = std::vector<std::string>(s_admitted.begin(), s_admitted.end());
  j["invited"] = std::vector<std::string>(s_invited.begin(), s_invited.end());
  j["saved_at"] = Now();
  try {
    const std::filesystem::path path = AccessPath();
    std::filesystem::create_directories(path.parent_path());
    const std::filesystem::path tmp = path.wstring() + L".tmp";
    {
      std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
      out << j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace);
    }
    MoveFileExW(tmp.c_str(), path.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
  } catch (...) {
  }
}

bool AdmittedLocked(const PlayerRef &p) {
  for (const std::string &k : PlayerKeys(p))
    if (s_admitted.count(k))
      return true;
  return false;
}

void AdmitLocked(const PlayerRef &p) {
  for (const std::string &k : PlayerKeys(p))
    s_admitted.insert(k);
  SaveAccessLocked();
}

const AuthSeen *AuthLocked(const PlayerRef &p) {
  if (p.machineId != 0) {
    auto it = s_authByMachine.find(p.machineId);
    if (it != s_authByMachine.end())
      return &it->second;
  }
  if (p.accountId > 1) {
    auto it = s_authByAccount.find(p.accountId);
    if (it != s_authByAccount.end())
      return &it->second;
  }
  return nullptr;
}

uint64_t ParseId(const nlohmann::json &j, const char *key) {
  try {
    return std::stoull(j.value(key, std::string("0")), nullptr, 0);
  } catch (...) {
    return 0;
  }
}

uint64_t LocalAccountId() {
  return AuthClient::Get().IsLoggedIn()
             ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
             : 0ULL;
}

} // namespace

JoinAccess Access() {
  std::lock_guard<std::mutex> lock(s_mutex);
  return s_access;
}

void SetAccess(JoinAccess access) {
  std::lock_guard<std::mutex> lock(s_mutex);
  if (s_access == access)
    return;
  s_access = access;
  SaveAccessLocked();
  NOBLE_INFO("HostModeration: lobby joins now {} ({} admitted)",
             AccessText(access), s_admitted.size());
}

const char *AccessText(JoinAccess access) {
  switch (access) {
  case JoinAccess::InviteOnly:
    return "INVITE ONLY";
  case JoinAccess::Password:
    return "PASSWORD";
  case JoinAccess::Locked:
    return "LOCKED";
  default:
    return "OPEN";
  }
}

std::string LobbyPassword() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return s_lobbyPassword;
}

void SetLobbyPassword(const std::string &password) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  s_lobbyPassword = password.substr(0, 63);
  SaveLocked();
  // Whoever already sent a password is checked again against the new one.
  s_authByMachine.clear();
  s_authByAccount.clear();
  NOBLE_INFO("HostModeration: lobby password {}",
             s_lobbyPassword.empty() ? "cleared" : "set");
}

bool AdvertHasPassword() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return !s_lobbyPassword.empty() && (s_access == JoinAccess::Password ||
                                      s_access == JoinAccess::InviteOnly);
}

void BeginLobbyAccess() {
  std::lock_guard<std::mutex> lock(s_mutex);
  s_access = JoinAccess::Open;
  s_admitted.clear();
  s_invited.clear();
  s_authByMachine.clear();
  s_authByAccount.clear();
  s_firstSeen.clear();
}

void RestoreLobbyAccess() {
  std::lock_guard<std::mutex> lock(s_mutex);
  try {
    std::ifstream in(AccessPath(), std::ios::binary);
    if (!in)
      return;
    const nlohmann::json j = nlohmann::json::parse(in, nullptr, false);
    if (j.is_discarded())
      return;
    // Only the lobby this host just left: a file from hours ago is an old lobby.
    if (Now() - j.value("saved_at", static_cast<int64_t>(0)) > 6 * 3600)
      return;
    const int a = j.value("access", 0);
    s_access = (a >= 0 && a <= 3) ? static_cast<JoinAccess>(a) : JoinAccess::Open;
    for (const auto &k : j.value("admitted", std::vector<std::string>()))
      s_admitted.insert(k);
    for (const auto &k : j.value("invited", std::vector<std::string>()))
      s_invited.insert(k);
    NOBLE_INFO("HostModeration: back in the lobby: joins {}, {} admitted, {} "
               "invited",
               AccessText(s_access), s_admitted.size(), s_invited.size());
  } catch (...) {
  }
}

void NoteInvited(const std::string &username) {
  if (username.empty())
    return;
  std::lock_guard<std::mutex> lock(s_mutex);
  s_invited.insert(Lower(username));
  SaveAccessLocked();
}

size_t AdmittedCount() {
  std::lock_guard<std::mutex> lock(s_mutex);
  return s_admitted.size();
}

void TickLobbyAccess(const std::vector<PlayerRef> &players) {
  static ULONGLONG s_last = 0;
  const ULONGLONG now = GetTickCount64();
  if (now - s_last < 1000)
    return;
  s_last = now;
  struct Removal {
    PlayerRef player;
    std::string kind;
  };
  std::vector<PlayerRef> admittedNow;
  std::vector<Removal> removals;
  {
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    for (const PlayerRef &p : players) {
      if (AdmittedLocked(p))
        continue;
      const AuthSeen *auth = AuthLocked(p);
      const bool invited = !p.name.empty() && s_invited.count(Lower(p.name));
      const bool passwordOk = auth && auth->ok && !s_lobbyPassword.empty() &&
                              s_access != JoinAccess::Locked;
      // A host's own friend invite gets through even a locked lobby.
      if (s_access == JoinAccess::Open || invited || passwordOk) {
        AdmitLocked(p);
        admittedNow.push_back(p);
        s_firstSeen.erase(p.machineId);
        continue;
      }
      // Give a new player a few seconds for its password to arrive.
      ULONGLONG &seen = s_firstSeen[p.machineId];
      if (seen == 0)
        seen = now;
      if (!auth && now - seen < 6000)
        continue;
      std::string kind = "locked";
      if (s_access == JoinAccess::InviteOnly)
        kind = (auth && auth->gavePassword && !s_lobbyPassword.empty())
                   ? "wrong_password"
                   : "invite";
      else if (s_access == JoinAccess::Password)
        kind = (auth && auth->gavePassword) ? "wrong_password" : "password";
      removals.push_back({p, kind});
    }
  }
  for (const PlayerRef &p : admittedNow) {
    const nlohmann::json j = {{"type", "lobby_admit"},
                              {"mid", std::to_string(p.machineId)},
                              {"account", std::to_string(p.accountId)}};
    NetworkTransport::Get().SendLobbyControlJson(j.dump());
  }
  for (const Removal &r : removals) {
    ULONGLONG &last = s_lastSent[r.player.machineId];
    if (last != 0 && now - last < 3000)
      continue;
    last = now;
    NetworkTransport::Get().SendHostRemove(r.player.machineId, r.kind, "", 0);
    NOBLE_INFO("HostModeration: '{}' may not join ({}); asked them to leave",
               r.player.name, r.kind);
  }
}

void SetJoinPassword(const std::string &password) {
  std::lock_guard<std::mutex> lock(s_mutex);
  s_joinPassword = password.substr(0, 63);
}

void OnJoinedLobby() {
  std::lock_guard<std::mutex> lock(s_mutex);
  s_joinedAt = GetTickCount64();
  s_guestAdmitted = false;
}

void GuestLobbyTick() {
  static ULONGLONG s_lastAuth = 0;
  const ULONGLONG now = GetTickCount64();
  std::string password;
  {
    std::lock_guard<std::mutex> lock(s_mutex);
    // Up to a minute after joining, every 2 s, until the host lets us in.
    if (s_joinedAt == 0 || s_guestAdmitted || now - s_joinedAt > 60000)
      return;
    if (now - s_lastAuth < 2000)
      return;
    s_lastAuth = now;
    password = s_joinPassword;
  }
  const std::string name = AuthClient::Get().IsLoggedIn()
                               ? AuthClient::Get().GetUsername()
                               : std::string();
  const nlohmann::json j = {
      {"type", "lobby_auth"},
      {"from", std::to_string(NetworkTransport::Get().GetLocalMachineId())},
      {"mid", std::to_string(MasterServerClient::GetLobbyMachineId())},
      {"account", std::to_string(LocalAccountId())},
      {"user", name},
      {"pw", password.empty() ? std::string() : PasswordHash(password)}};
  NetworkTransport::Get().SendLobbyControlJson(
      j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
}

void OnLobbyAuth(const std::string &json, uint64_t senderMachine) {
  try {
    const nlohmann::json j = nlohmann::json::parse(json);
    const uint64_t mid = ParseId(j, "mid");
    const uint64_t account = ParseId(j, "account");
    const std::string pw = j.value("pw", std::string());
    std::lock_guard<std::mutex> lock(s_mutex);
    LoadLocked();
    AuthSeen seen;
    seen.gavePassword = !pw.empty();
    seen.ok = seen.gavePassword && !s_lobbyPassword.empty() &&
              pw == PasswordHash(s_lobbyPassword);
    const bool first = mid != 0 && !s_authByMachine.count(mid);
    if (mid != 0)
      s_authByMachine[mid] = seen;
    if (senderMachine != 0 && senderMachine != mid)
      s_authByMachine[senderMachine] = seen;
    if (account > 1)
      s_authByAccount[account] = seen;
    if (first)
      NOBLE_INFO("HostModeration: join check from '{}': {}",
                 j.value("user", std::string()),
                 seen.ok ? "password right"
                         : (seen.gavePassword ? "password wrong" : "no password"));
  } catch (...) {
  }
}

void OnLobbyAdmit(const std::string &json) {
  try {
    const nlohmann::json j = nlohmann::json::parse(json);
    const uint64_t mid = ParseId(j, "mid");
    const uint64_t account = ParseId(j, "account");
    const bool me = (mid != 0 && mid == MasterServerClient::GetLobbyMachineId()) ||
                    (account > 1 && account == LocalAccountId());
    if (!me)
      return;
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_guestAdmitted)
      NOBLE_INFO("HostModeration: the host let us into the lobby");
    s_guestAdmitted = true;
  } catch (...) {
  }
}

// Not a secure hash: it keeps the password itself off the wire and out of the
// relay's logs. FNV-1a 64.
std::string PasswordHash(const std::string &password) {
  uint64_t h = 1469598103934665603ULL;
  const std::string text = "noble-lobby-pw|" + password;
  for (unsigned char c : text) {
    h ^= c;
    h *= 1099511628211ULL;
  }
  char buf[17] = {};
  std::snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(h));
  return buf;
}

std::string RemovalText(const std::string &kind, const std::string &reason,
                        int minutes) {
  if (kind == "locked")
    return "The host locked this lobby: no new players can join right now.";
  if (kind == "invite")
    return "This lobby is invite only. Ask the host to invite you.";
  if (kind == "password")
    return "This lobby needs a password. Join it from the server browser "
           "(or Direct Connect) and enter the password.";
  if (kind == "wrong_password")
    return "Wrong lobby password.";
  std::string text = kind == "ban" ? "You were banned by the host" : "You were removed by the host";
  if (kind == "ban")
    text += minutes > 0 ? " for " + std::to_string(minutes) + " more minute(s)"
                        : " (permanent)";
  text += ".";
  if (!reason.empty())
    text += " " + reason;
  return text;
}

} // namespace noble::HostModeration
