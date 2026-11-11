#pragma once

#include <windows.h>
#include <cstdint>

// Pepper's Pelican Overhaul (PepperMan): pilot controls for the "_scripted_doors" Pelican tags.
// Those tags open/close the rear ramp and landing gear from scripted object function 3, which campaign
// scripts set with (object_function_set 3 1) / (object_function_set 3 0). Noble sets the same value
// directly on a key press while the local player sits in a vehicle.
//
//   X in a vehicle    toggle Pelican ramp + landing gear (scripted object function 3: 0 = closed, 1 = open)
//
// The function is a single game-state value shared by every object that reads it, so all
// "_scripted_doors" Pelicans in the match open and close together. It is set on the machine that
// presses the key; online sync is untested (test local first, then host, then client).
// Dev build only for now: called from the SpawnTest game-tick hook.
namespace noble::pelican {

// Engine game thread, once per player tick. base = haloreach.dll module base.
void GameTick(uintptr_t base);

} // namespace noble::pelican
