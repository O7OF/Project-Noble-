# Project Noble — Halo: Reach Community Edition

An open-source, standalone Windows client that hosts **Halo: Reach**'s own
engine from a legitimate Halo: The Master Chief Collection (Steam) install —
without launching MCC. It provides the front-end menu, the 3D Armory, the
server browser, career/armory progression, and a dedicated server host.

> **Unofficial fan project.** Halo: Reach, Halo: The Master Chief Collection
> and all game assets are © Microsoft / 343 Industries. Noble ships **no game
> files**: it loads `haloreach.dll` and the user's own maps from their MCC
> install. You must own the game to use it.

## Status

This repository is the **open-source 0.9.16.35 base** — the first public
Project Noble release, kept stable for contributors. The current packaged
builds are published on the release repository:

**Releases:** https://github.com/O7OF/Project-Noble-/releases

## What's in the box

- **Front-end menu** — Native ImGui shell over the real Reach main-menu map.
- **3D Armory** — Spartan and Elite customisation on the engine's own pedestal,
  with the real permutation/region data from the game's model.
- **Multiplayer / Firefight / Forge / Campaign host** — Direct engine host,
  server browser, lobbies, join-in-progress.
- **Dedicated server** — Headless `noble-dedicated.exe` for hosting.
- **Career, challenges, armory unlocks** — Ranks, cR, purchases, commendations.
- **Mod support** — Map overrides, custom UI art and VFS user storage.

## Repositories

| What | Repo | Notes |
|---|---|---|
| Client, engine host, menus | **this repository** | C++20, CMake + Visual Studio |
| 
| Release downloads | https://github.com/O7OF/Project-Noble-/releases | Packaged Windows builds |


## Build

Requirements: Windows 10/11, Visual Studio 2026 with the C++ workload, CMake,
Git, and a local MCC install with Reach for running the client.

```powershell
cmake -S . -B build -A x64

msbuild build\noble-client.vcxproj    /p:Configuration=Release /p:Platform=x64 /m
msbuild build\noble-dev.vcxproj       /p:Configuration=Release /p:Platform=x64 /m
msbuild build\noble-dedicated.vcxproj /p:Configuration=Release /p:Platform=x64 /m
```

All three targets build from the same tree:

| Target | Output | Purpose |
|---|---|---|
| `noble-client` | `build\bin\Release\noble-client.exe` | Release client |
| `noble-dev` | `build\bin\Release\noble-dev.exe` | Development client (logging, test switches) |
| `noble-dedicated` | `build\bin\Release\noble-dedicated.exe` | Dedicated server host |

The menu map and string tables are embedded from `data/menumaps` at build
time; they are git-ignored and must be extracted from your own MCC install.
See [BUILDING.md](BUILDING.md) for details and troubleshooting.

## Run

```powershell
.\build\bin\Release\noble-client.exe -map mainmenu -mode menu
```

Useful test switches: `-menu-screen armory`, `-menu-capture <frame>`,
`-armory-unlock-all`, `-map <name> -mode multiplayer`.

## Contributing

Contributions are welcome. Read [CONTRIBUTING.md](CONTRIBUTING.md) and
[BUILDING.md](BUILDING.md) first, then open a focused pull request.

- Bugs / feature requests: use the issue templates.
- Questions and design: GitHub Discussions.
- Security: see [SECURITY.md](SECURITY.md) — do not file public issues.
- By participating you agree to the [Code of Conduct](CODE_OF_CONDUCT.md).

Please never commit game assets (maps, CSVs, OBJ/MTL, DLLs), personal data, or
anything that bypasses MCC ownership checks.

## License

The Noble source code is released under the [MIT License](LICENSE). Halo game
assets and trademarks are not licensed by it and are not distributed here.
Bundled third-party libraries keep their own licenses (Dear ImGui, spdlog,
nlohmann/json, MinHook, Opus).
