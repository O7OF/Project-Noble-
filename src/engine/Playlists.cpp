#include "engine/Playlists.hpp"

#include "common/Logger.hpp"
#include "vfs/VFS.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>

namespace noble {
namespace Playlists {

namespace {
constexpr size_t kMaxEntries = 100;
std::string BareName(const std::string& text) { return std::filesystem::path(text).filename().string(); }
}

std::filesystem::path Directory() { return VFS::GetUserStoragePath() / "playlists"; }

bool Load(const std::filesystem::path& file, Playlist& out) {
    std::ifstream in(file, std::ios::binary);
    if (!in) return false;
    try {
        const nlohmann::json j = nlohmann::json::parse(in);
        Playlist playlist;
        playlist.name = j.value("name", file.stem().string());
        playlist.shuffle = j.value("shuffle", false);
        playlist.randomType = j.value("random_type", false);
        playlist.autoStart = std::clamp(j.value("auto_start", 0), 0, 300);
        playlist.file = file;
        if (j.contains("entries") && j["entries"].is_array()) {
            for (const auto& item : j["entries"]) {
                if (!item.is_object() || playlist.entries.size() >= kMaxEntries) continue;
                Entry entry;
                entry.map = BareName(item.value("map", std::string()));
                entry.variant = BareName(item.value("variant", std::string()));
                entry.mod = BareName(item.value("mod", std::string()));
                if (!entry.map.empty()) playlist.entries.push_back(std::move(entry));
            }
        }
        out = std::move(playlist);
        return true;
    } catch (const std::exception& e) {
        NOBLE_WARN("Playlists: '{}' is not a valid playlist ({})", file.string(), e.what());
        return false;
    }
}

std::filesystem::path PathFor(const std::string& playlistName) {
    std::string name;
    for (const char c : playlistName) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '-' || c == '_') name.push_back(c);
    }
    while (!name.empty() && name.back() == ' ') name.pop_back();
    return name.empty() ? std::filesystem::path() : Directory() / (name + ".json");
}

bool Save(const Playlist& playlist, std::filesystem::path& savedAs) {
    if (PathFor(playlist.name).empty()) return false;
    std::error_code ec;
    std::filesystem::create_directories(Directory(), ec);
    nlohmann::json j;
    j["name"] = playlist.name;
    j["shuffle"] = playlist.shuffle;
    j["random_type"] = playlist.randomType;
    j["auto_start"] = playlist.autoStart;
    j["entries"] = nlohmann::json::array();
    for (const Entry& e : playlist.entries) j["entries"].push_back({ { "map", e.map }, { "variant", e.variant }, { "mod", e.mod } });
    savedAs = PathFor(playlist.name);
    std::ofstream out(savedAs, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << j.dump(2);
    return static_cast<bool>(out);
}

std::vector<Playlist> Scan(const std::vector<std::filesystem::path>& modFolders) {
    std::vector<Playlist> list;
    std::error_code ec;
    std::vector<std::filesystem::path> folders = { Directory() };
    for (const std::filesystem::path& mod : modFolders) folders.push_back(mod / "playlists");
    for (const std::filesystem::path& folder : folders) {
        for (const auto& entry : std::filesystem::directory_iterator(folder, ec)) {
            if (_wcsicmp(entry.path().extension().c_str(), L".json") != 0) continue;
            Playlist playlist;
            if (Load(entry.path(), playlist) && !playlist.entries.empty()) list.push_back(std::move(playlist));
        }
        ec.clear();
    }
    std::sort(list.begin(), list.end(), [](const Playlist& a, const Playlist& b) { return a.name < b.name; });
    return list;
}

} // namespace Playlists
} // namespace noble
