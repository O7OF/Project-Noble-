# Phase 7 progress: AI paths on custom and Forge maps

Written for the next agent. Newest first inside each section. Plan: `docs\PLAN-PHASE7-AI-PATHS-2026-11-21.md`.

## Status (2026-11-21 16:40) - Place a squad at any point; spawn heights from the nav mesh (tested, pushed)

- `spawn::PlaceSquadAt(name, pos)` (commit 7ade3bb): moves the squad's spawn points to a 1.2 m ring around `pos`,
  stands each on the nav mesh (`NavGroundZ`: nearest-height vertex within 4 m in x/y of the runtime pathfinding data,
  vertices at +0x30 count / +0x34 packed, 12 bytes each), calls ai_place, then restores the map's points. Dev trigger
  `-map-ai-at "squad|x,y,z@seconds"` (env `NOBLE_MAP_AI_AT`). This is the call a Forge-menu "place AI" will use.
- Test p8f: two Elites placed at 38.2,32.4 stood at z 3.8-4.1 and walked off (23 m in 15 s).
- p8c-p8e: 36.7,43.4 is a **death spot** (actors placed there end at z -3..-5 and die; a player teleported there
  dies too), so it was the test site, not the code.
- `PlaceMapSquads` (commit 4b0d0b5) now stands every squad spawn point on the nav mesh before placing (the fixed
  authored z 5.5 dropped one actor to z -47/-50). p8g (settings `hostMapAi: 1`): 3 points corrected, 14 AI at
  z 4.0-6.4, none fell; count drops to 12 because Troopers and Covenant fight each other.
- Forge-menu "place AI" is **not wired yet**: Noble's `forge_palette.json` has no spawn UI. Needs a Forge-side
  trigger (palette entry or hotkey) calling `PlaceSquadAt` at the cursor / player, and a hands-on test by Tyler.

## Status (2026-11-21 16:10) - Step 2: MAP AI host option + whole-map zones (tested)

**MAP AI (host option, custom games and Forge lobbies):** `Settings::hostMapAi` (0/1), lobby row "MAP AI" (host
only). In the match the host (`-mode multiplayer|forge`, no `-connect`) checks once a second; 15 s in, if the map has
AI paths (`spawn::MapHasAiPaths()`: a non-zero per-BSP pathfinding pointer and at least one squad), it lifts the
ai_place gate (`spawn::EnableMpAiPlacement()`, the same 5-byte patch as `-mp-ai`) and places every squad
(`spawn::PlaceMapSquads()`); 20 s after the actor count reaches 0 it places them again. Maps without AI paths are
logged and left alone. Guests never place AI.

**Whole-map zones:** `tools\noble-ai-zones-from-navmesh.ps1` walks every walkable sector of the baked nav mesh (links
-> outline), samples a 3.5 m grid inside each, and writes one zone + one objective ("fight" task over all its areas)
per 160 m tile, 20 m areas (<= 64 per zone, the task-area limit), <= 2,000 firing positions per zone (limit 2,048).
Forge World: 7,795 walkable sectors -> 17 zones, 17,398 firing positions in ~3 s; the bake then drops 2,040 points
lying just outside the ground (12%). Squads matching `noble_ai_*` get the zone / objective of their tile. Limits
found with `tools\research\block-maxima.ps1`: zones 128, firing positions 2,048 / zone, areas 128 / zone, objectives
64, tasks 100, task areas 64, squads 256, spawn points 128 / squad.

**Test p8a** (settings `hostMapAi: 1` only, no dev switches, player placed 52 m from the squads): "placed 5 squads |
actors 0 -> 14"; an Elite walked 52 m to the player across the generated zone and killed them on every respawn;
others moved up to 15 m. One spawn point drops an actor to z -50 (spawn height is a fixed 5.5; take it from the nav
mesh next).

Tools: `tools\research\navmesh-near.ps1 -X -Y` (nearest nav-mesh vertices, for test positions).
Note: a `Settings.hpp` change recompiles most files and prints the project's existing warnings (ChangelogData,
main.cpp, ReachHost a1..a3, etc.); none come from these changes.

## Status (2026-11-21 14:15) - PROPER FIX: "don't strip pathfinding" (no solo build, no runtime hack)

The scenario's top-level `flags` field has bit 1 **"don't strip pathfinding"** (bit 13 is "survival"; ff20 = 8192).
The cache builder strips the nav mesh from multiplayer scenarios unless this flag is set. Forge World now:
`type = multiplayer` (unchanged from stock) + `flags |= 2` + `structure bsps[0].flags` bit 9 cleared + bake + build.
- Build: 799 MiB, **identical tag list to the plain MP build (16,992 tags, 0 missing)**, so Monitor, Elite players,
  MP objects and Megalo all stay. The solo-type build (below) dropped 403 tags (`globals\mp` and everything under
  it: Monitor, Elite / elite_noble players, MP objects, Megalo, MP weapon sounds) and must not be used.
- Test p7x (Slayer, `-mp-ai` only): scenario type 1, pathfinding pointer set, 12 AI walk up to 12 m and fight.
  p7y: Forge mode loads with the nav mesh.
- `-mp-ai-pathing` (type reset) is no longer needed; kept as a dev switch for solo-built maps only.
- Backups: `HREK\noble_mods\phase7_backup_2026-11-21\forge_halo.map.mp_keep_pathfinding` / `.scenario.mp_keep_pathfinding`.
- Tools: `noble-tag-set-flags.ps1 -Block ''` edits top-level flags; `noble-tag-set-enum.ps1` sets enums.

### Size (14:20)
- Shipped today: main package 360 MB zipped; separate Forge World pack `Noble-ForgeWorld-v0.20.34-win64.zip` 653 MB
  (its map 760 MB; HREK builds a self-contained map, stock MCC forge_halo.map is 332 MB because it uses MCC's
  shared maps).
- AI-capable MP build: 799 MiB, **682 MB zipped (+29 MB, ~4%)**. The AI work does not double the install; the
  Forge World pack was already a separate ~650 MB download.
- `tool build-cache-file <scenario> pc english compress-most`: ran ~25 min, then "hash mismatch, verification
  failed" and crashed; no map written. Not usable.
- Untried, biggest possible saving: build with MCC's shared maps (`build-cache-file-use-sharing`, see
  `HREK\BuildMapsSharedOptimizedPlusSharedSounds.ps1`), back toward the stock 332 MB. Needs a hash check of the
  player's MCC shared maps and a re-check after MCC updates.

### Number 2 design (next session / 15:45 resume)
- `-mp-ai` -> a real host option (lobby row "AI" on maps whose scenario has baked pathfinding; detect at runtime by
  the per-BSP pathfinding pointer being non-zero) instead of a dev switch.
- Whole-map firing positions: generate areas / firing positions from the baked sectors (scenario
  `ai pathfinding data` -> sector polygons via links / vertices), not a local grid; one zone, a few objectives.
- Forge-menu AI: Noble already NOPs the Forge AI spawn check (ForgePatches, hr+0x692CB5) so the menu places biped
  OBJECTS without actors. Plan: treat a Forge-placed AI biped as a spawn marker: remove it and place a squad there
  (one generic `noble_ai_dynamic` squad per character whose spawn point Noble moves at runtime, or
  `PlaceTemplateDatum` with a squad template). 360 `actor_create_internal(unit, swarm, character, squad, bool)`
  (0x82613F98) is the alternative (attach an actor to an existing biped); MCC address not found yet.

## Status (2026-11-21 13:25) - AI WALK ON FORGE WORLD (superseded by the 14:15 fix above)

**Proven (run p7v):** on a Forge World built with scenario type = solo, with `-mp-ai -mp-ai-pathing`, 14 AI spawn,
walk on the baked ground (up to 15 m, 6-10 of 12 moving per 5 s), close to 1-10 m of the player and fight; the
Slayer match ran 90 s with no crash. Forge mode ("Basic Editing") also loads and runs on that build (p7w).

**The blocker that took the afternoon:** HREK's cache builder packs the baked nav mesh into the structure BSP's
resource **only for solo scenarios**. ff20 (and every Firefight map) is a *solo* scenario; Forge World is
*multiplayer*, so on an MP-built map the runtime pathfinding pointer stays 0, every actor's sector lookup fails
(actor `+0x500` "no pathfinding location" = 1) and AI can shoot but never walk. Zones / firing positions / objectives
were not the cause (they are still needed for good behaviour).

**The fix (dev):** build Forge World with `scenario.type = solo` (keeps the nav mesh), and at the first game tick of
a multiplayer match set the in-memory type (u16 at scenario+0) back to 1 = multiplayer (`-mp-ai-pathing`). Without
that reset the solo type crashes player-view / HUD code (`hr+0x30D88A` in `0x30D5A8`) after ~25 s (run p7s).

### Runtime facts (MCC haloreach.dll, verified)
- Per-BSP runtime records: `hr+0x27B7BA0`, 0x490 bytes each, filled by `0x97C70(bsp, record)`; record +0x78
  (= `hr+0x27B7C18 + bsp*0x490`) = pathfinding data pointer. Set only if the BSP is active (bit in `hr+0x4E2FB2C`),
  `scenario.structure bsps[bsp].flags` (element 0xAC, u16 at +0x50) lacks 0x200 "no pathfinding", and the BSP tag's
  resource (u16 index at BSP tag +0x520) holds a non-empty block.
- `collision_surface_get_sector` = `0x724D24` (reads that pointer; 0 -> no sector).
- `biped_find_pathfinding_location` = `0x4A4424`: biped +0x9DC bsp, +0x9E4 ground collision surface, +0x9E8 sector.
- `actor_find_pathfinding_location` = `0x75B2C8`: actor (0xD00) +0x40C computed flag, +0x410 point, +0x41C ref frame /
  bsp, +0x420 sector ref, +0x4EC last valid, **+0x500 = 1 when no location** (360 layout is MCC - 0x10 here).
- Scenario tag: u16 type at +0 (0 solo, 1 multiplayer); structure bsps count +0x50 / block +0x54.
- ff20 in multiplayer mode and Forge World in Firefight mode both fail to start (engine loop returns 1).

### Test squads, zone, objective (HREK scenario, scripts in tools\)
- Squads (`noble-forge-ai-squads.ps1 -Centre @(38.4,84.5,5.5) -Replace`): 5 squads, 14 AI, 6 m around the MP
  respawn point (38.4, 84.5) on the Island (dry land; the scenario's player start at (-157,-225) is a beach and
  `-player-at` there drowned the player).
- Zone / objective (`noble-forge-ai-zone.ps1 -CentreX 38.4 -CentreY 84.5 -CentreZ 4.4`): `noble_zone` with 16 areas
  (4x4 grid of 12 m, corners from baked vertices), 576 firing positions (2 m grid; heights from nearby baked
  vertices; tool.exe does not generate firing positions, only Sapien does), `noble_obj` with one task
  `noble_fight` over all areas, squads' initial zone / objective set. `generate-pathfinding` fills the firing
  positions' cluster / sector (50 of 576 report "outside BSP" and are dropped).
- Run order: squads -> zone -> `tool generate-pathfinding` -> `tool build-cache-file`.

### Earlier status (12:00)
14 AI spawn, stand on baked ground and attack, but hardly move. (Cause found at 13:15, above.)

## Decision: approach

Changed from the plan's option C (Noble's own bake + sidecar) to **option A first: bake with HREK's own tool**, because:

1. HREK `tool.exe generate-pathfinding <scenario>` exists and works from the command line (no Sapien needed).
2. Noble already ships its own HREK-built `forge_halo.map` (`data\maps`, Pelican / Spectre / Boathog), so a rebuilt
   Forge World is an existing delivery path, not a new one.
3. The bake writes the engine's real data (`scenario.ai pathfinding data`), so no in-memory sector surgery is needed
   for the map's own ground. The bake also "stitches in pathfindable objects" (scenario-placed scenery), which is the
   route for Forge layouts later (place a layout's pieces as scenery in a scenario copy, bake, ship the map).

Option C stays the plan for arbitrary player-made layouts if per-layout map builds turn out too heavy.

## Findings

### Stock Forge World has no pathfinding, no AI characters, no squads
- `forge_halo.scenario`: `structure bsps[0].flags` raw 512 = bit 9 **"no pathfinding"**. With it set,
  `generate-pathfinding` reports success but writes nothing. ff20 sets the same flag only on its non-playable BSPs.
- `ai pathfinding data` (scenario block, one element per BSP: sectors, links, bsp2d refs / nodes, vertices, object
  refs, hints, instanced geometry refs, seams, doors) was empty. The BSP tag's own `pathfinding data` block is empty
  on every map; the data lives in the scenario.
- `character palette`, `squads`, `zones`, `ai objectives`: all 0. The `.map` carries the squad templates (from
  globals) but no `objects\characters\*\ai\*` characters, so nothing could be built from them.

### After the bake (flag cleared)
- 54 s for BSP 0: **10,972 sectors, 37,771 links, 60,685 bsp2d refs, 27,178 vertices, 492 hints, 2,217 instanced
  geometry refs, 3 seams**. Scenario tag 11.25 MB -> 13.37 MB.

### Engine gate: AI only in campaign / survival
- `ai_place` core `0x6C2B10` returns at once unless `0x58B5C` (360: `game_is_campaign_or_survival`) is true:
  game globals (root+0x48) +0x10 == 1 (campaign) or 2 (survival). Multiplayer and Forge are **3**.
- `0x694CB4` (per-squad placement) also needs AI globals (root+0x30) +1 != 0; it is 1 on Forge World.
- `game_is_campaign_or_survival` has 107 call sites (team relations, damage, metagame, vehicles); the AI update loop
  is not one of them, so placed actors run in MP.
- Dev switch `-mp-ai` patches only the call at `hr+0x6C2B2B` (`E8 2C 60 99 FF` -> `B0 01 90 90 90`, bytes checked
  first). Noble's `PlaceTemplateDatum` calls `0x694CB4` directly and never needed this.

### Tests (all `tools\mp-session-test.ps1 -Players 1 -Map forge_halo -Mode multiplayer`, map override to the
HREK build)
- p7a / p7b (no `-mp-ai`): 5 squads exist at runtime, 0 actors placed; gate values AI +1 = 1, game +0x10 = 3.
- p7c (`-mp-ai -horde 14 -horde-no-teleport -horde-full-lod`): 14 actors at the spawn points (player start
  -157,-225), dropped from z 4.8 to ground ~0 and stayed; a few moved 1-2.5 m (idle). Player 510 m away.
- p7d (same, teleport on): actors moved 4-32 m from the player (z 44.6, the spawn platform). Player killed by an
  Elite sword (capture `p7d-*-host-cap0.png`); 3 actors died. No walking (0 moved > 1 m per 5 s).

## Tools added
- `tools\noble-tag-dump-block.ps1` - fields and values of one top-level block (ManagedBlam, read-only).
- `tools\noble-tag-flags.ps1` - flag names / raw values of a block's flags fields.
- `tools\noble-tag-set-flags.ps1` - set / clear bits in one flags field (dry run unless `-Save`).
- `tools\noble-forge-ai-squads.ps1` - adds characters to the character palette and test squads (spawn points + one
  designer cell) to a scenario (dry run unless `-Save`; skips what exists).
- `tools\research\extract-func.ps1` (function body from a Ghidra text export by RVA or name),
  `callers-of.ps1` (which functions contain a pattern), `find-call.ps1` (E8 calls to a target inside a function,
  through the PE section table), `map-tag-names.ps1` (tag names in a `.map` matching a regex).
- noble-dev: `-mp-ai` (above); `-ai-watch` (every 5 s: actor count, how many moved > 1 m, furthest from spawn, z
  range, distance to the player, first 6 positions; with no actors it logs the two ai_place gate values).

## HREK state (D:\Steam\steamapps\common\HREK)
- Backups before Phase 7: `noble_mods\phase7_backup_2026-11-21\` (forge_halo.scenario, both BSP tags, seams,
  `forge_halo.map.before_phase7` = the shipped Pelican / Spectre build).
- Changed: `levels\multi\forge_halo\forge_halo.scenario` = flag cleared + baked pathfinding + 5 characters + 5 squads
  (`noble_ai_elite/grunt/jackal/brute/trooper`, 14 actors, 6 m around the player start).
- `maps\forge_halo.map` rebuilt (836 MB). **Not copied to `data\maps`** (the shipped map is unchanged).
- Logs: `noble_mods\phase7_genpath_1.log` (flag set, empty), `phase7_genpath_2.log` (real bake), `phase7_build_1.log`.

## HREK state (13:25)
- `forge_halo.scenario` is now **type solo** + baked + squads / zone / objective. Backups in
  `noble_mods\phase7_backup_2026-11-21\`: `forge_halo.scenario` (stock), `*.mp_with_ai_13h15` (MP type with AI,
  scenario and map). `HREK\maps\forge_halo.map` = the solo build (803 MB). `data\maps` (shipped) unchanged.
- Dev switches: `-mp-ai`, `-mp-ai-pathing`, `-mp-ai-all` (diagnostic only: forces game_is_campaign_or_survival),
  `-ai-watch` (now also dumps one actor, the per-BSP pathfinding pointers and the biped ground fields once),
  `-player-at x,y,z@s`.

## Next (16:40; Tyler's order 1 -> 5)
1. Done: solo build dropped 403 MP tags; fixed with "don't strip pathfinding" (14:15).
2. Mostly done: MAP AI host option, whole-map zones, PlaceSquadAt, nav-mesh heights. Left: Forge-menu trigger for
   `PlaceSquadAt` (hands-on test); team rules (Troopers vs Covenant vs players).
3. Forge pieces: place a small layout's objects as scenery in a scenario copy, bake, confirm AI walk onto them.
4. One-command bake script for other maps (BSP flag, "don't strip pathfinding", characters, squads,
   generate-pathfinding, zones from nav mesh, build-cache-file).
5. Shipping: map size, map hash for online play, guests (the host places AI; guests need the same map).
