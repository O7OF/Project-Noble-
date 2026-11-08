#include "OwnershipVerifier.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <fstream>
#include <string>

namespace noble {

bool OwnershipVerifier::VerifySteamOwnership(ReachInstallInfo& info) {
    NOBLE_INFO("Verifying Steam ownership tokens for Halo: Reach (AppID: {}, DLC AppID: {})...", 
        STEAM_MCC_APP_ID, STEAM_REACH_DLC_APP_ID);

    // Locate appmanifest_976730.acf in parent steamapps directory
    fs::path steamAppsDir = info.mccRootPath.parent_path().parent_path(); // .../steamapps/common/... -> .../steamapps
    fs::path manifestPath = steamAppsDir / ("appmanifest_" + std::to_string(STEAM_MCC_APP_ID) + ".acf");

    if (!fs::exists(manifestPath)) {
        NOBLE_ERROR("Steam app manifest not found at: {}", manifestPath.string());
        MessageBoxW(nullptr,
            L"Steam Ownership Verification Failed:\n\n"
            L"Could not locate Steam manifest for Halo: The Master Chief Collection.\n"
            L"You must legitimately own MCC and the Halo: Reach DLC on Steam to run Project Noble.",
            L"Project Noble - Ownership Required",
            MB_ICONERROR | MB_OK);
        return false;
    }

    // Read manifest content
    std::ifstream manifestFile(manifestPath);
    if (!manifestFile.is_open()) {
        NOBLE_ERROR("Failed to open Steam app manifest.");
        return false;
    }

    std::string manifestContent((std::istreambuf_iterator<char>(manifestFile)),
                                 std::istreambuf_iterator<char>());

    // Verify MCC AppID
    if (manifestContent.find(std::to_string(STEAM_MCC_APP_ID)) == std::string::npos) {
        NOBLE_ERROR("AppID {} not found in manifest.", STEAM_MCC_APP_ID);
        return false;
    }

    // Verify Reach DLC (AppID 1064220 or depot 976731)
    bool hasReachDlc = (manifestContent.find(std::to_string(STEAM_REACH_DLC_APP_ID)) != std::string::npos) ||
                       (manifestContent.find(std::to_string(STEAM_REACH_DEPOT_ID)) != std::string::npos);

    if (!hasReachDlc) {
        NOBLE_ERROR("Halo: Reach DLC token (AppID {}) not detected in Steam manifest.", STEAM_REACH_DLC_APP_ID);
        MessageBoxW(nullptr,
            L"Steam Ownership Verification Failed:\n\n"
            L"Halo: Reach DLC was not detected in your Steam library.\n"
            L"Project Noble strictly supports Halo: Reach. Please ensure you own and have installed the Halo: Reach DLC.",
            L"Halo: Reach DLC Required",
            MB_ICONERROR | MB_OK);
        return false;
    }

    info.isOwnedOnSteam = true;
    info.isReachDlcInstalled = true;

    NOBLE_INFO("Steam ownership successfully verified! Halo: Reach DLC is owned and activated.");
    return true;
}

} // namespace noble
