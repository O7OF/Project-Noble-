# HANDOFF for a new DeepSeek session: engine decompiles -> Noble upgrades (2026-11-15)

Written by Claude (claude.ai project "project rea"). Claude's usage is nearly exhausted, so this is the full briefing. Read this first, then `00-START-HERE.md`, then `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`.
Rules that still apply: DeepSeek owns `src`, builds and tests. No game assets and no raw decompile text in the Compendium. Mirror Compendium changes to `D:\Project-Noble-Closed-Source\knowledge` (Claude cannot reach it). Give Tyler exact PowerShell commands, one per code block. Do not publish a release or bump the version without Tyler saying "release it". The 128-player work stays on hold until Tyler lifts it.

## 0. Next session - read this first (updated 2026-11-15 13:00 ET)

**Work is resuming on the ranked top-10 list.** Read, in order: `00-START-HERE.md` -> this handoff ->
`project-docs\TOP10-NEXT-UPGRADES-2026-11-15.md` -> `project-docs\CROSS-GAME-COMPARISON-2026-11-15.md` (the evidence
behind the ranking). Then follow the rules in `PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` (prove before writing;
mirror docs to `knowledge`; no release/version bump/public push/tag without Tyler saying "release it"; networking and
security stay in Claude's lane; the 128-player hold stands).

Current state: v0.19.103, repo `D:\Project-Noble-Closed-Source` at `b2065bd` (tracked tree clean), no processes or
jobs running. Engine-prompt Stage 1, stabilization, the documentation pass and the cross-game comparison are all done;
the modder guide, player guide, master list (7 PROVEN / 3 FAILED / 58 NOT STARTED), address table and top-10 list are
current and mirrored. **Nothing from the top-10 has been started.**


### Where things are (read this before starting)
- **Docs (source of truth; mirrored to the repo's `knowledge` folder):** `C:\Users\tyler\Desktop\Compendium` -
  `00-START-HERE.md` (index/read order), `project-docs` (handoffs, guides, RE notes, QA, prompts),
  `decompile-exports\topics` (master list, address table, cross-game CSV, topic chapters).
- **Repo/code:** `D:\Project-Noble-Closed-Source` (`src`, `tools\limits`, `docs`, `release-notes`). Builds:
  `build\bin\Release` (`noble-dev.exe` / `noble-client.exe`). Test harness scripts live in
  `build\bin\Release\data\userdata\claude-tests-2026-11-11\tools` (`launch_noble.ps1`, `camp_test.ps1`,
  `match_test.ps1`, `forge_test.ps1`, `test-limits.ps1`, ...).
- **Decompiles:** MCC Reach `D:\MCC HALO REACH DECOMP\decompiled` (FUN_*, `hr+RVA` = addr - 0x180000000);
  Xbox 360 retail `D:\ReachX360\ghidra_retail\decompiled_v3` (**v3 only**, the named reference); other MCC engines
  `D:\MCC_DECOMP\{halo1,halo2,halo3,halo3odst,halo4}\decompiled`. Ghidra MCP is connected to
  `haloreach-copy.dll` (base `0x180000000`).
- **Reference map for tools/paths:** `project-docs\REFERENCE-LIBRARY-2026-11-15.md`.
- **Rules/phase plan:** `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md`; verification rules in the
  `TOP10` doc and `STABILIZATION-REPORT`.
- **Per-task background and starting points:** `project-docs\TOP10-NEXT-UPGRADES-2026-11-15.md` has the ranking
  (`What it delivers / Depends on / Cross-game evidence`) **and a `Starting points per item` table saying exactly
  which docs, master-list IDs, source files and decompile paths to read for each of the ten items.**


Start: ask Tyler which item number(s) and order he wants (or use the numbers he gives in his message), restate that
item's plan/size/risk/dependencies, then begin with the read-only probe step. Do not start other items until chosen;
no release work.

## 1. Why this exists (the goal)
Noble is a Halo Reach MCC community client. We now have decompiles of: MCC `haloreach.dll` (the build Noble loads), the Xbox 360 retail Reach (with real function names from the leaked Omaha linker map), the 360 dev-kit beta, and all other MCC engines - Halo CE (**79,637 functions, finished 2026-11-15 10:19 ET, 2 failed**), Halo 2 (39,362), Halo 3 (20,444), ODST (21,484), Halo 4 (36,876). Purpose: stop guessing. Use the engine's own code to (a) fix bugs (hex numbers in UI, white visors), (b) raise limits (actors, particles, objects, players), (c) improve AI/pathfinding, networking, lighting/armor, (d) enable modder tools (Forge++, Megalo compiler, nav baking), (e) document everything for future agents.

## 2. Where everything is
- Compendium: `C:\Users\tyler\Desktop\Compendium` (read `00-START-HERE.md`). Decompile material: `decompile-exports` (README, scripts, indexes, `topics` chapters + `engine_source_map_x360_retail.csv`).
- MCC Reach decompile: `D:\MCC HALO REACH DECOMP\decompiled\hr_*.c` (names `FUN_*`, notes use `hr+0xRVA`, image base 0x180000000). DLL hash 738dd2d2...0894.
- Retail 360 Reach: `D:\ReachX360\ghidra_retail\decompiled_v3\hr_00000..00016.c` (**use v3**; 99 stubs left vs ~10,100 in older runs). Named via `symbols.csv` (57,153 rows). VA base 0x82000000. `functions_index.csv` lists name, size, file.
- Dev-kit beta: `D:\ReachX360\ghidra\decompiled` (no names; same stub issue, not fixable by name).
- Other games: `D:\MCC_DECOMP\{halo3,halo3odst,halo4,halo2,halo1}\decompiled` - all finished 2026-11-15 (CE last, 79,637 functions / 2 failed; each folder has `functions_index.csv`). Re-run batches: `D:\ReachX360\mcc_batches`.
- How to search: use grep/Select-String on the .c files for a string literal or a name. Retail files hold names in `// ==== name rva 0x... ====` headers.
- Existing notes: `project-docs\NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md`, `NOTE-MCC-HALOREACH-FULL-DECOMP-...`, `NOTE-REACH-X360-RETAIL-AND-OMAHA-MAP-...`, `NOTE-HEX-BUG-RUNTIME-CHECK-...`.

## 3. Method (how to turn a retail name into an MCC patch)
1. Find the system by name in the retail map (`engine_source_map_x360_retail.csv`, topic chapters) and read its body in `decompiled_v3`.
2. Match it in MCC `haloreach.dll`: search `hr_*.c` for the same string literals, constants and call shape. Record `hr+0xRVA` in a table in the Compendium.
3. Prove with a read-only runtime probe before any write (log values; compare to expectation).
4. Implement in `src` as a hook; test single player, host+client, save/load, saved film. Pools that are replicated/saved must stay consistent on all peers.
5. Document the result (name -> hr+RVA -> behaviour -> test result). Mark anything unproven as UNPROVEN.
Caveats: Ghidra lacks VMX128 (some 360 functions have gaps). MCC differs from 360 in renderer/audio/network transport; match by structure, not address.

## 4. Tasks for DeepSeek, in priority order

**Status 2026-11-15 10:50 ET** (from `WORKFLOW-LEDGER` / `TODO-MASTER`; this session has not changed `src`): T1 IN PROGRESS (stock-slot fallback shipped `96fd2c4`; shared-heap / engine-visor-table route still open, needs Tyler's decision); T2 UNPROVEN (runtime check did not reproduce the theory, needs the exact repro screen/mode); T3 not started (highest leverage); T4-T6 not started (armour-effect recolour proof done `64e6d22`, selectable variants not; elite lighting not); T7 ON HOLD (Tyler); T8 READY (all six MCC decompiles are now finished, including CE); T9 not checked; T10 not started.

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
- Never use `decompiled` or `decompiled_v2` for the retail game (empty stubs).
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
  `D:\Project-Noble-Closed-Source\knowledge` (hashes verified).
- **Proven:** repo at `eea9868` (`main` = `wip-merge`, 0 tracked changes, 44 pre-existing untracked lines); v0.19.103
  is the latest release, draft=false, 4 assets, published 2026-11-15T14:57:37Z (GitHub API); Halo CE decompile
  output is complete on disk (79,637 functions / 2 failed, 40 `hr_*.c`, `functions_index.csv` 79,638 lines;
  `ghidra.log` "DONE: 79637 functions, 2 failed"); all pointer targets resolve in both the Compendium and the
  `knowledge` mirror.
- **Unproven / not done:** no engine task (T1-T10) started; no `src` change, no build, no commit, no version bump,
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
  QA-VERIFY new section; mirrors to `knowledge`. Recommendation: HOLD the release (no new user-facing content since
  v0.19.103); ship a small stability release only on Tyler's request + "release it".

### 2026-11-15 12:40 ET - Documentation pass (modder guide, player guide, master-list statuses)
- Wrote the modder guide (all 68 engine hooks with MCC RVAs measured from a live startup log; tooling modules;
  launch flags incl. a 216-entry auto-generated appendix; 38-rule variant editor; appearance data; unverified table)
  and the player guide (Firefight/Marine/appearance, with known issues). Corrected the earlier draft's two errors:
  Firefight forces skulls 0, and the 7 stock armour effects ARE selectable (only new recoloured variants are dev-only).
- Updated the master upgrade list: every one of the 68 items now marks PROVEN / FAILED / NOT STARTED with MCC
  addresses where proven (7/3/58). Updated the address table with the proven MCC-only hook targets and the
  startup-patch table. Updated TODO-MASTER, 00-START-HERE and this handoff; mirrored everything to `knowledge`.
- No code changes were needed for the docs; repo unchanged at `b2065bd`, version still 0.19.103, nothing published.
- Unverified items are flagged in the guides' tables (horde-full-lod/infantry, Alert-Carry, tag_get, 128/16 gates,
  extended profiles, -engine-cmd, per-setting effects, Firefight rule behaviour, Armory in-match visuals, mod/.mvar
  contracts, Forge editor monitor).

### 2026-11-15 12:55 ET - Cross-game comparison finished + top-10 ranked list (nothing started)
- Finished the comparison for the systems not covered in Stage 1: `project-docs\CROSS-GAME-COMPARISON-2026-11-15.md`.
  Method: extracted every engine's data-array constructor literals (name, capacity, record size) from the decompiles:
  Reach 110 pools, Halo 3 83, ODST 86, Halo 4 112 - raw table in `decompile-exports\topics\crossgame-pools-2026-11-15.csv`.
- Findings: H3/ODST raised actor (96->256), squad (256->335/512) and objectives (64->100/125) capacities; Reach's
  proven pool hook can replicate those. H4 is the closest sibling (same allocator family) and kept Reach's counts but
  grew per-entity records (players 0x490 -> 0xF18). No later MCC engine has more than 16 player slots. Reach already
  equals/exceeds H4 and H3/ODST for particles/effects/decals/lights/render states - no later-game support for raising
  those. Networking/stability/rendering-feature internals are UNVERIFIED (no symbols in the other engines).
- Wrote the ranked list `project-docs\TOP10-NEXT-UPGRADES-2026-11-15.md` from the master list (7 PROVEN / 3 FAILED /
  58 NOT STARTED): 1 Extended-limits stabilization, 2 cross-game pool raises, 3 player-count expansion (hold), 4
  Firefight AI/wave options, 5 native team/spawn hooks + mod API, 6 navmesh baking, 7 exact visor colour, 8
  armour-effect variants, 9 Elite lighting, 10 Forge++/variant/Megalo tooling.
- Master list updated with a cross-game evidence section (statuses unchanged). **No item started** - waiting for
  Tyler to choose. No code/build/release changes.

### 2026-11-15 13:00 ET - Next-session pointer added (work resuming on the top-10 list)
- Added section 0 above so the next session reads 00-START-HERE -> handoff -> TOP10 -> cross-game comparison first.
- Status: v0.19.103 / `b2065bd`, tracked clean, nothing running; Stage 1 + stabilization + documentation +
  cross-game/top-10 complete; **no top-10 item started**. Next session should confirm Tyler's chosen item number(s)
  and order, restate the plan/risk, then begin at the read-only-probe step.

### 2026-11-15 13:15 ET - Top-10 #1 started: Extended-limits probe (claim disproven) + pinned-profile change set
- Tyler chose **#1 then #2**. Item #1 read-only probe first: added a dev-only `NOBLE_ENGINE_LIMITS_PROBE` override
  (`#ifdef NOBLE_DEV_BUILD`, release builds unaffected) because the 4407101 gates made non-Forge profiles unreachable.
- **Result: the 2026-11-14 claim is disproven on the current build.** Campaign m10 renders on Stock/V16/Full and on
  Tyler's exact reported combo **Full + 1024 AI actors** (4/4 normal frames, gameplay capture verified); Firefight
  ff_unearthed Full+1024 rendered 5/5 captures and a 10-minute soak; local MP host ran. The 11-14 campaign black screen
  was the `145510a` launch-block regression (host player id `+0xF0` skipped), fixed in v0.19.95 `dad10d2` six hours
  *after* `4407101` disabled the profiles; the "network kicks" were not reproducible solo and are most likely peer
  profile mismatches. Evidence table: `project-docs\NOTE-EXTENDED-LIMITS-REENABLE-2026-11-15.md`.
- **Tyler's decision:** Extended is no longer a user setting. Every install runs the pinned
  **Full (V16 + particles + light volumes) + 1024 AI actors** profile; ENGINE LIMITS and AI ACTORS pickers are removed;
  campaign saves are per-profile (`saves\HaloReach_ext_full_a1024`, legacy Stock folder untouched).
- Code done (private commit, no release): `limits::kShippedProfile/kShippedAiActors` + `ActiveSaveSuffix()`
  (`EngineLimits`), pin clamp in `Settings`, mode gate removed + pinned actor publishing + fallback errors in
  `main.cpp`, pickers removed in `SettingsPanel`, save-folder suffix in `ReachHost`, release pin + dev-only
  overrides in `PoolResize`, `full.nlp` packaging guard in `publish-release.ps1`, dev probe override.
- Verification: both gates built clean (client 13:20:13, dev 13:24:51) and **pinned-path end-to-end passed** in the
  main menu, campaign, Firefight, multiplayer and Forge: Full active, AI actors 1024 (network id 0x03000400), saves
  under `HaloReach_ext_full_a1024`, 0 crashes, 0 fallback warnings. Committed **`0848280`** (`wip-merge` = `main`,
  pushed to the private repo; version still 0.19.103, no release/tag/public push). Not done yet: campaign checkpoint
  save->load under the pinned profile, the 30-minute Stage-3 acceptance runs, and the join-path gaps handed to Claude.
- New Tyler bugs queued at the bottom of `TODO-MASTER` (items 17/18: pause-menu scroll wheel, ESC pause not pausing).
  No release/version bump; 128-player hold untouched.

### 2026-11-15 14:15 ET - Top-10 #1 complete; #2 done (cross-game pool raises + layout generation)
- **#1 marked COMPLETE by Tyler** (13:55 ET): commit `0848280`; gates clean; pinned-path verified in menu/campaign/
  Firefight/MP/Forge; remaining QA (campaign save->load, 30-min acceptance, Claude join gaps, old-save notice) carried
  as follow-ups, not blockers.
- **#2 done:** commit **`8f0fe85`** - squad 256 -> **512**, objectives 64 -> **125** on the proven `data_array_new`
  hook (ODST precedent; actor 96->256 already exceeded by the pinned 1,024). Live campaign report: squad 120/512,
  objectives 28/125; Firefight + Forge 0 crashes; gates clean; detail in
  `project-docs\NOTE-POOL-RAISES-2026-11-15.md`.
- **Layout generation added:** `kShippedLayoutGeneration = 1`, mixed into `ProfileNetworkId` (pinned id is now
  **0x03010400**), so a build with a different pool/layout pin refuses to join even at the same profile/actor count;
  join-refusal text updated. Rule for the future: bump the generation whenever the pinned layout changes.
- Next: **#3 (player-count expansion) chosen by Tyler at 14:20 ET** - the 128-player HOLD is LIFTED and, because
  Claude is unavailable, Tyler widened the lane: DeepSeek now owns the networking work for item #3 (session
  membership, replication, join handshake) as well as the client-side parts. Security stays out of scope; no release.
  Next: read the limits-players docs + SlotPatch/SlotMessage code, then reproduce the known stops with the local
  multi-instance harness before changing anything.

### 2026-11-15 14:20 ET - Item #3 started: 128-player hold lifted, lane widened (Tyler)
- Tyler chose **item #3** (player counts 32 -> 64 -> 128 + campaign co-op > 4) and confirmed "Lift hold, DeepSeek
  takes everything": the 128-player hold is **LIFTED**, and the session/membership/replication parts that were
  Claude's lane are now DeepSeek's for this item (Claude unavailable). Security items remain out of scope.
- Starting state per the 2026-11-09/10 notes: 32-slot patches are implemented but never passed a full test
  (`SlotPatch`, `SlotMessage`, `LobbyPatches`; high-slot join stalled at player 16 + uncaved tables; UI picker
  clamped to 32); campaign co-op >4 unproven. Item #3 has no release authority - version stays 0.19.103.
- **Campaign JIP probe (15:20 ET):** the join-in-progress path already exists (LobbyPatches join-accept/boot-keep +
  evaluate_coop_insertion override; BlamEngine guest campaign sync). Host solo m10 + 4 sequential mid-mission guests:
  **all joined** (msg 26 true), host engine player table reached 5/16, 0 crashes - but each joiner gets its own team
  (t1/t2/t3), so team count 3 fires GameResults on the guest and it exits. Next change: force campaign JIP players onto
  team 0 through the engine team-change path and suppress the guest-side game-results exit; then re-run probe-jip and
  the 5-player tests. Harness: `build\bin\Release\probe-jip.ps1`.
- **Campaign >4 research probes (2026-11-16, commit 16a5ee3):** env-gated, log-only diagnostics for the campaign launch
  state machine, launch status setter, membership-vs-launch-data comparison, session-start tick, restart contexts and
  player-create counters (all installed only with \NOBLE_JIP_PROBE); plus env-gated experiment switches for declared
  roster/wait barriers/slot-state and session-start skips. Main finding: >4 players makes the engine enter a network
  session-start mode instead of the direct launch path; that mode does not complete under the current host emulation.
  Next work: satisfy that session-start path or patch the mode choice (state-3 decision hr+0x40BBC0 / comparison
  hr+0x39EBF0), then implement extra-player spawn-in. Ship increment stays f7a2c74 (4 players).
- **Campaign co-op SHIP INCREMENT (2026-11-16, commit f7a2c74):** campaign co-op joins are stable for stock-sized
  sessions (up to 4 players): campaign guests start in the multiplayer engine class (no external-launch abort), team-0
  forcing, exact roster declaration, real channel readiness wait. Verified 3- and 4-player launches all active on m10.
  **16 players still blocked by the engine's campaign game-engine start cap (4):** declaring 5+ closes the session at
  simulation start; declaring 4 keeps the engine alive with the extra joiner as a non-active membership entry. Probe
  diagnostics are log-only behind \NOBLE_JIP_PROBE. Remaining work: lift the campaign game-engine player cap.
- **Campaign co-op >4 (14:45-15:15 ET, 5 local instances, low-spec settings):** the >4 launch path works (host logs
  "co-op roster is over 4; creating the session with capacity 8 (0xE8)"); a 4-player m10 session ran live (host state 6,
  engine game-player table 4/16) and on Exodus (m50) **all five players formed the same co-op session** (pmask 0x1F,
  peers p0-p4 state 8, each guest decoded msg26 15x, 0 timeouts/crashes). Every instance then exited at the peer-ready
  point (`EngineEvent reason=0`, no relaunch) without starting the mission - 5-player session formation proven,
  in-mission 5-player still to fix (next: launch-state/peer-barrier instrumentation, compare 4-player m50).
  Failure mode learned: campaign has no join-in-progress; guests that miss the host's pre-mission wait window get
  `external_launch_individual_state_timeout` (guest reason 4) or make the host abort (reason 2). Harness:
  `probe-camp5.ps1` + `low-spec-settings.json` (1280x720, FXAA off, 30fps, muted, AOC); peak 22.2 GB/31.8 GB.
- **First milestones (14:35 ET):** with item #1's pinned profile the 32-player patch now installs fully (110 sites,
  23 masks, 139 bounds, membership msg #32) and the pools run at **32** (players/fire team/simulation view); a
  2-instance normal-slot LAN join succeeds (host 2 peers/2 players, pmask 3; guest decodes msg 26 0xA300, acks msg 35;
  msg 44 stream; 0 crashes). **High-slot (peer 17/player 16) still fails** - host removes the high-slot player right
  after the ack (producer 2 players -> 1), CLOSE reason 10; guest CONNECT REFUSED reason 3. Next: host-side player
  add/remove dev trace. Full details/evidence: `project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md`.