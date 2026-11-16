# Agent coordination - Project Noble

Two agents may work on this repo. Read this file before touching anything. It lives in `knowledge\`
(git-excluded), so it is never committed or pushed.

## The agents

| Name | What it is | How it runs | Model |
|---|---|---|---|
| **lead** | DeepSeek Harness agent (this session) | DSH app in `C:\Users\tyler\Documents\deepseek-harness\default-workspace` | deepseek-flash (DeepSeek) |
| **cc** | Claude Code CLI | `C:\Users\tyler\Desktop\claude-deepseek.ps1` (interactive) or `tools\claude-deepseek-run.ps1 -Prompt ...` (one-shot, spawned by lead) | deepseek-v4-pro (DeepSeek API) |

Both share **one checkout**: `D:\Project-Noble-Closed-Source`. There is no lock service - the file
below and `Get-Process noble-dev, noble-client` are the only coordination.

## Scope ownership (default split)

| Scope | Owner | Notes |
|---|---|---|
| `src\` (all C++), `CMakeLists.txt` | lead | cc may read; may not write unless lead hands it a file |
| `tools\` (PowerShell/Python/F# test + tag scripts), `build\` | lead | cc may read |
| `data\` (palettes, packs, maps) | lead | cc may read |
| `knowledge\` - drafts, digests, compendium chapters | **cc** | lead assembles the final compendium and owns `00/01/02-*` |
| `knowledge\` - `00-START-HERE.md`, `01-PROJECT-HISTORY.md`, `02-LESSONS-AND-RULES.md` | lead | cc proposes changes in a draft, lead applies |
| Git commits / pushes / branch state | lead | cc never commits or pushes |
| Running the game / tests | lead | single GPU + monitor; see rules |

If a task crosses scopes, it is split, not shared. Write the split into "Active claims" first.

## Protocol

1. Read this file and `02-LESSONS-AND-RULES.md` before doing anything.
2. Add a claim line under **Active claims** with date/time, your name, scope and a one-line task.
3. Work only inside your scope. Never edit a file another agent has claimed.
4. Before running a test, check for `noble-dev` / `noble-client` and check Active claims for a test lock.
5. When done: append a dated result line under **Recent results**, remove your claim.
6. If you find a collision (a file changed under you), stop and report it; re-apply your change on top of
   the other agent's version. Never restore from memory.

## Commit and push cadence (Tyler, 2026-11-14)

Every **fully finished major feature** gets: `git pull --ff-only` → build the release gates (`noble-dev`,
`noble-client`) → commit → `git push origin wip-merge`. `main` moves only on Tyler's explicit word, after the feature
is verified. Half-finished work stays uncommitted; test-only, env-gated instrumentation may ride along with the
feature commit that used it. Never leave a finished feature uncommitted across sessions.

## Active claims

(none as of 2026-11-14 19:55 ET - every claim from earlier today is complete and recorded under Recent results.
Add your claim here before writing.)

| When | Agent | Scope | Task |
|---|---|---|---|
|  |  |  |  |

## Recent results

- 2026-11-15 12:55 - Cross-game comparison DONE for the remaining systems (Reach vs H3/ODST/H4) plus a ranked top-10
  next-work list. Extracted pool-constructor tables (Reach 110 / H3 83 / ODST 86 / H4 112 pools; raw CSV in
  `decompile-exports\topics\crossgame-pools-2026-11-15.csv`). Findings: later games raised AI actor/squad/objectives
  caps (96->256, 256->512, 64->125); H4 is the closest sibling and kept Reach's counts with larger per-entity records;
  no MCC engine exceeds 16 player slots; Reach already equals H4 for particle/effect/decal/light/render-state counts.
  Docs: `project-docs\CROSS-GAME-COMPARISON-2026-11-15.md`, `project-docs\TOP10-NEXT-UPGRADES-2026-11-15.md`.
  Nothing started; no code/build/release changes.

- 2026-11-15 12:40 - Documentation pass DONE (feature work paused): modder guide with all 68 engine hooks + measured
  MCC RVAs, tooling modules, launch flags (auto-generated 216-flag appendix), 38-rule variant editor and appearance
  data; player guide for Firefight/Marine/appearance; master upgrade list now marks every item PROVEN/FAILED/NOT
  STARTED (7/3/58) with MCC addresses; address table gained the proven MCC-only hook targets; TODO/00-START-HERE/
  handoff updated; all mirrored to `knowledge\`. Unverifiable items are flagged in the guides. No code changes
  (repo `b2065bd`, version 0.19.103, no release).

- 2026-11-15 12:05 - Stabilization pass DONE (feature work paused): change list + test proof in
  `project-docs\STABILIZATION-REPORT-2026-11-15.md`, 16-scenario regression in `QA-VERIFY-2026-11-15.md` (campaign,
  Firefight, multiplayer, Forge, Armory, server browser+codec, social, lobby share-code UI, saved-film playback, save
  write; default + max-settings attempt) = 0 real crashes. Crashable options clamped in `b2065bd` (campaign MAX
  PLAYERS=4, multiplayer picker <=32, ENGINE LIMITS info-only Stock); both gates rebuilt; pushed main+wip-merge;
  version still 0.19.103. Release notes/known issues drafted in `project-docs\RELEASE-DRAFT-AFTER-0.19.103-2026-11-15.md`;
  recommendation is HOLD (no new user-facing content since v0.19.103).

- 2026-11-15 11:35 - Phase-2 Stage 1 status report DONE (`project-docs\STAGE1-STATUS-REPORT-2026-11-15.md`),
  network boundary written (`project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`). Pool inspector proven in-match: heavy
  Firefight ff_unearthed top fills page-reference/script-node/character-cache 85.7/80.2/77.5%, object 71.9%; heavy
  Forge forge_halo (1,495 spawned objects) havok components 1,500/1,500 = 100%, page ranges 91%, object 76.5%.
  Blocking: Engine Limits profiles are force-disabled (upstream 4407101) - all runs Stock, banks 1/3/4 only ~5-32 KB
  headroom. Fixed test harness tick gate (`d636b31`, -horde/-spawn-test alive again since 2026-11-09); built both gates;
  pushed main+wip-merge (version unchanged, no release). Next: Stage 2 preparation only.

- 2026-11-15 10:50 - DeepSeek engine-upgrade phase opened (docs only, no `src\` change, no build/commit/release): O7's
  three requests recorded in `project-docs\NOTE-O7-REQUESTS-2026-11-15.md`; `00-START-HERE.md` got the handoff / prompt /
  master-list / address-table pointer rows plus the v0.19.103 current state; Halo CE decompile finish (79,637 fns, 2
  failed, 10:19 ET) recorded in `decompile-exports\README.md` / `HANDOFF-DEEPSEEK` / `01-PROJECT-HISTORY`;
  `decompile-exports\topics\mcc-address-table.md` started with the 5 proven hex-bug property-editor matches. Repo
  unchanged at `eea9868`; release/version and 128-player work still on Tyler's holds.

- 2026-11-15 07:45 - Social screen JOIN WITH CODE button added (ca770e2, capture social_join_code.png): the #13 plan''s last UI entry point is done; only the real two-PC host->code->join test remains.

- 2026-11-15 07:20 - Armour-effect colour research: dumped all armour_fx particles; colours are palette-bitmap driven (fire: fire.bitmap/covenant_fire.bitmap; storm/lightning: plasma_blue_01/blue_plasma). Recolour recipe (duplicate tags + gradients, ship via mod) written to project-docs/RE-ARMOR-FX-COLORS-2026-11-15.md; selectable new entries still need engine effect-table work.

- 2026-11-15 07:18 - Safe in-game test input (`c584c8a`): `-scoreboard-at` moved out of FillLocalInput into a synthetic
  overlay-mask path, so unfocused match captures can open Noble's scoreboard with no focus/mouse/cursor capture. Verified
  with `scoreboard_safe.png` (window not foreground, Tyler keeps mouse). Builds clean; release still held.


- 2026-11-15 07:10 - Visor engine-colour RE progress (no runtime patch): found the model shader-parameter palette with a
  literal `visor_tint` parameter (id 0x8018f; slot 6 in FUN_18024334c), the model parameter initialiser
  FUN_18032596c/FUN_180325a38, and the colour application path FUN_1803246b8/FUN_180324f90 feeding colours from the
  global appearance table DAT_182a03c58 + index*0x2ae (palette dwords +0x197..). MCC-style property names
  prop_visor_color_index/rgb/count/exist in the DLL, which suggests exact custom RGB is representable. Notes + next
  tests: `project-docs\RE-VISOR-COLOR-2026-11-15.md`.


- 2026-11-15 06:58 - Lobby share codes core DONE (`88d4937`): NB1 Crockford-Base32 codec (23 chars, CRC-8, self-test 0
  failures), Main Menu "JOIN WITH CODE" entry, Server Browser join-by-code modal (resolves lobby via master API), lobby
  HOSTING panel shows COPY CODE/share code for public+private lobbies. Captures `join_code_main/join_code_modal2/
  join_code_lobby.png`. Follow-ups: Social screen quick action + real two-PC join test. QA checklist:
  `project-docs\QA-VERIFY-2026-11-15.md`. Dev build gates rebuilt; no release (held).


- 2026-11-15 06:40 - Hex-bug runtime check (from Claude's hand-off NOTE-HEX-BUG-ROOT-CAUSE): temporary hook on hr+0x2D288C ran
  in solo MP (20_sword_slayer) and campaign (m35) for ~150 s total. The expander is exercised (stringId 20806 respawn,
  20983 armor lock, 52378 campaign) with caller hr+0x2D2592, but **no 8-hex-digit output leak and no `ctx+0x180 == -1`
  call appeared**; handles were large negative values, not -1. Static theory NOT confirmed; hook removed, builds clean.
  Full result: `project-docs\NOTE-HEX-BUG-RUNTIME-CHECK-2026-11-15.md`. Next step is the exact screen/mode repro or a
  trace of contexts that can have +0x180 == -1.
- 2026-11-15 06:20 - Items 5/6, 8, 9, 10, 11, 12 of Tyler's queue are DONE and pushed to main+wip-merge (latest
  `0efb619`): variant-drawer search/category filter, server-browser Refresh feedback + pinned FAVOURITE footer, clickable
  What's New hint, full-row Game Options clicks, controller toggle crouch, M&K toggle zoom. Per Tyler (06:05) version
  bumps/releases are HELD until the whole queue is done; one release at the end. Remaining: lobby share codes (13),
  visor engine colours (14), armour effect colours (15), multiplayer (16).


- 2026-11-15 05:35 - RELEASE v0.19.102 PUBLISHED (main = wip-merge = `8db1ce8`): public download/updater now carry the
  map-picker sections + FOLD, campaign MAX PLAYERS, visor stock-slot fallback (no white), Settings > Game Options
  alignment, FOV guard, details wrapping. Assets: base 357.1 MB sha256 bc200798..., Forge pack 652.6 MB sha256
  aca0b6e9...; release is GitHub "latest". Zip verified: elite_armor + noble_menu, zero RR names, version.txt 0.19.102.
  Dev build gates rebuilt from the same commit.


- 2026-11-15 05:25 - RECHECK + cleanup (`96fd2c4` + follow-up commit): re-verified all recent work. Settings Game Options
  tracks still aligned (8 camera tracks all x=855..1343 in the 1080p capture). Campaign/custom MODDED sections now also
  fold again by clicking the expanded mod's heading row (FOLD) - previously there was no way back once expanded.
  Visor root cause corrected: the engine only has five real visor slots; extended finishes were written as invalid
  slots and rendered white. Every finish now maps to a valid stock slot; exact custom tints are Armory-only until the
  shared-tag-heap/table work is done (see handoff "Visor colour follow-up"). Removed the broken -visor-cam-test hook and
  dead campaign-scoping helpers after they failed to produce a reliable front-view capture.


- 2026-11-15 05:10 - DOCS: added `project-docs\REFERENCE-LIBRARY-2026-11-15.md` (Ghidra MCP/how to map RVA vs image
  base, Desktop decompile exports, repo `docs\` multiplayer/limits/AI work, drafts, external tag/asset folders, test
  harness + every hook, Compendium mirror policy). Handoff/TODO/START-HERE/HISTORY/COORDINATION updated with what was
  fixed this session (commits, files, evidence, Ghidra addresses) and the live queue status; sync the changed docs into
  `C:\Users\tyler\Desktop\Compendium\`. `main` = `wip-merge` = `ec585f0`.
- 2026-11-15 05:05 - SETTINGS > GAME OPTIONS sliders aligned (`ec585f0`): slider track used to size from the current
  value text ("OUT 120 %" rows ended further left than "100 %" rows); fixed value column 132 * UI scale + labels
  ellipsized at the slider column. Evidence `settings_camera_aligned.png`, `settings_camera_crop.png`.
- 2026-11-15 04:55 - MOD BANNERS + VISORS (`8f467e1`): chevrons removed, count and "CLICK TO EXPAND" spaced from measured
  widths; in-game visor now resolves by visor id -> MCC slot with the stock Black/Gold swap (Armory catalog order vs MCC
  item order had violet/ice/crimson/onyx swapped). Engine truth: Ghidra `0x18003B2F4`, patch `0x2EDC5E`,
  `0x1800393CC`. New `src\common\ArmoryVisor.hpp` used by ReachHost + ElitePack. Evidence `banner_clean_camp.png`,
  `banner_clean_custom.png`; mapping check violet->10, ice->7, crimson->8, onyx->9, black->4, gold->3.
- 2026-11-15 04:45 - CAMPAIGN/CUSTOM PICKERS (`5f99735`): collapsed mod sections (click expands; search still expands and
  filters), campaign MAX PLAYERS 4/8/12/16 wired to HOST CO-OP, campaign search clamp, ASCII "-" drawer header (the "?"
  placeholder bug), new repeatable `-menu-click-at` + `-menu-debug-layout` test hooks. Evidence `camp_modded_new.png`,
  `camp_mod_click.png`, `camp_players_click3.png`, `camp_players_pick16.png`, `custom_modded_new.png`.
- 2026-11-15 04:05 - FOV GUARD (`3a3c98e`): `CameraFov::Apply()` refuses to write while `ReachArmory::IsMenuMode()`, so
  saving SETTINGS > VIDEO no longer zooms the Armory model; `CameraFov::Tick()` applies the saved FOV at match start.
  Test hook `-fov-apply-test-at`; evidence `fov_fixed_0/1.png` + log "not applying the match FOV here".
- 2026-11-15 03:15 - SERVER-BROWSER DETAILS WRAP (`d422a74`): mod/version warnings wrap inside the pane and sit under the
  server address; details photo/row heights trimmed so DOWNLOAD MOD & JOIN is visible without scrolling. Evidence
  `sb_wrap2/3/4.png`.

- 2026-11-15 03:55 - RELEASE v0.19.101 PUBLISHED (main = wip-merge = `ada9f5d`): upstream v0.19.100 (`7026ab1`) merged
  (fast-forward with no conflicts - upstream had merged our commits), version bumped to 0.19.101, release notes +
  changelog added, both gates rebuilt. Release assets: `Project-Noble-v0.19.101-win64.zip` 357.1 MB
  (sha256 13a65d65...), `Noble-ForgeWorld-v0.19.101-win64.zip` 652.6 MB (sha256 d64122fd...), both uploaded and
  published at https://github.com/O7OF/Project-Noble-/releases/tag/v0.19.101. Zip verified (elite_armor pack + noble
  menu map, zero RR-named entries). Dev build pushed to GitHub main/wip-merge in the same commit.
- 2026-11-15 02:40 - LOADING SCREEN REDESIGN PUSHED (`93f61dd` + upstream merge `ea572ae`, `main` = `wip-merge` =
  `ea572ae`): top-left LOADING / WAITING FOR PLAYERS block (title, map+progress / joined-of-total status, thin bar,
  lobby roster with HOSTING/READY/WAITING rows, PRESS T FOR CHAT), top-right mode label, complete (uncropped) map
  picture over a dim soft copy, Reach "Auntie Dot" diamond grid behind the picture (drifting + wave), Noble/UNSC emblems
  bottom corners. `BlamEngine.cpp` publishes the remote-player membership wait so the host's WAITING FOR PLAYERS screen
  stays up instead of going black. Art extracted from HREK via ManagedBlam, kept in `assets\ui\noble\loading` (not
  committed). Captures: `final_load_camp/mp/ff/forge.png`, `final_load_wait.png` in the claude-tests folder.
- 2026-11-15 02:30 - SERVER BROWSER OPTIONS: `-server-browser-style 1|2|3` added (1 = shipped amber tactical cards,
  2 = large blue-glass cards, 3 = compact one-line table) with the existing `-server-browser-demo` rows for captures;
  captures `sb_style1/2/3.png`. Default stays 1 until Tyler picks.
- 2026-11-15 02:05 - Merged upstream v0.19.99 (`45f365d`, register-clobber/JIP/Workshop changes) into the loading-screen
  branch; only `src/engine/BlamEngine.cpp` conflicted (their membership-wait rework vs our waiting flag) - kept their
  elapsed/settle logic and re-added the instrumentation. Gates rebuilt; loading screen + server browser smoke captured.

- 2026-11-14 20:40 - KNOWLEDGE REFRESH (docs only): handoff/TODO/START-HERE/HISTORY/BRIEF/this file updated for the late
  wave. Repo now at `aae1675` on `main` and `wip-merge`; releases v0.19.90 and v0.19.91 now carry the optional Forge
  World asset (ours, attached); the RecommendedMaps fallback fix is on `main` but not yet in any published executable.
- 2026-11-14 20:31 - PUSHED `aae1675` to `main` and `wip-merge`: `RecommendedMaps` now scans `releases?per_page=10`
  newest-first and uses the newest release that carries a `Noble-ForgeWorld-*.zip` (upstream published v0.19.90/v0.19.91
  base-zip-only, which had broken the in-client download). Tested three ways with a local release JSON: newest release
  without the pack -> uses the older release's pack (download, checksum ok, install marker); newest with pack -> uses it;
  no release with a pack -> clean error. Real GitHub list resolves the v0.19.90 asset. Also attached
  `Noble-ForgeWorld-v0.19.90-win64.zip` and `...-v0.19.91-win64.zip` (+sha256, 652.6 MB, sha `32c04269...`) to those
  releases so already-shipped clients find the pack in the latest release again.
- 2026-11-14 20:20 - MERGED upstream `a576e43`/`8f7a5b8` (v0.19.90) and `2ec2b49` (v0.19.91) into `wip-merge`
  (fast-forward, no conflicts) and rebuilt `noble-dev` + `noble-client`. Retested: theater saved-film playback with
  `-film <film>` (map/mode resolved from the film, 50 s playback, captures `theater_film_m10_25s/40s.png`), Armory
  preview after the FrontEndMenu merge (depth still 1920x1080, Marine in front of the lattice,
  `armory_after_v01991_42s.png`), and a Sword Base match (elite pack loaded, 67,051 pointers relocated, session
  started, `match_swordbase_45s.png`).
- 2026-11-14 19:55 - KNOWLEDGE REFRESH (docs only, no code): handoff `HANDOFF-NEXT-AGENT-2026-11-14.md` rewritten for
  the evening state, `TODO-MASTER-2026-11-14.md` updated, `00-START-HERE.md` / `01-PROJECT-HISTORY.md` / `AGENT-BRIEF.md`
  refreshed. Recorded: upstream `origin/main` is `a576e43` (v0.19.90, not merged locally, merge conflict-free);
  upstream's v0.19.90 release has no `Noble-ForgeWorld-*` asset so the in-client Forge World download errors until an
  asset is attached or `RecommendedMaps` falls back to older releases.
- 2026-11-14 19:07 - PUSHED AND RELEASED v0.19.89 (`0e8ca81`, `main` = `wip-merge` = `0e8ca81`): the Elite armour pack
  no longer ships RR names - `elite_armor.map` + Noble's own `NBRL` `.noblereloc` sidecar, `.rremotes`/`.rrneeds`/
  `.rrpmcg` removed, embedded table `spartan_armor_parts.txt`; loader and publish script updated. Verified in a
  multiplayer match: 127,379 packed pointers relocated, extra cache `haloreach\maps\elite_armor` load returned 1, pack
  Elite biped applied. Release assets: base 356.9 MB (sha 8e0ec141...), Forge World 652.6 MB (sha 32c04269...).
- 2026-11-14 18:50 - PUSHED `bd420bf`: de-RR the embedded item->region table, drop unused RR sidecars (no code change to
  behaviour; 191 Armory items map identically).
- 2026-11-14 18:27 - PUSHED `8c0f091` (v0.19.87): Armory lattice was masking with the 1600x1600 shadow-map depth buffer;
  now picks the depth matching the swapchain aspect (1920x1080 D24S8) and keeps it across frames. Proof:
  `ArmoryDepth: ... using 1920x1080 cleared to 0`, captures `armory_fix4_1.png` / `armory_nopack_1.png`. Menu music 15%.
- 2026-11-14 17:53 - PUSHED v0.19.86 (`97fa1a3`): `mainmenu_all.map` (285 MB) retired from the package; base zip
  607 -> 356 MB; verified with mainmenu_all.map absent that the VFS serves `noble_menu.map`, the Armory finds its
  GLOBALS table and the Marine tab/customisation works.
- 2026-11-14 17:37 - PUSHED updater fix (v0.19.85): update/reinstall had downloaded the Forge World pack because the
  loose asset match took any `Project-Noble-*` zip; now the exact client zip is required and the optional pack is named
  `Noble-ForgeWorld-*` so old clients cannot match it. Engine limits defaulted to Stock outside Forge in the same line.
- 2026-11-14 17:05 - PUSHED v0.19.84 audio fix: the double gain stage (session volume plus engine mixer) is gone, MASTER
  MUTE is menu-only again, MUSIC/OTHER write the live mixer floats mid-match, background mute kept.
- 2026-11-14 16:52 - PUSHED release split: Forge World is a separate optional asset, and a RECOMMENDED MAPS download
  row was added to MODS. Tested 16:57 end-to-end against a local HTTP server: downloading -> checksum ok -> installed;
  screenshots `rec_ui.png` / `rec_ui2.png`.
- 2026-11-14 16:30 - ARMORY SPECIES SWITCH FIX pushed to MAIN (`bc1f011`). Tyler's video (16:19) showed SPARTAN/ELITE
  tabs still displaying the MARINE preview for seconds and the Marine looking wrong/"blue" after Female Marine ->
  Spartan. Cause: every species flip used the Elite settle path (`kSpeciesSettleMs` 350 ms + rebuild-busy), which
  exists only for the 0x0B Elite pedestal camera; it kept the previous character (and its colour sync) on screen
  while the new species waited. Fix: only Elite-involved flips wait now; Spartan<->Marine/Female apply at once. The
  "species switch to X applied" log now names the real species instead of always "Elite". Both gates build.
- 2026-11-14 16:10 - TROOPER MODE REMOVED at Tyler's request and pushed to MAIN (`7578b41`): CAMPAIGN is back to
  VANILLA / MODDED with the original title/kicker/search/hint; `-trooper`, the TROOPER tab, trooper-mod
  auto-detection, `SettingsData::campaignTrooper`, `src/engine/TrooperMode.*`, the NOBLE_TEST_SKIP_CINEMATICS Space
  pulses and the DevCheats vitals probe are all gone. Kept: the cutscene-recovery opt-in fix
  (`NOBLE_CUTSCENE_RECOVERY=1`, default off) and the dedicated-server link fix. Both release gates build; vanilla
  m20 campaign spawn verified normal (Spartan in the courtyard, HUD fine). `origin/main` = `7578b41`; opening a
  trooper-style campaign is now only through the MODDED tab with a mod that ships a player-capable Marine.
- 2026-11-14 15:57 - REGRESSION FOUND + FIXED + PUSHED TO MAIN (`b357a93`). The upstream 0.19.82 merge added a
  cutscene "recovery" (detour on `cinematic_skip_stop_internal` + skip checks in `FillLocalInput`/pad input) that
  dispatches `cinematic_stop`, `camera_control 0`, `player_control_fade_in_all_input 0`, `unit_raise_weapon`,
  `object_can_take_damage (players)`, `PauseMenu::CancelGameOver` and wipes the rasterizer fade every frame. Pressing
  skip during a mission's opening cinematic broke the intro: player spawned out of bounds, no arms, input locked
  (Tyler on m20, 15:45 log). Fixed by gating the whole recovery behind `NOBLE_CUTSCENE_RECOVERY=1` (off): detour not
  installed, `OnFrameTick` fade wipe off, ReachHost skip checks no longer call `RecoverFromCutsceneSkip`; skip goes to
  the engine as before. Verified on m20 (`cutscene recovery disabled` logged, no command dispatch, 30 s capture
  normal on-foot DEFEND COURTYARD). Both release gates built. `origin/main` = `b357a93`.
- 2026-11-14 15:20 - FEATURE DONE + PUSHED: CAMPAIGN > TROOPER now auto-uses an installed player-capable Marine mod
  (commits `e1c650b` feature, `fdaeefe` probes; `wip-merge` at `fdaeefe`). When the TROOPER tab opens, Noble scans an
  installed campaign mod's first map for the ASCII token `marineplayer` (mod maps are uncompressed) — found in
  Steam Workshop "Improved Trooper Campaign" item 3127073871, whose maps carry
  `objects\characters\marine - copy\marineplayer2` — and launches stock missions with `-mod <folder>`; otherwise it
  falls back to `-trooper`. Verified: log "using player-capable Marine assets from mod ...", child resolved
  `haloreach\maps\m10.map` to the workshop map, session started. This also fixes the shield-HUD WARNING in-game
  (that mod's biped has the player health setup); the built-in swap still warns. Witnesses: `modtest_capture_*`,
  `trooper_mod_launch_*` in the claude-tests folder. Both release gates build.
- 2026-11-14 14:37 - FEATURE DONE + PUSHED: CAMPAIGN > TROOPER menu option (commit `7659de1`, pushed `wip-merge`
  to `7e29b3c`). Three source tabs VANILLA / TROOPER / MODDED (n); TROOPER appends `-trooper` to the match launch;
  choice persists as `SettingsData::campaignTrooper` and restores on the next menu visit; `-menu-tab 1` still opens
  MODDED. Verified with menu captures (tabs + restored TROOPER tab) and end-to-end `-menu-screen campaign -auto-play`
  -> child logged `TrooperMode: on for m10`. Both release gates built. Also committed `7e29b3c` (env-gated model
  dump/force/jmad test hooks). Witness: `campaign_tabs_vanilla.png`, `campaign_tabs_trooper.png`,
  `menu_trooper_capture_900001.png` in the claude-tests folder.
- 2026-11-14 14:07 - Marine legs: animation-graph swap test FAILED but informative. `hlmt`'s animation reference
  is at `+0x20` (datum `+0x2C`); `NOBLE_TROOPER_JMAD=1` copies the Spartan model's `jmad` reference onto the
  marine model (both groups `jmad`; 0xE771059A -> 0xA1503767, copy verified). The player then died ~7 s after
  spawn every time ("Respawn in..." at 14:04:29), so the Spartan graph cannot drive the marine unit. A fix needs
  a marine-specific player animation graph authored in HREK/Blender; remaining path recorded in compendium §4.6.
- 2026-11-14 13:32 - PUSHED TO MAIN on Tyler's instruction. `wip-merge` commits 540d450 (bank-3 beam guard +
  dedicated link fix), 0d51b92 (no-activate launch, unfocused cutscene skip, model dump) and 3b83ec9 (merge of
  origin/main 0.19.79-0.19.81) fast-forwarded into `main`. Release gates `noble-dev` and `noble-client` built and
  linked before the push. Both `origin/main` and `origin/wip-merge` are now `3b83ec9`. Working tree clean except
  four tracked armory PNGs that tests rewrite.
- 2026-11-14 13:17 - Marine legs: permutation hypothesis REJECTED. Forcing region 2, then all 15 regions, to
  permutation 0 (`NOBLE_TROOPER_FORCE_REGION`/`_PERM`, 15/15 `rc 0`) leaves the 150 s on-foot capture unchanged.
  `m35.map` has one marine biped only (`objects\characters\marine\marine`, NPC) and no `marineplayer2`. New lead:
  the NPC animation graph has no player standing stance, so the model folds; next test is a player-capable jmad
  (or a pack-delivered marine player biped).
- 2026-11-14 13:04 - Test harness fixed: tests now launch the game without activation (`SW_SHOWNOACTIVATE` via
  CreateProcess in `camp_test.ps1` and `match_elite.ps1`) and the cutscene-skip Space pulses moved BEFORE the
  focus gate in `FillLocalInput` (`InjectTestSkipInput`), with `camp_test.ps1 -SkipCinematics` defaulting to 75 s.
  Verified: zero "mouse captured" lines and the 45 s capture is gameplay, not the intro.
- 2026-11-14 12:59 - `NOBLE_MODEL_DUMP=1` runtime dump added (`TrooperMode::DumpModelState`): logs the local
  unit's biped/hlmt/model, object variant field, and every region's name string-id, permutation count and applied
  permutation. Marine m35: 15 regions, variant 255, only 4 regions set; Spartan: 15 regions, 12 set. Under the
  HREK `marine.model` variant-0 (`trooper_light`) ordering, runtime index 2 (11 perms, unset) is `legs`.
- 2026-11-14 12:40 - Ghidra MCP bridge is live: GhidraMCP 2.0 extension installed under
  `Ghidra\Extensions\GhidraMCP` (one copy only), headless PyGhidra server on `127.0.0.1:8081`, launcher
  `tools\ghidra-mcp-start.ps1`, DSH bundle `@local/dsh-mcp-ghidra` (tools as `mcp__ghidra__*`). First analysis of
  `haloreach-copy.dll` was still running at 12:40; verify with `get_program_info` before relying on it.
- 2026-11-14 12:13 - Compendium Appendix C added: tools, engines and reference material (GDC talks incl. Halo:
  Reach Effects Tech / Chris Tchou and I Shot You First / David Aldridge, Microsoft Learn MCC docs,
  c20.reclaimers.net, HREK tools, Assembly, ManagedBlam, Reclaimer, Bonobo/Baboon/Foundry, Blender, Ghidra, Discord
  sources, installed-path table, gaps). Derived from cc's round-3 draft, trimmed and merged; link list expanded
  from a GDC Vault search. cc's draft kept in `drafts\draft-tools-resources.md`.
- 2026-11-14 12:10 - Compendium round 2 merged: `PROJECT-NOBLE-COMPENDIUM.md` now has chapters 4 (models, armour,
  animation, including the open Trooper-leg case) and 5 (Forge palettes + pack pipeline, ElitePack, Pelican/
  Spectre/Boathog, Option 2 plan); tooling/bugs/open questions renumbered. cc's drafts kept in `drafts\`.
  Fixed cc's noted path error (`ForgeMenu` is `src\ui\`, not `src\engine\`).
- 2026-11-14 12:01 - Compendium v1 assembled: `PROJECT-NOBLE-COMPENDIUM.md` (335 lines) from cc's two drafts plus
  the repo docs and today's evidence. Corrections applied to cc's memory draft: live BANK values replaced with the
  2026-11-14 10:52-11:09 observations (bank 1 7,860,328 / 8,519,680; bank 4 51,656,592 / 52,756,480 instead of the
  older overflow numbers), and an unverified DLL hash removed. `AGENT-BRIEF.md` is the two-page bootstrap.
  Still to draft: model/animation chapter, Forge/pack pipeline chapter, UI/audio notes.
- 2026-11-14 11:56 - Marine legs PARKED in the TODO (model-data lead recorded). Compendium work started:
  `00-START-HERE.md` current state refreshed, `AGENT-BRIEF.md` written (two-page bootstrap for any agent),
  `knowledge\drafts\` assignment ready for cc.
- 2026-11-14 11:52 - Marine legs: third hypothesis REJECTED. A pulsed Space hold (2 s down / 0.5 s up, the
  press-and-hold Tyler described) across the intro window changed nothing at 90/120/150 s. The pose is not a
  cutscene. Remaining evidence: unit shadow shows a standing figure while the render model is folded at the
  hips, so the fault is in the model data applied to the swapped biped (regions/permutations/nodes), not in
  pose, armour choices or timing. Next: dump the marine model's region/permutation/node data from m35.
- 2026-11-14 11:49 - Marine legs: two hypotheses tested and REJECTED. (a) Default profile appearance
  (`appearance.dat` removed) - same legless/folded render. (b) Cutscene pose - a test-only synthetic Space hold
  (`NOBLE_TEST_SKIP_CINEMATICS=<s>`, added to `FillLocalInput`) for 30 s made no difference at 90/120/150 s.
  The Marine stays folded/kneeling while its shadow shows a standing figure, so the likely fault is the model's
  region/permutation set (Spartan representation data on the marine model) or a node/skeleton mismatch, not pose or
  armour choices. Next: dump the marine model's regions/permutations from m35 and the live unit's permutation state.
- 2026-11-14 11:41 - Covenant airstrike palette CONFIRMED working in Forge by Tyler (blocker closed).
- 2026-11-14 11:45 - Marine legs diagnosed (tag + code): the campaign marine biped `objects\characters\marine\marine`
  has its own model (`marine.mode`) and animation (`marine.jmad`); `MarineArmor` applies nothing on stock maps
  ("0 of 31 names found", "is not a Marine"). The likely cause is the player's **Spartan armour profile**
  (`ReachHost::ApplyArmoryAppearance`, `profile+0x..` piece ids) being applied to the swapped marine model, whose
  regions/permutations don't match - shown as a bald head and missing legs. Test to prove: same trooper mission with
  a default/renamed `appearance.dat`. Fix candidate: skip/zero the Spartan piece ids when Trooper mode is requested.
- 2026-11-14 11:34 - cc route verified: Claude Code 2.1.323 -> `api.deepseek.com/anthropic`, model
  `deepseek-v4-pro`. The `[1m]` suffix is rejected by the DeepSeek API (400); both launchers fixed.
  One-shot route `tools\claude-deepseek-run.ps1` returns output; key read from the User environment only.
- 2026-11-14 11:26 - Forge World rebuilt with `target_laser_covenant` (+7 Covenant variants) and installed in
  `data\maps`; VFS serving confirmed by a Forge load test.
- 2026-11-14 11:24 - Marine legs reproduced on m35 campaign Trooper mode: same rendering as the pre-merge
  `tr35h` capture, so it is not a merge regression.
- 2026-11-14 10:52 - lead found the merged-build session-end cause: v0.19.69 runtime beam pools added
  ~9.2 MB to game-state bank 3 (58.6 MB -> 68.05 MB of 67,043,328), silent bump-allocator overflow.
  Guard added in `PoolResize.cpp`; Sword Base ran 70 s clean afterwards.
- 2026-11-14 11:12 - lead tested `a3a4d50`: third person lock holds, F5 ignored, captures `trlock_00/01`.
- 2026-11-14 11:20 - lead added `target_laser_covenant` (+7 Covenant variants) to `ff_weapons_covenant`.
- 2026-11-14 11:16 - lead added `-NoFocus` to `camp_test.ps1` / `match_elite.ps1`: tests no longer take
  Tyler's mouse; F5 needs focus and does not register unfocused.

## Test rules that apply to both agents

- Never test while Tyler is playing: `Get-Process noble-dev, noble-client`.
- Windows on the AOC (second monitor), never the Samsung: `NOBLE_WINDOW_SECONDARY=1`.
- Back up/restore `build\bin\Release\data\appearance.dat` around every test (the scripts do).
- Do not take the mouse when using `-NoFocus`; ask Tyler for a focus window if a hotkey is required.
- One game instance at a time - a test lock is exclusive.
- Only `noble-dev` and `noble-client` are release gates; the dedicated build does not block a push.
- 2026-11-15 07:15 - Scoreboard hexbug follow-up cancelled by Tyler (he confirmed Noble has no old scoreboard visible). Code check: Noble suppresses the engine scoreboard key and draws its own panel; it does not draw over/hide an engine widget because the engine screen never opens. Temporary hook/test flags removed; tree clean, builds 0.19.102.
- 2026-11-15 08:20 - Armour-effect recolour proof attempted: HREK bitmap import does not commit pixel data for fire.bitmap (exporter reports rasterizer\invalid; importer says in-game size 0). HREK fully restored; next route is ManagedBlam import or patch-tag-field + rebuild Forge World. Details in RE-ARMOR-FX-COLORS-2026-11-15.md.
- 2026-11-15 08:35 - ARMOUR EFFECT RECOLOUR PROOF: patching head_fire.particle's palette reference to plasma_blue_01 via ManagedBlam + rebuilding Forge World with tool.exe gives blue/cyan Legendary flames in-game (captures blueflame_0/1.png). Dev hook -armor-effect-test <0..6> committed (64e6d22). Original tag/profile restored; blue variant saved as head_fire_blue.particle. Next: expose variants in Armory.