# Handoff - Forge AI spawners, Forge names, v0.20.36 (2026-11-22 04:05)

For the next agent. Read `docs\PHASE7-AI-PATHS-PROGRESS.md` (top sections) first. Repo `D:\Project-Noble-Closed-Source`,
branch `main` = `origin/main` at `bf5a1b4`. Standing rules: no Co-Authored-By / Claude-Session / "generated with" lines
anywhere; commit only built (noble-dev + noble-client, no new warnings) and tested work; no release without Tyler's OK;
never commit RR data, logs or dumps; never use computer use (Tyler keeps his mouse); network/server is O7's.

## Released today
- **v0.20.35** (01:15): MAP AI, Forge PLACE AI, AI Forge World pack.
- **v0.20.36** (03:10, `1b9f05f`): Forge menu names (26 strings in `ui\cui\strings\forge_strings`), Forge World check for
  updates / UPDATE button in MODS > RECOMMENDED MAPS, fixed pack lookup (`/releases/latest` first; the release list is
  ordered by tag commit date and picked v0.20.28). Pack map 838,799,360 bytes, SHA-256 82E125E5...0DC5.

## Tyler's open requests (03:50, from his test video and pause-menu screenshot)
1. **Remove PLACE AI / AI TYPE from the ESC menu** - DONE in code, NOT built or committed (`src\ui\PauseMenu.cpp`: rows and
   handlers removed; `spawn::RequestPlaceAiInFront` / `PollPlaceAiNotice` kept for `-place-ai-at`).
2. **AI as its own top-level Forge category** (not inside Other (MCC)). Blocker: Reach allows 16 map variant palette
   categories and Forge World has 16 (`AddElement` asserts). Options: repurpose `ff_thorage_hidden` (index 14: generator,
   vending machine, dinghy; probably hidden), or move Other (MCC)'s 4 entries. Decide only after item 7.
3. **AI spawners spawn nothing** (first build: character attached straight to the marker - ignored).
   Fixed in tags, NOT tested: the engine creates AI from **effect parts** whose type is a character
   (`FUN_180120238`, case `'char'` -> `FUN_180692c5c`; Noble's ForgePatches already NOP the check at hr+0x692CB5,
   the same 6 bytes Forge World AI Battleground's modded dll changes). Each spawner is now
   `objects\noble\ai_spawners\<kind>\noble_ai_spawn_<kind>.scenery` (copy of `respawn_point.scenery`, multiplayer type
   set to ordinary) with attachment -> `noble_ai_spawn_<kind>.effect` (location 0 marker `root`, 1 event, 1 part type =
   character, loop start event -1). Kinds: elite, grunt, jackal, brute, trooper (characters from the scenario palette).
   Palette: entry `noble_ai` ("AI", variants Elite/Grunt/Jackal/Brute/Trooper) appended at the END of the last category
   `ff_thorage_more` (Other (MCC)). Tool: `tools\noble-forge-ai-spawners.ps1 [-Save]` (backs up the scenario into
   `HREK\noble_mods\forge_ai_spawners_backup\`), names in `tools\noble-forge-add-strings.ps1`.
   **A `tool build-cache-file levels\multi\forge_halo\forge_halo` was running when the session stopped** (log
   `HREK\noble_mods\forge_ai_spawners_backup\build3_20261122.log`). Check it finished ("successfully built cache file").
4. **AI options**: which Elite / Grunt / Trooper / Jackal / Hunter etc. (ranks, weapons). Needs more characters in the
   map (campaign characters; the AI Battleground mod ships 180+ variants, 246 character-related tags). One spawner
   (scenery + effect) per character variant; generate them with the spawner tool.
5. **Vehicle AI**: Warthog filled with Marines, Wraith, Ghost with drivers. Not started. Likely an effect part with a
   vehicle + squad/character, or a squad template with vehicle seats; research in the decomp first.
6. **Pelican**: landing gear invisible (collision present), door closed, no open/close. Cause: Pepper's Pelican
   (`pelican_gear_scripted_doors.vehicle`) drives ramp AND gear from scripted object function 3, which starts at 0.
   `src\engine\PelicanControls.cpp` (in release builds too) toggles it with **X inside a vehicle**; the function is global
   (all Pelicans together); online sync untested. Fix ideas: start the function at 1 (gear down) at match start, a
   visible prompt, per-Pelican state.
7. **Saved-map safety (must check before any palette change)**: unknown whether Reach map variants store palette
   positions or tag datum indices. If tag indices, any rebuild that ADDS tags shifts later indices (the spawner build
   added 5+ tags at index 0x30E9 after `thorage_named_location_area`) and maps saved on v0.20.36 could load wrong
   objects. Test: save a map variant on v0.20.36 (pieces after the insertion point, e.g. Target Designator, flyable
   Pelican), load it on the new build. Or find the map variant load in the decomp (quota / placement structs).

## Test state
- Tyler's launcher `C:\Users\tyler\Desktop\Noble Launcher\data\maps\forge_halo.map` = the FIRST spawner build
  (direct character attachment, SHA 769CEB61...; spawns nothing). Backups in `...\data\maps-backup\`
  (`forge_halo.map.v0.20.35-20261122-0339`, `.v0.20.34`).
- Dev `build\bin\Release\data\maps\forge_halo.map` = the same first spawner build; v0.20.36 map backed up as
  `HREK\noble_mods\forge_ai_spawners_backup\forge_halo.map.v0.20.36`.
- `HREK\maps\forge_halo.map` = whatever build3 produced (effect spawners with `root`).
- **Do not ship** the spawner map until items 3 and 7 are tested. To release without it, put
  `forge_halo.map.v0.20.36` back.
- Scripted placement test: `-forge-piece-at <index hex>[/<type>]|x,y,z@s` (SpawnTest.cpp; optional object type added
  today, NOT built). The map's name-buffer index (0x30E9 for the Elite spawner) did NOT match the runtime instance table
  (groups 112 / 192 / 112 for three identical-type spawners), and placing 0x30E9 crashed in `object_new` (0x46D7B2).
  Find tags by name another way (map header +0x10 tag index offset, +0x20 count; see SharedTagCache::ParseDonorTags).
- Test runs abort while Tyler's game is open (RAM guard): check `Get-Process Noble` first.

## Uncommitted files
`src\ui\PauseMenu.cpp`, `src\host\SpawnTest.cpp`, `tools\noble-forge-ai-spawners.ps1` (new),
`tools\noble-forge-add-strings.ps1` (+6 AI names), `tools\research\tag-find-field.ps1` (new). Build both targets, test,
then commit (no attribution lines).

## Other notes
- `build\bin\Release\data\mods\armor_fx` = local test pack (old Forge World + Boneyard from the 11/17 armor flame work);
  Tyler OK'd removing it (move to a backup outside the game).
- Dev `userdata` was wiped by a `robocopy /MIR` mistake (test data only; Tyler accepted). Keep test backups outside the
  folder they restore.
- `mp-session-test.ps1`: call with `&` so `-ExtraHostArgs @(...)` arrives as separate arguments; `-place-ai-at` takes
  a number (0 Elites .. 4 Troopers).
- PowerShell + ManagedBlam: return block fields with `return ,$f`; never test a TagElement for truthiness or pass it to
  `.IndexOf` (PowerShell enumerates it); TagFieldData uses `GetData()` / `SetData()`; one ManagedBlam start per process.
- Remaining Phase 7: Forge pieces raised above ground (ground finder `0x4A41A4`), team rules (Troopers vs players), O7
  map check for guests.
