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
//   F7 in a match            place every AI squad the map defines (Firefight/campaign maps; Forge World has
//                            none). Press again for more; with -ai-actors N the actor pool holds N, stock 96.
//   F8 in a match            crowd of 128 copies of your Spartan in front of you, on/off (render test for
//                            128 players; the engine has only 16 player slots, so these are bodies, and
//                            ones the engine cleans up as corpses are put back twice a second)
//   -spawn-test <count>      spawn <count> automatically once a match is running
//   -spawn-max <objects>     stop when this many objects are in use (default 8,000; V16 with the
//                            scheduler guard tops out near 8,800 moving objects)
//
// Progress, refusals and the object pool's live usage go to the log ("SpawnTest: ..."). On Stock,
// object_new starts refusing near 2,048 objects; with V16 it continues toward 32,767.
#include <windows.h>

namespace noble::spawn {

// Development builds only (no-op otherwise). Call once after haloreach.dll is loaded.
void Install(HMODULE engine);

} // namespace noble::spawn
