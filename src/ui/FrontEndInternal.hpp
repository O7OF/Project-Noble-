#pragma once

#include "FrontEndMenu.hpp"
#include "ArmorCustomizer.hpp"
#include "CareerPanel.hpp"
#include "ChangelogData.hpp"
#include "InputPrompts.hpp"
#include "MenuSounds.hpp"
#include "ModCatalog.hpp"
#include "ModDownloadPrompt.hpp"
#include "NobleStyle.hpp"
#include "NobleUi.hpp"
#include "ServerBrowser.hpp"
#include "SettingsPanel.hpp"
#include "SocialOverlay.hpp"
#include "TeamColors.hpp"
#include "UIManager.hpp"
#include "UiArt.hpp"
#include "UiPad.hpp"
#include "WorkshopManager.hpp"
#include "audio/ProcessAudio.hpp"
#include "audio/ReachAudio.hpp"
#include "audio/VoiceChat.hpp"
#include "career/Career.hpp"
#include "common/BuildIdentity.hpp"
#include "common/DeviceIdentity.hpp"
#include "common/Logger.hpp"
#include "common/MapNames.hpp"
#include "common/RecommendedMaps.hpp"
#include "common/Relaunch.hpp"
#include "common/Settings.hpp"
#include "common/Updater.hpp"
#include "common/Version.hpp"
#include "engine/BlamEngine.hpp"
#include "engine/FrameCapture.hpp"
#include "engine/MapGameTypes.hpp"
#include "engine/Playlists.hpp"
#include "engine/ReachArmory.hpp"
#include "engine/ReachHost.hpp"
#include "engine/SessionVariant.hpp"
#include "engine/VariantRules.hpp"
#include "hooks/HookManager.hpp"
#include "host/D3D11Renderer.hpp"
#include "host/EngineLimits.hpp"
#include "host/Window.hpp"
#include "network/AuthClient.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/SessionManager.hpp"
#include "network/SocialClient.hpp"
#include "network/UPnP.hpp"
#include "vfs/VFS.hpp"

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cwctype>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <format>
#include <fstream>
#include <functional>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <set>
#include <shellapi.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <windows.h>

namespace noble::fe {

using InputPrompts::Prompt;

template <typename T, size_t N> constexpr int Count(const T (&)[N]) {
  return static_cast<int>(N);
}

struct MenuMap {
  const char *scenario; // passed to BlamEngine::LaunchMap
  const char *name;     // shown in the list
  const char *blurb;    // one line under the preview
};

// Same maps as BlamEngine's multiplayer table (launch indices 190-209).
inline const MenuMap kMaps[] = {
    {"50_panopticon", "Boardwalk", "New Alexandria waterfront."},
    {"70_boneyard", "Boneyard", "A ship-breaking yard on the coast."},
    {"45_launch_station", "Countdown", "Inside a Sabre launch facility."},
    {"30_settlement", "Powerhouse", "A hydroelectric plant in the hills."},
    {"52_ivory_tower", "Reflection", "A penthouse high above the city."},
    {"35_island", "Spire", "A Covenant spire on a rocky island."},
    {"20_sword_slayer", "Sword Base", "The atrium of ONI's Sword Base."},
    {"45_aftship", "Zealot", "Aboard a Covenant corvette."},
    {"dlc_slayer", "Anchor 9", "A space station in orbit over Reach."},
    {"dlc_invasion", "Breakpoint", "An ONI research site under attack."},
    {"dlc_medium", "Tempest", "A Forerunner coastline."},
    {"condemned", "Condemned", "An orbital station breaking apart."},
    {"trainingpreserve", "Highlands", "A training preserve in the hills."},
    {"cex_beaver_creek", "Battle Canyon", "A remake of a canyon classic."},
    {"cex_damnation", "Penance", "A remake of a Covenant classic."},
    {"cex_timberland", "Ridgeline", "A remake of a forest classic."},
    {"cex_prisoner", "Solitary", "A remake of a prison classic."},
    {"cex_hangemhigh", "High Noon", "A remake of a tomb classic."},
    {"cex_headlong", "Breakneck", "A remake of a city classic."},
    {"forge_halo", "Forge World", "The largest canvas Reach has."},
};
constexpr int kDefaultMap = 6; // Sword Base: the map tested most

// Campaign missions (launch index 178-189).
inline const MenuMap kMissions[] = {
    {"m05", "Noble Actual", "Arrive on Reach and meet Noble Team. (Cinematic)"},
    {"m10", "Winter Contingency",
     "A relay station on the frontier has gone silent."},
    {"m20", "ONI: Sword Base", "The Covenant are attacking a vital ONI base."},
    {"m30", "Nightfall", "Scout behind enemy lines under cover of dark."},
    {"m35", "Tip of the Spear", "Two armies collide. Take out the spire."},
    {"m45", "Long Night of Solace", "Take the fight into orbit."},
    {"m50", "Exodus", "Get civilians out of an occupied city."},
    {"m52", "New Alexandria", "Air support over a city under siege."},
    {"m60", "The Package", "Back to Sword Base for one last job."},
    {"m70", "The Pillar of Autumn", "Deliver the package to the ship."},
    {"m70_bonus", "Lone Wolf", "Hold out for as long as you can."},
};
inline const char *const kDifficulties[] = {"EASY", "NORMAL", "HEROIC", "LEGENDARY"};
inline const float kDifficultyMultiplier[] = {0.75f, 1.0f, 1.25f, 1.5f};

inline const MenuMap kFirefightMaps[] = {
    {"ff50_park", "Beachhead", ""},      {"ff45_corvette", "Corvette", ""},
    {"ff20_courtyard", "Courtyard", ""}, {"ff60_icecave", "Glacier", ""},
    {"ff70_holdout", "Holdout", ""},     {"ff60_airview", "Outpost", ""},
    {"ff10_prototype", "Overlook", ""},  {"ff30_waterfront", "Waterfront", ""},
    {"ff_unearthed", "Unearthed", ""},   {"cex_ff_halo", "Installation 04", ""},
};

// Reach's campaign skulls. `bit` is MCC's skull number (launch block +0x48),
// `icon` the name in MCC's skulls texture pack, `multiplier` the
// campaign-scoring bonus.
struct Skull {
  const char *name;
  int bit;
  const char *icon;
  const char *description;
  float multiplier;
  bool primary;
};
inline const Skull kSkulls[] = {
    {"Iron", 18, "IRON",
     "Co-op: dying sends everyone back to the last checkpoint. Solo: death "
     "restarts the mission.",
     1.2f, true},
    {"Black Eye", 3, "BLACKEYE",
     "Your shields only recharge when you melee enemies.", 1.1f, true},
    {"Tough Luck", 36, "TOUGHLUCK",
     "Enemies dodge grenades and dive out of danger far more often.", 1.1f,
     true},
    {"Catch", 7, "CATCH", "Enemies throw many more grenades.", 1.1f, true},
    {"Cloud", 13, "CLOUD", "Your motion tracker is turned off.", 1.1f, true},
    {"Famine", 11, "FAMINE",
     "Weapons dropped by enemies carry half the usual ammunition.", 1.2f, true},
    {"Thunderstorm", 34, "THUNDERSTORM",
     "Enemies are promoted to their higher ranks.", 1.2f, true},
    {"Tilt", 35, "TILT", "Enemy strengths and weaknesses are exaggerated.",
     1.2f, true},
    {"Mythic", 23, "MYTHIC", "Enemies have much more health.", 1.2f, true},
    {"Blind", 4, "BLIND", "Your HUD and first-person weapon are hidden.", 1.0f,
     false},
    {"Cowbell", 8, "COWBELL", "Explosions throw things much farther.", 1.0f,
     false},
    {"Grunt Birthday Party", 16, "GRUNTBIRTHDAYPARTY",
     "Headshots on Grunts end in a burst of confetti.", 1.0f, false},
    {"IWHBYD", 19, "IWHBYD", "Rare combat dialogue becomes much more common.",
     1.0f, false},
};
constexpr int kSkullCount =
    static_cast<int>(sizeof(kSkulls) / sizeof(kSkulls[0]));
inline const char *const kDifficultyNames[] = {"Easy", "Normal", "Heroic",
                                        "Legendary"};


struct MenuVariant {
  std::string label;
  std::filesystem::path path;
  std::string description; // the file's own text; empty when it only holds a
                           // key for a string table Noble can't read
  bool custom = false; // a player-made game mode (a .json preset), listed under
                       // its own heading
  std::string
      type; // MapGameTypes key from the file name; empty when it can't be told
};

struct VariantName {
  const char *stem; // lower-case file stem
  const char *label;
};

struct ForgeMap {
  std::string title, description, creator;
  std::string scenario; // the stock map it is built on
  std::string group; // list heading, from the pack's file name prefixes ("INF
                     // ", "BTB ", ...)
  std::filesystem::path path;
};


struct Ui {
  ImDrawList *dl = nullptr;
  ImVec2 origin = ImVec2(0.0f, 0.0f);
  ImVec2 size;
  float s = 1.0f;
};

inline const ImU32 kWhite = IM_COL32(236, 240, 244, 255);
inline const ImU32 kDim = IM_COL32(160, 170, 180, 255);
inline const ImU32 kDisabled = IM_COL32(110, 118, 126, 200);
inline const ImU32 kAccent = IM_COL32(143, 216, 255, 255);

inline constexpr const wchar_t *kForgeVariant = L"00_basic_editing_054.bin";

enum class Screen {
  Main,
  Campaign,
  Skulls,
  Firefight,
  CustomGame,
  Forge,
  Theater,
  Servers,
  Settings,
  Armory,
  Mods,
  Career,
  PreGameLobby,
  Social
};


struct ListItem {
  std::string label;
  bool enabled = true;
  bool header = false;
  bool expanded = false;
  std::string thumb; // scenario whose small preview is drawn before the label
                     // (empty = none)
  std::filesystem::path
      image;      // a mod map's own thumbnail file (wins over thumb)
  int number = 0; // shown before the thumbnail when > 0
};


struct HintExtra {
  InputPrompts::Button button;
  const char *key;
  const char *label;
  // Clickable extras get a hover plate and report a click through HintExtraClicked(label) on the next frame (used by the
  // main-menu "What's New" hint so Tyler can click it, not just press N).
  bool clickable = false;
};


struct MainEntry {
  const char *label;
  const char *description;
  Screen target;
  bool enabled;
  bool quit;
};
inline const MainEntry kMainEntries[] = {
    {"Campaign",
     "Play the story of Noble Team, from Winter Contingency to Lone Wolf.",
     Screen::Campaign, true, false},
    {"Server Browser",
     "Browse and join online multiplayer, co-op and custom games.",
     Screen::Servers, true, false},
    {"Join with Code",
     "Join a friend's lobby or game from a code they shared (Discord, chat).",
     Screen::Servers, true, false},
    {"Firefight",
     "Hold out against waves of Covenant on the map and game type you choose.",
     Screen::Firefight, true, false},
    {"Custom Game",
     "Play any multiplayer map and game type, host a game or join one.",
     Screen::CustomGame, true, false},
    {"Forge", "Edit multiplayer maps with Forge. Experimental in Noble.",
     Screen::Forge, true, false},
    {"Theater",
     "Watch saved films and local video clips from Campaign, Firefight, Custom "
     "Games, and Forge.",
     Screen::Theater, true, false},
    {"Social", "Manage your friends list, find players, and view game invites.",
     Screen::Social, true, false},
    {"Service Record",
     "Your rank, credits, challenges, commendations and recent games.",
     Screen::Career, true, false},
    {"Armory",
     "Customize your Spartan and Sangheili armor in Reach's 3D Armory.",
     Screen::Armory, true, false},
    {"Account",
     "Log in or register your account to sync data and unlock developer "
     "access.",
     Screen::Main, true, false},
    {"Mods", "Workshop maps and mods.", Screen::Mods, true, false},
    {"Settings", "Controls, key bindings, audio, video and gameplay options.",
     Screen::Settings, true, false},
    {"Quit", "Close Noble.", Screen::Main, true, true},
};

// "Continue: <mode> on <map>" (spec 4.1), built into the main-menu list per
// frame, above Campaign, only when a match has been started before. Activating
// it relaunches that match with the same settings.
inline const MainEntry kContinueEntry = {
    "Continue",
    "Start the campaign mission you last played, on the same difficulty and "
    "with the same skulls.",
    Screen::Main, true, false};

// Developer-only row, built into the main-menu list per frame so it sits
// directly above Quit. The gold DEV badge next to the player name is the only
// other place developer status is shown.
inline const MainEntry kDeveloperEntry = {"Developer Menu",
                                   "Unlock armors, grant credits, set rank or "
                                   "reset a career. Developer accounts only.",
                                   Screen::Main, true, false};


struct MapEntry {
  std::string scenario, name, blurb;
  // A modded map with a new scenario name is loaded through a stock map the
  // engine already knows: launchScenario is that stock scenario (its launch
  // index / map id select the match), and mapOverride is the mod's .map file
  // served in place of the stock maps<launchScenario>.map.
  std::string launchScenario;
  std::filesystem::path mapOverride;
  std::filesystem::path imageThumbnail, imageLarge, imageLoading;
  bool enabled = true;
  const ModEntry *mod = nullptr;
  bool header = false;
  bool custom = false;
  std::filesystem::path mvar; // Forge map laid over `scenario`
  std::string forgeGroup; // Forge map heading this row is under (also set on
                          // the heading)
};


enum class ModMode { Campaign, Firefight, Multiplayer };

struct TheaterEntry {
  std::filesystem::path path;
  std::string title;
  std::string description;
  std::string author;
  std::string scenario;   // e.g. "20_sword_slayer", "m10", "ff60_icecave"
  std::string mapDisplay; // e.g. "Sword Base", "Winter Contingency"
  std::string launchMode; // "campaign", "multiplayer", "forge", "firefight"
  std::string modeLabel;  // "CAMPAIGN", "CUSTOM GAME", "FORGE", "FIREFIGHT",
                          // "LOCAL VIDEO"
  uint32_t durationSec = 0;
  uint64_t timestamp = 0; // Unix epoch seconds
  uint64_t fileSize = 0;
  bool isVideo = false; // true for .mp4/.mkv/.webm/.avi/.mov/.wmv
  bool isSaved = false; // true if stored in data\theater\saved
};


struct MenuState {
  Screen screen = Screen::Main;
  int mainIndex = 0;
  int mission = 1;
  int difficulty = 1;
  int focusRow = 0;
  int map = kDefaultMap;
  int variant = 0;
  int ffMap = 0;
  int ffVariant = 0;
  int forgeMap = 19;
  std::filesystem::path forgeCustomVariant;
  int skull = 0;
  uint64_t skulls = 0;
  int campaignSource = 0;
  int campaignMaxPlayers = 4; // HOST CO-OP size; 4..16
  int ffSource = 0;
  int mpSource = 0;
  int forgeSource = 0;
  std::set<std::string> openMods;
  int campaignModded = 0;
  int ffModded = 0;
  int mpModded = 0;
  int forgeModded = 0;
  int mpForgeMap = 0; // row in the FORGE MAPS tab
  int forgeForgeMap = 0;
  int lobbyNetMode = 1; // 0 = Local (LAN), 1 = Online (P2P)
  int lobbyPrivacy =
      0; // 0 = Public (Master Server), 1 = Private (Direct Connect)
  bool confirmQuit =
      false; // "Quit Project Noble?" box on the main menu (Esc or QUIT)
  int quitChoice = 1; // 0 = Yes, 1 = No (No is preselected)
  bool showAccountModal = false;
  int accountTab = 0; // 0 = Login, 1 = Register
  char accountUser[32] = "";
  char accountPass[32] = "";
  std::string accountStatusMsg = "";
  bool accountStatusError = false;

  // Pre-game lobby state
  bool isLobbyHost = false;
  std::string lobbySessionId;
  std::string lobbySessionToken;
  std::string lobbyMap = "20_sword_slayer";
  std::filesystem::path
      lobbyMapOverride; // modded .map served in place of the stock lobbyMap
  std::filesystem::path lobbyMapVariant; // Forge map (.mvar) laid over
                                         // lobbyMap; empty for the plain map
  std::string lobbyMapTitle;
  std::filesystem::path lobbyMapImage, lobbyMapThumb;
  std::string lobbyMode = "multiplayer";
  std::string lobbyVariantPath;
  std::string lobbyVariantName = "Slayer";
  // Workshop mod this lobby runs. A guest without it sees the Download & Join
  // prompt before joining.
  std::string lobbyModId;
  std::string lobbyModTitle;
  std::filesystem::path lobbyModFolder;
  int lobbyDifficulty = 1;
  uint64_t lobbySkulls = 0;
  int lobbyMaxPlayers = 16;
  int sessionMaxPlayers =
      16; // picker value; above EffectiveMultiplayerCap() (16 today) it is clamped/disabled
  int ffMaxPlayers = 16; // players the Firefight lobby will allow (4..16)
  std::string lobbyHostEndpoint;
  std::vector<LobbyPlayer> lobbyRoster;
  bool lobbyLaunchPending = false;
  std::string lobbyLaunchMap;
  std::string lobbyLaunchMapVariant; // host's Forge map file name
  std::string lobbyLaunchMode;
  std::string lobbyLaunchVariant;
  int lobbyLaunchDiff = 1;
  uint64_t lobbyLaunchSkulls = 0;
  std::string lobbyLaunchMapOverride;
  std::string lobbyLaunchModFolder;
  std::string lobbyLaunchModId;
  double lastLobbyPoll = 0.0;
  int lobbyPollFailures =
      0; // consecutive failed lobby-state fetches; a guest only leaves after 3
  int lobbyMenuRow = 0;   // Focused row in pre-game options list
  int lobbyFocusZone = 0; // 0 = left options list, 1 = right session roster
  // Playlists. -1 = none chosen.
  std::vector<Playlists::Playlist> playlists;
  int playlistChoice = -1;
  int playlistPosition = 0;
  bool playlistEditOpen = false;
  Playlists::Playlist playlistEdit;
  char playlistName[48] = {};
  int playlistEditRow = 0, playlistEditMap = 0, playlistEditVariant = 0,
      playlistEditEntry = 0;
  std::string playlistMessage;
  // Voting in the lobby.
  std::string voteId;
  std::vector<std::string> voteOptions;
  std::vector<int> voteCounts;
  int voteMsLeft = 0;
  int voteRow = 0;
  int voteCast = -1;
  int voteRerolls = 0;
  bool voteWasUp = false;
  std::vector<Playlists::Entry>
      voteEntries; // host only: what each option plays
  std::string voteMessage;
  // What the lobby's variant is, worked out once per variant: teams on or off,
  // and a player-made mode's changed rules.
  std::string lobbyModeKey;
  bool lobbyTeamsOn = false;
  int lobbyTeamCount =
      2; // teams this lobby deals (2..8, Halo: Reach's multi-team modes)
  bool lobbyTeamsManual = false; // the host has assigned teams by hand; don't
                                 // re-deal them automatically
  int lobbyModeChanges = 0;
  std::string lobbyModeSummary;
  bool teamsPushFailed = false;
  double autoStartAt =
      0.0;
  // RESTART MISSION / NEXT MISSION with everyone (campaign, Firefight): the
  // countdown holds until this many players are back in the lobby, at most
  // until autoStartWaitLimit (ImGui time). 0 = don't wait.
  int autoStartNeedPlayers = 0;
  // NEXT MAP VOTE: ImGui time the lobby's map vote opens by itself after a
  // playlist match; 0 = not set.
  double autoVoteAt = 0.0;
  double autoStartWaitLimit = 0.0; // ImGui time the next rotation match starts; 0 = not counting
  std::filesystem::path
      optionsFile; // the saved mode being edited; empty for a new one
  std::string
      confirmAction; // a save-over or delete waiting for its second press
  bool optionsOpen =
      false; // Custom Game: the game options editor replaces the setup screen
  bool pendingInviteJoin = false;
  std::string pendingInviteConnect;
  std::string pendingInviteSession;
  std::string pendingInviteMap;
  std::string pendingInviteMode;
  int optionsRow = 0;
  char optionsName[48] = {};
  std::string optionsBase;             // base variant file name
  std::filesystem::path optionsSource; // the .bin the editor decoded (may be
                                       // the writable game_variants folder)
  std::vector<uint8_t> optionsDecoded; // the base variant's real values
  std::map<std::string, int> optionsValues; // only what differs from the base
  std::string optionsMessage;
  char optionsSearch[48] =
      {}; // filters the list by option name, description or category
  bool optionsChangedOnly =
      false; // list only the options that differ from the base
  std::set<std::string> optionsOpenGroups; // folded-open category headers
  int optionsFirst =
      0; // first list row drawn (the list scrolls with the focus)
  int optionsLastFocus = -1; // focus seen last frame: the view only follows the
                             // focus when it moves
  int optionsPresetPick = 0; // focused preset button
  char mapSearch[64] = {};   // the map, mission and mod lists' search text
  int mapSearchRow = 0;
  char variantSearch[64] = {};    // the game type lists' search text
  std::string variantOpenMode;    // the one game type category dropped down
  bool variantPickerOpen = false; // Custom Game / Firefight: the full game type
                                  // list replaces the setup screen
  int variantPickerRow = 0;
  bool variantPickerModded = false;
  std::string variantPickerMap; // the map it filters for and its display name
  std::string variantPickerMapName;
  bool lobbyPickerOpen =
      false; // The game type list is showing in place of the option rows
  int lobbyPickerRow = 0;
  bool lobbyTeamPickerOpen = false; // the CHANGE TEAM dropdown is showing
  int lobbyTeamPickerRow = 0;
  double lobbyTeamPickerOpenedAt =
      0.0; // ImGui time it opened: its first frames ignore clicks
  bool lobbyProfileOpen =
      false; // The selected player's card is showing in place of the roster
  bool lobbyReady = true;       // Player ready toggle
  int lobbyRosterSelection = 0; // Selected player index in party roster
  int lobbyRosterScroll[2] =
      {}; // First row shown in each roster column (red or all, blue)
  // Social screen state
  int socialTab = 0; // 0 = Friends, 1 = Requests, 2 = Find Players
  char socialSearchBuf[64] = "";
  std::vector<UserSearchResult> socialSearchResults;
  bool socialSearching = false;
  double lastSocialPoll = 0.0;
  Screen socialPreviousScreen = Screen::Main;
  // Theater screen state
  int theaterSource =
      0; // 0 = All, 1 = Recent Films, 2 = Saved Films, 3 = Local Videos
  int theaterIndex = 0; // Selected row in Theater list
  char theaterSearch[64] = "";
  int theaterSearchRow = 0;
  std::string theaterStatusMsg;
  std::string theaterConfirmDelete;

  // Unified tile map picker & drawer state (Campaign / Firefight / CustomGame /
  // Forge / Lobby)
  std::string mapDrawerKey; // key of map tile currently expanded to show [BASE
                            // MAP] + variants
  int mapDrawerFocus =
      -1; // -1 = map grid tile focused, 0 = [BASE MAP], 1..N = variant index
  char mapDrawerFilter[64] = {}; // filter text inside the expanded [BASE MAP] + variants drawer
  int mapDrawerCategory = 0;     // 0 = all, 1..N = forge/variant category inside the drawer
  float mapGridScrollY =
      0.0f; // vertical scroll offset of 2-column map tile grid
  std::filesystem::path mpSelectedMvar; // Custom Game active .mvar on current
                                        // vanilla/mod map (empty = base map)
  std::filesystem::path forgeSelectedMvar; // Forge active .mvar on current
                                           // vanilla/mod map (empty = base map)
  std::filesystem::path
      modSelectedMvar;             // active .mvar when browsing modded maps
  bool lobbyMapPickerOpen = false; // Pre-Game Lobby modal tile map picker
  int optionsCategoryIdx =
      0; // Active category tab index in tiled Game Options editor
  int optionsFocusZone =
      0; // 0 = category list on left, 1 = option rows / action buttons
  int optionsRuleFocus =
      0; // Focused rule index within active Game Options category
  bool optionsRuleDropdownOpen =
      false; // Optional quick dropdown for level/multi-value rules
};


struct OverlayContext {
  bool initialized = false;
  std::filesystem::path reachDir;
  std::vector<MenuVariant> variants;
  std::vector<MenuVariant> ffVariants;
  std::vector<ForgeMap> forgeMaps;
  bool forgeMapsScanned = false; // the scan is lazy: the maps menu needs it
                                 // before the Forge tab is opened
  std::filesystem::path forgeVariant;
  std::vector<TheaterEntry> theaterEntries;
  bool theaterScanned = false;
  bool launchedFromTheater = false;
  std::vector<ModEntry> mods;
  std::vector<const ModEntry *> campaignMods;
  std::vector<const ModEntry *> ffMods;
  std::vector<const ModEntry *> mpMods;
  MenuState st;
  SettingsPanel settingsPanel;
  CareerPanel careerPanel;
  bool modsScanned = false;
  unsigned long long captureFrame = 0;
  unsigned long long totalFrames = 0;
  unsigned long long sceneReadyFrames = 0;
  bool closed = false;
  void *hMatchProcess = nullptr;
  bool matchRunning = false;
  bool hasPendingJoin = false;
  FrontEndChoice pendingJoinChoice;
};


struct VariantRow {
  const MenuVariant *variant = nullptr; // nullptr on a heading
  std::string mode;
  std::string type;
};
struct VariantChoices {
  std::vector<VariantRow> rows;
  std::vector<ListItem> items;
};

struct ResolvedEntry {
  MapEntry map;
  const MenuVariant *variant = nullptr;
};

struct MapTileGroup {
  MapEntry base;
  std::string key;
  std::vector<MapEntry> variants;
  std::string groupHeading;
  // Collapsed mod section: a single full-width banner (base.header is set and base.mod points at the mod) listing how
  // many maps the mod has. Clicking it expands the section (inserts the mod into MenuState::openMods) and the maps
  // appear as tiles on the next frame.
  int sectionMapCount = 0;
};


struct TacticalGridResult {
  bool pickedBaseOrVariant = false;
  bool moveFocusRight = false;
};


// Shared cross-module UI state
inline bool g_menuMusicMuted = false;
inline bool g_showAccountModal = false;
inline bool g_showDevMenu = false;
inline bool g_pickerOpenNow = false;
inline bool g_pickerBlocksNav = false;

// Core helpers (FrontEndMenu.cpp)
ImFont *Font(ImFont *f);
float TextWidth(ImFont *font, float px, const char *text);
std::string NumberWithCommas(int64_t v);
void Text(const Ui &ui, ImFont *font, float px, ImVec2 pos, ImU32 col,
          const char *text, float wrapWidth = 0.0f);
std::string Ellipsize(ImFont *font, float px, const std::string &text,
                      float maxWidth);
std::string FitWrapped(ImFont *font, float px, const std::string &text,
                       float maxWidth, float maxHeight);
std::string Upper(std::string s);
std::string PlainDescription(const std::string &in, size_t maxChars = 160);
float BlurbBlockHeight(const Ui &ui, float width, const char *blurb);
std::wstring Lower(std::wstring s);
std::string VariantPathType(const std::filesystem::path &path);
std::filesystem::path VariantBasePath(const std::filesystem::path &reachDir,
                                      const std::string &base);
int FindVariant(const std::vector<MenuVariant> &list, const wchar_t *file);
std::vector<MenuVariant> ScanVariants(const std::filesystem::path &reachDir,
                                      bool firefight);
OverlayContext &GetOverlayCtx();
std::vector<ForgeMap> ScanForgeMaps();
void DrawTitle(const Ui &ui, ImVec2 pos, const char *title,
               const char *sideLabel = nullptr);
void DrawTopTelemetryBar(const Ui &ui, const char *centerTag);
int DrawList(const Ui &ui, const char *id, ImVec2 pos, float width,
             const std::vector<ListItem> &items, int &selected, bool keyboard,
             float rowHeight = 30.0f, int maxRows = 0,
             bool hoverSelects = true);
bool DrawSourceTabs(const Ui &ui, ImVec2 pos, int &active, int moddedCount,
                    int forgeCount = -1, float *tabsEnd = nullptr);
bool HintClicked(Prompt p);
bool HintExtraClicked(const char* label);
bool AcceptPressed();
void DrawHints(const Ui &ui,
               const std::vector<std::pair<Prompt, const char *>> &hints,
               const std::vector<HintExtra> &extras = {},
               float left = 0.075f);
void OpenAccountModal();
float PlayerRow(const Ui &ui, ImVec2 pos, float width, int rankIndex,
                int emblemIndex, int nameplateIndex, int team,
                const std::string &serviceTag, const std::string &name,
                bool dev, bool selected, int emblemBackground = 0,
                const UiArt::EmblemColors &emblemColors = {},
                bool tester = false);
std::string PlayerServiceTag(const SettingsData &sd);
void DrawParty(const Ui &ui, const std::string &playerName,
               const char *status);
float DifficultyTiles(const Ui &ui, const char *id, ImVec2 pos, float width,
                      int &difficulty, bool focused);
int OptionRow(const Ui &ui, const char *id, ImVec2 pos, float width,
              const char *label, const char *value, bool focused,
              const char *note = nullptr, bool arrows = true);
void DrawMiddleHint(const Ui &ui, ImVec2 pos, float width, const char *value,
                    bool hovered, bool active, bool down = false);
int ClickRow(const Ui &ui, const char *id, ImVec2 pos, float width,
             const char *label, const char *value, bool focused, bool *clicked,
             bool down = false);
// `enabled`, when given, is asked per choice index: a disabled choice is drawn dim in the list, cannot be
// picked and the arrow/nav stepping skips it. `disabledReason` is shown under the choices (only while a choice
// is disabled) so the player can read why a value is unavailable instead of just finding it dead.
int PickerRow(
    const Ui &ui, const char *id, ImVec2 pos, float width, const char *label,
    const char *value, bool focused,
    const std::function<void(std::vector<std::string> &, int &)> &fill,
    int *picked, bool openNow = false, const char *note = nullptr,
    bool arrowHint = true,
    const std::function<bool(int)> &enabled = {},
    const char *disabledReason = nullptr);
// How many seats the next multiplayer session can really declare. Stock engines seat 16; the 17-32 path is a
// dev-only 32-player slot patch (src/host/SlotPatch.cpp) where a 17th player still cannot join, so the picker,
// lobby header, advert and server-browser row all clamp to this number instead of the picked one.
int EffectiveMultiplayerCap();
bool ActionButton(const Ui &ui, const char *id, ImVec2 pos, float width,
                  const char *label, bool primary = true,
                  bool focused = false);
float HeroBanner(const Ui &ui, ImVec2 pos, float width, float height,
                 const std::string &scenario, const std::string &kicker,
                 const char *title, const char *blurb,
                 const std::filesystem::path &image = {});
bool BigAction(const Ui &ui, const char *id, ImVec2 pos, float width,
               const char *label, const char *sub, bool primary, bool enabled,
               bool focused = false);
float SkullStrip(const Ui &ui, ImVec2 pos, float size, uint64_t mask,
                 int limit = 6);
std::string SkullSummary(uint64_t mask);
float ScoreMultiplier(int difficulty, uint64_t mask);
float LobbyStripHeight(const Ui &ui);
void DrawLobbyControls(const Ui &ui, ImVec2 pos, float width, MenuState &st);
bool KnownScenario(const std::string &scenario, const MenuMap *maps,
                   int count);
std::vector<MapEntry> BaseMaps(const MenuMap *maps, int count);
std::string OpenKey(ModMode mode, const ModEntry &mod);
std::vector<MapEntry> ModMaps(const ModEntry &mod,
                              const std::vector<ModMap> &listed,
                              const MenuMap *base, int baseCount,
                              const std::string &carrier);
std::vector<MapEntry> ModdedMaps(const std::vector<const ModEntry *> &mods,
                                 ModMode mode, const MenuMap *base,
                                 int baseCount,
                                 const std::set<std::string> &open,
                                 const std::string &carrier);
bool ContainsNoCase(std::string text, std::string needle);
bool DrawSearchBox(const Ui &ui, const char *id, ImVec2 pos, float width,
                   char *text, size_t size, const char *hint);
bool RequestMatchLaunch(FrontEndChoice &outChoice, bool &done,
                        const FrontEndChoice &req);
bool AutoPlayNow();
std::string AutoPlayMap();
std::string AutoPlayVariantStem();
void GoBackScreen(MenuState &st, Screen to);

// Pickers (FrontEndPickers.cpp)
void BeginVariantList(MenuState &st, const std::vector<MenuVariant> &list,
                      const std::string &scenario, bool unrestricted,
                      const MenuVariant *current);
const MenuVariant *DrawVariantList(
    const Ui &ui, MenuState &st, const std::vector<MenuVariant> &list,
    const std::string &scenario, bool unrestricted, ImVec2 pos, float width,
    float rowHeight, int rowCount, bool keyboard, VariantRow &shown);
void DrawVariantInfo(const Ui &ui, ImVec2 pos, float width,
                     const VariantRow &row, bool compact = false);
std::string RuleValueText(const VariantRules::Rule &rule, int value);
void OpenGameOptions(OverlayContext &ctx, const MenuVariant &current);
void RefreshPlaylists(OverlayContext &ctx);
std::vector<MapEntry> AllMultiplayerMaps(OverlayContext &ctx);
bool ResolveEntry(OverlayContext &ctx, const Playlists::Entry &entry,
                  bool randomType, ResolvedEntry &out);
void ApplyEntryToLobby(MenuState &st, const ResolvedEntry &e);
std::vector<MapTileGroup>
BuildMapTileGroups(OverlayContext &ctx, bool modded, bool forgeTab,
                   ModMode modMode, const std::vector<const ModEntry *> &modList,
                   const MenuMap *baseMaps, int baseCount,
                   const std::string &carrier, bool attachForgeVariants,
                   const char *searchQuery);
TacticalGridResult DrawTacticalMapGrid(
    const Ui &ui, MenuState &st, ImVec2 gridPos, ImVec2 gridSize,
    const std::vector<MapTileGroup> &groups, int &selectedGroupIdx,
    std::filesystem::path &selectedMvar, bool hasFocus, const char *emptyText,
    const char *baseItemTag, bool autoScrollSelection = false,
    ModMode openMode = ModMode::Multiplayer);
float DrawMapDetailHeaderCard(const Ui &ui, ImVec2 cardPos, float cardW,
                              float previewH, const MapEntry *baseMap,
                              const MapEntry *variantMap,
                              const char *fallbackModeLabel);
void DrawTacticalGameOptions(OverlayContext &ctx, const Ui &ui, float left,
                             float top, bool ff, bool forge, bool back);
void DrawCampaignScreen(OverlayContext &ctx, const Ui &ui, float left,
                        float top, FrontEndChoice &choice, bool &done,
                        bool back);
void DrawSkullsScreen(OverlayContext &ctx, const Ui &ui, float left, float top,
                      const SettingsData &settings, bool back);
void DrawFirefightCustomForgeScreen(OverlayContext &ctx, const Ui &ui,
                                    float left, float top,
                                    FrontEndChoice &choice, bool &done,
                                    bool back);

// Lobby (FrontEndLobby.cpp)
void ClearPendingLobbyEvents();
void TakePendingLobbyEvents(MenuState &st);
void EnqueueLobbyLaunch(const std::string &lMap, const std::string &lMode,
                        const std::string &lVar, int lDiff, uint64_t lSkulls,
                        const std::string &hIp, uint16_t hPort,
                        const std::string &sId, const std::string &lMapOverride,
                        const std::string &lModFolder,
                        const std::string &lModId,
                        const std::string &lMapVariant = "");
void EnqueueLobbyClosed(const std::string &reason);
void EnqueueLobbyRoster(std::vector<LobbyPlayer> &&updated);
void SetupHostLobby(
    MenuState &st, const std::string &mapScenario, const std::string &mode,
    const std::string &varPath, const std::string &varName, int diff,
    uint64_t skulls, int maxPlayers, const ModEntry *mod = nullptr,
    const std::filesystem::path &mapOverride = {},
    const std::string &mapTitle = {},
    const std::filesystem::path &mapImage = {},
    const std::filesystem::path &mapThumb = {},
    const std::string &existingSessionId = {},
    const std::string &existingSessionToken = {});
void EnterLobby(OverlayContext &ctx, const ServerEntry &s,
                const std::string &modId = {},
                const std::string &modTitle = {},
                const std::filesystem::path &modFolder = {});
void RefreshLobbyMode(OverlayContext &ctx);
void AssignLobbyTeams(MenuState &st);
void StartLobbyVote(OverlayContext &ctx);
void FinishLobbyVote(OverlayContext &ctx);
void DrawPreGameLobby(OverlayContext &ctx, const Ui &ui, float left, float top,
                      FrontEndChoice &choice, bool &done, bool back);

// Social / Onboarding / Changelog / Music / Invites (FrontEndSocial.cpp)
void DrawSocialScreen(OverlayContext &ctx, const Ui &ui, float left, float top,
                      FrontEndChoice &choice, bool &done, bool back);
bool DrawFirstStart(const Ui &ui);
bool ShouldShowChangelogModal(bool manualOpen, bool firstStartActive);
void DrawChangelogModal(const Ui &ui, bool &ioManualOpen);
void DrawMusicPlayer(const Ui &ui);
void DrawInviteToast(const Ui &ui, MenuState &st);

} // namespace noble::fe
