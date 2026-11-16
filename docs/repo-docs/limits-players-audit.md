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

## Trace results (DeepSeek's run, analysed by Claude 13:05 ET)
Source: `docs/limits-players-trace.txt` (menu + Custom Game screen, and a Slayer match start; no join/kill/quit).
- **300 distinct engine instructions** touched the slot regions, in **63 functions**: peersA 116, playersA 87,
  playersB 67, maskA 28, peersB 2; 66 of them write. Most live in 0x380000-0x45FFFF (network), plus 0x160000 and
  a few elsewhere.
- Addressing of the player-slot hits: 82 `[base+disp]`, 53 `[base+index*scale+disp]`, 19 other. Many use an
  **interior pointer** (e.g. `r8` = &player.field, then `[r8-0x67]`), i.e. the engine computes a player-record
  pointer once and then reads fields off it.
- **This makes the rewrite much smaller than the static ceiling (~1,100):** retarget the places that *compute* a
  slot pointer (`imul idx,0xB0` / `imul idx,0x158` + the array base), and every field access through that pointer
  follows automatically. A first heuristic scan found 43 peer-pointer derivations and only 2 obvious player ones
  (0x395E85, 0x39DCB7); the player derivations mostly use other shapes (base added before the multiply, or a
  `lea` chain), so the tool must follow the register from the multiply to its first memory use.
- **The tracer stalls the game** after a few minutes (both runs: process alive, frames stop). Likely cause: the
  session object contains buffers the OS writes (socket I/O); kernel writes to a guard page fail instead of
  faulting into the handler, so network calls break. Good enough for mapping; for longer runs, guard only the
  player/peer record pages, not the whole first 0x4538 bytes.

## Next steps (32-player milestone)
1. Tool `tools/limits/slot_rewrite.py`: for each traced instruction, walk backwards in its function to the slot
   pointer's derivation (multiply by 0xB0 / 0x158 and array base); emit the derivation list. Add the static
   candidates from the 426-function scan that follow the same shapes.
2. Patch plan: each derivation redirects to Noble-owned arrays of 32 players / 33 peers (two copies, as in the
   object), placed in `.hkwork`; the mask stays 32-bit for this milestone; bounds `cmp …,0x10` / `0x11` in
   those functions raised to 0x20 / 0x21.
3. Players / fire team / simulation-view pools to 32 via PoolResize rules.
4. 2-PC LAN test with Tyler: join, play, kills, respawns, quit; then a 3+ player test.

## Rewrite inventory from the trace (Claude, 13:20 ET) — `tools/limits/slot_rewrite.py`
Exact array bounds: peers 17 × 0x158 at +0x60 (to +0x1718) and +0x2270; players 16 × 0xB0 at +0x1740 (to +0x2240)
and +0x3950 (to +0x4450). Anything at +0x1738/+0x173C, +0x2240..+0x226C, +0x4450..+0x4538 is a **scalar field**
(counts, mask, leader ids) and does not move under option 1.
- Of the 300 traced instructions, **192 touch real slot records** (peersA 107, playersA 83, peersB 2; the second
  player copy at +0x3950 was not touched in this run), and **108 only touch scalar fields** (+0x4464 ×40, mask
  +0x173C ×28, +0x4468 ×15, +0x1738 ×9, …).
- Those 192 accesses come from only **77 distinct address derivations**: 39 `imul idx, 0xB0/0x158`, 23 `add`
  (mostly base + an earlier imul result), 6 `lea`, 2 loaded pointers, 1 `sub`, 5 where the pointer arrives as a
  function argument (follow the caller), 1 other.
- **Patch shape (per derivation, generated by the tool, reviewed by hand):** slots 0..15 stay where they are; for
  index ≥ 16 the derivation is redirected to Noble-owned overflow storage (per session object, per copy), e.g. at
  `imul rbx,rax,0x158` → cave: `if idx < 16: rbx = idx*rec else rbx = overflow(obj) + (idx-16)*rec - obj - base`,
  so the unchanged access `[rbx+obj+base+field]` lands in the overflow record. Keeps every existing slot address
  identical (no change for 16-player games) and touches ~40-80 sites instead of ~1,100.
- Coverage gap: joins, leaves, kills and the second copy were not exercised. The 2-instance dedicated-server test
  (noble-dedicated + client on one PC) is needed to finish the list.

## Patch design for the 32-slot build (Claude, 13:45 ET)
Run 3 (LAN join) included: **417 slot-record accesses from 136 address sites** (`docs/limits-players-sites.json`):
65 `imul`, 27 `add`, 18 `lea`, 15 argument-passed, 5 `sub`, 4 loaded, 2 other.

**Dominant shape (65 imul sites):** `imul dst, idx, 0xB0|0x158` then `[dst + obj*1 + disp]` where `obj` is the
session object and `disp` = array base + field offset (e.g. `cmp [rax+r11+0x38],8`, `mov rbx,[rbx+rsi+0x60]`).

**Overflow storage at a constant distance.** The 4 session objects sit at S = RVA 0x2B07418 + k × 0x61668. Put one
overflow block per object in `.hkwork` at O + k × 0x61668 (same stride), so overflow(obj) = obj + (O − S) for
every object. Each block holds the extra records for slots 16..31: players A and B (16 × 0xB0 each) and peers
A and B (16 × 0x158 each), 0x4100 bytes used per block (stride wastes virtual space only; zero-filled).
For an access through array base B with record size R and overflow array offset V:
`dst' = dst + C`, with **C = (O − S) + V − B − 16·R**, a constant per array (fits in imm32: |O − S| ≈ 0x8000000).

**Cave per imul site** (the 7-byte `imul r64,r64,imm32` is replaced by a 5-byte jmp + nops):
```
cmp  idx, 16          ; before the imul, since dst may equal idx
jb   .low
imul dst, idx, R
add  dst, C           ; slot >= 16 -> overflow record
jmp  back
.low:
imul dst, idx, R
jmp  back
```
Slots 0..15 keep their exact addresses (16-player behaviour unchanged). The array (and so C) is identified from
the use's displacement range. Sites whose result feeds several arrays, or whose `add`/`lea` chains combine
base and index differently, are listed for hand review.

**Other pieces for 32:** `cmp idx,0x10` / `0x11` loop bounds in the touched functions → 0x20 / 0x21; players /
fire team / simulation view pools → 32 (PoolResize rules); the 32-bit mask stays as is. Code caves: ~65 × ~30 B
≈ 2 KB, so the caves go into a new executable block at the end of `.hkwork`'s raw data or into Noble-allocated
executable memory with rel32-reachable placement (the `.deltrac` gap is used up).

## Step 1 done, step 2 inventory (Claude, 14:25 ET)
- **Step 1 committed (`fb1aa73`):** `SlotPatch` redirects the 67 imul sites; two-instance LAN regression passed
  (both processes patched, guest joined the host's session, played, clean shutdown).
- **Run 4 total:** 420 accesses / 139 sites. Of the 72 non-imul sites, **6 `add` sites are already covered**
  (they add an index×record value produced by a patched imul). **66 remain** (`docs/limits-players-rest.json`):
  add 21, lea 18, argument-passed 15, sub 5, loaded 4, other 3.
- **Pointer-stepping loops (12 of the 66 sites, in 6 functions):** 0xD6B8, 0x3915A8, 0x392C90, 0x39DD4C,
  0x408DDC, 0x41F510 walk the slot arrays with `add ptr,0xB0/0x158` up to 16/17 records. These cannot be
  redirected per access: each needs its loop split (slots 0..15/16 in place, then 16/17..31/32 in the shadow
  object at obj + delta) via a hook or rewritten function. Claude.
- **The other 54 sites:** need a per-site review of whether the address depends on a slot index at all
  (`lea rbx,[r14+0x30]` and similar are object-field pointers that stay valid) or reaches a slot by another
  shape (`lea r9,[rax+rdi*1]` with a pre-multiplied index, a slot pointer passed in from a caller, …).
  Tool support: `slot_rewrite.py` chains are in the JSON; review each with objdump around the site.
- After both: loop bounds `cmp …,0x10/0x11` → 0x20/0x21 in the touched functions, pools → 32, then a
  three-instance LAN test (Release3 data folder).

## Step 2b/3 findings (Claude, 14:40 ET)
**The 6 pointer-stepping loop functions** share one shape. Example 0xD6B8: `r8` = player array (+0x73) from
0x398200, `r9d` = slot counter, mask in `[rsp+0x58]`; per slot `bt mask,r9d` → copy fields into a **16-entry
stack table** (`rbp-0x80`, 16 × 0x20, filled with `mov r15d,0x10` / `call 0xD540`), then
`inc r9d / add r8,0xB0 / cmp r9d,0x10 / jl`.
For 32 each such loop needs: (1) a cave at the pointer step: `add r8,R; cmp r9d,T; jne +; add r8,C; +: cmp r9d,2T`
(the same C = delta − T·R as the imul caves, so slot T continues in the shadow object), (2) the bound raised,
(3) any **local 16-entry table grown** (stack frame, like the AI fixes). 0xD6B8's table is probably the player
list handed to the host API (MCC/Noble), so Noble's host side must accept 32 entries too. Next: do the 6 one by one.

**Bounds pass:** the 97 functions the trace touched contain **174 `cmp reg,0x10/0x11`**
(`docs/limits-players-bounds.json`): **111 are bit-scans over the 32-bit player mask** (`bt mask,i` then
`cmp i,0x10`) → raise the 1-byte immediate to 0x20 (mask already 32 bits); **60 are loop bounds** → raise
after checking each loop's body stays inside redirected accesses; 3 other → review. Patch these as plain
1-byte .nlp-style edits applied by SlotPatch (with byte checks), only when the 32-player mode is on.

## 54/58-site review result (DeepSeek, 14:25 ET; table in docs/limits-players-review.md)
**26 A** (object fields, record copies, mask-bit loops) · **17 B** (already covered by the imul caves) ·
**15 C** (pointer from a caller or out-param; producers named, all reached through covered paths) · **0 D**.
(58 rather than 54: four sites sit on function boundaries next to 0x3915A8 / 0x392C90 and were kept in.)
**Conclusion:** no new derivation shapes. Remaining code work for 32 players = the 6 pointer-stepping loop
functions (cave at the step + bound + grown local tables), the bounds pass (111 bit-scans + 60 loops), the
pools to 32, and Noble's host-side player list (0xD6B8 hands a 16-entry table to the host API).
Re-check the 15 C sites only if a caller ever passes a stack copy of a slot record.
