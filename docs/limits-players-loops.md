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

## 0x396828 stack copies S1 / S2: sites that address them (2026-11-09, before implementing plan C)
S1 = rsp+0x30 = rbp-0xD0 (copy of half B), S2 = rbp+0x2140 (copy of static half A); 0x2210 bytes each.
- Only pointers to S1/S2 handed out: the two memcpy calls (#5 0x396A6F, #6 0x396A7F) and the memcmp (0x396B3F).
  No other callee receives an S1/S2 pointer.
- **One imul addresses them: 0x396AB4 `imul rcx, rdx, 0x158` (peer index, NOT caved)**, used by six operands in the
  same block: 0x396AC2 [rbp+rcx+8] (S1), 0x396AC6 [rbp+rcx+0x2218] (S2), 0x396ACD / 0x396AD5 [rbp+rcx+4] (S1),
  0x396ADA / 0x396AE5 [rbp+rcx+0x2214] (S2) - peer fields +0xAC/+0xB0 in both copies.
- Its peer scans 0x396AA1 / 0x396B0A are ALREADY raised to 0x20 in SlotBoundsTable (d107dc0), so in today's
  32-player mode peers 17..31 would write inline past the 17 peer records of S1/S2 (into the copies' player
  arrays). 32-player mode must not be switched on before 0x396AB4 gets its side-area cave.

## Plan C implemented (2026-11-09, built, not tested)
- 0x396828 frame 0x8F00 -> 0xC540 (probe size `mov eax`, `lea rbp`, epilogue; own unwind copy: ALLOC_LARGE 0x18A8,
  save slots 0x18AF/B0/B1; GS cookie offset 0x8EF0 unchanged). sideS1 = rbp+0x8E00, sideS2 = rbp+0xB010 (0x2210
  apart, 15 x 0x158 each), pushes at rbp+0xC440.
- #5/#6 -> StackCopyMirror: memcpy, then always writes all 15 side records (source session's shadow peers, else 0).
- 0x396B3F -> StackCompareMirror: engine memcmp, then the two side areas when the inline copies are equal.
- 0x396AB4 cave: idx >= 17 -> rcx = 0x8EA8 + (idx - 17) * 0x158, else the original imul.
- Frame growth is one table (`kFrames`: 0x390DB0, 0x396828), all byte-, context- and unwind-checked.
- **Safety gate:** `-slot-players 32` now refuses (and logs the blockers) unless `NOBLE_SLOT_PLAYERS_UNSAFE=1`.

## C12-L step 4: pools and the host player list (2026-11-09, built, not tested)
- `players` (16 x 1,168), `fire team` (16 x 212), `simulation view data array` (16 x 280): PoolResize raises them to
  32 at data_array_new only when `slots::Players32Active()` (SlotPatch fully installed). Memory comes from the
  caller's allocator object, not the AI bank budget; data_array_new returns null when out of room -> stock 16 + error.
  The hook is installed for this even without -ai-actors (32-player dev override only).
- `simulation distributed view` (16 x 77,240) is not a data_array_new pool: 0x3D0263 allocates 0x12DBF4 from the
  region bump allocator 0x387B0, 0x3D0282 records the range (0x3D0310), 0x3D02B2 initialises with count 16. Three
  call-site wrappers: the 32-record size (0x25B774) is used only if the current region keeps 16 MB free afterwards
  (no clean null return in 0x387B0); then the range call and the init get the 32 size/count. Logged either way.
  Open: whatever indexes the views by peer/machine with a 16 bound has not been reviewed.
- **Host player list (0xD6B8, 16 x 0x20 at rbp-0x80):** handed to host vtable +0xF0 = slot 30 of Noble's own
  ReachHost (the MCC host replacement at 0xC1A100). Noble does not implement slot 30 (generic SlotDispatcher stub,
  returns 0). Nothing consumes the list, so it is NOT grown: its bound 0xD8D0 stays 16 (first 16 players listed),
  which cannot overflow. Grow it only if Noble ever implements slot 30.

## C12-L (c): peer table at session+0x446C caved (2026-11-09, built: no; tested: no)
- Table: 17 x 0xC at session+0x446C (= block+0x443C): +0 byte flag, +4 dword (index into a 0x18D0-stride table,
  -1 = none), +8 dword. Empty entry = {0, -1, -1} (0x39D0AC). Cannot grow in place (session fields at +0x4528..).
- **47 indexed operands** `[base + idx*4 + disp]`, disp 0x443C/40/44 (block base) or 0x446C/70/74 (session base),
  all in 0x390001..0x44ABC5. Every index register holds 3 * peer (`lea x,[y+y*2]`, `r13 = rbp*2 + rbp` at
  0x39D08A, `rax = r13*2 + r13` at 0x404F9A/0x4057C8), checked by hand. Plus two pointer-stepping reset loops:
  0x39C77C/0x39C78C (0x39C6B4, `lea rsi,[rsi+0xC]; cmp ebp,0x11`) and 0x39C800/0x39C818 (0x39C7AC,
  `add rcx,0xC; cmp edx,0x11`). No other reference to these offsets in 0x380000-0x460000 (no lea'd pointers).
- `tools/limits/gen_table44.py build/stock_text.asm src/host/SlotTableSites.inc` (input: the capstone listing,
  not the DLL; it encodes each operand and checks the length against the next instruction). SlotPatch byte-checks
  every row against the loaded engine (all or nothing with the rest of 32-player mode).
- Operand cave (flags preserved with pushfq/popfq): peer < 17 (or -1) -> original; base inside the 4 static
  sessions -> `lea idx,[idx+K]`, original, `lea idx,[idx-K]` (skipped when the instruction overwrites idx),
  K = (delta - 204) / 4 -> shadow entry `shadow + 0x446C + (peer-17)*0xC`; base anywhere else -> original at the
  stock address and a per-site counter (dev guard thread logs new counts every 5 s:
  `SlotPatch guard: peer table site +0x... reached peer >= 17 on a non-session block`).
- Step caves: at counter 17 the pointer moves to shadow entry 17 and the loop runs to 32; on a non-session block it
  ends at 17 like stock (counted).
- Shadow entries start as {0, -1, -1}; whole-block copies/clears (#1 0x39C4D5, #3 0x39C71A) now mirror the table
  too (`kArrays` entry with `table = true`, no record ctor).
- Already raised before this (so unsafe mode walked past the table until now): 0x39CC14 / 0x39CC4D (0x39CBF0 scan).
- **Still 17:** 0x39C701 (calls 0x39D0AC per peer; its table sites are caved now, review the rest of 0x39D0AC
  before raising), 0x39C769 / 0x39C7F9 (peer field resets, imul caved). Raise with the next bounds pass.
- Cave bytes were checked by compiling the emitter on Linux and disassembling sample caves (objdump).
- Test: run 3 of `run_slot32_smoke.bat` must log "peer table +0x446C: 47 sites + 2 reset loops caved" and no
  "peer table site ... does not match".

## Smoke test 2026-11-09 01:30-01:45 (one window, Sword Base, ENGINE LIMITS = Full, 120 s each)
| Run | Options | Result |
|---|---|---|
| 1 | none | clean, no SlotPatch lines |
| 2 | `-slot-patch -slot-players 32` | refused as designed, 16-player mode, 67 step-1 sites, clean |
| 3 | + `NOBLE_SLOT_PLAYERS_UNSAFE=1` | fell back to 16: `unwind entry of +0x390DB0 does not match` |
| 3b | same, with the new frame diagnostic | both frame tables had the GS handler RVA typed as 0x79370E (byte 32 `0E`); the engine has **0x7937E0** (a real handler after int3 padding). Call sites and all 49 peer-table sites matched |
| 3c | same, tables fixed | **32-player mode on**, 120 s clean: 8 steps, 139 bounds, both frames, 6 mirrors, side areas, peer table 47 + 2; pools players / fire team / simulation view / distributed view 16 -> 32; no FATAL, no STACK OVERRUN, no guard hits |
Note: with the typo the unwind copies would have pointed exception handling at a non-function; the check caught it.
`FramesMatch` now runs every check and `DescribeFrameEntry` logs the found entry; Install reports frames / call sites /
peer table sites together. Only 1 player was in the game, so slots >= 16 were never used: this proves install and
no regressions, not 32 players.

## C12-L (d): runtime discovery of unreviewed slot paths (2026-11-09, built: no)
No disassembly scan (stopped by safeguards before): unreviewed sites are found by running the game.
- `-trace-members` (dev builds) already guard-pages the 4 session objects and records every engine instruction that
  touches a slot array. It now also covers the peer table (+0x446C..+0x4538, region `peerTable`) and compares each
  instruction with `src/host/MemberKnown.inc` (471 reviewed accesses: the 420 in `limits-players-sites.json`, the 47
  peer-table operands, the 4 accesses in the two caved reset loops; `tools/limits/gen_member_known.py`).
- Any other instruction is logged once within 5 s as `MEMBER-NEW|rva|region|R/W|count|offsets`; the full `MEMBER|`
  dump every 30 s ends each line with `known` / `NEW`.
- In 32-player mode the peer-table operands run from caves (outside the engine), so they are not recorded; accesses
  through caved imul sites still are (the memory access itself stays in engine code).
- Run: `powershell -ExecutionPolicy Bypass -File tools\lan-three-instance.ps1` (host + 2 clients, all with
  `-trace-members -slot-patch -slot-players 32`, client 2 joins after 90 s; then team change if the game type has
  teams, client 1 leaves, host returns to the lobby). Release3 is created on first use (junctions + own device id).
- Each MEMBER-NEW site is reviewed by hand like the peer-table sites: follow the address back to its index, cave it
  if a slot >= 16 can reach it, then add it to the known list.
- Limit: only paths the runs exercise are found. Repeat with other modes/events until no MEMBER-NEW appears.

## Three-instance trace 2026-11-09 01:58-02:03 (host + 2 clients, 32-player mode, ENGINE LIMITS = Full)
All three ran ~5 min and exited cleanly; no FATAL / STACK OVERRUN / guard hits. MEMBER-NEW showed ~130-150 per
window, but most were scalar fields inside the trace's old coarse regions (peer mask +0x173C, counts/ids at
+0x2240..+0x226C and +0x4450..+0x4468). Filtered to real slot records and table entries: **26 accesses**.
| Accesses | What | Done |
|---|---|---|
| 12 (+6 not hit, same form) | peer table read as `peer + 0x5B0` (block) / `+ 0x5B4` (session), `lea rax,[rax+rax*2]`, `[base + rax*4 (+0x30)]` - the table offset folded into the index, so the 0x4440 text search missed them. 18 sites: 0x38F518 0x393B2E 0x395411 0x3961BE 0x396248 0x396EEF 0x3970DC 0x3971CC 0x3C02BE 0x3E2010 0x3E3560 0x3E3717 0x3E380F 0x400119 0x400220 0x403F3D 0x420252 0x44A441 | caved like the 47 (generator form 2): the short access is patched together with its lea (7-9 bytes, nothing branches into the access), the cave checks peer + bias >= bias + 17 before the lea |
| 1 | 0x2A38C6 in 0x2A3870: counts matching players over the 16 records 0x398088 returns (`add rax,0xB0; cmp edx,0x10`) - would under-count, not overflow | step cave 0x2A38CF: at 16 move to the shadow record 16 and run to 32 on a static session, stock 16 elsewhere |
| 14 | 0x39E091..0x39E119 in 0x39DD4C: compares player records of blocks A/B by pointer | reviewed, blocked: its bounds stay 16/17 because it fills the fixed 17 x 0x180 tables in 0x396828's frame (#32). No overflow; slots >= 16 not compared until #32 |
- `-trace-members` regions now cover only the slot records and the table, so scalar fields no longer show as NEW.
- Known list 504 (= 420 traced + 65 table sites + 4 reset-loop accesses + 0x2A38C6 + 14 in 0x39DD4C).
- Rerun `tools\lan-three-instance.ps1` after the build: expected 0 MEMBER-NEW for the same actions. Not covered yet:
  team change (Slayer default is FFA), lobby reset with all clients present, Firefight/other modes.
