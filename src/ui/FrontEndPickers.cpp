#include "FrontEndInternal.hpp"

namespace noble::fe {

static const char *GameTypeBlurb(const std::string &type) {
  if (type == "slayer")
    return "Score points by killing the other players.";
  if (type == "ctf")
    return "Take the enemy flag back to your own base.";
  if (type == "assault")
    return "Carry the bomb into the enemy base and arm it.";
  if (type == "territories")
    return "Capture and hold the areas of the map.";
  if (type == "koth")
    return "Hold the hill to score.";
  if (type == "oddball")
    return "Hold the skull for as long as you can.";
  if (type == "juggernaut")
    return "Kill the Juggernaut to become the Juggernaut.";
  if (type == "infection")
    return "The infected hunt the survivors, and every kill turns another.";
  if (type == "headhunter")
    return "Collect skulls from kills and bank them at the drop points.";
  if (type == "stockpile")
    return "Gather flags and hold them at your base when the timer runs out.";
  if (type == "invasion")
    return "Spartans against Elites, fought in phases across the map.";
  if (type == "race")
    return "Drive through the checkpoints.";
  if (type == "rally")
    return "Reach the rally points before the others.";
  if (type == "firefight")
    return "Fight wave after wave of Covenant together.";
  return "A variant with its own rules.";
}

// The game types a map can host, grouped by mode. A heading drops down its
// variants, one mode at a time. Searching shows every match as a plain list,
// and a single mode (Firefight) needs no headings.
static std::string VariantMode(const MenuVariant &v) {
  // Put a player-made mode under the game type it starts from, not in one
  // "custom" heap. VIP and Action Sack variants therefore get their own
  // headings like the shipped types.
  if (!v.type.empty())
    return MapGameTypes::Name(v.type);
  return v.custom ? std::string("Custom Game Modes") : std::string("Other");
}

static VariantChoices BuildVariantChoices(const std::vector<MenuVariant> &list,
                                          const std::string &scenario,
                                          bool unrestricted,
                                          const std::string &openMode,
                                          const char *query) {
  std::vector<const MenuVariant *> offered;
  for (const MenuVariant &v : list) {
    if (unrestricted || v.type.empty() ||
        MapGameTypes::Supports(scenario, v.type))
      offered.push_back(&v);
  }
  std::stable_sort(offered.begin(), offered.end(),
                   [](const MenuVariant *a, const MenuVariant *b) {
                     const std::string ma = VariantMode(*a),
                                       mb = VariantMode(*b);
                     return ma != mb ? ma < mb : a->label < b->label;
                   });
  VariantChoices out;
  const bool searching = query[0] != 0;
  const bool oneMode = !offered.empty() && VariantMode(*offered.front()) ==
                                               VariantMode(*offered.back());
  std::string heading;
  for (const MenuVariant *v : offered) {
    const std::string mode = VariantMode(*v);
    if (searching) {
      if (!ContainsNoCase(v->label, query) && !ContainsNoCase(mode, query))
        continue;
    } else if (!oneMode && mode != heading) {
      heading = mode;
      const auto count = std::count_if(
          offered.begin(), offered.end(),
          [&](const MenuVariant *o) { return VariantMode(*o) == mode; });
      out.rows.push_back({nullptr, mode, v->type});
      out.items.push_back({mode + "  (" + std::to_string(count) + ")", true,
                           true, mode == openMode});
    }
    if (searching || oneMode || mode == openMode) {
      out.rows.push_back({v, mode, v->type});
      out.items.push_back({v->label});
    }
  }
  return out;
}

// Opens a game type list on the current variant: its mode dropped down and its
// row selected.
void BeginVariantList(MenuState &st, const std::vector<MenuVariant> &list,
                      const std::string &scenario, bool unrestricted,
                      const MenuVariant *current) {
  st.variantSearch[0] = 0;
  st.variantOpenMode = current ? VariantMode(*current) : std::string();
  st.variantPickerRow = 0;
  const VariantChoices choices = BuildVariantChoices(
      list, scenario, unrestricted, st.variantOpenMode, st.variantSearch);
  for (size_t i = 0; i < choices.rows.size(); ++i) {
    if (choices.rows[i].variant == current)
      st.variantPickerRow = static_cast<int>(i);
  }
}

// Search box and list. A heading toggles; returns the variant chosen, if any.
// `shown` gets the selected row.
const MenuVariant *DrawVariantList(const Ui &ui, MenuState &st,
                                   const std::vector<MenuVariant> &list,
                                   const std::string &scenario,
                                   bool unrestricted, ImVec2 pos, float width,
                                   float rowHeight, int rowCount, bool keyboard,
                                   VariantRow &shown) {
  if (DrawSearchBox(ui, "##variant_search", pos, width, st.variantSearch,
                    sizeof(st.variantSearch), "Search game types"))
    st.variantPickerRow = 0;
  const ImVec2 listPos(pos.x, pos.y + 40.0f * ui.s);
  VariantChoices choices = BuildVariantChoices(
      list, scenario, unrestricted, st.variantOpenMode, st.variantSearch);
  if (choices.items.empty())
    choices.items.push_back({"No game type matches", false, false});
  const int hit = DrawList(
      ui, "variant_list", listPos, width, choices.items, st.variantPickerRow,
      keyboard && !ImGui::GetIO().WantTextInput, rowHeight, rowCount, true);
  const MenuVariant *picked = nullptr;
  if (hit >= 0 && hit < static_cast<int>(choices.rows.size())) {
    if (choices.rows[hit].variant) {
      picked = choices.rows[hit].variant;
    } else {
      // Rows shift when another mode closes, so the selection follows the
      // heading that was pressed.
      const std::string mode = choices.rows[hit].mode;
      st.variantOpenMode = st.variantOpenMode == mode ? std::string() : mode;
      choices = BuildVariantChoices(list, scenario, unrestricted,
                                    st.variantOpenMode, st.variantSearch);
      for (size_t i = 0; i < choices.rows.size(); ++i) {
        if (!choices.rows[i].variant && choices.rows[i].mode == mode)
          st.variantPickerRow = static_cast<int>(i);
      }
      MenuSounds::Click();
    }
  }
  shown =
      choices.rows.empty()
          ? VariantRow{}
          : choices.rows[std::clamp(st.variantPickerRow, 0,
                                    static_cast<int>(choices.rows.size()) - 1)];
  return picked;
}

// Name and description of a list row; the file name and a note about rules too,
// unless `compact` (the lobby's tight column).
void DrawVariantInfo(const Ui &ui, ImVec2 pos, float width,
                     const VariantRow &row, bool compact) {
  if (row.mode.empty())
    return;
  const float right = pos.x + width;
  const MenuVariant *variant = row.variant;
  Text(ui, NobleStyle::TitleFont(), 24.0f, pos, kWhite,
       Upper(variant ? variant->label : row.mode).c_str());
  const char *description = variant && !variant->description.empty()
                                ? variant->description.c_str()
                                : GameTypeBlurb(row.type);
  Text(ui, nullptr, 16.0f, ImVec2(pos.x, pos.y + 34.0f * ui.s),
       IM_COL32(205, 218, 230, 255), description, right);
  if (compact || !variant)
    return;
  const MenuVariant &v = *variant;
  const float textH =
      Font(nullptr)->CalcTextSizeA(16.0f * ui.s, FLT_MAX, width, description).y;
  const float y = pos.y + 34.0f * ui.s + textH + 10.0f * ui.s;
  Text(ui, NobleStyle::SmallFont(), 14.0f, ImVec2(pos.x, y), kDim,
       v.path.filename().string().c_str(), right);
  Text(ui, NobleStyle::SmallFont(), 14.0f, ImVec2(pos.x, y + 20.0f * ui.s),
       kDim, "Uses this variant's default rules.", right);
}

std::string RuleValueText(const VariantRules::Rule &rule, int value) {
  const int nameIndex = value - rule.minValue;
  if (rule.names && nameIndex >= 0 &&
      nameIndex < static_cast<int>(rule.names->size()))
    return (*rule.names)[nameIndex];
  if (rule.bit >= 0)
    return value ? "On" : "Off";
  return std::to_string(value) + rule.unit;
}

// GAME OPTIONS: options that are fixed levels rather than numbers. In Reach's
// trait lists the first choice ("Default", value = the rule's minimum) means
// "unchanged": the player keeps the game's normal level. `normal` names that
// level, so the row reads "OFF" or "ALLIES" instead of "DEFAULT"; nullptr keeps
// the list's own word where the normal level isn't certain. `levels` explains
// what each level does and is shown with the option's description.
struct RuleLevels {
  const char *key;
  const char *normal;
  const char *levels;
};

static const RuleLevels *FindRuleLevels(const char *key) {
  static const RuleLevels s_levels[] = {
      {"headshot_immune", "Off",
       "Off: headshots do their usual extra damage. On: they do no extra "
       "damage."},
      {"assassin_immune", "Off",
       "Off: players can be assassinated from behind. On: they can't."},
      {"primary_weapon", "Loadout",
       "Loadout: the game type's own spawn weapon. None: spawn without one."},
      {"secondary_weapon", "Loadout",
       "Loadout: the game type's own second weapon. None: spawn without one."},
      {"ability", "Loadout",
       "Loadout: the game type's own armor ability. None: spawn without one."},
      {"grenade_count", "Loadout",
       "Loadout: whatever the game type gives. Map Default: the map's own "
       "grenades. "
       "Otherwise None, or that many frag and/or plasma grenades at spawn."},
      {"weapon_pickup", "On",
       "On: players can pick up and swap weapons. Off: they keep what they "
       "spawned with."},
      {"infinite_ammo", "Off",
       "Off: normal ammo. On: reserve ammo never runs out, but you still "
       "reload. "
       "Bottomless: the magazine never empties, so there are no reloads."},
      {"ability_usage", nullptr,
       "Off: armor abilities can't be used. Unlimited: they never run out of "
       "energy. "
       "Normal and On both allow them; the game's exact difference between the "
       "two isn't confirmed yet."},
      {"active_camo", "Off",
       "Off: fully visible. Worst: barely see-through. Poor: partly "
       "see-through, still easy to spot. "
       "Good: very hard to see. Best: almost invisible."},
      {"shield_vampirism", "Off",
       "Off: the usual. On: shield damage you deal is added to your own "
       "shields."},
      {"grenade_regen", "Off",
       "Off: the usual. On: players get new grenades over time. Works in "
       "custom games; "
       "Firefight doesn't apply it yet."},
      {"aura", "None",
       "None: no glow. Team Color: a glow in the team's main color. Darkened "
       "and Pastel: the player's own "
       "armor colors, darker or lighter."},
      {"waypoint", "Allies",
       "None: nobody sees a marker over the player. Allies: teammates see it. "
       "All: everyone sees it, enemies too."},
      {"visible_name", "Allies",
       "None: nobody sees the player's name. Allies: teammates see it. All: "
       "everyone sees it, enemies too."},
      {"tracker_mode", "Normal",
       "Off: no motion tracker. Friends: it only shows allies. Normal: it "
       "shows enemies who move "
       "quickly or fire. All: it shows every enemy, even one standing still."},
      {"tracker_range", "25 m", nullptr},
  };
  for (const RuleLevels &l : s_levels)
    if (std::strcmp(l.key, key) == 0)
      return &l;
  return nullptr;
}

// The value as the GAME OPTIONS row shows it: numbers for numeric rules
// (RuleValueText), the level's own name for level rules (with "unchanged"
// spelled as the game's normal level).
static std::string OptionValueText(const VariantRules::Rule &rule, int value) {
  const RuleLevels *levels = FindRuleLevels(rule.key);
  if (levels && levels->normal && value == rule.minValue && rule.names &&
      !rule.names->empty() && (*rule.names)[0] == "Default")
    return levels->normal;
  return RuleValueText(rule, value);
}

// The option's description plus, for level options, what the levels mean.
static std::string OptionHelpText(const VariantRules::Rule &rule) {
  std::string help = rule.help;
  if (const RuleLevels *levels = FindRuleLevels(rule.key);
      levels && levels->levels)
    help += std::string(" ") + levels->levels;
  return help;
}

// GAME OPTIONS quick presets: each one replaces the current changes with its
// own list, which the player can then tweak and SAVE. Values are raw rule
// values (see the rule tables in VariantRules.cpp: weapons -1 None, 0 DMR, 4
// Energy Sword, 5 Magnum, 8 Rocket Launcher, 10 Sniper Rifle; shield_mult 1 =
// 0%; tracker_mode 1 = Off; grenade_count 2 = None; weapon_pickup 2 = Off;
// infinite_ammo 2 = On). Only the base traits every player shares can be set,
// so there is no Infection-style preset: it needs separate zombie traits.
struct OptionsPreset {
  const char *name;
  const char *help;
  std::vector<std::pair<const char *, int>> values;
};

static const std::vector<OptionsPreset> &OptionsPresets() {
  static const std::vector<OptionsPreset> s_presets = {
      {"CLASSIC", "The base game type's own rules, with nothing changed.", {}},
      {"SWAT",
       "No shields, no motion tracker, no grenades. Spawn with a DMR and a "
       "Magnum.",
       {{"shield_mult", 1},
        {"tracker_mode", 1},
        {"grenade_count", 2},
        {"primary_weapon", 0},
        {"secondary_weapon", 5}}},
      {"SNIPERS",
       "Spawn with a Sniper Rifle and a Magnum, infinite ammo, no motion "
       "tracker, no grenades.",
       {{"primary_weapon", 10},
        {"secondary_weapon", 5},
        {"infinite_ammo", 2},
        {"tracker_mode", 1},
        {"grenade_count", 2}}},
      {"ROCKETS",
       "Spawn with a Rocket Launcher and a Magnum, infinite ammo, no grenades.",
       {{"primary_weapon", 8},
        {"secondary_weapon", 5},
        {"infinite_ammo", 2},
        {"grenade_count", 2}}},
      {"SWORDS",
       "Energy Sword only: no second weapon, no grenades, no weapon pickups.",
       {{"primary_weapon", 4},
        {"secondary_weapon", -1},
        {"grenade_count", 2},
        {"weapon_pickup", 2}}},
  };
  return s_presets;
}

// Reads the lobby's variant when it changes. The Teams flag is in the decoded
// record, or in a player-made mode's rules.

void OpenGameOptions(OverlayContext &ctx, const MenuVariant &current) {
  MenuState &st = ctx.st;
  VariantRules::Preset preset;
  const bool isMode =
      current.custom && VariantRules::Load(current.path, preset);
  st.optionsBase = isMode ? preset.base : current.path.filename().string();
  st.optionsSource =
      isMode ? VariantBasePath(ctx.reachDir, preset.base) : current.path;
  st.optionsValues = isMode ? preset.values : std::map<std::string, int>{};
  std::snprintf(st.optionsName, sizeof(st.optionsName), "%s",
                (isMode ? current.label : current.label + " Custom").c_str());
  st.optionsFile = isMode ? current.path : std::filesystem::path();
  st.optionsRow = 2; // the first option (row 0 is the presets, row 1 the first
                     // category heading)
  st.optionsMessage.clear();
  st.optionsSearch[0] = 0;
  st.optionsChangedOnly = false;
  st.optionsOpenGroups.clear();
  if (!VariantRules::Rules().empty())
    st.optionsOpenGroups.insert(
        VariantRules::Rules().front().group); // only the first opens
  st.optionsFirst = 0;
  st.optionsLastFocus = -1;
  st.optionsPresetPick = 0;
  st.confirmAction.clear();
  if (VariantRules::Decode(st.optionsSource, st.optionsDecoded)) {
    NOBLE_INFO("FrontEndMenu: game options opened on '{}'", st.optionsBase);
    st.optionsOpen = true;
    MenuSounds::Click();
  } else {
    NOBLE_WARN("FrontEndMenu: could not read variant '{}' for its game options",
               st.optionsBase);
    MenuSounds::Error();
  }
}

// ---- Playlists ----
void RefreshPlaylists(OverlayContext &ctx) {
  std::vector<std::filesystem::path> folders;
  for (const ModEntry &mod : ctx.mods)
    folders.push_back(mod.folder);
  ctx.st.playlists = Playlists::Scan(folders);
}

// Every multiplayer map a playlist can name: the stock ones, then each mod's.
std::vector<MapEntry> AllMultiplayerMaps(OverlayContext &ctx) {
  std::vector<MapEntry> maps = BaseMaps(kMaps, Count(kMaps));
  std::set<std::string> everyMod;
  for (const ModEntry *mod : ctx.mpMods)
    everyMod.insert(OpenKey(ModMode::Multiplayer, *mod));
  for (MapEntry &m : ModdedMaps(ctx.mpMods, ModMode::Multiplayer, kMaps,
                                Count(kMaps), everyMod, "70_boneyard")) {
    if (!m.header && m.enabled)
      maps.push_back(std::move(m));
  }
  return maps;
}

// A playlist entry turned into what the lobby needs. False when this PC lacks
// the map.
bool ResolveEntry(OverlayContext &ctx, const Playlists::Entry &entry,
                  bool randomType, ResolvedEntry &out) {
  bool found = false;
  for (MapEntry &m : AllMultiplayerMaps(ctx)) {
    const std::string mod =
        m.mod ? m.mod->folder.filename().string() : std::string();
    if (m.scenario == entry.map && mod == entry.mod) {
      out.map = std::move(m);
      found = true;
      break;
    }
  }
  if (!found)
    return false;
  std::vector<const MenuVariant *> allowed;
  out.variant = nullptr;
  for (const MenuVariant &v : ctx.variants) {
    if (!entry.mod.empty() || v.type.empty() ||
        MapGameTypes::Supports(entry.map, v.type))
      allowed.push_back(&v);
    if (v.path.filename().string() == entry.variant)
      out.variant = &v;
  }
  static std::mt19937 s_random{std::random_device{}()};
  if ((randomType || !out.variant) && !allowed.empty())
    out.variant = allowed[s_random() % allowed.size()];
  return out.variant != nullptr;
}

void ApplyEntryToLobby(MenuState &st, const ResolvedEntry &e) {
  st.lobbyMap =
      e.map.launchScenario.empty() ? e.map.scenario : e.map.launchScenario;
  st.lobbyMapOverride = e.map.mapOverride;
  st.lobbyMapVariant = e.map.mvar;
  st.lobbyMapTitle = e.map.name;
  st.lobbyMapImage = e.map.imageLoading;
  st.lobbyMapThumb = e.map.imageThumbnail;
  st.lobbyModId = e.map.mod ? e.map.mod->workshopId : std::string();
  st.lobbyModTitle = e.map.mod ? e.map.mod->title : std::string();
  st.lobbyModFolder = e.map.mod ? e.map.mod->folder : std::filesystem::path();
  st.lobbyVariantPath = e.variant->path.string();
  st.lobbyVariantName = e.variant->label;
  MasterServerClient::Get().UpdateLobbyAdvert(
      st.lobbyMap, st.lobbyMode, st.lobbyVariantName, st.lobbyDifficulty,
      st.lobbySkulls);
  MasterServerClient::Get().UpdateLobbyModAdvert(
      !st.lobbyModId.empty(), st.lobbyModId, st.lobbyModTitle);
  NOBLE_INFO("FrontEndMenu: lobby now {} on {}", st.lobbyVariantName,
             st.lobbyMapTitle);
}

// Opens a vote with up to three different entries from the active playlist.

struct MapCardMeta {
  std::string players;
  std::string modes;
  std::string terrain;
};

static MapCardMeta GetMapCardMeta(const MapEntry *entry,
                                  const char *fallbackModeLabel) {
  if (!entry)
    return {"1-16 Players",
            fallbackModeLabel ? fallbackModeLabel : "CUSTOM GAME",
            "REACH / COMBAT ZONE"};
  if (entry->mod) {
    const std::string carrier =
        entry->launchScenario.empty() ? entry->scenario : entry->launchScenario;
    return {entry->custom ? "Custom Map" : "Modded Map",
            "MOD: " + Upper(entry->mod->title),
            carrier.empty()
                ? "CUSTOM ENVIRONMENT"
                : ("CARRIER: " + Upper(MapNames::Display(carrier)))};
  }
  struct StockMeta {
    const char *scenario;
    const char *players;
    const char *modes;
    const char *terrain;
  };
  static const StockMeta kMeta[] = {
      {"20_sword_slayer", "8-16 Players", "SLAYER, CTF, SWAT, ODDBALL",
       "INDUSTRIAL / VERTICAL"},
      {"45_launch_station", "8-16 Players", "SLAYER, CTF, ASSAULT, TERRITORIES",
       "ORBITAL LAUNCH / SYMMETRICAL"},
      {"50_panopticon", "8-16 Players", "SLAYER, CTF, KOTH, ASSAULT",
       "URBAN WATERFRONT / ASYMMETRICAL"},
      {"70_boneyard", "12-16 Players", "INVASION, BTB SLAYER, CTF",
       "SHIPYARD COAST / VEHICLE BTB"},
      {"30_settlement", "6-12 Players", "SLAYER, KOTH, TERRITORIES, STOCKPILE",
       "HYDROELECTRIC / MIXED COMBAT"},
      {"52_ivory_tower", "6-12 Players", "SLAYER, SWAT, INFECTION, JUGGERNAUT",
       "HIGH-RISE ATRIUM / VERTICAL"},
      {"35_island", "12-16 Players", "INVASION, BTB SLAYER, HEADHUNTER",
       "COVENANT SPIRE / AERIAL BTB"},
      {"45_aftship", "4-8 Players", "SLAYER, SWAT, ARENA, GRIFBALL",
       "CORVETTE INTERIOR / ZERO-G"},
      {"dlc_slayer", "4-10 Players", "SLAYER, CTF, TEAM OBJECTIVE",
       "ORBITAL DRYDOCK / LOW-G"},
      {"dlc_invasion", "12-16 Players", "INVASION, BTB SLAYER, STOCKPILE",
       "ARCTIC EXCAVATION / LARGE BTB"},
      {"dlc_medium", "8-16 Players", "BTB SLAYER, CTF, TERRITORIES",
       "FORERUNNER COAST / VEHICLES"},
      {"condemned", "6-12 Players", "SLAYER, CTF, KOTH, INFECTION",
       "ORBITAL STATION / CORRIDORS"},
      {"trainingpreserve", "12-16 Players", "BTB SLAYER, CTF, STOCKPILE",
       "MILITARY PRESERVE / VEHICLE BTB"},
      {"cex_beaver_creek", "4-8 Players", "SLAYER, CTF, ODDBALL",
       "CANYON BASIN / SYMMETRICAL"},
      {"cex_damnation", "4-8 Players", "SLAYER, KOTH, TEAM OBJECTIVE",
       "FORERUNNER TEMPLE / MULTI-LEVEL"},
      {"cex_timberland", "12-16 Players", "BTB SLAYER, CTF, ASSAULT",
       "FOREST HIGHLANDS / CLASSIC BTB"},
      {"cex_prisoner", "2-8 Players", "SLAYER, SWAT, TEAM ARENA",
       "CELLBLOCK ATRIUM / VERTICAL"},
      {"cex_hangemhigh", "4-10 Players", "SLAYER, CTF, TEAM SNIPERS",
       "TOMB COURTYARD / SIGHTLINES"},
      {"cex_headlong", "12-16 Players", "BTB SLAYER, CTF, 1-BOMB, INFECTION",
       "CITY SECTOR / URBAN BTB"},
      {"forge_halo", "2-16+ Players", "CUSTOM, FORGE, RACE, INFECTION",
       "HALO RING / SANDBOX CANVAS"},
      // Campaign missions
      {"m05", "1-4 Co-op", "CAMPAIGN / CINEMATIC",
       "VIERY TERRITORY / PROLOGUE"},
      {"m10", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "FRONTIER / RECONNAISSANCE"},
      {"m20", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "ONI SWORD BASE / DEFENSE"},
      {"m30", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "VIERY HIGHLANDS / STEALTH"},
      {"m35", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "COVENANT FRONT / ASSAULT"},
      {"m45", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "ORBITAL / SABRE & CORVETTE"},
      {"m50", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "NEW ALEXANDRIA / EVACUATION"},
      {"m52", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "OCCUPIED CITY / FALCON OPS"},
      {"m60", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "GLACIER COMPLEX / SIEGE"},
      {"m70", "1-4 Co-op", "CAMPAIGN, SCORING, LASO",
       "ASZOD SHIPYARDS / FINALE"},
      {"m70_bonus", "1-4 Co-op", "CAMPAIGN / SURVIVAL FINALE",
       "SHIPYARD SURFACE / LAST STAND"},
      // Firefight maps
      {"ff50_park", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "BEACHHEAD COAST / VEHICLES"},
      {"ff45_corvette", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "CORVETTE HANGAR / CLOSE QUARTERS"},
      {"ff20_courtyard", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "SWORD BASE COURTYARD / VERTICAL"},
      {"ff60_icecave", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "FROZEN GLACIER / MIXED COMBAT"},
      {"ff70_holdout", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "BUNKER COMPLEX / FORTIFIED"},
      {"ff60_airview", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "HIGHLAND OUTPOST / WIDE SIGHTLINES"},
      {"ff10_prototype", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "VIERY FARMLAND / ELEVATED RIDGE"},
      {"ff30_waterfront", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "INDUSTRIAL DOCKS / MULTI-LEVEL"},
      {"ff_unearthed", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "MINING FACILITY / VEHICLE YARD"},
      {"cex_ff_halo", "1-4+ Co-op", "FIREFIGHT, GENERATOR, SCORE ATTACK",
       "ALPHA HALO / FORERUNNER RUINS"},
  };
  for (const StockMeta &m : kMeta) {
    if (entry->scenario == m.scenario)
      return {m.players, m.modes, m.terrain};
  }
  return {"2-16 Players", fallbackModeLabel ? fallbackModeLabel : "CUSTOM GAME",
          "REACH / COMBAT ZONE"};
}

static UiTexture ResolveMapTileTexture(const MapEntry &m, bool highRes) {
  UiTexture tex;
  if (m.mod) {
    if (highRes && !m.imageLarge.empty())
      tex = UiArt::ImageFile(m.imageLarge);
    if (!tex && !m.imageThumbnail.empty())
      tex = UiArt::ImageFile(m.imageThumbnail);
    if (!tex && !m.imageLoading.empty())
      tex = UiArt::ImageFile(m.imageLoading);
    if (!tex && !m.scenario.empty())
      tex = UiArt::MapPreview(m.scenario, true);
    if (!tex && !m.scenario.empty())
      tex = UiArt::MapPhoto(m.scenario);
    if (!tex && !m.launchScenario.empty())
      tex = UiArt::MapPreview(m.launchScenario, true);
  } else {
    tex = UiArt::MapPreview(m.scenario, true);
    if (!tex)
      tex = UiArt::MapPhoto(m.scenario);
    if (!tex && highRes)
      tex = UiArt::LoadingBackground(m.scenario);
  }
  return tex;
}

std::vector<MapTileGroup> BuildMapTileGroups(
    OverlayContext &ctx, bool modded, bool forgeTab, ModMode modMode,
    const std::vector<const ModEntry *> &modList, const MenuMap *baseMaps,
    int baseCount, const std::string &carrier, bool includeForgeVariants,
    const char *searchQuery) {
  if (includeForgeVariants && !ctx.forgeMapsScanned) {
    ctx.forgeMaps = ScanForgeMaps();
    ctx.forgeMapsScanned = true;
  }
  const std::string query = searchQuery ? searchQuery : "";
  auto makeVariantEntry = [](const MapEntry &parent, const ForgeMap &fm) {
    MapEntry v = parent;
    v.name = fm.title;
    if (!fm.description.empty())
      v.blurb = fm.description;
    else if (!fm.creator.empty())
      v.blurb = "Map variant by " + fm.creator + " (on " + parent.name + ").";
    else
      v.blurb = "Map variant on " + parent.name + ".";
    v.mvar = fm.path;
    v.forgeGroup = fm.group;
    return v;
  };

  std::vector<MapTileGroup> groups;
  if (modded) {
    for (const ModEntry *mod : modList) {
      if (!mod)
        continue;
      const std::vector<ModMap> &listed =
          (modMode == ModMode::Campaign)    ? mod->campaign
          : (modMode == ModMode::Firefight) ? mod->firefight
                                            : mod->multiplayer;
      std::vector<MapEntry> maps =
          ModMaps(*mod, listed, baseMaps, baseCount, carrier);
      if (maps.empty())
        continue;
      // A mod is one collapsed section until the player opens it (or types a search): its maps used to be all on
      // screen at once, which made a 10-mod campaign list hundreds of tiles long. The section banner shows the mod
      // title and how many maps it contains; clicking it expands the section.
      const bool open =
          !query.empty() || ctx.st.openMods.count(OpenKey(modMode, *mod)) != 0;
      if (!open) {
        MapTileGroup section;
        section.key = "modsection:" + OpenKey(modMode, *mod);
        MapEntry banner = maps.front();
        banner.name = mod->title;
        banner.blurb = PlainDescription(mod->description, 180);
        banner.header = true;
        banner.enabled = false;
        banner.mod = mod;
        section.base = std::move(banner);
        section.sectionMapCount = static_cast<int>(maps.size());
        groups.push_back(std::move(section));
        continue;
      }
      bool firstInMod = true;
      for (MapEntry &m : maps) {
        if (m.header)
          continue;
        std::vector<MapEntry> vars;
        if (includeForgeVariants) {
          const std::string matchScen =
              m.launchScenario.empty() ? m.scenario : m.launchScenario;
          for (const ForgeMap &fm : ctx.forgeMaps) {
            if (fm.scenario == matchScen)
              vars.push_back(makeVariantEntry(m, fm));
          }
          std::sort(vars.begin(), vars.end(),
                    [](const MapEntry &a, const MapEntry &b) {
                      return a.name < b.name;
                    });
        }
        const bool baseMatch = query.empty() || ContainsNoCase(m.name, query) ||
                               ContainsNoCase(mod->title, query) ||
                               ContainsNoCase(m.scenario, query);
        std::vector<MapEntry> filteredVars;
        if (!query.empty() && !baseMatch) {
          for (const MapEntry &v : vars) {
            if (ContainsNoCase(v.name, query) ||
                ContainsNoCase(v.forgeGroup, query))
              filteredVars.push_back(v);
          }
          if (filteredVars.empty())
            continue;
        } else {
          filteredVars = std::move(vars);
        }
        MapTileGroup g;
        g.key = "mod:" + mod->id + ":" + m.scenario + ":" +
                m.mapOverride.filename().string();
        g.base = std::move(m);
        g.variants = std::move(filteredVars);
        if (firstInMod) {
          g.groupHeading = mod->title;
          firstInMod = false;
        }
        groups.push_back(std::move(g));
      }
    }
    return groups;
  }

  if (forgeTab) {
    // Group all saved .mvar Forge variants under their actual canvas/base map
    // tiles!
    for (int i = 0; i < baseCount; ++i) {
      MapEntry base{baseMaps[i].scenario,
                    baseMaps[i].name,
                    baseMaps[i].blurb,
                    {},
                    {},
                    {},
                    {},
                    {},
                    true};
      std::vector<MapEntry> vars;
      for (const ForgeMap &fm : ctx.forgeMaps) {
        if (fm.scenario == base.scenario)
          vars.push_back(makeVariantEntry(base, fm));
      }
      if (vars.empty())
        continue;
      std::sort(vars.begin(), vars.end(),
                [](const MapEntry &a, const MapEntry &b) {
                  if (a.forgeGroup != b.forgeGroup)
                    return a.forgeGroup < b.forgeGroup;
                  return a.name < b.name;
                });
      const bool baseMatch = query.empty() || ContainsNoCase(base.name, query);
      if (!baseMatch) {
        std::vector<MapEntry> kept;
        for (const MapEntry &v : vars) {
          if (ContainsNoCase(v.name, query) ||
              ContainsNoCase(v.forgeGroup, query))
            kept.push_back(v);
        }
        if (kept.empty())
          continue;
        vars = std::move(kept);
      }
      MapTileGroup g;
      g.key = "forge:" + base.scenario;
      g.base = std::move(base);
      g.variants = std::move(vars);
      groups.push_back(std::move(g));
    }
    // Put Forge World first in Forge Maps tab so it is immediately
    // front-and-center.
    std::stable_sort(groups.begin(), groups.end(),
                     [](const MapTileGroup &a, const MapTileGroup &b) {
                       const bool aw = a.base.scenario == "forge_halo";
                       const bool bw = b.base.scenario == "forge_halo";
                       if (aw != bw)
                         return aw;
                       return a.variants.size() > b.variants.size();
                     });
    return groups;
  }

  for (int i = 0; i < baseCount; ++i) {
    MapEntry base{baseMaps[i].scenario,
                  baseMaps[i].name,
                  baseMaps[i].blurb,
                  {},
                  {},
                  {},
                  {},
                  {},
                  true};
    std::vector<MapEntry> vars;
    if (includeForgeVariants) {
      for (const ForgeMap &fm : ctx.forgeMaps) {
        if (fm.scenario == base.scenario)
          vars.push_back(makeVariantEntry(base, fm));
      }
      std::sort(
          vars.begin(), vars.end(),
          [](const MapEntry &a, const MapEntry &b) { return a.name < b.name; });
    }
    const bool baseMatch = query.empty() || ContainsNoCase(base.name, query) ||
                           ContainsNoCase(base.scenario, query);
    if (!baseMatch) {
      std::vector<MapEntry> kept;
      for (const MapEntry &v : vars) {
        if (ContainsNoCase(v.name, query) ||
            ContainsNoCase(v.forgeGroup, query))
          kept.push_back(v);
      }
      if (kept.empty())
        continue;
      vars = std::move(kept);
    }
    MapTileGroup g;
    g.key = "stock:" + base.scenario;
    g.base = std::move(base);
    g.variants = std::move(vars);
    groups.push_back(std::move(g));
  }
  return groups;
}

TacticalGridResult
DrawTacticalMapGrid(const Ui &ui, MenuState &st, ImVec2 gridPos,
                    ImVec2 gridSize, const std::vector<MapTileGroup> &groups,
                    int &selectedGroupIdx, std::filesystem::path &selectedMvar,
                    bool hasFocus, const char *emptyText,
                    const char *baseItemTag, bool autoScrollSelection,
                    ModMode openMode) {
  TacticalGridResult res;
  const ImVec2 gridEnd(gridPos.x + gridSize.x, gridPos.y + gridSize.y);

  // Outer tactical framed container matching the reference image
  ui.dl->AddRectFilled(gridPos, gridEnd, IM_COL32(8, 13, 20, 170), 2.0f * ui.s);
  NobleUi::DrawTacticalFrame(ui.dl, gridPos, gridEnd, ui.s, false, false, false,
                             false);

  if (groups.empty()) {
    selectedGroupIdx = 0;
    st.mapDrawerKey.clear();
    st.mapDrawerFocus = -1;
    const float tw =
        TextWidth(NobleStyle::TitleFont(), 19.0f * ui.s, emptyText);
    Text(ui, NobleStyle::TitleFont(), 19.0f,
         ImVec2(gridPos.x + (gridSize.x - tw) * 0.5f,
                gridPos.y + gridSize.y * 0.44f),
         kDim, emptyText);
    return res;
  }

  selectedGroupIdx =
      std::clamp(selectedGroupIdx, 0, static_cast<int>(groups.size()) - 1);
  const float crispMax = std::max(1.0f, ImGui::GetIO().DisplaySize.y / 1080.0f);
  const bool typing = ImGui::GetIO().WantTextInput;
  const bool mouseMoved = ImGui::GetIO().MouseDelta.x != 0.0f ||
                          ImGui::GetIO().MouseDelta.y != 0.0f;

  // Build visual 2-column row plan (mod headings start a fresh row cleanly,
  // expanded drawers attach right under their row)
  struct VisualRow {
    std::string heading;
    int col[2] = {-1, -1};
    int expandedCol =
        -1; // 0 or 1 if one of the 2 tiles in this row has its drawer open
    bool section = false; // full-width collapsed mod banner row
    float y = 0.0f;
    float tilesH = 0.0f;
    float drawerH = 0.0f;
    int drawerVisibleRows = 0;
  };

  const float pad = 14.0f * ui.s;
  const float colGap = 14.0f * ui.s;
  const float rowGap = 14.0f * ui.s;
  const float scrollTrackW = 10.0f * ui.s;
  const float innerW = gridSize.x - pad * 2.0f - scrollTrackW;
  const float tileW = std::max(120.0f * ui.s, (innerW - colGap) * 0.5f);
  const float tileH = std::round(tileW * 0.48f);
  const float headingH = 28.0f * ui.s;
  const float drawerHeadH = 30.0f * ui.s;
  const float drawerFilterH = 36.0f * ui.s;
  const float drawerRowH = 34.0f * ui.s;

  std::vector<VisualRow> vrows;
  std::vector<std::pair<int, int>> groupToRowCol(groups.size(), {0, 0});
  const float sectionH = 48.0f * ui.s;
  for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
    const std::string &hdr = groups[i].groupHeading;
    const bool isSection = groups[i].sectionMapCount > 0;
    const bool needNewRow = vrows.empty() || isSection || !hdr.empty() ||
                            vrows.back().col[1] >= 0;
    if (needNewRow) {
      VisualRow vr;
      vr.heading = isSection ? std::string() : hdr;
      vr.section = isSection;
      vr.col[0] = i;
      vr.tilesH = isSection ? sectionH : tileH;
      vrows.push_back(std::move(vr));
    } else {
      vrows.back().col[1] = i;
    }
    const int rIdx = static_cast<int>(vrows.size()) - 1;
    const int cIdx = (vrows.back().col[0] == i) ? 0 : 1;
    groupToRowCol[i] = {rIdx, cIdx};
    if (!isSection && st.mapDrawerKey == groups[i].key) {
      vrows[rIdx].expandedCol = cIdx;
      const int totalDrawerItems =
          1 + static_cast<int>(groups[i].variants.size());
      const int vis = std::clamp(totalDrawerItems, 1, 6);
      vrows[rIdx].drawerVisibleRows = vis;
      vrows[rIdx].drawerH =
          12.0f * ui.s + drawerHeadH + drawerFilterH + vis * drawerRowH + 10.0f * ui.s;
    }
  }

  int expandedGroupIdx = -1;
  for (const VisualRow &vr : vrows) {
    if (vr.expandedCol >= 0)
      expandedGroupIdx = vr.col[vr.expandedCol];
  }

  // ---- Expanded drawer variant filter (text search + game-mode/forge category) ----
  // Test hooks: "-drawer-filter-test <text>" and "-drawer-category-test <n>" pre-fill the drawer filter when it opens,
  // so a capture can show the filtered list without any keyboard input.
  static bool s_testDrawerParsed = false;
  static std::string s_testDrawerFilter;
  static int s_testDrawerCategory = 0;
  if (!s_testDrawerParsed) {
    s_testDrawerParsed = true;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i + 1 < argc; ++i) {
      if (_wcsicmp(argv[i], L"-drawer-filter-test") == 0) {
        std::wstring ws(argv[i + 1]);
        s_testDrawerFilter.assign(ws.begin(), ws.end());
      } else if (_wcsicmp(argv[i], L"-drawer-category-test") == 0) {
        s_testDrawerCategory = _wtoi(argv[i + 1]);
      }
    }
    if (argv) LocalFree(argv);
  }
  static std::string s_lastDrawerKey;
  if (st.mapDrawerKey != s_lastDrawerKey) {
    s_lastDrawerKey = st.mapDrawerKey;
    st.mapDrawerFilter[0] = '\0';
    st.mapDrawerCategory = 0;
    if (!st.mapDrawerKey.empty()) {
      if (!s_testDrawerFilter.empty())
        std::snprintf(st.mapDrawerFilter, sizeof(st.mapDrawerFilter), "%s", s_testDrawerFilter.c_str());
      st.mapDrawerCategory = std::max(0, s_testDrawerCategory);
    }
  }
  std::vector<std::string> drawerCats;
  std::vector<int> drawerFiltered;
  int drawerTotalVariants = 0;
  if (expandedGroupIdx >= 0) {
    const MapTileGroup &eg = groups[expandedGroupIdx];
    drawerTotalVariants = static_cast<int>(eg.variants.size());
    for (const MapEntry &v : eg.variants) {
      if (v.forgeGroup.empty()) continue;
      if (std::find(drawerCats.begin(), drawerCats.end(), v.forgeGroup) == drawerCats.end())
        drawerCats.push_back(v.forgeGroup);
    }
    std::sort(drawerCats.begin(), drawerCats.end());
    st.mapDrawerCategory =
        std::clamp(st.mapDrawerCategory, 0, static_cast<int>(drawerCats.size()));
    const std::string needle = st.mapDrawerFilter;
    const std::string selectedCat =
        st.mapDrawerCategory > 0 ? drawerCats[st.mapDrawerCategory - 1] : std::string();
    for (int vi = 0; vi < static_cast<int>(eg.variants.size()); ++vi) {
      const MapEntry &v = eg.variants[vi];
      if (!selectedCat.empty() && v.forgeGroup != selectedCat) continue;
      if (!needle.empty() && !ContainsNoCase(v.name, needle) &&
          !ContainsNoCase(v.forgeGroup, needle))
        continue;
      drawerFiltered.push_back(vi);
    }
  }
  if (expandedGroupIdx < 0) {
    st.mapDrawerFocus = -1;
  } else if (st.mapDrawerFocus >= 0) {
    const int maxD = 1 + static_cast<int>(drawerFiltered.size());
    st.mapDrawerFocus = std::clamp(st.mapDrawerFocus, 0, maxD);
  }

  float totalContentH = pad;
  for (size_t r = 0; r < vrows.size(); ++r) {
    if (!vrows[r].heading.empty())
      totalContentH += headingH + 6.0f * ui.s;
    vrows[r].y = totalContentH;
    totalContentH += vrows[r].tilesH + vrows[r].drawerH + rowGap;
  }
  const float maxScroll = std::max(0.0f, totalContentH - gridSize.y + pad);

  // Handle keyboard & controller navigation inside the grid / expanded drawer
  bool movedByInput = autoScrollSelection;
  if (hasFocus && !typing) {
    const auto [curR, curC] = groupToRowCol[selectedGroupIdx];
    const bool inDrawer = expandedGroupIdx >= 0 && st.mapDrawerFocus >= 0;
    if (!inDrawer) {
      if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || UiPad::Left()) {
        if (curC == 1 && vrows[curR].col[0] >= 0) {
          selectedGroupIdx = vrows[curR].col[0];
          movedByInput = true;
          MenuSounds::Scroll();
        }
      } else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || UiPad::Right()) {
        if (curC == 0 && vrows[curR].col[1] >= 0) {
          selectedGroupIdx = vrows[curR].col[1];
          movedByInput = true;
          MenuSounds::Scroll();
        } else {
          res.moveFocusRight = true;
          MenuSounds::Scroll();
        }
      } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || UiPad::Down()) {
        if (vrows[curR].expandedCol == curC) {
          st.mapDrawerFocus = 0;
          movedByInput = true;
          MenuSounds::Scroll();
        } else if (curR + 1 < static_cast<int>(vrows.size())) {
          const VisualRow &nxt = vrows[curR + 1];
          selectedGroupIdx =
              (curC == 1 && nxt.col[1] >= 0) ? nxt.col[1] : nxt.col[0];
          movedByInput = true;
          MenuSounds::Scroll();
        }
      } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up()) {
        if (curR > 0) {
          const VisualRow &prv = vrows[curR - 1];
          if (prv.expandedCol >= 0) {
            selectedGroupIdx = prv.col[prv.expandedCol];
            st.mapDrawerFocus =
                static_cast<int>(groups[selectedGroupIdx].variants.size());
          } else {
            selectedGroupIdx =
                (curC == 1 && prv.col[1] >= 0) ? prv.col[1] : prv.col[0];
          }
          movedByInput = true;
          MenuSounds::Scroll();
        }
      }
      if (AcceptPressed() || UiPad::ActionY()) {
        const bool wasY = UiPad::ActionY();
        if (!wasY)
          UiPad::ConsumeAccept();
        UiPad::ConsumeAll();
        const MapTileGroup &grp = groups[selectedGroupIdx];
        if (grp.sectionMapCount > 0 && grp.base.mod) {
          // Collapsed mod banner: Accept / click expands or folds the whole mod section.
          const std::string k = OpenKey(openMode, *grp.base.mod);
          if (!st.openMods.erase(k))
            st.openMods.insert(k);
          st.mapDrawerKey.clear();
          st.mapDrawerFocus = -1;
          MenuSounds::Click();
        } else if (st.mapDrawerKey == grp.key && wasY) {
          st.mapDrawerKey.clear();
          st.mapDrawerFocus = -1;
          MenuSounds::Back();
        } else {
          st.mapDrawerKey = grp.key;
          // Focus currently active mvar inside drawer (0 = Base Map)
          int initFocus = 0;
          if (!selectedMvar.empty()) {
            for (size_t vi = 0; vi < grp.variants.size(); ++vi) {
              if (grp.variants[vi].mvar == selectedMvar) {
                initFocus = static_cast<int>(vi) + 1;
                break;
              }
            }
          }
          st.mapDrawerFocus = initFocus;
          movedByInput = true;
          MenuSounds::Click();
        }
      }
    } else {
      // Navigating inside open [BASE MAP] + Variants Drawer (the list may be filtered).
      const MapTileGroup &grp = groups[expandedGroupIdx];
      const int itemCount = 1 + static_cast<int>(drawerFiltered.size());
      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up()) {
        if (st.mapDrawerFocus > 0) {
          --st.mapDrawerFocus;
          movedByInput = true;
        } else {
          st.mapDrawerFocus = -1;
          movedByInput = true;
        }
        MenuSounds::Scroll();
      } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || UiPad::Down()) {
        if (st.mapDrawerFocus + 1 < itemCount) {
          ++st.mapDrawerFocus;
          movedByInput = true;
          MenuSounds::Scroll();
        } else {
          const int drR = groupToRowCol[expandedGroupIdx].first;
          if (drR + 1 < static_cast<int>(vrows.size())) {
            st.mapDrawerFocus = -1;
            const int c = groupToRowCol[expandedGroupIdx].second;
            selectedGroupIdx = (c == 1 && vrows[drR + 1].col[1] >= 0)
                                   ? vrows[drR + 1].col[1]
                                   : vrows[drR + 1].col[0];
            movedByInput = true;
            MenuSounds::Scroll();
          }
        }
      } else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || UiPad::Right()) {
        res.moveFocusRight = true;
        MenuSounds::Scroll();
      } else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || UiPad::Left()) {
        st.mapDrawerFocus = -1;
        movedByInput = true;
        MenuSounds::Scroll();
      }
      if (AcceptPressed()) {
        UiPad::ConsumeAccept();
        if (st.mapDrawerFocus == 0) {
          selectedMvar.clear();
        } else if (st.mapDrawerFocus >= 1 &&
                   st.mapDrawerFocus - 1 < static_cast<int>(drawerFiltered.size())) {
          selectedMvar = grp.variants[drawerFiltered[st.mapDrawerFocus - 1]].mvar;
        }
        selectedGroupIdx = expandedGroupIdx;
        res.pickedBaseOrVariant = true;
        MenuSounds::Click();
      }
      if (UiPad::ActionY()) {
        UiPad::ConsumeAll();
        st.mapDrawerKey.clear();
        st.mapDrawerFocus = -1;
        MenuSounds::Back();
      }
    }
  }

  // Determine whether the mouse is hovering an open variant drawer with
  // overflow, so wheel scrolling inside the drawer doesn't simultaneously
  // scroll the outer map grid.
  bool hoveringScrollableDrawer = false;
  if (expandedGroupIdx >= 0) {
    const int drR = groupToRowCol[expandedGroupIdx].first;
    const VisualRow &expVr = vrows[drR];
    const int totalDrawerItems = 1 + static_cast<int>(drawerFiltered.size());
    if (totalDrawerItems > expVr.drawerVisibleRows) {
      const float expRowDrawY = gridPos.y + expVr.y - st.mapGridScrollY;
      const float expDTop = expRowDrawY + expVr.tilesH + 10.0f * ui.s;
      const float expDW = tileW * 2.0f + colGap;
      const ImVec2 expList0(gridPos.x + pad + 8.0f * ui.s,
                            expDTop + drawerHeadH + drawerFilterH + 4.0f * ui.s);
      const ImVec2 expList1(gridPos.x + pad + expDW - 8.0f * ui.s,
                            expRowDrawY + expVr.tilesH + expVr.drawerH -
                                6.0f * ui.s);
      hoveringScrollableDrawer =
          ImGui::IsMouseHoveringRect(expList0, expList1, false);
    }
  }

  // Smooth scrolling (mouse wheel + right analog stick + focus-follow)
  const bool hoveringGrid = ImGui::IsMouseHoveringRect(gridPos, gridEnd, false);
  if (hoveringGrid && !hoveringScrollableDrawer &&
      ImGui::GetIO().MouseWheel != 0.0f) {
    st.mapGridScrollY -= ImGui::GetIO().MouseWheel * 72.0f * ui.s;
  }
  const bool drawerOwnsStick = (hasFocus && st.mapDrawerFocus >= 0);
  if (!drawerOwnsStick) {
    const float rStickScroll = UiPad::AnalogScrollY(false);
    if (rStickScroll != 0.0f) {
      st.mapGridScrollY += rStickScroll;
    }
  }
  if (movedByInput && selectedGroupIdx >= 0 &&
      selectedGroupIdx < static_cast<int>(groups.size())) {
    const int rIdx = groupToRowCol[selectedGroupIdx].first;
    const float itemTop = vrows[rIdx].y -
                          (!vrows[rIdx].heading.empty() ? headingH : 0.0f) -
                          8.0f * ui.s;
    const float itemBottom = vrows[rIdx].y + vrows[rIdx].tilesH +
                             vrows[rIdx].drawerH + 10.0f * ui.s;
    if (itemTop < st.mapGridScrollY)
      st.mapGridScrollY = itemTop;
    if (itemBottom > st.mapGridScrollY + gridSize.y)
      st.mapGridScrollY = itemBottom - gridSize.y;
  }
  st.mapGridScrollY = std::clamp(st.mapGridScrollY, 0.0f, maxScroll);

  static std::unordered_map<std::string, int> s_drawerScroll;

  // Render grid inside crisp clip rect
      ui.dl->PushClipRect(gridPos, gridEnd, true);
      for (int r = 0; r < static_cast<int>(vrows.size()); ++r) {
        const VisualRow &vr = vrows[r];
        const float rowDrawY = gridPos.y + vr.y - st.mapGridScrollY;
        const float rowFullBottom = rowDrawY + vr.tilesH + vr.drawerH;
        if (rowFullBottom < gridPos.y - headingH - 10.0f * ui.s ||
            rowDrawY - headingH > gridEnd.y + 10.0f * ui.s)
          continue;

        if (!vr.heading.empty()) {
          const float hy = rowDrawY - headingH - 4.0f * ui.s;
          const ImVec2 h0(gridPos.x + pad, hy),
              h1(gridPos.x + pad + innerW, hy + headingH - 4.0f * ui.s);
          ui.dl->AddRectFilledMultiColor(
              h0, h1, IM_COL32(28, 54, 80, 185), IM_COL32(14, 24, 38, 95),
              IM_COL32(14, 24, 38, 95), IM_COL32(28, 54, 80, 185));
          ui.dl->AddRectFilled(h0, ImVec2(h0.x + 3.0f * ui.s, h1.y), kAccent);
          Text(ui, NobleStyle::TitleFont(), 16.0f,
               ImVec2(h0.x + 10.0f * ui.s, h0.y + 3.0f * ui.s), kWhite,
               Upper(vr.heading).c_str());
          // A mod section that has been expanded can be folded again by clicking its heading row (the banner row is
          // only drawn while the section is closed).
          const int hdrGroup = vr.col[0] >= 0 ? vr.col[0] : vr.col[1];
          ImGui::SetCursorScreenPos(h0);
          ImGui::PushID("foldhdr");
          ImGui::PushID(hdrGroup);
          const bool hdrClicked = ImGui::InvisibleButton(
              "##fold", ImVec2(h1.x - h0.x, h1.y - h0.y));
          const bool hdrHovered = ImGui::IsItemHovered();
          ImGui::PopID();
          ImGui::PopID();
          const char *foldLbl = "FOLD";
          const float foldW =
              TextWidth(NobleStyle::SmallFont(), 12.5f * ui.s, foldLbl);
          Text(ui, NobleStyle::SmallFont(), 12.5f,
               ImVec2(h1.x - foldW - 10.0f * ui.s, h0.y + 5.0f * ui.s),
               hdrHovered ? kAccent : IM_COL32(143, 216, 255, 190), foldLbl);
          if (hdrClicked) {
            if (hdrGroup >= 0 &&
                hdrGroup < static_cast<int>(groups.size()) &&
                groups[hdrGroup].base.mod) {
              st.openMods.erase(OpenKey(openMode, *groups[hdrGroup].base.mod));
            }
            st.mapDrawerKey.clear();
            st.mapDrawerFocus = -1;
            if (hdrGroup >= 0 && hdrGroup < static_cast<int>(groups.size()))
              selectedGroupIdx = hdrGroup;
            MenuSounds::Back();
          }
        }

        if (vr.section) {
          // Collapsed mod section: one full-width banner. Click (or Accept) expands the mod into its map tiles.
          const int gIdx = vr.col[0];
          const MapTileGroup &grp = groups[gIdx];
          const ImVec2 s0(gridPos.x + pad, rowDrawY);
          const ImVec2 s1(gridPos.x + pad + innerW, rowDrawY + vr.tilesH);
          const bool isSel = (gIdx == selectedGroupIdx);
          const bool isFocused = hasFocus && isSel;
          ImGui::SetCursorScreenPos(s0);
          ImGui::PushID("modsec");
          ImGui::PushID(gIdx);
          const bool secClicked =
              ImGui::InvisibleButton("##modsec", ImVec2(s1.x - s0.x, s1.y - s0.y));
          const bool secHovered = ImGui::IsItemHovered();
          ImGui::PopID();
          ImGui::PopID();
          if (secHovered && mouseMoved)
            MenuSounds::Hover(ImGui::GetID(grp.key.c_str()));
          if (secClicked) {
            selectedGroupIdx = gIdx;
            if (grp.base.mod) {
              const std::string k = OpenKey(openMode, *grp.base.mod);
              if (!st.openMods.erase(k))
                st.openMods.insert(k);
            }
            st.mapDrawerKey.clear();
            st.mapDrawerFocus = -1;
            MenuSounds::Click();
          }
          ui.dl->AddRectFilled(s0, s1,
                               (secHovered || isSel)
                                   ? IM_COL32(24, 46, 68, 235)
                                   : IM_COL32(13, 24, 36, 225),
                               4.0f * ui.s);
          ui.dl->AddRectFilled(s0, ImVec2(s0.x + 3.0f * ui.s, s1.y),
                               kAccent, 2.0f * ui.s);
          NobleUi::DrawTacticalFrame(ui.dl, s0, s1, ui.s, isFocused,
                                     isSel || secHovered, false, true);
          const char *unit =
              (baseItemTag && std::strstr(baseItemTag, "MISSION")) ? "MISSIONS"
                                                                   : "MAPS";
          char cnt[64];
          std::snprintf(cnt, sizeof(cnt), "%d %s", grp.sectionMapCount, unit);
          const char *expandLbl = "CLICK TO EXPAND";
          ImFont *smallFont = NobleStyle::SmallFont();
          const float cntW = TextWidth(smallFont, 14.0f * ui.s, cnt);
          const float expW = TextWidth(smallFont, 12.5f * ui.s, expandLbl);
          // Right-align "CLICK TO EXPAND", then the count with a fixed gap before it. The old layout put the count at a
          // fixed 120 px from the edge and drew a chevron over the label at narrow widths.
          const float rightEdge = s1.x - 16.0f * ui.s;
          const float gapBetween = 18.0f * ui.s;
          const bool showExpand = rightEdge - expW - gapBetween - cntW >
                                  s0.x + 120.0f * ui.s;
          const float expX = rightEdge - expW;
          const float cntX = (showExpand ? expX - gapBetween : rightEdge) - cntW;
          const float countY = s0.y + 16.0f * ui.s;
          Text(ui, smallFont, 14.0f, ImVec2(cntX, countY),
               IM_COL32(150, 180, 205, 235), cnt);
          if (showExpand)
            Text(ui, smallFont, 12.5f, ImVec2(expX, countY), kAccent,
                 expandLbl);
          const float titleRight = cntX - 20.0f * ui.s;
          const float titleMaxW = std::max(80.0f * ui.s, titleRight - (s0.x + 16.0f * ui.s));
          const std::string modTitle = Ellipsize(
              NobleStyle::TitleFont(), 20.0f * ui.s, Upper(grp.base.name),
              titleMaxW);
          Text(ui, NobleStyle::TitleFont(), 20.0f,
               ImVec2(s0.x + 16.0f * ui.s, s0.y + 11.0f * ui.s), kWhite,
               modTitle.c_str());
        } else {
        // Draw up to 2 tiles in this row
        for (int c = 0; c < 2; ++c) {
          const int gIdx = vr.col[c];
          if (gIdx < 0)
            continue;
          const MapTileGroup &grp = groups[gIdx];
          const ImVec2 t0(gridPos.x + pad + c * (tileW + colGap), rowDrawY);
          const ImVec2 t1(t0.x + tileW, t0.y + tileH);
          const bool isSelected = (gIdx == selectedGroupIdx);
          const bool isFocused =
              hasFocus && isSelected && st.mapDrawerFocus < 0;
          const bool isExpanded = (st.mapDrawerKey == grp.key);

          ImGui::SetCursorScreenPos(t0);
          ImGui::PushID("mtile");
          ImGui::PushID(gIdx);
          const bool clicked =
              ImGui::InvisibleButton("##t", ImVec2(tileW, tileH));
          const bool hovered = ImGui::IsItemHovered();
          ImGui::PopID();
          ImGui::PopID();

          if (hovered && mouseMoved && selectedGroupIdx != gIdx) {
            MenuSounds::Hover(ImGui::GetID(grp.key.c_str()));
          }
          if (clicked) {
            const bool same = (selectedGroupIdx == gIdx);
            selectedGroupIdx = gIdx;
            // Keep previously active mvar only if belonging to this map;
            // otherwise reset cleanly to Base Map
            if (!selectedMvar.empty()) {
              bool belongs = false;
              for (const MapEntry &v : grp.variants)
                if (v.mvar == selectedMvar) {
                  belongs = true;
                  break;
                }
              if (!belongs)
                selectedMvar.clear();
            }
            if (same && isExpanded) {
              st.mapDrawerKey.clear();
              st.mapDrawerFocus = -1;
              MenuSounds::Back();
            } else {
              st.mapDrawerKey = grp.key;
              st.mapDrawerFocus = 0;
              MenuSounds::Click();
            }
          }

          // Cover background (crisp aspect, no upscaling)
          ui.dl->AddRectFilled(t0, t1, IM_COL32(12, 18, 28, 240));
          if (UiTexture tex = ResolveMapTileTexture(grp.base, false)) {
            const ImU32 tint = grp.base.enabled ? IM_COL32_WHITE
                                                : IM_COL32(150, 150, 150, 180);
            ui.dl->PushClipRect(t0, t1, true);
            UiArt::DrawCoverCrisp(ui.dl, tex, t0, t1, tint, crispMax);
            ui.dl->PopClipRect();
          }

          // Bottom gradient scrim so white/orange title & player info always
          // read crisp
          const float scrimY = t0.y + tileH * 0.36f;
          ui.dl->AddRectFilledMultiColor(
              ImVec2(t0.x, scrimY), t1, IM_COL32(5, 9, 14, 0),
              IM_COL32(5, 9, 14, 0), IM_COL32(5, 9, 14, 240),
              IM_COL32(5, 9, 14, 240));

          // Variant / Mod pill top-right
          if (!grp.variants.empty() || grp.base.mod) {
            char badge[40];
            if (!grp.variants.empty())
              std::snprintf(badge, sizeof(badge), "BASE + %zu",
                            grp.variants.size());
            else
              std::snprintf(badge, sizeof(badge), "MOD MAP");
            const float bw =
                TextWidth(NobleStyle::SmallFont(), 12.0f * ui.s, badge) +
                14.0f * ui.s;
            const float bh = 20.0f * ui.s;
            const ImVec2 b0(t1.x - bw - 10.0f * ui.s, t0.y + 10.0f * ui.s),
                b1(b0.x + bw, b0.y + bh);
            ui.dl->AddRectFilled(b0, b1,
                                 isExpanded ? IM_COL32(255, 168, 62, 60)
                                            : IM_COL32(10, 22, 34, 205),
                                 2.0f * ui.s);
            ui.dl->AddRect(b0, b1,
                           isExpanded ? IM_COL32(255, 185, 80, 230)
                                      : IM_COL32(143, 216, 255, 140),
                           2.0f * ui.s);
            Text(ui, NobleStyle::SmallFont(), 12.0f,
                 ImVec2(b0.x + 7.0f * ui.s, b0.y + 3.0f * ui.s),
                 isExpanded ? IM_COL32(255, 220, 150, 255) : kAccent, badge);
          }

          // Bottom Map Title & Players / Active Variant subtitle (like SWORD
          // BASE / 8-16 Players)
          const MapCardMeta meta = GetMapCardMeta(&grp.base, nullptr);
          const std::string titleStr =
              Ellipsize(NobleStyle::TitleFont(), 22.0f * ui.s,
                        Upper(grp.base.name), tileW - 28.0f * ui.s);
          std::string subStr = meta.players;
          if (isSelected && !selectedMvar.empty()) {
            for (const MapEntry &v : grp.variants) {
              if (v.mvar == selectedMvar) {
                subStr = "VARIANT: " + v.name;
                break;
              }
            }
          } else if (isExpanded) {
            subStr = meta.players + "  \xC2\xB7  SELECTING BELOW";
          }
          subStr = Ellipsize(NobleStyle::SmallFont(), 14.0f * ui.s, subStr,
                             tileW - 28.0f * ui.s);

          const float textX = t0.x + 14.0f * ui.s;
          const float titleY = t1.y - 46.0f * ui.s;
          Text(ui, NobleStyle::TitleFont(), 22.0f,
               ImVec2(textX + 1.0f * ui.s, titleY + 1.0f * ui.s),
               IM_COL32(0, 0, 0, 220), titleStr.c_str());
          Text(ui, NobleStyle::TitleFont(), 22.0f, ImVec2(textX, titleY),
               grp.base.enabled ? kWhite : kDisabled, titleStr.c_str());
          Text(ui, NobleStyle::SmallFont(), 14.0f,
               ImVec2(textX, t1.y - 22.0f * ui.s),
               (isSelected && !selectedMvar.empty())
                   ? IM_COL32(255, 198, 100, 255)
                   : IM_COL32(190, 210, 225, 235),
               subStr.c_str());

          NobleUi::DrawTacticalFrame(ui.dl, t0, t1, ui.s, isFocused,
                                     isSelected || isExpanded, hovered, true);
        }

        // Inline Expanded [BASE MAP] + Variants Drawer directly underneath this
        // row!
        if (vr.expandedCol >= 0) {
          const int gIdx = vr.col[vr.expandedCol];
          const MapTileGroup &grp = groups[gIdx];
          const float dTop = rowDrawY + vr.tilesH + 10.0f * ui.s;
          const float dW = tileW * 2.0f + colGap;
          const ImVec2 d0(gridPos.x + pad, dTop);
          const ImVec2 d1(d0.x + dW, rowDrawY + vr.tilesH + vr.drawerH);

          // Pointer notch connecting to the expanded tile above it
          const float ownerCenterX = gridPos.x + pad +
                                     vr.expandedCol * (tileW + colGap) +
                                     tileW * 0.5f;
          ui.dl->AddTriangleFilled(ImVec2(ownerCenterX - 9.0f * ui.s, d0.y),
                                   ImVec2(ownerCenterX + 9.0f * ui.s, d0.y),
                                   ImVec2(ownerCenterX, d0.y - 8.0f * ui.s),
                                   IM_COL32(255, 168, 62, 230));

          ui.dl->AddRectFilled(d0, d1, IM_COL32(10, 17, 26, 245), 6.0f * ui.s);
          NobleUi::DrawTacticalFrame(ui.dl, d0, d1, ui.s,
                                     hasFocus && st.mapDrawerFocus >= 0, true,
                                     false, true);

          // Drawer Header
          const ImVec2 hd0(d0.x + 1.0f, d0.y + 1.0f),
              hd1(d1.x - 1.0f, d0.y + drawerHeadH);
          ui.dl->AddRectFilled(hd0, hd1, IM_COL32(22, 40, 58, 220), 5.0f * ui.s,
                               ImDrawFlags_RoundCornersTop);
          ui.dl->AddLine(ImVec2(hd0.x, hd1.y), ImVec2(hd1.x, hd1.y),
                         IM_COL32(255, 175, 70, 140), 1.0f);
          char headTxt[128];
          std::snprintf(headTxt, sizeof(headTxt),
                        "%s  -  BASE MAP & VARIANTS (%zu)",
                        Upper(grp.base.name).c_str(), grp.variants.size());
          Text(ui, NobleStyle::TitleFont(), 15.5f,
               ImVec2(hd0.x + 12.0f * ui.s, hd0.y + 6.0f * ui.s),
               IM_COL32(255, 200, 115, 255), headTxt);

          // Close pill
          const float closeW = 92.0f * ui.s;
          const ImVec2 cl0(hd1.x - closeW - 8.0f * ui.s, hd0.y + 4.0f * ui.s),
              cl1(hd1.x - 8.0f * ui.s, hd1.y - 4.0f * ui.s);
          ImGui::SetCursorScreenPos(cl0);
          if (ImGui::InvisibleButton("##close_drawer",
                                     ImVec2(closeW, cl1.y - cl0.y))) {
            st.mapDrawerKey.clear();
            st.mapDrawerFocus = -1;
            MenuSounds::Back();
          }
          const bool clHov = ImGui::IsItemHovered();
          ui.dl->AddRectFilled(cl0, cl1,
                               clHov ? IM_COL32(60, 90, 120, 220)
                                     : IM_COL32(18, 28, 40, 200),
                               5.0f * ui.s);
          ui.dl->AddRect(cl0, cl1,
                         IM_COL32(143, 216, 255, clHov ? 200 : 120),
                         5.0f * ui.s);
          Text(ui, NobleStyle::SmallFont(), 12.0f,
               ImVec2(cl0.x + 14.0f * ui.s, cl0.y + 4.0f * ui.s), kWhite,
               "COLLAPSE");

          int &dScroll = s_drawerScroll[grp.key];
          // Filter row inside the drawer: variant text search + game-mode/forge category. Forge World AI
          // Battlegrounds alone has 353 variants, so scrolling without these was unusable.
          {
            const ImVec2 fr0(d0.x + 1.0f, hd1.y);
            const ImVec2 fr1(d1.x - 1.0f, hd1.y + drawerFilterH);
            ui.dl->AddRectFilled(fr0, fr1, IM_COL32(13, 24, 36, 240));
            ui.dl->AddLine(ImVec2(fr0.x, fr1.y), ImVec2(fr1.x, fr1.y),
                           IM_COL32(255, 175, 70, 90), 1.0f * ui.s);
            const float padF = 8.0f * ui.s;
            float fx = fr0.x + padF;
            const float fy = fr0.y + (drawerFilterH - 26.0f * ui.s) * 0.5f;
            ImGui::PushID(grp.key.c_str());
            ImGui::SetCursorScreenPos(ImVec2(fx, fy));
            ImGui::SetNextItemWidth(220.0f * ui.s);
            if (ImGui::InputTextWithHint("##drawerfilter", "Filter variants...",
                                         st.mapDrawerFilter, sizeof(st.mapDrawerFilter))) {
              dScroll = 0;
              st.mapDrawerFocus = 0;
            }
            fx += 230.0f * ui.s;
            if (!drawerCats.empty()) {
              const std::string catLabel =
                  st.mapDrawerCategory <= 0
                      ? std::string("ALL MODES")
                      : Upper(drawerCats[st.mapDrawerCategory - 1]);
              const float lw =
                  TextWidth(NobleStyle::SmallFont(), 12.5f * ui.s, catLabel.c_str());
              const float catW = std::max(130.0f * ui.s, lw + 30.0f * ui.s);
              // left arrow
              ImGui::SetCursorScreenPos(ImVec2(fx, fy + 2.0f * ui.s));
              if (ImGui::InvisibleButton("##catleft", ImVec2(20.0f * ui.s, 22.0f * ui.s))) {
                st.mapDrawerCategory =
                    st.mapDrawerCategory <= 0 ? static_cast<int>(drawerCats.size())
                                              : st.mapDrawerCategory - 1;
                dScroll = 0;
                st.mapDrawerFocus = 0;
                MenuSounds::Scroll();
              }
              const bool catLeftHov = ImGui::IsItemHovered();
              const ImVec2 cat0(fx + 20.0f * ui.s, fy);
              const ImVec2 cat1(cat0.x + catW, fy + 26.0f * ui.s);
              // The whole pill is clickable too (cycles forward) - easier target than the two small arrows.
              ImGui::SetCursorScreenPos(cat0);
              if (ImGui::InvisibleButton("##catcycle", ImVec2(catW, 26.0f * ui.s))) {
                st.mapDrawerCategory = (st.mapDrawerCategory + 1) %
                                       (static_cast<int>(drawerCats.size()) + 1);
                dScroll = 0;
                st.mapDrawerFocus = 0;
                MenuSounds::Scroll();
              }
              ImGui::SetCursorScreenPos(ImVec2(cat1.x, fy + 2.0f * ui.s));
              if (ImGui::InvisibleButton("##catright", ImVec2(20.0f * ui.s, 22.0f * ui.s))) {
                st.mapDrawerCategory = (st.mapDrawerCategory + 1) %
                                       (static_cast<int>(drawerCats.size()) + 1);
                dScroll = 0;
                st.mapDrawerFocus = 0;
                MenuSounds::Scroll();
              }
              const bool catRightHov = ImGui::IsItemHovered();
              ui.dl->AddRectFilled(cat0, cat1, IM_COL32(22, 40, 58, 225), 3.0f * ui.s);
              ui.dl->AddRect(cat0, cat1, IM_COL32(143, 216, 255, 110), 3.0f * ui.s);
              Text(ui, NobleStyle::SmallFont(), 12.5f,
                   ImVec2(cat0.x + (catW - lw) * 0.5f, cat0.y + 6.0f * ui.s),
                   kAccent, catLabel.c_str());
              const float ah = 6.0f * ui.s;
              const float acy = fy + 13.0f * ui.s;
              ui.dl->AddTriangleFilled(
                  ImVec2(fx + 13.0f * ui.s, acy - ah), ImVec2(fx + 13.0f * ui.s, acy + ah),
                  ImVec2(fx + 6.0f * ui.s, acy),
                  catLeftHov ? kWhite : IM_COL32(143, 216, 255, 190));
              ui.dl->AddTriangleFilled(
                  ImVec2(cat1.x + 7.0f * ui.s, acy - ah), ImVec2(cat1.x + 7.0f * ui.s, acy + ah),
                  ImVec2(cat1.x + 14.0f * ui.s, acy),
                  catRightHov ? kWhite : IM_COL32(143, 216, 255, 190));
              fx = cat1.x + 28.0f * ui.s;
            }
            ImGui::PopID();
            char filterInfo[80];
            std::snprintf(filterInfo, sizeof(filterInfo), "%d OF %d VARIANTS",
                          static_cast<int>(drawerFiltered.size()), drawerTotalVariants);
            const float fiW = TextWidth(NobleStyle::SmallFont(), 12.5f * ui.s, filterInfo);
            Text(ui, NobleStyle::SmallFont(), 12.5f,
                 ImVec2(fr1.x - fiW - 10.0f * ui.s, fr0.y + 11.0f * ui.s),
                 IM_COL32(150, 180, 205, 235), filterInfo);
          }

          // Scrollable variant rows inside drawer
          const int totalItems = 1 + static_cast<int>(drawerFiltered.size());
          const int maxDScroll = std::max(0, totalItems - vr.drawerVisibleRows);
          static int s_lastDrawerFocus = -999;
          if (st.mapDrawerFocus >= 0 &&
              st.mapDrawerFocus != s_lastDrawerFocus) {
            if (st.mapDrawerFocus < dScroll)
              dScroll = st.mapDrawerFocus;
            if (st.mapDrawerFocus >= dScroll + vr.drawerVisibleRows)
              dScroll = st.mapDrawerFocus - vr.drawerVisibleRows + 1;
          }
          s_lastDrawerFocus = st.mapDrawerFocus;
          const ImVec2 list0(d0.x + 8.0f * ui.s,
                             hd1.y + drawerFilterH + 4.0f * ui.s),
              list1(d1.x - 8.0f * ui.s, d1.y - 6.0f * ui.s);
          if (totalItems > vr.drawerVisibleRows) {
            if (ImGui::IsMouseHoveringRect(list0, list1, false) &&
                ImGui::GetIO().MouseWheel != 0.0f) {
              dScroll -= static_cast<int>(ImGui::GetIO().MouseWheel);
            }
            static float s_drawerStickAccum = 0.0f;
            const float drStick = (hasFocus && st.mapDrawerFocus >= 0)
                                      ? UiPad::AnalogScrollY(false)
                                      : 0.0f;
            if (drStick != 0.0f) {
              s_drawerStickAccum += drStick / std::max(1.0f, drawerRowH);
              const int stepRows = static_cast<int>(s_drawerStickAccum);
              if (stepRows != 0) {
                dScroll += stepRows;
                s_drawerStickAccum -= static_cast<float>(stepRows);
              }
            } else {
              s_drawerStickAccum = 0.0f;
            }
          }
          dScroll = std::clamp(dScroll, 0, maxDScroll);

          const float rowW =
              (list1.x - list0.x) - (maxDScroll > 0 ? 14.0f * ui.s : 0.0f);
                               for (int v = 0; v < vr.drawerVisibleRows &&
                                               dScroll + v < totalItems;
                                    ++v) {
                                 const int itemIdx = dScroll + v;
                                 const bool isBase = (itemIdx == 0);
                                 const MapEntry *varEntry =
                                     isBase ? &grp.base
                                            : &grp.variants[drawerFiltered[itemIdx - 1]];
                                 const bool itemActive =
                                     (selectedGroupIdx == gIdx) &&
                                     (isBase
                                          ? selectedMvar.empty()
                                          : (selectedMvar == varEntry->mvar));
                                 const bool itemFocus =
                                     hasFocus && (st.mapDrawerFocus == itemIdx);
                                 const ImVec2 r0(list0.x,
                                                 list0.y + v * drawerRowH),
                                     r1(list0.x + rowW,
                                        r0.y + drawerRowH - 3.0f * ui.s);

                                 ImGui::SetCursorScreenPos(r0);
                                 ImGui::PushID("dvar");
                                 ImGui::PushID(itemIdx);
                                 const bool rowClick = ImGui::InvisibleButton(
                                     "##v", ImVec2(rowW, r1.y - r0.y));
                                 const bool rowHov = ImGui::IsItemHovered();
                                 ImGui::PopID();
                                 ImGui::PopID();

                                 if (rowClick) {
                                   selectedGroupIdx = gIdx;
                                   selectedMvar = isBase
                                                      ? std::filesystem::path{}
                                                      : varEntry->mvar;
                                   st.mapDrawerFocus = itemIdx;
                                   res.pickedBaseOrVariant = true;
                                   MenuSounds::Click();
                                 }

                                 const ImU32 rbg =
                                     itemFocus    ? IM_COL32(54, 96, 138, 225)
                                     : itemActive ? IM_COL32(28, 66, 94, 210)
                                     : rowHov     ? IM_COL32(32, 50, 70, 200)
                                                  : IM_COL32(14, 22, 32, 185);
        ui.dl->AddRectFilled(r0, r1, rbg, 5.0f * ui.s);
        if (itemFocus)
          NobleUi::DrawFocusHighlight(ui.dl, r0, r1, ui.s, true);
        else if (itemActive)
          ui.dl->AddRect(r0, r1, IM_COL32(255, 175, 70, 210), 5.0f * ui.s, 0,
                         1.5f * ui.s);
        else
          ui.dl->AddRect(r0, r1, IM_COL32(95, 130, 160, 68), 5.0f * ui.s);

        const char *tag =
            isBase ? (baseItemTag ? baseItemTag : "BASE MAP")
                   : (!varEntry->forgeGroup.empty()
                          ? varEntry->forgeGroup.c_str()
                          : "VARIANT");
        const float tagW = std::max(
            84.0f * ui.s,
            TextWidth(NobleStyle::SmallFont(), 12.0f * ui.s, tag) +
                16.0f * ui.s);
        const ImVec2 tg0(r0.x + 8.0f * ui.s, r0.y + 5.0f * ui.s),
            tg1(tg0.x + tagW, r1.y - 5.0f * ui.s);
        ui.dl->AddRectFilled(
            tg0, tg1,
            isBase ? IM_COL32(45, 110, 155, 150)
                   : IM_COL32(255, 168, 62, 55),
            4.0f * ui.s);
                                 Text(ui, NobleStyle::SmallFont(), 12.0f,
                                      ImVec2(tg0.x + 8.0f * ui.s,
                                             tg0.y + 3.0f * ui.s),
                                      isBase ? kAccent
                                             : IM_COL32(255, 205, 115, 255),
                                      Upper(tag).c_str());

                                 const std::string nameLabel =
                                     isBase ? (grp.base.name +
                                               "  (Standard Unmodified)")
                                            : varEntry->name;
                                 const float nameX = tg1.x + 12.0f * ui.s;
                                 const float activeW =
                                     itemActive ? 74.0f * ui.s : 8.0f * ui.s;
                                 const std::string fittedName = Ellipsize(
                                     NobleStyle::TitleFont(), 16.0f * ui.s,
                                     nameLabel, (r1.x - activeW) - nameX);
                                 Text(ui, NobleStyle::TitleFont(), 16.0f,
                                      ImVec2(nameX, r0.y + 6.0f * ui.s),
                                      itemActive || itemFocus
                                          ? kWhite
                                          : IM_COL32(215, 225, 235, 240),
                                      fittedName.c_str());

                                 if (itemActive) {
                                   Text(ui, NobleStyle::SmallFont(), 12.5f,
                                        ImVec2(r1.x - 66.0f * ui.s,
                                               r0.y + 8.0f * ui.s),
                                        IM_COL32(125, 235, 150, 255),
                                        "SELECTED");
                                 }
                               }
      if (maxDScroll > 0) {
        float drScrollFloat = static_cast<float>(dScroll);
        const float tx = list1.x - 7.0f * ui.s;
        NobleUi::DrawInteractiveScrollbar(
            ui.dl, "drawer_sb", ImVec2(tx, list0.y),
            ImVec2(tx + 6.0f * ui.s, list1.y), drScrollFloat,
            static_cast<float>(maxDScroll),
            static_cast<float>(vr.drawerVisibleRows),
            static_cast<float>(totalItems), ui.s);
        dScroll =
            std::clamp(static_cast<int>(drScrollFloat + 0.5f), 0, maxDScroll);
      }
    }
        }
  }
  ui.dl->PopClipRect();

  // Right interactive scrollbar for whole grid
  if (maxScroll > 1.0f) {
    const float tx = gridEnd.x - 9.0f * ui.s;
    const float tTop = gridPos.y + 8.0f * ui.s,
                tBot = gridEnd.y - 8.0f * ui.s;
    NobleUi::DrawInteractiveScrollbar(
        ui.dl, "map_grid_sb", ImVec2(tx, tTop),
        ImVec2(tx + 7.0f * ui.s, tBot), st.mapGridScrollY, maxScroll,
        gridSize.y, totalContentH + pad, ui.s);
  }
  return res;
}

        // Unified Right Detail Card matching SWORD BASE reference card: Title,
        // Crisp Artwork, Modes/Terrain, Description
        float DrawMapDetailHeaderCard(const Ui &ui, ImVec2 cardPos, float cardW,
                                      float previewH, const MapEntry *baseMap,
                                      const MapEntry *activeVariant,
                                      const char *fallbackMode) {
          const float pad = 16.0f * ui.s;
          float y = cardPos.y + 12.0f * ui.s;
          const MapEntry *displayEntry =
              activeVariant ? activeVariant : baseMap;
          const std::string titleRaw = displayEntry
                                           ? Upper(displayEntry->name)
                                           : std::string("NO MAP SELECTED");
          float titlePx = 28.0f * ui.s;
          while (titlePx > 19.0f * ui.s &&
                 TextWidth(NobleStyle::TitleFont(), titlePx, titleRaw.c_str()) >
                     cardW - pad * 2.0f) {
            titlePx -= 2.0f * ui.s;
          }
          const std::string title = Ellipsize(NobleStyle::TitleFont(), titlePx,
                                              titleRaw, cardW - pad * 2.0f);
          Text(ui, NobleStyle::TitleFont(), titlePx / ui.s,
               ImVec2(cardPos.x + pad, y), kWhite, title.c_str());
          y += titlePx + 6.0f * ui.s;
          if (activeVariant && baseMap && activeVariant != baseMap) {
            const std::string varTag = Ellipsize(
                NobleStyle::SmallFont(), 13.0f * ui.s,
                "VARIANT ON " + Upper(baseMap->name), cardW - pad * 2.0f);
            Text(ui, NobleStyle::SmallFont(), 13.0f,
                 ImVec2(cardPos.x + pad, y - 2.0f * ui.s),
                 IM_COL32(255, 195, 95, 255), varTag.c_str());
            y += 18.0f * ui.s;
          }
          ui.dl->AddLine(ImVec2(cardPos.x + pad, y),
                         ImVec2(cardPos.x + cardW - pad, y),
                         IM_COL32(143, 216, 255, 80), 1.0f);
          y += 10.0f * ui.s;

          // Crisp Cover Artwork Box (zero blurry upscaling)
          const ImVec2 img0(cardPos.x + pad, y),
              img1(cardPos.x + cardW - pad, y + previewH);
          ui.dl->AddRectFilled(img0, img1, IM_COL32(6, 10, 16, 240));
          if (baseMap) {
            if (UiTexture tex = ResolveMapTileTexture(*baseMap, true)) {
              const float crispMax =
                  std::max(1.0f, ImGui::GetIO().DisplaySize.y / 1080.0f);
              ui.dl->PushClipRect(img0, img1, true);
              UiArt::DrawCoverCrisp(ui.dl, tex, img0, img1, IM_COL32_WHITE,
                                    crispMax);
              ui.dl->PopClipRect();
            }
          }
          ui.dl->AddRect(img0, img1, IM_COL32(143, 216, 255, 110), 1.0f * ui.s);
          y = img1.y + 10.0f * ui.s;

          // GAME MODES / TERRAIN / LORE matching reference screenshot
          const MapCardMeta meta = GetMapCardMeta(baseMap, fallbackMode);
          auto metaRow = [&](const char *label, const std::string &val) {
            Text(ui, NobleStyle::SmallFont(), 12.5f, ImVec2(cardPos.x + pad, y),
                 IM_COL32(135, 175, 205, 225), label);
            const std::string fitted =
                Ellipsize(NobleStyle::TitleFont(), 15.0f * ui.s, Upper(val),
                          cardW - pad * 2.0f);
            Text(ui, NobleStyle::TitleFont(), 15.0f,
                 ImVec2(cardPos.x + pad, y + 14.0f * ui.s), kWhite,
                 fitted.c_str());
            y += 34.0f * ui.s;
          };
          metaRow("GAME MODES:", meta.modes);
          metaRow("TERRAIN:", meta.terrain);

          const std::string blurb =
              displayEntry ? displayEntry->blurb : std::string();
          if (!blurb.empty()) {
            const float lineH = 15.5f * ui.s * 1.3f;
            const std::string fittedBlurb = FitWrapped(
                nullptr, 15.5f * ui.s, blurb, cardW - pad * 2.0f, lineH * 2.0f);
            const float hUsed = std::min(
                Font(nullptr)
                    ->CalcTextSizeA(15.5f * ui.s, FLT_MAX, cardW - pad * 2.0f,
                                    fittedBlurb.c_str())
                    .y,
                lineH * 2.0f);
            Text(ui, nullptr, 15.5f, ImVec2(cardPos.x + pad, y),
                 IM_COL32(202, 216, 228, 240), fittedBlurb.c_str(),
                 cardPos.x + cardW - pad);
            y += hUsed + 8.0f * ui.s;
          }
          ui.dl->AddLine(ImVec2(cardPos.x + pad, y),
                         ImVec2(cardPos.x + cardW - pad, y),
                         IM_COL32(255, 255, 255, 34), 1.0f);
          return y + 8.0f * ui.s;
        }

        // Sleek Tiled & Tabbed Game Options Screen (replaces old collapsible
        // text accordion)
        void DrawTacticalGameOptions(OverlayContext & ctx, const Ui &ui,
                                     float left, float top, bool ff, bool forge,
                                     bool back) {
          MenuState &st = ctx.st;
          const std::vector<VariantRules::Rule> &rules = VariantRules::Rules();
          DrawTopTelemetryBar(ui, "GAME OPTIONS");
          DrawTitle(ui, ImVec2(left, top), "GAME OPTIONS",
                    forge ? "FORGE" : (ff ? "FIREFIGHT" : "MULTIPLAYER"));

          const float px = left + 26.0f * ui.s;
          const float maxRight = ui.origin.x + ui.size.x * 0.94f;
          const float fullW = maxRight - px;
          float y = top + 72.0f * ui.s;
          const bool typing = ImGui::GetIO().WantTextInput;

          // Row 1: Mode Name + Rule Search + Changed Only toggle
          const float nameW = fullW * 0.30f;
          const float toggleW = 190.0f * ui.s;
          const float searchW = fullW - nameW - toggleW - 20.0f * ui.s;
          DrawSearchBox(ui, "##mode_name", ImVec2(px, y), nameW, st.optionsName,
                        sizeof(st.optionsName), "Game mode name");
          if (!typing && ImGui::GetIO().KeyCtrl &&
              ImGui::IsKeyPressed(ImGuiKey_F, false))
            ImGui::SetKeyboardFocusHere();
          DrawSearchBox(ui, "##options_search",
                        ImVec2(px + nameW + 10.0f * ui.s, y), searchW,
                        st.optionsSearch, sizeof(st.optionsSearch),
                        "Search rules or categories (Ctrl+F)");
          {
            const ImVec2 t0(maxRight - toggleW, y),
                t1(maxRight, y + ImGui::GetFrameHeight());
            ImGui::SetCursorScreenPos(t0);
            bool flip = ImGui::InvisibleButton("##changed_only",
                                               ImVec2(toggleW, t1.y - t0.y));
            const bool hovered = ImGui::IsItemHovered();
            if (!typing &&
                (ImGui::IsKeyPressed(ImGuiKey_X, false) || UiPad::ActionX()))
              flip = true;
            if (flip) {
              st.optionsChangedOnly = !st.optionsChangedOnly;
              MenuSounds::Change();
            }
            NobleUi::DrawBox(ui.dl, t0, t1,
                             st.optionsChangedOnly ? "CHANGED ONLY: ON"
                                                   : "CHANGED ONLY: OFF",
                             hovered, st.optionsChangedOnly, false);
          }
          y += 42.0f * ui.s;

          // Row 2: Quick Presets as Sleek Tactical Tiles
          const std::vector<OptionsPreset> &presets = OptionsPresets();
          const auto applyPreset = [&](const OptionsPreset &preset) {
            st.optionsValues.clear();
            for (const auto &[key, wanted] : preset.values) {
              for (const VariantRules::Rule &rule : rules) {
                if (std::strcmp(rule.key, key) != 0)
                  continue;
                const int value =
                    std::clamp(wanted, rule.minValue, rule.maxValue);
                if (value != VariantRules::Read(rule, st.optionsDecoded.data()))
                  st.optionsValues[rule.key] = value;
              }
            }
            st.confirmAction.clear();
            st.optionsMessage = std::string(preset.name) + " applied (" +
                                std::to_string(st.optionsValues.size()) +
                                " modified rule(s)).";
            NOBLE_INFO("FrontEndMenu: game options preset {} ({} change(s))",
                       preset.name, st.optionsValues.size());
            MenuSounds::Click();
          };
          {
            const int pCount = static_cast<int>(presets.size());
            const float labelW = 92.0f * ui.s;
            const float pGap = 8.0f * ui.s;
            const float tileW =
                (fullW - labelW - pGap * std::max(0, pCount - 1)) /
                std::max(1, pCount);
            const float tileH = 42.0f * ui.s;
            Text(ui, NobleStyle::TitleFont(), 16.0f,
                 ImVec2(px, y + 11.0f * ui.s), kAccent, "PRESETS");
            for (int i = 0; i < pCount; ++i) {
              const ImVec2 t0(px + labelW + i * (tileW + pGap), y),
                  t1(t0.x + tileW, y + tileH);
              ImGui::SetCursorScreenPos(t0);
              ImGui::PushID("opt_preset");
              ImGui::PushID(i);
              const bool clicked =
                  ImGui::InvisibleButton("##p", ImVec2(tileW, tileH));
              const bool hovered = ImGui::IsItemHovered();
              ImGui::PopID();
              ImGui::PopID();
              char sub[32];
              if (presets[i].values.empty())
                std::snprintf(sub, sizeof(sub), "BASE DEFAULTS");
              else
                std::snprintf(sub, sizeof(sub), "%zu RULE%s",
                              presets[i].values.size(),
                              presets[i].values.size() == 1 ? "" : "S");
              ui.dl->AddRectFilled(t0, t1,
                                   hovered ? IM_COL32(34, 62, 92, 225)
                                           : IM_COL32(12, 20, 30, 215),
                                   2.0f * ui.s);
              NobleUi::DrawTacticalFrame(
                  ui.dl, t0, t1, ui.s, hovered && st.optionsPresetPick == i,
                  st.optionsPresetPick == i, hovered, false);
              Text(ui, NobleStyle::TitleFont(), 16.0f,
                   ImVec2(t0.x + 12.0f * ui.s, t0.y + 4.0f * ui.s), kWhite,
                   presets[i].name);
              Text(ui, NobleStyle::SmallFont(), 11.5f,
                   ImVec2(t0.x + 12.0f * ui.s, t0.y + 23.0f * ui.s), kDim, sub);
              if (clicked) {
                st.optionsPresetPick = i;
                applyPreset(presets[i]);
              }
            }
            y += tileH + 12.0f * ui.s;
          }

          // Group all rules cleanly into Category list
          struct CategoryBucket {
            std::string name;
            std::vector<int> ruleIndices;
            int changedCount = 0;
          };
          std::vector<CategoryBucket> categories;
          const std::string query = st.optionsSearch;
          const bool filtering = !query.empty() || st.optionsChangedOnly;
          for (int i = 0; i < static_cast<int>(rules.size()); ++i) {
            const VariantRules::Rule &r = rules[i];
            const bool isChanged = st.optionsValues.count(r.key) != 0;
            const bool matchesQuery =
                query.empty() || ContainsNoCase(r.label, query) ||
                ContainsNoCase(r.help, query) || ContainsNoCase(r.group, query);
            if (st.optionsChangedOnly && !isChanged)
              continue;
            if (!matchesQuery)
              continue;
            if (categories.empty() || categories.back().name != r.group) {
              categories.push_back({r.group, {}, 0});
            }
            categories.back().ruleIndices.push_back(i);
            if (isChanged)
              ++categories.back().changedCount;
          }
          if (!categories.empty())
            st.optionsCategoryIdx =
                std::clamp(st.optionsCategoryIdx, 0,
                           static_cast<int>(categories.size()) - 1);
          else
            st.optionsCategoryIdx = 0;

          // LB / RB cycles categories effortlessly on controller
          if (!typing && !categories.empty()) {
            if (UiPad::TabPrev() || ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
              st.optionsCategoryIdx =
                  (st.optionsCategoryIdx + static_cast<int>(categories.size()) -
                   1) %
                  static_cast<int>(categories.size());
              st.optionsRuleFocus = 0;
              st.optionsFirst = 0;
              MenuSounds::Tab(-1);
            } else if (UiPad::TabNext() ||
                       ImGui::IsKeyPressed(ImGuiKey_E, false)) {
              st.optionsCategoryIdx = (st.optionsCategoryIdx + 1) %
                                      static_cast<int>(categories.size());
              st.optionsRuleFocus = 0;
              st.optionsFirst = 0;
              MenuSounds::Tab(1);
            }
          }

          const float bodyBottom = ui.origin.y + ui.size.y - 68.0f * ui.s;
          const float bodyH = std::max(140.0f * ui.s, bodyBottom - y);
          const float catW = fullW * 0.23f;
          const float gap = 14.0f * ui.s;
          const float midW = fullW * 0.48f;
          const float rightW = fullW - catW - midW - gap * 2.0f;

          // Column 1: Category Tiles
          const ImVec2 cBox0(px, y), cBox1(px + catW, y + bodyH);
          ui.dl->AddRectFilled(cBox0, cBox1, IM_COL32(8, 13, 20, 185),
                               2.0f * ui.s);
          NobleUi::DrawTacticalFrame(ui.dl, cBox0, cBox1, ui.s, false, false,
                                     false, false);
          Text(ui, NobleStyle::SmallFont(), 13.0f,
               ImVec2(cBox0.x + 12.0f * ui.s, cBox0.y + 8.0f * ui.s), kAccent,
               "CATEGORIES [LB/RB]");
          float cy = cBox0.y + 30.0f * ui.s;
          ui.dl->PushClipRect(cBox0, cBox1, true);
          for (int ci = 0; ci < static_cast<int>(categories.size()); ++ci) {
            const float ch = 36.0f * ui.s;
            if (cy + ch > cBox1.y - 6.0f * ui.s)
              break;
            const ImVec2 b0(cBox0.x + 8.0f * ui.s, cy),
                b1(cBox1.x - 8.0f * ui.s, cy + ch - 4.0f * ui.s);
            ImGui::SetCursorScreenPos(b0);
            ImGui::PushID("opt_cat");
            ImGui::PushID(ci);
            const bool clk =
                ImGui::InvisibleButton("##c", ImVec2(b1.x - b0.x, b1.y - b0.y));
            const bool hov = ImGui::IsItemHovered();
            ImGui::PopID();
            ImGui::PopID();
            const bool active = (ci == st.optionsCategoryIdx);
            ui.dl->AddRectFilled(b0, b1,
                                 active ? IM_COL32(42, 86, 126, 225)
                                        : (hov ? IM_COL32(28, 46, 68, 195)
                                               : IM_COL32(14, 22, 32, 165)),
                                 2.0f * ui.s);
            if (active)
              ui.dl->AddRectFilled(b0, ImVec2(b0.x + 3.0f * ui.s, b1.y),
                                   IM_COL32(255, 172, 66, 255));
            const std::string cName = Ellipsize(
                NobleStyle::TitleFont(), 15.5f * ui.s,
                Upper(categories[ci].name), (b1.x - b0.x) - 52.0f * ui.s);
            Text(ui, NobleStyle::TitleFont(), 15.5f,
                 ImVec2(b0.x + 12.0f * ui.s, b0.y + 7.0f * ui.s),
                 active ? kWhite : kDim, cName.c_str());
            char cnt[24];
            if (categories[ci].changedCount > 0)
              std::snprintf(cnt, sizeof(cnt), "%d*",
                            categories[ci].changedCount);
            else
              std::snprintf(cnt, sizeof(cnt), "%zu",
                            categories[ci].ruleIndices.size());
            const float cw =
                TextWidth(NobleStyle::SmallFont(), 13.0f * ui.s, cnt);
            Text(ui, NobleStyle::SmallFont(), 13.0f,
                 ImVec2(b1.x - cw - 10.0f * ui.s, b0.y + 9.0f * ui.s),
                 categories[ci].changedCount > 0 ? NobleUi::kAmber : kDim, cnt);
            if (clk) {
              st.optionsCategoryIdx = ci;
              st.optionsRuleFocus = 0;
              st.optionsFirst = 0;
              MenuSounds::Change();
            }
            cy += ch;
          }
          ui.dl->PopClipRect();

          // Active Category's Rules + Action buttons (Save/Reset/Delete live in
          // Right Column, plus controller D-pad navigates rules cleanly)
          const std::vector<int> emptyList;
          const std::vector<int> &activeRules =
              categories.empty()
                  ? emptyList
                  : categories[st.optionsCategoryIdx].ruleIndices;
          const bool canDelete =
              !st.optionsFile.empty() &&
              st.optionsFile.parent_path() == VariantRules::Directory();
          const int ruleCount = static_cast<int>(activeRules.size());
          const int saveRow = ruleCount, resetRow = ruleCount + 1,
                    deleteRow = ruleCount + 2;
          const int maxRowCount = ruleCount + (canDelete ? 3 : 2);
          st.optionsRuleFocus =
              std::clamp(st.optionsRuleFocus, 0, std::max(0, maxRowCount - 1));

          if (!typing && maxRowCount > 0) {
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || UiPad::Down()) {
              st.optionsRuleFocus = (st.optionsRuleFocus + 1) % maxRowCount;
              MenuSounds::Scroll();
            } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up()) {
              st.optionsRuleFocus =
                  (st.optionsRuleFocus + maxRowCount - 1) % maxRowCount;
              MenuSounds::Scroll();
            }
          }

          // Column 2: Scrollable Option Rows
          const float mX = cBox1.x + gap;
          const ImVec2 m0(mX, y), m1(mX + midW, y + bodyH);
          ui.dl->AddRectFilled(m0, m1, IM_COL32(8, 13, 20, 185), 2.0f * ui.s);
          NobleUi::DrawTacticalFrame(ui.dl, m0, m1, ui.s, false, false, false,
                                     false);
          const std::string activeCatTitle =
              categories.empty()
                  ? (st.optionsChangedOnly ? std::string("NO CHANGED RULES YET")
                                           : std::string("NO MATCHING RULES"))
                  : (Upper(categories[st.optionsCategoryIdx].name) +
                     (filtering ? " (FILTERED)" : ""));
          Text(ui, NobleStyle::TitleFont(), 17.0f,
               ImVec2(m0.x + 14.0f * ui.s, m0.y + 8.0f * ui.s), kWhite,
               activeCatTitle.c_str());

          const float listTop = m0.y + 34.0f * ui.s;
          const float listBot = m1.y - 10.0f * ui.s;
          const float rowStep = 38.0f * ui.s;
          const int visibleRuleRows =
              std::max(1, static_cast<int>((listBot - listTop) / rowStep));
          if (st.optionsRuleFocus < ruleCount) {
            if (st.optionsRuleFocus < st.optionsFirst)
              st.optionsFirst = st.optionsRuleFocus;
            if (st.optionsRuleFocus >= st.optionsFirst + visibleRuleRows)
              st.optionsFirst = st.optionsRuleFocus - visibleRuleRows + 1;
          }
          if (ImGui::IsMouseHoveringRect(m0, m1, false) &&
              ImGui::GetIO().MouseWheel != 0.0f) {
            st.optionsFirst -= static_cast<int>(ImGui::GetIO().MouseWheel);
          }
          const float rStick = UiPad::AnalogScrollY(true);
          if (rStick != 0.0f && ruleCount > visibleRuleRows) {
            st.optionsFirst += (rStick > 0.0f) ? 1 : -1;
          }
          st.optionsFirst = std::clamp(
              st.optionsFirst, 0, std::max(0, ruleCount - visibleRuleRows));

          const float ruleW = midW - 28.0f * ui.s;
          const VariantRules::Rule *focusedRule = nullptr;
          ui.dl->PushClipRect(ImVec2(m0.x, listTop), ImVec2(m1.x, listBot),
                              true);
          for (int vis = 0;
               vis < visibleRuleRows && st.optionsFirst + vis < ruleCount;
               ++vis) {
            const int idxInCat = st.optionsFirst + vis;
            const int rIdx = activeRules[idxInCat];
            const VariantRules::Rule &rule = rules[rIdx];
            const bool rowFocused =
                (!typing && st.optionsRuleFocus == idxInCat);
            if (rowFocused)
              focusedRule = &rule;
            const float ry = listTop + vis * rowStep;
            const int baseVal =
                VariantRules::Read(rule, st.optionsDecoded.data());
            const auto edIt = st.optionsValues.find(rule.key);
            int curVal =
                (edIt != st.optionsValues.end()) ? edIt->second : baseVal;
            const std::string valStr = OptionValueText(rule, curVal);

            std::vector<int> pickValues;
            int pickedChoice = -1;
            const bool listable =
                rule.order || (rule.maxValue - rule.minValue + 1) <= 40;
            const auto fillChoices = [&](std::vector<std::string> &choices,
                                         int &current) {
              if (rule.order) {
                for (int v : *rule.order)
                  pickValues.push_back(v);
              } else {
                for (int v = rule.minValue; v <= rule.maxValue; ++v) {
                  if (!rule.names || !(*rule.names)[v - rule.minValue].empty())
                    pickValues.push_back(v);
                }
              }
              for (size_t n = 0; n < pickValues.size(); ++n) {
                choices.push_back(OptionValueText(rule, pickValues[n]));
                if (pickValues[n] == curVal)
                  current = static_cast<int>(n);
              }
            };
            const int change =
                listable
                    ? PickerRow(ui, rule.key, ImVec2(m0.x + 12.0f * ui.s, ry),
                                ruleW, rule.label, valStr.c_str(), rowFocused,
                                fillChoices, &pickedChoice,
                                rowFocused && AcceptPressed(),
                                curVal == baseVal ? "(default)" : nullptr)
                    : OptionRow(ui, rule.key, ImVec2(m0.x + 12.0f * ui.s, ry),
                                ruleW, rule.label, valStr.c_str(), rowFocused,
                                curVal == baseVal ? "(default)" : nullptr);
            if (edIt != st.optionsValues.end()) {
              const float nw =
                  TextWidth(NobleStyle::TitleFont(), 18.0f * ui.s, rule.label);
              NobleUi::ChangedDot(ui.dl,
                                  ImVec2(m0.x + 22.0f * ui.s + nw + 9.0f * ui.s,
                                         ry + 16.0f * ui.s),
                                  ui.s);
            }
            if (change != 0 || pickedChoice >= 0) {
              int next = std::clamp(curVal, rule.minValue, rule.maxValue);
              if (pickedChoice >= 0 &&
                  pickedChoice < static_cast<int>(pickValues.size())) {
                next = pickValues[pickedChoice];
              } else if (rule.order) {
                const std::vector<int> &order = *rule.order;
                const int count = static_cast<int>(order.size());
                const auto at = std::find(order.begin(), order.end(), curVal);
                const int pos = at == order.end()
                                    ? (change > 0 ? -1 : 0)
                                    : static_cast<int>(at - order.begin());
                next = order[(pos + change + count) % count];
              } else {
                const int span = rule.maxValue - rule.minValue + 1;
                for (int step = 0; step < span; ++step) {
                  next = next + change > rule.maxValue   ? rule.minValue
                         : next + change < rule.minValue ? rule.maxValue
                                                         : next + change;
                  if (!rule.names ||
                      !(*rule.names)[next - rule.minValue].empty())
                    break;
                }
              }
              st.optionsRuleFocus = idxInCat;
              focusedRule = &rule;
              if (next == baseVal)
                st.optionsValues.erase(rule.key);
              else
                st.optionsValues[rule.key] = next;
            }
          }
          ui.dl->PopClipRect();
          if (!focusedRule && !activeRules.empty())
            focusedRule = &rules[activeRules.front()];

          // Column 3: Focused Rule Detail + Live Changes Summary +
          // Save/Reset/Delete Actions
          const float rX = m1.x + gap;
          const ImVec2 rBox0(rX, y), rBox1(rX + rightW, y + bodyH);
          ui.dl->AddRectFilled(rBox0, rBox1, IM_COL32(8, 13, 20, 205),
                               2.0f * ui.s);
          NobleUi::DrawTacticalFrame(ui.dl, rBox0, rBox1, ui.s, false, true,
                                     false, false);

          const float rPad = 14.0f * ui.s;
          float ry = rBox0.y + 12.0f * ui.s;
          const float rInnerW = rightW - rPad * 2.0f;
          if (focusedRule) {
            Text(ui, NobleStyle::SmallFont(), 12.5f, ImVec2(rBox0.x + rPad, ry),
                 kAccent, "RULE DETAILS");
            ry += 18.0f * ui.s;
            Text(ui, NobleStyle::TitleFont(), 20.0f, ImVec2(rBox0.x + rPad, ry),
                 kWhite,
                 Ellipsize(NobleStyle::TitleFont(), 20.0f * ui.s,
                           Upper(focusedRule->label), rInnerW)
                     .c_str());
            ry += 26.0f * ui.s;
            const int baseV =
                VariantRules::Read(*focusedRule, st.optionsDecoded.data());
            const auto edIt = st.optionsValues.find(focusedRule->key);
            const int curV =
                edIt != st.optionsValues.end() ? edIt->second : baseV;
            char cmp[96];
            std::snprintf(cmp, sizeof(cmp),
                          "DEFAULT: %s   \xE2\x86\x92   CURRENT: %s",
                          OptionValueText(*focusedRule, baseV).c_str(),
                          OptionValueText(*focusedRule, curV).c_str());
            Text(ui, NobleStyle::SmallFont(), 13.0f, ImVec2(rBox0.x + rPad, ry),
                 edIt != st.optionsValues.end() ? NobleUi::kAmber : kDim, cmp);
            ry += 22.0f * ui.s;
            const std::string help = OptionHelpText(*focusedRule);
            const float maxHelpH = 86.0f * ui.s;
            const std::string fittedHelp =
                FitWrapped(nullptr, 15.0f * ui.s, help, rInnerW, maxHelpH);
            Text(ui, nullptr, 15.0f, ImVec2(rBox0.x + rPad, ry),
                 IM_COL32(205, 218, 232, 240), fittedHelp.c_str(),
                 rBox0.x + rPad + rInnerW);
            ry += std::min(maxHelpH,
                           Font(nullptr)
                               ->CalcTextSizeA(15.0f * ui.s, FLT_MAX, rInnerW,
                                               fittedHelp.c_str())
                               .y) +
                  12.0f * ui.s;
          }

          ui.dl->AddLine(ImVec2(rBox0.x + rPad, ry), ImVec2(rBox1.x - rPad, ry),
                         IM_COL32(255, 255, 255, 36), 1.0f);
          ry += 10.0f * ui.s;
          char modHead[48];
          std::snprintf(modHead, sizeof(modHead), "ACTIVE CHANGES (%zu)",
                        st.optionsValues.size());
          Text(ui, NobleStyle::SmallFont(), 13.0f, ImVec2(rBox0.x + rPad, ry),
               st.optionsValues.empty() ? kDim : NobleUi::kAmber, modHead);
          ry += 20.0f * ui.s;
          const float btnAreaH = (canDelete ? 168.0f : 114.0f) * ui.s;
          const float summaryBottom = rBox1.y - btnAreaH - 18.0f * ui.s;
          int shownDiffs = 0;
          for (const VariantRules::Rule &rule : rules) {
            const auto it = st.optionsValues.find(rule.key);
            if (it == st.optionsValues.end())
              continue;
            if (ry + 18.0f * ui.s > summaryBottom) {
              Text(ui, NobleStyle::SmallFont(), 12.5f,
                   ImVec2(rBox0.x + rPad, ry), kDim, "...more rules modified");
              break;
            }
            char line[96];
            std::snprintf(line, sizeof(line), "\xE2\x80\xA2 %s: %s", rule.label,
                          OptionValueText(rule, it->second).c_str());
            Text(ui, NobleStyle::SmallFont(), 12.5f, ImVec2(rBox0.x + rPad, ry),
                 kWhite,
                 Ellipsize(NobleStyle::SmallFont(), 12.5f * ui.s, line, rInnerW)
                     .c_str());
            ry += 17.0f * ui.s;
            ++shownDiffs;
          }
          if (shownDiffs == 0 && ry + 18.0f * ui.s <= summaryBottom) {
            Text(ui, NobleStyle::SmallFont(), 13.0f, ImVec2(rBox0.x + rPad, ry),
                 kDim, "Using unmodified base rules.");
          }

          if (!st.optionsMessage.empty()) {
            Text(ui, NobleStyle::SmallFont(), 13.0f,
                 ImVec2(rBox0.x + rPad, rBox1.y - btnAreaH - 22.0f * ui.s),
                 IM_COL32(255, 196, 108, 255),
                 Ellipsize(NobleStyle::SmallFont(), 13.0f * ui.s,
                           st.optionsMessage, rInnerW)
                     .c_str());
          }

          float by = rBox1.y - btnAreaH;
          bool save =
              BigAction(ui, "mode_save", ImVec2(rBox0.x + rPad, by), rInnerW,
                        "SAVE MODE", "Save & select in mode list", true, true,
                        st.optionsRuleFocus == saveRow);
          by += 56.0f * ui.s;
          bool reset = BigAction(ui, "mode_reset", ImVec2(rBox0.x + rPad, by),
                                 rInnerW, "RESET ALL", "Restore base rules",
                                 false, true, st.optionsRuleFocus == resetRow);
          by += 56.0f * ui.s;
          bool erase =
              canDelete &&
              BigAction(ui, "mode_delete", ImVec2(rBox0.x + rPad, by), rInnerW,
                        "DELETE MODE", "Remove saved custom mode", false, true,
                        st.optionsRuleFocus == deleteRow);

          if (!typing && AcceptPressed()) {
            if (st.optionsRuleFocus == saveRow)
              save = true;
            if (st.optionsRuleFocus == resetRow)
              reset = true;
            if (canDelete && st.optionsRuleFocus == deleteRow)
              erase = true;
          }

          const auto reloadModes = [&](const std::filesystem::path &select) {
            if (forge) {
              st.forgeCustomVariant = select;
              if (st.screen == Screen::PreGameLobby) {
                st.lobbyVariantPath =
                    select.empty() ? ctx.forgeVariant.string() : select.string();
                st.lobbyVariantName =
                    select.empty() ? "Basic Editing" : std::string(st.optionsName);
                st.lobbyModeKey.clear();
                RefreshLobbyMode(ctx);
                AssignLobbyTeams(st);
                MasterServerClient::Get().UpdateLobbyAdvert(
                    st.lobbyMap, st.lobbyMode, st.lobbyVariantName,
                    st.lobbyDifficulty, st.lobbySkulls);
              }
              return;
            }
            ctx.variants = ScanVariants(ctx.reachDir, false);
            ctx.ffVariants = ScanVariants(ctx.reachDir, true);
            ctx.forgeMaps = ScanForgeMaps();
            ctx.forgeMapsScanned = true;
            int &index = ff ? st.ffVariant : st.variant;
            const std::vector<MenuVariant> &modes =
                ff ? ctx.ffVariants : ctx.variants;
            index = FindVariant(modes, ff ? L"hr_ff_standard_1set.bin"
                                          : L"slayer_054.bin");
            for (size_t i = 0; i < modes.size(); ++i)
              if (modes[i].path == select)
                index = static_cast<int>(i);
            if (st.screen == Screen::PreGameLobby && index >= 0 &&
                index < static_cast<int>(modes.size())) {
              st.lobbyVariantPath = modes[static_cast<size_t>(index)].path.string();
              st.lobbyVariantName = modes[static_cast<size_t>(index)].label;
              st.lobbyModeKey.clear();
              RefreshLobbyMode(ctx);
              AssignLobbyTeams(st);
              MasterServerClient::Get().UpdateLobbyAdvert(
                  st.lobbyMap, st.lobbyMode, st.lobbyVariantName,
                  st.lobbyDifficulty, st.lobbySkulls);
            }
          };
          if (reset) {
            st.optionsValues.clear();
            st.confirmAction.clear();
            st.optionsMessage = "Reset all options to base defaults.";
            MenuSounds::Back();
          }
          if (erase) {
            if (st.confirmAction != "delete") {
              st.confirmAction = "delete";
              st.optionsMessage = "Press DELETE again to remove permanently.";
              MenuSounds::Error();
            } else {
              std::error_code removeError;
              std::filesystem::remove(st.optionsFile, removeError);
              NOBLE_INFO("FrontEndMenu: game mode deleted: {}",
                         st.optionsFile.filename().string());
              reloadModes({});
              st.optionsOpen = false;
              MenuSounds::Back();
            }
          }
          if (save) {
            std::string file;
            for (const char c : std::string(st.optionsName)) {
              if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' ||
                  c == '-' || c == '_')
                file.push_back(c);
            }
            while (!file.empty() && file.back() == ' ')
              file.pop_back();
            if (file.empty()) {
              st.optionsMessage = "Give the mode a name first.";
              MenuSounds::Error();
            } else {
              VariantRules::Preset preset;
              preset.name = st.optionsName;
              preset.base = st.optionsBase;
              preset.values = st.optionsValues;
              const std::filesystem::path path =
                  VariantRules::Directory() / (file + ".json");
              std::error_code existsError;
              const bool wouldReplace =
                  std::filesystem::exists(path, existsError) &&
                  path != st.optionsFile;
              if (wouldReplace && st.confirmAction != "replace") {
                st.confirmAction = "replace";
                st.optionsMessage =
                    "Name exists — press SAVE again to overwrite.";
                MenuSounds::Error();
              } else if (VariantRules::Save(path, preset)) {
                NOBLE_INFO("FrontEndMenu: game mode saved: '{}' on '{}' with "
                           "{} change(s)",
                           preset.name, preset.base, preset.values.size());
                reloadModes(path);
                st.optionsOpen = false;
                MenuSounds::Click();
              } else {
                st.optionsMessage = "Couldn't save the game mode.";
                MenuSounds::Error();
              }
            }
          }
          if (back) {
            if (!st.optionsValues.empty()) {
              VariantRules::Preset preset;
              preset.name = st.optionsName[0] ? std::string(st.optionsName)
                                              : std::string("Custom Rules");
              preset.base = st.optionsBase;
              preset.values = st.optionsValues;
              const std::filesystem::path targetPath =
                  !st.optionsFile.empty()
                      ? st.optionsFile
                      : (VariantRules::Directory() /
                         (ff ? "_active_firefight.json"
                             : (forge ? "_active_forge.json"
                                      : "_active_custom.json")));
              if (VariantRules::Save(targetPath, preset)) {
                NOBLE_INFO("FrontEndMenu: auto-applied game options '{}' on "
                           "'{}' with {} change(s)",
                           preset.name, preset.base, preset.values.size());
                reloadModes(targetPath);
              }
            } else if (!st.optionsFile.empty()) {
              const std::string fname = st.optionsFile.filename().string();
              if (!fname.empty() && fname[0] == '_') {
                std::error_code removeError;
                std::filesystem::remove(st.optionsFile, removeError);
                reloadModes(VariantBasePath(ctx.reachDir, st.optionsBase));
              } else {
                VariantRules::Preset preset;
                preset.name = st.optionsName[0] ? std::string(st.optionsName)
                                                : st.optionsFile.stem().string();
                preset.base = st.optionsBase;
                preset.values = st.optionsValues;
                if (VariantRules::Save(st.optionsFile, preset))
                  reloadModes(st.optionsFile);
              }
            }
            st.optionsOpen = false;
            MenuSounds::Back();
          }
          DrawHints(ui,
                    {{Prompt::Accept, "Select / Apply"},
                     {Prompt::Back, "Back"},
                     {Prompt::Change, "Change"},
                     {Prompt::Tabs, "Category"}},
                    {{InputPrompts::Button::X, "X", "Changed Only"}});
        }

        void DrawCampaignScreen(OverlayContext & ctx, const Ui &ui, float left,
                                float top, FrontEndChoice &choice, bool &done,
                                bool back) {
          MenuState &st = ctx.st;
          DrawTopTelemetryBar(ui, "CAMPAIGN");
          DrawTitle(ui, ImVec2(left, top), "CAMPAIGN", "MAP SELECT");
          float tabsEnd = 0.0f;
          if (DrawSourceTabs(
                  ui, ImVec2(left + 26.0f * ui.s, top + 72.0f * ui.s),
                  st.campaignSource, static_cast<int>(ctx.campaignMods.size()),
                  -1, &tabsEnd)) {
            st.focusRow = 0;
            st.mapDrawerKey.clear();
            st.mapDrawerFocus = -1;
            st.mapGridScrollY = 0.0f;
          }
          const bool modded = st.campaignSource == 1;
          // Search sits right of the tabs, but is clamped inside the screen (it used to run off the right edge on
          // narrow/4:3 UIs and was clipped).
          const float campScreenRight = ui.origin.x + ui.size.x * 0.94f;
          float searchX = std::max(left + 26.0f * ui.s + 240.0f * ui.s,
                                   tabsEnd + 10.0f * ui.s);
          float searchW = 220.0f * ui.s;
          if (searchX + searchW > campScreenRight - 8.0f * ui.s)
            searchX = std::max(left + 26.0f * ui.s, campScreenRight - searchW - 8.0f * ui.s);
          searchW = std::min(searchW, std::max(80.0f * ui.s, campScreenRight - searchX - 8.0f * ui.s));
          if (DrawSearchBox(ui, "##mission_search",
                            ImVec2(searchX, top + 72.0f * ui.s), searchW,
                            st.mapSearch, sizeof(st.mapSearch),
                            "Search missions & mods"))
            st.mapSearchRow = 0;
          if (ImGui::IsItemActive())
            st.focusRow = 0;

          const std::vector<MapTileGroup> groups = BuildMapTileGroups(
              ctx, modded, false, ModMode::Campaign, ctx.campaignMods,
              kMissions, Count(kMissions), "m10", false, st.mapSearch);
          const bool searching = st.mapSearch[0] != 0;
          int &pick = searching ? st.mapSearchRow
                                : (modded ? st.campaignModded : st.mission);

          static Screen s_campLastScr = Screen::Main;
          static int s_campLastSrc = -1;
          const bool firstEnter = (s_campLastScr != st.screen ||
                                   s_campLastSrc != st.campaignSource);
          s_campLastScr = st.screen;
          s_campLastSrc = st.campaignSource;

          const float gridX = left + 24.0f * ui.s;
          const float gridTop = top + 114.0f * ui.s;
          const float rightMax = ui.origin.x + ui.size.x * 0.94f;
          const float gridW = (rightMax - gridX) * 0.56f;
          const float gridBot = ui.origin.y + ui.size.y - 68.0f * ui.s;

          std::filesystem::path campDummyMvar;
          const TacticalGridResult gridRes = DrawTacticalMapGrid(
              ui, st, ImVec2(gridX, gridTop), ImVec2(gridW, gridBot - gridTop),
              groups, pick, campDummyMvar, st.focusRow == 0,
              searching ? "Nothing matches your search"
                        : "No Reach campaign mods installed",
              "BASE MISSION", firstEnter, ModMode::Campaign);
          if (gridRes.pickedBaseOrVariant) {
            st.mapDrawerKey.clear();
            st.mapDrawerFocus = -1;
            st.focusRow = 1;
          } else if (gridRes.moveFocusRight) {
            st.focusRow = 1;
          }

          const MapTileGroup *selGrp =
              groups.empty()
                  ? nullptr
                  : &groups[std::clamp(pick, 0,
                                       static_cast<int>(groups.size()) - 1)];
          const MapEntry *mission =
              (selGrp && selGrp->base.enabled) ? &selGrp->base : nullptr;
          const ModEntry *mod = mission ? mission->mod : nullptr;

          // Right Detail & Launch Card (anchored below DrawParty cleanly)
          const float px = gridX + gridW + 24.0f * ui.s;
          const float pw = rightMax - px;
          const float cardTop = std::max(gridTop, ui.origin.y + 150.0f * ui.s);
          const float cardBot = gridBot;
          const ImVec2 rc0(px, cardTop), rc1(px + pw, cardBot);
          ui.dl->AddRectFilled(rc0, rc1, IM_COL32(8, 14, 22, 215), 2.0f * ui.s);
          NobleUi::DrawTacticalFrame(ui.dl, rc0, rc1, ui.s, st.focusRow > 0,
                                     true, false, false);

          const float pad = 16.0f * ui.s;
          const float innerW = pw - pad * 2.0f;
          const float actionsY = cardBot - 62.0f * ui.s;
          const float stripY = actionsY - LobbyStripHeight(ui) - 8.0f * ui.s;
          const float controlsH =
              98.0f * ui.s + 8.0f * ui.s + 36.0f * ui.s + 30.0f * ui.s;
          const float metaFixedH = 231.0f * ui.s;
          const float previewH =
              std::clamp(stripY - cardTop - controlsH - metaFixedH,
                         68.0f * ui.s, innerW * 292.0f / 668.0f);

          float y = DrawMapDetailHeaderCard(ui, rc0, pw, previewH,
                                            selGrp ? &selGrp->base : nullptr,
                                            nullptr, "CAMPAIGN");
          const float diffY = y;
          y += DifficultyTiles(ui, "diff", ImVec2(px + pad, y), innerW,
                               st.difficulty, st.focusRow == 1) +
               8.0f * ui.s;
          const float maxPlayersY = y;
          static const bool s_dbgCampaignLayout =
              wcsstr(GetCommandLineW(), L"-menu-debug-layout") != nullptr;
          if (s_dbgCampaignLayout) {
            NOBLE_INFO("CampaignLayout: cardTop={:.0f} diffY={:.0f} maxPlayersY={:.0f} cardBot={:.0f} px={:.0f} innerW={:.0f} actionY={:.0f} valueX={:.0f}",
                       cardTop, diffY, maxPlayersY, cardBot, px, innerW, actionsY,
                       px + pad + innerW * 0.48f);
          }
          // HOST CO-OP size: campaign lobbies used to have no player-count control. The row used to
          // offer 4/8/12/16, but 5-player campaign co-op is not proven and crashes at 5 (Tyler,
          // 2026-11-15 Phase-2 Stage 2 report). 4 is the only proven campaign count, so the row is
          // clamped to it until the session/membership work proves more (see
          // project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md). Do not re-open without a real 5-player test.
          static const int kCampCaps[] = {4};
          constexpr int kNumCampCaps = 1;
          int campCapIdx = kNumCampCaps - 1;
          for (int ci = 0; ci < kNumCampCaps; ++ci) {
            if (kCampCaps[ci] >= st.campaignMaxPlayers) {
              campCapIdx = ci;
              break;
            }
          }
          char campCapText[32];
          std::snprintf(campCapText, sizeof(campCapText), "%d PLAYERS",
                        kCampCaps[campCapIdx]);
          int campCapPicked = -1;
          const int campCapStep = PickerRow(
              ui, "camp_players", ImVec2(px + pad, y), innerW, "MAX PLAYERS",
              campCapText, st.focusRow == 2,
              [&](std::vector<std::string> &choices, int &current) {
                for (int ci = 0; ci < kNumCampCaps; ++ci)
                  choices.push_back(std::to_string(kCampCaps[ci]) +
                                    " PLAYERS");
                current = campCapIdx;
              },
              &campCapPicked, st.focusRow == 2 && AcceptPressed());
          if (campCapStep)
            campCapIdx =
                (campCapIdx + campCapStep + kNumCampCaps) % kNumCampCaps;
          if (campCapPicked >= 0)
            campCapIdx = campCapPicked;
          if (campCapStep || campCapPicked >= 0)
            st.campaignMaxPlayers = kCampCaps[campCapIdx];
          y += 36.0f * ui.s;
          const std::string skullText = SkullSummary(st.skulls);
          if (OptionRow(ui, "skulls", ImVec2(px + pad, y), innerW, "SKULLS",
                        skullText.c_str(), st.focusRow == 3, nullptr, false)) {
            st.screen = Screen::Skulls;
          }
          ImGui::SetCursorScreenPos(ImVec2(px + pad, y));
          if (ImGui::InvisibleButton("##skullrow",
                                     ImVec2(innerW, 32.0f * ui.s)))
            st.screen = Screen::Skulls;
          if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ui.dl->AddRectFilled(
                ImVec2(px + pad + innerW * 0.48f, y + 2.0f * ui.s),
                ImVec2(px + pw - pad, y + 30.0f * ui.s),
                IM_COL32(143, 216, 255, 28), 2.0f * ui.s);
            ui.dl->AddRectFilled(
                ImVec2(px + pad + innerW * 0.48f, y + 29.0f * ui.s),
                ImVec2(px + pw - pad, y + 31.0f * ui.s),
                IM_COL32(143, 216, 255, 140));
          }
          y += 36.0f * ui.s;
          const float stripW =
              SkullStrip(ui, ImVec2(px + pad + 8.0f * ui.s, y + 2.0f * ui.s),
                         20.0f * ui.s, st.skulls, 13);
          char mult[64];
          std::snprintf(mult, sizeof(mult), "Score multiplier  x%.2f",
                        ScoreMultiplier(st.difficulty, st.skulls));
          Text(ui, nullptr, 15.5f,
               ImVec2(px + pad + 8.0f * ui.s +
                          (stripW > 0.0f ? stripW + 12.0f * ui.s : 0.0f),
                      y + 3.0f * ui.s),
               kDim, mult);
          DrawLobbyControls(ui, ImVec2(px + pad, stripY), innerW, st);

          const bool canStart = mission && mission->enabled;
          const float bw = (innerW - 10.0f * ui.s) * 0.5f;
          std::string sub = mission
                                ? std::string(mission->name) + " \xC2\xB7 " +
                                      kDifficultyNames[st.difficulty]
                                : std::string("Pick a mission");
          const bool startSolo =
              ((BigAction(ui, "start", ImVec2(px + pad, actionsY), bw,
                          "PLAY SOLO", sub.c_str(), true, canStart,
                          st.focusRow == 4) ||
                (st.focusRow == 4 && AcceptPressed())) &&
               canStart) ||
              (AutoPlayNow() && canStart);
          const char *coopSub =
              st.lobbyNetMode == 1
                  ? (st.lobbyPrivacy == 0 ? "Public Online P2P"
                                          : "Private Online P2P")
                  : "Local LAN Co-op";
          const bool hostCoop =
              ((BigAction(ui, "coop",
                          ImVec2(px + pad + bw + 10.0f * ui.s, actionsY),
                          innerW - bw - 10.0f * ui.s, "HOST CO-OP", coopSub,
                          true, canStart, st.focusRow == 5) ||
                (st.focusRow == 5 && AcceptPressed())) &&
               canStart);
          const std::string launchScenario =
              mission
                  ? (mission->launchScenario.empty() ? mission->scenario
                                                     : mission->launchScenario)
                  : std::string();
          if (hostCoop) {
            SetupHostLobby(st, launchScenario, "campaign", "", "Campaign",
                           st.difficulty, st.skulls, st.campaignMaxPlayers,
                           mission->mod, mission->mapOverride, mission->name,
                           mission->imageLoading, mission->imageThumbnail);
          } else if (startSolo) {
            FrontEndChoice req;
            req.play = true;
            req.host = false;
            req.mode = "campaign";
            req.map = launchScenario;
            req.mapOverride = mission->mapOverride;
            req.mapTitle = mission->name;
            req.mapImage = mission->imageLoading;
            req.mapThumb = mission->imageThumbnail;
            req.difficulty = st.difficulty;
            req.skulls = st.skulls;
            if (mod) {
              req.modFolder = mod->folder;
              req.modTitle = mod->title;
            }
            RequestMatchLaunch(choice, done, req);
          }
          if (!g_pickerBlocksNav && st.focusRow > 0) {
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) ||
                ImGui::IsKeyPressed(ImGuiKey_Tab) || UiPad::Down())
              st.focusRow = std::min(5, st.focusRow + 1);
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up())
              st.focusRow = std::max(1, st.focusRow - 1);
            if (st.focusRow == 4 &&
                (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || UiPad::Right())) {
              st.focusRow = 5;
              MenuSounds::Scroll();
            }
            if (st.focusRow == 5 &&
                (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || UiPad::Left())) {
              st.focusRow = 4;
              MenuSounds::Scroll();
            }
          }
          if (st.focusRow == 3 && AcceptPressed())
            st.screen = Screen::Skulls;
          if (!ImGui::GetIO().WantTextInput && UiPad::ActionX()) {
            st.screen = Screen::Skulls;
            MenuSounds::Click();
          }
          if (back) {
            if (st.focusRow > 0)
              st.focusRow = 0;
            else if (!st.mapDrawerKey.empty()) {
              st.mapDrawerKey.clear();
              st.mapDrawerFocus = -1;
            } else
              GoBackScreen(st, Screen::Main);
          }
          DrawHints(ui,
                    {{Prompt::Accept, "Select / Expand"},
                     {Prompt::Back, "Back"},
                     {Prompt::Change, "Change"},
                     {Prompt::Tabs, "Vanilla / Modded"}},
                    {{InputPrompts::Button::X, "X", "Skulls"}});
        }

        void DrawSkullsScreen(OverlayContext & ctx, const Ui &ui, float left,
                              float top, const SettingsData &settings,
                              bool back) {
          MenuState &st = ctx.st;
          DrawTitle(ui, ImVec2(left, top), "SKULLS", "CAMPAIGN");
          const float gx = left + 46.0f * ui.s;
          float gy = top + 92.0f * ui.s;
          const float cell = 78.0f * ui.s, icon = 64.0f * ui.s;
          auto section = [&](const char *name, float y) {
            Text(ui, NobleStyle::TitleFont(), 20.0f, ImVec2(gx, y), kWhite,
                 name);
            ui.dl->AddLine(ImVec2(gx, y + 26.0f * ui.s),
                           ImVec2(gx + cell * 5.0f, y + 26.0f * ui.s),
                           IM_COL32(255, 255, 255, 60));
          };
          if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight) ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadLStickRight))
            st.skull = std::min(kSkullCount - 1, st.skull + 1);
          if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft) ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadLStickLeft))
            st.skull = std::max(0, st.skull - 1);
          if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadDpadDown) ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadLStickDown))
            st.skull = std::min(kSkullCount - 1, st.skull + 5);
          if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadDpadUp) ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadLStickUp))
            st.skull = std::max(0, st.skull - 5);
          bool toggle =
              AcceptPressed() || ImGui::IsKeyPressed(ImGuiKey_Space, false);

          section("Primary Skulls", gy);
          gy += 36.0f * ui.s;
          int primaryIndex = 0, secondaryIndex = 0;
          float secondaryTop = gy + cell * 2.0f + 14.0f * ui.s;
          for (int i = 0; i < kSkullCount; ++i) {
            const Skull &sk = kSkulls[i];
            int idx = sk.primary ? primaryIndex++ : secondaryIndex++;
            const float baseY = sk.primary ? gy : secondaryTop + 36.0f * ui.s;
            const ImVec2 a(gx + (idx % 5) * cell, baseY + (idx / 5) * cell);
            ImGui::SetCursorScreenPos(a);
            ImGui::PushID(i);
            if (ImGui::InvisibleButton("##skull", ImVec2(icon, icon))) {
              st.skull = i;
              toggle = true;
            }
            if (ImGui::IsItemHovered() && ImGui::GetIO().MouseDelta.x != 0.0f)
              st.skull = i;
            ImGui::PopID();
            const bool on = (st.skulls & (1ull << sk.bit)) != 0;
            if (i == st.skull)
              ui.dl->AddCircle(ImVec2(a.x + icon * 0.5f, a.y + icon * 0.5f),
                               icon * 0.56f, kWhite, 0, 2.0f * ui.s);
            if (on)
              ui.dl->AddCircleFilled(
                  ImVec2(a.x + icon * 0.5f, a.y + icon * 0.5f), icon * 0.52f,
                  IM_COL32(230, 160, 90, 70));
            UiTexture tex = UiArt::SkullIcon(sk.icon);
            const ImU32 tint = on ? IM_COL32(255, 205, 160, 255)
                                  : IM_COL32(255, 255, 255, 105);
            if (tex)
              ui.dl->AddImage(tex.id, a, ImVec2(a.x + icon, a.y + icon),
                              ImVec2(0, 0), ImVec2(1, 1), tint);
            else
              Text(ui, nullptr, 14.0f, ImVec2(a.x, a.y + icon * 0.4f),
                   on ? kWhite : kDim, sk.name, a.x + icon);
          }
          section("Secondary Skulls", secondaryTop);
          if (toggle) {
            st.skulls ^= 1ull << kSkulls[st.skull].bit;
            {
              SettingsData d = Settings::Get().Snapshot();
              d.campaignSkulls = st.skulls;
              Settings::Get().Update(d);
            }
            MenuSounds::Click();
          }

          const Skull &sel = kSkulls[st.skull];
          const float dx = ui.origin.x + ui.size.x * 0.60f;
          float dy = top + 70.0f * ui.s;
          UiTexture big = UiArt::SkullIcon(sel.icon);
          const float bs = 170.0f * ui.s;
          if (big)
            ui.dl->AddImage(big.id, ImVec2(dx + 40.0f * ui.s, dy),
                            ImVec2(dx + 40.0f * ui.s + bs, dy + bs));
          dy += bs + 18.0f * ui.s;
          // Dark panel behind the text: it sits on the background art, and the
          // dim description was unreadable over it.
          ui.dl->AddRectFilled(ImVec2(dx - 14.0f * ui.s, dy - 8.0f * ui.s),
                               ImVec2(dx + 314.0f * ui.s, dy + 176.0f * ui.s),
                               IM_COL32(8, 14, 22, 190), 3.0f * ui.s);
          Text(ui, NobleStyle::HeroFont(), 34.0f, ImVec2(dx, dy), kWhite,
               sel.name);
          dy += 42.0f * ui.s;
          Text(ui, nullptr, 17.0f, ImVec2(dx, dy), IM_COL32(205, 218, 230, 255),
               sel.description, dx + 300.0f * ui.s);
          dy += 74.0f * ui.s;
          const bool selOn = (st.skulls & (1ull << sel.bit)) != 0;
          Text(ui, nullptr, 17.0f, ImVec2(dx, dy), kWhite,
               selOn ? "Status: On" : "Status: Off");
          dy += 24.0f * ui.s;
          char mult[64];
          if (sel.multiplier > 1.0f)
            std::snprintf(mult, sizeof(mult), "Score bonus: +%.1f",
                          sel.multiplier);
          else
            std::snprintf(mult, sizeof(mult), "No score bonus");
          Text(ui, nullptr, 17.0f, ImVec2(dx, dy), IM_COL32(205, 218, 230, 255),
               mult);

          const float by = ui.origin.y + ui.size.y - 104.0f * ui.s;
          ui.dl->AddRectFilledMultiColor(
              ImVec2(left + 40.0f * ui.s, by),
              ImVec2(ui.origin.x + ui.size.x * 0.80f, by + 40.0f * ui.s),
              IM_COL32(40, 52, 64, 210), IM_COL32(40, 52, 64, 60),
              IM_COL32(40, 52, 64, 60), IM_COL32(40, 52, 64, 210));
          Text(ui, NobleStyle::TitleFont(), 24.0f,
               ImVec2(left + 52.0f * ui.s, by + 6.0f * ui.s), kWhite,
               settings.playerName.c_str());
          char total[64];
          std::snprintf(total, sizeof(total), "x%.2f",
                        ScoreMultiplier(st.difficulty, st.skulls));
          Text(ui, NobleStyle::TitleFont(), 24.0f,
               ImVec2(ui.origin.x + ui.size.x * 0.80f - 90.0f * ui.s,
                      by + 6.0f * ui.s),
               kWhite, total);
          if (back)
            GoBackScreen(st, Screen::Campaign);
          DrawHints(ui, {{Prompt::Accept, "Toggle"}, {Prompt::Back, "Back"}});
        }

        void DrawFirefightCustomForgeScreen(
            OverlayContext & ctx, const Ui &ui, float left, float top,
            FrontEndChoice &choice, bool &done, bool back) {
          MenuState &st = ctx.st;
          const SettingsData settings = Settings::Get().Snapshot();
          const bool ff = st.screen == Screen::Firefight;
          const bool forge = st.screen == Screen::Forge;
          if (st.playlistEditOpen) {
            // Playlist editor: choose a map and game type, add them, repeat;
            // the list is on the right.
            DrawTopTelemetryBar(ui, "PLAYLIST");
            DrawTitle(ui, ImVec2(left, top), "PLAYLIST", "MULTIPLAYER");
            const std::vector<MapEntry> allMaps = AllMultiplayerMaps(ctx);
            const float px = left + 18.0f * ui.s, pw = ui.size.x * 0.42f;
            const float panelTop = top + 64.0f * ui.s;
            const float lx = px + pw + 30.0f * ui.s,
                        lw = (ui.origin.x + ui.size.x * 0.93f) - lx;
            const float panelBot = ui.origin.y + ui.size.y * 0.90f;

            ui.dl->AddRectFilled(
                ImVec2(px - 12.0f * ui.s, panelTop - 10.0f * ui.s),
                ImVec2(px + pw + 12.0f * ui.s, panelBot),
                IM_COL32(8, 15, 24, 200), 2.0f * ui.s);
            NobleUi::DrawTacticalFrame(
                ui.dl, ImVec2(px - 12.0f * ui.s, panelTop - 10.0f * ui.s),
                ImVec2(px + pw + 12.0f * ui.s, panelBot), ui.s, true, true,
                false, false);

            ui.dl->AddRectFilled(
                ImVec2(lx - 12.0f * ui.s, panelTop - 10.0f * ui.s),
                ImVec2(lx + lw + 12.0f * ui.s, panelBot),
                IM_COL32(8, 15, 24, 200), 2.0f * ui.s);
            NobleUi::DrawTacticalFrame(
                ui.dl, ImVec2(lx - 12.0f * ui.s, panelTop - 10.0f * ui.s),
                ImVec2(lx + lw + 12.0f * ui.s, panelBot), ui.s, false, true,
                false, false);

            float y = panelTop + 4.0f * ui.s;
            DrawSearchBox(ui, "##playlist_name", ImVec2(px, y), pw,
                          st.playlistName, sizeof(st.playlistName),
                          "Playlist name");
            y += 44.0f * ui.s;
            const bool typing = ImGui::GetIO().WantTextInput;
            const bool canDeleteList =
                !st.playlistEdit.file.empty() &&
                st.playlistEdit.file.parent_path() == Playlists::Directory();
            constexpr int kMapRow = 0, kTypeRow = 1, kAddRow = 2,
                          kShuffleRow = 3, kRandomRow = 4, kAutoRow = 5,
                          kSaveRow = 6, kDeleteRow = 7;
            const int kRows = canDeleteList ? 8 : 7;
            if (!typing &&
                (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || UiPad::Down())) {
              st.playlistEditRow = (st.playlistEditRow + 1) % kRows;
              MenuSounds::Scroll();
            }
            if (!typing &&
                (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up())) {
              st.playlistEditRow = (st.playlistEditRow + kRows - 1) % kRows;
              MenuSounds::Scroll();
            }
            if (!typing && canDeleteList &&
                (st.playlistEditRow == kSaveRow ||
                 st.playlistEditRow == kDeleteRow) &&
                (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) ||
                 ImGui::IsKeyPressed(ImGuiKey_RightArrow) || UiPad::Left() ||
                 UiPad::Right())) {
              st.playlistEditRow =
                  (st.playlistEditRow == kSaveRow) ? kDeleteRow : kSaveRow;
              MenuSounds::Scroll();
            }
            st.playlistEditMap =
                allMaps.empty()
                    ? 0
                    : std::clamp(st.playlistEditMap, 0,
                                 static_cast<int>(allMaps.size()) - 1);
            const MapEntry *editMap =
                allMaps.empty() ? nullptr : &allMaps[st.playlistEditMap];
            std::vector<const MenuVariant *> types;
            for (const MenuVariant &v : ctx.variants) {
              if (editMap &&
                  (editMap->mod || v.type.empty() ||
                   MapGameTypes::Supports(editMap->scenario, v.type)))
                types.push_back(&v);
            }
            st.playlistEditVariant =
                types.empty() ? 0
                              : std::clamp(st.playlistEditVariant, 0,
                                           static_cast<int>(types.size()) - 1);
            const int mapStep =
                OptionRow(ui, "pl_map", ImVec2(px, y), pw, "MAP",
                          editMap ? editMap->name.c_str() : "-",
                          !typing && st.playlistEditRow == kMapRow);
            if (mapStep != 0 && !allMaps.empty())
              st.playlistEditMap = (st.playlistEditMap + mapStep +
                                    static_cast<int>(allMaps.size())) %
                                   static_cast<int>(allMaps.size());
            y += 36.0f * ui.s;
            const int typeStep = OptionRow(
                ui, "pl_type", ImVec2(px, y), pw, "GAME TYPE",
                types.empty() ? "-"
                              : types[st.playlistEditVariant]->label.c_str(),
                !typing && st.playlistEditRow == kTypeRow);
            if (typeStep != 0 && !types.empty())
              st.playlistEditVariant = (st.playlistEditVariant + typeStep +
                                        static_cast<int>(types.size())) %
                                       static_cast<int>(types.size());
            y += 40.0f * ui.s;
            const float bw = (pw - 10.0f * ui.s) * 0.5f;
            bool add =
                BigAction(ui, "pl_add", ImVec2(px, y), pw, "ADD TO PLAYLIST",
                          "Adds this map and game type", true,
                          editMap != nullptr && !types.empty(),
                          st.playlistEditRow == kAddRow);
            y += 62.0f * ui.s;
            const int shuffleStep =
                OptionRow(ui, "pl_shuffle", ImVec2(px, y), pw, "ORDER",
                          st.playlistEdit.shuffle ? "SHUFFLED" : "IN ORDER",
                          !typing && st.playlistEditRow == kShuffleRow);
            if (shuffleStep != 0)
              st.playlistEdit.shuffle = !st.playlistEdit.shuffle;
            y += 36.0f * ui.s;
            const int randomStep = OptionRow(
                ui, "pl_random", ImVec2(px, y), pw, "RANDOM GAME TYPE",
                st.playlistEdit.randomType ? "ON" : "OFF",
                !typing && st.playlistEditRow == kRandomRow);
            if (randomStep != 0)
              st.playlistEdit.randomType = !st.playlistEdit.randomType;
            y += 36.0f * ui.s;
            static const int kAutoChoices[] = {0, 15, 30, 60};
            int autoAt = 0;
            for (int i = 0; i < 4; ++i)
              if (kAutoChoices[i] == st.playlistEdit.autoStart)
                autoAt = i;
            const std::string autoText =
                st.playlistEdit.autoStart > 0
                    ? std::to_string(st.playlistEdit.autoStart) + " S"
                    : std::string("OFF");
            const int autoStep = OptionRow(
                ui, "pl_auto", ImVec2(px, y), pw, "NEXT MATCH STARTS BY ITSELF",
                autoText.c_str(), !typing && st.playlistEditRow == kAutoRow);
            if (autoStep != 0)
              st.playlistEdit.autoStart =
                  kAutoChoices[(autoAt + autoStep + 4) % 4];
            y += 42.0f * ui.s;
            bool save =
                BigAction(ui, "pl_save", ImVec2(px, y), canDeleteList ? bw : pw,
                          "SAVE PLAYLIST", "Then pick it on Custom Game", true,
                          !st.playlistEdit.entries.empty(),
                          st.playlistEditRow == kSaveRow);
            bool eraseList =
                canDeleteList &&
                BigAction(ui, "pl_delete", ImVec2(px + bw + 10.0f * ui.s, y),
                          bw, "DELETE", "Removes this playlist", false, true,
                          st.playlistEditRow == kDeleteRow);
            if (!typing && AcceptPressed() && canDeleteList &&
                st.playlistEditRow == kDeleteRow)
              eraseList = true;
            if (eraseList) {
              if (st.confirmAction != "delete") {
                st.confirmAction = "delete";
                st.playlistMessage =
                    "Press DELETE again to remove this playlist.";
                MenuSounds::Error();
              } else {
                std::error_code removeError;
                std::filesystem::remove(st.playlistEdit.file, removeError);
                NOBLE_INFO("FrontEndMenu: playlist deleted: {}",
                           st.playlistEdit.file.filename().string());
                RefreshPlaylists(ctx);
                st.playlistChoice = -1;
                st.playlistEditOpen = false;
                MenuSounds::Back();
              }
            }
            if (!typing && AcceptPressed()) {
              if (st.playlistEditRow == kAddRow)
                add = editMap != nullptr && !types.empty();
              if (st.playlistEditRow == kSaveRow)
                save = !st.playlistEdit.entries.empty();
              if (st.playlistEditRow == kShuffleRow)
                st.playlistEdit.shuffle = !st.playlistEdit.shuffle;
              if (st.playlistEditRow == kRandomRow)
                st.playlistEdit.randomType = !st.playlistEdit.randomType;
            }
            if (add) {
              Playlists::Entry entry;
              entry.map = editMap->scenario;
              entry.variant =
                  types[st.playlistEditVariant]->path.filename().string();
              entry.mod = editMap->mod
                              ? editMap->mod->folder.filename().string()
                              : std::string();
              st.playlistEdit.entries.push_back(std::move(entry));
              st.playlistEditEntry =
                  static_cast<int>(st.playlistEdit.entries.size()) - 1;
              MenuSounds::Click();
            }
            // The list so far. Y or a click on a row takes it out again.
            Text(ui, NobleStyle::TitleFont(), 18.0f,
                 ImVec2(lx, panelTop + 4.0f * ui.s), kAccent,
                 ("ENTRIES (" + std::to_string(st.playlistEdit.entries.size()) +
                  ")")
                     .c_str());
            std::vector<ListItem> rows;
            for (const Playlists::Entry &entry : st.playlistEdit.entries) {
              std::string mapName = entry.map, typeName = entry.variant;
              for (const MapEntry &m : allMaps)
                if (m.scenario == entry.map) {
                  mapName = m.name;
                  break;
                }
              for (const MenuVariant &v : ctx.variants)
                if (v.path.filename().string() == entry.variant)
                  typeName = v.label;
              rows.push_back({mapName + " - " + typeName});
            }
            if (rows.empty())
              rows.push_back({"Nothing added yet", false, false});
            const int gone = DrawList(
                ui, "playlist_rows", ImVec2(lx, panelTop + 34.0f * ui.s), lw,
                rows, st.playlistEditEntry, false, 32.0f, 14, true);
            int removeRow = gone;
            if (!typing && ImGui::IsKeyPressed(ImGuiKey_GamepadFaceUp, false))
              removeRow = st.playlistEditEntry;
            if (removeRow >= 0 &&
                removeRow < static_cast<int>(st.playlistEdit.entries.size())) {
              st.playlistEdit.entries.erase(st.playlistEdit.entries.begin() +
                                            removeRow);
              MenuSounds::Back();
            }
            std::error_code listExistsError;
            const std::filesystem::path listTarget =
                Playlists::PathFor(st.playlistName);
            if (save && !listTarget.empty() &&
                std::filesystem::exists(listTarget, listExistsError) &&
                listTarget != st.playlistEdit.file &&
                st.confirmAction != "replace") {
              save = false;
              st.confirmAction = "replace";
              st.playlistMessage = "A playlist with this name exists. Press "
                                   "SAVE again to replace it.";
              MenuSounds::Error();
            }
            if (save) {
              st.playlistEdit.name = st.playlistName;
              std::filesystem::path savedAs;
              if (Playlists::Save(st.playlistEdit, savedAs)) {
                NOBLE_INFO("FrontEndMenu: playlist saved: '{}' with {} entries",
                           st.playlistEdit.name,
                           st.playlistEdit.entries.size());
                RefreshPlaylists(ctx);
                st.playlistChoice = -1;
                for (size_t i = 0; i < st.playlists.size(); ++i)
                  if (st.playlists[i].file == savedAs)
                    st.playlistChoice = static_cast<int>(i);
                st.playlistEditOpen = false;
                MenuSounds::Click();
              } else {
                st.playlistMessage = "Give it a name first.";
                MenuSounds::Error();
              }
            }
            if (!st.playlistMessage.empty())
              Text(ui, nullptr, 15.0f, ImVec2(px, y + 60.0f * ui.s),
                   IM_COL32(255, 190, 120, 255), st.playlistMessage.c_str());
            if (back) {
              st.playlistEditOpen = false;
              MenuSounds::Back();
            }
            DrawHints(ui,
                      {{Prompt::Accept, "Select"},
                       {Prompt::Back, "Back"},
                       {Prompt::Change, "Change"}},
                      {{InputPrompts::Button::Y, "CLICK", "Remove Entry"}});
            return;
          }
          if (st.optionsOpen) {
            DrawTacticalGameOptions(ctx, ui, left, top, ff, forge, back);
            return;
          }
          if (st.variantPickerOpen && !forge) {
            DrawTopTelemetryBar(ui, "GAME TYPE");
            const std::vector<MenuVariant> &list =
                ff ? ctx.ffVariants : ctx.variants;
            int &current = ff ? st.ffVariant : st.variant;
            DrawTitle(ui, ImVec2(left, top), "GAME TYPE",
                      ff ? "FIREFIGHT" : "MULTIPLAYER");
            DrawParty(ui, settings.playerName, "SELECTING GAME TYPE");
            Text(ui, nullptr, 17.0f,
                 ImVec2(left + 30.0f * ui.s, top + 72.0f * ui.s), kDim,
                 st.variantPickerModded
                     ? "Every game type, on a modded map."
                     : ("Game types " + st.variantPickerMapName + " can host.")
                           .c_str());

            const float listX = left + 24.0f * ui.s;
            const float listTop = top + 108.0f * ui.s;
            const float listBot = ui.origin.y + ui.size.y - 68.0f * ui.s;
            const float rightMax = ui.origin.x + ui.size.x * 0.94f;
            const float listW = (rightMax - listX) * 0.48f;
            const ImVec2 lc0(listX, listTop), lc1(listX + listW, listBot);
            ui.dl->AddRectFilled(lc0, lc1, IM_COL32(8, 14, 22, 200),
                                 2.0f * ui.s);
            NobleUi::DrawTacticalFrame(ui.dl, lc0, lc1, ui.s, true, true, false,
                                       false);

            const float rowH = 38.0f;
            const int rows = std::max(
                4, static_cast<int>((listBot - listTop - 24.0f * ui.s) /
                                    (rowH * ui.s)));
            VariantRow shown;
            const MenuVariant *picked = DrawVariantList(
                ui, st, list, st.variantPickerMap, ff || st.variantPickerModded,
                ImVec2(listX + 12.0f * ui.s, listTop + 12.0f * ui.s),
                listW - 24.0f * ui.s, rowH, rows, true, shown);
            if (!shown.mode.empty()) {
              const float infoX = listX + listW + 24.0f * ui.s;
              const float infoW = rightMax - infoX;
              const float infoTop =
                  std::max(listTop, ui.origin.y + 150.0f * ui.s);
              const ImVec2 ic0(infoX, infoTop),
                  ic1(infoX + infoW, infoTop + 230.0f * ui.s);
              ui.dl->AddRectFilled(ic0, ic1, IM_COL32(8, 14, 22, 215),
                                   2.0f * ui.s);
              NobleUi::DrawTacticalFrame(ui.dl, ic0, ic1, ui.s, false, true,
                                         false, false);
              DrawVariantInfo(
                  ui, ImVec2(infoX + 18.0f * ui.s, infoTop + 18.0f * ui.s),
                  infoW - 36.0f * ui.s, shown);
            }
            if (picked) {
              current = static_cast<int>(picked - list.data());
              NOBLE_INFO("FrontEndMenu: game type picked: {} ({})",
                         picked->label, picked->path.filename().string());
              st.variantPickerOpen = false;
              MenuSounds::Change();
            } else if (back) {
              st.variantPickerOpen = false;
              MenuSounds::Back();
            }
            DrawHints(ui, {{Prompt::Accept, "Choose"}, {Prompt::Back, "Back"}});
            return;
          }

          DrawTopTelemetryBar(ui, ff      ? "FIREFIGHT"
                                  : forge ? "FORGE"
                                          : "MULTIPLAYER");
          DrawTitle(ui, ImVec2(left, top),
                    ff      ? "FIREFIGHT"
                    : forge ? "FORGE"
                            : "CUSTOM GAME",
                    "MAP SELECT");
          const MenuMap *baseMaps = ff ? kFirefightMaps : kMaps;
          const int baseCount = ff ? Count(kFirefightMaps) : Count(kMaps);
          const std::vector<const ModEntry *> &modList =
              ff ? ctx.ffMods : ctx.mpMods;
          int &source = ff ? st.ffSource : forge ? st.forgeSource : st.mpSource;
          float tabsEnd = 0.0f;
          if (DrawSourceTabs(
                  ui, ImVec2(left + 26.0f * ui.s, top + 72.0f * ui.s), source,
                  static_cast<int>(modList.size()),
                  ff ? -1 : static_cast<int>(ctx.forgeMaps.size()), &tabsEnd)) {
            st.focusRow = 0;
            st.mapDrawerKey.clear();
            st.mapDrawerFocus = -1;
            st.mapGridScrollY = 0.0f;
          }
          const bool modded = source == 1;
          const bool forgeTab = source == 2 && !ff;
          const ModMode modMode =
              ff ? ModMode::Firefight : ModMode::Multiplayer;
          const std::string carrier =
              ff ? "ff60_icecave" : (forge ? "forge_halo" : "70_boneyard");
          const float searchX = std::max(left + 26.0f * ui.s + 240.0f * ui.s,
                                         tabsEnd + 10.0f * ui.s);
          if (DrawSearchBox(ui, "##map_search",
                            ImVec2(searchX, top + 72.0f * ui.s), 180.0f * ui.s,
                            st.mapSearch, sizeof(st.mapSearch),
                            "Search maps & mods"))
            st.mapSearchRow = 0;
          if (ImGui::IsItemActive())
            st.focusRow = 0;
          const bool searching = st.mapSearch[0] != 0;

          const std::vector<MapTileGroup> groups = BuildMapTileGroups(
              ctx, modded, forgeTab, modMode, modList, baseMaps, baseCount,
              carrier, !ff && !forgeTab, st.mapSearch);

          int &realIndex = forgeTab ? (forge ? st.forgeForgeMap : st.mpForgeMap)
                           : modded ? (ff      ? st.ffModded
                                       : forge ? st.forgeModded
                                               : st.mpModded)
                                    : (ff      ? st.ffMap
                                       : forge ? st.forgeMap
                                               : st.map);
          int &mapIndex = searching ? st.mapSearchRow : realIndex;
          std::filesystem::path ffDummyMvar;
          std::filesystem::path &activeMvar = ff       ? ffDummyMvar
                                              : modded ? st.modSelectedMvar
                                              : forge  ? st.forgeSelectedMvar
                                                       : st.mpSelectedMvar;

          if (const std::string autoMap = AutoPlayMap();
              !autoMap.empty() && source == 0) {
            for (size_t i = 0; i < groups.size(); ++i) {
              if (groups[i].base.scenario == autoMap ||
                  groups[i].base.launchScenario == autoMap) {
                mapIndex = static_cast<int>(i);
                break;
              }
            }
          }
          if (groups.empty())
            mapIndex = 0;
          else
            mapIndex =
                std::clamp(mapIndex, 0, static_cast<int>(groups.size()) - 1);

          static Screen s_mpLastScr = Screen::Main;
          static int s_mpLastSrc = -1;
          const bool firstEnter =
              (s_mpLastScr != st.screen || s_mpLastSrc != source);
          s_mpLastScr = st.screen;
          s_mpLastSrc = source;

          const float gridX = left + 24.0f * ui.s;
          const float gridTop = top + 114.0f * ui.s;
          const float rightMax = ui.origin.x + ui.size.x * 0.94f;
          const float gridW = (rightMax - gridX) * 0.56f;
          const float gridBot = ui.origin.y + ui.size.y - 68.0f * ui.s;

          const TacticalGridResult gridRes = DrawTacticalMapGrid(
              ui, st, ImVec2(gridX, gridTop), ImVec2(gridW, gridBot - gridTop),
              groups, mapIndex, activeMvar, st.focusRow == 0,
              searching  ? "Nothing matches your search"
              : forgeTab ? "No Forge maps in data\\map_variants"
              : ff       ? "No Reach Firefight mods installed"
                         : "No Reach multiplayer mods installed",
              forge ? "BASE CANVAS" : "BASE MAP", firstEnter, modMode);
          if (gridRes.pickedBaseOrVariant) {
            if (searching && !groups.empty()) {
              const std::string chosenKey =
                  groups[std::clamp(mapIndex, 0,
                                    static_cast<int>(groups.size()) - 1)]
                      .key;
              const std::vector<MapTileGroup> full = BuildMapTileGroups(
                  ctx, modded, forgeTab, modMode, modList, baseMaps, baseCount,
                  carrier, !ff && !forgeTab, "");
              for (size_t i = 0; i < full.size(); ++i) {
                if (full[i].key == chosenKey) {
                  realIndex = static_cast<int>(i);
                  break;
                }
              }
              st.mapSearch[0] = 0;
            }
            st.mapDrawerKey.clear();
            st.mapDrawerFocus = -1;
            st.focusRow = 1;
          } else if (gridRes.moveFocusRight) {
            st.focusRow = 1;
          }

          const int clampedGroup =
              groups.empty() ? 0
                             : std::clamp(mapIndex, 0,
                                          static_cast<int>(groups.size()) - 1);
          const MapTileGroup *selGrp =
              groups.empty() ? nullptr : &groups[clampedGroup];
          const MapEntry *selVar = nullptr;
          if (selGrp && !activeMvar.empty()) {
            for (const MapEntry &v : selGrp->variants) {
              if (v.mvar == activeMvar) {
                selVar = &v;
                break;
              }
            }
            if (!selVar)
              activeMvar.clear();
          }
          const MapEntry *map =
              selVar
                  ? selVar
                  : (selGrp && selGrp->base.enabled ? &selGrp->base : nullptr);
          const ModEntry *mod = map ? map->mod : nullptr;

          // "-auto-play-variant <text>": a test run can ask for a specific game
          // type.
          if (const std::string autoStem = AutoPlayVariantStem();
              !autoStem.empty() && !modded) {
            const std::vector<MenuVariant> &list =
                ff ? ctx.ffVariants : ctx.variants;
            int &v = ff ? st.ffVariant : st.variant;
            std::string needle = autoStem;
            for (char &c : needle)
              c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            int prefix = -1, anywhere = -1;
            for (size_t i = 0; i < list.size(); ++i) {
              std::string stem = list[i].path.stem().string();
              for (char &c : stem)
                c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
              if (stem.rfind(needle, 0) == 0) {
                prefix = static_cast<int>(i);
                break;
              }
              if (anywhere < 0 && stem.find(needle) != std::string::npos)
                anywhere = static_cast<int>(i);
            }
            const int chosen = prefix >= 0 ? prefix : anywhere;
            if (chosen >= 0) {
              v = chosen;
              static bool s_logged = false;
              if (!s_logged) {
                s_logged = true;
                NOBLE_INFO("FrontEndMenu: -auto-play-variant {} -> {}",
                           autoStem, list[chosen].path.filename().string());
              }
            }
          }

          // Game-type safety: a vanilla map only hosts the types it was built
          // for (modded maps are unrestricted).
          std::string gameType;
          std::string typeConflict;
          if (!ff && !forge && map && map->mod == nullptr) {
            const std::vector<MenuVariant> &list = ctx.variants;
            if (!list.empty()) {
              gameType = list[std::clamp(st.variant, 0,
                                         static_cast<int>(list.size()) - 1)]
                             .type;
            }
            if (!gameType.empty() &&
                !MapGameTypes::Supports(map->scenario, gameType)) {
              std::string good;
              const std::vector<std::string> allowed =
                  MapGameTypes::RequiredMaps(gameType);
              for (size_t i = 0; i < allowed.size(); ++i) {
                std::string name = allowed[i];
                for (const MenuMap &m : kMaps)
                  if (m.scenario == allowed[i]) {
                    name = m.name;
                    break;
                  }
                if (i)
                  good += (i + 1 == allowed.size()) ? " or " : ", ";
                good += name;
              }
              typeConflict = std::string(MapGameTypes::Name(gameType)) +
                             " isn't set up on " + map->name + "  Pick " +
                             good + " (modded maps are never restricted).";
              static std::string s_lastRefusal;
              if (s_lastRefusal != (map->scenario + "|" + gameType)) {
                s_lastRefusal = map->scenario + "|" + gameType;
                NOBLE_INFO(
                    "FrontEndMenu: refused to start {} on {} - {} has no "
                    "setup for it",
                    MapGameTypes::Name(gameType), map->name, map->scenario);
              }
            }
          }
          const bool forgeMissing =
              forge && !std::filesystem::exists(ctx.forgeVariant);

          // Right Detail & Match Setup Card
          const float px = gridX + gridW + 24.0f * ui.s;
          const float pw = rightMax - px;
          const float cardTop = std::max(gridTop, ui.origin.y + 150.0f * ui.s);
          const float cardBot = gridBot;
          const ImVec2 rc0(px, cardTop), rc1(px + pw, cardBot);
          ui.dl->AddRectFilled(rc0, rc1, IM_COL32(8, 14, 22, 215), 2.0f * ui.s);
          NobleUi::DrawTacticalFrame(ui.dl, rc0, rc1, ui.s, st.focusRow > 0,
                                     true, false, false);

          const float pad = 16.0f * ui.s;
          const float innerW = pw - pad * 2.0f;
          const float actionsY = cardBot - 62.0f * ui.s;
          const float stripY = actionsY - LobbyStripHeight(ui) - 8.0f * ui.s;
          const float controlsH = (forge ? 72.0f
                                   : ff  ? 214.0f
                                         : 144.0f) *
                                      ui.s +
                                  (forgeMissing ? 26.0f * ui.s : 0.0f) +
                                  (typeConflict.empty() ? 0.0f : 42.0f * ui.s);
          const float metaFixedH = (selVar ? 232.0f : 212.0f) * ui.s;
          const float previewH =
              std::clamp(stripY - cardTop - controlsH - metaFixedH,
                         58.0f * ui.s, innerW * 292.0f / 668.0f);

          const char *fallbackMode = ff      ? "FIREFIGHT"
                                     : forge ? "FORGE"
                                             : "MULTIPLAYER";
          float y = DrawMapDetailHeaderCard(ui, rc0, pw, previewH,
                                            selGrp ? &selGrp->base : nullptr,
                                            selVar, fallbackMode);

          int focusRows = 1;
          const bool navReady = (st.focusRow > 0);
          if (!forge) {
            const std::vector<MenuVariant> &list =
                ff ? ctx.ffVariants : ctx.variants;
            int &v = ff ? st.ffVariant : st.variant;
            const char *label =
                list.empty()
                    ? "Default"
                    : list[std::clamp(v, 0, static_cast<int>(list.size()) - 1)]
                          .label.c_str();
            const int dv = OptionRow(ui, "variant", ImVec2(px + pad, y), innerW,
                                     "GAME TYPE", label,
                                     st.focusRow == focusRows, nullptr, false);
            const float valueX = px + pad + innerW * 0.48f,
                        rowHeight = 32.0f * ui.s;
            ImGui::SetCursorScreenPos(ImVec2(px + pad, y));
            bool openList = ImGui::InvisibleButton(
                "##variant_label", ImVec2(innerW * 0.48f, rowHeight));
            ImGui::SetCursorScreenPos(ImVec2(valueX, y));
            openList =
                ImGui::InvisibleButton("##variant_value",
                                       ImVec2(innerW * 0.52f, rowHeight)) ||
                openList;
            DrawMiddleHint(ui, ImVec2(px + pad, y), innerW, label,
                           ImGui::IsItemHovered(), false, true);
            if (navReady && st.focusRow == focusRows && AcceptPressed())
              openList = true;
            if (openList && !list.empty()) {
              st.variantPickerOpen = true;
              st.variantPickerModded = modded;
              st.variantPickerMap = map ? map->scenario : std::string();
              st.variantPickerMapName = map ? map->name : std::string();
              BeginVariantList(
                  st, list, st.variantPickerMap, modded || ff,
                  &list[std::clamp(v, 0, static_cast<int>(list.size()) - 1)]);
              MenuSounds::Click();
            }
            if (!list.empty()) {
              const int count = static_cast<int>(list.size());
              auto allowed = [&](int index) {
                if (modded || !map)
                  return true;
                const std::string &type = list[index].type;
                return type.empty() ||
                       MapGameTypes::Supports(map->scenario, type);
              };
              if (dv) {
                int next = v;
                for (int step = 0; step < count; ++step) {
                  next = (next + dv + count) % count;
                  if (allowed(next))
                    break;
                }
                v = next;
              } else if (!allowed(v)) {
                for (int step = 1; step <= count; ++step) {
                  const int candidate = (v + step) % count;
                  if (allowed(candidate)) {
                    v = candidate;
                    break;
                  }
                }
              }
            }
            y += 36.0f * ui.s;
            ++focusRows;
          }
          if (!typeConflict.empty()) {
            ui.dl->AddRectFilled(ImVec2(px + pad, y),
                                 ImVec2(px + pw - pad, y + 38.0f * ui.s),
                                 IM_COL32(90, 26, 26, 150));
            Text(ui, nullptr, 14.0f,
                 ImVec2(px + pad + 10.0f * ui.s, y + 4.0f * ui.s),
                 IM_COL32(255, 190, 180, 255), typeConflict.c_str(),
                 px + pw - pad - 10.0f * ui.s);
            y += 42.0f * ui.s;
          }
          if (!ff) {
            if (forge) {
              const char *label =
                  st.forgeCustomVariant.empty() ? "EDIT" : "CUSTOM (EDIT)";
              bool middleClicked = false;
              const int optionsChange =
                  ClickRow(ui, "forge_options", ImVec2(px + pad, y), innerW,
                           "GAME OPTIONS", label, st.focusRow == focusRows,
                           &middleClicked);
              ImGui::SetCursorScreenPos(ImVec2(px + pad, y));
              const bool clicked = ImGui::InvisibleButton(
                                       "##forge_options_open",
                                       ImVec2(innerW * 0.48f, 32.0f * ui.s)) ||
                                   middleClicked;
              if ((clicked || optionsChange != 0 ||
                   (navReady && st.focusRow == focusRows && AcceptPressed())) &&
                  std::filesystem::exists(ctx.forgeVariant)) {
                MenuVariant forgeMv;
                forgeMv.path = st.forgeCustomVariant.empty()
                                   ? ctx.forgeVariant
                                   : st.forgeCustomVariant;
                forgeMv.label = st.forgeCustomVariant.empty() ? "Basic Editing"
                                                              : "Forge Custom";
                forgeMv.custom = !st.forgeCustomVariant.empty();
                OpenGameOptions(ctx, forgeMv);
              }
              ++focusRows;
            } else {
              const std::vector<MenuVariant> &list = ctx.variants;
              bool middleClicked = false;
              const int optionsChange =
                  ClickRow(ui, "game_options", ImVec2(px + pad, y), innerW,
                           "GAME OPTIONS", "EDIT", st.focusRow == focusRows,
                           &middleClicked);
              ImGui::SetCursorScreenPos(ImVec2(px + pad, y));
              const bool clicked = ImGui::InvisibleButton(
                                       "##game_options_open",
                                       ImVec2(innerW * 0.48f, 32.0f * ui.s)) ||
                                   middleClicked;
              if ((clicked || optionsChange != 0 ||
                   (navReady && st.focusRow == focusRows && AcceptPressed())) &&
                  !list.empty()) {
                OpenGameOptions(
                    ctx, list[std::clamp(st.variant, 0,
                                         static_cast<int>(list.size()) - 1)]);
              }
              ++focusRows;
            }
            y += 36.0f * ui.s;
            if (!forge) {
              const bool chosen =
                  st.playlistChoice >= 0 &&
                  st.playlistChoice < static_cast<int>(st.playlists.size());
              const int count = static_cast<int>(st.playlists.size());
              int playlistPicked = -1;
              const int playlistStep = PickerRow(
                  ui, "playlist", ImVec2(px + pad, y), innerW, "PLAYLIST",
                  chosen ? st.playlists[st.playlistChoice].name.c_str()
                         : "NONE",
                  st.focusRow == focusRows,
                  [&](std::vector<std::string> &choices, int &current) {
                    choices.push_back("None");
                    for (const auto &playlist : st.playlists)
                      choices.push_back(playlist.name);
                    current = chosen ? st.playlistChoice + 1 : 0;
                  },
                  &playlistPicked, false, nullptr, false);
              if (playlistStep != 0)
                st.playlistChoice =
                    ((st.playlistChoice + 1 + playlistStep + count + 1) %
                     (count + 1)) -
                    1;
              else if (playlistPicked >= 0)
                st.playlistChoice = playlistPicked - 1;
              ImGui::SetCursorScreenPos(ImVec2(px + pad, y));
              const bool openEditor = ImGui::InvisibleButton(
                  "##playlist_open", ImVec2(innerW * 0.48f, 32.0f * ui.s));
              if (openEditor ||
                  (navReady && st.focusRow == focusRows && AcceptPressed())) {
                st.playlistEdit = chosen ? st.playlists[st.playlistChoice]
                                         : Playlists::Playlist{};
                std::snprintf(st.playlistName, sizeof(st.playlistName), "%s",
                              chosen ? st.playlistEdit.name.c_str()
                                     : "My Playlist");
                st.playlistEditRow = 0;
                st.playlistMessage.clear();
                st.confirmAction.clear();
                st.playlistEditOpen = true;
                MenuSounds::Click();
              }
              ++focusRows;
              y += 36.0f * ui.s;
            }
            // 48+ seats are untested and the 128-player work is on Tyler's hold; multiplayer also
            // "stopped at 34 players" in his 2026-11-15 report. Clamp the picker to the implemented
            // 32-slot work (stock 16 stays the default); Claude's session/replication work owns
            // re-enabling higher counts (see project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md).
            static const int kCaps[] = {4, 8, 12, 16, 24, 32};
            constexpr int kNumCaps =
                static_cast<int>(sizeof(kCaps) / sizeof(kCaps[0]));
            int capIdx = 3;
            for (int ci = 0; ci < kNumCaps; ++ci) {
              if (kCaps[ci] >= st.sessionMaxPlayers) {
                capIdx = ci;
                break;
              }
            }
            char capText[32];
            std::snprintf(capText, sizeof(capText), "%d PLAYERS",
                          kCaps[capIdx]);
            int capPicked = -1;
            const int dc = PickerRow(
                ui, "players", ImVec2(px + pad, y), innerW, "MAX PLAYERS",
                capText, st.focusRow == focusRows,
                [&](std::vector<std::string> &choices, int &current) {
                  for (int ci = 0; ci < kNumCaps; ++ci)
                    choices.push_back(std::to_string(kCaps[ci]) + " PLAYERS");
                  current = capIdx;
                },
                &capPicked,
                navReady && st.focusRow == focusRows && AcceptPressed());
            if (dc) {
              capIdx = (capIdx + dc + kNumCaps) % kNumCaps;
              st.sessionMaxPlayers = kCaps[capIdx];
            } else if (capPicked >= 0) {
              st.sessionMaxPlayers = kCaps[capPicked];
            }
            y += 36.0f * ui.s;
            ++focusRows;
          }
          if (ff) {
            static const int kFfCaps[] = {4, 8, 12, 16};
            constexpr int kNumFfCaps = 4;
            int ffCapIdx = 3;
            for (int ci = 0; ci < kNumFfCaps; ++ci) {
              if (kFfCaps[ci] >= st.ffMaxPlayers) {
                ffCapIdx = ci;
                break;
              }
            }
            char ffCapText[32];
            std::snprintf(ffCapText, sizeof(ffCapText), "%d PLAYERS",
                          kFfCaps[ffCapIdx]);
            int ffCapPicked = -1;
            const int dcFf = PickerRow(
                ui, "ff_players", ImVec2(px + pad, y), innerW, "MAX PLAYERS",
                ffCapText, st.focusRow == focusRows,
                [&](std::vector<std::string> &choices, int &current) {
                  for (int ci = 0; ci < kNumFfCaps; ++ci)
                    choices.push_back(std::to_string(kFfCaps[ci]) + " PLAYERS");
                  current = ffCapIdx;
                },
                &ffCapPicked,
                navReady && st.focusRow == focusRows && AcceptPressed());
            if (dcFf) {
              ffCapIdx = (ffCapIdx + dcFf + kNumFfCaps) % kNumFfCaps;
              st.ffMaxPlayers = kFfCaps[ffCapIdx];
            } else if (ffCapPicked >= 0) {
              st.ffMaxPlayers = kFfCaps[ffCapPicked];
            }
            y += 36.0f * ui.s;
            ++focusRows;

            bool ffMiddleClicked = false;
            const int ffOptionsChange = ClickRow(
                ui, "ff_options", ImVec2(px + pad, y), innerW, "GAME OPTIONS",
                "EDIT", st.focusRow == focusRows, &ffMiddleClicked);
            ImGui::SetCursorScreenPos(ImVec2(px + pad, y));
            const bool ffClicked =
                ImGui::InvisibleButton("##ff_options_open",
                                       ImVec2(innerW * 0.48f, 30.0f * ui.s)) ||
                ffMiddleClicked;
            if ((ffClicked || ffOptionsChange != 0 ||
                 (navReady && st.focusRow == focusRows && AcceptPressed())) &&
                !ctx.ffVariants.empty()) {
              OpenGameOptions(
                  ctx, ctx.ffVariants[std::clamp(
                           st.ffVariant, 0,
                           static_cast<int>(ctx.ffVariants.size()) - 1)]);
            }
            ++focusRows;
            y += 36.0f * ui.s;
            y += DifficultyTiles(ui, "ffdiff", ImVec2(px + pad, y), innerW,
                                 st.difficulty, st.focusRow == focusRows) +
                 8.0f * ui.s;
            ++focusRows;
          }
          if (forgeMissing) {
            Text(ui, nullptr, 15.0f, ImVec2(px + pad + 8.0f * ui.s, y),
                 IM_COL32(255, 190, 120, 255),
                 "MCC's Forge game type (00_basic_editing_054.bin) wasn't "
                 "found.");
          }
          DrawLobbyControls(ui, ImVec2(px + pad, stripY), innerW, st);

          const int soloRow = focusRows;
          const int hostRow = focusRows + 1;
          const bool canStart = map && map->enabled && typeConflict.empty();
          const float bw = (innerW - 10.0f * ui.s) * 0.5f;
          std::string sub;
          if (!map)
            sub = "Pick a map";
          else if (ff)
            sub = "Hold out alone \xC2\xB7 " +
                  std::string(kDifficultyNames[st.difficulty]);
          else if (forge)
            sub = map->name + " \xC2\xB7 Forge";
          else {
            const std::vector<MenuVariant> &list = ctx.variants;
            sub = map->name + " \xC2\xB7 " +
                  (list.empty()
                       ? std::string("Default")
                       : list[std::clamp(st.variant, 0,
                                         static_cast<int>(list.size()) - 1)]
                             .label);
          }

          const char *soloLabel = ff      ? "PLAY SOLO"
                                  : forge ? "SOLO FORGE"
                                          : "PLAY LOCAL";
          const char *hostLabel = ff      ? "HOST CO-OP"
                                  : forge ? "HOST FORGE"
                                          : "HOST LOBBY";
          const char *hostSub =
              st.lobbyNetMode == 1
                  ? (st.lobbyPrivacy == 0 ? "Public Online P2P"
                                          : "Private Online P2P")
                  : "Local LAN Lobby";

          const bool autoPlay = AutoPlayNow();
          const bool startSolo =
              ((BigAction(ui, "start", ImVec2(px + pad, actionsY), bw,
                          soloLabel, sub.c_str(), true, canStart,
                          st.focusRow == soloRow) ||
                (st.focusRow == soloRow && AcceptPressed())) &&
               canStart) ||
              (autoPlay && canStart);
          const bool startHost =
              (BigAction(ui, "host",
                         ImVec2(px + pad + bw + 10.0f * ui.s, actionsY),
                         innerW - bw - 10.0f * ui.s, hostLabel, hostSub, true,
                         canStart, st.focusRow == hostRow) ||
               (st.focusRow == hostRow && AcceptPressed())) &&
              canStart;

          if (startHost) {
            std::string varPath, varName;
            if (ff) {
              if (!ctx.ffVariants.empty()) {
                varPath = ctx.ffVariants[st.ffVariant].path.string();
                varName = ctx.ffVariants[st.ffVariant].label;
              } else {
                varName = "Firefight Standard";
              }
            } else if (forge) {
              varPath = (!st.forgeCustomVariant.empty() &&
                         std::filesystem::exists(st.forgeCustomVariant))
                            ? st.forgeCustomVariant.string()
                            : ctx.forgeVariant.string();
              varName = st.forgeCustomVariant.empty() ? "Basic Editing"
                                                      : "Custom Forge";
            } else {
              if (!ctx.variants.empty()) {
                varPath = ctx.variants[st.variant].path.string();
                varName = ctx.variants[st.variant].label;
              } else {
                varName = "Slayer";
              }
            }
            const std::string mode =
                ff ? "firefight" : (forge ? "forge" : "multiplayer");
            const std::string lobbyScenario =
                map ? (map->launchScenario.empty() ? map->scenario
                                                   : map->launchScenario)
                    : std::string("20_sword_slayer");
            SetupHostLobby(st, lobbyScenario, mode, varPath, varName,
                           ff ? st.difficulty : 1, 0,
                           ff ? st.ffMaxPlayers : st.sessionMaxPlayers,
                           map ? map->mod : nullptr,
                           map ? map->mapOverride : std::filesystem::path(),
                           map ? map->name : std::string(),
                           map ? map->imageLoading : std::filesystem::path(),
                           map ? map->imageThumbnail : std::filesystem::path());
            if (map && !map->mvar.empty()) {
              st.lobbyMapVariant = map->mvar;
              NOBLE_INFO("FrontEndMenu: lobby Forge map '{}' ({})", map->name,
                         map->mvar.filename().string());
            }
            if (!ff && !forge && st.playlistChoice >= 0 &&
                st.playlistChoice < static_cast<int>(st.playlists.size())) {
              const Playlists::Playlist &playlist =
                  st.playlists[st.playlistChoice];
              st.playlistPosition =
                  playlist.shuffle ? static_cast<int>(std::random_device{}() %
                                                      playlist.entries.size())
                                   : 0;
              ResolvedEntry resolved;
              if (ResolveEntry(ctx, playlist.entries[st.playlistPosition],
                               playlist.randomType, resolved))
                ApplyEntryToLobby(st, resolved);
            }
          } else if (startSolo && map) {
            FrontEndChoice req;
            req.play = true;
            req.host = false;
            req.map = map->launchScenario.empty() ? map->scenario
                                                  : map->launchScenario;
            req.mapOverride = map->mapOverride;
            req.mapTitle = map->name;
            req.mapImage = map->imageLoading;
            req.mapThumb = map->imageThumbnail;
            req.mapVariant = map->mvar;
            if (mod) {
              req.modFolder = mod->folder;
              req.modTitle = mod->title;
            }
            if (ff) {
              req.mode = "firefight";
              req.difficulty = st.difficulty;
              if (!ctx.ffVariants.empty())
                req.variantPath = ctx.ffVariants[st.ffVariant].path.string();
            } else {
              req.mode = forge ? "forge" : "multiplayer";
              if (forge) {
                req.variantPath =
                    (!st.forgeCustomVariant.empty() &&
                     std::filesystem::exists(st.forgeCustomVariant))
                        ? st.forgeCustomVariant.string()
                        : ctx.forgeVariant.string();
              } else if (!ctx.variants.empty()) {
                req.variantPath = ctx.variants[st.variant].path.string();
              }
            }
            RequestMatchLaunch(choice, done, req);
          }
          if (!g_pickerBlocksNav && st.focusRow > 0) {
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) ||
                ImGui::IsKeyPressed(ImGuiKey_Tab) || UiPad::Down())
              st.focusRow = std::min(hostRow, st.focusRow + 1);
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || UiPad::Up())
              st.focusRow = std::max(1, st.focusRow - 1);
            if (st.focusRow == soloRow &&
                (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || UiPad::Right())) {
              st.focusRow = hostRow;
              MenuSounds::Scroll();
            }
            if (st.focusRow == hostRow &&
                (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || UiPad::Left())) {
              st.focusRow = soloRow;
              MenuSounds::Scroll();
            }
          }
          if (!ImGui::GetIO().WantTextInput && UiPad::ActionX()) {
            if (forge && std::filesystem::exists(ctx.forgeVariant)) {
              MenuVariant forgeMv;
              forgeMv.path = st.forgeCustomVariant.empty()
                                 ? ctx.forgeVariant
                                 : st.forgeCustomVariant;
              forgeMv.label = st.forgeCustomVariant.empty() ? "Basic Editing"
                                                            : "Forge Custom";
              forgeMv.custom = !st.forgeCustomVariant.empty();
              OpenGameOptions(ctx, forgeMv);
              MenuSounds::Click();
            } else if (ff && !ctx.ffVariants.empty()) {
              OpenGameOptions(
                  ctx, ctx.ffVariants[std::clamp(
                           st.ffVariant, 0,
                           static_cast<int>(ctx.ffVariants.size()) - 1)]);
              MenuSounds::Click();
            } else if (!ff && !forge && !ctx.variants.empty()) {
              OpenGameOptions(ctx,
                              ctx.variants[std::clamp(
                                  st.variant, 0,
                                  static_cast<int>(ctx.variants.size()) - 1)]);
              MenuSounds::Click();
            }
          }
          if (back) {
            if (st.focusRow > 0)
              st.focusRow = 0;
            else if (!st.mapDrawerKey.empty()) {
              st.mapDrawerKey.clear();
              st.mapDrawerFocus = -1;
            } else
              GoBackScreen(st, Screen::Main);
          }
          const char *tabsHint =
              ff ? "Vanilla / Modded" : "Vanilla / Modded / Forge Maps";
          if (forge) {
            DrawHints(ui,
                      {{Prompt::Accept, "Select"},
                       {Prompt::Back, "Back"},
                       {Prompt::Tabs, tabsHint}},
                      {{InputPrompts::Button::X, "X", "Game Options"}});
          } else {
            DrawHints(ui,
                      {{Prompt::Accept, "Select"},
                       {Prompt::Back, "Back"},
                       {Prompt::Change, "Change"},
                       {Prompt::Tabs, tabsHint}},
                      {{InputPrompts::Button::X, "X", "Game Options"}});
          }
        }

      } // namespace noble::fe
