# Project Noble - documentation pack (snapshot 2026-11-15)

This folder is a **copy** of the Project Noble documentation set, assembled on 2026-11-15 for sharing. The originals
remain where they are (`C:\Users\tyler\Desktop\Compendium\` and the project's `knowledge\` mirror) and stay current;
this pack is a frozen snapshot for sending to someone else.

No game assets, no binaries, no raw decompile text are included - only Markdown notes and tables.

## Build/state this snapshot describes

- Project Noble v0.19.103 (closed-source Halo Reach MCC community client), commit `b2065bd` on the private repo.
- Engine research is based on MCC `haloreach.dll` build 1.3528.0.0 (SHA-256 `738dd2d2...0894`), the Xbox 360 retail
  Reach decompile (named from the Omaha map) and all other MCC engine decompiles finished 2026-11-15.
- Verification language used throughout: **PROVEN** = demonstrated by a test/runtime probe (MCC address given where
  known); **FAILED** = attempted and ruled out/reverted; **NOT STARTED** = not implemented (partial foundations named
  in the master list); **UNVERIFIED** = code/docs exist but no test evidence.

## Contents

### 01-guides - start here
| File | What it is |
|---|---|
| `MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md` | Every engine hook/patch Noble applies (68 detours with MCC RVAs measured from a live startup log; what/when/example/limits), the host tooling modules (pool inspector/resize, engine limits, AI/object stress harness, capture, map-variant tools), launch/test flags (including a complete 216-entry flag appendix), the 38-rule game-variant editor and its JSON format, appearance/profile data, and an explicit "could not verify by test" table. |
| `PLAYER-GUIDE-FIREFIGHT-MARINE-APPEARANCE-2026-11-15.md` | Player-facing guide: Firefight setup/lobby/options, share codes, the Armory Marine tab, visor/colour/armour-effect/voice/appearance options, settings, known issues. |

### 02-engine-reference
| File | What it is |
|---|---|
| `MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md` | The master list of possible engine upgrades/fixes/builds (68 items across limits, stability, networking, rendering, AI, gameplay, tooling), each now marked PROVEN / FAILED / NOT STARTED with MCC addresses where proven. |
| `mcc-address-table.md` | Retail function -> MCC `hr+RVA` matches that are proven, open candidates marked UNPROVEN, and a table of proven MCC-only hook targets (pool init, bank table, AI/object spawn, saved film, etc.). |
| `README-decompile-exports.md` | Index of the engine decompiles: where the raw text lives on Tyler's PC, function counts, how to search/re-run, and the v3 retail-decompile caveat. |
| `TOPIC-networking-multiplayer.md`, `TOPIC-ai-pathfinding-objectives.md`, `TOPIC-armor-appearance-colors-armory.md`, `TOPIC-lighting-rendering.md` | Topic chapters for the engine subsystems, written from the retail names/decompiles. |

### 03-qa-and-release
| File | What it is |
|---|---|
| `QA-VERIFY-2026-11-15.md` | QA/release verification log incl. the full 16-scenario stabilization regression (campaign, Firefight, multiplayer, Forge, Armory, lobby codes, saved film, save-write; default and max-settings attempts). |
| `STABILIZATION-REPORT-2026-11-15.md` | Change list with commits and whether each change is test-proven, the regression matrix and evidence, the crash-option clamps, and coverage gaps. |
| `STAGE1-STATUS-REPORT-2026-11-15.md` | Engine-prompt status report: what is proven/unproven, live pool-inspector readings on heavy Firefight/Forge maps, the engine-limits constraint, and the H3/ODST/H4 pool-system comparison. |
| `RELEASE-DRAFT-AFTER-0.19.103-2026-11-15.md` | Draft release notes, known-issues list and a ship-vs-hold recommendation. **Not published** - included here as planning context. |

### 04-project-state
| File | What it is |
|---|---|
| `00-START-HERE.md` | Index of the whole knowledge folder, read order, current project state and pointers. |
| `TODO-MASTER-2026-11-14.md` | The live to-do/queue history (Tyler's ordered items) plus the open issues/known limitations. |
| `HANDOFF-FINAL-2026-11-15.md` | The prior session's final handoff (v0.19.103 release state, proven/unproven summary, release/QA rules). |
| `HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` | The full briefing for the engine-upgrade work (where every decompile lives, the RE method, task priorities, pitfalls, session log). |
| `WORKFLOW-LEDGER-2026-11-15.md` | Chronological engineering log of the 2026-11-15 sessions. |
| `REFERENCE-LIBRARY-2026-11-15.md` | What external tools/paths exist on Tyler's machine and when to use them (Ghidra MCP, decompile exports, HREK, tag tools, test harness). Some paths are machine-specific. |

### 05-research-and-prompts
| File | What it is |
|---|---|
| `RE-VISOR-COLOR-2026-11-15.md` | Visor-colour reverse-engineering notes: the appearance path, five real engine visor slots, shareable material experiments, and what was ruled out. |
| `RE-ARMOR-FX-COLORS-2026-11-15.md` | Armour-effect recolour research: the palette/tag route, the successful blue-flame proof, and why selectable variants need more engine work. |
| `NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md`, `NOTE-HEX-BUG-RUNTIME-CHECK-2026-11-15.md` | The "numbers shown as hex" UI bug: static root-cause candidate, the runtime check that did not reproduce it, and candidate fixes. |
| `NOTE-MCC-HALOREACH-FULL-DECOMP-2026-11-15.md`, `NOTE-REACH-X360-RETAIL-AND-OMAHA-MAP-2026-11-15.md` | Notes on the completed MCC Reach / Xbox 360 retail decompiles and the Omaha symbol map. |
| `PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md`, `PROMPT-DEEPSEEK-PHASE2-FEATURES-2026-11-15.md` | The internal starter prompts/phase plans for the engine-upgrade work. **Internal working instructions** - included for completeness, not a public spec. |
| `NOTE-O7-REQUESTS-2026-11-15.md` | Community-dev feature requests recorded and mapped to phases/holds. |
| `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md` | The networking/security boundary note (what belongs to the other engineer). |

## Caveats for the person receiving this

- These are **engineering notes**, not official documentation or a mod API. Anything about hooks/flags is
  version-specific to the `haloreach.dll` build above and can change with a game update.
- The master list and QA logs deliberately separate **PROVEN** from **UNVERIFIED**. Do not treat the NOT STARTED or
  UNVERIFIED items as features.
- Some paths in the notes are absolute paths on Tyler's PC and some refer to a private repository; they are left in
  place so the notes stay consistent with the originals. There are no game files, DLLs, saves or keys in this pack.
- Single-player/custom-game experimentation only; do not use the test hooks in public/competitive sessions.
