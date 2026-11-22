#pragma once
// Development-build object spawn test (LIMITS-TODO, "prove the limits under load").
//
// Creates objects straight through the engine (object_placement_data_new 0x49CC24 + object_new 0x46D6E0),
// the way GreenKnight's 3,000-biped test did, so Forge's 651-item map-variant list does not apply. Objects
// are copies of a weapon already in the match (or a biped if there is none), laid out in a grid above it.
// The work runs on the engine's game thread from a hook on its per-player tick (0x61A38), because the
// engine keeps game state in thread-local storage.
//
//   Numpad 9 in a match      spawn 500 more objects
//   Numpad 7 in a match      place every AI squad the map defines (Firefight/campaign maps; Forge World has
//                            none). Press again for more; with -ai-actors N the actor pool holds N, stock 96.
//   Numpad 8 in a match      crowd of 128 copies of your Spartan in front of you, on/off (render test for
//                            128 players; the engine has only 16 player slots, so these are bodies, and
//                            ones the engine cleans up as corpses are put back twice a second)
//   -spawn-test <count>      spawn <count> automatically once a match is running
//   -spawn-max <objects>     stop when this many objects are in use (default 8,000; V16 with the
//                            scheduler guard tops out near 8,800 moving objects)
//
// Progress, refusals and the object pool's live usage go to the log ("SpawnTest: ..."). On Stock,
// object_new starts refusing near 2,048 objects; with V16 it continues toward 32,767.
#include <windows.h>
#include <cstdint>
#include <functional>
#include <string>

namespace noble::spawn {

// Firefight allies (game thread only): runs `place` (one engine squad placement), then moves every actor it created
// into scenario squad `destSquad` with that squad's team and walks them next to the local player. Returns the number
// of actors created, -1 if the engine state was not readable.
int PlaceAllies(const std::function<bool()>& place, int destSquad);
// Allies from PlaceAllies still alive (game thread). 0 clears the list.
int AlliesAlive();
// Live actor pool usage (game thread). False if the pool is not readable.
bool ActorsInUse(int& used, int& capacity);

// MAP AI (Phase 7). ai_place refuses outside campaign / Firefight; this lets it run in custom games and Forge by
// changing only ai_place's own game_is_campaign_or_survival call (hr+0x6C2B2B). Idempotent; true once patched.
bool EnableMpAiPlacement();
// True when the loaded map has AI paths: a baked nav mesh loaded for one of its BSPs and at least one squad
// (game thread). Stock MP maps have neither.
bool MapHasAiPaths();
// ai_place for every squad the map defines (game thread). Returns the number of new actors, -1 if not possible.
int PlaceMapSquads();
// Places one map squad (by name) at a point: its spawn points are moved to a 1.2 m ring around `pos` for the
// placement and then restored (game thread). Returns the number of new actors, -1 if not possible. The building
// block for AI placed from the Forge menu.
int PlaceSquadAt(const char* squadName, const float pos[3]);
// Forge pause-menu "PLACE AI" (any thread): queues one squad of `kind` (0 Elites, 1 Grunts, 2 Jackals, 3 Brutes,
// 4 Troopers; the map's noble_ai_* squads) to be placed 6 m in front of the local player on the next game tick.
void RequestPlaceAiInFront(int kind);
// The last PLACE AI result line (empty before the first); `serial` changes with every new result.
std::string PlaceAiResult(uint32_t& serial);

// Development builds only (no-op otherwise). Call once after haloreach.dll is loaded.
void Install(HMODULE engine);

void TriggerAiHorde(int count = 25);
void TriggerPlaceAllSquads();
void TriggerCrowdToggle();
void TriggerSpawnObjects(int count = 50);

} // namespace noble::spawn
