#pragma once

#include <cstdint>

namespace noble {

// The maths behind the mouse and pad settings, pulled out of ReachHost so it can be tested without the engine.
// Every function here is driven by a value from SettingsData; the self-test proves that changing that value changes
// the output, which is what "the setting does something" means.
namespace InputTuning {

// Radial dead zone with rescaling: input inside `zone` becomes 0, and the edge of the zone still reaches full speed.
// `zone` is 0..0.95; 0 disables it. Values are -32768..32767.
void ApplyDeadZone(int16_t& x, int16_t& y, float zone);

// Stick layout: 0 = Default (left moves, right looks), 1 = Southpaw (swapped). Other values do nothing.
void ApplyStickLayout(int layout, int16_t& leftX, int16_t& leftY, int16_t& rightX, int16_t& rightY);

// Controller look speed, 1.00..10.00, with 2.00 = Reach's stock speed. Measured with -fake-pad and the HUD compass:
//   Below 2.00 uses Reach's profile value, whose turn rate is linear (18.5 deg + 2.42 deg per step over the test). Higher values
//   than the stock 18 turn slower, and mouse look is ignored while a controller is active, so neither can go above stock.
//   Above 2.00 the stick is boosted instead: partial pushes turn faster, up to what a full push does at stock.
int LookProfileByte(float speed);    // 0..18; 18 at 2.00 and above
float LookStickGain(float speed);    // 1 up to 2.00, then speed / 2
// Boosts one stick axis by `gain`, leaving the engine's own dead zone alone so a worn stick doesn't start the view moving.
int16_t ApplyLookGain(int16_t value, float gain);

// Mouse look curve, applied to the per-frame cursor delta (in pixels).
//   smoothing 0..0.9: exponential average; 0 passes the delta through untouched.
//   acceleration 0..1: fast flicks turn further; 0 passes the delta through untouched.
// `smoothX`/`smoothY` keep the filter state between frames (zero them when a menu owns the pointer).
void ApplyMouseCurve(float& dx, float& dy, float acceleration, float smoothing, float& smoothX, float& smoothY);
void ResetMouseCurve(float& smoothX, float& smoothY);

// Headless check (run by `-test-map-gametypes`). Returns the number of failures.
int SelfTest();

} // namespace InputTuning
} // namespace noble
