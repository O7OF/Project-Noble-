# Halo Customs MCC Megapack in Noble (2026-11-09)

Community custom game types (`.bin`) and Forge maps (`.mvar`) from the Halo Customs MCC Megapack
(ModDB "Halo MCC Reach Megapack", Dec 2019). The pack group gave Noble permission to ship it. Credits:
`data\credits\megapack.txt` (written by the import script: pack credits + every creator named in the files).

## Import

    powershell -ExecutionPolicy Bypass -File tools\megapack\import-megapack.ps1

Source defaults to `Desktop\Reach game modes and maps`. The script:
- skips MCC's own files (`*_054.bin`, `hr_*`, `00_basic_editing*`, `*_cl_031.mvar`, `hr_forgeWorld_*`);
- skips files the pack marks as not working (`(NEED MAP)`, `(Broken)`, `(Might be broken)`, `(Questionable)`,
  `(Try)`, `(NEED Gametype)`);
- sets aside Xbox 360 exports in `build\megapack\convert` (38 files at the time of writing: a `_cmp` zlib body or
  big-endian header text; 4 KB-padded sizes 8192/12288/16384/20480/24576/32768). Re-save them in ReachVariantTool and
  run the script again with `-Source` on the re-saved folder;
- copies the rest into `data\game_variants` / `data\map_variants` (repo, shipped by `build-package.ps1`) and
  `build\bin\Release\data\...` (dev runs); writes `build\megapack\manifest.csv`.

Header layout used (PC files): `_blf` 0x00, `chdr` 0x30 (creator ASCII 0x88, modifier 0xAC, base map id int32 0x6C,
title UTF-16 LE 0xC0, description 0x1C0), body chunk 0x2F0 (`mpvr` v54 0x5028 bytes / `mvar` v31 0x7028 bytes).

## Menu

- Game types: any `.bin` in `data\game_variants` was already listed (ScanVariants); grouped by detected type, the
  rest under "Other".
- Forge maps (new): Custom Game and Forge have a third source tab, FORGE MAPS (n). Headings come from the pack's
  file prefixes (BTB, INF, INV, MINI, Race, SLYR, OBST, MISC, Hot Pursuit, Cops & Robbers). Scanned from
  `data\map_variants` and MCC's own saved Forge map folders (`LocalLow\MCC\...\HaloReach\Map`). Only PC-format
  files whose base map id is a known Reach map are listed.
- Launch: the map's `.mvar` goes to the match process as `-mvar` (BlamEngine already parses it into the launch block
  and takes the map id from it). Lobbies send its file name as `launch_data.map_variant`; guests use their own copy.
  **The master server must pass `map_variant` through** in `launch_data`, or guests start the plain map.

## Open

- Re-save the 38 Xbox 360 files.
- Guests' lobby screen shows the base map name, not the Forge map's title (no advert field for it yet).
- No pairing of a Forge map with its game type yet (e.g. "INF 30 Days and Nights" + "30 Days and Nights.bin").
- Map hash check before anyone plays a mismatched file online.
