# Project Noble — Closed Source

A standalone Windows client that hosts **Halo: Reach**'s own engine from a
legitimate Halo: The Master Chief Collection (Steam) install — without
launching MCC. It provides the front-end menu, the 3D Armory, the server
browser, career/armory progression, and a dedicated server host.

> **Unofficial fan project.** Halo: Reach, Halo: The Master Chief Collection
> and all game assets are © Microsoft / 343 Industries. Noble ships **no game
> files**: it loads `haloreach.dll` and the user's own maps from their MCC
> install. You must own the game to use it.

## Status

This is the **private, closed-source development tree**. All new work happens
here and is pushed only here.

The public "Halo: Reach Community Edition" repository
(`O7OF/Project-Noble-Source-Build`) is **frozen at the 0.9.16.35 open-source
base**; it no longer receives updates. Do not push to it.

There is no public contribution process. Do not open pull requests or issues
against this repository from outside.

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
| Closed-source client (this) | `O7OF/Project-Noble-Closed-Source` | Private. All development. |
| Frozen open-source base | `O7OF/Project-Noble-Source-Build` | Public, archived at 0.9.16.35. |

Packaged builds are distributed through the existing release channel; the
in-client updater lives in `src/common/Updater.cpp` and still points at it.

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

## Rules for this repository

- **Never commit game assets** (maps, CSVs, OBJ/MTL, DLLs), personal data, or
  anything that bypasses MCC ownership checks.
- **Never push this tree to a public remote.** The only remote is the private
  closed-source repository.
- Build artifacts (`build/`) and local game data stay out of git.

## License

Proprietary. See [LICENSE](LICENSE). Third-party libraries and the historical
MIT grant for the pre-closure open-source code are documented in
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).
