# MCC address table - retail name -> MCC `haloreach.dll` RVA (proven rows)

Started 2026-11-15 in the DeepSeek engine-upgrade phase (starter-prompt phase 1 step 2). One row per **proven** match; open candidates go in the "Open candidates" section labelled UNPROVEN. Statuses were added 2026-11-15 12:20 ET to line up with the master-list PROVEN/FAILED/NOT STARTED pass. Add a row only after the match is proven by the method below, and keep the Compendium copy and the repo `knowledge\` copy in sync.
open candidates go in the "Open candidates" section labelled UNPROVEN. Add a row only after the match is proven by the
method below, and keep the Compendium copy and the repo `knowledge\` copy in sync.

## Conventions
- **MCC build:** `haloreach.dll` SHA-256 `738dd2d2...0894`, image base `0x180000000`. `hr+RVA` = address - base
  (e.g. `FUN_1802d288c` -> `hr+0x2D288C`). Raw pseudocode: `D:\MCC HALO REACH DECOMP\decompiled\hr_*.c`
  (27,578 functions).
- **Retail:** Xbox 360 Reach review build (Jul 24 2010, Omaha linker map), base `0x82000000`; names from
  `symbols.csv`. Use **`D:\ReachX360\ghidra_retail\decompiled_v3\`** only (v3 has 99 stubs; `decompiled\` /
  `decompiled_v2\` have ~10,000 empty stubs and must not be used for bodies).
- **PROVEN here** means the retail function and the MCC function are the same code, matched statically (same struct
  layout, constants, string literals and call shape). Runtime behaviour is a separate proof and is noted in the
  "how proven" column when it exists.

## Proven matches

| Retail name | Retail VA | MCC function | MCC hr+RVA | What it does | How proven | Date |
|---|---|---|---|---|---|---|
| `cui_property_editor_get` | `0x82709a08` | `FUN_1802e9720` | `0x2E9720` | Property-editor lookup: 14-entry binary search by type tag. Editor struct is {type tag, can_convert_from, convert_from, to_string} and its layout is identical in retail and MCC; tags: bool 0, long 1, real 2, const_wchar_ptr 3, string_id 6, qword 8. Editors are original behaviour and do **not** zero-pad. | Static decompile comparison: same struct/type tags, same constants and call shape | 2026-11-15 |
| `g_*_property_editor` handler table (`g_bool/long/real/qword/const_wchar_ptr/string_id/...`) | `0x82020018..0x82020410` | table at `hr+0xB4C800` | `0xB4C800` | Array of property-editor handlers used by the editors above. | Static: same table layout/entry step as retail | 2026-11-15 |
| `qword_property_to_string` | `0x827c4df0` | `FUN_180381f10` | `0x381F10` | Formats a qword as text via `_i64tow_s(val, buf, cap, 16)` (radix 16). | Static: same formatting call and radix | 2026-11-15 |
| `long_property_to_string` | `0x82714de8` | `FUN_1802f3ab8` | `0x2F3AB8` | Formats a long as text via `_itow_s(val, buf, cap, 10)` (radix 10). | Static: same formatting call and radix | 2026-11-15 |
| `real_property_to_string` | `0x827c4d58` | `FUN_180381eb0` | `0x381EB0` | Formats a real as text (`usnzprintf`). | Static: same call shape | 2026-11-15 |

Source note: all five rows come from `project-docs\NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md` (section "What the Xbox 360
decompile proved"). The note's other retail-side findings are below because they have no MCC address recorded yet.

## Additional retail-side findings (no MCC match recorded yet)

| Retail item | Retail VA | Finding | Status |
|---|---|---|---|
| `c_number_formatter_component` (leading_zeroes, digit_count) | not recorded | Formats floats as decimal via `%0*.*f` - rules it out as a hex source. | UNPROVEN relevance |
| `parse_hud_scoreboard_string` | `0x828619f8` | Stub shared with `parse_build_number_string` and `parse_vehicle_set_string`. | UNPROVEN relevance |
| private-use UTF-32 text tokens | n/a | `0xE424` = HUD quantity, `0xE425` = game engine; handled by `parse_*_string` functions. | UNPROVEN relevance |

## Open candidates (UNPROVEN)

| Candidate | MCC hr+RVA | Why it is a candidate | Status / next test |
|---|---|---|---|
| localized-string expander leaking `\a<8 hex>` | `FUN_1802d288c` = `hr+0x2D288C` | On failure it leaves the 8 hex digits in the output, which matches "0000001c"/"Respawn in 00000001". | UNPROVEN: a solo MP/campaign read-only hook never saw the failure path or an 8-hex leak (`project-docs\NOTE-HEX-BUG-RUNTIME-CHECK-2026-11-15.md`). Next: exact repro screen/mode, scoreboard open, or trace the string-context creator (`hr_00004.c:71443` and `:71603` callers in the MCC export). |

## MCC-only addresses recorded elsewhere (no retail name paired yet)

| MCC hr+RVA | What | Status | Source |
|---|---|---|---|
| `0x3B2F4` (`FUN_18003b2f4`) | Visor/global item -> engine slot translator | Static RE; used by the shipped `ArmoryVisor` id mapping (`8f467e1`), in-game mapping verified by eye | `REFERENCE-LIBRARY-2026-11-15.md` §1; `01-PROJECT-HISTORY.md` Phase 13 |
| `0x2EDC5E` | Direct item-157 patch site (visor mapping) | Static RE, patch shipped in `src\engine\EnginePatches.cpp`; re-verify the 7 bytes against the installed DLL before reuse | `REFERENCE-LIBRARY-2026-11-15.md` §1 |
| `0x393CC` (inside `FUN_180039128`) | Profile settings -> item id mapping | Static RE | `REFERENCE-LIBRARY-2026-11-15.md` §1 |
| `0x24334C` (`FUN_18024334c`) | Shader-parameter slot -> id (`visor_tint` = slot 6 -> `0x8018f`) | Static RE; a forced-slot test (08:40) was negative on the visible Armory visor | `project-docs\RE-VISOR-COLOR-2026-11-15.md` |
| `0x32596C`, `0x325A38` | Model shader-parameter table init / append entry | Static RE | `project-docs\RE-VISOR-COLOR-2026-11-15.md` |
| `0x3246B8`, `0x324F90`, `0x32510C`, `0x2C8480` | Player-model colour application path | Static RE; forcing a colour in `FUN_1802c8480` did not change the Armory visor (08:40 negative) | `project-docs\RE-VISOR-COLOR-2026-11-15.md` |

## Proven MCC-only addresses (hook targets verified in game or by live probe)

These have no retail name paired yet, but the MCC side is proven by a runtime test or live probe. Keep the proof
column honest: an address that was only signature-checked is listed as UNPROVEN.

| MCC hr+RVA | What | Status / how proven | Evidence | Date |
|---|---|---|---|---|
| `hr+0x12FCC` | `data_array_new` / pool initialiser (name, count, record size, alignment, allocator) | **PROVEN** - pool resizing by name works and the pool report sees the raised capacities (`test-limits.ps1`: object 2,048 Stock / 32,767 Extended PASS) | `AI-LIMITS-REPORT-2026-11-08.md`; `src/host/PoolResize.hpp` | 2026-11-08 |
| `hr+0x27BF270` | Game-state bank table: 8 banks, stride 0x18F0, +0xA0 capacity / +0xA8 used | **PROVEN** - live `BANK|` lines in every pool report; bank usage/overflow guard | `src/host/PoolReport.cpp`; Stage-1 + stabilization logs | 2026-11-15 |
| `hr+0x61A38` | Per-player control tick `(unit handle, playerIndex, c, dt1, dt2)` | **PROVEN** - temporary debug log showed arg1 = unit handle, arg2 = 0; after gating on arg2 the horde/spawn-test run (`d636b31`) | `src/host/SpawnTest.cpp`; `qa-regression*` logs | 2026-11-15 |
| `hr+0x6C2B10` | `ai_place(aiValue, count, flag)` | **PROVEN** - `-horde` placed actors | Stage-1 Firefight log (`stage1-firefight.log`) | 2026-11-15 |
| `hr+0x49CC24` | `object_placement_data_new(placement, tag, owner, extra)` | **PROVEN** - `-spawn-test` built placements for 1,495 objects | Stage-1 Forge log (`stage1-forge.log`) | 2026-11-15 |
| `hr+0x46D6E0` | `object_new(placement)` -> object handle | **PROVEN** - 1,495 objects spawned (5 refused) | Stage-1 Forge log | 2026-11-15 |
| `hr+0x46CB38`, `hr+0x46C8B4`, `hr+0x717F50` | `object_set_position`, `object_refresh`, `actor_teleported` | **PROVEN** - horde moved placed actors next to the player (`moved` counts in the log) | `SpawnTest.cpp`; horde logs | 2026-11-15 |
| `hr+0x4F9A74` | `unit_exit_vehicle(unit, mode)` | **PROVEN** - used by the horde step for placed squads (infantry path) | `SpawnTest.cpp`; horde logs | 2026-11-15 |
| `hr+0x6C1C14` | `ai_set_team(actor, team)` | **PROVEN** - `-horde-team 0` applied and logged | Stage-1 Firefight log | 2026-11-15 |
| `hr+0x6BF204`, `hr+0x6BF0EC` | `ai_set_blind`, `ai_set_deaf` | **PROVEN** - default horde path sets blind/deaf on placed actors | `SpawnTest.cpp`; horde logs | 2026-11-15 |
| `hr+0x3ABE70` | `c_saved_film_manager::session_monitor` hook | **PROVEN** - saved-film playback resolves the film's map/mode and plays it with the director overlay | stabilization QA (`qa-film-playback.log`) | 2026-11-15 |
| `hr+0x6C1A00`, `hr+0x70AB58` | `ai_force_active`, `ai_force_full_lod` | **UNPROVEN** - signature-checked and hooked; the `-horde-full-lod` path was not exercised in an accepted run | `src/host/SpawnTest.cpp` | - |
| `hr+0x1EB650`, `hr+0x47A854` | `object_destroy`, `object_held` | **UNPROVEN** - signature-checked; `-horde-infantry` path not exercised in an accepted run | `src/host/SpawnTest.cpp` | - |

### Startup patches (installed and signature-checked every run; behaviour not re-proven by this table)

| MCC hr+RVA | Patch | Notes |
|---|---|---|
| `hr+0x8262E8` | `IsWhitespace` underflow crash patch | installed at startup; prior session recorded it as a crash fix |
| `hr+0x105324` -> `hr+0x1055C1` | armour-composite fallback jump patch | installed at startup; prior session recorded a crash fix at 0x105597 |
| `hr+0x99A0D` | null-biped dereference crash patch | installed at startup (trampoline) |
| `hr+0x5DF80` | `player_control_update` (Lower weapon / Alert Carry hook) | installs; keybind behaviour not re-tested in the stabilization pass |


## How to add a row (method - starter prompt + briefing)
1. Pick the retail name from `MASTER-UPGRADE-OPPORTUNITIES` / `engine_source_map_x360_retail.csv` /
   `decompile-exports\x360-retail\functions_index.csv`.
2. Read the body in `decompiled_v3\`; note strings, constants, callees.
3. Find the MCC match in `D:\MCC HALO REACH DECOMP\decompiled\hr_*.c` by the same strings/constants/call shape.
4. Record the row here (retail name, VA, MCC function, hr+RVA, behaviour, how proven, date). Mark it UNPROVEN until
   step 2/3 are done; a static match is PROVEN for identity only.
5. Prove runtime behaviour with a read-only probe before any write (briefing §3).