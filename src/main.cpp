#include "common/Logger.hpp"
#include "common/Types.hpp"
#include "common/Version.hpp"
#include "host/SteamLocator.hpp"
#include "host/OwnershipVerifier.hpp"
#include "host/Window.hpp"
#include "host/D3D11Renderer.hpp"
#include "host/Loader.hpp"
#include "hooks/HookManager.hpp"
#include "hooks/SigScanner.hpp"
#include "vfs/VFS.hpp"
#include "engine/BlamEngine.hpp"
#include "engine/CameraFov.hpp"
#include "engine/ReachArmory.hpp"
#include "engine/ElitePack.hpp"
#include "ui/UIManager.hpp"
#include "ui/FrontEndMenu.hpp"
#include "ui/LoadScreen.hpp"
#include "network/SessionManager.hpp"
#include "network/MasterServerClient.hpp"
#include "network/AuthClient.hpp"
#include "network/UPnP.hpp"
#include "common/Settings.hpp"
#include "common/Relaunch.hpp"
#include "ui/PauseMenu.hpp"
#include "ui/UiArt.hpp"
#include "audio/ReachAudio.hpp"
#include "hooks/LobbyPatches.hpp"
#include "hooks/ForgePatches.hpp"
#include "hooks/EnginePatches.hpp"

#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <chrono>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

static LONG WINAPI NobleCrashHandler(EXCEPTION_POINTERS* pExceptionInfo) {
    if (pExceptionInfo && pExceptionInfo->ExceptionRecord) {
        uintptr_t faultAddr = reinterpret_cast<uintptr_t>(pExceptionInfo->ExceptionRecord->ExceptionAddress);
        HMODULE hMod = nullptr;
        WCHAR modPath[MAX_PATH] = L"Unknown Module";
        if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCWSTR>(faultAddr), &hMod)) {
            GetModuleFileNameW(hMod, modPath, MAX_PATH);
        }

        spdlog::error("FATAL EXCEPTION CAUGHT: Code 0x{:08X} at Address 0x{:X} (Module: {})", 
            pExceptionInfo->ExceptionRecord->ExceptionCode,
            faultAddr,
            std::filesystem::path(modPath).string());

        // Capture call stack
        void* stackFrames[32] = { nullptr };
        USHORT frameCount = CaptureStackBackTrace(0, 32, stackFrames, nullptr);
        for (USHORT i = 0; i < frameCount; ++i) {
            uintptr_t frameAddr = reinterpret_cast<uintptr_t>(stackFrames[i]);
            HMODULE frameMod = nullptr;
            WCHAR frameModPath[MAX_PATH] = L"Unknown";
            if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                   reinterpret_cast<LPCWSTR>(frameAddr), &frameMod)) {
                GetModuleFileNameW(frameMod, frameModPath, MAX_PATH);
            }
            spdlog::error(" Frame {:02d}: 0x{:X} [{}]", i, frameAddr, std::filesystem::path(frameModPath).filename().string());
        }

        // Generate crash minidump
        HANDLE hDumpFile = CreateFileW(L"noble-crash.dmp", GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hDumpFile != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION mei{};
            mei.ThreadId = GetCurrentThreadId();
            mei.ExceptionPointers = pExceptionInfo;
            mei.ClientPointers = FALSE;
            MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &mei, nullptr, nullptr);
            CloseHandle(hDumpFile);
            spdlog::error("Crash minidump written to noble-crash.dmp");
        }
    }
    spdlog::default_logger()->flush();
    return EXCEPTION_CONTINUE_SEARCH;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);

    SetUnhandledExceptionFilter(&NobleCrashHandler);

    // 1. Initialize logging
    noble::Logger::Init("noble-client.log");
    NOBLE_INFO("Starting {} - {}", noble::APP_NAME, noble::VERSION_STRING);

    // 2. Discover Steam MCC installation and verify Reach DLC assets
    auto reachInstall = noble::SteamLocator::LocateAndVerifyReach();
    if (!reachInstall) {
        NOBLE_ERROR("Halo: Reach installation could not be located. Aborting launch.");
        MessageBoxW(nullptr, 
            L"Could not locate Halo: Reach DLC in any Steam library.\n\n"
            L"Please verify that Halo: The Master Chief Collection with the Halo: Reach DLC is installed on Steam.",
            L"Halo: Reach Community Edition - Launch Error", 
            MB_ICONERROR | MB_OK);
        return 1;
    }

    // 3. Strict Steam Ownership Verification
    // Unowned copies are strictly prohibited from launching
    if (!noble::OwnershipVerifier::VerifySteamOwnership(*reachInstall)) {
        NOBLE_ERROR("Steam ownership verification failed. Aborting launch.");
        return 1;
    }

    // 4. Initialize MinHook hooking framework early (required by VFS and engine hooks)
    if (!noble::HookManager::Initialize()) {
        NOBLE_ERROR("Failed to initialize MinHook.");
        return 1;
    }

    // 5. Locate authentic data directory (resolves blocks, menumaps, armor, and userdata)
    auto FindDataDirectory = []() -> fs::path {
        wchar_t exePathBuf[MAX_PATH] = { 0 };
        if (GetModuleFileNameW(nullptr, exePathBuf, MAX_PATH)) {
            fs::path exeDir = fs::path(exePathBuf).parent_path();
            std::vector<fs::path> candidates = {
                exeDir / "data",
                exeDir / ".." / "data",
                exeDir / ".." / ".." / "data",
                exeDir / ".." / ".." / ".." / "data",
                fs::current_path() / "data"
            };
            for (const auto& cand : candidates) {
                std::error_code ec;
                if (fs::exists(cand / "menumaps" / "mainmenu_all.map", ec) ||
                    fs::exists(cand / "launch_block_1.bin", ec)) {
                    return fs::absolute(cand);
                }
            }
        }
        return fs::absolute("data");
    };

    fs::path userStoragePath = FindDataDirectory();
    NOBLE_INFO("Resolved authentic Project Noble data directory: {}", userStoragePath.string());

    // Initialize Virtual File System (VFS)
    // Directs all file I/O: Retail MCC files remain read-only; custom mods and configs route to data/
    if (!noble::VFS::Initialize(*reachInstall, userStoragePath)) {
        NOBLE_ERROR("Failed to initialize Virtual File System (VFS).");
        return 1;
    }

    // Configure Host object paths early
    noble::ReachHost::Get().SetPaths(reachInstall->reachDirectory, reachInstall->mapsDirectory, userStoragePath);
    noble::ReachArmory::Initialize(userStoragePath);

    // Player settings (data\userdata\settings.json): window size, controls, volumes, name, servers.
    noble::Settings::Get().Load();
    noble::AuthClient::Get().Initialize();
    const noble::SettingsData startupSettings = noble::Settings::Get().Snapshot();

    // 6. Create Win32 host window
    auto window = std::make_unique<noble::Window>(L"Halo: Reach Community Edition",
        static_cast<uint32_t>(startupSettings.width), static_cast<uint32_t>(startupSettings.height));
    window->SetBorderless(startupSettings.borderless);
    if (!window->Create()) {
        NOBLE_ERROR("Failed to create Win32 host window.");
        return 1;
    }

    // 7. Initialize Direct3D 11 runtime environment
    auto renderer = std::make_unique<noble::D3D11Renderer>();
    if (!renderer->Initialize(window->GetHwnd(), window->GetWidth(), window->GetHeight())) {
        NOBLE_ERROR("Failed to initialize Direct3D 11 renderer.");
        return 1;
    }

    // Initialize LoadScreen wireframe particle renderer
    noble::LoadScreen::Get().Initialize(renderer->GetDevice(), renderer->GetContext());

    // Initialize ImGui UI Manager
    noble::UIManager::Initialize(window->GetHwnd(), renderer->GetDevice(), renderer->GetContext());
    {
        // Menu and loading-screen pictures: the player's own art, Noble's art, or MCC's UI packs.
        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        noble::UiArt::Init(renderer->GetDevice(), reachInstall->reachDirectory.parent_path(),
                           std::filesystem::path(exePath).parent_path());
        // Reach's menu sounds and menu music, decoded from MCC's sound bank into data\userdata\cache\audio.
        noble::ReachAudio::Init(reachInstall->reachDirectory,
                                std::filesystem::path(exePath).parent_path() / "data" / "userdata" / "cache" / "audio");
    }

    window->Show(nCmdShow);
    noble::ReachHost::Get().SetDisplaySize(window->GetWidth(), window->GetHeight());

    // LAN discovery and hosting must already run while the front-end menu is open, so a game
    // hosted from the SERVERS tab (or by someone else on the LAN) shows up in the list.
    noble::SessionManager::Get().Initialize();
    const std::string masterUrl = startupSettings.masterServerUrl.empty() ? "https://outbreak-z-test-server.onrender.com" : startupSettings.masterServerUrl;
    noble::MasterServerClient::Get().SetUrl(masterUrl);

    // 8. Load and bootstrap haloreach.dll directly in process memory
    auto loader = std::make_unique<noble::Loader>();
    if (!loader->LoadReachDll(reachInstall->reachDllPath)) {
        NOBLE_ERROR("Failed to load haloreach.dll into process space.");
        return 1;
    }

    // Direct all haloreach.dll asset and map file I/O through Virtual File System (VFS)
    noble::VFS::HookTargetModule(loader->GetModuleHandle());

    // The SETTINGS > VIDEO field-of-view slider writes the engine's own camera FOV global.
    noble::CameraFov::SetEngineModule(loader->GetModuleHandle());

    // Apply 32/64 player lobby expansion detours
    noble::LobbyPatches::Apply(loader->GetModuleHandle());

    // Apply engine stability and appearance cap patches (unlocks all 200+ armor permutations in haloreach.dll)
    noble::EnginePatches::Apply(loader->GetModuleHandle());

    // 9. Instantiate Blam! Engine & Data Access interfaces
    auto blamEngine = std::make_unique<noble::BlamEngine>();
    noble::EngineHostContext hostContext;
    hostContext.hWnd = window->GetHwnd();
    hostContext.pDevice = renderer->GetDevice();
    hostContext.pContext = renderer->GetContext();
    hostContext.pSwapChain = renderer->GetSwapChain();
    hostContext.width = window->GetWidth();
    hostContext.height = window->GetHeight();

    if (!blamEngine->Initialize(*reachInstall, loader->GetModuleHandle(), loader->GetExports(), hostContext)) {
        NOBLE_ERROR("Failed to initialize Blam! engine host interfaces.");
        return 1;
    }

    // Parse command line arguments for direct map and variant launches (e.g. from NobleLauncher or Relaunch)
    std::string launchMap = "";
    std::string launchMode = "mainmenu";
    std::string customMvar = "";
    std::string customVariant = "";
    fs::path modFolder;
    int launchDifficulty = 1;
    uint64_t launchSkulls = 0;
    bool launchHost = false;
    bool hostOnline = true;
    bool hostPublic = true;
    int hostMaxPlayers = 16;
    std::string connectAddress = "";

    auto WideToNarrow = [](const std::wstring& w) -> std::string {
        if (w.empty()) return "";
        int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (len <= 0) return "";
        std::string s(len - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), len, nullptr, nullptr);
        return s;
    };

    int numArgs = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &numArgs);
    if (argvW) {
        for (int i = 1; i < numArgs; ++i) {
            std::wstring arg = argvW[i];
            if (arg == L"-map" && i + 1 < numArgs) {
                launchMap = WideToNarrow(argvW[++i]);
            } else if (arg == L"-mode" && i + 1 < numArgs) {
                launchMode = WideToNarrow(argvW[++i]);
            } else if (arg == L"-mvar" && i + 1 < numArgs) {
                customMvar = WideToNarrow(argvW[++i]);
            } else if (arg == L"-variant" && i + 1 < numArgs) {
                customVariant = WideToNarrow(argvW[++i]);
            } else if (arg == L"-mod" && i + 1 < numArgs) {
                modFolder = fs::path(argvW[++i]);
            } else if (arg == L"-skulls" && i + 1 < numArgs) {
                launchSkulls = _wcstoui64(argvW[++i], nullptr, 0);
            } else if (arg == L"-difficulty" && i + 1 < numArgs) {
                launchDifficulty = _wtoi(argvW[++i]);
            } else if (arg == L"-host") {
                launchHost = true;
            } else if (arg == L"-online") {
                hostOnline = true;
            } else if (arg == L"-lan") {
                hostOnline = false;
            } else if (arg == L"-public") {
                hostPublic = true;
            } else if (arg == L"-private") {
                hostPublic = false;
            } else if (arg == L"-max-players" && i + 1 < numArgs) {
                hostMaxPlayers = _wtoi(argvW[++i]);
            } else if (arg == L"-connect" && i + 1 < numArgs) {
                connectAddress = WideToNarrow(argvW[++i]);
            }
        }
        LocalFree(argvW);
    }

    if (!modFolder.empty()) {
        NOBLE_INFO("Activating mod folder '{}'", modFolder.string());
        noble::VFS::SetActiveMod(modFolder);
    }

    // Map-info sandbox. The engine builds its map list by enumerating "haloreach\\maps\\info\\*.*"
    // RELATIVE TO THE WORKING DIRECTORY (root prefix "haloreach\\" at RVA 0x985D28, FindFirstFileW).
    // With an empty list every launch fails with "no map selected" and the menu has no backdrop.
    {
        std::error_code ec;
        const fs::path sandboxRoot = userStoragePath / "sandbox";
        const fs::path sandboxInfo = sandboxRoot / "haloreach" / "maps" / "info";
        const fs::path mccInfo = reachInstall->mapsDirectory / "info";
        fs::create_directories(sandboxInfo, ec);
        size_t copied = 0;
        for (const auto& entry : fs::directory_iterator(mccInfo, ec)) {
            if (!entry.is_regular_file(ec)) continue;
            fs::copy_file(entry.path(), sandboxInfo / entry.path().filename(), fs::copy_options::overwrite_existing, ec);
            if (!ec) ++copied;
        }
        if (copied > 0 && SetCurrentDirectoryW(sandboxRoot.c_str())) {
            NOBLE_INFO("Map-info sandbox: copied {} file(s) from '{}'; working directory is now '{}'",
                copied, mccInfo.string(), sandboxRoot.string());
        } else {
            NOBLE_ERROR("Map-info sandbox setup failed (copied {} from '{}'). The engine will report 'no map selected'.",
                copied, mccInfo.string());
        }
    }

    // Check if we should launch into the Front-End Menu + 3D Armory (mainmenu_all.map inside haloreach.dll)
    const bool showMenu = noble::FrontEndMenu::ShouldShow();
    if (showMenu) {
        noble::ReachArmory::SetMenuMode(true);
        noble::FrontEndMenu::Initialize(reachInstall->reachDirectory);
        launchMap = "mainmenu";
        launchMode = "mainmenu";
    } else {
        noble::ReachArmory::SetMenuMode(false);
        if (launchMap.empty()) {
            launchMap = "20_sword_slayer";
            launchMode = "multiplayer";
        }

        // Remember this launch for the post-game "PLAY AGAIN" button and the loading screen.
        auto toWide = [](const std::string& s) -> std::wstring {
            if (s.empty()) return L"";
            const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
            if (len <= 0) return L"";
            std::wstring w(len - 1, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
            return w;
        };
        std::wstring args = L"-map \"" + toWide(launchMap) + L"\" -mode \"" + toWide(launchMode) + L"\"";
        args += L" -difficulty " + std::to_wstring(launchDifficulty);
        if (!customVariant.empty()) args += L" -variant \"" + toWide(customVariant) + L"\"";
        if (!customMvar.empty()) args += L" -mvar \"" + toWide(customMvar) + L"\"";
        if (launchSkulls != 0) args += L" -skulls " + std::to_wstring(launchSkulls);
        if (!modFolder.empty()) args += L" -mod \"" + modFolder.wstring() + L"\"";
        if (!connectAddress.empty()) {
            args += L" -connect \"" + toWide(connectAddress) + L"\"";
        } else if (launchHost) {
            args += L" -host";
            if (hostOnline) args += L" -online";
            else args += L" -lan";
            if (hostPublic) args += L" -public";
            else args += L" -private";
            args += L" -max-players " + std::to_wstring(hostMaxPlayers);
        }
        noble::Relaunch::SetCurrentGameArgs(args);
        noble::PauseMenu::SetLoadingMap(launchMap);
        noble::UiArt::LoadingBackground(launchMap);
        noble::UiArt::MapPreview(launchMap, true);
    }

    // Elite armour pack (data\armor\<name>.map + .rrreloc, local only): loaded with multiplayer and Firefight maps.
    if (!showMenu) noble::ElitePack::Install(loader->GetModuleHandle(), userStoragePath, launchMode);

    if (!connectAddress.empty()) {
        launchHost = false; // Guest joining remote match
        NOBLE_INFO("Launching Halo: Reach Engine as Guest connecting to '{}' (Map: {}, Mode: {})", connectAddress, launchMap, launchMode);
        blamEngine->LaunchMap(launchMap, launchMode, launchDifficulty, launchSkulls, customMvar, customVariant, connectAddress);
    } else {
        NOBLE_INFO("Launching Halo: Reach Engine: Target Map='{}', Mode='{}'", launchMap, launchMode);
        blamEngine->LaunchMap(launchMap, launchMode, launchDifficulty, launchSkulls, customMvar, customVariant);
    }

// 10. Initialize P2P & Self-Hosting Network Subsystem
    noble::SessionManager::Get().Initialize();

    if (launchHost) {
        noble::HostConfig cfg;
        std::string modeTitle = launchMode;
        if (!modeTitle.empty()) modeTitle[0] = static_cast<char>(toupper(static_cast<unsigned char>(modeTitle[0])));
        cfg.sessionName = startupSettings.playerName + "'s " + modeTitle;
        if (launchMode == "campaign") cfg.sessionName = startupSettings.playerName + "'s Co-op Campaign";
        else if (launchMode == "firefight") cfg.sessionName = startupSettings.playerName + "'s Firefight";
        else if (launchMode == "forge") cfg.sessionName = startupSettings.playerName + "'s Forge";

        cfg.gameMode = launchMode;
        cfg.mapName = launchMap;
        cfg.gametypeName = (launchMode == "firefight" ? "Firefight" : launchMode == "forge" ? "Forge" : "Slayer");
        cfg.maxPlayers = hostMaxPlayers;
        cfg.port = 11774;
        cfg.mode = hostOnline ? noble::NetworkMode::Online_P2P : noble::NetworkMode::Local_LAN;

        if (noble::SessionManager::Get().StartHosting(cfg)) {
            NOBLE_INFO("main: Started self-hosting match '{}' on port {} (Mode: {})", cfg.sessionName, cfg.port, hostOnline ? "Online P2P" : "Local LAN");
            if (hostOnline && hostPublic && noble::MasterServerClient::Get().Enabled()) {
                noble::MasterAdvert advert;
                advert.name = cfg.sessionName;
                advert.port = cfg.port;
                advert.maxPlayers = cfg.maxPlayers;
                advert.map = cfg.mapName;
                advert.mode = cfg.gameMode;
                advert.hasPassword = false;
                advert.isMod = !modFolder.empty();
                advert.address = noble::UPnP::Get().GetExternalIP();
                noble::MasterServerClient::Get().StartAdvertising(advert);
                NOBLE_INFO("main: Advertised hosted match to master server ({}:{})", advert.address, advert.port);
            }
        } else {
            NOBLE_WARN("main: Failed to start self-hosting match");
        }
    }

    NOBLE_INFO("Authentic Halo: Reach host message loop active. Driving frames...");

    // 11. Native Win32 message pump: Drive frame rendering, wireframe dots, UI overlays, and swap chain Present
    while (window->ProcessMessages()) {
        blamEngine->Update(0.016f);

        // haloreach.dll's engine thread presents (host slot 1). After a match it stops drawing; then the
        // post-game panel is drawn and presented from here (PauseMenu::HoldFrame, as RR does).
        // The front-end menu likewise once the engine drops its menu map (FrontEndMenu::HoldFrame).
        if (!noble::PauseMenu::HoldFrame() && !noble::FrontEndMenu::HoldFrame()) Sleep(10);
    }

    // After a match the engine thread can be stuck (it waits for a main menu map MCC doesn't ship), and its
    // Shutdown could then wait forever. Make sure the process still ends.
    std::thread([] {
        Sleep(5000);
        NOBLE_WARN("Engine shutdown took over 5 s; exiting anyway.");
        noble::Logger::Shutdown();
        TerminateProcess(GetCurrentProcess(), 0);  // not ExitProcess: its DLL shutdown can hang (see below)
    }).detach();

    // 13. Clean shutdown (Correct order: Hooks & VFS -> Engine -> Loader -> Renderer)
    NOBLE_INFO("Initiating clean host shutdown...");
    noble::MasterServerClient::Get().Shutdown();
    noble::SessionManager::Get().Shutdown();
    // The engine thread keeps drawing Noble's overlay (ImGui) until the engine stops, so stop it first:
    // shutting ImGui down while a frame is still coming crashed in ImGui_ImplDX11_NewFrame (2026-10-28).
    blamEngine->Shutdown();
    noble::UiArt::Shutdown();
    noble::UIManager::Shutdown();
    noble::LoadScreen::Get().Shutdown();

    NOBLE_INFO("Engine shut down; exiting process without unhooking or unloading haloreach.dll.");
    noble::Logger::Shutdown();
    TerminateProcess(GetCurrentProcess(), 0);  // not ExitProcess: its DLL shutdown can hang (see below)
    return 0;   // not reached
}
