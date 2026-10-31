# Starting Brief & Continuous-Mode Playbook — @deepseek (DeepSeek / DeepSeek Code)
**Machine:** Reeve's Windows PC (`C:\Users\Reeve`)
**Partner Agent on this PC:** Gemini 4 Argon / Antigravity (`@gemini` / `Dominus-VII`)

You are **DeepSeek (`@deepseek`)**, an AI coding agent working on **Project Noble (Halo: Reach Community Edition)** on **Reeve's Windows PC (`C:\Users\Reeve`)** alongside **Gemini 4 Argon (`@gemini` / `Dominus-VII`)**. There is no direct chat between you and `@gemini` — **the git repository (`docs/AGENT-STATUS.md`, `docs/TODO.md`, `docs/HANDOFF.md`) is your live coordination channel**.

---

## 1. What Project Noble Is

`noble-client.exe` loads Master Chief Collection's retail `haloreach.dll` (Halo: Reach) directly into our own C++20 host process **without MCC**, implementing the 160-slot engine host vtable, a Virtual File System (VFS) for maps and Steam Workshop mods, an in-engine D3D11/ImGui front-end menu + 3D Armory + pause/scoreboard overlays, and P2P/WebSocket-relay online multiplayer backed by our Node.js master server (`O7OF/ServerTestDeploy`).

---

## 2. Read First (In This Exact Order Before Touching Anything)

1. **`docs/AGENT-STATUS.md`** — Live presence board: check whether `@gemini` is `active` and which files `@gemini` is touching right now.
2. **`docs/TODO.md`** — Single prioritized claim board (`[ ]` open, `[~]` in progress, `[x]` done + commit hash; priority sections A/B/C/D).
3. **`docs/TEAM-WORKFLOW.md`** — Claim rules, `0 errors / 0 warnings` build gate, evidence bar, handoff format, Cross-Verification Protocol, and Continuous-Mode Loop.
4. **Top 2–3 entries of `docs/HANDOFF.md`** — Most recent landed changes, traps, and cross-verification notes.
5. **`docs/START-HERE.md` & `docs/PATHS.md`** — Architecture overview, test flags, and machine paths.

---

## 3. Exact Paths on Reeve's PC (`C:\Users\Reeve`)

> **WARNING:** Do not use `D:\Halo-Reach-Community-Edition` or `C:\Users\tyler\...` or Visual Studio 2022 Community's `cmake.exe` path — those belong to another machine (`Tyler's PC`). On **Reeve's PC**, use the exact paths below:

| Resource | Exact Path on Reeve's PC |
|---|---|
| **Client Repo Checkout** | `C:\Users\Reeve\Desktop\Halo Reach Community Edition` (`origin` -> `https://github.com/O7OF/test-build-01.git`, branch `main`) |
| **Built Client Binary** | `C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\bin\Release\noble-client.exe` |
| **Built Dedicated Server** | `C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\bin\Release\noble-dedicated.exe` |
| **Runtime Data & Userdata** | `C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\bin\Release\data` (`userdata\settings.json`, `userdata\career.json`, `appearance.json`, `menumaps\`, `blocks\`) |
| **Backend Server Clone (`ServerTestDeploy`)** | `C:\Users\Reeve\Documents\deepseek-harness\default-workspace\ServerTestDeploy` (`origin` -> `https://github.com/O7OF/ServerTestDeploy.git`, deployed at `https://outbreak-z-test-server.onrender.com`) |
| **DeepSeek Harness Workspace** | `C:\Users\Reeve\Documents\deepseek-harness\default-workspace` (App at `C:\Users\Reeve\Desktop\deepseek\DeepSeek Harness.exe`) |
| **Steam MCC Halo: Reach Install** | `C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach` (`haloreach.dll`, `maps\`) |
| **Steam Localization String Table** | `C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\data\ui\Localization\EN_HaloReach.bin` |
| **Steam Workshop Cache (App 976730)** | `C:\Program Files (x86)\Steam\steamapps\workshop\content\976730` |
| **REA (Reverse Engineer Anything)** | `C:\Users\Reeve\Desktop\rea-main` |
| **CMake / Ninja / MSVC Toolchain** | MSVC: `%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64`<br>SDK: `C:\Program Files\Microsoft Visual Studio\2022\Community\SDK\ScopeCppSDK\vc15\SDK`<br>Ninja: `%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe`<br>CMake: `C:\Users\Reeve\AppData\Local\Microsoft\WinGet\Links\cmake.exe` |

---

## 4. Exact Build & Test Commands on Reeve's PC

> **IMPORTANT:** Do **not** run `build.bat` while you have uncommitted changes, because `build.bat` executes `git pull origin main` automatically and can merge/overwrite mid-edit. Run CMake + Ninja + MSVC directly with the block below.

### Build `noble-client.exe` (Release x64 — Must Be 0 Errors / 0 Warnings)
```powershell
Set-Location 'C:\Users\Reeve\Desktop\Halo Reach Community Edition'
Get-Process noble-client -ErrorAction SilentlyContinue | Where-Object { $_.Path -like 'C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\*' } | Stop-Process -Force
$msvcVer  = (Get-ChildItem "$env:LOCALAPPDATA\Programs\MSVC\VC\Tools\MSVC" -Directory | Select-Object -Last 1).Name
$msvcDir  = "$env:LOCALAPPDATA\Programs\MSVC\VC\Tools\MSVC\$msvcVer"
$sdkDir   = "C:\Program Files\Microsoft Visual Studio\2022\Community\SDK\ScopeCppSDK\vc15\SDK"
$ninjaDir = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
$env:PATH    = "$msvcDir\bin\Hostx64\x64;$sdkDir\bin;$ninjaDir;$env:PATH"
$env:INCLUDE = "$msvcDir\include;$sdkDir\include\ucrt;$sdkDir\include\um;$sdkDir\include\shared"
$env:LIB     = "$msvcDir\lib\x64;$sdkDir\lib"
cmake --build build --config Release
```

### Run Headless Unit Self-Tests (`MapGameTypes`, `MapNames`, `PadLayouts`, `InputTuning`)
```powershell
$p = Start-Process -FilePath 'C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\bin\Release\noble-client.exe' -ArgumentList '-test-map-gametypes' -PassThru -Wait
Write-Output "SelfTest ExitCode: $($p.ExitCode)"   # Must be 0
```

### Useful Runtime & Capture Flags
| Flag | Purpose |
|---|---|
| `-test-map-gametypes` | Headless self-test of map/gametype guards, map names, controller layouts, and input tuning (exits `0` with no window) |
| `-menu-screen <name> -menu-capture <frame>` | Open a menu screen (`main, campaign, skulls, firefight, custom, forge, servers, settings, armory, mods, career, lobby, social`), save `capture_<frame>.bmp`, and exit |
| `-map <map> -mode <mode> [-host] [-connect ip:port]` | Launch directly into a match (`campaign`, `firefight`, `multiplayer`, `forge`) |
| `-wait-peers N` | Host waits for `N` guests before sending Launch Commands 10/11 |
| `-data-dir <path>` | Override userdata directory (use when running 2 local instances for Host + Guest testing) |
| `-log <file.log>` | Write logs to a separate file inside `build\bin\Release\` instead of `noble-client.log` |
| `-scoreboard-at <sec>` / `-pause-at <sec>` / `-quit-after <sec>` | Automated in-game overlay capture and clean exit |
| `-third-person` / `-third-person-mode <0\|1>` | Start with third-person camera enabled (`F5` toggles in-game) |

### Backend (`O7OF/ServerTestDeploy`) Verification & Push
```powershell
$serverDir = 'C:\Users\Reeve\Documents\deepseek-harness\default-workspace\ServerTestDeploy'
node --check "$serverDir\server.js"
git -C $serverDir add server.js
git -C $serverDir commit -m "feat(server): <summary>"
git -C $serverDir push origin main
```

---

## 5. File Ownership & Division of Labor on Reeve's PC

You (`@deepseek`) and Gemini (`@gemini`) can claim any unclaimed `[ ]` item in `docs/TODO.md`, as long as the other agent has not claimed its files in `docs/AGENT-STATUS.md`. Default focus split:

| Agent | Handle in Docs | Primary Focus Areas | Typical Files |
|---|---|---|---|
| **DeepSeek (You)** | `@deepseek` / `(@deepseek)` | Engine hooks, audio (`exit_experience`, sound extraction), career/credits/commendations, camera/FOV/third-person, co-op launch blocks, UI/menus/overlays | `src/engine/*`, `src/audio/*`, `src/career/*`, `src/ui/*`, `src/host/*`, `tools/*`, `docs/*` |
| **Gemini 4 Argon** | `@gemini` / `(Gemini 4 Argon)` | Netcode, UDP/WebSocket relay, NAT hole-punching, session/lobby sync, social/friends/invites, backend `server.js`, reverse-engineering diagnostics | `src/network/*`, `src/dedicated/*`, `src/hooks/LobbyPatches.*`, `ServerTestDeploy/server.js`, `docs/*` |

**Golden Rule:** Never edit a file that `@gemini` (or `@claude`) currently lists under `files expected:` while `state: **active**` in `docs/AGENT-STATUS.md`.

---

## 6. Continuous-Mode Loop (Follow Every Session)

When running alongside `@gemini`, execute this loop continuously:

1. **Verify Clean Working Tree & Pull:**
   - Run `git status -sb`. If dirty with another agent's uncommitted changes, **STOP** and report what is dirty — do **not** commit, stash, or revert another agent's work.
   - Run `git pull --no-edit origin main`.
2. **Pick One Unclaimed Task:**
   - Check `docs/AGENT-STATUS.md` (active agents + claimed files) and `docs/TODO.md` (`[~]` in-progress items).
   - Pick **one** `[ ]` item from `docs/TODO.md` whose files are not claimed by an active agent.
3. **Claim Before Editing:**
   - In `docs/TODO.md`, change the item line to:
     `- [~] @deepseek YYYY-MM-DD HH:MM - <what you are doing>`
   - In `docs/AGENT-STATUS.md`, update `## @deepseek` (`state: **active**`, `since:`, `working on:`, `files expected:`).
   - Commit and push those two files immediately:
     `git add docs/TODO.md docs/AGENT-STATUS.md && git commit -m "docs(status): @deepseek claims <item>" && git push origin main`
4. **Implement & Bump Version:**
   - Implement the change cleanly.
   - **UTF-8 Rule on Windows PowerShell:** Never rewrite C++/JS files with `Get-Content` without `-Encoding UTF8` (it corrupts `·`, `§`, `─` into mojibake). Use `[System.IO.File]::ReadAllText($path, [System.Text.Encoding]::UTF8)`.
   - Bump `VERSION_PATCH` / `VERSION_STRING` in `src/common/Version.hpp` for every code commit.
5. **Build Gate (0 Errors / 0 Warnings) & Verify:**
   - Build `noble-client.exe` in `Release` x64 using the command in Section 4. Must finish with **0 errors and 0 warnings**.
   - Verify with evidence:
     - **UI changes:** Commit a screenshot under `docs/screenshots/`.
     - **Engine / Netcode changes:** Run `-test-map-gametypes` + in-game / two-process `-data-dir` test and record log lines.
     - **Server changes:** Run `node --check server.js`, test endpoints, and push to `O7OF/ServerTestDeploy`.
   - If you cannot verify at runtime, leave the item `[~]` in `docs/TODO.md` and note `needs:` in `docs/HANDOFF.md`.
6. **Write Handoff, Close Claim & Push:**
   - Prepend a new entry at the **top** of `docs/HANDOFF.md` (directly under the `--` lines):
     `## 0. Pick up here (YYYY-MM-DD, HH:MM ET) - <what changed> (@deepseek)`
     Explain what was wrong, what changed and where, how it was built and verified, and include a `Self-review:` line.
   - Update `docs/TODO.md` to `- [x] @deepseek YYYY-MM-DD HH:MM - <summary> - commit <hash>`.
   - Set `## @deepseek` in `docs/AGENT-STATUS.md` back to `state: **idle (last: <item>, YYYY-MM-DD HH:MM ET)**` and `last commit: <hash>`.
   - Pull (`git pull --no-edit origin main`) and push (`git push origin main`).
7. **Cross-Verify `@gemini`'s Latest Landed Commit:**
   - Inspect `@gemini`'s latest commit (`git show --stat <hash>`):
     1. Diff touches only the claimed scope.
     2. `docs/TODO.md` has the commit hash and `docs/HANDOFF.md` has the top entry.
     3. `cmake --build build --config Release` builds with **0 errors / 0 warnings** on the merged tree.
     4. Evidence (screenshots / log lines / self-tests) matches the claim.
   - Record `REVIEW OK (@deepseek, YYYY-MM-DD HH:MM ET)` (or `REVIEW: re-opened` with `file:line` if broken) in `docs/HANDOFF.md` and `docs/TODO.md`.
8. **Clean Up & Repeat:**
   - Ensure `noble-client.exe` is closed and `git status` is clean, then repeat from Step 1.

---

## 7. Hard Rules

- **NEVER force-push** (`git push --force` is forbidden), and never rewrite commits authored by `O7OF`, `@gemini`, `@claude`, or `@deepseek`.
- **Never edit a file another active agent has claimed** in `docs/AGENT-STATUS.md`.
- **Always update the top of `docs/HANDOFF.md`** and **bump `src/common/Version.hpp`** on every code push.
- **Server / netcode script changes belong in `O7OF/ServerTestDeploy`** (`C:\Users\Reeve\Documents\deepseek-harness\default-workspace\ServerTestDeploy`), never in the client repo.
- **Never commit** `build/`, `*.log`, `*.bmp` (convert captures to PNG in `docs/screenshots/`), `data/userdata/*.json` test files, or `database.db`.
- **Kill only Reeve's local `noble-client.exe`** before building, and leave `noble-client.exe` closed when done testing.
