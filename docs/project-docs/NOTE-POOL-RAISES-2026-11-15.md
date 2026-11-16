# Cross-game pool raises: squad 512, objectives 125 (top-10 item #2)

Date: 2026-11-15 14:00-14:15 ET. Owner: DeepSeek (lead). Preceded by item #1
(`project-docs\NOTE-EXTENDED-LIMITS-REENABLE-2026-11-15.md`), which pinned the engine profile and landed `0848280`.
Commit for this item: **`8f0fe85`** (`wip-merge` = `main`, pushed private; version still 0.19.103).

## What was raised

| Pool | Stock (Reach) | Now | Record | Cross-game precedent |
|---|---|---|---|---|
| actor | 96 | **1,024** (already, via pinned AI actors) | 3,328 B | H3/ODST 256; our pinned profile exceeds both |
| **squad** | 256 | **512** | 236 B | ODST 512 (H3 335); H4 kept Reach's 256 |
| **objectives** | 64 | **125** | 24 B | ODST 125 (H3 100); H4 kept Reach's 64 |

Probe evidence (read-only, live pool report on the pinned build): campaign m10 uses **squad 120/256** and
**objectives 28/64** - i.e. these were the next real AI ceilings after the actor pool was pinned at 1,024.
Both are created by the proven `data_array_new` hook (`hr+0x12FCC`) and are raised by name in `PoolResize`.
Cost is 59 KB (squad) + 1 KB (objectives) of the +6 MB AI bank; bank 1 still has ~580 KB headroom after the raises.

## Layout generation tag (peer safety)

Raising pool capacities changes the game-state layout, so the pinned profile's network id now carries a layout
generation: `kShippedLayoutGeneration = 1` and `ProfileNetworkId = [profile:8][layout generation:8][AI actors:16]`.
The pinned build advertises **`0x03010400`** instead of `0x03000400`. A build with a different layout is refused by the
existing join check even when the profile name and actor count match; the refusal text now names the layout generation
and says both players must run the same Noble build. Rule for future work: **bump `kShippedLayoutGeneration` whenever pool
capacities / bank sizes / stack frames in the pin change**, and keep the save-folder suffix rules in mind.

## Files changed

| File | Change |
|---|---|
| `src\host\PoolResize.cpp` | Two rules in `kRules`: `squad` -> max(stock, 512), `objectives` -> max(stock, 125). Only active with an extended profile (pinned build always is). |
| `src\host\EngineLimits.hpp` | `kShippedLayoutGeneration = 1` + explanation. |
| `src\host\EngineLimits.cpp` | `ProfileNetworkId` mixes the generation into bits 16-23. |
| `src\ui\FrontEndMenu.cpp` | Join-refusal text names the layout generation; "same Noble build" wording (no user engine-limits setting any more). |

## Verification

| Run (pinned path, no dev probe) | Live pool report | Captures | Crash |
|---|---|---|---|
| campaign m10 (`pool2-campaign`) | `objectives 28/125`, `squad 120/512`; log "AI limits: squad 256 -> 512 (59 KB)", "objectives 64 -> 125 (1 KB)"; `BANK|1|7,922,248|8,519,680` | 2/2 render (49.4, 70.8 mean) | 0 |
| Firefight ff_unearthed (`pool2-firefight`) | squad 512 / objectives 125; same raise log | 2/2 render (75.2, 74.4) | 0 |
| Forge forge_halo (`pool2-forge`) | squad 512 applied (no AI squads on the map) | 2/2 render | 0 |
| campaign m10 on the final binary (`pool2-gen-campaign`) | network id **`0x03010400`**, squad 120/512, objectives 28/125, save folder `HaloReach_ext_full_a1024` | 1/1 render (62.4) | 0 |

Gates: `noble-dev` + `noble-client` rebuilt clean after the change.

## Remaining / follow-ups

- Long-run acceptance is still owed for item #2: a 30-minute heavy Firefight with the raises (and the Stage-3
  acceptance from item #1). Short runs prove the caps apply and nothing immediate breaks; squads/objectives were not
  individually soaked.
- No static-array blocker for squad/objectives was found in the audit notes (unlike the actor frame buffers), and the
  later-engine precedent raised exactly these two; still, any crash seen in long sessions should first bisect these
  two rules with `-ai-pools` / the rule list.
- Save/film note: this build's layout differs from the pre-raise dev build `0848280` (no released users yet). Before any
  release, run the campaign save->load and saved-film checks on this final layout, and bump the layout generation again
  if the pin changes afterwards.
- Networking: peers now compare layout generation through `limits_id`; backend passthrough is still Claude's item
  (`NOTE-NETWORK-HANDOFF-TO-CLAUDE.md` section 6). 128-player hold untouched.
