#include "HostVotes.hpp"

#include "common/Logger.hpp"
#include "network/NetworkTransport.hpp"

#include <Windows.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <set>

namespace noble::HostVotes {
namespace {

std::mutex s_mutex;
bool s_loaded = false;
int s_passPercent = 60;
bool s_hostVeto = true;
int s_cooldown = 60;
bool s_enabled = true;

constexpr int kVoteSeconds = 30;

struct Active {
  bool on = false;
  uint64_t id = 0;
  Kind kind = Kind::Kick;
  uint64_t target = 0;
  std::string targetName;
  std::string starterName;
  std::set<uint64_t> voters;
  std::map<uint64_t, bool> votes; // host side
  int yes = 0, no = 0, need = 1, voterCount = 0;
  ULONGLONG endsAt = 0;
  int myChoice = -1;
};
Active s_vote;
ULONGLONG s_cooldownUntil = 0;
struct Result {
  bool show = false, passed = false, vetoed = false;
  Kind kind = Kind::Kick;
  std::string targetName;
  ULONGLONG until = 0;
};
Result s_result;
// Host: passed votes waiting for the match code.
struct Pending {
  bool on = false;
  Kind kind = Kind::Kick;
  uint64_t target = 0;
  std::string targetName;
};
Pending s_pending;
uint64_t s_nextId = 1;
std::vector<Voter> s_lastPlayers; // host: last HostTick list

std::filesystem::path FilePath() {
  wchar_t exe[MAX_PATH] = {};
  GetModuleFileNameW(nullptr, exe, MAX_PATH);
  return std::filesystem::path(exe).parent_path() / "data" / "userdata" /
         "host_votes.json";
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
    s_passPercent = std::clamp(j.value("pass_percent", 60), 50, 100);
    s_hostVeto = j.value("host_veto", true);
    s_cooldown = std::clamp(j.value("cooldown_seconds", 60), 0, 3600);
    s_enabled = j.value("enabled", true);
  } catch (...) {
  }
}

void SaveLocked() {
  nlohmann::json j = {{"pass_percent", s_passPercent},
                      {"host_veto", s_hostVeto},
                      {"cooldown_seconds", s_cooldown},
                      {"enabled", s_enabled}};
  try {
    const auto path = FilePath();
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << j.dump(2);
  } catch (...) {
  }
}

uint64_t ParseId(const nlohmann::json &j, const char *key) {
  try {
    if (j.contains(key) && j[key].is_string())
      return std::stoull(j[key].get<std::string>(), nullptr, 0);
    if (j.contains(key) && j[key].is_number_unsigned())
      return j[key].get<uint64_t>();
  } catch (...) {
  }
  return 0;
}

uint64_t LocalId() { return NetworkTransport::Get().GetLocalMachineId(); }

void Send(const nlohmann::json &j) {
  NetworkTransport::Get().SendLobbyControlJson(
      j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
}

std::string NameOfLocked(uint64_t id) {
  for (const Voter &v : s_lastPlayers)
    if (v.machineId == id)
      return v.name;
  return {};
}

// Host: start a vote. Returns "" or why it can't start.
std::string StartLocked(Kind kind, uint64_t target, std::string targetName,
                        uint64_t starter) {
  LoadLocked();
  if (!s_enabled)
    return "The host turned votes off.";
  if (s_vote.on)
    return "A vote is already running.";
  const ULONGLONG now = GetTickCount64();
  if (now < s_cooldownUntil)
    return "Wait " + std::to_string((s_cooldownUntil - now + 999) / 1000) +
           " s before starting another vote.";
  int targetTeam = 0;
  if (kind == Kind::Kick) {
    if (target == 0 || target == NetworkTransport::kDefaultHostMachineId ||
        target == LocalId())
      return "You can't vote to kick the host.";
    bool found = false;
    for (const Voter &v : s_lastPlayers)
      if (v.machineId == target) {
        found = true;
        targetTeam = v.team;
        if (targetName.empty())
          targetName = v.name;
      }
    if (!found)
      return "That player is no longer in the game.";
  }
  std::set<uint64_t> voters;
  for (const Voter &v : s_lastPlayers) {
    if (v.machineId == 0)
      continue;
    if (kind == Kind::Kick) {
      if (v.machineId == target)
        continue;
      if (targetTeam != 0 && v.team != targetTeam)
        continue; // only the target's team votes
    }
    voters.insert(v.machineId);
  }
  if (voters.empty())
    return "Nobody can vote on that.";
  s_vote = Active{};
  s_vote.on = true;
  s_vote.id = s_nextId++;
  s_vote.kind = kind;
  s_vote.target = target;
  s_vote.targetName = targetName;
  s_vote.starterName = NameOfLocked(starter);
  s_vote.voters = voters;
  s_vote.voterCount = static_cast<int>(voters.size());
  s_vote.need = (std::max)(
      1, (s_vote.voterCount * s_passPercent + 99) / 100);
  s_vote.endsAt = now + kVoteSeconds * 1000ULL;
  // Whoever asked for the vote votes yes.
  if (voters.count(starter)) {
    s_vote.votes[starter] = true;
    s_vote.yes = 1;
    if (starter == LocalId())
      s_vote.myChoice = 1;
  }
  std::vector<std::string> ids;
  for (uint64_t v : voters)
    ids.push_back(std::to_string(v));
  Send({{"type", "vote_start"},
        {"id", std::to_string(s_vote.id)},
        {"kind", static_cast<int>(kind)},
        {"target", std::to_string(target)},
        {"name", targetName},
        {"starter", s_vote.starterName},
        {"starter_id", std::to_string(starter)},
        {"voters", ids},
        {"need", s_vote.need},
        {"seconds", kVoteSeconds}});
  Send({{"type", "vote_state"},
        {"id", std::to_string(s_vote.id)},
        {"yes", s_vote.yes},
        {"no", s_vote.no}});
  NOBLE_INFO("HostVotes: vote {} '{}' started by '{}' ({} voter(s), {} needed)",
             KindText(kind), targetName, s_vote.starterName,
             s_vote.voterCount, s_vote.need);
  return {};
}

// Host: finish the vote (passed / failed / vetoed).
void EndLocked(bool passed, bool vetoed) {
  Send({{"type", "vote_end"},
        {"id", std::to_string(s_vote.id)},
        {"passed", passed},
        {"vetoed", vetoed},
        {"yes", s_vote.yes},
        {"no", s_vote.no}});
  NOBLE_INFO("HostVotes: vote {} '{}' {} ({} yes, {} no)",
             KindText(s_vote.kind), s_vote.targetName,
             vetoed ? "vetoed" : (passed ? "passed" : "failed"), s_vote.yes,
             s_vote.no);
  if (passed && !vetoed) {
    s_pending.on = true;
    s_pending.kind = s_vote.kind;
    s_pending.target = s_vote.target;
    s_pending.targetName = s_vote.targetName;
  }
  s_result = {true, passed && !vetoed, vetoed, s_vote.kind, s_vote.targetName,
              GetTickCount64() + 5000};
  s_vote.on = false;
  s_cooldownUntil = GetTickCount64() + s_cooldown * 1000ULL;
}

void CountLocked(uint64_t voter, bool yes) {
  if (!s_vote.on || !s_vote.voters.count(voter))
    return;
  s_vote.votes[voter] = yes;
  s_vote.yes = 0;
  s_vote.no = 0;
  for (const auto &kv : s_vote.votes)
    (kv.second ? s_vote.yes : s_vote.no)++;
  if (voter == LocalId())
    s_vote.myChoice = yes ? 1 : 0;
  Send({{"type", "vote_state"},
        {"id", std::to_string(s_vote.id)},
        {"yes", s_vote.yes},
        {"no", s_vote.no}});
  if (s_vote.yes >= s_vote.need)
    EndLocked(true, false);
  else if (s_vote.no > s_vote.voterCount - s_vote.need)
    EndLocked(false, false);
}

} // namespace

const char *KindText(Kind kind) {
  switch (kind) {
  case Kind::EndRound:
    return "END ROUND";
  case Kind::EndGame:
    return "END GAME";
  default:
    return "KICK";
  }
}

int PassPercent() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return s_passPercent;
}
void SetPassPercent(int percent) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  s_passPercent = std::clamp(percent, 50, 100);
  SaveLocked();
}
bool HostVeto() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return s_hostVeto;
}
void SetHostVeto(bool on) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  s_hostVeto = on;
  SaveLocked();
}
int CooldownSeconds() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return s_cooldown;
}
void SetCooldownSeconds(int seconds) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  s_cooldown = std::clamp(seconds, 0, 3600);
  SaveLocked();
}
bool VotesEnabled() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  return s_enabled;
}
void SetVotesEnabled(bool on) {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  s_enabled = on;
  SaveLocked();
}

std::string Request(Kind kind, uint64_t targetMachine,
                    const std::string &targetName) {
  if (NetworkTransport::Get().IsHost()) {
    std::lock_guard<std::mutex> lock(s_mutex);
    const std::string why = StartLocked(kind, targetMachine, targetName, LocalId());
    if (!why.empty())
      NOBLE_INFO("HostVotes: vote not started: {}", why);
    return why;
  }
  {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_vote.on)
      return "A vote is already running.";
  }
  Send({{"type", "vote_request"},
        {"kind", static_cast<int>(kind)},
        {"target", std::to_string(targetMachine)},
        {"name", targetName},
        {"from", std::to_string(LocalId())}});
  NOBLE_INFO("HostVotes: asked the host for a vote {} '{}'", KindText(kind),
             targetName);
  return {};
}

void Cast(bool yes) {
  std::lock_guard<std::mutex> lock(s_mutex);
  if (!s_vote.on || s_vote.myChoice >= 0)
    return;
  if (NetworkTransport::Get().IsHost()) {
    CountLocked(LocalId(), yes);
    return;
  }
  s_vote.myChoice = yes ? 1 : 0;
  Send({{"type", "vote_cast"},
        {"id", std::to_string(s_vote.id)},
        {"yes", yes},
        {"from", std::to_string(LocalId())}});
}

void Veto() {
  std::lock_guard<std::mutex> lock(s_mutex);
  LoadLocked();
  if (!NetworkTransport::Get().IsHost() || !s_vote.on || !s_hostVeto)
    return;
  EndLocked(false, true);
}

void HostTick(const std::vector<Voter> &players) {
  std::lock_guard<std::mutex> lock(s_mutex);
  s_lastPlayers = players;
  if (!s_vote.on)
    return;
  // A voter who left no longer counts.
  std::set<uint64_t> present;
  for (const Voter &v : players)
    present.insert(v.machineId);
  if (s_vote.kind == Kind::Kick && !present.count(s_vote.target)) {
    EndLocked(false, false);
    return;
  }
  bool changed = false;
  for (auto it = s_vote.voters.begin(); it != s_vote.voters.end();) {
    if (!present.count(*it)) {
      s_vote.votes.erase(*it);
      it = s_vote.voters.erase(it);
      changed = true;
    } else {
      ++it;
    }
  }
  if (changed) {
    s_vote.voterCount = static_cast<int>(s_vote.voters.size());
    s_vote.need = (std::max)(1, (s_vote.voterCount * s_passPercent + 99) / 100);
    if (s_vote.voterCount == 0) {
      EndLocked(false, false);
      return;
    }
  }
  if (GetTickCount64() >= s_vote.endsAt)
    EndLocked(s_vote.yes >= s_vote.need, false);
}

bool TakeResult(Kind &kind, uint64_t &targetMachine, std::string &targetName) {
  std::lock_guard<std::mutex> lock(s_mutex);
  if (!s_pending.on)
    return false;
  kind = s_pending.kind;
  targetMachine = s_pending.target;
  targetName = s_pending.targetName;
  s_pending = Pending{};
  return true;
}

View Current() {
  std::lock_guard<std::mutex> lock(s_mutex);
  View v;
  const ULONGLONG now = GetTickCount64();
  // A guest whose host went quiet drops the vote after it should have ended.
  if (s_vote.on && now > s_vote.endsAt + 5000)
    s_vote.on = false;
  if (s_vote.on) {
    v.active = true;
    v.kind = s_vote.kind;
    v.targetName = s_vote.targetName;
    v.starterName = s_vote.starterName;
    v.yes = s_vote.yes;
    v.no = s_vote.no;
    v.need = s_vote.need;
    v.voters = s_vote.voterCount;
    v.secondsLeft = s_vote.endsAt > now
                        ? static_cast<int>((s_vote.endsAt - now + 999) / 1000)
                        : 0;
    v.canVote = s_vote.voters.count(LocalId()) != 0;
    v.myChoice = s_vote.myChoice;
  } else if (s_result.show && now < s_result.until) {
    v.showResult = true;
    v.kind = s_result.kind;
    v.targetName = s_result.targetName;
    v.passed = s_result.passed;
    v.vetoed = s_result.vetoed;
  }
  return v;
}

void OnMessage(const std::string &type, const std::string &json,
               uint64_t senderMachine, bool isHost) {
  try {
    const nlohmann::json j = nlohmann::json::parse(json);
    uint64_t from = senderMachine;
    if (from == 0)
      from = ParseId(j, "from");
    std::lock_guard<std::mutex> lock(s_mutex);
    if (isHost) {
      if (type == "vote_request") {
        const int k = j.value("kind", 0);
        if (k < 0 || k > 2)
          return;
        const std::string why =
            StartLocked(static_cast<Kind>(k), ParseId(j, "target"),
                        j.value("name", std::string()), from);
        if (!why.empty())
          NOBLE_INFO("HostVotes: vote request refused: {}", why);
      } else if (type == "vote_cast") {
        if (ParseId(j, "id") == s_vote.id)
          CountLocked(from, j.value("yes", false));
      }
      return;
    }
    // Guest: the host's vote messages.
    const uint64_t id = ParseId(j, "id");
    if (type == "vote_start") {
      if (s_vote.on && id == s_vote.id)
        return; // the same start twice (UDP and relay)
      s_vote = Active{};
      s_vote.on = true;
      s_vote.id = id;
      const int k = j.value("kind", 0);
      s_vote.kind = (k >= 0 && k <= 2) ? static_cast<Kind>(k) : Kind::Kick;
      s_vote.target = ParseId(j, "target");
      s_vote.targetName = j.value("name", std::string());
      s_vote.starterName = j.value("starter", std::string());
      for (const auto &v : j.value("voters", nlohmann::json::array())) {
        try {
          s_vote.voters.insert(std::stoull(v.get<std::string>(), nullptr, 0));
        } catch (...) {
        }
      }
      s_vote.voterCount = static_cast<int>(s_vote.voters.size());
      s_vote.need = j.value("need", 1);
      s_vote.endsAt = GetTickCount64() + j.value("seconds", kVoteSeconds) * 1000ULL;
      if (ParseId(j, "starter_id") == LocalId())
        s_vote.myChoice = 1;
    } else if (type == "vote_state") {
      if (s_vote.on && id == s_vote.id) {
        s_vote.yes = j.value("yes", 0);
        s_vote.no = j.value("no", 0);
      }
    } else if (type == "vote_end") {
      if (id == s_vote.id) {
        s_result = {true, j.value("passed", false), j.value("vetoed", false),
                    s_vote.kind, s_vote.targetName, GetTickCount64() + 5000};
        s_vote.on = false;
      }
    }
  } catch (...) {
  }
}

} // namespace noble::HostVotes
