#pragma once
// Development-build object spawn test (LIMITS-TODO, "prove the limits under load").
//
// Creates objects straight through the engine (object_placement_data_new 0x49CC24 + object_new 0x46D6E0),
// the way GreenKnight's 3,000-biped test did, so Forge's 651-item map-variant list does not apply. Objects
// are copies of a weapon already in the match (or a biped if there is none), laid out in a grid above it.
// The work runs on the engine's game thread from a hook on its per-player tick (0x61A38), because the
// engine keeps game state in thread-local storage.
//
//   F9 in a match            spawn 500 more objects
//   -spawn-test <count>      spawn <count> automatically once a match is running
//   -spawn-max <objects>     stop when this many objects are in use (default 6,000: V16 runs out of
//                            scheduler jobs near 6,500 moving objects and the engine crashes)
//
// Progress, refusals and the object pool's live usage go to the log ("SpawnTest: ..."). On Stock,
// object_new starts refusing near 2,048 objects; with V16 it continues toward 32,767.
#include <windows.h>

namespace noble::spawn {

// Development builds only (no-op otherwise). Call once after haloreach.dll is loaded.
void Install(HMODULE engine);

} // namespace noble::spawn
