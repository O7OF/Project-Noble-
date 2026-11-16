# Project Noble - knowledge folder: START HERE
Built 2026-11-14 by Claude for Tyler and any agent picking up the work. Current state refreshed 2026-11-15 12:55 ET (v0.19.103 + b2065bd; Stage 1, stabilization, documentation and cross-game/top-10 passes done).
Location on Tyler's PC: `D:\Project-Noble-Closed-Source\knowledge` (inside the repo folder but **git-excluded** via
`.git\info\exclude` -> `/knowledge/`, so it is never committed or pushed). Contains no game assets.

## What Project Noble is (one paragraph)
`noble-client.exe` / `noble-dev.exe` load MCC's own `haloreach.dll` (Halo: Reach) and drive it **without MCC**: own front-end
menu (ImGui), Armory with a 3D preview, custom games / Forge / Firefight / campaign, server browser, master server, P2P +
relay netcode, mods (Steam Workshop folders served through a VFS), extended engine limits. Reach Renaissance (RR, `D:\Reach`)
is the reference for behaviour only; no RR code/data is shipped. Partners: Tyler (GitHub WeabooTyler117: builds, tests,
reverse engineering; not a C++ developer), O7OF (repo owner, most upstream code), Dominus (joined 2026-10-28).

## Read in this order
1. **This file** (current state below).
2. **This session's packet** (section below) - DeepSeek briefing handoff, starter prompt + phase plan, master upgrade
   list, started MCC address table. This is the live plan and supersedes `project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md`
   as the current task list.
3. `02-LESSONS-AND-RULES.md` - Tyler's rules, working method, pitfalls, key engine addresses. Mandatory.
4. `project-docs\REFERENCE-LIBRARY-2026-11-15.md` - **external reference library**: Ghidra MCP usage + module facts,
   decompile exports on the Desktop, repo `docs` multiplayer/limits/AI work, `knowledge\drafts` networking draft,
   tag/asset tool folders, test harness and hooks. Read before researching anything.
5. `desktop-guides\TODO-MASTER-2026-11-14.md` - the live to-do list (engine-upgrade phase, then Tyler's ordered
   build/deploy queue).
6. `01-PROJECT-HISTORY.md` - condensed timeline 10-27 .. 11-15: what was tried, what failed, what fixed it.
7. `pc-handoffs\HANDOFF-ELITE-2026-11-12-NIGHT.md` - the detailed day log 11-12 .. 11-14 (every address, capture, dead end).
8. Topic docs as needed (index below).

## DeepSeek engine-upgrade packet (2026-11-15) - pointers

The current work plan for this phase. Rules of the road are in the prompt: no version bump, no release/tag/public push
and no release-asset upload unless Tyler says "release it"; the 128-player work stays ON HOLD until Tyler lifts it.

| Doc | Path in this folder | What it is |
|---|---|---|
| Briefing handoff (read first) | `project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` | Why the phase exists, where every decompile lives, the method (retail name -> MCC hook), tasks T1-T10 in priority order, known pitfalls. |
| Starter prompt + phase plan | `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` | Git/release rules, Step 0 reading list, Phases 1-6, addendum A-E (O7's requests, established facts, doc-update rules, where to look, working rules). |
| Master upgrade list | `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md` | "What can be upgraded": A limits, B stability, C networking, D rendering, E AI, F gameplay/UX, G tooling, H do-not-touch, I recommended order, J workflow. |
| MCC address table (started) | `decompile-exports\topics\mcc-address-table.md` | One row per proven retail-name -> MCC `hr+RVA` match; open candidates labelled UNPROVEN. |
| Decompile index | `decompile-exports\README.md` | Every decompile (MCC Reach, 360 retail v3, dev-kit, other MCC games), function counts, how to search and re-run. |
| O7's requests | `project-docs\NOTE-O7-REQUESTS-2026-11-15.md` | Community-dev asks (more hooks, native team change/swap, player-limit expansion) mapped to phases and holds. |
| Previous session handoff | `HANDOFF-FINAL-2026-11-15.md` | v0.19.103 state (commit/hashes), proven/unproven table, next-agent list. |
| Phase-2 feature plan (current) | `project-docs\PROMPT-DEEPSEEK-PHASE2-FEATURES-2026-11-15.md` | Stages 1-8 after the engine prompt; networking + security excluded and handed to Claude. |
| Stage 1 status report | `project-docs\STAGE1-STATUS-REPORT-2026-11-15.md` | Engine-prompt proven/unproven, pool-inspector readings (heavy Firefight + Forge), engine-limits constraint, H3/ODST/H4 pool comparison. |
| Stabilization report (2026-11-15 12:05) | `project-docs\STABILIZATION-REPORT-2026-11-15.md` | Change list + test proof, 16-scenario regression matrix, fixes/clamps (`b2065bd`), coverage gaps. |
| Release draft + known issues | `project-docs\RELEASE-DRAFT-AFTER-0.19.103-2026-11-15.md` | Draft release notes, known-issues list, ship-vs-hold recommendation (NOT published, version not bumped). |
| Modder guide (hooks/options) | `project-docs\MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md` | All 68 engine hooks with MCC RVAs + what/when/example/limits, tooling modules, launch flags, variant rules, appearance data, unverified list. |
| Player guide (Firefight/Marine/appearance) | `project-docs\PLAYER-GUIDE-FIREFIGHT-MARINE-APPEARANCE-2026-11-15.md` | Player-facing: Firefight setup/lobby, lobby codes, Armory Marine tab + slots, visors/effects/voices/Elite sets, known issues. |
| Cross-game comparison | `project-docs\CROSS-GAME-COMPARISON-2026-11-15.md` | Reach vs H3/ODST/H4 for the remaining systems (pools/AI/render/network/UI/tooling): what later games raised or changed, and whether Reach can do the same. Raw data `decompile-exports\topics\crossgame-pools-2026-11-15.csv`. |
| Top-10 ranked next work | `project-docs\TOP10-NEXT-UPGRADES-2026-11-15.md` | The ten most valuable not-yet-done items with size/risk/dependencies and cross-game evidence; Tyler chose **#1 then #2** and #1 is in progress (2026-11-15 13:15 ET). |
| Network handoff (Claude) | `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md` | Boundary note: player counts, pinned engine-limit profile (join-path + backend `limits_id` gaps, section 6), replicated appearance, session/security work for Claude. |
| Extended limits re-enable (item #1) | `project-docs\NOTE-EXTENDED-LIMITS-REENABLE-2026-11-15.md` | 2026-11-15: probe disproved the 11-14 black-screen/kick claim, Tyler pinned Full+1024 for all installs, no pickers; per-profile save folders; evidence tables + remaining tests. |
| Cross-game pool raises (item #2) | `project-docs\NOTE-POOL-RAISES-2026-11-15.md` | 2026-11-15: squad 256 -> 512, objectives 64 -> 125 on the pool hook (live measurements), layout generation 1 in the network id, tests + follow-ups. |
| Player-count expansion (item #3) | `project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md` | 2026-11-15: hold lifted, lane widened; 32-player patch installs and pools run at 32; normal-slot LAN join works; high-slot blocker (host removes player 16) and next steps. |

These files are mirrored in the repo-side copy `D:\Project-Noble-Closed-Source\knowledge` (matching subfolders).

## Current state (2026-11-15 10:50 ET - v0.19.103 released; engine-upgrade phase opened)
- Repo `D:\Project-Noble-Closed-Source`: **`main` = `wip-merge` = `eea9868`**, tracked tree clean (the untracked
  asset/doc files from earlier sessions are still there - do NOT `git clean`), version **0.19.103**. **Release
  v0.19.103 is published and latest** (re-verified 2026-11-15 10:40 ET via the GitHub API: published 14:57:37Z,
  draft=false; base zip 374,494,102 B sha256 `81260682...`, Forge pack 684,304,362 B sha256 `be693143...`). The next
  release must be a NEW version (e.g. 0.19.104) and only when Tyler says "release it".
- **Engine-upgrade phase (DeepSeek, this session) has started.** All decompiles are finished: MCC Reach 27,578 fns;
  Xbox 360 retail v3 33,787 named; 360 dev-kit beta 22,317; MCC Halo CE 79,637 (finished 2026-11-15 10:19 ET, 2
  failed), Halo 2 39,362, Halo 3 20,444, ODST 21,484, Halo 4 36,876. Current step is starter-prompt phase 1: RTTI
  check on `haloreach.dll`, `decompile-exports\topics\mcc-address-table.md` started (5 proven rows), engine-state pool
  inspector, then free diagnostics. No `src` changes yet this session.
- **Holds / waits:** 128-player work is ON HOLD until Tyler lifts it; release/version bump/tag/public assets only on
  Tyler's "release it"; two-PC lobby-code test (task-3) and Tyler's campaign-skulls retest still pending Tyler.
  Multiplayer is last.
- **Hex-number UI bug** ("0000001c" / "Respawn in 00000001"): still UNPROVEN. Static candidate `FUN_1802d288c`
  (`hr+0x2D288C`) leaks the 8 hex digits on its failure path; the solo MP/campaign read-only hook never saw it fire
  (`project-docs\NOTE-HEX-BUG-RUNTIME-CHECK-2026-11-15.md`). Needs the exact repro screen/mode.
- **Phase-2 Stage 1 is done (2026-11-15 11:35 ET).** Report: `project-docs\STAGE1-STATUS-REPORT-2026-11-15.md`;
  network/security boundary: `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`. Repo now `main` = `wip-merge` =
  `d636b31` (test-harness fix; version still 0.19.103, no release). Key facts: pool inspector proven in-match (124
  pools + 8 banks); heavy Firefight/Forge readings are in the report; **Engine Limits (V16/V16P/Full) are force-
  disabled in this build** (commit 4407101 - settings reset + non-Forge forced Stock) so count raising needs the
  Stage 3 black-screen fix plus re-enabling; -horde/-spawn-test were dead since 2026-11-09 and now work (`d636b31`).

- **Stabilization pass done (2026-11-15 12:05 ET, feature work paused):** 16-scenario regression (campaign,
  Firefight, multiplayer, Forge, Armory, server browser + lobby share-code UI, social, saved-film playback, save
  write; default and max-settings attempt) on v0.19.103 + the harness fix = **0 real crashes**. Crash-capable UI
  options clamped/hidden (`b2065bd`): campaign MAX PLAYERS = 4 only, multiplayer picker capped at 32, ENGINE LIMITS
  information-only Stock. Both gates rebuilt; version still 0.19.103, nothing published. Recommendation: hold the
  release until real feature/fix work lands (see the release draft).

- **Documentation pass done (2026-11-15 12:40 ET):** modder guide (68 engine hooks + MCC RVAs + flags + tooling),
  player guide (Firefight/Marine/appearance), master upgrade list now marks all 68 items PROVEN/FAILED/NOT STARTED
  (7/3/58) with MCC addresses, and the address table gained the proven MCC-only hook targets. Everything is
  mirrored to `knowledge`; the guides flag every item that could not be verified by test. No code changes;
  repo still `b2065bd`, version 0.19.103, nothing published.

- **Cross-game comparison + top-10 done (2026-11-15 12:55 ET):** Reach vs H3/ODST/H4 pool-constructor data shows
  later games raised AI caps (actor 256, squad 512, objectives 100/125) but not particle/render caps; H4 kept
  Reach's counts and grew per-entity records; no MCC engine exceeds 16 player slots. Ranked top-10 recorded;
  **no work started** - Tyler is choosing item numbers/order; work resumes on the top-10 list. Docs: `project-docs\CROSS-GAME-COMPARISON-2026-11-15.md`,
  `project-docs\TOP10-NEXT-UPGRADES-2026-11-15.md`.

- **Top-10 #1 and #2 COMPLETE (2026-11-15 14:15 ET, DeepSeek; #1 signed off by Tyler at 13:55):**
  #1 probe **disproved** the 11-14 "Extended limits black-screen / Firefight-MP kick" claim - the campaign black screen
  was the `145510a` BlamEngine launch-block regression, fixed in v0.19.95 `dad10d2`. Extended is now **pinned for every
  install** (Full = V16 + particles + light volumes, + 1024 AI actors), pickers removed, campaign saves per-profile
  (`saves\HaloReach_ext_full_a1024`; legacy Stock saves untouched) - commit **`0848280`**. #2 then raised **squad
  256 -> 512** and **objectives 64 -> 125** on the same proven pool hook (live campaign: squad 120/512, objectives
  28/125), and added **layout generation 1** to the profile network id (pinned id **`0x03010400`**) so different-layout
  builds refuse to join - commit **`8f0fe85`**. Both gates built clean; pinned-path runs in menu/campaign/Firefight/MP/
  Forge, plus Firefight/Forge with the raises, had 0 crashes. Version still 0.19.103, no release, private pushes only.
  Details: `project-docs\NOTE-EXTENDED-LIMITS-REENABLE-2026-11-15.md`, `project-docs\NOTE-POOL-RAISES-2026-11-15.md`.
  **Next:** Tyler lifted the 128-player hold and item **#3 (player counts)** is in progress (first 32-player milestone
  reached; high-slot blocker being traced). See `project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md`.

- **Remaining main-queue RE items from v0.19.103:** exact per-player visor colour (stock-slot fallback accepted; notes
  `project-docs\RE-VISOR-COLOR-2026-11-15.md`) and selectable armour-effect variants (recolour proven; Armory entries
  need engine effect-table work, `project-docs\RE-ARMOR-FX-COLORS-2026-11-15.md`).

## Current state snapshot 2026-11-15 05:10 ET (historical; superseded by the block above)
- Repo `D:\Project-Noble-Closed-Source`. **`main` and `wip-merge` both at `8db1ce8`** (pushed, working tree clean),
  version **0.19.102** and release **v0.19.102** published from `8db1ce8` (loading screen + server browser + all map-picker/visor/settings fixes) (base zip 357.1 MB + optional
  `Noble-ForgeWorld` 652.6 MB). Commits after the release are dev-only: `d422a74` server-browser details wrapping,
  `3a3c98e` FOV match-only, `5f99735` mod sections + campaign MAX PLAYERS + click hook, `8f467e1` banner polish +
  in-game visor mapping, `96fd2c4` Settings > Game Options slider alignment.
- **Current work queue (Tyler's ordered list, full detail in TODO-MASTER)**: camera-flicker docket; remaining items are
  (5/6) search + game-mode category filters inside expanded variant drawers, (8) server-browser Refresh/Favourite click,
  (9) "What's New (N)" hint click, (10) Game Options row clicks in Custom/Firefight/Forge, (11) controller toggle crouch,
  (12) M&K toggle zoom, (13) lobby share codes + prominent JOIN WITH CODE - CORE DONE `88d4937` (plan:
  `project-docs\PLAN-LOBBY-CODES-2026-11-15.md`), (14) engine-level custom visor colours - static RE done, notes in `project-docs\RE-VISOR-COLOR-2026-11-15.md`, (15) NEW armour-effect colour
  variants (flame/lightning), (16) multiplayer LAST (campaign 4 -> Firefight AI spawn multipliers -> 16 -> 128 players).
  **Read `docs\limits-players*.md` + `knowledge\drafts\draft-session-networking.md` before multiplayer work** - the
  32-slot work is already implemented (built, not tested).
- **New loading screen** (campaign / custom game / firefight / forge): top-left LOADING / WAITING FOR PLAYERS block with
  lobby roster and progress bar, top-right mode label, complete (uncropped) map picture over a soft copy, Reach
  "Auntie Dot" diamond grid behind the picture, Noble + UNSC emblems bottom corners. Host waiting for remote players now
  keeps this screen up instead of a black frame. Art lives in `assets\ui\noble\loading` (extracted from HREK, not
  committed). Details in the handoff.
- **Server browser**: blue-glass card layout (option 2) is the default; styles 1/3 remain switchable. Modded sessions
  show MOD / MOD - NOT INSTALLED, `MOD: <title>`, version warnings, DOWNLOAD MOD & JOIN (Workshop prompt verified).
- Upstream still lands big overhauls straight to `main` roughly hourly; always fetch and expect merges. Releases:
  v0.19.84-89 ours; v0.19.90/91 upstream base-only with our attached Forge assets; v0.19.92-100 upstream's; v0.19.101 ours.
- Test harness: `-NoFocus` everywhere (never takes Tyler's mouse); windows open on the AOC (second monitor); F5 needs
  focus, so ask first. Rule of the project: prove every cause with a one-variable test and a capture/log.
- **Docs mirror**: `C:\Users\tyler\Desktop\Compendium` and this `knowledge` folder are the same doc set in two places
  (Compendium is the hand-off copy Tyler/Claude read; knowledge is the repo-side copy). After editing either one, copy
  the changed files to the matching path in the other and keep both in sync. The 2026-11-15 DeepSeek phase writes in
  the Compendium and mirrors into `knowledge`.

## Folder index
| Folder | What's in it |
|---|---|
| `00-START-HERE.md`, `01-PROJECT-HISTORY.md`, `02-LESSONS-AND-RULES.md` | The compressed master documents (this set). |
| `project-docs` | Copies of the claude.ai project "project rea" docs (handoffs, findings, TODOs 11-10 .. 11-14, REFERENCE-REACH-MODDING-KNOWLEDGE). |
| `decompile-exports` | Decompile scripts, function indexes and `topics` (master upgrade list, started mcc-address-table, 4 topic chapters). Raw decompile text stays on the PC. |
| `pc-handoffs` | Handoffs that lived only on the PC: HANDOFF-ELITE-2026-11-12-NIGHT (big log through 11-14), -1730, NOTES-WING-FIX. |
| `repo-docs` | Copies of the repo's docs\ (limits-*, megapack, pelican, swarm, mod-map-mount-findings, collision-walk-crash, OVERNIGHT, MOD-BUGS, HANDOFF-CONTENT-2026-11-09, RR-mainmenu-decompiled). |
| `repo-docs\root` | Repo root docs: **HANDOFF.md** (220 KB, full history 10-28 .. 11-09 incl. netcode/master server/Armory v1), AGENT-STATUS, LIMITS-HANDOFF/TODO, AI-LIMITS-*, BUILDING, README, DEEPSEEK-START*. |
| `desktop-guides` | hex_editor_guide_v16 (haloreach.dll patch guide), halopedia_armor_pipeline, Noble-AI-Limits-Report, TODO-MASTER-2026-11-14. |
| `discord-compendium` | The community knowledge base built from the Discord exports + c20.reclaimers.net: **00_ULTIMATE_GUIDE** (Reach/H3 modding guide, parts A-E + networking), 00_FIELD_GUIDE_FINAL, 00_GUIDE_REACH_H3_MODDING, 00_INDEX, topic files 01-12 (shaders, cc maps, bitmaps, models/Blender, rigging, ripping tools, networking, scripting/AI/pathfinding, tools/links, Elites), notes\. |
| `test-tools` | Copy of every test/ManagedBlam/Reclaimer script from `...\claude-tests-2026-11-11\tools` (snapshot; the live copies stay there). |

## Things NOT copied (point to them)
- **The full external path index (Ghidra exports, decompile export folders, tag/asset tools, README): see
  `project-docs\REFERENCE-LIBRARY-2026-11-15.md`.** Quick list: extracted Reach tags `C:\Users\tyler\Desktop\ex`;
  tag packs `so many tags`, `spectre`, `Pelican`; tag editor `assembly`; Forge tools `Forgeworks-Halo-3-main`;
  variant tools `reach varient tool and editor`, `Reach game modes and maps`, `MVAR`; AI limits report on the Desktop;
  Ghidra decompile exports `Reach.exe\functions` and `host-probe.exe\functions`.
- **The decompiles themselves** (raw `.c` pseudocode, ~140 MB per engine): MCC Reach
  `D:\MCC HALO REACH DECOMP\decompiled`; Xbox 360 retail `D:\ReachX360\ghidra_retail\decompiled_v3` (**v3 only**);
  360 dev-kit `D:\ReachX360\ghidra\decompiled`; other MCC games `D:\MCC_DECOMP\{halo1,halo2,halo3,halo3odst,halo4}\decompiled`.
  Only indexes/scripts/notes are in `decompile-exports` (no raw decompile text in the Compendium).
- Raw Discord exports: `D:\discord chat\full` (e.g. `full\halomods`); compendium source: `D:\discord chat\compendium`.
- HREK (editing kit, tags, ManagedBlam, tool.exe): `D:\Steam\steamapps\common\HREK`. Noble's HREK edits + backups: `HREK\noble_mods`.
- RE workspace: the path `D:\REACH REVERSE ENGINEER` given in older notes does not exist (checked 2026-11-14). Original asset sets: `D:\Project-Noble-ORIGINAL\data\menumaps\objects\characters\{elite,spartans}`.
- Test folder (live tools, captures, logs): `D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11`.
- The claude.ai project "project rea" holds the same project-docs plus the original ULTIMATE-GUIDE PART1-3, FIELD GUIDE,
  COMPENDIUM-INDEX and DISCORD-HALOMODS-MATERIALS-SHADER-DIGEST (a raw excerpt; source = `D:\discord chat\full\halomods`).
- **Chat transcripts themselves:** earlier Claude sessions can't be read back; only what they wrote into these docs survives.
  If something is missing here, it was never written down.

## Keep it current
After each session: update `desktop-guides\TODO-MASTER-*.md` (or add a new dated one), append to `01-PROJECT-HISTORY.md`,
add new handoffs to `project-docs` or `pc-handoffs`, refresh "Current state" above, add anything new you learned about
external tools/paths to `project-docs\REFERENCE-LIBRARY-2026-11-15.md`, **and copy every changed knowledge file into
`C:\Users\tyler\Desktop\Compendium` (matching subfolder)** so the hand-off mirror is never stale. Record WHAT/WHY/HOW it
was verified (evidence path + commit) in both the handoff and TODO. For the engine-upgrade phase, also keep
`decompile-exports\topics` (master upgrade list, address table, topic chapters) and the packet docs current.

- Continuous work workflow for the remaining queue: `project-docs\WORKFLOW-CONTINUOUS-2026-11-15.md` + live ledger `project-docs\WORKFLOW-LEDGER-2026-11-15.md` (shared task board ids task-1..task-9; hourly continuation scheduled 09:30-22:30 ET).