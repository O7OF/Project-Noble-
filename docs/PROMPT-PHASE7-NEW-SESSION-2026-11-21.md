# Prompt for a new session: Project Noble Phase 7 - AI paths on custom and Forge maps (2026-11-21)

Copy everything below the line into a new session.

---

You are picking up **Phase 7 of Project Noble**: AI that can find its way around **custom and Forge maps** in Halo:
Reach. Project Noble is Tyler's (GitHub WeabooTyler117) Halo: Reach PC client. It hosts MCC's `haloreach.dll`. His
partner **O7** (O7OF) owns networking, the repo and the master server. The released build is **v0.20.34**
(2026-11-21). Phases 1-6 of the host-tools plan are done or parked on O7's server, and Phase 7 starts fresh here.

## 0. Rules (Tyler's, standing)

- **No** `Co-Authored-By`, `Claude-Session` or "generated with" lines in commits, PRs or docs, ever.
- Commit and push finished, built (both gates) and tested work to the **closed repo `main`**
  (`D:\Project-Noble-Closed-Source`). **No public release** (`tools\publish-release.ps1`) unless Tyler says so.
- Park only items that need sign-off on **O7's actual master server**. Everything else, decide and build.
- Test unattended on the AOC monitor without stealing focus (the harness has a focus guard).
- Back up and restore `build\bin\Release\data\userdata\` (backup: `data\userdata\claude-tests-2026-11-11\backup-r4`).
  Delete `host_*.json` and test game modes afterwards.
- Never commit RR data, logs or dumps. Network-side server changes are O7's.
- Keep the docs updated for the next agent (section 5).

## 1. Read first, in this order (all of it, before writing code)

Tyler wants the session to read **everything in `knowledge` and `docs`**, plus the Xbox 360 and MCC decompiles.
Read every `.md` / `.txt` file in full. For the large data (CSV exports, test tools, evidence, decompiled C),
list every file, read the indexes, and search the rest with `Select-String`. They are too big to read whole.

1. **The Phase 7 plan (written for this session):** `D:\Project-Noble-Closed-Source\docs\PLAN-PHASE7-AI-PATHS-2026-11-21.md`
   (also in the claude.ai project as `claude/PLAN-PHASE7-AI-PATHS-2026-11-21.md`).
2. **Project state and rules:** `D:\Project-Noble-Closed-Source\knowledge\00-START-HERE.md`, `01-PROJECT-HISTORY.md`,
   `02-LESSONS-AND-RULES.md`, `AGENT-BRIEF.md`, `AGENT-COORDINATION.md`, `HANDOFF-FINAL-2026-11-15.md`,
   `PROJECT-NOBLE-COMPENDIUM.md`, `NOTE-PLAYER-COUNTS-2026-11-15.md`; then every subfolder of `knowledge\`:
   `04-project-state`, `05-research-and-prompts`, `decompile-exports`, `desktop-guides`, `discord-compendium`,
   `docs`, `drafts`, `pc-handoffs`, `project-docs`, `repo-docs`, `test-tools`.
3. **Docs:** `D:\Project-Noble-Closed-Source\docs\` - `00-README-START-HERE.md`, `00-START-HERE.md`, `limits-ai.md`,
   `ANTI-REVERSING.md`, `collision-walk-crash.md`, and every subfolder: `01-guides`, `02-engine-reference`
   (address tables), `03-qa-and-release`, `04-project-state`, `05-research-and-prompts`, `desktop-guides`,
   `discord-compendium`, `drafts`, `evidence`, `pc-handoffs`, `project-docs`, `repo-docs`, `test-tools`.
4. **AI-specific research already done:** `knowledge\decompile-exports\topics\TOPIC-ai-pathfinding-objectives.md`
   (start here for AI), `topics\engine_source_map_x360_retail.csv`, `topics\mcc-address-table.md`,
   `topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`, `topics\crossgame-pools-2026-11-15.csv`, and the
   other `topics\*.md` files.
5. **Host-tools status:** claude.ai project doc `claude/TODO-HOST-TOOLS-AND-REMAINING-2026-11-20.md` (Phase 5b has
   the AI placement findings below in full) and `docs\HANDOFF-O7-HOST-TOOLS-NETWORK-2026-11-20.md`.

## 2. Decompiles and reverse-engineering sources

| What | Where | Notes |
|---|---|---|
| **MCC haloreach.dll, decompiled C** | `D:\MCC_DECOMP\MCC HALO REACH DECOMP\decompiled\hr_00000.c` .. `hr_00014.c` + `functions_index.csv` (rva,name,size,file,status) | Same build Noble loads (SHA-256 738dd2d2…). Image base 0x180000000; RVA = address - 0x180000000. Search here first. |
| MCC Ghidra project | `D:\MCC_DECOMP\MCC HALO REACH DECOMP\project\haloreach_full.gpr` | Opened by the `ghidra-reach` MCP helper (decompile by address, xrefs, strings). |
| MCC DLL copy / scripts | `...\MCC HALO REACH DECOMP\dll\`, `DecompileAll.java`, `run_decompile.bat` | The live DLL is `d:\steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll`. |
| MCC export index | `D:\Project-Noble-Closed-Source\knowledge\decompile-exports\mcc-haloreach\functions_index.csv` | |
| **Xbox 360 retail, decompiled C (with names)** | `D:\ReachX360\ghidra_retail\decompiled_v4\hr_00000.c` .. + `functions_index.csv`; symbols `D:\ReachX360\ghidra_retail\symbols.csv` | Real Bungie function names (`ai_*`, `path_*`, `sector_*`, `hs_*`). Match them to MCC by call shape, strings and constants. Same symbols: `knowledge\decompile-exports\x360-retail\symbols.csv`. |
| 360 retail Ghidra project | `D:\ReachX360\ghidra_retail\project_v3\reach_retail_v3.rep` | |
| Xbox 360 dev-kit beta (older build, more debug strings) | `D:\ReachX360\ghidra\decompiled_v3\`, `decompiled_v2\`, project `D:\ReachX360\ghidra\project\reach_dev.rep`; `D:\ReachX360\dev kit beta build\`; index `knowledge\decompile-exports\x360-devkit-beta\functions_index.csv` | |
| 360 binaries and maps | `D:\ReachX360\default.xex`, `default_base.exe`, the ISO, `map packs and dlc\`, `mcc_batches\`, `xextool\` | |
| Other Halo decompiles (cross-reference) | `D:\MCC_DECOMP\halo1`, `halo2`, `halo3`, `halo3odst`, `halo4`; `D:\MCC_DECOMP\DECOMP-QUALITY-CHECK-2026-11-18.md` | Halo 3 / ODST share Reach's AI and pathfinding lineage. |
| Tools | `D:\MCC_DECOMP\tools\rr\` | |

Ghidra "project locked": the Claude app sometimes starts the `ghidra-reach` helper twice, and the second copy holds
`NobleReach.lock`. Ask Tyler to stop the extra process by PID; killing it yourself needs his explicit naming.

## 3. Engine facts from Phase 5b that matter for AI (all proven in-game, 2026-11-21)

- **Game thread only.** AI calls read thread-local game state: root = `gs:[0x58][tls_index]`, `tls_index` at
  `haloreach+0xC17B18`. Root +0x30 = AI globals, +0x40 -> +0x50 = squad runtime records (0xEC each, team at +0xE7),
  +0x48 = game globals, +0x18 = players. Noble's slot-1 frame tick does NOT have this state; calls from there fault.
  SpawnTest's per-player control tick hook (`haloreach+0x61A38`, player 0, all builds) does:
  `ReachHost::GameThreadTick()` runs there (`src\engine\ReachHost.cpp`, `src\host\SpawnTest.cpp`).
- **Scenario:** qword `haloreach+0xC1A230`. Squads +0x398 / +0x39C (0x6C each, char[32] name); squad groups
  +0x38C / +0x390; script globals +0x43C / +0x440 (0x28 each). Tag addresses: page table
  `haloreach+0x4E39F20[packed >> 28] + packed * 4`. Tag instance table: qword `haloreach+0xC1A600`, 8 bytes per tag
  (u16 group, u16 salt, u32 address); datum = salt << 16 | index. Tag names come from the `.map` file's name
  buffer (see `LoadSquadTemplateNames`).
- **AI placement:** `ai_place` worker `0x6C2B10(ai, count, limbo)` -> `0x694CB4(squad, params)`; with a squad
  template datum in params (`PlaceTemplateDatum`) it spawns real actors. Per-actor migrate `0x6AB15C(actor, squad)`,
  per-actor team `0x6A7C38(actor, team)`, squad team `0x6C1C14(ai, team)`. Script function table: scan the DLL for
  the name string's pointer (name RVA = file offset + 0xE00); entry +16 = parser `0x180198F2C`, +24 = evaluator.
- **Script globals at runtime:** data array "hs globals" (found by name from the root; 8-byte records, short at +4);
  scenario global i = record (used - count + i). Helpers: `ScenarioGlobals`, `ShortGlobal`.
- **Spawn and teleport near the player:** SpawnTest horde code (`object_set_position` + refresh + actor teleported).
- `BlamEngine::ExecuteCommand` runs console commands only, never HaloScript AI functions.
- Forge World and most custom maps have **no squads, no AI pathfinding data (sectors / pathfinding mesh) and no
  objectives**. That is the core Phase 7 problem; the plan doc lists the options.

## 4. Workflow

- cmake is not on PATH: `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`.
  Build both gates: `cmake --build build --config Release --target noble-dev noble-client` (no new warnings).
- Test: `& .\tools\mp-session-test.ps1 -Players 2 -Map <map> -Mode <mode> -Tag <t> -Seconds N
  [-LimitsProbe 0] [-CaptureAt S] [-SettingsFile <json>] -ExtraHostArgs @(...)`. Run it with `&`, not
  `powershell -File` (that joins the args array). Logs: `build\bin\Release\host-<tag>-*.log`; captures:
  `data\userdata\claude-tests-2026-11-11\`. `-LimitsProbe 0` logs live pools (actors, squads).
- Dev options already in noble-dev: `-ff-dump-squads`, `-ff-place-squad/-at/-count/-template`, `-ff-allies`,
  `-ff-global name=value`, `-variant-poke off=val`, `-decode-variants`, `-horde N`, `-spawn-test N`, Numpad 6/7/8/9.
- Engine research: search the MCC decompiled C first, confirm in Ghidra, and name things from the 360 symbols.

## 5. Docs to keep current

- New: `claude/PHASE7-AI-PATHS-PROGRESS.md` (project) and `docs\PHASE7-AI-PATHS-PROGRESS.md` (repo). Record
  findings, addresses, decisions and test results as you go, written for the next agent.
- Update `docs\PLAN-PHASE7-AI-PATHS-2026-11-21.md` when the plan changes.
- `claude/TODO-HOST-TOOLS-AND-REMAINING-2026-11-20.md`: mark Phase 7 progress.
- `docs\HANDOFF-O7-HOST-TOOLS-NETWORK-2026-11-20.md`: only if network messages change.

## 6. First steps

1. Read section 1 and inventory section 2 (list every file; read the indexes).
2. Re-check the Phase 7 plan's options against the 360 names (`ai_*`, `path*`, `sector*`, `pathfinding*`,
   `objective*`, `firing_position*`, `cs_*`) and the MCC addresses. Decide the approach: e.g. generate pathfinding
   data for Forge maps, use the engine's own fallback, or simple direct steering for Noble-placed AI. Write the
   decision and reasons into the progress doc.
3. Prove the smallest working step on Forge World (one AI that moves toward a player without falling through
   the map), then build up. Commit and push each tested piece to the closed `main`.
