# HANDOFF for a new DeepSeek session: engine decompiles -> Noble upgrades (2026-11-15)

Written by Claude (claude.ai project "project rea"). Claude's usage is nearly exhausted, so this is the full briefing. Read this first, then `00-START-HERE.md`, then `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`.
Rules that still apply: DeepSeek owns `src\`, builds and tests. No game assets and no raw decompile text in the Compendium. Mirror Compendium changes to `D:\Project-Noble-Closed-Source\knowledge\` (Claude cannot reach it). Give Tyler exact PowerShell commands, one per code block. Do not publish a release or bump the version without Tyler saying "release it". The 128-player work stays on hold until Tyler lifts it.

## 1. Why this exists (the goal)
Noble is a Halo Reach MCC community client. We now have decompiles of: MCC `haloreach.dll` (the build Noble loads), the Xbox 360 retail Reach (with real function names from the leaked Omaha linker map), the 360 dev-kit beta, and all other MCC engines - Halo CE (**79,637 functions, finished 2026-11-15 10:19 ET, 2 failed**), Halo 2 (39,362), Halo 3 (20,444), ODST (21,484), Halo 4 (36,876). Purpose: stop guessing. Use the engine's own code to (a) fix bugs (hex numbers in UI, white visors), (b) raise limits (actors, particles, objects, players), (c) improve AI/pathfinding, networking, lighting/armor, (d) enable modder tools (Forge++, Megalo compiler, nav baking), (e) document everything for future agents.

## 2. Where everything is
- Compendium: `C:\Users\tyler\Desktop\Compendium\` (read `00-START-HERE.md`). Decompile material: `decompile-exports\` (README, scripts, indexes, `topics\` chapters + `engine_source_map_x360_retail.csv`).
- MCC Reach decompile: `D:\MCC HALO REACH DECOMP\decompiled\hr_*.c` (names `FUN_*`, notes use `hr+0xRVA`, image base 0x180000000). DLL hash 738dd2d2...0894.
- Retail 360 Reach: `D:\ReachX360\ghidra_retail\decompiled_v3\hr_00000..00016.c` (**use v3**; 99 stubs left vs ~10,100 in older runs). Named via `symbols.csv` (57,153 rows). VA base 0x82000000. `functions_index.csv` lists name, size, file.
- Dev-kit beta: `D:\ReachX360\ghidra\decompiled\` (no names; same stub issue, not fixable by name).
- Other games: `D:\MCC_DECOMP\{halo3,halo3odst,halo4,halo2,halo1}\decompiled\` - all finished 2026-11-15 (CE last, 79,637 functions / 2 failed; each folder has `functions_index.csv`). Re-run batches: `D:\ReachX360\mcc_batches\`.
- How to search: use grep/Select-String on the .c files for a string literal or a name. Retail files hold names in `// ==== name rva 0x... ====` headers.
- Existing notes: `project-docs\NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md`, `NOTE-MCC-HALOREACH-FULL-DECOMP-...`, `NOTE-REACH-X360-RETAIL-AND-OMAHA-MAP-...`, `NOTE-HEX-BUG-RUNTIME-CHECK-...`.

## 3. Method (how to turn a retail name into an MCC patch)
1. Find the system by name in the retail map (`engine_source_map_x360_retail.csv`, topic chapters) and read its body in `decompiled_v3`.
2. Match it in MCC `haloreach.dll`: search `hr_*.c` for the same string literals, constants and call shape. Record `hr+0xRVA` in a table in the Compendium.
3. Prove with a read-only runtime probe before any write (log values; compare to expectation).
4. Implement in `src\` as a hook; test single player, host+client, save/load, saved film. Pools that are replicated/saved must stay consistent on all peers.
5. Document the result (name -> hr+RVA -> behaviour -> test result). Mark anything unproven as UNPROVEN.
Caveats: Ghidra lacks VMX128 (some 360 functions have gaps). MCC differs from 360 in renderer/audio/network transport; match by structure, not address.

## 4. Tasks for DeepSeek, in priority order

**Status 2026-11-15 10:50 ET** (from `WORKFLOW-LEDGER` / `TODO-MASTER`; this session has not changed `src\`): T1 IN PROGRESS (stock-slot fallback shipped `96fd2c4`; shared-heap / engine-visor-table route still open, needs Tyler's decision); T2 UNPROVEN (runtime check did not reproduce the theory, needs the exact repro screen/mode); T3 not started (highest leverage); T4-T6 not started (armour-effect recolour proof done `64e6d22`, selectable variants not; elite lighting not); T7 ON HOLD (Tyler); T8 READY (all six MCC decompiles are now finished, including CE); T9 not checked; T10 not started.

**Status update 2026-11-15 12:40 ET:** the per-item PROVEN / FAILED / NOT STARTED statuses (with MCC
addresses where known) are now recorded in the master upgrade list (`decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`),
not in this file. Current counts: 7 PROVEN, 3 FAILED, 58 NOT STARTED of 68 items. Modder + player guides are in
`project-docs\MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md` and `project-docs\PLAYER-GUIDE-FIREFIGHT-MARINE-APPEARANCE-2026-11-15.md`.

**T1. Visors (exact colour in matches).** Current state (b1680d1): fallback maps every finish to one of 5 stock visor slots, so nothing is white; Armory shows exact colours (Noble repaints menu materials). In-match exact RGB is not achieved. HREK/map tint routes are ruled out. Ask the decomps: how does the engine turn player appearance (visor value) into a render material? Start at retail `player_model_customization` (0x821eacc8), `player_appearance` (0x824bce08), `c_player_trait_appearance`, `render_models`/object change-colour code, then find the MCC equivalents. Question to answer: where does the engine index visor materials, how many entries does that table have, and can an extra entry or a per-object colour override (change-colour/"color_math") be injected per player? Armour effects already recolour via particle palette (proven, `-armor-effect-test`).
**T2. Hex numbers in UI** (`0000001c`, "Respawn in 00000001"). Candidate: MCC `FUN_1802d288c` (hr+0x2D288C) leaves `\a<8 hex digits>` unexpanded when the ctx handle at ctx+0x180 is -1. UNPROVEN: a solo runtime check saw no leak. Next test: same read-only hook with the scoreboard (Tab) open, screenshot, and check whether Noble hides or overlays the original engine scoreboard (Tyler recalls the original Reach scoreboard drawing BEHIND Noble's). Hook source: `decompile-exports\mcc-haloreach\hexbug_diag.cpp`.
**T3. Limit/pool audit (highest leverage).** Build an engine-state inspector: enumerate the ~196 fixed pools (`g_*_allocator`, `data_new` calls) in MCC, log fill levels, and log which pool fails when a crash/assert happens. Cross-check what Noble already raised (actors, particles) against the related pools listed in master list sections A1/A2 (props, squads, effect locations, decals, impacts).
**T4. Free diagnostics.** Net latency/framerate overlay (`g_network_interface_show_latency_and_framerate_metrics_on_chud`), AI debug draw, crash minidump path (master list B6, C1, C9, E11).
**T5. AI.** Raise `g_path_state_traverse_max_iteration_count`; study `objectives_update`, `path_state_build_path`, `actor_select_firing_position_internal` in v3; marines as base troopers via data first (character/squad tags) and check `k_num_character_definition_cache_entries`.
**T6. Armory/lighting.** Elite lighting via cinematic SH lighting / object lights / texture-camera constants (`k_default_texture_camera_exposure`, `illum_scale`); armour effect variants selectable in the Armory.
**T7. Multiplayer 128** (ON HOLD). When lifted, follow master list A5 (membership -> players -> per-player entities -> UI) and keep deterministic/replicated sizes identical on all peers.
**T8. Cross-game comparison.** For each system (pools, AI, pathfinding, networking, Forge) follow it Halo CE -> 2 -> 3 -> ODST -> 4 and compare against Reach; record what changed and which limits grew. Reach names flow outward by matching shared strings/constants.
**T9. RTTI check.** Check whether MCC `haloreach.dll` keeps C++ RTTI/vtable names (halo1.dll showed TypeDescriptors). If yes, auto-name many `FUN_*` and update the indexes.
**T10. Tooling roadmap.** Forge++ overlay, map-variant/Megalo editors, navmesh baker, pool inspector (master list section G).
Not mine to decide: release/version bump (waiting on Tyler), the two-PC lobby-code test (needs Tyler).

## 5. Answer to "can these issues be solved with all the games decompiled?"
Likely helpful for visors and hex, not guaranteed. The decompiles show how the engine reads appearance and formats text, which removes guesswork; the visor limit may still be a data-table limit that needs an engine patch. Treat decomps as a map, then prove every change with a probe.

## 6. Known pitfalls
- Never use `decompiled\` or `decompiled_v2\` for the retail game (empty stubs).
- Device bridge limits for Claude: only connected folders can be written; DeepSeek has the full disk.
- "Unprocessed TypeDescriptor"/"Could not create Data" Ghidra log lines are harmless.
- Keep raw decompile text out of the Compendium; commit only names, addresses, notes.

## 7. Session log (append-only)
### 2026-11-15 10:50 ET - DeepSeek session opened (docs intake, engine-upgrade phase)
- **Done:** read this handoff, `00-START-HERE`, the starter prompt (incl. git/release rules + addendum), the master
  list, `decompile-exports\README` and both hex-bug notes. Recorded O7's three requests in
  `project-docs\NOTE-O7-REQUESTS-2026-11-15.md`; added the packet pointer rows and refreshed the current state in
  `00-START-HERE.md`; started `decompile-exports\topics\mcc-address-table.md` (5 proven property-editor matches);
  recorded the Halo CE decompile finish (79,637 functions, 2 failed, 2026-11-15 10:19 ET) here and in
  `decompile-exports\README.md` / `01-PROJECT-HISTORY.md` / `MASTER` / TODO-MASTER; mirrored everything changed to
  `D:\Project-Noble-Closed-Source\knowledge\` (hashes verified).
- **Proven:** repo at `eea9868` (`main` = `wip-merge`, 0 tracked changes, 44 pre-existing untracked lines); v0.19.103
  is the latest release, draft=false, 4 assets, published 2026-11-15T14:57:37Z (GitHub API); Halo CE decompile
  output is complete on disk (79,637 functions / 2 failed, 40 `hr_*.c`, `functions_index.csv` 79,638 lines;
  `ghidra.log` "DONE: 79637 functions, 2 failed"); all pointer targets resolve in both the Compendium and the
  `knowledge\` mirror.
- **Unproven / not done:** no engine task (T1-T10) started; no `src\` change, no build, no commit, no version bump,
  no release/tag/public push. Hex-number UI bug still UNPROVEN. Exact per-player visor colour still open (stock-slot
  fallback shipped); armour-effect variants still not selectable.
- **Next:** phase 1 of the starter prompt - RTTI/vtable check on `haloreach.dll`, engine-state pool inspector, free
  diagnostics; then T3 (limit/pool audit). Tyler decisions still pending: visor RE scope, 128-player hold, release
  go/no-go (only on the words "release it"), two-PC lobby-code test, campaign-skulls retest.

### 2026-11-15 11:35 ET - Phase-2 Stage 1 done; test harness fixed
- Read the new `PROMPT-DEEPSEEK-PHASE2-FEATURES-2026-11-15.md` in full and followed Stage 1 only. Preconditions:
  address table started + pool inspector working; team/spawn hooks NOT proven (phase 3 not started) - noted in the report.
- New docs: `project-docs\STAGE1-STATUS-REPORT-2026-11-15.md` (proven/unproven table, pool readings on heavy Firefight
  ff_unearthed and heavy Forge forge_halo, engine-limits constraint, H3/ODST/H4 pool-system comparison) and
  `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md` (the networking/security boundary for Claude).
- Harness fix `d636b31`: `SpawnTest::Hook_Tick` gated on the wrong argument (unit handle instead of player index),
  so `-horde`/`-spawn-test`/dev cheats/cinematic gating had been dead since `7f31cf2` (2026-11-09). Both gates built;
  horde and object-spawn verified in Firefight and Forge; pushed main+wip-merge (version unchanged).
- Blocking: engine-limit profiles (V16/V16P/Full) are force-disabled in this build (upstream `4407101`); Stock bank
  headroom is only ~5-32 KB in banks 1/3/4 -> Stage 2 count raising and Stage 4 AI counts need the Stage 3 fix plus
  re-enable, and Firefight/multiplayer re-enable crosses into Claude's networking lane (handoff note `2).
- Next: Stage 2 preparation only (reproduce 5-player campaign crash + 34-player MP stop, find non-network causes,
  clamp UI rows, list network blockers). No release/version bump; 128-player hold unchanged.

### 2026-11-15 12:05 ET - Stabilization pass after v0.19.103 (feature work paused)
- Tyler paused features and asked for a stabilization pass on everything since the v0.19.103 baseline: change list +
  test proof, full regression (campaign/Firefight/multiplayer/Forge/Armory/lobby codes/save-load/saved films, default
  and max settings), record in QA-VERIFY, fix regressions only, clamp/hide crash-capable options, draft release
  notes + known issues, recommend ship vs hold. No version bump/publish.
- Result: 16 scenarios, 0 real crashes; campaign checkpoint save written; saved-film playback and invite-code codec
  (0 failures) proven; all UI screens render. Max-settings attempts (extendedLimits=3 + aiActors=1024 pre-set) reset
  to Stock and played with no black screen.
- Fixes: only crash-capable option clamps (`b2065bd`) - campaign MAX PLAYERS 4, multiplayer picker max 32, ENGINE
  LIMITS information-only Stock; both gates rebuilt and pushed main+wip-merge; version still 0.19.103.
- Docs: `project-docs\STABILIZATION-REPORT-2026-11-15.md`, `project-docs\RELEASE-DRAFT-AFTER-0.19.103-2026-11-15.md`,
  QA-VERIFY new section; mirrors to `knowledge\`. Recommendation: HOLD the release (no new user-facing content since
  v0.19.103); ship a small stability release only on Tyler's request + "release it".

### 2026-11-15 12:40 ET - Documentation pass (modder guide, player guide, master-list statuses)
- Wrote the modder guide (all 68 engine hooks with MCC RVAs measured from a live startup log; tooling modules;
  launch flags incl. a 216-entry auto-generated appendix; 38-rule variant editor; appearance data; unverified table)
  and the player guide (Firefight/Marine/appearance, with known issues). Corrected the earlier draft's two errors:
  Firefight forces skulls 0, and the 7 stock armour effects ARE selectable (only new recoloured variants are dev-only).
- Updated the master upgrade list: every one of the 68 items now marks PROVEN / FAILED / NOT STARTED with MCC
  addresses where proven (7/3/58). Updated the address table with the proven MCC-only hook targets and the
  startup-patch table. Updated TODO-MASTER, 00-START-HERE and this handoff; mirrored everything to `knowledge\`.
- No code changes were needed for the docs; repo unchanged at `b2065bd`, version still 0.19.103, nothing published.
- Unverified items are flagged in the guides' tables (horde-full-lod/infantry, Alert-Carry, tag_get, 128/16 gates,
  extended profiles, -engine-cmd, per-setting effects, Firefight rule behaviour, Armory in-match visuals, mod/.mvar
  contracts, Forge editor monitor).