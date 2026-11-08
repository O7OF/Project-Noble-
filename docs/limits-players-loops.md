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
4. **Step loops: 6 installed, 8 blocked, 11 not reviewed** (lists in `gen_steps.py`). Installed:
   0x391B94 (offset form), 0x39322C, 0x39EC61, 0x39ECD8, 0x408F2E, 0x411986. Blocked because the same loop fills a
   fixed table: 0xD8C9, 0x39E212/9, 0x3FFD7F, 0x41E9B5, 0x41F576, 0x41F61E, 0x41F95B.
5. **Bounds: 137 raised, 41 kept at 16/17.** Kept = every entry in a function that also fills a fixed table
   (0xD6B8, 0x38DA74, 0x390DB0, 0x39C6B4, 0x39DD4C, 0x3C0248, 0x40A760, 0x41E7C8, 0x41F510) and the 3 `other`
   entries. 0x390DB0's `cmp rcx,0x10` copies are 16-character name lengths and must never be raised.

## Still open before 32 players can be switched on for real
**Fixed tables (16/17 entries, outside the session object unless noted):**
| Where | Table | Needed |
|---|---|---|
| 0xD6B8 | 16 × 0x20 player list at rbp-0x80, passed to the host interface `[0xC1A100]` vtable +0xF0 | find the receiver (Noble or engine), then a 32-entry list |
| 0x390DB0 / 0x38DA74 | 16 qword pointers at rbp+0x150 | bigger frame or a Noble buffer |
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
