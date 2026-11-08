# Start here (new DeepSeek session)

You are joining an existing project as one of two coding agents. The other agent is Claude. Tyler is
the person you work for. He tests every build himself and decides what gets pushed.

## The project

**Project Noble** is a standalone Windows client (C++20, CMake, MSVC) that runs Halo: Reach's own
engine (`haloreach.dll` from the player's Steam MCC install) without launching MCC. It has its own
menus (ImGui), lobbies, server browser, career/challenges, a dedicated server and mod support.

- Repository: `D:\Project-Noble-Closed-Source` (private, branch `main`, remote `O7OF/Project-Noble-Closed-Source`)
- Untouched backup of v0.19.20: `D:\Project-Noble-ORIGINAL`. **Never edit anything there.**
- Shell: PowerShell on Windows.
- Three build targets: `noble-client` (release client), `noble-dev` (dev client with logs/test
  switches), `noble-dedicated` (headless server).

## What you're working on

**Extended Engine Limits:** raising Reach's engine limits (object pool 2,048 → 32,767, render
lists, physics pools; later particles, light volumes, AI actors) so 64-player lobbies render
everyone and Flood horde modes become possible. The rule that matters most: Noble **never modifies the player's
installed `haloreach.dll`**. It builds a patched copy in its own folder and loads that.

Read these in order before doing anything:

1. `LIMITS-HANDOFF.md`: research, verified facts, architecture, the shared code interface.
2. `LIMITS-TODO.md`: task list with owners (C = Claude, D = DeepSeek), files and acceptance criteria.
3. `AGENT-STATUS.md`: who owns what right now, plus the coordination rules.

## Rules

- **Claim a task in `AGENT-STATUS.md` before editing**, listing the files/slices you will touch.
  Keep the claim updated and add a log line when you finish.
- **Never edit Claude's files or slices** (see its claims). Need a change there? Write it in
  `AGENT-STATUS.md`.
- Pull before editing, commit small and often, pull again before pushing. **Never push without
  Tyler's OK.** Never force-push or rewrite anyone's commits.
- There are uncommitted changes from Claude in the working tree (EngineLimits, Loader, CMakeLists,
  Career.cpp, docs). Don't commit them as yours and don't revert them.
- **Do NOT run `build.bat`.** It packages and **publishes a GitHub release**. Build with:

  ```powershell
  cmake -S . -B build -A x64
  msbuild build\noble-client.vcxproj    /p:Configuration=Release /p:Platform=x64 /m
  msbuild build\noble-dev.vcxproj       /p:Configuration=Release /p:Platform=x64 /m
  msbuild build\noble-dedicated.vcxproj /p:Configuration=Release /p:Platform=x64 /m
  ```

- `tools/limits/recipes/` and `data/limits/*.nlp` contain copies of Reach code and are gitignored
  on purpose. Never commit them.

## Current state

Claude finished C1-C3 (uncommitted, not yet compiled with MSVC):

- `src/host/EngineLimits.hpp/.cpp`: `limits::PrepareEngineDll(stockDll, profile)` builds or reuses
  `%LOCALAPPDATA%\ProjectNoble\engine\v16-<hash>\haloreach.dll` and falls back to the stock DLL on
  any problem. **It is real; call it directly, no stub needed.**
- `src/host/Loader.*`: new `LoadReachDll(dllPath, dependencyDll)`. Pass the patched copy as the first
  argument and the **stock** path as the second, so sound (xaudio2) still loads.
- `tools/limits/build_recipe.py` and the generated `data/limits/v16.nlp`.

## Your tasks, in order

1. **Baseline build** of all three targets with Claude's changes, before your own edits. Fix compile
   errors only if they're trivial (a missing include); otherwise report them in `AGENT-STATUS.md`.
2. **D1**: `extendedLimits` setting + SETTINGS row "ENGINE LIMITS: Stock / Extended".
3. **D2**: call `PrepareEngineDll` before `LoadReachDll` in `src/main.cpp` (step 8) and
   `src/dedicated/main.cpp` (dedicated: `-limits v16` flag). Log
   `Engine limits: requested X, active Y, reason Z`. If it falls back to Stock, start normally.
4. **D3**: session gating. Every player and the server in a match must have the same profile; refuse
   mismatched joins with a clear message and add a browser tag.
5. **D4**: campaign saves in a separate folder per profile (`limits::SaveFolderSuffix`).
6. **D5**: show the active profile in the UI and the log header.
7. **D8** (independent, any time): loading-screen "LOADING" text partly covered by the mission text.

Details and file lists for each task are in `LIMITS-TODO.md`.

## When you report back to Tyler

After D1 + D2 build, give him test steps: turn on SETTINGS > ENGINE LIMITS > Extended, restart, check
the log for the "Engine limits" line and that `%LOCALAPPDATA%\ProjectNoble\engine\v16-...\haloreach.dll`
exists, then start a match. Keep reports short: what changed, which files, how to test, anything
blocked.
