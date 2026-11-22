# Phase 7 progress: AI paths on custom and Forge maps

Written for the next agent. Newest first inside each section. Plan: `docs\PLAN-PHASE7-AI-PATHS-2026-11-21.md`.

## Status (2026-11-22 01:05) - our open items closed or advanced

- **Dedicated server tick (Phase 5 item):** `0x61A38` is called from `0x62A78` in a loop over **local** players only
  (max 4). `noble-dedicated` loads haloreach.dll but runs no engine session (stub), so nothing breaks today; when it
  gets a session, GameThreadTick work (Firefight allies, wave pacing, MAP AI, PLACE AI) needs a game-thread hook not
  tied to a local player.
- **Guest up close (p12a, 2 players, MAP AI):** the guest's capture shows an Elite, a Jackal and a Grunt on screen
  firing plasma at the guest, 9 red radar dots; the guest died and respawned; no crash. AI reach guests through the
  engine's own sync. (`-player-at` on a guest moves player 0, the host's body, locally - use it on the host only.)
- **Team rules (p12b):** 3 Troopers alone killed the player twice; Covenant also attack players; Troopers and
  Covenant fight each other. Kept as is (every AI fights everyone); allying Troopers with players is Tyler's call.
- **Forge pieces (step 3):** probes and results in `docs\REACH-PATHFINDING-DATA.md` section 4 (commit 618be77): pieces
  resting on the ground already work (the engine uses the ground sector below); raised pieces give no sector and the
  engine's ground finder (`0x4A41A4`) ignores Forge crates, so that is the next hook point.
- **ReShade** (Tyler's request): ReShade 6.8 + RenoDX / DLSS5 feeder (installed 2026-10-17) were in MCC's
  `Binaries\Win64` (and `host64`), and ReShade's `dxgi.dll` was being loaded into `noble-dev.exe` (Noble loads Reach from
  that folder). All 24 items moved (not deleted) to `D:\MCC-ReShade-backup-2026-11-22\`; next run created no ReShade log.

## Status (2026-11-21 23:50) - PLACE AI button, AI Forge World in the pack, step 3 research

- **PLACE AI / AI TYPE** (commit b1bbf2f): host's match menu in Forge. PLACE AI puts the chosen `noble_ai_*` squad
  (Elites, Grunts, Jackals, Brutes, Troopers; AI TYPE cycles) on the nav mesh in front of the player (6 m, then
  4/8/3/10/12 m, then where the player stands); result on the in-match note card ("Placed 2 Elites in front of you."
  or why not). Queued for the game tick (`spawn::RequestPlaceAiInFront`). Dev trigger `-place-ai-at <kind>@<s>`.
  p10b: 2 Elites placed 4 m ahead and walked; p10c (no map override): MAP AI placed 14 and PLACE AI 3 Jackals.
  **Waiting for Tyler's hands-on test.**
- **AI Forge World is now the Forge World pack map** (Tyler, 22:44): `build\bin\Release\data\maps\forge_halo.map`
  = the HREK AI build (838,791,168 bytes, SHA-256 EB9899D5...FDB9). The old one (v0.20.34, 81CD2BDE...) is backed up as
  `HREK\noble_mods\phase7_backup_2026-11-21\forge_halo.map.shipped_v0.20.34`. Tag check: the AI build has 872 more
  tags (AI characters and effects) and lacks 2 (`spartans\fx\flashlight\flashlight_1p`,
  `armor_fx\cheap_lights\bungie_helmet`), which only the 11-13 flashlight experiment's `spartans.biped` (now
  `spartans.biped.bak`) referenced; the flashlight is parked. `publish-release.ps1` packs this file into the
  ForgeWorld zip at the next release (no release made). Guests need it too (O7 map-check item).
- **Step 3 research** (commits fde2836, 79cc976): `docs\REACH-PATHFINDING-DATA.md` - runtime nav layout (element
  sizes measured with the new `-nav-dump` probe), how an actor's sector is found, and that objects resolve through
  `0x76F1E8` (scenario placement entry -> pathfinding references -> object ref; object sectors in object-local
  space). Forge pieces have no scenario entry, so the route is: hook `0x76F1E8` for Forge pieces, add per-piece-type
  sectors in a free tag page (pages 0 and 6-11 are free), and add links to the ground. Proof order in that doc.

## Day summary (2026-11-21, reviewed 21:00): what, how, why

| Commit | What | How | Why |
|---|---|---|---|
| `f648900` | `-mp-ai`, `-ai-watch`, HREK tag tools | 5-byte patch of the ai_place gate call; ManagedBlam scripts to read / set flags, add squads | ai_place refuses in MP; stock Forge World had no nav mesh / squads |
| `704fc56` | AI walk (solo-built map, superseded) | built Forge World as type solo + runtime type reset | proved the nav mesh was being stripped from MP builds |
| `aaa440d` | the real fix | scenario flag "don't strip pathfinding" (bit 1) | keeps the nav mesh in a normal MP build (no lost MP tags) |
| `4a388a1` | size results, step-2 design | measured zips; `compress-most` crashed | Tyler's install-size question: +29 MB, not double |
| `19d89fb` | MAP AI host option + whole-map zones | lobby row -> `hostMapAi`; host places squads 15 s in; zones / firing positions generated from nav-mesh sectors | AI usable without dev switches; AI need somewhere to go map-wide |
| `7ade3bb` | `PlaceSquadAt`, `-map-ai-at` | move a squad's spawn points to the target, place, restore | the call a Forge-menu "place AI" will use |
| `4b0d0b5` | nav-mesh spawn heights | `NavGroundZ` reads the loaded nav-mesh vertices | authored heights dropped an AI through the map |
| `6c3998a` | `noble-map-ai-bake.ps1`, Forge piece tool | one script runs every step; pieces as crates / scenery wrappers | step 4 (other maps); step 3 research |
| `23d9539`, `65eb86c` | docs | | |

Review at 21:00: both gates build with 0 warnings in the changed files; `main` = `origin/main`; no attribution
lines; only source, tools and docs committed (no logs, dumps or RR data); userdata restored from `phase7-pre`, no
`host_*` files left (a stale `Release2\...\host_notice.txt` from the 11-20 join-in-progress test was deleted).
Docs updated: this file, plan (status block), compendium (section 9 + A.5), knowledge bank (section 9), O7 handoff
(MAP AI sign-off items), project TODO / progress copies. `tools\noble-forge-ai-zone.ps1` (old local 4x4 zone) is
superseded by `noble-ai-zones-from-navmesh.ps1`; kept for reference. `-mp-ai-pathing` is obsolete (solo-built maps).

## Status (2026-11-21 16:50) - Step 4 done (one-command bake); step 3 blocked (bake removes added objects)

**Step 4: `tools\noble-map-ai-bake.ps1 -Tag <scenario> -Centre x,y,z [-Bsp 0] [-SkipBuild] [-SkipSquads]`**
(commit 6c3998a). Backs up the scenario and built map into `HREK\noble_mods\ai_bake_<map>_<time>\` (with `bake.log`
and each tool log), clears "no pathfinding" on every BSP, sets "don't strip pathfinding", adds characters + squads
around `-Centre`, `generate-pathfinding`, zones from the nav mesh, `generate-pathfinding` again (fills the firing
positions' sectors), `build-cache-file`. Stops at the first failure. Forge World run: 6 min (72 s + 18 s + 72 s +
176 s), 17 zones / 17,318 firing positions, map 800 MB. Test p9b (MAP AI setting only): 14 AI placed, 3 spawn heights
corrected, AI walked up to 14.5 m. It also cleared the flag on Forge World's BSP 1 (stock had it set; no harm seen).
The built map stays in `HREK\maps`; copying into `data\maps` is a separate, deliberate step.

**Step 3 (Forge pieces in the nav mesh): not working yet.** `tools\noble-forge-bake-objects.ps1 -Layout <json>`
places Forge pieces (crate tags such as `levels\multi\dlc\objects\dlc_ff_bridge_2x5`) in the scenario, as crates
(`-Kind crate`) or wrapped in scenery tags `objects\noble\forge_baked\<piece>\<piece>.scenery` (default; model,
bounding radius / offset copied, pathfinding policy STATIC). Test layout `tools\forge-layouts\island_test.json`
(bridge, wall, shallow ramp near 38,32). Findings:
- The placement saves and reads back (crate palette 135 -> 138, crates 0 -> 3; scenery 19 -> 22).
- `tool generate-pathfinding` then writes the scenario back **without** them: crates, scenery, their palette
  entries and object names all return to stock (scenario size byte-identical to before). Tried crates, scenery,
  STATIC policy, editor source, negative unique ids, no object name, neutral owner team: same each time. Other
  ManagedBlam edits (flags, squads, zones) survive the bake, so the tool rebuilds the object placement blocks of
  this scenario from elsewhere (not found yet). "object refs" stays 0; nav-mesh vertices around the pieces are
  identical before / after (checked with `navmesh-near.ps1` on a saved copy).
- `tools\research\pathfinding-object-refs.ps1`: ff10_prototype has 7 object refs (ff60_airview 182, ff20 0) whose
  unique ids match none of the placed scenery / crates / machines, so stitched objects may come from another source
  (child scenarios / structure design / instanced geometry).
- Next ideas: (a) find what the tool rebuilds the object blocks from (tool log with verbose flags; Ghidra on tool.exe
  for the strip); (b) bake the pieces into the structure BSP as instanced geometry; (c) Noble's own runtime sectors
  for Forge pieces (plan option C), which also covers player-made layouts without a map build.
- HREK state: `forge_halo.scenario` = the stock object set again (pieces gone), baked; backups
  `phase7_backup_2026-11-21\forge_halo.scenario.before_step3`, `forge_halo.map.before_step3`; bake logs
  `noble_mods\phase7_genpath_step3*.log`. The three scenery wrapper tags stay in `HREK\tags\objects\noble\forge_baked\`
  (not referenced by any scenario; the script reuses them).

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

## Next (16:50; Tyler's order 1 -> 5)
1. Done: solo build dropped 403 MP tags; fixed with "don't strip pathfinding" (14:15).
2. Mostly done: MAP AI host option, whole-map zones, PlaceSquadAt, nav-mesh heights. Left: Forge-menu trigger for
   `PlaceSquadAt` (hands-on test); team rules (Troopers vs Covenant vs players).
3. Blocked: generate-pathfinding removes pieces added to Forge World's scenario (see 16:50). Ideas listed there.
4. Done: `tools\noble-map-ai-bake.ps1` (one command, tested on Forge World).
5. Shipping (needs Tyler / O7): put the AI Forge World build into the Forge World pack (`data\maps`, +29 MB zipped)
   or keep it dev-only; map hash check so guests with the old map cannot join an AI session (O7: session / network);
   guests receive AI through the normal object sync (host-only placement; test with 2 players); MAP AI stays a host
   option. No public release without Tyler.
