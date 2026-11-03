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

The release exe embeds the menu map and string tables through
`src/res/blocks.rc`. To build, those files must exist on disk (they are
git-ignored and never committed):

- `data/menumaps/mainmenu_all.map`
- `data/menumaps/mainmenu_all.string_ids.csv`

Extract them from your own MCC install; do not redistribute them. If they are
missing the resource compiler reports `RC2135`.

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
