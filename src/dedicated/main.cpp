#include "DedicatedServer.hpp"
#include "common/Logger.hpp"
#include "common/Version.hpp"
#include "host/SteamLocator.hpp"
#include "host/OwnershipVerifier.hpp"
#include "host/Loader.hpp"
#include "vfs/VFS.hpp"
#include "hooks/LobbyPatches.hpp"
#include "hooks/ForgePatches.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    noble::Logger::Init("noble-dedicated.log");

    std::cout << "=============================================================\n";
    std::cout << " Halo: Reach Community Edition - Headless Dedicated Server\n";
    std::cout << " Version: " << noble::VERSION_STRING << "\n";
    std::cout << "=============================================================\n";

    std::string configFile = "dedicated_server.json";
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-config" && i + 1 < argc) {
            configFile = argv[++i];
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
    auto loader = std::make_unique<noble::Loader>();
    if (!loader->LoadReachDll(reachInstall->reachDllPath)) {
        NOBLE_ERROR("Failed to load haloreach.dll into dedicated server address space.");
        return 1;
    }
    noble::VFS::HookTargetModule(loader->GetModuleHandle());

    // 4. Apply lobby expansion patches (expand from 16 to 32/64 players)
    noble::LobbyPatches::Apply(loader->GetModuleHandle());
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
