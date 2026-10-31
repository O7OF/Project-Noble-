# START HERE — Project Noble

One page to get oriented. Read this, then `docs/TODO.md`, then the newest entries at the top of `docs/HANDOFF.md`.

## What this is

Project Noble is a community edition of Halo: Reach that runs the MCC engine (`haloreach.dll`) **without MCC's menu**:
our own C++20 client (`noble-client.exe`) hosts the engine's host interface, replaces the front-end with an ImGui menu,
and adds online play through our own master server + relay. Tyler (`WeabooTyler117`) tests; O7OF owns the repos and
writes a lot of the netcode.

- **Client repo**: `O7OF/test-build-01` — checkout at `D:\Halo-Reach-Community-Edition` (branch `main`)
- **Server repo**: `O7OF/ServerTestDeploy` — deployed at `https://outbreak-z-test-server.onrender.com`
  (rule: **any server/netcode script change goes there**, never the client repo)
- **Local backend clone**: `C:\Users\tyler\Documents\deepseek-harness\default-workspace\ServerTestDeploy`
  (`.\tools\server-repo.ps1` clones/pulls it)

**Heads at the time of writing:** client `97989d1`, backend `b8fb2ad`. Always `git pull` before starting.

## Build

```powershell
Get-Process noble-client -ErrorAction SilentlyContinue | Stop-Process -Force
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build --config Release -- -m
```

(One command per block, real path above. Never use `build.bat`. Build must be **0 errors / 0 warnings**.)

Run the client from `build\bin\Release`. Useful flags:

| flag | what it does |
|---|---|
| `-menu-screen <name> -menu-capture <frame>` | drive a menu screen and save a BMP (`main, campaign, skulls, firefight, custom, forge, servers, settings, armory, mods, career, lobby, social`) |
| `-map <map> -mode <mode> [-host] [-connect ip:port]` | launch a match directly |
| `-wait-peers N` | host waits for N guests before Command 10/11 |
| `-data-dir <path>` | separate profile/userdata (run multiple local instances) |
| `-scoreboard-test` / `-scoreboard-at <s>` | capture the scoreboard overlay |
| `-third-person` | start with the third-person camera on (F5 toggles in game) |
| `-capture-frames 900,1200` | save frames by number |
| `-log <file>` | write the log somewhere specific |

## Current state (2026-10-31)

Working: **campaign co-op (2-3 players)**, **Forge online**, **Firefight**, armory previews, scoreboard overlay,
third-person camera (F5), backend 1.3.0 (relay heartbeat + `/api/relay/stats`), lobby identity pass-through.

In progress / next, in priority order (`docs/TODO.md` has the full list):

1. **Third-person tuning** — distance + shoulder swap (the `haloreach+0x1DCADC` follow hook), settings row, host/mode gate
2. **Scoreboard overlay** — live K/D/score columns (needs the engine player-record hook)
3. **Campaign CONTINUE → last checkpoint** — the biggest campaign item
4. **Emblem backdrop + symbol picker**, **controller presets**, **readability pass**
5. **Housekeeping** — version bump, CI artifact, handoff dedupe

## Where everything is

| doc | what it is |
|---|---|
| `docs/TODO.md` | the single prioritized to-do list (checkboxes, owners, done markers) |
| `docs/HANDOFF.md` | chronological log, newest entry first — **the** history |
| `docs/HANDOFF-FOR-CLAUDE.md` | the original cold-start brief (traps, house rules, older backlog) |
| `docs/TEAM-WORKFLOW.md` | how Claude and DeepSeek work on this together without stepping on each other |
| `docs/PATHS.md` | **machine map** - every path on this PC that is not in the repo (MCC, HREK, RR install, Ghidra project, disassembly, SERVER clone, logs, userdata) |
| `docs/AGENT-STATUS.md` | **live presence board** - check it first: who is active and which files they are touching |
| `docs/CLAUDE-PROMPT.txt` | the exact first prompt/instructions for a Claude session on this PC |
| `docs/menu-redesign-spec.md` | the menu design contract (layout, typography, screens) |
| `docs/player-count-scaling.md` | how RR does >4-player co-op, staged plan |
| `docs/campaign-coop-timeline-2026-10-30.md` | the two-sided log timeline from the campaign co-op bug hunt |
| `docs/screenshots/` | committed capture evidence (scoreboard overlay, third person) |

## Hard rules

1. **Always update `docs/HANDOFF.md`** — a new `## 0. Pick up here (YYYY-MM-DD, HH:MM ET) - <what> (<name>)` entry at the
   top for every change, then commit + push.
2. **Server/netcode scripts go to `O7OF/ServerTestDeploy`**, never the client repo.
3. **Never commit `build/`** (gitignored). Kill `noble-client.exe` before building.
4. `data/appearance.json` is the live armory profile and the client rewrites it — back it up before touching ownership.
5. `settings.json` `playerName` has been clobbered to `Bob` by account-less runs — check identity after test runs.
6. Never force-push, never rewrite O7's commits, never commit a dirty `database.db`.
7. Leave the client closed when you finish testing.
