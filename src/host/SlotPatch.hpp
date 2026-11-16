#pragma once

#include <windows.h>
#include <cstdint>

// C12 (more than 16 real players), step 1: redirects every traced player/peer slot address calculation in
// the engine's network session code so that slots past the original arrays (players >= 16, peers >= 17) land in
// Noble-owned "shadow" session objects. Slots below that keep their exact addresses.
// Opt-in: -slot-patch (NOBLE_SLOT_PATCH=1), Extended profiles only. While the players pool is still 16 the
// redirected path is never taken, so this build must behave exactly like an unpatched one (regression test).
namespace noble::slots {
void Install(HMODULE engine, bool extendedProfile);

// 32-player mode requested or active via extended profile.
bool Players32Requested();

// 32-player mode is installed (every check passed and every patch is in). Pools may only grow when this is true.
bool Players32Active();

// Dev diagnostics: peer record `peer` of the static session object `session` (shadow object for peers >= 17).
uint8_t* PeerRecord(uint8_t* session, int32_t peer);

// Headless synthetic validation of the shadow-slot layouts for 32/64/128 capacity (dev builds, no engine needed):
// shadow array offsets and biases, overlap/stride fit, and which wire limits each capacity needs. Returns the
// number of failed checks (0 = all layouts valid). Prints one line per capacity.
int SlotLayoutSelfTest();
}
