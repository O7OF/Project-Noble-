# Cross-game comparison - Reach vs Halo 3, ODST and Halo 4 (2026-11-15)

Scope: finish the comparison for the systems not covered by the earlier Stage-1 pass. Stage 1 already compared the
**pool/limit foundation** (data-array header signature, pool-name presence, live Reach pool report). This note covers the
remaining systems, compares Reach against Halo 3, ODST and Halo 4 from `D:\MCC_DECOMP`, records what the later games
raised or changed, and says whether the same change is possible in Reach. It feeds the ranked list in
`TOP10-NEXT-UPGRADES-2026-11-15.md`.

## Method (and its limits)

- Sources: MCC `haloreach.dll` decompile (`D:\MCC HALO REACH DECOMP\decompiled\hr_*.c`, 27,578 functions, named
  `FUN_*`) and the three other MCC engine decompiles: Halo 3 (20,444 functions), ODST (21,484), Halo 4 (36,876)
  (`D:\MCC_DECOMP\{halo3,halo3odst,halo4}\decompiled\hr_*.c`).
- **Key method:** every engine creates its fixed data arrays with a single constructor call whose arguments are
  literals: `FUN_xxx("pool name", count, record_size, 0, vtable)`. Extracting those calls gives a like-for-like
  table of **pool name -> capacity -> record size** per engine. This is the strongest quantitative evidence available
  (it is compiled-in data, not a guess).
- The extracted tables: Reach 110 pools, Halo 3 83, ODST 86, Halo 4 112. Raw data:
  `decompile-exports\topics\crossgame-pools-2026-11-15.csv` (name, count, record size, source file/line per engine).
- **Limits of this method:** Halo 3/ODST/Halo 4 have **no symbol names** (only `FUN_*`), so we can compare capacities,
  record sizes, pool presence and quoted string literals - not exact algorithms. Pool names differ between engines,
  so only same-name rows are true like-for-like; a missing name can mean "renamed", not "removed". Record sizes are
  struct sizes and differ by engine version/content, so a bigger record is evidence of more per-entity state, not a
  raised limit. String-literal scans are a crude presence signal only. Anything that needs a named function or a
  runtime probe is marked **UNVERIFIED**.
- Reach reference: for capacity comparisons the MCC Reach constructor literals are used (Stock build), and the live
  pool report from the 2026-11-15 runs is used for observed usage/capacity in game.

## 1. Engine baseline

| Engine | Data-array implementation | Pools | Functions | players | actor |
|---|---|---|---|---|---|
| Reach (MCC) | restricted allocation manager, header signature `0x64407440`, constructor `FUN_180012fcc` | 110 | 27,578 | 16 x 0x490 (1,168 B) | 96 x 0xD00 (3,328 B) |
| Halo 3 | vftable-based data array, constructor `FUN_18013003c` | 83 | 20,444 | 16 x 0x2F8 (760 B) | **256** x 0xAA0 (2,720 B) |
| ODST | same vftable constructor as H3, `FUN_180141e00` | 86 | 21,484 | 16 x 0x418 (1,048 B) | **256** x 0xAB0 (2,736 B) |
| Halo 4 | restricted allocation manager like Reach, constructor `FUN_1800432d8` | 112 | 36,876 | 16 x **0xF18** (3,864 B) | 96 x **0x1380** (4,992 B) |

Headline: **Halo 4 is the closest architectural sibling of Reach** (same allocator design, ~same pool count);
Halo 3/ODST use a different data-array implementation (which is why Reach's pool-header scanner/patch does not
transfer there - already noted in the Stage-1 report). **No engine raised the 16-player cap**; Halo 4 instead grew the
per-player record by ~3.3x (and the actor record by ~1.5x), i.e. it added state per entity rather than more slots.

## 2. What later games raised or changed, system by system

### 2a. Limits / AI actor pools (recap + new capacity evidence)

This was the Stage-1 system; the capacity extraction now quantifies it.

| Pool (same name in all engines) | Reach | Halo 3 | ODST | Halo 4 | Read |
|---|---|---|---|---|---|
| actor | **96** | **256** | **256** | 96 | H3/ODST raised actor capacity 2.7x; H4 kept Reach's 96 but grew the record 1.5x |
| squad | 256 | 335 | **512** | 256 | ODST doubled squads; H4 kept Reach's count |
| objectives | 64 | **100** | **125** | 64 | H3/ODST raised objective capacity 1.6-2x; H4 kept 64 |
| prop_ref | 1,024 | 1,024 | 1,024 | 1,024 | unchanged everywhere |
| object | 2,048 | 2,048 | 2,048 | 2,048 | unchanged everywhere |
| players | 16 | 16 | 16 | 16 | unchanged everywhere (H4 record 3.3x bigger) |
| simulation view data array | 16 x 0x118 | (renamed/absent) | (renamed/absent) | 16 x 0x120 | slot count unchanged; H4 record slightly bigger |
| cached object render states | 1,024 x 0x270 | 768 x 0x4D8 | 768 x 0x4D8 | 1,024 x 0x298 | Reach = H4; H3/ODST lower |
| leaf system | 24 | 32 | 32 | 24 | H3/ODST slightly higher |
| ragdolls | 7 | (absent) | 5 | **32** | H4 raised ragdolls ~4.5x |
| screen_effect | 64 | 64 | 64 | **80** | H4 slightly higher |
| hs globals | 4,096 | 3,072 | 3,072 | 4,096 | Reach = H4, higher than H3/ODST |

**Same change possible in Reach?** The actor and squad raises are exactly the kind of change Noble's proven pool hook
(`data_array_new` at `hr+0x12FCC`) already performs - the 2026-11-08 tests held 1,024 actors and 32,767 object slots on
the invented V16 profiles. So: **actor 256 / squad 512 / objectives 100-125 are technically possible and directly
supported by later-game precedent**; what blocks shipping them is the currently-disabled Extended profile and the
memory/serialization budget (REVIEW: banks and network state), not the mechanism. **Player >16 is NOT supported by any
later game** - every MCC engine keeps 16 slots - so raising it is an architecture/networking project (Claude's lane),
not a pool-number change. **Particle/decal/light/render-state raises are NOT supported by later-game precedent**:
Reach is already equal to H4 and above H3/ODST for effect/particle/decal/lights counts, and H4 kept the 1,024 render
states that Reach has. The master-list items that propose raising those (D10, A2/A11/A12) are therefore low-value or
unsupported by this evidence.

### 2b. Stability / crash handling

- The later engines show the same data-array allocator *family* only in H4; H3/ODST use a different implementation, so
  their crash/allocator behaviour is not comparable via this method.
- Pool names that relate to stability are present in Reach and H4 (lock hierarchy nodes/links, cached object render
  states, screen effects); H3/ODST lack the lock-hierarchy pool names (renamed or different scheme).
- No decompile evidence was found for "alloc-failure logging", crash-dump providers or defensive guards being *raised*
  or *added* in the later engines; those are engine-control-flow details that this symbol-less comparison cannot see.
- **Verdict: no portable, evidence-backed stability change identified.** Reach's own Noble-side guards (bank overflow
  warning, scheduler/collision guards, crash patches) remain the relevant safety net. **UNVERIFIED** for the later
  games.

### 2c. Networking / session

- Same-name network-ish pools: Reach and H4 both have `simulation view data array` 16 (H4 record 0x120 vs Reach 0x118)
  and `sim. object glue` 2,048 (H4 record 0x10 vs Reach 0xC); H3/ODST do not have these pools under those names.
- `players` is 16 in every engine (see 2a), so **no later MCC engine demonstrates a >16-slot session layout**.
- H4's much larger `players` and `actor` records show it added per-entity state, not slot count.
- Session/host-migration/replication code cannot be identified structurally from symbol-less decompiles; no quoted
  literals for "replication"/"host migration" were found in any engine, so this note makes **no claim** about what
  later games changed there. **UNVERIFIED**; networking stays in Claude's lane
  (`NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`).
- **Same change possible in Reach?** Only the per-entity-state direction is evidenced; it would be a sprawling change
  and is not worth doing. Player-count work remains an original engineering task, not a port.

### 2d. Rendering / lighting

Like-named render pools (Reach / H3 / ODST / H4):

| Pool | Reach | H3 | ODST | H4 |
|---|---|---|---|---|
| particle_emitter | 832 | 576 | 576 | 832 |
| particle_location | 576 | 576 | 576 | 576 |
| particle_system | 576 | 576 | 320 | 576 |
| effect | 920 | 640 | 640 | 920 |
| effect event | 1,800 | 640 | 640 | 1,800 |
| effect location | 1,380 | 1,152 | 1,152 | 1,380 |
| decal | 2,048 | (renamed) | (renamed) | 2,048 |
| decal_system | 704 | 956 | 704 | (renamed) |
| light_volume | 160 | 160 | 160 | 160 |
| lights | 600 | 400 | 400 | 600 |
| contrail / beam / c_*_gpu families | 160 each | 160 | 160 | (renamed/moved) |
| cached object render states | 1,024 | 768 | 768 | 1,024 |

**What changed:** later games did **not** raise the big render budgets; Reach is equal to H4 (particle/effect/lights/
decals/render states) and above H3/ODST for most. H4 raised ragdolls/screen-effect a little (2a) and changed record
sizes. H3/ODST have a slightly higher Ødecal_systemØ (956) but Reach's separate ØdecalØ 2,048 gives it more total.
**Same change possible in Reach?** Raising particles/decals/lights is technically possible through the same pool hook but
is **not justified by later-game precedent** (Reach already at/above them); the real rendering gaps (Elite relighting,
shadow/DoF controls) are engine behaviour, not pool counts - **UNVERIFIED for porting**.
Practical takeaway: the "raise particle/decal/light capacity" master-list items (A2/A11/A12/D10) are **not** where the
later-game evidence points; the H3/ODST-style AI pools are.

### 2e. AI / pathfinding / objectives

- **Pool presence differs sharply:** Reach and H4 have `path_influence` (2), `stimulus-data` (256), `stimulus-ref`
  (1,024), `fire team` (16), `ai directives` (16), `ai_cue` (256), `ai_sync_action_arranger` (32) and `objectives`
  (64). H3 and ODST have **none of those pool names**; they instead use different names (`squad_patrol` exists in
  ODST=16, H3 absent; `formations` present in ODST=16).
- So **H4 is the right reference for Reach's AI architecture** (same pool family); H3/ODST's AI is a different
  generation (their higher actor/squad counts come with a different perception/path/influence model). Copying H3/ODST
  AI behaviour into Reach is **not** a mechanical port.
- What later games raised: H3/ODST actor 256 / squad 335-512 / objectives 100-125; H4 kept Reach's counts but grew the
  per-actor record 1.5x. No capacity number for the path influence map itself changed (2 in Reach and H4).
- **Same change possible in Reach?** The capacity raises are possible (2a). The behavioural systems (influence maps,
  perception, objectives, path iteration caps) are Reach-specific and can be tuned via its named tunables; later-game
  values cannot be transplanted directly because the architectures differ. Any claim that H3/ODST's AI "works better"
  would need gameplay testing - **UNVERIFIED**.

### 2f. Gameplay / UI / appearance / theater / variants

Like-named pool evidence (Reach / H3 / ODST / H4):
- `chud widgets[0..3]` = 128 each in Reach/H3/ODST; H4 has different (Hud Audio Data) widget pools - H4 reworked the UI
  plumbing, not the per-player widget limit.
- `players` 16 everywhere, H4 record 3.3x bigger (2a).
- `campaign levels/insertions`: Reach 64/64, H3 64/32, ODST 64/32, H4 64/64 - unchanged.
- `megalo_objects`: Reach 512, H4 256, **H3/ODST absent** - Megalo-style variant scripting exists in Reach and H4;
  Reach's pool is larger than H4's, so there is no later-game capacity raise to copy. Halo 3/ODST use the older
  game-variant format, not Megalo.
- Quoted-literal scan (crude): "megalo" appears 4x in Reach, 12x in H4, 0 in H3/ODST; "theater" 4x in all; "saved film"
  Reach 1 / H4 1; "variant" H3/ODST 12 vs Reach 1 / H4 2 (format code differs). "hud"/"chud"/"objective" similar across
  engines. No "armor"/"visor" string literals (data-driven through tags).
- **What changed:** H4 added per-player state and reworked HUD/audio pools; H3/ODST use the older variant format.
  **Same change possible in Reach?** The variant/Megalo tooling is a Reach/H4-family feature; H4 is the comparison
  reference (see tooling below). Appearance (visor/armour) is tag/appearance-record driven, so cross-game pool data says
  nothing; the existing Reach RE notes remain the reference. **UNVERIFIED for appearance specifics.**

### 2g. Tooling / scripting / Forge / maps

- Script/global pools: `hs globals` Reach 4,096 = H4 4,096 > H3/ODST 3,072. Reach has the largest runtime-resource
  budget of the three: `runtime resource locations` Reach 1,200 vs H3/ODST 320 (H4 renamed/absent). H3/ODST have
  `tracking hs thread` 324 (Reach uses a different tracking pool), `test data array` 101.
- `megalo_objects` Reach 512 vs H4 256 / absent H3/ODST → Reach already exceeds H4; Megalo tooling work should use
  H4 only as a design reference for the variant format, not for capacity.
- No later-game evidence found for: Forge budget values ("budget" has no quoted literal in any engine), navmesh
  baking/runtime navigation ("navmesh"/"nav_mesh" no quoted literals), Megalo compiler, script-function browser.
  These are **UNVERIFIED** - the decompile/string method cannot rank them.
- **Same change possible in Reach?** The tooling directions are all original work; later games provide no directly
  transplantable code path in this comparison. Forge/AI tooling on custom maps remains the big modder-value item
  (`TOP10-NEXT-UPGRADES` #6).

## 3. What the comparison implies for the master list

- **Supported by later-game precedent (consider doing):**
  - Actor pool raise 96 → 256 (Halo 3/ODST both 256; Reach pool hook already tested at 1,024).
  - Squad pool raise 256 → 335-512 (H3 335, ODST 512; ODST is the strongest precedent).
  - Objectives pool raise 64 → 100-125 (H3 100, ODST 125).
  - These need the Extended profile re-enabled (see #1 in the ranked list); they are not "new engine features".
- **Not supported / deprioritise:**
  - Particle/effect/decal/light raises (A2/A11/A12) - Reach already equal to H4 and above H3/ODST.
  - Cached object render states raise (D10) - Reach already has the H4 value (1,024); H3/ODST lower. The 8k/16k crash
    attempt was chasing a number no later game uses.
  - Player count >16 (A5) - no later MCC engine exceeds 16 slots; H4 grew per-player state instead. It is an
    architecture/networking change, consistent with Tyler's 128-player hold.
- **No evidence either way (flag UNVERIFIED):** stability/alloc-failure, replication/host migration, rendering
  feature changes (lighting/shadows/DoF), appearance specifics, Forge/Megalo/navmesh tooling.
- The extracted tables in `crossgame-pools-2026-11-15.csv` should be re-used whenever a master-list item claims a later
  game "raised X": check the same-name row before spending time.

## 4. Verification status of this comparison

- **Proven** (method): constructor call tables per engine; capacities/record sizes as compiled; pool-name presence;
  function counts; engine implementation split (Reach/H4 restricted manager vs H3/ODST vftable arrays).
- **Inferred, not tested in game:** every "same change possible in Reach" statement (the pool hook mechanism is proven,
  but the specific capacity raises under the Extended profile have not been re-tested since that profile was disabled).
- **UNVERIFIED:** anything about networking/replication internals, stability changes, rendering feature parity,
  appearance specifics and tooling capabilities; string-literal counts are presence signals only.
