#pragma once

#include <cstdint>

// C12-L #32: the membership update message (type 0x1A) for 32 players. Part of 32-player mode only: SlotPatch
// checks Match() together with its other 32-player checks (all or nothing) and calls Apply() after its own
// patches. Design and site list: docs/limits-players-message.md; tables: SlotMessageTable.inc
// (tools/limits/gen_message.py). 16-player mode never touches any of these bytes.
namespace noble::slots::message {

// Every patched site still has its stock bytes (the two decoder limits may also carry the EnginePatches security
// fix). Logs each mismatch.
bool Match(uintptr_t base);

// Installs the caves, immediate edits and the producer call-site stubs. shadowDelta = shadow sessions - sessions.
// Returns false (nothing changed) if memory near the engine could not be allocated.
bool Apply(uintptr_t base, int64_t shadowDelta);

// Dev test (NOBLE_SLOT_HIGH=1, 32-player mode only): the host gives joining peers slots >= 17 and players >= 16 (the
// first entry, the host itself, keeps slot 0), so a 3-instance LAN test runs the shadow-slot paths.
bool ApplyHighSlotTest(uintptr_t base);

// For logs: number of caves / edits.
unsigned CaveCount();
unsigned EditCount();

} // namespace noble::slots::message
