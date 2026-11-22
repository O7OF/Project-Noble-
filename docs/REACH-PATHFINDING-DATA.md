# Reach runtime pathfinding data (MCC haloreach.dll 1.3528.0.0) - Phase 7 step 3 research, 2026-11-21

Written for the next agent. Goal: let AI walk onto Forge pieces by adding nav data at runtime (plan option C),
because HREK's `generate-pathfinding` deletes objects added to Forge World's scenario (`docs\PHASE7-AI-PATHS-PROGRESS.md`).
Sources: MCC decompile (`D:\MCC_DECOMP\...\hr_000NN.c`, `tools\research\extract-func.ps1`) and the dev probe
`-nav-dump` (noble-dev; log of run p11a, Forge mode on the AI Forge World). Tags: **proven** = measured at runtime;
**decompile** = read from the decompile, not yet exercised.

## 1. Where the data is
- Per-BSP runtime record: `hr+0x27B7BA0 + bsp*0x490`; the nav data pointer is at +0x78 (`hr+0x27B7C18 + bsp*0x490`),
  set by `0x97C70` when the BSP loads (needs the BSP active in `hr+0x4E2FB2C`, no "no pathfinding" flag, non-empty
  BSP resource). **proven**
- Addresses inside are packed tag addresses: `pages[packed >> 28] + packed * 4` (64-bit arithmetic), page table
  `hr+0x4E39F20` (16 qwords). On Forge World pages 1-5 and 12-15 are used, **0 and 6-11 are free**. **proven**
- Nav data = tag block headers, 12 bytes each (count i32, packed address u32, definition pointer):

| Offset | Block | Element size (measured) | Fields (tag definition order) | Forge World BSP 0 / 1 |
|---|---|---|---|---|
| +0x00 | sectors | 8 | flags u16 (bit 0 walkable), hint index i16, first link i32 | 10,953 / 360 |
| +0x0C | links | 16 | vertex 1, vertex 2, flags, hint, forward link, reverse link, left sector, right sector (u16 each) | 37,761 / 1,364 |
| +0x18 | bsp2d refs | 4 | node or sector ref (i32) | 60,685 / 1,857 |
| +0x24 | bsp2d nodes | 20 | plane (a, b, d floats), left child i32, right child i32 | 41,882 / 1,509 |
| +0x30 | vertices | 12 | point (x, y, z) | 27,186 / 983 |
| +0x3C | object refs | 24 (decompile) | flags, bsps block (12-byte header at +4), object id | 0 / 0 |
| +0x48 | hints | - | type, next, data 0-3 | 492 / 476 |
| +0x54 | instanced geometry refs | - | pathfinding object index | 2,217 / 58 |
| +0x60 | structure checksum | | | |

## 2. How an actor finds its sector (decompile)
- `actor_find_pathfinding_location` `0x75B2C8` -> `0x75AF38(unit)` -> for bipeds `biped_find_pathfinding_location`
  `0x4A4424`: biped +0x9DC bsp, +0x9E4 ground collision surface, +0x9EC / +0x9F0 (ground instance / **ground
  object**), +0x9F4, +0x9F8 point; it calls `collision_surface_get_sector` `0x724D24` and caches the sector ref at
  +0x9E8.
- Sector ref = u16 BSP index + u16 sector index; valid when the BSP is active and sector < that BSP's count
  (`0x72DE28`).
- `0x724D24(bsp, surface, instance, object, objectBsp, point)` has three cases:
  1. structure surface -> bsp2d lookup in the BSP's own tree (`0x775350(data, plane, surface, data+0x18)`);
  2. instanced geometry -> instanced geometry refs (+0x54) -> object ref -> its bsp2d tree;
  3. **object** (`object != -1`) -> `0x76F1E8(object, bsp)` gives the object ref index -> object ref (+0x3C, 24 bytes) ->
     its per-BSP entries (block at ref +4 / +8, 24 bytes each; entry +0 compared with the object's collision BSP
     index) -> the point is moved into the object's space (`0x18DD90` / `0x18E13C`, object node matrices) -> bsp2d
     lookup in that entry's tree (entry +8). So object sectors are stored **in object-local space**.
- `0x76F1E8(object, bsp)`: `0x46D274(object + 0x88)` returns the object's **scenario placement entry**; for scenery
  (type 6) / crates (10) the entry's "pathfinding references" block is at +0x74 (+0x7C for machines 7 / type 12);
  element[bsp] +2 = object ref index. Objects without a scenario entry (Forge pieces from a map variant) return -1.

## 3. Runtime design for Forge pieces (option C, not built yet)
1. **Per piece type, once:** build walkable sectors for the piece's collision model in object-local space (top
   faces with a walkable slope; polygons -> sectors, edges -> links, a bsp2d tree over them). Same data for every
   placed copy of that piece. Offline (from HREK collision models) or at load (from the map's collision model tag).
2. **At match start:** allocate Noble memory, map it as a free tag page (e.g. 6), copy the BSP's nav blocks there
   with room to grow, append the pieces' sectors / links / nodes / vertices and one object ref per placed piece,
   and repoint the block headers. Re-run when Forge edits change the layout (pieces that move are disabled).
3. **Hook `0x76F1E8`:** for a registered Forge object return its object ref index; otherwise call the original.
4. **Links between the piece and the ground:** without them AI can stand on a piece but not path onto it. Stock
   bakes create them in "Stitching objects together"; Noble must add links (or hints) where a piece's walkable edge
   touches ground sectors. This is the least known part; study a baked map with object refs (ff60_airview: 182).
5. Firing positions on pieces (optional): AI already use dynamic firing positions near players.

## 4. Probe results (2026-11-22 00:30-01:00, runs p13a-p13h, AI Forge World)
Dev switches: `-nav-probe` (hooks `0x76F1E8`, logs each object asked about; every 5 s calls `0x75AF38` on the
player like an AI asking for its target, and logs the player's ground fields), `-forge-piece-at <tag hex>|x,y,z@s`
(places a Forge piece with object_placement_data_new / object_new; picks a tag by a weak heuristic, it placed
`ff_bridge_diag` 0x24DA), `-map-ai-at "!squad|..."` (no snap to the nav mesh).
- **Piece resting on the ground** (p13g, bridge at z 3.6, player standing on it, capture confirms): the engine
  reports the **ground BSP surface and a valid sector** under the player. Low pieces need nothing: AI locate a
  player on them through the ground below. **proven**
- **Raised piece** (p13h, bridge at z 7, player on top at z 7.0): ground bsp / surface / object all -1, location
  sector ref `FFFFFFFF`. `0x76F1E8` is **never called**: the ground finder (`0x4A41A4`, called from `0x4A4424`) does not
  treat a Forge crate as pathfinding ground at all. **proven**
- The map's own spawn platform (63.5,236.4,44.6) is BSP, not a Forge piece.
- `ai_place` places nothing at a spawn point with no nav sector under it (Elites above a raised piece: actors 0 -> 0).

**Design change:** hooking `0x76F1E8` alone is not enough. Next: decompile `0x4A41A4` (which collision it accepts as
ground: BSP, instanced geometry, objects with pathfinding data) and hook there for bipeds over a registered Forge
piece. Cheap partial fix first: when a biped is over a raised Forge piece, return the ground sector straight below
(AI then come to the spot under the piece and shoot up instead of losing the target). Full fix as in section 3.

Proof order: (a) hook `0x76F1E8` and log which objects the engine asks about while a biped stands on a Forge
block (confirms the path); (b) one hand-made flat sector for one block type, registered for one placed block, check
`+0x500` (no location) clears for an actor standing on it; (c) links to the ground; (d) the generator.
