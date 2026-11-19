#pragma once

#include <cstdint>

namespace noble {

// Noble-only appearance extras (2026-11-19).
//
// Extra customization that the engine's own appearance record cannot carry (flame colour variants today) travels over
// Noble's own control channel instead of being written into engine player data. Writing it into the engine's
// replicated appearance record broke co-op joining (the engine validates those bytes and drops the player).
//
// Every Noble client sends "my player id -> my extras" every couple of seconds. The host merges what it receives and
// rebroadcasts the whole table, so every guest also learns about the other guests. Renderers then look a remote player
// up by the engine player record they are drawing.
namespace AppearanceSync {

// The engine-side player id (XUID) Noble reports for the local user. Single source of truth: the host interface's
// user-identity slot returns this value to the engine.
uint64_t LocalXuid();

// Call once per engine frame; it rate-limits itself and does nothing without an active Noble session.
void Tick();

// A remote player's extras arrived (any sender). Ignores the local player's own id.
void SetRemote(uint64_t xuid, int flameVariant);

// Flame colour variant (0..29) for the player whose engine player record is `playerRecord`, or -1 if unknown/none.
int FlameForPlayerRecord(const uint8_t* playerRecord);

} // namespace AppearanceSync
} // namespace noble
