#pragma once
// One-shot check that the loaded Halo: Reach engine is the MCC build Noble's game addresses were made for.
// Every hooked or patched engine location in Noble belongs to one exact build; after an MCC update they may
// point at the wrong code. The full per-address list and an old/new DLL comparison live in
// tools/check-addresses.py (dev tool); this is the cheap runtime half.

namespace noble::GameBuildCheck {

// Runs once (after the engine module is loaded); logs a plain-text result. Cheap to call every frame.
void CheckOnce();

// true = the loaded engine matches the build Noble was made for (also true until the check has run).
bool IsKnownBuild();

} // namespace noble::GameBuildCheck
