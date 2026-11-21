# Phase 7 progress: AI paths on custom and Forge maps

Written for the next agent. Newest first inside each section. Plan: `docs\PLAN-PHASE7-AI-PATHS-2026-11-21.md`.

## Status (2026-11-21 12:00)

**Proven on Forge World (HREK-built map, dev switches):** 14 AI (Elites, Grunts, Jackals, Brutes, Troopers) spawn,
stand on baked ground without falling through, see the player and attack (an Elite sword-killed the player, run p7d).
**Not yet:** they hardly move (0-2.5 m in 60 s). Squads have no zone / areas / firing positions, so the actors pick
nowhere to go. Next step: give them firing positions and a default objective (plan step 5).

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

## Next
1. Movement: zones / areas / firing positions for the test squads (or dynamic firing positions), and a default
   objective ("fight the players"), then re-test that they walk to and chase the player on the ground.
2. Forge pieces: place a small layout's objects as scenery in a scenario copy, bake, confirm AI walk onto them.
3. Team rules in MP (Troopers vs Covenant vs players), the 3 deaths in p7d.
4. Shipping: map size (+50 MB), map hash for online play, and whether `-mp-ai` becomes a host option.
