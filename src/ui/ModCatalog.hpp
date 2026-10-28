#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace noble {

// The Halo: Reach mods a player can pick when starting a campaign, Firefight or custom game.
//
// MCC mods (Steam Workshop items for app 976730, or folders in Noble's data\mods) are a folder with
// ModInfo.json ("Engine": "HaloReach"), maps\<scenario>.map, and one small JSON per map in campaign\,
// firefight\ or multiplayer\ ({"ScenarioFile": "levels/.../ff60_icecave", "Title": {"Neutral": ...},
// "Description": {...}}). Campaign mods may list their maps in CampaignInfo.json ("CampaignMaps").
// When a mod is chosen, VFS::SetActiveMod makes the engine read the mod's files before MCC's.
struct ModMap {
    std::string scenario;     // e.g. "ff60_icecave" (the map file name, lower case)
    std::string title;
    std::string description;
};

struct ModEntry {
    std::string id;           // Workshop id or folder name
    std::string title;
    std::string description;
    std::filesystem::path folder;
    std::vector<ModMap> campaign, firefight, multiplayer;
    std::vector<std::string> mapFiles;   // scenario names of every maps\*.map in the mod (lower case)
};

namespace ModCatalog {

std::vector<ModEntry> Scan(const std::filesystem::path& workshopPath, const std::filesystem::path& localModsPath);

} // namespace ModCatalog
} // namespace noble
