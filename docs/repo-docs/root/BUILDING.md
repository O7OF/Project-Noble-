# Building Project Noble

## Requirements

- Windows 10 or 11 (64-bit)
- Visual Studio 2026 with "Desktop development with C++"
- CMake 3.20+ (the one bundled with Visual Studio works)
- Git
- Halo: The Master Chief Collection (Steam) with Halo: Reach installed, for
  running the client — not needed to compile

## Configure

From the repository root:

```powershell
cmake -S . -B build -A x64
```

CMake fetches Dear ImGui, spdlog, nlohmann/json, MinHook and Opus through
`FetchContent` on the first configure.

## Build all three targets

```powershell
msbuild build\noble-client.vcxproj      /p:Configuration=Release /p:Platform=x64 /m
msbuild build\noble-dev.vcxproj         /p:Configuration=Release /p:Platform=x64 /m
msbuild build\noble-dedicated.vcxproj   /p:Configuration=Release /p:Platform=x64 /m
```

Outputs:

| target | output | purpose |
|---|---|---|
| noble-client | `build\bin\Release\noble-client.exe` | release client |
| noble-dev | `build\bin\Release\noble-dev.exe` | development client with log + test switches |
| noble-dedicated | `build\bin\Release\noble-dedicated.exe` | dedicated server host |

## Data files

A fresh checkout contains everything needed to **compile**:

- `data/profile_1.bin`, `data/launch_block_1.bin`, `data/lb_m_c_1.bin`
- `data/menumaps/mainmenu_all.string_ids.csv` and the two
  `mainmenu_armory*.string_ids.csv` tables
- `data/blocks/*.bin`
- `src/res/spartan.rrpmcg` (the Spartan item -> region table, always embedded)

Two large game-data files are **not** in git because GitHub rejects files over
100 MB. They are distributed as release-zip assets and must be present at
runtime for the custom Armory:

| file | size | purpose |
|---|---|---|
| `data/menumaps/mainmenu_all.map` | ~298 MB | the Armory menu-map mod: pedestal scene and `rr_mark_a`/`rr_mark_b` GLOBALS |
| `data/armor/rr_elite_pack.map` + `.rrreloc`/`.rrneeds`/`.rremotes`/`.rrpmcg` | ~103 MB | the armour pack: Spartan/Elite models and the piece -> permutation table |

Put them here:

```powershell
# From a Reach Renaissance extraction / the release zip:
Copy-Item "D:\Reach\data\menumaps\mainmenu_all.map" data\menumaps\ -Force
Copy-Item "D:\Reach\data\armor\rr_elite_pack.map*"       data\armor\  -Force
```

The client serves `data\menumaps\mainmenu_all.map` as the engine's
`haloreach\maps\mainmenu.map` (VFS) and loads the pack as an extra cache.
Without the map the Armory falls back to the stock MCC menu map (no custom
pedestal/pieces); without the pack the Armory has no pack models or .rrpmcg.

The pack's own `.rrpmcg` is preferred; `src/res/spartan.rrpmcg` is embedded
as a fallback. The pack's own string table is read from the map, so the
`mainmenu_armory*.string_ids.csv` files are only fallbacks.

The stock `haloreach\maps\mainmenu.map` does not contain the modded Armory
scene; use the Armory-capable `mainmenu_all.map` above.

## Package a release zip

GitHub's repository file limit is 100 MB, so the map and pack travel in a
release-asset zip:

```powershell
.\tools\make-release-zip.ps1
# -> %USERPROFILE%\Desktop\Noble-Armory-Pack.zip (Noble.exe + data, incl. the map and pack)
```

Upload that zip as a GitHub **Release asset** (not a normal repo file). The
source itself stays small and can be uploaded as the repository or a source
zip.

## Run

```powershell
.\build\bin\Release\noble-client.exe -map mainmenu -mode menu
```

Useful test switches: `-map <name> -mode multiplayer`, `-menu-screen armory`,
`-menu-capture <frame>`, `-armory-unlock-all`. Keep a log from
`noble-dev.exe` when reporting engine issues.

## Troubleshooting

- **LNK1104: cannot open noble-client.exe** — close any running Noble process
  and rebuild.
- **CMake generator mismatch** — delete `build/CMakeCache.txt` and
  `build/CMakeFiles`, then reconfigure.
- **RC2135: file not found** — a resource in `src/res/blocks.rc` is missing
  from `data/`; the tracked files above should cover a fresh checkout.
- **Black Armory / no character** — `data\menumaps\mainmenu_all.map` or
  `data\armor\rr_elite_pack.map` is missing; copy them from the release zip.
- **Wrong armour pieces** — the pack is present but its `.rrpmcg` does not
  match the map; use the pair from the same release zip.
