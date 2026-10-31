#pragma once

#include <windows.h>

namespace noble {

// Third-person camera toggle for the local player.
//
// Reach's camera mode is gated by a small engine function at haloreach+0x48A910 (verified prologue
// 48 89 5C 24 08 48 89 74 24 10); it takes a player index and answers whether that player's camera may use the
// third-person path. Reach Renaissance hooks exactly this function (and a follow/camera update at +0x1DCADC for
// distance and shoulder tuning), so a detour that answers "yes" is enough to let the engine's own third-person
// camera run.
//
// F5 toggles it in game; -third-person starts with it on. The engine's follow camera needs no extra offset for the
// default distance - the +0x1DCADC hook is only needed if we want to tune distance / swap shoulders later.
class CameraThirdPerson {
public:
    // Installs the detour on the engine module. Safe to call once after haloreach.dll is loaded.
    static bool Install(HMODULE haloreach);

    static void SetEnabled(bool enabled);
    static void SetMode(int mode);   // experiment switch
    static void SetDistancePercent(int percent);   // 100 = engine default, >100 pulls the camera back
    static bool IsEnabled();
    // True when the engine itself is asking for the third-person path (Forge's monitor/editor mode).
    static bool EngineWantsThirdPerson();
    static void Toggle();

    // Called from the input poll: F5 edge-toggles the camera.
    static void PollHotkey(bool f5Down);

    // Third-person weapon aim zoom (held via right-click in third person).
    static void SetZoomHeld(bool held);
    static bool IsZoomHeld();
    // Current FOV multiplier (1.0 when unzoomed or in first person; < 1.0 while zooming in third person).
    static float EffectiveFovScale();
};

} // namespace noble
