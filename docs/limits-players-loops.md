# C12 step 2: loops, bounds and tables for 32 players (Claude, 2026-11-08 15:30 ET)

Stock haloreach.dll 1.3528.0.0, RVAs. Follows `docs/limits-players-audit.md` and DeepSeek's
`docs/limits-players-bounds-review.md`. **Written, not built or tested yet** (this session had no shell on the PC).

## What is new in code
| File | What |
|---|---|
| `tools/limits/gen_steps.py` → `src/host/SlotStepTable.inc` | 6 pointer-stepping loop caves (hand-reviewed list in the script; bytes checked against the stock DLL) |
| `tools/limits/gen_bounds.py` → `src/host/SlotBoundsTable.inc` | 137 bound/mask edits: 134 `cmp reg,0x10/0x11` → 0x20, plus 3 raw edits in the step loops |
| `src/host/SlotPatch.cpp` | `-slot-players 32` / `NOBLE_SLOT_PLAYERS=32` installs the steps + bounds (all byte-checked, all or nothing); shadow records are built with the engine's own record constructors |

Regenerate with (stock DLL path from the Steam folder; never commit the DLL):
```
python -I tools/limits/gen_steps.py "<MCC>\haloreach\haloreach.dll" src/host/SlotStepTable.inc
python -I tools/limits/gen_bounds.py "<MCC>\haloreach\haloreach.dll" docs/limits-players-bounds.json src/host/SlotBoundsTable.inc
```

## Findings
1. **Peers stop at 32, not 33.** The peer-valid mask (`+0x5C` / block `+0x2C`) is a DWORD, and loops such as 0x3919E4
   use a 17-bit constant `0x1FFFF`. 32-player mode is therefore 32 players + 32 peers; peer slots 17..31 and player
   slots 16..31 live in the shadow object. All raised bounds use 0x20.
2. **Membership block base.** Many functions work on a *block* pointer = session + 0x30 (copy A) or + 0x2240 (copy B):
   block+0x2C = peer mask, block+0x30 = peer array. The step caves use the same constant C as the imul caves, so they
   are correct for either copy and all four sessions.
3. **Shadow records must be constructed.** The session constructor 0x3CF864 runs peer ctor 0x16FC60 (17×) and player
   ctor 0x3CFA38 (16×) on both copies; zero-filled shadow slots would read as valid id 0. SlotPatch now runs the same
   ctors on the shadow slots (32-player mode). The session ctor also runs 0x10062C over a **17 × 0xC table at
   +0x61574**, a peer table not in the trace list (see tables; not shadowed yet).
4. **Step loops: 8 installed, 16 blocked, 1 not a session array** (lists in `gen_steps.py`; C12-L step 1 reviewed the
   last 11 on 2026-11-09). Installed: 0x391B94 (offset form), 0x39322C, 0x39EC61, 0x39ECD8, 0x408F2E, 0x411986,
   **0x44BDE3, 0x44CCEF** (both read-only walks; bounds 0x44BDEA / 0x44CCF6 raised with them). Newly blocked by their
   own fixed tables: 0x404B4A (17-dword stack table, 0x90 frame), 0x41FC64 (17 x 6 stack table), 0x4314D3 (0x35-byte
   output entries), 0x431B1E (16 x 0x3A), 0x44CB51 / 0x44CCA1 / 0x44CD5F (16-entry tables in the 0xA10 struct built by
   0x44BCE8), 0x4509E8 (4-byte table at rbp+0x990). 0x4215AE is not a session array (0x4E0E908 object, +0xF00). Blocked because the same loop fills a
   fixed table: 0xD8C9, 0x39E212/9, 0x3FFD7F, 0x41E9B5, 0x41F576, 0x41F61E, 0x41F95B.
5. **Bounds: 137 raised, 41 kept at 16/17.** Kept = every entry in a function that also fills a fixed table
   (0xD6B8, 0x38DA74, 0x390DB0, 0x39C6B4, 0x39DD4C, 0x3C0248, 0x40A760, 0x41E7C8, 0x41F510) and the 3 `other`
   entries. 0x390DB0's `cmp rcx,0x10` copies are 16-character name lengths and must never be raised.

## Still open before 32 players can be switched on for real
**Fixed tables (16/17 entries, outside the session object unless noted):**
| Where | Table | Needed |
|---|---|---|
| 0xD6B8 | 16 × 0x20 player list at rbp-0x80, passed to the host interface `[0xC1A100]` vtable +0xF0 | find the receiver (Noble or engine), then a 32-entry list |
| 0x390DB0 / 0x38DA74 | 16 qword pointers at rbp+0x150 | **done (C12-L step 3, 32-player mode):** frame +0x100, table moved to rbp+0x1F8 (32 entries), own unwind copy (block shared with 0x35B1E4), 0x38DAE6 -> 0x20. The 0x390DB0 bounds stay 16 until its 6 untraced imuls (0x390E85/F13/F44/FF5, 0x391342/3B2) are caved |
| 0x39C6B4 | 17 × 0xC at session +0x4440 | shadow copy (same delta) |
| ctor 0x3CF864 | 17 × 0xC at session +0x61574 (ctor 0x10062C) | find users, shadow copy |
| 0x39DD4C ← 0x396828 | 16 × 0x180 in the caller's frame | caller frame +0x1800 |
| 0x3FFC9C | 17 × 0xC in the caller object at +0xC | find the object |
| 0x41E884, 0x41F510, 0x41FBE8 | 17 × 6 stack tables (memset 0x66) | bigger frames |
| 0x41F510, 0x40A760 | 16 × 0xA0 in the 0x4E0E910 singleton / vtable 0xA47750 object | grow the objects (+0xA00) |
| 0x3C0248, 0x41E7C8 | 6-byte tables at +0x34 | grow (+0x60) |

**Other 17-bit peer masks (`0x1FFFF`) not changed yet:** 0x39D1A3, 0x39F1B2, 0x39F203, 0x3C196F, 0x3C1F68,
0x4037B1, 0x4053FB, 0x414395, 0x44C3AA (and 0xA0B86, 0x2C8A59, 0x30426B, 0x313E21, 0x3389F6 outside the network range;
check whether they are peer masks at all). Also 0x408F10 tests only `si` (16 bits) of a player bitmask.

**Not looked at yet, and probably the biggest remaining items:**
- **Whole-block copies.** Membership updates copy block A ↔ B and into messages as whole structs (that is how the
  mask gets written). Those copies only move the inline slots; the shadow slots need a matching copy, so every
  block-copy routine needs a hook that also copies the shadow part.
- **Network encoding.** The membership/player update messages almost certainly encode slot counts and masks with
  fixed bit widths (16/17 entries). 32 players needs those encoders/decoders widened, which changes the protocol
  (every machine in a session must run the same build, which the `limits_id` join check already enforces).
- **Players pool to 32** (PoolResize `players`, fire team, simulation view) is still not raised; steps + bounds
  alone change nothing visible until it is.

## How to test what is here
1. Build. Two-instance LAN pair as before with `-slot-patch` (16-player mode): must behave exactly as step 1.
2. Same pair with `-slot-patch -slot-players 32`: only 2 players exist, so mask bits ≥ 16 are never set and the
   raised bounds and step caves must not change anything either. Log must show `32-player mode (dev): 6 step
   loops, 137 loop bounds raised`. Any crash names the RVA (FATAL / STACK OVERRUN handlers).

## C12-L step 2 blocker: 169 untraced imul sites (2026-11-09, handed to the cloud session / Tyler)
Most of the bounds are already raised by `d107dc0` (101/111 bit-scans, 26/31 C1, 6/8 C4). The rest wait for step 3
tables: 0x390DB0 (every loop indexes the 16-qword table at rbp+0x150, cookie right after it) and 0x39C701 (callee
0x39D0AC indexes the 17 x 0xC table at block+0x4440 by peer).

**Blocker:** the bounds review assumed per-slot helpers are covered by imul caves. They are not always: 0x39D950
(called per player from 0x39C6B4) has uncaved `imul` at 0x39D957 (0xB0) and 0x39D97A (0x158). In 0x380000-0x460000
there are **169 `imul r, r, 0xB0|0x158` sites not in `SlotPatchTable.inc`** (430 in the whole DLL); the trace never
ran those paths. Raising more bounds before they are handled sends slots >= 16 through inline addresses.

Wanted: a sort tool (Python, `tools/limits/`) that marks each of the 169 as slot / not slot, then caves for the slot
ones in a separate table installed only with `-slot-players 32` (16-player mode must stay exactly step 1).
Rule: follow the imul destination until it is overwritten; slot site if a memory operand using it has a
displacement inside the first record of a slot array, session-relative or block-relative (block = session + 0x30):
players 0x1740 / 0x3950 (record 0xB0), peers 0x60 / 0x2270 (record 0x158). Check it against the 67 caved sites
(it should mark them all slot). Listing used: capstone text of the stock DLL (`%TEMP%\stock_text.asm`, not in git).

## C12-L step 3 progress and design blocker (2026-11-09)
- Done: 0x390DB0 frame growth (32-entry name table, own unwind copy), commit cba2c21. 32-player mode only.
- #31 (17 x 0xC at block+0x4440): cannot grow in place (session fields at +0x4528..+0x4578 follow it). Needs a cave
  at each of the ~47 indexed sites (`[base + 3*idx*4 + 0x443C/0x4440/0x4444]` block-relative, `+0x4470` session-relative:
  0x39000B, 0x3939FD, 0x3955B6, 0x3958F6, 0x39672D, 0x39692D, 0x396BD8, 0x39D0D5, 0x39D517, 0x400016, 0x4003CC, ...;
  full list from `grep '\*4 + 0x44(3c|40|44|6c|70|74)\]'`) plus the step loops 0x39C78C / 0x39C818. Same kind of
  sorting job as the 169 imul sites; added to that handoff.
- **Blocker (#32, 0x396828): stack copies of the membership block.** Its 0x8F00 frame holds a membership copy
  (halves at [rbp+rcx+4] / [rbp+rcx+0x2214]) and two 0x2550-byte objects with 17 x 0x180 entries that 0x39DD4C fills.
  The copy only has room for the inline slots, and the imul caves move slots >= 16 by a fixed delta from the STATIC
  sessions, which is wrong for any copy. Before more tables are grown, decide how copies are handled: (a) hook every
  block copy/clear (memcpy/memset of 0x4710/0x2210-sized blocks) and give each copy its own shadow, keyed by address;
  or (b) make the caves base-aware (static sessions -> shadow, anything else -> a per-copy side buffer). Either way it
  is the "whole-block copies" item, and it gates #32, #38, #45, #47, #51-57 and the network encoding.

## Membership block copies/clears: scope count (2026-11-09, plan (b) + copy-site propagation)
Search: every memcpy (0x794276) / memset (0x78932A) / `rep movs|stos` / inlined movups copy loop whose size is a
membership block or slot-array size (0x4710 whole block, 0x2210 / 0x220C half, 0x16D8 peer array, 0xB00 player
array, 0x2550 derived object). `rep stos` in 0x380000-0x460000: all <= 0x22 dwords (none). No inlined copy loops.
Half-block layout: half A = session+0x38 (block+8), half B = session+0x2248; peers at half+0x28, players at half+0x1708.

| # | Site | Function | What | Source -> destination |
|---|---|---|---|---|
| 1 | 0x39C4D5 | 0x39C484 | memcpy 0x4710 (whole block) | caller's block (stack arg of 0x391F0C) -> static session+0x30 |
| 2 | 0x39C4F3 | 0x39C484 | memset 0x2210 | clears half B of the destination (dst+0x2218) |
| 3 | 0x39C71A | 0x39C6B4 | memset 0x4710 | membership reset (block) |
| 4 | 0x39C7C4 | 0x39C7AC | memset 0x220C | half-B reset (block+0x221C) |
| 5 | 0x396A6F | 0x396828 | memcpy 0x2210 | static half A (session+0x38) -> stack copy S2 (rbp+0x2140) |
| 6 | 0x396A7F | 0x396828 | memcpy 0x2210 | rbx (half B) -> stack copy S1 (rsp+0x30) |
| 7 | 0x396CBE | 0x396828 | memcpy 0x2210 | static half A -> static half B, same session |
| 8 | 0x16FEA9 | 0x16FE2C | memset 0x16D8 | whole peer array clear |
| c | 0x396B3F | 0x396828 | memcmp (0x794270) 0x2210 | S2 vs S1 change test: must also compare slots 16-31 |

**8 copy/clear sites + 1 compare.** Limits of the search: copies whose size comes from a register or argument are
not found by immediate; 0x39C484 has one caller (0x3920AC), so it is not a generic copier.

**Flagged (field-by-field fills, no whole-block copy into them):**
- The SOURCE of #1: 0x391F0C passes a caller-owned block (stack argument [rsp+0xD0]); it is most likely filled by the
  network decode field by field. That decode is the network-encoding item and needs its own handling.
- 0x39DD4C fills the two 0x2550 objects in 0x396828's frame per peer (17 x 0x180) after memset; derived layout, not
  the membership layout (table #32).
- 0x39FCC3: per-record 0x80-byte copy of one player record into a non-membership buffer (rbx); per-slot, covered by
  whatever cave serves its imul (0x39FCAE).

**Design snag for base-aware caves:** the imul caves run at `imul dst, idx, R`, where only the index is known; the
base register is added later, at the memory operand. A range check on the base needs either (1) the base register
per site (the use instruction after each imul), or (2) moving the redirect from the imul to the memory-use
instruction. Either way each of the 67 existing sites (and the new ones) needs its base register recorded.
