#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace noble {

namespace fs = std::filesystem;

// Core Reach identification constants
constexpr uint32_t STEAM_MCC_APP_ID = 976730;
constexpr uint32_t STEAM_REACH_DLC_APP_ID = 1064220;
constexpr uint32_t STEAM_REACH_DEPOT_ID = 976731;

struct ReachInstallInfo {
    fs::path mccRootPath;          // e.g. "C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection"
    fs::path reachDirectory;       // e.g. "<mccRoot>\haloreach"
    fs::path reachDllPath;         // e.g. "<reachDirectory>\haloreach.dll"
    fs::path mapsDirectory;        // e.g. "<reachDirectory>\maps"
    fs::path fmodDirectory;        // e.g. "<reachDirectory>\fmod"
    fs::path workshopDirectory;    // e.g. "C:\...\steamapps\workshop\content\976730"
    bool isOwnedOnSteam = false;
    bool isReachDlcInstalled = false;
    bool areFilesVerified = false;
};

#pragma pack(push, 1)
struct ReachLibrarySettings {
    wchar_t gameRoot[84];      // Reach directory e.g. "...\haloreach"
    wchar_t mapsDirectory[84]; // Maps directory e.g. "...\haloreach\maps"
    wchar_t userDirectory[87]; // Local data directory e.g. "data"
};
#pragma pack(pop)
static_assert(sizeof(ReachLibrarySettings) == 510, "ReachLibrarySettings must be 510 bytes");

// Function prototypes for haloreach.dll exports
typedef void* (*CreateGameEngineFn)(void* host_interface);
typedef void* (*CreateDataAccessFn)();
typedef void  (*SetLibrarySettingsFn)(const void* settings);

struct ReachExports {
    CreateGameEngineFn   CreateGameEngine = nullptr;
    CreateDataAccessFn   CreateDataAccess = nullptr;
    SetLibrarySettingsFn SetLibrarySettings = nullptr;

    bool IsValid() const {
        return CreateGameEngine != nullptr &&
               CreateDataAccess != nullptr &&
               SetLibrarySettings != nullptr;
    }
};

} // namespace noble
