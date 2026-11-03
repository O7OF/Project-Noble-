#pragma once

#include <windows.h>
#include <cstdint>

namespace noble {

// Halo: Reach's camera field of view, in haloreach.dll's data at RVA 0xB42C10 (float, degrees;
// 78.0f in the shipped image). The engine's camera maths multiplies it by pi/180 and only accepts
// 1..150 degrees, and its own settings refresh re-applies the per-user block value (78) unless the
// global was changed from outside - so Noble writes its setting here and re-asserts it periodically.
// Evidence: engine camera/FOV analysis.
//
// Apply/Tick/Read run on the engine thread (ReachHost slot 1); SetEngineModule runs once from main.
class CameraFov {
public:
    static constexpr float kMinDegrees = 1.0f;    // engine validator: 0.017453292 rad
    static constexpr float kMaxDegrees = 150.0f;  // engine validator: 2.6179938 rad
    static constexpr uint32_t kFovRva = 0xB42C10; // haloreach.dll+0xB42C10

    // haloreach.dll base address. Until this is called Apply() is a no-op.
    static void SetEngineModule(HMODULE module);

    // Writes `degrees` (clamped to the engine's 1..150 range) into the engine global.
    // Returns false when the module is not known yet or the write faulted.
    static bool Apply(float degrees);

    // Reads the value the engine currently holds (degrees). Returns false when unavailable.
    static bool Read(float& degrees);

    // Engine thread, every frame: applies Settings::fieldOfView when it changed and re-asserts it
    // every couple of seconds (the engine refreshes its camera settings block from the profile).
    static void Tick();
};

} // namespace noble
