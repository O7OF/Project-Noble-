# Pelican + Spectre in Forge World (2026-11-09)

Status: **working in local Forge** (Tyler, 09:56): Pelican visible, flies, shoots, seats work; Spectre drives.
Parked behind multiplayer (Tyler's priority order: multiplayer -> multiplayer optimization / bugs -> Pelican,
swarm, ReachVariantTool).

## Credits and permission
- **Pepper's Pelican Overhaul (Reach)** by **PepperMan** (Nexus MCC mod 2475, v1.0.2). Page: "Anyone has permission
  to use the tags for their own mods, but please credit me!" Tyler confirmed permission. Also credit Bungie
  (original model), Crisp (Foundry add-on), Krevil, Arttumiro, J-S, Zed. Not for paid use.
- **H2A T-46 Spectre** by Spartan Tanner (Nexus 2423), permission granted; credit Spartan Tanner (+ Halo Archive /
  Halo Model Resource servers). Not for sale.

## What is installed (HREK only, never MCC)
`tools\hrek\hrek-mod.ps1 -Mod pelican|spectre [-Action install|status|restore]`
- Pelican: 132 files; replaced stock (backed up, SHA256 manifest in `HREK\noble_mods\pelican`):
  marine / spartans / spartans_ai `.model_animation_graph`, `trainingpreserve\...\ca_preserve_pelican_hull.shader`,
  `pelican\shaders\pelican_hull(_damaged).shader`. `forge_halo.scenario` is protected (backed up) under pelican.
- Spectre: 101 new files (`tags\stannerreach\...`, `fmod\pc\sfx.st_spectre.fsb(.info)`).
- `-Action restore` puts stock files back hash-checked; edited mod files are moved to `noble_mods\<mod>\edited`.
  Restoring pelican also restores the stock forge_halo.scenario (removes both mods' palette entries).
- Build: `HREK\build_forge_halo.bat` -> `HREK\maps\forge_halo.map` (log `build_forge_halo_log.txt`, ~2-3 min).
- Test: `run_pelican_test.bat` (noble-dev.exe, `-map forge_halo -mode forge -map-override HREK\maps\forge_halo.map`,
  log `pelican-forge.log`).

## forge_halo.scenario edits (all appended, never inserted: map variants store palette indices)
| Block | Entry | Content |
|---|---|---|
| vehicle palette | 11 | `objects\vehicles\human\pelican_new\pelican_gear_scripted_doors.vehicle` |
| vehicle palette | 12 | `...\pelican_door_scripted_doors.vehicle` |
| vehicle palette | 13 | `stannerreach\objects\vehicles\covenant\spectre_variants\spectre_variants.vehicle` |
| map variant palettes `3. ff_vehicles` | 9 `pelican` | variants `pelican` -> pelican_gear_scripted_doors, `pelican_door` -> pelican_door_scripted_doors; max 4, price 100 |
| map variant palettes `3. ff_vehicles` | 10 `spectre` | 5 variants of spectre_variants.vehicle, variant names default / classic / classic_needler / antiair / armored; max 16 (since 09:58), price 100 |

Lesson: a vehicle only in the map variant palette builds into the map, spawns and drives, but is **invisible**
(render resources not streamed). It must also be in the scenario's **vehicle palette**.

## Pelican tags (from the mod page)
- 4 vehicles: `pelican_{gear,door}_{auto,scripted_doors}`. `_auto`: ramp/gear open only when the pilot gets out.
  `_scripted_doors`: driven by scripted object function 3 (`object_function_set 3 1` open / `3 0` close).
- Model variants: default, default_hook_hog, default_hook_scorp, no_interior, damaged, no_interior_cine,
  default_nophys; grey: oni, oni_hook_hog, oni_hook_scorp, oni_no_interior, damaged_oni_no_interior(_cine),
  oni_nophys. Do not use filler variants.
- Cannon: co-pilot by default. Spartan seats have "spartan" in the name. 1000 vitality.
- Hook: warthog seat `pelican_lc`, scorpion seat `pelican_scorpion`; load with `vehicle_load_magic`, drop with
  `vehicle_unload`; scorpion hook permutation `object_set_permutation <pelican> "hook" "scorpion"` /
  `"scorpion_unloaded"` on drop.
- The page says it syncs in multiplayer (within the synced attachment limit).

## Noble code
- `src/engine/PelicanControls.cpp/.hpp` (2026-11-09 10:12, dev build only, called from SpawnTest's game tick):
  **X** in any vehicle toggles scripted object function 3: `(*(float**)(root + 0x3C0))[3]` = 0 / 1. That is all
  `object_function_set` does (evaluator RVA 0x19AAB4, clamps to [0, 1]). Global: all scripted Pelicans move
  together. Online sync untested. Logs `Pelican: X, ramp + landing gear OPEN/CLOSED`.
- Built: not yet (as of 10:20). Tested: not yet.

## Reverse-engineering notes (haloreach.dll 1.3528.0.0)
- HS function table: entry = name ptr +0x00, parser +0x10 (0x180198F2C generic), evaluator +0x18.
- `object_function_set` evaluator 0x19AAB4: index (0..3), value clamped 0..1 -> `(*(float**)(root+0x3C0))[index]`.
- `vehicle_unload` evaluator 0x19F494 -> **RVA 0x4F9F38(vehicle handle, seat string_id)**.
- `vehicle_load_magic` evaluator 0x19F43C -> **RVA 0x4F9CF8(vehicle, seat string_id, object_list)**. Per unit:
  `0x2367E4(vehicle, seat_name, 2, buf)` fills seat candidates (16-byte entries: +0 object, +4 seat index), returns
  count; `0x484E48(unit, seat_obj, seat_idx)` = can enter; if unit has a parent (`obj+0x14 != -1`)
  `0x49F210(unit, 0x15)`; then a 0x44-byte unit action {+0 = 0x10 enter vehicle, +4 = vehicle, +8 = 7,
  +0xA = seat index, +0xC = 0} submitted with `0x49F24C(unit, &action)`.
- Object data: `+0x0` tag index, `+0x14` parent object, `+0x44` position, `+0x50` forward; record type byte at
  record+4 (0 biped, 1 vehicle). Local unit: `0x53EF8(0)`.

## Next steps (in order)
1. Seat prompts: back up `HREK\tags\ui\hud\hud_messages.multilingual_unicode_string_list` (copy into
   `noble_mods\pelican`), then `tool.exe strings ui\hud` (compiles the mod's `data\ui\hud\hud_messages.txt`), rebuild map.
2. Build noble-dev, rebuild forge_halo, test X (log line; ramp + gear animate; players can exit with ramp open).
3. Forge menu names (blank): find the string list that names stock entries ("banshee") and add pelican /
   pelican_door / spectre*.
4. Hook pick-up/drop: add hook variants (variant names `default_hook_hog`, `default_hook_scorp`) to Forge entry 9;
   Noble key: pilot over a warthog/scorpion -> load into `pelican_lc` / `pelican_scorpion`; key again -> unload.
   Blocker: runtime string_id for the seat names (map-specific). Options: parse the map file's string table (Noble
   already parses map headers in SharedTagCache) or read the seat labels from the vehicle tag and match by text.
5. Per-Pelican doors instead of the global function (would need a per-object function or tag change).
6. Add both vehicles to Noble's own Forge menu `data\forge_palette.json`; Spectre sound bank via VFS at runtime;
   map hash check before any online play on a rebuilt map.
