# Mod bug findings, session 2026-11-10 (companion to MOD-BUGS-2026-11-10.md)
(Copy of claude.ai project doc.)

## #3 Mod weapon sounds: FIXED and confirmed by Tyler
- Cause: mods keep `fmod\pc\*.fsb` and `game_variants\` under `<mod>\haloreach\`, maps directly under `<mod>\maps`. The engine asks for `haloreach\fmod\pc\sfx.fsb`; VFS reduced it to `fmod\pc\sfx.fsb` and never looked in `<mod>\haloreach\`.
- Fix: `src/vfs/VFS.cpp`, extra lookup `s_activeMod / "haloreach" / p` in the active-mod block. Confirmed in Civil Unrest and Prophet's Hand.

## #2 Trepanation: menu-background bug FIXED, map still does not load
- Cause of the menu background: carrier `trepanation` has no map id, so ResolveMapId returned 0 and Noble launched the main menu.
- Fix: `FrontEndMenu.cpp` (`ModMaps`): a map's base scenario is used as carrier only if it is a known stock scenario or contains `forge_halo`; otherwise the mode default (`70_boneyard` for multiplayer) or `data\map_carriers.json`.
- Now: engine reads 23,958 tags, then `map load returned 0` and `Slot 4 reason=0`. Trepanation is 2,157,719,552 bytes (> 2 GiB); several header offsets above 2^31. Suspected signed 32-bit limit in haloreach.dll; unproven.

## #1 Custom-map missions stop loading: NOT fixed (at that time)
- Rule seen in every log: a mod map served under a slot whose scenario differs from the map's own fails (`map load returned 0`, or fallback to `maps\mainmenu.map`, then `Slot 4 reason=0` ~4 s later).
- `m50rvh` (header name `m50`) loads when served as `maps\m50.map` and fails when served as `m10`.
- Rewriting header name (0xC0) and path (0xE0) does NOT fix it: the engine checks something inside the map data.
- Using the map's own stock MP/Firefight slot as carrier doesn't work in campaign mode.
- Next if resumed: reverse-engineer the load check in haloreach.dll (read-only).

## Leftovers
- `NOBLE_MAP_NAME_OVERRIDE` diagnostic is a dead end.
- `data\map_carriers.json` restored to the two forge_halo entries.
