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

    // Cutscene skip recovery: clears screen fade, cancels cinematic camera, unlocks input and restores first-person
    static void RecoverFromCutsceneSkip();

    // Frame tick watchdog: ensures screen fade is cleared during gameplay and monitors cutscene states
    static void OnFrameTick();

    // Checks if the engine currently has a cinematic playing
    static bool CinematicInProgress();

    // Determines if GameOver / GameFinish should be blocked to prevent false premature kicks
    static bool ShouldBlockPrematureGameOver();

    // Clears the rasterizer screen fade overlay in memory
    static void ClearRasterizerScreenFade(uintptr_t base = 0);

private:
    static bool s_isApplied;
};

} // namespace noble
