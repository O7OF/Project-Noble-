#include "common/Logger.hpp"
#include "common/Types.hpp"
#include "host/SteamLocator.hpp"
#include "host/OwnershipVerifier.hpp"
#include "host/Window.hpp"
#include "host/D3D11Renderer.hpp"
#include "host/Loader.hpp"
#include "hooks/HookManager.hpp"
#include "hooks/SigScanner.hpp"

#include <windows.h>
#include <memory>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);

    // 1. Initialize logging
    noble::Logger::Init("noble-client.log");
    NOBLE_INFO("Starting Halo: Reach Community Edition (noble-client)...");

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

    // 4. Create Win32 host window
    auto window = std::make_unique<noble::Window>(L"Halo: Reach Community Edition", 1280, 720);
    if (!window->Create()) {
        NOBLE_ERROR("Failed to create Win32 host window.");
        return 1;
    }

    // 5. Initialize Direct3D 11 runtime environment
    auto renderer = std::make_unique<noble::D3D11Renderer>();
    if (!renderer->Initialize(window->GetHwnd(), window->GetWidth(), window->GetHeight())) {
        NOBLE_ERROR("Failed to initialize Direct3D 11 renderer.");
        return 1;
    }

    window->Show(nCmdShow);

    // 6. Initialize MinHook hooking framework
    if (!noble::HookManager::Initialize()) {
        NOBLE_ERROR("Failed to initialize MinHook.");
        return 1;
    }

    // 7. Load and bootstrap haloreach.dll directly in process memory
    auto loader = std::make_unique<noble::Loader>();
    if (!loader->LoadReachDll(reachInstall->reachDllPath)) {
        NOBLE_ERROR("Failed to load haloreach.dll into process space.");
        return 1;
    }

    // 8. Test memory pattern scanner on haloreach.dll
    NOBLE_INFO("Verifying signature scanner over haloreach.dll memory range...");
    // Sample search for standard function prologue in haloreach.dll .text section
    uintptr_t testSig = noble::SigScanner::FindPattern(
        loader->GetBaseAddress(), 
        loader->GetModuleSize(), 
        "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC"
    );

    if (testSig != 0) {
        NOBLE_INFO("Signature scanner verified! Found test prologue pattern at 0x{:X}", testSig);
    } else {
        NOBLE_WARN("Signature scanner test scan completed (pattern not found, normal for generic probe).");
    }

    NOBLE_INFO("Bootstrapper Phase 1 complete. Entering host render loop...");

    // 9. Host frame and window message loop
    while (window->ProcessMessages()) {
        renderer->BeginFrame();

        // Render pass (Phase 3 will inject ImGui overlay here)

        renderer->EndFrame(true);
        Sleep(1);
    }

    // 10. Clean shutdown
    NOBLE_INFO("Initiating clean host shutdown...");
    loader->Unload();
    noble::HookManager::Shutdown();
    renderer->Shutdown();
    noble::Logger::Shutdown();

    return 0;
}
