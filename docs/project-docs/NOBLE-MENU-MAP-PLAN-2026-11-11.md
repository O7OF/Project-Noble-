# Noble menu map: plan and progress (2026-11-11, updated 01:05 ET)
(Copy of claude.ai project doc. NOBLE-MENU-MAP-FINDINGS-2026-11-11.md overrides this where they conflict.)

Decision (Tyler): base = Bungie's stock `levels\ui\mainmenu\mainmenu` scenario. RR's mainmenu_all.map is REFERENCE ONLY
(Tyler has RR's permission; `*.map` is gitignored anyway). Edit HREK in place after hash-checked backups (pattern:
`tools\hrek\hrek-mod.ps1`, backups under `HREK\noble_mods\<name>\`). The scenario name must stay `mainmenu` (engine asks
for mainmenu.map); rename the build output to a Noble file name afterwards.

## Goals (from Tyler)
No flicker; editable undersuits; own lighting/background/effects; no actors re-spawning or dropping weapons on each
click; no overlapping pieces; extra armour (Prophet's Hand Halo 2 Anniversary Elites + some Halo 3, more Halo 3 and
Halo 4 Spartan armours). RR's base was a fix-and-break cycle.
LATER PHASE: third character type (Marine / ODST) with armour+colour customising.

## DECISION 2026-11-11 00:34: camera approach
Script-driven views everywhere, no model-yaw memory write:
- Orbit scenes 10 (Spartan armory), 11 (Elite armory), 12 (whole body), 20 (colours): 72 cameras each, 5 degrees apart.
- Close-ups 13 head, 15 chest, 16 arm, 17 leg, 18 knee, 19 shoulder: 8 each (every 45 degrees). Plus roster camera (scene 14).
- Noble then only writes rr_turn. The yaw write at haloreach.dll+0xBE5AD8 is an RR-gap workaround and a flicker suspect.
- Script functions available (tool.exe script-doc): camera_set_relative, object_set_facing, object_teleport,
  object_set_scale. NOT available: object_rotate_by_offset.

## DONE (camera points)
- tool.exe patch-tag-field CANNOT add array elements (only edit existing). No XML import verb.
- ManagedBlam (HREK\bin\ManagedBlam.dll, official .NET tag API) works from PowerShell 5.1:
  `[Bungie.ManagedBlamSystem]::Start($Hrek,$cb)` with a ManagedBlamCrashCallback, `TagFile.Load(TagPath.FromPathAndType(
  'levels\ui\mainmenu\zz_patchtest','scnr*'))`, find Block (FieldName), `AddElement()`, fields `name` (String .Data),
  `position` / `orientation` (ArraySingle .Data, orientation is yaw,pitch,roll in DEGREES), `Save()`. PowerShell pitfall:
  never return a TagFieldBlock from a function (it is unrolled into elements); use `$script:` variable. Block max = 512.
- Generator: `tools\noble-menu-add-cameras.ps1` (dry run by default, `-Save` writes). Adds 337 points. Centres = stock
  pedestals: chief_armory (-31.8452,-23.4328,-24.69), elite_armory (-31.9089,-10.3847,-24.69), chief roster (-31.8452,-23.4328,-34.69).

## Facts verified 2026-11-11
- RR map and our HREK build: same header version 13, same build string "Jul 16 2023 16:12:13". RR = 5025 tags /
  298,520,576 bytes; HREK stock build = 4904 tags / 268,095,488 bytes.
- Engine picks the menu map in `src\vfs\VFS.cpp`: a read of `haloreach\maps\mainmenu.map` is served from
  `<userStorage>\menumaps\...` (noble_menu.map first, then mainmenu_all.map).
- MCC's Reach maps folder has NO mainmenu map: 343's stock menu is not a loose file.
- Bungie's script: data hsc `data\levels\ui\mainmenu\scripts\mainmenu.hsc` has no armory logic.
- RR decompiled script copy in repo: `docs\RR-mainmenu-decompiled.txt` (do not commit).
- Noble finds the GLOBALS table by markers rr_mark_a=62615533 (0x03BB6FED) / rr_mark_b=62568019 (0x03BAB653) at slots 0/1,
  then 16 string_id pairs at slots 4..35, then rr_scene(0x24) ... rr_card_done(0x33); 52 slots. Declaration ORDER and
  TYPES must match (ReachArmory.cpp RrGlobalSlot enum).
- RR scene visibility: 0-9 lobby paths; 10/12/13/15-20 only chief_armory; 11 only elite_armory; 14 only chief (roster).

## Design hazards seen in RR's script (avoid in ours)
1. `rr_armory` applies ALL 16 pairs every time; unused pairs default to head:default.
2. `object_create_anew` of chief + chief_armory on armour-effect change = idle restart / weapon throw.
3. `rr_fx` targets chief + chief_armory only (no Elite effect).
4. `rr_scenes` (every tick) re-runs object_hide on all 9 characters, etc.

## Build command
`cd HREK; .\tool.exe build-cache-file levels\ui\mainmenu\mainmenu pc english`; output HREK\maps\mainmenu.map.
Then copy to build\bin\Release\data\menumaps\noble_menu.map and run tools\menu_sidcsv.ps1.
