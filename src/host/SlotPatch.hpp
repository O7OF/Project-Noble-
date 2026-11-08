#pragma once

#include <windows.h>

// C12 (more than 16 real players), step 1: redirects every traced player/peer slot address calculation in
// the engine's network session code so that slots past the original arrays (players >= 16, peers >= 17) land in
// Noble-owned "shadow" session objects. Slots below that keep their exact addresses.
// Opt-in: -slot-patch (NOBLE_SLOT_PATCH=1), Extended profiles only. While the players pool is still 16 the
// redirected path is never taken, so this build must behave exactly like an unpatched one (regression test).
namespace noble::slots {
void Install(HMODULE engine, bool extendedProfile);
}
