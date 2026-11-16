# Handoff: Noble Armory and front-end menu (2026-11-11, 07:25 ET)

(Copy of claude.ai project doc. Historical: later sessions fixed several items - see ..\01-PROJECT-HISTORY.md.)

For the next agent picking up Tyler's Noble work (Halo Reach MCC custom client, repo `D:\Project-Noble-Closed-Source` on
Tyler's PC, reached through the device bridge). Read this first, then `claude/NOBLE-ARMORY-STATUS-2026-11-11.md`
(older status, mostly folded in below) and `HANDOFF.md` (long project history).

## 0. Working rules with Tyler
- **Nothing is committed.** All of the work below is uncommitted on Tyler's disk. Commit/push only when he asks
  (merge origin/main into wip-merge before pushing). Never commit RR material (`*.map` is gitignored).
- Tell Tyler before big changes. He builds and tests himself in **PowerShell**: `.\build-dev.bat` from
  `D:\Project-Noble-Closed-Source` (not cmd syntax; he sometimes types `\build-dev.bat` without the dot, which fails).
  Log: `D:\Project-Noble-Closed-Source\build\bin\Release\noble-dev.log` (stage it with the bridge after each test).
- Bridge pattern that works: stage the file, edit a copy in a **fresh** `/mnt/user-data/outputs/<new dir>`, commit with
  `expectedMtimeMs` from the last stage, then re-stage and compare md5. (Writing and committing in parallel shipped stale
  files twice.) Keep CRLF line endings.
- MSVC gotchas hit tonight: `near`/`far` are Windows macros (don't name variables that); `std::format` is used (not
  fmt); functions with `__try` can't hold objects with destructors (log outside the SEH function).
- Tyler sends screen recordings; pull frames with ffmpeg into a contact sheet and read them. Look at the end of the video
  too.

## 1. Done this session (uncommitted, built and tested unless marked)
- Menu music: Reach soundtrack only (`src/audio/ReachAudio.cpp`, `kOtherGamesMusic = false`; CE/H3/ODST can be re-enabled).
- Layered Bungie menu art per screen (Firefight, Custom Games (+Servers), Campaign (+Skulls)), colour-matched to
  Tyler's references and placed in the gaps between panels: `assets\ui\noble\screens\<name>\layers.json` + PNGs,
  `UiArt` ScreenLayers/DrawLayered (parallax, drift, flicker). Main menu: slideshow of the stock pictures (20 s, 1.5 s fade).
- Armory backdrop: procedural Auntie Dot lattice (pixel shader in `FrontEndMenu.cpp` kBackdropPs, 36 s loop).
  **Dimmed ~12% at 07:05 (core 0.81, glow 0.70) - built, Tyler hasn't commented yet.**
- LOADING... (white on a dark box) until the character is presentable; character masked from the lattice with the
  engine depth buffer (ClearDepthStencilView hook, CPU 32x18 read-back, biggest depth gap = threshold).
- Stale GLOBALS table rejected after 20 s (no more blind fallback that showed the ship scene).
- Species switch: picture held only for Spartan/Elite switches; placement debounced 4 frames (fixed the "jump").
  Effect/armour rebuilds: no hold (Tyler accepts an occasional one-frame white flash over stutter). Tyler confirmed the
  stutter/flicker fix.
- Nameplate click crash: fallback draw path no longer touches the D3D context. **Crash fixed; untested by Tyler.**
- Bigger, brighter scrollbars (`NobleStyle.cpp` + per-frame size in FrontEndMenu). Built.
- Firefight Voice preview: clickable, shows the X key on mouse/keyboard, pad X on controller. Built.
- Emblem colour preview whites match. Picker grids no longer flip between picture and text layouts.
- **Elite Armor Effect picker now shows the same pictures as the Spartan one** (`ArmorCustomizer.cpp`, Elite effect row
  copies `pic/picDir` from `FindSpartanItem("armor effect", id)`). Built; Tyler hasn't confirmed visually.

## 2. Open bug being worked: Elite colours never change (highest priority)
Symptom: in the Armory the Elite stays stock Minor teal/blue whatever primary/secondary is picked (videos
07:03 and 07:08). Spartan colours work.

What the logs proved (diagnostics added to `src/engine/ReachArmory.cpp`, all marked "Diagnostic (2026-11-11)"):
1. **Old path (colour "records" scan) finds nothing.** `ScanEliteColorRecordsIn` looks for 16-byte runs equal to one of
   11 stock Elite rank palettes (`MatchEliteColorPattern`, from host-probe scan_elite @ 0x1407B6658) or the last colours
   written. Full scans of ~5.5 GB match nothing, even on the Minor set; loosening the match (+-10 per channel) didn't
   help, and the near-miss dump only shows texture noise. In earlier sessions today (05:30-06:27) it found 4-5 records,
   so something about the Elite's data changed. Tyler says another member was changing Elite "lighting" (shaders?) -
   ask Tyler which files; that's the prime suspect.
2. **Live per-object colours ARE written and stick.** `SyncArmoryLiveBipedEngineTickSEH` finds the Elite bipeds
   (definition index 0x7E, change-colour block 96 bytes = 4 slots), writes base+active slot colours and calls the engine's
   SetChangeColor (haloreach+0x473858). Log line `Elite live sync: seen N rewrote K ... want (...)` shows base/active ==
   wanted, rewrote 0 after the first tick (the engine does not put its own colours back). 4-8 Elite bipeds are seen.
3. So the menu Elite's look does **not** come from the object change colours. Most likely it comes from shader/tag data
   (per-rank Elite shader constants) or the rank palette record that the old scan used to find.

Next steps (suggested):
- `UpdateTagShaderConstantsSEH` only has Spartan branches (spartan_armor, rubber suit, visor); `words` is unused. Walk
  the 'rmsh' tags the same way and dump float4 constants of Elite shaders (look for values near Minor's palette:
  0xFF1F3E72 -> ~(0.12,0.24,0.45), 0xFF5167A9 etc.) to find where the Elite tint lives; then add an Elite branch.
- Ask Tyler what the "lighting" change touched (which tags / which map build). Compare against the 05:30 build where
  records were found.
- The `ex` folder (below) has the Elite `_cc` masks: Minor chest/shoulder use only the R channel (primary); body
  (`elite_base_upper_cc`) uses R+G (secondary only on the body). So even when fixed, secondary will only show on the body.
- When done, remove the diagnostics: near-miss dump + "full Elite colour scan matched nothing" / "full scan skipped"
  lines, the `s_diag*` arrays and the `Elite live sync` log. The tolerant `closeTo` match can stay or go.

## 3. Armory to-do list (not started unless noted)
1. Elite colours (section 2).
2. Emblem on the in-game armour is wrong/missing: Noble's ApplyArmoryAppearance writes no emblem fields; find the
   Reach profile emblem offsets (use `-capture-profile` before/after captures, see HANDOFF.md). Emblem layer art is in
   `ex\ui\cui\common\emblems[0..95].png`.
3. Nameplate click: engine stops drawing the menu (crash already fixed); find why the engine stalls.
4. Elite armour pieces don't change (stock Elite render model has no elite_* regions; needs RR-style regions). Log
   also warns `'h2a_space_helmet' isn't in the menu map's string_ids` for the H2A Ranger set.
5. Close-up cameras don't frame the piece (chest/wrist/utility/knees show the wrong side).
6. Knife stuck on the chest whatever the chest piece (`flair_chest_b` "default" is the chest knife).
7. Knee guards: male default looks small with a gap; default should probably show none; FJ/Para = default.
8. Grenadier helmet looks off on the female (probably male/female swapped). Tyler: low priority, "not an issue".
9. Re-check helmets/attachments that didn't show; confirm armour effects use the production script.
10. Clean-up: test hooks (`noble-test-skip/head/delay.txt`), "script state" log, unused `HoldStageBriefly`,
    section-2 diagnostics, `zz_patchtest.scenario` + `HREK\reports\zz_patchtest`, HREK\temp test files.
11. Commit when Tyler asks.

## 4. Ideas discussed but not checked yet
- **Third character: Marine / ODST** with armour + colour customising. Menu side looks feasible (extra biped + scenes +
  script pairs in the menu map; keep the layout extensible). Game side is the big job: spawning that model, multiplayer
  sync, and Noble code assumes rr_elite 0/1. Asset: smudude07's "ODST playable marine" (Nexus 1865, permission granted,
  credit smudude07, not for sale; not downloaded yet). `ex\objects\characters\marine\bitmaps` has Marine textures.
- Community request (TRAXUSi): more player-host object types beyond Reach's three (spartan, elite, monitor) for
  Workshop class mods.
- More armour: Prophet's Hand Halo 2 Anniversary Elites, some Halo 3 Elites, Halo 3 and Halo 4 Spartan armours (as in an
  earlier build).
- Picture thumbnails per piece rendered once and cached (`data\ui\cache\armory\`); Elite pieces still have none.
- Stance poses; rank/cR locks from the career list.
- Re-enable the other soundtracks (CE/H2/H3/ODST) later (one flag).
- Undersuit colouring / own lighting in the Noble menu map (see `claude/NOBLE-MENU-MAP-PLAN-2026-11-11.md`).
- Use Auntie Dot idle animation frames (`ex\ui\cui\common\auntie_dot_idles`) or `img_auntiedot_background.png` in the
  Armory/loading screens.

## 5. Resource: `C:\Users\tyler\Desktop\ex` (connected 06:47)
About 1,700 files of extracted Reach tags (mostly PNG bitmaps) plus `elite.obj/.mtl` and `spartans.obj/.mtl`. Useful:
- `objects\characters\elite\bitmaps\elite_{base,chest,gauntlet,helmet,shoulder,thigh,illum_cards}` - per-rank diffuse,
  `_cc` change-colour masks, normals.
- `objects\characters\spartans\bitmaps\...` (helmets 182 files, chest, shoulders, etc.), `objects\ca_dev\dlc_armorsets`.
- `ui\cui\common\emblems[0..95]`, `commendations` (144), `skulls_lg_ui`, `auntie_dot_idles`, `lobbies` (105),
  `levels\ui\mainmenu\bitmaps` (53 menu-map bitmaps).

## 6. Files touched (versions at that time)
- `src/engine/ReachArmory.cpp` and `.hpp`
- `src/ui/FrontEndMenu.cpp`, `src/ui/ArmorCustomizer.cpp`, `src/ui/UiArt.cpp/.hpp`,
  `src/ui/NobleStyle.cpp`, `src/audio/ReachAudio.cpp`, `src/engine/ReachHost.cpp` (back to original)
- `assets\ui\noble\screens\*`, `tools\noble-menu-list-refs.ps1`, `tools\noble-export-menu-art.ps1`
