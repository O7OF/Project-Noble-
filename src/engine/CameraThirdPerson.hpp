#pragma once

#include <windows.h>

namespace noble {

// Third-person camera toggle, vehicle first-person toggle, and camera FOV / offset modifiers for the local player.
//
// Reach's camera mode is gated by:
// - haloreach+0x48A910 (per-player third-person mode check, verified prologue 48 89 5C 24 08 48 89 74 24 10)
// - haloreach+0xC6948 (unit_should_use_third_person_camera, which forces 3rd person when the unit is seated in a vehicle)
// - haloreach+0xC6EE8 (c_first_person_camera::update, which writes the 1st-person field_of_view in radians to [observerCommand + 0x28])
// - haloreach+0x1DCADC / +0x1E2028 (c_orbiting_camera::update / c_following_camera::update for 3rd-person distance, offset, and zoom)
class CameraThirdPerson {
public:
    // Installs the detours on the engine module. Safe to call once after haloreach.dll is loaded.
    static bool Install(HMODULE haloreach);

    static void SetEnabled(bool enabled);
    static void SetMode(int mode);   // experiment switch
    static void SetDistancePercent(int percent);   // 100 = engine default, >100 pulls the camera back
    static bool IsEnabled();
    // True when the engine itself is asking for the third-person path (Forge's monitor/editor mode).
    static bool EngineWantsThirdPerson();
    static void Toggle();

    // Called from the input poll: edge-toggles the third-person camera (bound in SETTINGS > CONTROLS > KEYBOARD & MOUSE, unbound by default).
    static void PollHotkey(bool keyDown);

    // Vehicle 1st-person camera toggle (bound in SETTINGS > CONTROLS > KEYBOARD & MOUSE, unbound by default).
    static void SetVehicleFirstPerson(bool enabled);
    static bool IsVehicleFirstPerson();
    static void ToggleVehicleFirstPerson();
    static void PollVehicleFirstPersonHotkey(bool keyDown);
    static bool IsInVehicle();

    // Third-person weapon aim zoom (held via Zoom keybind in third person).
    static void SetZoomHeld(bool held);
    static bool IsZoomHeld();
    // Current FOV multiplier (1.0 when unzoomed or in first person; < 1.0 while zooming in third person).
    static float EffectiveFovScale();
};

} // namespace noble
