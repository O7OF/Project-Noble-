#include "DedicatedServer.hpp"
#include "common/Logger.hpp"
#include "common/Version.hpp"
#include "host/SteamLocator.hpp"
#include "host/OwnershipVerifier.hpp"
#include "host/Loader.hpp"
#include "host/EngineLimits.hpp"
#include "vfs/VFS.hpp"
#include "hooks/LobbyPatches.hpp"
#include "hooks/ForgePatches.hpp"

#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    noble::Logger::Init("noble-dedicated.log");

    std::cout << "=============================================================\n";
    std::cout << " Project Noble - Headless Dedicated Server (Closed Source)\n";
    std::cout << " Version: " << noble::VERSION_STRING << "\n";
    std::cout << "=============================================================\n";

    std::string configFile = "dedicated_server.json";
    std::string limitsArg;   // -limits v16: the engine-limits profile this server runs (must match its clients)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-config" && i + 1 < argc) {
            configFile = argv[++i];
        } else if (arg == "-limits" && i + 1 < argc) {
            limitsArg = argv[++i];
        }
    }

    // 1. Locate and verify Steam MCC installation
    auto reachInstall = noble::SteamLocator::LocateAndVerifyReach();
    if (!reachInstall || !reachInstall->areFilesVerified) {
        NOBLE_ERROR("Failed to locate local Steam installation of Halo: Reach (MCC).");
        return 1;
    }

    if (!noble::OwnershipVerifier::VerifySteamOwnership(*reachInstall)) {
        NOBLE_ERROR("Steam ownership verification failed. Terminating dedicated server.");
        return 1;
    }

    // 2. Initialize VFS
    noble::VFS::Initialize(*reachInstall, "./dedicated_data");

    // 3. Load haloreach.dll headless
    // Extended engine limits (LIMITS-HANDOFF.md): the server must run the same profile as the players it hosts
    // (the game-state layout differs), so -limits picks it and the console states it. An unknown value, a
    // missing patch file or a different installed DLL build falls back to Stock with the reason logged; the
    // server still starts, and D3's join check keeps clients on the matched profile.
    noble::limits::Profile requestedLimits = noble::limits::Profile::Stock;
    if (!limitsArg.empty()) {
        if (_stricmp(limitsArg.c_str(), "v16") == 0) requestedLimits = noble::limits::Profile::V16;
        else if (_stricmp(limitsArg.c_str(), "stock") != 0)
            NOBLE_WARN("Engine limits: unknown -limits '{}', using Stock", limitsArg);
    }
    const noble::limits::PrepareResult engineDll =
        noble::limits::PrepareEngineDll(reachInstall->reachDllPath, requestedLimits);
    const char* limitsReason = engineDll.reason.empty() ? "none" : engineDll.reason.c_str();
    NOBLE_INFO("Engine limits: requested {}, active {}, reason {}", noble::limits::ProfileName(requestedLimits),
               noble::limits::ProfileName(engineDll.active), limitsReason);
    std::cout << " Engine limits: requested " << noble::limits::ProfileName(requestedLimits) << ", active "
              << noble::limits::ProfileName(engineDll.active) << ", reason " << limitsReason << "\n";

    auto loader = std::make_unique<noble::Loader>();
    if (!loader->LoadReachDll(engineDll.dllPath, reachInstall->reachDllPath)) {
        NOBLE_ERROR("Failed to load haloreach.dll into dedicated server address space.");
        return 1;
    }
    noble::VFS::HookTargetModule(loader->GetModuleHandle());

    // 4. Lobby & Forge patches disabled per launch configuration
    // noble::LobbyPatches::Apply(loader->GetModuleHandle());
    noble::ForgePatches::Apply(loader->GetModuleHandle());

    // 5. Load configuration and start server
    noble::DedicatedConfig config = noble::DedicatedConfig::LoadFromFile(configFile);
    noble::DedicatedServer server;
    if (!server.Start(config)) {
        NOBLE_ERROR("Failed to start dedicated server.");
        return 1;
    }

    std::cout << "\nDedicated server active! Type 'help' for console commands, 'status' for stats, or 'quit' to stop.\n\n";

    // 6. Interactive console command loop
    std::string line;
    while (server.IsRunning() && std::getline(std::cin, line)) {
        server.ProcessConsoleCommand(line);
    }

    server.Stop();
    NOBLE_INFO("Dedicated server process terminated cleanly.");
    return 0;
}
