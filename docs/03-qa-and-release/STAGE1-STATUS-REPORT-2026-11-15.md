# Stage 1 status report - engine-prompt proven/unproven + pool inspector readings (2026-11-15)

Owner: DeepSeek (lead). Stage 1 of `PROMPT-DEEPSEEK-PHASE2-FEATURES-2026-11-15.md`. Scope excludes networking
and security (see `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md` - that boundary is Claude's).
Status written 2026-11-15 11:35 ET, one stage at a time - Stage 2 has not been started.

Build used for every measurement: **v0.19.103**, repo `D:\Project-Noble-Closed-Source` at release commit `eea9868`
plus the local test-harness fix `d636b31` (see `3; version unchanged - no release, no bump, nothing published).
Runs: `noble-dev.exe` on the AOC (second monitor), launched unfocused (`NOBLE_WINDOW_SECONDARY=1`, SW_SHOWNOACTIVATE),
settings muted and restored around each run. Evidence logs:
`build\bin\Release\stage1-firefight.log`, `stage1-forge.log`, `stage1-ff-trial.log`, `stage1-tickdiag.log`;
runner `build\bin\Release\stage1-run.ps1`.

## 1. Phase-2 precondition check (the new prompt says "use only when Phase 1-3 are done")

| Precondition | State 2026-11-15 11:35 | Evidence |
|---|---|---|
| Address table started | **YES** - 5 proven retail -> MCC rows + open candidates | `decompile-exports\topics\mcc-address-table.md` |
| Pool inspector working | **YES - proven in-match today** (124 named pools + 8 game-state banks, live, in Firefight and Forge) | `src\host\PoolReport.cpp`; `stage1-*.log` |
| Team/spawn hooks proven | **NO** - phase 3 not started (team/spawn functions still unproven retail->MCC matches) | `HANDOFF-DEEPSEEK` status table |

So the low-level tools Stage 1 needs are working; the hook work that Stages 2-4 will use is not done yet. That does not
block this status report, but it does block raising count options later.

## 2. What from the engine prompt is proven, and what is not

| Engine-prompt item | Status | Evidence |
|---|---|---|
| T3 pool inspector / limit audit | **PARTLY PROVEN**: inspector runs in-match and reports all pools + banks; alloc-failure logging (B1) NOT done | `PoolReport.cpp`, today's logs |
| Decompile inventory complete (all MCC engines) | **PROVEN** (counts re-verified on disk 2026-11-15 10:40 ET) | MC Reach 27,578 fns; 360 retail v3 33,787 named; dev-kit 22,317; CE 79,637/2 failed; H2 39,362; H3 20,444; ODST 21,484; H4 36,876. DLL SHA-256 `738dd2d2...0894` matches docs |
| Armour-effect recolour proof | **PROVEN earlier** (`64e6d22`, blueflame captures), not re-run today | `WORKFLOW-LEDGER` 08:35 entry; `RE-ARMOR-FX-COLORS-2026-11-15.md` |
| Visor stock-slot fallback (no white) | **PROVEN shipped** (`96fd2c4`); exact per-player colour NOT proven | `01-PROJECT-HISTORY.md` Phase 13 correction |
| T1 exact per-player visor colour | **NOT PROVEN** - map/HREK tint route closed; shared-heap/engine-visor-table route open, needs Tyler's scope decision | `RE-VISOR-COLOR-2026-11-15.md` |
| T2 hex-number UI bug | **NOT PROVEN** - static theory, runtime check did not reproduce; needs exact repro screen/mode | `NOTE-HEX-BUG-ROOT-CAUSE` / `NOTE-HEX-BUG-RUNTIME-CHECK` |
| T4 free diagnostics (net overlay, AI draw, minidump) | NOT STARTED | - |
| T5 AI tunables (path iteration cap, tasks) | NOT STARTED | - |
| T6 Armory/lighting (elite lighting, selectable FX variants) | NOT DONE (recolour proof only) | `RE-ARMOR-FX-COLORS` |
| T7 128 players | **ON HOLD by Tyler**; Tyler reports MP stopped at 34 players (Stage 2 will measure) | this prompt, Stage 2 |
| T8 cross-game comparison | Decompiles finished; **first pool-system comparison done today** (`6) | this report |
| T9 RTTI/vtable check on haloreach.dll | NOT CHECKED | - |
| T10 tooling roadmap | NOT STARTED | - |
| Team/spawn/native hooks (phase 3) | NOT PROVEN (precondition above) | - |

## 3. Test-harness bug found and fixed while producing these readings

The heavy-load harness was dead since 2026-11-09 and nothing in the docs noticed:

- `-horde`, `-spawn-test`, dev cheats and the cinematic auto-spawn gate all hang off the per-player tick hook
  `FUN_180061a38`. Ghidra shows the real signature is `(uint unitHandle, int playerIndex, uint c, float dt1, float dt2)`
  - the player index is the **second** argument (it is forwarded to `player_control_update`). Commit `7f31cf2`
  (O7OF, 2026-11-09) gated the body on the **first** argument, so the whole tick body never ran.
- Evidence: a temporary tick log printed `a=-328204288, b=0` (first arg is a unit handle; player index is 0). After
  the fix a `-horde 100` run logs `queued 100 actors` and a `-spawn-test 1500` run spawns 1,495 objects.
- Fix: `src/host/SpawnTest.cpp` only, +6/-2, gate on the second argument and pass it to `DevCheats::GameTick`.
  Commit **`d636b31`** on `main` + `wip-merge` (private repo, pushed). Both gates built clean
  (`noble-dev.exe` 11:19:51, `noble-client.exe` 11:19:56). Two full runs after the fix had no `FATAL`/`EXCEPTION`.
- This matters for Stage 2/4: the tools needed to stress AI/objects now actually work again.

## 4. Blocking finding: extended engine limits are force-disabled in the current build

The engine prompt assumes Noble's raised-limit profiles (V16 / V16+Particles / Full) can be used. On v0.19.103 they
cannot:

- `src\common\Settings.cpp:253-261` - on every settings load, `extendedLimits` and `aiActors` are **reset to Stock (0)
  and force-saved** ("Non-stock engine limits cause blackscreens in Campaign and boots in Firefight" - O7OF commit
  `4407101`, 2026-11-14 17:44).
- `src\main.cpp:1708-1717` - even with a non-stock setting, every non-Forge launch is forced to `Stock` ("only Forge
  supports extended limits until stability patches land").
- Consequence: all measurements in this report are **Stock-profile**; `-ai-actors` is ignored (the pool-resize hook
  only runs on Extended). Empirically the log says `Engine limits: requested Stock, active Stock` even when settings
  were pre-set to Extended.
- The Stock game-state banks (RVA `0x27BF270`) are nearly full on purpose-built small sizes and report the **same
  used/capacity in both runs** (map-independent game-state allocations):

| bank | used | capacity | headroom |
|---|---|---|---|
| 0 | 125,992 | 131,072 | 5,080 B (96.1%) |
| 1 | 2,222,752 | 2,228,224 | **5,472 B (99.75%)** |
| 2 | 50,740 | 65,536 | 14,796 B |
| 3 | 3,572,556 | 3,604,480 | **31,924 B (99.1%)** |
| 4 | 4,760,712 | 4,784,128 | **23,416 B (99.5%)** |
| 5 | 5,124 | 65,536 | 60,412 B |
| 6 | 338,488 | 393,216 | 54,728 B |
| 7 | 0 | 589,824 | 589,824 B |

Tens of KB of headroom in the main banks means any pool/count raise needs the enlarged V16 banks - which the current
build refuses to activate. Stage 2 ("raise counts") and Stage 4 ("Firefight/AI counts") therefore cannot reach raised
caps until the blackscreen/network-kick root cause is fixed and the restriction removed. That work overlaps Stage 3
(black screen) and the networking boundary (network kicks in Firefight/MP) - see the handoff note.

## 5. Pool inspector readings (Stock profile, 124 named pools found)

Method: `-pool-report-exit N` logs `BANK|`, one `POOL|name|used|cap|recordSize|live` line per pool, then
`POOL-REPORT-DONE`. Heavy load was produced with the (now working) dev harness: `-horde` places AI squads, and
`-spawn-test` spawns copies of a placed weapon/biped on a grid. The scan is taken ~130-150 s in, after the load and
the hordes/objects exist. Full lists are in the logs; top fill percentages:

### 5a. Heavy Firefight - Unearthed (`ff_unearthed`, map 218), Firefight, difficulty 1, `-horde 100 -horde-team 0 -spawn-test 1000`

| Pool | used / cap | fill |
|---|---|---|
| path_influence | 2 / 2 | 100% (fixed pair, by design) |
| water physics cache | 85 / 96 | **88.5%** |
| page reference ranges | 7,178 / 8,379 | **85.7%** |
| tag resource cache page state | 7,178 / 8,379 | **85.7%** |
| script node | 16,138 / 20,125 | **80.2%** |
| character properties cache | 31 / 40 | **77.5%** |
| lock hierarchy nodes | 99 / 128 | 77.3% |
| object | 1,472 / 2,048 | **71.9%** |
| clump | 13 / 20 | 65.0% |
| sim. object glue | 1,225 / 2,048 | 59.8% |
| c_contrail_gpu::s_row | 81 / 160 | 50.6% |
| hs globals | 1,914 / 4,096 | 46.7% |
| tag resource cache / lruv reverse map | 3,636 / 8,380 | 43.4% |
| contrail / location / system / gpu | 65 / 160 each | 40.6% |
| multiplayer levels | 20 / 50 | 40.0% |
| cached object render states | 374 / 1,024 | 36.5% |
| (actor) | 31 / 96 | 32.3% |
| (effect / effect event / effect location) | 76 / 920, 76 / 1,800, 67 / 1,380 | 8.3 / 4.2 / 4.9% |

Notes: `-horde` reported "capped at the 96 free actor slots" then "placed no new actors after trying all 32 squads" -
Firefight's own wave manager owns the scenario squads, so the dev horde does not make a big live-actor count on this
map (31 actor records at scan). The actor count is wave-driven; the object pool was pushed by the 1,000 dev objects.
The near-full pools here are the **tag-page/script/cache pools and the character-properties cache**, not the actor pool.

### 5b. Heavy Forge - Forge World (`forge_halo`, map 3006), Forge, `-spawn-test 1500 -spawn-max 8000`

1,495 of 1,500 objects spawned (5 refused); object pool 1,567 / 2,048 at scan.

| Pool | used / cap | fill |
|---|---|---|
| havok components | 1,500 / 1,500 | **100%** |
| water physics cache | 96 / 96 | **100%** |
| path_influence | 2 / 2 | 100% (fixed pair) |
| page reference ranges | 12,119 / 13,321 | **91.0%** |
| tag resource cache page state | 12,119 / 13,321 | **91.0%** |
| sim. object glue | 1,599 / 2,048 | 78.1% |
| lock hierarchy nodes | 99 / 128 | 77.3% |
| object | 1,567 / 2,048 | **76.5%** |
| tag resource cache / lruv reverse map | 9,907 / 13,322 | 74.4% |
| script node | 2,817 / 4,566 | 61.7% |
| campaigns | 1 / 2 | 50% |
| lock hierarchy links | 479 / 1,024 | 46.8% |
| hs globals | 1,754 / 4,096 | 42.8% |
| tag resource cache control stat | 12,124 / 29,720 | 40.8% |
| cached object render states | 363 / 1,024 | 35.4% |

Notes: Forge World has no AI squads (actor/prop/squad = 0, expected). The first ceiling for object-heavy Forge is
**havok components (1,500/1,500)**, not the object pool; the object pool and the tag-page pools are next. The capacity
of some pools is map-dependent (page reference ranges 8,379 on Unearthed vs 13,321 on Forge World; script node
20,125 vs 4,566), so a "safe maximum" must be measured per map family, not assumed global.

## 6. Cross-game comparison required by the prompt (limits/pools): Halo 3, ODST, Halo 4

Method: string/decompile scan of `D:\MCC_DECOMP\{halo3,halo3odst,halo4,halo2}\decompiled\` (Ghidra text), looking for
Reach's data-array header signature `0x64407440` and a set of distinctive Reach pool-name strings.

| Game | Reach data-array header signature (0x64407440) | Data-array implementation seen | Distinctive Reach pool names present |
|---|---|---|---|
| Halo 4 | **YES** (1 match) | restricted-allocation-manager style, same family as Reach | megalo_objects, path_influence, sim. object glue, performance_runtime_definition, stimulus-data all present |
| Halo 2 | **YES** (1) | same family | partial (cached object render states, tag/level names differ) |
| Halo 3 | **NO** | vftable-based data-array ctor, e.g. `FUN_18013003c("test data array", 0x65, 4, 0, &vftable)` | lacks megalo_objects, path_influence, sim. object glue, performance_runtime_definition, stimulus-data |
| ODST | **NO** | same vftable-based ctor as H3 | same as H3, plus squad_patrol |

What changed vs Reach: Reach and H4 (and H2) allocate fixed pools through the same restricted-allocation-manager
header (name / record size / 0x64407440 signature / capacity / bitmap), so Reach's pool header scan and by-name
resize hook are portable to H4. H3/ODST use a different, class/vftable data-array constructor and lack several
Reach-only pools; the Reach scanner/patch does **not** transfer there and would need its own allocator hook.
Caveat: this is string/decompile evidence, not a runtime comparison - treat as a starting map, verify before porting.

## 7. Needs Tyler

1. **Stage 2 go/no-go and inputs.** Stage 2 is preparation: reproduce the 5-player campaign crash and the 34-player
   multiplayer stop, then clamp UI rows to proven counts. I need the exact repro paths you used (map/mode/menu path,
   host vs LAN vs server) to reproduce fast.
2. **Stage 3 clarification.** "Noble's illumination and particle effect options set to max" - are those the ENGINE LIMITS
   options (STOCK / EXTENDED (V16) / EXTENDED + PARTICLES / FULL (V16 + PARTICLES + LIGHT VOLUMES))? In v0.19.103 they
   are force-reset to Stock (`4), so the black screen cannot be coming from them in this build unless your build differs.
   The exact build + steps + map are needed before Stage 3.
3. **128-player hold** still stands; nothing in Stage 1 touched it.
4. **Release:** none. No version bump, no public push, no tag, no assets (and the rules keep it that way until you say
   "release it").

## 8. Needs Claude (networking / security boundary)

Recorded in `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`: player-count expansion session/replication work, the
extended-limit profile matching that causes Firefight/multiplayer kicks, replicated appearance for exact visor colour,
host migration/128-player session work, and all security items. DeepSeek stops at that boundary.

## 9. Next step (after Tyler's go) - Stage 2 preparation only

1. Reproduce the 5-player campaign failure: map/mode/path, load vs in-play, crash vs black screen, log + pool inspector
   readings at failure; check whether it is the same black screen as Stage 3.
2. Find what stops campaign >4 and multiplayer at 34: which session/membership check, pool, array or UI limit
   (name -> hr+RVA for non-network parts); fix non-network causes; list network blockers in the handoff note.
3. Clamp the campaign MAX PLAYERS row to the highest count proven to work.
4. Do not change session/membership/replication code; do not start 128-player work.
