#pragma once

#include <windows.h>
#include <string>

namespace noble {

// Trooper mode (prototype, 2026-11-14): "-trooper" makes the campaign player a UNSC Marine. After the mission map's
// tags load, every player representation in the globals that uses the Spartan biped is pointed at the map's own
// marine biped (objects\characters\marine\marine). Stock maps are compressed, so the marine's tag index per mission
// comes from a table built offline with Reclaimer (tools rc_trooper.fsx); the index is checked against the live
// tag table (must be a biped) before anything is changed. Cutscenes keep their own actors.
namespace TrooperMode {

// Call once after haloreach.dll is loaded and before the engine starts the match. Returns true when Trooper mode is
// on for this mission (the caller locks the camera to third person: the marine has no first-person arms).
bool Install(HMODULE engine, const std::string& map, const std::string& mode);

bool Active();   // the player representations were switched to the marine

} // namespace TrooperMode
} // namespace noble
