#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace noble {

// A host's or map creator's playlist: named entries of map and game mode, played in order or shuffled, optionally with a
// random game type on each map. Saved as JSON in data\playlists; a mod can ship its own in a playlists folder.
// Entries hold bare names only (scenario, variant file name, mod folder name) and are looked up against what this PC has,
// so a playlist from a mod or another player can't point at a path.
namespace Playlists {

struct Entry {
    std::string map;        // scenario name
    std::string variant;    // file name in game_variants or game_modes
    std::string mod;        // mod folder name; empty = a stock map
};

struct Playlist {
    std::string name;
    std::vector<Entry> entries;
    bool shuffle = false;
    bool randomType = false;
    std::filesystem::path file;
};

std::filesystem::path Directory();
std::vector<Playlist> Scan(const std::vector<std::filesystem::path>& modFolders);
bool Load(const std::filesystem::path& file, Playlist& out);
bool Save(const Playlist& playlist, std::filesystem::path& savedAs);

} // namespace Playlists
} // namespace noble
