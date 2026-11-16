# Swarm zombie test (Renette's flood tags) — parked 2026-11-08

Status: parked to work on player counts. Tags install and build; spawning in waves is not solved yet.

## What is installed (HREK, D:\Steam\steamapps\common\HREK\tags)
- Renette's real tags (from Discord, 2026-11-08):
  - `objects\characters\zombies\zeke\swarm\swarm_civilian\swarm_grunt_old.character` (his current character)
  - `objects\characters\zombies\zeke\swarm\ai\style\swarm_old.style`
  - `objects\weapons\zombies\swarm_claw\swarm_claw.weapon` (energy sword based)
  - `objects\characters\floodcombat_human\shaders\{floodcombat_human,floodcombat_damaged,flood_fronds}.shader`
  - `objects\characters\flood_infection\shaders\flood_infection.shader`
  - bitmaps in `floodcombat_human\bitmaps\` and `flood_infection\bitmaps\`
- From the earlier upload: `floodtohr\floodtohrasset\*` (model, render/collision/physics/imposter model, animation graph, frame events), `swarm_civilian.biped`, `run.effect`, damage effects.
- Still missing from Renette: `flood_infection\bitmaps\flood_fronds_{diffuse,zbump,illum}.bitmap`.
- `swarm_civilian.character` (my edited civilian copy) is unused now; the test uses his `swarm_grunt_old`.

## Unearthed test build
- Edited `levels\dlc\ff_unearthed\ff_unearthed.scenario` character palette: grunt, grunt_heavy, grunt_major, grunt_specops, grunt_ultra, jackal, jackal_major -> `swarm_grunt_old`.
- Original backed up as `ff_unearthed.scenario.orig` next to it (restore by copying over).
- Build: `HREK\build_swarm_unearthed.bat` (log `HREK\build_swarm_log.txt`). Builds clean, ~2-4 min. Output `HREK\maps\ff_unearthed.map`.
- Run: `D:\Project-Noble-Closed-Source\run_swarm_test.bat` / `run_swarm_test_fulllod.bat` (noble-dev.exe, `-map-override`, infantry-only on; logs `swarm-normal.log` / `swarm-fulllod.log`). Stock game files are not touched.

## Findings
- Spawn keys moved 2026-11-09: F6/F7/F8/F9 are now Numpad 6/7/8/9 (Num Lock must be on). Entries below use the old F-key names.
- F7/F6 only exist in noble-dev.exe (SpawnTest is NOBLE_DEV_BUILD only).
- The swarm character loads and spawns in game (seen once as a turret gunner).
- Unearthed's F6-placeable squads (`sq_sur_wave_01..08`, `sur_fireteam_01..04`) get their characters from the
  Firefight wave system at spawn time (no squad template field set; waves 1-4 have one designer cell, rest none),
  so palette swaps don't reach them -> F6 spawned jackals.
- Firefight wave squad templates (`ai\squad_templates\sq_ff_grunt_4`, `sq_ff_cov_e1_g3`, `sq_ff_cov_b1_g3`) reference the stock grunt characters + guns directly.
- Build warnings (non-fatal): flood model materials not in Reach global materials (generic impact fx); character asks for model variant `swarm` that the model lacks (falls back to default).
- Renette: MCC AI behaves better than editing-kit AI; pauses with ~90+ actors are likely the AI LOD scheduler (try `NOBLE_HORDE_FULL_LOD=1`); evade/dive works for enemy team, so swap those animations instead of idle.

## Next step when resumed
Add a new squad (fixed cell: swarm_grunt_old + swarm_claw, spawn points copied from an existing squad) — easiest in
Sapien (Renette's workflow), or ask Renette for his squad/scenario setup. Then rebuild and F6. Foundation's field
search and grid editing were unreliable for nested squad cells.

## 2026-11-09 night: waves spawn swarm zombies; models render white
- `tools\swarm\swarm-swap-build.ps1`: backs up HREK's grunt/jackal character tags and the grunt/jackal weapon tags,
  copies `swarm_grunt_old.character` / `swarm_claw.weapon` over them, runs `build_swarm_unearthed.bat`, always
  restores the originals (hash-checked). Wave squad templates (`sq_ff_grunt_4`, `sq_ff_cov_e1_g3`, `sq_ff_cov_b1_g3`)
  use the 5 grunt characters + brute captain / elite officer leaders. `jackal_sniper.character` inherits from jackal.
- Run 04:42 (full LOD, F6 + F7): swarm zombies spawned, 1,024 actors, no crash, ~17-19 fps at 900-1,024 actors
  (same as earlier full-LOD hordes). F7 places the map's own squads: jackal snipers came out as zombies with beam
  rifles -> jackal weapons added to the swap list.
- **White models:** the render model uses `floodcombat_human\shaders\floodcombat_human` + `flood_fronds`; the
  shaders' bitmaps exist and are fine. The four flood shaders reference **10-index (Halo 3/ODST style) shader
  templates** (`_0_2_0_1_1_2_0_0_0_0`, `_0_2_0_1_1_0_0_0_0_0`, `_0_1_1_2_0_0_1_0_0_0`, `_0_2_0_1_1_0_1_0_0_0`);
  Reach templates have 12 indices (HREK has 2,118 of them) -> no template -> white. Fix: Reach versions of the
  shaders from Renette, or re-save them in Foundation (then rebuild).
- Still missing: `flood_infection\bitmaps\flood_fronds_{diffuse,zbump,illum}.bitmap`. `flood_infection.bitmap` /
  `_zbump` have exactly the sizes of `floodcombat_damaged.bitmap` / `floodcombat_human.bitmap` (maybe copies).
- **Body texture fixed (05:08):** `floodcombat_human.shader` re-made in Foundation as a Reach shader (copy of
  `grunt\shaders\grunt_skin.shader`: albedo default, bump detail, cook_torrance) with base_map
  `floodcombat_human`, bump_map `floodcombat_human_zbump`, bump_detail_map `flood_skin1_zbump`. Original (H3) kept in
  `build\swarm\floodcombat_human.shader.bak`. Leftover from grunt_skin: albedo `detail_map = skin_detail`.
  Tendril tips (`flood_fronds.shader`) still white: H3 shader + missing `flood_fronds_*` bitmaps (Renette).
- **Movement (05:25):** `swarm_grunt_old.character` already allows it: jump height `infinite`, gain elevation, climb,
  perch, boosted jump, cure isolation jump, high priority pathfinding. With F6's teleport the squads crowded under a
  ledge (kept their Firefight task areas); with `-horde-no-teleport` they spawn at their own points, path up, jump
  and lunge at the player (05:33). Both swarm .bat files now pass `-horde-no-teleport` (infantry-only stays on).

## Status and next steps (2026-11-09 10:20, parked behind multiplayer)
Tyler's order: multiplayer -> multiplayer optimization / bugs -> swarm + Pelican + ReachVariantTool.
What works: waves spawn swarm zombies (Renette's `swarm_grunt_old`), bodies textured, jumping/pathing with
`-horde-no-teleport`; 1,024 actors held without a crash (full LOD run 04:42, ~17-19 fps at 900-1,024 actors).
Open, in order:
1. **Swarm-only Firefight variant** in ReachVariantTool from `hr_ff_standard_1set.bin`: every squad slot
   Grunts/Jackals (keep Squad Count defaults 3/5/3, empty slots become Brutes), Uses Dropship off, Bonus Wave off;
   save `swarm_ff.bin`; pass it with `-variant` (exists in main.cpp) from the swarm .bat files. Stops Hunters and the
   map's vehicle waves.
2. **Tendril tips white:** `flood_fronds.shader` is an H3 shader + missing `flood_fronds_{diffuse,zbump,illum}`
   bitmaps (ask Renette). Stopgap: copy the fixed body shader over it.
3. **Audio:** replace grunt voices with `Desktop\The Flood\flood_gm2\floodcombat` (233 WAVs); first find where the
   swarm character's voices come from (its voices block is empty: biped `swarm_civilian.biped` or parent
   `ai\generic.character`).
4. **Performance goal (Tyler):** 1,000 actors playable without tanking frame rate, AI still behaving. Ideas to
   measure first (BANK / POOL diagnostics, `-pool-trace`): actor LOD (`ai_force_lod` exists at RVA 0x70AB58, full
   LOD hurt fps), the AI update scheduler (0x70ABBC, 1,024 entries), fewer firing-position / prop searches per actor,
   render-state budget, and imposters for distant bodies. Profile before changing anything.
5. Swarm .bat files: `run_swarm_test.bat`, `run_swarm_test_fulllod.bat`; swap/build script
   `tools\swarm\swarm-swap-build.ps1` (uncommitted).
