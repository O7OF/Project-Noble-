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

The release exe can embed the menu map and string table through
`src/res/blocks.rc`. They are game data: git-ignored, never committed, and
must come from your own MCC install.

- `data/menumaps/mainmenu_all.map`
- `data/menumaps/mainmenu_all.string_ids.csv`

If both files are present CMake embeds them. If they are missing the build
still succeeds: CMake compiles without the embedded menu data and the client
reads `data/menumaps` at runtime instead. There is no `RC2135` failure for a
fresh checkout.

To copy the map from your own install:

```powershell
.\tools\fetch-menu-data.ps1
# or point it at an extracted map:
.\tools\fetch-menu-data.ps1 -MapPath "D:\Reach\data\menumaps\mainmenu_all.map"
```

The script checks the MCC `haloreach\maps` folder and a Reach Renaissance
extraction. Not every MCC install keeps the menu map in `haloreach\maps`; if
yours does not, use `-MapPath` with your extracted `mainmenu_all.map`.

Use the **Armory-capable** menu map (the one with the Armory scenes and the
H2A/Council/Schism Elite regions). A different menu map may load, but those
extra Elite pieces will not have regions to render.

The stock `haloreach\maps\mainmenu.map` loads the front-end but does **not**
contain the Armory scene: the 3D preview stays on `LOADING ARMORY`. For the
Armory, compile the menu map with **HREK** (with the Armory scene) and place it
as `data/menumaps/mainmenu.map` or `data/menumaps/mainmenu_all.map`. It is
served loose at runtime, so it never has to be embedded in the exe.

The string table (`mainmenu_all.string_ids.csv`) is an extracted dump, not a
file MCC ships. Supply the matching CSV next to the map; the Armory needs it at
runtime. Do not redistribute the map or the CSV.

The Armory's optional armour exports live in `data/armor/` and are also
git-ignored.

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
  from `data/` (see above).
- **Black Armory / no character** — the menu map is missing or from a
  different build; restore the map + matching CSV.
