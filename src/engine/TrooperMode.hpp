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

// Test-only instrumentation: with NOBLE_MODEL_DUMP=1 in the environment, once per process, about two seconds after a
// local unit exists, log that unit's model chain and every region's name string-id, permutation count and the
// permutation index/name actually applied. Used to compare a Spartan run with a Trooper (marine) run; no effect
// unless the variable is set. Call every tick (it gates itself).
void DumpModelState();

} // namespace TrooperMode
} // namespace noble
