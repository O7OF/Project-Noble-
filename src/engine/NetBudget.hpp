#pragma once

namespace noble::NetBudget {

// Raises the game's network upload budget from its 2010 Xbox 360 values to modern ones (2026-11-19).
// The engine caps a host's total upload at 384 / 512 kbit/s and also at 50 / 75% of its estimate of the upload speed.
// Noble raises the two fixed caps; the "% of your upload speed" rule stays, so slow connections still hold back.
// Optional: Settings "hostUploadKbps" tells the engine the real upload speed when its own estimate is too low.
// Writes only when the engine's settings block still holds the values this game build is known to use.
// Call often (cheap); it does its work at most every two seconds.
void Tick();

} // namespace noble::NetBudget
