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
#include "ui/UIManager.hpp"
#include "ui/FrontEndMenu.hpp"
#include "ui/LoadScreen.hpp"
#include "network/SessionManager.hpp"
#include "hooks/LobbyPatches.hpp"
#include "hooks/ForgePatches.hpp"

#include <windows.h>
#include <shellapi.h>
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

    // 6. Create Win32 host window
    auto window = std::make_unique<noble::Window>(L"Halo: Reach Community Edition", 1280, 720);
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

    window->Show(nCmdShow);

    // Front-end menu. MCC ships no mainmenu.map for Reach, so with no -map (or -map/-mode mainmenu,
    // which the launcher's Main Menu button passes) Noble shows its own menu before the engine starts.
    std::string frontEndMap;
    std::string frontEndVariant;
    if (noble::FrontEndMenu::ShouldShow()) {
        noble::FrontEndChoice choice = noble::FrontEndMenu::Run(*window, *renderer, reachInstall->reachDirectory);
        if (!choice.play) {
            NOBLE_INFO("Exiting from the front-end menu.");
            noble::UIManager::Shutdown();
            noble::Logger::Shutdown();
            ExitProcess(0);
        }
        frontEndMap = choice.map;
        frontEndVariant = choice.variantPath;
    }

    // 8. Load and bootstrap haloreach.dll directly in process memory
    auto loader = std::make_unique<noble::Loader>();
    if (!loader->LoadReachDll(reachInstall->reachDllPath)) {
        NOBLE_ERROR("Failed to load haloreach.dll into process space.");
        return 1;
    }

    // Direct all haloreach.dll asset and map file I/O through Virtual File System (VFS)
    noble::VFS::HookTargetModule(loader->GetModuleHandle());

    // Apply 32/64 player lobby expansion detours
    noble::LobbyPatches::Apply(loader->GetModuleHandle());

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

    // Parse command line arguments for direct map and variant launches (e.g. from NobleLauncher)
    std::string launchMap = "";
    std::string launchMode = "mainmenu";
    std::string customMvar = "";
    std::string customVariant = "";
    int launchDifficulty = 1;
    uint32_t launchSkulls = 0;

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
            } else if (arg == L"-difficulty" && i + 1 < numArgs) {
                launchDifficulty = _wtoi(argvW[++i]);
            }
        }
        LocalFree(argvW);
    }

    // A choice made in the front-end menu replaces the menu request.
    if (!frontEndMap.empty()) {
        launchMap = frontEndMap;
        launchMode = "multiplayer";
        if (customVariant.empty()) customVariant = frontEndVariant;
    }

    // If no specific map requested, default to the working 20_sword_slayer test
    if (launchMap.empty()) {
        launchMap = "20_sword_slayer";
        launchMode = "multiplayer";
    }

    // Map-info sandbox. The engine builds its map list by enumerating "haloreach\\maps\\info\\*.*"
    // RELATIVE TO THE WORKING DIRECTORY (root prefix "haloreach\\" at RVA 0x985D28, FindFirstFileW).
    // With an empty list every launch fails with "no map selected" and the menu has no backdrop.
    // Like Reach Renaissance, copy the player's own MCC .mapinfo files into a sandbox and run the
    // engine from there; the MCC install is never modified. Map files themselves still resolve
    // through host slot 48 to the MCC folder.
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

    NOBLE_INFO("Launching Halo: Reach Engine: Target Map='{}', Mode='{}'", launchMap, launchMode);
    blamEngine->LaunchMap(launchMap, launchMode, launchDifficulty, launchSkulls, customMvar, customVariant);

    // 10. Initialize P2P & Self-Hosting Network Subsystem
    noble::SessionManager::Get().Initialize();

    NOBLE_INFO("Authentic Halo: Reach host message loop active. Driving frames...");

    // 11. Native Win32 message pump: Drive frame rendering, wireframe dots, UI overlays, and swap chain Present
    while (window->ProcessMessages()) {
        blamEngine->Update(0.016f);

        // Allow haloreach.dll's internal render thread (Slot 1) to drive presentation
        Sleep(10);
    }

    // 13. Clean shutdown (Correct order: Hooks & VFS -> Engine -> Loader -> Renderer)
    NOBLE_INFO("Initiating clean host shutdown...");
    noble::SessionManager::Get().Shutdown();
    noble::UIManager::Shutdown();
    noble::LoadScreen::Get().Shutdown();
    blamEngine->Shutdown();

    // haloreach.dll's worker threads can still be running after its Shutdown call returns.
    // Removing hooks, restoring the IAT or unloading the DLL underneath them crashed in ntdll
    // (0xC0000005 on exit). Leave hooks and the DLL in place and let ExitProcess stop the
    // engine threads before anything is torn down.
    NOBLE_INFO("Engine shut down; exiting process without unhooking or unloading haloreach.dll.");
    noble::Logger::Shutdown();
    ExitProcess(0);
}
