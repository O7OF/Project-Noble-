#include "ModCatalog.hpp"
#include "common/Logger.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <set>

namespace noble {
namespace ModCatalog {

namespace {

namespace fs = std::filesystem;
using nlohmann::json;

std::string Lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool ReadJson(const fs::path& path, json& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF) text.erase(0, 3);   // UTF-8 BOM
    out = json::parse(text, nullptr, false, true);
    return !out.is_discarded() && out.is_object();
}

// The Steam published-file id MCC puts in ModInfo.json:
//   "ModIdentifier": { "HostedModIds": { "SteamWorkshopId": 1234567890 } }
// Older mod manifests have no ModIdentifier, so the caller may fall back to the folder name.
std::string HostedWorkshopId(const json& info) {
    if (!info.contains("ModIdentifier") || !info["ModIdentifier"].is_object()) return {};
    const json& identifier = info["ModIdentifier"];
    if (!identifier.contains("HostedModIds") || !identifier["HostedModIds"].is_object()) return {};
    const json& hosted = identifier["HostedModIds"];
    if (!hosted.contains("SteamWorkshopId")) return {};
    const json& id = hosted["SteamWorkshopId"];
    if (id.is_string()) return id.get<std::string>();
    if (id.is_number_unsigned()) return std::to_string(id.get<uint64_t>());
    if (id.is_number_integer() && id.get<int64_t>() > 0) return std::to_string(id.get<int64_t>());
    return {};
}

bool AllDigits(const std::string& text) {
    return !text.empty() && text.size() <= 20 &&
           std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
}

// MCC localised text: { "Neutral": "..." } or a plain string.
std::string Text(const json& j, const char* key) {
    if (!j.contains(key)) return {};
    const json& v = j[key];
    if (v.is_string()) return v.get<std::string>();
    if (v.is_object()) {
        if (v.contains("Neutral") && v["Neutral"].is_string()) return v["Neutral"].get<std::string>();
        for (const auto& item : v.items()) if (item.value().is_string()) return item.value().get<std::string>();
    }
    return {};
}

std::vector<std::string> StringList(const json& j, const char* key) {
    std::vector<std::string> out;
    if (j.contains(key) && j[key].is_array()) {
        for (const json& v : j[key]) if (v.is_string()) out.push_back(v.get<std::string>());
    }
    return out;
}

bool ReadMap(const fs::path& file, ModMap& map) {
    json j;
    if (!ReadJson(file, j)) return false;
    std::string fullScenario = j.value("ScenarioFile", std::string());
    if (fullScenario.empty()) return false;
    std::string norm = fullScenario;
    for (char& c : norm) if (c == '\\') c = '/';
    const size_t slash = norm.find_last_of('/');
    if (slash != std::string::npos) {
        map.scenario = Lower(norm.substr(slash + 1));
        std::string parent = norm.substr(0, slash);
        const size_t parentSlash = parent.find_last_of('/');
        if (parentSlash != std::string::npos) parent = parent.substr(parentSlash + 1);
        map.baseScenario = Lower(parent);
    } else {
        map.scenario = Lower(norm);
    }
    map.title = Text(j, "Title");
    map.description = Text(j, "Description");
    map.forgeMap = false;
    for (const std::string& flag : StringList(j, "Flags")) {
        if (Lower(flag) == "forgemap") map.forgeMap = true;
    }
    if (j.contains("Images") && j["Images"].is_object()) {
        map.imageLarge = std::filesystem::path(Text(j["Images"], "Large"));
        map.imageThumbnail = std::filesystem::path(Text(j["Images"], "Thumbnail"));
        map.imageLoading = std::filesystem::path(Text(j["Images"], "LoadingScreen"));
    }
    if (map.title.empty()) map.title = map.scenario;
    return true;
}

// The map JSONs of one kind: the ones the manifest lists, plus any other <folder>\*.json.
std::vector<ModMap> ReadMaps(const fs::path& mod, const std::vector<std::string>& listed, const char* folder) {
    std::vector<fs::path> files;
    for (const std::string& rel : listed) files.push_back(mod / fs::path(rel).make_preferred());
    std::error_code ec;
    for (const auto& e : fs::directory_iterator(mod / folder, ec)) {
        const std::string name = Lower(e.path().filename().string());
        if (e.is_regular_file(ec) && e.path().extension() == ".json" && name.find(".scenario.") == std::string::npos) {
            files.push_back(e.path());
        }
    }
    std::vector<ModMap> maps;
    std::set<std::string> seen;
    for (const fs::path& f : files) {
        ModMap m;
        if (!ReadMap(f, m) || !seen.insert(m.scenario).second) continue;
        maps.push_back(std::move(m));
    }
    return maps;
}

void ScanFolder(const fs::path& root, bool workshop, std::vector<ModEntry>& out) {
    std::error_code ec;
    if (!fs::is_directory(root, ec)) return;
    for (const auto& dir : fs::directory_iterator(root, ec)) {
        if (!dir.is_directory(ec)) continue;
        try {
        json info;
        const bool hasInfo = ReadJson(dir.path() / "ModInfo.json", info);
        const std::string engine = hasInfo ? Lower(info.value("Engine", std::string())) : std::string();
        if (workshop && engine.find("reach") == std::string::npos) continue;   // other games' Workshop items
        if (!workshop && !engine.empty() && engine.find("reach") == std::string::npos) continue;

        ModEntry mod;
        mod.id = dir.path().filename().string();
        mod.folder = dir.path();
        mod.workshopId = hasInfo ? HostedWorkshopId(info) : std::string();
        // Workshop folders are named after the published-file id. Some older manifests omit ModIdentifier;
        // the folder name still identifies the item in that case.
        if (mod.workshopId.empty() && workshop && AllDigits(mod.id)) mod.workshopId = mod.id;
        mod.title = hasInfo ? Text(info, "Title") : std::string();
        mod.description = hasInfo ? Text(info, "Description") : std::string();
        if (mod.title.empty()) mod.title = workshop ? "Workshop item " + mod.id : mod.id;

        json contents = hasInfo && info.contains("GameModContents") && info["GameModContents"].is_object()
                            ? info["GameModContents"] : json::object();
        std::vector<std::string> campaignList = StringList(contents, "CampaignMaps");
        json campaignInfo;
        if (ReadJson(dir.path() / "CampaignInfo.json", campaignInfo)) {
            for (const std::string& s : StringList(campaignInfo, "CampaignMaps")) campaignList.push_back(s);
        }
        mod.campaign = ReadMaps(dir.path(), campaignList, "campaign");
        mod.firefight = ReadMaps(dir.path(), StringList(contents, "FirefightMaps"), "firefight");
        mod.multiplayer = ReadMaps(dir.path(), StringList(contents, "MultiplayerMaps"), "multiplayer");
        auto markMapFiles = [&](std::vector<ModMap>& maps) {
            for (ModMap& m : maps) {
                m.hasMapFile = fs::is_regular_file(dir.path() / "maps" / (m.scenario + ".map"), ec);
            }
        };
        markMapFiles(mod.campaign);
        markMapFiles(mod.firefight);
        markMapFiles(mod.multiplayer);
        auto markImages = [&](std::vector<ModMap>& maps) {
            for (ModMap& m : maps) {
                auto resolve = [&](fs::path& p) {
                    if (p.empty()) return;
                    if (p.is_relative()) p = dir.path() / p;
                    if (!fs::is_regular_file(p, ec)) p.clear();
                };
                resolve(m.imageLarge);
                resolve(m.imageThumbnail);
                resolve(m.imageLoading);
            }
        };
        markImages(mod.campaign);
        markImages(mod.firefight);
        markImages(mod.multiplayer);
        for (const auto& e : fs::directory_iterator(dir.path() / "maps", ec)) {
            if (e.is_regular_file(ec) && Lower(e.path().extension().string()) == ".map") {
                mod.mapFiles.push_back(Lower(e.path().stem().string()));
            }
        }
        if (mod.campaign.empty() && mod.firefight.empty() && mod.multiplayer.empty() && mod.mapFiles.empty()) continue;
        out.push_back(std::move(mod));
        } catch (const std::exception& e) {
            NOBLE_WARN("ModCatalog: skipping {} ({})", dir.path().string(), e.what());
        }
    }
}

} // namespace

const ModEntry* FindByWorkshopId(const std::vector<ModEntry>& mods, const std::string& workshopId) {
    if (workshopId.empty()) return nullptr;
    for (const ModEntry& mod : mods) {
        if (!mod.workshopId.empty() && mod.workshopId == workshopId) return &mod;
        if (mod.id == workshopId) return &mod;
    }
    return nullptr;
}

std::vector<ModEntry> Scan(const fs::path& workshopPath, const fs::path& localModsPath) {
    std::vector<ModEntry> mods;
    try {
        ScanFolder(workshopPath, true, mods);
        ScanFolder(localModsPath, false, mods);
    } catch (const std::exception& e) {
        NOBLE_WARN("ModCatalog: scanning mods failed: {}", e.what());
    }
    std::sort(mods.begin(), mods.end(), [](const ModEntry& a, const ModEntry& b) { return Lower(a.title) < Lower(b.title); });
    NOBLE_INFO("ModCatalog: {} Halo: Reach mods", mods.size());
    return mods;
}

} // namespace ModCatalog
} // namespace noble
