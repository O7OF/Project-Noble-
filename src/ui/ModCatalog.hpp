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
    std::string baseScenario; // e.g. "forge_halo" (parent directory from ScenarioFile, lower case)
    std::string title;
    std::string description;
    bool forgeMap = false;    // the map JSON lists the "ForgeMap" flag
    bool hasMapFile = false;  // maps\<scenario>.map exists in the mod folder (checked once at scan time)
    // The rest of MCC's map manifest, kept so the launcher can act on it (the engine never reads these JSONs;
    // MCC does, and uses them for a map's category and Forge palette). See ModCatalog::Scan.
    std::vector<uint32_t> validObjectTypes;   // "ValidMultiplayerObjectTypes": objects a Forge palette offers
    std::string defaultForgeWeapon;           // "MapDefaultPrimaryWeaponForge"
    std::string gameCategory;                 // "campaign" / "firefight" / "multiplayer", from the JSON folder
    // The mod's own art (Images.Large / Thumbnail / LoadingScreen in the map JSON), absolute and
    // checked to exist. Empty when the mod has no picture for that slot.
    std::filesystem::path imageLarge, imageThumbnail, imageLoading;
};

struct ModEntry {
    std::string id;           // Workshop id or folder name
    std::string workshopId;   // Steam published-file id from ModIdentifier.HostedModIds (empty for local-only mods)
    std::string title;
    std::string description;
    std::filesystem::path folder;
    std::vector<ModMap> campaign, firefight, multiplayer;
    std::vector<std::string> mapFiles;   // scenario names of every maps\*.map in the mod (lower case)
};

namespace ModCatalog {

std::vector<ModEntry> Scan(const std::filesystem::path& workshopPath, const std::filesystem::path& localModsPath);

// Finds an installed mod by Steam published-file id, or by its folder name for local mods. nullptr when
// the player does not have it (the lobby's Download & Join prompt then offers it).
const ModEntry* FindByWorkshopId(const std::vector<ModEntry>& mods, const std::string& workshopId);

} // namespace ModCatalog
} // namespace noble
