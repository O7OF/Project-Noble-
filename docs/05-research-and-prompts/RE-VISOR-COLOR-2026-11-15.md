# Visor colour engine RE notes - 2026-11-15

Status: static RE progress only; no runtime patch applied. This is the "decompile" work Tyler asked to prepare while
the exact custom-colour feature waits.

## What the engine already has

The per-player model render path has a **shader-parameter palette** with an entry literally called `visor_tint`:

- `FUN_18024334c(int slot)` maps slot indices to shader parameter ids:
  - 0..5 -> `0x8015d`..`0x8015f`, `0xc1`, `0x8018d`, `0x8018e` (armour colour slots)
  - **6 -> `0x8018f` = `visor_tint`**
  - 7..12 -> `0x80190`, `0x80192`, `0x80193`, `0x8061b`, `0x8061c`, `0x80194`
- `FUN_18032596c(model)` initialises a model's parameter list with these ids (`FUN_180325a38(model, id, slot)`).
- The name table in `.rdata` (RVA `0x9c2b80`..) is `{uint64 id, char* name}`; the `visor_tint` entry is
  `id 0x8018f`, name at `0x18099e2a8`.
- Other relevant strings (string pool around `0x1809b7200`): `prop_visor_color_count`, `prop_visor_color_locked`,
  `prop_visor_color_index`, `prop_visor_color_rgb`, `prop_visor_color_new`, `prop_player_model_visor_color`,
  `prop_initial_visor_color_index`, `prop_visor_color_locked`, `prop_initial_visor_color_index`.
  So the MCC UI property system exposes a visor colour **index** and an **RGB** property, i.e. the engine-side
  representation has both. That is the strongest hint that exact custom RGB is representable, not just the five stock
  visor items.

## How the model renders a player's colours

- `FUN_180324f90(model, ..., appearanceRecord)`: reads a per-appearance record from the global appearance table
  `DAT_182a03c58 + index * 0x2ae` (same table that earlier notes used for `DAT_182a03c50` / per-player records),
  copies 8 dwords of palette colours from `record[0x197..0x19e]`, transforms them via `FUN_18032510c`, and feeds
  `FUN_1802c8480`.
- `FUN_1803246b8(model, ..., ...)`: uses the record's `+0x20c`, `+0x270`, `+0x274`, `+0x278` fields (colour indices)
  and calls `FUN_18024334c` for the shader-parameter slot, then `FUN_1803247d4(...)` to push primary/secondary/
  tertiary/visor colours into the render model.
- The relevant palette colours live in the same global appearance table the game uses for armour colour; visor slot
  colours appear in its `+0x197..` dword range (this is the piece to confirm at runtime with a write/read test).

## Two possible implementations (unchanged from the earlier handoff, now with concrete addresses)

1. **Global palette write (simplest, affects everyone with that finish).** Patch the five visor palette entries in the
   global appearance table at runtime so the stock visor slots render as custom RGB. This would prove the shader path
   accepts arbitrary colours, but all players using that finish would see the same custom colour. Not enough for
   "each player keeps their own" unless combined with per-record writes.
2. **Per-player appearance record / shader param (correct, harder).** Find where a player's appearance record is
   applied to their biped render model (the caller chain of `FUN_1803246b8` / `FUN_180324f90`) and set the visor slot's
   colour from the local profile's visor RGB (or write the RGB into that player's appearance record). If the property
   system can be given `prop_visor_color_rgb` per profile, this is the clean path; if not, hook the model-build path and
   override the `visor_tint` parameter for the local biped only.

## Suggested next tests

- Write a diagnostic that logs `FUN_1803246b8` arguments for the local player model in a match (record pointer, colour
  indices, the `FUN_18024334c` slot) - confirms which slot is visor and which record fields carry the visor index.
- Temporarily force the visor colour dword in `FUN_180324f90`'s `record[0x197..]` copy to a known magenta and capture the
  in-game biped; if the visor turns magenta, hypothesis 1/2 are validated and we can scope the per-player override.
- Ask Claude for the context creation around the profile's visor RGB property (the handoff has the MCC decompile
  callers) so we can decide whether the clean per-profile path exists or the hook is required.

## Useful Ghidra addresses (image base 0x180000000; validated against haloreach-copy.dll sha256 738dd2d2...)

- `FUN_18024334c` - slot -> shader parameter id (`visor_tint` = 6 -> 0x8018f).
- `FUN_18032596c` - initialise a model's shader parameter table.
- `FUN_180325a38` - append one shader parameter entry.
- `FUN_1803246b8`, `FUN_180324f90`, `FUN_18032510c`, `FUN_1802c8480` - player model colour application path.
- `.rdata` name table entries for visor strings around RVA `0x9c2bc8` (`visor_tint`) and string pool `0x9b7200`.
- Existing known sites: visor item translator `0x3b2f4`, profile application `0x2eda08`, patch site `0x2edc5e`.

## Test 2026-11-15 08:40: forcing FUN_1802c8480 slot 6 - negative

Temporary hook on `FUN_1802c8480(model, colors[8], flag)` forced `model+0x674` (slot 6, i.e. the visor index in
`FUN_18024334c`) to magenta after the original call, then captured the Armory with `-menu-screen armory
-armory-focus visor`. Log showed one call with `slots6 before=00000000` and the rest zero except slot7 FFFFFFFF; the
Armory visor stayed orange (blood orange). So this colour setter is **not** the path that renders the visible Armory
visor. Hook removed; gates rebuilt clean; no repo changes.

Next ideas: patch the spartan visor shader/material tag used by the map via ManagedBlam (same approach that made the
blue-fire palette proof work) to validate the shader tint path map-wide; or hook the actual shader-parameter upload /
render-model draw path rather than the appearance colour setter.

## 2026-11-15 09:25: visor shader tags + front-view capture attempt

- Dumped the actual visor shader tags. `objects\characters\spartans\shaders\spartan_helmets\spartan_visor_default.shader`
  and `spartan_helmet_interior_visor.shader` both reference `spartans\shaders\templates\spartan_visor_template.shader`
  and expose tint parameters we can patch with the same ManagedBlam approach that recoloured the flames:
  `env_tint_color` (argb), `self_illum_color`, `specular_tint`, `fresnel_color`, plus `self_illum_intensity`.
  Patching e.g. `env_tint_color` on `spartan_visor_default.shader`, rebuilding Forge World and loading with
  `-map-override` should give a map-wide visor tint - a useful pipeline proof for the eventual per-player override.
- To capture the visor, added a temporary `-third-person-front` test camera (mirror third-person pos through
  `unit_get_head_position`); the capture (`front_cam_test.png`) still looked first-person/forward - the head query likely
  returned invalid or the engine camera wrote after the hook. Reverted; no code committed. Need a different capture angle
  (fix the front camera using the engine camera marker, or use the Armory for visual checks if its menu map can be
  rebuilt).

## 2026-11-15 09:40: front-view capture now works (`b1680d1`)

`-third-person-front` (test/dev only) now mirrors the third-person observer position through the focus point after the
engine camera update and flips the forward vector, so the local player model can be captured from the front:
`-map 20_sword_slayer -mode multiplayer -third-person -third-person-front` -> **`front_cam_final.png`** (Spartan visible
from the front, window unfocused). This unblocks visual checks for visor/armour colour work.

Also inspected the visor shader parameter layout: `spartan_visor_default.shader` has `env_tint_color` at
`render_method.parameters[3]`, `self_illum_color` at [8], `fresnel_color` at [11]. Their value lives in the parameter's
`animated parameters[0].function` struct (custom function editor), not a simple color element, so patching the tag
directly needs that struct's data (or a different shader parameter) - next attempt.

## 2026-11-15 09:50: visor tint value is a custom-function blob
`env_tint_color` on `spartan_visor_default.shader` lives at `render_method.parameters[3]`; the parameter type is
`5:argb color` but its value is stored in `animated parameters[0].function.data`, a `TagFieldCustomFunctionEditor`
binary blob (same for `self_illum_color` [8] and `fresnel_color` [11]). ManagedBlam can read/save the tag but setting
that blob requires encoding the function editor format. Alternative paths: patch the render-method shader constants
(`rmsh`) at runtime (the Armory's existing material tint does this), or force the visor tint at the model-shader-parameter
level once the render-model upload hook is found.

## 2026-11-15 09:20: visor shader tags CAN be recoloured, but matches do not use them

ManagedBlam exposes the animation-function editor value: `TagFieldCustomFunctionEditor.Value` is a
`TagValueCustomFunctionEditor` with `SetPossibleColorCount(TagFile,int)`, `SetColor(int, Bungie.Game.GameColor)`,
`FromArgb/FromRgb`, etc. Using it I set `env_tint_color` = magenta on `spartan_visor_default.shader`, then on **all 35**
`*visor*.shader` tags under `objects\characters\spartans\shaders\spartan_helmets` (each file backed up, map rebuilt with
tool.exe, exit 0) and captured the local Spartan from the front with `-third-person-front -armor-effect-test 0`
(`visor_tint_crop.png`).

**Result: the visor stayed white/gold - no magenta.** The Armory's own tint code explains why: it scans the *menu map's*
`rmsh` tags and writes floats f4[6]/f4[7]/f4[17]; in campaign/match maps that scan finds **0 rmsh tags**, so match spartan
armour/visor materials do not live in the map (or in HREK tags) - they come from MCC's shared tag heap/`shared.map`.
Editing HREK shader source tags therefore cannot tint match visors. All 35 tags restored from backup; no repo changes.

This also confirms the ASIO-style static approach is wrong for matches: the remaining #14 path is a **runtime shared-tag-heap
scan** (find the engine's shared rmsh tags at runtime and patch the same f4 slots the Armory does - but map-wide for every
Spartan) or the engine visor-colour table/lookup extension for true per-player colours.

## 2026-11-15 09:20: blind shared-heap float scan - rejected as unsafe / ineffective

Next tried a temporary in-process scan for the Armory's visor parameter signature (f4[1][0]~=3.0f, f4[8][0]~=1.0f)
across all committed readable memory, patching f4[6]/[7]/[17] to magenta at match start (`-visor-heap-test`). It found
64-200 "candidates", but they were false positives from arbitrary float arrays (several at +4-byte offsets of the same
block; existing values like (0,1,1,1)), and the front-view capture stayed white/gold - the real rmsh blocks were not
identified. Because patching arbitrary process memory is unsafe, the probe was removed and the tree rebuilt clean.

Conclusion: the remaining #14 path needs the **actual shared tag table / rmsh tag structure** (find the engine's shared
tag-cache global and iterate tag entries like the Armory does for the menu map), not signature scanning. Then patch only
validated rmsh parameter blocks. No repo changes kept.

## 2026-11-15 09:25: tag-table scan attempt + reversed group-magic discovery

Tried a safer probe: find real tag tables by scanning for a group-table magic and validating the tag-table header at
+0x3FFF8, then iterate `rmsh` tag entries and patch only visor parameter blocks (f4Count 21, f4[1][0]~=3.0f,
f4[8][0]~=1.0f). It found no valid tables: scanning for the raw 'scnr' bytes only matched two string occurrences.

Important detail discovered: Halo stores tag-group magics **byte-reversed** in memory. The Armory constant 0x726D7368
comments "rmsh" but its bytes are "hsmr"; HREK's group list shows reversed names (`rncs` for scnr, `hsmr` for rmsh).
So the scan must look for 0x73636E72 ("rncs") for scnr and 0x726D7368 ("hsmr") for rmsh.

Next concrete step (no blind scanning): read the active tag table pointer from `[haloreach+0xC1A600]` at match start, dump
its group-table entries to confirm the layout/offset, then look for other tag-table pointers in the engine globals (shared
cache tables) and iterate their `rmsh` tags. The probe was removed again; both gates rebuilt clean, no repo changes.

## 2026-11-15 09:30: active tag-table diagnostic

Read `[haloreach+0xC1A600]` at match start:
- active table pointer valid, **tag count 15985** at +0x3FFF8 (so that offset is right);
- but the group-table dump at +0x3FFFC did **not** show reversed tag-group magics (first entries looked like non-magic data),
  so the group-table offset/layout in the match tag table differs from what the Armory code assumes (or the pointer is
  not the header start of the structure we think).
- A module-pointer scan looking for other tag tables produced many false positives (4-entry validation too weak); no
  trustworthy shared-cache table candidate.
- Probe removed; both gates rebuilt clean; no repo changes.

Next: dump the raw active table header/group area and cross-check with Ghidra's tag-table code, then find the shared-cache
table/global. This is genuinely larger engine RE than the queue fixes; exact per-player custom visor remains the only
unfinished main-queue item besides the multiplayer hold.

## 2026-11-15 09:32: match-map rmsh data is stripped/not resident

Using the verified tag-table layout (from FUN_18002f56c: tag entries +0, count +0x3FFF8, group table +0x3FFFC, group
count +0x41000) I iterated the active Forge World tag table at runtime:
- active table tags=15985 groups=235, rmsh group index 163, **723 rmsh tag entries**;
- but **every** rmsh tag failed the sp-data validation (spCount/pointer unpack), i.e. their tag data is not resident in the
  map table (stripped/streamed or resolved from the shared cache), so there are no map-local rmsh parameter blocks to tint.
  This matches the Armory scan finding 0 matching visor materials in match maps.

Conclusion: exact custom visor colours cannot be done by patching map tags at runtime. Options are now:
1. **Map-wide shared-heap tint**: locate the engine's shared-tag-cache table/global (not found yet) and patch its resident
   rmsh parameters when loaded; affects all Spartans (not per-player).
2. **Per-player engine visor table/lookup extension**: the correct fix; requires decompile/scoping of the appearance record
   and engine visor-colour table - a dedicated RE project, not a quick fix.
Current shipped behaviour (nearest valid stock slot; nothing white) stays until one of these is chosen.

## 2026-11-15 09:36: shared tag-table scan - found a candidate, scan too slow to complete

Next attempt: find other tag tables by locating the reversed `rmsh` group magic (0x726D7368) anywhere in process memory,
derive the candidate group-table/table address for every possible group index, and validate the header
(count +0x3FFF8 1..0x8000, group count +0x41000 1..256, matching group slot). This found a real-looking candidate table
(`tags=1073, groups=255, rmshGroup=184`) - likely a shared/extra cache table - but the full-memory scan is far too slow
and blocks the render thread for tens of seconds, so it did not finish/process before the match quit. Running it as a
one-shot synchronous scan is not viable; if this path is pursued, it needs a background thread and/or a direct pointer to
the shared cache table (not a memory sweep). Probe removed again; both gates clean.

Exact per-player visor remains the only unfinished main-queue feature besides the multiplayer hold.
