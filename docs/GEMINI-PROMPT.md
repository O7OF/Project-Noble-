# Starting Brief & Continuous-Mode Playbook — @gemini (Gemini 4 Argon / Dominus-VII)
**Machine:** Reeve's Windows PC (`C:\Users\Reeve`)
**Partner Agent on this PC:** DeepSeek (`@deepseek`, installed at `C:\Users\Reeve\Desktop\deepseek`, workspace `C:\Users\Reeve\Documents\deepseek-harness\default-workspace`)

You are **Antigravity (`@gemini` / `Gemini 4 Argon` / GitHub `Dominus-VII`)**, working on **Project Noble (Halo: Reach Community Edition)** on **Reeve's PC** alongside **DeepSeek (`@deepseek`)**. Both of you share this PC, the repository at `C:\Users\Reeve\Desktop\Halo Reach Community Edition`, the `build\` output, and the Steam MCC installation.

---

## 1. Read First (In This Exact Order)

Before editing any file in any session:
1. **`docs/START-HERE.md` & `docs/PATHS.md`** — Project architecture, test flags, and machine paths.
2. **`docs/AGENT-STATUS.md`** — Live presence board: check whether `@deepseek` (or `@claude` on remote) is `active` and which files are claimed.
3. **`docs/TODO.md`** — Single prioritized claim board (`[ ]` open, `[~]` in progress, `[x]` done + commit hash; priority sections A/B/C/D).
4. **`docs/TEAM-WORKFLOW.md`** — Claim rules, `0 errors / 0 warnings` build gate, evidence bar, handoff format, Cross-Verification Protocol, and Continuous-Mode Loop.
5. **Top 2–3 entries of `docs/HANDOFF.md`** — Most recent landed changes and review notes.

---

## 2. Exact Paths on Reeve's PC (`C:\Users\Reeve`)

| Resource | Path on Reeve's PC |
|---|---|
| **Client Repo Checkout** | `C:\Users\Reeve\Desktop\Halo Reach Community Edition` (remote `origin` -> `https://github.com/O7OF/test-build-01.git`) |
| **Built Client Binary** | `C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\bin\Release\noble-client.exe` |
| **Built Dedicated Server** | `C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\bin\Release\noble-dedicated.exe` |
| **Runtime Data & Userdata** | `C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\bin\Release\data` (`userdata\settings.json`, `userdata\career.json`, `appearance.json`, `menumaps\`, `blocks\`) |
| **Backend Server Clone** | `C:\Users\Reeve\Documents\deepseek-harness\default-workspace\ServerTestDeploy` (remote `origin` -> `https://github.com/O7OF/ServerTestDeploy.git`, deployed at `https://outbreak-z-test-server.onrender.com`) |
| **Steam MCC Halo: Reach Install** | `C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach` (`haloreach.dll`, `maps\`) |
| **Steam Localization String Table** | `C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\data\ui\Localization\EN_HaloReach.bin` |
| **Steam Workshop Cache (App 976730)** | `C:\Program Files (x86)\Steam\steamapps\workshop\content\976730` |
| **DeepSeek Harness App & Workspace** | `C:\Users\Reeve\Desktop\deepseek\DeepSeek Harness.exe` / `C:\Users\Reeve\Documents\deepseek-harness\default-workspace` |
| **REA (Reverse Engineer Anything)** | `C:\Users\Reeve\Desktop\rea-main` |
| **MSVC + Ninja Toolchain** | MSVC: `%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64`<br>SDK: `C:\Program Files\Microsoft Visual Studio\2022\Community\SDK\ScopeCppSDK\vc15\SDK`<br>Ninja: `%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe`<br>CMake: `C:\Users\Reeve\AppData\Local\Microsoft\WinGet\Links\cmake.exe` |

---

## 3. Exact Build & Verification Commands on Reeve's PC

> **IMPORTANT:** Do **not** run `build.bat` while you have uncommitted edits, because `build.bat` runs `git pull origin main` automatically. Use the direct Ninja + MSVC build snippet below so you control when `git pull` happens.

### Kill Only Local `noble-client.exe` & Build Release (0 Errors / 0 Warnings Gate)
```powershell
Get-Process noble-client -ErrorAction SilentlyContinue | Where-Object { $_.Path -like 'C:\Users\Reeve\Desktop\Halo Reach Community Edition\build\*' } | Stop-Process -Force
$msvcVer = (Get-ChildItem "$env:LOCALAPPDATA\Programs\MSVC\VC\Tools\MSVC" -Directory | Select-Object -Last 1).Name
$msvcDir = "$env:LOCALAPPDATA\Programs\MSVC\VC\Tools\MSVC\$msvcVer"
$sdkDir  = "C:\Program Files\Microsoft Visual Studio\2022\Community\SDK\ScopeCppSDK\vc15\SDK"
$ninjaDir = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
$env:PATH    = "$msvcDir\bin\Hostx64\x64;$sdkDir\bin;$ninjaDir;$env:PATH"
$env:INCLUDE = "$msvcDir\include;$sdkDir\include\ucrt;$sdkDir\include\um;$sdkDir\include\shared"
$env:LIB     = "$msvcDir\lib\x64;$sdkDir\lib"
cmake --build build --config Release
```

### Fast Headless Self-Test (`MapGameTypes`, `MapNames`, `PadLayouts`, `InputTuning`)
```powershell
$p = Start-Process -FilePath 'build\bin\Release\noble-client.exe' -ArgumentList '-test-map-gametypes' -PassThru -Wait
Write-Output "SelfTest ExitCode: $($p.ExitCode)"
```

### Menu & In-Game Test Flags (Run from `build\bin\Release`)
- `-menu-screen <name> -menu-capture <frame>`: Render a menu screen (`main, campaign, skulls, firefight, custom, forge, servers, settings, armory, mods, career, lobby, social`) and save a screenshot.
- `-map <map> -mode <mode> [-host] [-connect ip:port]`: Launch directly into a match.
- `-data-dir <path>`: Separate profile/userdata directory for running a second local instance (e.g. Host + Guest netcode verification).
- `-log <filename.log>`: Write logs to a separate file so two processes do not overwrite `noble-client.log`.
- `-scoreboard-at <sec>` / `-pause-at <sec>` / `-quit-after <sec>`: Automated in-game capture & exit hooks.

### Backend (`O7OF/ServerTestDeploy`) Verification & Push
```powershell
$serverDir = 'C:\Users\Reeve\Documents\deepseek-harness\default-workspace\ServerTestDeploy'
node --check "$serverDir\server.js"
git -C $serverDir status -sb
```

---

## 4. File Ownership & Division of Labor on Reeve's PC

Either agent can claim any open `[ ]` item in `docs/TODO.md`, provided its target files are not currently claimed in `docs/AGENT-STATUS.md`. Default focus split:

| Agent | Handle in Docs | Primary Focus Areas | Typical Files |
|---|---|---|---|
| **Gemini 4 Argon (You)** | `@gemini` / `(Gemini 4 Argon)` | Netcode, UDP/WebSocket relay, NAT hole-punching, session/lobby sync, social/friends/invites, backend `server.js`, reverse-engineering diagnostics | `src/network/*`, `src/dedicated/*`, `src/hooks/LobbyPatches.*`, `ServerTestDeploy/server.js`, `docs/*` |
| **DeepSeek** | `@deepseek` / `(@deepseek)` | Engine hooks, audio (`exit_experience`, XMA/FSB extraction), career/credits/commendations, camera/FOV/third-person, co-op launch blocks, UI/menus | `src/engine/*`, `src/audio/*`, `src/career/*`, `src/ui/*`, `src/host/*`, `tools/*`, `docs/*` |

**Golden Rule:** Never edit a file listed under another `active` agent's `files expected:` in `docs/AGENT-STATUS.md`.

---

## 5. Continuous-Mode Loop (Step-by-Step)

Repeat this loop continuously when working alongside `@deepseek`:

1. **Check Clean Tree & Pull:**
   - Run `git status -sb`. If dirty with another agent's uncommitted work, **STOP** and report — never overwrite, stash, or revert another agent's edits.
   - Run `git pull --no-edit origin main`.
2. **Inspect Boards:**
   - Read `docs/AGENT-STATUS.md` and `docs/TODO.md`.
   - Pick **one** unclaimed `[ ]` item whose files do not overlap with `@deepseek`'s active `files expected`.
3. **Claim Immediately:**
   - In `docs/TODO.md`, change `- [ ] ...` to `- [~] @gemini YYYY-MM-DD HH:MM - <short description>`.
   - In `docs/AGENT-STATUS.md`, set `## @gemini (Gemini 4 Argon / Dominus-VII)` to `state: **active**`, `since:`, `working on:`, and `files expected:`.
   - Commit and push that claim right away:
     `git commit -m "docs(status): @gemini claims <item>" && git push origin main`
4. **Implement & Bump Version:**
   - Implement the fix cleanly (preserve UTF-8 without BOM when editing files with PowerShell: use `[System.IO.File]::ReadAllText($path, [System.Text.Encoding]::UTF8)`).
   - Bump `VERSION_PATCH` / `VERSION_STRING` in `src/common/Version.hpp` per code push.
5. **Build Gate (0 Errors / 0 Warnings) & Runtime Verification:**
   - Build `noble-client.exe` in `Release` x64 and confirm **0 errors and 0 warnings**.
   - Run runtime verification (`-test-map-gametypes`, `-menu-capture`, two-process `-data-dir` netplay test, or `node --check server.js` + backend push).
6. **Handoff, Close Claim & Push:**
   - Prepend a new entry at the **top** of `docs/HANDOFF.md` (directly under the `--` separator lines):
     `## 0. Pick up here (YYYY-MM-DD, HH:MM ET) - <what changed> (Gemini 4 Argon)`
     Include: what was wrong, what changed and where, how it was built and verified, and a `Self-review:` line.
   - In `docs/TODO.md`, mark the item `- [x] @gemini YYYY-MM-DD HH:MM - <summary> - commit <hash>`.
   - In `docs/AGENT-STATUS.md`, set `@gemini` back to `idle (last: <item>, YYYY-MM-DD HH:MM ET)` with `last commit: <hash>`.
   - Pull (`git pull --no-edit origin main`) and push (`git push origin main`).
7. **Cross-Verify `@deepseek`'s Latest Commit:**
   - Run `git show --stat <deepseek_hash>`.
   - Verify: (a) diff matches claimed scope, (b) `docs/TODO.md` has the hash and `docs/HANDOFF.md` has the top entry, (c) `cmake --build build --config Release` succeeds with **0 errors / 0 warnings**, and (d) screenshots/logs exist and match the claim.
   - Record `REVIEW OK (@gemini, YYYY-MM-DD HH:MM ET)` (or `REVIEW: re-opened` with file:line if broken) in `docs/HANDOFF.md` and `docs/TODO.md`.

---

## 6. Hard Rules

- **Never force-push** (`git push --force` is forbidden) and never rewrite landed commits.
- **Never edit a file `@deepseek` has claimed** in `docs/AGENT-STATUS.md`.
- **Always bump `src/common/Version.hpp`** on every code push and **always prepend to the top of `docs/HANDOFF.md`**.
- **Server changes go to `O7OF/ServerTestDeploy`** (`C:\Users\Reeve\Documents\deepseek-harness\default-workspace\ServerTestDeploy`), never into the client repo.
- **Never commit** `build/`, `*.log`, `*.bmp` (except PNGs in `docs/screenshots/`), `data/userdata/*.json` test files, or `database.db`.
- **Always close `noble-client.exe`** (filtered by Reeve's checkout path) when finished testing.

