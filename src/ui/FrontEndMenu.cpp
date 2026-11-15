#include "FrontEndInternal.hpp"

namespace noble {
namespace fe {

const VariantName kVariantNames[] = {
    {"slayer_054", "Slayer"},
    {"slayer_pro_054", "Slayer Pro"},
    {"slayer_classic_054", "Classic Slayer"},
    {"slayer_covy_054", "Elite Slayer"},
    {"swat_054", "SWAT"},
    {"ctf_054", "Capture the Flag"},
    {"ctf_1flag_054", "One Flag CTF"},
    {"ctf_multiteam_054", "Multi Flag CTF"},
    {"ctf_neutralflag_054", "Neutral Flag"},
    {"assault_054", "Assault"},
    {"assault_one_bomb_054", "One Bomb"},
    {"assault_neutral_bomb_054", "Neutral Bomb"},
    {"headhunter_054", "Headhunter"},
    {"headhunter_pro_054", "Headhunter Pro"},
    {"infection_054", "Infection"},
    {"infection_safehavens_054", "Safe Havens"},
    {"juggernaut_054", "Juggernaut"},
    {"koth_054", "King of the Hill"},
    {"koth_crazyking_054", "Crazy King"},
    {"oddball_054", "Oddball"},
    {"race_054", "Race"},
    {"rally_054", "Rally"},
    {"skeeball_default_054", "Halo Ball"},
    {"stockpile_054", "Stockpile"},
    {"territories_054", "Territories"},
    {"territories-3plot_054", "3 Plots"},
    {"territories-landgrab_054", "Land Grab"},
    {"3nvasion_054", "Invasion"},
    {"3nvasion_boneyard_054", "Invasion: Boneyard"},
    {"3nvasion_spire_054", "Invasion: Spire"},
    {"3nvasion_dlc_054", "Invasion: Breakpoint"},
    {"2nvasion_slayer_054", "Invasion Slayer"},
    {"hr_4v4_team_3plots_dmr_ar_500points_tu", "Team 3 Plots (MCC)"},
    {"hr_4v4_team_assault_dmr_ar_3points_tu", "Team Assault (MCC)"},
    {"hr_4v4_team_crazyking_dmr_ar_150points_tu", "Team Crazy King (MCC)"},
    {"hr_4v4_team_headhunter_dmr_ar_50points_tu", "Team Headhunter (MCC)"},
    {"hr_4v4_team_multiflag_dmr_ar_3points_tu", "Team Multi Flag (MCC)"},
    {"hr_4v4_team_oddball_dmr_ar_150points_tu", "Team Oddball (MCC)"},
    {"hr_4v4_team_onebomb_dmr_ar_3points_tu", "Team One Bomb (MCC)"},
    {"hr_4v4_team_oneflag_dmr_ar_4rounds_tu", "Team One Flag (MCC)"},
    {"hr_4v4_team_slayer_dmr_ar_50points_tu", "Team Slayer (MCC)"},
    {"hr_4v4_team_stockpile_dmr_ar_10points_tu", "Team Stockpile (MCC)"},
    {"hr_4v4_team_territories_dmr_ar_4rounds_3min_tu",
     "Team Territories (MCC)"},
    {"hr_ffa_escalationslayer_mixed_tu", "Escalation Slayer (MCC)"},
    {"hr_ffa_escalationslayer_mixed_vanilla", "Escalation Slayer (original)"},
    {"hr_ffa_juggernaut_ar_mag_150points_tu", "Juggernaut (MCC)"},
    {"hr_ff_classic_1set", "Classic Firefight"},
    {"hr_ff_generatordefense_1set", "Generator Defense"},
    {"hr_ff_gruntpocalypse_1round", "Gruntpocalypse"},
    {"hr_ff_rocketfight_1set", "Rocketfight"},
    {"hr_ff_scoreattack_1round", "Score Attack"},
    {"hr_ff_standard_1set", "Firefight"},
    {"hr_ff_versus_2turns", "Firefight Versus"},
};

std::wstring Lower(std::wstring s) {
  std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) {
    return static_cast<wchar_t>(std::towlower(c));
  });
  return s;
}

// A shipped variant starts with a _blf header holding its name (UTF-16, offset
// 0xC0) and description (0x1C0), checked against all 54 files in MCC's
// game_variants. Only these two strings are read: the rules need the field map.
struct VariantHeaderText {
  std::string name;
  std::string description;
};
VariantHeaderText ReadVariantHeaderText(const std::filesystem::path &path) {
  constexpr size_t kNameAt = 0xC0, kNameChars = 64, kDescriptionAt = 0x1C0,
                   kDescriptionChars = 256;
  VariantHeaderText out;
  std::ifstream file(path, std::ios::binary);
  std::vector<char> data(kDescriptionAt + kDescriptionChars * 2);
  file.read(data.data(), static_cast<std::streamsize>(data.size()));
  if (static_cast<size_t>(file.gcount()) < data.size() ||
      std::memcmp(data.data(), "_blf", 4) != 0)
    return out;
  const auto read = [&](size_t at, size_t chars) {
    std::wstring wide;
    for (size_t i = 0; i < chars; ++i) {
      const wchar_t c = static_cast<wchar_t>(
          static_cast<unsigned char>(data[at + i * 2]) |
          (static_cast<unsigned char>(data[at + i * 2 + 1]) << 8));
      if (c == 0)
        break;
      wide.push_back(c);
    }
    char text[1024] = {};
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()),
                        text, sizeof(text) - 1, nullptr, nullptr);
    return std::string(text);
  };
  out.name = read(kNameAt, kNameChars);
  out.description = read(kDescriptionAt, kDescriptionChars);
  if (out.description.find("hr_gvar_") != std::string::npos)
    out.description.clear(); // a key, not text
  return out;
}

// A known file keeps Noble's wording; any other uses the name inside the file,
// then a tidied file name.
std::string VariantLabel(const std::wstring &stem,
                         const std::string &nameInFile) {
  const std::wstring lower = Lower(stem);
  for (const VariantName &v : kVariantNames) {
    std::wstring w(v.stem, v.stem + strlen(v.stem));
    if (w == lower)
      return v.label;
  }
  if (!nameInFile.empty())
    return nameInFile;
  std::string out;
  bool up = true;
  std::wstring s = stem;
  if (s.size() > 4 && Lower(s.substr(s.size() - 4)) == L"_054")
    s.resize(s.size() - 4);
  for (wchar_t c : s) {
    if (c == L'_' || c == L'-') {
      out.push_back(' ');
      up = true;
      continue;
    }
    char ch = c < 0x80 ? static_cast<char>(c) : '?';
    out.push_back(
        up ? static_cast<char>(::toupper(static_cast<unsigned char>(ch))) : ch);
    up = false;
  }
  return out;
}

// The stock MCC folder and Noble's own writable game_variants folder. The
// engine resolves both (BlamEngine::ResolveVariantFile), so the menu must scan
// both too.
std::filesystem::path VariantBasePath(const std::filesystem::path &reachDir,
                                      const std::string &base) {
  std::error_code ec;
  const std::filesystem::path name = std::filesystem::path(base).filename();
  for (const std::filesystem::path &folder :
       {reachDir / "game_variants",
        VFS::GetUserStoragePath() / "game_variants"}) {
    const std::filesystem::path candidate = folder / name;
    if (std::filesystem::is_regular_file(candidate, ec))
      return candidate;
    ec.clear();
  }
  return {};
}

std::vector<MenuVariant> ScanVariants(const std::filesystem::path &reachDir,
                                      bool firefight) {
  std::vector<MenuVariant> list;
  std::error_code ec;
  std::set<std::wstring> seen; // the engine resolves the stock folder first, so
                               // it wins a name clash
  std::vector<std::filesystem::path> scanFolders = {
      reachDir / "game_variants", VFS::GetUserStoragePath() / "game_variants"};
  wchar_t userProfile[MAX_PATH] = {};
  if (GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH) > 0) {
    const std::filesystem::path uProf(userProfile);
    scanFolders.push_back(uProf / "AppData" / "LocalLow" / "MCC" / "Temporary" /
                          "UserContent" / "HaloReach" / "GameType");
    scanFolders.push_back(uProf / "AppData" / "LocalLow" / "MCC" /
                          "LocalVault" / "UserContent" / "HaloReach" /
                          "GameType");
    scanFolders.push_back(uProf / "AppData" / "LocalLow" / "MCC" /
                          "LocalVault" / "game_variants");
    scanFolders.push_back(uProf / "AppData" / "LocalLow" / "MCC" / "Temporary" /
                          "game_variants");
  }
  for (const std::filesystem::path &folder : scanFolders) {
    for (const auto &entry : std::filesystem::directory_iterator(folder, ec)) {
      if (!entry.is_regular_file(ec) ||
          Lower(entry.path().extension().wstring()) != L".bin")
        continue;
      const std::wstring lower = Lower(entry.path().filename().wstring());
      if (lower == Lower(kForgeVariant) || !seen.insert(lower).second)
        continue;
      const bool isFirefightVariant = lower.rfind(L"hr_ff_", 0) == 0;
      if (isFirefightVariant != firefight)
        continue;
      const VariantHeaderText text = ReadVariantHeaderText(entry.path());
      const std::wstring stem = entry.path().stem().wstring();
      // A player-made .bin often has a generic file name (or an RVT export
      // name), so the name inside the variant is the better clue. The stem
      // still wins for the shipped MCC files, except that a VIP / Action Sack /
      // Grifball / Plus name is always honoured.
      std::string type = MapGameTypes::FromVariantStem(stem);
      const std::string named = MapGameTypes::FromVariantText(text.name);
      if (!named.empty() &&
          (type.empty() || named == "vip" || named == "action_sack" ||
           named == "grifball" || named == "plus"))
        type = named;
      list.push_back({VariantLabel(stem, text.name), entry.path(),
                      text.description, false, type});
    }
    ec.clear();
  }
  // Player-made game modes: this PC's own, and any a mod ships in a game_modes
  // folder for its maps.
  std::vector<std::filesystem::path> modeFolders = {VariantRules::Directory()};
  for (const std::filesystem::path &mods :
       {VFS::GetWorkshopPath(), VFS::GetUserStoragePath() / "mods"}) {
    for (const auto &mod : std::filesystem::directory_iterator(mods, ec))
      modeFolders.push_back(mod.path() / "game_modes");
    ec.clear();
  }
  for (const std::filesystem::path &folder : modeFolders) {
    for (const auto &entry : std::filesystem::directory_iterator(folder, ec)) {
      VariantRules::Preset preset;
      if (!VariantRules::IsPresetFile(entry.path()) ||
          !VariantRules::Load(entry.path(), preset))
        continue;
      const bool baseIsFirefight =
          Lower(std::filesystem::path(preset.base).wstring())
              .rfind(L"hr_ff_", 0) == 0;
      if (baseIsFirefight != firefight ||
          VariantBasePath(reachDir, preset.base).empty())
        continue;
      MenuVariant mode;
      mode.label =
          preset.name.empty() ? entry.path().stem().string() : preset.name;
      mode.path = entry.path();
      mode.description = "Your own rules, starting from " + preset.base + ".";
      mode.custom = true;
      mode.type = MapGameTypes::FromVariantStem(
          std::filesystem::path(preset.base).stem().wstring());
      list.push_back(std::move(mode));
    }
    ec.clear();
  }
  std::sort(list.begin(), list.end(),
            [](const MenuVariant &a, const MenuVariant &b) {
              return a.label < b.label;
            });
  return list;
}

// The game type a variant path is, also for a player-made game mode (which
// takes it from its base file).
std::string VariantPathType(const std::filesystem::path &path) {
  VariantRules::Preset preset;
  if (VariantRules::IsPresetFile(path) && VariantRules::Load(path, preset))
    return MapGameTypes::FromVariantStem(
        std::filesystem::path(preset.base).stem().wstring());
  return MapGameTypes::FromVariantStem(path.stem().wstring());
}

int FindVariant(const std::vector<MenuVariant> &list, const wchar_t *file) {
  for (size_t i = 0; i < list.size(); ++i) {
    if (Lower(list[i].path.filename().wstring()) == Lower(file))
      return static_cast<int>(i);
  }
  return 0;
}

// ---------------------------------------------------------------------------------------------
// Forge maps (map variants, .mvar): Noble's data\map_variants (the Halo Customs
// megapack lives there) and the folders MCC itself saves Forge maps to. A Forge
// map is a layout laid over a stock map; the engine reads it from the launch
// block (BlamEngine, "-mvar").
// ---------------------------------------------------------------------------------------------
// A map variant's base map id -> scenario. The ids are BlamEngine's
// kMultiplayerMaps ones.
const char *ScenarioForMapId(int32_t id) {
  struct Id {
    int32_t id;
    const char *scenario;
  };
  static const Id kIds[] = {
      {1035, "50_panopticon"},     {1080, "70_boneyard"},
      {1020, "45_launch_station"}, {1055, "30_settlement"},
      {1150, "52_ivory_tower"},    {1200, "35_island"},
      {1000, "20_sword_slayer"},   {1040, "45_aftship"},
      {2001, "dlc_slayer"},        {2002, "dlc_invasion"},
      {2004, "dlc_medium"},        {1500, "condemned"},
      {1510, "trainingpreserve"},  {10020, "cex_beaver_creek"},
      {10010, "cex_damnation"},    {10030, "cex_timberland"},
      {10070, "cex_prisoner"},     {10060, "cex_hangemhigh"},
      {10050, "cex_headlong"},     {3006, "forge_halo"},
  };
  for (const Id &m : kIds)
    if (m.id == id)
      return m.scenario;
  return nullptr;
}

// The pack names its files "<prefix> <title>": the prefix becomes the heading
// and is dropped from the title.
struct ForgeGroup {
  const char *prefix;
  const char *group;
  bool strip;
};
const ForgeGroup kForgeGroups[] = {
    {"btb", "Big Team", true},
    {"inf", "Infection", true},
    {"inv", "Invasion", true},
    {"mini", "Minigames", true},
    {"race", "Race", true},
    {"slyr", "Slayer", true},
    {"obst", "Obstacle Courses", true},
    {"misc", "Other", true},
    {"hot pursuit", "Hot Pursuit", false},
    {"cops & robbers", "Cops & Robbers", false},
};

// True when `text` starts with `prefix` (case-insensitive) followed by a space,
// ':', '-' or the end.
bool StartsWithWord(const std::string &text, const char *prefix) {
  const size_t n = std::strlen(prefix);
  if (text.size() < n)
    return false;
  for (size_t i = 0; i < n; ++i) {
    if (::tolower(static_cast<unsigned char>(text[i])) != prefix[i])
      return false;
  }
  return text.size() == n || text[n] == ' ' || text[n] == ':' || text[n] == '-';
}

std::string StripForgePrefix(const std::string &text, const char *prefix) {
  size_t at = std::strlen(prefix);
  while (at < text.size() &&
         (text[at] == ' ' || text[at] == ':' || text[at] == '-'))
    ++at;
  return at < text.size() ? text.substr(at) : text;
}

// Only a PC-format map variant: "_blf", "chdr" at 0x30 and the "mvar" body at
// 0x2F0, at least 0x7329 bytes (what BlamEngine hands parse_map_variant). Xbox
// 360 exports (a compressed "_cmp" body, big-endian header text) are skipped:
// re-save them in ReachVariantTool. Header layout checked against the
// megapack's PC files (2026-11-09): creator 0x88 (ASCII), base map id 0x6C
// (int32), title 0xC0 and description 0x1C0 (UTF-16 LE).
bool ReadForgeMap(const std::filesystem::path &path, ForgeMap &out) {
  constexpr size_t kSize = 0x7329;
  std::ifstream file(path, std::ios::binary);
  std::vector<unsigned char> d(kSize);
  file.read(reinterpret_cast<char *>(d.data()),
            static_cast<std::streamsize>(d.size()));
  if (static_cast<size_t>(file.gcount()) < kSize)
    return false;
  if (std::memcmp(d.data(), "_blf", 4) != 0 ||
      std::memcmp(d.data() + 0x30, "chdr", 4) != 0 ||
      std::memcmp(d.data() + 0x2F0, "mvar", 4) != 0)
    return false;
  if (d[0xC0] == 0 && d[0xC1] != 0)
    return false; // big-endian title: an Xbox 360 export
  int32_t mapId = 0;
  std::memcpy(&mapId, d.data() + 0x6C, sizeof(mapId));
  const char *scenario = ScenarioForMapId(mapId);
  if (!scenario)
    return false;
  const auto utf16 = [&](size_t at, size_t chars) {
    std::wstring wide;
    for (size_t i = 0; i < chars; ++i) {
      const wchar_t c =
          static_cast<wchar_t>(d[at + i * 2] | (d[at + i * 2 + 1] << 8));
      if (c == 0)
        break;
      wide.push_back(c);
    }
    char text[1024] = {};
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), static_cast<int>(wide.size()),
                        text, sizeof(text) - 1, nullptr, nullptr);
    return std::string(text);
  };
  out = {};
  out.path = path;
  out.scenario = scenario;
  out.title = utf16(0xC0, 64);
  out.description = utf16(0x1C0, 128);
  for (size_t i = 0x88; i < 0x98 && d[i] >= 0x20 && d[i] < 0x7F; ++i)
    out.creator.push_back(static_cast<char>(d[i]));
  if (out.title.empty() || out.title[0] == '$')
    out.title = path.stem().string(); // a string key, not a title
  if (out.description.rfind("$", 0) == 0)
    out.description.clear();
  const std::string stem = path.stem().string();
  out.group = "Unsorted"; // no pack prefix (e.g. a player's own saved map)
  for (const ForgeGroup &g : kForgeGroups) {
    if (!StartsWithWord(stem, g.prefix))
      continue;
    out.group = g.group;
    if (g.strip && StartsWithWord(out.title, g.prefix))
      out.title = StripForgePrefix(out.title, g.prefix);
    break;
  }
  return true;
}

// A map variant saved by Noble is JSON:
// {"version","name","base_map_id","mode","data":"<base64>"} where data is the
// variant in the engine's own BLF/"mvar" container. Unpack it into <stem>.mvar
// next to the JSON (the form the engine mounts through "-mvar"), reusing the
// file when it is already current.
bool UnpackJsonForgeMap(const std::filesystem::path &jsonPath,
                        std::filesystem::path &outMvar) {
  std::error_code ec;
  std::ifstream file(jsonPath, std::ios::binary);
  if (!file)
    return false;
  nlohmann::json j;
  try {
    j = nlohmann::json::parse(file);
  } catch (...) {
    return false;
  }
  const std::string encoded = j.value("data", std::string());
  if (encoded.empty())
    return false;

  static const auto value = [](char c) -> int {
    if (c >= 'A' && c <= 'Z')
      return c - 'A';
    if (c >= 'a' && c <= 'z')
      return c - 'a' + 26;
    if (c >= '0' && c <= '9')
      return c - '0' + 52;
    if (c == '+')
      return 62;
    if (c == '/')
      return 63;
    return -1;
  };
  std::vector<unsigned char> bytes;
  bytes.reserve(encoded.size() / 4 * 3 + 3);
  uint32_t buffer = 0;
  int bits = 0;
  for (char c : encoded) {
    const int v = value(c);
    if (v < 0)
      continue;
    buffer = (buffer << 6) | static_cast<uint32_t>(v);
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      bytes.push_back(static_cast<unsigned char>((buffer >> bits) & 0xFF));
    }
  }
  if (bytes.size() < 0x7329)
    return false;

  outMvar = jsonPath;
  outMvar.replace_extension(L".mvar");
  bool current = std::filesystem::exists(outMvar, ec) &&
                 std::filesystem::file_size(outMvar, ec) == bytes.size() &&
                 std::filesystem::last_write_time(outMvar, ec) >=
                     std::filesystem::last_write_time(jsonPath, ec);
  if (current)
    return true;
  std::ofstream out(outMvar, std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char *>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
  return static_cast<bool>(out);
}

std::vector<ForgeMap> ScanForgeMaps() {
  std::vector<ForgeMap> list;
  std::vector<std::filesystem::path> folders = {VFS::GetUserStoragePath() /
                                                "map_variants"};
  // MCC ships its own Forge map variants with the game: haloreachmap_variants
  // (the maps' defaults) and haloreachhopper_map_variants (the matchmaking
  // hopper layouts, 266 files). Listing them makes every base variant playable
  // and gives a large, known-good set to check loading/saving against.
  const std::filesystem::path reachDir = VFS::GetBaseMccPath();
  folders.push_back(reachDir / "map_variants");
  folders.push_back(reachDir / "hopper_map_variants");
  wchar_t userProfile[MAX_PATH] = {};
  if (GetEnvironmentVariableW(L"USERPROFILE", userProfile, MAX_PATH) > 0) {
    const std::filesystem::path uProf(userProfile);
    folders.push_back(uProf / "AppData" / "LocalLow" / "MCC" / "Temporary" /
                      "UserContent" / "HaloReach" / "Map");
    folders.push_back(uProf / "AppData" / "LocalLow" / "MCC" / "LocalVault" /
                      "UserContent" / "HaloReach" / "Map");
  }
  std::set<std::wstring> seen;
  std::error_code ec;
  int skipped = 0;
  for (const std::filesystem::path &folder : folders) {
    // A saved variant exists twice: the JSON (source of truth) and the .mvar
    // unpacked next to it for the engine. Walk the JSONs first and let them
    // claim the stem, otherwise the stale .mvar can be listed and the saved
    // variant is never seen (and never unpacked) until something else refreshes
    // it.
    std::vector<std::filesystem::directory_entry> entries;
    for (const auto &entry : std::filesystem::directory_iterator(folder, ec))
      entries.push_back(entry);
    std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b) {
      const bool aj = Lower(a.path().extension().wstring()) == L".json";
      const bool bj = Lower(b.path().extension().wstring()) == L".json";
      if (aj != bj)
        return aj;
      return a.path().wstring() < b.path().wstring();
    });
    for (const auto &entry : entries) {
      if (!entry.is_regular_file(ec))
        continue;
      const std::wstring ext = Lower(entry.path().extension().wstring());
      if (ext != L".mvar" && ext != L".json")
        continue;
      if (!seen.insert(Lower(entry.path().stem().wstring())).second)
        continue;
      std::filesystem::path variantFile = entry.path();
      // A saved map variant is JSON (base64 payload + name + base map): unpack
      // it next to itself as the .mvar the engine mounts, and read the metadata
      // from that.
      if (ext == L".json") {
        std::filesystem::path unpacked;
        if (!UnpackJsonForgeMap(entry.path(), unpacked)) {
          ++skipped;
          continue;
        }
        variantFile = unpacked;
      }
      ForgeMap map;
      if (ReadForgeMap(variantFile, map)) {
        map.path = variantFile; // the engine gets the .mvar; the JSON stays the
                                // source of truth
        list.push_back(std::move(map));
      } else {
        ++skipped;
      }
    }
    ec.clear();
  }
  std::sort(list.begin(), list.end(), [](const ForgeMap &a, const ForgeMap &b) {
    return a.group != b.group ? a.group < b.group : a.title < b.title;
  });
  NOBLE_INFO("FrontEndMenu: {} Forge map(s) listed, {} skipped (not a "
             "PC-format map variant)",
             list.size(), skipped);
  return list;
}

// ---------------------------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------------------------
ImFont *Font(ImFont *f) { return f ? f : ImGui::GetFont(); }

float TextWidth(ImFont *font, float px, const char *text) {
  return Font(font)->CalcTextSizeA(px, FLT_MAX, 0.0f, text).x;
}

// "19620000" -> "19,620,000" (the Armory's credit balance next to the player
// card).
std::string NumberWithCommas(int64_t v) {
  const bool negative = v < 0;
  std::string d = std::to_string(negative ? -v : v), out;
  for (size_t i = 0; i < d.size(); ++i) {
    if (i && (d.size() - i) % 3 == 0)
      out += ',';
    out += d[i];
  }
  return negative ? "-" + out : out;
}

void Text(const Ui &ui, ImFont *font, float px, ImVec2 pos, ImU32 col,
          const char *text, float wrapRight) {
  ui.dl->AddText(Font(font), px * ui.s, pos, col, text, nullptr,
                 wrapRight > pos.x ? wrapRight - pos.x : 0.0f);
}

// One-line label that fits |width| pixels: cuts at a word boundary and appends
// "...". Menu rows and the banner title use this instead of running under the
// next column / off the art.
std::string Ellipsize(ImFont *font, float px, const std::string &text,
                      float width) {
  if (width <= 0.0f)
    return std::string();
  ImFont *f = Font(font);
  if (f->CalcTextSizeA(px, FLT_MAX, 0.0f, text.c_str()).x <= width)
    return text;
  const std::string dots = "...";
  const float dotsW = f->CalcTextSizeA(px, FLT_MAX, 0.0f, dots.c_str()).x;
  size_t lo = 0, hi = text.size();
  while (lo < hi) {
    const size_t mid = (lo + hi + 1) / 2;
    const std::string part = text.substr(0, mid);
    if (f->CalcTextSizeA(px, FLT_MAX, 0.0f, part.c_str()).x + dotsW <= width)
      lo = mid;
    else
      hi = mid - 1;
  }
  std::string cut = text.substr(0, lo);
  const size_t space = cut.find_last_of(' ');
  if (space != std::string::npos && space > cut.size() / 2)
    cut.resize(space);
  while (!cut.empty() && (cut.back() == ' ' || cut.back() == '\t'))
    cut.pop_back();
  return cut + dots;
}

// Word-aware fit for the one/two-line description under a map picture: keeps
// whole words, caps at |maxHeight| pixels and ends the last line with "...".
// Can't overflow into the option rows.
std::string FitWrapped(ImFont *font, float px, const std::string &text,
                       float width, float maxHeight) {
  if (text.empty() || width <= 0.0f || maxHeight <= 0.0f)
    return std::string();
  ImFont *f = Font(font);
  std::string body = text;
  auto heightOf = [&](const std::string &s) {
    return f->CalcTextSizeA(px, FLT_MAX, width, s.c_str()).y;
  };
  if (heightOf(body) <= maxHeight)
    return body;
  for (int guard = 0; guard < 4096 && !body.empty(); ++guard) {
    const size_t space = body.find_last_of(' ');
    if (space == std::string::npos)
      break;
    body.resize(space);
    while (!body.empty() && (body.back() == ' ' || body.back() == '\t'))
      body.pop_back();
    std::string candidate = body + "...";
    if (heightOf(candidate) <= maxHeight)
      return candidate;
  }
  return Ellipsize(font, px, text, width);
}

std::string Upper(std::string s) {
  for (char &c : s)
    c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
  return s;
}

// Workshop descriptions are written for a Steam page: BBCode tags, [h1]Map
// List[/h1] sections, hard line breaks and sometimes thousands of characters. A
// menu row shows two lines at most, so tags are dropped, whitespace is
// collapsed and the text is cut back to a whole word on a "...".
std::string PlainDescription(const std::string &in, size_t maxChars) {
  std::string out;
  out.reserve(std::min(in.size(), maxChars + 4));
  bool pendingSpace = false;
  for (size_t i = 0; i < in.size() && out.size() < maxChars; ++i) {
    const char c = in[i];
    if (c == '[') { // BBCode / Steam markup tag, e.g. [h1] or [/b]
      const size_t close = in.find(']', i + 1);
      if (close != std::string::npos && close - i <= 40) {
        i = close;
        pendingSpace = !out.empty();
        continue;
      }
    }
    if (c == '\r' || c == '\n' || c == '\t' || c == ' ') {
      pendingSpace = !out.empty();
      continue;
    }
    if (pendingSpace) {
      out.push_back(' ');
      pendingSpace = false;
    }
    out.push_back(c);
  }
  if (out.size() >= maxChars) {
    const size_t cut = out.find_last_of(' ');
    if (cut != std::string::npos && cut > maxChars / 2)
      out.resize(cut);
    out += "...";
  }
  return out;
}

// Height of the description under a map picture: one or two lines plus the gaps
// above and below. The setup screens reserve exactly this and HeroBanner draws
// exactly this, so a long mod blurb cannot push the option rows over the
// network strip (2026-10-30).
float BlurbBlockHeight(const Ui &ui, float width, const char *blurb) {
  if (!blurb || !*blurb)
    return 0.0f;
  const float lineH = 17.0f * ui.s * 1.35f;
  const float textH =
      Font(nullptr)->CalcTextSizeA(17.0f * ui.s, FLT_MAX, width, blurb).y;
  return std::min(textH, lineH * 2.0f) + 16.0f * ui.s;
}

std::string s_lobbyBackdropMap; // set each frame from the lobby state
std::filesystem::path
    s_lobbyBackdropImage; // mod map art for the pre-game backdrop

// The Armory's presentation. The engine draws the model from a fixed scene
// camera. Noble copies the finished frame and places it on the right:
// whole-body cameras are enlarged around the character (the menu renders 1.5x,
// so this never upscales what the window shows), while per-slot close-ups keep
// the old fit-under-the-rule placement. A soft additive floor light lifts the
// black chamber below the model.
namespace ArmoryStage {

ID3D11RenderTargetView *s_frameRtv =
    nullptr; // the back buffer holding this frame's engine image; null when
             // there is none
ID3D11Texture2D *s_copy = nullptr;
ID3D11ShaderResourceView *s_copySrv = nullptr;
UINT s_copyW = 0, s_copyH = 0;
DXGI_FORMAT s_copyFormat = DXGI_FORMAT_UNKNOWN;
ID3D11BlendState *s_opaque = nullptr;
ID3D11BlendState *s_additive = nullptr;
ID3D11DeviceContext *s_context = nullptr;
float s_windowScale =
    1.0f; // back-buffer pixels per window pixel (menu supersampling)
bool s_stageHeld = false; // this frame shows the held picture (and its depth)
                          // instead of the engine's

void UseOpaque(const ImDrawList *, const ImDrawCmd *) {
  if (s_context)
    s_context->OMSetBlendState(
        s_opaque, nullptr, 0xFFFFFFFF); // the engine's alpha is not coverage
}
void UseAdditive(const ImDrawList *, const ImDrawCmd *) {
  if (s_context)
    s_context->OMSetBlendState(s_additive, nullptr, 0xFFFFFFFF);
}

// ---------------------------------------------------------------------------------------------------------------
// Armory backdrop: a procedural glowing diamond lattice (after the "Auntie Dot"
// Reach dreamscene). Drawn by a pixel shader at full resolution, so the lines
// stay sharp at 1080p/1440p/4K. A wave of light spreads out from the centre
// along the lattice to the screen edges and back. The engine's frame is
// composited on top: where the engine image is black (the empty chamber around
// the character) the lattice shows; on the character it does not.
// ---------------------------------------------------------------------------------------------------------------
const char *const kBackdropPs = R"HLSL(
cbuffer Params : register(b0) {
    float2 screenSize;   // the menu rectangle, menu units
    float2 framePos;     // engine frame placement inside it
    float2 frameSize;
    float2 center;       // lattice centre
    float  time;
    float  frameAlpha;   // 0..1 fade-in of the engine frame
    float  hasFrame;
    float  pad;
    float  clearDepth;   // the value the engine cleared its depth buffer to: untouched = empty background
    float  hasDepth;
    float  depthW;
    float  depthH;
    float  depthThreshold;   // > 0: nearer than this = character (measured on the CPU from a depth sample grid)
    float  pad1, pad2, pad3;
};
Texture2D frameTex : register(t0);
Texture2D<float> depthTex : register(t1);
SamplerState samp : register(s0);
struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };

float Luma(float3 c) { return dot(c, float3(0.299, 0.587, 0.114)); }

float3 Lattice(float2 px) {
    float H = screenSize.y;
    float2 q = (px - center) / H;
    float2 r = float2(q.x + q.y, q.y - q.x) * 0.70710678;   // 45 degrees: the bars become axis-aligned
    float cell = 0.078;                                       // bar spacing, in screen heights
    float2 g = r / cell;
    float2 id = floor(g);
    float2 f = g - id;

    // The wave: 0 -> 1 -> 0 over T seconds. Its radius grows from the centre to past the screen corners.
    float T = 36.0;                                                 // one full out-and-back cycle, seconds
    float w = 0.5 - 0.5 * cos(6.2831853 * time / T);
    float R = lerp(0.10, 1.55, w * w * (3.0 - 2.0 * w));

    float3 acc = 0;
    [unroll] for (int fam = 0; fam < 2; ++fam) {
        float across = fam == 0 ? f.y : f.x;
        float along  = fam == 0 ? f.x : f.y;
        float nearHi = across > 0.5 ? 1.0 : 0.0;
        float dLine = min(across, 1.0 - across) * cell;
        float lineIdx = (fam == 0 ? id.y : id.x) + nearHi;
        float gap = 0.17;                                           // dark node at each crossing
        float inside = min(along - gap, 1.0 - gap - along) * cell;  // > 0 inside the bar
        float dist = length(float2(max(-inside, 0.0), dLine));
        // Each bar lights as a whole when the wave reaches its middle, so the light follows the lattice.
        float2 mid = fam == 0 ? float2(id.x + 0.5, lineIdx) : float2(lineIdx, id.y + 0.5);
        float d = max(abs(mid.x), abs(mid.y)) * cell;              // diamond rings, like the lattice
        float lit = saturate((R - d) / 0.09 + 0.5);
        float front = exp(-((d - R) / 0.07) * ((d - R) / 0.07));
        float fall = lerp(1.0, 0.22, saturate(d / 1.3));   // outer bars dimmer: the character stays the focus
        float intensity = lit * fall + front * 0.55;
        float core = smoothstep(0.0042, 0.0012, dist);
        float glow = exp(-dist / 0.010) * 0.50 + exp(-dist / 0.040) * 0.10;
        acc += float3(0.58, 0.80, 0.95) * core * intensity * 0.81;
        acc += float3(0.08, 0.34, 0.85) * glow * intensity * 0.70;
        acc += float3(0.07, 0.09, 0.12) * core * (1.0 - lit);   // unlit bars stay faintly visible
    }
    float3 bg = float3(0.010, 0.014, 0.022) * (1.0 - 0.6 * saturate(length(q) / 1.3));
    // Soft shoulder instead of a hard clip, so stacked glows never burn out to flat white-blue.
    acc = acc / (1.0 + 0.35 * acc);
    return bg + acc;
}

float4 main(PS_INPUT input) : SV_Target {
    float2 px = input.uv * screenSize;
    float3 lat = Lattice(px);
    float3 outc = lat;
    if (hasFrame > 0.5) {
        float2 fuv = (px - framePos) / frameSize;
        if (all(fuv >= 0.0) && all(fuv <= 1.0)) {
            float3 fc = frameTex.Sample(samp, fuv).rgb;
            float keep;
            if (hasDepth > 0.5) {
                // Exact: the character is the geometry NEARER than the room. The engine uses reversed depth (cleared
                // to 0 = infinitely far, bigger = nearer). The room behind the model (a back wall fills the top half of
                // the frame) is measured at the frame's left and right edges, where the character never is; anything
                // clearly nearer than that is the character. 3x3 average for a soft, anti-aliased outline.
                int2 dmax = int2(depthW - 1, depthH - 1);
                // The room = the FARTHEST thing seen at the frame's edges (smallest reversed depth that is not empty).
                // In close-ups the character itself reaches some edges; taking the farthest edge sample still finds
                // the wall wherever any edge shows it. No geometry at the edges at all: everything drawn is character.
                float room = 1e30;
                [unroll] for (int ex = 0; ex < 2; ++ex) {
                    [unroll] for (int ey = 0; ey < 6; ++ey) {
                        float e = depthTex.Load(int3(int2((ex == 0 ? 0.015 : 0.985) * depthW, (0.03 + 0.18 * ey) * depthH), 0));
                        if (e > clearDepth + 1e-7) room = min(room, e);
                    }
                }
                [unroll] for (int tx = 1; tx < 6; ++tx) {
                    float e = depthTex.Load(int3(int2((tx / 6.0) * depthW, 0.015 * depthH), 0));
                    if (e > clearDepth + 1e-7) room = min(room, e);
                }
                if (room > 1e29) room = clearDepth;
                float threshold = depthThreshold > 0.0 ? depthThreshold : room * 1.25 + 1e-6;
                int2 c = int2(fuv * float2(depthW, depthH));
                float cover = 0.0;
                [unroll] for (int dy = -1; dy <= 1; ++dy) {
                    [unroll] for (int dx = -1; dx <= 1; ++dx) {
                        int2 q = clamp(c + int2(dx, dy), int2(0, 0), dmax);
                        cover += depthTex.Load(int3(q, 0)) > threshold ? 1.0 : 0.0;
                    }
                }
                keep = 1.0 - cover / 9.0;
            } else {
                // Where is the character? Brightness alone can't tell: dark armour (an Elite's teal plates, a black
                // undersuit) is as dark as the empty room, so a pure brightness key let the lattice show through the
                // model. Instead the lattice is hidden wherever clearly lit armour is NEAR: three rings of samples
                // (about 1%, 2.5% and 4.5% of the screen height). Dark parts sit inside or next to lit ones, so they
                // are covered too; away from the model the lattice comes back gradually, like a soft shadow behind him.
                keep = 1.0 - smoothstep(0.05, 0.14, Luma(fc));
                float2 unit = float2(screenSize.y / frameSize.x, screenSize.y / frameSize.y);   // 1 screen-height in frame uv
                static const float radii[3]   = { 0.010, 0.025, 0.045 };
                static const float weights[3] = { 1.00, 0.85, 0.55 };
                [unroll] for (int ring = 0; ring < 3; ++ring) {
                    float lit = 0.0;
                    [unroll] for (int k = 0; k < 8; ++k) {
                        float ang = 6.2831853 * (k + 0.5 * ring) / 8.0;
                        float2 off = float2(cos(ang), sin(ang)) * radii[ring] * unit;
                        lit = max(lit, smoothstep(0.06, 0.16, Luma(frameTex.SampleLevel(samp, fuv + off, 0).rgb)));
                    }
                    keep *= 1.0 - lit * weights[ring];
                }
            }
            outc = fc * frameAlpha + lat * lerp(1.0, keep, frameAlpha);
        }
    }
    return float4(outc, 1.0);
}
)HLSL";

struct BackdropParams {
  float screenW, screenH, frameX, frameY, frameW, frameH, centerX, centerY;
  float time, frameAlpha, hasFrame, pad;
  float clearDepth, hasDepth, depthW, depthH;
  float depthThreshold, pad1, pad2, pad3;
};
static_assert(sizeof(BackdropParams) % 16 == 0, "constant buffer size");
ID3D11PixelShader *s_backdropPs = nullptr;
ID3D11Buffer *s_backdropCb = nullptr;
ID3D11ShaderResourceView *s_blankSrv =
    nullptr; // bound while there is no engine frame (the shader ignores it)
ID3D11ShaderResourceView *s_depthSrv =
    nullptr; // engine depth copy for the lattice mask (see CaptureEngineDepth)
bool s_backdropFailed = false;
BackdropParams s_backdropParams{};

bool EnsureBackdrop(ID3D11Device *device) {
  if (s_backdropPs && s_backdropCb && s_opaque)
    return true;
  if (s_backdropFailed || !device)
    return false;
  if (!s_opaque) {
    D3D11_BLEND_DESC blend{};
    blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blend, &s_opaque);
  }
  ID3DBlob *code = nullptr;
  ID3DBlob *errors = nullptr;
  const HRESULT hr =
      D3DCompile(kBackdropPs, strlen(kBackdropPs), "noble_armory_backdrop",
                 nullptr, nullptr, "main", "ps_4_0",
                 D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &code, &errors);
  if (FAILED(hr) || !code) {
    NOBLE_WARN("FrontEndMenu: Armory backdrop shader failed to compile: {}",
               errors ? std::string(static_cast<const char *>(
                                        errors->GetBufferPointer()),
                                    errors->GetBufferSize())
                      : "?");
    if (errors)
      errors->Release();
    if (code)
      code->Release();
    s_backdropFailed = true;
    return false;
  }
  if (errors)
    errors->Release();
  const bool okPs = SUCCEEDED(device->CreatePixelShader(
      code->GetBufferPointer(), code->GetBufferSize(), nullptr, &s_backdropPs));
  code->Release();
  D3D11_BUFFER_DESC cb{};
  cb.ByteWidth = sizeof(BackdropParams);
  cb.Usage = D3D11_USAGE_DYNAMIC;
  cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  const bool okCb =
      SUCCEEDED(device->CreateBuffer(&cb, nullptr, &s_backdropCb));
  {
    const uint32_t black = 0xFF000000u;
    D3D11_TEXTURE2D_DESC td{};
    td.Width = td.Height = 1;
    td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA init{&black, 4, 4};
    ID3D11Texture2D *tex = nullptr;
    if (SUCCEEDED(device->CreateTexture2D(&td, &init, &tex)) && tex) {
      device->CreateShaderResourceView(tex, nullptr, &s_blankSrv);
      tex->Release();
    }
  }
  if (!okPs || !okCb || !s_opaque || !s_blankSrv) {
    NOBLE_WARN("FrontEndMenu: Armory backdrop could not be created; using the "
               "plain black chamber");
    s_backdropFailed = true;
    return false;
  }
  NOBLE_INFO("FrontEndMenu: Armory lattice backdrop ready");
  return true;
}

void UseBackdrop(const ImDrawList *, const ImDrawCmd *) {
  if (!s_context || !s_backdropPs || !s_backdropCb)
    return;
  D3D11_MAPPED_SUBRESOURCE mapped{};
  if (SUCCEEDED(s_context->Map(s_backdropCb, 0, D3D11_MAP_WRITE_DISCARD, 0,
                               &mapped))) {
    std::memcpy(mapped.pData, &s_backdropParams, sizeof(s_backdropParams));
    s_context->Unmap(s_backdropCb, 0);
  }
  s_context->PSSetShader(s_backdropPs, nullptr, 0);
  s_context->PSSetConstantBuffers(0, 1, &s_backdropCb);
  if (s_backdropParams.hasDepth > 0.5f && s_depthSrv)
    s_context->PSSetShaderResources(1, 1, &s_depthSrv);
  s_context->OMSetBlendState(s_opaque, nullptr, 0xFFFFFFFF);
}

// "LOADING..." in the middle of the stage while the menu map starts and the
// character is dressed.
void DrawLoading(const Ui &ui, ImVec2 centre) {
  const char *const kFull = "LOADING...";
  const float px = 22.0f * ui.s;
  const float fullW = TextWidth(NobleStyle::TitleFont(), px, kFull);
  const int dots = static_cast<int>(ImGui::GetTime() * 2.5) % 4;
  const std::string text =
      std::string("LOADING") + std::string(static_cast<size_t>(dots), '.');
  const ImVec2 padding(22.0f * ui.s, 10.0f * ui.s);
  ui.dl->AddRectFilled(ImVec2(centre.x - fullW * 0.5f - padding.x,
                              centre.y - px * 0.5f - padding.y),
                       ImVec2(centre.x + fullW * 0.5f + padding.x,
                              centre.y + px * 0.5f + padding.y),
                       IM_COL32(2, 6, 12, 150),
                       6.0f * ui.s); // ~20% more see-through than before
  Text(ui, NobleStyle::TitleFont(), 22.0f,
       ImVec2(centre.x - fullW * 0.5f, centre.y - px * 0.5f),
       IM_COL32(236, 240, 244, 245), text.c_str());
}

// ---------------------------------------------------------------------------------------------------------------
// Engine depth capture for the Armory lattice. The depth buffer is not bound
// any more when Noble draws, so the engine's ClearDepthStencilView is hooked:
// during each engine frame the biggest single-sample depth buffer it clears is
// remembered (with the value it was cleared to), and at UI time Noble copies
// it. Pixels still holding the clear value are empty background; everything
// else is the character (or other geometry).
// ---------------------------------------------------------------------------------------------------------------
using ClearDsvFn = void(STDMETHODCALLTYPE *)(ID3D11DeviceContext *,
                                             ID3D11DepthStencilView *, UINT,
                                             FLOAT, UINT8);
ClearDsvFn s_origClearDsv = nullptr;
std::mutex s_depthMutex;
ID3D11Texture2D *s_frameDepth = nullptr; // chosen this engine frame (AddRef'd)
float s_frameDepthClear = 1.0f;
int s_frameDepthClears = 0;
bool s_frameDepthAspectOk = false; // the chosen depth matches the frame's
                                   // aspect (not a square shadow map)
double s_screenAspect =
    0.0; // swap-chain aspect, used to recognise the scene depth buffer
ID3D11Texture2D *s_sceneDepth =
    nullptr; // the persistent scene depth buffer (AddRef'd), once identified
float s_sceneDepthClear = 0.0f;
std::string
    s_frameDepthLog; // every depth clear this frame, for the diagnostic log
bool s_depthHookTried = false;
ID3D11Texture2D *s_depthCopy = nullptr;
D3D11_TEXTURE2D_DESC s_depthCopyDesc{};
float s_depthClear = 1.0f;
bool s_depthValid = false;

void STDMETHODCALLTYPE HookClearDsv(ID3D11DeviceContext *ctx,
                                    ID3D11DepthStencilView *dsv, UINT flags,
                                    FLOAT depth, UINT8 stencil) {
  if (dsv && (flags & D3D11_CLEAR_DEPTH)) {
    ID3D11Resource *res = nullptr;
    dsv->GetResource(&res);
    ID3D11Texture2D *tex = nullptr;
    if (res)
      res->QueryInterface(__uuidof(ID3D11Texture2D),
                          reinterpret_cast<void **>(&tex));
    if (res)
      res->Release();
    if (tex) {
      D3D11_TEXTURE2D_DESC d{};
      tex->GetDesc(&d);
      std::lock_guard<std::mutex> lock(s_depthMutex);
      ++s_frameDepthClears;
      if (s_frameDepthLog.size() < 400) {
        s_frameDepthLog +=
            std::format(" {}x{}/f{}/s{}/c{:.1f}", d.Width, d.Height,
                        static_cast<int>(d.Format), d.SampleDesc.Count, depth);
      }
      D3D11_TEXTURE2D_DESC best{};
      if (s_frameDepth)
        s_frameDepth->GetDesc(&best);
      // The scene depth must match the screen's aspect ratio. Larger depth
      // buffers are often square shadow maps (seen: a 1600x1600 R32G8X24 shadow
      // pass outranking the 1920x1080 D24S8 scene depth), and using one as the
      // lattice mask drew the lattice over the character. Among matching ones
      // the biggest wins; with no aspect yet, depth formats (D24S8 / D32)
      // outrank typeless depth-stencil (shadow) formats.
      if (d.SampleDesc.Count == 1 && d.ArraySize == 1) {
        const double candidateAspect =
            static_cast<double>(d.Width) / static_cast<double>(d.Height);
        const bool aspectOk =
            s_screenAspect > 0.0 &&
            std::abs(candidateAspect - s_screenAspect) <= s_screenAspect * 0.02;
        auto formatRank = [](DXGI_FORMAT f) {
          switch (f) {
          case DXGI_FORMAT_D24_UNORM_S8_UINT:
          case DXGI_FORMAT_D32_FLOAT:
          case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            return 2;                         // scene depth formats
          case DXGI_FORMAT_R32G8X24_TYPELESS: // shadow passes use typeless
                                              // depth-stencil
          case DXGI_FORMAT_R24G8_TYPELESS:
            return 0;
          default:
            return 1;
          }
        };
        const uint64_t area = static_cast<uint64_t>(d.Width) * d.Height;
        const uint64_t bestArea =
            static_cast<uint64_t>(best.Width) * best.Height;
        // Keep the scene depth across frames: the engine does not clear it
        // every frame, and re-ranking each frame let a shadow map win on the
        // frames where only the shadow pass was cleared.
        if (aspectOk && s_sceneDepth != tex) {
          if (s_sceneDepth)
            s_sceneDepth->Release();
          s_sceneDepth = tex;
          s_sceneDepth->AddRef();
          s_sceneDepthClear = depth;
          NOBLE_INFO("ArmoryDepth: scene depth is {}x{} (format {}, cleared to "
                     "{:.1f})",
                     d.Width, d.Height, static_cast<int>(d.Format), depth);
        }
        bool better = false;
        if (aspectOk != s_frameDepthAspectOk) {
          better = aspectOk;
        } else {
          const int candidateRank = formatRank(d.Format);
          const int bestRank = formatRank(best.Format);
          better = candidateRank != bestRank ? candidateRank > bestRank
                                             : area > bestArea;
        }
        if (better) {
          if (s_frameDepth)
            s_frameDepth->Release();
          s_frameDepth = tex;
          s_frameDepth->AddRef();
          s_frameDepthClear = depth;
          s_frameDepthAspectOk = aspectOk;
        }
      }
      tex->Release();
    }
  }
  s_origClearDsv(ctx, dsv, flags, depth, stencil);
}

void InstallDepthHook(ID3D11DeviceContext *context) {
  if (s_depthHookTried || !context)
    return;
  s_depthHookTried = true;
  void **vtable = *reinterpret_cast<void ***>(context);
  void *target = vtable[53]; // ID3D11DeviceContext::ClearDepthStencilView
  if (!HookManager::InstallHook(target, reinterpret_cast<void *>(&HookClearDsv),
                                reinterpret_cast<void **>(&s_origClearDsv),
                                "D3D11 ClearDepthStencilView (Armory depth)")) {
    NOBLE_WARN("ArmoryDepth: could not hook ClearDepthStencilView; the lattice "
               "uses the brightness mask");
  }
}

DXGI_FORMAT TypelessOf(DXGI_FORMAT f) {
  switch (f) {
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
  case DXGI_FORMAT_R24G8_TYPELESS:
    return DXGI_FORMAT_R24G8_TYPELESS;
  case DXGI_FORMAT_D32_FLOAT:
  case DXGI_FORMAT_R32_TYPELESS:
    return DXGI_FORMAT_R32_TYPELESS;
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
  case DXGI_FORMAT_R32G8X24_TYPELESS:
    return DXGI_FORMAT_R32G8X24_TYPELESS;
  case DXGI_FORMAT_D16_UNORM:
  case DXGI_FORMAT_R16_TYPELESS:
    return DXGI_FORMAT_R16_TYPELESS;
  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}
DXGI_FORMAT ReadFormatOf(DXGI_FORMAT typeless) {
  switch (typeless) {
  case DXGI_FORMAT_R24G8_TYPELESS:
    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  case DXGI_FORMAT_R32_TYPELESS:
    return DXGI_FORMAT_R32_FLOAT;
  case DXGI_FORMAT_R32G8X24_TYPELESS:
    return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
  case DXGI_FORMAT_R16_TYPELESS:
    return DXGI_FORMAT_R16_UNORM;
  default:
    return DXGI_FORMAT_UNKNOWN;
  }
}

void UpdateDepthThreshold(ID3D11Device *device, ID3D11DeviceContext *context,
                          float clear);

// UI time: copy the depth buffer the engine used for the frame just finished,
// then start collecting the next one.
void CaptureEngineDepth(ID3D11Device *device, ID3D11DeviceContext *context) {
  ID3D11Texture2D *src = nullptr;
  float clear = 1.0f;
  int clears = 0;
  std::string log;
  {
    std::lock_guard<std::mutex> lock(s_depthMutex);
    if (s_sceneDepth) {
      // The persistent scene depth is the right buffer even on frames where the
      // engine did not clear it.
      src = s_sceneDepth;
      src->AddRef();
      clear = s_sceneDepthClear;
      clears = 1;
      if (s_frameDepth) {
        s_frameDepth->Release();
        s_frameDepth = nullptr;
      }
    } else {
      src = s_frameDepth;
      s_frameDepth = nullptr;
      s_frameDepthAspectOk = false;
      clear = s_frameDepthClear;
      clears = s_frameDepthClears;
    }
    s_frameDepthClears = 0;
    log.swap(s_frameDepthLog);
  }
  static ULONGLONG s_lastLog = 0;
  const ULONGLONG now = GetTickCount64();
  const bool logNow = now - s_lastLog >= 3000;
  if (logNow)
    s_lastLog = now;
  if (!src) {
    // The menu draws at up to 60 fps and the engine can be slower: no new
    // engine frame since the last UI frame means the copy we have still matches
    // the picture on screen. Keep it (dropping it switched to the brightness
    // mask every other frame: lattice flickering over dark armour).
    static ULONGLONG s_lastDepthMs = 0;
    (void)s_lastDepthMs;
    return;
  }
  D3D11_TEXTURE2D_DESC d{};
  src->GetDesc(&d);
  const DXGI_FORMAT typeless = TypelessOf(d.Format);
  if (typeless == DXGI_FORMAT_UNKNOWN) {
    if (logNow)
      NOBLE_INFO("ArmoryDepth: unsupported depth format {} ({}x{})",
                 static_cast<int>(d.Format), d.Width, d.Height);
    src->Release();
    s_depthValid = false;
    return;
  }
  if (!s_depthCopy || s_depthCopyDesc.Width != d.Width ||
      s_depthCopyDesc.Height != d.Height ||
      s_depthCopyDesc.Format != typeless) {
    if (s_depthSrv) {
      s_depthSrv->Release();
      s_depthSrv = nullptr;
    }
    if (s_depthCopy) {
      s_depthCopy->Release();
      s_depthCopy = nullptr;
    }
    D3D11_TEXTURE2D_DESC c{};
    c.Width = d.Width;
    c.Height = d.Height;
    c.MipLevels = 1;
    c.ArraySize = 1;
    c.Format = typeless;
    c.SampleDesc.Count = 1;
    c.Usage = D3D11_USAGE_DEFAULT;
    c.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SHADER_RESOURCE_VIEW_DESC v{};
    v.Format = ReadFormatOf(typeless);
    v.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    v.Texture2D.MipLevels = 1;
    if (FAILED(device->CreateTexture2D(&c, nullptr, &s_depthCopy)) ||
        !s_depthCopy ||
        FAILED(
            device->CreateShaderResourceView(s_depthCopy, &v, &s_depthSrv)) ||
        !s_depthSrv) {
      NOBLE_WARN(
          "ArmoryDepth: could not create the depth copy ({}x{} format {})",
          d.Width, d.Height, static_cast<int>(typeless));
      if (s_depthCopy) {
        s_depthCopy->Release();
        s_depthCopy = nullptr;
      }
      src->Release();
      s_depthValid = false;
      return;
    }
    s_depthCopyDesc = c;
    NOBLE_INFO(
        "ArmoryDepth: copying engine depth {}x{} (format {}, cleared to {})",
        d.Width, d.Height, static_cast<int>(d.Format), clear);
  }
  context->CopyResource(s_depthCopy, src);
  src->Release();
  s_depthClear = clear;
  s_depthValid = true;
  UpdateDepthThreshold(device, context, clear);
  if (logNow)
    NOBLE_INFO("ArmoryDepth: last frame's depth clears:{} -> using {}x{} "
               "cleared to {}",
               log, d.Width, d.Height, clear);
}

// Depth sample grid read back on the CPU (a few frames late, never stalling):
// the character is the NEAREST cluster of depths and the room/wall a far one,
// so the biggest jump between sorted sample depths separates them. This works
// in whole-body views (wall behind) and close-ups (character at the frame
// edges) alike.
constexpr UINT kGridW = 32, kGridH = 18;
ID3D11Texture2D *s_depthStage[3] = {};
bool s_depthStagePending[3] = {};
DXGI_FORMAT s_depthStageFormat = DXGI_FORMAT_UNKNOWN;
int s_depthStageNext = 0;
int s_depthGridCounter = 0;
float s_depthThreshold = -1.0f;

float ReadDepthTexel(const uint8_t *p, DXGI_FORMAT typeless) {
  switch (typeless) {
  case DXGI_FORMAT_R32_TYPELESS:
  case DXGI_FORMAT_R32G8X24_TYPELESS: {
    float f;
    std::memcpy(&f, p, 4);
    return f;
  }
  case DXGI_FORMAT_R24G8_TYPELESS: {
    uint32_t v;
    std::memcpy(&v, p, 4);
    return static_cast<float>(v & 0xFFFFFFu) / 16777215.0f;
  }
  case DXGI_FORMAT_R16_TYPELESS: {
    uint16_t v;
    std::memcpy(&v, p, 2);
    return static_cast<float>(v) / 65535.0f;
  }
  default:
    return 0.0f;
  }
}

void UpdateDepthThreshold(ID3D11Device *device, ID3D11DeviceContext *context,
                          float clear) {
  const DXGI_FORMAT fmt = s_depthCopyDesc.Format;
  // Collect finished read-backs.
  for (int i = 0; i < 3; ++i) {
    if (!s_depthStagePending[i] || !s_depthStage[i])
      continue;
    D3D11_MAPPED_SUBRESOURCE m{};
    if (context->Map(s_depthStage[i], 0, D3D11_MAP_READ,
                     D3D11_MAP_FLAG_DO_NOT_WAIT, &m) != S_OK)
      continue;
    s_depthStagePending[i] = false;
    const UINT texel = fmt == DXGI_FORMAT_R32G8X24_TYPELESS
                           ? 8u
                           : (fmt == DXGI_FORMAT_R16_TYPELESS ? 2u : 4u);
    std::vector<float> v;
    v.reserve(kGridW * kGridH);
    for (UINT y = 0; y < kGridH; ++y) {
      const uint8_t *row =
          static_cast<const uint8_t *>(m.pData) + y * m.RowPitch;
      for (UINT x = 0; x < kGridW; ++x) {
        const float d = ReadDepthTexel(row + x * texel, fmt);
        if (d > clear + 1e-7f)
          v.push_back(d);
      }
    }
    context->Unmap(s_depthStage[i], 0);
    if (v.empty()) {
      s_depthThreshold = -1.0f;
      continue;
    }
    std::sort(v.begin(), v.end(),
              std::greater<float>()); // nearest first (reversed depth)
    float bestRatio = 1.0f;
    size_t bestAt = v.size();
    for (size_t k = 0; k + 1 < v.size(); ++k) {
      if (v[k + 1] <= 0.0f)
        break;
      const float r = v[k] / v[k + 1];
      if (r > bestRatio) {
        bestRatio = r;
        bestAt = k;
      }
    }
    // A real character/room gap is large; small steps are just the character's
    // own depth (rifle vs body).
    const float threshold =
        (bestRatio >= 1.6f && bestAt < v.size())
            ? std::sqrt(v[bestAt] * v[bestAt + 1])
            : v.back() *
                  0.5f; // one cluster only: everything drawn is the character
    s_depthThreshold = threshold;
    static ULONGLONG s_lastLog = 0;
    const ULONGLONG now = GetTickCount64();
    if (now - s_lastLog >= 3000) {
      s_lastLog = now;
      NOBLE_INFO(
          "ArmoryDepth: grid {} samples, nearest {:.6g}, farthest {:.6g}, "
          "biggest gap x{:.2f} after {} samples -> threshold {:.6g}",
          v.size(), v.front(), v.back(), bestRatio,
          bestAt == v.size() ? 0 : bestAt + 1, threshold);
    }
  }
  // Start a new read-back every 4th frame.
  if (++s_depthGridCounter % 4 != 0 || !s_depthCopy)
    return;
  if (s_depthStageFormat != fmt) {
    for (auto &st : s_depthStage)
      if (st) {
        st->Release();
        st = nullptr;
      }
    for (bool &p : s_depthStagePending)
      p = false;
    s_depthStageFormat = fmt;
  }
  const int i = s_depthStageNext;
  if (s_depthStagePending[i])
    return; // still in flight
  if (!s_depthStage[i]) {
    D3D11_TEXTURE2D_DESC d{};
    d.Width = kGridW;
    d.Height = kGridH;
    d.MipLevels = 1;
    d.ArraySize = 1;
    d.Format = fmt;
    d.SampleDesc.Count = 1;
    d.Usage = D3D11_USAGE_STAGING;
    d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    if (FAILED(device->CreateTexture2D(&d, nullptr, &s_depthStage[i])) ||
        !s_depthStage[i])
      return;
  }
  const UINT w = s_depthCopyDesc.Width, h = s_depthCopyDesc.Height;
  for (UINT gy = 0; gy < kGridH; ++gy) {
    for (UINT gx = 0; gx < kGridW; ++gx) {
      const UINT sx =
          std::min(w - 1, static_cast<UINT>((gx + 0.5f) * w / kGridW));
      const UINT sy =
          std::min(h - 1, static_cast<UINT>((gy + 0.5f) * h / kGridH));
      const D3D11_BOX box{sx, sy, 0, sx + 1, sy + 1, 1};
      context->CopySubresourceRegion(s_depthStage[i], 0, gx, gy, 0, s_depthCopy,
                                     0, &box);
    }
  }
  s_depthStagePending[i] = true;
  s_depthStageNext = (i + 1) % 3;
}

void UnbindDepth(const ImDrawList *, const ImDrawCmd *) {
  if (!s_context)
    return;
  ID3D11ShaderResourceView *none = nullptr;
  s_context->PSSetShaderResources(1, 1, &none);
}

bool Prepare(ID3D11Device *device, ID3D11DeviceContext *context) {
  if (!device || !context)
    return false;
  s_context = context;
  if (!s_frameRtv) {
    // Reuse existing captured stage if frame is temporarily holding, so
    // character never flashes out
    return (s_copySrv != nullptr && s_opaque != nullptr &&
            s_additive != nullptr);
  }
  ID3D11Resource *resource = nullptr;
  s_frameRtv->GetResource(&resource);
  ID3D11Texture2D *backBuffer = nullptr;
  if (resource)
    resource->QueryInterface(__uuidof(ID3D11Texture2D),
                             reinterpret_cast<void **>(&backBuffer));
  if (resource)
    resource->Release();
  if (!backBuffer)
    return false;
  D3D11_TEXTURE2D_DESC desc{};
  backBuffer->GetDesc(&desc);
  bool ok = desc.SampleDesc.Count == 1;
  if (ok && (!s_copy || s_copyW != desc.Width || s_copyH != desc.Height ||
             s_copyFormat != desc.Format)) {
    if (s_copySrv) {
      s_copySrv->Release();
      s_copySrv = nullptr;
    }
    if (s_copy) {
      s_copy->Release();
      s_copy = nullptr;
    }
    D3D11_TEXTURE2D_DESC copy = desc;
    copy.Usage = D3D11_USAGE_DEFAULT;
    copy.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    copy.CPUAccessFlags = 0;
    copy.MiscFlags = 0;
    copy.MipLevels = 1;
    copy.ArraySize = 1;
    ok = SUCCEEDED(device->CreateTexture2D(&copy, nullptr, &s_copy)) &&
         s_copy &&
         SUCCEEDED(
             device->CreateShaderResourceView(s_copy, nullptr, &s_copySrv));
    s_copyW = desc.Width;
    s_copyH = desc.Height;
    s_copyFormat = desc.Format;
  }
  // While the character is being rebuilt (effect change) or switched (species),
  // keep the last finished picture: the engine shows the new biped for a frame
  // in default colours / un-posed (white armour flash, Elite jump).
  const bool hold =
      ReachArmory::IsStageHeld() && s_copySrv != nullptr && s_copy != nullptr;
  s_stageHeld = hold;
  if (ok && s_copy && !hold)
    context->CopyResource(s_copy, backBuffer);
  backBuffer->Release();
  // Each state on its own: EnsureBackdrop() may already have made s_opaque, and
  // skipping s_additive then left Prepare() returning false for good (the
  // character never appeared behind "LOADING").
  if (!s_opaque) {
    D3D11_BLEND_DESC blend{};
    blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blend, &s_opaque);
  }
  if (!s_additive) {
    D3D11_BLEND_DESC blend{};
    blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blend.RenderTarget[0].BlendEnable = TRUE;
    blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    device->CreateBlendState(&blend, &s_additive);
  }
  return ok && s_copySrv && s_opaque && s_additive;
}

void Draw(const Ui &ui, ID3D11Device *device, ID3D11DeviceContext *context,
          float ruleY) {
  // The character shows only once the menu script runs and the first armour
  // apply has settled; before that the stage shows the lattice with
  // "LOADING..." (the engine frame then is the main menu's camera or a black
  // room).
  const bool presentable = ReachArmory::IsCharacterPresentable();
  // No fresh engine frame (s_frameRtv null) = the fallback path, where the menu
  // is drawn from the main thread while the engine thread may still use the
  // same D3D11 immediate context. Nothing here may touch the context then
  // (copies, read-backs, hooks): doing so crashed inside the NVIDIA driver
  // (06:22, clicking Nameplate).
  const bool engineFrame = s_frameRtv != nullptr;
  if (engineFrame)
    InstallDepthHook(context);
  const bool haveFrame = presentable && Prepare(device, context);
  if (haveFrame && engineFrame && !s_stageHeld)
    CaptureEngineDepth(device, context);
  s_context = context;
  const bool backdrop =
      (engineFrame || s_backdropPs != nullptr) && EnsureBackdrop(device);
  if (!haveFrame && !backdrop)
    return;
  const float W = ui.size.x, H = ui.size.y;
  constexpr float kStageX =
      0.7425f; // the middle of the free area right of the picker
  float k = 1.0f;
  ImVec2 p0;
  // Whole-body vs close-up placement follows the menu's selection, which
  // changes the instant a row or species is clicked - a frame before the engine
  // shows that camera. Switching placement on that frame drew the OLD picture
  // at the other size for one frame (the Elite "jump" when picking Spartan,
  // Tyler's 06:18 video). The placement only follows after the new value has
  // held for a few frames, and never while the picture itself is held.
  static bool s_fullBodyPlacement = true;
  static int s_placementPending = 0;
  {
    const bool wantFull = UIManager::GetArmorCustomizer().IsFullBodyCamera();
    if (wantFull == s_fullBodyPlacement) {
      s_placementPending = 0;
    } else if (!ReachArmory::IsStageHeld() && ++s_placementPending >= 4) {
      s_fullBodyPlacement = wantFull;
      s_placementPending = 0;
    }
  }
  if (s_fullBodyPlacement) {
    // Whole-body cameras frame the pedestal, so the character sits small in the
    // middle of the engine frame. Enlarge that frame around the character until
    // it fills the stage and the weapon prop below the feet is off-screen. The
    // menu process renders 1.5x its window, so a 1.5x draw here is still 1:1
    // source-to-screen (no pixelation); never go past that.
    const float windowScale = std::max(1.0f, s_windowScale);
    // 1.5x filled the stage but cropped the model: the helmet ran under the
    // player card and the feet went off the bottom edge. The character is ~0.64
    // of the engine frame tall (0.18..0.82), so 1.2x shows all of it with
    // headroom, and still stays under the supersample limit (no pixelation).
    constexpr float kWholeBodyZoom = 1.12f;
    k = std::min(kWholeBodyZoom, windowScale);
    constexpr float kFocusX =
        0.590f; // character centre in the engine frame (0x0C / 0x0B)
    constexpr float kFocusY = 0.505f;
    const float targetX = ui.origin.x + W * kStageX;
    // The character spans ~0.16..0.81 of the engine frame; at 1.12x centring it
    // at 0.60 puts the helmet at ~0.21 (clear of the player card, which ends at
    // ~0.16) and the boots at ~0.94.
    const float targetY = ui.origin.y + H * 0.60f;
    p0 = ImVec2(targetX - W * kFocusX * k, targetY - H * kFocusY * k);
  } else {
    // Per-slot close-ups already fill the frame; keep the old whole-frame
    // placement under the rule.
    constexpr float kModelX = 0.656f;
    k = std::clamp((H - ruleY) / H, 0.70f, 0.90f);
    p0 = ImVec2(ui.origin.x + W * kStageX - W * kModelX * k, ruleY);
  }
  const ImVec2 p1(p0.x + W * k, p0.y + H * k);

  if (backdrop) {
    s_backdropParams.screenW = W;
    s_backdropParams.screenH = H;
    s_backdropParams.frameX = p0.x - ui.origin.x;
    s_backdropParams.frameY = p0.y - ui.origin.y;
    s_backdropParams.frameW = p1.x - p0.x;
    s_backdropParams.frameH = p1.y - p0.y;
    s_backdropParams.centerX = W * kStageX;
    s_backdropParams.centerY = H * 0.52f;
    s_backdropParams.time =
        static_cast<float>(std::fmod(ImGui::GetTime(), 36000.0));
    s_backdropParams.frameAlpha = std::clamp(
        static_cast<float>(ReachArmory::CharacterPresentableForMs()) / 350.0f,
        0.0f, 1.0f);
    s_backdropParams.hasFrame = haveFrame ? 1.0f : 0.0f;
    s_backdropParams.clearDepth = s_depthClear;
    s_backdropParams.hasDepth =
        (haveFrame && s_depthValid && s_depthSrv) ? 1.0f : 0.0f;
    s_backdropParams.depthW = static_cast<float>(s_depthCopyDesc.Width);
    s_backdropParams.depthH = static_cast<float>(s_depthCopyDesc.Height);
    s_backdropParams.depthThreshold = s_depthThreshold;
    const ImTextureID tex = haveFrame
                                ? reinterpret_cast<ImTextureID>(s_copySrv)
                                : reinterpret_cast<ImTextureID>(s_blankSrv);
    ui.dl->AddCallback(&UseBackdrop, nullptr);
    ui.dl->AddImage(tex, ui.origin, ImVec2(ui.origin.x + W, ui.origin.y + H));
    ui.dl->AddCallback(&UnbindDepth, nullptr);
    ui.dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
  } else {
    ui.dl->AddRectFilled(ui.origin, ImVec2(ui.origin.x + W, ui.origin.y + H),
                         IM_COL32(0, 0, 0, 255));
    ui.dl->AddCallback(&UseOpaque, nullptr);
    ui.dl->AddImage(reinterpret_cast<ImTextureID>(s_copySrv), p0, p1);
    ui.dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
  }
  if (!haveFrame) {
    DrawLoading(ui, ImVec2(ui.origin.x + W * kStageX, ui.origin.y + H * 0.52f));
    ui.dl->AddRectFilledMultiColor(
        ui.origin, ImVec2(ui.origin.x + W * 0.56f, ui.origin.y + H),
        IM_COL32(4, 8, 14, 255), IM_COL32(4, 8, 14, 0), IM_COL32(4, 8, 14, 0),
        IM_COL32(4, 8, 14, 255));
    return;
  }

  // Floor pool only. The old 30-ring chest glow was additive white/blue over
  // the model and added ~30/255 to every channel, which desaturated armour
  // colours. The engine already lights the model; keep the atmosphere below the
  // feet and leave the body untouched.
  ui.dl->AddCallback(&UseAdditive, nullptr);
  const auto glow = [&](ImVec2 centre, float radiusX, float radiusY,
                        int rings) {
    for (int i = 0; i < rings; ++i) {
      const float t = 1.0f - static_cast<float>(i) / rings;
      const ImU32 step = IM_COL32(0, i % 2 ? 0 : 40, 255, 1);
      constexpr int kPoints = 48;
      for (int n = 0; n < kPoints; ++n) {
        const float a = static_cast<float>(n) / kPoints * 6.2831853f;
        ui.dl->PathLineTo(ImVec2(centre.x + std::cos(a) * radiusX * t,
                                 centre.y + std::sin(a) * radiusY * t));
      }
      ui.dl->PathFillConvex(step);
    }
  };
  glow(ImVec2(ui.origin.x + W * kStageX, p0.y + H * k * 0.84f), H * 0.40f,
       H * 0.055f, 10);
  ui.dl->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

  // The panels on the left sit on a dark edge, and the picture's left end
  // doesn't show as a seam.
  ui.dl->AddRectFilledMultiColor(
      ui.origin, ImVec2(ui.origin.x + W * 0.56f, ui.origin.y + H),
      IM_COL32(4, 8, 14, 255), IM_COL32(4, 8, 14, 0), IM_COL32(4, 8, 14, 0),
      IM_COL32(4, 8, 14, 255));
}

} // namespace ArmoryStage

void DrawBackdrop(const Ui &ui, Screen screen) {
  const ImVec2 o = ui.origin;
  const ImVec2 extent(ui.origin.x + ui.size.x, ui.origin.y + ui.size.y);
  // Front-end screens use the selected menu background (the player's own
  // pictures in data\ui\custom\backgrounds first, then Noble/Reach art). This
  // is the backdrop the player chose; it also means the main menu never shows
  // the engine map's camera cuts. The Armory is the one exception: its live 3D
  // Spartan/Elite is drawn straight into the backbuffer and must show through.
  const bool needBackdropImage = screen != Screen::Armory;
  if (needBackdropImage) {
    ui.dl->AddRectFilled(o, extent, IM_COL32(8, 12, 18, 255));
    // The lobby shows the map that is going to load, when there is a picture of
    // it.
    UiTexture bg;
    if (screen == Screen::PreGameLobby && !s_lobbyBackdropImage.empty()) {
      bg = UiArt::ImageFile(s_lobbyBackdropImage);
    } else if (screen == Screen::PreGameLobby && !s_lobbyBackdropMap.empty()) {
      const std::string scenario = MapNames::ScenarioOf(s_lobbyBackdropMap);
      bg = UiArt::MapPhoto(scenario.empty() ? s_lobbyBackdropMap : scenario);
    }
    // Fixed artwork for the screens that have their own: layered
    // (screens\<name>\layers.json) first, then one picture (screens\<name>.*),
    // in data\ui\custom then data\ui\noble.
    bool drewLayered = false;
    if (!bg) {
      const char *screenArt = nullptr;
      switch (screen) {
      case Screen::Firefight:
        screenArt = "firefight";
        break;
      case Screen::CustomGame:
      case Screen::Servers:
        screenArt = "custom_games";
        break;
      case Screen::Campaign:
      case Screen::Skulls:
        screenArt = "campaign";
        break;
      case Screen::Forge:
        screenArt = "forge";
        break;
      default:
        break;
      }
      if (screenArt) {
        if (const UiArt::LayeredArt *layered = UiArt::ScreenLayers(screenArt)) {
          UiArt::DrawLayered(ui.dl, *layered, o, extent, ImGui::GetTime());
          drewLayered = true;
        } else {
          bg = UiArt::ScreenBackground(screenArt);
        }
      }
    }
    if (drewLayered) {
      // drawn above
    } else if (bg) {
      UiArt::DrawCover(ui.dl, bg, o, extent, IM_COL32(255, 255, 255, 255));
    } else {
      // Shared menu background. On the main menu it changes every kSlideSeconds
      // with a cross-fade.
      constexpr double kSlideSeconds = 20.0;
      constexpr double kFadeSeconds = 1.5;
      static double s_slideStart = -1.0;
      static double s_fadeStart = -100.0;
      static UiTexture s_previous;
      const double now = ImGui::GetTime();
      if (s_slideStart < 0.0)
        s_slideStart = now;
      UiTexture current = UiArt::MenuBackground();
      if (screen == Screen::Main && now - s_slideStart >= kSlideSeconds) {
        s_slideStart = now;
        const UiTexture next = UiArt::MenuBackgroundNext();
        if (next && next.id != current.id) {
          s_previous = current;
          s_fadeStart = now;
          current = next;
        }
      }
      const double fade = (now - s_fadeStart) / kFadeSeconds;
      if (fade < 1.0 && s_previous) {
        UiArt::DrawCover(ui.dl, s_previous, o, extent,
                         IM_COL32(255, 255, 255, 255));
        const int a = static_cast<int>(255.0 * (fade < 0.0 ? 0.0 : fade));
        if (current)
          UiArt::DrawCover(ui.dl, current, o, extent,
                           IM_COL32(255, 255, 255, a));
      } else if (current) {
        UiArt::DrawCover(ui.dl, current, o, extent,
                         IM_COL32(255, 255, 255, 255));
      }
    }
  }

  if (screen == Screen::Armory) {
    // In the Armory, provide sleek subtle left-side menu darkening while
    // keeping the 3D Spartan/Elite character preview completely unoccluded on
    // the right side
    ui.dl->AddRectFilledMultiColor(
        o, ImVec2(ui.origin.x + ui.size.x * 0.44f, extent.y),
        IM_COL32(4, 8, 14, 180), IM_COL32(4, 8, 14, 85), IM_COL32(4, 8, 14, 85),
        IM_COL32(4, 8, 14, 180));
    ui.dl->AddRectFilledMultiColor(
        ImVec2(ui.origin.x + ui.size.x * 0.44f, ui.origin.y),
        ImVec2(ui.origin.x + ui.size.x * 0.60f, extent.y),
        IM_COL32(4, 8, 14, 85), IM_COL32(4, 8, 14, 0), IM_COL32(4, 8, 14, 0),
        IM_COL32(4, 8, 14, 85));
    ui.dl->AddRectFilledMultiColor(
        ImVec2(ui.origin.x, ui.origin.y + ui.size.y * 0.82f), extent,
        IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 160),
        IM_COL32(0, 0, 0, 160));
  } else if (screen == Screen::PreGameLobby) {
    // Pre-game lobby: subtle left darkening for menu items and bottom vignette
    // matching Halo: Reach 360/MCC
    ui.dl->AddRectFilledMultiColor(
        o, ImVec2(ui.origin.x + ui.size.x * 0.48f, extent.y),
        IM_COL32(0, 0, 0, 175), IM_COL32(0, 0, 0, 60), IM_COL32(0, 0, 0, 60),
        IM_COL32(0, 0, 0, 175));
    ui.dl->AddRectFilledMultiColor(
        ImVec2(ui.origin.x, ui.origin.y + ui.size.y * 0.75f), extent,
        IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 185),
        IM_COL32(0, 0, 0, 185));
  } else {
    // Darken the left half (where the menu list is) and the bottom, like
    // Reach's native menus.
    ui.dl->AddRectFilledMultiColor(
        o, ImVec2(ui.origin.x + ui.size.x * 0.45f, extent.y),
        IM_COL32(0, 0, 0, 190), IM_COL32(0, 0, 0, 105), IM_COL32(0, 0, 0, 105),
        IM_COL32(0, 0, 0, 190));
    ui.dl->AddRectFilledMultiColor(
        ImVec2(ui.origin.x + ui.size.x * 0.45f, ui.origin.y),
        ImVec2(ui.origin.x + ui.size.x * 0.78f, extent.y),
        IM_COL32(0, 0, 0, 105), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0),
        IM_COL32(0, 0, 0, 105));
    ui.dl->AddRectFilledMultiColor(
        ImVec2(ui.origin.x, ui.origin.y + ui.size.y * 0.72f), extent,
        IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 0), IM_COL32(0, 0, 0, 165),
        IM_COL32(0, 0, 0, 165));
  }
}

void DrawTitle(const Ui &ui, ImVec2 pos, const char *title,
               const char *sideLabel) {
  const float d = 15.0f * ui.s;
  const ImVec2 c(pos.x + d, pos.y + 28.0f * ui.s);
  ui.dl->AddQuad(ImVec2(c.x, c.y - d), ImVec2(c.x + d, c.y),
                 ImVec2(c.x, c.y + d), ImVec2(c.x - d, c.y), kAccent,
                 2.0f * ui.s);
  ui.dl->AddQuad(ImVec2(c.x, c.y - d * 0.45f), ImVec2(c.x + d * 0.45f, c.y),
                 ImVec2(c.x, c.y + d * 0.45f), ImVec2(c.x - d * 0.45f, c.y),
                 IM_COL32(143, 216, 255, 140), 1.5f * ui.s);
  Text(ui, NobleStyle::HeroFont(), 50.0f, ImVec2(pos.x + d * 2.6f, pos.y),
       IM_COL32(220, 226, 232, 215), title);
  if (sideLabel) {
    // Vertical label (SOLO, SURVIVAL, MULTIPLAYER...) in the margin to the left
    // of the title. It used to run down from pos.x + 2, right beside the tabs
    // and lists that start at pos.x + 34 and the text overlapped their top rows
    // once the UI scaled up. Each letter is centred in a column as wide as the
    // widest one, which ends a gap left of pos.x, so nothing at or right of the
    // title's left edge can be under it.
    const float px = 26.0f * ui.s;
    float columnW = 0.0f;
    for (const char *p = sideLabel; *p; ++p) {
      const char ch[2] = {*p, 0};
      columnW = std::max(columnW, TextWidth(NobleStyle::TitleFont(), px, ch));
    }
    const float gap = 10.0f * ui.s;
    const float columnX =
        std::max(ui.origin.x + 4.0f * ui.s, pos.x - gap - columnW);
    float y = pos.y + 70.0f * ui.s;
    for (const char *p = sideLabel; *p; ++p) {
      const char ch[2] = {*p, 0};
      const float w = TextWidth(NobleStyle::TitleFont(), px, ch);
      Text(ui, NobleStyle::TitleFont(), 26.0f,
           ImVec2(columnX + (columnW - w) * 0.5f, y),
           IM_COL32(140, 150, 160, 70), ch);
      y += 24.0f * ui.s;
    }
  }
}

bool AcceptPressed();

int DrawList(const Ui &ui, const char *id, ImVec2 pos, float width,
             const std::vector<ListItem> &items, int &selected, bool keyboard,
             float rowHeight, int maxRows, bool hoverSelects) {
  const float rh = rowHeight * ui.s;
  int activated = -1;
  const int n = static_cast<int>(items.size());
  if (n == 0)
    return -1;
  selected = std::clamp(selected, 0, n - 1);

  if (keyboard) {
    // Rows that cannot be started (a modded map Reach has no launch index for)
    // are still selectable, so the map banner can say why; only Accept is
    // blocked for them.
    auto step = [&](int dir) {
      selected = (selected + dir + n) % n;
      MenuSounds::Scroll();
    };
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) || UiPad::Down())
      step(1);
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) || UiPad::Up())
      step(-1);
    if (AcceptPressed() && items[selected].enabled) {
      UiPad::ConsumeAccept();
      activated = selected;
    }
  }

  static std::unordered_map<ImGuiID, int> s_first, s_lastSelected;
  const ImGuiID key = ImGui::GetID(id);
  int &first = s_first[key];
  if (maxRows > 0 && n > maxRows) {
    auto lastSel = s_lastSelected.find(key);
    if (lastSel == s_lastSelected.end() || lastSel->second != selected) {
      if (selected < first)
        first = selected;
      else if (selected >= first + maxRows)
        first = selected - maxRows + 1;
      s_lastSelected[key] = selected;
    }
    const ImVec2 area0 = pos,
                 area1(pos.x + width + 14.0f * ui.s, pos.y + maxRows * rh);
    const float wheel = ImGui::GetIO().MouseWheel;
    if (wheel != 0.0f && ImGui::IsMouseHoveringRect(area0, area1, false)) {
      first -= NobleUi::ConsumeWheelSteps(id ? id : "##list_wheel", wheel);
    }
    static std::unordered_map<ImGuiID, float> s_stickAccumMap;
    float &stickAccum = s_stickAccumMap[key];
    const float rStick = UiPad::AnalogScrollY(false);
    if (rStick != 0.0f) {
      stickAccum += rStick / std::max(1.0f, rh);
      const int stepRows = static_cast<int>(stickAccum);
      if (stepRows != 0) {
        first += stepRows;
        stickAccum -= static_cast<float>(stepRows);
      }
    } else {
      stickAccum = 0.0f;
    }
    first = std::clamp(first, 0, n - maxRows);
    const float trackX = pos.x + width + 6.0f * ui.s, trackH = maxRows * rh;
    float scrollRows = static_cast<float>(first);
    char sbId[96];
    std::snprintf(sbId, sizeof(sbId), "%s_sb", id ? id : "list");
    NobleUi::DrawInteractiveScrollbar(
        ui.dl, sbId, ImVec2(trackX, pos.y),
        ImVec2(trackX + 6.0f * ui.s, pos.y + trackH), scrollRows,
        static_cast<float>(n - maxRows), static_cast<float>(maxRows),
        static_cast<float>(n), ui.s);
    first =
        std::clamp(static_cast<int>(std::lround(scrollRows)), 0, n - maxRows);
  } else {
    first = 0;
  }
  const int last = maxRows > 0 ? std::min(n, first + maxRows) : n;

  for (int i = first; i < last; ++i) {
    const ImVec2 a(pos.x, pos.y + (i - first) * rh);
    const ImVec2 b(a.x + width, a.y + rh);
    ImGui::SetCursorScreenPos(a);
    ImGui::PushID(id);
    ImGui::PushID(i);
    ImGui::InvisibleButton("##row", ImVec2(width, rh));
    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    ImGui::PopID();
    ImGui::PopID();
    const bool mouseMoved =
        ImGui::GetIO().MouseDelta.x * ImGui::GetIO().MouseDelta.x +
            ImGui::GetIO().MouseDelta.y * ImGui::GetIO().MouseDelta.y >
        0.0f;
    if (hovered && selected != i && mouseMoved) {
      if (hoverSelects)
        selected = i;
      MenuSounds::Hover(ImGui::GetID(id) + i);
    }
    if (hovered && !hoverSelects && i != selected && items[i].enabled) {
      ui.dl->AddRectFilled(a, b, IM_COL32(143, 216, 255, 20), 5.0f * ui.s);
    }
    if (clicked) {
      selected = i;
      if (items[i].enabled)
        activated = i;
    }
    if (i == selected) {
      if (keyboard) {
        NobleUi::DrawFocusHighlight(ui.dl, a, b, ui.s, true);
      } else {
        ui.dl->AddRectFilledMultiColor(
            a, b, IM_COL32(255, 255, 255, 46), IM_COL32(255, 255, 255, 4),
            IM_COL32(255, 255, 255, 4), IM_COL32(255, 255, 255, 46));
        ui.dl->AddRectFilled(a, ImVec2(a.x + 2.0f * ui.s, b.y), kWhite);
      }
    }
    if (items[i].header) {
      const float cx = a.x + 14.0f * ui.s, cy = a.y + rh * 0.5f,
                  t = 5.0f * ui.s;
      const ImU32 ac = i == selected ? kWhite : kAccent;
      if (items[i].expanded)
        ui.dl->AddTriangleFilled(ImVec2(cx - t, cy - t * 0.6f),
                                 ImVec2(cx + t, cy - t * 0.6f),
                                 ImVec2(cx, cy + t * 0.8f), ac);
      else
        ui.dl->AddTriangleFilled(ImVec2(cx - t * 0.6f, cy - t),
                                 ImVec2(cx - t * 0.6f, cy + t),
                                 ImVec2(cx + t * 0.8f, cy), ac);
      // Workshop mod names can be very long ("... (NEW CUSTOM MAP "STAGING
      // GROUND" OUT NOW!)"), so the heading is clipped to its row instead of
      // running into the column on the right.
      ui.dl->PushClipRect(ImVec2(a.x, a.y), ImVec2(b.x - 4.0f * ui.s, b.y),
                          true);
      const std::string headerLabel =
          Ellipsize(NobleStyle::TitleFont(), 19.0f * ui.s,
                    Upper(items[i].label), b.x - a.x - 32.0f * ui.s);
      Text(ui, NobleStyle::TitleFont(), 19.0f,
           ImVec2(a.x + 28.0f * ui.s, a.y + (rh - 19.0f * ui.s) * 0.5f), ac,
           headerLabel.c_str());
      ui.dl->PopClipRect();
      ui.dl->AddLine(ImVec2(a.x, b.y - 1.0f), ImVec2(b.x, b.y - 1.0f),
                     IM_COL32(143, 216, 255, 60), 1.0f);
      continue;
    }
    const ImU32 col =
        !items[i].enabled ? kDisabled : (i == selected ? kWhite : kDim);
    float textX = a.x + (items[0].header ? 28.0f : 9.0f) * ui.s;
    if ((!items[i].thumb.empty() || !items[i].image.empty()) &&
        rh >= 34.0f * ui.s) {
      // Number, then the map's small preview (MCC's levels pack), then the
      // name.
      if (items[i].number > 0) {
        char num[8];
        std::snprintf(num, sizeof(num), "%02d", items[i].number);
        Text(ui, NobleStyle::SmallFont(), 14.0f,
             ImVec2(textX, a.y + (rh - 14.0f * ui.s) * 0.5f),
             i == selected ? kAccent : IM_COL32(120, 135, 150, 255), num);
        textX += 24.0f * ui.s;
      }
      const float th = rh - 8.0f * ui.s, tw = th * 316.0f / 160.0f;
      const ImVec2 t0(textX, a.y + 4.0f * ui.s),
          t1(textX + tw, a.y + 4.0f * ui.s + th);
      ui.dl->AddRectFilled(t0, t1, IM_COL32(14, 20, 26, 220));
      const UiTexture tex = items[i].image.empty()
                                ? UiArt::MapPreview(items[i].thumb, false)
                                : UiArt::ImageFile(items[i].image);
      if (tex) {
        UiArt::DrawCover(ui.dl, tex, t0, t1,
                         items[i].enabled ? IM_COL32_WHITE
                                          : IM_COL32(255, 255, 255, 90));
      }
      ui.dl->AddRect(t0, t1,
                     i == selected ? IM_COL32(170, 220, 255, 200)
                                   : IM_COL32(255, 255, 255, 40),
                     0.0f, 0, 1.0f);
      textX += tw + 10.0f * ui.s;
    }
    const std::string label =
        Ellipsize(NobleStyle::TitleFont(), 20.0f * ui.s, Upper(items[i].label),
                  b.x - textX - 6.0f * ui.s);
    ui.dl->PushClipRect(ImVec2(textX, a.y), ImVec2(b.x - 2.0f * ui.s, b.y),
                        true);
    Text(ui, NobleStyle::TitleFont(), 20.0f,
         ImVec2(textX, a.y + (rh - 20.0f * ui.s) * 0.5f), col, label.c_str());
    ui.dl->PopClipRect();
  }
  if (activated >= 0)
    MenuSounds::Click();
  return activated;
}

// `forgeCount` >= 0 adds a third tab, FORGE MAPS (Custom Game and Forge).
// `tabsEnd` gets the x just past the last tab, so a search box can sit after
// them.
bool DrawSourceTabs(const Ui &ui, ImVec2 pos, int &active, int moddedCount,
                    int forgeCount, float *tabsEnd) {
  const char *labels[3] = {"VANILLA", "MODDED", "FORGE MAPS"};
  const int tabs = forgeCount >= 0 ? 3 : 2;
  if (active >= tabs)
    active = 0;
  int next = active;
  float x = pos.x;
  for (int i = 0; i < tabs; ++i) {
    char text[32];
    if (i == 1)
      std::snprintf(text, sizeof(text), "%s (%d)", labels[i], moddedCount);
    else if (i == 2)
      std::snprintf(text, sizeof(text), "%s (%d)", labels[i], forgeCount);
    else
      std::snprintf(text, sizeof(text), "%s", labels[i]);
    const float w =
        TextWidth(NobleStyle::TitleFont(), 20.0f * ui.s, text) + 24.0f * ui.s;
    const float h = 30.0f * ui.s;
    ImGui::SetCursorScreenPos(ImVec2(x, pos.y));
    ImGui::PushID("srctab");
    ImGui::PushID(i);
    if (ImGui::InvisibleButton("##t", ImVec2(w, h)))
      next = i;
    const bool hovered = ImGui::IsItemHovered();
    ImGui::PopID();
    ImGui::PopID();
    const bool on = active == i;
    if (on) {
      ui.dl->AddRectFilled(ImVec2(x, pos.y), ImVec2(x + w, pos.y + h),
                           IM_COL32(60, 120, 170, 90));
      ui.dl->AddRectFilled(ImVec2(x, pos.y + h - 2.0f * ui.s),
                           ImVec2(x + w, pos.y + h), kAccent);
    } else if (hovered) {
      ui.dl->AddRectFilled(ImVec2(x, pos.y), ImVec2(x + w, pos.y + h),
                           IM_COL32(255, 255, 255, 18));
    }
    Text(ui, NobleStyle::TitleFont(), 20.0f,
         ImVec2(x + 12.0f * ui.s, pos.y + 4.0f * ui.s), on ? kWhite : kDim,
         text);
    x += w + 6.0f * ui.s;
  }
  if (tabsEnd)
    *tabsEnd = x;
  const bool typing =
      ImGui::GetIO().WantTextInput; // Q and E are letters in a search box
  if ((!typing && ImGui::IsKeyPressed(ImGuiKey_Q, false)) ||
      ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false))
    next = std::max(0, active - 1);
  if ((!typing && ImGui::IsKeyPressed(ImGuiKey_E, false)) ||
      ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false))
    next = std::min(tabs - 1, active + 1);
  if (next == active)
    return false;
  MenuSounds::Tab(next > active ? 1 : -1);
  active = next;
  return true;
}

using InputPrompts::Prompt;

// A button hint at the bottom (ESC Back, ENTER Select, ...) clicked with the
// mouse. Set by DrawHints, read on the next frame as if the key had been
// pressed.
int s_hintClicked = -1;   // Prompt clicked during the last frame, -1 = none
int s_hintThisFrame = -1; // the click being handled this frame
bool HintClicked(Prompt p) { return s_hintThisFrame == static_cast<int>(p); }
static std::string s_hintExtraClicked;   // clickable HintExtra label clicked last frame
static std::string s_hintExtraThisFrame; // the clickable HintExtra label being handled this frame
bool HintExtraClicked(const char *label) {
  return label && s_hintExtraThisFrame == label;
}

bool AcceptPressed() {
  // A modal, text input box, or chat line owns ENTER/A while it is up;
  // otherwise typing Enter would trigger match start or menu actions under it.
  if (ModDownloadPrompt::Get().IsActive())
    return false;
  if (ImGui::GetIO().WantTextInput || SocialOverlay::IsTyping())
    return false;
  return UiPad::AcceptPressed() || HintClicked(Prompt::Accept);
}

// A prompt for a button with no Prompt of its own: Xbox glyph on a controller,
// key box on keyboard and mouse.
void DrawHints(const Ui &ui,
               const std::vector<std::pair<Prompt, const char *>> &hints,
               const std::vector<HintExtra> &extras, float left) {
  float x = ui.origin.x + ui.size.x * left;
  const float y = ui.origin.y + ui.size.y - 46.0f * ui.s;
  ui.dl->AddLine(ImVec2(x, y - 12.0f * ui.s),
                 ImVec2(ui.origin.x + ui.size.x * 0.62f, y - 12.0f * ui.s),
                 IM_COL32(255, 255, 255, 50), 1.0f);
  for (const auto &[prompt, label] : hints) {
    const float x0 = x;
    x += InputPrompts::Draw(ui.dl, NobleStyle::SmallFont(), ImVec2(x, y),
                            22.0f * ui.s, prompt) +
         7.0f * ui.s;
    Text(ui, NobleStyle::SmallFont(), 17.0f, ImVec2(x, y + 2.0f * ui.s), kWhite,
         label);
    x += TextWidth(NobleStyle::SmallFont(), 17.0f * ui.s, label);
    // Back and Select can also be clicked (Change / Tabs only describe keys, so
    // they are not buttons).
    if (prompt == Prompt::Back || prompt == Prompt::Accept) {
      const ImVec2 r0(x0 - 6.0f * ui.s, y - 5.0f * ui.s);
      const ImVec2 r1(x + 8.0f * ui.s, y + 27.0f * ui.s);
      if (ImGui::IsMouseHoveringRect(r0, r1, false)) {
        ui.dl->AddRectFilled(r0, r1, IM_COL32(143, 216, 255, 40), 3.0f * ui.s);
        ui.dl->AddRect(r0, r1, IM_COL32(143, 216, 255, 120), 3.0f * ui.s);
        if (ImGui::IsMouseClicked(0)) {
          s_hintClicked = static_cast<int>(prompt);
          MenuSounds::Click();
        }
      }
    }
    x += 26.0f * ui.s;
  }
  const bool pad = InputPrompts::Current() == InputPrompts::Device::Gamepad;
  for (const HintExtra &extra : extras) {
    const float x0 = x;
    x += (pad ? InputPrompts::DrawButton(ui.dl, NobleStyle::SmallFont(),
                                         ImVec2(x, y), 22.0f * ui.s,
                                         extra.button)
              : InputPrompts::DrawKey(ui.dl, NobleStyle::SmallFont(),
                                      ImVec2(x, y), 22.0f * ui.s, extra.key)) +
         7.0f * ui.s;
    Text(ui, NobleStyle::SmallFont(), 17.0f, ImVec2(x, y + 2.0f * ui.s), kWhite,
         extra.label);
    x += TextWidth(NobleStyle::SmallFont(), 17.0f * ui.s, extra.label);
    if (extra.clickable) {
      const ImVec2 r0(x0 - 6.0f * ui.s, y - 5.0f * ui.s);
      const ImVec2 r1(x + 8.0f * ui.s, y + 27.0f * ui.s);
      if (ImGui::IsMouseHoveringRect(r0, r1, false)) {
        ui.dl->AddRectFilled(r0, r1, IM_COL32(143, 216, 255, 40), 3.0f * ui.s);
        ui.dl->AddRect(r0, r1, IM_COL32(143, 216, 255, 120), 3.0f * ui.s);
        if (ImGui::IsMouseClicked(0)) {
          s_hintExtraClicked = extra.label ? extra.label : "";
          MenuSounds::Click();
        }
      }
    }
    x += 26.0f * ui.s;
  }
}

static bool s_openAccountPopup = false;
static int s_accountTab = 0;
static char s_accountUser[32] = "";
static char s_accountPass[32] = "";
static std::string s_accountStatusMsg = "";
static bool s_accountStatusError = false;
static bool s_openDevMenuPopup = false;
static std::string s_devStatusMsg = "";

void OpenAccountModal() {
  g_showAccountModal = true;
  s_openAccountPopup = true;
  s_accountStatusMsg.clear();
  if (AuthClient::Get().IsLoggedIn()) {
    std::snprintf(s_accountUser, sizeof(s_accountUser), "%s",
                  AuthClient::Get().GetUsername().c_str());
  } else {
    std::snprintf(s_accountUser, sizeof(s_accountUser), "%s",
                  Settings::Get().Snapshot().accountUsername.c_str());
  }
  s_accountPass[0] = 0;
}

static void OpenDevMenuModal() {
  g_showDevMenu = true;
  s_openDevMenuPopup = true;
  s_devStatusMsg.clear();
}

void DrawDevMenuModal(const Ui &ui) {
  if (!g_showDevMenu)
    return;

  // Strict security check: only developers can access this menu!
  if (!AuthClient::Get().IsDeveloper()) {
    g_showDevMenu = false;
    return;
  }

  if (s_openDevMenuPopup) {
    ImGui::OpenPopup("NOBLE DEVELOPER CONSOLE");
    s_openDevMenuPopup = false;
  }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                          ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(480.0f * ui.s, 0));

  if (ImGui::BeginPopupModal("NOBLE DEVELOPER CONSOLE", &g_showDevMenu,
                             ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f),
                       "[DEV] NOBLE DEVELOPER MENU");
    ImGui::TextDisabled("Logged in as Developer: %s (User ID: %d)",
                        AuthClient::Get().GetUsername().c_str(),
                        AuthClient::Get().GetUserId());
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4));

    // 1. UNLOCK ALL ARMORS
    if (ImGui::Button("UNLOCK ALL ARMORS", ImVec2(220, 32))) {
      Career::DevSetArmoryUnlockAll(true);
      s_devStatusMsg = "SUCCESS: All Spartan and Elite armors unlocked!";
      MenuSounds::Click();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Unlocks all armors & permutations");

    // 2. GIVE MAX CREDITS
    if (ImGui::Button("GIVE MAX CREDITS", ImVec2(220, 32))) {
      Career::DevGrantMaxCredits();
      s_devStatusMsg = "SUCCESS: Max credits granted (+999,999,999 cR)!";
      MenuSounds::Click();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Grants 999,999,999 cR spendable balance");

    // 3. GIVE MAX RANK
    if (ImGui::Button("GIVE MAX RANK", ImVec2(220, 32))) {
      Career::DevSetMaxRank();
      s_devStatusMsg = "SUCCESS: Promoted to highest rank (Inheritor)!";
      MenuSounds::Click();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Sets rank to Inheritor (20,000,000 cR)");

    ImGui::Dummy(ImVec2(0, 4));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4));

    // Convenience: UNLOCK ALL & MAX EVERYTHING
    if (ImGui::Button("MAX EVERYTHING", ImVec2(220, 32))) {
      Career::DevSetArmoryUnlockAll(true);
      Career::DevGrantMaxCredits();
      Career::DevSetMaxRank();
      s_devStatusMsg =
          "SUCCESS: All armors unlocked, Inheritor rank & 999M cR granted!";
      MenuSounds::Click();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Unlock all armors, max rank & credits");

    // Reset progress option
    if (ImGui::Button("RESET CAREER", ImVec2(220, 28))) {
      Career::DevResetProgress();
      s_devStatusMsg = "Career reset: 0 cR, Recruit rank, armors locked.";
      MenuSounds::Click();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Reset to default (testing only)");

    if (!s_devStatusMsg.empty()) {
      ImGui::Dummy(ImVec2(0, 6));
      ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s",
                         s_devStatusMsg.c_str());
    }

    ImGui::Dummy(ImVec2(0, 8));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 4));
    if (ImGui::Button("CLOSE", ImVec2(100, 30))) {
      g_showDevMenu = false;
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  } else {
    g_showDevMenu = false;
  }
}

void DrawAccountModal(const Ui &ui) {
  if (!g_showAccountModal)
    return;

  if (s_openAccountPopup) {
    ImGui::OpenPopup("ACCOUNT LOGIN / REGISTER");
    s_openAccountPopup = false;
  }
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                          ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(440.0f * ui.s, 0));

  if (ImGui::BeginPopupModal("ACCOUNT LOGIN / REGISTER", &g_showAccountModal,
                             ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_AlwaysAutoResize)) {
    bool isLoggedIn = AuthClient::Get().IsLoggedIn();
    if (isLoggedIn) {
      static uint64_t s_lastDevCheck = 0;
      uint64_t now = GetTickCount64();
      if (now - s_lastDevCheck > 2500) {
        s_lastDevCheck = now;
        AuthClient::Get().RefreshDeveloperStatusAsync();
      }

      const bool isDev = AuthClient::Get().IsDeveloper();
      const bool isTester = AuthClient::Get().IsTester();
      ImVec4 userCol = isDev ? ImVec4(1.0f, 0.85f, 0.2f, 1.0f)
                             : (isTester ? ImVec4(0.3f, 0.85f, 0.95f, 1.0f)
                                         : ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
      ImGui::TextColored(userCol, "Logged in as: %s",
                         AuthClient::Get().GetUsername().c_str());
      if (isDev) {
        ImGui::SameLine();
        const ImVec2 chip = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            chip, ImVec2(chip.x + 46.0f * ui.s, chip.y + 18.0f * ui.s),
            IM_COL32(220, 160, 20, 240), 3.0f * ui.s);
        ImGui::GetWindowDrawList()->AddText(
            ImGui::GetFont(), 11.0f * ui.s,
            ImVec2(chip.x + 10.0f * ui.s, chip.y + 3.0f * ui.s),
            IM_COL32(10, 10, 10, 255), "DEV");
        ImGui::Dummy(ImVec2(50.0f * ui.s, 18.0f * ui.s));
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped("Developer account (ID: %d). The Developer Menu is "
                           "on the main menu, above Quit.",
                           AuthClient::Get().GetUserId());
        ImGui::PopStyleColor();
      } else if (isTester) {
        ImGui::SameLine();
        const ImVec2 chip = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            chip, ImVec2(chip.x + 58.0f * ui.s, chip.y + 18.0f * ui.s),
            IM_COL32(30, 160, 210, 240), 3.0f * ui.s);
        ImGui::GetWindowDrawList()->AddText(
            ImGui::GetFont(), 11.0f * ui.s,
            ImVec2(chip.x + 8.0f * ui.s, chip.y + 3.0f * ui.s),
            IM_COL32(10, 10, 10, 255), "TESTER");
        ImGui::Dummy(ImVec2(62.0f * ui.s, 18.0f * ui.s));
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
        ImGui::TextWrapped(
            "Tester account (ID: %d). Verified community tester tag active.",
            AuthClient::Get().GetUserId());
        ImGui::PopStyleColor();
      } else {
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::TextDisabled("Standard account (ID: %d)",
                            AuthClient::Get().GetUserId());
      }
      ImGui::Separator();
      ImGui::Dummy(ImVec2(0, 8));
      ImGui::TextUnformatted("Display name");
      ImGui::SetNextItemWidth(-FLT_MIN);
      ImGui::InputText("##newname", s_accountUser, sizeof(s_accountUser));
      if (ImGui::Button("CHANGE NAME", ImVec2(-1, 30.0f * ui.s))) {
        if (AuthClient::Get().SetPlayerName(s_accountUser)) {
          s_accountStatusMsg =
              "Display name changed. Your player ID is unchanged.";
          s_accountStatusError = false;
        } else {
          s_accountStatusMsg = "Enter a name.";
          s_accountStatusError = true;
        }
      }
      if (!s_accountStatusMsg.empty()) {
        ImGui::TextColored(s_accountStatusError
                               ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f)
                               : ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                           "%s", s_accountStatusMsg.c_str());
      }
      ImGui::Dummy(ImVec2(0, 8));
      if (ImGui::Button("LOG OUT", ImVec2(-1, 30.0f * ui.s))) {
        AuthClient::Get().Logout();
        s_accountStatusMsg = "Logged out.";
        s_accountStatusError = false;
        s_accountPass[0] = 0;
      }
      ImGui::Dummy(ImVec2(0, 4));
      if (ImGui::Button("CLOSE", ImVec2(-1, 32.0f * ui.s))) {
        g_showAccountModal = false;
        ImGui::CloseCurrentPopup();
      }
    } else {
      // Login / Register: two equal tabs over full-width fields, and the same
      // button pairing as above.
      const float tabW =
          (ImGui::GetContentRegionAvail().x - 10.0f * ui.s) * 0.5f;
      const bool loginTab = (s_accountTab == 0);
      const ImVec4 on(0.20f, 0.43f, 0.61f, 1.0f);
      if (loginTab)
        ImGui::PushStyleColor(ImGuiCol_Button, on);
      if (ImGui::Button("LOG IN", ImVec2(tabW, 30.0f * ui.s)))
        s_accountTab = 0;
      if (loginTab)
        ImGui::PopStyleColor();
      ImGui::SameLine();
      if (!loginTab)
        ImGui::PushStyleColor(ImGuiCol_Button, on);
      if (ImGui::Button("REGISTER", ImVec2(tabW, 30.0f * ui.s)))
        s_accountTab = 1;
      if (!loginTab)
        ImGui::PopStyleColor();
      ImGui::Dummy(ImVec2(0, 6));

      ImGui::TextDisabled(loginTab ? "Log in to your Noble account."
                                   : "Create a Noble account.");
      ImGui::Dummy(ImVec2(0, 4));
      ImGui::TextUnformatted("Username");
      ImGui::SetNextItemWidth(-FLT_MIN);
      ImGui::InputText("##acc_user", s_accountUser, sizeof(s_accountUser));
      ImGui::TextUnformatted("Password");
      ImGui::SetNextItemWidth(-FLT_MIN);
      ImGui::InputText("##acc_pass", s_accountPass, sizeof(s_accountPass),
                       ImGuiInputTextFlags_Password);

      static bool s_rememberMe = true;
      static bool s_rememberInit = false;
      if (!s_rememberInit) {
        s_rememberMe = Settings::Get().Snapshot().rememberAccount;
        s_rememberInit = true;
      }
      ImGui::Dummy(ImVec2(0, 2));
      ImGui::Checkbox("Remember Me", &s_rememberMe);

      if (!s_accountStatusMsg.empty()) {
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::TextColored(s_accountStatusError
                               ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f)
                               : ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
                           "%s", s_accountStatusMsg.c_str());
      }

      ImGui::Dummy(ImVec2(0, 8));
      const float subW =
          (ImGui::GetContentRegionAvail().x - 10.0f * ui.s) * 0.5f;
      if (s_accountTab == 0) {
        if (ImGui::Button("LOG IN##submit", ImVec2(subW, 32.0f * ui.s)) ||
            (ImGui::IsKeyPressed(ImGuiKey_Enter) && ImGui::IsWindowFocused())) {
          auto res = AuthClient::Get().Login(s_accountUser, s_accountPass,
                                             s_rememberMe);
          if (res.success) {
            s_accountStatusMsg = "Login successful!";
            s_accountStatusError = false;
            s_accountPass[0] = 0;
            g_showAccountModal = false;
            ImGui::CloseCurrentPopup();
          } else {
            s_accountStatusMsg = res.error;
            s_accountStatusError = true;
          }
        }
      } else {
        if (ImGui::Button("REGISTER##submit", ImVec2(subW, 32.0f * ui.s)) ||
            (ImGui::IsKeyPressed(ImGuiKey_Enter) && ImGui::IsWindowFocused())) {
          auto res = AuthClient::Get().Register(s_accountUser, s_accountPass,
                                                s_rememberMe);
          if (res.success) {
            s_accountStatusMsg = "Registration successful! Logged in.";
            s_accountStatusError = false;
            s_accountPass[0] = 0;
            g_showAccountModal = false;
            ImGui::CloseCurrentPopup();
          } else {
            s_accountStatusMsg = res.error;
            s_accountStatusError = true;
          }
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("CANCEL", ImVec2(subW, 32.0f * ui.s))) {
        g_showAccountModal = false;
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::EndPopup();
  } else {
    g_showAccountModal = false;
  }
}

// One player row the way MCC draws it: rank badge, emblem chip, then a
// nameplate bar that carries the service tag on its top line and the gamertag
// underneath. Used by MY PARTY and by the pre-game lobby roster so the two
// always match. rankIndex/emblemIndex/nameplateIndex may be 0 or negative when
// the data is not known (a remote player), in which case that piece is drawn as
// a plain placeholder or left out.
float PlayerRow(const Ui &ui, ImVec2 pos, float width, int rankIndex,
                int emblemIndex, int nameplateIndex, int team,
                const std::string &serviceTag, const std::string &name,
                bool dev, bool selected, int emblemBackground,
                const UiArt::EmblemColors &emblemColors, bool tester) {
  const float h = 40.0f * ui.s;
  const float rankW = 29.0f * ui.s; // square, centred on the row
  const float embW = 30.0f * ui.s;
  const float stripW = team ? 6.0f * ui.s : 0.0f;
  const float gap = 4.0f * ui.s;
  float x = pos.x;

  // Rank badge: a square box centred on the row height, so the art never sits
  // high or low.
  const ImVec2 r0(x, pos.y + (h - rankW) * 0.5f);
  const ImVec2 r1(r0.x + rankW, r0.y + rankW);
  if (const UiTexture rank = UiArt::RankIcon(rankIndex)) {
    UiArt::DrawContain(ui.dl, rank, r0, r1);
  } else {
    ui.dl->AddRectFilled(r0, r1, IM_COL32(26, 34, 44, 220));
    ui.dl->AddRect(r0, r1, IM_COL32(90, 120, 150, 120), 0.0f, 0, 1.0f);
  }
  x += rankW + gap;

  // Emblem tile + team strip + nameplate bar are one block: the tile is flush
  // with the bar and the block gets a single frame, with a thin divider between
  // tile and bar (two frames looked like a gap).
  const ImVec2 blockStart(x, pos.y);
  const ImVec2 blockEnd(pos.x + width, pos.y + h);

  const ImVec2 e0(x, pos.y), e1(x + embW, pos.y + h);
  ui.dl->AddRectFilled(e0, e1, IM_COL32(20, 14, 18, 250));
  UiArt::DrawEmblem(ui.dl, emblemIndex, emblemBackground,
                    ImVec2(e0.x + 2.0f * ui.s, e0.y + 2.0f * ui.s),
                    ImVec2(e1.x - 2.0f * ui.s, e1.y - 2.0f * ui.s),
                    emblemColors);
  x += embW;

  // Team strip: only when the game type assigns teams (Free for All, the main
  // menu and a lobby with no team mode show nothing).
  if (stripW > 0.0f) {
    const ImVec2 s0(x, pos.y), s1(x + stripW, pos.y + h);
    ui.dl->AddRectFilled(s0, s1, TeamStyleFor(team).strip);
    x += stripW;
  }

  // Nameplate bar: service tag on the top line, gamertag below.
  const ImVec2 p0(x, pos.y), p1(blockEnd);
  ui.dl->AddRectFilled(p0, p1,
                       selected ? IM_COL32(36, 78, 112, 240)
                                : IM_COL32(22, 26, 32, 240));
  if (nameplateIndex > 0) {
    if (const UiTexture plate = UiArt::Nameplate(nameplateIndex)) {
      // Drawn 8 s to the left and clipped, so the plate's own dark left border
      // does not read as a gap between the emblem tile and the bar.
      ui.dl->PushClipRect(p0, p1, true);
      UiArt::DrawCover(ui.dl, plate, ImVec2(p0.x - 8.0f * ui.s, p0.y), p1,
                       selected ? IM_COL32(205, 230, 255, 245)
                                : IM_COL32(255, 255, 255, 245));
      ui.dl->PopClipRect();
      // Dark bands top and bottom so the tag and the name stay readable on the
      // art.
      ui.dl->AddRectFilledMultiColor(
          p0, p1, IM_COL32(4, 8, 14, 150), IM_COL32(4, 8, 14, 95),
          IM_COL32(4, 8, 14, 185), IM_COL32(4, 8, 14, 185));
    }
  }

  // One frame around the whole block, plus a divider where the emblem meets the
  // bar.
  ui.dl->AddRect(blockStart, blockEnd, IM_COL32(190, 200, 212, 110), 0.0f, 0,
                 1.0f);
  ui.dl->AddLine(ImVec2(p0.x, pos.y + 1.0f), ImVec2(p0.x, pos.y + h - 1.0f),
                 IM_COL32(10, 12, 16, 220), 1.0f);

  // Service tag, and "[DEV]" beside it in gold text for developer accounts only
  // (no chip, no extra tag for regular players).
  const float pad = 8.0f * ui.s;
  const float tagY = pos.y + 3.0f * ui.s;
  const std::string tagText =
      serviceTag.empty() ? std::string("[117]") : "[" + serviceTag + "]";
  Text(ui, NobleStyle::SmallFont(), 11.5f, ImVec2(p0.x + pad, tagY),
       IM_COL32(205, 220, 235, 235), tagText.c_str());
  if (dev) {
    const float tagW =
        TextWidth(NobleStyle::SmallFont(), 11.5f * ui.s, tagText.c_str());
    Text(ui, NobleStyle::SmallFont(), 11.5f,
         ImVec2(p0.x + pad + tagW + 5.0f * ui.s, tagY),
         IM_COL32(240, 196, 70, 255), "[DEV]");
  } else if (tester) {
    const float tagW =
        TextWidth(NobleStyle::SmallFont(), 11.5f * ui.s, tagText.c_str());
    Text(ui, NobleStyle::SmallFont(), 11.5f,
         ImVec2(p0.x + pad + tagW + 5.0f * ui.s, tagY),
         IM_COL32(70, 210, 240, 255), "[TESTER]");
  }

  ui.dl->PushClipRect(ImVec2(p0.x + pad, pos.y + 13.0f * ui.s),
                      ImVec2(p1.x - pad, p1.y), true);
  Text(ui, NobleStyle::TitleFont(), 16.5f,
       ImVec2(p0.x + pad, pos.y + 15.0f * ui.s), kWhite, name.c_str());
  ui.dl->PopClipRect();

  return h;
}

// The service tag the player is actually wearing: the Armory profile is what
// Reach is given (profile +0x1AC), so the menus follow it and only fall back to
// settings.json when no look has been saved yet. The two used to drift
// (settings said RTRD, the Armory said N06) and the party row disagreed with
// the Armory (2026-10-30).
std::string PlayerServiceTag(const SettingsData &sd) {
  const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
  if (!sd.serviceTag.empty() &&
      (prof.serviceTag.empty() ||
       (prof.serviceTag == "N06" && sd.serviceTag != "N06" &&
        sd.serviceTag != "NOBL"))) {
    return sd.serviceTag;
  }
  if (!prof.serviceTag.empty())
    return prof.serviceTag;
  return sd.serviceTag.empty() ? std::string("117") : sd.serviceTag;
}

// A compact "update available / downloading / ready" panel under MY PARTY. Only
// the menu process checks for releases; the button starts the download or hands
// over to the update script.
bool DrawUpdateBanner(const Ui &ui) {
  const Updater::Status st = Updater::Get().Snapshot();
  if (st.state != Updater::State::Available &&
      st.state != Updater::State::Downloading &&
      st.state != Updater::State::Ready) {
    return false;
  }
  const float w = 330.0f * ui.s;
  const float h = 74.0f * ui.s;
  const float x = ui.origin.x + ui.size.x - w - 40.0f * ui.s;
  const float y = ui.origin.y + 34.0f * ui.s + 128.0f * ui.s;
  ImDrawList *dl = ui.dl;
  dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h),
                    IM_COL32(8, 16, 26, 236), 3.0f * ui.s);
  dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), IM_COL32(143, 216, 255, 110),
              3.0f * ui.s, 0, 1.0f);

  const char *head = st.state == Updater::State::Ready ? "UPDATE READY"
                     : st.state == Updater::State::Downloading
                         ? "DOWNLOADING UPDATE"
                         : "UPDATE AVAILABLE";
  Text(ui, NobleStyle::SmallFont(), 13.0f,
       ImVec2(x + 12.0f * ui.s, y + 7.0f * ui.s), kAccent, head);
  Text(ui, NobleStyle::TitleFont(), 18.0f,
       ImVec2(x + 12.0f * ui.s, y + 22.0f * ui.s), kWhite,
       st.latestTag.empty() ? "NEW VERSION" : st.latestTag.c_str());

  const ImVec2 b0(x + 12.0f * ui.s, y + h - 30.0f * ui.s);
  const ImVec2 b1(x + w - 12.0f * ui.s, y + h - 8.0f * ui.s);
  if (st.state == Updater::State::Downloading) {
    const float p = std::clamp(st.progress, 0.0f, 1.0f);
    dl->AddRectFilled(b0, b1, IM_COL32(20, 30, 42, 230), 2.0f * ui.s);
    dl->AddRectFilled(b0, ImVec2(b0.x + (b1.x - b0.x) * p, b1.y),
                      IM_COL32(0, 120, 170, 230), 2.0f * ui.s);
    char label[64];
    std::snprintf(label, sizeof(label), "%.0f%% of %s", p * 100.0f,
                  Updater::FormatBytes(st.assetSize).c_str());
    Text(ui, NobleStyle::SmallFont(), 13.0f,
         ImVec2(b0.x + 8.0f * ui.s, b0.y + 3.0f * ui.s), kWhite, label);
    return false;
  }

  std::string label = st.state == Updater::State::Ready
                          ? "RESTART & INSTALL"
                          : "DOWNLOAD  " + Updater::FormatBytes(st.assetSize);
  ImGui::SetCursorScreenPos(b0);
  ImGui::PushID("noble_update_button");
  const bool clicked =
      ImGui::InvisibleButton("##update", ImVec2(b1.x - b0.x, b1.y - b0.y));
  const bool hovered = ImGui::IsItemHovered();
  ImGui::PopID();
  dl->AddRectFilled(
      b0, b1, hovered ? IM_COL32(30, 78, 110, 245) : IM_COL32(18, 46, 68, 235),
      2.0f * ui.s);
  dl->AddRect(b0, b1,
              hovered ? IM_COL32(180, 235, 255, 220)
                      : IM_COL32(100, 170, 210, 120),
              2.0f * ui.s);
  Text(ui, NobleStyle::SmallFont(), 14.0f,
       ImVec2(b0.x + ((b1.x - b0.x) - TextWidth(NobleStyle::SmallFont(),
                                                14.0f * ui.s, label.c_str())) *
                         0.5f,
              b0.y + 3.0f * ui.s),
       kWhite, label.c_str());
  if (clicked) {
    MenuSounds::Click();
    if (st.state == Updater::State::Ready) {
      if (Updater::Get().ApplyAndRestart())
        return true;
      MenuSounds::Back();
    } else {
      Updater::Get().StartDownload();
    }
  }
  return false;
}

void DrawParty(const Ui &ui, const std::string &playerName,
               const char *status) {
  const float w = 330.0f * ui.s;
  const float x = ui.origin.x + ui.size.x - w - 40.0f * ui.s;
  float y = ui.origin.y + 34.0f * ui.s;
  ui.dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w * 0.80f, y + 26.0f * ui.s),
                       IM_COL32(0, 0, 0, 200));
  Text(ui, NobleStyle::TitleFont(), 17.0f,
       ImVec2(x + 8.0f * ui.s, y + 3.0f * ui.s), kWhite, "MY PARTY");
  y += 26.0f * ui.s;
  const float x2 = x + 36.0f * ui.s;
  ui.dl->AddRectFilled(ImVec2(x2, y), ImVec2(x + w, y + 28.0f * ui.s),
                       IM_COL32(20, 26, 32, 215));
  Text(ui, NobleStyle::TitleFont(), 17.0f,
       ImVec2(x2 + 8.0f * ui.s, y + 5.0f * ui.s), kWhite, status);
  Text(ui, NobleStyle::TitleFont(), 17.0f,
       ImVec2(x + w - 42.0f * ui.s, y + 5.0f * ui.s), kWhite, "1/16");
  y += 28.0f * ui.s;
  // MCC-style player row: rank badge, emblem, then the nameplate bar with the
  // service tag on top and the gamertag under it (same helper the pre-game
  // lobby uses).
  {
    const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
    const SettingsData sd = Settings::Get().Snapshot();
    static int s_rank = -1;
    static ULONGLONG s_rankRead = 0;
    if (s_rank < 0 || GetTickCount64() - s_rankRead > 2000) {
      s_rank = Career::RankIndex(Career::Get().credits);
      s_rankRead = GetTickCount64();
    }
    y += PlayerRow(ui, ImVec2(x2, y), x + w - x2, s_rank, prof.emblemIndex,
                   prof.nameplateIndex, 0 /* no team in the main menu */,
                   PlayerServiceTag(sd), playerName,
                   AuthClient::Get().IsDeveloper(), false,
                   prof.emblemBackgroundIndex,
                   UIManager::GetArmorCustomizer().EmblemColorsNow(),
                   AuthClient::Get().IsTester());
  }
  const float accH = 22.0f * ui.s;
  const ImVec2 accP0(x2, y);
  const ImVec2 accP1(x + w, y + accH);
  ImGui::SetCursorScreenPos(accP0);
  ImGui::PushID("party_account_btn");
  bool accClicked =
      ImGui::InvisibleButton("##acc", ImVec2(accP1.x - accP0.x, accH));
  bool accHov = ImGui::IsItemHovered();
  ImGui::PopID();
  if (accClicked) {
    OpenAccountModal();
    MenuSounds::Click();
  }
  ui.dl->AddRectFilled(accP0, accP1,
                       accHov ? IM_COL32(30, 48, 68, 230)
                              : IM_COL32(15, 24, 34, 200));
  ui.dl->AddRect(accP0, accP1,
                 accHov ? IM_COL32(143, 216, 255, 180)
                        : IM_COL32(60, 90, 120, 100),
                 0.0f, 0, 1.0f);
  if (AuthClient::Get().IsLoggedIn()) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "ONLINE: %s",
                  AuthClient::Get().GetUsername().c_str());
    const ImU32 col =
        AuthClient::Get().IsDeveloper()
            ? IM_COL32(255, 215, 0, 255)
            : (AuthClient::Get().IsTester() ? IM_COL32(70, 210, 240, 255)
                                            : IM_COL32(120, 220, 160, 255));
    Text(ui, NobleStyle::SmallFont(), 12.0f,
         ImVec2(x2 + 8.0f * ui.s, y + 4.0f * ui.s), col, buf);
  } else {
    Text(ui, NobleStyle::SmallFont(), 12.0f,
         ImVec2(x2 + 8.0f * ui.s, y + 4.0f * ui.s),
         IM_COL32(200, 210, 220, 200), "[ LOGIN / REGISTER ]");
  }

  // The developer menu moved into the main-menu list (above Quit, developer
  // accounts only).
}

// DIFFICULTY as four tiles with Reach's difficulty icons (from the player's
// Reach art, see UiArt::DifficultyIcon; plain named tiles when there is none).
// Click picks; Left/Right change it when the row has focus. Returns the height
// used.
float DifficultyTiles(const Ui &ui, const char *id, ImVec2 pos, float width,
                      int &difficulty, bool focused) {
  const float top = 6.0f * ui.s; // keeps the label clear of the row above
  const float labelH = 26.0f * ui.s;
  const float gap = 8.0f * ui.s;
  const float tw = (width - gap * 3.0f) / 4.0f;
  const float th = 66.0f * ui.s;
  pos.y += top;
  Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(pos.x + 10.0f * ui.s, pos.y),
       focused ? kWhite : kDim, "DIFFICULTY");
  if (focused)
    ui.dl->AddRectFilled(
        ImVec2(pos.x, pos.y),
        ImVec2(pos.x + 2.0f * ui.s, pos.y + labelH - 6.0f * ui.s), kAccent);
  const float ty = pos.y + labelH;
  static const char *const kNames[4] = {"EASY", "NORMAL", "HEROIC",
                                        "LEGENDARY"};
  ImGui::PushID(id);
  for (int d = 0; d < 4; ++d) {
    const ImVec2 a(pos.x + d * (tw + gap), ty);
    const ImVec2 b(a.x + tw, ty + th);
    ImGui::SetCursorScreenPos(a);
    ImGui::PushID(d);
    const bool clicked = ImGui::InvisibleButton("##tile", ImVec2(tw, th));
    const bool hovered = ImGui::IsItemHovered();
    const ImGuiID hoverId = ImGui::GetItemID();
    ImGui::PopID();
    const bool sel = d == difficulty;
    ui.dl->AddRectFilled(a, b,
                         sel ? IM_COL32(52, 98, 138, 200)
                             : (hovered ? IM_COL32(40, 56, 72, 200)
                                        : IM_COL32(16, 22, 30, 190)));
    ui.dl->AddRect(a, b,
                   sel ? kAccent : IM_COL32(170, 220, 255, hovered ? 120 : 45),
                   0.0f, 0, sel ? 2.0f : 1.0f);
    const UiTexture icon = UiArt::DifficultyIcon(d);
    const float nameH = 20.0f * ui.s;
    if (icon) {
      UiArt::DrawContain(
          ui.dl, icon, ImVec2(a.x + 4.0f * ui.s, a.y + 4.0f * ui.s),
          ImVec2(b.x - 4.0f * ui.s, b.y - nameH),
          sel || hovered ? IM_COL32_WHITE : IM_COL32(255, 255, 255, 170));
    }
    const float nw =
        TextWidth(NobleStyle::TitleFont(), 15.0f * ui.s, kNames[d]);
    Text(ui, NobleStyle::TitleFont(), 15.0f,
         ImVec2(a.x + (tw - nw) * 0.5f,
                icon ? b.y - nameH : a.y + (th - nameH) * 0.5f),
         sel ? kWhite : kDim, kNames[d]);
    if (hovered)
      MenuSounds::Hover(hoverId);
    if (clicked && difficulty != d) {
      difficulty = d;
      MenuSounds::Change();
    }
  }
  ImGui::PopID();
  if (focused) {
    int change = 0;
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || UiPad::Left())
      change = -1;
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || UiPad::Right())
      change = 1;
    if (change) {
      difficulty = std::clamp(difficulty + change, 0, 3);
      MenuSounds::Change();
    }
  }
  return top + labelH + th;
}

// `note`, when given, follows the value in dim lower-case ("(default)").
// `arrows` false: a row that only opens something (the Skulls screen): no
// arrows, the value is centred in the whole value area.
int OptionRow(const Ui &ui, const char *id, ImVec2 pos, float width,
              const char *label, const char *value, bool focused,
              const char *note, bool arrows) {
  const float h = 32.0f * ui.s;
  int change = 0;
  const ImVec2 b(pos.x + width, pos.y + h);
  // Faint plate under every row: the setup screens sit on bright map art, and
  // dim rows were hard to read.
  ui.dl->AddRectFilled(pos, b, IM_COL32(8, 12, 18, 85));
  if (focused) {
    ui.dl->AddRectFilledMultiColor(
        pos, b, IM_COL32(60, 120, 170, 120), IM_COL32(60, 120, 170, 20),
        IM_COL32(60, 120, 170, 20), IM_COL32(60, 120, 170, 120));
    ui.dl->AddRectFilled(pos, ImVec2(pos.x + 2.0f * ui.s, b.y), kAccent);
  }
  Text(ui, NobleStyle::TitleFont(), 18.0f,
       ImVec2(pos.x + 10.0f * ui.s, pos.y + 6.0f * ui.s),
       focused ? kWhite : kDim, label);
  const float vx = pos.x + width * 0.48f;
  const float vw = width * 0.52f;
  const float aw = 28.0f * ui.s;
  if (arrows) {
    ImGui::PushID(id);
    ImGui::SetCursorScreenPos(ImVec2(vx, pos.y));
    if (ImGui::InvisibleButton("##l", ImVec2(aw, h)))
      change = -1;
    const bool hl = ImGui::IsItemHovered();
    ImGui::SetCursorScreenPos(ImVec2(vx + vw - aw, pos.y));
    if (ImGui::InvisibleButton("##r", ImVec2(aw, h)))
      change = 1;
    const bool hr = ImGui::IsItemHovered();
    ImGui::PopID();
    const float cy = pos.y + h * 0.5f, ah = 6.0f * ui.s;
    ui.dl->AddTriangleFilled(ImVec2(vx + aw * 0.65f, cy - ah),
                             ImVec2(vx + aw * 0.65f, cy + ah),
                             ImVec2(vx + aw * 0.3f, cy), hl ? kWhite : kDim);
    const float rx = vx + vw - aw;
    ui.dl->AddTriangleFilled(ImVec2(rx + aw * 0.35f, cy - ah),
                             ImVec2(rx + aw * 0.35f, cy + ah),
                             ImVec2(rx + aw * 0.7f, cy), hr ? kWhite : kDim);
  }
  const std::string v = Upper(value);
  const float vtw = TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, v.c_str());
  const float gap = note ? 6.0f * ui.s : 0.0f;
  const float ntw =
      note ? TextWidth(NobleStyle::SmallFont(), 15.0f * ui.s, note) : 0.0f;
  const float tx = vx + (vw - (vtw + gap + ntw)) * 0.5f;
  Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(tx, pos.y + 6.0f * ui.s),
       kWhite, v.c_str());
  if (note)
    Text(ui, NobleStyle::SmallFont(), 15.0f,
         ImVec2(tx + vtw + gap, pos.y + 9.0f * ui.s), kDim, note);
  if (focused) {
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || UiPad::Left())
      change = -1;
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || UiPad::Right())
      change = 1;
  }
  if (change)
    MenuSounds::Change();
  return change;
}

// A choice list (PickerRow) is open: set while one is drawn, read at the start
// of the next frame so Back and the up/down keys act on the list instead of the
// screen under it.

// The mark of a row whose middle can be clicked: a plate and underline when the
// mouse is over it (or its list is open) and a hand cursor. No caret or
// chevron. (`value` and `down` are kept for the callers.)
void DrawMiddleHint(const Ui &ui, ImVec2 pos, float width, const char *value,
                    bool hovered, bool open, bool down) {
  const float h = 32.0f * ui.s, aw = 28.0f * ui.s;
  const float vx = pos.x + width * 0.48f, vw = width * 0.52f;
  if (hovered)
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
  // The highlight covers the whole value section (arrows included), so the
  // player sees where the box is.
  const bool inSection = ImGui::IsMouseHoveringRect(
      pos, ImVec2(pos.x + width, pos.y + h)); // the whole row, label included
  if (inSection || hovered || open) {
    ui.dl->AddRectFilled(ImVec2(vx, pos.y + 2.0f * ui.s),
                         ImVec2(vx + vw, pos.y + h - 2.0f * ui.s),
                         IM_COL32(143, 216, 255, open ? 40 : 28), 2.0f * ui.s);
    ui.dl->AddRectFilled(ImVec2(vx, pos.y + h - 3.0f * ui.s),
                         ImVec2(vx + vw, pos.y + h - 1.0f * ui.s),
                         IM_COL32(143, 216, 255, open ? 200 : 140));
  }
  (void)aw;
  (void)value;
  (void)down;
}

// An OptionRow whose middle also opens something when clicked, with the hint
// drawn. Returns the arrows' change (-1, 0, 1); `clicked` is set when the
// middle was clicked.
int ClickRow(const Ui &ui, const char *id, ImVec2 pos, float width,
             const char *label, const char *value, bool focused, bool *clicked,
             bool down) {
  const int change =
      OptionRow(ui, id, pos, width, label, value, focused, nullptr, false);
  ImGui::PushID(id);
  // The whole row is the click target. It used to be only the middle/value 52%, so clicking the row's label (or the
  // empty left half) did nothing - that is why GAME OPTIONS looked unclickable on Custom/Firefight/Forge.
  ImGui::SetCursorScreenPos(pos);
  const bool hit = ImGui::InvisibleButton("##row", ImVec2(width, 32.0f * ui.s));
  DrawMiddleHint(ui, pos, width, value, ImGui::IsItemHovered(), false, down);
  ImGui::PopID();
  if (hit)
    MenuSounds::Click();
  if (clicked)
    *clicked = hit;
  return change;
}

// An OptionRow whose middle can also be clicked: it opens a list of every
// choice, so a player picks one directly instead of stepping the arrows through
// them all. The middle shows that it is clickable: a plate and underline on
// hover, a hand cursor and a small drop-down caret after the value. `fill` runs
// only while the list is open: it fills the choice texts and the index of the
// current one. Returns the arrows' change (-1, 0, 1) like OptionRow; `picked`
// gets the chosen index on the frame a choice is clicked, else -1. `openNow`
// (the row's Accept key, say) opens the list too.
int PickerRow(
    const Ui &ui, const char *id, ImVec2 pos, float width, const char *label,
    const char *value, bool focused,
    const std::function<void(std::vector<std::string> &, int &)> &fill,
    int *picked, bool openNow, const char *note, bool labelOpens) {
  if (picked)
    *picked = -1;
  ImGui::PushID(id);
  const bool wasOpen = ImGui::IsPopupOpen("##pickpopup");
  ImGui::PopID();
  // No side arrows: the whole value section is the button (the left and right
  // keys still step the value).
  int change =
      OptionRow(ui, id, pos, width, label, value, focused, note, false);
  if (wasOpen)
    change = 0; // the list is open: left/right must not also step the value
                // behind it
  const float h = 32.0f * ui.s;
  const float vx = pos.x + width * 0.48f, vw = width * 0.52f;
  ImGui::PushID(id);
  ImGui::SetCursorScreenPos(ImVec2(vx, pos.y));
  bool clicked = ImGui::InvisibleButton("##pick", ImVec2(vw, h));
  const bool hovered = ImGui::IsItemHovered();
  if (hovered)
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
  if (labelOpens) {
    // A click on the label opens the list too: the whole row is the button.
    ImGui::SetCursorScreenPos(pos);
    if (ImGui::InvisibleButton("##picklabel", ImVec2(width * 0.48f, h)))
      clicked = true;
    if (ImGui::IsItemHovered())
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
  }
  if (clicked || (openNow && !wasOpen)) {
    ImGui::OpenPopup("##pickpopup");
    MenuSounds::Click();
  }
  const bool open = ImGui::IsPopupOpen("##pickpopup");
  if (open)
    g_pickerOpenNow = true;
  const bool inSection = ImGui::IsMouseHoveringRect(
      pos, ImVec2(pos.x + width, pos.y + h)); // the whole row, label included
  if (inSection || hovered || open) {
    ui.dl->AddRectFilled(ImVec2(vx, pos.y + 2.0f * ui.s),
                         ImVec2(vx + vw, pos.y + h - 2.0f * ui.s),
                         IM_COL32(143, 216, 255, open ? 40 : 28), 2.0f * ui.s);
    ui.dl->AddRectFilled(ImVec2(vx, pos.y + h - 3.0f * ui.s),
                         ImVec2(vx + vw, pos.y + h - 1.0f * ui.s),
                         IM_COL32(143, 216, 255, open ? 200 : 140));
  }
  if (open && fill) {
    std::vector<std::string> choices;
    int current = -1;
    fill(choices, current);
    const float rowH = 30.0f * ui.s;
    const int count = static_cast<int>(choices.size());
    // Fit the room on screen: below the row, or above it when there is more
    // space there; the list scrolls (mouse wheel or its scrollbar) when the
    // choices don't all fit. The bottom keeps clear of the key hints.
    const float wanted = count * rowH + 8.0f * ui.s;
    const float below = ui.origin.y + ui.size.y - 84.0f * ui.s - (pos.y + h);
    const float above = pos.y - (ui.origin.y + 8.0f * ui.s);
    const bool openUp = wanted > below && above > below;
    const float listH =
        std::max(rowH * 2.0f, std::min(wanted, openUp ? above : below));
    ImGui::SetNextWindowPos(ImVec2(vx, openUp ? pos.y - listH : pos.y + h));
    ImGui::SetNextWindowSize(ImVec2(vw, listH));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(10, 16, 24, 248));
    ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(143, 216, 255, 150));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 4.0f * ui.s));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    static ImGuiID s_scrolledFor = 0;
    const ImGuiID popupId = ImGui::GetID("##pickpopup");
    if (ImGui::BeginPopup("##pickpopup",
                          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                              ImGuiWindowFlags_NoTitleBar |
                              ImGuiWindowFlags_NoSavedSettings)) {
      ImDrawList *pd = ImGui::GetWindowDrawList();
      const bool firstFrame = s_scrolledFor != popupId;
      // Keyboard and pad: up/down move the cursor, Accept picks it, Back (Esc /
      // B) closes the list and nothing else. Ignored on the frame the list
      // opens, so the press that opened it doesn't also pick.
      static int s_cursor = 0;
      if (firstFrame)
        s_cursor = std::max(0, current);
      bool cursorMoved = false;
      bool acceptKey = false;
      if (wasOpen && count > 0) {
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || UiPad::Down()) {
          s_cursor = (s_cursor + 1) % count;
          cursorMoved = true;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up()) {
          s_cursor = (s_cursor + count - 1) % count;
          cursorMoved = true;
        }
        acceptKey = ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                    (!UiPad::IsVirtualCursorActive() && UiPad::AcceptPressed());
        if (UiPad::BackPressed()) {
          UiPad::ConsumeBack();
          ImGui::CloseCurrentPopup();
          MenuSounds::Back();
        }
        if (cursorMoved)
          MenuSounds::Scroll();
      }
      const ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
      for (int i = 0; i < count; ++i) {
        ImGui::PushID(i);
        const ImVec2 p = ImGui::GetCursorScreenPos();
        bool chosen = ImGui::InvisibleButton("##c", ImVec2(vw, rowH));
        const bool over = ImGui::IsItemHovered();
        if (over && (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f))
          s_cursor = i;
        if (acceptKey && i == s_cursor)
          chosen = true;
        if ((firstFrame && i == current) || (cursorMoved && i == s_cursor))
          ImGui::SetScrollHereY(0.5f);
        if (i == current)
          pd->AddRectFilled(p, ImVec2(p.x + vw, p.y + rowH),
                            IM_COL32(60, 120, 170, 110));
        if (i == s_cursor)
          pd->AddRectFilled(p, ImVec2(p.x + vw, p.y + rowH),
                            IM_COL32(143, 216, 255, 40));
        if (i == current)
          pd->AddRectFilled(p, ImVec2(p.x + 2.0f * ui.s, p.y + rowH), kAccent);
        const std::string text = Upper(choices[i].c_str());
        const float tw =
            TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, text.c_str());
        pd->AddText(NobleStyle::TitleFont(), 18.0f * ui.s,
                    ImVec2(p.x + (vw - tw) * 0.5f, p.y + 5.0f * ui.s),
                    i == current || i == s_cursor ? kWhite : kDim,
                    text.c_str());
        ImGui::PopID();
        if (chosen) {
          if (picked)
            *picked = i;
          MenuSounds::Change();
          ImGui::CloseCurrentPopup();
        }
      }
      s_scrolledFor = popupId;
      ImGui::EndPopup();
    } else {
      s_scrolledFor = 0;
    }
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
  }
  ImGui::PopID();
  return change;
}

bool ActionButton(const Ui &ui, const char *id, ImVec2 pos, float width,
                  const char *label, bool primary, bool focused) {
  const float h = 40.0f * ui.s;
  ImGui::SetCursorScreenPos(pos);
  ImGui::PushID(id);
  const bool pressed = ImGui::InvisibleButton("##b", ImVec2(width, h));
  const bool hovered = ImGui::IsItemHovered();
  ImGui::PopID();
  const float r = 6.0f * ui.s;
  const ImVec2 p1(pos.x + width, pos.y + h);
  const ImU32 bg = primary
                       ? ((hovered || focused) ? IM_COL32(70, 152, 206, 255)
                                               : IM_COL32(44, 108, 154, 242))
                       : ((hovered || focused) ? IM_COL32(52, 72, 92, 242)
                                               : IM_COL32(22, 32, 44, 232));
  ui.dl->AddRectFilled(pos, p1, bg, r);
  if (focused) {
    NobleUi::DrawFocusHighlight(ui.dl, pos, p1, ui.s, true);
  } else {
    ui.dl->AddRect(pos, p1, IM_COL32(143, 216, 255, hovered ? 225 : 95), r, 0,
                   hovered ? 1.4f * ui.s : 1.0f);
  }
  const float tw = TextWidth(NobleStyle::TitleFont(), 20.0f * ui.s, label);
  Text(ui, NobleStyle::TitleFont(), 20.0f,
       ImVec2(pos.x + (width - tw) * 0.5f, pos.y + 9.0f * ui.s), kWhite, label);
  if (hovered)
    MenuSounds::Hover(ImGui::GetID(id));
  if (pressed)
    MenuSounds::Click();
  return pressed;
}

float DrawPreview(const Ui &ui, ImVec2 pos, float width,
                  const std::string &scenario, const char *name,
                  const char *blurb) {
  const float h = width * 292.0f / 668.0f;
  const float r = 6.0f * ui.s;
  const ImVec2 b(pos.x + width, pos.y + h);
  ui.dl->AddRectFilled(pos, b, IM_COL32(14, 20, 28, 230), r);
  UiTexture tex = UiArt::MapPreview(scenario, true);
  if (tex)
    UiArt::DrawCover(ui.dl, tex, pos, b);
  ui.dl->AddRect(pos, b, IM_COL32(143, 216, 255, 130), r, 0, 1.0f);
  float y = b.y + 10.0f * ui.s;
  Text(ui, NobleStyle::TitleFont(), 26.0f, ImVec2(pos.x, y), kWhite, name);
  y += 32.0f * ui.s;
  if (blurb && *blurb) {
    Text(ui, nullptr, 17.0f, ImVec2(pos.x, y), kDim, blurb, pos.x + width);
    ImFont *font = Font(nullptr);
    const float blurbHeight =
        font->CalcTextSizeA(17.0f * ui.s, FLT_MAX, width, blurb).y;
    y += blurbHeight + 8.0f * ui.s;
  }
  return y;
}

// Spec §3 HeroBanner: the map picture with a dark gradient, an accent kicker
// line, the title, and the blurb under the picture. Returns the y below the
// blurb.
float HeroBanner(const Ui &ui, ImVec2 pos, float width, float height,
                 const std::string &scenario, const std::string &kicker,
                 const char *title, const char *blurb,
                 const std::filesystem::path &image) {
  const ImVec2 b(pos.x + width, pos.y + height);
  ui.dl->AddRectFilled(pos, b, IM_COL32(14, 20, 26, 230));
  const UiTexture art = image.empty() ? UiArt::MapPreview(scenario, true)
                                      : UiArt::ImageFile(image);
  const float crispMax = std::max(1.0f, ImGui::GetIO().DisplaySize.y / 1080.0f);
  if (art)
    UiArt::DrawCoverCrisp(ui.dl, art, pos, b, IM_COL32_WHITE, crispMax);
  const float gy = pos.y + height * 0.45f;
  ui.dl->AddRectFilledMultiColor(ImVec2(pos.x, gy), b, IM_COL32(4, 8, 12, 0),
                                 IM_COL32(4, 8, 12, 0), IM_COL32(4, 8, 12, 235),
                                 IM_COL32(4, 8, 12, 235));
  ui.dl->AddRect(pos, b, IM_COL32(170, 200, 220, 120), 0.0f, 0, 1.0f);
  const float pad = 16.0f * ui.s;
  // Long mod titles shrink until the whole name fits before falling back to an
  // ellipsis.
  std::string bannerTitle = Upper(title);
  float titlePx = 34.0f * ui.s;
  while (titlePx > 22.0f * ui.s &&
         TextWidth(NobleStyle::TitleFont(), titlePx, bannerTitle.c_str()) >
             width - pad * 2.0f) {
    titlePx -= 2.0f * ui.s;
  }
  bannerTitle = Ellipsize(NobleStyle::TitleFont(), titlePx, bannerTitle,
                          width - pad * 2.0f);
  Text(ui, NobleStyle::TitleFont(), titlePx / ui.s,
       ImVec2(pos.x + pad, b.y - pad - titlePx), kWhite, bannerTitle.c_str());
  if (!kicker.empty()) {
    const std::string bannerKicker = Ellipsize(
        NobleStyle::SmallFont(), 15.0f * ui.s, kicker, width - pad * 2.0f);
    Text(ui, NobleStyle::SmallFont(), 15.0f,
         ImVec2(pos.x + pad, b.y - pad - titlePx - 22.0f * ui.s), kAccent,
         bannerKicker.c_str());
  }
  float y = b.y + 8.0f * ui.s;
  if (blurb && *blurb) {
    // Two lines at most: a long workshop blurb is clipped here instead of
    // running over the option rows.
    const float lineH = 17.0f * ui.s * 1.35f;
    const std::string fitted = FitWrapped(nullptr, 17.0f * ui.s, blurb,
                                          width - 4.0f * ui.s, lineH * 2.0f);
    // The text's own height: the band and the clip stop there, because the rows
    // below start 8 px under it. (The band used to be two full 1.35-spaced
    // lines, about 8 px taller than the space reserved, so it ran into the Game
    // Type row.)
    const float usedH = std::min(
        Font(nullptr)
            ->CalcTextSizeA(17.0f * ui.s, FLT_MAX, width, fitted.c_str())
            .y,
        lineH * 2.0f);
    const ImVec2 t0(pos.x, y), t1(pos.x + width, y + usedH);
    // The description sits over map art, so it gets a soft dark band and a 1 px
    // shadow: plain grey text on a bright picture was borderline unreadable
    // (Tyler, 2026-10-30).
    ui.dl->AddRectFilledMultiColor(
        ImVec2(t0.x, t0.y - 4.0f * ui.s), ImVec2(t1.x, t1.y + 3.0f * ui.s),
        IM_COL32(4, 8, 12, 40), IM_COL32(4, 8, 12, 150),
        IM_COL32(4, 8, 12, 150), IM_COL32(4, 8, 12, 40));
    ui.dl->PushClipRect(ImVec2(pos.x, y - 2.0f * ui.s),
                        ImVec2(pos.x + width, y + usedH + 2.0f * ui.s), true);
    Text(ui, nullptr, 17.0f, ImVec2(pos.x + 1.0f * ui.s, y + 1.0f * ui.s),
         IM_COL32(0, 0, 0, 210), fitted.c_str(), pos.x + width);
    Text(ui, nullptr, 17.0f, ImVec2(pos.x, y), IM_COL32(214, 224, 236, 255),
         fitted.c_str(), pos.x + width);
    ui.dl->PopClipRect();
    y += usedH + 8.0f * ui.s;
  }
  return y;
}

// Spec §3 BigAction: large button with a title and a subtitle line. Disabled
// buttons are greyed and show their subtitle as the reason.
bool BigAction(const Ui &ui, const char *id, ImVec2 pos, float width,
               const char *label, const char *sub, bool primary, bool enabled,
               bool focused) {
  const float h = 50.0f * ui.s;
  ImGui::SetCursorScreenPos(pos);
  ImGui::PushID(id);
  const bool pressed =
      ImGui::InvisibleButton("##b", ImVec2(width, h)) && enabled;
  const bool hovered = ImGui::IsItemHovered() && enabled;
  ImGui::PopID();
  const float r = 6.0f * ui.s;
  const ImVec2 b(pos.x + width, pos.y + h);
  const ImU32 bg = !enabled ? IM_COL32(20, 26, 34, 205)
                   : primary
                       ? (hovered || focused ? IM_COL32(68, 152, 208, 255)
                                             : IM_COL32(44, 108, 154, 242))
                       : (hovered || focused ? IM_COL32(52, 72, 92, 242)
                                             : IM_COL32(22, 32, 44, 232));
  ui.dl->AddRectFilled(pos, b, bg, r);
  if (focused && enabled) {
    NobleUi::DrawFocusHighlight(ui.dl, pos, b, ui.s, true);
  } else {
    ui.dl->AddRect(pos, b,
                   enabled ? IM_COL32(143, 216, 255, hovered ? 220 : 100)
                           : IM_COL32(255, 255, 255, 34),
                   r, 0, hovered ? 1.4f * ui.s : 1.0f);
  }
  const std::string actionLabel = Ellipsize(
      NobleStyle::TitleFont(), 20.0f * ui.s, label, width - 28.0f * ui.s);
  Text(ui, NobleStyle::TitleFont(), 20.0f,
       ImVec2(pos.x + 14.0f * ui.s, pos.y + 5.0f * ui.s),
       enabled ? kWhite : kDisabled, actionLabel.c_str());
  if (sub && *sub) {
    const std::string actionSub = Ellipsize(
        NobleStyle::SmallFont(), 14.0f * ui.s, sub, width - 28.0f * ui.s);
    Text(ui, NobleStyle::SmallFont(), 14.0f,
         ImVec2(pos.x + 14.0f * ui.s, pos.y + 29.0f * ui.s),
         enabled ? IM_COL32(200, 225, 245, 230) : kDisabled, actionSub.c_str());
  }
  if (hovered)
    MenuSounds::Hover(ImGui::GetID(id));
  if (pressed)
    MenuSounds::Click();
  return pressed;
}

// Small icons of the skulls that are on, in a row; returns the width used.
float SkullStrip(const Ui &ui, ImVec2 pos, float size, uint64_t mask,
                 int maxIcons);

float SkullStrip(const Ui &ui, ImVec2 pos, float size, uint64_t mask,
                 int maxIcons) {
  float x = pos.x;
  int shown = 0;
  for (int i = 0; i < kSkullCount && shown < maxIcons; ++i) {
    if (!(mask & (1ull << kSkulls[i].bit)))
      continue;
    if (UiTexture icon = UiArt::SkullIcon(kSkulls[i].icon)) {
      ui.dl->AddImage(icon.id, ImVec2(x, pos.y),
                      ImVec2(x + size, pos.y + size));
    } else {
      ui.dl->AddRect(ImVec2(x, pos.y), ImVec2(x + size, pos.y + size),
                     IM_COL32(255, 255, 255, 80));
    }
    x += size + 3.0f * ui.s;
    ++shown;
  }
  return x - pos.x;
}

std::string SkullSummary(uint64_t mask) {
  int count = 0;
  for (const Skull &s : kSkulls)
    if (mask & (1ull << s.bit))
      ++count;
  if (count == 0)
    return "0";
  // All thirteen on reads as "All Skulls" (Tyler, 2026-10-30).
  if (count >= kSkullCount)
    return "All Skulls";
  if (count == 1) {
    for (const Skull &s : kSkulls)
      if (mask & (1ull << s.bit))
        return s.name;
  }
  return std::to_string(count) + " on";
}

float ScoreMultiplier(int difficulty, uint64_t mask) {
  float m = kDifficultyMultiplier[std::clamp(difficulty, 0, 3)];
  for (const Skull &s : kSkulls) {
    if ((mask & (1ull << s.bit)) && s.multiplier > 1.0f)
      m += s.multiplier;
  }
  return m;
}

constexpr float kLobbyStripHeight = 34.0f; // multiplied by ui.s

float LobbyStripHeight(const Ui &ui) { return kLobbyStripHeight * ui.s; }

struct RightColumnY {
  float actions =
      0.0f; // top of PLAY SOLO / HOST CO-OP (BigAction is 50 * s tall)
  float strip = 0.0f;   // top of the network / lobby chip strip
  float content = 0.0f; // the option rows must end at or above this
};

RightColumnY RightColumnLayout(const Ui &ui) {
  RightColumnY y;
  const float hints = ui.origin.y + ui.size.y -
                      58.0f * ui.s; // DrawHints' rule sits at size.y - 58 * s
  y.actions = hints - 14.0f * ui.s - 50.0f * ui.s;
  y.strip = y.actions - 12.0f * ui.s - LobbyStripHeight(ui);
  y.content = y.strip - 10.0f * ui.s;
  return y;
}

// Map picture height: fills whatever the rows and the strip leave, never taller
// than the picture's own aspect (292/668). Short or very narrow windows shrink
// it instead of pushing the rows into the strip.
float HeroBannerHeight(const Ui &ui, float columnWidth, float contentBottom,
                       float rowsH, float blurbH, float bannerTop) {
  const float maxH = columnWidth * 292.0f / 668.0f;
  const float minH = std::min(64.0f * ui.s, maxH);
  return std::max(minH,
                  std::min(maxH, contentBottom - rowsH - blurbH - bannerTop));
}

void DrawLobbyControls(const Ui &ui, ImVec2 pos, float width, MenuState &st) {
  const float stripH = LobbyStripHeight(ui);
  const float chipH = 30.0f * ui.s;
  const float chipY = pos.y + (stripH - chipH) * 0.5f;
  const float right = pos.x + width;
  bool online = (st.lobbyNetMode == 1);
  float x = pos.x;

  // Chip width follows its label, so NETWORK/LOBBY text is never clipped or
  // overflows the column.
  auto drawChip = [&](const char *id, const char *label, ImU32 bg,
                      ImU32 bgHover, ImU32 edge) {
    const float w =
        TextWidth(NobleStyle::SmallFont(), 13.0f * ui.s, label) + 22.0f * ui.s;
    const ImVec2 a(x, chipY);
    const ImVec2 b(a.x + w, a.y + chipH);
    ImGui::SetCursorScreenPos(a);
    ImGui::PushID(id);
    const bool pressed = ImGui::InvisibleButton("##chip", ImVec2(w, chipH));
    const bool hovered = ImGui::IsItemHovered();
    ImGui::PopID();
    ui.dl->AddRectFilled(a, b, hovered ? bgHover : bg);
    ui.dl->AddRect(a, b, hovered ? IM_COL32(205, 235, 255, 235) : edge, 0.0f, 0,
                   1.0f);
    Text(ui, NobleStyle::SmallFont(), 13.0f,
         ImVec2(a.x + 11.0f * ui.s, a.y + 7.0f * ui.s), kWhite, label);
    x = b.x + 8.0f * ui.s;
    if (pressed)
      MenuSounds::Click();
    return pressed;
  };

  // Network mode: Online P2P (master server + UPnP) or local LAN discovery.
  {
    const ImU32 bg =
        online ? IM_COL32(28, 80, 128, 215) : IM_COL32(52, 54, 62, 215);
    const ImU32 hov =
        online ? IM_COL32(40, 110, 170, 235) : IM_COL32(84, 84, 94, 235);
    const ImU32 edge =
        online ? IM_COL32(120, 170, 215, 110) : IM_COL32(150, 155, 165, 110);
    if (drawChip("btn_lobby_net", online ? "NETWORK: ONLINE" : "NETWORK: LOCAL",
                 bg, hov, edge)) {
      online = !online;
      st.lobbyNetMode = online ? 1 : 0;
    }
  }

  // Lobby privacy only exists online.
  if (online) {
    const bool isPublic = (st.lobbyPrivacy == 0);
    const ImU32 bg =
        isPublic ? IM_COL32(24, 90, 68, 215) : IM_COL32(80, 58, 28, 215);
    const ImU32 hov =
        isPublic ? IM_COL32(36, 120, 90, 235) : IM_COL32(110, 80, 40, 235);
    const ImU32 edge =
        isPublic ? IM_COL32(100, 180, 140, 120) : IM_COL32(190, 150, 90, 120);
    if (drawChip("btn_lobby_priv",
                 isPublic ? "LOBBY: PUBLIC" : "LOBBY: PRIVATE", bg, hov,
                 edge)) {
      st.lobbyPrivacy = isPublic ? 1 : 0;
    }
  }

  // Short connection line, right-aligned in the column. The candidates go from
  // most to least detailed and the first one that fits is drawn; when none fits
  // the line is left out instead of running into the chips or off the screen.
  const bool upnpReady =
      UPnP::Get().IsActive() && UPnP::Get().GetExternalIP() != "Unknown";
  const bool listed = (st.lobbyPrivacy == 0);
  std::vector<std::string> lines;
  ImU32 dot = IM_COL32(140, 160, 180, 200);
  if (online) {
    const char *host = upnpReady ? "UPnP READY" : "PORT 11774";
    lines.push_back(std::string(host) + " \xC2\xB7 " +
                    (listed ? "LISTED PUBLICLY" : "DIRECT CONNECT"));
    lines.push_back(host);
    lines.push_back(listed ? "LISTED" : "DIRECT");
    dot =
        upnpReady ? IM_COL32(110, 220, 150, 230) : IM_COL32(235, 190, 90, 230);
  } else {
    lines.push_back("LOCAL NETWORK \xC2\xB7 DISCOVERABLE");
    lines.push_back("LOCAL NETWORK");
    dot = IM_COL32(150, 180, 210, 220);
  }
  const float avail = right - x - 4.0f * ui.s;
  for (const std::string &line : lines) {
    const float w =
        TextWidth(NobleStyle::SmallFont(), 12.0f * ui.s, line.c_str());
    if (w > avail)
      continue;
    const float tx = right - w;
    // Backing plate: this line sits straight on the map art, and dim text
    // vanished over bright maps (Forge World).
    ui.dl->AddRectFilled(
        ImVec2(tx - 17.0f * ui.s, pos.y + 2.0f * ui.s),
        ImVec2(right + 5.0f * ui.s, pos.y + stripH - 2.0f * ui.s),
        IM_COL32(8, 14, 22, 190), 2.0f * ui.s);
    ui.dl->AddCircleFilled(ImVec2(tx - 9.0f * ui.s, pos.y + stripH * 0.5f),
                           3.0f * ui.s, dot);
    Text(ui, NobleStyle::SmallFont(), 12.0f,
         ImVec2(tx, pos.y + (stripH - 13.0f * ui.s) * 0.5f),
         IM_COL32(205, 218, 230, 255), line.c_str());
    break;
  }
}

bool KnownScenario(const std::string &scenario, const MenuMap *maps,
                   int count) {
  for (int i = 0; i < count; ++i)
    if (scenario == maps[i].scenario)
      return true;
  return false;
}

// The stock map a modded map with a new scenario is loaded through. The engine
// picks a map by launch index/map id, so the mod map must carry the scenario
// map id of a stock map; the stock map's file is then replaced by the mod's
// .map through VFS::SetMapOverride. Boneyard (map id 1080) is what the
// campaign-to-Forge conversions and most Forge canvases are built on; Glacier
// is the Firefight canvas and Winter Contingency the campaign base. A mod built
// on a different base map can be pointed at it by adding "<scenario>": "<stock
// scenario>" to data\map_carriers.json (loaded by CarrierForScenario).
std::string CarrierForScenario(const std::string &scenario,
                               const std::string &fallback) {
  static std::mutex s_mutex;
  static bool s_loaded = false;
  static std::unordered_map<std::string, std::string> s_carriers;
  std::lock_guard<std::mutex> lock(s_mutex);
  if (!s_loaded) {
    s_loaded = true;
    const std::filesystem::path path =
        VFS::GetUserStoragePath() / "map_carriers.json";
    std::ifstream file(path, std::ios::binary);
    if (file.is_open()) {
      try {
        const nlohmann::json j = nlohmann::json::parse(file);
        if (j.is_object()) {
          for (const auto &item : j.items()) {
            if (item.value().is_string()) {
              std::string key = item.key();
              for (char &c : key)
                c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
              s_carriers[key] = item.value().get<std::string>();
            }
          }
        }
        NOBLE_INFO("FrontEndMenu: {} map carrier override(s) from '{}'",
                   s_carriers.size(), path.string());
      } catch (const std::exception &e) {
        NOBLE_WARN("FrontEndMenu: '{}' is not valid map carrier JSON ({})",
                   path.string(), e.what());
      }
    }
  }
  std::string key = scenario;
  for (char &c : key)
    c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
  const auto it = s_carriers.find(key);
  if (it != s_carriers.end() && fallback != "m10")
    return it->second;
  if (fallback != "m10" && key.find("forge_halo") != std::string::npos)
    return "forge_halo";
  return fallback;
}

std::vector<MapEntry> ModMaps(const ModEntry &mod,
                              const std::vector<ModMap> &listed,
                              const MenuMap *base, int baseCount,
                              const std::string &carrier) {
  std::vector<MapEntry> out;
  for (const ModMap &m : listed) {
    const bool known = KnownScenario(m.scenario, base, baseCount);
    if (known) {
      // A mission on a stock map slot (the mod replaces that map's file) still
      // has its own pictures: the menu art and the loading screen come from the
      // mission's JSON, not from the stock map.
      MapEntry stock{m.scenario, m.title, PlainDescription(m.description, 150),
                     {},         {},      {},
                     {},         {},      true,
                     &mod};
      stock.imageThumbnail = m.imageThumbnail;
      stock.imageLarge = m.imageLarge.empty() ? m.imageThumbnail : m.imageLarge;
      stock.imageLoading =
          m.imageLoading.empty() ? stock.imageLarge : m.imageLoading;
      out.push_back(std::move(stock));
      continue;
    }
    const std::filesystem::path mapFile =
        mod.folder / "maps" / (m.scenario + ".map");
    if (!m.hasMapFile) {
      out.push_back({m.scenario,
                     m.title,
                     "This map needs maps\\" + m.scenario +
                         ".map, which the mod does not contain.",
                     {},
                     {},
                     {},
                     {},
                     {},
                     false,
                     &mod,
                     false,
                     true});
      continue;
    }
    MapEntry entry;
    entry.scenario = m.scenario;
    entry.name = m.title;
    // The map JSON's parent folder is only a usable carrier when it is a map
    // the engine knows (a stock scenario, or a Forge World canvas). A name such
    // as "trepanation" has no map id, and launching it starts the main menu
    // instead of the map, so those use the mode's default carrier (or the
    // data\map_carriers.json entry for the map).
    const bool baseUsable =
        !m.baseScenario.empty() &&
        (KnownScenario(m.baseScenario, base, baseCount) ||
         (carrier != "m10" &&
          m.baseScenario.find("forge_halo") != std::string::npos));
    const std::string defaultCarrier = baseUsable ? m.baseScenario : carrier;
    entry.launchScenario = CarrierForScenario(m.scenario, defaultCarrier);
    entry.mapOverride = mapFile;
    entry.imageThumbnail = m.imageThumbnail;
    entry.imageLarge = m.imageLarge.empty() ? m.imageThumbnail : m.imageLarge;
    entry.imageLoading = m.imageLoading.empty() ? m.imageLarge : m.imageLoading;
    entry.enabled = true;
    entry.mod = &mod;
    entry.custom = true;
    entry.blurb = "Custom map \xC2\xB7 loads through " +
                  MapNames::Display(entry.launchScenario) + ".";
    out.push_back(std::move(entry));
  }
  const bool hasManifestMaps = !mod.campaign.empty() ||
                               !mod.firefight.empty() ||
                               !mod.multiplayer.empty();
  if (!hasManifestMaps) {
    for (const std::string &file : mod.mapFiles) {
      if (!KnownScenario(file, base, baseCount)) {
        if (std::any_of(out.begin(), out.end(),
                        [&](const MapEntry &e) { return e.scenario == file; }))
          continue;
        const std::filesystem::path mapFile =
            mod.folder / "maps" / (file + ".map");
        MapEntry entry;
        entry.scenario = file;
        entry.name = MapNames::Display(file);
        std::string fileCarrier = carrier;
        if (carrier != "m10" && file.find("forge_halo") != std::string::npos)
          fileCarrier = "forge_halo";
        entry.launchScenario = CarrierForScenario(file, fileCarrier);
        entry.mapOverride = mapFile;
        entry.enabled = true;
        entry.mod = &mod;
        entry.custom = true;
        entry.blurb = "Custom map \xC2\xB7 loads on " +
                      MapNames::Display(entry.launchScenario);
        out.push_back(std::move(entry));
        continue;
      }
      if (std::any_of(out.begin(), out.end(),
                      [&](const MapEntry &e) { return e.scenario == file; }))
        continue;
      for (int i = 0; i < baseCount; ++i) {
        if (file == base[i].scenario)
          out.push_back({file,
                         base[i].name,
                         base[i].blurb,
                         {},
                         {},
                         {},
                         {},
                         {},
                         true,
                         &mod});
      }
    }
  }
  return out;
}

std::vector<MapEntry> BaseMaps(const MenuMap *maps, int count) {
  std::vector<MapEntry> out;
  for (int i = 0; i < count; ++i)
    out.push_back({maps[i].scenario,
                   maps[i].name,
                   maps[i].blurb,
                   {},
                   {},
                   {},
                   {},
                   {},
                   true});
  return out;
}

std::string OpenKey(ModMode mode, const ModEntry &mod) {
  return std::to_string(static_cast<int>(mode)) + ":" + mod.id;
}

std::vector<MapEntry> ModdedMaps(const std::vector<const ModEntry *> &mods,
                                 ModMode mode, const MenuMap *base,
                                 int baseCount,
                                 const std::set<std::string> &open,
                                 const std::string &carrier) {
  std::vector<MapEntry> out;
  for (const ModEntry *mod : mods) {
    const std::vector<ModMap> &listed =
        mode == ModMode::Campaign    ? mod->campaign
        : mode == ModMode::Firefight ? mod->firefight
                                     : mod->multiplayer;
    std::vector<MapEntry> maps =
        ModMaps(*mod, listed, base, baseCount, carrier);
    if (maps.empty())
      continue;
    MapEntry heading;
    heading.name = mod->title + "  (" + std::to_string(maps.size()) + ")";
    heading.header = true;
    heading.mod = mod;
    // A heading has no map of its own; the banner shows the mod's first map
    // picture instead of black.
    heading.scenario = maps.front().scenario;
    heading.imageLarge = maps.front().imageLarge;
    heading.imageThumbnail = maps.front().imageThumbnail;
    heading.blurb = PlainDescription(mod->description, 180);
    heading.enabled = open.count(OpenKey(mode, *mod)) != 0;
    out.push_back(heading);
    if (!heading.enabled)
      continue;
    for (MapEntry &m : maps)
      out.push_back(std::move(m));
  }
  return out;
}

void ToggleMod(std::set<std::string> &open, ModMode mode, const ModEntry &mod) {
  const std::string k = OpenKey(mode, mod);
  if (!open.erase(k))
    open.insert(k);
}

bool ContainsNoCase(std::string text, std::string needle) {
  const auto lower = [](std::string &s) {
    for (char &c : s)
      c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
  };
  lower(text);
  lower(needle);
  return text.find(needle) != std::string::npos;
}

// One search box for every list. Returns true when the text changed. Typing
// needs a keyboard.
bool DrawSearchBox(const Ui &ui, const char *id, ImVec2 pos, float width,
                   char *text, size_t size, const char *hint) {
  ImGui::SetCursorScreenPos(pos);
  ImGui::SetNextItemWidth(width);
  (void)ui;
  return ImGui::InputTextWithHint(id, hint, text, size);
}

// The list to show: normally `ModdedMaps` or the stock maps; while searching,
// every matching map with no mod headings.
std::vector<MapEntry>
ListedMaps(bool modded, const std::vector<const ModEntry *> &mods, ModMode mode,
           const MenuMap *base, int baseCount,
           const std::set<std::string> &open, const std::string &carrier,
           const char *query) {
  std::set<std::string> everyMod = open;
  if (query[0] != 0)
    for (const ModEntry *mod : mods)
      everyMod.insert(OpenKey(mode, *mod));
  std::vector<MapEntry> all =
      modded ? ModdedMaps(mods, mode, base, baseCount, everyMod, carrier)
             : BaseMaps(base, baseCount);
  if (query[0] == 0)
    return all;
  std::vector<MapEntry> hits;
  for (MapEntry &m : all) {
    if (m.header)
      continue;
    if (ContainsNoCase(m.name, query) ||
        (m.mod && ContainsNoCase(m.mod->title, query)))
      hits.push_back(std::move(m));
  }
  return hits;
}

void FixSelection(const std::vector<MapEntry> &list, int &selected) {
  selected = list.empty()
                 ? 0
                 : std::clamp(selected, 0, static_cast<int>(list.size()) - 1);
}

std::string ForgeOpenKey(const std::string &group) { return "forge:" + group; }

// The FORGE MAPS tab: a heading per group that drops down its maps, like the
// mod headings; while searching, every match (title, heading or creator) with
// no headings.
std::vector<MapEntry> ForgeMapEntries(const std::vector<ForgeMap> &forgeMaps,
                                      const std::set<std::string> &open,
                                      const char *query) {
  const auto row = [](const ForgeMap &m) {
    MapEntry e;
    e.scenario = m.scenario;
    e.name = m.title;
    const std::string by =
        m.creator.empty() ? std::string() : " by " + m.creator;
    e.blurb =
        !m.description.empty()
            ? PlainDescription(m.description, 150) + " (Forge map" + by + ")"
            : "Forge map" + by + " on " + MapNames::Display(m.scenario) + ".";
    e.mvar = m.path;
    e.forgeGroup = m.group;
    return e;
  };
  std::vector<MapEntry> out;
  if (query[0] != 0) {
    for (const ForgeMap &m : forgeMaps) {
      if (ContainsNoCase(m.title, query) || ContainsNoCase(m.group, query) ||
          ContainsNoCase(m.creator, query))
        out.push_back(row(m));
    }
    return out;
  }
  for (size_t i = 0; i < forgeMaps.size();) {
    size_t end = i;
    while (end < forgeMaps.size() && forgeMaps[end].group == forgeMaps[i].group)
      ++end;
    MapEntry heading;
    heading.name = forgeMaps[i].group + "  (" + std::to_string(end - i) + ")";
    heading.header = true;
    heading.scenario = forgeMaps[i].scenario;
    heading.forgeGroup = forgeMaps[i].group;
    heading.blurb = "Community Forge maps. From the Halo Customs MCC Megapack "
                    "and your own saved Forge maps.";
    heading.enabled = open.count(ForgeOpenKey(forgeMaps[i].group)) != 0;
    out.push_back(heading);
    if (heading.enabled)
      for (size_t k = i; k < end; ++k)
        out.push_back(row(forgeMaps[k]));
    i = end;
  }
  return out;
}

// Maps menu nesting: under each stock map, its own Forge map variants, as a
// collapsible heading. MCC ships them (haloreach\map_variants and
// haloreach\hopper_map_variants) and a player's own saved ones live in
// data\map_variants, so "atom" or "cliffhanger" appear under Forge World where
// they belong rather than only in the separate Forge maps tab.
void InsertMapVariants(std::vector<MapEntry> &list,
                       const std::vector<ForgeMap> &forgeMaps,
                       std::set<std::string> &open) {
  if (forgeMaps.empty() || list.empty())
    return;
  std::vector<MapEntry> out;
  out.reserve(list.size() + forgeMaps.size());
  for (const MapEntry &e : list) {
    out.push_back(e);
    if (e.header || e.mod || e.custom || !e.mvar.empty() || e.scenario.empty())
      continue; // stock map rows only
    std::vector<const ForgeMap *> variants;
    for (const ForgeMap &m : forgeMaps)
      if (m.scenario == e.scenario)
        variants.push_back(&m);
    if (variants.empty())
      continue;
    MapEntry heading;
    heading.name = "Map variants  (" + std::to_string(variants.size()) + ")";
    heading.header = true;
    heading.scenario = e.scenario;
    heading.forgeGroup = "variants:" + e.scenario;
    heading.enabled = open.count(ForgeOpenKey(heading.forgeGroup)) != 0;
    heading.blurb =
        "Forge map variants built on " + MapNames::Display(e.scenario) + ".";
    out.push_back(heading);
    if (!heading.enabled)
      continue;
    for (const ForgeMap *m : variants) {
      MapEntry row;
      row.scenario = m->scenario;
      row.name = m->title;
      row.blurb = !m->description.empty()
                      ? PlainDescription(m->description, 150) + " (Forge map)"
                      : "Forge map on " + MapNames::Display(m->scenario) + ".";
      row.mvar = m->path;
      row.forgeGroup = m->group;
      out.push_back(row);
    }
  }
  static int s_logged = 0;
  if (s_logged < 6) {
    ++s_logged;
    int rows = 0;
    for (const MapEntry &e : out)
      if (!e.mvar.empty())
        ++rows;
    std::string sample;
    for (const ForgeMap &m : forgeMaps) {
      sample += m.scenario + " ";
      if (sample.size() > 200)
        break;
    }
    sample += " || ";
    for (const MapEntry &e : out) {
      if (e.header || e.mod || e.custom)
        continue;
      sample += e.scenario + " ";
      if (sample.size() > 400)
        break;
    }
    NOBLE_INFO(
        "FrontEndMenu: map list {} row(s), {} variants nested; base rows: {}",
        out.size(), rows, sample);
  }
  list.swap(out);
}

void ToggleForgeGroup(std::set<std::string> &open, const std::string &group) {
  const std::string k = ForgeOpenKey(group);
  if (!open.erase(k))
    open.insert(k);
}

std::vector<ListItem> ToItems(const std::vector<MapEntry> &list) {
  std::vector<ListItem> items;
  for (const MapEntry &m : list) {
    if (m.header) {
      items.push_back({m.name, true, true, m.enabled});
    } else {
      ListItem it{m.name, m.enabled, false, false};
      if (m.imageThumbnail.empty())
        it.thumb = m.scenario;
      else
        it.image = m.imageThumbnail;
      it.number = static_cast<int>(std::count_if(
                      items.begin(), items.end(),
                      [](const ListItem &x) { return !x.header; })) +
                  1;
      items.push_back(it);
    }
  }
  return items;
}

Screen ParseScreenArg(const std::wstring &v) {
  const std::wstring s = Lower(v);
  if (s == L"campaign")
    return Screen::Campaign;
  if (s == L"skulls")
    return Screen::Skulls;
  if (s == L"firefight")
    return Screen::Firefight;
  if (s == L"custom")
    return Screen::CustomGame;
  if (s == L"forge")
    return Screen::Forge;
  if (s == L"theater" || s == L"theatre")
    return Screen::Theater;
  if (s == L"servers")
    return Screen::Servers;
  if (s == L"settings")
    return Screen::Settings;
  if (s == L"armory")
    return Screen::Armory;
  if (s == L"mods")
    return Screen::Mods;
  if (s == L"career")
    return Screen::Career;
  if (s == L"lobby" || s == L"pregame")
    return Screen::PreGameLobby;
  if (s == L"social" || s == L"friends" || s == L"community")
    return Screen::Social;
  return Screen::Main;
}

std::wstring ToWide(const std::string &s) {
  if (s.empty())
    return L"";
  const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
  if (len <= 0)
    return L"";
  std::wstring w(len - 1, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
  return w;
}

// ---------------------------------------------------------------------------------------------
// Theater Mode: Local Saved Film (.film) BLF parser, autosave sync, and local
// video manager
// ---------------------------------------------------------------------------------------------
std::string ScenarioFromReachMapId(int32_t mapId, int mode = 0) {
  return noble::BlamEngine::ScenarioFromReachMapId(mapId, mode);
}

std::string ReadUtf16LeToUtf8(const uint8_t *bytes, size_t maxChars) {
  std::wstring w;
  w.reserve(maxChars);
  for (size_t i = 0; i < maxChars; ++i) {
    const wchar_t ch = static_cast<wchar_t>(
        bytes[i * 2] | (static_cast<uint16_t>(bytes[i * 2 + 1]) << 8));
    if (ch == 0)
      break;
    w.push_back(ch);
  }
  if (w.empty())
    return {};
  const int len =
      WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()),
                          nullptr, 0, nullptr, nullptr);
  if (len <= 0)
    return {};
  std::string out(static_cast<size_t>(len), '\0');
  WideCharToMultiByte(CP_UTF8, 0, w.c_str(), static_cast<int>(w.size()),
                      out.data(), len, nullptr, nullptr);
  return out;
}

std::string FormatDurationMmSs(uint32_t seconds) {
  const uint32_t hrs = seconds / 3600;
  const uint32_t mins = (seconds % 3600) / 60;
  const uint32_t secs = seconds % 60;
  char buf[32];
  if (hrs > 0)
    std::snprintf(buf, sizeof(buf), "%u:%02u:%02u", hrs, mins, secs);
  else
    std::snprintf(buf, sizeof(buf), "%02u:%02u", mins, secs);
  return buf;
}

std::string FormatTimestampUtcOrLocal(uint64_t unixSec) {
  if (unixSec == 0)
    return "Unknown Date";
  const std::time_t t = static_cast<std::time_t>(unixSec);
  std::tm tmLocal{};
  if (localtime_s(&tmLocal, &t) != 0)
    return "Unknown Date";
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
                tmLocal.tm_year + 1900, tmLocal.tm_mon + 1, tmLocal.tm_mday,
                tmLocal.tm_hour, tmLocal.tm_min);
  return buf;
}

std::string FormatFileSizeShort(uint64_t bytes) {
  char buf[48];
  if (bytes >= 1024ULL * 1024ULL) {
    std::snprintf(buf, sizeof(buf), "%.2f MB",
                  static_cast<double>(bytes) / (1024.0 * 1024.0));
  } else if (bytes >= 1024ULL) {
    std::snprintf(buf, sizeof(buf), "%.1f KB",
                  static_cast<double>(bytes) / 1024.0);
  } else {
    std::snprintf(buf, sizeof(buf), "%llu B",
                  static_cast<unsigned long long>(bytes));
  }
  return buf;
}

std::filesystem::path TheaterRootDir() {
  return VFS::GetUserStoragePath() / "theater";
}

void SyncAutosaveFilmsToTheater() {
  std::error_code ec;
  const std::filesystem::path theaterRoot = TheaterRootDir();
  const std::filesystem::path filmsDir = theaterRoot / "films";
  const std::filesystem::path savedDir = theaterRoot / "saved";
  const std::filesystem::path videosDir = theaterRoot / "videos";
  std::filesystem::create_directories(filmsDir, ec);
  std::filesystem::create_directories(savedDir, ec);
  std::filesystem::create_directories(videosDir, ec);

  std::vector<std::filesystem::path> scanDirs;
  const std::filesystem::path userdataRoot =
      VFS::GetUserStoragePath() / "userdata";
  if (std::filesystem::exists(userdataRoot, ec))
    scanDirs.push_back(userdataRoot);

  char *userProfile = nullptr;
  size_t upLen = 0;
  if (_dupenv_s(&userProfile, &upLen, "USERPROFILE") == 0 && userProfile) {
    const std::filesystem::path mccAutosave =
        std::filesystem::path(userProfile) / "AppData" / "LocalLow" / "MCC" /
        "Temporary" / "HaloReach" / "autosave";
    if (std::filesystem::exists(mccAutosave, ec))
      scanDirs.push_back(mccAutosave);
    std::free(userProfile);
  }
  if (scanDirs.empty())
    return;

  auto readFilmTimestamp = [](const std::filesystem::path &p) -> uint64_t {
    std::ifstream f(p, std::ios::binary);
    if (!f.is_open())
      return 0;
    uint8_t hdr[0x90] = {};
    f.read(reinterpret_cast<char *>(hdr), sizeof(hdr));
    if (f.gcount() < 0x80 || std::memcmp(hdr, "_blf", 4) != 0)
      return 0;
    uint64_t ts = 0;
    std::memcpy(&ts, hdr + 0x78, sizeof(uint64_t));
    return ts;
  };

  for (const auto &scanRoot : scanDirs) {
    for (std::filesystem::recursive_directory_iterator
             it(scanRoot,
                std::filesystem::directory_options::skip_permission_denied, ec),
         end;
         it != end && !ec; it.increment(ec)) {
      if (!it->is_regular_file(ec))
        continue;
      const std::filesystem::path src = it->path();
      std::string ext = src.extension().string();
      for (char &c : ext)
        c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
      if (ext != ".film")
        continue;

      const uint64_t srcSize = std::filesystem::file_size(src, ec);
      if (srcSize < 0x340)
        continue;
      const uint64_t srcTs = readFilmTimestamp(src);

      std::filesystem::path dst = filmsDir / src.filename();
      if (std::filesystem::exists(dst, ec)) {
        const uint64_t dstSize = std::filesystem::file_size(dst, ec);
        const uint64_t dstTs = readFilmTimestamp(dst);
        if (dstTs == srcTs && srcTs != 0) {
          // Same film session: update if the autosave grew as the match
          // finished
          if (srcSize > dstSize) {
            std::filesystem::copy_file(
                src, dst, std::filesystem::copy_options::overwrite_existing,
                ec);
          }
          continue;
        }
        if (dstSize == srcSize && dstTs == srcTs)
          continue;
        // Different match with the same gametype filename: suffix with its
        // timestamp so older films are never lost
        const std::string stem = src.stem().string();
        const uint64_t tag =
            srcTs != 0 ? srcTs : static_cast<uint64_t>(srcSize);
        dst = filmsDir / (stem + "_" + std::to_string(tag) + ".film");
        if (std::filesystem::exists(dst, ec)) {
          if (srcSize > std::filesystem::file_size(dst, ec)) {
            std::filesystem::copy_file(
                src, dst, std::filesystem::copy_options::overwrite_existing,
                ec);
          }
          continue;
        }
      }
      std::filesystem::copy_file(
          src, dst, std::filesystem::copy_options::overwrite_existing, ec);
      if (!ec) {
        NOBLE_INFO("Theater: Synced saved film '{}' -> '{}'",
                   src.filename().string(), dst.string());
      }
    }
  }
}

bool ParseSavedFilmFile(const std::filesystem::path &filePath,
                        bool isSavedFolder, TheaterEntry &out) {
  std::error_code ec;
  const uint64_t fsize = std::filesystem::file_size(filePath, ec);
  if (ec || fsize < 0x340)
    return false;

  std::ifstream in(filePath, std::ios::binary);
  if (!in.is_open())
    return false;

  uint8_t buf[0x0A40] = {};
  in.read(reinterpret_cast<char *>(buf), sizeof(buf));
  const std::streamsize got = in.gcount();
  if (got < 0x340)
    return false;
  if (std::memcmp(buf, "_blf", 4) != 0 ||
      std::memcmp(buf + 0x30, "chdr", 4) != 0)
    return false;

  out = TheaterEntry{};
  out.path = filePath;
  out.fileSize = fsize;
  out.isVideo = false;
  out.isSaved = isSavedFolder;

  const uint8_t chdrMode = buf[0x006A];
  int32_t mapId = 0;
  std::memcpy(&mapId, buf + 0x006C, sizeof(int32_t));
  std::memcpy(&out.timestamp, buf + 0x0078, sizeof(uint64_t));
  std::memcpy(&out.durationSec, buf + 0x02C0, sizeof(uint32_t));

  char authorBuf[17] = {};
  std::memcpy(authorBuf, buf + 0x0088, 16);
  out.author = authorBuf;
  if (out.author.empty()) {
    out.author = Settings::Get().Snapshot().playerName;
    if (out.author.empty())
      out.author = "Noble Six";
  }

  out.title = ReadUtf16LeToUtf8(buf + 0x00C0, 128);
  out.description = ReadUtf16LeToUtf8(buf + 0x01C0, 128);
  if (out.title.empty())
    out.title = filePath.stem().string();

  // Extract scenario stem from flmh.
  // In Reach .film files, flmh begins at offset 0x340. The scenario path string
  // is stored at flmh + 0x5EC (file offset 0x92C), typically prefixed with
  // "haloreach\maps\" or "levels\multi\". Preceding it at flmh + 0x5E8 (file
  // offset 0x928) is the 32-bit map ID.
  std::string scenarioPath;
  if (got >= 0x0930 && std::memcmp(buf + 0x0340, "flmh", 4) == 0) {
    const char *pScen = reinterpret_cast<const char *>(buf + 0x092C);
    size_t len = 0;
    while (len < 255 && pScen[len] != '\0' &&
           (static_cast<unsigned char>(pScen[len]) >= 32 &&
            static_cast<unsigned char>(pScen[len]) < 127)) {
      ++len;
    }
    if (len > 0) {
      scenarioPath.assign(pScen, len);
    } else {
      // Scan buffer within flmh for "maps\" or "levels\" if header offsets vary
      for (size_t off = 0x0340; off + 16 < static_cast<size_t>(got); ++off) {
        if (std::memcmp(buf + off, "haloreach\\maps\\", 15) == 0 ||
            std::memcmp(buf + off, "maps\\", 5) == 0 ||
            std::memcmp(buf + off, "levels\\", 7) == 0) {
          size_t slen = 0;
          while (slen < 255 && buf[off + slen] != '\0' &&
                 (static_cast<unsigned char>(buf[off + slen]) >= 32 &&
                  static_cast<unsigned char>(buf[off + slen]) < 127)) {
            ++slen;
          }
          if (slen > 0) {
            scenarioPath.assign(reinterpret_cast<const char *>(buf + off),
                                slen);
            break;
          }
        }
      }
    }
  }
  if (!scenarioPath.empty()) {
    const size_t slash = scenarioPath.find_last_of("\\/");
    out.scenario = (slash != std::string::npos) ? scenarioPath.substr(slash + 1)
                                                : scenarioPath;
  }
  if (out.scenario.size() > 4 &&
      out.scenario.substr(out.scenario.size() - 4) == ".map") {
    out.scenario.resize(out.scenario.size() - 4);
  }
  bool validScen = !out.scenario.empty();
  for (char c : out.scenario) {
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
      validScen = false;
      break;
    }
  }
  if (!validScen) {
    out.scenario = ScenarioFromReachMapId(mapId, chdrMode);
  }
  out.mapDisplay = MapNames::Display(out.scenario);

  const bool looksForge = ContainsNoCase(out.title, "Basic Editing") ||
                          ContainsNoCase(out.title, "Forge") ||
                          ContainsNoCase(out.description, "Basic Editing") ||
                          out.scenario == "forge_halo";
  if (chdrMode == 3) {
    out.launchMode = "campaign";
    out.modeLabel = "CAMPAIGN";
  } else if (chdrMode == 4) {
    out.launchMode = "firefight";
    out.modeLabel = "FIREFIGHT";
  } else if (looksForge) {
    out.launchMode = "forge";
    out.modeLabel = "FORGE";
  } else {
    out.launchMode = "multiplayer";
    out.modeLabel = "CUSTOM GAME";
  }

  if (out.description.empty()) {
    out.description = out.title + " on " + out.mapDisplay + " \xC2\xB7 " +
                      FormatTimestampUtcOrLocal(out.timestamp);
  }
  return true;
}

bool IsVideoExtension(const std::string &extLower) {
  return extLower == ".mp4" || extLower == ".mkv" || extLower == ".webm" ||
         extLower == ".avi" || extLower == ".mov" || extLower == ".wmv";
}

std::vector<TheaterEntry> ScanTheaterEntries() {
  SyncAutosaveFilmsToTheater();

  std::vector<TheaterEntry> entries;
  std::set<std::wstring> seenPaths;
  std::error_code ec;

  const std::filesystem::path root = TheaterRootDir();
  const std::filesystem::path savedDir = root / "saved";
  const std::filesystem::path filmsDir = root / "films";
  const std::filesystem::path videosDir = root / "videos";

  auto scanFolder = [&](const std::filesystem::path &folder,
                        bool isSavedFolder) {
    if (!std::filesystem::exists(folder, ec))
      return;
    for (std::filesystem::directory_iterator
             it(folder,
                std::filesystem::directory_options::skip_permission_denied, ec),
         end;
         it != end && !ec; it.increment(ec)) {
      if (!it->is_regular_file(ec))
        continue;
      const std::filesystem::path p = it->path();
      const std::filesystem::path absP =
          std::filesystem::weakly_canonical(p, ec);
      if (!seenPaths.insert(absP.empty() ? p.wstring() : absP.wstring()).second)
        continue;

      std::string ext = p.extension().string();
      for (char &c : ext)
        c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));

      if (ext == ".film") {
        TheaterEntry e;
        if (ParseSavedFilmFile(p, isSavedFolder, e)) {
          entries.push_back(std::move(e));
        }
      } else if (IsVideoExtension(ext)) {
        TheaterEntry e;
        e.path = p;
        e.title = p.stem().string();
        e.fileSize = std::filesystem::file_size(p, ec);
        e.isVideo = true;
        e.isSaved = isSavedFolder;
        e.scenario = "20_sword_slayer";
        for (const MenuMap &m : kMissions)
          if (ContainsNoCase(e.title, m.scenario) ||
              ContainsNoCase(e.title, m.name)) {
            e.scenario = m.scenario;
            break;
          }
        for (const MenuMap &m : kMaps)
          if (ContainsNoCase(e.title, m.scenario) ||
              ContainsNoCase(e.title, m.name)) {
            e.scenario = m.scenario;
            break;
          }
        for (const MenuMap &m : kFirefightMaps)
          if (ContainsNoCase(e.title, m.scenario) ||
              ContainsNoCase(e.title, m.name)) {
            e.scenario = m.scenario;
            break;
          }
        e.mapDisplay = "Local Video (" + Upper(ext.substr(1)) + ")";
        e.launchMode = "video";
        e.modeLabel = "LOCAL VIDEO";
        e.author = Settings::Get().Snapshot().playerName.empty()
                       ? "Local Recording"
                       : Settings::Get().Snapshot().playerName;
        const auto ftime = std::filesystem::last_write_time(p, ec);
        if (!ec) {
          const auto sctp =
              std::chrono::clock_cast<std::chrono::system_clock>(ftime);
          e.timestamp = static_cast<uint64_t>(
              std::chrono::duration_cast<std::chrono::seconds>(
                  sctp.time_since_epoch())
                  .count());
        }
        e.description = "Local video file in " +
                        p.parent_path().filename().string() + "\\" +
                        p.filename().string() + " (" +
                        FormatFileSizeShort(e.fileSize) + ") \xC2\xB7 " +
                        FormatTimestampUtcOrLocal(e.timestamp);
        entries.push_back(std::move(e));
      }
    }
  };

  scanFolder(savedDir, true);
  scanFolder(filmsDir, false);
  scanFolder(videosDir, false);
  scanFolder(root, false);

  std::sort(entries.begin(), entries.end(),
            [](const TheaterEntry &a, const TheaterEntry &b) {
              if (a.timestamp != b.timestamp)
                return a.timestamp > b.timestamp;
              return a.title < b.title;
            });
  return entries;
}

// Persistent state for in-engine overlay mode
// The engine draws the menu scene and calls RenderOverlay from slot 1. MCC
// ships no Reach menu map, so about a minute after start the engine gives up on
// it ("_map_state_none") and stops calling slot 1; HoldFrame then draws the
// menu from the main thread so it keeps working.
std::recursive_mutex s_overlayMutex;
std::atomic<IDXGISwapChain *> s_overlaySwapChain{nullptr};
std::atomic<ULONGLONG> s_lastEngineOverlayFrame{0};
static std::atomic<bool> s_isMatchRunning{false};
static std::atomic<bool> s_launchInFlight{false};
static std::atomic<ULONGLONG> s_lastLaunchTick{0};
static std::atomic<bool> s_holdFrameOwned{false};
static std::atomic<bool> s_resetInputEdges{false};
static std::atomic<bool> s_resetFrameTimer{false};
static std::atomic<bool> s_resetScreenTransition{false};
static std::atomic<ULONGLONG> s_postMatchInputSettleUntil{0};

bool RequestMatchLaunch(FrontEndChoice &outChoice, bool &done,
                        const FrontEndChoice &req) {
  if (FrontEndMenu::IsMatchRunning() || s_launchInFlight.load()) {
    NOBLE_WARN("FrontEndMenu: ignoring duplicate launch request while a match "
               "or launch is active");
    UiPad::ConsumeAll();
    return false;
  }
  outChoice = req;
  outChoice.play = true;
  done = true;
  UiPad::ConsumeAll();
  return true;
}

OverlayContext &GetOverlayCtx() {
  static OverlayContext s_ctx;
  return s_ctx;
}

// Rescans Workshop + data\mods and rebuilds the per-mode lists the setup
// screens use. Called at menu start and again after the Download & Join prompt
// installs the lobby's mod.
static void RefreshModCatalog(OverlayContext &ctx) {
  ctx.mods = ModCatalog::Scan(VFS::GetWorkshopPath(),
                              VFS::GetUserStoragePath() / "mods");
  ctx.campaignMods.clear();
  ctx.ffMods.clear();
  ctx.mpMods.clear();
  for (const ModEntry &m : ctx.mods) {
    auto replaces = [&](const MenuMap *base, int count) {
      return std::any_of(
          m.mapFiles.begin(), m.mapFiles.end(),
          [&](const std::string &f) { return KnownScenario(f, base, count); });
    };
    if (!m.campaign.empty() || replaces(kMissions, Count(kMissions)))
      ctx.campaignMods.push_back(&m);
    if (!m.firefight.empty() || replaces(kFirefightMaps, Count(kFirefightMaps)))
      ctx.ffMods.push_back(&m);
    if (!m.multiplayer.empty() || replaces(kMaps, Count(kMaps)))
      ctx.mpMods.push_back(&m);
  }
}

// "-auto-play": the capture harness cannot press PLAY LOCAL / HOST LOBBY, so a
// test run can ask the setup screens to start the selected match by themselves
// after a couple of seconds. Test-only, no settings involved.
bool AutoPlayRequested() {
  static const bool requested = [] {
    bool found = false;
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i < argc; ++i) {
      if (_wcsicmp(argv[i], L"-auto-play") == 0)
        found = true;
    }
    if (argv)
      LocalFree(argv);
    return found;
  }();
  return requested;
}

bool AutoPlayNow() {
  if (!AutoPlayRequested())
    return false;
  // Time, not frames: the menu can draw at 2 fps while the engine loads, which
  // made a frame count take 45 s.
  static ULONGLONG firstDraw = 0;
  if (firstDraw == 0)
    firstDraw = GetTickCount64();
  return GetTickCount64() - firstDraw > 3000;
}

// "-auto-play-map <scenario>": select that map, so a test run can walk every
// map.
std::string AutoPlayMap() {
  static const std::string scenario = [] {
    std::string out;
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i + 1 < argc; ++i) {
      if (_wcsicmp(argv[i], L"-auto-play-map") == 0) {
        for (const wchar_t *c = argv[i + 1]; c && *c; ++c)
          out.push_back(static_cast<char>(*c));
      }
    }
    if (argv)
      LocalFree(argv);
    return out;
  }();
  return scenario;
}

// "-auto-play-variant <text>": pick the first variant whose file stem contains
// <text> (e.g. "ctf", "3nvasion").
std::string AutoPlayVariantStem() {
  static const std::string stem = [] {
    std::string out;
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i + 1 < argc; ++i) {
      if (_wcsicmp(argv[i], L"-auto-play-variant") == 0) {
        for (const wchar_t *c = argv[i + 1]; c && *c; ++c)
          out.push_back(static_cast<char>(*c));
      }
    }
    if (argv)
      LocalFree(argv);
    return out;
  }();
  return stem;
}

void UpdateSceneForScreen(Screen screen) {
  switch (screen) {
  case Screen::Main:
    ReachArmory::SetTargetScene("start_menu");
    break;
  case Screen::Campaign:
  case Screen::Skulls:
    ReachArmory::SetTargetScene("campaign");
    break;
  case Screen::Firefight:
    ReachArmory::SetTargetScene("firefight");
    break;
  case Screen::CustomGame:
  case Screen::Servers:
    ReachArmory::SetTargetScene("custom_games");
    break;
  case Screen::Forge:
    ReachArmory::SetTargetScene("forge");
    break;
  case Screen::Theater:
    ReachArmory::SetTargetScene("theater");
    break;
  case Screen::Armory:
    UIManager::GetArmorCustomizer().UpdateArmoryCamera();
    break;
  default:
    // Settings, Service Record, Mods, ...: the main menu's scene, never the
    // Armory's.
    ReachArmory::SetTargetScene("start_menu");
    break;
  }
}

void FeedEngineThreadInputs(HWND hwnd, ImGuiIO &io) {
  // Test-only "-menu-click-at <seconds> <x> <y>" (repeatable): injects left clicks at screen coordinates (ImGui
  // display space, same 1920x1080 space as the capture BMPs) after the given time. The mouse never moves and the window
  // can stay unfocused, so hover/click bugs on menu buttons can be reproduced with -capture-at.
  struct TestClick {
    double at = 0.0;
    float x = 0.0f;
    float y = 0.0f;
  };
  static bool s_clickParsed = false;
  static std::vector<TestClick> s_clicks;
  static size_t s_clickIdx = 0;
  static int s_clickPhase = 0; // 0 = waiting, 1 = press, 2 = release
  if (!s_clickParsed) {
    s_clickParsed = true;
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i + 3 < argc; ++i) {
      if (_wcsicmp(argv[i], L"-menu-click-at") == 0) {
        TestClick c;
        c.at = _wtof(argv[i + 1]);
        c.x = static_cast<float>(_wtof(argv[i + 2]));
        c.y = static_cast<float>(_wtof(argv[i + 3]));
        if (c.at > 0.0)
          s_clicks.push_back(c);
      }
    }
    if (argv)
      LocalFree(argv);
  }
  if (!s_clicks.empty()) {
    if (s_clickIdx < s_clicks.size()) {
      const TestClick &c = s_clicks[s_clickIdx];
      if (s_clickPhase == 0 && ImGui::GetTime() >= c.at)
        s_clickPhase = 1;
      if (s_clickPhase == 1) {
        io.AddMousePosEvent(c.x, c.y);
        io.AddMouseButtonEvent(0, true);
        s_clickPhase = 2;
        return;
      }
      if (s_clickPhase == 2) {
        io.AddMousePosEvent(c.x, c.y);
        io.AddMouseButtonEvent(0, false);
        s_clickPhase = 0;
        ++s_clickIdx;
        return;
      }
    }
    const TestClick &last = s_clicks.back();
    io.AddMousePosEvent(last.x, last.y);
    io.AddMouseButtonEvent(0, false);
    io.AddMouseButtonEvent(1, false);
    // Test-only "-menu-wheel-at <seconds> <notches>" (repeatable): wheel over the parked mouse position, so lists that need
    // scrolling (Settings category rows) can be captured. Positive = wheel up.
    struct TestWheel { double at = 0.0; float notches = 0.0f; };
    static bool s_wheelParsed = false;
    static std::vector<TestWheel> s_wheels;
    static size_t s_wheelIdx = 0;
    if (!s_wheelParsed) {
      s_wheelParsed = true;
      int wargc = 0;
      LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
      for (int i = 1; wargv && i + 2 < wargc; ++i) {
        if (_wcsicmp(wargv[i], L"-menu-wheel-at") == 0) {
          TestWheel w;
          w.at = _wtof(wargv[i + 1]);
          w.notches = static_cast<float>(_wtof(wargv[i + 2]));
          if (w.at > 0.0) s_wheels.push_back(w);
        }
      }
      if (wargv) LocalFree(wargv);
    }
    if (s_wheelIdx < s_wheels.size() && ImGui::GetTime() >= s_wheels[s_wheelIdx].at) {
      io.AddMouseWheelEvent(0.0f, s_wheels[s_wheelIdx].notches);
      ++s_wheelIdx;
    }
    return;
  }

  // Test-only: "-menu-ignore-input" freezes the menu for screenshots. The
  // window still renders, but the mouse/keyboard cannot move the selection or
  // cycle the Armory camera while a capture is taken.
  static const bool s_ignoreInput =
      wcsstr(GetCommandLineW(), L"-menu-ignore-input") != nullptr;
  if (s_ignoreInput) {
    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    io.AddMouseButtonEvent(0, false);
    io.AddMouseButtonEvent(1, false);
    return;
  }
  RECT client{};
  GetClientRect(hwnd, &client);
  POINT cursor{};
  const bool focused = Window::IsForeground(hwnd);
  const bool resetEdges = s_resetInputEdges.exchange(false);
  const ULONGLONG settleUntil = s_postMatchInputSettleUntil.load();
  const bool settling = settleUntil != 0 && GetTickCount64() < settleUntil;
  if (!settling && settleUntil != 0) {
    s_postMatchInputSettleUntil.store(0);
  }
  static bool s_waitMouseRelease = false;
  if (resetEdges) {
    (void)Window::TakeLeftClicks();
    (void)Window::TakeWheelSteps();
    (void)Window::TakeTypedChars();
    s_waitMouseRelease = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
  }

  const bool vcOwnsPointer = UiPad::IsVirtualCursorControllingMouse();
  if (vcOwnsPointer) {
    if (resetEdges || settling || !focused) {
      (void)Window::TakeLeftClicks();
      (void)Window::TakeWheelSteps();
    } else {
      const float wheel = Window::TakeWheelSteps();
      if (wheel != 0.0f) {
        io.AddMouseWheelEvent(0.0f, wheel);
      }
    }
  } else if (!settling && focused && GetCursorPos(&cursor) &&
             ScreenToClient(hwnd, &cursor) && client.right > 0 &&
             client.bottom > 0) {
    const float sx = io.DisplaySize.x / static_cast<float>(client.right);
    const float sy = io.DisplaySize.y / static_cast<float>(client.bottom);
    io.AddMousePosEvent(static_cast<float>(cursor.x) * sx,
                        static_cast<float>(cursor.y) * sy);

    const bool lDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    if (!lDown)
      s_waitMouseRelease = false;
    if (resetEdges || s_waitMouseRelease) {
      (void)Window::TakeLeftClicks();
      (void)Window::TakeWheelSteps();
      io.AddMouseButtonEvent(0, false);
      io.AddMouseButtonEvent(1, false);
    } else {
      if (Window::TakeLeftClicks() > 0 && !lDown) {
        io.AddMouseButtonEvent(0, true);
        io.AddMouseButtonEvent(0, false);
      } else {
        io.AddMouseButtonEvent(0, lDown);
      }
      const bool rDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
      io.AddMouseButtonEvent(1, rDown);

      const float wheel = Window::TakeWheelSteps();
      if (wheel != 0.0f) {
        io.AddMouseWheelEvent(0.0f, wheel);
      }
    }
  } else {
    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    io.AddMouseButtonEvent(0, false);
    io.AddMouseButtonEvent(1, false);
    if (resetEdges || settling) {
      (void)Window::TakeLeftClicks();
      (void)Window::TakeWheelSteps();
    }
  }

  struct KeyMap {
    int vk;
    ImGuiKey key;
    char ch;
  };
  static const KeyMap kKeys[] = {
      {VK_UP, ImGuiKey_UpArrow, 0},     {VK_DOWN, ImGuiKey_DownArrow, 0},
      {VK_LEFT, ImGuiKey_LeftArrow, 0}, {VK_RIGHT, ImGuiKey_RightArrow, 0},
      {VK_RETURN, ImGuiKey_Enter, 0},   {VK_ESCAPE, ImGuiKey_Escape, 0},
      {VK_TAB, ImGuiKey_Tab, 0},        {VK_SPACE, ImGuiKey_Space, ' '},
      {VK_BACK, ImGuiKey_Backspace, 0}, {VK_DELETE, ImGuiKey_Delete, 0},
      {'Q', ImGuiKey_Q, 'Q'},           {'E', ImGuiKey_E, 'E'},
      {'A', ImGuiKey_A, 'A'},           {'B', ImGuiKey_B, 'B'},
      {'C', ImGuiKey_C, 'C'},           {'D', ImGuiKey_D, 'D'},
      {'F', ImGuiKey_F, 'F'},           {'G', ImGuiKey_G, 'G'},
      {'H', ImGuiKey_H, 'H'},           {'I', ImGuiKey_I, 'I'},
      {'J', ImGuiKey_J, 'J'},           {'K', ImGuiKey_K, 'K'},
      {'L', ImGuiKey_L, 'L'},           {'M', ImGuiKey_M, 'M'},
      {'N', ImGuiKey_N, 'N'},           {'O', ImGuiKey_O, 'O'},
      {'P', ImGuiKey_P, 'P'},           {'R', ImGuiKey_R, 'R'},
      {'S', ImGuiKey_S, 'S'},           {'T', ImGuiKey_T, 'T'},
      {'U', ImGuiKey_U, 'U'},           {'V', ImGuiKey_V, 'V'},
      {'W', ImGuiKey_W, 'W'},           {'X', ImGuiKey_X, 'X'},
      {'Y', ImGuiKey_Y, 'Y'},           {'Z', ImGuiKey_Z, 'Z'},
      {'0', ImGuiKey_0, '0'},           {'1', ImGuiKey_1, '1'},
      {'2', ImGuiKey_2, '2'},           {'3', ImGuiKey_3, '3'},
      {'4', ImGuiKey_4, '4'},           {'5', ImGuiKey_5, '5'},
      {'6', ImGuiKey_6, '6'},           {'7', ImGuiKey_7, '7'},
      {'8', ImGuiKey_8, '8'},           {'9', ImGuiKey_9, '9'},
  };
  static bool s_prevDown[sizeof(kKeys) / sizeof(kKeys[0])] = {};
  for (size_t i = 0; i < sizeof(kKeys) / sizeof(kKeys[0]); ++i) {
    const bool down =
        focused && ((GetAsyncKeyState(kKeys[i].vk) & 0x8000) != 0);
    if (resetEdges) {
      if (s_prevDown[i]) {
        io.AddKeyEvent(kKeys[i].key, false);
      }
      // Adopt currently held keys as already-down so releasing Match Exit keys
      // cannot fire menu edges.
      s_prevDown[i] = down;
    } else if (down != s_prevDown[i]) {
      if (settling && down) {
        s_prevDown[i] = true;
      } else {
        io.AddKeyEvent(kKeys[i].key, down);
        s_prevDown[i] = down;
      }
    }
  }
}

// Lobby events from the WebSocket relay thread. The callbacks used to write
// MenuState's strings and roster on that thread while the menu read them on its
// own; two launch events 0.1 s apart (2026-10-29, joining O7OF) then killed the
// menu without a crash log (heap corruption ends the process before the crash
// handler runs). The relay thread now only fills this, and the menu takes it
// over at the top of the lobby screen.

void DrawTopTelemetryBar(const Ui &ui, const char *centerTag) {
  const float minX = ui.origin.x + ui.size.x * 0.34f;
  const float maxX = ui.origin.x + ui.size.x * 0.65f;
  if (maxX - minX < 210.0f * ui.s)
    return;
  const float cy = ui.origin.y + 30.0f * ui.s;
  const float cx = (minX + maxX) * 0.5f;
  const float halfCenter = std::clamp(
      TextWidth(NobleStyle::SmallFont(), 13.0f * ui.s, centerTag) * 0.5f +
          26.0f * ui.s,
      48.0f * ui.s, 110.0f * ui.s);
  const float wingH = 10.0f * ui.s;
  const float skew = 6.0f * ui.s;

  // Fine top hairline rule
  ui.dl->AddLine(ImVec2(minX, cy - wingH - 4.0f * ui.s),
                 ImVec2(maxX, cy - wingH - 4.0f * ui.s),
                 IM_COL32(143, 216, 255, 55), 1.0f);

  // Left slanted tactical segments
  const float segW = 8.0f * ui.s, segGap = 4.0f * ui.s;
  int idx = 0;
  for (float x = cx - halfCenter - 10.0f * ui.s - segW;
       x >= minX + 6.0f * ui.s && idx < 10; x -= (segW + segGap), ++idx) {
    const ImU32 col =
        idx < 4 ? IM_COL32(95, 165, 210, 145) : IM_COL32(75, 115, 145, 70);
    ui.dl->AddQuadFilled(ImVec2(x + skew, cy - wingH * 0.5f),
                         ImVec2(x + segW + skew, cy - wingH * 0.5f),
                         ImVec2(x + segW, cy + wingH * 0.5f),
                         ImVec2(x, cy + wingH * 0.5f), col);
  }
  // Right slanted tactical segments
  idx = 0;
  for (float x = cx + halfCenter + 10.0f * ui.s;
       x + segW + skew <= maxX - 6.0f * ui.s && idx < 10;
       x += (segW + segGap), ++idx) {
    const ImU32 col =
        idx < 4 ? IM_COL32(95, 165, 210, 145) : IM_COL32(75, 115, 145, 70);
    ui.dl->AddQuadFilled(ImVec2(x, cy - wingH * 0.5f),
                         ImVec2(x + segW, cy - wingH * 0.5f),
                         ImVec2(x + segW + skew, cy + wingH * 0.5f),
                         ImVec2(x + skew, cy + wingH * 0.5f), col);
  }
  // Center tag trapezoid pill
  const float h = 18.0f * ui.s;
  const ImVec2 p0(cx - halfCenter, cy - h * 0.5f);
  const ImVec2 p1(cx + halfCenter, cy - h * 0.5f);
  const ImVec2 p2(cx + halfCenter - 6.0f * ui.s, cy + h * 0.5f);
  const ImVec2 p3(cx - halfCenter + 6.0f * ui.s, cy + h * 0.5f);
  ui.dl->AddQuadFilled(p0, p1, p2, p3, IM_COL32(18, 30, 44, 225));
  ui.dl->AddQuad(p0, p1, p2, p3, IM_COL32(143, 216, 255, 130), 1.0f);
  const float tw = TextWidth(NobleStyle::SmallFont(), 12.5f * ui.s, centerTag);
  Text(ui, NobleStyle::SmallFont(), 12.5f,
       ImVec2(cx - tw * 0.5f, cy - 6.5f * ui.s), IM_COL32(215, 232, 246, 235),
       centerTag);
}

// ---- Container window mode: the match is a child process presenting inside
// this launcher window ----
// Starts a match process inside this launcher window and remembers it. The
// launcher is the only process that starts matches in container mode (a match
// cannot replace itself: see Relaunch::HandOff).
bool StartContainerChild(const std::wstring &args, OverlayContext &ctx) {
  std::lock_guard<std::recursive_mutex> lock(s_overlayMutex);
  bool expected = false;
  if (!s_launchInFlight.compare_exchange_strong(expected, true)) {
    NOBLE_WARN("FrontEndMenu: StartContainerChild ignored because a launch is "
               "already in flight");
    return false;
  }
  struct LaunchGuard {
    std::atomic<bool> &flag;
    ~LaunchGuard() { flag.store(false); }
  } guard{s_launchInFlight};

  const ULONGLONG now = GetTickCount64();
  const ULONGLONG prev = s_lastLaunchTick.load();
  if (prev != 0 && now - prev < 1500 && ctx.matchRunning) {
    NOBLE_WARN("FrontEndMenu: StartContainerChild throttled within 1500ms of "
               "previous launch");
    return false;
  }
  if (ctx.matchRunning && ctx.hMatchProcess) {
    if (WaitForSingleObject(reinterpret_cast<HANDLE>(ctx.hMatchProcess), 0) ==
        WAIT_TIMEOUT) {
      NOBLE_WARN("FrontEndMenu: StartContainerChild ignored because a match "
                 "process is already running");
      return false;
    }
  }
  std::wstring full = args;
  if (full.find(L"-container-hwnd") == std::wstring::npos) {
    full += L" -container-hwnd " +
            std::to_wstring(reinterpret_cast<uintptr_t>(Window::ActiveHwnd()));
  }
  // The match process must use the same machine id the lobby roster published for this player. Otherwise
  // its wire id is random and every machine-id-keyed feature (ping rows, per-peer mute, name lookup) misses.
  {
    const uint64_t lobbyMachine = MasterServerClient::GetLobbyMachineId();
    if (lobbyMachine != 0) {
      wchar_t idText[32] = {};
      _snwprintf_s(idText, _countof(idText), _TRUNCATE, L"0x%016llX",
                   static_cast<unsigned long long>(lobbyMachine));
      SetEnvironmentVariableW(L"NOBLE_MACHINE_ID", idText);
    }
  }
  HANDLE hChild = nullptr;
  if (!Relaunch::StartChildMatch(full, reinterpret_cast<void **>(&hChild))) {
    NOBLE_ERROR("FrontEndMenu: failed to start the match process in the "
                "container window");
    return false;
  }
  s_lastLaunchTick.store(GetTickCount64());
  ctx.hMatchProcess = hChild;
  ctx.matchRunning = true;
  FrontEndMenu::SetMatchRunning(true);
  Window::FlushInputEvents();
  UiPad::ConsumeAll();
  s_resetInputEdges.store(true);
  s_resetFrameTimer.store(true);
  return true;
}

// The child match ended. PLAY AGAIN / NEXT MISSION leave a relaunch request
// behind (see Relaunch::RequestContainerRelaunch): start that successor and
// stay in the match. Otherwise the menu comes back, and it takes the keyboard
// focus back from the match window that has just closed. What a match process's
// exit code means when it did not leave normally. 0 is a normal exit; the rest
// are the ways Noble or Windows ends a process that went wrong.
static const char *CrashExitMeaning(DWORD code) {
  switch (code) {
  case 0xC0000005u:
    return "access violation";
  case 0xC0000409u:
    return "fail-fast (abort / unhandled C++ exception / stack check)";
  case 0xC00000FDu:
    return "stack overflow";
  case 0xC0000374u:
    return "heap corruption";
  case 0xD3D10001u:
    return "GPU device lost (Noble ended the match)";
  case 3u:
    return "Noble's terminate handler ended it (see the match's own log)";
  default:
    return nullptr;
  }
}

void ContainerMatchEnded(OverlayContext &ctx, DWORD exitCode) {
  if (ctx.hMatchProcess)
    CloseHandle(reinterpret_cast<HANDLE>(ctx.hMatchProcess));
  ctx.hMatchProcess = nullptr;

  ctx.st.lobbyLaunchPending = false;
  ClearPendingLobbyEvents();

  const std::wstring successor = Relaunch::TakeContainerRelaunch();
  if (successor == L"-quit-app") {
    ctx.closed = true;
    ctx.matchRunning = false;
    FrontEndMenu::SetMatchRunning(false);
    if (!ctx.st.lobbySessionId.empty()) {
      if (ctx.st.isLobbyHost) {
        MasterServerClient::Get().StopAdvertisingSync();
        SessionManager::Get().StopHosting();
      } else {
        uint64_t fallbackId =
            (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL);
        if (fallbackId < 2ULL)
          fallbackId = 2ULL;
        uint64_t pId =
            AuthClient::Get().IsLoggedIn()
                ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
                : fallbackId;
        MasterServerClient::Get().LeaveLobbySync(ctx.st.lobbySessionId, pId);
      }
      NetworkTransport::Get().StopRelay();
      ctx.st.lobbySessionId.clear();
      ctx.st.lobbySessionToken.clear();
      ctx.st.lobbyHostEndpoint.clear();
      ctx.st.lobbyRoster.clear();
    }
    NOBLE_INFO("FrontEndMenu: match requested application quit; closing "
               "container window");
    if (HWND hwnd = Window::ActiveHwnd()) {
      PostMessageW(hwnd, WM_CLOSE, 0, 0);
    }
    return;
  }

  const bool isLobbyReturn =
      successor.find(L"-lobby-return") != std::wstring::npos;
  const bool isLeaveLobby = successor.empty() || successor == L"-leave-lobby" ||
                            successor == L"-main-menu";
  if (!isLobbyReturn && !isLeaveLobby) {
    if (StartContainerChild(successor, ctx)) {
      NOBLE_INFO("FrontEndMenu: container match relaunched (the previous match "
                 "exited with {})",
                 exitCode);
      return;
    }
    NOBLE_WARN("FrontEndMenu: container relaunch could not be started; "
               "restoring the menu");
  }

  std::lock_guard<std::recursive_mutex> lock(s_overlayMutex);
  // Keep IsMatchRunning() true until lobby teardown, audio/scene sync, cursor
  // restoration, and input flushes finish so Engine Thread (RenderOverlay) and
  // Main Thread (HoldFrame) cannot race across the transition or flash stale
  // frames.
  ctx.st.confirmQuit = false;
  ctx.st.lobbyLaunchPending = false;
  ctx.st.playlistEditOpen = false;
  ctx.st.confirmAction.clear();
  ctx.st.optionsOpen = false;
  ctx.st.optionsRuleDropdownOpen = false;
  ctx.st.variantPickerOpen = false;
  ctx.st.lobbyPickerOpen = false;
  ctx.st.lobbyTeamPickerOpen = false;
  ctx.st.lobbyProfileOpen = false;
  ctx.st.theaterConfirmDelete.clear();
  ctx.st.lobbyMapPickerOpen = false;

  if (isLobbyReturn) {
    ctx.st.screen = Screen::PreGameLobby;
    ctx.st.isLobbyHost =
        (successor.find(L"-lobby-return host") != std::wstring::npos);
    ctx.st.lastLobbyPoll = 0.0;
    NOBLE_INFO("FrontEndMenu: container returned to lobby as {}",
               ctx.st.isLobbyHost ? "host" : "guest");
  } else if (ctx.launchedFromTheater) {
    ctx.launchedFromTheater = false;
    ctx.st.screen = Screen::Theater;
    NOBLE_INFO("FrontEndMenu: container returned to Theater lobby");
  } else {
    ctx.st.screen = Screen::Main;
    NOBLE_INFO("FrontEndMenu: container returned to main menu");
  }
  if (const char *crash = CrashExitMeaning(exitCode)) {
    NOBLE_ERROR("FrontEndMenu: the match process CRASHED, exit code 0x{:08X}: "
                "{}. Check noble-dev.log / noble-client.log "
                "and noble-*crash.dmp beside the exe, and Windows Event Viewer "
                "(Application).",
                exitCode, crash);
  } else if ((exitCode & 0xC0000000u) == 0xC0000000u) {
    NOBLE_ERROR("FrontEndMenu: the match process ended with the Windows error "
                "status 0x{:08X}",
                exitCode);
  }
  NOBLE_INFO("FrontEndMenu: container child match finished (exit code {}), "
             "restoring menu",
             exitCode);
  FrontEndMenu::InvalidateForgeMaps(); // a variant saved in Forge must show up
                                       // without restarting Noble
  FrontEndMenu::SyncTheaterFiles();    // sync any newly recorded .film from the
                                       // match into data\theater\films
  SessionManager::Get().Initialize();
  MasterServerClient::Get().SetUrl(Settings::Get().Snapshot().masterServerUrl);

  if (isLobbyReturn) {
    if (ctx.st.isLobbyHost) {
      const ModEntry *mod = nullptr;
      for (const ModEntry &m : ctx.mods) {
        if (!ctx.st.lobbyModFolder.empty() &&
            m.folder == ctx.st.lobbyModFolder) {
          mod = &m;
          break;
        }
      }
      SetupHostLobby(
          ctx.st, ctx.st.lobbyMap, ctx.st.lobbyMode, ctx.st.lobbyVariantPath,
          ctx.st.lobbyVariantName, ctx.st.lobbyDifficulty, ctx.st.lobbySkulls,
          ctx.st.lobbyMaxPlayers, mod, ctx.st.lobbyMapOverride,
          ctx.st.lobbyMapTitle, ctx.st.lobbyMapImage, ctx.st.lobbyMapThumb,
          ctx.st.lobbySessionId, ctx.st.lobbySessionToken);
      if (successor.find(L"-lobby-advance 1") != std::wstring::npos &&
          ctx.st.playlistChoice >= 0 &&
          ctx.st.playlistChoice < static_cast<int>(ctx.st.playlists.size())) {
        const Playlists::Playlist &playlist =
            ctx.st.playlists[ctx.st.playlistChoice];
        if (!playlist.entries.empty()) {
          const int n = static_cast<int>(playlist.entries.size());
          int at = ctx.st.playlistPosition;
          if (playlist.shuffle && n > 1) {
            const int was = at;
            while (at == was)
              at = static_cast<int>(std::random_device{}() % n);
          } else {
            at = (at + 1) % n;
          }
          ResolvedEntry resolved;
          if (ResolveEntry(ctx, playlist.entries[at], playlist.randomType,
                           resolved)) {
            ApplyEntryToLobby(ctx.st, resolved);
          }
          ctx.st.playlistPosition = at;
        }
        if (playlist.autoStart > 0) {
          ctx.st.autoStartAt = ImGui::GetTime() + playlist.autoStart + 2.0;
        }
      }
    } else {
      ctx.st.lobbyLaunchPending = false;
      ctx.st.screen = Screen::PreGameLobby;
      // Guest returning to lobby: restart relay session with the host if
      // session ID is present
      if (!ctx.st.lobbySessionId.empty()) {
        std::string hostEndpoint = ctx.st.lobbyHostEndpoint;
        uint16_t hostPort = 11774;
        const size_t colon = hostEndpoint.rfind(':');
        if (colon != std::string::npos) {
          hostPort = static_cast<uint16_t>(
              std::atoi(hostEndpoint.c_str() + colon + 1));
          hostEndpoint = hostEndpoint.substr(0, colon);
        }
        const auto jinfo = MasterServerClient::Get().JoinSessionSync(
            ctx.st.lobbySessionId, MasterServerClient::GetLobbyMachineId(),
            11774, hostEndpoint, hostPort);
        if (jinfo.ok && !jinfo.relayUrl.empty()) {
          NetworkTransport::Get().StartRelay(
              jinfo.relayUrl, ctx.st.lobbySessionId, false,
              MasterServerClient::GetLobbyMachineId());
        }
      }
    }
  } else if (successor == L"-main-menu" ||
             ctx.st.screen == Screen::PreGameLobby ||
             !ctx.st.lobbySessionId.empty() ||
             !ctx.st.lobbyHostEndpoint.empty()) {
    // Player left the match or lobby disbanded: return to main menu or
    // appropriate mode screen.
    if (ctx.st.isLobbyHost) {
      MasterServerClient::Get().StopAdvertisingSync();
      SessionManager::Get().StopHosting();
      NetworkTransport::Get().StopRelay();
      ctx.st.lobbySessionId.clear();
      ctx.st.lobbySessionToken.clear();
      ctx.st.lobbyRoster.clear();
      ctx.st.screen = (successor == L"-main-menu" || isLeaveLobby)
                          ? Screen::Main
                      : (ctx.st.lobbyMode == "campaign")  ? Screen::Campaign
                      : (ctx.st.lobbyMode == "firefight") ? Screen::Firefight
                      : (ctx.st.lobbyMode == "forge")     ? Screen::Forge
                                                          : Screen::CustomGame;
    } else {
      uint64_t fallbackId =
          (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL);
      if (fallbackId < 2ULL)
        fallbackId = 2ULL;
      uint64_t pId = AuthClient::Get().IsLoggedIn()
                         ? static_cast<uint64_t>(AuthClient::Get().GetUserId())
                         : fallbackId;
      if (!ctx.st.lobbySessionId.empty()) {
        MasterServerClient::Get().LeaveLobbySync(ctx.st.lobbySessionId, pId);
      }
      NetworkTransport::Get().StopRelay();
      ctx.st.lobbySessionId.clear();
      ctx.st.lobbySessionToken.clear();
      ctx.st.lobbyHostEndpoint.clear();
      ctx.st.lobbyRoster.clear();
      ctx.st.screen = Screen::Main;
    }
    ctx.st.focusRow = 0;
  }

  MasterServerClient::Get().RequestOnlineCount();
  ReachAudio::SetMenuMusicEnabled(true);
  ReachAudio::SetMusicVolume(Settings::Get().Snapshot().volumeMusic);
  ProcessAudio::ApplyAll(Settings::Get().Snapshot().volumeGame,
                         Settings::Get().Snapshot().volumeMusic,
                         Settings::Get().Snapshot().volumeOther,
                         Settings::Get().Snapshot().masterMute);
  UpdateSceneForScreen(ctx.st.screen);

  // Restore launcher cursor, discard stale clicks/wheel/keys accumulated during
  // or while leaving the child match, reset edge detectors, and settle briefly.
  Window::FlushInputEvents();
  UiPad::ResetAfterMatch();
  if (ImGui::GetCurrentContext()) {
    ImGuiIO &io = ImGui::GetIO();
    io.ClearInputKeys();
    io.ClearInputMouse();
    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    ImGui::SetWindowFocus(nullptr);
  }
  ctx.st.focusRow = 0;
  ctx.st.lobbyFocusZone = 0;
  s_resetInputEdges.store(true);
  s_resetFrameTimer.store(true);
  s_resetScreenTransition.store(true);
  s_postMatchInputSettleUntil.store(GetTickCount64() + 300);
  if (!s_holdFrameOwned.load()) {
    // Give HostSlot1_FrameTick a fresh grace period to resume presenting before
    // HoldFrame considers taking over (prevents dual-thread clear/3D flicker).
    s_lastEngineOverlayFrame.store(GetTickCount64());
  }

  ctx.closed = false;
  ctx.matchRunning = false;
  FrontEndMenu::SetMatchRunning(false);
  // The match window held the keyboard focus; hand it back to the launcher
  // window (the window's own thread does the SetFocus: this code runs on the
  // engine thread or main loop).
  Window::RequestFocus();
}

void DrawFrontEndContents(OverlayContext &ctx, ID3D11Device *device,
                          ID3D11DeviceContext *context, FrontEndChoice &choice,
                          bool &done) {
  if (ctx.matchRunning && ctx.hMatchProcess) {
    DWORD exitCode = 0;
    if (GetExitCodeProcess(reinterpret_cast<HANDLE>(ctx.hMatchProcess),
                           &exitCode) &&
        exitCode != STILL_ACTIVE) {
      ContainerMatchEnded(ctx, exitCode);
    }
    if (FrontEndMenu::IsMatchRunning()) {
      // The child match is presenting in the child window inside this
      // container.
      return;
    }
  }

  ImGuiViewport *vp = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(vp->Pos);
  ImGui::SetNextWindowSize(vp->Size);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoScrollWithMouse;
  // These screens read the pad and keyboard themselves. With ImGui's own
  // navigation on as well, its invisible cursor sat on some other button and
  // one A press could act twice (seen on the lobby's COPY INVITE, 2026-10-31).
  // The panel screens below keep it: their lists and buttons are plain ImGui
  // widgets.
  if (ctx.st.screen != Screen::Servers && ctx.st.screen != Screen::Mods) {
    flags |= ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus;
  }
  ImGui::Begin("##NobleFrontEnd", nullptr, flags);
  ImGui::PopStyleVar(2);

  MenuState &st = ctx.st;
  static ULONGLONG s_screenEnterTick = 0;
  {
    static int s_lastScreen = -1;
    if (s_resetScreenTransition.exchange(false)) {
      s_lastScreen = -1;
    }
    if (static_cast<int>(st.screen) != s_lastScreen) {
      NOBLE_INFO("FrontEndMenu: screen {} -> {}", s_lastScreen,
                 static_cast<int>(st.screen));
      if (s_lastScreen != -1) {
        st.confirmQuit = false;
        st.playlistEditOpen = false;
        st.optionsOpen = false;
        st.optionsRuleDropdownOpen = false;
        st.variantPickerOpen = false;
        st.lobbyPickerOpen = false;
        st.lobbyTeamPickerOpen = false;
        st.lobbyProfileOpen = false;
        st.lobbyMapPickerOpen = false;
      }
      s_lastScreen = static_cast<int>(st.screen);
      s_screenEnterTick = GetTickCount64();
      g_pickerOpenNow = false;
      UiPad::ConsumeAll();
      ImGui::SetWindowFocus(nullptr);
      // Variants saved in a match (or another menu) must be visible the moment
      // the maps screen is entered.
      FrontEndMenu::InvalidateForgeMaps();
    }
  }
  Ui ui;
  ui.dl = ImGui::GetWindowDrawList();
  ui.origin = ImVec2(0.0f, 0.0f);
  ui.size = vp->Size;
  // 4K back buffer = 2.4 (was capped at 2.0), times the player's UI SCALE
  // setting (75 - 150 %).
  ui.s = std::clamp(vp->Size.y / 900.0f, 0.75f, 4.0f) * Settings::UiScale();
  // Scrollbars follow the UI scale (a fixed pixel width was a hairline on the
  // 4K/supersampled menu).
  ImGui::GetStyle().ScrollbarSize = std::round(14.0f * ui.s);
  ImGui::GetStyle().GrabMinSize = std::round(24.0f * ui.s);

  s_lobbyBackdropMap = st.lobbyMap;
  s_lobbyBackdropImage =
      st.lobbyMapImage.empty() ? st.lobbyMapThumb : st.lobbyMapImage;
  DrawBackdrop(ui, st.screen);
  DrawAccountModal(ui);
  DrawDevMenuModal(ui);

  UpdateSceneForScreen(st.screen);

  const float left = ui.size.x * 0.075f;
  const float top = ui.size.y * 0.09f;
  const SettingsData settings = Settings::Get().Snapshot();
  s_hintThisFrame = std::exchange(s_hintClicked, -1);
  s_hintExtraThisFrame = std::exchange(s_hintExtraClicked, std::string());
  const bool modPromptOpen = ModDownloadPrompt::Get().IsActive();
  // A choice list that was open last frame owns Back and the up/down keys this
  // frame.
  g_pickerBlocksNav = std::exchange(g_pickerOpenNow, false);
  // Esc in a text box only leaves the box.
  const bool back = !modPromptOpen && !g_pickerBlocksNav &&
                    !g_showAccountModal && !g_showDevMenu &&
                    !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId) &&
                    !ImGui::GetIO().WantTextInput &&
                    (UiPad::BackPressed() || HintClicked(Prompt::Back));
  auto goBack = [&](Screen to) {
    UiPad::ConsumeBack();
    st.screen = to;
    st.focusRow = 0;
    st.lobbyFocusZone = 0;
    st.mapSearch[0] = 0;
    MenuSounds::Back();
  };
  auto saveSkulls = [&] {
    SettingsData d = Settings::Get().Snapshot();
    d.campaignSkulls = st.skulls;
    Settings::Get().Update(d);
  };

  switch (st.screen) {
  case Screen::Main: {
    // While the quit box or a modal is open nothing behind it reacts to the
    // mouse (no hover, sounds or clicks).
    static bool s_showChangelogManual = false;
    const bool firstStart = DrawFirstStart(ui);
    if (firstStart)
      ImGui::GetIO().ClearInputKeys();
    const bool showChangelog =
        !st.confirmQuit && !g_showAccountModal && !g_showDevMenu &&
        !modPromptOpen &&
        ShouldShowChangelogModal(s_showChangelogManual, firstStart);
    const bool modal = firstStart || showChangelog || st.confirmQuit ||
                       g_showAccountModal || g_showDevMenu || modPromptOpen;
    ImGui::BeginDisabled(modal);
    DrawParty(ui, settings.playerName, "In Main Menu Lobby");
    if (DrawUpdateBanner(ui))
      done = true; // update script takes over after this frame
    const float titleY = ui.size.y * 0.16f;
    DrawTitle(ui, ImVec2(left, titleY), "MAIN MENU");
    Text(ui, nullptr, 17.0f, ImVec2(left + 4.0f * ui.s, titleY + 66.0f * ui.s),
         kDim, "This party is local to this PC.");
    // The list is built per frame so the developer row can sit above Quit
    // without shifting indices.
    std::vector<const MainEntry *> rows;
    const SettingsData mainSnap = Settings::Get().Snapshot();
    const bool canContinue = mainSnap.lastPlayedMode == "campaign" &&
                             !mainSnap.lastPlayedMap.empty();
    if (canContinue)
      rows.push_back(&kContinueEntry);
    for (const MainEntry &e : kMainEntries) {
      if (e.quit && AuthClient::Get().IsDeveloper())
        rows.push_back(&kDeveloperEntry);
      rows.push_back(&e);
    }
    if (rows.empty())
      rows.push_back(&kMainEntries[0]);
    st.mainIndex =
        std::clamp(st.mainIndex, 0, static_cast<int>(rows.size()) - 1);
    std::vector<ListItem> items;
    for (const MainEntry *e : rows)
      items.push_back({e->label, e->enabled});

    // Fit the list, its description and the hint row: shrink the row spacing
    // first, then lift the list, so the description can never be drawn on top
    // of "ENTER Select / ESC Quit" (2026-10-30).
    const float descWidth = 430.0f * ui.s;
    const char *desc = rows[st.mainIndex]->description;
    // Reserve two lines whatever the selected entry says. The layout used to be
    // measured from the *selected* description, so a one-line entry and a
    // two-line entry put the list at different heights and moving the cursor
    // made the whole menu jump (reported 2026-10-30).
    const float descH =
        Font(nullptr)->CalcTextSizeA(17.0f * ui.s, FLT_MAX, 0.0f, "Ag").y *
            2.0f +
        2.0f * ui.s;
    const float descGap = 10.0f * ui.s;
    const float hintTop = ui.size.y - 54.0f * ui.s;
    float listY = titleY + 96.0f * ui.s; // below the "This party is local" line
                                         // at +66 (+76 overlapped it)
    float step = 26.0f;                  // ui.s units
    const float rowCount = static_cast<float>(rows.size());
    auto blockHeight = [&](float s) {
      return rowCount * s * ui.s + descGap + descH + 8.0f * ui.s;
    };
    if (blockHeight(step) > hintTop - listY) {
      step = std::max(18.0f, (hintTop - listY - descGap - descH - 8.0f * ui.s) /
                                 (rowCount * ui.s));
      if (blockHeight(step) > hintTop - listY) {
        listY = std::max(ui.size.y * 0.14f, hintTop - blockHeight(step));
      }
    }
    const int hit = DrawList(ui, "main", ImVec2(left, listY), 440.0f * ui.s,
                             items, st.mainIndex, !modal, step);
    ImGui::EndDisabled();
    const float descY = listY + rowCount * step * ui.s + descGap;
    std::string descLine = desc;
    if (rows[st.mainIndex] == &kContinueEntry) {
      std::string mapName = mainSnap.lastPlayedMap;
      for (const MenuMap &m : kMissions)
        if (m.scenario == mapName) {
          mapName = m.name;
          break;
        }
      const char *difficulty =
          kDifficulties[std::clamp(mainSnap.lastPlayedDifficulty, 0, 3)];
      // Honest about what it does: Noble has no save/checkpoint resume, so the
      // mission starts again.
      descLine = "Continue the campaign: " + mapName + " on " + difficulty +
                 " (starts the mission again).";
    }
    Text(ui, nullptr, 17.0f, ImVec2(left + 4.0f * ui.s, descY), kDim,
         descLine.c_str(), left + descWidth);
    if (!modal) {
      if (hit >= 0) {
        const MainEntry *picked = rows[hit];
        if (picked->quit) {
          st.confirmQuit = true;
          st.quitChoice = 1;
        } else if (picked == &kContinueEntry) {
          choice.play = true;
          choice.host = false;
          choice.map = mainSnap.lastPlayedMap;
          choice.mode = "campaign"; // CONTINUE is campaign-only by design
          choice.variantPath = mainSnap.lastPlayedVariant;
          choice.difficulty = mainSnap.lastPlayedDifficulty;
          choice.skulls = mainSnap.lastPlayedSkulls;
          // The mod may have been removed since; a stale folder would make the
          // new process fail to launch, so fall back to the vanilla game
          // instead.
          if (!mainSnap.lastPlayedMod.empty() &&
              std::filesystem::exists(mainSnap.lastPlayedMod)) {
            choice.modFolder = mainSnap.lastPlayedMod;
          }
          done = true;
        } else if (picked == &kDeveloperEntry) {
          OpenDevMenuModal();
        } else if (std::string(picked->label) == "Join with Code") {
          st.screen = Screen::Servers;
          UIManager::GetServerBrowser().OpenJoinCodeModal();
        } else if (std::string(picked->label) == "Account") {
          OpenAccountModal();
        } else {
          st.screen = picked->target;
          if (st.screen == Screen::Social) {
            st.socialPreviousScreen = Screen::Main;
          }
        }
        st.focusRow = 0;
      } else if (back) {
        // Esc on the main menu asks before closing Noble.
        st.confirmQuit = true;
        st.quitChoice = 1;
        MenuSounds::Back();
      } else if (!ImGui::GetIO().WantTextInput &&
                 (UiPad::ActionY() || ImGui::IsKeyPressed(ImGuiKey_N, false) ||
                  HintExtraClicked("What's New"))) {
        s_showChangelogManual = true;
        MenuSounds::Click();
      }
      DrawHints(ui, {{Prompt::Accept, "Select"}, {Prompt::Back, "Quit"}},
                {{InputPrompts::Button::Y, "N", "What's New", true}});
    } else if (st.confirmQuit) {
      // Dim everything and show the question in the middle.
      ui.dl->AddRectFilled(
          ui.origin, ImVec2(ui.origin.x + ui.size.x, ui.origin.y + ui.size.y),
          IM_COL32(0, 0, 0, 170));
      const float bw = 480.0f * ui.s, bh = 200.0f * ui.s;
      const ImVec2 b0(ui.origin.x + (ui.size.x - bw) * 0.5f,
                      ui.origin.y + (ui.size.y - bh) * 0.5f);
      const ImVec2 b1(b0.x + bw, b0.y + bh);
      ui.dl->AddRectFilled(b0, b1, IM_COL32(12, 20, 30, 248), 4.0f * ui.s);
      ui.dl->AddRect(b0, b1, IM_COL32(143, 216, 255, 180), 4.0f * ui.s, 0,
                     1.5f);
      ui.dl->AddLine(ImVec2(b0.x, b0.y + 3.0f * ui.s),
                     ImVec2(b1.x, b0.y + 3.0f * ui.s), kAccent, 3.0f * ui.s);
      Text(ui, NobleStyle::TitleFont(), 26.0f,
           ImVec2(b0.x + 24.0f * ui.s, b0.y + 22.0f * ui.s), kWhite,
           "QUIT PROJECT NOBLE?");
      Text(ui, nullptr, 17.0f, ImVec2(b0.x + 24.0f * ui.s, b0.y + 64.0f * ui.s),
           kDim, "Are you sure you want to close the game?");
      const float btnW = (bw - 24.0f * ui.s * 3.0f) * 0.5f;
      const float btnY = b1.y - 64.0f * ui.s;
      if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false) ||
          ImGui::IsKeyPressed(ImGuiKey_RightArrow, false) ||
          ImGui::IsKeyPressed(ImGuiKey_UpArrow, false) ||
          ImGui::IsKeyPressed(ImGuiKey_DownArrow, false) ||
          ImGui::IsKeyPressed(ImGuiKey_Tab, false) || UiPad::Left() ||
          UiPad::Right() || UiPad::Up() || UiPad::Down()) {
        st.quitChoice = 1 - st.quitChoice;
        MenuSounds::Scroll();
      }
      const bool mouseMovedQuit = ImGui::GetIO().MouseDelta.x != 0.0f ||
                                  ImGui::GetIO().MouseDelta.y != 0.0f;
      const bool yes =
          ActionButton(ui, "quit_yes", ImVec2(b0.x + 24.0f * ui.s, btnY), btnW,
                       "YES", st.quitChoice == 0, st.quitChoice == 0);
      if (ImGui::IsItemHovered() && mouseMovedQuit)
        st.quitChoice = 0;
      const bool no =
          ActionButton(ui, "quit_no", ImVec2(b0.x + 48.0f * ui.s + btnW, btnY),
                       btnW, "NO", st.quitChoice == 1, st.quitChoice == 1);
      if (ImGui::IsItemHovered() && mouseMovedQuit)
        st.quitChoice = 1;
      if (yes || (AcceptPressed() && st.quitChoice == 0)) {
        UiPad::ConsumeAccept();
        st.confirmQuit = false;
        done = true;
      } else if (no || back || (AcceptPressed() && st.quitChoice == 1)) {
        UiPad::ConsumeAccept();
        UiPad::ConsumeBack();
        st.confirmQuit = false;
        if (!no)
          MenuSounds::Back();
      }
      DrawHints(ui, {{Prompt::Accept, "Select"}, {Prompt::Back, "Cancel"}});
    }
    ImGui::BeginDisabled(modal);
    DrawMusicPlayer(ui);
    ImGui::EndDisabled();

    std::string version(VERSION_STRING);
    const size_t paren = version.find(" (");
    if (paren != std::string::npos)
      version.resize(paren);
    char ver[64];
    std::snprintf(ver, sizeof(ver), "PROJECT NOBLE  v%s", version.c_str());
    const float verW = TextWidth(NobleStyle::SmallFont(), 14.0f * ui.s, ver);
    const float verX = ui.origin.x + ui.size.x - verW - 24.0f * ui.s;
    const float verY = ui.origin.y + ui.size.y - 30.0f * ui.s;
    Text(ui, NobleStyle::SmallFont(), 14.0f, ImVec2(verX, verY),
         IM_COL32(200, 210, 220, 110), ver);

    {
      const char *wnLbl = "WHAT'S NEW";
      const float wnTw =
          TextWidth(NobleStyle::SmallFont(), 12.5f * ui.s, wnLbl);
      const float wnW = wnTw + 22.0f * ui.s;
      const float wnH = 24.0f * ui.s;
      const ImVec2 wn0(verX - wnW - 14.0f * ui.s, verY - 4.0f * ui.s);
      const ImVec2 wn1(wn0.x + wnW, wn0.y + wnH);
      bool wnHov = false;
      bool wnClick = false;
      if (!modal) {
        ImGui::SetCursorScreenPos(wn0);
        wnClick = ImGui::InvisibleButton("##main_whats_new", ImVec2(wnW, wnH));
        wnHov = ImGui::IsItemHovered();
      }
      ui.dl->AddRectFilled(wn0, wn1,
                           wnHov ? IM_COL32(28, 60, 88, 232)
                                 : IM_COL32(14, 26, 40, 195),
                           2.0f * ui.s);
      ui.dl->AddRect(wn0, wn1,
                     wnHov ? IM_COL32(143, 216, 255, 230)
                           : IM_COL32(110, 165, 210, 125),
                     2.0f * ui.s, 0, 1.0f);
      Text(ui, NobleStyle::SmallFont(), 12.5f,
           ImVec2(wn0.x + 11.0f * ui.s, wn0.y + 5.0f * ui.s),
           wnHov ? kWhite : IM_COL32(180, 214, 240, 215), wnLbl);
      if (wnClick) {
        s_showChangelogManual = true;
        MenuSounds::Click();
      }
    }

    if (showChangelog) {
      DrawChangelogModal(ui, s_showChangelogManual);
    }
    break;
  }

  case Screen::Campaign: {
    DrawCampaignScreen(ctx, ui, left, top, choice, done, back);
    break;
  }

  case Screen::Skulls: {
    DrawSkullsScreen(ctx, ui, left, top, settings, back);
    break;
  }

  case Screen::Firefight:
  case Screen::CustomGame:
  case Screen::Forge: {
    DrawFirefightCustomForgeScreen(ctx, ui, left, top, choice, done, back);
    break;
  }
  case Screen::Theater: {
    if (!ctx.theaterScanned) {
      ctx.theaterEntries = ScanTheaterEntries();
      ctx.theaterScanned = true;
    }

    DrawTitle(ui, ImVec2(left, top), "THEATER", "LOBBY");

    // Count items per Theater category tab
    int countAll = static_cast<int>(ctx.theaterEntries.size());
    int countRecent = 0, countSaved = 0, countVideos = 0;
    for (const TheaterEntry &e : ctx.theaterEntries) {
      if (e.isVideo)
        ++countVideos;
      else if (e.isSaved)
        ++countSaved;
      else
        ++countRecent;
    }

    // Category tabs: ALL / RECENT FILMS / SAVED FILMS / VIDEOS (Q / E or LB /
    // RB to cycle)
    if (!ImGui::GetIO().WantTextInput) {
      if (UiPad::TabPrev()) {
        st.theaterSource = (st.theaterSource + 3) % 4;
        st.focusRow = 0;
        st.theaterStatusMsg.clear();
        st.theaterConfirmDelete.clear();
        MenuSounds::Scroll();
      } else if (UiPad::TabNext() || HintClicked(Prompt::Tabs)) {
        st.theaterSource = (st.theaterSource + 1) % 4;
        st.focusRow = 0;
        st.theaterStatusMsg.clear();
        st.theaterConfirmDelete.clear();
        MenuSounds::Scroll();
      }
    }

    const std::string tabLabels[4] = {
        "ALL (" + std::to_string(countAll) + ")",
        "RECENT (" + std::to_string(countRecent) + ")",
        "SAVED (" + std::to_string(countSaved) + ")",
        "VIDEOS (" + std::to_string(countVideos) + ")"};
    float tabX = left + 34.0f * ui.s;
    const float tabY = top + 70.0f * ui.s;
    const float tabH = 26.0f * ui.s;
    for (int t = 0; t < 4; ++t) {
      const float tw = TextWidth(NobleStyle::SmallFont(), 13.0f * ui.s,
                                 tabLabels[t].c_str()) +
                       16.0f * ui.s;
      const ImVec2 a(tabX, tabY);
      const ImVec2 b(tabX + tw, tabY + tabH);
      ImGui::SetCursorScreenPos(a);
      ImGui::PushID(8800 + t);
      const bool pressed = ImGui::InvisibleButton("##thtab", ImVec2(tw, tabH));
      const bool hovered = ImGui::IsItemHovered();
      ImGui::PopID();
      const bool active = (st.theaterSource == t);
      ui.dl->AddRectFilled(a, b,
                           active ? IM_COL32(36, 78, 112, 235)
                                  : (hovered ? IM_COL32(28, 44, 62, 210)
                                             : IM_COL32(12, 18, 26, 185)));
      ui.dl->AddRect(a, b, active ? kAccent : IM_COL32(110, 145, 175, 80));
      Text(ui, NobleStyle::SmallFont(), 13.0f,
           ImVec2(a.x + 8.0f * ui.s, a.y + 5.0f * ui.s), active ? kWhite : kDim,
           tabLabels[t].c_str());
      if (pressed) {
        st.theaterSource = t;
        st.focusRow = 0;
        st.theaterStatusMsg.clear();
        st.theaterConfirmDelete.clear();
        MenuSounds::Scroll();
      }
      tabX += tw + 5.0f * ui.s;
    }

    // Filter entries by tab and search query
    std::vector<const TheaterEntry *> filtered;
    for (const TheaterEntry &e : ctx.theaterEntries) {
      if (st.theaterSource == 1 && (e.isVideo || e.isSaved))
        continue;
      if (st.theaterSource == 2 && (e.isVideo || !e.isSaved))
        continue;
      if (st.theaterSource == 3 && !e.isVideo)
        continue;
      if (st.theaterSearch[0] != '\0') {
        if (!ContainsNoCase(e.title, st.theaterSearch) &&
            !ContainsNoCase(e.mapDisplay, st.theaterSearch) &&
            !ContainsNoCase(e.modeLabel, st.theaterSearch) &&
            !ContainsNoCase(e.author, st.theaterSearch)) {
          continue;
        }
      }
      filtered.push_back(&e);
    }

    int &pick =
        (st.theaterSearch[0] != '\0') ? st.theaterSearchRow : st.theaterIndex;
    pick = filtered.empty()
               ? 0
               : std::clamp(pick, 0, static_cast<int>(filtered.size()) - 1);

    std::vector<ListItem> items;
    items.reserve(filtered.size());
    for (size_t i = 0; i < filtered.size(); ++i) {
      const TheaterEntry *e = filtered[i];
      std::string rowLabel = e->title;
      if (e->isVideo) {
        rowLabel = "[VIDEO] " + rowLabel + "  (" +
                   FormatFileSizeShort(e->fileSize) + ")";
      } else {
        if (e->isSaved && st.theaterSource == 0)
          rowLabel = "[SAVED] " + rowLabel;
        if (e->durationSec > 0)
          rowLabel += "  [" + FormatDurationMmSs(e->durationSec) + "]";
      }
      ListItem it{rowLabel, true, false, false};
      it.thumb = e->scenario;
      it.number = static_cast<int>(i) + 1;
      items.push_back(std::move(it));
    }
    if (filtered.empty()) {
      const char *emptyMsg =
          (st.theaterSearch[0] != '\0') ? "Nothing matches your search"
          : (st.theaterSource == 2)
              ? "No bookmarked films in data\\theater\\saved"
          : (st.theaterSource == 3)
              ? "No video clips in data\\theater\\videos"
              : "No saved films or videos in data\\theater yet";
      items.push_back({emptyMsg, false, false, false});
    }

    const float listTop = top + 108.0f * ui.s;
    const float listRow = 40.0f;
    const int listRows = std::max(
        4, static_cast<int>((ui.origin.y + ui.size.y - 74.0f * ui.s - listTop) /
                            (listRow * ui.s)));
    const int hit = DrawList(
        ui, "theater_list", ImVec2(left + 34.0f * ui.s, listTop), 400.0f * ui.s,
        items, pick, st.focusRow == 0 && !ImGui::GetIO().WantTextInput, listRow,
        listRows, false);
    if (hit >= 0 && !filtered.empty()) {
      pick = std::clamp(hit, 0, static_cast<int>(filtered.size()) - 1);
      st.focusRow = 1;
      st.theaterStatusMsg.clear();
      st.theaterConfirmDelete.clear();
    }

    const TheaterEntry *sel = filtered.empty() ? nullptr : filtered[pick];

    // Right column: Halo: Reach Theater Lobby card + metadata + controls +
    // action buttons
    const float px = ui.origin.x + ui.size.x * 0.47f, pw = ui.size.x * 0.44f;
    const RightColumnY col = RightColumnLayout(ui);
    const float actionsY = col.actions;

    // Search box sits neatly at top-right of the right column header
    if (DrawSearchBox(ui, "##theater_search",
                      ImVec2(px + pw - 180.0f * ui.s, top + 64.0f * ui.s),
                      180.0f * ui.s, st.theaterSearch, sizeof(st.theaterSearch),
                      "Search films/videos")) {
      st.theaterSearchRow = 0;
    }

    const float rowsH = 36.0f * ui.s * 4.0f + 56.0f * ui.s;
    const float bannerTop = top + 96.0f * ui.s;
    const std::string fallbackBlurb =
        "Play any Campaign, Firefight, Custom Game, or Forge match to "
        "automatically record a saved film, or drop .film and video files "
        "(.mp4, .mkv, .webm, .avi) into data\\theater.";
    const char *blurb = sel ? sel->description.c_str() : fallbackBlurb.c_str();
    const float blurbH = BlurbBlockHeight(ui, pw, blurb);
    const float bannerH =
        HeroBannerHeight(ui, pw, col.content, rowsH, blurbH, bannerTop);
    const std::string kicker =
        sel ? ("HALO: REACH \xC2\xB7 THEATER \xC2\xB7 " + sel->modeLabel)
            : std::string("HALO: REACH \xC2\xB7 THEATER");
    const std::string bannerTitle =
        sel ? sel->title : std::string("NO FILM SELECTED");
    const std::string bannerScen =
        sel ? sel->scenario : std::string("20_sword_slayer");

    float y = HeroBanner(ui, ImVec2(px, bannerTop), pw, bannerH, bannerScen,
                         kicker, bannerTitle.c_str(), blurb);

    // Info / option rows
    const std::string mapAndMode =
        sel ? (sel->mapDisplay + " \xC2\xB7 " + sel->modeLabel)
            : std::string("None");
    const std::string durationAndSize =
        sel ? ((sel->isVideo
                    ? std::string("Video Clip")
                    : ("Length " + FormatDurationMmSs(sel->durationSec))) +
               " \xC2\xB7 " + FormatFileSizeShort(sel->fileSize))
            : std::string("-");
    const std::string authorAndDate =
        sel ? (sel->author + " \xC2\xB7 " +
               FormatTimestampUtcOrLocal(sel->timestamp))
            : std::string("-");

    OptionRow(ui, "th_location", ImVec2(px, y), pw, "MAP & MODE",
              mapAndMode.c_str(), false, nullptr, false);
    y += 36.0f * ui.s;
    OptionRow(ui, "th_details", ImVec2(px, y), pw, "DURATION & SIZE",
              durationAndSize.c_str(), false, nullptr, false);
    y += 36.0f * ui.s;
    OptionRow(ui, "th_recorded", ImVec2(px, y), pw, "RECORDED BY",
              authorAndDate.c_str(), false, nullptr, false);
    y += 36.0f * ui.s;

    // File management bar: SAVE FILM (bookmark to data\theater\saved), DELETE
    // FILE, REFRESH
    {
      const float btnGap = 8.0f * ui.s;
      const float btnW = (pw - btnGap * 2.0f) / 3.0f;
      const float btnH = 30.0f * ui.s;
      auto smallAction = [&](const char *id, float bx, const char *label,
                             bool enabled, bool focused, bool danger = false) {
        const ImVec2 a(bx, y);
        const ImVec2 b(bx + btnW, y + btnH);
        ImGui::SetCursorScreenPos(a);
        ImGui::PushID(id);
        const bool pressed =
            ImGui::InvisibleButton("##thact", ImVec2(btnW, btnH)) && enabled;
        const bool hovered = ImGui::IsItemHovered() && enabled;
        ImGui::PopID();
        const ImU32 bg = !enabled ? IM_COL32(16, 20, 26, 140)
                         : danger
                             ? (hovered || focused ? IM_COL32(130, 36, 36, 235)
                                                   : IM_COL32(80, 24, 24, 210))
                             : (hovered || focused ? IM_COL32(42, 86, 124, 235)
                                                   : IM_COL32(20, 32, 46, 210));
        const ImU32 edge = !enabled               ? IM_COL32(80, 90, 100, 60)
                           : (hovered || focused) ? kAccent
                                                  : IM_COL32(120, 155, 185, 95);
        ui.dl->AddRectFilled(a, b, bg);
        ui.dl->AddRect(a, b, edge);
        const float tw =
            TextWidth(NobleStyle::SmallFont(), 13.0f * ui.s, label);
        Text(ui, NobleStyle::SmallFont(), 13.0f,
             ImVec2(a.x + (btnW - tw) * 0.5f, a.y + 6.0f * ui.s),
             enabled ? kWhite : kDisabled, label);
        if (pressed)
          MenuSounds::Click();
        return pressed;
      };

      const bool canSaveBookmark = sel && !sel->isVideo && !sel->isSaved;
      const char *saveLabel =
          (sel && sel->isSaved) ? "SAVED IN LIBRARY" : "SAVE TO LIBRARY";
      if (smallAction("th_save", px, saveLabel, canSaveBookmark,
                      st.focusRow == 1)) {
        std::error_code ec;
        const std::filesystem::path savedDir = TheaterRootDir() / "saved";
        std::filesystem::create_directories(savedDir, ec);
        const std::filesystem::path dst = savedDir / sel->path.filename();
        std::filesystem::copy_file(
            sel->path, dst, std::filesystem::copy_options::overwrite_existing,
            ec);
        if (!ec) {
          st.theaterStatusMsg =
              "Saved '" + sel->title + "' to data\\theater\\saved.";
          ctx.theaterScanned = false;
        } else {
          st.theaterStatusMsg = "Could not copy film to data\\theater\\saved.";
        }
      }

      const bool confirmingDelete =
          sel && st.theaterConfirmDelete == sel->path.string();
      const char *delLabel =
          confirmingDelete ? "CONFIRM DELETE?" : "DELETE FILE";
      if (smallAction("th_delete", px + btnW + btnGap, delLabel, sel != nullptr,
                      st.focusRow == 2, confirmingDelete)) {
        if (!confirmingDelete) {
          st.theaterConfirmDelete = sel->path.string();
          st.theaterStatusMsg =
              "Press CONFIRM DELETE? again to permanently remove '" +
              sel->path.filename().string() + "'.";
          MenuSounds::Error();
        } else {
          std::error_code ec;
          const std::string removedName = sel->path.filename().string();
          std::filesystem::remove(sel->path, ec);
          st.theaterConfirmDelete.clear();
          if (!ec) {
            st.theaterStatusMsg = "Deleted '" + removedName + "'.";
            ctx.theaterScanned = false;
          } else {
            st.theaterStatusMsg = "Could not delete '" + removedName + "'.";
          }
        }
      }

      if (smallAction("th_refresh", px + (btnW + btnGap) * 2.0f, "REFRESH LIST",
                      true, st.focusRow == 3)) {
        ctx.theaterScanned = false;
        st.theaterConfirmDelete.clear();
        st.theaterStatusMsg =
            "Rescanned data\\theater and synced recent autosave films.";
      }
      y += 38.0f * ui.s;
    }

    // Theater Controls reference card / status message
    {
      const ImVec2 c0(px, y);
      const ImVec2 c1(px + pw, y + 48.0f * ui.s);
      ui.dl->AddRectFilled(c0, c1, IM_COL32(10, 16, 24, 195));
      ui.dl->AddRect(c0, c1, IM_COL32(100, 140, 175, 75));
      Text(ui, NobleStyle::SmallFont(), 12.5f,
           ImVec2(c0.x + 10.0f * ui.s, c0.y + 6.0f * ui.s), kAccent,
           "THEATER CAMERA & PLAYBACK CONTROLS");
      Text(ui, NobleStyle::SmallFont(), 12.5f,
           ImVec2(c0.x + 10.0f * ui.s, c0.y + 24.0f * ui.s), kDim,
           "SPACE Pause/Play  \xC2\xB7  F Free Camera  \xC2\xB7  C 1st/3rd "
           "Person  \xC2\xB7  X Timeline  \xC2\xB7  Z Hide HUD  \xC2\xB7  RMB "
           "Fast-Forward");
      y += 54.0f * ui.s;
    }

    if (!st.theaterStatusMsg.empty()) {
      Text(ui, NobleStyle::SmallFont(), 13.5f,
           ImVec2(px + 4.0f * ui.s, col.strip + 6.0f * ui.s),
           IM_COL32(143, 216, 255, 240), st.theaterStatusMsg.c_str());
    } else {
      const std::string folderHint =
          "Local Theater Folder: " + TheaterRootDir().string();
      Text(ui, NobleStyle::SmallFont(), 13.0f,
           ImVec2(px + 4.0f * ui.s, col.strip + 6.0f * ui.s), kDim,
           folderHint.c_str());
    }

    // Primary action buttons at bottom of right column
    const float bw = (pw - 10.0f * ui.s) * 0.5f;
    const bool canWatch = sel != nullptr;
    const char *watchLabel =
        (sel && sel->isVideo) ? "WATCH VIDEO" : "START FILM";
    const std::string watchSub =
        !sel           ? "Select a film or video"
        : sel->isVideo ? ("Open " + sel->path.filename().string())
                       : (sel->mapDisplay + " \xC2\xB7 " + sel->modeLabel);
    const int watchRow = 4;
    const int folderRow = 5;

    const bool startWatch = ((BigAction(ui, "th_watch", ImVec2(px, actionsY),
                                        bw, watchLabel, watchSub.c_str(), true,
                                        canWatch, st.focusRow == watchRow) ||
                              (st.focusRow == watchRow && AcceptPressed())) &&
                             canWatch);
    const bool openFolder =
        BigAction(ui, "th_folder", ImVec2(px + bw + 10.0f * ui.s, actionsY),
                  pw - bw - 10.0f * ui.s, "THEATER FOLDER",
                  "Open data\\theater in Explorer", true, true,
                  st.focusRow == folderRow) ||
        (st.focusRow == folderRow && AcceptPressed());

    if (startWatch && sel) {
      if (sel->isVideo) {
        const std::filesystem::path absVideo =
            std::filesystem::absolute(sel->path);
        ShellExecuteW(nullptr, L"open", absVideo.c_str(), nullptr,
                      absVideo.parent_path().c_str(), SW_SHOWNORMAL);
        st.theaterStatusMsg = "Opened video '" + sel->path.filename().string() +
                              "' in media player.";
      } else {
        FrontEndChoice req;
        req.play = true;
        req.host = false;
        req.map = sel->scenario;
        req.mode = sel->launchMode;
        req.savedFilm = sel->path;
        req.mapTitle = sel->title;
        RequestMatchLaunch(choice, done, req);
      }
    }
    if (openFolder) {
      std::error_code ec;
      const std::filesystem::path root =
          std::filesystem::absolute(TheaterRootDir(), ec);
      std::filesystem::create_directories(root / "films", ec);
      std::filesystem::create_directories(root / "saved", ec);
      std::filesystem::create_directories(root / "videos", ec);
      ShellExecuteW(nullptr, L"explore", root.c_str(), nullptr, nullptr,
                    SW_SHOWNORMAL);
      st.theaterStatusMsg =
          "Opened '" + root.string() + "' in Windows Explorer.";
    }

    if (!g_pickerBlocksNav && st.focusRow > 0 &&
        (ImGui::IsKeyPressed(ImGuiKey_DownArrow) ||
         ImGui::IsKeyPressed(ImGuiKey_Tab) || UiPad::Down())) {
      st.focusRow = std::min(folderRow, st.focusRow + 1);
    }
    if (!g_pickerBlocksNav && st.focusRow > 0 &&
        (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up())) {
      st.focusRow = std::max(1, st.focusRow - 1);
    }
    if (back) {
      if (st.focusRow > 0)
        st.focusRow = 0;
      else
        goBack(Screen::Main);
    }
    DrawHints(ui, {{Prompt::Accept, "Select"},
                   {Prompt::Back, "Back"},
                   {Prompt::Tabs, "All / Recent / Saved / Videos"}});
    break;
  }

  case Screen::Armory: {
    // MCC Halo: Reach "CUSTOMIZATION / APPEARANCE" header + nameplate + live
    // in-engine 3D Spartan/Elite
    ArmoryStage::Draw(ui, device, context, top + 64.0f * ui.s);
    Text(ui, NobleStyle::SmallFont(), 15.0f, ImVec2(left, top - 8.0f * ui.s),
         kAccent, "CUSTOMIZATION");
    Text(ui, NobleStyle::HeroFont(), 44.0f, ImVec2(left, top + 6.0f * ui.s),
         kWhite, "APPEARANCE");

    // Top-right: the same player row as MY PARTY on the main menu, with the
    // spendable cR balance just to its left (the Armory is where credits get
    // spent).
    {
      const auto &prof = UIManager::GetArmorCustomizer().GetProfile();
      const float rowW = 400.0f * ui.s;
      const ImVec2 cardPos(ui.origin.x + ui.size.x * 0.925f - rowW,
                           top + 8.0f * ui.s);
      const std::string credits = "CR: " + NumberWithCommas(Career::Balance());
      const float creditW =
          TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, credits.c_str());
      Text(ui, NobleStyle::TitleFont(), 18.0f,
           ImVec2(cardPos.x - 18.0f * ui.s - creditW, cardPos.y + 20.0f * ui.s),
           IM_COL32(143, 216, 255, 235), credits.c_str());
      PlayerRow(ui, cardPos, rowW, Career::RankIndex(Career::Get().credits),
                prof.emblemIndex, prof.nameplateIndex, 0,
                PlayerServiceTag(settings), settings.playerName,
                AuthClient::Get().IsDeveloper(), false,
                prof.emblemBackgroundIndex,
                UIManager::GetArmorCustomizer().EmblemColorsNow(),
                AuthClient::Get().IsTester());
    }

    // Thin horizontal rule with active "HALO: REACH" underline
    const float ruleY = top + 64.0f * ui.s;
    ui.dl->AddLine(ImVec2(left, ruleY),
                   ImVec2(ui.origin.x + ui.size.x * 0.925f, ruleY),
                   IM_COL32(140, 180, 220, 55), 1.0f);
    const char *gameTab = "HALO: REACH";
    const float tabW =
        TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, gameTab);
    const float tabX = ui.origin.x + (ui.size.x - tabW) * 0.5f;
    Text(ui, NobleStyle::TitleFont(), 18.0f, ImVec2(tabX, ruleY - 24.0f * ui.s),
         kWhite, gameTab);
    ui.dl->AddLine(ImVec2(tabX - 8.0f * ui.s, ruleY),
                   ImVec2(tabX + tabW + 8.0f * ui.s, ruleY), kAccent,
                   2.5f * ui.s);

    ImGui::SetCursorScreenPos(ImVec2(left, ruleY + 12.0f * ui.s));
    ImGui::BeginChild(
        "##armory_panel",
        ImVec2(ui.size.x * 0.90f,
               ui.origin.y + ui.size.y - (ruleY + 12.0f * ui.s) - 62.0f * ui.s),
        ImGuiChildFlags_None,
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoScrollbar);
    auto &armory = UIManager::GetArmorCustomizer();
    armory.Render(device, context);
    ImGui::EndChild();

    if (back && !ImGui::GetIO().WantTextInput) {
      if (!armory.HandleBackInput()) {
        goBack(Screen::Main);
      }
    }
    if (armory.IsColorModalOpen()) {
      DrawHints(ui, {{Prompt::Accept, "Confirm"},
                     {Prompt::Back, "Close"},
                     {Prompt::Change, "Adjust RGB"}});
    } else if (armory.IsPickerFocused()) {
      DrawHints(ui,
                {{Prompt::Accept, "Equip / Buy"}, {Prompt::Back, "Slot List"}},
                {{InputPrompts::Button::DpadDown, "ARROWS", "Grid"},
                 {InputPrompts::Button::RStick, "Q/E", "Rotate"}});
    } else {
      DrawHints(ui,
                {{Prompt::Accept, "Browse"},
                 {Prompt::Back, "Back"},
                 {Prompt::Change, "Cycle"}},
                {{InputPrompts::Button::LB, "LB/RB", "Species"},
                 {InputPrompts::Button::Y, "Y", "Random"},
                 {InputPrompts::Button::RStick, "Q/E", "Rotate"}});
    }
    break;
  }

  case Screen::Servers:
  case Screen::Settings:
  case Screen::Career:
  case Screen::Mods: {
    const char *title = st.screen == Screen::Servers    ? "SERVER BROWSER"
                        : st.screen == Screen::Settings ? "SETTINGS"
                        : st.screen == Screen::Career   ? "SERVICE RECORD"
                                                        : "MODS";
    DrawTopTelemetryBar(ui, title);
    // Full-width tactical container cleanly framed under the telemetry strip
    const float px = ui.origin.x + 24.0f * ui.s;
    const float py = ui.origin.y + 88.0f * ui.s;
    ui.dl->AddRectFilled(
        ui.origin, ImVec2(ui.origin.x + ui.size.x, ui.origin.y + ui.size.y),
        IM_COL32(4, 8, 12, 222));
    DrawTitle(ui, ImVec2(px, ui.origin.y + 18.0f * ui.s), title);
    const float panelW = ui.size.x - 48.0f * ui.s;
    const float panelH = ui.origin.y + ui.size.y - py - 62.0f * ui.s;
    const ImVec2 pc0(px, py), pc1(px + panelW, py + panelH);
    ui.dl->AddRectFilled(pc0, pc1, IM_COL32(8, 14, 22, 185), 2.0f * ui.s);
    NobleUi::DrawTacticalFrame(ui.dl, pc0, pc1, ui.s, false, false, false,
                               false);

    ImGui::SetCursorScreenPos(ImVec2(px + 8.0f * ui.s, py + 8.0f * ui.s));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::BeginChild("##panel",
                      ImVec2(panelW - 16.0f * ui.s, panelH - 16.0f * ui.s),
                      ImGuiChildFlags_None, ImGuiWindowFlags_None);
    if (st.screen == Screen::Servers) {
      UIManager::GetServerBrowser().Render();
      if (UIManager::GetServerBrowser().ConsumeCloseForUpdate())
        done = true; // update script takes over
    } else if (st.screen == Screen::Settings) {
      ctx.settingsPanel.Render();
    } else if (st.screen == Screen::Career) {
      ctx.careerPanel.Render();
    } else {
      if (!ctx.modsScanned) {
        const auto steamapps =
            ctx.reachDir.parent_path().parent_path().parent_path();
        wchar_t exe[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exe, MAX_PATH);
        UIManager::GetWorkshopManager().RefreshMods(
            steamapps / "workshop" / "content" / "976730",
            std::filesystem::path(exe).parent_path() / "data" / "mods");
        ctx.modsScanned = true;
      }
      // RECOMMENDED MAPS: optional map packs that live on this client's GitHub
      // release (Forge World etc.). Drawn above the mod list so the rows are
      // visible without scrolling.
      {
        RecommendedMaps &recommended = RecommendedMaps::Get();
        ImGui::TextColored(ImVec4(0.56f, 0.85f, 1.0f, 1.0f),
                           "RECOMMENDED MAPS");
        ImGui::Dummy(ImVec2(0.0f, 2.0f));
        for (const RecommendedMaps::Item &item : recommended.Items()) {
          const bool installed = recommended.IsInstalled(item);
          const RecommendedMaps::Status status = recommended.Snapshot(item.id);
          ImGui::PushID(item.id.c_str());
          ImGui::TextUnformatted(item.title.c_str());
          ImGui::TextColored(ImVec4(0.65f, 0.72f, 0.80f, 1.0f), "%s",
                             item.description.c_str());
          if (installed) {
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "INSTALLED");
          } else if (status.state == RecommendedMaps::State::Resolving ||
                     status.state == RecommendedMaps::State::Downloading ||
                     status.state == RecommendedMaps::State::Verifying ||
                     status.state == RecommendedMaps::State::Installing) {
            const char *phase =
                status.state == RecommendedMaps::State::Resolving
                    ? "Contacting GitHub..."
                : status.state == RecommendedMaps::State::Verifying
                    ? "Verifying..."
                : status.state == RecommendedMaps::State::Installing
                    ? "Installing..."
                    : "Downloading...";
            char label[160];
            if (status.total) {
              std::snprintf(
                  label, sizeof(label), "%s %s / %s", phase,
                  RecommendedMaps::FormatBytes(status.received).c_str(),
                  RecommendedMaps::FormatBytes(status.total).c_str());
            } else {
              std::snprintf(label, sizeof(label), "%s", phase);
            }
            ImGui::ProgressBar(status.progress, ImVec2(380.0f, 20.0f), label);
          } else {
            if (status.state == RecommendedMaps::State::Error) {
              ImGui::TextColored(ImVec4(0.95f, 0.45f, 0.35f, 1.0f),
                                 "DOWNLOAD FAILED: %s", status.error.c_str());
            }
            std::string buttonLabel = "DOWNLOAD##dl";
            if (item.approxBytes)
              buttonLabel = "DOWNLOAD (" +
                            RecommendedMaps::FormatBytes(item.approxBytes) +
                            ")##dl";
            if (ImGui::Button(buttonLabel.c_str(), ImVec2(200.0f, 28.0f)))
              recommended.StartDownload(item.id);
          }
          ImGui::Dummy(ImVec2(0.0f, 12.0f));
          ImGui::PopID();
        }
        ImGui::Dummy(ImVec2(0.0f, 8.0f));
        ImGui::Separator();
      }
      UIManager::GetWorkshopManager().Render();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    const bool serverModalOpen = (st.screen == Screen::Servers &&
                                  UIManager::GetServerBrowser().IsModalOpen());
    const bool escDiscards = (st.screen == Screen::Settings) &&
                             ImGui::IsKeyPressed(ImGuiKey_Escape, false) &&
                             !ctx.settingsPanel.IsCapturingKey() &&
                             !ImGui::GetIO().WantTextInput &&
                             ctx.settingsPanel.DiscardOnEscape();
    if (back && serverModalOpen && !ImGui::GetIO().WantTextInput) {
      UIManager::GetServerBrowser().CloseModals();
      MenuSounds::Back();
    } else if (back && !escDiscards && !ImGui::GetIO().WantTextInput &&
               !ctx.settingsPanel.IsCapturingKey()) {
      goBack(Screen::Main);
    }
    if (st.screen == Screen::Settings)
      DrawHints(ui,
                {{Prompt::Accept, "Select"},
                 {Prompt::Back, "Back"},
                 {Prompt::Change, "Change"},
                 {Prompt::Tabs, "Category"}},
                {}, 0.01875f);
    else if (st.screen == Screen::Career)
      DrawHints(ui, {{Prompt::Back, "Back"}, {Prompt::Tabs, "Switch Tab"}},
                ctx.careerPanel.IsCommendationsTab()
                    ? std::vector<HintExtra>{{InputPrompts::Button::DpadDown,
                                              "ARROWS", "Navigate"},
                                             {InputPrompts::Button::LT, "PgUp",
                                              "Mode"}}
                    : std::vector<HintExtra>{{InputPrompts::Button::DpadDown,
                                              "ARROWS", "Navigate"},
                                             {InputPrompts::Button::RStick,
                                              "WHEEL", "Scroll"}},
                0.01875f);
    else if (st.screen == Screen::Servers)
      DrawHints(ui,
                {{Prompt::Accept, "Join"},
                 {Prompt::Back, "Back"},
                 {Prompt::Tabs, "Mode Filter"}},
                {{InputPrompts::Button::X, "X", "Refresh"},
                 {InputPrompts::Button::Y, "Y", "Favourite"}},
                0.01875f);
    else
      DrawHints(ui, {{Prompt::Back, "Back"}}, {}, 0.01875f);
    break;
  }

  case Screen::PreGameLobby: {
    DrawPreGameLobby(ctx, ui, left, top, choice, done, back);
    break;
  }

  case Screen::Social: {
    DrawSocialScreen(ctx, ui, left, top, choice, done, back);
    break;
  }
  }

  if (!choice.play && !done && ctx.hasPendingJoin) {
    FrontEndChoice req = std::move(ctx.pendingJoinChoice);
    ctx.hasPendingJoin = false;
    RequestMatchLaunch(choice, done, req);
  }

  // Friend invites reach the player on any screen (Settings > Online services
  // can turn this off).
  DrawInviteToast(ui, st);
  if (st.pendingInviteJoin) {
    st.pendingInviteJoin = false;
    ServerEntry s;
    s.id = st.pendingInviteSession;
    const size_t colon = st.pendingInviteConnect.rfind(':');
    s.hostAddress = colon != std::string::npos
                        ? st.pendingInviteConnect.substr(0, colon)
                        : st.pendingInviteConnect;
    s.port = colon != std::string::npos
                 ? static_cast<uint16_t>(
                       std::atoi(st.pendingInviteConnect.c_str() + colon + 1))
                 : 11774;
    s.mapName =
        st.pendingInviteMap.empty() ? "20_sword_slayer" : st.pendingInviteMap;
    s.gameMode =
        st.pendingInviteMode.empty() ? "multiplayer" : st.pendingInviteMode;
    s.maxPlayers = 16;
    EnterLobby(ctx, s);
  }

  // The Workshop-mod gate sits above every screen: the Server browser stays
  // behind it until the player downloads the lobby's mod or cancels.
  ModDownloadPrompt::Get().Render();

  if (s_screenEnterTick != 0) {
    const ULONGLONG elapsed = GetTickCount64() - s_screenEnterTick;
    if (elapsed < 150) {
      const float t = static_cast<float>(elapsed) / 150.0f;
      const float smooth = t * t * (3.0f - 2.0f * t);
      const int alpha = static_cast<int>(std::round((1.0f - smooth) * 95.0f));
      if (alpha > 0) {
        ui.dl->AddRectFilled(vp->Pos,
                             ImVec2(vp->Pos.x + vp->Size.x,
                                    vp->Pos.y + vp->Size.y),
                             IM_COL32(8, 13, 21, alpha));
      }
    }
  }

  ImGui::End();
}

void GoBackScreen(MenuState &st, Screen to) {
  UiPad::ConsumeBack();
  st.screen = to;
  st.focusRow = 0;
  st.lobbyFocusZone = 0;
  st.mapSearch[0] = 0;
  MenuSounds::Back();
}

} // namespace fe

using namespace noble::fe;

// A Forge map saved during a match is not in the list yet: the scan is cached,
// so drop it.
void FrontEndMenu::InvalidateForgeMaps() {
  OverlayContext &ctx = GetOverlayCtx();
  ctx.forgeMapsScanned = false;
  ctx.forgeMaps.clear();
  ctx.theaterScanned = false;
}

void FrontEndMenu::SyncTheaterFiles() {
  SyncAutosaveFilmsToTheater();
  OverlayContext &ctx = GetOverlayCtx();
  ctx.theaterScanned = false;
}

std::string FrontEndMenu::StockBlurb(const std::string &scenario) {
  for (const MenuMap &m : kMissions)
    if (scenario == m.scenario)
      return m.blurb;
  for (const MenuMap &m : kMaps)
    if (scenario == m.scenario)
      return m.blurb;
  for (const MenuMap &m : kFirefightMaps)
    if (scenario == m.scenario)
      return m.blurb;
  return {};
}

bool FrontEndMenu::IsMatchRunning() { return s_isMatchRunning.load(); }

void FrontEndMenu::SetMatchRunning(bool running) {
  s_isMatchRunning.store(running);
}

void FrontEndMenu::CheckMatchStatus() {
  std::lock_guard<std::recursive_mutex> lock(s_overlayMutex);
  OverlayContext &ctx = GetOverlayCtx();
  if (!ctx.matchRunning || !ctx.hMatchProcess)
    return;
  DWORD exitCode = 0;
  if (GetExitCodeProcess(reinterpret_cast<HANDLE>(ctx.hMatchProcess),
                         &exitCode) &&
      exitCode != STILL_ACTIVE) {
    ContainerMatchEnded(ctx, exitCode);
  }
}

bool FrontEndMenu::ShouldShow() {
  bool hasMap = false;
  bool menuRequested = false;
  bool hasFilm = false;
  int argc = 0;
  LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argv) {
    for (int i = 1; i < argc; ++i) {
      const std::wstring arg = argv[i];
      const std::wstring value = (i + 1 < argc) ? Lower(argv[i + 1]) : L"";
      if (arg == L"-map" && i + 1 < argc) {
        hasMap = true;
        if (value == L"mainmenu" || value == L"menu")
          menuRequested = true;
      } else if (arg == L"-mode" && i + 1 < argc &&
                 (value == L"mainmenu" || value == L"menu")) {
        menuRequested = true;
      } else if (arg == L"-film") {
        hasFilm = true;
      }
    }
    LocalFree(argv);
  }
  if (hasFilm)
    return false;
  return !hasMap || menuRequested;
}

void FrontEndMenu::Initialize(const std::filesystem::path &reachDir) {
  // A relaunch request is only valid while the match that wrote it is exiting
  // (PLAY AGAIN / NEXT MISSION). One left over from a killed match must not
  // start a game by surprise.
  Relaunch::ClearContainerRelaunch();
  // Publish the saved Armory look for the match process, even when the Armory
  // is never opened this run: a match reads it to apply wheel colours (palette
  // colours and the visor index come from the appearance record, so those work
  // without it).
  UIManager::GetArmorCustomizer().SyncToReachArmory();
  OverlayContext &ctx = GetOverlayCtx();
  ctx.reachDir = reachDir;
  // UiArt::Init() has run by now (main.cpp initialises the art before the
  // window is shown). A plate retired or hidden in this build must not survive
  // in the saved appearance; do it before the first PlayerRow draws, so the
  // main menu never shows a plate the Armory would refuse.
  UIManager::GetArmorCustomizer().ValidateNameplate();
  ctx.variants = ScanVariants(reachDir, false);
  ctx.ffVariants = ScanVariants(reachDir, true);
  ctx.forgeMaps = ScanForgeMaps();
  ctx.forgeVariant = reachDir / "game_variants" / kForgeVariant;

  RefreshModCatalog(ctx);

  ctx.st = MenuState{};
  ctx.st.variant = FindVariant(ctx.variants, L"slayer_054.bin");
  ctx.st.ffVariant = FindVariant(ctx.ffVariants, L"hr_ff_standard_1set.bin");
  ctx.st.skulls = Settings::Get().Snapshot().campaignSkulls;

  ReachAudio::SetMenuMusicEnabled(true);
  ReachAudio::SetMusicVolume(Settings::Get().Snapshot().volumeMusic);
  ReachAudio::PlayMenuMusic(Settings::Get().Snapshot().menuMusicTrack);

  int startTab = 0;
  std::string recommendedDownload; // test-only: "-recommended-download <id>"
                                   // starts a download at menu start
  {
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i + 1 < argc; ++i) {
      if (std::wstring(argv[i]) == L"-menu-screen")
        ctx.st.screen = ParseScreenArg(argv[i + 1]);
      if (std::wstring(argv[i]) == L"-menu-capture")
        ctx.captureFrame = _wtoi64(argv[i + 1]);
      if (std::wstring(argv[i]) == L"-menu-tab")
        startTab = _wtoi(argv[i + 1]);
      if (std::wstring(argv[i]) == L"-recommended-download") {
        recommendedDownload.clear();
        for (const wchar_t *c = argv[i + 1]; c && *c; ++c)
          recommendedDownload.push_back(static_cast<char>(*c));
      }
    }
    if (argv)
      LocalFree(argv);
  }
  // Recommended maps (MODS > RECOMMENDED MAPS). The install folder is the one
  // that holds Noble.exe; the marker files decide whether an item already shows
  // as installed.
  {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    RecommendedMaps::Get().Initialize(
        std::filesystem::path(exePath).parent_path());
    if (!recommendedDownload.empty())
      RecommendedMaps::Get().StartDownload(recommendedDownload);
  }
  // Test-only: "-lobby-demo host|guest [-lobby-demo-players N] [-lobby-demo-red
  // N] [-lobby-demo-ffa 1] [-lobby-demo-mode campaign|firefight|forge]" opens
  // the pre-game lobby with a made-up roster and no network, so a capture can
  // show every state on one PC.
  {
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::wstring demo, demoMode, demoMap, demoSession;
    int demoPlayers = 1;
    int demoRed = -1;
    int demoTeams = 0;
    bool demoFfa = false;
    for (int i = 1; argv && i + 1 < argc; ++i) {
      if (_wcsicmp(argv[i], L"-lobby-demo") == 0)
        demo = Lower(argv[i + 1]);
      if (_wcsicmp(argv[i], L"-lobby-demo-session") == 0)
        demoSession = argv[i + 1];
      if (_wcsicmp(argv[i], L"-lobby-demo-players") == 0)
        demoPlayers = std::clamp(_wtoi(argv[i + 1]), 0, 128);
      if (_wcsicmp(argv[i], L"-lobby-demo-mode") == 0)
        demoMode = Lower(argv[i + 1]);
      if (_wcsicmp(argv[i], L"-lobby-demo-map") == 0)
        demoMap = argv[i + 1];
      if (_wcsicmp(argv[i], L"-lobby-demo-red") == 0)
        demoRed = _wtoi(argv[i + 1]); // that many red, the rest blue
      if (_wcsicmp(argv[i], L"-lobby-demo-teams") == 0)
        demoTeams = std::clamp(_wtoi(argv[i + 1]), 2, kTeamCount);
      if (_wcsicmp(argv[i], L"-lobby-demo-ffa") == 0)
        demoFfa = _wtoi(argv[i + 1]) != 0;
    }
    if (argv)
      LocalFree(argv);
    if (!demo.empty()) {
      MenuState &st = ctx.st;
      st.screen = Screen::PreGameLobby;
      st.isLobbyHost = demo == L"host";
      st.lobbyHostEndpoint = (st.isLobbyHost || demoPlayers == 0) ? "" : "demo";
      // Test-only: a made-up but valid 16-hex session id so the host's SHARE CODE panel can be captured offline.
      if (st.isLobbyHost && demoSession.size() == 16)
        st.lobbySessionId.assign(demoSession.begin(), demoSession.end());
      if (demoMode == L"campaign") {
        st.lobbyMode = "campaign";
        st.lobbyMap = kMissions[0].scenario;
        st.lobbyMaxPlayers = 16;
      } else if (demoMode == L"firefight") {
        st.lobbyMode = "firefight";
        st.lobbyMap = kFirefightMaps[0].scenario;
        st.lobbyVariantName = "Firefight";
      } else if (demoMode == L"forge") {
        st.lobbyMode = "forge";
      }
      if (const wchar_t *line =
              wcsstr(GetCommandLineW(), L"-lobby-demo-unknown-mode")) {
        (void)line;
        st.lobbyVariantName = "Someone's Own Mode";
      }
      if (demoTeams > 0)
        st.lobbyTeamCount = demoTeams;
      st.lobbyMaxPlayers = std::max(st.lobbyMaxPlayers, demoPlayers);
      if (!st.isLobbyHost && demoPlayers > 0)
        st.lobbyRosterSelection = demoPlayers > 1 ? 1 : 0;
      for (int i = 0; i < demoPlayers; ++i) {
        LobbyPlayer p;
        p.peerId = static_cast<uint64_t>(i + 1);
        p.isHost = i == 0;
        p.username = i == 0 ? "Noble Six" : "Spartan " + std::to_string(i + 1);
        p.serviceTag = "N" + std::to_string(i + 10);
        if (i > 0) { // other players carry their own rank, emblem, background
                     // and nameplate, as the roster does
          p.rankIndex = i % 30;
          p.emblemId = (i * 7) % 60 + 1;
          p.emblemBackgroundId = (i * 3) % 40 + 1;
          p.nameplateId = i % 20 + 1;
        }
        p.team = demoFfa           ? 0
                 : demoTeams > 0   ? (i % demoTeams) + 1
                 : demoRed >= 0    ? (i < demoRed ? 1 : 2)
                 : demoPlayers > 2 ? (i % 2) + 1
                                   : 0;
        st.lobbyRoster.push_back(p);
      }
      if (!demoMap.empty()) {
        std::string scenario;
        for (const wchar_t c : demoMap) {
          if (c < 128)
            scenario.push_back(
                static_cast<char>(::tolower(static_cast<unsigned char>(c))));
        }
        const ModEntry *foundMod = nullptr;
        const ModMap *foundMap = nullptr;
        int foundKind = -1;
        for (const ModEntry &mod : ctx.mods) {
          for (int kind = 0; kind < 3 && !foundMap; ++kind) {
            const std::vector<ModMap> &list = kind == 0   ? mod.campaign
                                              : kind == 1 ? mod.firefight
                                                          : mod.multiplayer;
            for (const ModMap &m : list) {
              if (m.scenario == scenario && m.hasMapFile) {
                foundMod = &mod;
                foundMap = &m;
                foundKind = kind;
                break;
              }
            }
          }
          if (foundMap)
            break;
        }
        if (foundMap) {
          st.lobbyMode = foundKind == 0         ? "campaign"
                         : foundKind == 1       ? "firefight"
                         : demoMode == L"forge" ? "forge"
                                                : "multiplayer";
          const std::string carrier = foundKind == 0   ? "m10"
                                      : foundKind == 1 ? "ff60_icecave"
                                                       : "70_boneyard";
          st.lobbyMap = CarrierForScenario(foundMap->scenario, carrier);
          st.lobbyMapOverride =
              foundMod->folder / "maps" / (foundMap->scenario + ".map");
          st.lobbyMapTitle = foundMap->title;
          st.lobbyMapImage = foundMap->imageLarge.empty()
                                 ? foundMap->imageThumbnail
                                 : foundMap->imageLarge;
          st.lobbyMapThumb = foundMap->imageThumbnail;
          st.lobbyModFolder = foundMod->folder;
          st.lobbyModId = foundMod->workshopId;
          st.lobbyModTitle = foundMod->title;
          NOBLE_INFO("FrontEndMenu: -lobby-demo-map {} -> lobby '{}' via '{}'",
                     scenario, foundMap->title, st.lobbyMap);
        } else {
          NOBLE_WARN(
              "FrontEndMenu: -lobby-demo-map {} not found in any installed mod",
              scenario);
        }
      }
      NOBLE_INFO("FrontEndMenu: -lobby-demo host={} players={} mode={}",
                 st.isLobbyHost, demoPlayers, st.lobbyMode);
    }
  }
  // Test-only: open the "Workshop mod required" dialog on a fake missing item
  // so -menu-capture can photograph it without a second machine hosting a
  // modded lobby.
  {
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i < argc; ++i) {
      if (_wcsicmp(argv[i], L"-mod-prompt-demo") == 0) {
        ModDownloadPrompt::Get().Show("9999999999", "CUSTOM MAP PACK", {},
                                      true);
      }
      // Test-only: a real Workshop id, so the Steam Web API picture / download
      // size can be captured.
      if (_wcsicmp(argv[i], L"-mod-prompt-id") == 0 && i + 1 < argc) {
        std::string id;
        for (const wchar_t *c = argv[i + 1]; c && *c; ++c)
          id.push_back(static_cast<char>(*c));
        ModDownloadPrompt::Get().Show(id, "Workshop mod", {}, false);
      }
    }
    if (argv)
      LocalFree(argv);
  }
  // Test-only: "-open-mods" expands every Workshop mod heading, so
  // -auto-play-map / -menu-capture can reach the modded map rows without a
  // click.
  {
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool openAll = false;
    for (int i = 1; argv && i < argc; ++i) {
      if (_wcsicmp(argv[i], L"-open-mods") == 0)
        openAll = true;
    }
    if (argv)
      LocalFree(argv);
    if (openAll) {
      for (const ModEntry &mod : ctx.mods) {
        ctx.st.openMods.insert(OpenKey(ModMode::Campaign, mod));
        ctx.st.openMods.insert(OpenKey(ModMode::Firefight, mod));
        ctx.st.openMods.insert(OpenKey(ModMode::Multiplayer, mod));
      }
      NOBLE_INFO("FrontEndMenu: -open-mods expanded {} mod heading(s)",
                 ctx.mods.size());
    }
  }
  ctx.settingsPanel.SetTab(startTab);
  ctx.careerPanel.SetTab(startTab);
  // -menu-tab also picks the VANILLA / MODDED source tab of the setup screens,
  // so a test run can capture the modded lists (long workshop names and
  // descriptions) without clicking.
  switch (ctx.st.screen) {
  case Screen::Campaign:
    ctx.st.campaignSource = startTab ? 1 : 0;
    break;
  case Screen::Firefight:
    ctx.st.ffSource = startTab ? 1 : 0;
    break;
  case Screen::CustomGame:
    ctx.st.mpSource = startTab ? 1 : 0;
    break;
  case Screen::Forge:
    ctx.st.forgeSource = startTab ? 1 : 0;
    break;
  case Screen::Theater:
    ctx.st.theaterSource = std::clamp(startTab, 0, 3);
    break;
  default:
    break;
  }
  ctx.modsScanned = false;
  ctx.totalFrames = 0;
  ctx.sceneReadyFrames = 0;
  ctx.closed = false;
  ctx.initialized = true;

  // Menu background: pick and upload the texture here, on the engine thread
  // that owns the device. The fallback draw path (HoldFrame, main thread) must
  // never create textures - it used to hang there.
  UiArt::WarmMenuBackground();

  if (ctx.st.screen == Screen::Armory) {
    UIManager::GetArmorCustomizer().SyncToReachArmory();
  }

  // Server Browser Join Pre-Game Lobby Handler. A lobby whose map comes from a
  // Steam Workshop mod is checked BEFORE joining: a player without the item
  // gets Download & Join / Cancel, and only starts the lobby join once the mod
  // is in the Workshop cache. (The old behaviour walked straight into a lobby
  // whose map the guest could not load.)
  auto enterLobby = [&ctx](const ServerEntry &s, const std::string &modId,
                           const std::string &modTitle,
                           const std::filesystem::path &modFolder) {
    EnterLobby(ctx, s, modId, modTitle, modFolder);
  };

  RefreshPlaylists(ctx);
  // "-lobby-return": a match that just ended came back to its lobby, in this
  // new process. Each match is its own process, so the lobby's lasting part is
  // the server's session: the host registers the same address and port again,
  // which keeps the session id and roster, and guests join it again once the
  // host is back (PauseMenu's post-game panel).
  {
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::map<std::wstring, std::wstring> back;
    for (int i = 1; argv && i + 1 < argc; ++i) {
      if (_wcsnicmp(argv[i], L"-lobby-", 7) == 0)
        back[Lower(argv[i])] = argv[i + 1];
    }
    if (argv)
      LocalFree(argv);
    const auto text = [&](const wchar_t *key) {
      const auto it = back.find(key);
      if (it == back.end())
        return std::string();
      char narrow[1024] = {};
      WideCharToMultiByte(CP_UTF8, 0, it->second.c_str(), -1, narrow,
                          sizeof(narrow) - 1, nullptr, nullptr);
      return std::string(narrow);
    };
    const auto path = [&](const wchar_t *key) {
      const auto it = back.find(key);
      return it == back.end() ? std::filesystem::path()
                              : std::filesystem::path(it->second);
    };
    const std::string role = text(L"-lobby-return");
    if (!role.empty()) {
      const std::filesystem::path modFolder = path(L"-lobby-mod-folder");
      NOBLE_INFO("FrontEndMenu: back from a match as {} of session '{}'", role,
                 text(L"-lobby-session"));
      if (role == "host") {
        const std::filesystem::path variant = path(L"-lobby-variant");
        std::string varName = text(L"-lobby-variant-name");
        for (const auto *list : {&ctx.variants, &ctx.ffVariants}) {
          for (const MenuVariant &v : *list)
            if (v.path == variant)
              varName = v.label;
        }
        const ModEntry *mod = nullptr;
        for (const ModEntry &m : ctx.mods)
          if (!modFolder.empty() && m.folder == modFolder)
            mod = &m;
        ctx.st.lobbyNetMode = text(L"-lobby-net") == "lan" ? 0 : 1;
        ctx.st.lobbyPrivacy = text(L"-lobby-privacy") == "private" ? 1 : 0;
        const std::string maxPlayers = text(L"-lobby-max-players");
        const std::string difficulty = text(L"-lobby-difficulty");
        const std::string skulls = text(L"-lobby-skulls");
        SetupHostLobby(
            ctx.st, text(L"-lobby-map"), text(L"-lobby-mode"), variant.string(),
            varName, difficulty.empty() ? 1 : std::atoi(difficulty.c_str()),
            skulls.empty() ? 0 : std::strtoull(skulls.c_str(), nullptr, 10),
            maxPlayers.empty() ? 16 : std::atoi(maxPlayers.c_str()), mod,
            path(L"-lobby-map-override"), text(L"-lobby-map-title"),
            path(L"-lobby-map-image"), path(L"-lobby-map-thumb"),
            text(L"-lobby-session"), text(L"-lobby-session-token"));
        // The playlist carries on: NEXT IN ROTATION moves to its next entry,
        // RETURN TO LOBBY stays on this one.
        const std::filesystem::path listFile = path(L"-lobby-playlist");
        for (size_t i = 0; i < ctx.st.playlists.size(); ++i) {
          if (!listFile.empty() && ctx.st.playlists[i].file == listFile)
            ctx.st.playlistChoice = static_cast<int>(i);
        }
        if (ctx.st.playlistChoice >= 0) {
          const Playlists::Playlist &playlist =
              ctx.st.playlists[ctx.st.playlistChoice];
          const std::string position = text(L"-lobby-playlist-pos");
          int at = position.empty() ? 0 : std::atoi(position.c_str());
          if (text(L"-lobby-advance") == "1" && playlist.autoStart > 0)
            ctx.st.autoStartAt =
                playlist.autoStart + 2.0; // ImGui time starts at 0 here
          if (text(L"-lobby-advance") == "1" && !playlist.entries.empty()) {
            const int n = static_cast<int>(playlist.entries.size());
            if (playlist.shuffle && n > 1) {
              const int was = at;
              while (at == was)
                at = static_cast<int>(std::random_device{}() % n);
            } else {
              at = (at + 1) % n;
            }
            ResolvedEntry resolved;
            if (ResolveEntry(ctx, playlist.entries[at], playlist.randomType,
                             resolved))
              ApplyEntryToLobby(ctx.st, resolved);
          }
          ctx.st.playlistPosition = at;
        }
      } else {
        ServerEntry s;
        s.id = text(L"-lobby-session");
        const std::string endpoint = text(L"-lobby-endpoint");
        const size_t colon = endpoint.rfind(':');
        s.hostAddress = endpoint.substr(0, colon);
        s.port = static_cast<uint16_t>(
            colon == std::string::npos
                ? 11774
                : std::atoi(endpoint.c_str() + colon + 1));
        s.mapName = text(L"-lobby-map");
        s.gameMode = text(L"-lobby-mode");
        s.maxPlayers = 16;
        enterLobby(s, std::string(), std::string(), modFolder);
      }
    }
  }

  UIManager::GetServerBrowser().SetDirectJoinHandler(
      [](const std::string &map, const std::string &mode,
         const std::string &addr, const std::string &sessId) {
        std::lock_guard<std::recursive_mutex> lock(s_overlayMutex);
        OverlayContext &ov = GetOverlayCtx();
        if (ov.matchRunning || s_launchInFlight.load()) {
          NOBLE_WARN("FrontEndMenu: direct join ignored while a match or "
                     "launch is already active");
          return;
        }
        ov.pendingJoinChoice = FrontEndChoice{};
        ov.pendingJoinChoice.play = true;
        ov.pendingJoinChoice.host = false;
        ov.pendingJoinChoice.map = map.empty() ? "20_sword_slayer" : map;
        ov.pendingJoinChoice.mode = mode.empty() ? "multiplayer" : mode;
        ov.pendingJoinChoice.connectAddress = addr;
        ov.pendingJoinChoice.sessionId = sessId;
        ov.hasPendingJoin = true;
        NOBLE_INFO("FrontEndMenu: queued container direct join -> {}:{} ({})",
                   ov.pendingJoinChoice.map, ov.pendingJoinChoice.mode, addr);
      });

  UIManager::GetServerBrowser().SetJoinLobbyHandler(
      [&ctx, enterLobby](const ServerEntry &s) {
        // Version gate (belt and braces: the browser already refuses mismatched
        // builds).
        if (!s.build.empty() &&
            !noble::AreBuildsCompatible(s.build, noble::kBuildTag)) {
          UIManager::GetServerBrowser().SetStatus(
              "Can't join: version mismatch (host " +
              std::string(noble::ExtractBaseVersion(s.build)) + ", you " +
              std::string(noble::ExtractBaseVersion(noble::kBuildTag)) + ").");
          return;
        }
        std::string modId = s.modId;
        std::string modTitle = s.modTitle;
        if (!s.id.empty() && MasterServerClient::Get().Enabled()) {
          const LobbyState peek =
              MasterServerClient::Get().FetchLobbyStateSync(s.id);
          // Engine limits (D3): host and guests must run the same engine build
          // and AI actor count, or the match desyncs. Hosts that advertise
          // "limits_id" are checked; older hosts (no field) are let in.
          const uint32_t mine = noble::limits::LocalNetworkId();
          if (peek.ok && peek.limitsId >= 0 &&
              static_cast<uint32_t>(peek.limitsId) != mine) {
            const auto describe = [](uint32_t id) {
              const auto profile =
                  noble::limits::ProfileFromInt(static_cast<int>(id >> 24));
              const uint32_t actors = id & 0xFFFF;
              return std::string(noble::limits::ProfileName(profile)) +
                     (actors ? ", AI actors " + std::to_string(actors)
                             : std::string());
            };
            const std::string msg =
                "Can't join: this game uses engine limits " +
                describe(static_cast<uint32_t>(peek.limitsId)) + "; you have " +
                describe(mine) +
                ". Change SETTINGS > ENGINE to match and restart Noble.";
            NOBLE_WARN("ServerBrowser join refused: {}", msg);
            UIManager::GetServerBrowser().SetStatus(msg);
            return;
          }
          // The list refreshes every two seconds; if the host picked its mod
          // after the last refresh, use the lobby endpoint's answer before
          // letting the player in.
          if (modId.empty() && s.isMod && peek.ok) {
            modId = peek.modId;
            modTitle = peek.modTitle;
          }
        }

        std::filesystem::path modFolder;
        if (!modId.empty()) {
          if (const ModEntry *installed =
                  ModCatalog::FindByWorkshopId(ctx.mods, modId)) {
            modFolder = installed->folder;
            if (modTitle.empty())
              modTitle = installed->title;
          }
          if (modFolder.empty())
            modFolder = SteamWorkshop::FindInstalledFolder(modId);
          if (modFolder.empty()) {
            ModDownloadPrompt::Get().Show(
                modId, modTitle,
                [&ctx, s, enterLobby, modId,
                 modTitle](const std::filesystem::path &downloaded) {
                  RefreshModCatalog(ctx); // the new item belongs in the setup
                                          // screens and the Mods list
                  enterLobby(s, modId, modTitle, downloaded);
                });
            return;
          }
        }
        enterLobby(s, modId, modTitle, modFolder);
      });

  // Real-time WebSocket Lobby Callbacks
  // These run on the WebSocket relay thread: they only queue the event (see
  // PendingLobbyEvents).
  NetworkTransport::Get().SetLobbyLaunchCallback(
      [](const std::string &lMap, const std::string &lMode,
         const std::string &lVar, int lDiff, uint64_t lSkulls,
         const std::string &hIp, uint16_t hPort, const std::string &sId,
         const std::string &lMapOverride, const std::string &lModFolder,
         const std::string &lModId, const std::string &lMapVariant) {
        NOBLE_INFO("FrontEndMenu: Received WebSocket real-time lobby launch "
                   "event! (map: {}, mode: {}, mod: {}, mvar: {})",
                   lMap, lMode, lModId, lMapVariant);
        fe::EnqueueLobbyLaunch(lMap, lMode, lVar, lDiff, lSkulls, hIp, hPort,
                               sId, lMapOverride, lModFolder, lModId,
                               lMapVariant);
      });

  NetworkTransport::Get().SetLobbyClosedCallback([](const std::string &reason) {
    NOBLE_INFO("FrontEndMenu: Received lobby_closed event! (reason: {})",
               reason);
    fe::EnqueueLobbyClosed(reason);
  });

  NetworkTransport::Get().SetLobbyRosterCallback(
      [](const std::string &rosterJson) {
        try {
          auto j = nlohmann::json::parse(rosterJson);
          if (j.contains("roster") && j["roster"].is_array()) {
            std::vector<LobbyPlayer> updated;
            for (const auto &item : j["roster"]) {
              LobbyPlayer p;
              p.peerId = item.value("peerId", 1ULL);
              p.machineId = item.value("machineId", "");
              p.username = item.value("username", "Spartan");
              p.serviceTag = item.value("serviceTag", "117");
              p.rankIndex = item.value("rankIndex", -1);
              p.emblemId = item.value("emblemId", 0);
              p.emblemBackgroundId = item.value("emblemBackgroundId", 0);
              p.nameplateId = item.value("nameplateId", 0);
              p.team = item.value("team", 0);
              p.isHost = item.value("isHost", false);
              p.isDev = item.value("isDev", false);
              p.isTester = item.value("isTester", false);
              p.ping = item.value("ping", 0);
              try {
                const uint64_t machine =
                    p.machineId.empty() ? 0
                                        : std::stoull(p.machineId, nullptr, 0);
                if (machine != 0)
                  SocialOverlay::SetPlayerName(machine, p.username);
                if (machine != 0 &&
                    machine == NetworkTransport::Get().GetRelayMachineId()) {
                  VoiceChat::SetLocalTeam(p.team);
                }
              } catch (...) {
              }
              updated.push_back(p);
            }
            fe::EnqueueLobbyRoster(std::move(updated));
          }
        } catch (...) {
        }
      });

  UpdateSceneForScreen(ctx.st.screen);

  NOBLE_INFO("FrontEndMenu: initialized in-engine 3D menu overlay ({} game "
             "variants, {} Firefight variants)",
             ctx.variants.size(), ctx.ffVariants.size());
}

void FrontEndMenu::RenderOverlay(IDXGISwapChain *swapChain) {
  if (!swapChain || IsMatchRunning())
    return;
  std::lock_guard<std::recursive_mutex> lock(s_overlayMutex);
  if (IsMatchRunning())
    return;
  s_overlaySwapChain.store(swapChain);
  s_lastEngineOverlayFrame.store(GetTickCount64());
  if (s_holdFrameOwned.exchange(false)) {
    s_resetFrameTimer.store(true);
    NOBLE_INFO(
        "FrontEndMenu: engine resumed drawing the menu; yielding HoldFrame");
  }
  RenderOverlayImpl(swapChain, false);
}

bool FrontEndMenu::HoldFrame() {
  if (IsMatchRunning())
    return false;
  std::lock_guard<std::recursive_mutex> lock(s_overlayMutex);
  if (IsMatchRunning())
    return false;
  OverlayContext &ctx = GetOverlayCtx();
  if (!ctx.initialized || ctx.closed || !ReachArmory::IsMenuMode())
    return false;
  const ULONGLONG now = GetTickCount64();
  const ULONGLONG last = s_lastEngineOverlayFrame.load();
  IDXGISwapChain *swapChain = s_overlaySwapChain.load();
  if (!swapChain)
    return false;
  if (!s_holdFrameOwned.load()) {
    if (last == 0 || now - last < 1200)
      return false;
    s_holdFrameOwned.store(true);
    s_resetFrameTimer.store(true);
    NOBLE_INFO(
        "FrontEndMenu: engine stopped drawing the menu; Noble draws it itself");
  }
  RenderOverlayImpl(swapChain, true);
  FrameCapture::OnFrame(nullptr, 0); // keeps -quit-after working in test runs
  const ULONGLONG t0 = GetTickCount64();
  const HRESULT hr = swapChain->Present(1, 0);
  const ULONGLONG elapsed = GetTickCount64() - t0;
  // DXGI_STATUS_OCCLUDED (or immediate Present during window transitions) does
  // not block on VSync; pace to ~60 FPS so HoldFrame never spins at 1000+ FPS.
  if (hr == DXGI_STATUS_OCCLUDED || elapsed < 10) {
    Sleep(16);
  }
  return true;
}

void FrontEndMenu::RenderOverlayImpl(IDXGISwapChain *swapChain, bool clear) {
  if (IsMatchRunning())
    return;
  OverlayContext &ctx = GetOverlayCtx();
  if (!ctx.initialized || ctx.closed || !swapChain ||
      !ImGui::GetCurrentContext())
    return;

  DXGI_SWAP_CHAIN_DESC desc{};
  if (FAILED(swapChain->GetDesc(&desc)) || !desc.OutputWindow)
    return;
  const HWND hwnd = desc.OutputWindow;

  ID3D11Device *device = nullptr;
  if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device),
                                  reinterpret_cast<void **>(&device))) ||
      !device)
    return;
  ID3D11DeviceContext *context = nullptr;
  device->GetImmediateContext(&context);

  ID3D11Texture2D *backBuffer = nullptr;
  ID3D11RenderTargetView *rtv = nullptr;
  if (SUCCEEDED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                     reinterpret_cast<void **>(&backBuffer))) &&
      backBuffer) {
    device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
  }

  if (context && rtv) {
    D3D11_TEXTURE2D_DESC bbDesc{};
    backBuffer->GetDesc(&bbDesc);
    ArmoryStage::s_screenAspect = bbDesc.Height
                                      ? static_cast<double>(bbDesc.Width) /
                                            static_cast<double>(bbDesc.Height)
                                      : 0.0;

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.DisplaySize = ImVec2(static_cast<float>(bbDesc.Width),
                            static_cast<float>(bbDesc.Height));

    static LARGE_INTEGER s_freq{}, s_last{};
    LARGE_INTEGER now{};
    QueryPerformanceCounter(&now);
    if (s_freq.QuadPart == 0)
      QueryPerformanceFrequency(&s_freq);
    const bool resetTimer = s_resetFrameTimer.exchange(false);
    float dt = (!resetTimer && s_last.QuadPart)
                   ? static_cast<float>(now.QuadPart - s_last.QuadPart) /
                         static_cast<float>(s_freq.QuadPart)
                   : 1.0f / 60.0f;
    if (dt <= 0.0f || dt > 0.25f)
      dt = 1.0f / 60.0f;
    s_last = now;
    io.DeltaTime = dt;

    ArmoryStage::s_frameRtv = clear ? nullptr : rtv;
    if (clear) {
      // No engine frame underneath: start from the menu's dark backdrop colour.
      const float black[4] = {0.02f, 0.03f, 0.05f, 1.0f};
      context->ClearRenderTargetView(rtv, black);
    }

    ImGui_ImplDX11_NewFrame();
    // Do not call ImGui_ImplWin32_NewFrame() here: it overwrites io.DisplaySize
    // with the unscaled window client rect and queues an unscaled client-space
    // mouse position via ImGui_ImplWin32_UpdateMouseData, which causes the
    // cursor/hover position to flicker upward when supersampling (1.5x) is
    // active.
    io.DisplaySize = ImVec2(static_cast<float>(bbDesc.Width),
                            static_cast<float>(bbDesc.Height));
    {
      RECT client{};
      GetClientRect(hwnd, &client);
      ArmoryStage::s_windowScale = client.bottom > 0
                                       ? static_cast<float>(bbDesc.Height) /
                                             static_cast<float>(client.bottom)
                                       : 1.0f;
      static LONG s_lastClientW = -1, s_lastClientH = -1;
      if (client.right != s_lastClientW || client.bottom != s_lastClientH) {
        s_lastClientW = client.right;
        s_lastClientH = client.bottom;
        if (client.right > 0 && client.bottom > 0 &&
            (client.right != static_cast<LONG>(bbDesc.Width) ||
             client.bottom != static_cast<LONG>(bbDesc.Height))) {
          NOBLE_INFO("FrontEndMenu: window client {}x{}, menu back buffer "
                     "{}x{} ({:.2f}x supersample)",
                     client.right, client.bottom, bbDesc.Width, bbDesc.Height,
                     ArmoryStage::s_windowScale);
        }
      }
    }
    UiPad::Feed(io);
    FeedEngineThreadInputs(hwnd, io);
    static const bool s_socialDemo =
        wcsstr(GetCommandLineW(), L"-social-demo") != nullptr;
    const bool chatScreenOk =
        (ctx.st.screen == Screen::PreGameLobby) || s_socialDemo;
    SocialOverlay::Poll(chatScreenOk && !io.WantTextInput &&
                        !ctx.st.confirmQuit);
    // Text drawn by plain ImGui widgets (search boxes, descriptions, help
    // lines, input fields) used the fonts' fixed 15-18 px and turned tiny on a
    // native 4K back buffer. Scale it like every custom-drawn label
    // (back-buffer height / 900). Fonts are rasterised at 2.5x density, so this
    // stays crisp.
    io.FontGlobalScale = NobleUi::Scale();
    ImGui::NewFrame();
    if (SocialOverlay::IsTyping())
      io.ClearInputKeys(); // the line takes the keyboard, not the screen under
                           // it
    InputPrompts::Update();
    // Master mute / mute in background: Noble's own audio only (menu music and
    // menu sounds). Read twice a second rather than every frame, and applied
    // here so it also covers the pause menu and the fallback draw path.
    {
      static ULONGLONG s_nextMuteCheck = 0;
      const ULONGLONG muteNow = GetTickCount64();
      if (muteNow >= s_nextMuteCheck) {
        s_nextMuteCheck = muteNow + 500;
        const SettingsData audio = Settings::Get().Snapshot();
        bool muted = audio.masterMute || g_menuMusicMuted;
        bool bgMuted = false;
        if (!muted && audio.muteInBackground) {
          // "Active" means the foreground window belongs to this process: that
          // covers the menu window and the engine's, without having to know
          // which one is up. IsForeground() also counts the launcher window
          // that hosts a container-mode match, so a match running inside it is
          // not muted as if Noble were in the background.
          const HWND foreground = GetForegroundWindow();
          DWORD ownerPid = 0;
          if (foreground)
            GetWindowThreadProcessId(foreground, &ownerPid);
          bgMuted = foreground != nullptr &&
                    ownerPid != GetCurrentProcessId() &&
                    !Window::IsForeground(hwnd);
          if (bgMuted)
            muted = true;
        }
        ReachAudio::SetMuted(muted);
        // MASTER MUTE / menu-music mute are menu-only; muteInBackground mutes
        // the game mix itself.
        ReachHost::SetRuntimeVolumes(bgMuted ? 0.0f : audio.volumeGame,
                                     bgMuted ? 0.0f : audio.volumeMusic,
                                     bgMuted ? 0.0f : audio.volumeOther);
      }
    }

    FrontEndChoice choice;
    bool done = false;
    try {
      DrawFrontEndContents(ctx, device, context, choice, done);
      if (chatScreenOk) {
        SocialOverlay::Draw(
            8.0f, 196.0f,
            900.0f); // voice above the title, chat over the Friends panel
      }
    } catch (const std::exception &e) {
      NOBLE_ERROR("FrontEndMenu: exception in menu loop: {}", e.what());
    } catch (...) {
      NOBLE_ERROR("FrontEndMenu: unknown exception in menu loop");
    }

    UiPad::DrawVirtualCursor(ImGui::GetForegroundDrawList());
    ImGui::Render();

    ID3D11RenderTargetView *oldRtv = nullptr;
    ID3D11DepthStencilView *oldDsv = nullptr;
    context->OMGetRenderTargets(1, &oldRtv, &oldDsv);
    context->OMSetRenderTargets(1, &rtv, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    context->OMSetRenderTargets(1, &oldRtv, oldDsv);
    if (oldRtv)
      oldRtv->Release();
    if (oldDsv)
      oldDsv->Release();

    ++ctx.totalFrames;
    if (ReachArmory::IsSceneReady()) {
      ++ctx.sceneReadyFrames;
    }

    if (ctx.captureFrame > 0) {
      const bool readyToCapture = (ctx.sceneReadyFrames >= ctx.captureFrame) ||
                                  (ctx.totalFrames >= ctx.captureFrame + 480);
      if (readyToCapture) {
        FrameCapture::SaveNow(swapChain, ctx.totalFrames);
        NOBLE_INFO("FrontEndMenu: -menu-capture done at frame {} "
                   "(sceneReadyFrames={}), closing",
                   ctx.totalFrames, ctx.sceneReadyFrames);
        ctx.closed = true;
        ReachAudio::SetMenuMusicEnabled(false);
        ReachAudio::StopMusic();
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
      }
    }

    if (done && !ctx.closed) {
      ctx.closed = true;
      ReachAudio::SetMenuMusicEnabled(false);
      ReachAudio::StopMusic();
      if (choice.play) {
        // Remember what was launched, for the \"Continue\" row on the main menu
        // (spec 4.1). A lobby join (connectAddress) is not a local game, so it
        // does not overwrite this. CONTINUE is a campaign feature (MCC's
        // Continue = the campaign mission you were on), so only a campaign
        // launch updates it; Firefight, Forge, multiplayer and lobby joins
        // leave it alone.
        if (choice.connectAddress.empty() && choice.savedFilm.empty() &&
            !choice.map.empty() && choice.mode == "campaign") {
          SettingsData d = Settings::Get().Snapshot();
          d.lastPlayedMode = choice.mode;
          d.lastPlayedMap = choice.map;
          d.lastPlayedVariant = choice.variantPath;
          d.lastPlayedDifficulty = choice.difficulty;
          d.lastPlayedSkulls = choice.skulls;
          d.lastPlayedMod = choice.modFolder.empty()
                                ? std::string()
                                : choice.modFolder.string();
          Settings::Get().Update(d);
        }
        std::wstring args = L"-map \"" + ToWide(choice.map) + L"\" -mode \"" +
                            ToWide(choice.mode) + L"\"";
        // Forward the test-only capture hooks, so a harness that opened the
        // menu can still photograph the match (the child is a fresh process
        // with its own command line).
        for (const wchar_t *flag :
             {L"-pause-at", L"-scoreboard-at", L"-scoreboard-demo",
              L"-scoreboard-demo-players", L"-postgame-role", L"-pad-script",
              L"-quit-after", L"-fake-pad", L"-fake-pad-amount",
              L"-capture-at"}) {
          int argc = 0;
          LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
          for (int a = 1; argv && a + 1 < argc; ++a) {
            if (_wcsicmp(argv[a], flag) == 0) {
              args += L" ";
              args += flag;
              args += L" ";
              args += argv[a + 1];
              break;
            }
          }
          if (argv)
            LocalFree(argv);
        }
        // The lobby's team assignment: the match applies each player's team in
        // the engine at spawn
        // (`player_override_team`, 0-based). Without this the engine dealt its
        // own teams and a host who picked purple was red in the match.
        bool anyTeam = false;
        for (const auto &player : ctx.st.lobbyRoster)
          anyTeam = anyTeam || player.team > 0;
        if (anyTeam && !ctx.st.lobbyRoster.empty() &&
            choice.mode != "campaign" && choice.mode != "firefight") {
          std::wstring teams;
          for (const auto &player : ctx.st.lobbyRoster) {
            if (!teams.empty())
              teams += L",";
            teams += std::to_wstring(std::clamp(player.team, 1, kTeamCount));
          }
          args += L" -lobby-teams \"" + teams + L"\"";
        }
        args += L" -difficulty " + std::to_wstring(choice.difficulty);
        // The match process only keeps this for the way back: post-game's NEXT
        // IN ROTATION.
        if (choice.host && ctx.st.playlistChoice >= 0 &&
            ctx.st.playlistChoice < static_cast<int>(ctx.st.playlists.size())) {
          args += L" -lobby-playlist \"" +
                  ctx.st.playlists[ctx.st.playlistChoice].file.wstring() +
                  L"\"";
          args += L" -lobby-playlist-pos " +
                  std::to_wstring(ctx.st.playlistPosition);
          if (ctx.st.playlists[ctx.st.playlistChoice].autoStart > 0)
            args += L" -lobby-playlist-auto " +
                    std::to_wstring(
                        ctx.st.playlists[ctx.st.playlistChoice].autoStart);
        }
        if (!choice.variantPath.empty())
          args += L" -variant \"" + ToWide(choice.variantPath) + L"\"";
        if (choice.skulls != 0)
          args += L" -skulls " + std::to_wstring(choice.skulls);
        if (!choice.modFolder.empty())
          args += L" -mod \"" + choice.modFolder.wstring() + L"\"";
        if (!choice.mapOverride.empty())
          args += L" -map-override \"" + choice.mapOverride.wstring() + L"\"";
        if (!choice.mapVariant.empty())
          args += L" -mvar \"" + choice.mapVariant.wstring() + L"\"";
        if (!choice.savedFilm.empty())
          args += L" -film \"" + choice.savedFilm.wstring() + L"\"";
        ctx.launchedFromTheater = !choice.savedFilm.empty();
        if (noble::LiveDumpSeconds() > 0)
          args +=
              L" -mvar-live-dump " + std::to_wstring(noble::LiveDumpSeconds());
        if (noble::PageScanSeconds() > 0)
          args +=
              L" -mvar-page-scan " + std::to_wstring(noble::PageScanSeconds());
        if (!choice.mapTitle.empty())
          args += L" -map-title \"" + ToWide(choice.mapTitle) + L"\"";
        if (!choice.mapImage.empty())
          args += L" -map-image \"" + choice.mapImage.wstring() + L"\"";
        if (!choice.mapThumb.empty())
          args += L" -map-thumb \"" + choice.mapThumb.wstring() + L"\"";
        if (choice.host) {
          args += L" -host";
          if (choice.hostOnline)
            args += L" -online";
          else
            args += L" -lan";
          if (choice.hostPublic)
            args += L" -public";
          else
            args += L" -private";
          args += L" -max-players " + std::to_wstring(choice.hostMaxPlayers);
          if (choice.expectedPeers > 0) {
            args += L" -wait-peers " + std::to_wstring(choice.expectedPeers);
          }
          // Loading screen session block: let the match process show the lobby roster (host first).
          if (!choice.rosterNames.empty()) {
            std::wstring rosterCsv;
            for (size_t r = 0; r < choice.rosterNames.size(); ++r) {
              if (r) rosterCsv += L"|";
              rosterCsv += ToWide(choice.rosterNames[r]);
            }
            args += L" -loading-roster \"" + rosterCsv + L"\"";
          }
          if (!choice.sessionId.empty()) {
            args += L" -session-id \"" + ToWide(choice.sessionId) + L"\"";
            if (!choice.sessionToken.empty()) {
              args +=
                  L" -session-token \"" + ToWide(choice.sessionToken) + L"\"";
            }
          }
        } else if (!choice.connectAddress.empty()) {
          // A guest launched from the in-engine menu (pre-game lobby): the
          // match process joins the host. Without this the relaunch started an
          // unconnected local match (main.cpp's in-process path for the
          // Noble-drawn menu reads choice.connectAddress itself).
          args += L" -connect \"" + ToWide(choice.connectAddress) + L"\"";
          if (!choice.sessionId.empty())
            args += L" -session-id \"" + ToWide(choice.sessionId) + L"\"";
          if (!choice.sessionToken.empty())
            args += L" -session-token \"" + ToWide(choice.sessionToken) + L"\"";
        }
        NOBLE_INFO("FrontEndMenu: launching match inside container window "
                   "0x{:X} ({} {})",
                   reinterpret_cast<uintptr_t>(hwnd), choice.mode, choice.map);
        if (StartContainerChild(args, ctx)) {
          done = false;
          choice.play = false;
          ReachAudio::StopMusic();
          ReachAudio::SetMenuMusicEnabled(false);
        } else {
          ctx.closed = false;
          done = false;
          choice.play = false;
        }
      } else {
        NOBLE_INFO("FrontEndMenu: quit from menu");
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
      }
    }
  }

  if (rtv)
    rtv->Release();
  if (backBuffer)
    backBuffer->Release();
  if (context)
    context->Release();
  device->Release();
}

FrontEndChoice FrontEndMenu::Run(Window &window, D3D11Renderer &renderer,
                                 const std::filesystem::path &reachDir) {
  Initialize(reachDir);
  OverlayContext &ctx = GetOverlayCtx();
  FrontEndChoice choice;
  bool done = false;

  window.SetCustomWndProc([](HWND h, UINT m, WPARAM w, LPARAM l) {
    return UIManager::HandleWndProc(h, m, w, l);
  });

  UIManager::GetServerBrowser().SetDirectJoinHandler(
      [&choice, &ctx, &done](const std::string &map, const std::string &mode,
                             const std::string &addr,
                             const std::string &sessId) {
        choice.play = true;
        choice.host = false;
        choice.map = map;
        choice.mode = mode;
        choice.connectAddress = addr;
        choice.sessionId = sessId;
        if (mode == "forge" && std::filesystem::exists(ctx.forgeVariant)) {
          choice.variantPath = ctx.forgeVariant.string();
        }
        done = true;
      });

  while (!done && window.ProcessMessages()) {
    renderer.BeginFrame();
    UIManager::BeginFrame();
    ++ctx.totalFrames;

    DrawFrontEndContents(ctx, renderer.GetDevice(), renderer.GetContext(),
                         choice, done);

    UIManager::EndFrame();
    if (ctx.captureFrame && ctx.totalFrames == ctx.captureFrame) {
      FrameCapture::SaveNow(renderer.GetSwapChain(), ctx.totalFrames);
      choice.play = false;
      done = true;
    }
    renderer.EndFrame(true);
  }

  ReachAudio::StopMusic();
  UIManager::GetServerBrowser().SetDirectJoinHandler(nullptr);
  UIManager::GetServerBrowser().SetJoinLobbyHandler(nullptr);
  window.SetCustomWndProc(nullptr);
  return choice;
}

} // namespace noble
