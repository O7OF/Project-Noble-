#pragma once

#include <string>

namespace noble {

// Player-facing names for map scenarios. The engine and the master server speak in scenario names
// ("forge_halo", "20_sword_slayer", "m10_b", "dlc_slayer.map"), which is not what a player should read in the
// server browser or anywhere else. This maps them to the names the menus use, and prettifies modded maps that are
// not in the table ("my_custom_map.map" -> "My Custom Map").
namespace MapNames {

// Display name for a scenario / map file name. Never empty: unknown names are prettified.
std::string Display(const std::string& scenarioOrFile);

// True when `name` (a scenario or a map file) is one of Halo: Reach's own maps.
bool IsVanilla(const std::string& scenarioOrFile);

// The canonical scenario key for a display name or file name, or an empty string when it is unknown. Used by the
// server browser so searching for "Sword Base" also finds "20_sword_slayer".
std::string ScenarioOf(const std::string& name);

// Headless check of the table above (run by `-test-map-gametypes`). Returns the number of failures.
int SelfTest();

} // namespace MapNames
} // namespace noble
