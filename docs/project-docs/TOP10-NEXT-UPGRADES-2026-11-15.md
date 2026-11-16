# Top 10 most valuable things not yet done (ranked 2026-11-15; re-ordered 2026-11-16)
## REVISED ORDER - 2026-11-16 (Tyler): multiplayer first, campaign last

The ranking below is superseded for prioritisation. New order: **#1 = multiplayer large sessions** (16-player end-to-end,
then the high-slot peer-17/player-16 join fix, then 32 / 64 / 128; owned by a new DeepSeek session - see
`project-docs\PROMPT-MULTIPLAYER-AGENT-2026-11-16.md`), then the remaining items in the order: Firefight AI/wave options,
native team/spawn hooks + mod API, navmesh baking, exact per-player visor colour, armour-effect variants, Elite
lighting, Forge/variant/Megalo tooling, and **last** campaign co-op > 4 (16-player campaign, **parked**).
Items #1 (Extended limits) and #2 (cross-game pool raises) below are already complete; the old #3 is split: the
multiplayer half is the new #1, the campaign half is parked last.



Reviews the whole master upgrade list (`decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`,
68 items: 7 PROVEN, 3 FAILED, 58 NOT STARTED) plus the engine-prompt task list and the Phase-2 plan, and ranks the best
remaining work by value. Evidence for the cross-game statements is in `CROSS-GAME-COMPARISON-2026-11-15.md`.

**Status 2026-11-15 13:40 ET (supersedes "nothing started"):** Tyler chose **#1 then #2**. Item #1 is
implemented as commit **`0848280`** (private push, version still 0.19.103, no release): the read-only probe
**disproved the 11-14 "Extended limits black-screen/kick" claim** (it was the `145510a` launch-block regression,
fixed in v0.19.95), and the client-side change set for a **pinned Full + 1024 AI actors profile (no pickers,
per-profile save folders)** is in. Both gates built clean and pinned-path end-to-end passed in menu/campaign/
Firefight/MP/Forge (0 crashes). Details + evidence: `project-docs\NOTE-EXTENDED-LIMITS-REENABLE-2026-11-15.md`.
**#1 COMPLETE (2026-11-15 13:55 ET, Tyler):** code landed as `0848280`, gates clean, five-mode pinned-path
verification passed; remaining items (campaign save->load round trip, 30-minute acceptance runs, Claude's join-path
gaps, old-save notice) are follow-ups/known issues, not blockers.

**#2 COMPLETE (2026-11-15 14:15 ET):** cross-game pool raises landed as `8f0fe85` - **squad 256 -> 512**
(ODST precedent) and **objectives 64 -> 125** (ODST), measured live on campaign m10 (squad 120/512, objectives
28/125); actor 96 -> 256 was already exceeded by the pinned 1,024 actors. Layout generation 1 added to the profile
network id (`0x03010400`) so different-layout builds refuse to join. Campaign/Firefight/Forge runs 0 crashes; gates
clean. Follow-ups: long heavy-Firefight soak, final save/film layout check before release.

**#3 IN PROGRESS (2026-11-15 14:35 ET):** Tyler lifted the 128-player hold and widened the lane - DeepSeek owns the
session/membership/replication work for this item too (Claude unavailable). First milestone reached: on the pinned build
the 32-player patch **installs fully** (110 slot sites, 23 masks, 139 bounds, membership message #32) and the pools run
live at **players/fire team/simulation-view 32**; a two-instance LAN join at normal slots succeeds (host 2 peers/2 players,
pmask 3, guest decodes the 0xA300 message and acks msg 35, msg 44 stream, 0 crashes). **High-slot (peer 17/player 16) still
fails**: the host removes the high-slot player right after the ack (producer 2 players -> 1 player), closes reason 10, the
guest gets CONNECT REFUSED reason 3. Next step is the host-side player-add/remove trace.
**Campaign co-op >4 (5 local instances, low-spec settings):** the >4 session path works (host logs "co-op roster is
over 4; creating the session with capacity 8") and a 4-player m10 session ran live; on Exodus (m50) **all five players
formed the same co-op session** (pmask 0x1F, peers p0-p4 state 8, 0 timeouts/crashes) but every instance then exited at
the peer-ready point (EngineEvent reason=0) without starting the mission - session formation proven, in-mission 5-player
still to fix. Details: `project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md`. No release authority; version stays 0.19.103.

**Size** = rough effort for one agent: S <= 2 days, M = 3-5 days, L = 1-2 weeks, XL > 2 weeks.
**Risk** = Low = contained/testable; Med = engine behaviour, needs probes + save/film/QA checks; High = crash/desync/
networking or unknown RE. Every item still follows the project rule: read-only probe first, then hook, then test.

| # | Item (master IDs) | What it delivers | Value | Size | Risk | Depends on | Cross-game evidence |
|---|---|---|---|---|---|---|---|
| 1 | **Stabilize and re-enable the Extended engine-limit profiles** (B4/B6; blocker behind A1-A3) | Brings back V16 / V16+Particles / Full safely, so the already-proven pool raises (actors, objects) can ship. Directly addresses the "max illumination/particles black screen" and the Firefight/MP network-kick reason that made upstream disable the profiles (`4407101`). | Highest - it unlocks the whole limits roadmap and Tyler's max-settings feature; currently all heavy limit work is dead-ended. | L | High | Stage 3 black-screen repro; Claude for the network-profile matching (join enforcement) | H4 is the same allocator family and runs with richer per-entity records; no later engine shows the black-screen/limit profile mechanism, so this is Reach/Noble-specific stability work, not a port. |
| 2 | **Ship the cross-game-justified pool raises** (A1/A2 partly; master additions) | Actor pool 96 -> **256** (H3/ODST precedent), squad 256 -> **512** (ODST), objectives 64 -> **100-125** (H3/ODST). Measure with the pool inspector; keep save/film/network sizes consistent. | Very high - it is the concrete way to make heavy Firefight/AI scenes work, and later-game data proves the numbers are sane. | S-M (table changes + tests) | Med | #1 (profiles), boundary with Claude for replicated sizes | Direct: H3/ODST actor 256, ODST squad 512, H3/ODST objectives 100/125. Reach's proven hook already held 1,024 actors in tests. |
| 3 | **Player-count expansion 32 -> 64 -> 128 and campaign co-op > 4** (A5; Phase-2 Stage 2; Tyler's hold) | Real sessions above 16 seats; fixes the 34-player stop; makes the campaign MAX PLAYERS row honest. | Highest user-visible value if the hold is lifted, but it is the largest project on the list. | XL | High | Tyler lifting the 128 hold; Claude owns session/membership/replication; #1 for profile matching | **No later MCC engine has >16 player slots** (all 16; Halo 4 grew the per-player record 3.3x instead), so this is original architecture work, not a port. |
| 4 | **Firefight options: enemies per wave/dropship, friendly AI count, wave pacing** (Phase-2 Stage 4; E8-ish) | Host options for Firefight enemy density, allied AI and pacing, with safe clamps; ties into the working AI/object harness. | High - Tyler asked for this explicitly; it makes Firefight customisable and uses the AI pool work from #2. | L | Med | #1/#2 (pools), Firefight wave/squad data (tag-driven, needs verification) | H3/ODST raised actor/squad capacity 2.7-2x; H4 kept Reach's counts. No later game exposes wave options, so the feature is original but the capacity headroom is evidenced. |
| 5 | **Native team/spawn/round hooks + a documented mod API** (O7 request; Phase-3 hooks) | Exposes the engine's team change/swap, spawn/round and results functions as tested hooks, then documents them (modder guide updated). | High for the modding community; O7's request; makes Noble a platform. | L | Med | Phase-1 address work (done/ongoing); read-only probes; tests | Cannot be compared cross-game (H3/ODST/H4 have no symbols); the retail-named Reach decompile is the reference. |
| 6 | **Pathfinding/navmesh baking for custom and Forge maps** (E10/G6; Phase-2 Stage 8) | AI that can walk custom/Forge maps without per-map scripting; offline nav data + a "bake pathfinding" action + default objectives. | Highest modder value on the list (unlocks AI on any map), but the largest and least certain. | XL | High | Engine path/sector format RE; map/variant sidecar format; #4 for objectives | No evidence in the other decompiles (no "navmesh"/"nav_mesh" strings anywhere; symbol-less) - **UNVERIFIED**; H4 would be the only useful comparison but needs deeper RE. |
| 7 | **Exact per-player visor colour in matches** (master F7 remainder; Phase-2 Stage 6.1) | Real per-player custom visor RGB instead of the nearest stock slot, visible to all peers. | High user-visible value; the stock-slot fallback is a known compromise. | M-L | High | Engine/shared-cache visor-table route (RE); if replicated, Claude's networking lane | Appearance is tag/appearance-record driven in all engines; pool data says nothing. Existing Reach RE notes remain the reference - **UNVERIFIED** for later-game help. |
| 8 | **Selectable armour-effect colour variants** (master F7/T6; Phase-2 Stage 6.2) | Blue/purple/etc. flame and lightning variants selectable in the Armory, replicating to peers. | Medium-high player value; the recolour itself is proven (blue-flame proof, `64e6d22`). | M | Med | Tag/effect-table work; Forge World rebuild for tests; possibly replication | H3/ODST/H4 have no usable cross-game data (appearance is tag data) - **UNVERIFIED**; the project's own recolour proof is the starting point. |
| 9 | **Elite lighting/shine** (master D1; Phase-2 Stage 6.3) | Fix dark/flat Elite gameplay lighting and Armory preview (cinematic SH lighting / object lights / texture-camera constants). | Medium - visible Armory/Elite quality; a long-standing parked item. | M-L | Med | Rendering-constant RE; Armory capture rig (exists) | Rendering constants/pools show Reach = H4 for lights; no later-game feature evidence in the decompiles - **UNVERIFIED**; must be found in Reach's own code. |
| 10 | **Forge++ / map-variant / Megalo tooling package** (G1/G2/G3) | External map-variant viewer/editor, Megalo variant tooling, Forge budget/overlay improvements - the modder toolchain. | Medium-high for creators; builds on the existing `mvar` decode/encode + `-mvar-*` tools. | L-XL | Med | #6 (maps) partly overlaps; format RE; safe write path | Reach and H4 have Megalo (Reach's megalo pool 512 vs H4 256; H3/ODST absent); H4's variant format is the only sibling reference. Partially **UNVERIFIED**. |


## Starting points per item (read these before planning the chosen item)

| # | Item | Read first / where the background is | Key source / data / test hook |
|---|---|---|---|
| 1 | Extended limits | `STABILIZATION-REPORT-2026-11-15.md` `4, `STAGE1-STATUS-REPORT-2026-11-15.md` `4, `QA-VERIFY-2026-11-15.md`; master B4/B6 | `src\host\EngineLimits.*`, `data\limits\{v16,v16p,full}.nlp`, upstream commit `4407101`; server-side profile matching in `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md` |
| 2 | Cross-game pool raises | `CROSS-GAME-COMPARISON-2026-11-15.md` `2a + `crossgame-pools-2026-11-15.csv`; master A1/A5; `knowledge\repo-docs\root\AI-LIMITS-REPORT-2026-11-08.md` | `src\host\PoolResize.*`, `src\host\PoolReport.*`, `src\host\SpawnTest.*` (stress harness), test `tools\limits\test-limits.ps1` |
| 3 | Player counts | `docs\limits-players*.md`, `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md` `1, master A5, handoff `4 T7; `CROSS-GAME-COMPARISON` `2a/`2c | `src\host\SlotPatch.*`, `SlotMessage.*`, `src\hooks\LobbyPatches.cpp`, `src\host\PoolResize.cpp` (simulation view 16->32) |
| 4 | Firefight AI/wave options | `PROMPT-DEEPSEEK-PHASE2-FEATURES-2026-11-15.md` Stage 4; master E3/E8; `docs\limits-ai.md` | `src\host\SpawnTest.*` (horde/objects), Firefight wave/squad tags (VERIFY), pool names from `crossgame-pools CSV` (actor/squad) |
| 5 | Native team/spawn hooks | `PROMPT-DEEPSEEK-ENGINE-PHASES` Phase 3; `NOTE-O7-REQUESTS-2026-11-15.md`; handoff `3-`4 | retail names in `decompiled_v3`, MCC match via `hr_*.c`, record in `mcc-address-table.md`, probe via Ghidra MCP |
| 6 | Navmesh baking | `PROMPT-DEEPSEEK-PHASE2-FEATURES` Stage 8; master E10/G6; handoff `2/`3 | retail `path.obj`/`ai_sector_structures`/`path_influence_map` in `decompiled_v3`; H4 as comparison in `D:\MCC_DECOMP\halo4\decompiled` |
| 7 | Exact visor colour | `RE-VISOR-COLOR-2026-11-15.md` (whole note), master F7, Phase-2 Stage 6.1 | `src\common\ArmoryVisor.hpp`, visor patches in `src\hooks\EnginePatches.cpp` (`0x2EDC5E`, `0x3B2F4`), `ReachArmory.*` |
| 8 | Armour-effect variants | `RE-ARMOR-FX-COLORS-2026-11-15.md` (whole note), master F7/T6, Phase-2 Stage 6.2 | `src\ui\ArmorCustomizer.cpp` (effect ids), Elite-effect patches in `EnginePatches.cpp` (`0x10580C/0x105837`), `ForgePatches` |
| 9 | Elite lighting | master D1/D2, Phase-2 Stage 6.3; render pool evidence in `CROSS-GAME-COMPARISON` `2d | `src\engine\ReachArmory.*` (lights colour slots), texture-camera / SH-lighting retail names in master D1/D2 and `decompiled_v3` |
| 10 | Forge/variant/Megalo tooling | master G1/G2/G3; `src\mvar` module; `decompile-exports\topics\TOPIC-*` | `src\hooks\ForgePatches.cpp`, `src\engine\ForgePlacement.*`, `-mvar-*` / `-variant-*` flags in `MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md` |

**Also always:** `project-docs\REFERENCE-LIBRARY-2026-11-15.md` (external tools/paths), `MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md` (hook/flag mechanics), `MASTER-UPGRADE-OPPORTUNITIES` (item IDs + MCC addresses), `mcc-address-table.md` (proven addresses), and the current `QA-VERIFY`/`STABILIZATION-REPORT` for what is already tested.


## Honourable mentions (not in the top 10)

- **Alloc-failure logging + crash diagnosis** (B1/G7 extension) - **S, Low risk**, the cheapest high-leverage safety
  item; would make #1/#2/#4 much safer to iterate. Recommended to fold into #1 rather than run alone.
- **Path-iteration cap** (E1, `g_path_state_traverse_max_iteration_count`) - S-M, Med; cheap AI-performance win.
- **Respawn tunables** (B5) - S-M, Med; small gameplay fixes.
- **Motion blur / DoF / exposure toggles** (D4) - M, Med; pure Reach rendering work.
- **Hex-number UI bug** (F1) - unresolved; S-M but High uncertainty (no repro yet).
- **Marine/allied-AI mode built into Noble** (Phase-2 Stage 5) - L, High; depends on #4 and animation work.
- **Cached object render states raise** (D10) - **do not do**; Reach already has H4's 1,024 and the 8k/16k attempts
  crashed; no later-game precedent supports it.
- **Particle/decal/light capacity raises** (A2/A11/A12) - **do not prioritise**; Reach already equals H4 and exceeds
  H3/ODST for these.

## Recommendation

- If Tyler wants **one path**: start with **#1 then #2** - it is the only sequence that turns the already-proven pool
  research into shipped, measurable capacity wins, and it de-risks every later AI/Forge feature.
- #3 (players) needs the 128 hold lifted and Claude's session/replication work; do not start it first.
- #6 (navmesh) and #7 (exact visor) are the two highest-value "hard" projects; both start with a read-only RE
  investigation that can be time-boxed to a few days before committing.
- Fold the honourable-mention alloc-failure logging into #1.

**Nothing above has been started.** Reply with the number(s) Tyler wants and the order.