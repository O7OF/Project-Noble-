# Extended engine limits: re-enable investigation + pinned-profile change (top-10 item #1)

Date: 2026-11-15 13:00-14:00 ET. Owner: DeepSeek (lead). Starting state: repo `b2065bd`, v0.19.103, tracked clean.
Scope: top-10 item #1 ("Stabilize and re-enable the Extended engine-limit profiles"), read-only probe first, then the
minimum client-side change set. Networking/security stayed Claude's lane; the 128-player hold was not touched.

## 0. TL;DR

- **The 2026-11-14 claim "Extended engine limits cause campaign black screens and Firefight/MP kicks" does not
  reproduce on the current build.** Campaign, Firefight and a local multiplayer host all render normally on
  Extended (V16), Full, and on the exact combo a community user reported broken: **Full (light volumes + particles)
  + 1024 AI actors**.
- The 11-14 campaign black screen was the **`145510a` launch-block regression** (host player identifier `+0xF0` and
  slot descriptor skipped for campaign/firefight), fixed in v0.19.95 `dad10d2` - about six hours *after* `4407101`
  disabled the profiles because of the symptom. The Extended renderer patches were never the cause.
- The Firefight/MP "kicks" were not reproduced with a local host. The plausible remaining cause is a **peer profile
  mismatch** (host on one layout, guest on another) - a networking item that the pinned-profile design now removes by
  construction (every install runs the same layout).
- **Tyler's decision (2026-11-15):** Extended is no longer a user setting. Every install runs one pinned profile -
  **Full (V16 + particles + light volumes) + 1024 AI actors** - and the ENGINE LIMITS / AI ACTORS pickers are removed.
  Save folders are per-profile (`saves\HaloReach_ext_full_a1024`) so campaign saves of different layouts never mix.
- Client-side change set implemented (section 4); probe/build/soak evidence in sections 1 and 5.

## 1. Probe evidence (no engine-behaviour change; dev-only profile override)

Method: dev-only `NOBLE_ENGINE_LIMITS_PROBE=<0..3>` override added under `#ifdef NOBLE_DEV_BUILD` (release builds
unaffected), launched unfocused on the AOC (`NOBLE_WINDOW_SECONDARY=1`, no focus/mouse), muted, captures via
`-capture-at`. Frame brightness sampled on a 64x36 grid: a black frame has mean luminance ~0 and 0% non-black
samples; rendering frames here are 24-82 with 96-100% non-black samples. Installed `haloreach.dll` verified stock
(13,229,016 B, sha256 `738dd2d2...0894`) so the `.nlp` patches apply; logs confirmed "requested X, active X",
object pool 32,767 and the grown game-state banks.

| # | Time | Mode / map | Profile | AI actors | Duration | Captures (mean lum) | Result |
|---|---|---|---|---|---|---|---|
| 1 | 12:53 | campaign m10 | Stock (control) | 96 | 110 s | 79.2, 51.1 | renders, 0 crash |
| 2 | 12:55 | campaign m10 | V16 | 96 | 150 s | 79.4, 50.0, 69.8 | renders (gameplay capture verified), 0 crash |
| 3 | 12:59 | campaign m10 | Full | 96 | 150 s | 24.3, 60.4, 74.6 | renders, 0 crash |
| 4 | 13:01 | Firefight ff_unearthed | Full | 1024 | 260 s | 66.9, 71.7, 68.0, 69.3, 68.6 | renders, 0 crash |
| 5 | 13:06 | MP 20_sword_slayer, local host | Full | 1024 | 80 s | 51.9, 52.0 | session runs, 0 crash |
| 6 | 13:07 | campaign m10 (reported combo) | Full | 1024 | 180 s | 82.2, 49.5, 71.3, 71.0 | renders, 0 crash |
| 7 | 13:11 | Firefight ff_unearthed soak | Full | 1024 | 620 s | `PENDING` | `PENDING` |

Pool readings during the pinned combo: `object 32,767`, `actor 96 -> 1024`, `BANK|3|58,613,580|67,043,328` (the
render bank the pinned Full profile allocates), plus the expected "beam_system/beam_location/beam 160 -> 65535 does
not fit; keeping stock 160" warnings from `540d450`'s bank-headroom guard (deliberate: no overflow).

## 2. Why the profiles were disabled (evidence chain)

| Date/commit | What happened |
|---|---|
| 2026-11-08 | Profiles proven: V16 Forge 6,516 objects; Full survived 5 min plasma Firefight. Campaign save/load on V16 was never tested. |
| 2026-11-13/14 | v0.19.69-v0.19.84: profiles user-selectable and working; `540d450` (11-14 13:28) fixed a real Full-profile crash - the runtime beam expansions overflowed game-state bank 3 and ended every session on its first frame; fixed by charging those pools against the real bank headroom. |
| 2026-11-14 16:15 `145510a` | BlamEngine/ReachHost rewrite skipped the host player identifier (`+0xF0`) and slot descriptor for campaign/firefight: the local player was never bound and the mission sat on a black frame. |
| 2026-11-14 16:59 `bb0412e` | Campaign launches forced to the stock DLL; the release note attributed the black frame to "every Extended profile patches the engine renderer". |
| 2026-11-14 17:44 `4407101` | Settings force-reset extendedLimits/aiActors to 0 on load and non-Forge launches forced Stock, citing "infinite blackscreen loads in Campaign and network kicks in Firefight/Multiplayer". |
| 2026-11-15 01:05 `dad10d2` | The launch-block regression fixed ("without +0xF0 the local player is not bound, the mission sat on a black frame"); all stock missions render. |
| 2026-11-15 11:35 | Stabilization QA on the fixed build: campaign m10 Stock PASS; max-settings attempts reset to Stock by design, so profiles could not be exercised. |
| 2026-11-15 13:00 (this note) | Probe on the fixed build: campaign/Firefight/MP on V16/Full/Full+1024 all render; no black screen, no kick. The disable is stale. |

Conclusion: keep `540d450`'s bank guard (still needed); drop the two `4407101` gates. The campaign black screen is
fixed by `dad10d2` and the solo "kick" did not exist; remaining kick risk is a peer profile mismatch, handled by the pin.

## 3. Decision and design (Tyler, 2026-11-15)

- Every install runs the same engine-limit profile: **`limits::kShippedProfile = Profile::Full`** and
  **`limits::kShippedAiActors = 1024`** (`src\host\EngineLimits.hpp`). Compile-time constants; a future build retunes
  them and every install moves together (the network id and the save suffix derive from them).
- **No picker, no user setting:** SETTINGS > ENGINE shows the active profile read-only; the ENGINE LIMITS and AI ACTORS
  selector rows are removed. `Settings::Clamp` pins the stored values on every load, so old settings files are corrected.
- **Per-profile save folders:** `ReachHost`'s user-data folder hook returns
  `<data>\userdata\<device>\saves\HaloReach<limits::ActiveSaveSuffix()>`. Stock stays in the historical `HaloReach`
  folder (existing saves untouched); the pinned profile uses `HaloReach_ext_full_a1024`. The suffix includes the AI
  actor count, so retuning the pin later moves to a new folder instead of mixing layouts.
- **Release builds are pinned hard:** `PoolResize` ignores `-ai-actors` / `NOBLE_AI_ACTORS` and the pool-bisect
  overrides unless `NOBLE_DEV_BUILD`. Dev builds keep them for A/B stability tests.
- **Network:** every peer advertises the same `limits_id`; a stock/old client is refused by the existing join check.
  Remaining enforcement gaps are listed for Claude in `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`.
- **Packaging:** the release must carry `data\limits\full.nlp`; `publish-release.ps1` now refuses to package without it.

## 4. Files changed

| File | Change |
|---|---|
| `src\host\EngineLimits.hpp` / `.cpp` | `kShippedProfile` / `kShippedAiActors` constants; new `ActiveSaveSuffix()` (profile + actor count). |
| `src\common\Settings.hpp` / `.cpp` | Defaults pinned; `Clamp()` pins both values on load; removed `4407101`'s force-reset and force-save. |
| `src\main.cpp` | Requested profile = pin; removed the non-Forge force-Stock gate; publishes the pinned actor count; loud errors when the pin cannot be applied; dev-only probe override kept. |
| `src\ui\SettingsPanel.cpp` | ENGINE LIMITS and AI ACTORS pickers removed; active profile stays visible read-only. |
| `src\engine\ReachHost.cpp` | Save folder gets `ActiveSaveSuffix()`; legacy kind1/kind2 migration only runs on Stock. |
| `src\host\PoolResize.cpp` | Non-dev builds always use `kShippedAiActors` and ignore the pool-size overrides. |
| `tools\publish-release.ps1` | Throws if `data\limits\full.nlp` is missing from the package. |
| dev test tooling (not shipped) | `NOBLE_ENGINE_LIMITS_PROBE` override (`#ifdef NOBLE_DEV_BUILD`) + `build\bin\Release\probe-limits.ps1` capture/brightness harness. |

## 5. Verification (final gate/test results)

- Gates: `noble-client` rebuilt clean 13:20:13 ET; `noble-dev` rebuilt clean 13:24:51 ET. Committed as
  **`0848280`** on `wip-merge` = `main` and pushed to the private repo (`origin`) - version still 0.19.103, no
  release/tag/public push.
- Pinned-profile end-to-end (normal settings path, no dev probe): campaign / Firefight / MP / Forge / main menu
  launched and logged "requested Full ... active Full" + "AI actors 1024 (network id 0x03000400)" + user-data folder
  `...\saves\HaloReach_ext_full_a1024` and rendered with 0 crashes - see the pinned-path results appended below.
- Soak: Firefight Full+1024 ran 620 s on the probe build: 10/10 captures render, 0 crash hits, heavy particle FX at
  the end, pools healthy (object 725/32,767, actor 101/1,024).
- **Pinned-path end-to-end** (2026-11-15 13:26-13:34 ET, `noble-dev.exe` built from this change set, **no probe
  env**, normal settings file): all five runs logged "requested Full ... active Full" and
  "AI actors 1024 (network id 0x03000400)", used the new folder `...\saves\HaloReach_ext_full_a1024`, had **0 crash
  hits and 0 profile-fallback warnings**, and rendered:

  | Run | Captures (mean lum, non-black %) | Note |
  |---|---|---|
  | main menu | 44.5, 97.7% | menu runs on the pinned engine now (previously forced Stock) |
  | campaign m10 | 49.4/96.5%, 70.8/99.3% | wrote `asq_campaign_m10_*.temp` into the new profile save folder |
  | Firefight ff_unearthed | 74.5/100%, 73.7/100% | |
  | multiplayer 20_sword_slayer (local host) | 52.0/100%, 51.9/100% | session runs |
  | Forge forge_halo | 45.8/98.9%, 46.8/99.4% | |
- The legacy Stock save folder `saves\HaloReach` was not written/modified by the pinned runs (checkpoint temp went
  to the new folder); a full save -> quit -> load round trip under the pinned layout is still to be tested.
- UI proof: SETTINGS > ENGINE now shows only read-only `ENGINE LIMITS: Full (V16 + particles + light volumes)` and
  `ACTIVE AI POOL: 1024 ACTORS` - no selector rows (capture `probe_pinned-settings_cap00.png`).

## 6. Known issues / follow-ups

- **Old campaign progress is not visible to the pinned profile.** Saves from Stock/older builds stay in
  `saves\HaloReach` and are intentionally never loaded under the new layout (mixing layouts can crash/corrupt). Do not
  delete them. A user-facing notice + optional backup is recommended before the next release.
- **Campaign checkpoint save -> load under the pinned profile is still to be tested end-to-end** (play past a
  checkpoint, quit, relaunch, load). This was never proven for V16/Full.
- **Films:** films recorded before the pin were made under a different layout; the team already decided film
  compatibility is not guaranteed. New films are consistent (everyone on the pin). Consider a stamp/refusal later.
- **MCC engine update:** a new installed `haloreach.dll` invalidates the `.nlp`; Noble logs the fallback (now an error
  for the pinned profile). The release checklist must regenerate `data\limits\*.nlp`; publish-release now blocks a
  package without `full.nlp`.
- **Join paths that do not peek the host's `limits_id`** (lobby share code, direct connect, campaign co-op insertion)
  remain for Claude - written into `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`.
- **Acceptance runs still owed** (Stage-3 criterion): 30 minutes at max settings in Firefight, campaign and a
  multiplayer map with no black screen. This session proved ~4.3 min Firefight, ~3 min campaign, ~1.5 min MP and a
  10-minute Firefight soak; longer sessions should be run before a release.
- Tyler's two new bugs (pause-menu scroll wheel does nothing; ESC pause does not pause) are queued at the bottom of
  `TODO-MASTER-2026-11-14.md` (items 17 and 18).
