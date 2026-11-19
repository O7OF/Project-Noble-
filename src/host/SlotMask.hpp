#pragma once
// 64-player work, step 1 (2026-11-19): Noble's own versions of the engine functions that walk the 32-bit
// peer/player valid masks, written against a 64-bit mask (the engine's DWORD = low word, the high word lives in
// SlotPatch's shadow object). Inventory: docs/02-engine-reference/MASK-ACCESS-INVENTORY-2026-11-19.md.
//
// Dev-only and off by default:
//   NOBLE_MASK64_VERIFY=1  run the engine's function AND Noble's version on every call, keep the engine's result,
//                          and count any difference (works in normal games; proves each replacement is identical).
//   NOBLE_MASK64=1         use Noble's version (needed once a capacity above 32 is installed).
#include <windows.h>

namespace noble::slotmask {

void Install(HMODULE engine);

// Dev: one log line with call / mismatch counts per replaced function (rate-limited inside).
void ReportStats();

} // namespace noble::slotmask
