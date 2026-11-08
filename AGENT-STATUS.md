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

