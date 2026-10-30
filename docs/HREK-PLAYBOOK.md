# HREK playbook — what the editing kit lets us actually build

Everything below was verified by exporting real tags from `D:\Steam\steamapps\common\HREK` (see
`docs/HREK-FINDINGS.md` for the extraction and `tool.exe` requirements). It is the "so what can we do with it" list.

## Armory: the room is lit by *baked lightmaps*, and the Spartan is lit by scripted lights

`data\levels\ui\mainmenu\scripts\mainmenu.hsc` (783 lines) is the menu's HaloScript. The relevant parts:

```
L138-147  (pvs_set_object chief_armory)  (pvs_set_object elite_armory) ...
L148-157  (cinematic_light_object chief_armory target_main appearance_spartan spartan_armory_light_anchor)
          (cinematic_light_object elite_armory target_main appearance_elite elite_armory_light_anchor)
L162      (custom_animation_loop chief_armory objects\characters\spartans\spartans "depot:unarmed:idle" FALSE)
```

So each appearance location (main menu, armory, exit, pgcr, screenshot) has its own Spartan object and its own
**cinematic light** with an anchor object. The Spartan is lit dynamically by that light, which is exactly why the model
reads correctly while the chamber around it is black.

The exported lighting tag (`tools/emblem/mainmenu_lighting.xml`) confirms the other half:

```
generic light definitions = 0
generic light instances   = 0
regions = 0
material info = 1        <- per-material emissive override
instance imposter infos = 49
```

Zero generic lights means **static geometry is lit only by baked lightmaps**. Noble's `data\menumaps\mainmenu_all.map`
(284 MB) is our own build; if it was built without baked lightmaps, the room renders black while the scripted cinematic
lights still light the Spartan. That is the armory "void" explained end to end.

Three ways to fix it, cheapest first:

1. **Scripted lights (cheap, no bake).** Add extra `cinematic_light_object` calls in `mainmenu.hsc` for the room (attach a
   light to a room object or place additional anchors). Rebuild the scenario's script data + the map; no lightmap farm run.
   Only works if Reach's cinematic lights contribute to structure shading — worth one experiment.
2. **Emissive materials (cheap, no bake).** `material info` in the lighting tag has `emissive power`, `emissive color`,
   `emissive quality`, `emissive focus`, `attenuation falloff` per material. Making the armory chamber's materials
   slightly emissive self-lights the room. Tag edit + map rebuild.
3. **Bake lightmaps (correct, expensive).** `tool lightmap-farm <scenario> <bsp> <light-group> <quality> ...` then
   `tool build-cache-file-generate-main-menu-layout`. This is what MCC's own menu map has; it is the full-fidelity fix.

Also useful: `instance imposter infos = 49` are the menu's backdrop imposters (`campaign_act2_midground01`, ...) — the same
mechanism could give the armory a real painted backdrop instead of black geometry.

## Emblems: a real 53 x 80 editor is in the data

Exported to `tools/emblem/emblem_library.xml`:

```
bitmaps = 119   shapes = 160   front emblems = 80   back emblems = 53
bitmap resolution = 64, antialias sharpen = 1.6
atlas = tags\ui\cui\common\emblems\emblems.bitmap
```

- **53 backgrounds**: blank, circle, diamond, plus, square, triangle, vertical_stripe, horiztonal_stripe, cleft,
  criss_cross, 8_point_star, star, cowboy_hat, thick_star, banner, 4_diamonds, sun, hexagon, vertical_hexagon, chalice, ...
- **80 foregrounds**: septagon, bullseye, vortex, halt, spartan, power_icon, spartan_league, delta, music_note, stuck,
  phoenix, champion, jolly_roger, active_rooster, campfire, radioactive, smiley, frowney, no_camping, sol, ...
- Each emblem is authored as layers of shapes with transforms, e.g. `jolly_roger -> shape skull, multiplier 1,
  scale 0.75,0.75`; `flaming_ninja -> head_background, scale 0.7,0.7`.

That is the picker Tyler asked for: two menus (background / foreground) over authentic Reach art, composited from the
119-sprite atlas.

## Variants and modes: the engine has a real sandbox (Forge) game engine

`tags\multiplayer\game_engine_settings.game_engine_settings_definition` (exported, 201 KB) contains a **`sandbox variants`**
block whose entry is `variant_name_sandbox_editor` with its own miscellaneous options (e.g. `teams enabled`). So the
engine ships a Forge/sandbox *game* variant, not just a map editor path.

Related material for host-controlled modes:

- `tags\multiplayer\game_variant_settings\...` — every variant setting/trait the UI can expose (export one to XML to see
  the field names; this is how a Noble-side custom-mode screen could be populated with real settings).
- `data\multiplayer\megalo\*.txt` — the Megalo source for every shipped variant (slayer, ctf, koth, juggernaut, invasion,
  assault, `cinema_test`, ...) plus `MegaloEdit.exe`, so custom variants can be authored and built without hex editing.

## Third person: no trait exists, so it is a mode or a camera patch

Checked and confirmed: `game_engine_settings` has **no camera/third-person field**, and "third" appears only in unrelated
tags ("third party"). Reach has no third-person player trait, so a variant cannot switch it on.

The two real routes:

1. **Mode-based (matches "a specific game mode").** The **sandbox/Forge engine** is third-person by construction: the
   editor camera is a third-person camera. HREK ships the sandbox variant definition (`variant_name_sandbox_editor`), so
   launching engine tag `0x04` with that variant is the authentic path, and everyone in that mode gets the third-person
   editor camera. (Noble currently launches Forge as Megalo tag `0x03`; the earlier `0x04` attempt stalled at launch
   state 2, but that was before we knew the sandbox variant existed — worth retrying with the variant data from HREK.)
2. **Camera patch (any mode, client-side).** The first-person camera is computed per frame from the player's head; pushing
   that camera back/up gives third person. This is a rendering change, so it is inherently **client-side**: a host toggle
   can gate whether the client applies it, but it cannot force guests to. Would need a Ghidra pass over the player view /
   camera setup in `haloreach.dll` (we already have the analysed project and the tooling).

Recommendation: try route 1 first (authentic, mode-gated, no RE), keep route 2 as the "any mode" option.

## Other kit assets worth pulling

- `tags\ui\` (1,318 tags) — nameplates, emblem screens, CUI layouts: real reference for the menu design doc.
- `tags\objects\` (17,172) — armour permutations, lights, effects: source for a proper armory light object or backdrop.
- `tags\globals\globals.globals` — engine globals (weapons, damage, camera defaults) exportable to XML.
- `data\levels\...\scripts\*.hsc` — every level's HaloScript (62 files), including `forge_halo`; the forge scripts are the
  reference for how sandbox mode arranges its camera and editing objects.


---

## Atlas export status (2026-10-31)

`tool export-bitmap-dds <abs path>\tags\ui\cui\common\emblems\emblems.bitmap <prefix>` runs but reports
`rasterizer\invalid / ERROR failed to decompress [0x80070057] / attempting to save directly`, and the only file produced is
`emblemsinvalid_00_00.dds` (64 KB). Pillow rejects it (`Unimplemented DXGI format 91`), and `export-bitmap-tga` fails to
decompress the same data. So the shipped `emblems.bitmap` does not carry pixels the kit can decode - the atlas has to come
from somewhere else:

1. MCC's own emblem images, which Noble already loads (`UiArt::EmblemByName`), composited per the HREK XML transforms; or
2. the emblem atlases in the MCC packs (`E_002_P_SML` family), already reachable through `UiArt`; or
3. a different export verb on the tag (`export-bitmap-pfm` untried) once the bitmap group resolves.

The emblem *data* (53 backs, 80 fronts, 160 shapes, transforms, atlas indices) is already captured in
`tools/emblem/emblem_library.xml`; only the sprite sheet is missing.
