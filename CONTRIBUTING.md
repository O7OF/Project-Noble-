# Contributing to Project Noble

Thanks for helping. This is a C++20 Windows client that hosts Halo: Reach's
engine from a local MCC install. Please read this before opening a PR.

## Before you start

- You need Windows 10/11, Visual Studio 2026 with the C++ workload, CMake, and
  a legitimate Steam copy of Halo: The Master Chief Collection with Reach.
- Never commit game assets (maps, CSVs, OBJ/MTL, audio, DLLs from MCC) or
  anything that bypasses ownership checks. `data/menumaps`, `data/armor`,
  `data/userdata` and `appearance.dat` are git-ignored on purpose.
- Check existing issues and pull requests before starting; describe the
  architecture or known issue you are touching in the PR.

## Build

See [BUILDING.md](BUILDING.md). Build all three targets before sending a PR:

```powershell
cmake -S . -B build -A x64
msbuild build\noble-client.vcxproj /p:Configuration=Release /p:Platform=x64
msbuild build\noble-dev.vcxproj    /p:Configuration=Release /p:Platform=x64
msbuild build\noble-dedicated.vcxproj /p:Configuration=Release /p:Platform=x64
```

`build.bat` builds the current checkout and does not touch git; pull your
branch manually first if you want the latest commits.

## Pull requests

1. Fork the repository and create a topic branch from `main`:
   `feat/<short-name>`, `fix/<short-name>`, `notes/<short-name>`.
2. Keep the change focused. One feature or fix per PR.
3. In the PR, describe what changed, where, how it was built and how it was
   tested.
4. Test it: launch the client, open the Armory, cycle the affected armour /
   colours / visor, and check `noble-client.log` / `noble-dev.log` for errors.
   For engine changes, test at least one multiplayer map.
5. Open the PR with the template. Describe the reproduction, the fix, and the
   test evidence. Small, tested PRs are merged fastest.

## Coding style

- C++20, 4 spaces, braces on the same line, `noble::` namespace.
- Prefer RAII and standard containers. Never add a game-asset dependency to a
  header.
- Keep engine memory access behind the existing `Safe*` helpers.
- Comments explain *why*, not *what*. Name the engine offset or tag when a
  constant comes from the engine.
- `.editorconfig` covers the basics.

## Reporting bugs

Use the bug report template. Include the version (top-right of the menu),
whether it is `noble-client` or `noble-dev`, the map, the exact steps, and the
last lines of `noble-client.log` or `noble-dev.log`. If the game crashed,
attach `noble-crash.dmp` if it exists.

## Conduct

By participating you agree to [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
