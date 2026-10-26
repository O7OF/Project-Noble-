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
#include "network/SessionManager.hpp"
#include "hooks/LobbyPatches.hpp"
#include "hooks/ForgePatches.hpp"

#include <windows.h>
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

    // 5. Initialize Virtual File System (VFS)
    // Directs all file I/O: Retail MCC files remain read-only; custom mods and configs route to ./data/
    fs::path userStoragePath = "data";
    if (!noble::VFS::Initialize(*reachInstall, userStoragePath)) {
        NOBLE_ERROR("Failed to initialize Virtual File System (VFS).");
        return 1;
    }

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

    // Connect ImGui window message handling
    window->SetCustomWndProc(&noble::UIManager::HandleWndProc);
    window->Show(nCmdShow);

    // 8. Initialize Custom UI Overlay (ImGui + Reach Theme)
    if (!noble::UIManager::Initialize(window->GetHwnd(), renderer->GetDevice(), renderer->GetContext())) {
        NOBLE_ERROR("Failed to initialize UI Manager.");
        return 1;
    }

    // Refresh workshop & local mod index
    noble::UIManager::GetWorkshopManager().RefreshMods(reachInstall->workshopDirectory, userStoragePath / "mods");

    // 9. Load and bootstrap haloreach.dll directly in process memory
    auto loader = std::make_unique<noble::Loader>();
    if (!loader->LoadReachDll(reachInstall->reachDllPath)) {
        NOBLE_ERROR("Failed to load haloreach.dll into process space.");
        return 1;
    }

    // Direct all haloreach.dll asset and map file I/O through Virtual File System (VFS)
    noble::VFS::HookTargetModule(loader->GetModuleHandle());

    // Lobby & Forge patches disabled per launch sequence specification

    // 10. Instantiate Blam! Engine & Data Access interfaces
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

    // 11. Initialize P2P & Self-Hosting Network Subsystem
    noble::SessionManager::Get().Initialize();

    NOBLE_INFO("Phase 4 Host, P2P Network, VFS & Mission Launcher active. Entering render loop...");

    // 12. Frame timing & message loop
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (window->ProcessMessages()) {
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
        lastFrameTime = currentFrameTime;

        // Blam simulation tick
        blamEngine->Update(deltaTime);

        // Graphics render pass
        renderer->BeginFrame();
        blamEngine->Render();

        // Custom UI Overlay render pass (Missions, Servers, Armor Locker, Workshop Mods)
        noble::UIManager::BeginFrame();
        noble::UIManager::Render(blamEngine.get());
        noble::UIManager::EndFrame();

        renderer->EndFrame(true);
        Sleep(1);
    }

    // 13. Clean shutdown
    NOBLE_INFO("Initiating clean host shutdown...");
    noble::SessionManager::Get().Shutdown();
    noble::UIManager::Shutdown();
    blamEngine->Shutdown();
    loader->Unload();
    noble::VFS::Shutdown();
    noble::HookManager::Shutdown();
    renderer->Shutdown();
    noble::Logger::Shutdown();

    return 0;
}
