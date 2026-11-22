# Phase 7 plan: AI paths on custom and Forge maps (research, 2026-11-21; updated 01:45, status 21:00)

Research plan written before the work. **Status 2026-11-21 21:00:** option A (bake with HREK's tool) is built and
tested; see `docs\PHASE7-AI-PATHS-PROGRESS.md` for everything done. Changes to this plan since:
- Extra blocker found and solved: the cache builder strips the nav mesh from multiplayer scenarios unless the
  scenario flag "don't strip pathfinding" (bit 1) is set (not foreseen in section 5).
- Built: MAP AI host option, whole-map zones from the nav mesh, PlaceSquadAt, one-command bake
  (`tools\noble-map-ai-bake.ps1`).
- Step "Forge pieces as scenery, bake" is blocked: the bake removes objects added to Forge World's scenario.
  Option C (Noble's own runtime sectors for Forge pieces) moves up as the likely route for pieces and player layouts.
- Remaining: Forge-menu AI trigger, MP team rules, shipping (Tyler / O7).

## 1. How Reach's AI finds its way (what we know)

- **The nav mesh is baked, not built at runtime.** Ground AI walk on "sectors": groups of walkable collision
  surfaces stored in the level's structure BSP (pathfinding data: sectors, links between them, hints such as
  jump / climb / hoist). The editing kit makes this data when a level is compiled (Tool) or with Sapien's
  "Generate all Pathfinding Data". Our own decompile map says the same: there is no runtime nav-mesh generator in
  the engine; sectors come from the compile (`knowledge\decompile-exports\topics\TOPIC-ai-pathfinding-objectives.md`).
- **Engine pieces involved** (Xbox 360 retail names from that topic file; MCC x64 addresses still to be matched):
  `path.obj` (A*-style search: `path_state_new`, `path_state_build_path`, heap, estimated distance, hint penalties,
  `g_path_state_traverse_max_iteration_count`), `ai_sector_structures` (the sector + hint data), `path_influence_map`
  (2D danger / preference grid), `path_flying` and `path_3d_obstacle_avoidance` (flying units, no mesh needed),
  `path_obstacles` / `path_obstacle_avoidance` (moving around objects), `actor_firing_position` and
  `actor_dynamic_firing_position` (where AI stand to fight), `objectives.obj` (task tree).
- **Objects.** Static scenery can be made part of the walkable mesh at bake time; anything that moves is only an
  obstacle that AI step around (rough "pathfinding spheres"). Forge objects are placed at runtime, so to the AI they
  are obstacles: AI walk on the map's own ground and just move out of the way on top of Forge pieces. A community
  Forge World AI mod reports exactly that and works around it by shipping a modded map with extra pathfinding baked
  over large Forge World areas (a "pathfinding box") - proof that adding baked sectors to a map works.
- **Flying AI** (Banshees, Phantoms, Sentinels) already path without a mesh, so they work on Forge maps today.

## 2. What "AI paths on custom / Forge maps" needs

1. Walkable data on top of Forge pieces (and on custom maps compiled without pathfinding).
2. Links between that data and the map's own sectors, so AI can walk onto a Forge floor.
3. Somewhere to stand and something to do: firing positions and a default objective, because a Forge map has no
   AI zones / areas / tasks.

## 3. Options

| | Idea | Good | Bad |
|---|---|---|---|
| A | **Offline bake with the official kit.** Turn a Forge layout (.mvar) into a scenario where the pieces are static scenery, run the HREK pathfinding generation, ship the result as a map mod. | Uses the real tools; engine reads it as normal data. | Needs HREK + a converter per layout; every edit means a re-bake; Forge objects must exist as scenery tags. |
| B | **Runtime sector builder in Noble.** At match start, build sectors from the placed Forge objects' collision and add them to the BSP's sector data in memory. | Works for any layout, no extra files. | Needs the full in-memory layout of the pathfinding data and every place that caches it; highest risk. |
| C | **Noble bake + sidecar (recommended).** An offline Noble tool reads the .mvar and the objects' collision, builds walkable polygons (Recast-style voxelisation, MIT/zlib-licensed library), converts them into Reach sectors + links, and writes `<variant>.noblepath` next to the map variant. At match start Noble loads the sidecar and joins it to the BSP's own sector data. | Bake once per layout, fast at runtime, can be shared with the map; same engine data path as A. | Still needs the sector data layout (same research as B), but the risky code runs offline and can be checked before a match. |

Recommendation: **C**, reaching it through a small proof first (below). A stays the fallback for one-off showcase maps.

## 4. Step-by-step path

**Step 0 - tools (half a day).** Free the Ghidra project (it was locked by the open window overnight), set up x64dbg
on a Forge World match, and turn on the engine's AI debug views if the MCC build still has them (`ai_debug`,
`ai_globals` switches) to see sectors and paths on screen.

**Step 1 - find the data (1-2 days).**
- Match the 360 names to MCC x64 `haloreach.dll` by shared strings and call shape (`knowledge\decompile-exports\README.md`
  method): `path_state_new`, `path_state_build_path`, the `ai_sector_structures` functions, the BSP's pathfinding-data
  accessor, `actor_dynamic_firing_position`.
- Dump one stock MP BSP's pathfinding block (Forge World) from memory: sector count, sector record, link record,
  vertex list, how a sector maps to collision surfaces, how hints are stored.
- Output: `docs\REACH-PATHFINDING-DATA.md` with the record layouts and the MCC addresses.

**Step 2 - one-day proof (the go / no-go).**
- Dev build only: at match start, add one hand-made rectangular sector on top of a flat Forge block, linked to the
  ground sector at a ramp, and spawn a few Marines with `ai_place`-style code that walk to a point on the block.
- Pass: they walk up the ramp and stand on the block. Fail: write down which cache or check rejected it.
- If it fails for a reason we can't get around, switch to option A for showcase maps and stop.

**Step 3 - the bake tool (about a week).**
- `tools\noble-pathbake` (offline, runs on the host's PC): read the .mvar (Noble already loads map variants:
  `src\mvar\*`), get each object's collision (the object's physics / collision model from the map cache), voxelise and
  build walkable polygons with Recast-style settings sized to a Spartan / Elite (step height, slope, radius).
- Convert polygons to Reach sectors and links, add links to the base BSP sectors where they touch, write
  `<variant>.noblepath` (versioned, with a hash of the .mvar so a changed layout is detected).

**Step 4 - load at match start (2-3 days).** Noble reads the sidecar for the launched map variant, checks the hash,
joins its sectors to the BSP data in memory before AI start, and logs counts. Host and guests load the same file
(it travels with the map variant like a mod file).

**Step 5 - somewhere to stand, something to do (2-3 days).**
- Firing positions: generate them on the baked polygons (spaced grid + cover-ish spots next to walls), or lean on
  `actor_dynamic_firing_position` if it can make them on the fly.
- Default objective: one generic "fight the players" task (the Firefight survival tasks are the model) so spawned
  squads advance and fight instead of standing still.

**Step 6 - Forge workflow and polish.** A BAKE AI PATHS button in Forge (or the lobby) that runs the tool on the
current layout; on-screen view of the baked mesh; warnings for pieces AI can't reach.

**Step 7 - tests.** Forge World layouts with ramps, bridges, floating platforms; 1, 8 and 16 players; Firefight on a
Forge canvas; the 1,024-actor profile; guests see the same AI behaviour.

## 5. Risks and unknowns

- The in-memory sector data may be packed for the original BSP size, or cached by other systems (cover, hints,
  influence map); adding sectors could need more than one pointer swap.
- Links between new and stock sectors must match the engine's rules for edges, or paths will stop at the seam.
- Forge objects move if they are not locked; baked data only holds for pieces that stay put (bake only "fixed" ones).
- Performance: big layouts may make many sectors; keep the A* iteration cap in mind.
- Multiplayer: the sidecar must be identical on every PC (hash check, refuse the bake when it differs).

## 6. Research tools on Tyler's PC (found 2026-11-21)

- **`D:\MCC_DECOMP\MCC HALO REACH DECOMP\decompiled\hr_000NN.c`** - the whole MCC `haloreach.dll` decompiled to text
  (15 files, 52 MB, same build: SHA-256 738dd2d2...; `functions_index.csv` lists every function). Searching it with
  Select-String / grep is much faster than Ghidra calls, and it answered the variant questions in minutes (Firefight
  waves, social flags). Start Step 1 here: search for the 360 function shapes (A* heap, sector loops) and for the
  BSP pathfinding-data accessors, then confirm in Ghidra.
- **Ghidra (`ghidra-reach` connector)**: the Claude app may start the helper twice; the second copy locks the project
  (`NobleReach.lock`) and the other copy reports "locked". Fix used on 2026-11-21: end the copy holding the lock and the
  copy that cached the failure, so only a fresh copy runs; lasting fix: give the helper its own project copy.
- Other games' decompiles in `D:\MCC_DECOMP` (Halo 3, ODST, Halo 4) can help compare how later engines handle object
  pathfinding.

## 7. First tasks for the new session

1. Close Ghidra on the PC or give the headless tool its own copy; run Step 1 and write the data doc.
2. Do the Step 2 proof and report pass / fail before building the bake tool.

Sources: Microsoft's Halo 2 AI pathfinding and AI overview docs (sectors, pathfindable scenery / machinery),
Microsoft's Halo 3 Sapien overview ("Generate all Pathfinding Data"), Damián Isla's Halo 2 spatial-competence talk
(nav mesh for ground AI, pathfinding spheres for dynamic objects), the Forge World AI Battleground mod pages (AI on
Forge objects only move out of the way; extra baked pathfinding over Forge World), Noble's own decompile topic file.
- https://learn.microsoft.com/en-us/halo-master-chief-collection/h2/ai/aipathfinding
- https://learn.microsoft.com/en-us/halo-master-chief-collection/h2/ai/aioverview
- https://learn.microsoft.com/en-us/halo-master-chief-collection/h3/tools/sapien/overview
- https://web.eecs.umich.edu/~sugih/courses/eecs494/fall06/lectures/lecture18-halo2.pdf
- https://www.nexusmods.com/halothemasterchiefcollection/mods/2136
- https://catalogue.smods.ru/?p=189035
