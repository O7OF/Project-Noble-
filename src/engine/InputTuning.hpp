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

// Controller look speed, 1.00..10.00. Reach's profile only holds whole steps 1..19, so 10.00 maps to the stock 19 and in-between
// speeds use the next step up with the stick scaled down to match. `LookProfileValue` is the 1..19 step, `LookStickScale` the
// scale (1 when the speed lands on a step).
int LookProfileValue(float speed);
float LookStickScale(float speed);

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
