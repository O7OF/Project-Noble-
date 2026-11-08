# C12 audit pass 1: code that indexes the players pool with a fixed 16 (Claude, 2026-11-08 10:35 ET)

Companion to `docs/limits-players.md` (DeepSeek). Stock haloreach.dll 1.3528.0.0, RVAs.

## Method
1. Every function that loads the players pool from the game-state root (`mov reg,0x18` then
   `mov reg,[root+reg]`, the same access `player_get` uses): **426 functions**.
2. Of those, the ones with a compare against 0x10 / 0xF (loop bounds or index checks sized for 16 players):
   **33 functions**, listed below with other hints: 0xFFFF masks, bit operations (per-player bitfields),
   `imul …, 0x490` (player record indexing) and stack frame size (per-player stack tables).
3. Not covered yet: static per-player tables in `.data` indexed by player index, 16-bit player masks in network
   messages (session code doesn't go through the pool), and the `simulation view` / `fire team` users.

## Candidates (highest count first)
| Function RVA | cmp 0x10 | cmp 0xF | 0xFFFF masks | bit ops | player recs (×0x490) | frame |
|---|---|---|---|---|---|---|
| 0x41ED54 | 5 | 0 | 0 | 0 | 0 | 0xD0 |
| 0x14F430 | 3 | 0 | 0 | 1 | 1 | 0x90 |
| 0x261B00 | 2 | 1 | 0 | 0 | 1 | 0x1D0 |
| 0x809C0 | 2 | 0 | 0 | 2 | 0 | 0x50 |
| 0xF2DAC | 2 | 0 | 0 | 5 | 4 | 0x60 |
| 0x105160 | 2 | 0 | 0 | 0 | 1 | 0x220 |
| 0x10DD28 | 1 | 1 | 0 | 0 | 0 | 0x70 |
| 0x1F80C8 | 2 | 0 | 0 | 0 | 0 | 0x1C0 |
| 0x2C1964 | 2 | 0 | 0 | 0 | 0 | 0x40 |
| 0x42136C | 2 | 0 | 0 | 1 | 0 | 0x60 |
| 0x69A964 | 2 | 0 | 0 | 1 | 1 | 0x3E0 |
| 0x48A68 | 0 | 1 | 0 | 0 | 1 | 0x60 |
| 0x51974 | 1 | 0 | 0 | 0 | 0 | 0x40 |
| 0x52514 | 1 | 0 | 0 | 1 | 1 | 0x80 |
| 0x7FA80 | 1 | 0 | 0 | 0 | 0 | 0x90 |
| 0xF88D0 | 1 | 0 | 0 | 0 | 0 | 0xD0 |
| 0xF8F08 | 0 | 1 | 0 | 0 | 0 | 0x40 |
| 0xF9D28 | 1 | 0 | 0 | 0 | 0 | 0x0 |
| 0x1099CC | 1 | 0 | 0 | 0 | 0 | 0xC0 |
| 0x10AE54 | 0 | 1 | 0 | 6 | 3 | 0xD0 |
| 0x124D00 | 1 | 0 | 0 | 0 | 2 | 0x150 |
| 0x14CD44 | 1 | 0 | 0 | 0 | 0 | 0x70 |
| 0x18C5F0 | 1 | 0 | 0 | 13 | 0 | 0x30 |
| 0x1B13DC | 0 | 1 | 0 | 0 | 1 | 0x40 |
| 0x1CDA48 | 1 | 0 | 0 | 0 | 1 | 0x120 |
| 0x1F39D4 | 0 | 1 | 0 | 1 | 1 | 0x230 |
| 0x2DB54C | 1 | 0 | 0 | 0 | 0 | 0xB0 |
| 0x3CCCC0 | 0 | 1 | 0 | 2 | 1 | 0x30 |
| 0x4AEE58 | 1 | 0 | 0 | 0 | 0 | 0x160 |
| 0x4FEC58 | 1 | 0 | 0 | 0 | 1 | 0x120 |
| 0x68BC3C | 1 | 0 | 0 | 2 | 1 | 0xA10 |
| 0x69D8AC | 1 | 0 | 0 | 1 | 1 | 0x4E0 |
| 0x6A0C00 | 1 | 0 | 0 | 1 | 0 | 0x110 |

## Next steps (16 → 32 milestone)
1. Disassemble each candidate; mark each `cmp …,0x10` as **player count** (raise to 32 with the pool) or
   unrelated (e.g. 16 = some other constant). Functions with bit ops and a 16 bound are likely 16-bit player masks:
   these need widening, not just a bigger bound.
2. Same scan for the `fire team` pool (16 × 0xD4) and `simulation view data array` (16 × 0x118) users.
3. Scan `.data`/`.bss` for arrays with stride × 16 indexed by a player index (look for `imul idx, stride` +
   rip-relative base inside player-pool functions).
4. Test profile: players 32 via the PoolResize rule (name `players`), plus the confirmed bound patches as
   .nlp ops; local 2-instance match; any crash is named by the FATAL / STACK OVERRUN handlers.
5. Network side (session membership, replication masks) is a separate pass on the LobbyPatches RVAs.

## Classification so far (top 4)
- **0x41ED54, 0x14F430: real 16-player loops over a 32-bit player mask** at `[struct + 0x173C]`
  (`bt reg, index` for index 0..15). 67 instructions in the engine read `+0x173C` as a DWORD. A DWORD holds
  32 bits, so **the 16 → 32 milestone needs only the loop bounds raised**; 64/128 need this mask widened to
  64/128 bits everywhere it is read or written (67 sites + every writer), which is the protocol-bump work.
  Next: identify the struct (likely the session membership / game-engine globals holding the active-player
  mask) and list all 67 sites with their bound.
- **0xF2DAC: not a player count** (0x10 / 0xF compare an object type byte at +0x11 and a team field +0x39C).
- **0x261B00: not a player count** (a `switch` on a message/event id: 9, 0x10, 0x14, …).
Remaining 29 candidates: same check pending.

## Key finding (10:58 ET): the player mask lives in the session membership object, with inline 16-slot arrays

- The +0x173C mask has **exactly one writer**: the constructor at **0x3CF864** (vtable 0xA490D8), which zeroes it.
  All other 66 sites read it, in 32 functions (0x14F430, 0x16FE2C, 0x2A3680, 0x38FBD4, 0x38FEAC, 0x390DB0,
  0x3915A8, 0x391F0C, 0x39816C, 0x3A3224, 0x3DC138, 0x3EB738, 0x3EBA00, 0x4044C0, 0x404728, 0x4076CC,
  0x407AB0, 0x408DDC, 0x409E8C, 0x4105A8, 0x4118C8, 0x411D98, 0x412014, 0x415E28, 0x4160AC, 0x416670,
  0x41ED54, 0x41F510, 0x431288, 0x431A1C, 0x44E7E8, 0x4505F0), nearly all in the 0x38xxxx-0x45xxxx network code.
  The mask is set elsewhere by whole-struct copies (membership updates), not by named field writes.
- The constructor lays out **two copies** of the same membership block (current + transmitted/previous):
  | Offset | Contents |
  |---|---|
  | +0x0060 | 17 × 0x158 peer records (16 + 1 for the host/local) |
  | +0x173C | 32-bit player-valid mask |
  | +0x1740 | 16 × 0xB0 player records |
  | +0x226C | (second block) peer count / header |
  | +0x2270 | 17 × 0x158 peer records |
  | +0x394C | 32-bit player-valid mask |
  | +0x3950 | 16 × 0xB0 player records |
  | +0x4538 | 4 × 0x80 records, then +0x4748 a large sub-object (+0x5C5D0 / +0x5C7A0 fields) |
- **Consequence:** 16 players and 17 peers are *inline arrays in the session object*. Going to 32 shifts every
  field after +0x1740 (and after +0x3950, +0x4538, +0x4748 …), and every instruction in the network code that
  addresses those fields by constant offset must change. This is the real C12 wall, larger than the pools.
- Options for 16 → 32:
  1. **Relocate the arrays:** keep the object layout, point the per-player/per-peer arrays at Noble-owned storage
     by patching each `[obj + 0x1740 + i*0xB0]`-style access (needs a full xref list of +0x60 / +0x1740 / +0x2270 /
     +0x3950 accesses, and the mask widened only if > 32).
  2. **Grow the object:** find its allocation (static global or heap), enlarge it, and patch every constant
     offset ≥ +0x1740 in the network code. More edits, but no indirection.
  3. **Noble-side session layer:** keep the engine at 16 per *engine* session and run several engine sessions
     / relay inside Noble for 64-128. Most work, but avoids rewriting the engine's network code.
  Recommend measuring option 1 first: count the xrefs for each array base offset.

### Size of option 1 (upper bound, network code 0x380000-0x460000)
Instructions whose displacement falls inside each array (some belong to other structs that happen to use the
same offsets, so these are ceilings, not exact):
| Region | Accesses |
|---|---|
| players A (+0x1740..+0x226C) | 550 |
| players B (+0x3950..+0x4538) | 611 |
| mask A (+0x173C) | 62 · mask B (+0x394C) | 1 |
| peers B (+0x2270..+0x394C) | 406 |
| peers A (+0x1B8..+0x173C) | 5,627 (heavily polluted by small offsets used everywhere) |
**Takeaway:** about 1,100-1,200 player-array accesses to retarget, plus the peer arrays. Patching each one by
hand isn't practical; the realistic route is a **tooled rewrite**: identify the session-object pointer flow per
function (the base register holding the membership object), then retarget accesses automatically and verify
with the same disassembly checks used for the AI frames. Option 3 (Noble-side relay of several 16-player engine
sessions) should be scoped against that before committing to either.
