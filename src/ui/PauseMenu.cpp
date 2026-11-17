#include "PauseMenu.hpp"
#include "../engine/DevCheats.hpp"
#include "FrontEndMenu.hpp"
#include "InputPrompts.hpp"
#include "LoadScreen.hpp"
#include "MenuSounds.hpp"
#include "NobleStyle.hpp"
#include "NobleUi.hpp"
#include "SettingsPanel.hpp"
#include "SocialOverlay.hpp"
#include "UiArt.hpp"
#include "UiPad.hpp"
#include "TeamColors.hpp"
#include "audio/ReachAudio.hpp"
#include "audio/VoiceChat.hpp"
#include "career/Career.hpp"
#include "common/Logger.hpp"
#include "common/EngineModule.hpp"
#include "common/MapNames.hpp"
#include "common/Relaunch.hpp"
#include "common/Settings.hpp"
#include "engine/BlamEngine.hpp"
#include "engine/CameraThirdPerson.hpp"
#include "engine/ForgePlacement.hpp"
#include "engine/FrameCapture.hpp"
#include "engine/KeyBindings.hpp"
#include "engine/ReachHost.hpp"
#include "engine/SessionVariant.hpp"
#include "hooks/LobbyPatches.hpp"
#include "host/Window.hpp"
#include "mvar/ForgePlacementApply.hpp"
#include "mvar/MapVariant.hpp"
#include "mvar/MapVariantLibrary.hpp"
#include "mvar/MapVariantWriter.hpp"
#include "network/MasterServerClient.hpp"
#include "network/NetworkTransport.hpp"
#include "network/SessionManager.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <d3d11.h>
#include <dxgi.h>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <mutex>
#include <nlohmann/json.hpp>
#include <set>
#include <shellapi.h>
#include <string>
#include <thread>

namespace noble {

namespace {

std::atomic<bool> s_open{false};
std::atomic<bool> s_isLoadingActive{false};
// The challenge list shown on the pause menu is built once when the menu opens
// and then every two seconds, not on every frame (building it takes the career
// lock and may read the career file).
std::atomic<bool> s_challengesStale{true};
std::atomic<bool> s_gameOver{false};
std::atomic<bool> s_scoreboardShotRequest{false};
std::atomic<bool> s_boardPointer{false}; // scoreboard actions own the pointer
std::atomic<bool> s_boardReport{false};  // the Report window is up
std::atomic<bool> s_showSettings{
    false};               // the SETTINGS page opened from the pause menu
static bool s_teamPickerOpen = false;
static int s_teamPickerRow = 0;
static ULONGLONG s_teamPickerOpenedAt = 0;
// Render (engine thread) vs HoldFrame (main thread). Recursive because the post-game state is also
// touched from inside RenderImpl (guest match_over handling) and by ShowGameOver /
// RefreshGameOverAward, which must not interleave with a frame being drawn.
std::recursive_mutex s_renderMutex;
std::mutex s_rtvMutex;    // protects cached DX11 RTV / back buffer across ResizeBuffers & Shutdown
IDXGISwapChain *s_cachedSwapChain = nullptr;
ID3D11Texture2D *s_cachedBackBuffer = nullptr;
ID3D11RenderTargetView *s_cachedRtv = nullptr;
UINT s_cachedRtvWidth = 0;
UINT s_cachedRtvHeight = 0;
DXGI_FORMAT s_cachedRtvFormat = DXGI_FORMAT_UNKNOWN;
std::atomic<IDXGISwapChain *> s_swapChain{
    nullptr}; // last swap chain the engine presented
std::atomic<ULONGLONG> s_lastEngineFrame{
    0}; // GetTickCount64 of the last slot-1 frame
std::atomic<ULONGLONG> s_gameOverAt{0};
std::atomic<bool> s_resetGameOverImGui{false};
ULONGLONG s_gameOverSettleUntil = 0;
int s_gameOverPadButton = -1;
double s_gameOverPadArmedAt = 0.0;
bool s_guestAutoReturned = false;
// Set by the transport callbacks (relay thread) and handled on the render thread or, once the engine
// has stopped drawing, on the main thread (PauseMenu::PollHostMatchState).
std::atomic<bool> s_guestShouldReturnLobby{false};
std::atomic<bool> s_guestShouldDisband{false};
std::atomic<bool> s_lobbyCallbacksRegistered{false};
std::atomic<bool> s_guestReturnedToLobby{false};   // PollHostMatchState started a relaunch
bool s_gameOverWaitRelease = false;
bool s_autoCancelled = false;
double s_autoAt = 0.0;
bool s_autoInited = false;
std::mutex s_mapMutex;
std::string s_loadingMap;
std::string s_loadingMapName;
std::string s_displayMapName;
std::filesystem::path s_loadingImage;
std::filesystem::path s_loadingThumb;
std::string
    s_loadingDescription; // mod map blurb for the loading screen (plain text)
// Session context for the loading screen: mode label (top right), the lobby roster (names of the players this launch
// is for, host first), how many players the match was launched for, and whether this process hosts it.
std::string s_loadingMode;                  // "campaign" / "firefight" / "forge" / "multiplayer"
std::vector<std::string> s_loadingPlayers;  // lobby roster names, host first (may be empty)
int s_loadingExpectedPlayers = 0;           // total players this launch waits for (0/1 = solo)
bool s_loadingHost = true;
bool s_loadingJoiner = false;               // this process joins a remote host

// True once the engine has passed its loading screen but the host is still waiting for the remote players this launch
// expects to join membership: the loading screen keeps drawing, now as the WAITING FOR PLAYERS screen, instead of the
// black frame the engine shows on its own.
bool LoadingWaitingForPlayers() {
  if (BlamEngine::IsSavedFilmPlayback()) return false;
  return BlamEngine::IsWaitingForRemotePlayers();
}

struct MapName {
  const char *scenario;
  const char *name;
};

// Display names for the loading screen (same names as the front-end menu).
const MapName kMapNames[] = {
    {"50_panopticon", "Boardwalk"},
    {"70_boneyard", "Boneyard"},
    {"45_launch_station", "Countdown"},
    {"30_settlement", "Powerhouse"},
    {"52_ivory_tower", "Reflection"},
    {"35_island", "Spire"},
    {"20_sword_slayer", "Sword Base"},
    {"45_aftship", "Zealot"},
    {"dlc_slayer", "Anchor 9"},
    {"dlc_invasion", "Breakpoint"},
    {"dlc_medium", "Tempest"},
    {"condemned", "Condemned"},
    {"trainingpreserve", "Highlands"},
    {"cex_beaver_creek", "Battle Canyon"},
    {"cex_damnation", "Penance"},
    {"cex_timberland", "Ridgeline"},
    {"cex_prisoner", "Solitary"},
    {"cex_hangemhigh", "High Noon"},
    {"cex_headlong", "Breakneck"},
    {"forge_halo", "Forge World"},
    {"m05", "Noble Actual"},
    {"m10", "Winter Contingency"},
    {"m20", "ONI: Sword Base"},
    {"m30", "Nightfall"},
    {"m35", "Tip of the Spear"},
    {"m45", "Long Night of Solace"},
    {"m50", "Exodus"},
    {"m52", "New Alexandria"},
    {"m60", "The Package"},
    {"m70", "The Pillar of Autumn"},
    {"m70_bonus", "Lone Wolf"},
    {"ff10_prototype", "Overlook"},
    {"ff20_courtyard", "Courtyard"},
    {"ff30_waterfront", "Waterfront"},
    {"ff45_corvette", "Corvette"},
    {"ff50_park", "Beachhead"},
    {"ff60_airview", "Outpost"},
    {"ff60_icecave", "Glacier"},
    {"ff70_holdout", "Holdout"},
    {"ff_unearthed", "Unearthed"},
    {"cex_ff_halo", "Installation 04"},
};

std::string DisplayName(const std::string &scenario) {
  for (const MapName &m : kMapNames) {
    if (scenario == m.scenario)
      return m.name;
  }
  return scenario;
}

std::string Upper(std::string s) {
  for (char &c : s)
    c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
  return s;
}

std::string LoadingModeLabel(const std::string &mode) {
  if (_stricmp(mode.c_str(), "campaign") == 0)
    return "CAMPAIGN";
  if (_stricmp(mode.c_str(), "firefight") == 0)
    return "FIREFIGHT";
  if (_stricmp(mode.c_str(), "forge") == 0 ||
      _stricmp(mode.c_str(), "sandbox") == 0)
    return "FORGE";
  return "CUSTOM GAME";
}

// Starts a new Noble (empty args = front-end menu) and closes this one.
// One value from a command line built by Relaunch ("-map x -variant \"a b\"").
std::wstring ArgValue(const std::wstring &args, const wchar_t *name) {
  std::wstring value;
  int argc = 0;
  LPWSTR *argv = CommandLineToArgvW((L"x " + args).c_str(), &argc);
  for (int i = 1; argv && i + 1 < argc; ++i) {
    if (_wcsicmp(argv[i], name) == 0)
      value = argv[i + 1];
  }
  if (argv)
    LocalFree(argv);
  return value;
}

// The arguments that bring this match's lobby back in a new process
// (FrontEndMenu reads the -lobby-* set).
std::wstring LobbyReturnArgs(bool host, bool advance = false) {
  const std::wstring game = Relaunch::CurrentGameArgs();
  const auto quoted = [](const std::wstring &value) {
    return L"\"" + value + L"\"";
  };
  std::wstring out = host ? L"-lobby-return host" : L"-lobby-return guest";
  const auto copy = [&](const wchar_t *from, const wchar_t *to) {
    std::wstring value = ArgValue(game, from);
    if (value.empty())
      value = ArgValue(GetCommandLineW(), from);
    if (!value.empty())
      out += std::wstring(L" ") + to + L" " + quoted(value);
  };
  copy(L"-map", L"-lobby-map");
  copy(L"-mode", L"-lobby-mode");
  copy(L"-variant", L"-lobby-variant");
  copy(L"-difficulty", L"-lobby-difficulty");
  copy(L"-skulls", L"-lobby-skulls");
  copy(L"-mod", L"-lobby-mod-folder");
  copy(L"-map-override", L"-lobby-map-override");
  copy(L"-map-title", L"-lobby-map-title");
  copy(L"-map-image", L"-lobby-map-image");
  copy(L"-map-thumb", L"-lobby-map-thumb");

  // Keep session id and token for both host and guest so the session is
  // preserved!
  copy(L"-session-id", L"-lobby-session");
  copy(L"-session-token", L"-lobby-session-token");

  if (host) {
    copy(L"-max-players", L"-lobby-max-players");
    const std::wstring own = GetCommandLineW();
    out += (game.find(L" -lan") != std::wstring::npos ||
            own.find(L" -lan") != std::wstring::npos)
               ? L" -lobby-net lan"
               : L" -lobby-net online";
    out += (game.find(L" -private") != std::wstring::npos ||
            own.find(L" -private") != std::wstring::npos)
               ? L" -lobby-privacy private"
               : L" -lobby-privacy public";
    // Only the menu that started this match knows the playlist, so it's on this
    // process's own command line.
    const std::wstring list = ArgValue(own, L"-lobby-playlist");
    if (!list.empty()) {
      out += L" -lobby-playlist " + quoted(list) + L" -lobby-playlist-pos " +
             quoted(ArgValue(own, L"-lobby-playlist-pos"));
      if (advance)
        out += L" -lobby-advance 1";
    }
  } else {
    std::wstring endpoint = ArgValue(game, L"-connect");
    if (endpoint.empty())
      endpoint = ArgValue(GetCommandLineW(), L"-connect");
    if (!endpoint.empty())
      out += L" -lobby-endpoint " + quoted(endpoint);
  }
  return out;
}

static std::atomic<int> s_hostLobbyState{0};

void SetHostBackInLobby(int st) { s_hostLobbyState.store(st); }

// For a guest on the post-game panel: 0 still waiting, 1 the host is back in
// the lobby, 2 the session has gone. The server says the match is running until
// the host's new process registers again.
int PollHostBackInLobby() {
  if (s_hostLobbyState.load() != 0)
    return s_hostLobbyState.load();
  static bool s_started = false;
  if (!s_started) {
    s_started = true;
    const std::string session = [] {
      std::wstring wide = ArgValue(Relaunch::CurrentGameArgs(), L"-session-id");
      if (wide.empty())
        wide = ArgValue(GetCommandLineW(), L"-session-id");
      char text[128] = {};
      WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, text, sizeof(text) - 1,
                          nullptr, nullptr);
      return std::string(text);
    }();
    if (session.empty()) {
      // In LAN / direct connect mode where no master server session was
      // created: poll LAN discovery to see when host has returned to lobby
      std::thread([] {
        for (int i = 0; i < 25; ++i) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          auto sessions = SessionManager::Get().GetDiscoveredSessions();
          if (!sessions.empty()) {
            NOBLE_INFO(
                "PauseMenu: detected host LAN lobby, pulling back to lobby");
            s_hostLobbyState = 1;
            return;
          }
        }
        s_hostLobbyState = 2;
      }).detach();
    } else {
      std::thread([session] {
        int missing = 0;
        while (true) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          const LobbyState state =
              MasterServerClient::Get().FetchLobbyStateSync(session);
          if (!state.ok || state.isClosed) {
            if (++missing >= 25) {
              NOBLE_WARN(
                  "PauseMenu: host lobby closed or timed out (missing: {})",
                  missing);
              s_hostLobbyState = 2;
              return;
            }
            continue;
          }
          missing = 0;
          if (!state.inProgress) {
            NOBLE_INFO("PauseMenu: the host is back in the lobby (session: {})",
                       session);
            s_hostLobbyState = 1;
            return;
          }
        }
      }).detach();
    }
  }
  return s_hostLobbyState.load();
}

// Finds " -name value" in a built command line. Returns false if it is not
// there; [begin, end) covers the separator, the name and the value (quoted or
// not).
bool FindArgSpan(const std::wstring &args, const std::wstring &name,
                 size_t &begin, size_t &end) {
  for (size_t at = args.find(name); at != std::wstring::npos;
       at = args.find(name, at + 1)) {
    const size_t after = at + name.size();
    if ((at != 0 && args[at - 1] != L' ') || after >= args.size() ||
        args[after] != L' ')
      continue;
    size_t value = after + 1;
    if (value < args.size() && args[value] == L'"') {
      const size_t close = args.find(L'"', value + 1);
      end = close == std::wstring::npos ? args.size() : close + 1;
    } else {
      const size_t space = args.find(L' ', value);
      end = space == std::wstring::npos ? args.size() : space;
    }
    begin = at != 0 ? at - 1 : 0;
    return true;
  }
  return false;
}

void RemoveArg(std::wstring &args, const std::wstring &name) {
  size_t begin = 0, end = 0;
  if (FindArgSpan(args, name, begin, end))
    args.erase(begin, end - begin);
}

void SetArg(std::wstring &args, const std::wstring &name,
            const std::wstring &value) {
  RemoveArg(args, name);
  if (!args.empty() && args.front() == L' ')
    args.erase(0, 1);
  args += (args.empty() ? L"" : L" ") + name + L" \"" + value + L"\"";
}

// The mission after this one in campaign order, or "" at the end of the
// campaign or for anything that isn't a stock mission. Missions can be named
// with a scene suffix ("m10_b").
std::wstring NextCampaignMission(const std::wstring &game) {
  static const wchar_t *const kOrder[] = {L"m05", L"m10", L"m20",      L"m30",
                                          L"m35", L"m45", L"m50",      L"m52",
                                          L"m60", L"m70", L"m70_bonus"};
  std::wstring mode = ArgValue(game, L"-mode");
  if (mode.empty())
    mode = ArgValue(GetCommandLineW(), L"-mode");
  if (mode.empty())
    mode = ArgValue(GetCommandLineW(), L"-lobby-mode");
  if (_wcsicmp(mode.c_str(), L"campaign") != 0 &&
      wcsstr(GetCommandLineW(), L"-campaign") == nullptr &&
      game.find(L"-campaign") == std::wstring::npos)
    return {};
  if (!ArgValue(game, L"-mod").empty() ||
      !ArgValue(game, L"-map-override").empty())
    return {};
  std::wstring map = ArgValue(game, L"-map");
  if (map.empty())
    map = ArgValue(GetCommandLineW(), L"-map");
  if (map.empty())
    map = ArgValue(GetCommandLineW(), L"-lobby-map");
  std::transform(map.begin(), map.end(), map.begin(),
                 [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
  constexpr int kCount = static_cast<int>(sizeof(kOrder) / sizeof(kOrder[0]));
  int at = -1;
  for (int i = 0; i < kCount; ++i)
    if (map == kOrder[i])
      at = i;
  if (at < 0) {
    for (int i = 0; i < kCount; ++i)
      if (map.rfind(std::wstring(kOrder[i]) + L"_", 0) == 0)
        at = i;
  }
  return at >= 0 && at + 1 < kCount ? kOrder[at + 1] : L"";
}

std::wstring MissionTitle(const std::wstring &mission) {
  char narrow[64] = {};
  WideCharToMultiByte(CP_UTF8, 0, mission.c_str(), -1, narrow,
                      sizeof(narrow) - 1, nullptr, nullptr);
  const std::string title = MapNames::Display(narrow);
  wchar_t wide[128] = {};
  MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, wide, 127);
  return wide;
}

// Arguments that start the next mission, or "" if there is none. A hosted game
// goes back to its lobby with the next mission selected, so guests stay
// together; a solo game starts it straight away. Difficulty and skulls carry
// over.
std::wstring NextMissionArgs(const std::wstring &next, std::wstring lobbyArgs) {
  if (next.empty())
    return {};
  const std::wstring title = MissionTitle(next);
  std::wstring game = Relaunch::CurrentGameArgs();
  if (game.find(L" -host") != std::wstring::npos) {
    SetArg(lobbyArgs, L"-lobby-map", next);
    SetArg(lobbyArgs, L"-lobby-map-title", title);
    RemoveArg(lobbyArgs, L"-lobby-map-image");
    RemoveArg(lobbyArgs, L"-lobby-map-thumb");
    return lobbyArgs;
  }
  SetArg(game, L"-map", next);
  SetArg(game, L"-map-title", title);
  RemoveArg(game, L"-map-image");
  RemoveArg(game, L"-map-thumb");
  return game;
}

void RelaunchAndClose(HWND hwnd, const std::wstring &args, const char *what) {
  NOBLE_INFO("PauseMenu: {} requested", what);
  Career::EnsureMatchSaved(what);
  const bool isLobbyReturn = args.find(L"-lobby-return") != std::wstring::npos;
  const bool keepHostSession =
      args.find(L"-lobby-return host") != std::wstring::npos;
  const bool isMainMenu = (args == L"-main-menu");
  // A host going back to the lobby (or on to the next mission) tells the party right away, so the
  // guests' post-game panels unlock RETURN TO LOBBY instead of waiting for the master server to
  // notice the session is out of its match.
  if (keepHostSession && NetworkTransport::Get().IsActive() &&
      NetworkTransport::Get().IsHost()) {
    NetworkTransport::Get().SendLobbyControlJson(
        "{\"type\":\"host_return_to_lobby\"}");
  }
  if (args.empty() || isMainMenu) {
    // Leaving match to menu: notify master server and peers immediately without
    // blocking the UI thread.
    std::wstring wSession =
        ArgValue(Relaunch::CurrentGameArgs(), L"-session-id");
    if (wSession.empty())
      wSession = ArgValue(GetCommandLineW(), L"-session-id");
    const bool isHostCmd =
        (wcsstr(GetCommandLineW(), L"-host") != nullptr ||
         Relaunch::CurrentGameArgs().find(L" -host") != std::wstring::npos);
    if (!wSession.empty()) {
      char sessUtf8[128] = {};
      WideCharToMultiByte(CP_UTF8, 0, wSession.c_str(), -1, sessUtf8,
                          sizeof(sessUtf8) - 1, nullptr, nullptr);
      const std::string sId(sessUtf8);
      if (isHostCmd) {
        NetworkTransport::Get().SendLobbyControlJson(
            "{\"type\":\"lobby_disband\"}");
        std::thread([sId]() {
          MasterServerClient::Get().DisbandSessionSync(sId);
        }).detach();
      } else {
        uint64_t fallbackId =
            (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL);
        if (fallbackId < 2ULL)
          fallbackId = 2ULL;
        NetworkTransport::Get().SendDisconnectPacket();
        std::thread([sId, fallbackId]() {
          MasterServerClient::Get().LeaveLobbySync(sId, fallbackId);
        }).detach();
      }
    } else if (!isHostCmd) {
      NetworkTransport::Get().SendDisconnectPacket();
    }
  }

  std::wstring containerVal = ArgValue(GetCommandLineW(), L"-container-hwnd");
  if (!containerVal.empty()) {
    // The launcher process owns the lobby/session: shutting it down here can block on HTTP/WebSocket
    // joins and made the post-match buttons look dead. Disconnect, hand the relaunch over and exit.
    NetworkTransport::Get().ShutdownForExit();
    if (isMainMenu || args.empty()) {
      Relaunch::RequestContainerRelaunch(L"-main-menu");
      NOBLE_INFO("PauseMenu: {} in container mode - returning to main menu in "
                 "container launcher window",
                 what);
      TerminateProcess(GetCurrentProcess(), 0);
      return;
    } else if (isLobbyReturn) {
      Relaunch::RequestContainerRelaunch(args);
      NOBLE_INFO("PauseMenu: {} in container mode - returning to lobby in "
                 "container launcher window",
                 what);
      TerminateProcess(GetCurrentProcess(), 0);
      return;
    } else {
      // PLAY AGAIN / NEXT MISSION / RESTART MATCH: a hosted match cannot start
      // its own successor (the launcher tracks one match process and would
      // bring its menu back over the new game), so record what to start and let
      // the launcher window start it once this process is gone.
      std::wstring childArgs = args;
      if (childArgs.find(L"-container-hwnd") == std::wstring::npos) {
        childArgs += L" -container-hwnd " + containerVal;
      }
      if (Relaunch::RequestContainerRelaunch(childArgs)) {
        NOBLE_INFO("PauseMenu: {} in container mode - the launcher window "
                   "starts the next match",
                   what);
        TerminateProcess(GetCurrentProcess(), 0);
      } else {
        NOBLE_WARN("PauseMenu: {} in container mode - could not hand the "
                   "relaunch to the launcher",
                   what);
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
      }
      return;
    }
  }

  MasterServerClient::Get().Shutdown(keepHostSession);
  SessionManager::Get().Shutdown();
  NetworkTransport::Get().ShutdownForExit();
  Relaunch::HandOff(
      args); // ends this process; returns only if the new one could not start
  PostMessageW(hwnd, WM_CLOSE, 0, 0);
}

void QuitFromMatch(HWND hwnd, const char *what) {
  NOBLE_INFO("PauseMenu: {}", what);
  Career::EnsureMatchSaved(what);
  std::wstring wSession = ArgValue(Relaunch::CurrentGameArgs(), L"-session-id");
  if (wSession.empty())
    wSession = ArgValue(GetCommandLineW(), L"-session-id");
  const bool isHostCmd =
      (wcsstr(GetCommandLineW(), L"-host") != nullptr ||
       Relaunch::CurrentGameArgs().find(L" -host") != std::wstring::npos);
  if (!wSession.empty()) {
    char sessUtf8[128] = {};
    WideCharToMultiByte(CP_UTF8, 0, wSession.c_str(), -1, sessUtf8,
                        sizeof(sessUtf8) - 1, nullptr, nullptr);
    const std::string sId(sessUtf8);
    if (isHostCmd) {
      NetworkTransport::Get().SendLobbyControlJson(
          "{\"type\":\"lobby_disband\"}");
      std::thread([sId]() {
        MasterServerClient::Get().DisbandSessionSync(sId);
      }).detach();
    } else {
      uint64_t fallbackId =
          (MasterServerClient::GetLobbyMachineId() & 0x7FFFFFFFULL);
      if (fallbackId < 2ULL)
        fallbackId = 2ULL;
      NetworkTransport::Get().SendDisconnectPacket();
      std::thread([sId, fallbackId]() {
        MasterServerClient::Get().LeaveLobbySync(sId, fallbackId);
      }).detach();
    }
  } else if (!isHostCmd) {
    NetworkTransport::Get().SendDisconnectPacket();
  }

  std::wstring containerVal = ArgValue(GetCommandLineW(), L"-container-hwnd");
  if (!containerVal.empty()) {
    MasterServerClient::Get().Shutdown(false);
    SessionManager::Get().Shutdown();
    Relaunch::RequestContainerRelaunch(L"-quit-app");
    HWND root = hwnd ? GetAncestor(hwnd, GA_ROOT) : nullptr;
    if (root && root != hwnd) {
      PostMessageW(root, WM_CLOSE, 0, 0);
    }
    TerminateProcess(GetCurrentProcess(), 0);
    return;
  }
  MasterServerClient::Get().Shutdown(false);
  SessionManager::Get().Shutdown();
  NetworkTransport::Get().ShutdownForExit();
  PostMessageW(hwnd, WM_CLOSE, 0, 0);
}

ImVec2 CenteredWindow(float width) {
  const ImGuiIO &io = ImGui::GetIO();
  ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
      ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);
  return ImVec2(-1.0f, 38.0f);
}

constexpr ImGuiWindowFlags kPanelFlags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
    ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus;

// The SETTINGS page is the same panel the front-end menu shows, reused inside
// the pause menu so the player can change settings during a match (FOV, VSync,
// frame cap, mouse, ...). It lives for the life of the process because it keeps
// the edited copy and the key-rebind state between frames.
SettingsPanel &PauseSettings() {
  static SettingsPanel panel;
  return panel;
}

void DrawSettingsPage(HWND hwnd);

} // namespace

namespace {
// The engine has no pause API, but it does expose its game_speed console
// global. Zero freezes the simulation while the render/input thread keeps
// pumping, which is exactly what a local pause needs. Only used when the
// session has no remote players; an online match must keep running.
void SetLocalSimulationPaused(bool paused) {
  static bool s_applied = false;
  if (paused == s_applied)
    return;
  if (paused && NetworkTransport::Get().GetGamePeerCount() > 0)
    return;
  BlamEngine *engine = BlamEngine::Active();
  if (!engine)
    return;
  if (engine->ExecuteCommand(paused ? "game_speed 0" : "game_speed 1")) {
    s_applied = paused;
    NOBLE_INFO("PauseMenu: local simulation {}", paused ? "paused" : "resumed");
  }
}

static ULONGLONG s_openedAt = 0;
static int s_focus = 0;
static int s_pausePane = 0;       // 0 = action list, 1 = challenges panel
static int s_challengeFocus = 0;
static bool s_resetPauseScroll = false;
static bool s_waitForRelease = true;
static bool s_confirmRestart = false;
static std::string
    s_saveMapVariantResult; // shown under the SAVE MAP VARIANT row after a save
static bool s_saveVariantOpen = false; // the "name this map variant" box is up
static bool s_saveVariantJustOpened = false;
static bool s_saveVariantNeedsFocus = false;
static char s_saveVariantName[64] = "My Forge Map";
} // namespace

// Forge: save the map being edited as a map variant.
//
// The engine owns the save. Its Forge save writes a map variant file into the
// player's profile
// (<userdata>/<id>/saves/HaloReach/autosave/asq*.map), in the same BLF/"mvar"
// container as MCC's .mvar files - which is exactly what BlamEngine's
// parse_map_variant takes on the way in. Noble asks for the save with the
// engine's own "save_map" command, waits for the new file, and copies it into
// data\map_variants so it appears in the maps menu under the map it was built
// on. Forge: save the map being edited as a map variant.
//
// The engine owns the map data; its Forge save writes a map variant into the
// player's profile
// (<userdata>/<id>/saves/HaloReach/autosave/asq*.map) in the same BLF/"mvar"
// container MCC's .mvar files use, which is exactly what BlamEngine's
// parse_map_variant takes on the way in. Noble asks for that save with the
// engine's own "save_map" command, waits for the file on a background thread
// (waiting here froze the render thread for seconds), and stores it as JSON:
// {"version","name","base_map","mode","data"} with the variant base64-encoded -
// the same idea as Project Reclaimer's Halo 3 map files, so a saved map is one
// small readable file that can be shared.
namespace {
// The container around a captured engine chunk comes from an existing variant:
// BLF header + chdr + trailing chunks. The session's own .mvar is preferred;
// otherwise any shipped variant works, because the engine reads the map and
// objects from the mvar chunk that is being replaced.
// "Save as copy" never silently replaces an existing variant: append (2), (3),
// ... until the name is free.
std::string UniqueVariantName(const std::filesystem::path &library,
                              const std::string &requested) {
  namespace fs = std::filesystem;
  std::error_code ec;
  const std::string base = noble::mvar::SanitizeVariantName(requested);
  std::string candidate = base;
  for (int n = 2; n < 100; ++n) {
    if (!fs::exists(library / (candidate + ".json"), ec))
      return candidate;
    candidate = base + " (" + std::to_string(n) + ")";
  }
  return candidate;
}

// The map's build palette, harvested from variants that contain the definitions
// (tag -> variant/quota index).
noble::mvar::TagPalette LoadMapPalette(const std::filesystem::path &dataDir,
                                       uint32_t mapId) {
  noble::mvar::TagPalette palette;
  std::ifstream file(dataDir / "mvar_palette" /
                         (std::to_string(mapId) + ".json"),
                     std::ios::binary);
  if (!file)
    return palette;
  nlohmann::json json;
  try {
    json = nlohmann::json::parse(file);
  } catch (...) {
    return palette;
  }
  if (!json.contains("tags"))
    return palette;
  for (auto it = json["tags"].begin(); it != json["tags"].end(); ++it) {
    const std::string value = it.value().get<std::string>();
    const size_t comma = value.find(',');
    if (comma == std::string::npos)
      continue;
    palette[static_cast<uint32_t>(std::stoul(it.key(), nullptr, 16))] = {
        std::stoi(value.substr(0, comma)), std::stoi(value.substr(comma + 1))};
  }
  return palette;
}

std::filesystem::path
FindVariantTemplate(const std::filesystem::path &dataDir) {
  namespace fs = std::filesystem;
  std::error_code ec;
  const fs::path session = noble::SessionMapVariantPath();
  if (!session.empty() && fs::exists(session, ec))
    return session;
  const fs::path candidates[] = {
      dataDir / "map_variants",
      noble::ReachHost::Get().GetReachDir() / "map_variants",
      noble::ReachHost::Get().GetReachDir() / "hopper_map_variants"};
  for (const fs::path &dir : candidates) {
    for (const auto &entry : fs::directory_iterator(dir, ec)) {
      if (!entry.is_regular_file(ec))
        continue;
      if (entry.path().extension() != ".mvar")
        continue;
      return entry.path();
    }
  }
  return {};
}
} // namespace

std::filesystem::path PauseMenu::SaveForgeMapVariant(const std::string &name) {
  namespace fs = std::filesystem;
  std::error_code ec;
  const fs::path dataDir = ReachHost::Get().GetDataDir();
  const fs::path library = dataDir / "map_variants";

  // The engine owns the map data, so ask it to write the variant first and give
  // it a short window: when the hosted engine's save does run, those bytes are
  // the edited map. In this build it usually writes nothing at all
  // ("save_map" needs the engine's own save UI), so fall back to the variant
  // this session was launched with. That keeps SAVE working from Noble's own
  // pause menu instead of failing silently. MvarDump's WriteFile watch keeps
  // every buffer the engine hands to a save file under data/mvar_write; a fresh
  // one that parses as a variant is the edited map even when the file itself
  // ends up empty.
  const fs::path captureDir = fs::current_path() / "data" / "mvar_write";
  std::set<std::wstring> beforeCaptures;
  for (const auto &entry : fs::directory_iterator(captureDir, ec)) {
    if (entry.is_regular_file(ec))
      beforeCaptures.insert(entry.path().filename().wstring());
  }
  ec.clear();

  // Direct path first: serialize the live session variant ourselves. It does
  // not touch the save UI or the save device, so it is the only source that
  // works while the game is paused, and it carries the Forge edits.
  {
    // Forge commits placements on simulation ticks, and the pause menu freezes
    // the sim (game_speed 0). Let it run for a few frames so the live variant
    // contains anything placed since the last tick, then freeze again.
    if (BlamEngine::Active()) {
      BlamEngine::Active()->ExecuteCommand("game_speed 1");
      Sleep(150);
    }
    std::vector<uint8_t> liveChunk;
    const bool captured =
        BlamEngine::Active() &&
        BlamEngine::Active()->TrySerializeLiveMapVariant(liveChunk);
    if (BlamEngine::Active())
      BlamEngine::Active()->ExecuteCommand("game_speed 0");
    if (captured) {
      (void)noble::TakeForgePlacements();
      const fs::path tmpl = FindVariantTemplate(dataDir);
      if (!tmpl.empty()) {
        std::string detail;
        const fs::path out = noble::mvar::SaveVariantChunkToLibrary(
            liveChunk, tmpl, name, library, detail);
        if (!out.empty()) {
          if (out.extension() == ".json") {
            fs::path staleMvar = out;
            staleMvar.replace_extension(".mvar");
            fs::remove(staleMvar, ec);
          }
          noble::SetSessionMapVariantPath(out);
          NOBLE_INFO("PauseMenu: map variant '{}' saved from the live variant "
                     "to '{}' (template '{}')",
                     name, out.string(), tmpl.string());
          FrontEndMenu::InvalidateForgeMaps();
          return out;
        }
        NOBLE_WARN("PauseMenu: live variant chunk could not be saved: {}",
                   detail);
      } else {
        NOBLE_WARN("PauseMenu: no template .mvar to wrap the live variant in");
      }
    }
  }

  fs::path source;
  if (BlamEngine::Active()) {
    fs::path autosaveDir;
    for (const auto &entry : fs::directory_iterator(dataDir / "userdata", ec)) {
      if (!entry.is_directory(ec))
        continue;
      const fs::path candidate =
          entry.path() / "saves" / "HaloReach" / "autosave";
      if (fs::is_directory(candidate, ec)) {
        autosaveDir = candidate;
        break;
      }
    }
    std::set<std::wstring> before;
    if (!autosaveDir.empty()) {
      for (const auto &entry : fs::directory_iterator(autosaveDir, ec)) {
        if (entry.is_regular_file(ec))
          before.insert(entry.path().filename().wstring());
      }
    }

    const bool engineSave = BlamEngine::Active()->TriggerMapVariantSave();
    NOBLE_INFO("PauseMenu: engine map-variant save {}",
               engineSave ? "invoked" : "faulted");
    if (!engineSave)
      BlamEngine::Active()->ExecuteCommand("save_map");
    for (int attempt = 0;
         attempt < 20 && source.empty() && !autosaveDir.empty(); ++attempt) {
      Sleep(50);
      for (const auto &entry : fs::directory_iterator(autosaveDir, ec)) {
        if (!entry.is_regular_file(ec))
          continue;
        if (before.count(entry.path().filename().wstring()))
          continue;
        std::wstring ext = entry.path().extension().wstring();
        for (wchar_t &c : ext)
          c = static_cast<wchar_t>(::towlower(c));
        if (ext != L".map")
          continue;
        if (fs::file_size(entry.path(), ec) < 0x7329)
          continue;
        source = entry.path();
        NOBLE_INFO("PauseMenu: the engine wrote map variant '{}'",
                   entry.path().string());
        break;
      }
      ec.clear();
    }
    if (source.empty())
      NOBLE_WARN("PauseMenu: the engine wrote no map variant; using this "
                 "session's variant");
  }

  // Best source: the chunk the engine's serializer produced. Its save routine
  // builds that chunk from the live variant before handing it to the save
  // device, so it holds the edits even when the device write fails.
  std::vector<uint8_t> capturedChunk;
  if (source.empty() && BlamEngine::Active() &&
      BlamEngine::Active()->TakeCapturedMapVariantChunk(capturedChunk)) {
    const fs::path tmpl = FindVariantTemplate(dataDir);
    if (!tmpl.empty()) {
      std::string detail;
      const fs::path out = noble::mvar::SaveVariantChunkToLibrary(
          capturedChunk, tmpl, name, library, detail);
      if (!out.empty()) {
        NOBLE_INFO("PauseMenu: map variant '{}' saved from the engine "
                   "serializer to '{}' (template '{}')",
                   name, out.string(), tmpl.string());
        FrontEndMenu::InvalidateForgeMaps();
        return out;
      }
      NOBLE_WARN("PauseMenu: captured variant chunk could not be saved: {}",
                 detail);
    }
  }

  if (source.empty()) {
    std::vector<fs::path> fresh;
    for (const auto &entry : fs::directory_iterator(captureDir, ec)) {
      if (!entry.is_regular_file(ec))
        continue;
      if (beforeCaptures.count(entry.path().filename().wstring()))
        continue;
      fresh.push_back(entry.path());
    }
    for (const fs::path &candidate : fresh) {
      const noble::mvar::MapVariant captured =
          noble::mvar::ReadMapVariant(candidate);
      if (!captured.valid)
        continue;
      source = candidate;
      NOBLE_INFO("PauseMenu: using captured engine save '{}'",
                 candidate.string());
      break;
    }
  }
  if (source.empty())
    source = noble::SessionMapVariantPath();
  if (source.empty()) {
    NOBLE_WARN("PauseMenu: no variant to save (the session was not launched "
               "with -mvar)");
    return {};
  }

  std::string detail;
  const fs::path out =
      noble::mvar::SaveVariantToLibrary(source, name, library, detail);
  if (out.empty()) {
    NOBLE_WARN("PauseMenu: could not save map variant '{}': {}", name, detail);
    return {};
  }
  NOBLE_INFO("PauseMenu: map variant '{}' saved to '{}' (source '{}')", name,
             out.string(), source.string());
  FrontEndMenu::InvalidateForgeMaps();
  return out;
}
bool PauseMenu::IsOpen() { return s_open.load(); }

bool PauseMenu::IsLoading() { return s_isLoadingActive.load(); }

void PauseMenu::Shutdown() {
  std::lock_guard<std::mutex> lock(s_rtvMutex);
  if (s_cachedRtv) {
    s_cachedRtv->Release();
    s_cachedRtv = nullptr;
  }
  if (s_cachedBackBuffer) {
    s_cachedBackBuffer->Release();
    s_cachedBackBuffer = nullptr;
  }
  s_cachedSwapChain = nullptr;
  s_cachedRtvWidth = 0;
  s_cachedRtvHeight = 0;
  s_cachedRtvFormat = DXGI_FORMAT_UNKNOWN;
}

void PauseMenu::Toggle() {
  if (s_gameOver.load())
    return; // the post-game panel replaces the pause menu
  // Esc steps back out of the scoreboard actions instead of opening the pause
  // menu.
  if (s_boardPointer.load()) {
    if (!s_boardReport.exchange(false))
      s_boardPointer.store(false);
    MenuSounds::Back();
    return;
  }
  // Esc on the SETTINGS page goes back to the pause menu instead of resuming.
  // While a key rebind is waiting for a key, Esc belongs to the rebind
  // (SettingsPanel::PollRebind cancels it).
  if (s_open.load() && s_showSettings.load()) {
    if (!PauseSettings().IsCapturingKey() &&
        !PauseSettings().DiscardOnEscape()) {
      s_showSettings.store(false);
      MenuSounds::Back();
      NOBLE_INFO("PauseMenu: settings closed");
    }
    return;
  }
  const bool nowOpen = !s_open.load();
  s_openedAt = nowOpen ? GetTickCount64() : 0;
  s_focus = 0;
  s_pausePane = 0;
  s_challengeFocus = 0;
  s_resetPauseScroll = nowOpen;
  s_waitForRelease = true;
  s_confirmRestart = false;
  (void)Window::TakeWheelSteps(); // forget scroll notches accumulated before transition
  if (nowOpen)
    Window::TakeLeftClicks(); // forget clicks made in game (shots) before the
                              // menu opened
  s_open.store(nowOpen);
  if (nowOpen)
    s_challengesStale.store(true);
  SetLocalSimulationPaused(nowOpen);
  NOBLE_INFO("PauseMenu: {}", nowOpen ? "opened" : "closed");
}

void PauseMenu::Close() {
  s_openedAt = 0;
  s_focus = 0;
  s_pausePane = 0;
  s_challengeFocus = 0;
  s_resetPauseScroll = false;
  s_waitForRelease = true;
  s_confirmRestart = false;
  s_teamPickerOpen = false;
  s_showSettings.store(false);
  (void)Window::TakeWheelSteps();
  if (s_open.exchange(false)) {
    SetLocalSimulationPaused(false);
    NOBLE_INFO("PauseMenu: closed");
  }
}

void PauseMenu::ShowGameOver() {
  // May be called from the engine thread (hooks / slot 6) or the main thread (guest match_over while
  // the engine no longer draws): serialise against the frame that reads this state.
  std::lock_guard<std::recursive_mutex> lock(s_renderMutex);
  if (noble::BlamEngine::IsSavedFilmPlayback()) {
    NOBLE_INFO(
        "PauseMenu: suppressing game over during Theater saved film playback");
    return;
  }
  if (!s_gameOver.exchange(true)) {
    s_open.store(false);
    s_showSettings.store(false);
    SetLocalSimulationPaused(false);
    const ULONGLONG now = GetTickCount64();
    s_gameOverAt.store(now);
    s_autoAt = 0.0;
    s_autoCancelled = false;
    s_autoInited = false;
    s_gameOverPadButton = -1;
    s_gameOverPadArmedAt = 0.0;
    s_guestAutoReturned = false;
    s_gameOverWaitRelease = true;
    s_gameOverSettleUntil = now + 450;
    s_resetGameOverImGui.store(true);
    Window::TakeLeftClicks();
    (void)Window::TakeWheelSteps();
    Window::FlushInputEvents();
    NOBLE_INFO("PauseMenu: game over, showing post-game panel");
  }
}

void PauseMenu::CancelGameOver() {
  std::lock_guard<std::recursive_mutex> lock(s_renderMutex);
  if (s_gameOver.exchange(false)) {
    s_autoAt = 0.0;
    s_autoCancelled = true;
    s_autoInited = false;
    s_gameOverPadButton = -1;
    s_gameOverPadArmedAt = 0.0;
    s_guestAutoReturned = false;
    s_gameOverWaitRelease = false;
    s_gameOverSettleUntil = 0;
    NOBLE_INFO("PauseMenu: game over dismissed - gameplay resumed");
  }
}

void PauseMenu::RefreshGameOverAward() {
  std::lock_guard<std::recursive_mutex> lock(s_renderMutex);
  if (!s_gameOver.load())
    return;
  s_gameOverAt.store(GetTickCount64());
  s_gameOverPadButton = -1;
  s_gameOverPadArmedAt = 0.0;
  s_gameOverWaitRelease = true;
  s_resetGameOverImGui.store(true);
  NOBLE_INFO("PauseMenu: post-game award refreshed (host results replaced the live numbers)");
}

void PauseMenu::PollHostMatchState() {
  if (!NetworkTransport::Get().IsActive() || NetworkTransport::Get().IsHost())
    return;
  if (NetworkTransport::Get().TakeMatchOverRequest()) {
    // The relay thread only flags it: career/UI work happens here, after the match stopped touching
    // the session. Always consume the packet, even when the panel is already up: the host's numbers
    // replace this client's provisional settlement.
    const bool panelOpen = s_gameOver.load();
    const std::string matchOverJson = NetworkTransport::Get().TakeMatchOverJson();
    NOBLE_INFO("PauseMenu: host match_over received; {} the post-game panel",
               panelOpen ? "refreshing" : "showing");
    try {
      if (!matchOverJson.empty())
        Career::OnMatchOverJson(matchOverJson);
      else
        Career::EnsureMatchSaved("match_over");
    } catch (...) {
      NOBLE_ERROR("PauseMenu: could not settle the host's match_over");
    }
    if (panelOpen)
      RefreshGameOverAward();
    else
      ShowGameOver();
  }
  if (s_guestShouldReturnLobby.exchange(false)) {
    NOBLE_INFO("PauseMenu: host returned to lobby, returning guest to "
               "pre-game lobby");
    Career::EnsureMatchSaved("host_returned_to_lobby");
    SetHostBackInLobby(1);
    if (!s_gameOver.load()) {
      s_guestReturnedToLobby.store(true);
      RelaunchAndClose(Window::ActiveHwnd(), LobbyReturnArgs(false),
                       "host returned to pre-game lobby");
      return;
    }
  }
  if (s_guestShouldDisband.exchange(false)) {
    NOBLE_INFO("PauseMenu: host disbanded lobby / quit game, returning guest "
               "to main menu");
    Career::EnsureMatchSaved("host_disbanded_lobby");
    SetHostBackInLobby(2);
    if (!s_gameOver.load()) {
      s_guestReturnedToLobby.store(true);
      RelaunchAndClose(Window::ActiveHwnd(), L"-main-menu",
                       "host disbanded lobby");
    }
  }
}

bool PauseMenu::IsGameOver() { return s_gameOver.load(); }

namespace {
std::mutex s_boardMutex;
std::vector<PauseMenu::ScoreboardEntry> s_boardRows;
std::string s_boardMap;
std::string s_boardMode;
bool s_boardLogged = false;

std::atomic<int> s_localTeam{
    1}; // from the lobby launch; the engine's own sides start at red
std::atomic<bool> s_localTeamFromLobby{
    false}; // true once -lobby-teams told us the player's side
std::atomic<int> s_matchTeamCount{
    0}; // available teams in this match (2..8)

static std::string GetCurrentMatchMode() {
  if (!s_boardMode.empty())
    return s_boardMode;
  std::wstring wmode = ArgValue(Relaunch::CurrentGameArgs(), L"-mode");
  if (wmode.empty())
    wmode = ArgValue(GetCommandLineW(), L"-mode");
  if (wmode.empty())
    return {};
  char mb[64] = {};
  WideCharToMultiByte(CP_UTF8, 0, wmode.c_str(), -1, mb, sizeof(mb) - 1, nullptr, nullptr);
  return mb;
}

static void WritePlayerDatumTeam(int teamZeroBased) {
  __try {
    const uintptr_t reachBase =
        reinterpret_cast<uintptr_t>(GetModuleHandleW(L"haloreach.dll"));
    if (reachBase) {
      const uint32_t tlsIndex =
          *reinterpret_cast<const uint32_t *>(reachBase + 0xC17B18);
      const auto tlsArray = static_cast<uintptr_t>(__readgsqword(0x58));
      const uintptr_t block =
          *reinterpret_cast<const uintptr_t *>(tlsArray + tlsIndex * 8);
      const uintptr_t arr =
          block ? *reinterpret_cast<const uintptr_t *>(block + 0x18) : 0;
      if (arr) {
        const uintptr_t data = *reinterpret_cast<const uintptr_t *>(arr + 0x50);
        if (data) {
          *reinterpret_cast<int8_t *>(data + 0xAD) =
              static_cast<int8_t>(teamZeroBased);
        }
      }
    }
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }
}

int LocalTeamFromBoard() {
  std::lock_guard<std::mutex> lock(s_boardMutex);
  for (const auto &row : s_boardRows)
    if (row.local)
      return row.team;
  return s_localTeam.load(); // a solo launch has no roster yet: use the team
                             // the lobby assigned
}

} // namespace

void PauseMenu::SetMatchTeamCount(int count) {
  s_matchTeamCount.store(std::clamp(count, 2, kTeamCount));
}

int PauseMenu::GetMatchTeamCount() {
  const int count = s_matchTeamCount.load();
  if (count >= 2 && count <= kTeamCount) {
    return count;
  }
  const std::string mode = GetCurrentMatchMode();
  if (mode == "forge") {
    return kTeamCount; // 8 teams in Forge for spawn and testing
  }
  int maxTeam = 2;
  {
    std::lock_guard<std::mutex> lock(s_boardMutex);
    for (const auto &row : s_boardRows) {
      if (row.team > maxTeam)
        maxTeam = row.team;
    }
  }
  return std::clamp(maxTeam, 2, kTeamCount);
}

void PauseMenu::ChangeLocalTeam(int toTeam) {
  const int from = LocalTeamFromBoard();
  const int maxTeams = GetMatchTeamCount();
  int to = toTeam;
  if (to < 1 || to > maxTeams) {
    to = (from % maxTeams) + 1;
  }
  if (from == to && s_localTeamFromLobby.load())
    return;

  // 1. HaloScript team override (0-based)
  if (BlamEngine *engine = BlamEngine::Active()) {
    engine->ExecuteCommand("player_override_team 0 " + std::to_string(to - 1));
  }

  // 2. Direct game player datum update (player 0 datum at arr + 0x50, team at +0xAD)
  WritePlayerDatumTeam(to - 1);

  // 3. Update local states
  s_localTeam.store(to);
  s_localTeamFromLobby.store(true);

  // 4. Update local row in scoreboard
  {
    std::lock_guard<std::mutex> lock(s_boardMutex);
    for (auto &row : s_boardRows) {
      if (row.local)
        row.team = to;
    }
  }

  // 5. Update local voice chat channel
  VoiceChat::SetLocalTeam(to);

  // 6. Request biped suicide/respawn in gameplay modes (outside Forge)
  const std::string mode = GetCurrentMatchMode();
  if (mode != "forge") {
    DevCheats::RequestLocalSuicide();
  }

  MenuSounds::Change();
  NOBLE_INFO("PauseMenu: CHANGE TEAM requested ({} -> {})",
             TeamStyleFor(from).name, TeamStyleFor(to).name);
}

static std::string s_boardSession;

void PauseMenu::SetLocalTeam(int team) {
  s_localTeam.store(std::clamp(team, 1, 8));
  s_localTeamFromLobby.store(true);
}

void PauseMenu::SetScoreboardSession(const std::string &sessionId) {
  std::lock_guard<std::mutex> lock(s_boardMutex);
  s_boardSession = sessionId;
}

void PauseMenu::SetScoreboardRoster(std::vector<ScoreboardEntry> entries,
                                    const std::string &mapName,
                                    const std::string &modeName) {
  // The session roster can lag the lobby's team choice (the push may not have
  // landed yet, or a guest's copy is stale), and the panel then shows a side
  // the player is not on. The launch's own team wins for our row.
  if (s_localTeamFromLobby.load()) {
    for (auto &row : entries) {
      if (!row.local)
        continue;
      row.team = s_localTeam.load();
      break;
    }
  }
  std::lock_guard<std::mutex> lock(s_boardMutex);
  s_boardRows = std::move(entries);
  s_boardMap = mapName;
  s_boardMode = modeName;
  if (!s_boardLogged) {
    s_boardLogged = true;
    NOBLE_INFO("PauseMenu: scoreboard roster set ({} player(s), {} on {})",
               s_boardRows.size(), s_boardMode, s_boardMap);
  }
}

bool PauseMenu::WantsPointer() {
  return s_open.load() || s_gameOver.load() || s_boardPointer.load();
}

void PauseMenu::EnterScoreboardPointer() {
  if (s_open.load() || s_gameOver.load() || s_boardPointer.exchange(true))
    return;
  Window::TakeLeftClicks();
  NOBLE_INFO("PauseMenu: scoreboard actions opened");
}

void PauseMenu::SetLoadingMap(const std::string &scenario) {
  std::lock_guard<std::mutex> lock(s_mapMutex);
  s_loadingMap = scenario;
  s_loadingMapName = Upper(DisplayName(scenario));
  s_displayMapName.clear();
  s_loadingImage.clear();
  s_loadingThumb.clear();
  s_loadingDescription.clear();
  // Read the career file and build today's challenge list once off the render
  // thread so the first time the pause menu opens it does not stall.
  static std::once_flag s_warmChallengesOnce;
  std::call_once(s_warmChallengesOnce, [] {
    std::thread([] { (void)Career::Challenges(); }).detach();
  });
}

void PauseMenu::SetLoadingMapInfo(const std::string &title,
                                  const std::filesystem::path &image,
                                  const std::filesystem::path &thumbnail) {
  std::lock_guard<std::mutex> lock(s_mapMutex);
  if (!title.empty()) {
    s_displayMapName = title;
    s_loadingMapName = Upper(title);
  }
  s_loadingImage = image;
  s_loadingThumb = thumbnail;
}

void PauseMenu::SetLoadingMapDescription(const std::string &description) {
  // Workshop text is written for a Steam page: drop [tags], collapse
  // whitespace, cap the length.
  std::string out;
  bool pendingSpace = false;
  constexpr size_t kMax = 260;
  for (size_t i = 0; i < description.size() && out.size() < kMax; ++i) {
    const char c = description[i];
    if (c == '[') {
      const size_t close = description.find(']', i + 1);
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
  if (out.size() >= kMax) {
    const size_t cut = out.find_last_of(' ');
    if (cut != std::string::npos && cut > kMax / 2)
      out.resize(cut);
    out += "...";
  }
  std::lock_guard<std::mutex> lock(s_mapMutex);
  s_loadingDescription = out;
}

void PauseMenu::SetLoadingSession(const std::string &mode,
                                  std::vector<std::string> players,
                                  int expectedPlayers, bool host,
                                  bool joiner) {
  std::lock_guard<std::mutex> lock(s_mapMutex);
  s_loadingMode = mode;
  s_loadingPlayers = std::move(players);
  s_loadingExpectedPlayers = std::max(expectedPlayers, 0);
  s_loadingHost = host;
  s_loadingJoiner = joiner;
}

void PauseMenu::DrawMenu(HWND hwnd) {
  using namespace NobleUi;
  ImGuiIO &io = ImGui::GetIO();
  ImDrawList *bg = ImGui::GetBackgroundDrawList();
  const float W = io.DisplaySize.x;
  const float H = io.DisplaySize.y;

  // Atmospheric Halo: Reach gradient dim scrim over live gameplay.
  bg->AddRectFilledMultiColor(ImVec2(0.0f, 0.0f), ImVec2(W, H),
                              IM_COL32(4, 9, 16, 198), IM_COL32(4, 9, 16, 198),
                              IM_COL32(7, 14, 24, 218), IM_COL32(7, 14, 24, 218));

  if (s_showSettings.load()) {
    DrawSettingsPage(hwnd);
    return;
  }

  if (s_openedAt == 0)
    s_openedAt = GetTickCount64();
  const bool timeReady =
      (GetTickCount64() - s_openedAt) > 220; // ignore the opening frames
  if (s_waitForRelease && timeReady) {
    const bool anyHeld = ImGui::IsMouseDown(0) ||
                         (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 ||
                         ImGui::IsKeyDown(ImGuiKey_Enter) ||
                         ImGui::IsKeyDown(ImGuiKey_KeypadEnter) ||
                         ImGui::IsKeyDown(ImGuiKey_GamepadFaceDown);
    if (!anyHeld)
      s_waitForRelease = false;
  }
  const bool modalOpen =
      s_saveVariantOpen || ImGui::IsPopupOpen("##save_map_variant");
  const bool interactive = timeReady && !s_waitForRelease;
  const bool navInteractive = interactive && !modalOpen && !io.WantTextInput;

  const size_t peers = NetworkTransport::Get().GetGamePeerCount();
  const bool solo = peers == 0;
  const bool isGuest =
      NetworkTransport::Get().IsActive() && !NetworkTransport::Get().IsHost();
  const std::wstring restartArgs = Relaunch::CurrentGameArgs();

  const float s = Scale();
  const ImVec2 size(std::min(W - 36.0f * s, 1280.0f * s),
                    std::min(H - 36.0f * s, 748.0f * s));
  const ImVec2 winPos((W - size.x) * 0.5f, (H - size.y) * 0.5f);

  // Outer Halo: Reach tactical container background + L-corner reticle brackets.
  bg->AddRectFilledMultiColor(winPos, ImVec2(winPos.x + size.x, winPos.y + size.y),
                              IM_COL32(10, 17, 27, 240), IM_COL32(10, 17, 27, 240),
                              IM_COL32(7, 12, 20, 245), IM_COL32(7, 12, 20, 245));
  DrawTacticalFrame(bg, winPos, ImVec2(winPos.x + size.x, winPos.y + size.y), s,
                    false, true, false, false);
  bg->AddRectFilled(winPos, ImVec2(winPos.x + size.x, winPos.y + 3.0f * s),
                    kAccent);

  ImGui::SetNextWindowPos(winPos, ImGuiCond_Always);
  ImGui::SetNextWindowSize(size, ImGuiCond_Always);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      ImVec2(24.0f * s, 20.0f * s));
  ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
  ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
  if (!ImGui::Begin("##NoblePauseMenu", nullptr, kPanelFlags)) {
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    return;
  }

  ImDrawList *dl = ImGui::GetWindowDrawList();
  ImFont *titleFont = NobleStyle::TitleFont();
  ImFont *smallFont = NobleStyle::SmallFont();

  std::string currentMode = s_boardMode;
  if (currentMode.empty()) {
    std::wstring wmode = ArgValue(Relaunch::CurrentGameArgs(), L"-mode");
    if (wmode.empty())
      wmode = ArgValue(GetCommandLineW(), L"-mode");
    char mb[64] = {};
    WideCharToMultiByte(CP_UTF8, 0, wmode.c_str(), -1, mb, sizeof(mb) - 1,
                        nullptr, nullptr);
    currentMode = mb;
  }
  std::string headerMap;
  {
    std::lock_guard<std::mutex> lock(s_mapMutex);
    headerMap = s_loadingMapName;
  }
  if (headerMap.empty()) {
    std::lock_guard<std::mutex> lock(s_boardMutex);
    if (!s_boardMap.empty())
      headerMap = Upper(DisplayName(s_boardMap));
  }

  // ---- Header Bar ----
  const float padX = 24.0f * s;
  const float padY = 20.0f * s;
  const float innerW = size.x - padX * 2.0f;
  const float headerTop = winPos.y + padY;
  const float headerH = 58.0f * s;

  if (solo)
    SetLocalSimulationPaused(true);

  const char *mainTitle = solo ? "PAUSED" : "MATCH MENU";
  Text(dl, titleFont, 32.0f * s, ImVec2(winPos.x + padX, headerTop), kText,
       mainTitle);
  char subBuf[128];
  if (solo) {
    std::snprintf(subBuf, sizeof(subBuf), "Local simulation paused.");
  } else {
    std::snprintf(subBuf, sizeof(subBuf),
                  "Match stays live for %zu other player%s.", peers,
                  peers == 1 ? "" : "s");
  }
  Text(dl, smallFont, 14.5f * s,
       ImVec2(winPos.x + padX + 2.0f * s, headerTop + 34.0f * s), kTextDim,
       subBuf);

  // Right-side map + mode + session status pill
  {
    const std::string modeTag =
        currentMode.empty() ? std::string("HALO: REACH")
                            : ("HALO: REACH  \xC2\xB7  " +
                               LoadingModeLabel(currentMode));
    const std::string rightTop =
        headerMap.empty() ? modeTag : (modeTag + "  \xC2\xB7  " + headerMap);
    const float rtw = TextWidth(titleFont, 15.0f * s, rightTop.c_str());
    const float pillPadX = 12.0f * s;
    const float pillW = std::min(innerW * 0.56f, rtw + pillPadX * 2.0f);
    const ImVec2 hp1(winPos.x + padX + innerW, headerTop + 44.0f * s);
    const ImVec2 hp0(hp1.x - pillW, headerTop + 4.0f * s);
    dl->AddRectFilled(hp0, hp1, IM_COL32(16, 28, 44, 215), 3.0f * s);
    dl->AddRect(hp0, hp1, IM_COL32(143, 216, 255, 75), 3.0f * s);
    Text(dl, titleFont, 15.0f * s,
         ImVec2(hp0.x + pillPadX, hp0.y + 6.0f * s), kAccent, rightTop.c_str(),
         pillW - pillPadX * 1.5f);

    const char *stateLabel = solo ? "OFFLINE / SOLO" : "ONLINE SESSION";
    const ImU32 stateCol =
        solo ? IM_COL32(143, 216, 255, 220) : IM_COL32(118, 220, 152, 240);
    const float slw = TextWidth(smallFont, 12.5f * s, stateLabel);
    Text(dl, smallFont, 12.5f * s,
         ImVec2(hp1.x - slw - 4.0f * s, hp0.y + 24.0f * s), stateCol,
         stateLabel);
  }

  const float ruleY = headerTop + headerH;
  dl->AddLine(ImVec2(winPos.x + padX, ruleY),
              ImVec2(winPos.x + padX + innerW, ruleY),
              IM_COL32(143, 216, 255, 70), 1.0f);

  const float bodyTop = ruleY + 14.0f * s;
  const float footerH = 38.0f * s;
  const float bodyH = std::max(260.0f * s,
                               (winPos.y + size.y - padY - footerH) - bodyTop);
  const float listW = std::clamp(innerW * 0.34f, 290.0f * s, 410.0f * s);
  const float gap = 20.0f * s;
  const float sideW = innerW - listW - gap;

  // ---- Build left action deck entries ----
  struct Entry {
    const char *label;
    const char *subtitle;
    const char *description;
    const char *badge;
    bool danger;
    bool enabled;
  };
  const bool canRestart = !restartArgs.empty() && !isGuest;
  const int curTeam = LocalTeamFromBoard();
  const TeamStyle &curStyle = TeamStyleFor(curTeam);
  const char *teamBadge = curStyle.name;
  std::vector<Entry> entries = {
      {"RESUME", nullptr, "Return to gameplay immediately.", nullptr, false,
       true},
      {s_confirmRestart ? "CONFIRM RESTART?" : "RESTART MATCH", nullptr,
       canRestart
           ? (s_confirmRestart
                  ? "Select again to confirm restarting from the beginning."
                  : "Reload this match from the beginning.")
           : (isGuest ? "Only the session host can restart the match."
                      : "Unavailable: match launch arguments were not passed."),
       s_confirmRestart ? "CONFIRM" : nullptr, s_confirmRestart, canRestart},
      {"SETTINGS", nullptr,
       "Adjust video, audio, HUD, and gameplay options live.", nullptr, false,
       true},
      {"CONTROLS", nullptr,
       "Configure key bindings, mouse sensitivity, and controller layout.",
       nullptr, false, true},
  };
  int saveMapVariantEntry = -1;
  if (_stricmp(currentMode.c_str(), "forge") == 0) {
    saveMapVariantEntry = static_cast<int>(entries.size());
    entries.push_back(
        {"SAVE MAP VARIANT", nullptr,
         "Save your Forge map to data\\map_variants for reuse in Custom Games "
         "and Forge.",
         "FORGE", false, true});
  }
  int teamEntry = -1;
  const bool isMultiOrForge = (currentMode == "multiplayer" ||
                               currentMode == "forge" || currentMode.empty());
  if (BlamEngine::GameTeamsEnabled() || isMultiOrForge) {
    teamEntry = static_cast<int>(entries.size());
    entries.push_back({"CHANGE TEAM", nullptr,
                       "Switch your active team assignment.",
                       teamBadge, false, true});
  }
  const int leaveEntry = static_cast<int>(entries.size());
  if (noble::BlamEngine::IsSavedFilmPlayback()) {
    entries.push_back({"LEAVE THEATER", nullptr,
                       "Stop Theater playback and return to the front-end.",
                       nullptr, false, true});
  } else {
    entries.push_back(
        {"LEAVE GAME", nullptr,
         isGuest ? "Disconnect from this match and return to the menu."
                 : "End this match and return to the front-end menu.",
         nullptr, false, true});
  }
  const int quitEntry = static_cast<int>(entries.size());
  entries.push_back(
      {"QUIT", nullptr, "Close Project Noble and exit to desktop.", nullptr, true,
       true});
  const int entryCount = static_cast<int>(entries.size());

  // ---- Refresh cached challenges cleanly (built once on open + every 2s) ----
  static std::vector<Career::Challenge> s_cachedChallenges;
  static ULONGLONG s_cachedChallengesAt = 0;
  {
    const ULONGLONG tick = GetTickCount64();
    if (s_challengesStale.exchange(false) ||
        tick - s_cachedChallengesAt > 2000) {
      s_cachedChallenges = Career::Challenges();
      s_cachedChallengesAt = tick;
    }
  }
  const std::vector<Career::Challenge> &challenges = s_cachedChallenges;
  const int chCount = static_cast<int>(challenges.size());
  if (chCount > 0)
    s_challengeFocus = std::clamp(s_challengeFocus, 0, chCount - 1);
  else if (s_pausePane == 1)
    s_pausePane = 0;

  // ---- Unified Keyboard + Controller Navigation across Left (0) & Right (1) ----
  static ULONGLONG s_lastMove = 0;
  const ULONGLONG now = GetTickCount64();
  bool challengeNavMoved = false;
  if (navInteractive && !s_teamPickerOpen) {
    const bool goLeft =
        ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false) || UiPad::Left();
    const bool goRight =
        ImGui::IsKeyPressed(ImGuiKey_RightArrow, false) || UiPad::Right();
    const bool goUp =
        ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) || UiPad::Up();
    const bool goDown =
        ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) || UiPad::Down();

    if (s_pausePane == 0 && goRight && chCount > 0) {
      s_pausePane = 1;
      challengeNavMoved = true;
      s_lastMove = now;
      MenuSounds::Scroll();
    } else if (s_pausePane == 1 && goLeft) {
      s_pausePane = 0;
      s_lastMove = now;
      MenuSounds::Scroll();
    } else if ((goUp || goDown) && now - s_lastMove > 110) {
      s_lastMove = now;
      if (s_pausePane == 0) {
        s_focus = (s_focus + (goDown ? 1 : entryCount - 1)) % entryCount;
        if (s_focus != 1)
          s_confirmRestart = false;
      } else if (chCount > 0) {
        s_challengeFocus =
            (s_challengeFocus + (goDown ? 1 : chCount - 1)) % chCount;
        challengeNavMoved = true;
      }
      MenuSounds::Scroll();
    }
  }
  s_focus = std::clamp(s_focus, 0, entryCount - 1);
  if (s_focus != 1)
    s_confirmRestart = false;

  // ---- Left Column: Tactical Command Deck + Info Card ----
  const float descBoxH = 62.0f * s;
  const float deckAvailH = bodyH - descBoxH - 10.0f * s;
  const float rowGap = 5.0f * s;
  const float rowH = std::clamp(
      (deckAvailH - rowGap * static_cast<float>(std::max(1, entryCount - 1))) /
          static_cast<float>(entryCount),
      34.0f * s, 48.0f * s);
  const float leftX = winPos.x + padX;
  int activated = -1;

  for (int i = 0; i < entryCount; ++i) {
    ImGui::PushID(i);
    const float ry = bodyTop + static_cast<float>(i) * (rowH + rowGap);
    const ImVec2 t0(leftX, ry);
    const ImVec2 t1(leftX + listW, ry + rowH);
    const bool enabled = entries[i].enabled;
    const bool focused = (s_pausePane == 0 && s_focus == i);

    ImGui::SetCursorScreenPos(t0);
    if (DrawTile(dl, "##pause_tile", t0, t1, entries[i].label, nullptr, focused,
                 s_confirmRestart && i == 1, enabled, entries[i].badge) &&
        enabled && navInteractive) {
      s_pausePane = 0;
      s_focus = i;
      activated = i;
    }
    if (ImGui::IsItemHovered() && navInteractive &&
        NobleUi::IsMouseMoved(2.5f)) {
      s_pausePane = 0;
      s_focus = i;
      if (s_focus != 1)
        s_confirmRestart = false;
    }
    // Multi-team color strip accent on CHANGE TEAM, crimson edge on QUIT.
    if (i == teamEntry) {
      const ImU32 teamCol = TeamStyleFor(curTeam).strip;
      dl->AddRectFilled(ImVec2(t1.x - 4.0f * s, t0.y + 6.0f * s),
                        ImVec2(t1.x - 1.0f * s, t1.y - 6.0f * s), teamCol);
    } else if (entries[i].danger && focused) {
      dl->AddRect(t0, t1, IM_COL32(255, 130, 82, 210), 3.0f * s, 0, 1.5f);
    }
    ImGui::PopID();
  }

  if (navInteractive && !s_teamPickerOpen && s_pausePane == 0 &&
      (UiPad::AcceptPressed() ||
       ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
       ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false))) {
    UiPad::ConsumeAccept();
    if (entries[s_focus].enabled)
      activated = s_focus;
  }

  // Focused action description card anchored neatly at bottom-left.
  {
    const ImVec2 d0(leftX, bodyTop + bodyH - descBoxH);
    const ImVec2 d1(leftX + listW, bodyTop + bodyH);
    dl->AddRectFilled(d0, d1, IM_COL32(12, 20, 32, 220), 3.0f * s);
    dl->AddRectFilled(d0, ImVec2(d0.x + 3.0f * s, d1.y),
                      s_confirmRestart ? kAmber : kAccent);
    dl->AddRect(d0, d1, IM_COL32(143, 216, 255, 48), 3.0f * s);
    const char *descText = entries[s_focus].description;
    if (s_focus == saveMapVariantEntry && !s_saveMapVariantResult.empty())
      descText = s_saveMapVariantResult.c_str();
    Text(dl, smallFont, 13.5f * s,
         ImVec2(d0.x + 12.0f * s, d0.y + 10.0f * s), kTextDim, descText,
         listW - 24.0f * s);
  }

  // ---- Right Column: Service-Record Style Challenges Panel ----
  const float rightX = leftX + listW + gap;
  const ImVec2 chBox0(rightX, bodyTop);
  const ImVec2 chBox1(rightX + sideW, bodyTop + bodyH);
  dl->AddRectFilled(chBox0, chBox1, IM_COL32(11, 18, 29, 226), 3.0f * s);
  DrawTacticalFrame(dl, chBox0, chBox1, s, s_pausePane == 1, false, false,
                    false);

  int dailyDone = 0, dailyCount = 0, weeklyDone = 0, weeklyCount = 0;
  for (const Career::Challenge &c : challenges) {
    if (c.weekly) {
      ++weeklyCount;
      if (c.done)
        ++weeklyDone;
    } else {
      ++dailyCount;
      if (c.done)
        ++dailyDone;
    }
  }

  // Fixed challenges header strip (never scrolls out of view).
  const float chHdrH = 52.0f * s;
  {
    dl->AddRectFilled(chBox0, ImVec2(chBox1.x, chBox0.y + chHdrH),
                      IM_COL32(15, 25, 40, 238), 3.0f * s,
                      ImDrawFlags_RoundCornersTop);
    Text(dl, titleFont, 16.5f * s,
         ImVec2(chBox0.x + 14.0f * s, chBox0.y + 8.0f * s), kAccent,
         "CHALLENGES");

    char countsBuf[96];
    std::snprintf(countsBuf, sizeof(countsBuf), "DAILY %d/%d   \xC2\xB7   WEEKLY %d/%d",
                  dailyDone, dailyCount, weeklyDone, weeklyCount);
    Text(dl, smallFont, 13.0f * s,
         ImVec2(chBox0.x + 14.0f * s, chBox0.y + 29.0f * s), kTextDim,
         countsBuf);

    const std::string resetDaily = "Daily: " + Career::DailyResetText();
    const std::string resetWeekly = "Weekly: " + Career::WeeklyResetText();
    const float rdw = TextWidth(smallFont, 12.5f * s, resetDaily.c_str());
    const float rww = TextWidth(smallFont, 12.5f * s, resetWeekly.c_str());
    Text(dl, smallFont, 12.5f * s,
         ImVec2(chBox1.x - 14.0f * s - rdw, chBox0.y + 9.0f * s), kTextDim,
         resetDaily.c_str());
    Text(dl, smallFont, 12.5f * s,
         ImVec2(chBox1.x - 14.0f * s - rww, chBox0.y + 28.0f * s), kTextDim,
         resetWeekly.c_str());
    dl->AddLine(ImVec2(chBox0.x, chBox0.y + chHdrH),
                ImVec2(chBox1.x, chBox0.y + chHdrH),
                IM_COL32(143, 216, 255, 58), 1.0f);
  }

  // Scrollable Service-Record challenge cards body (starts unscrolled at top).
  const float chPad = 12.0f * s;
  const float chScrollTop = chBox0.y + chHdrH + 6.0f * s;
  const float chScrollH =
      std::max(100.0f * s, (chBox1.y - 8.0f * s) - chScrollTop);
  ImGui::SetCursorScreenPos(ImVec2(chBox0.x + chPad, chScrollTop));
  if (s_resetPauseScroll) {
    ImGui::SetNextWindowScroll(ImVec2(0.0f, 0.0f));
    s_resetPauseScroll = false;
  }
  ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 0));
  ImGui::BeginChild("##pausechallenges",
                    ImVec2(sideW - chPad * 2.0f, chScrollH),
                    ImGuiChildFlags_None, ImGuiWindowFlags_None);
  if (navInteractive)
    UiPad::ScrollWindow(false, true);

  const float cardW =
      std::max(160.0f * s, ImGui::GetContentRegionAvail().x - 4.0f * s);
  int cardIdx = 0;
  for (int weekly = 0; weekly <= 1; ++weekly) {
    const int secDone = weekly ? weeklyDone : dailyDone;
    const int secTotal = weekly ? weeklyCount : dailyCount;
    if (secTotal == 0)
      continue;
    char head[64];
    std::snprintf(head, sizeof(head), "%s  %d / %d",
                  weekly ? "WEEKLY CHALLENGES" : "DAILY CHALLENGES", secDone,
                  secTotal);
    SectionHeader(head, cardW);
    for (const Career::Challenge &c : challenges) {
      if (static_cast<int>(c.weekly) != weekly)
        continue;
      ImGui::PushID(cardIdx);
      const bool focused = (s_pausePane == 1 && cardIdx == s_challengeFocus);
      bool hovered = false, clicked = false;
      ImGui::BeginGroup();
      DrawChallengeCard(c, cardW, focused, hovered, clicked, true);
      ImGui::EndGroup();
      if (clicked ||
          (hovered && navInteractive &&
           NobleUi::IsMouseMoved(2.5f))) {
        s_challengeFocus = cardIdx;
        if (clicked) {
          s_pausePane = 1;
          MenuSounds::Click();
        }
      }
      if (focused && challengeNavMoved)
        ImGui::SetScrollHereY(0.45f);
      ImGui::Dummy(ImVec2(0.0f, 6.0f * s));
      ImGui::PopID();
      ++cardIdx;
    }
  }
  if (cardIdx == 0) {
    ImGui::Dummy(ImVec2(0.0f, 12.0f * s));
    Paragraph("No active challenges yet - complete a match to track daily "
              "and weekly challenge progress.");
  }
  ImGui::EndChild();
  ImGui::PopStyleColor();

  // ---- Act on selected action (never early-return before End()) ----
  if (activated == 0) {
    Close();
  } else if (activated == 1) {
    if (canRestart) {
      MenuSounds::Click();
      if (!s_confirmRestart) {
        s_confirmRestart = true;
      } else {
        Close();
        RelaunchAndClose(hwnd, restartArgs, "restart match");
      }
    }
  } else if (activated == 2) {
    MenuSounds::Click();
    Window::TakeLeftClicks();
    (void)Window::TakeWheelSteps();
    s_showSettings.store(true);
    NOBLE_INFO("PauseMenu: settings opened from the pause menu");
  } else if (activated == 3) {
    MenuSounds::Click();
    Window::TakeLeftClicks();
    (void)Window::TakeWheelSteps();
    PauseSettings().SetTab(InputPrompts::Current() ==
                                   InputPrompts::Device::Gamepad
                               ? 5
                               : 4); // Controller, or Keyboard & Mouse
    s_showSettings.store(true);
    NOBLE_INFO("PauseMenu: controls opened from the pause menu");
  } else if (saveMapVariantEntry >= 0 && activated == saveMapVariantEntry) {
    MenuSounds::Click();
    s_saveMapVariantResult.clear();
    {
      std::string suggested =
          noble::mvar::VariantTitle(noble::SessionMapVariantPath());
      if (suggested.empty())
        suggested = "My Forge Map";
      std::snprintf(s_saveVariantName, sizeof(s_saveVariantName), "%s",
                    suggested.c_str());
    }
    s_saveVariantOpen = true;
    s_saveVariantJustOpened = true;
    s_saveVariantNeedsFocus = true;
    ImGui::OpenPopup("##save_map_variant");
  } else if (teamEntry >= 0 && activated == teamEntry) {
    s_teamPickerOpen = true;
    s_teamPickerRow = std::clamp(curTeam - 1, 0, GetMatchTeamCount() - 1);
    s_teamPickerOpenedAt = GetTickCount64();
    MenuSounds::Click();
  } else if (activated == leaveEntry) {
    MenuSounds::Click();
    Close();
    const bool isHost =
        NetworkTransport::Get().IsHost() ||
        (wcsstr(GetCommandLineW(), L"-host") != nullptr) ||
        (Relaunch::CurrentGameArgs().find(L" -host") != std::wstring::npos);
    std::wstring wSession =
        ArgValue(Relaunch::CurrentGameArgs(), L"-session-id");
    if (wSession.empty())
      wSession = ArgValue(GetCommandLineW(), L"-session-id");
    if (isHost &&
        (!wSession.empty() || MasterServerClient::Get().IsAdvertising())) {
      char sessUtf8[128] = {};
      if (!wSession.empty()) {
        WideCharToMultiByte(CP_UTF8, 0, wSession.c_str(), -1, sessUtf8,
                            sizeof(sessUtf8) - 1, nullptr, nullptr);
      } else {
        std::string curSid = MasterServerClient::Get().GetCurrentSessionId();
        std::snprintf(sessUtf8, sizeof(sessUtf8), "%s", curSid.c_str());
      }
      NetworkTransport::Get().SendLobbyControlJson(
          "{\"type\":\"host_return_to_lobby\"}");
      MasterServerClient::Get().ReturnLobbySync(sessUtf8);
      std::wstring returnArgs = LobbyReturnArgs(true);
      RelaunchAndClose(hwnd, returnArgs, "host leave match to pre-game lobby");
    } else {
      RelaunchAndClose(hwnd, L"", "leave game");
    }
  } else if (activated == quitEntry) {
    Close();
    QuitFromMatch(hwnd, "quit requested");
  } else if (navInteractive && s_teamPickerOpen &&
             (UiPad::BackPressed() ||
              ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false) ||
              ImGui::IsKeyPressed(ImGuiKey_Escape, false))) {
    UiPad::ConsumeBack();
    s_teamPickerOpen = false;
    MenuSounds::Back();
  } else if (navInteractive && !s_teamPickerOpen &&
             (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false) ||
              (s_pausePane == 1 && UiPad::BackPressed()))) {
    if (s_pausePane == 1) {
      UiPad::ConsumeBack();
      s_pausePane = 0;
      MenuSounds::Back();
    } else {
      // B resumes. Esc is not read here: the input poll already toggles on Esc,
      // and closing here as well reopened the menu when both saw the same press.
      Close();
    }
  }

  // ---- Name this map variant (SAVE MAP VARIANT, Forge only) ----
  ImGui::SetNextWindowSize(ImVec2(520.0f * s, 0.0f), ImGuiCond_Appearing);
  if (ImGui::BeginPopupModal(
          "##save_map_variant", nullptr,
          ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
    ImGui::TextUnformatted("MAP VARIANT NAME");
    ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
    ImGui::TextWrapped("Shown under its base map in the maps menu, for Custom "
                       "Game and Forge.");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    if (s_saveVariantNeedsFocus) {
      ImGui::SetKeyboardFocusHere(0);
      s_saveVariantNeedsFocus = false;
    }
    ImGui::InputText("##variant_name", s_saveVariantName,
                     sizeof(s_saveVariantName));
    ImGui::Spacing();
    const bool typed = s_saveVariantName[0] != 0;
    const std::filesystem::path library =
        ReachHost::Get().GetDataDir() / "map_variants";
    const std::filesystem::path sessionPath = noble::SessionMapVariantPath();
    std::error_code overEc;
    const bool inUserLibrary =
        !sessionPath.empty() && std::filesystem::exists(sessionPath, overEc) &&
        std::filesystem::equivalent(sessionPath.parent_path(), library, overEc);
    std::string currentName =
        inUserLibrary ? noble::mvar::VariantTitle(sessionPath) : std::string{};
    if (inUserLibrary && currentName.empty()) {
      currentName = sessionPath.stem().string();
    }
    const bool canOverwrite = inUserLibrary && !currentName.empty();
    const auto finishSave = [](const std::filesystem::path &saved) {
      s_saveMapVariantResult =
          saved.empty() ? "Could not save the map variant (see the log)."
                        : "Saved: " + saved.filename().string();
      s_saveVariantOpen = false;
      ImGui::CloseCurrentPopup();
    };
    if (s_saveVariantJustOpened) {
      s_saveVariantJustOpened = false;
    } else if (typed && (ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                         ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false))) {
      MenuSounds::Click();
      finishSave(
          SaveForgeMapVariant(UniqueVariantName(library, s_saveVariantName)));
    }
    if (!canOverwrite) {
      ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
      ImGui::TextWrapped(
          "This map was not opened from a saved variant; use SAVE AS COPY.");
      ImGui::PopStyleColor();
    }
    ImGui::BeginDisabled(!canOverwrite);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.42f, 0.62f, 1.0f));
    if (ImGui::Button("OVERWRITE", ImVec2(150.0f * s, 34.0f * s)) &&
        canOverwrite) {
      MenuSounds::Click();
      finishSave(SaveForgeMapVariant(sessionPath.stem().string()));
    }
    ImGui::PopStyleColor();
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!typed);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.42f, 0.62f, 1.0f));
    if (ImGui::Button("SAVE AS COPY", ImVec2(170.0f * s, 34.0f * s)) && typed) {
      MenuSounds::Click();
      finishSave(
          SaveForgeMapVariant(UniqueVariantName(library, s_saveVariantName)));
    }
    ImGui::PopStyleColor();
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("CANCEL", ImVec2(120.0f * s, 34.0f * s)) ||
        ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false) ||
        ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
      MenuSounds::Back();
      s_saveVariantOpen = false;
      ImGui::CloseCurrentPopup();
    }
    if (!s_saveMapVariantResult.empty()) {
      ImGui::Spacing();
      ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::TextMuted());
      ImGui::TextWrapped("%s", s_saveMapVariantResult.c_str());
      ImGui::PopStyleColor();
    }
    ImGui::EndPopup();
  }

  // ---- CHANGE TEAM Tactical Selection Modal ----
  if (s_teamPickerOpen) {
    const int numTeams = GetMatchTeamCount();
    s_teamPickerRow = std::clamp(s_teamPickerRow, 0, numTeams - 1);

    // Dim backdrop
    dl->AddRectFilled(winPos, ImVec2(winPos.x + size.x, winPos.y + size.y),
                      IM_COL32(0, 0, 0, 190));

    // Nav inputs
    static ULONGLONG s_pickerLastMove = 0;
    const ULONGLONG pNow = GetTickCount64();
    if (navInteractive && (pNow - s_pickerLastMove > 110)) {
      if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true) || UiPad::Up()) {
        s_teamPickerRow = (s_teamPickerRow + numTeams - 1) % numTeams;
        s_pickerLastMove = pNow;
        MenuSounds::Scroll();
      } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true) || UiPad::Down()) {
        s_teamPickerRow = (s_teamPickerRow + 1) % numTeams;
        s_pickerLastMove = pNow;
        MenuSounds::Scroll();
      }
    }

    const float modalW = std::clamp(380.0f * s, 320.0f * s, innerW * 0.55f);
    const float modalHdrH = 50.0f * s;
    const float modalFootH = 38.0f * s;
    const float teamRowH = 40.0f * s;
    const float teamRowGap = 5.0f * s;
    const float modalBodyH = static_cast<float>(numTeams) * (teamRowH + teamRowGap) + 8.0f * s;
    const float modalH = modalHdrH + modalBodyH + modalFootH;
    const ImVec2 m0(winPos.x + (size.x - modalW) * 0.5f,
                    winPos.y + (size.y - modalH) * 0.5f);
    const ImVec2 m1(m0.x + modalW, m0.y + modalH);

    // Frame & body background
    dl->AddRectFilled(m0, m1, IM_COL32(11, 18, 29, 252), 4.0f * s);
    DrawTacticalFrame(dl, m0, m1, s, true, false, false, false);

    // Header strip
    const ImVec2 mh1(m1.x, m0.y + modalHdrH);
    dl->AddRectFilled(m0, mh1, IM_COL32(18, 28, 44, 250), 4.0f * s,
                      ImDrawFlags_RoundCornersTop);
    dl->AddLine(ImVec2(m0.x, mh1.y), mh1, IM_COL32(143, 216, 255, 120), 1.0f);
    Text(dl, titleFont, 17.0f * s, ImVec2(m0.x + 16.0f * s, m0.y + 8.0f * s),
         kAccent, "SELECT TEAM");
    Text(dl, smallFont, 12.5f * s, ImVec2(m0.x + 16.0f * s, m0.y + 28.0f * s),
         kTextDim, "CHOOSE YOUR SQUAD ASSIGNMENT");

    // Team items
    int pickedTeam = -1;
    const float startY = mh1.y + 6.0f * s;
    for (int t = 0; t < numTeams; ++t) {
      const int teamNum = t + 1;
      const TeamStyle &ts = TeamStyleFor(teamNum);
      const float ry = startY + static_cast<float>(t) * (teamRowH + teamRowGap);
      const ImVec2 r0(m0.x + 12.0f * s, ry);
      const ImVec2 r1(m1.x - 12.0f * s, ry + teamRowH);
      const bool isFocused = (s_teamPickerRow == t);
      const bool isCurrent = (curTeam == teamNum);

      ImGui::PushID(t);
      ImGui::SetCursorScreenPos(r0);
      if (ImGui::InvisibleButton("##team_btn", ImVec2(r1.x - r0.x, teamRowH))) {
        s_teamPickerRow = t;
        pickedTeam = teamNum;
      }
      if (ImGui::IsItemHovered() && NobleUi::IsMouseMoved(2.0f)) {
        s_teamPickerRow = t;
      }
      ImGui::PopID();

      // Card background
      const ImU32 bgCol = isFocused ? IM_COL32(28, 50, 78, 240)
                                    : (isCurrent ? IM_COL32(18, 26, 38, 220)
                                                 : IM_COL32(14, 20, 30, 200));
      dl->AddRectFilled(r0, r1, bgCol, 2.0f * s);
      if (isFocused) {
        dl->AddRect(r0, r1, IM_COL32(143, 216, 255, 220), 2.0f * s, 0, 1.5f);
      } else {
        dl->AddRect(r0, r1, IM_COL32(45, 65, 90, 100), 2.0f * s, 0, 1.0f);
      }

      // Team color strip indicator on left
      dl->AddRectFilled(ImVec2(r0.x + 4.0f * s, r0.y + 4.0f * s),
                        ImVec2(r0.x + 12.0f * s, r1.y - 4.0f * s), ts.strip,
                        1.5f * s);

      // Team name
      Text(dl, titleFont, 15.5f * s,
           ImVec2(r0.x + 22.0f * s, r0.y + (teamRowH - 18.0f * s) * 0.5f),
           isFocused ? kText : kTextDim, ts.name);

      // Current team badge indicator on right
      if (isCurrent) {
        const char *tag = "[ CURRENT ]";
        const float tw = TextWidth(smallFont, 12.0f * s, tag);
        Text(dl, smallFont, 12.0f * s,
             ImVec2(r1.x - tw - 12.0f * s, r0.y + (teamRowH - 14.0f * s) * 0.5f),
             IM_COL32(95, 235, 130, 240), tag);
      }
    }

    // Accept / Confirm selection
    if (navInteractive && (UiPad::AcceptPressed() ||
                           ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                           ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false))) {
      UiPad::ConsumeAccept();
      pickedTeam = s_teamPickerRow + 1;
    }

    if (pickedTeam >= 1 && pickedTeam <= numTeams) {
      ChangeLocalTeam(pickedTeam);
      s_teamPickerOpen = false;
    }

    // Modal Footer Prompts
    const ImVec2 mf0(m0.x, m1.y - modalFootH);
    dl->AddLine(mf0, ImVec2(m1.x, mf0.y), IM_COL32(40, 65, 95, 120), 1.0f);
    float pfx = m0.x + 16.0f * s;
    const float pfy = mf0.y + 9.0f * s;
    pfx += InputPrompts::Draw(dl, smallFont, ImVec2(pfx, pfy), 18.0f * s,
                              InputPrompts::Prompt::Accept) + 8.0f * s;
    Text(dl, smallFont, 12.0f * s, ImVec2(pfx, pfy + 2.0f * s), kTextDim, "Select");
    pfx += TextWidth(smallFont, 12.0f * s, "Select") + 20.0f * s;

    pfx += InputPrompts::Draw(dl, smallFont, ImVec2(pfx, pfy), 18.0f * s,
                              InputPrompts::Prompt::Back) + 8.0f * s;
    Text(dl, smallFont, 12.0f * s, ImVec2(pfx, pfy + 2.0f * s), kTextDim, "Cancel");
  }

  // ---- Bottom Input Prompts Strip ----
  {
    const float fy = winPos.y + size.y - padY - footerH + 4.0f * s;
    const float fh = 22.0f * s;
    float fx = leftX;
    dl->AddLine(ImVec2(leftX, fy - 8.0f * s),
                ImVec2(leftX + innerW, fy - 8.0f * s),
                IM_COL32(143, 216, 255, 42), 1.0f);
    fx += InputPrompts::Draw(dl, smallFont, ImVec2(fx, fy), fh,
                             InputPrompts::Prompt::Accept) +
          10.0f * s;
    Text(dl, smallFont, 13.5f * s, ImVec2(fx, fy + 3.0f * s), kTextDim,
         "Select");
    fx += TextWidth(smallFont, 13.5f * s, "Select") + 22.0f * s;

    fx += InputPrompts::Draw(dl, smallFont, ImVec2(fx, fy), fh,
                             InputPrompts::Prompt::Back) +
          10.0f * s;
    Text(dl, smallFont, 13.5f * s, ImVec2(fx, fy + 3.0f * s), kTextDim,
         "Resume");
    fx += TextWidth(smallFont, 13.5f * s, "Resume") + 22.0f * s;

    fx += InputPrompts::Draw(dl, smallFont, ImVec2(fx, fy), fh,
                             InputPrompts::Prompt::Change) +
          10.0f * s;
    Text(dl, smallFont, 13.5f * s, ImVec2(fx, fy + 3.0f * s), kTextDim,
         chCount > 0 ? "Switch Deck / Challenges" : "Navigate");

    const char *scrollHint =
        (InputPrompts::Current() == InputPrompts::Device::Gamepad)
            ? "Right Stick: Scroll Challenges"
            : "Mouse Wheel: Scroll Challenges";
    const float sw = TextWidth(smallFont, 13.0f * s, scrollHint);
    Text(dl, smallFont, 13.0f * s,
         ImVec2(leftX + innerW - sw, fy + 3.0f * s), kTextDim, scrollHint);
  }

  ImGui::End();
  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar();
}

// The front-end SETTINGS panel, reused while paused so settings can be changed
// during a match. Every value that Noble can reach live (FOV, VSync, frame cap,
// mouse, audio, key bindings) is applied on APPLY; the panels' own help lines
// list the few that need a restart.
namespace {

void DrawSettingsPage(HWND hwnd) {
  (void)hwnd;
  ImGuiIO &io = ImGui::GetIO();
  // Full screen (small margin): the three-column settings need the room.
  const float width = io.DisplaySize.x - 32.0f;
  const float height = io.DisplaySize.y - 32.0f;
  ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
      ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
  const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoSavedSettings;
  if (ImGui::Begin("##NoblePauseSettings", nullptr, flags)) {
    NobleStyle::Header("SETTINGS");
    const float ps = NobleUi::Scale();
    if (ImGui::Button("< BACK", ImVec2(110.0f * ps, 0))) {
      s_showSettings.store(false);
      MenuSounds::Back();
      NOBLE_INFO("PauseMenu: settings closed");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Applied changes also reach this match. B goes back to "
                        "the pause menu.");
    ImGui::Separator();
    if (UiPad::BackPressed() && !PauseSettings().IsCapturingKey() &&
        !ImGui::GetIO().WantTextInput) {
      UiPad::ConsumeBack();
      s_showSettings.store(false);
      MenuSounds::Back();
    }
    PauseSettings().Render();
  }
  ImGui::End();
}

} // namespace

// ---------------------------------------------------------------- post-game
// "exit experience" Reach-style staged reveal (spec 4.8): scoreboard, then each
// cR line counting up (Totals, or the slot machine for a random bonus), the
// total, challenge / commendation progress bars, and the rank bar filling up to
// the new total (RankUp and the new icon on a promotion). Enter, Space or a
// click on an empty spot skips to the end. PLAY AGAIN / RETURN TO MENU / QUIT
// work at any time.
namespace {

struct Reveal {
  ULONGLONG start = 0; // s_gameOverAt the reveal belongs to
  bool skipped = false;
  int linesSounded = 0;
  bool completionSounded = false;
  bool totalSounded = false;
  bool rankUpSounded = false;
  bool keyWasDown = true; // ignore a key still held from the game
};
Reveal s_reveal;

constexpr float kFirstLine = 0.8f, kLineStep = 0.5f, kLineCount = 0.35f,
                kSlotRoll = 0.9f;

float Ease(float x) {
  x = std::clamp(x, 0.0f, 1.0f);
  return 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x);
}

bool IsSlotLine(const Career::AwardLine &l) {
  return l.label == "Random bonus" || l.label == "Jackpot!";
}

// The credit count and bar sounds repeat several times in a few seconds and sat
// well above the rest of the UI.
constexpr float kCountSfxVolume = 0.25f;

// Follows the Other volume slider, like the menu sounds.
void Sfx(ReachAudio::Sfx sfx, float volume = 1.0f) {
  const auto settings = Settings::Get().Snapshot();
  if (settings.menuSounds)
    ReachAudio::PlaySfx(sfx, volume * settings.volumeOther);
}

std::string Grouped(long long v) {
  std::string digits = std::to_string(v < 0 ? -v : v), out;
  for (size_t i = 0; i < digits.size(); ++i) {
    if (i && (digits.size() - i) % 3 == 0)
      out.push_back(',');
    out.push_back(digits[i]);
  }
  return v < 0 ? "-" + out : out;
}

// Section heading drawn straight into a draw list (letter-spaced caps + a
// rule).
void Heading(ImDrawList *dl, ImVec2 pos, float width, float s,
             const char *text) {
  ImFont *f = NobleStyle::SmallFont();
  const float px = 15.0f * s;
  float x = pos.x;
  char ch[2] = {0, 0};
  for (const char *c = text; *c; ++c) {
    ch[0] = *c;
    NobleUi::Text(dl, f, px, ImVec2(x, pos.y), IM_COL32(150, 190, 220, 255),
                  ch);
    x += NobleUi::TextWidth(f, px, ch) + 1.8f * s;
  }
  if (x + 10.0f * s < pos.x + width)
    dl->AddLine(ImVec2(x + 10.0f * s, pos.y + px * 0.55f),
                ImVec2(pos.x + width, pos.y + px * 0.55f), NobleUi::kRule,
                1.0f);
}

} // namespace

// Test option "-scoreboard-demo N": draws the panel with a sample roster as the
// host, without needing a real session. 1 = as seen while the key is held, 2 =
// pointer free with a player selected, 3 = the Report window as well.
static int ScoreboardDemo() {
  static const int s_demo = [] {
    int value = 0, argc = 0;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    for (int i = 1; argv && i + 1 < argc; ++i) {
      if (_wcsicmp(argv[i], L"-scoreboard-demo") == 0)
        value = _wtoi(argv[i + 1]);
    }
    if (argv)
      LocalFree(argv);
    return value;
  }();
  return s_demo;
}

// A report that could not reach the master server stays on this PC (one JSON
// object per line).
static bool SaveReport(const std::string &player, const char *reason,
                       const std::string &map, const std::string &mode) {
  wchar_t exe[MAX_PATH] = {};
  GetModuleFileNameW(nullptr, exe, MAX_PATH);
  const std::filesystem::path file = std::filesystem::path(exe).parent_path() /
                                     "data" / "userdata" / "reports.jsonl";
  std::error_code ec;
  std::filesystem::create_directories(file.parent_path(), ec);
  std::ofstream out(file, std::ios::app);
  if (!out)
    return false;
  const nlohmann::json line = {
      {"time", static_cast<long long>(std::time(nullptr))},
      {"player", player},
      {"reason", reason},
      {"map", map},
      {"mode", mode}};
  out << line.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace)
      << "\n";
  return out.good();
}

// A button that cannot work yet still shows, greyed, and says why when hovered.
static void UnavailableButton(const char *label, const ImVec2 &size,
                              const char *why) {
  ImGui::BeginDisabled();
  ImGui::Button(label, size);
  ImGui::EndDisabled();
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    ImGui::SetTooltip("%s", why);
}

// In-match scoreboard. Noble's panel replaces Reach's own scoreboard (ReachHost
// keeps the key from the engine) and lists the session roster with rank badges,
// service tags and team colours. A click while the key is held frees the
// pointer for the player actions. Static sets for individual and global voice
// mutes across the match.
static std::set<std::string> s_mutedNames;
static bool s_muteAllActive = false;
static bool s_showPlayerModal = false;

struct RawScore {
  wchar_t name[17] = {};
  wchar_t tag[6] = {};
  int32_t team = -1;
  int32_t kills = 0;
  int32_t assists = 0;
  int32_t deaths = 0;
  int32_t score = 0;
  bool local = false;
  bool valid = false;
};

// POD SEH reader to avoid MSVC C2712 (no C++ objects requiring destruction
// inside __try). Reads haloreach.dll's g_game_results structure
// (haloreach+0x28B83B0):
//   - Player entries (16 slots, stride 0xD8) at base + 0xFEF8:
//       +0x00 (u8): player_valid
//       +0x10 (wchar_t[16]): player_name
//       +0x62 (i8): team_index (0..7)
//       +0x6C (wchar_t[5]): service_tag
//       +0xBA (i16): live player score (base + 0xFFB2 + slot * 0xD8)
//   - Live in_game_statistics array (16 slots, stride 0x708) at base + 0x10E50:
//       Each stat is a 15-bit uint16_t bitfield at base + 0x10E50 + slot *
//       0x708 + stat_id * 2 (bits 0..14 hold the value, read via raw &
//       0x7FFFu): stat_id 6 = in_round_score, 7 = in_game_total_score, 8 =
//       kills, 9 = assists, 10 = deaths.
//   - Also reads the TLS players datum array (tlsBase + 0x18, stride 0x490) for
//   player
//     presence, local-player flag (_player_is_local_machine_bit = 0x08), name,
//     team (+0xAD), and service tag (+0x10C), plus TLS game_engine_globals
//     (tlsBase + 0x20, +0x3270 + slot * 8) for live round/total score.
static void ReadRawScoresSEH(const uint8_t *reachBase, RawScore *outScores,
                             size_t count) {
  __try {
    const uint8_t *table = reachBase + 0x28B83B0;
    for (size_t i = 0; i < count && i < 16; ++i) {
      const uint8_t *p = table + 0xFEF8 + (i * 0xD8);
      if (p[0] != 0) {
        outScores[i].valid = true;
        const wchar_t *wname = reinterpret_cast<const wchar_t *>(p + 0x10);
        for (size_t c = 0; c < 16 && wname[c] != 0; ++c)
          outScores[i].name[c] = wname[c];
        outScores[i].name[16] = 0;
        const int8_t rawTeam = *reinterpret_cast<const int8_t *>(p + 0x62);
        outScores[i].team = (rawTeam >= 0 && rawTeam < 8) ? (rawTeam + 1) : 0;
        const wchar_t *wtag = reinterpret_cast<const wchar_t *>(p + 0x6C);
        for (size_t c = 0; c < 5 && wtag[c] != 0; ++c)
          outScores[i].tag[c] = wtag[c];
        outScores[i].tag[5] = 0;
      }

      const uint8_t *statsBase = table + 0x10E50 + (i * 0x708);
      auto readStat = [statsBase](size_t statId) -> int32_t {
        const uint16_t raw =
            *reinterpret_cast<const uint16_t *>(statsBase + statId * 2);
        if (raw == 0xFFFFu)
          return 0;
        return static_cast<int32_t>(raw & 0x7FFFu);
      };

      const int32_t k = readStat(8);
      const int32_t a = readStat(9);
      const int32_t d = readStat(10);
      const int32_t roundScore = readStat(6);
      const int32_t totalScore = readStat(7);
      const int16_t liveScore16 = *reinterpret_cast<const int16_t *>(p + 0xBA);

      outScores[i].kills = (k >= 0 && k < 32768) ? k : 0;
      outScores[i].assists = (a >= 0 && a < 32768) ? a : 0;
      outScores[i].deaths = (d >= 0 && d < 32768) ? d : 0;
      if (liveScore16 != 0) {
        outScores[i].score = liveScore16;
      } else if (totalScore != 0) {
        outScores[i].score = totalScore;
      } else {
        outScores[i].score = roundScore;
      }
    }

    // Supplement with live TLS players datum array (stride 0x490) and TLS
    // game_engine_globals score array so player identity, local flag, team, and
    // score are always populated in real time.
    const uint32_t tlsIdx =
        *reinterpret_cast<const uint32_t *>(reachBase + 0xC17B18);
    if (tlsIdx < 64) {
      const uintptr_t tlsVec = __readgsqword(0x58);
      if (tlsVec) {
        const uintptr_t tlsBase = *reinterpret_cast<const uintptr_t *>(
            tlsVec + static_cast<uintptr_t>(tlsIdx) * 8);
        if (tlsBase) {
          const uint8_t *playersTable =
              *reinterpret_cast<const uint8_t *const *>(tlsBase + 0x18);
          if (playersTable) {
            const int32_t maxP =
                *reinterpret_cast<const int32_t *>(playersTable + 0x44);
            const uint8_t *entries =
                *reinterpret_cast<const uint8_t *const *>(playersTable + 0x50);
            if (entries && maxP > 0 && maxP <= 16) {
              for (int32_t i = 0; i < maxP && static_cast<size_t>(i) < count;
                   ++i) {
                const uint8_t *entry = entries + static_cast<size_t>(i) * 0x490;
                const uint16_t salt =
                    *reinterpret_cast<const uint16_t *>(entry);
                if (salt == 0)
                  continue;
                const uint32_t flags =
                    *reinterpret_cast<const uint32_t *>(entry + 0x04);
                if ((flags & 0x01u) != 0)
                  continue; // skip _player_left_game_bit
                const wchar_t *wname =
                    reinterpret_cast<const wchar_t *>(entry + 0xB0);
                if (wname[0] == 0)
                  continue;
                outScores[i].valid = true;
                if (outScores[i].name[0] == 0) {
                  for (size_t c = 0; c < 16 && wname[c] != 0; ++c)
                    outScores[i].name[c] = wname[c];
                  outScores[i].name[16] = 0;
                }
                if (outScores[i].tag[0] == 0) {
                  const wchar_t *wtag =
                      reinterpret_cast<const wchar_t *>(entry + 0x10C);
                  for (size_t c = 0; c < 5 && wtag[c] != 0; ++c)
                    outScores[i].tag[c] = wtag[c];
                  outScores[i].tag[5] = 0;
                }
                if (outScores[i].team <= 0) {
                  int8_t rawTeam =
                      *reinterpret_cast<const int8_t *>(entry + 0xAD);
                  if (rawTeam < 0 || rawTeam >= 8) {
                    rawTeam = *reinterpret_cast<const int8_t *>(entry + 0x102);
                  }
                  outScores[i].team =
                      (rawTeam >= 0 && rawTeam < 8) ? (rawTeam + 1) : 0;
                }
                if ((flags & 0x08u) != 0) {
                  outScores[i].local = true;
                }
              }
            }
          }

          // Live score fallback from TLS game_engine_globals (+0x3270 + slot *
          // 8)
          const uint8_t *geGlobals =
              *reinterpret_cast<const uint8_t *const *>(tlsBase + 0x20);
          if (geGlobals) {
            for (size_t i = 0; i < count && i < 16; ++i) {
              if (outScores[i].score == 0) {
                const int16_t geTotalScore = *reinterpret_cast<const int16_t *>(
                    geGlobals + 0x3272 + i * 8);
                const int16_t geRoundScore = *reinterpret_cast<const int16_t *>(
                    geGlobals + 0x3270 + i * 8);
                if (geTotalScore != 0) {
                  outScores[i].score = geTotalScore;
                } else if (geRoundScore != 0) {
                  outScores[i].score = geRoundScore;
                }
              }
            }
          }
        }
      }
    }
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }
}

static std::string WideToUtf8Short(const wchar_t *w) {
  if (!w || w[0] == 0)
    return {};
  char buf[64] = {};
  WideCharToMultiByte(CP_UTF8, 0, w, -1, buf, static_cast<int>(sizeof(buf) - 1),
                      nullptr, nullptr);
  return buf;
}

// Polls active game engine results table (haloreach+0x28B83B0) and TLS players
// table so that player roster, kills, assists, deaths, scores, and ping update
// live in real time across Firefight, Custom Games, Campaign, and Forge.
static void UpdateLiveScoreboard() {
  HMODULE reach = noble::ReachModule();
  if (!reach)
    return;
  const uint8_t *reachBase = reinterpret_cast<const uint8_t *>(reach);
  RawScore scores[16] = {};
  ReadRawScoresSEH(reachBase, scores, 16);

  const SettingsData snap = Settings::Get().Snapshot();
  const int localRank = Career::RankIndex(Career::Get().credits);
  const bool netActive = NetworkTransport::Get().IsActive();

  std::lock_guard<std::mutex> lock(s_boardMutex);
  bool anyValid = false;
  bool anyTlsLocal = false;
  for (size_t i = 0; i < 16; ++i) {
    if (scores[i].valid && scores[i].name[0] != 0) {
      anyValid = true;
      if (scores[i].local) {
        anyTlsLocal = true;
      }
    }
  }

  if (anyValid) {
    for (size_t i = 0; i < 16; ++i) {
      if (!scores[i].valid || scores[i].name[0] == 0)
        continue;
      const std::string nameUtf8 = WideToUtf8Short(scores[i].name);
      const std::string tagUtf8 = WideToUtf8Short(scores[i].tag);
      if (nameUtf8.empty())
        continue;
      const bool isLocalPlayer =
          scores[i].local ||
          (!anyTlsLocal &&
           (_stricmp(nameUtf8.c_str(), snap.playerName.c_str()) == 0));

      PauseMenu::ScoreboardEntry *matched = nullptr;
      for (auto &existing : s_boardRows) {
        if (_stricmp(existing.name.c_str(), nameUtf8.c_str()) == 0) {
          matched = &existing;
          break;
        }
      }
      if (!matched && i < s_boardRows.size() && s_boardRows[i].name.empty()) {
        matched = &s_boardRows[i];
        matched->name = nameUtf8;
      }
      if (!matched) {
        // A row added from the transport peer list while the engine table was empty (LAN/direct launch):
        // give it the engine's real name instead of adding a duplicate row for the same player.
        for (auto &existing : s_boardRows) {
          if (existing.name.rfind("PLAYER ", 0) == 0) {
            matched = &existing;
            existing.name = nameUtf8;
            break;
          }
        }
      }
      if (!matched) {
        PauseMenu::ScoreboardEntry entry;
        entry.name = nameUtf8;
        entry.local = isLocalPlayer;
        entry.tag =
            entry.local && !snap.serviceTag.empty() ? snap.serviceTag : tagUtf8;
        entry.team = scores[i].team >= 0 ? scores[i].team : 1;
        entry.rank = entry.local ? localRank : 0;
        entry.ping = 0; // filled per player below; unknown stays -1 (drawn as "-")
        s_boardRows.push_back(entry);
        matched = &s_boardRows.back();
      }

      matched->local = isLocalPlayer;
      if (matched->local && !snap.serviceTag.empty()) {
        matched->tag = snap.serviceTag;
      } else if (!tagUtf8.empty()) {
        matched->tag = tagUtf8;
      }
      if (scores[i].team >= 0) {
        matched->team = scores[i].team;
      }
      if (matched->local && matched->rank <= 0) {
        matched->rank = localRank;
      }
      matched->kills = scores[i].kills;
      matched->assists = scores[i].assists;
      matched->deaths = scores[i].deaths;
      matched->score = scores[i].score;
    }
  } else {
    // Fallback if player names aren't in g_game_results yet: update by slot
    // index
    for (size_t i = 0; i < s_boardRows.size() && i < 16; ++i) {
      auto &row = s_boardRows[i];
      row.kills = scores[i].kills;
      row.assists = scores[i].assists;
      row.deaths = scores[i].deaths;
      row.score = scores[i].score;
    }
  }

  // A guest often has only the engine's results table, so a row can arrive without a machine id. Match
  // the name against the names the lobby/voice layer already tracks so its measured RTT is used instead
  // of "-".
  {
    const auto knownNames = SocialOverlay::GetPlayerNames();
    for (auto &row : s_boardRows) {
      if (row.machine != 0) continue;
      for (const auto &entry : knownNames) {
        if (entry.first != 0 && !entry.second.empty() &&
            _stricmp(entry.second.c_str(), row.name.c_str()) == 0) {
          row.machine = entry.first;
          break;
        }
      }
    }
  }

  // Real per-player RTT only. No round-robin fallback: attaching another peer's number to a row is worse
  // than showing "-". The roster seed carries each player's machine id and the match transport now uses
  // the same id the lobby published.
  int remoteRows = 0;
  for (const auto &row : s_boardRows)
    if (!row.local) ++remoteRows;
  for (auto &row : s_boardRows) {
    if (row.local) {
      row.ping = 0; // self / host reference
    } else if (netActive) {
      int ms = -1;
      if (row.machine != 0) {
        ms = NetworkTransport::Get().GetPeerPingMs(row.machine);
      } else if (row.hostPeer) {
        ms = NetworkTransport::Get().IsHost() ? 0 : NetworkTransport::Get().GetPeerPingMs(0);
      }
      // A direct-connect two-player session has exactly one remote peer, so its RTT is unambiguous even
      // without a lobby roster (host knows the guest, guest knows the host).
      if (ms < 0 && remoteRows == 1) {
        if (!NetworkTransport::Get().IsHost()) {
          ms = NetworkTransport::Get().GetPeerPingMs(0);
        } else {
          const auto pings = NetworkTransport::Get().GetAllPeerPingsMs();
          if (pings.size() == 1) ms = pings.begin()->second;
        }
      }
      row.ping = ms; // -1 means unknown and is drawn as "-"
    } else if (row.ping < 0) {
      row.ping = 0;
    }
  }

  // Direct/LAN launches pass no -session-id, so main.cpp never seeds the board from the master roster, and the
  // engine results table can stay empty (or list only the local player) while several peers are already in the
  // session. The transport knows every peer in the session, so top the board up to the live player count with
  // placeholder rows (unknown names stay "PLAYER n"); a named engine row that arrives later reuses its
  // placeholder (above). Also runs when the engine table has rows, which is the normal LAN/direct-launch case.
  {
    const size_t transportPeers = NetworkTransport::Get().GetGamePeerCount();
    const size_t gamePlayers = LobbyPatches::GetGamePlayerCount();
    // Authoritative live player count: the engine's own game-player table when it has entries; otherwise the
    // transport peer count (direct connects) or the msg-27 counter. Never let transport bookkeeping invent rows.
    size_t livePlayers = gamePlayers;
    if (livePlayers < 2) {
      livePlayers = (std::max)(transportPeers + 1, LobbyPatches::GetRemotePlayerAddCount() + 1);
    }
    if (livePlayers > 16) livePlayers = 16;
    if (livePlayers > 0 && s_boardRows.size() < livePlayers) {
      if (s_boardRows.empty()) {
        PauseMenu::ScoreboardEntry me;
        me.name =
            snap.playerName.empty() ? std::string("NobleSix") : snap.playerName;
        me.tag = snap.serviceTag;
        me.local = true;
        me.team = s_localTeam.load();
        me.rank = localRank;
        me.ping = 0;
        s_boardRows.push_back(me);
      }
      auto placeholderName = [&]() {
        return "PLAYER " + std::to_string(s_boardRows.size() + 1);
      };
      const auto pings = NetworkTransport::Get().GetAllPeerPingsMs();
      for (const auto &kv : pings) {
        if (s_boardRows.size() >= livePlayers)
          break;
        if (kv.first == 0 ||
            kv.first == NetworkTransport::Get().GetLocalMachineId())
          continue;
        bool known = false;
        for (const auto &row : s_boardRows) {
          if (row.machine == kv.first) {
            known = true;
            break;
          }
        }
        if (known)
          continue;
        PauseMenu::ScoreboardEntry entry;
        entry.machine = kv.first;
        entry.ping = kv.second;
        entry.name = placeholderName();
        entry.team = s_localTeam.load();
        s_boardRows.push_back(entry);
      }
      // Peers without a measured round-trip time still get a row.
      while (s_boardRows.size() < livePlayers) {
        PauseMenu::ScoreboardEntry entry;
        entry.name = placeholderName();
        entry.team = s_localTeam.load();
        s_boardRows.push_back(entry);
      }
      static size_t s_lastLiveRosterLogged = 0;
      if (s_lastLiveRosterLogged != s_boardRows.size()) {
        s_lastLiveRosterLogged = s_boardRows.size();
        NOBLE_INFO("PauseMenu: live roster {} player(s) (transport {}, "
                   "membership {})",
                   s_boardRows.size(), transportPeers,
                   LobbyPatches::GetRemotePlayerAddCount());
      }
    }
  }

  // Exactly one row may be "you". The engine's results table and the TLS player array can both flag a
  // slot, and on a guest that used to mark every row local - the panel then drew "YOU" on everyone.
  // Prefer the row whose name is this player, then the local machine id, then the first flagged row.
  {
    int localIndex = -1;
    for (size_t i = 0; i < s_boardRows.size(); ++i) {
      const auto &row = s_boardRows[i];
      if (!row.local) continue;
      if (localIndex < 0) localIndex = static_cast<int>(i);
      if (!snap.playerName.empty() &&
          _stricmp(row.name.c_str(), snap.playerName.c_str()) == 0) {
        localIndex = static_cast<int>(i);
        break;
      }
    }
    for (size_t i = 0; i < s_boardRows.size(); ++i) {
      s_boardRows[i].local = static_cast<int>(i) == localIndex;
    }
    if (localIndex >= 0 && !snap.serviceTag.empty()) {
      s_boardRows[static_cast<size_t>(localIndex)].tag = snap.serviceTag;
    }
  }
}

// Modernized in-match scoreboard overlay replicating the 2-column layout from
// Halo: Reach MCC. Supports mouse and gamepad navigation, individual and
// all-player mute toggles, live ping RTT, team score aggregate bars, emblem /
// mute icon badges, rank insignias, and player options.
static void DrawScoreboardPanel() {
  UpdateLiveScoreboard();

  using namespace NobleUi;
  const int demo = ScoreboardDemo();
  const bool pointer = s_boardPointer.load();
  const bool isHost = demo > 0 || (NetworkTransport::Get().IsActive() &&
                                   NetworkTransport::Get().IsHost());
  static std::string s_selected;
  static std::string s_status;
  static int s_reportReason = 0;
  static std::atomic<int> s_reportResult{0};
  static std::atomic<bool> s_reportSending{false};
  ImGuiIO &io = ImGui::GetIO();
  const float W = io.DisplaySize.x, H = io.DisplaySize.y;
  const float s = std::clamp(H / 1080.0f, 0.7f, 2.0f) * Settings::UiScale();

  std::vector<PauseMenu::ScoreboardEntry> rows;
  std::string mapName, modeName;
  {
    std::lock_guard<std::mutex> lock(s_boardMutex);
    rows = s_boardRows;
    mapName = s_boardMap;
    modeName = s_boardMode;
  }

  if (rows.empty()) {
    const SettingsData snap = Settings::Get().Snapshot();
    PauseMenu::ScoreboardEntry me;
    me.name = snap.playerName;
    me.tag = snap.serviceTag.empty() ? "NOBL" : snap.serviceTag;
    me.local = true;
    me.team = LocalTeamFromBoard();
    me.rank = Career::RankIndex(Career::Get().credits);
    me.ping = 0;
    rows.push_back(me);
  }

  if (demo > 0) {
    rows.clear();
    // Red Team roster matching the reference layout
    const struct MockPlayer {
      const char *name;
      const char *tag;
      int rank;
      int team;
      int score;
      int k;
      int d;
      int a;
      int ping;
      bool muted;
    } kMockRed[] = {
        {"ItsMeAdam", "ADAM", 28, 1, 18, 27, 4, 2, 51, false},
        {"Player 3", "", 14, 1, 11, 14, 6, 0, 100, false},
        {"EmberPrimeMain", "KING", 22, 1, 5, 6, 8, 2, 108, false},
        {"Dinoextremo29", "DINO", 18, 1, 5, 5, 7, 2, 101, true},
        {"gzhouse", "GZHO", 8, 1, 4, 4, 9, 1, 44, true},
        {"PR Glazer", "A062", 16, 1, 4, 4, 12, 2, 41, false},
        {"A Fellow Ape", "APE1", 6, 1, 3, 4, 8, 1, 101, false},
        {"LIOX", "LIOX", 10, 1, 2, 3, 6, 1, 111, false},
        {"Goppy", "EZPZ", 10, 1, 2, 2, 2, 0, 31, false},
        {"Commando414", "A414", 12, 1, 1, 1, 8, 2, 46, false},
        {"Steven Reed", "STEV", 14, 1, 1, 1, 1, 0, 83, false},
        {"NobleHalo", "NOBL", 26, 1, 0, 0, 8, 4, 95, false},
        {"PROJECT RYAN", "", 4, 1, 0, 0, 1, 0, 22, false},
        {"DANIMALS", "BF3", 10, 1, 0, 0, 1, 0, 47, false},
    };
    for (size_t i = 0; i < sizeof(kMockRed) / sizeof(kMockRed[0]); ++i) {
      PauseMenu::ScoreboardEntry e;
      e.name = kMockRed[i].name;
      e.tag = kMockRed[i].tag;
      e.rank = kMockRed[i].rank;
      e.team = kMockRed[i].team;
      e.score = kMockRed[i].score;
      e.kills = kMockRed[i].k;
      e.deaths = kMockRed[i].d;
      e.assists = kMockRed[i].a;
      e.ping = kMockRed[i].ping;
      e.muted = kMockRed[i].muted;
      e.machine = 1000ULL + i;
      if (i == 0)
        e.local = true;
      rows.push_back(e);
    }

    // Blue Team roster matching the reference layout
    const MockPlayer kMockBlue[] = {
        {"PhosTheFloss", "PHOS", 20, 2, 14, 15, 2, 1, 91, false},
        {"Daniel Orrin", "ORRN", 16, 2, 13, 15, 9, 2, 48, false},
        {"M DooMknight", "DOOM", 12, 2, 7, 8, 5, 0, 101, false},
        {"Raka Zodam", "RAKA", 8, 2, 6, 8, 5, 1, 56, false},
        {"JP", "N64", 16, 2, 6, 8, 9, 1, 55, false},
        {"hitler", "HITL", 18, 2, 5, 6, 5, 3, 17, false},
        {"RaptorGunner13", "R13", 14, 2, 5, 6, 8, 1, 50, false},
        {"Sakuya Izayoi", "2HU", 24, 2, 5, 5, 7, 1, 99, false},
        {"Lexa Varomaii", "VKAI", 16, 2, 4, 5, 3, 0, 56, false},
        {"Connor", "K488", 20, 2, 4, 4, 2, 0, 49, false},
        {"Stop Shooting", "STOP", 6, 2, 3, 3, 5, 1, 55, false},
        {"RainZaphkiel", "DÎ£V", 22, 2, 2, 2, 3, 0, 48, false},
        {"RED", "RED", 10, 2, 1, 1, 0, 0, 43, false},
        {"HighImCasual", "0408", 16, 2, 1, 1, 1, 0, 64, false},
    };
    for (size_t i = 0; i < sizeof(kMockBlue) / sizeof(kMockBlue[0]); ++i) {
      PauseMenu::ScoreboardEntry e;
      e.name = kMockBlue[i].name;
      e.tag = kMockBlue[i].tag;
      e.rank = kMockBlue[i].rank;
      e.team = kMockBlue[i].team;
      e.score = kMockBlue[i].score;
      e.kills = kMockBlue[i].k;
      e.deaths = kMockBlue[i].d;
      e.assists = kMockBlue[i].a;
      e.ping = kMockBlue[i].ping;
      e.muted = kMockBlue[i].muted;
      e.machine = 2000ULL + i;
      rows.push_back(e);
    }

    if (mapName.empty()) {
      mapName = "Global Battles v40";
      modeName = "Team Slayer";
    }
    if (s_selected.empty() && demo >= 2)
      s_selected = "Player 3";
    if ((demo == 3 || demo == 5) && s_boardPointer.load()) {
      static bool s_once = false;
      if (!s_once) {
        s_once = true;
        s_boardReport.store(true);
      }
    }
  }

  // Determine mode category: Campaign and Firefight are unified co-op (allied
  // UNSC team)
  std::string lowerMode = modeName;
  for (char &c : lowerMode)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  const bool isCoopMode = (lowerMode == "campaign" || lowerMode == "firefight");

  // Sort: by team first (if competitive), then by score descending, kills
  // descending, deaths ascending
  std::sort(rows.begin(), rows.end(),
            [isCoopMode](const PauseMenu::ScoreboardEntry &a,
                         const PauseMenu::ScoreboardEntry &b) {
              if (!isCoopMode && a.team != b.team)
                return a.team < b.team;
              if (a.score != b.score)
                return a.score > b.score;
              if (a.kills != b.kills)
                return a.kills > b.kills;
              return a.deaths < b.deaths;
            });

  bool hasTeams = false;
  if (!isCoopMode) {
    for (const auto &r : rows)
      hasTeams = hasTeams || r.team > 0;
  }

  // Multi-team colors and names (supporting all 8 Reach multiplayer teams)
  struct ScoreboardTeamStyle {
    const char *name;
    ImU32 headerCol;
    ImU32 rowBaseCol;
    ImU32 rowAltCol;
  };
  static const ScoreboardTeamStyle kTeamStyles[8] = {
      {"RED TEAM", IM_COL32(195, 42, 42, 245), IM_COL32(145, 30, 30, 150),
       IM_COL32(130, 25, 25, 150)},
      {"BLUE TEAM", IM_COL32(38, 92, 185, 245), IM_COL32(28, 62, 145, 150),
       IM_COL32(24, 52, 130, 150)},
      {"GREEN TEAM", IM_COL32(38, 160, 68, 245), IM_COL32(28, 120, 52, 150),
       IM_COL32(24, 105, 45, 150)},
      {"ORANGE TEAM", IM_COL32(220, 115, 28, 245), IM_COL32(165, 85, 20, 150),
       IM_COL32(145, 75, 18, 150)},
      {"PURPLE TEAM", IM_COL32(150, 48, 195, 245), IM_COL32(110, 35, 145, 150),
       IM_COL32(95, 30, 130, 150)},
      {"YELLOW TEAM", IM_COL32(215, 180, 30, 245), IM_COL32(160, 135, 22, 150),
       IM_COL32(140, 118, 18, 150)},
      {"BROWN TEAM", IM_COL32(140, 85, 45, 245), IM_COL32(105, 62, 32, 150),
       IM_COL32(90, 54, 28, 150)},
      {"PINK TEAM", IM_COL32(225, 80, 150, 245), IM_COL32(165, 58, 110, 150),
       IM_COL32(145, 50, 96, 150)}};

  std::vector<int> columnRows[2];
  int teamScores[2] = {0, 0};
  int teamKills[2] = {0, 0};
  int teamDeaths[2] = {0, 0};
  int teamAssists[2] = {0, 0};
  int activeTeamIndices[2] = {0, 1};
  int presentTeamCount = 1; // how many teams the match really has: one team is
                            // one column, not red-vs-blue

  if (isCoopMode) {
    // Campaign & Firefight: Unified UNSC Team in a single column
    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
      columnRows[0].push_back(i);
      teamScores[0] += rows[i].score;
      teamKills[0] += rows[i].kills;
      teamDeaths[0] += rows[i].deaths;
      teamAssists[0] += rows[i].assists;
    }
  } else if (hasTeams) {
    // Check which teams are active in the match
    std::vector<int> presentTeams;
    for (const auto &r : rows) {
      const int t = std::clamp(r.team, 1, 8);
      if (std::find(presentTeams.begin(), presentTeams.end(), t) ==
          presentTeams.end()) {
        presentTeams.push_back(t);
      }
    }
    std::sort(presentTeams.begin(), presentTeams.end());
    // Follow the teams that are actually in the match. A solo gold-team session
    // used to be drawn as "RED TEAM" with the gold row inside it, because the
    // headers were always teams 1 and 2.
    if (!presentTeams.empty()) {
      presentTeamCount = static_cast<int>(presentTeams.size());
      activeTeamIndices[0] = presentTeams[0] - 1;
      activeTeamIndices[1] =
          presentTeams.size() > 1 ? presentTeams[1] - 1 : presentTeams[0] - 1;
    }
    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
      const int t = std::clamp(rows[i].team, 1, 8);
      const int col =
          (presentTeamCount > 1 && t == (activeTeamIndices[1] + 1)) ? 1 : 0;
      columnRows[col].push_back(i);
      teamScores[col] += rows[i].score;
      teamKills[col] += rows[i].kills;
      teamDeaths[col] += rows[i].deaths;
      teamAssists[col] += rows[i].assists;
    }
  } else {
    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
      columnRows[0].push_back(i);
      teamScores[0] += rows[i].score;
      teamKills[0] += rows[i].kills;
      teamDeaths[0] += rows[i].deaths;
      teamAssists[0] += rows[i].assists;
    }
  }

  // In demo mode or if scores are unconfigured in real match, align totals with
  // the reference
  if (demo > 0) {
    teamScores[0] = 2;
    teamScores[1] = 0;
    teamKills[0] = 71;
    teamKills[1] = 87;
    teamDeaths[0] = 81;
    teamDeaths[1] = 64;
    teamAssists[0] = 17;
    teamAssists[1] = 11;
  }

  const int columnCount = (hasTeams && presentTeamCount > 1) ? 2 : 1;
  const float panelW = std::min(W * 0.96f, 1320.0f * s);
  const float colGap = 10.0f * s;
  const float colW = (panelW - (columnCount - 1) * colGap) / columnCount;
  const float rowH = 34.0f * s;
  const float headerH = 36.0f * s;
  const float subHeaderH = 22.0f * s;
  const float footerH = 42.0f * s;
  const float top = 56.0f * s;

  const int maxVisible = std::max(
      6, static_cast<int>(
             (H - top - headerH - subHeaderH - footerH - 40.0f * s) / rowH));
  const int tallest =
      static_cast<int>(std::max(columnRows[0].size(), columnRows[1].size()));
  const int visibleRows = std::min(maxVisible, std::max(14, tallest));
  const float totalListH = visibleRows * rowH;
  const float panelH = headerH + subHeaderH + totalListH + footerH;

  const ImVec2 p0((W - panelW) * 0.5f, top);
  const ImVec2 p1(p0.x + panelW, p0.y + panelH);

  if (pointer) {
    ImGui::SetNextWindowPos(p0, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_Always);
    ImGui::Begin(
        "##scoreboard", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus);
  }

  ImDrawList *dl = ImGui::GetBackgroundDrawList();

  // Dim the background behind the scoreboard cleanly
  dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, H), IM_COL32(0, 0, 0, 110));

  static const ImU32 kTeamHeaderCol[2] = {IM_COL32(195, 42, 42, 245),
                                          IM_COL32(38, 92, 185, 245)};
  static const ImU32 kTeamRowBaseCol[2] = {IM_COL32(145, 30, 30, 150),
                                           IM_COL32(28, 62, 145, 150)};
  static const ImU32 kTeamRowAltCol[2] = {IM_COL32(130, 25, 25, 150),
                                          IM_COL32(24, 52, 130, 150)};

  static int s_boardScroll[2] = {0, 0};
  bool selectedIsListed = false;
  for (const auto &r : rows) {
    if (!r.local && r.name == s_selected)
      selectedIsListed = true;
  }

  if (!pointer && UiPad::AcceptPressed()) {
    UiPad::ConsumeAccept();
    PauseMenu::EnterScoreboardPointer();
  }

  // Gamepad & Keyboard row navigation
  if (pointer && !s_boardReport.load() && !s_showPlayerModal) {
    int column = 0, row = -1;
    for (int c = 0; c < columnCount; ++c) {
      for (int k = 0; k < static_cast<int>(columnRows[c].size()); ++k) {
        if (rows[columnRows[c][k]].name == s_selected) {
          column = c;
          row = k;
        }
      }
    }
    const auto pick = [&](int c, int from, int step) {
      const std::vector<int> &list = columnRows[c];
      const int count = static_cast<int>(list.size());
      for (int k = std::clamp(from, 0, std::max(count - 1, 0)), tries = 0;
           count > 0 && k >= 0 && k < count && tries <= count;
           k += step, ++tries) {
        s_selected = rows[list[k]].name;
        s_status.clear();
        selectedIsListed = true;
        s_boardScroll[c] =
            std::clamp(s_boardScroll[c], k - std::max(visibleRows - 1, 0), k);
        MenuSounds::Scroll();
        return true;
      }
      return false;
    };
    if (UiPad::Down() || ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)) {
      if (!pick(column, row + 1, 1) && columnCount > 1)
        pick(1 - column, 0, 1);
    }
    if ((UiPad::Up() || ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)) &&
        row > 0) {
      pick(column, row - 1, -1);
    }
    if (columnCount > 1 && (UiPad::Left() || UiPad::Right() ||
                            ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false) ||
                            ImGui::IsKeyPressed(ImGuiKey_RightArrow, false))) {
      pick(1 - column, std::max(row, 0), 1);
    }
    if (UiPad::AnalogScrollY(false) != 0.0f && columnCount > 0) {
      static float s_sbAccum = 0.0f;
      s_sbAccum += UiPad::AnalogScrollY(false) / std::max(1.0f, 34.0f * s);
      const int step = static_cast<int>(s_sbAccum);
      if (step != 0) {
        s_boardScroll[column] =
            std::clamp(s_boardScroll[column] + step, 0,
                       std::max(0, static_cast<int>(columnRows[column].size()) -
                                       visibleRows));
        s_sbAccum -= static_cast<float>(step);
      }
    }
    if (UiPad::AcceptPressed() && !s_selected.empty()) {
      UiPad::ConsumeAccept();
      s_showPlayerModal = true;
      MenuSounds::Click();
    }
  }

  // Render both team columns side-by-side
  for (int col = 0; col < columnCount; ++col) {
    const std::vector<int> &list = columnRows[col];
    const int count = static_cast<int>(list.size());
    const float cx = p0.x + col * (colW + colGap);
    const bool overflow = count > visibleRows;

    if (overflow && pointer && !s_boardReport.load() && !s_showPlayerModal &&
        ImGui::IsMouseHoveringRect(
            ImVec2(cx, p0.y),
            ImVec2(cx + colW, p0.y + headerH + subHeaderH + totalListH),
            false) &&
        io.MouseWheel != 0.0f) {
      static const char *const kBoardWheelIds[2] = {"##sb_wheel_0",
                                                    "##sb_wheel_1"};
      const int wheelSteps =
          NobleUi::ConsumeWheelSteps(kBoardWheelIds[col & 1], io.MouseWheel);
      if (wheelSteps != 0)
        s_boardScroll[col] -= wheelSteps;
    }
    s_boardScroll[col] =
        std::clamp(s_boardScroll[col], 0, std::max(0, count - visibleRows));
    const int scroll = s_boardScroll[col];

    // 1. Solid Team Header Banner
    const ImVec2 h0(cx, p0.y);
    const ImVec2 h1(cx + colW, p0.y + headerH);
    const ImU32 bannerColor =
        isCoopMode ? IM_COL32(32, 64, 105, 250)
                   : (hasTeams ? kTeamStyles[activeTeamIndices[col]].headerCol
                               : IM_COL32(36, 68, 102, 245));
    dl->AddRectFilled(h0, h1, bannerColor, 2.0f * s);

    // Subtle bright top accent line
    dl->AddLine(ImVec2(h0.x, h0.y), ImVec2(h1.x, h0.y),
                IM_COL32(255, 255, 255, 90), 1.0f * s);

    // Left rank / placement badge on banner
    const int teamRank =
        isCoopMode ? 1
                   : ((col == 0) ? (teamScores[0] >= teamScores[1] ? 1 : 2)
                                 : (teamScores[1] > teamScores[0] ? 1 : 2));
    char rankStr[8];
    std::snprintf(rankStr, sizeof(rankStr), "%d", teamRank);
    const float rankColW = 28.0f * s;
    Text(dl, NobleStyle::TitleFont(), 20.0f * s,
         ImVec2(cx + 8.0f * s, p0.y + 7.0f * s), IM_COL32(255, 255, 255, 255),
         rankStr);

    // Team Name
    const char *teamTitle =
        isCoopMode ? "UNSC ALLIES"
                   : (hasTeams ? kTeamStyles[activeTeamIndices[col]].name
                               : "MATCH ROSTER");
    Text(dl, NobleStyle::TitleFont(), 20.0f * s,
         ImVec2(cx + rankColW + 6.0f * s, p0.y + 7.0f * s),
         IM_COL32(255, 255, 255, 255), teamTitle);

    // Column positions for stats on the right (matching headers & rows exactly)
    const float pingColW = 48.0f * s;
    const float aColW = 34.0f * s;
    const float dColW = 34.0f * s;
    const float kColW = 34.0f * s;
    const float scoreColW = 46.0f * s;

    const float pingX = cx + colW - pingColW - 4.0f * s;
    const float aX = pingX - aColW;
    const float dX = aX - dColW;
    const float kX = dX - kColW;
    const float scoreX = kX - scoreColW;

    // Header aggregate totals on the banner
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", teamScores[col]);
    Text(dl, NobleStyle::TitleFont(), 20.0f * s,
         ImVec2(scoreX + (scoreColW -
                          TextWidth(NobleStyle::TitleFont(), 20.0f * s, buf)) *
                             0.5f,
                p0.y + 7.0f * s),
         IM_COL32(255, 255, 255, 255), buf);

    std::snprintf(buf, sizeof(buf), "%d", teamKills[col]);
    Text(dl, NobleStyle::TitleFont(), 19.0f * s,
         ImVec2(
             kX + (kColW - TextWidth(NobleStyle::TitleFont(), 19.0f * s, buf)) *
                      0.5f,
             p0.y + 8.0f * s),
         IM_COL32(255, 255, 255, 235), buf);

    std::snprintf(buf, sizeof(buf), "%d", teamDeaths[col]);
    Text(dl, NobleStyle::TitleFont(), 19.0f * s,
         ImVec2(
             dX + (dColW - TextWidth(NobleStyle::TitleFont(), 19.0f * s, buf)) *
                      0.5f,
             p0.y + 8.0f * s),
         IM_COL32(255, 255, 255, 235), buf);

    std::snprintf(buf, sizeof(buf), "%d", teamAssists[col]);
    Text(dl, NobleStyle::TitleFont(), 19.0f * s,
         ImVec2(
             aX + (aColW - TextWidth(NobleStyle::TitleFont(), 19.0f * s, buf)) *
                      0.5f,
             p0.y + 8.0f * s),
         IM_COL32(255, 255, 255, 235), buf);

    // 2. Sub-header Labels Row ("PLAYERS", "SCORE", "K", "D", "A", "PING")
    const float subY = p0.y + headerH;
    dl->AddRectFilled(ImVec2(cx, subY), ImVec2(cx + colW, subY + subHeaderH),
                      IM_COL32(8, 12, 18, 220));

    Text(dl, NobleStyle::SmallFont(), 13.0f * s,
         ImVec2(cx + 8.0f * s, subY + 3.0f * s), IM_COL32(185, 205, 225, 255),
         "PLAYERS");
    Text(dl, NobleStyle::SmallFont(), 13.0f * s,
         ImVec2(scoreX + (scoreColW - TextWidth(NobleStyle::SmallFont(),
                                                13.0f * s, "SCORE")) *
                             0.5f,
                subY + 3.0f * s),
         IM_COL32(185, 205, 225, 255), "SCORE");
    Text(dl, NobleStyle::SmallFont(), 13.0f * s,
         ImVec2(
             kX + (kColW - TextWidth(NobleStyle::SmallFont(), 13.0f * s, "K")) *
                      0.5f,
             subY + 3.0f * s),
         IM_COL32(185, 205, 225, 255), "K");
    Text(dl, NobleStyle::SmallFont(), 13.0f * s,
         ImVec2(
             dX + (dColW - TextWidth(NobleStyle::SmallFont(), 13.0f * s, "D")) *
                      0.5f,
             subY + 3.0f * s),
         IM_COL32(185, 205, 225, 255), "D");
    Text(dl, NobleStyle::SmallFont(), 13.0f * s,
         ImVec2(
             aX + (aColW - TextWidth(NobleStyle::SmallFont(), 13.0f * s, "A")) *
                      0.5f,
             subY + 3.0f * s),
         IM_COL32(185, 205, 225, 255), "A");
    Text(dl, NobleStyle::SmallFont(), 13.0f * s,
         ImVec2(pingX + (pingColW - TextWidth(NobleStyle::SmallFont(),
                                              13.0f * s, "PING")) *
                            0.5f,
                subY + 3.0f * s),
         IM_COL32(185, 205, 225, 255), "PING");

    // 3. Player Rows
    const float listY = subY + subHeaderH;
    int localRow = -1;
    for (int k = 0; k < count; ++k) {
      if (rows[list[k]].local)
        localRow = k;
    }
    const bool pinLocal = localRow >= 0 && (localRow < scroll ||
                                            localRow >= scroll + visibleRows);

    for (int v = 0; v < visibleRows; ++v) {
      const int rowIdx =
          (pinLocal && v == visibleRows - 1) ? localRow : (scroll + v);
      const float y = listY + v * rowH;
      const ImVec2 r0(cx, y);
      const ImVec2 r1(cx + colW - (overflow ? 4.0f * s : 0.0f), y + rowH);

      if (rowIdx >= count) {
        // Empty filler row with subtle background
        dl->AddRectFilled(r0, r1,
                          (v % 2 == 0) ? IM_COL32(15, 20, 28, 120)
                                       : IM_COL32(12, 16, 22, 120));
        continue;
      }

      const auto &r = rows[list[rowIdx]];
      const bool isMuted =
          s_muteAllActive || r.muted || s_mutedNames.count(r.name) > 0 ||
          (r.machine != 0 && VoiceChat::IsPeerMuted(r.machine));
      bool hovered = false;

      if (pointer) {
        ImGui::SetCursorScreenPos(r0);
        ImGui::BeginDisabled(s_boardReport.load() || s_showPlayerModal);
        if (ImGui::InvisibleButton(r.name.c_str(),
                                   ImVec2(r1.x - r0.x, r1.y - r0.y))) {
          s_selected = r.name;
          s_status.clear();
          selectedIsListed = true;
          s_showPlayerModal = true;
          MenuSounds::Click();
        }
        hovered = ImGui::IsItemHovered();
        ImGui::EndDisabled();
      }

      const bool selected = (r.name == s_selected);
      const ImU32 baseBg =
          isCoopMode
              ? ((v % 2 == 0) ? IM_COL32(24, 42, 65, 160)
                              : IM_COL32(20, 36, 56, 160))
              : (hasTeams
                     ? ((v % 2 == 0)
                            ? kTeamStyles[activeTeamIndices[col]].rowBaseCol
                            : kTeamStyles[activeTeamIndices[col]].rowAltCol)
                     : ((v % 2 == 0) ? IM_COL32(24, 38, 54, 150)
                                     : IM_COL32(20, 32, 46, 150)));
      const ImU32 rowBg = r.local    ? IM_COL32(32, 80, 120, 220)
                          : selected ? IM_COL32(65, 115, 165, 190)
                          : hovered  ? IM_COL32(255, 255, 255, 38)
                                     : baseBg;

      dl->AddRectFilled(r0, r1, rowBg, 4.0f * s);

      // Selected or local highlight borders
      if (selected) {
        dl->AddRect(r0, r1, IM_COL32(143, 216, 255, 255), 4.0f * s, 0, 1.5f * s);
      } else if (r.local) {
        dl->AddRect(r0, r1, IM_COL32(80, 180, 235, 180), 4.0f * s, 0, 1.0f * s);
      }

      // 1. Placement Rank Number
      char pRank[8];
      std::snprintf(pRank, sizeof(pRank), "%d", rowIdx + 1);
      Text(dl, NobleStyle::TitleFont(), 17.0f * s,
           ImVec2(cx + 6.0f * s, y + 7.0f * s), IM_COL32(240, 244, 250, 255),
           pRank);

      // 2. Emblem or Mute Icon
      const float iconSize = 22.0f * s;
      const ImVec2 iconPos(cx + rankColW + 2.0f * s,
                           y + (rowH - iconSize) * 0.5f);

      if (isMuted) {
        // Red crossed box for muted players (matches Dinoextremo29 & gzhouse in
        // reference)
        dl->AddRectFilled(iconPos,
                          ImVec2(iconPos.x + iconSize, iconPos.y + iconSize),
                          IM_COL32(195, 38, 38, 240), 2.0f * s);
        dl->AddRect(iconPos, ImVec2(iconPos.x + iconSize, iconPos.y + iconSize),
                    IM_COL32(255, 180, 180, 200), 2.0f * s, 0, 1.0f * s);
        dl->AddLine(ImVec2(iconPos.x + 4.0f * s, iconPos.y + 4.0f * s),
                    ImVec2(iconPos.x + iconSize - 4.0f * s,
                           iconPos.y + iconSize - 4.0f * s),
                    IM_COL32(255, 255, 255, 255), 2.0f * s);
        dl->AddLine(
            ImVec2(iconPos.x + iconSize - 4.0f * s, iconPos.y + 4.0f * s),
            ImVec2(iconPos.x + 4.0f * s, iconPos.y + iconSize - 4.0f * s),
            IM_COL32(255, 255, 255, 255), 2.0f * s);
      } else {
        // Draw player emblem symbol
        UiArt::EmblemColors embCol;
        embCol.primary = IM_COL32(240, 240, 240, 255);
        embCol.secondary = hasTeams ? (col == 0 ? IM_COL32(255, 160, 160, 255)
                                                : IM_COL32(160, 200, 255, 255))
                                    : IM_COL32(180, 220, 240, 255);
        UiArt::DrawEmblem(dl, r.emblem > 0 ? r.emblem : 1, 0, iconPos,
                          ImVec2(iconPos.x + iconSize, iconPos.y + iconSize),
                          embCol);
      }

      // 3. Player Gamertag and Service Tag
      const float nameX = iconPos.x + iconSize + 8.0f * s;
      const float nameY = r.tag.empty() ? (y + 7.0f * s) : (y + 3.0f * s);
      const ImU32 nameCol =
          r.local ? IM_COL32(143, 225, 255, 255) : IM_COL32(245, 248, 252, 255);

      dl->PushClipRect(ImVec2(nameX, y), ImVec2(scoreX - 34.0f * s, y + rowH),
                       true);
      Text(dl, NobleStyle::TitleFont(), 16.5f * s, ImVec2(nameX, nameY),
           nameCol, r.name.c_str());

      if (r.local) {
        const float nw =
            TextWidth(NobleStyle::TitleFont(), 16.5f * s, r.name.c_str());
        Text(dl, NobleStyle::SmallFont(), 11.0f * s,
             ImVec2(nameX + nw + 6.0f * s, nameY + 3.0f * s),
             IM_COL32(143, 225, 255, 230), "YOU");
      }

      if (!r.tag.empty()) {
        Text(dl, NobleStyle::SmallFont(), 11.5f * s,
             ImVec2(nameX, y + 18.0f * s), IM_COL32(185, 205, 225, 210),
             r.tag.c_str());
      }
      dl->PopClipRect();

      // 4. Rank Insignia Icon (placed just before the SCORE column)
      const UiTexture rankIcon = UiArt::RankIcon(r.rank);
      const float rankIconSize = 22.0f * s;
      const float rankX = scoreX - rankIconSize - 8.0f * s;
      if (rankIcon) {
        UiArt::DrawContain(
            dl, rankIcon, ImVec2(rankX, y + (rowH - rankIconSize) * 0.5f),
            ImVec2(rankX + rankIconSize, y + (rowH + rankIconSize) * 0.5f));
      }

      // 5. Stats: Score, K, D, A
      std::snprintf(buf, sizeof(buf), "%d", r.score);
      Text(dl, NobleStyle::TitleFont(), 17.5f * s,
           ImVec2(scoreX + (scoreColW - TextWidth(NobleStyle::TitleFont(),
                                                  17.5f * s, buf)) *
                               0.5f,
                  y + 7.0f * s),
           IM_COL32(255, 255, 255, 255), buf);

      std::snprintf(buf, sizeof(buf), "%d", r.kills);
      Text(dl, ImGui::GetFont(), 15.5f * s,
           ImVec2(kX + (kColW - TextWidth(ImGui::GetFont(), 15.5f * s, buf)) *
                           0.5f,
                  y + 8.0f * s),
           IM_COL32(235, 240, 245, 240), buf);

      std::snprintf(buf, sizeof(buf), "%d", r.deaths);
      Text(dl, ImGui::GetFont(), 15.5f * s,
           ImVec2(dX + (dColW - TextWidth(ImGui::GetFont(), 15.5f * s, buf)) *
                           0.5f,
                  y + 8.0f * s),
           IM_COL32(195, 205, 215, 210), buf);

      std::snprintf(buf, sizeof(buf), "%d", r.assists);
      Text(dl, ImGui::GetFont(), 15.5f * s,
           ImVec2(aX + (aColW - TextWidth(ImGui::GetFont(), 15.5f * s, buf)) *
                           0.5f,
                  y + 8.0f * s),
           IM_COL32(195, 205, 215, 210), buf);

      // 6. Ping Badge (Pill with rounded corners). Unknown stays -1 and is drawn as "-".
      int ping = r.ping;
      if (!r.local && NetworkTransport::Get().IsActive()) {
        const int live =
            r.hostPeer
                ? (NetworkTransport::Get().IsHost() ? 0
                                                    : NetworkTransport::Get().GetPeerPingMs(0))
                : (r.machine ? NetworkTransport::Get().GetPeerPingMs(r.machine)
                             : -1);
        if (live >= 0)
          ping = live;
      }
      if (ping < -1)
        ping = -1;

      const ImU32 pingBadgeCol =
          ping < 0 ? IM_COL32(90, 105, 120, 180)
                   : (ping <= 100 ? IM_COL32(36, 175, 75, 230)
                                  : (ping <= 150 ? IM_COL32(215, 175, 35, 230)
                                                 : IM_COL32(215, 65, 50, 230)));

      const float pillW = 38.0f * s;
      const float pillH = 20.0f * s;
      const ImVec2 pill0(pingX + (pingColW - pillW) * 0.5f,
                         y + (rowH - pillH) * 0.5f);
      const ImVec2 pill1(pill0.x + pillW, pill0.y + pillH);

      dl->AddRectFilled(pill0, pill1, pingBadgeCol, 3.0f * s);
      const std::string pingText = (ping >= 0) ? std::to_string(ping) : "-";
      Text(dl, NobleStyle::SmallFont(), 12.0f * s,
           ImVec2(pill0.x + (pillW - TextWidth(NobleStyle::SmallFont(),
                                               12.0f * s, pingText.c_str())) *
                                0.5f,
                  pill0.y + 2.0f * s),
           IM_COL32(255, 255, 255, 255), pingText.c_str());
    }

    // Scrollbar track & thumb if list overflows
    if (overflow) {
      const float trackX = cx + colW - 4.0f * s;
      const float trackH = totalListH;
      const float maxScrollPx = static_cast<float>(count - visibleRows) * rowH;
      static float s_boardScrollPx[2] = {0.0f, 0.0f};
      static int s_lastBoardScroll[2] = {0, 0};
      const int safeCol = std::clamp(col, 0, 1);
      if (s_boardScroll[safeCol] != s_lastBoardScroll[safeCol]) {
        s_boardScrollPx[safeCol] =
            static_cast<float>(s_boardScroll[safeCol]) * rowH;
        s_lastBoardScroll[safeCol] = s_boardScroll[safeCol];
      }
      s_boardScrollPx[safeCol] =
          std::clamp(s_boardScrollPx[safeCol], 0.0f, maxScrollPx);
      if (pointer && !s_boardReport.load() && !s_showPlayerModal) {
        NobleUi::DrawInteractiveScrollbar(
            dl, safeCol == 0 ? "##sb_0" : "##sb_1",
            ImVec2(trackX, listY), ImVec2(trackX + 4.0f * s, listY + trackH),
            s_boardScrollPx[safeCol], maxScrollPx, totalListH, count * rowH, s);
        s_boardScroll[safeCol] = std::clamp(
            static_cast<int>(
                std::round(s_boardScrollPx[safeCol] / std::max(rowH, 1.0f))),
            0, std::max(0, count - visibleRows));
        s_lastBoardScroll[safeCol] = s_boardScroll[safeCol];
      } else {
        dl->AddRectFilled(ImVec2(trackX, listY),
                          ImVec2(trackX + 4.0f * s, listY + trackH),
                          IM_COL32(255, 255, 255, 24), 2.0f * s);
        const float thumbH = std::max(trackH * visibleRows / count, 18.0f * s);
        const float thumbY =
            listY + (trackH - thumbH) * scroll / std::max(1, count - visibleRows);
        dl->AddRectFilled(ImVec2(trackX, thumbY),
                          ImVec2(trackX + 4.0f * s, thumbY + thumbH), kAccent,
                          2.0f * s);
      }
    }
  }

  // 4. Footer Bar: Button Prompts (Left) and Match Watermark / Score Pills
  // (Right)
  const float footY = p0.y + headerH + subHeaderH + totalListH + 8.0f * s;

  // Prompts on the left
  const auto drawPrompt = [&](float &fx, const char *key, const char *label) {
    const float kw =
        TextWidth(NobleStyle::SmallFont(), 13.0f * s, key) + 12.0f * s;
    const float kh = 22.0f * s;
    dl->AddRectFilled(ImVec2(fx, footY + 2.0f * s),
                      ImVec2(fx + kw, footY + 2.0f * s + kh),
                      IM_COL32(255, 255, 255, 240), 3.0f * s);
    Text(dl, NobleStyle::SmallFont(), 13.0f * s,
         ImVec2(fx + 6.0f * s, footY + 4.0f * s), IM_COL32(12, 18, 24, 255),
         key);
    fx += kw + 6.0f * s;
    Text(dl, NobleStyle::SmallFont(), 14.0f * s, ImVec2(fx, footY + 5.0f * s),
         IM_COL32(230, 238, 245, 255), label);
    fx += TextWidth(NobleStyle::SmallFont(), 14.0f * s, label) + 20.0f * s;
  };

  float fx = p0.x + 8.0f * s;
  drawPrompt(fx, "A", "Player options");
  drawPrompt(fx, "Scroll", "Scroll");
  drawPrompt(fx, "Tab", "Close");

  // Watermark and mini score pills on the right (matches reference image)
  std::string watermark = mapName.empty() ? "Global Battles v40" : mapName;
  const float wmWidth =
      TextWidth(NobleStyle::TitleFont(), 18.0f * s, watermark.c_str());
  const float wmX = p1.x - wmWidth - 70.0f * s;
  Text(dl, NobleStyle::TitleFont(), 18.0f * s, ImVec2(wmX, footY + 2.0f * s),
       IM_COL32(200, 220, 240, 240), watermark.c_str());

  // Team score square pills (or co-op total score pill)
  if (isCoopMode) {
    char buf[32];
    const float pillBoxW = 48.0f * s;
    const float pillBoxH = 18.0f * s;
    const float pillBoxX = p1.x - 78.0f * s;
    dl->AddRectFilled(ImVec2(pillBoxX, footY),
                      ImVec2(pillBoxX + pillBoxW, footY + pillBoxH),
                      IM_COL32(32, 64, 105, 250), 2.0f * s);
    std::snprintf(buf, sizeof(buf), "%d", teamScores[0]);
    Text(dl, NobleStyle::TitleFont(), 15.0f * s,
         ImVec2(pillBoxX + (pillBoxW - TextWidth(NobleStyle::TitleFont(),
                                                 15.0f * s, buf)) *
                               0.5f,
                footY + 1.0f * s),
         IM_COL32(255, 255, 255, 255), buf);
  } else if (hasTeams) {
    char buf[32];
    const float pillBoxW = 26.0f * s;
    const float pillBoxH = 18.0f * s;
    const float pillBoxX = p1.x - 56.0f * s;

    // Team 1 pill
    dl->AddRectFilled(ImVec2(pillBoxX, footY),
                      ImVec2(pillBoxX + pillBoxW, footY + pillBoxH),
                      kTeamStyles[activeTeamIndices[0]].headerCol, 2.0f * s);
    std::snprintf(buf, sizeof(buf), "%d", teamScores[0]);
    Text(dl, NobleStyle::TitleFont(), 15.0f * s,
         ImVec2(pillBoxX + (pillBoxW - TextWidth(NobleStyle::TitleFont(),
                                                 15.0f * s, buf)) *
                               0.5f,
                footY + 1.0f * s),
         IM_COL32(255, 255, 255, 255), buf);

    // Team 2 pill (only when the match really has a second team)
    if (columnCount > 1) {
      dl->AddRectFilled(
          ImVec2(pillBoxX, footY + pillBoxH + 3.0f * s),
          ImVec2(pillBoxX + pillBoxW, footY + pillBoxH * 2.0f + 3.0f * s),
          kTeamStyles[activeTeamIndices[1]].headerCol, 2.0f * s);
      std::snprintf(buf, sizeof(buf), "%d", teamScores[1]);
      Text(dl, NobleStyle::TitleFont(), 15.0f * s,
           ImVec2(pillBoxX + (pillBoxW - TextWidth(NobleStyle::TitleFont(),
                                                   15.0f * s, buf)) *
                                 0.5f,
                  footY + pillBoxH + 4.0f * s),
           IM_COL32(255, 255, 255, 255), buf);
    }
  }

  if (pointer) {
    ImGui::End();

    // 5. Player Options Modal (Opens when user clicks or presses [A] on a
    // player)
    if (s_showPlayerModal && !s_selected.empty() && !s_boardReport.load()) {
      CenteredWindow(500.0f * s);
      ImGui::Begin("##PlayerOptionsModal", nullptr,
                   kPanelFlags | ImGuiWindowFlags_NoNavInputs |
                       ImGuiWindowFlags_NoNavFocus);

      // Find the selected player's entry
      const PauseMenu::ScoreboardEntry *selEntry = nullptr;
      for (const auto &r : rows) {
        if (r.name == s_selected) {
          selEntry = &r;
          break;
        }
      }

      std::string headerTitle = "PLAYER: " + Upper(s_selected);
      if (selEntry && !selEntry->tag.empty())
        headerTitle += " [" + selEntry->tag + "]";
      NobleStyle::Header(headerTitle.c_str());
      ImGui::Spacing();

      const bool isIndividualMuted =
          s_mutedNames.count(s_selected) > 0 ||
          (selEntry && selEntry->machine != 0 &&
           VoiceChat::IsPeerMuted(selEntry->machine));

      // Stats line
      if (selEntry) {
        ImGui::TextColored(
            NobleStyle::Accent(),
            "Score: %d   |   Kills: %d   |   Deaths: %d   |   Assists: %d",
            selEntry->score, selEntry->kills, selEntry->deaths,
            selEntry->assists);
        ImGui::Spacing();
      }

      const float btnW = (ImGui::GetContentRegionAvail().x - 10.0f * s) * 0.5f;

      // Individual Mute Toggle
      const char *muteLabel =
          isIndividualMuted ? "UNMUTE PLAYER" : "MUTE PLAYER";
      if (ImGui::Button(muteLabel, ImVec2(btnW, 38.0f * s)) ||
          ImGui::IsKeyPressed(ImGuiKey_GamepadFaceUp, false)) {
        if (isIndividualMuted) {
          s_mutedNames.erase(s_selected);
          if (selEntry && selEntry->machine != 0)
            VoiceChat::SetPeerMuted(selEntry->machine, false);
          s_status = "Unmuted " + s_selected;
        } else {
          s_mutedNames.insert(s_selected);
          if (selEntry && selEntry->machine != 0)
            VoiceChat::SetPeerMuted(selEntry->machine, true);
          s_status = "Muted " + s_selected;
        }
        MenuSounds::Click();
      }

      ImGui::SameLine(0.0f, 10.0f * s);

      // Global Mute Toggle
      const char *muteAllLabel =
          s_muteAllActive ? "UNMUTE ALL PLAYERS" : "MUTE ALL PLAYERS";
      if (ImGui::Button(muteAllLabel, ImVec2(btnW, 38.0f * s)) ||
          ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false)) {
        s_muteAllActive = !s_muteAllActive;
        for (const auto &r : rows) {
          if (!r.local && r.machine != 0)
            VoiceChat::SetPeerMuted(r.machine, s_muteAllActive);
        }
        s_status =
            s_muteAllActive ? "Muted all players" : "Unmuted all players";
        MenuSounds::Click();
      }

      ImGui::Spacing();

      // Report & Kick buttons
      if (NobleStyle::PrimaryButton("REPORT PLAYER", ImVec2(btnW, 38.0f * s)) ||
          ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft, false)) {
        s_reportReason = 0;
        s_boardReport.store(true);
        s_showPlayerModal = false;
        MenuSounds::Click();
      }

      ImGui::SameLine(0.0f, 10.0f * s);

      if (isHost && selEntry && !selEntry->local) {
        if (NobleStyle::DangerButton("KICK PLAYER", ImVec2(btnW, 38.0f * s))) {
          s_status = "Kicked " + s_selected;
          s_showPlayerModal = false;
          MenuSounds::Click();
        }
      } else {
        ImGui::BeginDisabled(true);
        ImGui::Button("KICK PLAYER", ImVec2(btnW, 38.0f * s));
        ImGui::EndDisabled();
      }

      ImGui::Spacing();

      if (ImGui::Button("CLOSE", ImVec2(-1, 34.0f * s)) ||
          ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false) ||
          ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        s_showPlayerModal = false;
        MenuSounds::Back();
      }

      ImGui::End();
    }

    // Report Dialog
    if (s_boardReport.load()) {
      static const char *const kReasons[4] = {
          "Cheating", "Abusive name or chat", "Griefing or team killing",
          "Something else"};
      ImGui::SetNextWindowFocus();
      CenteredWindow(460.0f * s);
      ImGui::Begin("##scoreboardreport", nullptr,
                   kPanelFlags | ImGuiWindowFlags_NoNavInputs |
                       ImGuiWindowFlags_NoNavFocus);
      if (UiPad::Down() || ImGui::IsKeyPressed(ImGuiKey_DownArrow, true))
        s_reportReason = (s_reportReason + 1) % 4;
      if (UiPad::Up() || ImGui::IsKeyPressed(ImGuiKey_UpArrow, true))
        s_reportReason = (s_reportReason + 3) % 4;
      NobleStyle::Header(("REPORT " + Upper(s_selected)).c_str());
      ImGui::Spacing();
      for (int i = 0; i < 4; ++i)
        ImGui::RadioButton(kReasons[i], &s_reportReason, i);
      ImGui::Spacing();
      ImGui::TextWrapped("This goes to the Project Noble team, with the map, "
                         "the mode and your name.");
      ImGui::Spacing();
      const float gap = 8.0f * s;
      const float half = (ImGui::GetContentRegionAvail().x - gap) * 0.5f;
      static int s_demoSend = 0;
      const bool demoSend =
          demo == 5 && !s_reportSending.load() && ++s_demoSend == 240;
      if (NobleStyle::PrimaryButton("SEND REPORT", ImVec2(half, 38.0f * s)) ||
          demoSend || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false)) {
        s_reportSending.store(true);
        std::string session;
        {
          std::lock_guard<std::mutex> lock(s_boardMutex);
          session = s_boardSession;
        }
        if (session.empty())
          session = MasterServerClient::Get().GetCurrentSessionId();
        std::thread([session, target = s_selected,
                     reason = std::string(kReasons[s_reportReason]), mapName,
                     modeName] {
          const std::string reporter = Settings::Get().Snapshot().playerName;
          const bool sent = MasterServerClient::Get().SendReportSync(
              session, reporter, target, reason, mapName, modeName);
          const bool saved =
              sent || SaveReport(target, reason.c_str(), mapName, modeName);
          NOBLE_INFO("PauseMenu: report sent={} savedLocally={}", sent,
                     !sent && saved);
          s_reportResult.store(sent ? 1 : saved ? 2 : 3);
          s_reportSending.store(false);
        }).detach();
        s_status = "Sending report";
        s_boardReport.store(false);
        MenuSounds::Click();
      }
      ImGui::SameLine(0.0f, gap);
      if (ImGui::Button("CANCEL", ImVec2(half, 38.0f * s)) ||
          ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false) ||
          ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        s_boardReport.store(false);
        MenuSounds::Back();
      }
      ImGui::End();
    }
  }
}

// Guest-side connection-lost panel: the host vanished (no packets for a while)
// but the engine keeps pumping frames, so without this the guest just sits
// frozen in the mission. Offers the only reliable exit - relaunch the menu
// process.
static bool DrawConnectionLostPanel(HWND hwnd) {
  using namespace NobleUi;
  ImGuiIO &io = ImGui::GetIO();
  const float W = io.DisplaySize.x, H = io.DisplaySize.y;
  const float s = std::clamp(H / 1080.0f, 0.7f, 2.0f) * Settings::UiScale();
  ImDrawList *dl = ImGui::GetBackgroundDrawList();
  dl->AddRectFilled(ImVec2(0, 0), ImVec2(W, H), IM_COL32(4, 6, 10, 210));
  const float panelW = std::min(W * 0.5f, 720.0f * s), panelH = 260.0f * s;
  const ImVec2 p0((W - panelW) * 0.5f, (H - panelH) * 0.5f),
      p1(p0.x + panelW, p0.y + panelH);
  dl->AddRectFilled(p0, p1, IM_COL32(10, 16, 26, 245), 8.0f * s);
  dl->AddRect(p0, p1, IM_COL32(70, 120, 165, 220), 8.0f * s, 0, 2.0f * s);
  dl->AddRectFilled(ImVec2(p0.x, p0.y), ImVec2(p0.x + 6.0f * s, p1.y), kAccent,
                    8.0f * s);
  Text(dl, NobleStyle::TitleFont(), 34.0f * s,
       ImVec2(p0.x + 36.0f * s, p0.y + 30.0f * s), IM_COL32(236, 240, 244, 255),
       "CONNECTION LOST");
  Text(dl, ImGui::GetFont(), 20.0f * s,
       ImVec2(p0.x + 38.0f * s, p0.y + 84.0f * s), IM_COL32(200, 210, 220, 255),
       "The host connection was lost.");
  Text(dl, NobleStyle::SmallFont(), 15.0f * s,
       ImVec2(p0.x + 38.0f * s, p0.y + 116.0f * s),
       IM_COL32(150, 190, 220, 255),
       "Holding the match open for host migration. A / Enter or click returns "
       "to the menu.");

  ImGui::SetNextWindowPos(
      ImVec2(p0.x + panelW * 0.5f - 120.0f * s, p0.y + panelH - 78.0f * s));
  ImGui::SetNextWindowSize(ImVec2(240.0f * s, 52.0f * s));
  ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
  ImGui::Begin("##NobleConnectionLost", nullptr,
               ImGuiWindowFlags_NoDecoration |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBackground |
                   ImGuiWindowFlags_NoNavInputs |
                   ImGuiWindowFlags_NoNavFocus);
  const bool activate =
      ImGui::Button("Return to menu", ImVec2(240.0f * s, 44.0f * s)) ||
      ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
      ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false);
  if (activate) {
    ImGui::End();
    ImGui::PopStyleColor();
    RelaunchAndClose(hwnd, L"", "connection lost - returning to the menu");
    return true;
  }
  ImGui::End();
  ImGui::PopStyleColor();
  return false;
}

void PauseMenu::DrawGameOver(HWND hwnd) {
  using namespace NobleUi;
  ImGuiIO &io = ImGui::GetIO();
  const float W = io.DisplaySize.x, H = io.DisplaySize.y;
  const float s = std::clamp(H / 1080.0f, 0.7f, 2.0f) * Settings::UiScale();
  const Career::Award a = Career::LastAward();
  const auto &ranks = Career::Ranks();

  std::string mapName;
  {
    std::lock_guard<std::mutex> lock(s_mapMutex);
    mapName = s_loadingMapName;
  }

  // Reveal clock (restarts for every new results screen).
  const ULONGLONG start = s_gameOverAt.load();
  if (s_reveal.start != start) {
    s_reveal = Reveal{};
    s_reveal.start = start;
  }
  const int lineCount = a.valid ? static_cast<int>(a.lines.size()) : 0;
  const float tTotal = kFirstLine + lineCount * kLineStep + 0.2f;
  const float tRank = tTotal + 0.9f;
  const float tEnd = tRank + 1.6f;
  float t = static_cast<float>(GetTickCount64() - start) / 1000.0f;

  // Skip: Enter / Space (fresh press, window in front) or a click away from the
  // buttons.
  bool skipKey = false;
  if (Window::IsForeground(hwnd)) {
    const bool down = (GetAsyncKeyState(VK_RETURN) & 0x8000) ||
                      (GetAsyncKeyState(VK_SPACE) & 0x8000);
    skipKey = down && !s_reveal.keyWasDown;
    s_reveal.keyWasDown = down;
  }
  if (skipKey && t < tEnd)
    s_reveal.skipped = true;
  if (s_reveal.skipped)
    t = tEnd + 1.0f;

  if (!s_reveal.completionSounded) {
    s_reveal.completionSounded = true;
    if (!s_reveal.skipped)
      Sfx(ReachAudio::Sfx::GameCompletion);
  }

  ImDrawList *bg = ImGui::GetBackgroundDrawList();
  bg->AddRectFilledMultiColor(
      ImVec2(0, 0), ImVec2(W, H), IM_COL32(6, 12, 20, 235),
      IM_COL32(6, 12, 20, 235), IM_COL32(2, 4, 8, 245), IM_COL32(2, 4, 8, 245));

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(W, H));
  const ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground |
      ImGuiWindowFlags_NoBringToFrontOnFocus |
      ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus;
  if (!ImGui::Begin("##NobleGameOver", nullptr, flags)) {
    ImGui::End();
    return;
  }
  ImDrawList *dl = ImGui::GetWindowDrawList();
  ImFont *hero = NobleStyle::HeroFont();
  ImFont *title = NobleStyle::TitleFont();
  ImFont *smallFont = NobleStyle::SmallFont();
  ImFont *body = ImGui::GetFont();
  const float margin = 64.0f * s;

  // ---- Header: GAME OVER, map, mode, time, difficulty.
  {
    const float d = 15.0f * s;
    const ImVec2 c(margin + d, 70.0f * s);
    dl->AddQuad(ImVec2(c.x, c.y - d), ImVec2(c.x + d, c.y),
                ImVec2(c.x, c.y + d), ImVec2(c.x - d, c.y), kAccent, 2.0f * s);
    dl->AddQuad(ImVec2(c.x, c.y - d * 0.45f), ImVec2(c.x + d * 0.45f, c.y),
                ImVec2(c.x, c.y + d * 0.45f), ImVec2(c.x - d * 0.45f, c.y),
                kAccent, 2.0f * s);
    Text(dl, hero, 56.0f * s, ImVec2(margin + 2.6f * d, 40.0f * s), kText,
         "GAME OVER");
    std::string sub = Upper(mapName.empty() ? a.map : mapName);
    if (a.valid) {
      char extra[96];
      std::snprintf(extra, sizeof(extra), "%s%s  %.0f MIN",
                    sub.empty() ? "" : "  -  ", Upper(a.type).c_str(),
                    a.minutes);
      sub += extra;
    }
    float x = margin + 2.6f * d;
    const float y = 104.0f * s;
    Text(dl, title, 24.0f * s, ImVec2(x, y), kTextDim, sub.c_str());
    x += TextWidth(title, 24.0f * s, sub.c_str()) + 20.0f * s;
    if (a.valid && a.difficulty >= 0) {
      static const char *const kDiff[4] = {"EASY", "NORMAL", "HEROIC",
                                           "LEGENDARY"};
      const int dIdx = std::clamp(a.difficulty, 0, 3);
      const UiTexture icon = UiArt::DifficultyIcon(dIdx);
      if (icon) {
        UiArt::DrawContain(dl, icon, ImVec2(x, y - 2.0f * s),
                           ImVec2(x + 30.0f * s, y + 28.0f * s));
        x += 36.0f * s;
      }
      Text(dl, title, 24.0f * s, ImVec2(x, y), kTextDim, kDiff[dIdx]);
    }
  }

  const float top = 170.0f * s;
  const float bottom = H - 120.0f * s;
  const float split = W * 0.58f;
  const float leftW = split - margin - 24.0f * s;
  const float rightX = split + 24.0f * s, rightW = W - margin - rightX;

  // ---- Left: scoreboard.
  float y = top;
  Heading(dl, ImVec2(margin, y), leftW, s, "SCOREBOARD");
  y += 30.0f * s;
  if (a.valid && !a.players.empty()) {
    const float colW = 74.0f * s;
    const char *const heads[5] = {"SCORE", "K", "D", "A", "MEDALS"};
    const float px = 15.0f * s;
    for (int c = 0; c < 5; ++c) {
      const float cx = margin + leftW - (5 - c) * colW;
      Text(dl, smallFont, px,
           ImVec2(cx + colW - TextWidth(smallFont, px, heads[c]) - 8.0f * s, y),
           kTextDim, heads[c]);
    }
    Text(dl, smallFont, px, ImVec2(margin + 12.0f * s, y), kTextDim, "PLAYER");
    y += 24.0f * s;
    const float rowH = 38.0f * s;
    const float appear = std::clamp(t / 0.4f, 0.0f, 1.0f);
    const int alpha = static_cast<int>(255 * appear);
    int shown = 0;
    for (const Career::ScoreRow &r : a.players) {
      if (y + rowH > top + (bottom - top) * 0.52f)
        break;
      const ImVec2 r0(margin, y), r1(margin + leftW, y + rowH - 4.0f * s);
      dl->AddRectFilled(
          r0, r1,
          r.local ? IM_COL32(40, 80, 115, static_cast<int>(160 * appear))
                  : IM_COL32(255, 255, 255, static_cast<int>(12 * appear)),
          2.0f * s);
      if (r.local)
        dl->AddRectFilled(r0, ImVec2(r0.x + 3.0f * s, r1.y), kAccent);
      float nx = margin + 12.0f * s;
      if (r.local) {
        const UiTexture icon = UiArt::RankIcon(a.rankAfter);
        if (icon) {
          UiArt::DrawContain(dl, icon, ImVec2(nx, y + 3.0f * s),
                             ImVec2(nx + 28.0f * s, y + rowH - 7.0f * s),
                             IM_COL32(255, 255, 255, alpha));
          nx += 36.0f * s;
        }
      }
      const float ty = y + (rowH - 4.0f * s) * 0.5f - 10.0f * s;
      const ImU32 nameCol = r.local ? IM_COL32(143, 216, 255, alpha)
                                    : IM_COL32(236, 240, 244, alpha);
      Text(dl, body, 18.0f * s, ImVec2(nx, ty), nameCol, r.name.c_str());
      const long long vals[5] = {r.score, r.kills, r.deaths, r.assists,
                                 r.medals};
      for (int c = 0; c < 5; ++c) {
        const std::string v = Grouped(vals[c]);
        const float cx = margin + leftW - (5 - c) * colW;
        Text(
            dl, body, 18.0f * s,
            ImVec2(cx + colW - TextWidth(body, 18.0f * s, v.c_str()) - 8.0f * s,
                   ty),
            IM_COL32(236, 240, 244, alpha), v.c_str());
      }
      y += rowH;
      ++shown;
    }
    if (shown < static_cast<int>(a.players.size())) {
      char more[48];
      std::snprintf(more, sizeof(more), "+%d more",
                    static_cast<int>(a.players.size()) - shown);
      Text(dl, smallFont, 15.0f * s, ImVec2(margin + 12.0f * s, y), kTextDim,
           more);
      y += 22.0f * s;
    }
  } else {
    Text(dl, body, 18.0f * s, ImVec2(margin, y), kTextDim,
         "No results from this game.");
    y += 30.0f * s;
  }

  // ---- Left: challenge and commendation progress made in this game.
  y += 18.0f * s;
  Heading(dl, ImVec2(margin, y), leftW, s, "PROGRESS");
  y += 30.0f * s;
  if (!a.valid || a.progress.empty()) {
    Text(dl, body, 18.0f * s, ImVec2(margin, y), kTextDim,
         "No challenge or commendation progress this game.");
  } else {
    static const char *const kKind[3] = {"DAILY", "WEEKLY", "COMMENDATION"};
    const float rowH = 56.0f * s;
    int i = 0;
    for (const Career::ProgressLine &p : a.progress) {
      if (y + rowH > bottom - 8.0f * s) {
        char more[64];
        std::snprintf(more, sizeof(more), "+%d more in the Service Record",
                      static_cast<int>(a.progress.size()) - i);
        Text(dl, smallFont, 15.0f * s, ImVec2(margin, y), kTextDim, more);
        break;
      }
      const float k = Ease((t - (0.6f + i * 0.25f)) / 0.9f);
      const ImVec2 r0(margin, y), r1(margin + leftW, y + rowH - 6.0f * s);
      const int kind = std::clamp(p.kind, 0, 2);
      const bool done = p.completed && k >= 1.0f;
      const ImU32 green = IM_COL32(140, 217, 140, 255);
      dl->AddRectFilled(
          r0, r1, done ? IM_COL32(30, 52, 36, 200) : IM_COL32(22, 30, 40, 200),
          3.0f * s);
      float x = margin + 12.0f * s;
      if (kind == 2 && !p.art.empty()) {
        const UiTexture art = UiArt::ReachArt("commendations", p.art);
        if (art) {
          const float bh = rowH - 16.0f * s, bw = bh * (287.0f / 90.0f);
          UiArt::DrawContain(dl, art, ImVec2(x, y + 5.0f * s),
                             ImVec2(x + bw, y + 5.0f * s + bh));
          x += bw + 12.0f * s;
        }
      }
      const float chipPx = 13.0f * s;
      const float chipW =
          TextWidth(smallFont, chipPx, kKind[kind]) + 10.0f * s;
      dl->AddRect(ImVec2(x, y + 8.0f * s),
                  ImVec2(x + chipW, y + 12.0f * s + chipPx), kTextDim,
                  2.0f * s);
      Text(dl, smallFont, chipPx, ImVec2(x + 5.0f * s, y + 10.0f * s), kTextDim,
           kKind[kind]);
      Text(dl, body, 18.0f * s, ImVec2(x + chipW + 10.0f * s, y + 6.0f * s),
           done ? green : kText, p.title.c_str());
      // Bar from the old value to the new one (old part blue-grey, this game's
      // part amber).
      const float denom = static_cast<float>(std::max<int64_t>(1, p.target));
      const float fBefore =
          std::clamp(static_cast<float>(p.before) / denom, 0.0f, 1.0f);
      const float fAfter =
          std::clamp(static_cast<float>(p.after) / denom, 0.0f, 1.0f);
      const float f = fBefore + (fAfter - fBefore) * k;
      std::string label;
      if (done)
        label = kind == 2
                    ? Upper(Career::CommendationTierName(p.tier)) + " REACHED"
                    : "COMPLETE";
      else
        label = Grouped(static_cast<long long>(p.before +
                                               (p.after - p.before) * k)) +
                " / " + Grouped(p.target);
      const float lw = TextWidth(smallFont, 15.0f * s, label.c_str());
      const float bx0 = x, bx1 = r1.x - lw - 20.0f * s, by = y + 36.0f * s;
      dl->AddRectFilled(ImVec2(bx0, by), ImVec2(bx1, by + 5.0f * s),
                        IM_COL32(255, 255, 255, 26), 2.0f * s);
      dl->AddRectFilled(ImVec2(bx0, by),
                        ImVec2(bx0 + (bx1 - bx0) * fBefore, by + 5.0f * s),
                        IM_COL32(110, 150, 180, 255), 2.0f * s);
      if (f > fBefore)
        dl->AddRectFilled(ImVec2(bx0 + (bx1 - bx0) * fBefore, by),
                          ImVec2(bx0 + (bx1 - bx0) * f, by + 5.0f * s),
                          done ? green : kAmber, 2.0f * s);
      Text(dl, smallFont, 15.0f * s,
           ImVec2(r1.x - lw - 10.0f * s, by - 6.0f * s), done ? green : kText,
           label.c_str());
      y += rowH;
      ++i;
    }
  }

  // ---- Right: credits, counting up line by line.
  float ry = top;
  Heading(dl, ImVec2(rightX, ry), rightW, s, "CREDITS");
  ry += 34.0f * s;
  if (a.valid) {
    for (int i = 0; i < lineCount; ++i) {
      const Career::AwardLine &l = a.lines[i];
      const float lt = kFirstLine + i * kLineStep;
      if (t < lt)
        break;
      const bool slot = IsSlotLine(l);
      if (s_reveal.linesSounded <= i && !s_reveal.skipped) {
        s_reveal.linesSounded = i + 1;
        if (slot)
          Sfx(ReachAudio::Sfx::SlotMachine, kCountSfxVolume);
        else if (l.credits > 0)
          Sfx(ReachAudio::Sfx::Totals, kCountSfxVolume);
      }
      const float k =
          std::clamp((t - lt) / (slot ? kSlotRoll : kLineCount), 0.0f, 1.0f);
      long long shownValue = static_cast<long long>(l.credits * Ease(k));
      if (slot && k < 1.0f) { // slot machine: rolling digits until it lands
        shownValue = (static_cast<long long>(t * 37.0f) % 10) * 1000 +
                     (static_cast<long long>(t * 91.0f) % 10) * 100 +
                     (static_cast<long long>(t * 53.0f) % 10) * 10;
      }
      const int fade =
          static_cast<int>(255 * std::clamp((t - lt) / 0.2f, 0.0f, 1.0f));
      Text(dl, body, 19.0f * s, ImVec2(rightX, ry),
           IM_COL32(160, 172, 184, fade), l.label.c_str());
      const std::string v = "+" + Grouped(shownValue) + " cR";
      Text(dl, body, 19.0f * s,
           ImVec2(rightX + rightW - TextWidth(body, 19.0f * s, v.c_str()), ry),
           slot ? IM_COL32(255, 214, 110, 255) : kAmber, v.c_str());
      ry += 30.0f * s;
      if (ry > top + (bottom - top) * 0.48f && i + 1 < lineCount) {
        int64_t rest = 0;
        for (int j = i + 1; j < lineCount; ++j)
          rest += a.lines[j].credits;
        const std::string more = std::to_string(lineCount - i - 1) +
                                 " more  +" + Grouped(rest) + " cR";
        if (t >= kFirstLine + (i + 1) * kLineStep)
          Text(dl, smallFont, 15.0f * s, ImVec2(rightX, ry), kTextDim,
               more.c_str());
        ry += 24.0f * s;
        break;
      }
    }
    // Total.
    ry += 8.0f * s;
    dl->AddLine(ImVec2(rightX, ry), ImVec2(rightX + rightW, ry), kRule, 1.0f);
    ry += 12.0f * s;
    if (t >= tTotal) {
      if (!s_reveal.totalSounded && !s_reveal.skipped) {
        s_reveal.totalSounded = true;
        Sfx(ReachAudio::Sfx::Totals, kCountSfxVolume);
      }
      const float k = Ease((t - tTotal) / 0.8f);
      Text(dl, title, 26.0f * s, ImVec2(rightX, ry), kAccent, "CREDITS EARNED");
      const std::string v =
          "+" + Grouped(static_cast<long long>(a.total * k)) + " cR";
      Text(dl, title, 30.0f * s,
           ImVec2(rightX + rightW - TextWidth(title, 30.0f * s, v.c_str()),
                  ry - 2.0f * s),
           kAmber, v.c_str());
    }
    ry += 54.0f * s;

    // ---- Right: rank. The bar fills from the old total to the new one.
    Heading(dl, ImVec2(rightX, ry), rightW, s, "RANK");
    ry += 30.0f * s;
    const float k = t < tRank ? 0.0f : Ease((t - tRank) / 1.4f);
    const int64_t shownCredits =
        a.creditsBefore +
        static_cast<int64_t>((a.creditsAfter - a.creditsBefore) * k);
    const int shownRank = std::clamp(Career::RankIndex(shownCredits), 0,
                                     static_cast<int>(ranks.size()) - 1);
    const bool promoted = shownRank > a.rankBefore;
    if (promoted && !s_reveal.rankUpSounded) {
      s_reveal.rankUpSounded = true;
      Sfx(ReachAudio::Sfx::RankUp);
    }
    const float iconSize = 120.0f * s;
    if (promoted) {
      const float pulse =
          0.5f + 0.5f * std::sin(static_cast<float>(ImGui::GetTime()) * 4.0f);
      dl->AddCircleFilled(
          ImVec2(rightX + iconSize * 0.5f, ry + iconSize * 0.5f),
          iconSize * 0.62f,
          IM_COL32(255, 186, 74, static_cast<int>(30 + 40 * pulse)));
    }
    const UiTexture icon = UiArt::RankIcon(shownRank);
    if (icon)
      UiArt::DrawContain(dl, icon, ImVec2(rightX, ry),
                         ImVec2(rightX + iconSize, ry + iconSize));
    else
      dl->AddRect(ImVec2(rightX, ry), ImVec2(rightX + iconSize, ry + iconSize),
                  IM_COL32(255, 255, 255, 50));
    const float tx = rightX + iconSize + 22.0f * s;
    if (promoted)
      Text(dl, title, 22.0f * s, ImVec2(tx, ry + 4.0f * s), kAmber, "PROMOTED");
    Text(dl, title, 30.0f * s, ImVec2(tx, ry + (promoted ? 30.0f : 14.0f) * s),
         kText, ranks[shownRank].name);
    const float barY = ry + iconSize - 34.0f * s;
    if (shownRank + 1 < static_cast<int>(ranks.size())) {
      const int64_t lo = ranks[shownRank].credits,
                    hi = ranks[shownRank + 1].credits;
      const float f =
          std::clamp(static_cast<float>(shownCredits - lo) /
                         static_cast<float>(std::max<int64_t>(1, hi - lo)),
                     0.0f, 1.0f);
      const float bx1 = rightX + rightW;
      dl->AddRectFilled(ImVec2(tx, barY), ImVec2(bx1, barY + 8.0f * s),
                        IM_COL32(255, 255, 255, 26), 3.0f * s);
      dl->AddRectFilled(ImVec2(tx, barY),
                        ImVec2(tx + (bx1 - tx) * f, barY + 8.0f * s), kAccent,
                        3.0f * s);
      const std::string v = Grouped(shownCredits) + " / " + Grouped(hi) + " cR";
      Text(dl, smallFont, 15.0f * s, ImVec2(tx, barY + 14.0f * s), kTextDim,
           v.c_str());
      const std::string next =
          std::string("Next: ") + ranks[shownRank + 1].name;
      Text(dl, smallFont, 15.0f * s,
           ImVec2(bx1 - TextWidth(smallFont, 15.0f * s, next.c_str()),
                  barY + 14.0f * s),
           kTextDim, next.c_str());
    } else {
      Text(dl, smallFont, 15.0f * s, ImVec2(tx, barY + 14.0f * s), kTextDim,
           "Highest rank reached");
    }
  }

  // ---- Buttons (usable at any time) and the skip hint.
  const float bh = 52.0f * s, bw = 260.0f * s, gap = 16.0f * s;
  const float by = H - 88.0f * s;
  float bx = W - margin - bw;
  bool overButton = false;
  // What happens next in a hosted game. "-postgame-role host|guest" shows
  // either one without a second PC.
  static const int s_roleTest = [] {
    const wchar_t *const line = GetCommandLineW();
    return wcsstr(line, L"-postgame-role host")    ? 1
           : wcsstr(line, L"-postgame-role guest") ? 2
                                                   : 0;
  }();
  const bool isCmdGuest =
      (wcsstr(GetCommandLineW(), L"-connect") != nullptr ||
       Relaunch::CurrentGameArgs().find(L" -connect") != std::wstring::npos);
  const bool isCmdHost =
      (wcsstr(GetCommandLineW(), L"-host") != nullptr ||
       Relaunch::CurrentGameArgs().find(L" -host") != std::wstring::npos);
  const bool online = NetworkTransport::Get().IsActive();
  const bool guestRole =
      s_roleTest == 2 ||
      (s_roleTest == 0 &&
       (isCmdGuest || (online && !NetworkTransport::Get().IsHost())));
  const bool hostRole =
      s_roleTest == 1 ||
      (s_roleTest == 0 && !guestRole &&
       (isCmdHost || (online && NetworkTransport::Get().IsHost())));
  const std::wstring again = Relaunch::CurrentGameArgs();
  // A guest's "again" would reconnect to a host who may have gone; only the
  // host or a local player can start another.
  const bool guestPlayer = guestRole;
  const bool canPlayAgain = !again.empty() && !guestPlayer;
  const bool hasPlaylist =
      wcsstr(GetCommandLineW(), L"-lobby-playlist") != nullptr;
  const std::wstring nextMission =
      guestPlayer ? std::wstring() : NextCampaignMission(again);
  const bool canNextMission = !nextMission.empty();
  // Controller / Keyboard: left and right pick a button, A/Enter presses it, and A/Space/Enter during the
  // reveal only skips it. The focus starts away from QUIT, so one stray A can
  // never close the game; a host or ready guest starts on going back to the lobby.
  const int lobbyState = guestRole ? PollHostBackInLobby() : 0;
  const bool canReturnLobby = guestRole && (lobbyState == 1);
  if (s_gameOverWaitRelease) {
    const bool anyInputHeld =
        ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) ||
        ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0) ||
        ((GetAsyncKeyState(VK_RETURN) & 0x8000) != 0) ||
        ((GetAsyncKeyState(VK_SPACE) & 0x8000) != 0) ||
        ImGui::IsKeyDown(ImGuiKey_GamepadFaceDown);
    if (!anyInputHeld) {
      s_gameOverWaitRelease = false;
    }
  }
  const bool interactive =
      !s_gameOverWaitRelease && GetTickCount64() >= s_gameOverSettleUntil;

  enum { kPadQuit, kPadMenu, kPadAgain, kPadNext, kPadLobby };
  std::vector<int> padOrder = {kPadQuit, kPadMenu};
  if (canPlayAgain)
    padOrder.push_back(kPadAgain);
  if ((hostRole && hasPlaylist) || canNextMission)
    padOrder.push_back(kPadNext);
  if (hostRole || canReturnLobby)
    padOrder.push_back(kPadLobby);
  if (s_gameOverPadButton < 0)
    s_gameOverPadButton = (hostRole || canReturnLobby) ? kPadLobby : kPadMenu;
  int padAt = 1;
  for (size_t i = 0; i < padOrder.size(); ++i)
    if (padOrder[i] == s_gameOverPadButton)
      padAt = static_cast<int>(i);
  if (interactive && (UiPad::Left() || ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true))) {
    const int nextAt =
        std::min(padAt + 1, static_cast<int>(padOrder.size()) - 1);
    if (nextAt != padAt) {
      padAt = nextAt;
      MenuSounds::Scroll();
    }
  }
  if (interactive && (UiPad::Right() || ImGui::IsKeyPressed(ImGuiKey_RightArrow, true))) {
    const int nextAt = std::max(padAt - 1, 0);
    if (nextAt != padAt) {
      padAt = nextAt;
      MenuSounds::Scroll();
    }
  }
  s_gameOverPadButton = padOrder[padAt];
  const bool padA = interactive && UiPad::AcceptPressed();
  // A is jump: a player still pressing it as the panel finishes must not leave
  // the screen, so arm shortly after reveal finishes.
  if (t < tEnd)
    s_gameOverPadArmedAt = 0.0;
  else if (s_gameOverPadArmedAt == 0.0)
    s_gameOverPadArmedAt = ImGui::GetTime() + 0.35;
  const bool padPress =
      padA && t >= tEnd && ImGui::GetTime() >= s_gameOverPadArmedAt;
  if (padA && t < tEnd) {
    UiPad::ConsumeAccept();
    s_reveal.skipped = true;
  }
  const bool pointerMoved = NobleUi::IsMouseMoved(2.5f);
  const auto syncButtonFocus = [&](int button) {
    if (ImGui::IsItemHovered()) {
      overButton = true;
      if (interactive && pointerMoved &&
          std::find(padOrder.begin(), padOrder.end(), button) != padOrder.end()) {
        if (s_gameOverPadButton != button) {
          s_gameOverPadButton = button;
          MenuSounds::Hover(ImGui::GetItemID());
        }
      }
    }
    if (s_gameOverPadButton == button) {
      NobleUi::DrawFocusHighlight(ImGui::GetWindowDrawList(),
                                  ImGui::GetItemRectMin(),
                                  ImGui::GetItemRectMax(), s, true);
    }
  };
  const bool buttonInteractive =
      interactive && t >= tEnd && ImGui::GetTime() >= s_gameOverPadArmedAt;

  ImGui::SetCursorScreenPos(ImVec2(bx, by));
  if ((NobleStyle::DangerButton("QUIT", ImVec2(bw, bh)) && buttonInteractive) ||
      (padPress && s_gameOverPadButton == kPadQuit)) {
    QuitFromMatch(hwnd, "quit requested from post-game panel");
  }
  syncButtonFocus(kPadQuit);
  bx -= bw + gap;
  ImGui::SetCursorScreenPos(ImVec2(bx, by));
  // The label says what the button really does: a host that leaves takes the party's session with
  // it (disband), a guest only leaves, and a local match simply goes back to the menu.
  const char *const menuLabel = hostRole ? "DISBAND LOBBY"
                                  : guestRole ? "LEAVE LOBBY"
                                              : "RETURN TO MENU";
  const char *const menuReason = hostRole ? "host disband lobby from the after-action report"
                                  : guestRole ? "guest leave lobby from the after-action report"
                                              : "return to menu";
  if ((ImGui::Button(menuLabel, ImVec2(bw, bh)) && buttonInteractive) ||
      (padPress && s_gameOverPadButton == kPadMenu))
    RelaunchAndClose(hwnd, L"-main-menu", menuReason);
  syncButtonFocus(kPadMenu);
  if (canPlayAgain) {
    bx -= bw + gap;
    ImGui::SetCursorScreenPos(ImVec2(bx, by));
    if ((NobleStyle::PrimaryButton("PLAY AGAIN", ImVec2(bw, bh)) && buttonInteractive) ||
        (padPress && s_gameOverPadButton == kPadAgain))
      RelaunchAndClose(hwnd, again, "play again");
    syncButtonFocus(kPadAgain);
  }
  const auto nextMissionButton = [&] {
    bx -= bw + gap;
    ImGui::SetCursorScreenPos(ImVec2(bx, by));
    if ((NobleStyle::PrimaryButton("NEXT MISSION", ImVec2(bw, bh)) && buttonInteractive) ||
        (padPress && s_gameOverPadButton == kPadNext))
      RelaunchAndClose(hwnd,
                       NextMissionArgs(nextMission, hostRole
                                                        ? LobbyReturnArgs(true)
                                                        : std::wstring()),
                       "next mission");
    syncButtonFocus(kPadNext);
  };
  if (canNextMission && !hostRole)
    nextMissionButton();
  // The host takes everyone back to the lobby: its new process registers the
  // same session again, and guests come back when the server says the match is
  // over. Next in rotation waits for rotation itself.
  if (hostRole) {
    if (canNextMission && !hasPlaylist) {
      nextMissionButton();
    } else {
      bx -= bw + gap;
      ImGui::SetCursorScreenPos(ImVec2(bx, by));
      if (hasPlaylist) {
        if ((ImGui::Button("NEXT IN ROTATION", ImVec2(bw, bh)) && buttonInteractive) ||
            (padPress && s_gameOverPadButton == kPadNext))
          RelaunchAndClose(hwnd, LobbyReturnArgs(true, true),
                           "next in rotation");
        syncButtonFocus(kPadNext);
      } else {
        UnavailableButton("NEXT IN ROTATION", ImVec2(bw, bh),
                          "Pick a playlist on Custom Game to use a rotation.");
        overButton |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
      }
    }
    bx -= bw + gap;
    ImGui::SetCursorScreenPos(ImVec2(bx, by));
    if ((NobleStyle::PrimaryButton("RETURN TO LOBBY", ImVec2(bw, bh)) && buttonInteractive) ||
        (padPress && s_gameOverPadButton == kPadLobby))
      RelaunchAndClose(hwnd, LobbyReturnArgs(true), "return to lobby");
    syncButtonFocus(kPadLobby);
  } else if (guestRole) {
    bx -= bw + gap;
    ImGui::SetCursorScreenPos(ImVec2(bx, by));
    if (canReturnLobby) {
      if ((NobleStyle::PrimaryButton("RETURN TO LOBBY", ImVec2(bw, bh)) && buttonInteractive) ||
          (padPress && s_gameOverPadButton == kPadLobby)) {
        RelaunchAndClose(hwnd, LobbyReturnArgs(false), "guest return to lobby");
      }
      syncButtonFocus(kPadLobby);
    } else {
      const char *guestBtnText =
          (lobbyState == 2 ? "HOST LEFT" : "WAITING FOR HOST");
      const char *guestBtnTip =
          (lobbyState == 2
               ? "The host's lobby closed. Return to the menu to play again."
               : "You can wait for the host or return to the menu at any "
                 "time.");
      UnavailableButton(guestBtnText, ImVec2(bw, bh), guestBtnTip);
      overButton |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
    }
    if (canReturnLobby && t >= 15.0f) {
      if (!s_guestAutoReturned) {
        s_guestAutoReturned = true;
        RelaunchAndClose(hwnd, LobbyReturnArgs(false),
                         "guest auto return to lobby");
      }
    }
  }
  if (hostRole || guestRole) {
    std::string current, display;
    {
      std::lock_guard<std::mutex> lock(s_mapMutex);
      current = s_loadingMap;
      display = s_displayMapName;
    }
    const std::string shownMap = !display.empty() ? Upper(display)
                                 : current.empty()
                                     ? std::string("-")
                                     : Upper(MapNames::Display(current));
    char nextName[64] = {};
    if (canNextMission)
      WideCharToMultiByte(CP_UTF8, 0, nextMission.c_str(), -1, nextName,
                          sizeof(nextName) - 1, nullptr, nullptr);
    const std::string strip =
        "THIS MAP: " + shownMap + "    NEXT: " +
        (canNextMission ? Upper(MapNames::Display(nextName))
                        : std::string("NOT SET"));
    Text(dl, smallFont, 15.0f * s, ImVec2(bx, by - 26.0f * s), kTextDim,
         strip.c_str());
  }
  // Auto-return / auto-advance countdown for host
  if (hostRole && t >= tEnd) {
    static const int s_autoSeconds =
        _wtoi(ArgValue(GetCommandLineW(), L"-lobby-playlist-auto").c_str());
    if (!s_autoInited) {
      s_autoInited = true;
      if (s_autoSeconds > 0) {
        s_autoAt = ImGui::GetTime() + s_autoSeconds;
      }
    }
    if (interactive && s_autoAt > 0.0 && !s_autoCancelled && UiPad::BackPressed()) {
      UiPad::ConsumeBack();
      s_autoAt = 0.0;
      s_autoCancelled = true;
      NOBLE_INFO("PauseMenu: lobby return countdown cancelled by host");
    }
    if (s_autoAt > 0.0 && !s_autoCancelled) {
      const int left =
          std::max(0, static_cast<int>(s_autoAt - ImGui::GetTime() + 0.999));
      char text[64];
      if (s_autoSeconds > 0) {
        std::snprintf(text, sizeof(text),
                      "Next match in %d s. B or Esc cancels.", left);
      } else {
        std::snprintf(text, sizeof(text),
                      "Returning party to lobby in %d s. B or Esc stays.",
                      left);
      }
      Text(dl, smallFont, 16.0f * s, ImVec2(margin, by + bh * 0.5f - 9.0f * s),
           kAmber, text);
      if (ImGui::GetTime() >= s_autoAt) {
        s_autoAt = 0.0;
        if (s_autoSeconds > 0) {
          RelaunchAndClose(hwnd, LobbyReturnArgs(true, true),
                           "next in rotation (countdown)");
        } else if (canNextMission && !hasPlaylist) {
          RelaunchAndClose(hwnd,
                           NextMissionArgs(nextMission, LobbyReturnArgs(true)),
                           "next mission (countdown)");
        } else {
          RelaunchAndClose(hwnd, LobbyReturnArgs(true),
                           "return to lobby (countdown)");
        }
      }
    }
  }
  if (t < tEnd) {
    Text(dl, smallFont, 16.0f * s, ImVec2(margin, by + bh * 0.5f - 9.0f * s),
         kTextDim, "Enter, Space or click: skip");
    if (interactive && (ImGui::IsMouseClicked(0) || skipKey)) {
      s_reveal.skipped = true;
      t = tEnd + 1.0f;
      s_gameOverPadArmedAt = ImGui::GetTime() + 0.35;
      UiPad::ConsumeAccept();
    }
  }
  ImGui::End();
}

namespace {

// Letter-spaced upper-case text, the Halo heading look ("C A M P A I G N"). Returns the drawn width.
float AddTrackedText(ImDrawList *dl, ImFont *font, float px, ImVec2 pos, ImU32 col,
                     const std::string &text, float tracking) {
  float x = pos.x;
  for (const char c : text) {
    const char one[2] = {c, 0};
    dl->AddText(font, px, ImVec2(x, pos.y), col, one);
    x += font->CalcTextSizeA(px, FLT_MAX, 0.0f, one).x + tracking;
  }
  return x - pos.x;
}

float TrackedTextWidth(ImFont *font, float px, const std::string &text, float tracking) {
  float width = 0.0f;
  for (const char c : text) {
    const char one[2] = {c, 0};
    width += font->CalcTextSizeA(px, FLT_MAX, 0.0f, one).x + tracking;
  }
  return width;
}

} // namespace

void PauseMenu::DrawLoading() {
  ImGuiIO &io = ImGui::GetIO();
  ImDrawList *dl = ImGui::GetBackgroundDrawList();
  const float w = io.DisplaySize.x;
  const float h = io.DisplaySize.y;
  const float s = (std::max)(0.6f, h / 1080.0f); // layout scale (1080p reference)

  std::string mapName, scenario, description, mode;
  std::filesystem::path modImage, modThumb;
  std::vector<std::string> players;
  int expectedPlayers = 0;
  bool host = true, joiner = false;
  {
    std::lock_guard<std::mutex> lock(s_mapMutex);
    mapName = s_loadingMapName;
    scenario = s_loadingMap;
    modImage = s_loadingImage;
    modThumb = s_loadingThumb;
    description = s_loadingDescription;
    mode = s_loadingMode;
    players = s_loadingPlayers;
    expectedPlayers = s_loadingExpectedPlayers;
    host = s_loadingHost;
    joiner = s_loadingJoiner;
  }
  if (mapName.empty() && !scenario.empty())
    mapName = Upper(DisplayName(scenario));

  // ---- Atmosphere Backdrop (very subtle dimmed scene + Auntie Dot diamond lattice) ----
  const ImVec2 full0(0.0f, 0.0f), full1(w, h);
  dl->AddRectFilled(full0, full1, IM_COL32(4, 7, 12, 255));
  const UiTexture scene =
      modImage.empty() ? UiArt::LoadingBackground(scenario) : UiArt::ImageFile(modImage);
  if (scene) {
    UiArt::DrawCover(dl, scene, full0, full1, IM_COL32(70, 86, 108, 255));
    dl->AddRectFilled(full0, full1, IM_COL32(4, 8, 14, 218));
  }
  dl->AddRectFilledMultiColor(full0, ImVec2(w * 0.55f, h), IM_COL32(3, 6, 11, 210),
                              IM_COL32(3, 6, 11, 0), IM_COL32(3, 6, 11, 0),
                              IM_COL32(3, 6, 11, 210));

  {
    const float cell = 138.0f * s;
    const float t = static_cast<float>(ImGui::GetTime());
    const float drift = std::fmod(t * 14.0f, cell);
    const float wave = std::fmod(t * 0.045f, 1.0f) * (w + 2.0f * h) - h;
    for (float c = -h - cell; c <= w + h + cell; c += cell) {
      const float cc = c + drift;
      const float d = std::abs(c - wave);
      const float lift = (std::max)(0.0f, 1.0f - d / (cell * 2.2f));
      const ImU32 gridCol =
          IM_COL32(120, 180, 216, static_cast<int>(18.0f + 44.0f * lift));
      const float thick = (1.0f + 0.7f * lift) * s;
      dl->AddLine(ImVec2(cc, 0.0f), ImVec2(cc + h, h), gridCol, thick);
      dl->AddLine(ImVec2(cc, 0.0f), ImVec2(cc - h, h), gridCol, thick);
      if (lift > 0.05f) {
        const ImU32 crest =
            IM_COL32(150, 205, 240, static_cast<int>(72.0f * lift));
        dl->AddLine(ImVec2(cc + 1.0f * s, 0.0f), ImVec2(cc + h + 1.0f * s, h),
                    crest, 0.8f * s);
        dl->AddLine(ImVec2(cc + 1.0f * s, 0.0f), ImVec2(cc - h + 1.0f * s, h),
                    crest, 0.8f * s);
      }
    }
  }

  // ---- Live Progress & Lobby Membership ----
  float progress = LoadScreen::Get().GetLoadProgress();
  const HMODULE engineMod = noble::ReachModule();
  if (engineMod) {
    float engineProg = *reinterpret_cast<const float *>(
        reinterpret_cast<uintptr_t>(engineMod) + 0x46F0E04);
    if (engineProg >= 0.0f && engineProg <= 1.0f && engineProg > progress) {
      progress = engineProg;
      LoadScreen::Get().SetLoadProgress(progress);
    }
  }
  progress = std::clamp(progress, 0.0f, 1.0f);
  const int progressPct = static_cast<int>(progress * 100.0f);

  const size_t peers = (std::max)(NetworkTransport::Get().GetGamePeerCount(),
                                  LobbyPatches::GetRemotePlayerAddCount());
  const size_t engineExpected =
      static_cast<size_t>((std::max)(0, BlamEngine::WaitingRemotePlayersExpected()));
  const size_t engineJoined =
      static_cast<size_t>((std::max)(0, BlamEngine::WaitingRemotePlayersJoined()));
  const size_t joined = 1 + (std::max)(peers, engineJoined);
  // The row count comes from live session membership too: a direct/LAN launch has no lobby roster (main.cpp
  // only knows the local name), so sizing from `players` alone drew 1 row while N were already in the session.
  const int wanted =
      (std::max)({expectedPlayers, static_cast<int>(players.size()),
                  1 + static_cast<int>(engineExpected), 1 + static_cast<int>(peers)});
  // Test evidence (T2): log once whenever the live roster size changes.
  {
    static int s_lastLiveRoster = -1;
    if (s_lastLiveRoster != wanted) {
      s_lastLiveRoster = wanted;
      NOBLE_INFO("PauseMenu: loading roster {} player(s) (transport {}, "
                 "membership {})",
                 wanted, peers, engineJoined);
    }
  }
  const size_t total = static_cast<size_t>(wanted);
  const bool online =
      expectedPlayers > 1 || peers > 0 || joiner || engineExpected > 0;
  const bool waitingForPlayers =
      expectedPlayers > static_cast<int>(joined) || engineExpected > engineJoined;
  const bool waitingForHost =
      joiner && peers == 0 && progress < 0.97f && !waitingForPlayers;
  const char *titleText = waitingForPlayers ? "WAITING FOR PLAYERS"
                          : waitingForHost  ? "WAITING FOR HOST"
                                            : "LOADING";

  const ImU32 white = IM_COL32(238, 243, 248, 255);
  const ImU32 muted = IM_COL32(158, 174, 188, 255);
  const ImU32 accent = ImGui::ColorConvertFloat4ToU32(NobleStyle::Accent());
  const ImU32 good = IM_COL32(118, 214, 142, 255);
  ImFont *font = ImGui::GetFont();
  ImFont *titleFont = NobleStyle::TitleFont();
  ImFont *smallFont = NobleStyle::SmallFont();
  if (!titleFont)
    titleFont = font;
  if (!smallFont)
    smallFont = font;

  // ---- Non-Overlapping Two-Column Tactical Layout ----
  // Left column strictly owns status + progress + roster; right column strictly
  // owns map preview art + briefing card; bottom corner logos stay clear below.
  const float outerX = 64.0f * s;
  const float topY = 54.0f * s;
  const float availW = w - 2.0f * outerX;
  const float colGap = 36.0f * s;
  const float leftW = std::clamp(availW * 0.36f, 320.0f * s, 540.0f * s);
  const float leftX = outerX;
  const float rightX = leftX + leftW + colGap;
  const float rightW = (std::max)(260.0f * s, (w - outerX) - rightX);

  const float logoSize = 86.0f * s;
  const float logoBottom = h - 38.0f * s;
  const float contentBottom = logoBottom - logoSize - 18.0f * s;

  // 1. Left Column: Title, Map/Status Line, Progress Bar, Framed Player Roster
  float titlePx = 42.0f * s;
  const float titleEstW = TrackedTextWidth(titleFont, titlePx, titleText, 1.2f * s);
  if (titleEstW > leftW && titleEstW > 1.0f)
    titlePx *= leftW / titleEstW;
  AddTrackedText(dl, titleFont, titlePx, ImVec2(leftX, topY), white, titleText,
                 1.2f * s);

  std::string status;
  char countBuf[96];
  if (waitingForPlayers) {
    std::snprintf(countBuf, sizeof(countBuf), "%d OF %d IN",
                  static_cast<int>(joined), static_cast<int>(total));
    status = mapName.empty() ? std::string(countBuf) : mapName + "  -  " + countBuf;
  } else if (waitingForHost) {
    status = mapName.empty() ? std::string("FINDING HOST")
                             : mapName + "  -  FINDING HOST";
  } else if (progress > 0.0f) {
    std::snprintf(countBuf, sizeof(countBuf), "%d%%", progressPct);
    status = mapName.empty() ? std::string(countBuf)
                             : mapName + "  -  LOADING " + countBuf;
  } else {
    status = mapName;
  }
  const float statusTop = topY + titlePx + 10.0f * s;
  const float statusPx = 16.5f * s;
  if (!status.empty()) {
    dl->AddText(font, statusPx, ImVec2(leftX, statusTop), muted, status.c_str(),
                nullptr, leftW);
  }

  const float barY = statusTop + 28.0f * s;
  const float barH = 6.0f * s;
  if (!waitingForPlayers) {
    char pctBuf[16];
    std::snprintf(pctBuf, sizeof(pctBuf), "%d%%", progressPct);
    const float pctW = smallFont->CalcTextSizeA(15.0f * s, FLT_MAX, 0.0f, pctBuf).x;
    const float barRight = leftX + leftW - pctW - 12.0f * s;
    NobleUi::DrawTacticalBar(dl, ImVec2(leftX, barY), ImVec2(barRight, barY + barH),
                             progress, accent, s);
    dl->AddText(smallFont, 15.0f * s, ImVec2(barRight + 10.0f * s, barY - 4.0f * s),
                accent, pctBuf);
  } else {
    const float joinFrac = total > 0 ? std::clamp(static_cast<float>(joined) /
                                                  static_cast<float>(total),
                                              0.0f, 1.0f)
                                     : 0.0f;
    NobleUi::DrawTacticalBar(dl, ImVec2(leftX, barY), ImVec2(leftX + leftW, barY + barH),
                             joinFrac, good, s);
  }

  // Framed Player Roster strictly constrained inside [leftX, leftX + leftW]
  const float rosterCardTop = barY + 24.0f * s;
  const float chatHintReserve = online ? 26.0f * s : 0.0f;
  const float rosterCardBottom =
      (std::max)(rosterCardTop + 110.0f * s, contentBottom - chatHintReserve);
  const ImVec2 rc0(leftX, rosterCardTop);
  const ImVec2 rc1(leftX + leftW, rosterCardBottom);
  dl->AddRectFilled(rc0, rc1, IM_COL32(8, 14, 23, 212), 4.0f * s);
  NobleUi::DrawTacticalFrame(dl, rc0, rc1, s, false, false, false, false);

  const float rHdrH = 32.0f * s;
  dl->AddRectFilled(rc0, ImVec2(rc1.x, rc0.y + rHdrH),
                    IM_COL32(14, 24, 38, 235), 4.0f * s,
                    ImDrawFlags_RoundCornersTop);
  dl->AddText(titleFont, 15.0f * s, ImVec2(rc0.x + 12.0f * s, rc0.y + 8.0f * s),
              accent, "PLAYERS");
  {
    char pCount[48];
    std::snprintf(pCount, sizeof(pCount), "%zu / %zu ACTIVE",
                  (std::min)(joined, total), total);
    const float pcw =
        smallFont->CalcTextSizeA(13.0f * s, FLT_MAX, 0.0f, pCount).x;
    dl->AddText(smallFont, 13.0f * s,
                ImVec2(rc1.x - 12.0f * s - pcw, rc0.y + 9.0f * s), muted,
                pCount);
  }
  dl->AddLine(ImVec2(rc0.x, rc0.y + rHdrH), ImVec2(rc1.x, rc0.y + rHdrH),
              IM_COL32(143, 216, 255, 55), 1.0f);

  const SettingsData &settings = Settings::Get().Snapshot();
  const std::string localName =
      settings.playerName.empty() ? std::string("NobleSix") : settings.playerName;
  const float rPad = 10.0f * s;
  const float gridTop = rc0.y + rHdrH + rPad;
  const float gridBottom = rc1.y - rPad;
  const float gridAvailH = (std::max)(40.0f * s, gridBottom - gridTop);
  const float gridAvailW = (std::max)(120.0f * s, leftW - 2.0f * rPad);

  const int rosterCols = wanted > 64 ? 4 : wanted > 32 ? 3 : wanted > 12 ? 2 : 1;
  const float cellGap = (wanted > 32 ? 4.0f : 6.0f) * s;
  const float colGapInner = 8.0f * s;
  const float colW =
      (gridAvailW - colGapInner * static_cast<float>(rosterCols - 1)) /
      static_cast<float>(rosterCols);
  const float maxCellH = (wanted > 32 ? 28.0f : wanted > 12 ? 34.0f : 42.0f) * s;
  int rowsPerColNeeded = (wanted + rosterCols - 1) / rosterCols;
  float cellH =
      (gridAvailH - cellGap * static_cast<float>((std::max)(0, rowsPerColNeeded - 1))) /
      static_cast<float>((std::max)(1, rowsPerColNeeded));
  cellH = std::clamp(cellH, 22.0f * s, maxCellH);
  int maxRowsPerCol = (std::max)(
      1, static_cast<int>((gridAvailH + cellGap) / (cellH + cellGap)));
  rowsPerColNeeded = (std::min)(rowsPerColNeeded, maxRowsPerCol);
  const int capacity = rosterCols * maxRowsPerCol;
  int visiblePlayers = wanted;
  std::string moreText;
  if (capacity < wanted) {
    visiblePlayers = capacity - 1;
    moreText = "+" + std::to_string(wanted - visiblePlayers) + " MORE";
  }

  const bool compactRow = cellH < 30.0f * s || rosterCols >= 2;
  const float namePx = (cellH < 26.0f * s ? 12.0f : compactRow ? 14.0f : 16.5f) * s;
  const float cellStatusPx = (cellH < 26.0f * s ? 10.5f : compactRow ? 11.5f : 13.0f) * s;

  dl->PushClipRect(ImVec2(rc0.x + rPad * 0.5f, gridTop),
                   ImVec2(rc1.x - rPad * 0.5f, gridBottom), true);
  for (int i = 0; i < visiblePlayers + (moreText.empty() ? 0 : 1); ++i) {
    const int col = i % rosterCols;
    const int rowInCol = i / rosterCols;
    const ImVec2 r0(rc0.x + rPad + col * (colW + colGapInner),
                    gridTop + rowInCol * (cellH + cellGap));
    const ImVec2 r1(r0.x + colW, r0.y + cellH);
    if (!moreText.empty() && i == visiblePlayers) {
      dl->AddRectFilled(r0, r1, IM_COL32(12, 20, 30, 210), 3.0f * s);
      const float mw =
          smallFont->CalcTextSizeA(cellStatusPx, FLT_MAX, 0.0f, moreText.c_str()).x;
      dl->AddText(smallFont, cellStatusPx,
                  ImVec2(r0.x + (colW - mw) * 0.5f,
                         r0.y + (cellH - cellStatusPx) * 0.5f),
                  muted, moreText.c_str());
      continue;
    }

    const bool isLocal =
        (i == 0 && players.empty()) ||
        (i < static_cast<int>(players.size()) &&
         players[static_cast<size_t>(i)] == localName);
    std::string name;
    if (i < static_cast<int>(players.size())) {
      name = players[static_cast<size_t>(i)];
    } else if (i == 0) {
      name = localName;
    } else {
      name = "PLAYER " + std::to_string(i + 1);
    }
    if (isLocal)
      name += " (YOU)";

    const bool present = isLocal || static_cast<size_t>(i) < joined;
    std::string rowStatus;
    ImU32 rowStatusCol = muted;
    if (isLocal) {
      rowStatus = host ? "HOSTING" : "READY";
      rowStatusCol = host ? good : accent;
    } else if (present) {
      rowStatus = "READY";
      rowStatusCol = accent;
    } else {
      rowStatus = compactRow ? "WAIT" : "WAITING...";
      rowStatusCol = IM_COL32(120, 134, 148, 255);
    }

    dl->AddRectFilled(r0, r1,
                      present ? IM_COL32(14, 22, 34, 232)
                              : IM_COL32(10, 16, 24, 175),
                      3.0f * s);
    dl->AddRectFilled(ImVec2(r0.x, r0.y + 4.0f * s),
                      ImVec2(r0.x + 3.0f * s, r1.y - 4.0f * s),
                      present ? (isLocal && host ? good : accent)
                              : IM_COL32(90, 104, 118, 170),
                      1.5f * s);
    dl->AddRect(r0, r1, IM_COL32(255, 255, 255, present ? 32 : 16), 3.0f * s);

    const float statusW =
        smallFont->CalcTextSizeA(cellStatusPx, FLT_MAX, 0.0f, rowStatus.c_str()).x;
    const float statusX = r1.x - 8.0f * s - statusW;
    dl->AddText(smallFont, cellStatusPx,
                ImVec2(statusX, r0.y + (cellH - cellStatusPx) * 0.5f),
                rowStatusCol, rowStatus.c_str());

    const ImVec2 nameClipMax(std::max(r0.x + 20.0f * s, statusX - 8.0f * s),
                             r1.y);
    dl->PushClipRect(r0, nameClipMax, true);
    dl->AddText(font, namePx,
                ImVec2(r0.x + 10.0f * s, r0.y + (cellH - namePx) * 0.5f),
                present ? white : IM_COL32(150, 162, 174, 235), name.c_str());
    dl->PopClipRect();
  }
  dl->PopClipRect();

  if (online) {
    AddTrackedText(dl, smallFont, 13.0f * s,
                   ImVec2(leftX, rosterCardBottom + 8.0f * s),
                   IM_COL32(145, 162, 180, 235), "PRESS T FOR CHAT", 2.0f * s);
  }

  // 2. Right Column: Mode Pill, Aspect-Preserved Map Artwork Showcase, Mod Briefing
  {
    const std::string modeLabel =
        mode.empty() ? std::string("HALO: REACH") : LoadingModeLabel(mode);
    const float modePx = 15.5f * s;
    const float tracking = 3.5f * s;
    const float labelW =
        TrackedTextWidth(titleFont, modePx, modeLabel, tracking);
    const float badgeW = labelW + 22.0f * s;
    const ImVec2 mb0(rightX + rightW - badgeW, topY + 2.0f * s);
    const ImVec2 mb1(rightX + rightW, topY + 32.0f * s);
    dl->AddRectFilled(mb0, mb1, IM_COL32(12, 22, 36, 220), 3.0f * s);
    dl->AddRect(mb0, mb1, IM_COL32(143, 216, 255, 80), 3.0f * s);
    AddTrackedText(dl, titleFont, modePx, ImVec2(mb0.x + 12.0f * s, mb0.y + 7.0f * s),
                   white, modeLabel, tracking);
  }

  const float descCardH = description.empty() ? 0.0f : 96.0f * s;
  const float captionH = 44.0f * s;
  const float artBoxTop = topY + 46.0f * s;
  const float artBoxBottom = contentBottom -
                             (descCardH > 0.0f ? descCardH + 12.0f * s : 0.0f);
  const float maxArtW = rightW;
  const float maxArtH = (std::max)(140.0f * s, artBoxBottom - artBoxTop - captionH);

  float drawW = maxArtW;
  float drawH = maxArtH;
  if (scene) {
    const float aspect = scene.height > 0
                             ? static_cast<float>(scene.width) /
                                   static_cast<float>(scene.height)
                             : 16.0f / 9.0f;
    drawW = maxArtW;
    drawH = drawW / aspect;
    if (drawH > maxArtH) {
      drawH = maxArtH;
      drawW = drawH * aspect;
    }
  }
  const float cardTotalH = drawH + captionH;
  const ImVec2 art0(rightX + (rightW - drawW) * 0.5f,
                    artBoxTop + (artBoxBottom - artBoxTop - cardTotalH) * 0.5f);
  const ImVec2 art1(art0.x + drawW, art0.y + drawH);
  const ImVec2 frame1(art1.x, art1.y + captionH);

  dl->AddRectFilled(art0, frame1, IM_COL32(8, 14, 22, 232), 4.0f * s);
  if (scene) {
    dl->AddImage(scene.id, art0, art1, ImVec2(0, 0), ImVec2(1, 1),
                 IM_COL32(255, 255, 255, 255));
  } else {
    const char *noArt = mapName.empty() ? "HALO: REACH" : mapName.c_str();
    const float nw =
        titleFont->CalcTextSizeA(26.0f * s, FLT_MAX, 0.0f, noArt).x;
    dl->AddText(titleFont, 26.0f * s,
                ImVec2(art0.x + (drawW - nw) * 0.5f, art0.y + drawH * 0.5f - 13.0f * s),
                IM_COL32(143, 216, 255, 110), noArt);
  }
  // Caption footer beneath map image (zero overlap over artwork pixels)
  dl->AddRectFilled(ImVec2(art0.x, art1.y), frame1, IM_COL32(11, 19, 31, 245));
  dl->AddLine(ImVec2(art0.x, art1.y), ImVec2(art1.x, art1.y),
              IM_COL32(143, 216, 255, 70), 1.0f);
  if (!mapName.empty()) {
    dl->AddText(titleFont, 19.0f * s, ImVec2(art0.x + 14.0f * s, art1.y + 12.0f * s),
                white, mapName.c_str(), nullptr, (std::max)(80.0f * s, drawW * 0.68f));
  }
  if (!scenario.empty()) {
    const std::string scenTag = Upper(scenario);
    const float stw =
        smallFont->CalcTextSizeA(12.5f * s, FLT_MAX, 0.0f, scenTag.c_str()).x;
    dl->AddText(smallFont, 12.5f * s,
                ImVec2(frame1.x - 14.0f * s - stw, art1.y + 16.0f * s),
                muted, scenTag.c_str());
  }
  NobleUi::DrawTacticalFrame(dl, art0, frame1, s, false, false, false, false);

  if (!description.empty()) {
    const ImVec2 d0(rightX, contentBottom - descCardH);
    const ImVec2 d1(rightX + rightW, contentBottom);
    dl->AddRectFilled(d0, d1, IM_COL32(9, 15, 24, 216), 3.0f * s);
    dl->AddRectFilled(d0, ImVec2(d0.x + 3.0f * s, d1.y), accent);
    dl->AddRect(d0, d1, IM_COL32(143, 216, 255, 50), 3.0f * s);
    dl->AddText(titleFont, 13.0f * s, ImVec2(d0.x + 12.0f * s, d0.y + 8.0f * s),
                accent, "MAP BRIEFING");
    dl->PushClipRect(ImVec2(d0.x + 10.0f * s, d0.y + 24.0f * s),
                     ImVec2(d1.x - 10.0f * s, d1.y - 6.0f * s), true);
    dl->AddText(smallFont, 14.5f * s,
                ImVec2(d0.x + 12.0f * s, d0.y + 26.0f * s),
                IM_COL32(204, 216, 228, 240), description.c_str(), nullptr,
                rightW - 24.0f * s);
    dl->PopClipRect();
  }

  // ---- Corner Emblems (strictly below contentBottom, never overlapping) ----
  const UiTexture nobleLogo = UiArt::LoadingLogoNoble();
  const UiTexture unscLogo = UiArt::LoadingLogoUnsc();
  const float logoLeft = outerX;
  const float logoRight = w - outerX - logoSize;
  if (nobleLogo) {
    dl->AddImage(nobleLogo.id, ImVec2(logoLeft, logoBottom - logoSize),
                 ImVec2(logoLeft + logoSize, logoBottom), ImVec2(0, 0),
                 ImVec2(1, 1), IM_COL32(172, 202, 230, 195));
  }
  if (unscLogo) {
    dl->AddImage(unscLogo.id, ImVec2(logoRight, logoBottom - logoSize),
                 ImVec2(logoRight + logoSize, logoBottom), ImVec2(0, 0),
                 ImVec2(1, 1), IM_COL32(150, 186, 220, 185));
  }
}

void PauseMenu::Render(IDXGISwapChain *swapChain, bool loading,
                       const std::function<void()> &extraOverlay) {
  if (swapChain)
    s_swapChain.store(swapChain);
  s_lastEngineFrame.store(GetTickCount64());
  std::lock_guard<std::recursive_mutex> lock(s_renderMutex);
  RenderImpl(swapChain, loading, false, extraOverlay);
  if (s_scoreboardShotRequest.exchange(false) && swapChain)
    FrameCapture::SaveNow(swapChain, 888888);
}

bool PauseMenu::HoldFrame() {
  // The engine may stop pumping frames at the exact moment the host sends the results: handle the
  // packet here so a guest always gets its after-action report and never argues with a dead session.
  PollHostMatchState();
  if (!s_gameOver.load())
    return false;
  const ULONGLONG now = GetTickCount64();
  const ULONGLONG gameOverAt = s_gameOverAt.load();
  // Give the engine a moment: it keeps drawing for several seconds after the
  // results.
  if (now - s_lastEngineFrame.load() < 1000 || now - gameOverAt < 1000)
    return false;
  IDXGISwapChain *swapChain = s_swapChain.load();
  if (!swapChain)
    return false;
  static bool s_logged = false;
  if (!s_logged) {
    s_logged = true;
    NOBLE_INFO("PauseMenu: engine stopped drawing after the match; Noble draws "
               "the post-game panel itself");
  }
  static unsigned long long s_holdFrames = 0;
  ++s_holdFrames;
  {
    std::lock_guard<std::recursive_mutex> lock(s_renderMutex);
    RenderImpl(swapChain, false, true, {});
    // Test runs: one screenshot of the held panel, and -quit-after keeps
    // working (it is normally checked from slot 1, which no longer runs).
    static const bool s_testCapture =
        wcsstr(GetCommandLineW(), L"-capture-frames") != nullptr;
    if (s_testCapture && s_holdFrames == 60)
      FrameCapture::SaveNow(swapChain, 900000);
    FrameCapture::OnFrame(nullptr, 0);
    swapChain->Present(1, 0);
  }
  return true;
}

void PauseMenu::RenderImpl(IDXGISwapChain *swapChain, bool loading, bool clear,
                           const std::function<void()> &extraOverlay) {
  const bool waitingForPlayersScreen = LoadingWaitingForPlayers();
  s_isLoadingActive.store(loading || waitingForPlayersScreen);

  const bool gameOver = s_gameOver.load();
  // -postgame-test: force the clean GAME OVER panel so its composition over the
  // engine's post-game UI can be captured without playing a match to its score
  // limit.
  if (!gameOver && (wcsstr(GetCommandLineW(), L"-postgame-test") ||
                    wcsstr(GetCommandLineW(), L"-postgame-role"))) {
    static int s_postGameTestFrames = 0;
    if (!loading && ++s_postGameTestFrames == 900) {
      NOBLE_INFO("PauseMenu: -postgame-test forcing the game-over panel");
      ShowGameOver();
    }
  }

  const bool open = s_open.load();
  // A local match (no remote players) freezes while the menu is up. If a peer
  // joins while it is open, the next frame resumes it, so online play keeps the
  // old non-pausing behaviour.
  const size_t peers = NetworkTransport::Get().GetGamePeerCount();
  SetLocalSimulationPaused(open && !loading && !gameOver && peers == 0);
  SocialOverlay::Poll(!open && !loading && !noble::BlamEngine::IsSavedFilmPlayback());
  const bool showFps = Settings::Get().Snapshot().showFps;
  static const bool s_scoreboardTest =
      wcsstr(GetCommandLineW(), L"-scoreboard-test") != nullptr;
  const int boardVk = KeyBindings::EffectiveKey(20);
  const int demo = ScoreboardDemo();
  static int s_demoFrames = 0;
  if (demo >= 2 && !loading && !open && !gameOver && s_demoFrames == 0)
    EnterScoreboardPointer();
  // A fresh press of the scoreboard key hands the pointer back to the game.
  const bool boardPhysical =
      boardVk > 0 && (GetAsyncKeyState(boardVk) & 0x8000) != 0;
  static bool s_boardWasDown = false, s_boardWasPointer = false;
  const bool wasPointer = s_boardPointer.load();
  if (wasPointer && s_boardWasPointer && boardPhysical && !s_boardWasDown &&
      demo == 0)
    s_boardPointer.store(false);
  s_boardWasDown = boardPhysical;
  s_boardWasPointer = wasPointer;
  if (gameOver || open || loading) {
    s_boardPointer.store(false);
    s_boardReport.store(false);
  }
  const bool boardPointer = s_boardPointer.load();
  const bool scoreboardHeld = !loading && !open && !gameOver &&
                              (ReachHost::IsKeyHeld(boardVk) ||
                               s_scoreboardTest || demo > 0 || boardPointer);

  DXGI_SWAP_CHAIN_DESC desc{};
  if (FAILED(swapChain->GetDesc(&desc)) || !desc.OutputWindow)
    return;
  const HWND hwnd = desc.OutputWindow;

  // Guests: if the host has been silent for a while, the engine keeps pumping
  // frames but the session is dead. Show a panel instead of freezing in the
  // mission.
  static bool s_connectionLost = false;
  if (!s_lobbyCallbacksRegistered.exchange(true)) {
    NetworkTransport::Get().SetLobbyReturnCallback([]() {
      NOBLE_INFO("PauseMenu: received host_return_to_lobby");
      s_guestShouldReturnLobby.store(true);
    });
    NetworkTransport::Get().SetLobbyClosedCallback(
        [](const std::string &reason) {
          NOBLE_INFO("PauseMenu: received lobby_closed/disband: {}", reason);
          s_guestShouldDisband.store(true);
        });
  }

  const bool guest =
      NetworkTransport::Get().IsActive() && !NetworkTransport::Get().IsHost();
  if (guest) {
    PollHostMatchState();
    if (s_guestReturnedToLobby.exchange(false))
      return;
  }

  const double hostSilence =
      guest ? NetworkTransport::Get().SecondsSinceRemotePacket() : 0.0;
  static ULONGLONG s_lastStateCheck = 0;
  const ULONGLONG nowTick = GetTickCount64();
  if (guest && hostSilence > 5.0 && nowTick - s_lastStateCheck > 2000) {
    s_lastStateCheck = nowTick;
    std::wstring wSession =
        ArgValue(Relaunch::CurrentGameArgs(), L"-session-id");
    if (wSession.empty())
      wSession = ArgValue(GetCommandLineW(), L"-session-id");
    if (!wSession.empty()) {
      char sessUtf8[128] = {};
      WideCharToMultiByte(CP_UTF8, 0, wSession.c_str(), -1, sessUtf8,
                          sizeof(sessUtf8) - 1, nullptr, nullptr);
      std::thread([sessUtf8, hostSilence]() {
        const LobbyState state =
            MasterServerClient::Get().FetchLobbyStateSync(sessUtf8);
        if (state.ok && !state.isClosed && !state.inProgress) {
          NOBLE_INFO("PauseMenu: host lobby poll shows inProgress=false, "
                     "returning guest to pre-game lobby");
          s_guestShouldReturnLobby.store(true);
        } else if ((state.ok && state.isClosed) ||
                   (!state.ok && hostSilence > 30.0)) {
          NOBLE_INFO("PauseMenu: host lobby poll shows closed={} ok={} after "
                     "{:.1f}s silence, disbanding to main menu",
                     state.isClosed, state.ok, hostSilence);
          s_guestShouldDisband.store(true);
        }
      }).detach();
    }
  }

  if (!loading && !open && !gameOver && guest && hostSilence > 15.0) {
    if (!s_connectionLost) {
      s_connectionLost = true;
      NOBLE_WARN("PauseMenu: host connection lost ({:.1f}s without packets) - "
                 "holding the match for host migration",
                 hostSilence);
    }
  } else if (guest && hostSilence < 2.0) {
    // Packets resumed, e.g. after the engine migrated the host. Drop the panel
    // instead of latching it forever.
    s_connectionLost = false;
  }
  const bool connectionLost = s_connectionLost;
  const bool isTheater = noble::BlamEngine::IsSavedFilmPlayback();
  // The engine's loading screen is gone before an online co-op host has all its players: keep drawing our screen as
  // WAITING FOR PLAYERS until the game session starts (otherwise this is a black frame with the FPS counter).
  if (!(open || gameOver || loading || waitingForPlayersScreen || showFps || scoreboardHeld ||
        connectionLost || extraOverlay || SocialOverlay::WantsFrame() ||
        isTheater) ||
      !swapChain || !ImGui::GetCurrentContext())
    return;

  ID3D11Device *device = nullptr;
  if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device),
                                  reinterpret_cast<void **>(&device))) ||
      !device)
    return;
  ID3D11DeviceContext *context = nullptr;
  device->GetImmediateContext(&context);

  ID3D11Texture2D *backBuffer = nullptr;
  if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                  reinterpret_cast<void **>(&backBuffer))) ||
      !backBuffer) {
    if (context)
      context->Release();
    device->Release();
    return;
  }

  D3D11_TEXTURE2D_DESC bbDesc{};
  backBuffer->GetDesc(&bbDesc);

  ID3D11RenderTargetView *rtv = nullptr;
  {
    std::lock_guard<std::mutex> rtvLock(s_rtvMutex);
    if (s_cachedSwapChain != swapChain || s_cachedBackBuffer != backBuffer ||
        !s_cachedRtv || s_cachedRtvWidth != bbDesc.Width ||
        s_cachedRtvHeight != bbDesc.Height ||
        s_cachedRtvFormat != bbDesc.Format) {
      if (s_cachedRtv) {
        s_cachedRtv->Release();
        s_cachedRtv = nullptr;
      }
      if (s_cachedBackBuffer) {
        s_cachedBackBuffer->Release();
        s_cachedBackBuffer = nullptr;
      }
      s_cachedSwapChain = swapChain;
      s_cachedBackBuffer = backBuffer;
      s_cachedBackBuffer->AddRef();
      s_cachedRtvWidth = bbDesc.Width;
      s_cachedRtvHeight = bbDesc.Height;
      s_cachedRtvFormat = bbDesc.Format;
      device->CreateRenderTargetView(backBuffer, nullptr, &s_cachedRtv);
    }
    rtv = s_cachedRtv;
    if (rtv)
      rtv->AddRef();
  }

  if (context && rtv) {
    // Feed ImGui ourselves on this thread (the Win32 backend's message handler
    // runs on the window thread and its event queue is not thread safe).
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |=
        ImGuiConfigFlags_NoMouseCursorChange; // the window proc shows the arrow
    io.DisplaySize = ImVec2(static_cast<float>(bbDesc.Width),
                            static_cast<float>(bbDesc.Height));

    static LARGE_INTEGER s_freq{}, s_last{};
    LARGE_INTEGER now{};
    QueryPerformanceCounter(&now);
    if (s_freq.QuadPart == 0)
      QueryPerformanceFrequency(&s_freq);
    float dt = s_last.QuadPart
                   ? static_cast<float>(now.QuadPart - s_last.QuadPart) /
                         static_cast<float>(s_freq.QuadPart)
                   : 1.0f / 60.0f;
    if (dt <= 0.0f || dt > 0.25f)
      dt = 1.0f / 60.0f;
    s_last = now;
    io.DeltaTime = dt;

    const bool uiScreenActive = ReachHost::IsUiScreenActive();
    const bool pointerUiActive =
        open || gameOver || boardPointer || connectionLost ||
        uiScreenActive || extraOverlay != nullptr;
    const bool gameOverJustReset = s_resetGameOverImGui.exchange(false);
    static bool s_gameOverMouseWaitRelease = false;
    if (gameOverJustReset) {
      Window::TakeLeftClicks();
      (void)Window::TakeWheelSteps();
      Window::FlushInputEvents();
      UiPad::ResetAfterMatch();
      io.ClearInputKeys();
      io.ClearInputMouse();
      io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
      s_gameOverMouseWaitRelease = true;
    }

    UiPad::Feed(io, pointerUiActive);

    RECT client{};
    GetClientRect(hwnd, &client);
    POINT cursor{};
    static bool s_boardWasPointerFrame = false;
    const bool focused = Window::IsForeground(hwnd);
    const bool gameOverSettling =
        gameOver && (GetTickCount64() < s_gameOverSettleUntil);
    const bool lDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    const bool rDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (!lDown && !rDown) {
      s_gameOverMouseWaitRelease = false;
    }
    if (UiPad::IsVirtualCursorControllingMouse()) {
      s_boardWasPointerFrame = boardPointer;
      if ((pointerUiActive || isTheater) && focused) {
        const float wheel = Window::TakeWheelSteps();
        if (wheel != 0.0f)
          io.AddMouseWheelEvent(0.0f, wheel);
      } else if (loading || waitingForPlayersScreen) {
        (void)Window::TakeWheelSteps();
      }
    } else if ((pointerUiActive || isTheater) && focused &&
               GetCursorPos(&cursor) && ScreenToClient(hwnd, &cursor) &&
               client.right > 0 && client.bottom > 0) {
      // Window may be resized while the back buffer stays the same size: scale
      // to back-buffer pixels.
      const float sx = io.DisplaySize.x / static_cast<float>(client.right);
      const float sy = io.DisplaySize.y / static_cast<float>(client.bottom);
      io.AddMousePosEvent(static_cast<float>(cursor.x) * sx,
                          static_cast<float>(cursor.y) * sy);

      if (gameOverSettling || s_gameOverMouseWaitRelease) {
        Window::TakeLeftClicks();
        (void)Window::TakeWheelSteps();
        io.AddMouseButtonEvent(0, false);
        io.AddMouseButtonEvent(1, false);
      } else {
        bool down = lDown;
        // The click that freed the scoreboard pointer is not a click on the
        // panel: wait for its release.
        static bool s_boardClickHeld = false;
        if (boardPointer && !s_boardWasPointerFrame && down)
          s_boardClickHeld = true;
        s_boardWasPointerFrame = boardPointer;
        if (s_boardClickHeld) {
          if (down) {
            down = false;
            Window::TakeLeftClicks();
          } else {
            s_boardClickHeld = false;
            Window::TakeLeftClicks();
          }
        }
        if (Window::TakeLeftClicks() > 0 && !down) {
          io.AddMouseButtonEvent(0, true);
          io.AddMouseButtonEvent(0, false);
        } else {
          io.AddMouseButtonEvent(0, down);
        }
        io.AddMouseButtonEvent(1, rDown);
        const float wheel = Window::TakeWheelSteps();
        if (wheel != 0.0f)
          io.AddMouseWheelEvent(0.0f, wheel);
      }
    } else {
      s_boardWasPointerFrame = false;
      io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
      io.AddMouseButtonEvent(0, false);
      io.AddMouseButtonEvent(1, false);
      if (pointerUiActive || isTheater || loading || waitingForPlayersScreen) {
        (void)Window::TakeWheelSteps();
      }
    }
    // Keyboard/WM_CHAR events are already forwarded to ImGui by
    // UIManager::HandleWndProc -> ImGui_ImplWin32_WndProcHandler. Plain ImGui
    // text (pause list, challenges, buttons, settings) uses the fonts' fixed
    // 15-18 px. The front-end menu scales it with the back buffer (FrontEndMenu
    // sets this before its NewFrame); the match frame never did, so on a 4K
    // back buffer every pause-menu label drew at about half size inside
    // full-size rows.
    io.FontGlobalScale = NobleUi::Scale();
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();
    InputPrompts::Update();
    if (gameOverJustReset)
      ImGui::SetWindowFocus(nullptr);
    if (SocialOverlay::IsTyping())
      io.ClearInputKeys(); // the line takes the keyboard, not the panels below
                           // it
    if (showFps) {
      char fpsLabel[32];
      std::snprintf(fpsLabel, sizeof(fpsLabel), "FPS %.0f", io.Framerate);
      ImDrawList *foreground = ImGui::GetForegroundDrawList();
      const ImVec2 textSize = ImGui::CalcTextSize(fpsLabel);
      const ImVec2 pos(io.DisplaySize.x - textSize.x - 24.0f, 16.0f);
      foreground->AddRectFilled(
          ImVec2(pos.x - 8.0f, pos.y - 5.0f),
          ImVec2(pos.x + textSize.x + 8.0f, pos.y + textSize.y + 5.0f),
          IM_COL32(10, 16, 24, 190), 5.0f);
      foreground->AddText(pos, IM_COL32(143, 216, 255, 255), fpsLabel);
    }
    if (gameOver) {
      DrawGameOver(hwnd);
    } else {
      if (loading || waitingForPlayersScreen)
        DrawLoading();
      if (open) {
        DrawMenu(hwnd);
      } else if (isTheater && !loading) {
        const float s = NobleUi::Scale();
        const float btnW = 120.0f * s;
        const float btnH = 32.0f * s;
        const float topY = showFps ? (52.0f * s) : (16.0f * s);
        ImGui::SetNextWindowPos(
            ImVec2(io.DisplaySize.x - btnW - 16.0f * s, topY));
        ImGui::SetNextWindowSize(ImVec2(btnW, btnH));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f * s);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                            ImVec2(4.0f * s, 2.0f * s));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg,
                              ImVec4(0.06f, 0.08f, 0.11f, 0.70f));
        ImGui::PushStyleColor(ImGuiCol_Border,
                              ImVec4(0.24f, 0.55f, 0.85f, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_Button,
                              ImVec4(0.10f, 0.14f, 0.18f, 0.60f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.18f, 0.32f, 0.48f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.25f, 0.45f, 0.65f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_Text,
                              ImVec4(0.88f, 0.92f, 0.96f, 0.95f));
        if (ImGui::Begin("##theater_pause_pill", nullptr,
                         ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoBringToFrontOnFocus)) {
          if (ImGui::Button("[ESC] MENU", ImVec2(-1, -1))) {
            Toggle();
          }
        }
        ImGui::End();
        ImGui::PopStyleColor(6);
        ImGui::PopStyleVar(3);
      }
      bool relaunched = false;
      if (connectionLost)
        relaunched = DrawConnectionLostPanel(hwnd);
      if (scoreboardHeld && !connectionLost && !relaunched) {
        DrawScoreboardPanel();
        // -scoreboard-demo: one screenshot about 30 s in, so the match is on
        // screen behind the panel, not a loading frame.
        if (demo > 0 && ++s_demoFrames == 900) {
          s_scoreboardShotRequest.store(true);
          NOBLE_INFO("PauseMenu: -scoreboard-demo {} capture requested", demo);
        }
        if (s_scoreboardTest) {
          static bool s_shot = false;
          if (!s_shot) {
            s_shot = true;
            s_scoreboardShotRequest.store(true);
            NOBLE_INFO("PauseMenu: -scoreboard-test capture requested");
          }
        }
      }
    }
    if (extraOverlay)
      extraOverlay();
    SocialOverlay::Draw(24.0f, 24.0f,
                        778.0f); // chat stays above the motion tracker
    if (open || gameOver || boardPointer || uiScreenActive || extraOverlay != nullptr) {
      UiPad::DrawVirtualCursor(ImGui::GetForegroundDrawList());
    }
    ImGui::Render();

    // The DX11 backend saves/restores most pipeline state but not the
    // output-merger targets.
    ID3D11RenderTargetView *oldRtv = nullptr;
    ID3D11DepthStencilView *oldDsv = nullptr;
    context->OMGetRenderTargets(1, &oldRtv, &oldDsv);
    context->OMSetRenderTargets(1, &rtv, nullptr);
    if (clear) {
      const float dark[4] = {0.016f, 0.031f, 0.055f, 1.0f};
      context->ClearRenderTargetView(rtv, dark);
    }
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    context->OMSetRenderTargets(1, &oldRtv, oldDsv);
    if (oldRtv)
      oldRtv->Release();
    if (oldDsv)
      oldDsv->Release();
  }

  if (rtv)
    rtv->Release();
  if (backBuffer)
    backBuffer->Release();
  if (context)
    context->Release();
  device->Release();
}

} // namespace noble
