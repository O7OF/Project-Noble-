# C11: visible render list / cached render states, 4,096 → 16,384 (research note, 2026-11-08 08:40 ET)

Status: **research only, nothing changed.** Written by DeepSeek from the V16 recipe and the built engine
(`%LOCALAPPDATA%\ProjectNoble\engine\v16-<hash>\haloreach.dll`). Tyler's repro: F9 ×10 in Forge
(5,000 objects), look at one spot — flicker and the weapon/HUD disappear past ~4,096 visible objects.

## What is already 4,096 (V16, guide "Cached object render states" / "Visible list and mesh pool")

| Thing | Stock | V16 (now) | Where the value lives |
|---|---|---|---|
| Cached object render states | 1,024 | **4,096** | capacity operand at RVA `0x2565E6` (`00 04 00 00` → `00 10 00 00`), 624-byte record |
| Visible render list entries | 1,024 | **4,096** | capacity operand at RVA `0x14EAB7` (same `0x400` → `0x1000`), 16-byte record at list+`0x1C` |
| Visible-list header | `0x0291C7B0` | `0x12BE9000` | V16 append block |
| Mesh header | `0x029207C0` | `0x12BF9020` | V16 append block, 168-byte records at mesh+`0x20` |

Verified in the built V16 DLL (cached copy `v16-11e3ac2c31ec`): both operands read `0x1000`.
The live pool report shows the first one: `POOL|cached object render states|10|4096|624|live`
(4,096 × 624 = 2,555,904 bytes of game-state memory today).

## What 16,384 would need

1. **Capacity operands** `0x2565E6` and `0x14EAB7`: `0x4000` instead of `0x1000`. Two overwrite rows, same
   pattern as V16's own rows (stock-byte check `00 10 00 00`).
2. **The two header blocks in V16's append block** are packed back to back:
   `0x12BE9000` (visible list: 0x20 header + 4,096 × 16 = 0x10020 total) then `0x12BF9020` (mesh: 0x20 +
   4,096 × 168 = 0xAC020). At 16,384 they need `0x40020` and `0x2A0020` — **+2.2 MB** in the append block,
   and every relocated reference into the two blocks (the V16 recipe's `STATE_BANKS.md` / `COORDINATED_OPERANDS.csv`
   tables) has to move with them. This is the same job `make_v16_particles.py` does for the particle block,
   just for V16's own headers, so it means rebuilding V16's append, not a small patch.
3. **Cached object render states pool**: 16,384 × 624 = 10,206,464 bytes (**+7.65 MB**) from a game-state bank.
   If the engine creates it through `data_array_new` (RVA `0x12FCC`, signature 0x64407440, 624-byte records),
   `src/host/PoolResize.cpp` can raise it at runtime by name — the pool report already prints its exact name,
   so this half needs **no** .nlp change: add `{ "cached object render states", ... }` to `kRules` and check the
   bank has room. The capacity operand at `0x2565E6` suggests a direct constant in its constructor instead;
   check which one before choosing (hook `-pool-trace` logs the allocator and data pointer for every pool).
4. **Memory** (LIMITS-HANDOFF.md formula): checkpoint = render + main + 0x260000; total = checkpoint + 2·main.
   +7.65 MB into the render bank grows the checkpoint and total by the same amount per copy; with V16+particles
   the render bank is 0xD80000 / checkpoint 0x3DB0000 — well inside the 64-bit address space, but the
   append-block growth also needs `.deltrac`/`.hkwork` room (`make_v16_particles.py` already checks the block
   fits before `.hkwork`).

## Order of work (suggested)

1. Try the runtime half first: `cached object render states` via `PoolResize` (name already in the pool
   report). If it is a data array, that fixes half of the flicker with no engine rebuild.
2. The visible list/mesh half needs the V16 append rebuilt — treat it as its own recipe step in
   `make_v16_particles.py` (larger block, recomputed fixups), do **not** hand-edit offsets.
3. Test with `-ai-actors 512` and F9 ×10 in Forge (5,000 objects) plus `-spawn-test 9000`; watch
   `POOL|cached object render states` and the visible list; PASS = no flicker, weapon/HUD stay on screen.
