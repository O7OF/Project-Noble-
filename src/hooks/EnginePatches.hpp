#pragma once

#include <windows.h>
#include <cstdint>

namespace noble {

class EnginePatches {
public:
    // Applies engine stability patches to haloreach.dll (e.g. whitespace underflow fix)
    static bool Apply(HMODULE hReachDll);

    // Queues a one-shot weapon lower / raise toggle (Alert Carry, bit 10 of player control flags)
    // to be consumed on the next PlayerControlUpdate tick for local player 0.
    static void RequestLowerWeaponToggle();

    static bool IsApplied() { return s_isApplied; }

private:
    static bool s_isApplied;
};

} // namespace noble
