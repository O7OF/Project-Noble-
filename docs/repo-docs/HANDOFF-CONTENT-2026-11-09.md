# Handoff: community content, Forge maps, Noble tooling (2026-11-09, updated 08:10 ET)

For the next agent. Read this first, then `LIMITS-HANDOFF.md` (32-player work) and `docs/swarm-test-notes.md` (flood).

## 0. Rules from Tyler (still in force)

- Never modify the installed `haloreach.dll`. Never edit `D:\Project-Noble-ORIGINAL`.
- Never run `build.bat`; use `build-dev.bat`. Tyler builds and tests; ask before any test run.
- Never push without Tyler's OK. Commit only when he asks (he usually runs the git commands himself).
- Never commit `recipes/`, `.nlp` files, or game assets (maps, rips, bitmaps, sound banks).
- 32-player work: everything behind `-slot-players 32`, byte-checked and all-or-nothing. Commit trailers:
  `docs/CLAUDE-CODE-START-C12.md`. Do NOT write a static scanner that lists imul sites from the disassembly
  (stopped by safeguards three times): find untraced sites at runtime (`-trace-members` + guard).
- Large files: keep CRLF line endings when editing (`src/` is CRLF). After writing a file to Tyler's PC, re-read it
  and compare: one write on 2026-11-09 reported success but left the old contents (see section 2).

## 1. State of the work streams

| Stream | State | Where |
|---|---|---|
| C12-L 32 players | Installs and runs clean (solo + 3 windows). Next blocker: #32 membership update message (5-bit counts/indices, payload size, frame growth). Team-change trace run still to do. | `LIMITS-HANDOFF.md`, `docs/limits-players-loops.md` |
| Megapack custom games + Forge maps | Code done and built; Forge maps tab works on screen. Search-box overlap fixed (needs rebuild). Launch of a Forge map not tested yet. | section 2, `docs/megapack.md` |
| Swarm (flood) zombies | Parked. Waves spawn swarm zombies, bodies textured, jumping works with `-horde-no-teleport`. | `docs/swarm-test-notes.md`, section 4 |
| Join failure (Tyler can't join a friend's game) | Diagnostic log lines added to `src/main.cpp` (`[GUEST] master server join ...`), uncommitted. Waiting for Tyler to retry and send the log. Suspect: host behind CGNAT, relay not used. | `src/main.cpp` |
| Spectre + Pelican in Forge World | Spectre permission OK (Pelican: confirm author). Installed into HREK 08:30 (section 3). ODST marine and Audio Overhaul removed from the list. | section 3 |

## 2. Megapack: custom game types and Forge maps (done 2026-11-09)

Source: Halo Customs MCC Megapack (ModDB "halo-mcc-reach-megapack", Dec 2019), on Tyler's PC at
`C:\Users\tyler\Desktop\Reach game modes and maps` (`game_variants\` 282 .bin, `map_variants\` 200 .mvar,
`INSTRUCTIONS.txt`). **Permission: the pack group allowed Noble to ship it.** Credits: Rai (packer), Insanity
(writer), Sarconik / iTz Truly Ninja (revisions), Halo Customs Discord; plus each file's creator (written by the import
script to `data\credits\megapack.txt`). Microsoft Game Content Usage Rules apply: never sold.

Files added/changed (not committed unless Tyler did it after 08:00):
- `tools\megapack\import-megapack.ps1`: sorts and copies the pack. Skips MCC stock (`*_054.bin`, `hr_*`,
  `00_basic_editing*`, `*_cl_031.mvar`, `hr_forgeWorld_*`) and files the pack marks broken (`(NEED MAP)`, `(Broken)`,
  `(Might be broken)`, `(Questionable)`, `(Try)`, `(NEED Gametype)`). Sets aside **38 Xbox 360 exports**
  (`_cmp` zlib body or big-endian chdr text; 4 KB-padded sizes) in `build\megapack\convert`. Copies the rest to repo
  `data\game_variants` / `data\map_variants` and `build\bin\Release\data\...`. Manifest: `build\megapack\manifest.csv`.
  Run: `powershell -ExecutionPolicy Bypass -File tools\megapack\import-megapack.ps1` (Tyler ran it; 181 Forge maps listed).
- `src/ui/FrontEndMenu.cpp/.hpp`: Forge maps.
  - `ForgeMap`, `ScenarioForMapId`, `ReadForgeMap`, `ScanForgeMaps` (after `FindVariant`): scans `data\map_variants` and
    `LocalLow\MCC\{Temporary,LocalVault}\UserContent\HaloReach\Map`; only PC-format files (>= 0x7329 bytes, `_blf`,
    `chdr` @0x30, `mvar` @0x2F0, LE title) with a known base map id (@0x6C). Header: creator 0x88, title 0xC0, description
    0x1C0. Group from file prefix: BTB→Big Team, INF→Infection, INV→Invasion, MINI→Minigames, Race, SLYR→Slayer,
    OBST→Obstacle Courses, MISC→Other, Hot Pursuit, Cops & Robbers, else "Unsorted".
  - `DrawSourceTabs(..., forgeCount, tabsEnd)`: third tab FORGE MAPS (n) in Custom Game and Forge (not Firefight);
    Q/E step through tabs. Search box now placed after the last tab (`tabsEnd`).
  - `MapEntry.mvar/forgeGroup`, `ForgeMapEntries`, `ToggleForgeGroup` (open keys `forge:<group>`), `st.mpForgeMap`,
    `st.forgeForgeMap`, banner kicker "FORGE MAP · <base map>".
  - Launch: `FrontEndChoice.mapVariant` → relaunch arg `-mvar "<path>"` (BlamEngine already parses it into the launch
    block @0x1D308 and takes the map id from it). Lobby: `st.lobbyMapVariant` (cleared in `SetupHostLobby`, set after it
    for a Forge map; playlists via `ApplyEntryToLobby`; lobby MAP cycling keeps the Forge map as a choice).
- `src/network/MasterServerClient.cpp/.hpp`: `LaunchLobbySync(..., mapVariant)` sends `launch_data.map_variant`
  (file name); `LobbyState.launchMapVariant`; guests resolve the name in `ctx.forgeMaps`.
  **Check the master server (separate repo, `tools\server-repo.ps1`) passes `map_variant` through**, else guests load
  plain Forge World.
- `build-package.ps1`: the release copy dropped every `*.bin`; now copies repo `data\game_variants` and
  `data\map_variants` (`*.bin *.mvar`, /PURGE) into the package. Uses the repo folders, not Release's (that one also
  holds testers' own saved variants).
- `docs/megapack.md`: details.

**Write glitch:** the first write of the search-box fix (about 07:55 ET) reported success but the file kept the old contents;
Tyler's build at that time had no fix. Re-written and verified byte-identical (device mtime 1794229283311, 425306 bytes).

Next for this stream:
1. Tyler rebuilds; check Search sits after the tabs and the "Unsorted" heading.
2. Test: Custom Game → FORGE MAPS → Infection → Lighthouse, game type Infection, PLAY LOCAL. Log must show
   `Custom map variant parsed successfully. Target MapId: 3006`.
3. Re-save the 38 Xbox files in ReachVariantTool (Save As), rerun the import with `-Source <folder>`.
4. Commit: `git add src/ui/FrontEndMenu.* src/network/MasterServerClient.* build-package.ps1 tools/megapack docs/megapack.md docs/HANDOFF-CONTENT-2026-11-09.md HANDOFF.md data/game_variants data/map_variants data/credits`
5. Later: guests' lobby shows the base map name (no advert field for the Forge map title); pair maps with their game
   type ("INF 30 Days and Nights" + "30 Days and Nights.bin"); group game types by the pack's themes; map hash check
   before online play.

## 3. Assets and tools on Tyler's PC (connected folders)

| What | Path | Status / licence | Plan |
|---|---|---|---|
| ReachVariantTool v2.3.2 (portable, DavidJCobb) | `C:\Users\tyler\Desktop\reach varient tool and editor\ReachVariantTool.exe` (+ `vc_redist.x64.exe`) | Program GPLv3 (since Sep 2021), docs CC0. Tyler 08:21: Noble will be open source; only the dev build is closed while O7 updates the public build. Still: no RVT code in the closed dev build, or in any build handed to testers, until that build ships GPLv3 source; and in-process code that hooks MCC may need DavidJCobb's OK (ask). Safe now: run RVT as a separate program, or write our own code from the CC0 docs. | Make/fix variants: swarm-only Firefight variant, re-save the 38 Xbox files, Noble's default playlists. |
| ReachVariantEditor source | same folder, `ReachVariantEditor-master\` | GPLv3 source | Reference only. |
| VS Code RVT extension (Unfunkable; faris0064 fork) | same folder, `VSCode-ReachVariantTool-main\` | GPLv3, v0.5.0, syntax highlighting/autocomplete for `.rvt` scripts | Only if we write Megalo scripts. |
| Megapack | `C:\Users\tyler\Desktop\Reach game modes and maps` | Permission granted | Done, section 2. |
| H2A T-46 Spectre (Spartan Tanner, Nexus 2423) | `C:\Users\tyler\Desktop\spectre` | **Permission granted.** Credit Spartan Tanner (+ Halo Archive / Halo Model Resource servers for the H2A rips). GCUR: not for sale. | Forge World palette. Install: `tags\stannerreach\...` → `HREK\tags\stannerreach\`, `fmod\pc\sfx.st_spectre.fsb(.info)` → `HREK\fmod\pc\` and Noble must serve the bank at runtime (VFS). Use `spectre_variants` (default/classic/classic_needler/antiair/armored). AI tags: link on the Nexus page. Also `.blend` sources in `spectreblendreach\`. |
| Flyable Pelican (author unknown, no readme) | `C:\Users\tyler\Desktop\Pelican` | **Permission not confirmed in writing: ask Tyler who made it and whether we have the OK (needed for credits).** | Forge World palette. **Overwrites stock HREK files**: `objects\characters\marine\marine.model_animation_graph`, `spartans\spartans.model_animation_graph`, `spartans_ai\spartans_ai.model_animation_graph` (~170 MB each), `data\ui\hud\hud_messages.txt`, `levels\dlc\trainingpreserve\shaders\ca_preserve_pelican_hull.shader`: back them up (hash-checked) before copying. Vehicles in `objects\vehicles\human\pelican_new\`: `pelican_{door,gear}_{auto,scripted_doors}.vehicle`; `gear_auto` is the likely Forge pick (confirm in Foundation). |
| Flood combat sounds | `C:\Users\tyler\Desktop\The Flood\flood_gm2\floodcombat` (233 WAVs) | Halo 2 rips | For the swarm audio swap (section 4). |

### Spectre + Pelican into Forge World: plan
**08:30: step 1 DONE.** `tools\hrek\hrek-mod.ps1 -Mod pelican|spectre [-Action status|restore]` installed both into HREK (Pelican 132 files, 6 stock replaced: 3 anim graphs, ca_preserve_pelican_hull, pelican\shaders\pelican_hull(_damaged); Spectre 101 new files). Backups + SHA256 manifest in `HREK\noble_mods\<mod>\`; forge_halo.scenario protected under pelican. Pelican = **Pepper's Pelican Overhaul (Reach) by PepperMan** (Nexus MCC 2475, v1.0.2): 'anyone has permission to use the tags, please credit me'; Tyler confirms permission. Credits: PepperMan; Bungie, Crisp (Foundry add-on), Krevil, Arttumiro, J-S, Zed. Added `HREK\build_forge_halo.bat` and repo `run_pelican_test.bat` (-map forge_halo -mode forge -map-override HREK\maps\forge_halo.map). Noble also has its own Forge menu, `data\forge_palette.json` (spawns any tag present in the loaded map; already lists the stock pelican\pelican): add the new Pelican/Spectre there after the build works. Next: Tyler in Foundation (step 2).

**09:56: WORKING in local Forge (Tyler):** Pelican flies, shoots, seats work; Spectre drives. forge_halo.scenario edits: map variant palettes `3. ff_vehicles` entries **9 pelican** (variants pelican -> pelican_gear_auto, pelican_door -> pelican_door_auto; max 4) and **10 spectre** (5 variants of spectre_variants.vehicle: default/classic/classic_needler/antiair/armored; max 16 since 09:58, needs rebuild). Invisible-model fix: the vehicles must ALSO be in the scenario **vehicle palette** (entries 11 pelican_gear_auto, 12 pelican_door_auto, 13 spectre_variants, appended after 10 shade); palette-only tags load but their render resources are not streamed. Always append (Add), never Insert: map variants reference palette indices.
Open items:
- Forge menu names blank: entry names (pelican, spectre, ...) have no text in the string list the Forge menu uses; find which multilingual_unicode_string_list holds "banshee" etc. and add ours.
- No seat prompts: the mod's `data\ui\hud\hud_messages.txt` (installed in HREK) adds pelican_enter_pilot/copilot/l01-l07/r01-r07 but was never compiled into `tags\ui\hud\hud_messages.multilingual_unicode_string_list` (still 2010). Find the tool.exe command for it (Tyler: run tool.exe with no args to list commands), compile, rebuild. Protect that tag in hrek-mod first.
- Ramp + landing gear (Tyler 09:57): the *_auto vehicles open ramp/drop gear only when the pilot gets out. Wanted: pilot key (Tyler suggested X; check conflicts) to toggle ramp and gear in flight. The *_scripted_doors vehicles drive the doors from a scripted object function (`scripted_object_function_d`) instead of unit_open: switch the palette to those and have Noble set that function (engine object_set_function_variable equivalent) on a key press; gear: find which function drives it. Must sync online (host sets it). Noble code, dev-tested first.
- Hook (Warthog/Scorpion under the Pelican): no pick-up/drop in stock Reach; possible Noble feature (objects_attach/detach on a key). Later.
- Add Pelican/Spectre to Noble's own Forge menu `data\forge_palette.json` (tags objects/vehicles/human/pelican_new/pelican_gear_auto, .../pelican_door_auto, stannerreach/objects/vehicles/covenant/spectre_variants/spectre_variants). Stock forge_halo also has hidden `ff_thorage` palette with a flyable stock Pelican and Phantom.
- Spectre engine sound bank (sfx.st_spectre.fsb) must be served by Noble VFS at runtime.
- Online: map hash check before any rebuilt-map play.
**10:12 ramp/gear button (dev build, needs build-dev + map rebuild):** `src/engine/PelicanControls.cpp/.hpp` (new, in CMake for noble + dedicated), called from SpawnTest Hook_Tick. X while in any vehicle toggles scripted object function 3 = (*(float**)(root+0x3C0))[3] between 0/1 (that is all object_function_set does: evaluator RVA 0x19AAB4, clamp 0..1). Global: all scripted Pelicans move together; online sync untested. forge_halo.scenario now uses pelican_gear_scripted_doors / pelican_door_scripted_doors in both the vehicle palette (11, 12) and ff_vehicles entry 9.
**Hook research (not coded):** HS function table entries: name ptr +0, parser +0x10, evaluator +0x18. vehicle_load_magic evaluator 0x19F43C -> RVA 0x4F9CF8(vehicle, seat string_id, object_list): per unit finds seats via 0x2367E4(vehicle, seat_name, 2, buf), checks 0x484E48(unit, seat_obj, seat_idx), detaches (0x49F210(unit,0x15)) if parented, then submits a unit action (type 0x10, +4 vehicle, +8 = 7, +0xA seat) via 0x49F24C(unit, action). vehicle_unload evaluator 0x19F494 -> RVA 0x4F9F38(vehicle, seat string_id). Seats: "pelican_lc" (warthog, hook variant default_hook_hog), "pelican_scorpion" (+ object_set_permutation hook scorpion / scorpion_unloaded). Blocker: runtime string_id for those names (map-specific); options: parse the map file string table, or read seat labels from the vehicle tag.
1. Swap script like `tools\swarm\swarm-swap-build.ps1`: back up the stock HREK files the Pelican overwrites
   (hash-checked), copy both tag sets and the Spectre sound bank, restore on demand.
2. Foundation: each vehicle needs a multiplayer object block (Forge category, cost/budget, spawn settings) and a
   Forge menu name (string list); add them to Forge World's (`levels\multi\forge_halo`) Forge vehicle palette.
3. Build `forge_halo` with Tool into a separate output (never over the MCC map); Noble serves it via
   `-map-override` / a mod folder; the Spectre bank via VFS.
4. Tyler tests in local Forge: place, enter, drive/fly.
5. Before online: **map hash check** in the lobby (refuse mismatched content with a clear message). Needed for any
   rebuilt map; build it once properly.
Needs: Tyler connects the HREK folder (`D:\Steam\steamapps\common\HREK`).

## 4. Swarm (flood) zombies: pending

- Tendril tips white: `flood_fronds.shader` is a Halo 3 shader + missing `flood_fronds_{diffuse,zbump,illum}` bitmaps
  (from Renette). Stopgap option: copy the fixed body shader over it.
- Hunters and the map's own vehicle waves still spawn: make a **swarm-only Firefight variant** in ReachVariantTool from
  `hr_ff_standard_1set.bin`: every squad slot Grunts/Jackals (keep Squad Count at defaults 3/5/3 or empty slots become
  Brutes), Uses Dropship off, Bonus Wave off; save `swarm_ff.bin`. Then wire the swarm test .bat to use it (check Noble
  can take a game variant from the command line: `-variant` exists in `main.cpp`).
- Audio: replace grunt voices with flood combat sounds; first find where the swarm character's voices come from
  (its voices block is empty; biped `swarm_civilian.biped` or parent `ai\generic.character`).
- Uncommitted: `tools\swarm\swarm-swap-build.ps1`, `run_swarm_test.bat`, `run_swarm_test_fulllod.bat`,
  `docs\swarm-test-notes.md`.

## 5. Mods reviewed for Noble (2026-11-09)

| Mod | Verdict |
|---|---|
| ReachVariantTool (Nexus 192 / GitHub DavidJCobb) | Use as a tool. |
| Megapack (ModDB) | Shipping (section 2). |
| Spectre (2423) | Forge World; permission granted. |
| Pelican | Forge World; author and permission to confirm with Tyler. |
| Remember the Fall of Reach (2869) | Don't ship (mixed others' assets, no permission). Players can install it themselves; the Modded tab lists it. |
| Flood Firefight (350, Tuco Studios) | Ignore (broken since 2020, permission needed). |
| Freecam (901) | Not needed; Noble can do a free camera itself. |
| Other Nexus links from Tyler's first batch of 11 | Notes lost in a context trim: ask Tyler to paste them again if needed. |

## 6. Uncommitted on Tyler's PC (as far as known)

- Megapack: `src/ui/FrontEndMenu.cpp/.hpp`, `src/network/MasterServerClient.cpp/.hpp`, `build-package.ps1`,
  `tools/megapack/import-megapack.ps1`, `docs/megapack.md`, `data/game_variants`, `data/map_variants`, `data/credits`,
  this file.
- Join diagnostics: `src/main.cpp`.
- Swarm: files in section 4.
Run `git status` before committing and commit each stream separately.

## 7. Tyler's requests at 08:07 ET (NOT started; no files changed for these)

### 7a. Engine limits: everyone on max, rows hidden for players
Tyler: "this should be DEFAULT and hidden except from devs for testing. All players should have max settings so they
are all on equal builds / engine edit." So players always run **ENGINE LIMITS = Full (V16 + particles + light
volumes)** and **AI ACTORS = 1024**; only `noble-dev.exe` (`NOBLE_DEV_BUILD`) shows the two rows and can change them.
Where (line numbers as of 08:08):
- `src/common/Settings.hpp` ~99-103: defaults `extendedLimits = 0`, `aiActors = 0` → `3` (`limits::Profile::Full`) and
  `1024`; update the comments.
- `src/common/Settings.cpp` ~72-77 (sanitize): after the clamps, `#ifndef NOBLE_DEV_BUILD` force
  `d.extendedLimits = static_cast<int>(limits::Profile::Full); d.aiActors = 1024;` so old settings files (0/0) and
  hand edits can't lower it.
- `src/ui/SettingsPanel.cpp` ~1252-1288: wrap the whole `Section("ENGINE")` block (both rows) in `#ifdef NOBLE_DEV_BUILD`.
- `src/main.cpp` ~710-716 (step 8): in release, always `SetEnvironmentVariableW(L"NOBLE_AI_ACTORS", L"1024")`
  (overwrite); keep today's "env/command line wins" logic for dev only.
- `src/host/PoolResize.cpp` `ReadActorSetting` ~236-251: honour `-ai-actors` only under `NOBLE_DEV_BUILD`.
- `src/dedicated/main.cpp` ~25-60: default `requestedLimits` is Stock and `-limits` picks it. Make the default Full and
  set `NOBLE_AI_ACTORS=1024` when not set; in a release server ignore `-limits` (warn), so servers match players.
Things to tell Tyler / check: AI ACTORS 1024 is labelled experimental in the code (last *proved* run was 512; comment
says 1,024 needs the 0x76EBF8 buffer fix and the F6/-horde test), although the swarm run held 1,024 actors without a
crash. Full raises memory use for every player. Saves move to the `_ext_full` save-folder suffix
(`limits::SaveFolderSuffix`), so existing players' saves under Stock won't show until migrated. Joins already refuse a
mismatched `limits_id` (`LocalNetworkId`), which is what keeps builds equal.

### 7b. Spawn / stress-test keys → numpad, dev only (DONE 08:16: F6/F7/F8/F9 → Numpad 6/7/8/9, Num Lock on; SpawnTest.cpp/.hpp, DevMenu.cpp, swarm notes; needs build)
Tyler: F6, F7, F8 → **Numpad 7, Numpad 8, Numpad 9**, and only for devs.
- Already dev-only: `src/host/SpawnTest.cpp` is wrapped in `#if defined(NOBLE_DEV_BUILD)` (Install at ~677 is a no-op
  in release; `Loader.cpp:154` calls it), and the DevMenu (+ key) is dev-only. Nothing to add for "devs only" beyond
  keeping it that way.
- Keys in `SpawnTest.cpp` ~598-628 (game tick hook): `VK_F6` (horde +100 actors around you) → `VK_NUMPAD7`,
  `VK_F7` (place every AI squad) → `VK_NUMPAD8`, `VK_F8` (crowd of 128 Spartans on/off) → `VK_NUMPAD9`. `VK_F9`
  (500 objects) was not mentioned: ask Tyler (Numpad 6?). Rename `s_f6Down`/`s_lastF6` etc. if wanted.
- Update the text too: log lines in `SpawnTest.cpp` (~601, 612, 628, 800, 836, plus the F7/F8 warnings ~511, 526),
  comments in `SpawnTest.hpp` ~10-13, DevMenu button labels/status text `src/ui/DevMenu.cpp` ~258-280, and
  `docs/swarm-test-notes.md`.
- VK_NUMPAD7-9 only fire with Num Lock on (off, those keys send Home/Up/PgUp). Either tell Tyler or also accept the
  scan codes.

## 8. Suggested order

1. Tyler: rebuild, test a Forge map launch, commit the megapack stream.
2. Section 7a (engine limits): small, do next. (7b done.)
3. Check the master server passes `launch_data.map_variant`.
4. Join failure: get Tyler's log after a retry.
5. Spectre + Pelican into Forge World (needs HREK folder) together with the map hash check.
6. Swarm-only Firefight variant, then flood audio.
7. C12-L: membership message (#32) and the team-change trace run.
