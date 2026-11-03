#pragma once

#include <windows.h>
#include <filesystem>
#include <optional>
#include <string>

namespace noble {

// Loads an Elite armour pack (a trimmed Reach cache file holding a player Elite with separate
// pieces) into a match, next to the match map, the way the engine loads its own extra caches.
//
// Stage A1 (this file): load the pack with the match map, rebase its packed tag pointers from the
// .rrreloc sidecar, and let the engine finish loading it. Players still use the map's Elite.
// Later stages: merge the pack's armour list, point the player Elite at the pack's biped, per-piece picks.
//
// Files (local only, never committed): <data>\armor\<name>.map and <name>.map.rrreloc
// (and .rrneeds / .PMGC for later stages). The pack is only used in multiplayer and Firefight
// matches, never in the menu or campaign. "-no-elite-pack" turns it off.
namespace ElitePack {

// Call once after haloreach.dll is loaded and before the engine starts the match.
void Install(HMODULE engine, const std::filesystem::path& dataDir, const std::string& mode);

// Slot 48 path resolver: the engine asks for "haloreach\maps\<pack>.map"; returns the pack's real path.
std::optional<std::filesystem::path> ResolveFile(const std::wstring& requested);

bool Loaded();   // the engine loaded the pack and its pointers were rebased

} // namespace ElitePack
} // namespace noble
