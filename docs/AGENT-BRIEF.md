# Project Noble - agent brief (read this first)

Two pages. It holds what an agent needs before touching the repo; the detail lives in `00-START-HERE.md`,
`01-PROJECT-HISTORY.md`, `02-LESSONS-AND-RULES.md`, `AGENT-COORDINATION.md` and the source folders of this
knowledge folder (`knowledge\`, git-excluded - never committed or pushed).

## What it is

`noble-client.exe` / `noble-dev.exe` host Halo: Reach's own engine (MCC's `haloreach.dll`) without launching MCC:
own ImGui front-end, 3D Armory, custom games / Forge / Firefight / campaign, server browser, master server, P2P +
relay netcode, mods served through a VFS, extended engine limits. Unofficial fan project; no game files ship.
Partners: Tyler (builds/tests/RE, not a C++ developer), O7OF (upstream), Dominus.

## Where things are

| Thing | Path |
|---|---|
| Repo | `D:\Project-Noble-Closed-Source` (branch `wip-merge`) |
| Build output | `build\bin\Release` (`noble-dev.exe`, `noble-client.exe`, `noble-dedicated.exe`) |
| Live test folder | `build\bin\Release\data\userdata\claude-tests-2026-11-11` (tools, captures, logs) |
| Test scripts | `...\claude-tests-2026-11-11\tools` |
| Knowledge | this folder (`D:\Project-Noble-Closed-Source\knowledge\`) |
| HREK (tags, tool.exe, edits) | `D:\Steam\steamapps\common\HREK` |
| MCC install (never modify) | `D:\Steam\steamapps\common\Halo The Master Chief Collection` |
| Reclaimer | `D:\RECLAIMER\Reclaimer-2.1.1645` |

## Build

```powershell
cmake --build build --config Release --target noble-dev noble-client noble-dedicated
```

from `D:\Project-Noble-Closed-Source`, with the VS2022 CMake at
`C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`.
New `.cpp` files must be added to `CMakeLists.txt` (client list and `DEDICATED_SOURCES` when used there).
Release gates: `noble-dev` and `noble-client` only; the dedicated build does not block a push.

## Test (non-negotiable rules)

- Never test while Tyler is playing: `Get-Process noble-dev, noble-client` first.
- Window on the **AOC** (second monitor): scripts set `NOBLE_WINDOW_SECONDARY=1`.
- **Do not take his mouse**: pass `-NoFocus` to `camp_test.ps1` / `match_elite.ps1`. F5 needs focus - ask first.
- Back up/restore `build\bin\Release\data\appearance.dat` around every test (scripts do).
- Prove every cause with a one-variable test plus a capture/log. Then look at the capture: crop or shrink images
  over ~3.7 MB.
- `tools\camp_test.ps1 -Name X -Map m35 -Frames '160,190' -Quit 195 -Extra '-third-person' -NoFocus`
- `tools\match_elite.ps1 -Quit 70 -Frames '40,60' -Extra '-third-person' -NoF5 -NoFocus`
- `tools\forge_test.ps1 -NoOverride` (serves `data\maps\forge_halo.map`), `armory_preview.ps1`, `menu_test.ps1`.
- Tag work: `runmb.ps1 -Script X.py -Out file -Env 'K=V;...'` (ManagedBlam through Blender), `build_map.ps1
  -Scenario <path>` (HREK `tool.exe build-cache-file`), Reclaimer scripts `dotnet fsi rc_*.fsx`.

## Engine facts (haloreach.dll RVAs)

| What | Where |
|---|---|
| Live tag table | `*(haloreach+0xC1A600)` |
| Game-state bank table | `+0x27BF270`, one 0x18F0 record per bank; `+0xA0` capacity, `+0xA8` bytes used |
| Zone-set rebuild (map load) | `+0x8EAF0`, prologue `48 8B C4 48 89 58 10` |
| Extra-cache step | `+0x30698` (only runs for extra caches) |
| matg player representations (was used by the removed Trooper mode) | `+0x474` count / `+0x478` pointer, stride `0x6C`, biped at `+0x40` |
| Game over / game finish | `+0x7BA58` / `+0x59330` (hooked by `LobbyPatches`) |
| Player unit lookup | `+0x53EF8` |
| Sim->render state copy | `+0x1CD389` (inside `+0x1CD2BC`) |
| Set permutation / variant | `+0x4729F4`, `+0x472B70`, `+0x473858` |
| Camera mode check / unit third person | `+0x48A910` / `+0xC6948` |
| Object's first dword | its definition datum |
| `data\forge_palette.json` | Noble's own Forge menu (13 tabs), not the native in-map palette |

Bank rule: the game-state allocator is a bump allocator with no bounds check. Anything that grows a pool must be
charged against the bank's remaining room or it silently corrupts the session (see the 2026-11-14 beam overflow:
68,052,044 of 67,043,328 bytes).

## Current state (11-14 20:40 ET)

- `main` and `wip-merge` both at `aae1675` (pushed): upstream v0.19.91 + the RecommendedMaps fallback fix, gates built
  20:13-20:14 and tested. Upstream releases fast (v0.19.90/v0.19.91 both base-zip-only); both merged and their missing
  optional Forge packs attached, so the latest release has the pack again.
- Live fix awaiting release: `aae1675` makes the in-client Forge World download scan the last 10 releases and use the
  newest one that has the pack (a base-only release can no longer break the row).
- Trooper mode removed (CAMPAIGN = VANILLA/MODDED); Marine Armory works without Marine mods. Open: publish a release
  with `aae1675`; `rr_*` HaloScript names inside `noble_menu.map` (invisible; Tyler's call); optional dev-only cleanup.
- Push rule: Tyler directs pushes (this session: main). Confirm before publishing a release; check GitHub first.

## Pitfalls that cost days

- Test with obviously different pieces (`odst_light`, not `space_marine`); confirm which actor you are looking at.
- Make sure the test script cannot undo the thing under test (a scripted F5 once turned third person off).
- Object writes from the render thread are overwritten by the sim->render copy: apply on the simulation thread.
- Stock MCC maps are compressed: string-id lookups find 0 names there (Reclaimer for names).
- HREK builds read split resource tags over the scenario's blocks and compile `hs_source`; bad ids silently strip
  scripts.
- PowerShell: `$t` == `$T`; `[IO.File]` relative paths resolve to System32; `R` is an alias.
- MSVC: `__try` functions cannot hold objects with destructors (C2712); split them.
- Merges: untracked files with the same names block a merge; dedicated build needs `#ifndef NOBLE_DEDICATED`
  around client-only calls.

## Working with a second agent

`AGENT-COORDINATION.md` owns the split (lead: `src\`, `tools\`, `data\`, git, tests; cc: `knowledge\drafts\`).
Claim before writing; one game instance at a time; never edit another agent's claimed files.
