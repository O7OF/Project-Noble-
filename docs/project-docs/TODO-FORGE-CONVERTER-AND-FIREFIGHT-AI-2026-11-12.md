# TO DO (ideas, not started): Forge map converter + Firefight AI revamp
(Copy of claude.ai project doc.) Added 2026-11-12 at Tyler's request. Research to-do, not a commitment.

## 1. Forge map converter (move Forge builds between maps)
Goal: take a Forge map variant built on one map and carry it to another map when the two share a similar object palette.
- Feasibility: **likely possible.** A Forge map variant (.mvar) is a header plus a list of placed objects, each pointing at
  a palette entry with a position and rotation, plus budget and spawn data. Noble can already load one at launch
  (`-mvar <path>`, launch block map variant area at +0x1D308).
- Steps: (1) parse the .mvar (verify format vs community tools); (2) map palette indexes to the target palette by tag name,
  drop/flag missing; (3) transform positions (user-picked offset or reference-point pair); (4) rewrite budget/counts, write .mvar.
- Hard parts: geometry differs (floating/buried objects -> manual offset + preview); target needs a Forge palette.
- **Firefight is the weak spot:** Firefight scenarios don't use a Forge map variant for placed objects; would need scenario
  edits or runtime spawning. (Forge on a Firefight canvas crashed at haloreach+0x10DB1B; upstream fix e119116 since.)

## 2. Firefight AI: 120+ actors and relaxed pathing
- **Actor count: largely solved.** Actor pool resized (`src/host/PoolResize.cpp`, `NOBLE_AI_ACTORS`, `-ai-actors`); swarm
  tests held 1,024 actors without a crash. 1,024 still labelled experimental (last proved 512, needs the 0x76EBF8 buffer
  fix); real-multiplayer performance/network cost untested.
- **Pathing: unknown.** Navigation is baked into structure_bsp at compile time: an authoring change, not a runtime switch.

## 3. Pathing automation tool (Tyler's idea)
- Tier A: automate Tool/Sapien: read walkable surfaces, flag failures, generate hints, re-bake (ManagedBlam + Tool).
- Tier B: own navigation generator writing structure_bsp pathfinding data (format undocumented; risky).
- Validation: Sapien pathfinding view + AI crossing test. Next: find how Reach Tool builds pathfinding; dump an existing
  map's pathfinding block via ManagedBlam.

## Suggested order
1. .mvar reader. 2. Palette mapping between two stock maps. 3. Offset/preview, then modded maps. 4. Firefight placement.
5. Pathing study (Tier A vs B). 6. Confirm 120+ AI in real multiplayer.
