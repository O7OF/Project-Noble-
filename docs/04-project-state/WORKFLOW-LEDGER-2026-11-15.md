# Workflow ledger - 2026-11-15

Append one entry per iteration (newest last). Read this before starting; update before finishing.

## Current state
- Branch: `main`=`wip-merge`=`ca770e2`, both gates built at 0.19.102, release HELD.
- Task board: see `team_task_list`; claim the first READY task.
- Next up: #15 bitmap-import alternative (ManagedBlam / `bitmaps-debug` / `patch-tag-field`), then #14 higher-level model
  hook, then #16 only if Tyler lifts the multiplayer hold.

## Task board (created 08:25)

| id | item | state |
|---|---|---|
| task-1 | #14-A visor higher-level model path + magenta capture | READY |
| task-2 | #15-A armour FX recolour proof | DONE 08:35 (64e6d22, blueflame_0/1.png) |
| task-3 | #13-B real two-PC join test | READY (external) |
| task-4 | QA + release prep | blocked by main queue (held) |
| task-5 | #14-B implement per-player visor colour | blocked by task-1 |
| task-6 | #15-B expose selectable effect variants | READY (task-2 done) |
| task-7 | Housekeeping (~3.5 GB cleanup) | blocked by task-4 |
| task-8 | Parked items | blocked by task-7 |
| task-9 | #16 multiplayer 128 players | ON HOLD, blocked by task-5/task-6 |

Hourly continuation scheduled: 09:30-22:30 ET (schedule-32e1f9b8), target this session; goal rearmed.

## Entries

### 2026-11-15 08:25 - workflow created
- Read: TODO-MASTER, QA-VERIFY, RE-VISOR-COLOR, RE-ARMOR-FX-COLORS.
- Decisions: style-2 server browser is already default (no work); #13 Social entry done; cancelled items removed.
- Blockers: colour list (defaults assumed), two-PC test, release hold, multiplayer hold.
- Next: task board task `#14-A` / `#15-A` (whichever is READY first).

### 2026-11-15 08:35 - task-2 (#15-A) DONE: palette recolour proof
- Hypothesis: point the particle palette reference at an existing blue palette instead of importing pixels.
- Method: ManagedBlam Set Path + Save on head_fire.particle's two `palette` refs -> plasma_blue_01.bitmap; tool.exe rebuilt
  Forge World (exit 0); `-armor-effect-test 5` + `-third-person` match on the rebuilt map.
- Evidence: `blueflame_0.png`, `blueflame_1.png` (blue/cyan flames on the local Spartan).
- Commit: `64e6d22` (dev test hook only). HREK original tag restored; blue variant saved as head_fire_blue.particle;
  appearance.dat restored.
- Next: task-6 #15-B (expose selectable variants), then task-1 #14-A (visor higher-level hook).

### 2026-11-15 08:40 - task-1 (#14-A) trial: FUN_1802c8480 force - no effect
- Forced slot 6 (model+0x674) magenta in the FUN_1802c8480 detour; Armory capture `visor_magenta_armory.png` unchanged
  (orange visor). Hook removed, gates rebuilt, tree clean.
- Next: tag-side shader patch via ManagedBlam (proven pattern from armour-fire palette) or hook the render-model shader
  parameter upload path. Then task-6 (#15-B) after that.

### 2026-11-15 08:55 - task-6 (#15-B) architecture mapped
- spartans.biped -> armor_fx.effect attachment; event StringIds 0x4D0/0x4D2/0x4D3/0x4D4/0x4D5 read via ManagedBlam.
- No contiguous item->event table in dll; next trace is where profile effect index becomes an event StringId (Ghidra).
- task-6 in progress; task-1 still in progress. Ghidra MCP back online.

### 2026-11-15 09:10 - #15-B trace + release prep
- StringId scan: searched haloreach.dll for clusters of 0x4D0/0x4D2..0x4D5; candidates inspected (e.g. FUN_1802bb884)
  turned out to be unrelated string-id usage, so the profile-index -> event-StringId conversion needs a different trace
  (hook the effect spawn or find the appearance-to-effect call site rather than scanning constants).
- Prepared `project-docs/RELEASE-DRAFT-NEXT-2026-11-15.md` with everything since v0.19.102 (no version bump / publish;
  release remains held per Tyler). QA doc remains the source for evidence.
- Next: dynamic trace of armour-effect spawn (hook effect creation / inspect the biped attachment call), then decide if
  selectable variants need engine table extension; keep #14 visor tag-side attempt queued.

### 2026-11-15 09:25 - visor shader tags found; front-camera capture attempt failed
- Found the visor shader tags and their tint parameters (`env_tint_color`, `self_illum_color`, `specular_tint`,
  `fresnel_color`) - ready to try the ManagedBlam tag patch + Forge World rebuild.
- Temporary `-third-person-front` camera did not produce a usable front view (capture front_cam_test.png); reverted,
  tree clean, gates rebuilt. Next: fix front camera from the engine camera marker or use a different capture path.

### 2026-11-15 09:40 - front camera test hook committed (`b1680d1`)
- `-third-person-front` mirrors the third-person observer through its focus point; front-capture works
  (`front_cam_final.png`). This is the capture path for the visor colour proof.
- Visor shader env_tint_color value sits in the parameter's animated-function struct, so the tag patch needs more work
  (or use a different parameter); logged in RE-VISOR-COLOR.
- Task-1 remains in progress; next: patch a visor shader colour parameter via ManagedBlam, rebuild Forge World and
  capture with -third-person-front.

### 2026-11-15 09:50 - task-7 housekeeping DONE (safe subset)
- Deleted 14 obsolete `noble_menu.map.*` / string-id backups from build/bin/Release/data/menumaps: **3,313.7 MB freed**.
- Kept active files: `noble_menu.map` (297 MB), `mainmenu_all.map` fallback (284.7 MB), its CSVs, and the live
  `noble_elite_pack.map` (logs show it is still loaded by ElitePack, so NOT stale - do not delete without an A/B test).
- Smoke test after cleanup: Armory/menu loaded and rendered (`menu_after_cleanup.png`); no errors.

### 2026-11-15 10:00 - effect tag_get trace negative
- HookedSharedTagGet trace never fired in a match (hook installed but game-time tag access is cached/other path); reverted,
  gates rebuilt, tree clean. Next: HS function table (effect_new_on_object_marker) or biped attachment update, or accept
  map-level variant.

### 2026-11-15 09:20 - visor shader-tag recolour: API works, but match uses shared heap
- Found the ManagedBlam function-editor colour API (`TagValueCustomFunctionEditor.SetColor` + `GameColor.FromArgb`), set
  env_tint_color magenta on all 35 visor shader tags, rebuilt Forge World, captured from the front: visor unchanged
  (visor_tint_crop.png). Match spartan materials are not in the map/HREK tags - they come from MCC shared heap/shared.map.
- All HREK visor tags restored from backups. Next #14 path: runtime shared-tag-heap rmsh scan + f4[6]/[7]/[17] patch, or
  engine visor table/lookup extension (per-player).

### 2026-11-15 09:20 - blind shared-heap scan rejected
- In-process signature scan for visor rmsh params found only false positives and did not change the visor; removed as
  unsafe (blind memory writes). Next #14 path: locate the shared tag-cache table/global and iterate real rmsh tags.

### 2026-11-15 09:25 - tag-table scan + reversed magic discovery
- Probe validated tag tables by scanning group magics; found none (only string false positives). Learned tag group magics
  are stored byte-reversed ("rncs"=scnr, "hsmr"=rmsh; Armory constant 0x726D7368 is "hsmr"). Probe removed, gates clean.
- Next: dump active tag table via [haloreach+0xC1A600] first, confirm layout, then find shared-cache tag tables in globals.

### 2026-11-15 09:30 - active tag table diagnostic
- Active tag table pointer works (count 15985 at +0x3FFF8) but group-table magics at +0x3FFFC do not look like tag group
  magics; tag-table layout needs Ghidra cross-check. Pointer scan for shared tables too noisy (false positives).
- Probe removed, gates clean. Exact custom visor is the remaining main-queue RE job; multiplayer is on Tyler's hold.

### 2026-11-15 09:32 - match-map rmsh tags exist but data not resident
- Active Forge table: 15985 tags, 235 groups, 723 rmsh entries, but all rmsh sp data fails validation -> map-local rmsh
  parameters are stripped/not resident. Exact visor requires shared-heap table or engine visor-table extension.
- Probe removed, both gates clean. Task-1 remains open pending Tyler's choice.

### 2026-11-15 09:31 - release packaging dry-run verified
- `publish-release.ps1 -SkipBuild -NoUpload` succeeded: base zip 357.1 MB (sha256 0ec9d1e3...), Forge pack 652.6 MB,
  staging contains no dev binaries/saves. No upload. Ready to bump version and publish on Tyler's go.

### 2026-11-15 09:36 - shared tag-table scan inconclusive (too slow)
- Located a plausible extra tag table (1073 tags, 255 groups, rmsh group 184) via reversed rmsh magic scan, but the
  full-memory sweep blocks the render thread and did not finish in-match. Removed probe; gates clean.
- Next (if pursued): background worker + direct shared-cache table pointer, or engine visor-table extension. Release
  packaging dry-run verified earlier; release/version bump and multiplayer remain on Tyler's holds.

### 2026-11-15 09:57 - RELEASE v0.19.103 PUBLISHED
- Version bump commit `eea9868` (main=wip-merge), both gates built 0.19.103.
- Published base + Forge pack + checksums; latest release, draft=false. Base sha256 81260682..., Forge sha256 be693143...
- Release notes v0.19.103 and in-game What's New entry included. Task-4 completed.

### 2026-11-15 10:50 - DeepSeek session opened: doc intake + engine-upgrade phase
- Read: `HANDOFF-FINAL-2026-11-15.md`, `PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` (full, incl. git/release rules and
  addendum A), `00-START-HERE`, `HANDOFF-DEEPSEEK-ENGINE-UPGRADE`, `MASTER-UPGRADE-OPPORTUNITIES`,
  `decompile-exports/README`, both hex-bug notes.
- Verified: repo `eea9868` (`main`=`wip-merge`, tracked tree clean), version 0.19.103, release v0.19.103 latest/draft=false
  via GitHub API (published 14:57:37Z); Halo CE decompile finished 10:19 ET - 79,637 functions, 2 failed
  (`D:\MCC_DECOMP\halo1\decompiled\`, `functions_index.csv` 79,638 lines incl. header).
- Wrote (Compendium): new `project-docs\NOTE-O7-REQUESTS-2026-11-15.md`; new
  `decompile-exports\topics\mcc-address-table.md` (5 proven property-editor rows + open candidates); `00-START-HERE.md`
  packet pointer rows + v0.19.103 current state; CE finish recorded in `decompile-exports\README.md`, `HANDOFF-DEEPSEEK`,
  `MASTER`, `01-PROJECT-HISTORY`; `TODO-MASTER` engine-upgrade section; mirrored all changed/new docs to
  `D:\Project-Noble-Closed-Source\knowledge\`.
- No `src\` change, no build, no commit, no version bump, no release/tag/public push (per Tyler's rules).
- Task board note: `team_task_list` is empty in this session; the task-1..task-9 rows above are ledger history only. Create
  and claim a shared task before the next work iteration.
- Next: phase 1 of the starter prompt - RTTI check on `haloreach.dll`, pool inspector, free diagnostics. 128-player ON
  HOLD; release waits for Tyler's "release it".

### 2026-11-15 11:35 - Phase-2 Stage 1 status report (+ harness fix d636b31)
- Read in full: `PROMPT-DEEPSEEK-PHASE2-FEATURES-2026-11-15.md`; all engine-prompt rules still apply; networking/
  security excluded and recorded in `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`.
- Preconditions: address table started (yes), pool inspector working (yes, proven in-match today), team/spawn hooks
  proven (NO - phase 3 not started). Stage 1 done anyway as instructed; Stages 2/4 gated on the hooks.
- Harness bug found/fixed: `SpawnTest::Hook_Tick` gated on arg1 (a unit handle) instead of arg2 (player index;
  decompiled `FUN_180061a38` -> `player_control_update`). `-horde`/`-spawn-test`/dev cheats were dead since upstream
  `7f31cf2` (2026-11-09). Fix `d636b31` (+6/-2), both gates rebuilt 11:19:51/56, horde+spawn-test verified.
- Measurements (Stock profile): heavy Firefight `ff_unearthed` (horde+1000 objects) - page reference ranges 85.7%,
  script node 80.2%, character properties cache 77.5% (31/40), object 1,472/2,048, water physics 88.5%; heavy Forge
  `forge_halo` (1,495 spawned objects) - havok components 1,500/1,500 (100%), water physics 96/96, page ranges 91%
  (12,119/13,321), object 1,567/2,048 (76.5%), tag cache 74.4%. Logs: `build\bin\Release\stage1-firefight.log`,
  `stage1-forge.log`.
- **Blocking finding:** Engine Limits (V16/V16P/Full) force-disabled by upstream `4407101` (`Settings.cpp:253-261`
  reset + `main.cpp:1708-1717` non-Forge forced Stock). Stock game-state banks map-independent: bank1 99.75%
  (5,472 B free), bank3 99.1% (31,924 B), bank4 99.5% (23,416 B) -> count raising needs Stage 3 fix + re-enable, and
  Firefight/multiplayer re-enable needs Claude's profile matching (see network handoff).
- Cross-game (required before code): H4 and H2 carry Reach's data-array signature `0x64407440`; H3/ODST use a
  vftable-based data-array ctor and lack several Reach-only pool names -> Reach pool scanner ports to H4/H2, not H3/ODST.
- Docs updated: new `STAGE1-STATUS-REPORT-2026-11-15.md` + `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`; 00-START-HERE pointers,
  TODO-MASTER, AGENT-COORDINATION, this ledger, HANDOFF-DEEPSEEK session log, REFERENCE-LIBRARY untouched today.
- Repo: `main` = `wip-merge` = `d636b31`, pushed private. Version unchanged (0.19.103). No release/tag/public push.
- Next: Stage 2 preparation only (5-player campaign crash + 34-player MP stop), after Tyler's go; Stage 3 before counts.

### 2026-11-15 12:05 - Stabilization pass after v0.19.103 (+ clamps b2065bd)
- Requested by Tyler: feature work paused; list changes with commits + test proof; full regression at default and
  maximum settings; record in QA-VERIFY; fix regressions only; clamp/hide crash-capable options; draft release notes +
  known issues; recommend ship vs hold; no version bump/publish.
- Delta since v0.19.103 release: one commit `d636b31` (test harness tick gate; dev-only). Also regressed the v0.19.103
  feature delta since v0.19.102 (drawer filters, server-browser footer, click fixes, lobby share codes, social button,
  test hooks) because .103 shipped without a full regression. Per-commit proof table: `STABILIZATION-REPORT-2026-11-15.md`.
- Regression matrix: 16 scenarios (7 menu/UI captures incl. lobby demo + codec self-test, 4 gameplay modes,
  saved-film playback, 4 max-settings attempts) on the AOC, unfocused, muted -> **0 real crashes**; every scenario
  reached its expected marker; campaign wrote checkpoint autosaves; max-settings attempts reset to Stock with no
  black screen. Raw: `build\bin\Release\qa-regression-results.txt`, `qa-regression-analysis.txt`, `qa-*.log`, `qa2_*.png`.
- None of the tested paths regressed. Clamped the crash-capable options in **`b2065bd`**: campaign MAX PLAYERS -> 4;
  multiplayer MAX PLAYERS picker -> max 32 (16 default); ENGINE LIMITS row information-only Stock; AI ACTORS 96
  (STOCK) text. Rebuilt both gates (noble-dev 11:51:53, noble-client 11:52:07), post-fix captures `qa3_*.png`, smoke
  Firefight clean; pushed main+wip-merge; **version unchanged (0.19.103), nothing published**.
- Docs: new `STABILIZATION-REPORT-2026-11-15.md` + `RELEASE-DRAFT-AFTER-0.19.103-2026-11-15.md` (draft notes + known
  issues + HOLD recommendation); QA-VERIFY section; 00-START-HERE/TODO-MASTER/AGENT-COORDINATION updated; mirrored to
  `knowledge\`.
- Gaps: real two-PC lobby-code join, multiplayer with real peers, campaign >4 repro, full save LOAD/resume, Forge
  editor monitor, Tyler's hand retests.
- Next: feature work stays paused; no release until Tyler says "release it" (if he wants a small stability release, the
  draft and checklist are ready).

### 2026-11-15 12:40 - Documentation pass (hooks, player guide, master-list statuses)
- Wrote `project-docs\MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md`: every engine hook/patch Noble applies (68 detours
  with hr+RVA measured from a live startup log; what/when/example/limits), host tooling (PoolReport, PoolResize,
  EngineLimits, SpawnTest, MvarDump, FrameCapture), launch/test flags (detailed table + auto-generated 216-flag
  appendix), the 38-rule game-variant editor + JSON format + -variant-poke, appearance/profile data, and a
  'could not verify by test' table.
- Wrote `project-docs\PLAYER-GUIDE-FIREFIGHT-MARINE-APPEARANCE-2026-11-15.md`: Firefight setup (maps, 7 game types,
  MAX PLAYERS 4/8/12/16 with >4 unverified, difficulty, game-options editor; no skulls/teams in FF; lobby flow),
  share codes (end-to-end untested), Armory Marine tab/slots, appearance (30 visor finishes -> 5 stock slots,
  7 stock armour effects + 12 Firefight voices selectable, Elite sets, custom colours, rank gates), known issues.
- Master list `MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`: all 68 items now mark PROVEN / FAILED / NOT STARTED
  with MCC addresses where proven (7 PROVEN, 3 FAILED, 58 NOT STARTED; evidence pointer added). Address table
  `mcc-address-table.md`: new proven MCC-only section (pool init 0x12FCC, bank table 0x27BF270, tick 0x61A38,
  ai_place 0x6C2B10, object spawn 0x49CC24/0x46D6E0, horde teleport/team/blind, saved-film monitor 0x3ABE70) plus
  startup-patch table.
- Doc errors found and fixed while writing (no code changes): the draft player guide claimed Firefight skulls and
  that armour effects were not selectable; code inventory shows FF forces skulls 0 and the 7 stock armour effects
  ARE selectable (only new recoloured variants are dev-only). Final player guide corrected.
- Everything mirrored to `knowledge\`; repo unchanged at `b2065bd` (tracked clean), version 0.19.103, no release.
- Flagged unverified: horde-full-lod/infantry paths, Alert-Carry behaviour, tag_get diagnostics, 128/16 gates,
  extended profiles, -engine-cmd, individual settings rows, Firefight variant-rule behaviour, Armory in-match
  visuals, mod/.mvar contracts, Forge editor monitor.

### 2026-11-15 12:50 - Second (independent) hook audit folded into the modder guide
- A second read-only audit of EnginePatches/ForgePatches/LobbyPatches/PoolResize/SchedulerGuard/CollisionWalkGuard/
  SpawnTest/camera/ReachHost confirmed the 68 measured hook RVAs and added detail that was folded into
  `project-docs\MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md`: new section 2h (limits/guard/scheduler hooks:
  `data_array_new` `0x12FCC`, `ai_update_stack` `0x70ABBC`, `ai_spawn_search_stack` `0x76E814`, `gs_failure_report`
  `0x788A78`, `scheduler_guard_prepare` `0x68ADD0`, collision-walk `0x150A30`, Loader IAT patch), the CameraFov global
  write (`0xB42C10`, not a hook), extra dev flags (`-test-map-gametypes`, `-variant-poke`/`-launch-poke`,
  `-slot-patch`, etc.) and the fact that `-pool-report-exit`/`-armory-validate`/`-armor-effect-test` are compiled out
  of release builds.
- Corrections from the audit: the visor `0x2EDC5E` patch is a 30-finish direct-map patch that was later superseded
  by the five-slot fallback (shipped behaviour is the fallback); the Firefight null-TLS patch `0x4E9F5C` has no
  test/log evidence (UNVERIFIED); some LobbyPatches slot-count/peer-capacity patches are pattern-scanned with no
  fixed RVA (UNVERIFIED, fail-safe skip). These were updated in the guide and its unverified table.
- Player guide and master list were reviewed for impact: no change needed (their visor wording is the 5-slot fallback,
  and the master-list F7 row already separates Armory mapping from exact per-player colour).
- Guide re-mirrored to `knowledge\` (hash verified). Repo unchanged (`b2065bd`), version 0.19.103, no release.
### 2026-11-16 16:50 - #15-B armour effect variants: engine chain proven, probe added
- Ghidra: item translator 0x3B2F4 mapping (Spartan 0xA6..0xAC, Elite 0xAD..0xB4 -> pmcg indices 0..6); appearance
  builder 0x1055EC bounds-checks index against the pmcg list count (+0x80/+0x8C) - no hard 0..6 clamp, index 7+ works
  if the list is extended. pmcg list/entry/change layout documented.
- Added dev-only `-armor-fx-probe` (translator detour, custom item ids 0xF0..0xF7 -> indices 7..14) and
  `-armor-effect-item <id>` profile override. Live runs: 0xAB->1 (flames), 0xAD->0 (Elite default), custom 0xF0->7 with
  no crash (builder skips it while count=7).
- Next: asset phase - new particle/effect variants, spartans.biped armor_fx permutations, extended pmcg effect lists,
  rebuild maps; then UI rows + replication. Doc: `docs/project-docs/RE-ARMOR-FX-VARIANTS-2026-11-16.md`.

### 2026-11-16 17:15 - #15-B first colour variant rendering (blue flames)
- Edited HREK tags (with .bak backups): armor_fx.effect event `legendary_flame_blue` -> `head_fire_blue.particle`;
  location 4 renamed `armor_fx_06` (new locations do not persist through ManagedBlam Save - workaround); render_model
  new permutation + marker group `armor_fx_06`; pmcg Spartan+Elite lists +`legendary_flame_blue` -> armor_fx_06 (8).
- Rebuilt Forge World (tool.exe, 226 s, 796,790,784 B). Tested Noble dev with `-map-override`, `-armor-fx-probe`,
  `-armor-effect-item 240`: translator maps custom 0xF0 -> pmcg index 7, blue flames render in third person; stock
  orange flames regression captured unchanged. Screenshots saved in the claude-tests folder.
- Next: more flame colours, then lightning; then Armory UI + profile persistence/replication, capability gating and
  distribution decision. Recipe committed: `docs/test-tools/armor_fx_add_variant.py`; doc updated.

### 2026-11-16 18:15 - #15-B 30-colour scaling attempt (visor palette): partial, ext-tag blocker
- Recolour pipeline works (bitmap pixel edit; gold palette renders via known-good particle). Generated 29 visor-colour
  variants + 5 ext effect tags + render-model region/markers + 29 pmcg entries + 5 extra biped attachments; map rebuilt.
- Blocker: effect-tag `locations` cannot gain elements through ManagedBlam Save (max 6 usable/tag); ext region/marker
  path works with a stock particle, but custom particles in ext tags are not consistently visible. Blue variant restored
  and verified; diagnostics documented in the RE doc; Armory UI wiring still pending.

### 2026-11-16 18:30 - #15-B primary-colour test: attachment tint does not affect particles
- Tested biped armor_fx attachment `change color = primary` with primary palette 2 (White) and 21 (Maroon): armour tinted
  correctly, flames stayed orange -> native effect tinting is not supported for these particle palettes (reverted).
- Added dev flag `-armor-primary-test <0..29>` for match captures; agreed direction: generate flame variant palettes from
  the 30 Reach primary colours (kReachColors) and derive the flame colour from the primary index, instead of 30 separate
  Armory effect rows. Still blocked on ext-effect-tag/custom-particle rendering; evidence in the RE doc.

### 2026-11-16 18:55 - #15-B root cause: engine caps unit attachments (first 10 only)
- Isolation proved slot-9 extra effect attachment renders (extra flames +23% pixels) but slots 10..13 are ignored, so only
  one free effect-tag attachment exists with the 9 stock unit attachments. 30 colours need either an engine attachment-cap
  patch, effect-tag location block extension, or dropping stock attachments (rejected).
- Clean state restored (biped 9 attachments, change color none, map rebuilt); blue variant still verified. Next: RE the
  attachment runtime limit, then generate primary-palette variants + auto/selectable armory UI.
