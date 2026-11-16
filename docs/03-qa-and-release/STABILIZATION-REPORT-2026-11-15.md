# Stabilization pass after v0.19.103 (2026-11-15 ~12:05 ET)

Requested by Tyler: feature work paused; list every change with commit + test proof, run a regression pass
(campaign, Firefight, multiplayer, Forge, Armory, lobby codes, save/load, saved films; default and maximum settings),
record it in QA-VERIFY, fix regressions only (no features), clamp/hide any option that can crash, draft release notes +
known issues, recommend ship vs hold. **No version bump, no tag, no publish** (Tyler has not said "release it").

Build under test: **v0.19.103** source (release commit `eea9868`) plus the post-release harness fix `d636b31` and the
stabilization clamps below. Both gates rebuilt clean after the clamps (`noble-dev.exe` 11:51:53,
`noble-client.exe` 11:52:07, version still 0.19.103). Runs: AOC second monitor, launched unfocused
(`NOBLE_WINDOW_SECONDARY=1`, SW_SHOWNOACTIVATE), settings muted and restored around every run.

Stabilization commit: **`b2065bd`** (clamps; private `main` = `wip-merge`, pushed; version still 0.19.103).

## 1. Every change and whether a test proves it

### A. Delta since the v0.19.103 RELEASE (what "changed since v0.19.103" literally means)

| Commit | When | Change | Proven by a test? |
|---|---|---|---|
| **`d636b31`** | 11-15 11:28 | Test harness: the per-player tick (`FUN_180061a38`) is now gated on the player index (arg 2) instead of a unit handle, and that index is passed to `DevCheats::GameTick`. Dev heavy-load tools (`-horde`, `-spawn-test`, dev cheats, cinematic auto-spawn gate) had been dead since `7f31cf2` (2026-11-09). | **YES**: `-horde` placed actors and `-spawn-test` spawned 1,495 objects in the Stage-1 runs; the full QA matrix below ran on this build with 0 crashes. This is dev/test-only code, not user-facing. |

That is the complete tracked delta since the release commit: **one file, +6/-2**. No other tracked changes existed
(`git status` clean; the 44 untracked asset/doc lines are the pre-existing files).

### B. The v0.19.103 feature delta since v0.19.102 (`8db1ce8`) - what this regression pass covered
(v0.19.103 shipped without a full product regression, so these are the changes that needed the QA)

| Commit | Change | Files | Proven by a test? |
|---|---|---|---|
| `ca42a24` | Map pickers: variant-drawer search box, category cycler, N OF M VARIANTS | FrontEndPickers.cpp, FrontEndInternal.hpp | **Partly** - proven by captures in the earlier session (`drawer_filter4.png`, `drawer_search_jump.png` 3/353, `drawer_cat_1.png` 11/353). Not re-captured today (needs interactive drawer navigation); code unchanged since. |
| `0f47a63` | Server browser: Refresh status + pinned FAVOURITE / COPY INVITE footer | ServerBrowser.cpp/.hpp | **Partly** - prior captures (`sbfix_click_0/1.png`, `sbfix_footer.png`); new QA capture shows the browser and footer render, and the click hook works (campaign MAX PLAYERS popup). Refresh/favourite click itself not re-clicked today. |
| `0efb619` | Menu clicks (Game Options whole-row, What's New hint) + controller toggle crouch + M&K toggle zoom | Settings, FrontEndMenu, ReachHost, SettingsPanel | **Partly** - prior captures (`gameoptions_labelclick.png`, `main_hint_click.png`, `settings_toggle_rows.png`); new QA captures show menus/settings render. Toggle behaviour not re-exercised today. |
| `88d4937` | Lobby share codes: NB1 codec, JOIN WITH CODE (Main Menu / Server Browser modal / Social), lobby COPY CODE | InviteCode.*, ServerBrowser.*, FrontEndLobby/Menu, FrontEndInternal.hpp, CMakeLists | **Codec YES** (new run: `-invite-code-selftest` → 0 failures). **UI YES** (new QA lobby capture shows the share code `NB1-…` + COPY CODE). **End-to-end two-PC join NO** - needs a second PC/Tyler. |
| `c584c8a` | Test-only: drive Noble's scoreboard with no focus/mouse/cursor capture | ReachHost.cpp | **YES** by prior session (`scoreboard_safe.png`); test-only code. |
| `ca770e2` | Social screen prominent JOIN WITH CODE | FrontEndSocial.cpp | **Partly** - prior capture `social_join_code.png`; new QA social screen render captured. |
| `64e6d22` | Test-only: `-armor-effect-test` dev hook | ArmorCustomizer.cpp | **YES** by prior session (blue flames capture); test-only code. |
| `b1680d1` | Test-only: `-third-person-front` capture camera | CameraThirdPerson.cpp | **YES** by prior session (`front_cam_final.png`); test-only code. |
| `eea9868` | Release v0.19.103: version bump + release notes + in-game changelog | CMakeLists, Version.hpp, BuildIdentity.hpp, noble.rc, ChangelogData.hpp, release-notes/v0.19.103.md | **YES** - gates build and report 0.19.103 (both exes rebuilt today); release itself was verified/published in the previous session. |

## 2. Regression matrix results (recorded in `QA-VERIFY-2026-11-15.md`)

16 scenarios, 0 real crashes (`FATAL EXCEPTION`/`0xC0000005`/crash-dump: **0 hits in every scenario**). The five
`FATAL/EXCEPTION` texthits per log are the boot lines "stability and crash patches…" / "crash patch…" - not failures;
each log also has two benign shutdown lines (`FreeLibraryAndExitThread … from haloreach+0x0` and `engine main loop
returned 1` on test-quit). Every scenario ended by `-quit-after` (forcedStop=false).

| # | Scenario | Settings | Result | Evidence |
|---|---|---|---|---|
| 1 | Main menu | default | PASS - renders, 0 crash | `qa2_menu-main.png` |
| 2 | Campaign picker | default | PASS - renders (already showed 4 PLAYERS default) | `qa2_menu-campaign.png` |
| 3 | Settings screen | default | PASS - renders | `qa2_menu-settings.png` |
| 4 | Armory | default | PASS - 3D Spartan + colours render | `qa2_menu-armory.png` |
| 5 | Server browser + invite-code codec | default | PASS - browser renders; **codec self-test 0 failures** | `qa2_menu-servers-codec.png`, log line `InviteCode self-test: 0 failure(s)` |
| 6 | Social screen | default | PASS - renders | `qa2_menu-social.png` |
| 7 | Pre-game lobby (demo host, 8 players) | default | PASS - roster + HOSTING share code + COPY CODE render | `qa2_lobby-demo-host.png` |
| 8 | Campaign (m10, difficulty 1) | default | PASS - loads, intro/play runs, **checkpoint save written** (`Checkpoint... done` + autosave MvarWrite), 0 crash | `qa2_match-campaign.png` |
| 9 | Firefight (ff_unearthed) | default | PASS - HUD + gameplay, 0 crash | `qa2_match-firefight.png` |
| 10 | Multiplayer (20_sword_slayer, local host) | default | PASS - in-game Slayer, 0 crash | `qa2_match-multiplayer.png` |
| 11 | Forge (forge_halo) | default | PASS - Forge World loads and plays (player view; editor/monitor toggle untested), 0 crash | `qa2_match-forge.png` |
| 12 | Saved film playback (`-film` m10 film) | default | PASS - map/mode resolved from film, theater playback + director controls, 0 crash | `qa2_film-playback.png` |
| 13 | Campaign max settings attempt | extendedLimits=3 + aiActors=1024 written first | PASS - build **reset to Stock as designed** ("requested Stock, active Stock"), loaded and played, 0 crash / no black screen | `qa2_max-campaign.png` |
| 14 | Firefight max settings attempt | same | PASS - Stock fallback, gameplay, 0 crash | `qa2_max-firefight.png` |
| 15 | Multiplayer max settings attempt | same | PASS - Stock fallback, gameplay, 0 crash | `qa2_max-multiplayer.png` |
| 16 | Forge max settings attempt | same | PASS - Stock fallback, plays, 0 crash | `qa2_max-forge.png` |

Raw evidence: `build\bin\Release\qa-regression-results.txt`, `qa-regression-analysis.txt`, per-scenario
`build\bin\Release\qa-*.log`, captures copied to
`build\bin\Release\data\userdata\claude-tests-2026-11-11\qa2_*.png` and post-fix `qa3_*.png`.

**Save/load scope:** save **writing** is proven (campaign checkpoint autosave files written mid-run: count went 49→50,
`MvarWrite` + `Checkpoint... done` in the log). A full in-game "load an old save and resume" click-path was not exercised
automatically; see known issues. **Saved films** are proven by playback of a real .film.

## 3. Regressions found and fixes applied (no new features)

No crash/regression was found in the tested paths. Three **latent crash/instability options** were clamped/hidden as
requested (commit after rebuild, version unchanged):

1. **Campaign co-op MAX PLAYERS** (campaign picker): options were 4/8/12/16; 5-player campaign co-op is unproven and
   crashes (Tyler's Stage-2 report). Clamped to **4** (only proven count) - `src/ui/FrontEndPickers.cpp`. Verified with
   a capture of the opened picker: only "4 PLAYERS" (`qa3_campaign-maxrow.png`).
2. **Multiplayer MAX PLAYERS** (custom-game host picker): options were 4…128; 48+ are untested and 128 is on Tyler's
   hold, and Tyler reports multiplayer "stopped at 34 players". Clamped to **4/8/12/16/24/32** (16 stays the default;
   32 is the implemented 32-slot work). `src/ui/FrontEndPickers.cpp`.
3. **ENGINE LIMITS** (Settings > Game): the four profile options (Stock / Extended / Extended+Particles / Full) are
   force-disabled in this build (upstream `4407101` resets settings and forces Stock; non-stock caused black-screen
   loads and network kicks). The row is now **information-only ("STOCK")** instead of an unusable, previously
   crash-reported selector; ACTIVE ENGINE LIMITS explains Stock; AI ACTORS shows 96 (STOCK) disabled. `src/ui/SettingsPanel.cpp`.
   Verified with a filtered Settings capture (`qa3_settings-engine.png`).
4. **Test-harness fix** `d636b31` (already committed before this pass) - needed for the stress runs; no user-facing change.

Post-fix smoke: fresh Firefight run on the rebuilt dev gate: 0 crash, capture saved (`qa3_smoke-firefight.png`); both
gates build clean; `git status` tracked-clean after commit.

## 4. What this pass did NOT cover (needs Tyler / a second PC / a real session)

- Real two-PC lobby-code join (private and public). The codec and both UI entry points are proven; the network join is not.
- Multiplayer with real peers at 16/24/32 (this pass ran local-host sessions only). The clamp caps the UI at 32; the
  actual seat limits still need Claude's networking work.
- Campaign co-op 5-player failure reproduction (Stage 2 preparation was not started - feature work is paused).
- Full save LOAD/resume flow (writing checkpoints is proven).
- Forge editor (monitor) toggle inside Forge World.
- Tyler's hand retest of the campaign skulls row and the toggle-crouch/toggle-zoom controls.