# Draft — Forge palette and the pack pipeline

Input chapter for `PROJECT-NOBLE-COMPENDIUM.md`. Confidence tags as in the compendium: **proven** (one-variable test
+ capture), **observed** (seen live, not isolated), **community** (community guides), **unverified** (code/docs, not
runtime-checked). Sources named per claim.

---

## 1. Two palettes (do not confuse them)

| Palette | Where | Edited by | Served by |
|---|---|---|---|
| Noble's own Forge menu | `data\forge_palette.json` | hand/script | `src\ui\ForgeMenu.cpp` (ImGui overlay, F3/INSERT) |
| Native in-map Forge palette | the scenario's **map variant palettes** block | ManagedBlam `scn_*.py` + HREK `tool.exe build-cache-file` | the VFS, from `data\maps\forge_halo.map` |

**Noble's own menu** is a JSON tab list with 13 tabs and **no airstrike entries**:
`SPAWNS, GOALS, WEAPONS, VEHICLES, EQUIPMENT, SCENERY, BUILDING BLOCKS, GADGETS, LIGHTS, NATURAL, UNSC INFANTRY,
COVENANT INFANTRY, SPECIAL & BOSSES`. Each item names a tag (`objects/…`) plus `max`/`price`; it places any tag
present in the loaded map. `ForgeMenu.cpp` loads the first `forge_palette.json` it finds (data dir, then exe dir),
falls back to a built-in VEHICLES list, and refuses to open outside Forge mode. Source: `data\forge_palette.json`,
`src\ui\ForgeMenu.cpp`. observed.

**Native palette** lives in the scenario tag's `map variant palettes` block: one palette per category
(`ff_weapons_human`, `ff_weapons_covenant`, `ff_vehicles`, plus MCC's hidden `ff_thorage_*` / `ff_cex_*` sets).
Each palette is `{name, entries[]}`; an entry is `{name, maximum allowed, price per instance, variants[]}` and a
variant is `{display name, object tag path, variant name}`. This is the palette Reach's real Forge menu reads.
Source: `scn_mvp.py` / `scn_palettes.py` dumps, `HANDOFF-CONTENT-2026-11-09.md`. observed.

## 2. The pipeline

1. **Edit the scenario** with ManagedBlam (Blender bridge, `runmb.ps1`): `scn_add_forge.py` (2026-11-13: `boathog`
   in `ff_vehicles`, `target_laser` in `ff_weapons_human`), `scn_add_covenant.py` (2026-11-14: airstrike, §3),
   `scn_palettes.py`/`scn_mvp.py` (read-only dumps). Idempotent: duplicate an existing entry
   (`DuplicateElement`), never `AddElement`.
2. **Rebuild** with HREK: `build_map.ps1 -Scenario levels\multi\forge_halo\forge_halo` →
   `tool.exe build-cache-file <scenario> pc english`. ~2–3 min.
3. **Install** the `.map` into `data\maps\forge_halo.map` (+ any `fmod` bank like `sfx.st_spectre.fsb`).
4. **Serve** via Noble's VFS (log `VFS: serving Noble's '…map' in place of MCC's`), proven by
   `forge_test.ps1 -NoOverride`.

Lesson (pelican.md): a vehicle that is only in a map variant palette builds/spawns/drives but is **invisible**
(render resources not streamed); it must also be in the scenario's **vehicle palette**. Always append palette
entries (map variants store palette indices), never insert. Source: `build_map.ps1`, `pelican.md`. observed.

## 3. 2026-11-14 Covenant airstrike (proven)

`scn_add_covenant.py` added entry **`target_laser_covenant`** to `ff_weapons_covenant` (copied from
`wep_plasmalauncher`; max 8, price 150) with 7 variants, all pointing at `custom\weapons\target_laser` variants:

| Variant | Object path | Type |
|---|---|---|
| `target_laser_corvette` | `…\variants\Corvette\corvette_bombing` | Corvette bombardment |
| `target_laser_seraph` | `…\variants\Seraph\seraph_run` | Seraph run |
| `target_laser_spirit_ghost` | `…\variants\Spirit\Ghost` | Spirit dropship (Ghost) |
| `target_laser_spirit_wraith` | `…\variants\Spirit\Wraith` | Spirit (Wraith) |
| `target_laser_glassing` | `…\variants\Glassing_Single\Glassing_Single` | glassing beam |
| `target_laser_halo_array` | `…\variants\Halo_Array\Halo_array` | Halo Array |
| `target_laser_game_ender` | `…\variants\Game_ender\Game_ender` | game ender |

Rebuilt map = **796,790,784 bytes**, installed in `data\maps`, and **confirmed working in Forge by Tyler**
(blocker closed). Before this the Covenant weapons palette had no `target_laser` entry at all. Source:
`scn_add_covenant.py`, `00-START-HERE.md` "Current state", AGENT-COORDINATION 11-41. proven.

## 4. ElitePack build and shrink

The Elite pack is an **extra cache** loaded alongside the map, injected by `ElitePack.cpp`: the map-load request's
extra-cache list is rewritten to add the pack (`haloreach+0xA38A3 → 0x141C0` loader, extra loader 0x31078, header
slot 0xAA634, per-cache step 0x30698), and a relocation sidecar (`.rrreloc`, magic `RRRL`) rebases the pack's packed
tag pointers after load. The pack then swaps the player representation's biped and appends Noble's Elite pieces
(see `draft-model-animation.md` §3). Source: `ElitePack.cpp` header comments. observed.

**Shrink (486 → 284 MB, done 2026-11-14, A/B proven).** The pack is now built from a minimal scenario
`levels\test\noble_pack\noble_pack` — a copy of `levels\test\box` with type set to multiplayer and the scenery /
vehicle / weapon / character palettes, squads, scripts, hs globals/source/syntax and thespian cleared
(`scn_packcopy.py`, `scn_blocks.py`). The first try (palettes not cleared) carried civilian + marine bipeds, which
made ElitePack's shield heuristic bail ("shields too close") and players kept the stock Elite. Compression was not the
route (deflate shrinks the old pack only 16%). Rebuild recipe: `build_map.ps1 -Scenario levels\test\noble_pack\noble_pack`;
copy `HREK\maps\noble_pack.map → data\armor\noble_elite_pack.map`; `runmb.ps1 -Script tagwalk.py -Env "MODE=build;A64=8;REC=1;PACK=<pack>"`.
Old pack kept as `…map.before-small` (+ `.rrreloc.before-small`). Source: night handoff 01:40 section. observed/proven.

## 5. Pelican / Spectre / Boathog tags

All three live in HREK only (never MCC), installed/restored by `tools\hrek\hrek-mod.ps1` with SHA-256 manifests and
backups under `HREK\noble_mods\<mod>\`.

| Item | Author / licence | Scenario edits |
|---|---|---|
| Pelican (flyable) | Pepper's Pelican Overhaul (PepperMan, Nexus 2475); credit PepperMan + Bungie/Crisp/Krevil/Arttumiro/J-S/Zed | vehicle palette 11 `pelican_gear_scripted_doors`, 12 `pelican_door_scripted_doors`; `ff_vehicles` entry 9 `pelican` (2 variants, max 4, price 100). Replaces 3 `*.model_animation_graph` + 3 shaders |
| Spectre (H2A T-46) | Spartan Tanner (Nexus 2423); credit + Halo Archive/Model Resource; not for sale | 101 new files; vehicle palette 13; `ff_vehicles` entry 10 `spectre` (5 variants default/classic/classic_needler/antiair/armored, max 16, price 100); sound bank `sfx.st_spectre.fsb` via VFS |
| Boathog | Spartan Tanner (GCUR) | `ff_vehicles` entry `boathog`, 4 variants default/gauss/rocket/troop (`stannerreach\…\boathog\boathog`) |

Pelican door/ramp control is Noble code: **X** toggles scripted object function 3 (all of `object_function_set`),
global across all scripted Pelicans; online sync untested. Source: `pelican.md`, `HANDOFF-CONTENT-2026-11-09.md`,
`scn_add_forge.py`. observed.

## 6. Option 2 — add-on pack vs the 750 MB Forge World (plan, not built)

Option 1 (current): a rebuilt Forge World shipped as `data\maps\forge_halo.map` (796 MB) and served by the VFS on
every normal Forge World launch — proven. Tyler wants **Option 2** next: a small add-on pack injected into *any*
map's Forge palette the way ElitePack injects a cache, so the airstrike/vehicles come from a pack instead of a
750 MB rebuilt map. No design or code exists yet beyond the ElitePack injection route being reusable; treat as
unverified. Source: `HANDOFF-NEXT-AGENT-2026-11-14.md`, `00-START-HERE.md` "Current state". unverified (plan).

---

## Open questions

- Which string list names the native Forge palette entries (pelican/spectre/etc.) — the stock Forge menu shows
  blank names; not found yet.
- Pelican seat prompts / door sync online (the `X` function is global and unsynced).
- Whether Option 2 can reuse ElitePack's extra-cache + relocation path unchanged for palette (not biped) injection.
- Map hash check for rebuilt maps before any online play (needed, not yet built).
- Spectre/Pelican `data\forge_palette.json` entries are not yet added to Noble's own menu.
