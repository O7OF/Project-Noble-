# Armour effect colour research (fire + lightning) - 2026-11-15

Status: asset research done; recolour recipe identified; no tag edits shipped yet.

## Assets (HREK `D:\Steam\steamapps\common\HREK\tags`)

Effect: `objects\characters\spartans\fx\armor_fx\armor_fx.effect`
Events -> particles:
- `legendary_flame` -> `particles\head_fire.particle`
- `eternal_flame` -> `particles\head_fire_bungie.particle`
- `storm` (Inclement Weather / lightning) -> `particles\thunder_cloud.particle`, `particles\electric_crackle.particle`,
  `particles\lightning_flash.particle`, plus `objects\vehicles\covenant\revenant\fx\running\particles\plasma_bolt.particle`
- `pestilence` -> `bugs.particle`, `pestilence_gas.particle`, `pestilence_drip.particle`
- `heart_attack` -> `cupid_heart.particle`

## How the colours are defined (from the ManagedBlam tag dump, `tools\runmb.ps1` + `tools\tagdump.py`)

Particles are **palette-driven**: each particle-system block references a palette bitmap and has `palette animation`,
`color` and `self_illum_color` parameters. Examples found in the flame/storm particles:
- Fire palettes: `fx\particles\fire\gradients\fire.bitmap`, `fx\particles\fire\gradients\covenant_fire.bitmap`
- Blue/plasma palettes (storm/lightning): `fx\particles\_gradients\plasma_blue_01.bitmap`,
  `fx\particles\plasma\gradients\blue_plasma.bitmap`
- Particle textures: `fx\bitmaps\fire\fire_large.bitmap`, `fire_clouds_256.bitmap`,
  `fx\bitmaps\plasma\plasma_clouds_128_a.bitmap`, `fx\bitmaps\atmospheric\soft_smoke_small/large.bitmap`,
  `fx\bitmaps\flash\flash_soft.bitmap`
- `head_fire.particle` contains both an orange fire palette and a covenant-fire palette, plus a blue-plasma palette for
  secondary bits (smoke/glow), so recolouring the palette bitmap is enough to recolour the flame.

The full dump is `build\bin\Release\data\userdata\claude-tests-2026-11-11\armor_fx_particles.txt` (1,027 lines, private
test folder).

## Recolour recipe (no engine work needed for colour proof)

1. Duplicate the effect/particle tags (e.g. `head_fire_blue.particle`, `thunder_cloud_purple.particle`) in HREK.
2. Recolour the palette gradient bitmap(s) (hue-rotate the fire/plasma gradients; `palette animation` fields then shift
   through the new colours) and optionally tweak `color` / `self_illum_color`.
3. Point the duplicated particle at the new palette, and the duplicated `armor_fx` event at the new particles
   (`armor_fx_blue.effect`), keeping the same event names.
4. Ship the tags in a mod/map (assets are not committed) and swap the effect tag used for the Armory entry. Exposing
   *new selectable* entries still needs either the engine effect table extended (like the visor-table problem) or a
   per-effect remap patch; the colour change itself is asset-side.

## Toolchain available (verified 2026-11-15 07:55)

`D:\Steam\steamapps\common\HREK\tool.exe` has `export-bitmap-tga/dds <bitmap-tag> <prefix>`, `reimport-bitmaps-single
<bitmap>`, `patch-tag-field <tag> <field-path> <value>`, and `build-cache-file levels\multi\forge_halo\forge_halo`
(the existing `build_forge_halo.bat`). That is enough to recolour a palette gradient or patch a particle colour field,
rebuild Forge World, load it with `-map-override`, and capture the result in Noble. Baseline `maps\forge_halo.map`
already exists (796 MB, 2026-11-14).

## Next steps

- Choose the colour list Tyler wants (e.g. blue/green/purple/white flames; purple/red lightning).
- Build one recoloured variant end-to-end in HREK and render it in the Armory/match to prove the path, then decide how
  the Armory presents the new "colour-changed" entries.
- Keep the effect *names* identical so the engine's `storm`/`legendary_flame` event lookups keep working.

## Bitmap import attempt (2026-11-15 08:20) - blocked

Tried the HREK bitmap pipeline on `fx\particles\fire\gradients\fire.bitmap`:
- `tool.exe export-bitmap-tga` exports a 64x256 TGA but reports `rasterizer\invalid` / "failed to decompress invalid -
  attempting to save directly", and the exported image is a sprite sheet, not the 256x16 palette source that exists in
  `C:\Users\tyler\Desktop\ex\fx\particles\fire\gradients\fire.png`.
- Recoloured (blue/cyan) copies were placed under `HREK\data\fx\particles\fire\gradients\fire.tif` and imported with
  `tool.exe bitmaps fx\particles\fire\gradients` / `import-bitmap-folder-as-single-tag`.
  The tag file changed size, but re-exporting still showed the original bitmap and the importer reported
  "in-game size: 0 bytes" - the pixel data/import settings did not commit.
- HREK was fully restored afterwards (`fire.bitmap.bak` -> `fire.bitmap`, stray `fire_00_00.bitmap` and created
  `data\fx` removed; `forge_halo.map` untouched at 796 MB / 2026-11-14). No repo files changed.

Next options: find the correct HREK bitmap import settings (source .tif must match the tag's bitmap group/format; try
`bitmaps-debug` to dump them), or import the ex PNGs through ManagedBlam, or bypass bitmaps and patch the particle
colour/self-illum fields with `patch-tag-field`, then rebuild `build_forge_halo.bat` and capture in Forge World.

## PROOF (2026-11-15 08:35): palette-reference patch works end-to-end

The bitmap-import dead end was bypassed by patching the particle's palette **reference** instead of its pixels:

1. Backed up `objects\characters\spartans\fx\armor_fx\particles\head_fire.particle`.
2. Patched both `palette` parameters from `fx\particles\fire\gradients\fire.bitmap` to
   `fx\particles\_gradients\plasma_blue_01.bitmap` with ManagedBlam (`TagFieldReference.Path = TagPath`, then
   `TagFile.Save()`). The file-path `tool.exe patch-tag-field` could not resolve the nested path, but ManagedBlam could.
3. Rebuilt Forge World: `tool.exe build-cache-file levels\multi\forge_halo\forge_halo` (270 s, exit 0,
   -> 796,790,784-byte `HREK\maps\forge_halo.map`).
4. Equipped the Legendary flames effect with the new dev test hook
   `-armor-effect-test 5` (sets/saves profile armor effect; dev builds only).
5. Launched `-map forge_halo -mode multiplayer -map-override <built map> -armor-effect-test 5 -third-person
   -capture-at 25,33 -quit-after 45 -NoFocus`.
6. **Captures `blueflame_0.png` / `blueflame_1.png` show blue/cyan flames around the local Spartan's helmet** in
   third person (bottom-right of `blueflame_1.png`). The unmodified map shows the normal orange flames.

Test artefacts: original `head_fire.particle` restored; the patched copy is kept as
`...\particles\head_fire_blue.particle` for building the variant; `appearance.dat` restored after the run. The rebuilt
Forge World map currently has blue flames (only used via `-map-override`; not shipped).

Next: duplicate the effect (`armor_fx` event -> head_fire_blue.particle) and expose the colour variants in the Armory
(effect-table/remap work), then repeat for lightning via a purple plasma palette.

## Engine wiring for selectable effects (2026-11-15 08:55)

- `objects\characters\spartans\spartans.biped` has one attachment: `objects\characters\spartans\fx\armor_fx\armor_fx.effect`
  (attachment block around the "change colors"/effect list, entry #5, primary scale alive, secondary scale camo).
- `armor_fx.effect` has 5 events; raw StringIds read from the tag with ManagedBlam:
  `legendary_flame` 0x4D0, `eternal_flame` 0x4D2, `heart_attack` 0x4D3, `pestilence` 0x4D4, `storm` 0x4D5.
  Birthday party is a separate special case (`grunt_birthday_party.effect`).
- Profile armor-effect index (0 none, 1 birthday, 2 heart, 3 pestilence, 4 storm, 5 legendary, 6 eternal) must be converted
  to one of those event StringIds somewhere. A raw scan of haloreach.dll for a contiguous 0x4D0..0x4D5 table found none,
  so the mapping is likely a switch/table of IDs inside code or a string-id lookup. Exposing NEW colour variants needs
  that mapping extended (or reusing/remapping an existing entry), which is the remaining #15-B RE task.
  Ghidra MCP is available again for this trace.

## 2026-11-15 10:00: tag_get trace for the effect spawn - no calls

Added a temporary trace inside `SharedTagCache::HookedTagGet` (with a `-tag-trace-effe` flag) to log effect-tag lookups and
the calling engine RVA. Even an unconditional first-call log never fired during a match: the hook is installed (log says
so) but the engine does not call this tag_get function for game-time tag access after load - it uses cached tag pointers
and other loader paths. So this cannot find the armor-effect spawn/event path. Hook removed, tree clean.

Next options: find the effect spawn via the HaloScript function table (`effect_new_on_object_marker`), hook the biped
effect-attachment update (the spartans.biped attachment block), or use the compiled tag/globals to find where the effect
event StringId is selected. Alternatively accept a map-level variant (the recolour proof already works via palette tags).


## 2026-11-17 status

The palette-reference proof became the 30 primary-colour variants. 24 are working via four extra effect tags; six are
blocked by the engine's 5-effect-attachment gating limit. The chosen follow-up is a runtime particle colour override so
stock maps work without map rebuilds/downloads. See `../04-project-state/HANDOFF-ARMOR-FX-COLOURS-2026-11-17.md` and
`RE-ARMOR-FX-RUNTIME-COLOR-PLAN-2026-11-17.md`.
