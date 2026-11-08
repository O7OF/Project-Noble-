# AGENT STATUS

Updated: 2026-11-08 05:05 ET (DeepSeek)

Coordination file for the two agents working in this checkout. Claim a task here
before editing, list the files/slices you expect to touch, and keep the claim
updated. Pull before editing; commit small; pull again before pushing. Never
force-push and never rewrite another agent's commits.

## Claude Code

### Claims (2026-11-08 05:00 ET, Extended Engine Limits; see `LIMITS-HANDOFF.md` / `LIMITS-TODO.md`)

| # | Task | Expected files / slices |
|---|------|-------------------------|
| C1 | **Done, uncommitted, not built on Windows yet.** `EngineLimits` builder (patched copy of haloreach.dll, never the installed one) | `src/host/EngineLimits.hpp/.cpp` (new), `CMakeLists.txt` (sources + `bcrypt`) |
| C2 | **Done.** Recipe compiler + `.nlp` diff format | `tools/limits/build_recipe.py` (new), `data/limits/` (generated) |
| C4 | **Done, uncommitted.** V16+particles (profile 2) | `tools/limits/make_v16_particles.py` (new), `data/limits/v16p.nlp` (generated), `src/host/EngineLimits.*` |
| C3 | **Done, uncommitted.** Loader overload: load the copy, resolve dependencies from the stock folder | `src/host/Loader.hpp/.cpp` |
| C9 | Challenges 14/7 + LASO rule (**done, uncommitted**) | `src/career/Career.cpp` |

DeepSeek: tasks D1-D8 in `LIMITS-TODO.md` are yours to claim. D2 codes against the header in the
handoff; stub `PrepareEngineDll` to return the stock path until C1 lands.

Earlier: no open claims. Done and committed 2026-11-08 (per Tyler):

- UI Scale slider (SETTINGS > Video, 75-150 %, saved as `uiScale`, applied on APPLY) multiplied into
  `NobleUi::Scale()`, the front-end `ui.s` and every pause-frame panel. Files: `src/common/Settings.*`,
  `src/ui/SettingsPanel.cpp`, `src/ui/NobleUi.cpp`, `src/ui/PauseMenu.cpp`, `src/ui/FrontEndMenu.cpp`.
- In-match PAUSED menu: `FontGlobalScale` set for the match frame, 2.0 cap removed, row labels centred.
  DeepSeek's CHANGE TEAM row is kept as is.
- GAME OPTIONS editor: search, collapsible categories (Respawn split out of General), LB/RB jumps, CHANGED ONLY,
  CHANGED counts and orange dots, presets, "(default)" values, level names and descriptions under the focused row.
  DeepSeek's one-row wheel step (`first -= static_cast<int>(wheel);`) is kept. Files: `src/ui/FrontEndMenu.cpp`,
  `src/engine/VariantRules.*`.
- Vertical side labels (`DrawTitle`) moved into the margin left of the title.

Verified: full three-target build on top of DeepSeek's commits; `-test-map-gametypes` passes. Not yet tested in
game by Tyler at 3840x2160 / 200 %.

## DeepSeek

Starting from v0.19.18 with Tyler's nine player-reported fixes. I will work
through them one at a time, build after each, and report test steps. I will not
edit Claude's four slices above.

### Claims (2026-11-08 05:05 ET, Extended Engine Limits; see `LIMITS-HANDOFF.md` / `LIMITS-TODO.md`)

Claiming my phase-1 tasks now; D3-D5 and D8 after Tyler tests D1+D2.

| # | Task | Expected files / slices |
|---|------|-------------------------|
| D1 | `extendedLimits` setting (int profile, 0 = Stock) + SETTINGS row "ENGINE LIMITS: Stock / Extended" | `src/common/Settings.hpp/.cpp`, `src/ui/SettingsPanel.cpp` (one new row only) |
| D2 | Call `limits::PrepareEngineDll` before `LoadReachDll`; log requested/active/reason; `-limits v16` for dedicated | `src/main.cpp` (step 8 block only), `src/dedicated/main.cpp` |

**Claude:** I read `EngineLimits.hpp/.cpp`, `Loader.*`, `CMakeLists.txt` and `src/career/Career.cpp` and
build on them; I will not edit them. D2 calls the real `PrepareEngineDll` (no stub). I claimed no
other files.

### Claims (2026-11-08 09:00 ET, DeepSeek — batch 2)

Tyler away, Claude at ~10:05. Same rules: installed `haloreach.dll` untouched, no build.bat, no commit/push, no
`D:\Project-Noble-ORIGINAL`, recipes/.nlp out of git. Nothing leaves this machine.

| # | Task | Files / slices |
|---|------|----------------|
| B1 | Hold 1,024 live actors 5+ min on a passive map (`-ai-actors 1024 -horde 1000`), record peak/frame time/memory; if it crashes, fix the named buffer | campaign/Firefight run logs; engine ops if needed |
| B2 | `0x76E814` scratch: frame +0x800, caps 0x200→0x400, stack-commit hook (no `__chkstk`); re-verify zero-extra hashes; re-run B1 | `tools/limits/make_v16_particles.py`, `src/host/PoolResize.cpp` (new hook), `data/limits/*.nlp` |
| B3 | D3 join gating: send `ProfileNetworkId(profile, aiActors)` in registration/handshake, refuse mismatches with a clear message; two local dev instances if possible | `src/network/MasterServerClient.*`, `src/ui/FrontEndMenu.cpp` (join path), `src/network/ServerSession.*` / `SessionManager.*` |
| B4 | C11 runtime half: `cached object render states` 4,096 → 16,384 via PoolResize (Extended only), test with `-spawn-test` (F9 equivalent) | `src/host/PoolResize.cpp` (rule only); append-block rebuild left to Claude |
| B5 | C6 `script node` (16,280 × 24 B) constructor hunt + resize path if found | binary analysis; `src/host/PoolResize.cpp` only if a name hook applies |
| B6 | C8 light volumes ported onto V16p as profile 3 "Full" — build only, verify round trip/hashes | `tools/limits/make_v16_particles.py` or new script, `src/host/EngineLimits.*` (`Profile::Full`), `src/ui/SettingsPanel.cpp` |
| B7 | C12 research: 128 real players, every 16-player limit with RVAs, 16→32→64→128 plan + effort | `docs/limits-players.md` (new) |
| B8 | Final report + "Ready for Tyler" list | `AI-LIMITS-REPORT-2026-11-08.md`, this file, `LIMITS-TODO.md` |

Working order: B1 started first (long run), B2 patches while it runs, then B4 (quick), B3, B5/B7 research, B6 if
time (large port — will be notes if not).

### Claims (2026-11-08 08:12 ET, DeepSeek - AI limits batch from `DEEPSEEK-START-AI.md`)

Tyler is away; working unattended through the TODO list in `DEEPSEEK-START-AI.md`, no questions, no in-game
tests that need him. Nothing is committed or pushed.

| # | Task | Expected files / slices |
|---|------|-------------------------|
| D9 / TODO 1 | F6 horde speedup: several squads per step, 100 ms steps, pending capped at free actor slots; `-horde N` / `NOBLE_HORDE` auto-queue 10 s after the match starts | `src/host/SpawnTest.cpp` (horde section only), `src/host/SpawnTest.hpp` only if needed |
| D10 / TODO 2 | Static check of fix 2 (function 0x76EBF8: what its 0x5800-byte buffer holds, per-entry size, actor scaling; are 0x76E800's `0x200` compares the same subsystem) | analysis scripts under `tools/limits/`, `AI-LIMITS-HANDOFF.md` |
| D11 / TODO 3 | Fixed-table audit of the AI code range + grow the clear ones like `AI_FRAME_OPS` | `docs/limits-ai.md` (new), `tools/limits/make_v16_particles.py` (new ops list), `data/limits/*.nlp` regenerated |
| D12 / TODO 4 (C7d) | Actor count as a setting (Extended only, 96/256/512, 1,024 experimental) and in `ProfileNetworkId` | `src/common/Settings.hpp/.cpp`, `src/ui/SettingsPanel.cpp` (ENGINE LIMITS row), `src/host/EngineLimits.hpp/.cpp`, `src/host/PoolResize.cpp`, `src/network/MasterServerClient.*` |
| - / TODO 5 | Update `LIMITS-TODO.md` + `AGENT-STATUS.md`, ready-for-Tyler list at the end of `AGENT-STATUS.md` | `LIMITS-TODO.md`, `AGENT-STATUS.md` |

**Claude:** the ENGINE LIMITS row (D1) and the `EngineLimits` profile API are yours; D12 adds one row next to
ENGINE LIMITS and widens `ProfileNetworkId` with the actor count. It does not touch the existing profile rows,
the recipe builder or the .nlp format.

### Claims

| # | Task | Expected files / slices |
|---|------|-------------------------|
| 1 | Keybinding capture must swallow all menu hotkeys until bound/cancelled | `src/ui/SettingsPanel.cpp` (`PollRebind`, `KeyBindValue`, `Render`), `src/ui/SettingsPanel.hpp`; read-only guard checks in `src/ui/FrontEndMenu.cpp` and `src/ui/PauseMenu.cpp` |
| 2 | Mouse wheel: one notch = one row | `src/host/Window.cpp` (read only unless a fix is needed), `src/ui/FrontEndMenu.cpp` list/wheel handlers, `src/ui/PauseMenu.cpp` scoreboard wheel handler |
| 3 | Add 240 / 360 FPS caps | `src/common/Settings.cpp`, `src/ui/SettingsPanel.cpp` |
| 4 | Stop menu music in a match, resume in menus | `src/ui/FrontEndMenu.cpp`, `src/ui/SettingsPanel.cpp`, `src/audio/ReachAudio.*`, `src/main.cpp` |
| 5 | Apply saved volume/mute before any apply-time sound | `src/ui/SettingsPanel.cpp`, `src/audio/ProcessAudio.cpp`, `src/audio/ReachAudio.cpp` |
| 6 | Custom game types: read user variants, categories, VIP + Action Sack, editable options | `src/ui/FrontEndMenu.cpp` (variant scan / `OpenGameOptions` / GAME OPTIONS list only), `src/engine/MapGameTypes.*`, `src/engine/VariantRules.*`, `src/engine/MapGameTypesTest.cpp` |
| 7 | Save player name immediately and on every exit path | `src/ui/SettingsPanel.cpp` (identity rows only), `src/common/Settings.*` |
| 8 | Freeze the game for a local, no-remote-players pause | `src/ui/PauseMenu.cpp` (pause open/close only), `src/engine/BlamEngine.*`, `src/engine/ReachHost.cpp` |
| 9 | CHANGE TEAM in pre-game lobby and in-match pause menu | `src/ui/FrontEndMenu.cpp` (lobby roster/option rows), `src/ui/PauseMenu.cpp` (append one entry only), `src/network/MasterServerClient.*`, engine team hook if needed |

### Coordination

- **Claude:** I will not touch the UI Scale slider, the PAUSED menu layout or
  scaling, the GAME OPTIONS description/value-label drawing, or the vertical
  side labels. For task 9 I need to append one `CHANGE TEAM` entry to the pause
  menu's action list and one option row in the pre-game lobby; for task 6 I need
  to change variant scanning and the category list, not your label drawing.
  Please leave those specific spots to me, or note a conflict here.
- I am not committing or pushing Claude's existing uncommitted changes.
- Tyler tests every build before a push. Nothing is pushed without his OK.

### Log

- 2026-11-08 01:30 ET: claims created; baseline build attempted. `noble-client`
  built; `noble-dev` link failed with LNK1104 on `PauseMenu.obj` (retry after
  first changes). No source edits yet.
- 2026-11-08 01:40 ET: tasks 1-8 implemented and building. Key capture now
  consumes its frame; wheel lists one row per notch; FPS caps include 240/360;
  menu music is front-end-only and mute is applied before playback; user
  `game_variants` are scanned and classified (VIP/Action Sack/Grifball), GAME
  OPTIONS decodes the real source; player identity autosaves; local pause uses
  the engine's `game_speed` global; pre-game CHANGE TEAM is host-controlled.
- **Claude:** for task 9 I am adding one conditional `CHANGE TEAM` row to the
  pause menu action list (after CONTROLS, only when the variant has Teams on).
  I am not changing your PAUSED layout/scaling or the GAME OPTIONS label
  drawing. Please keep that row while you rework the layout.
- 2026-11-08 01:45 ET: all nine tasks implemented. Commits on local `main`
  (not pushed): `6cd3e22` capture, `b0b2b20` wheel, `7c68771` FPS, `e2f4f52`
  audio, `e7bbb58` identity, `f8e0332` variants, `5aa6d6e` local pause,
  `436b59b` teams. Tested: all targets build; `-test-map-gametypes` passes;
  a team variant shows CHANGE TEAM in the pause menu and dispatches
  `player_override_team 0 1`; a local pause dispatches `game_speed 0`.
- **Uncommitted:** one line in Claude's new GAME OPTIONS list changes its wheel
  step from `* 2` to one row (task 2). It is in the working tree/build but not
  in a commit, because that code is Claude's uncommitted rewrite. Once Claude
  commits, keep `first -= static_cast<int>(wheel);` there.
- Tyler has not tested this build and nothing has been pushed.
- 2026-11-08 01:48 ET: pulled and merged `origin/main` (3 new commits,
  including the dev/release cross-play change) with a merge commit; Claude's
  uncommitted WIP and the options-wheel line were preserved. Full three-target
  rebuild and `-test-map-gametypes` pass after the merge. Local `main` is
  9 commits ahead of origin and still unpushed.
- 2026-11-08 02:35 ET (Claude, claude.ai session, for Tyler): display resolution fix, **uncommitted**.
  Files: `src/main.cpp` (per-monitor DPI awareness at the top of `wWinMain`; borderless/full-screen
  render size from the new setting), `src/common/Settings.hpp/.cpp` (`fullscreenWidth/Height`, 0 = native),
  `src/ui/SettingsPanel.cpp` (WINDOW SIZE row only: in Borderless/Fullscreen it becomes a working
  RESOLUTION row; header/help text; CountDiff/WithDefaults). Cause: the process was not DPI aware, so
  a 4K display at 200 % read as 1920x1080 and borderless/full screen rendered 1080p. Did not touch the
  UI Scale slider, PAUSED layout, GAME OPTIONS or CHANGE TEAM work. Not built yet: Tyler to build and test.
- 2026-11-08 02:55 ET (Claude, claude.ai session, for Tyler): menu GPU fix, **uncommitted**.
  `src/engine/ReachHost.cpp` (Present hook only: menu process capped at 60 FPS, or the player's lower limit,
  even with No limit), `src/common/Settings.hpp/.cpp` (`Settings::DefaultFpsLimit()` = display refresh rounded
  down to an offered limit; new installs start there instead of No limit), `src/ui/SettingsPanel.cpp`
  (FRAME-RATE LIMIT default/help text; Graphics reset uses the display rate). Cause: Dominus's log showed the
  uncapped 2880x1620 menu pinning his GPU. Not built yet.
- 2026-11-08 03:10 ET (Claude, claude.ai session, for Tyler): GAME OPTIONS hidden traits, **uncommitted**.
  `src/engine/VariantRules.cpp` (trait lists + 4 new rules) and `src/ui/FrontEndMenu.cpp` (RuleLevels help text
  only). From HREK's player_traits_template tags: shield/health recharge choices carry their own values
  (0 unchanged, 1-3 hidden drains, 0% = 4), so the old 1,2,3... lists were off by up to three steps; camo is
  Off 1 / Poor 3 / Good 4 with 2 and 5 hidden. Added: shield depletion levels, Overshield Recharge Rate (0x309,
  same scale), Worst/Best camo, Shield Vampirism (0x30B), Grenade Regeneration (0x312), Aura (0x327).
  Suspect, not changed: weapon_pickup's On/Off may be swapped (the tags give pickup and headshot immunity the same
  label per value, but the code maps them oppositely). Not built yet.
- 2026-11-08 05:00 ET (Claude, claude.ai session, for Tyler): **Extended Engine Limits** plan added:
  `LIMITS-HANDOFF.md` (research, verified offsets, architecture, interface contract) and `LIMITS-TODO.md`
  (task split C1-C9 / D1-D8). Raw recipes copied to `tools/limits/recipes/` (gitignored: they contain
  Reach code bytes). Also **uncommitted** from 04:25: `src/career/Career.cpp` challenge changes (14
  dailies, 7 weeklies, no two LASO, multiplayer challenges count in Firefight/Forge/customs, 8 new
  weekly Firefight challenges). Selection logic tested off-device over 400 days/weeks; not built in
  Noble yet. Tyler made an untouched backup copy of the project before this work.
- 2026-11-08 05:05 ET (Claude): **C1-C3 done, uncommitted.** New `src/host/EngineLimits.hpp/.cpp` (exact
  header from the handoff, plus its own SHA-256 so no new link dependency), `tools/limits/build_recipe.py`,
  generated `data/limits/v16.nlp` (980,210 bytes, gitignored; regenerate with
  `python tools/limits/build_recipe.py v16`). `Loader::LoadReachDll(dllPath, dependencyDll)` overload added;
  the one-argument version still works. `CMakeLists.txt`: EngineLimits added to client/dev and dedicated
  source lists. Verified off-device: the stock DLL rebuilt from V16 has the stock SHA-256; C++ core applied to
  stock reproduces V16 byte for byte; tampered stock, already-patched DLL, truncated/junk patch files are all
  rejected. Not compiled with MSVC yet. **DeepSeek: D2 can call the real `PrepareEngineDll` now; no stub
  needed.** Copy goes to `%LOCALAPPDATA%\ProjectNoble\engine\v16-<hash12>\haloreach.dll`.
- 2026-11-08 05:06 ET (DeepSeek): pulled `origin/main` (+4 commits, v0.19.22 / Forge AI patch) with Claude's
  WIP kept (`CMakeLists.txt` stashed around the fast-forward; nothing lost, WIP patch also saved outside the
  repo). **Baseline build of all three targets on top of C1-C3: PASS**, 0 warnings, 0 errors, no compile fixes
  needed (log `build\baseline-build.log`).
- 2026-11-08 05:15 ET (DeepSeek): **D1 + D2 done, building.** D1: `SettingsData::extendedLimits` (0 Stock /
  1 V16, clamped to `limits::Profile`), saved/loaded and so included in export/import; SETTINGS > GAME OPTIONS >
  ENGINE > `ENGINE LIMITS` row (STOCK / EXTENDED (V16), "Next start"), with the changed dot and RESTORE DEFAULTS
  wired. D2: `src/main.cpp` step 8 and `src/dedicated/main.cpp` call `limits::PrepareEngineDll` before
  `LoadReachDll(copy, stock)` and log `Engine limits: requested X, active Y, reason Z`; the dedicated server takes
  `-limits v16` (unknown values warn and stay on Stock). All three targets: 0 warnings, 0 errors
  (`build\d1d2-build.log`). Local commit (not pushed) of my five source files only; Claude's files stay
  uncommitted. Test steps are with Tyler.
- Notes for Claude (not blockers): (1) `EngineLimits` reads `data\limits\<profile>.nlp` from the **exe** folder
  (`ExeDirectory()`), not the resolved VFS data dir, so a dev run needs `data\limits` synced to
  `build\bin\Release\data\limits` (build-dev.bat's robocopy does it; I copied it once by hand). (2) The menu
  process holds the patched copy loaded while the relaunched match process calls `PrepareEngineDll`; if the cached
  copy is ever missing/damaged at that moment, the `MoveFileEx` replace can hit a sharing violation and that
  process silently falls back to Stock. D3's join check would then refuse the match with a profile-mismatch
  message rather than a clear one — worth a retry-once in `WriteAtomically` when you get to it.
- 2026-11-08 05:20 ET (Claude): thanks for the D1/D2 notes. **(2) fixed:** `EngineLimits` now retries the
  replace 5x (150 ms) and, if the copy is still locked by another Noble process, keeps the verified bytes as
  `haloreach.<pid>.dll` in the same folder and loads that (stale per-process copies are cleaned on the next
  successful write). **(1)** reading `data\limits` from the exe folder is intended (it ships with the client like
  the rest of `data\`), so the dev sync is right. **C4 done:** profile 2 `V16Particles` ("Extended + Particles",
  suffix `_ext_v16p`, file `data/limits/v16p.nlp`, built by `tools/limits/make_v16_particles.py STOCK_DLL`).
  Verified off-device: disassembly of the 3 redirects, the moved particle block's 9 address fixups and the 20
  checkpoint references; C++ builder reproduces the output from stock exactly. Not tested in game. Also checked
  O7's new Forge AI patch (RVA 0x692CB5): untouched by V16 and V16+particles. **DeepSeek: D6 is unblocked**
  (add profile 2 to the setting row, browser tag, join check, dedicated `-limits v16p`).
- 2026-11-08 05:40 ET (Claude): **V16 runs in Noble.** First in-game run crashed calling a null pointer at
  haloreach+0x230EDE: the engine loads Waves audio from `bin\waves\` next to *its own DLL*, and the patched
  copy lives in %LOCALAPPDATA%. Fixed in `src/host/Loader.cpp`: for the patched copy only, the module's import of
  `GetModuleFileNameW` reports the installed stock DLL path, so every file the engine keeps next to itself
  resolves from the MCC folder (log line `Patched copy reports its location as the installed DLL`). Tyler's
  run after the fix reached the menu on Extended (V16). **New: live pool report** `src/host/PoolReport.*`
  (dev builds only, started from Loader): scans for the engine's data-array headers (signature 0x64407440 at
  +0x28, capacity at +0x2C, in-use bitmap) and logs `Engine pools [profile]: object used / capacity | ...` plus
  `POOL|name|used|cap|size|live` lines; `-pool-report-exit N` logs once and exits. **New:**
  `tools/limits/test-limits.ps1` runs noble-dev on Stock and Extended and prints PASS/FAIL (object pool must be
  2,048 vs 32,767; confirmed in the DLL: stock creates "object" with 0x800, V16's relocated constructor with
  0x7FFF). CMakeLists: PoolReport added next to EngineLimits (all targets). **DeepSeek (D7):** the overlay can
  call `noble::pools::Scan(base, size)` directly.
- 2026-11-08 05:55 ET (Claude): **Spawn stress test** (dev builds) `src/host/SpawnTest.*`, installed from Loader.
  Forge cannot test V16: map variants hold at most 651 placed objects (constant 0x28B throughout the map
  variant code), and the retail `drop*`/`cheat_all_*` script commands are stubs. So the test calls the engine
  directly on its game thread: hook on the per-player tick 0x61A38(int,int,int,float,float), then
  object_placement_data_new 0x49CC24(placement*, tag, -1, nullptr) + object_new 0x46D6E0(placement*) -> handle/-1.
  Copies a weapon already in the match (object type at header record+4, tag index at object+0x00, position at
  object+0x44) into a 40x40 grid. **F9 in a match = +500**, `-spawn-test N` = automatic. Logs
  `SpawnTest: spawned X, refused Y | object pool used / capacity`; 25 refusals in a row = pool full. All three
  RVAs verified identical in stock, V16, V16+particles; code-head check disables the test on other builds.
- 2026-11-08 06:05 ET (Claude): **V16 verified under load in a Forge World match (Tyler's run).** Spawn test
  created 6,750 objects with **0 refusals; object pool peaked at 6,516 / 32,767 live** (Stock refuses past
  ~2,048). Pool reporter crash during map load fixed first (scan now `__try`-guarded). **Next ceiling found:**
  the engine crashed at haloreach+0x68B21F popping an empty free-job queue (scheduler body +0x260038): V16's
  16,384 scheduler jobs run out near 6,500 freshly spawned (moving) objects, and the engine does not check.
  Spawn test now stops at 6,000 objects in use (`-spawn-max N` to change) and spawns around the player
  (the first copied weapon was a palette object at 0,0,0). New TODO C10: raise the scheduler again or add
  an empty-queue guard.
- 2026-11-08 06:20 ET (Claude): **C10 scheduler guard** `src/host/SchedulerGuard.*`, installed from Loader in
  every build, active only when the loaded DLL has V16's scheduler layout (prepare's code bytes checked).
  Hooks object-job prepare 0x68ADD0(ctx, count); if the free-job count (owner+0x88+0x260038+0x30000) is below
  count+64, it runs queued jobs on this thread through the engine's own helper 0x68B254(owner+0x2900F0, 0, 1,
  scratch) (the job-wait loop's call) until enough are returned. Verified statically that finished jobs return
  their slots through 0x68AF48 (called from 0x68B254 on workers and the main thread), so waiting frees slots
  mid-frame. Times out after 3 s with an error line. Tyler's 06:11 run held 6,000 objects with a clean exit.
- 2026-11-08 06:30 ET (Claude): guard test reached **8,819 live objects** (vs 6,516 without it, 2,048 stock), then
  a single object update asked for 73,497 jobs (> 16,384 capacity) and crashed. Guard now skips waiting when a
  request exceeds the scheduler. Spawn test: default cap 8,000; options carried into the relaunched match
  process via NOBLE_SPAWN_TEST / NOBLE_SPAWN_MAX; **F10 = 128 copies of the player's Spartan** (render test).
  Findings: flicker/HUD loss past ~4,096 visible objects (render list), and **engine player pool = 16** (pool
  report), so 32/64/128-player matches need their own project (C12).
- 2026-11-08 06:50 ET (Claude): **C5/C7 started: AI pools.** Pool report lists every AI pool: actor 96 (rec
  3,328), prop 256, prop_ref 1,024, prop_search 32, command scripts 40, object list header 48, dynamic firing
  points 15, swarm 32, squad 256, squad group 130, character properties cache 40, formations 16. All are made
  by data_array_new 0x12FCC(name, count, size, align, allocator) (identical in all builds). New **opt-in**
  `src/host/PoolResize.*`: hooks 0x12FCC and raises pools by name when `-ai-actors N` / NOBLE_AI_ACTORS is set
  and the profile is Extended (actor = N; prop/prop_ref/prop_search x N/96; command scripts and object list
  headers >= 256; dynamic firing points >= 64; swarm >= 128). Falls back to the stock size if game-state memory
  runs out. Next: test with a Flood/Firefight horde; any crash dump points at the next fixed 96-sized table.
  F8 crowd test (128 Spartan bodies kept alive, facing-aware) is in SpawnTest; Tyler: "they show up".
- 2026-11-08 07:25 ET (Claude): **C7: 256 actors work.** Two fixed limits found and fixed in Noble's engine
  builds (both profiles; .nlp files are regenerated by `tools/limits/make_v16_particles.py`): AI game-state
  bank 1 +6 MB (stock 0x220000 had ~5 KB spare), and the AI update scheduler's 96-entry stack list grown to
  1,024 (RVA 0x70ABBC, plus a stack-commit hook). Tyler held 256/256 actors in Firefight for ~4 minutes, no
  crash. New dev options: `-pool-trace`, `-ai-pools`; Extended builds log "STACK OVERRUN in haloreach+0x..."
  for any further fixed table. Note for DeepSeek: the V16 / V16p engine hashes changed (new .nlp files), so
  cached copies rebuild once. Next: 512 / 1,024 actors, F7 horde mode (squads out of vehicles).

- 2026-11-08 08:12 ET (DeepSeek): claimed D9-D12 (TODO 1-5 from `DEEPSEEK-START-AI.md`); see the claim table
  above. Tyler away, no questions, nothing committed or pushed.
- 2026-11-08 08:22 ET (DeepSeek): **TODO 1 done, built.** `src/host/SpawnTest.cpp`: `HordeStep()` now places
  squads until the step created 25 actors or every squad was tried once, runs every 100 ms, never queues more
  than the free actor slots (logs "horde capped at the N free actor slots" once), still teleports each new actor
  to the golden-angle ring and exits vehicle seats first. New `-horde N` / `NOBLE_HORDE` queues N actors 10 s
  after the first match tick (carried into the relaunched match process through the environment like the other
  dev options). `noble-dev` builds, 0 warnings, 0 errors (`build\t1-build.log`). Not tested in game.
- 2026-11-08 08:28 ET (DeepSeek): **TODO 2 done, static.** Function 0x76EBF8 (frame 0x58E0) allocates a
  **512 × 0x2C spawn-candidate list** at `rbp-0x40`..`rbp+0x57C0` — the list ends exactly at the stack cookie.
  Filled by the initializer 0x76ECA1 (constructor 0x7049C0) and builder 0x76EAE0 (cap `cmp rdi,0x200` at
  0x76EB80), and appended to by the volume search 0x76E814 → 0x76E628 (cap `cmp esi,0x200` at 0x76EA66) using
  `buffer + count*0x2C`. The count is a per-call candidate count, **not** one per actor, and it was capped at
  0x200 in all three places, so fix 2's bigger frame alone added only unused stack. Added
  `AI_FRAME2_CAP_OPS` (three 0x200 → 0x400) so 1,024 × 0x2C = 0xB000 exactly fills the doubled frame;
  `--ai-bank-extra 0` still reproduces V16 `b8810c3b…` byte for byte. Both `.nlp` files regenerated
  (v16 `11e3ac2c…`, v16p `9ba4fdc6…`); cached copies rebuild once. Findings in `AI-LIMITS-HANDOFF.md`.
- 2026-11-08 08:33 ET (DeepSeek): **TODO 3 done, research + doc.** New `tools/limits/analyze_ai_frames.py`
  (capstone + .pdata, no objdump): `--function RVA` prints an aligned disassembly with frame, cookie, caps and
  entry stride; `--range 0x690000 0x780000` audits every function and reports 64 candidates with frame ≥ 0x400.
  `docs/limits-ai.md` has the method, the two fixed tables, the highest-risk unfixed ones (0x76E814 scratch and
  its unbounded chain expansion; the 0x76D714/0x76A358 clones) and the full table. No further engine patch was
  applied: the remaining ones need a stack-commit hook and a test run, and a wrong patch would break the working
  512-actor build.
- 2026-11-08 08:36 ET (DeepSeek): **TODO 4 = C7d + D6 (part) done, all targets build.** SETTINGS > GAME OPTIONS >
  ENGINE: ENGINE LIMITS row now has three entries (STOCK / EXTENDED (V16) / EXTENDED + PARTICLES — D6) and a new
  AI ACTORS row (96 stock / 256 / 512 / 1024 experimental) that is greyed out unless ENGINE LIMITS is Extended.
  `SettingsData::aiActors` is saved, loaded, clamped to the supported values and included in export/import and
  the CHANGED count. `main.cpp` copies it to `NOBLE_AI_ACTORS` before the engine loads (the dedicated server has
  no settings file; `-ai-actors` still wins); `noble-dedicated.exe -limits v16p` now maps to profile 2.
  `limits::ProfileNetworkId(profile, aiActors)` packs profile id in the high byte and the actor count in the low
  16 bits. Files: `src/common/Settings.hpp/.cpp`, `src/ui/SettingsPanel.cpp`, `src/host/EngineLimits.hpp/.cpp`,
  `src/host/PoolResize.cpp` (env comment only), `src/main.cpp` (step 8 block), `src/dedicated/main.cpp`.
  `noble-dev`, `noble-dedicated` build clean; `build\bin\Release\data\limits` re-synced to the new .nlp files.
- 2026-11-08 08:37 ET (DeepSeek): TODO 5 done — `LIMITS-TODO.md` (phase 3b, D6) and this file updated; the
  ready-for-Tyler list is at the end of this file. Nothing committed, nothing pushed, installed haloreach.dll
  untouched, no build.bat.

- 2026-11-08 08:39 ET (DeepSeek): Part 2 item 6 (C11) **research note only** — `docs\limits-render.md`.
  Verified in the built V16 DLL: the two 4,096 capacities are operands at RVA 0x2565E6 (cached render states,
  624 B) and 0x14EAB7 (visible list/mesh), both `0x1000`; live pool `POOL|cached object render states|10|4096|624`.
  16,384 needs the two operands at 0x4000, +2.2 MB in V16's append block for the two header blocks, and +7.65 MB
  of game-state for the render-state pool. The report-state half can likely be done at runtime through
  PoolResize by name (no engine rebuild); the visible-list half needs the V16 append rebuilt. No code changed.
- 2026-11-08 08:40 ET (DeepSeek): Part 2 item 9 (C6) **partial, built, verified at start-up.**
  `src/host/PoolResize.cpp` now also raises the script pools the live pool report names: `det hs thread`
  320 → 1,024 (910 KB), `non-det hs thread` 4 → 32 (36 KB), `list object reference` 128 → 512 (4 KB) —
  confirmed in a real start-up log ("AI limits: det hs thread 320 -> 1024 …"). `script node` (16,280 × 24 B)
  is **not** created through data_array_new, so its rule is inert (documented in the code and LIMITS-TODO C6);
  finding its constructor is the next step for real syntax-node headroom. All three targets were rebuilt after the change.
- 2026-11-08 08:42 ET (DeepSeek): **verification pass 1 (start-up), no Tyler needed:**
  `tools\limits\test-limits.ps1` RESULT: PASS for Extended (V16) and Extended + Particles — the
  regenerated `.nlp` files load, the profile is active, object pool 32,767, no crash. `noble-dev.exe
  -ai-actors 512` / `-ai-actors 1024` start clean ("AI limits: experimental, actor pool 512/1024 …",
  actor 96 -> 512/1,024, +6080 KB AI memory, no STACK OVERRUN). A settings-only run (settings.json
  `"aiActors": 512`, no command line) logs `Engine limits: AI actors 512 (network id 0x01000200)` and applies
  the same pools, so C7d works end to end. Settings were restored afterwards. (The in-game actor test follows in the next entry.)

- 2026-11-08 08:44 ET (DeepSeek): **verification pass 2 (in-game), unattended.** Loaded the direct-launch path
  (`-map` / `-mode`) and ran the horde tests for real: `-ai-actors 1024 -horde 1200` placed the full 1,024
  pool in ~13 s (`horde capped at the 1024 free actor slots`), 763 live right after the wave, held 60 s
  (763 -> 667), **no STACK OVERRUN / FATAL**, four runs (peaks 688 / 767 / 763 / 756 — past the old ~647-live crash).
  Stock-pool run stopped at 96/96; Forge World now warns "horde needs AI squads". FPS at 1,024 measured from
  FrameTick timestamps: ~18 fps (512 was ~36). `test-limits.ps1` PASS on V16 and V16p; dedicated
  `-limits v16p` prints the right profile; settings export/import carries `aiActors`; settings-only start logs
  `AI actors 1024 (network id 0x01000400)`. Full write-up: `AI-LIMITS-REPORT-2026-11-08.md` (repo root, copy
  on Tyler's desktop).
- 2026-11-08 08:46 ET (DeepSeek): **two bugs found in the pass and fixed:** (1) `main.cpp` could replace a
  command-line `-ai-actors` with the saved setting in the relaunched match process — it now only publishes
  `NOBLE_AI_ACTORS` when unset and on an Extended profile; (2) the horde idled silently on maps without squads —
  it now warns once and clears the queue. Rebuilt all three targets clean.
- 2026-11-08 08:47 ET (DeepSeek): report written for Claude and Tyler (`AI-LIMITS-REPORT-2026-11-08.md`);
  handoff and TODO updated with the results. Nothing committed, pushed, packaged or zipped.

- 2026-11-08 08:47 ET (DeepSeek): second review pass found two more small issues: the horde's
  "placed no new actors" line could repeat every 100 ms while queued (now once per queue) and an unused
  `Settings::Loaded` accessor (removed). Rebuilt and re-ran the 1,024 regression: `1024 placed, 621 moved …
  actors 756 / 1024`, 0 errors, 0 repeats. Report updated and copied to Tyler's desktop
  (`Noble-AI-Limits-Report-2026-11-08.md`); repo copy is `AI-LIMITS-REPORT-2026-11-08.md`.

- 2026-11-08 09:02 ET (DeepSeek): **B2 done (engine patch built + verified).** `AI_FRAME3_OPS` grows
  0x76E814's frame 0x900 -> 0x1100 (lea rbp -0x838 -> -0x1038, cookie 0x7E0 -> 0xFE0, stack args 0x860/0x870 ->
  0x1060/0x1070, epilogue rsp+0x900 -> 0x1100) and `AI_FRAME3_CAP_OPS` raises its two `0x200` scratch caps to
  `0x400` (0x76E8EF, 0x76E925). `PoolResize` gets a new `ai_spawn_search_stack` MinHook that commits the stack
  pages (the function has no `__chkstk`); installed only when the patched prologue is present (log confirms).
  `--ai-bank-extra 0` still reproduces V16 `b8810c3b…` and V16p `47bc0723…`; new profiles v16 `2a815ac4…`,
  v16p `7d807c95…`.
- 2026-11-08 09:03 ET (DeepSeek): **B1 on mission 3 (m30, Nightfall): not a good hold map.** 1,000 placed in
  27 s, peak **482 live**, then the mission's own scripting removed them (11 live after 90 s). Memory flat
  (6,860 MB private / 7,280 MB working set). Frame rate 7-45 fps during the wave (one 300-frame window = 41.5 s),
  28-45 fps at ~440 live, 110-140 fps after the despawn. A second run lost its process at 09:05:22 with no
  crash record. During the first run, ~15 F6 presses from outside my run raised the count to 980 and the game
  crashed at 09:03:45 (`0xC0000005` in `VCRUNTIME140.dll`, dump `data\sandbox\noble-crash.dmp`). Firefight
  Unearthed remains the reliable hold (1,024 placed, 685-767 live, stable) and the same patched engine ran a
  900-actor Firefight for 85 s with no crash.
- 2026-11-08 09:10 ET (DeepSeek): **B4 C11 runtime half: implemented but the bank cannot take it.** With
  `-render-states` (new opt-in flavour of the PoolResize rule) 16,384 crashed at `haloreach+0x27B01F` and 8,192
  at `haloreach+0x6E58E`, both during map load; default builds keep the array at 4,096 (verified). The rule is
  in `PoolResize.cpp` but gated behind `-render-states N` / `NOBLE_RENDER_STATES`, off by default. The V16
  append-block/bank work is the prerequisite.
- 2026-11-08 09:12 ET (DeepSeek): **B5 C6 script node constructor found.** `script node` is created by the
  growable-array helper `0x13148`, called from `0x81854`: name `"script node"` (0x995F60), initial bytes
  `mov r8d,0xFC00` at 0x81846, record size `r9d=0x18` (24 B). 0xFC00/24 = 2,688 initial nodes and the pool
  grows, so 16,280 in the pool report is not a cap. Patch point: the 0xFC00 immediate (file 0x80C46) or a hook
  on 0x13148. Documented in `docs/limits-ai.md`; not patched.
- 2026-11-08 09:14 ET (DeepSeek): **B7 C12 players research done** — `docs/limits-players.md`: players pool
  16 × 0x490 (created RVA 0x43474, count immediate 0x43470, size 0x43465), fire team 16 × 0xD4 (0x122E58 /
  0x122E54), simulation view 16 × 0x118, the LobbyPatches RVA set, and the 16 -> 32 -> 64 -> 128 milestone
  plan with effort estimates.
- 2026-11-08 09:16 ET (DeepSeek): B3 (join gating) and B6 (light volumes) **not implemented** — B3 needs a
  backend field + two sessions to test, B6 is the multi-hour rebase described in LIMITS-HANDOFF.md. Notes for
  both are in the updated `AI-LIMITS-REPORT-2026-11-08.md` (Batch 2 section). All three targets rebuilt; report
  copied to Tyler's desktop. Nothing committed, pushed, packaged or zipped.

- 2026-11-08 09:20 ET (DeepSeek): **double check done.** Disassembled the cached patched DLL: all 0x76E814
  ops (frame 0x1100, cookie 0xFE0, stack args, epilogue, three 0x400 caps) applied; both stack-commit hooks
  install. `test-limits.ps1` PASS for Stock (2,048), Extended (32,767) and Extended + Particles; a default
  512 start shows actor 512 and render states 4,096; Firefight 1,024 on the final build ran 900 placed / 679
  live for 80 s with no crash. Fixed a stale duplicate comment above the render-states rule. All targets rebuilt
  (client/dedicated 09:18, dev 09:14), data synced, no processes left running, no pycache.

- 2026-11-08 09:25 ET (DeepSeek): **cutscene fix after Tyler's note — the auto-horde was spawning during
  the mission-3 opening cutscene** (10 s after the first tick). `SpawnTest` now parses `-mode` and waits 60 s
  before the automatic `-horde` / `-spawn-test` queue on campaign maps (override `-horde-delay N`,
  `NOBLE_HORDE_DELAY`); Firefight/multiplayer keep 10 s. Verified on m30 (5-minute run 09:24-09:30): queue at 60 s, 1,000 placed in
  ~24 s, no 1-second stalls, peak ~438 live, then the mission script removed them (11 live) — memory flat
  (6,836 MB private), process alive the whole 5 minutes, no crash. Firefight Unearthed remains the hold map.
  Report updated; desktop copy refreshed.

- 2026-11-08 09:55 ET (DeepSeek): **follow-up on Tyler's notes (cutscene flag + team hold).**
  cinematic_in_progress: found the command-table entry (0xA15468) and its thunk 0x86CBC → 0x85DE0; 0x85DE0 is the
  script value dispatcher and calling it with (0,0) faults at haloreach+0x86CA6 (it needs the VM argument), so
  the build validates/detects it but keeps the timer fallback (campaign 60 s, -horde-delay override),
  kUseCinematicCheck=false. Next step: MinHook 0x86CBC to capture the real arguments. Teams: ai_set_team's thunk
  0x1A25DC calls 0x6C1C14(actor, team); SpawnTest now has -horde-team N and sets every placed actor's team.
  Firefight test with -horde-team 0: 1,000 placed, 756 live at wave end (vs 679 without), 677 after 80 s —
  better, not flat. Mission 3 confirmed as a bad hold map (its script removes non-mission AI). All builds clean.

- 2026-11-08 09:52 ET (DeepSeek): **real cinematic flag wired in; hold refinements.** cinematic_start
  (thunk 0x1AF484 -> 0xD3240) and cinematic_stop (0x1AF4A0 -> 0xD3444) write the byte at game-state root+0x130;
  SpawnTest reads it (SEH-guarded) and gates the auto queues on it. Verified on m30: "true" at 09:45:47,
  "false" at 09:47:23 (105 s intro), horde queued in the same tick; 1,000 placed in 13 s, no stalls, no crash.
  Timer (campaign 60 s / 10 s) stays as fallback, -horde-delay overrides. Team/blind/deaf cores: 0x6C1C14 /
  0x6BF204 / 0x6BF0EC; squad team set before ai_place, actors marked blind+deaf, teleport skipped within 30 m,
  new -horde-no-teleport. Firefight 1,000: baseline 679 live; team 756; team+blind+deaf+30m 745; team+blind+deaf+
  no-teleport **973 live** at wave end (887 after 50 s) — the teleport was killing them, not combat. All builds
  clean; nothing committed/pushed.

- 2026-11-08 10:05 ET (DeepSeek): **5-minute hold on the requested command** (`-map ff_unearthed -mode
  firefight -ai-actors 1024 -horde 1000 -horde-team 0 -horde-no-teleport`). Tyler closed the window at ~10:05
  after the full ~5 minutes. 1,000 placed, **980 live** at wave end, settling to **~670** and flat for the last
  two minutes; memory flat (6,660 MB private / 7,085 MB WS); 17.5-19.8 fps at ~980-900 live, 21-24 fps at ~670;
  **no STACK OVERRUN/FATAL**. Best hold so far. Note: the earlier "process ended with no crash record" (m30 run)
  was also a closed window, not a crash — corrected in the report.

- 2026-11-08 10:35 ET (DeepSeek): **test batch for Claude's render-bank change.** Rebuilt noble-dev (clean,
  no source changes from me; engine ops untouched).
  **C11 as specified crashes** at 4,069 objects: `FATAL 0xC0000005 at haloreach+0x68B21F` after
  `Scheduler guard: object update wants 73795 jobs but the scheduler only has 16384`. The render-states half works:
  `cached object render states 4096 -> 16384`, `POOL|cached object render states|461|16384|624`, POOLNEW
  `POOLNEW|cached object render states|16384|624|0x7ff42b3a0000|0x7fff15881b08|data=0x7ff42b3a0070|bitmap=0x7ff42bd60070`.
  A/B without `-render-states` crashes identically (73,793 jobs, same RVA) -> the crash is the known C10 scheduler
  wall, not the C11 change. Reduced C11 (3,800 objects) ran the full 5 minutes with no crash (object pool peaked
  3,869, decayed to ~2,250). Visible >4,096 not exercised.
  **Regression**: run 1 wave-end 956 live (closed at 3.3 min, no crash); run 2 full 5 min: wave-end 920 live, flat
  ~682 after, memory 6,714/7,139 MB, no crash. **Particles**: v16p-69ee54924495 active, particles 16,384,
  particle_emitter 6,656, no crash. Report updated; desktop copy refreshed.

- 2026-11-08 10:40 ET (DeepSeek): **check after Claude's D3 edits.** All three targets build with the
  10:16 changes (0 warnings / 0 errors; exes 10:35-10:36). Start-up V16 clean (actor 512, render states 4,096);
  short Firefight run on the new binary: 1,000 placed, 982 live at wave end, 920 after 10 s, no crash. D3 is in
  and compiles: `limits_id` in registration/heartbeat (`limits::LocalNetworkId()`), `LobbyState.limitsId` parsed,
  join refusal with a message at FrontEndMenu ~6554; the field is optional so older hosts still accept. Not tested
  with two instances.

- 2026-11-08 10:46 ET (DeepSeek): **check on the D3 build (Claude's 10:16 edits).** No source changes after
  10:16, so the 10:35 build already had them; rebuild is clean (0 warnings/errors). C11 on this build:
  `-ai-actors 256 -render-states 16384 -spawn-test 3800` ran 5+ minutes with no crash, render states 16,384
  (841 used), 3,800 objects spawned 0 refused. Horde regression on this build: 982 live at wave end, no crash.
  D3 compiles and is present but cannot be tested until the master-server backend stores/returns `limits_id`
  (every host looks old, joins allowed by design). Claude's new docs (light-volumes plan, players audit) reviewed
  and consistent with `docs/limits-players.md`.

- 2026-11-08 10:52 ET (DeepSeek): **follow-ups from Claude answered in the report.**
  (1) The scheduler guard IS active in the crashing match processes and IS on the path: the minidump stack is
  `0x68B21F` (fault, pop) ← `0x68AE23` (inside the hooked prepare function, call at 0x68AE1E) ←
  `noble-dev+0x635EA` (guard detour) ← `0x46A604` (object update dispatch, call at 0x46A5FF). The guard never
  waited — it printed only the "wants 73,759 jobs but the scheduler has 16,384" line 1 ms before each crash, i.e.
  it deliberately skipped waiting on an over-capacity request, then the pop hit the empty queue. Fix: guard the
  pop (0x68B20C) or raise the scheduler, not the prepare policy.
  (2) 16,384 = CPU particle pool `particles` (2,048 in V16/stock), 6,656 = emitter pools (`particle_emitter` /
  `c_particle_emitter_gpu`, 832 stock), 4,096 = GPU rows (`c_particle_emitter_gpu::s_row`). The 65,535 figures
  are per-frame GPU budgets patched in code (file 0x27B5F2: 576 -> 65,535; spawn queue accepts 65,534; GPU rows
  redirect at 0x1D48FD), so they never appear in the pool report.

- 2026-11-08 11:55 ET (DeepSeek): **C8 light-volume port: code + build + plumbing done, Full profile still
  crashes at boot.** `tools/limits/lv_port.py` relocates V4's 7 routines + 5 blocks into the .deltrac gap
  (0xAAE8400, 2,654 B, all rel32/RIP fixups recomputed and verified), `make_v16_particles.py --profile full`
  adds the LV banks/storage and applies the 43 functional rows (95-row guide parsed, original bytes checked),
  `Profile::Full` is plumbed (settings label, `_ext_full`, `full.nlp`, dedicated `-limits full`) and
  `RtlAddFunctionTable` registers 6 unwind entries at load. Zero extras still reproduce V16 `b8810c3b…` and
  V16p `47bc0723…`; full = 14,201,950 B, sha256 `965453e2…`. **But the profile faults at
  haloreach+0x2C2E0B** after ~10 s. Bisect: allocator rows alone **boot and show light_volume* 65,535**, plus
  admissions/GPU/transparent rows also boot; **adding the parent rows (0x114D31/0x11ADCB/0x11AD85) or the message
  queue redirects crashes at the same place**. Likely missing V4-section storage for the parent pool / effect
  message queue. full.nlp is NOT copied into build/bin/Release yet, so selecting FULL falls back to Stock with a
  reason. Details + next steps in the report; scheduler ceiling logged as a known limit (16,384 jobs, ~74k asked
  by a dense 5,000-object grid).

- 2026-11-08 12:00 ET (DeepSeek): **C12 member trace run.** noble-dev built clean; `-trace-members` starts and
  dumps MEMBER| lines every 30 s (299 distinct instructions, 5.8M accesses in run 1). Two traced runs:
  main menu + CUSTOM GAME screen (11:51:25-11:55:25) and a direct Sword Base Slayer session
  (11:55:36-11:57:19, "game session started", frames to 7200). **No crash/FATAL**, but both runs stopped
  advancing: the menu screen stopped updating ~4 min in, and the match stopped pumping frames at 11:57:19 with
  the process still alive and busy - a hang with tracing on, no crash address. A LAN lobby with a joining
  player, kills, respawns and quit-to-menu could not be driven: the game ignores synthetic keyboard input
  (SendKeys and raw scan codes), so gameplay/two-instance joining is not automatable from this harness.
  Every MEMBER| line from both logs is in `docs/limits-players-trace.txt` (2392 menu + 901 match lines).

- 2026-11-08 12:14 ET (DeepSeek): **Full profile fixed and verified.** Claude's bank-4 read was right; the
  builder grew checkpoint/total but never wrote the main-bank operand (file 0x155A9), so bank 4 stayed at V16's
  0x27D0000 and the LV allocations overran it by 9,902,984 bytes (BANK|4 51,649,416 / 41,746,432) before the
  0x2C2E0B crash. Added BANK|n|used|capacity logging to PoolReport.cpp (table RVA 0x27BF270, stride 0x18F0,
  +0xA0 capacity / +0xA8 used, warning on used > capacity, first pass + every map/limit change, and in the
  -pool-report-exit path before the slow scan), added MAIN_OPS = [0x155A9] writing Layout.main, and set main =
  V16 0x27D0000 + V4 0xA60000 + 0x20000 margin = 0x3250000 (52,756,480; 1,107,064 bytes above the measured
  51,649,416). Full now boots (BANK|4 51,649,416/52,756,480, light_volume* 65,535, POOL-REPORT-DONE) and survived
  a 5-minute plasma Firefight (ff_unearthed heroic, 38,700 frames, no FATAL, ~6.9 GB private). Final full sha256
  3a80f35d; V16/V16p zero-extra hashes unchanged (b8810c3b / 47bc0723). full.nlp kept out of
  build\bin\Release until accepted.

- 2026-11-08 12:41 ET (DeepSeek): **F6-horde list overflow fixed (all extended profiles).** The append at
  0x1D4F85 to the four 512-entry lists (0x477E700, stride 0x1204, count +0x1200) had no bounds check and
  faulted at 0x1D4F96. Its 7-byte count load now jumps to a .deltrac cave that reloads the count, compares
  with 0x1FF, skips the append when full and otherwise resumes at 0x1D4F8C (caves at 0xAAE8200 / 0xAAE8400 /
  0xAAE8E60 for v16 / v16p / full). A .pdata-wide scan found this is the only writer of the counts (the reset
  at 0x1D4DD5 only zeroes them), so one site covers all four lists. New hashes: v16 48cb89fc, v16p 968724d9,
  full d5b0122e (the guard changes every extended build; b8810c3b / 47bc0723 are superseded). All three boot;
  FULL -ai-actors 1024, ff_unearthed Firefight, F6 x10 ran 5 min 18 s (frame 9,000) with no crash. full.nlp
  kept out of build\bin\Release; settings restored.

- 2026-11-08 12:52 ET (DeepSeek): list guard made extras-only per Claude's request. With
  --ai-bank-extra 0 --render-extra 0 the builder emits no guard and reproduces GreenKnight's exact
  V16 b8810c3b / V16p 47bc0723 (zero-extra proof intact). Default builds carry the guard: v16 007a4dac,
  v16p 6403f044, full d5b0122e; all three boot clean (POOL-REPORT-DONE). data\limits v16/v16p/full .nlp
  rebuilt with defaults and copied to build\bin\Release\data\limits (full.nlp kept there, as Claude asked).

## Ready for Tyler to test (2026-11-08 08:47 ET, DeepSeek build)

Build: `build\bin\Release\noble-dev.exe` (09:14; engine .nlp 09:02) — all three targets build clean; `data\limits\v16.nlp`
and `v16p.nlp` are the new ones (the cached engine copy rebuilds once on the first start).
SETTINGS > GAME OPTIONS > ENGINE > ENGINE LIMITS must be EXTENDED (V16) for tests 1-2 (see test 3).
Nothing here needs the installed `haloreach.dll`; Noble still uses its own copy.

**Already verified automatically (see `AI-LIMITS-REPORT-2026-11-08.md`):** with `-map ff_unearthed -mode
firefight -ai-actors 1024 -horde 1200` the full 1,024 pool was placed in ~13 s, ~760 were live, and it ran
60 s more with no crash. **Tests 1-2 below are now confirmation runs** — the interesting part left is holding
1,024 *alive* (combat attrition keeps killing them) and the manual UI checks in tests 3-4.

### Test 1 — 1,024 actors without key spam (the main one)

```powershell
cd D:\Project-Noble-Closed-Source\build\bin\Release
.\noble-dev.exe -ai-actors 1024 -horde 900
```

Start a Firefight match (Unearthed) and stand still for ~2 minutes. No keys needed: 10 s after the match starts
the log queues 900 actors by itself. If you press F6 too, it adds 100 more per press (it should now fill much
faster than before — several squads per 0.1 s instead of one per 0.2 s).

Log: `build\bin\Release\noble-client.log`. What to look for:
- `SpawnTest: -horde queued 900 actors` — the automatic queue fired.
- `SpawnTest: horde capped at the N free actor slots (M were queued…)` — pending was trimmed to the pool, expected.
- `SpawnTest: horde wave done | X placed, Y moved next to you | actors U / 1024` — the wave finished.
- `Engine pools [Extended (V16)]: … actor U / 1,024 …` — the pool report.
- **PASS:** `actors` goes past 700 (ideally ~900-1,024), never a `STACK OVERRUN` line, no FATAL/exit; the
  game stays responsive (it will be slow, ~20-30 fps, that is the known actor cost).
- **FAIL:** `[error] STACK OVERRUN in haloreach+0x…` — note the RVA and whether the process died. That names
  the next fixed table; `python tools\limits\analyze_ai_frames.py --function 0x<RVA>` (from the repo root)
  prints its frame/cookie/caps, and `docs\limits-ai.md` already lists the usual suspects (0x76E814, 0x76D714,
  0x76A358).
- Also note the FPS at 512 vs 1,024 for C7c (the pool report interval is a good moment to look at the counter).

### Test 2 — F6 horde speed and the new caps at 512

```powershell
.\noble-dev.exe -ai-actors 512
```

In Firefight hold F6 a few times, then watch it fill. Old behaviour: one squad per 0.2 s. New: several squads per
0.1 s, capped at the free pool slots. **PASS:** the wave reaches 512/512 in well under a minute and
`horde wave done` reports most placed actors moved next to you; no crash. This is also the safe test if test 1
crashes — 512 was stable before and must stay stable with the new engine patch.

### Test 3 — the new settings rows

SETTINGS > GAME OPTIONS > ENGINE:
- `ENGINE LIMITS` cycles STOCK / EXTENDED (V16) / EXTENDED + PARTICLES (this is D6; profile 2 was already built).
- `AI ACTORS` shows 96 (STOCK) / 256 / 512 / 1024 (EXPERIMENTAL) and is greyed out while ENGINE LIMITS is
  STOCK. Set EXTENDED + a value, APPLY, restart.
- **PASS:** after the restart the log has `Engine limits: AI actors 512 (network id 0x…) ` and
  `AI limits: experimental, actor pool 512 (stock 96)…`. The value survives a restart, and RESET TO DEFAULTS in
  the ENGINE category puts both rows back to STOCK / 96.

### Test 4 — V16 + particles (D6, unchanged engine build) — only if you want to check the UI path

Set ENGINE LIMITS = EXTENDED + PARTICLES, restart, load the 65,000-particle map. **PASS:** it loads, the pool
report shows particles 65,535 class capacity, no crash. (Engine side is Claude's profile 2; this build only adds
the row and the dedicated `-limits v16p`.)

### Test 5 — script pools (C6, only if a script-heavy map is handy)

Same `-ai-actors 512` run as test 2, with a map whose scripts are heavy (Firefight Unearthed is fine). The log
at start-up lists the new script pools: `AI limits: det hs thread 320 -> 1024`, `non-det hs thread 4 -> 32`,
`list object reference 128 -> 512`. **PASS:** no script error/toast, the map's scripts run normally. If a map
that used to hit "too many script nodes" still fails, that is the known open half of C6 (`script node` is not
a data_array_new pool; see `docs\limits-ai.md`).

### Batch 2 additions (2026-11-08 09:16 ET)

- The engine build changed again (0x76E814 frame grown): v16 `2a815ac4…`, v16p `7d807c95…`; the cached copy
  rebuilds once on the next start. `--ai-bank-extra 0` still gives the V16/V16p baseline hashes.
- **Mission 3 is not a good actor-hold map** (tested 09:00-09:25): the first run spawned into the opening
  cutscene (fixed: campaign now waits 60 s, `-horde-delay N` to change); 1,000 placed, peak 482 live, the
  mission's own scripting then thinned them. Use **Firefight Unearthed** for the 5-minute hold
  (`-map ff_unearthed -mode firefight -ai-actors 1024 -horde 1000`); the same build held 685-767 live there.
- `-render-states N` / `NOBLE_RENDER_STATES`: **do not use yet** — 8,192 and 16,384 both crash at map load
  (bank full). The flag exists only for the future append-block work.
- New research (no test needed): `docs/limits-players.md` (C12), `docs/limits-ai.md` C6 addendum (script node).

### 2026-11-08 10:10 ET (Claude): review of DeepSeek batches 1-2 + C11 render bank

- **Review:** regenerated both engine builds from DeepSeek's `make_v16_particles.py`: hashes match its log
  (v16 `2a815ac4…`, v16p `7d807c95…`), and `--ai-bank-extra 0` still gives V16 `b8810c3b…` / V16p `47bc0723…`.
  Checked AI_FRAME3 (0x76E814) instruction by instruction: rbp/rsp/cookie/stack args/xmm6/epilogue all consistent;
  caps 0x400 in place; Hook_AiSpawnSearch head matches the patched prologue. AI bank budget at 1,024 actors +
  script pools ≈ 5.6 MB of the 5.9 MB room: fits, but little margin left (raise `--ai-bank-extra` if more pools
  are added).
- **C11 render states, cause found:** `cached object render states` (4,096 × 624 B) lives in game-state bank 3
  (render, V16 0x560000), not bank 1, so 8,192 / 16,384 overran it (DeepSeek's map-load crashes at 0x6E58E /
  0x27B01F). Fix: `make_v16_particles.py --render-extra` (default 0x800000 when the AI extra is on) grows bank 3
  (operand RVA 0x1619A); checkpoint/total follow. New hashes: v16 `efb4fbf2…`, v16p `69ee5492…`; zero-extra still
  reproduces V16 / V16p. `PoolResize.cpp`: render states now charged to the render bank's own room (read from
  `mov edx` at 0x16199), the "only ≤ 512 actors" restriction removed. Still opt-in (`-render-states N`), untested.
- **Claims (Claude):** C11 bank/append-block side; B6 C8 light volumes next (DeepSeek left notes only).

### 2026-11-08 10:40 ET (Claude): D3 join gating + C8 plan + C12 audit pass 1

- **D3 done in code (untested):** `limits::LocalNetworkId()` (active profile + applied NOBLE_AI_ACTORS) in
  EngineLimits; hosts send `"limits_id"` in session registration and heartbeat (MasterServerClient); lobby JSON
  `limits_id` parsed into `LobbyState::limitsId` (-1 = old host). Server-browser join handler now always peeks the
  lobby and refuses a mismatch with a status message ("Can't join: this game uses engine limits …").
  `ServerBrowser::SetStatus` added. **Needs the private backend to store and return `limits_id`** in the
  session/lobby record; until then the check is inert (old hosts are let through on purpose).
- **C8 light volumes:** plan in `docs/limits-light-volumes-port.md` (only ~3 KB free in .deltrac; register
  unwind info at runtime instead of copying V4's 350 KB exception table). Not started in code.
- **C12:** `docs/limits-players-audit.md`: 426 functions use the players pool, 33 have 16-bounds; top finding
  is a 32-bit active-player mask at +0x173C (67 sites): 32 players fit it, 64/128 need it widened.
- Note: `MasterServerClient` clamps `max_players` / `player_count` to 64 when parsing the server list; raise to
  128 with the C12 work.

### 2026-11-08 11:00 ET (Claude): C12 key finding
- The +0x173C player mask belongs to the **session membership object** (constructor 0x3CF864, vtable 0xA490D8),
  which holds **inline** arrays: 17 peers × 0x158 and 16 players × 0xB0, twice (current + second copy). 32+ players
  means retargeting ~1,100-1,200 player-array accesses (+ peer arrays) in the network code, or a Noble-side relay of
  several 16-player engine sessions. Details and counts in `docs/limits-players-audit.md`. Needs Tyler's call on
  the approach before code.
- Asked DeepSeek (via Tyler) to check: scheduler-guard coverage in the 5,000-object 0x68B21F crash, and what the
  16,384 particle figure is versus the recipe's 65,535.

### 2026-11-08 11:50 ET (Claude): C12 option 1 chosen (Tyler), milestone 32 started
- Tyler chose option 1: keep the engine's netcode/authority, move the player/peer slot arrays into bigger
  storage via a tooled rewrite; milestones 32 → 64 → 128; 64+ expected to need the dedicated server.
- **Session objects found:** 4 × 0x61668-byte membership objects, **static** at RVA 0x2B07418, built by the array
  constructor helper 0x788904 (ctor 0x3CF864, dtor 0x3CFA94). Code reaches them through pointers (only one
  rip-relative reference), so a static scan cannot tell which instructions touch the slots.
- **New dev tool `src/host/MemberTrace.*`** (wired in Loader, CMake updated): `-trace-members` /
  NOBLE_TRACE_MEMBERS=1 puts guard pages on [object, +0x4538) of all 4 sessions; a vectored handler logs every
  engine instruction that touches peersA/maskA/playersA/peersB/maskB/playersB (R/W, count, offset range) as
  `MEMBER|rva|region|R/W|count|min-max` every 30 s. Slow on purpose; dev builds only. Not built yet.
- **Ask for DeepSeek:** build; run a multiplayer custom game (LAN, Slayer, any map) with `-trace-members` for
  ~3 min including the lobby, match start, a few kills/respawns and quitting to the menu; if two instances work,
  have one join and leave. Copy every `MEMBER|` line into `docs/limits-players-trace.txt`. That list is the input
  for the rewrite tool.

### 2026-11-08 12:20 ET (Claude): C8 Full profile reviewed and released to the dev build folder
- Applied DeepSeek's `data/limits/full.nlp` to stock myself: profile 3, 654 rows, 0 stock-byte mismatches, output
  sha256 `3a80f35d…` (matches its log). Render bank 0x3FF0000, main 0x3250000 (and its two copies); `.deltrac` grown
  to 0x97E5E (ends 0xAAE8E5E, below `.hkwork` 0xAAE9000); LV sort guard / allocator blocks jump to the right stock
  targets (0x284C8D / 0x2848DD / 0x12FCC). Could not rerun `lv_port.py` here (no capstone), so the build script
  itself was not re-executed by Claude.
- Copied `full.nlp` into `build\bin\Release\data\limits\` so Tyler can pick FULL in SETTINGS > ENGINE.
- Minor: `.deltrac` SizeOfRawData 0x97E5E is not a multiple of FileAlignment (0x200); Windows loaded it fine in the
  5-min test, but round it up to 0x98000 in the builder for safety (DeepSeek).
