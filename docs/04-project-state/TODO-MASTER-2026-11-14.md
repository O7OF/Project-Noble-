# Project Noble - master to-do (updated 2026-11-15 12:40 ET)

## ARMOR FX PRIMARY-COLOUR FLAMES - 2026-11-17 (24/30 working; runtime path chosen)

**Read first:** `HANDOFF-ARMOR-FX-COLOURS-2026-11-17.md` in this folder, plus
`../05-research-and-prompts/RE-ARMOR-FX-RUNTIME-COLOR-PLAN-2026-11-17.md` and
`../project-docs/RE-ARMOR-FX-VARIANTS-2026-11-16.md`.

State: **24 of 30 colours working and selectable** (steel..rose); 6 hidden (rust/coral/peach/gold/yellow/pale).
Working build committed as `789c950`; build clean. Armory verified clean for NONE/Birthday, correct for Steel/Brick and
PRIMARY MATCH auto-green; in-match verified via Forge World override and Boneyard VFS mod. Stock flame stacking fixed by
markerless `armor_fx_default` perm0 + shifting stock perms/markers.

Next tasks (priority):
- [ ] **T1 runtime particle colour override** - chosen path; no map rebuilds, all 30 colours. See runtime plan doc.
      Ghidra MCP is declared but not callable through run_code's `tools` object right now - fix/verify MCP first.
- [ ] T2 prove runtime tint (magenta local flame on stock map, no rebuild/override) then per-effect/per-player.
- [ ] T3 restore the four cosmetic attachments dropped for the 24-colour build (night_vision.light,
      bungie_helmet.cheap_light, flashlight_1p.light, shield_down.light) once runtime retires extra attachments.
- [ ] T4 re-add the 6 hidden colours (catalog rows, thumbnails, Armory list -> 38 entries) once runtime works.
- [ ] T5 decide map-mod route disposition: batch paused after Forge World + Boneyard (1.3 GB mod folder);
      script `test-tools/build_all_mod_maps.ps1` can resume. Full set estimate ~25-30 GB.
- [ ] T6 thumbnails/photos: Tyler handles the effect photo set; update `data/ui/reach/armory/armor_effects/flame_pri_*.png`.

Gotchas: 5 effect attachment total gating limit; effect locations cap 6 and Save drops extras; menu CSV must be
regenerated after every mainmenu build; ext tag location names must be real `flame_pri_*` markers; stale game processes
fake captures; PowerShell `>` writes UTF-16 and can corrupt sources.


Live list. Newest state at the top. **Read `project-docs\REFERENCE-LIBRARY-2026-11-15.md` before researching anything**
(Ghidra MCP, decompile exports, repo `docs\` multiplayer work, external tag/asset folders, test hooks). Detail:
`project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` + `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md`
(engine-upgrade phase); earlier state in `project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md` (05:10 ET) and
`01-PROJECT-HISTORY.md` Phases 8-13.

## DOCUMENTATION PASS - DONE 2026-11-15 12:40 ET (feature work still paused)

- Modder guide: `project-docs\MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md` - all 68 engine hooks with MCC RVAs (measured
  from a live startup log) + what/when/example/limits, the host tooling modules (PoolReport/PoolResize/EngineLimits/
  SpawnTest/MvarDump/FrameCapture), launch/test flags (detailed + an auto-generated appendix of all 216 flag literals),
  the 38-rule game-variant editor and its JSON format, appearance/profile data, and an explicit 'could not verify' table.
- Player guide: `project-docs\PLAYER-GUIDE-FIREFIGHT-MARINE-APPEARANCE-2026-11-15.md` - Firefight setup (10 maps, 7
  game types, MAX PLAYERS 4/8/12/16 unverified beyond 4, difficulty, 38-rule GAME OPTIONS, no skulls/teams in Firefight),
  the Armory Marine tab + 9 Marine/ODST slots, appearance (30 visor finishes -> 5 stock slots in match, 7 selectable
  stock armour effects + 12 Firefight voices, Elite sets, colours incl. custom wheel, nameplates/emblems, rank gates).
- Master upgrade list: every one of the 68 items now marks **PROVEN / FAILED / NOT STARTED** with the MCC address where
  known (7 PROVEN, 3 FAILED, 58 NOT STARTED) - `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`.
- Address table: new 'Proven MCC-only addresses' section (pool init `hr+0x12FCC`, bank table `hr+0x27BF270`, per-player
  tick `hr+0x61A38`, ai_place `hr+0x6C2B10`, object spawn `hr+0x49CC24`/`hr+0x46D6E0`, horde teleport/team/blind hooks,
  saved-film monitor `hr+0x3ABE70`) + startup-patch table - `decompile-exports\topics\mcc-address-table.md`.
- Doc errors fixed while documenting: the earlier player-guide draft claimed Firefight had skulls and that armour
  effects were not selectable; the code inventory shows Firefight forces skulls 0 and the 7 stock armour effects ARE
  selectable (only new recoloured variants are dev-proof). Corrected in the final player guide.
- **Unverified (flagged in the guides):** horde-full-lod/infantry paths, Alert-Carry behaviour, tag_get diagnostics,
  engine-side 128/16 gates, extended profiles, -engine-cmd, individual settings rows, Firefight variant-rule
  behaviour, Armory in-match visuals beyond visor/effects, mod packaging/.mvar round-trip, Forge editor monitor.
- No code changes were needed (no code errors found that the docs depend on); repo still `b2065bd`, version 0.19.103.

## STABILIZATION PASS - DONE 2026-11-15 12:05 ET (feature work paused)

- Scope: full regression of everything since the v0.19.103 baseline; record in QA-VERIFY; fix regressions only;
  clamp/hide crash-capable options; draft release notes + known issues. Docs:
  `project-docs\STABILIZATION-REPORT-2026-11-15.md`, `project-docs\RELEASE-DRAFT-AFTER-0.19.103-2026-11-15.md`,
  `project-docs\QA-VERIFY-2026-11-15.md` (new section).
- Changes since the v0.19.103 release: exactly one commit, `d636b31` (test harness tick gate fix; dev-only). The
  v0.19.103 feature delta (drawer filters, server-browser footer, click fixes, lobby share codes, social button,
  test hooks) was regressed too; per-commit test-proof table is in the stabilization report.
- QA: 16 scenarios, **0 real crashes** - campaign m10 (checkpoint autosave written), Firefight ff_unearthed,
  multiplayer 20_sword_slayer (local host), Forge forge_halo, Armory, server browser + codec self-test (0 failures),
  social, lobby demo with NB1 share code, saved-film playback; plus the same four modes with extendedLimits=3 +
  aiActors=1024 pre-set (build resets to Stock, no black screen, no crash). Captures `qa2_*.png`, post-fix `qa3_*.png`.
- Clamps added, commit **`b2065bd`** (main = wip-merge, pushed; **version still 0.19.103, no release**): campaign MAX
  PLAYERS = 4 only (5-player co-op crashes/unproven); multiplayer MAX PLAYERS picker capped at 32 (16 default; 128
  hold); ENGINE LIMITS information-only Stock (profiles are force-disabled); AI ACTORS shows 96 (STOCK).
- Not covered (needs Tyler/second PC): real two-PC lobby-code join; multiplayer with real peers; campaign >4 repro;
  full save LOAD/resume; Forge editor monitor toggle; Tyler's skulls-row/toggle-crouch retests.
- Recommendation: **hold the release** - no user-facing feature content since v0.19.103; only ship a small stability
  release if Tyler wants the crashable options removed from users' builds. Then bump/publish only on "release it".

## ENGINE-UPGRADE PHASE - DeepSeek (opened 2026-11-15 10:40 ET)

- Read first: `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` (git/release rules + phases 1-6) and
  `project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` (briefing, tasks T1-T10). Pointers at the top of
  `00-START-HERE.md`; candidate work in `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`.
- Done 10:50 (docs only, no `src\` change): O7's three requests recorded (`project-docs\NOTE-O7-REQUESTS-2026-11-15.md`);
  `decompile-exports\topics\mcc-address-table.md` started with the 5 proven property-editor matches; Halo CE decompile
  completion (79,637 functions, 2 failed, 2026-11-15 10:19 ET) recorded in `decompile-exports\README.md` + this list.
- Next (phase 1): RTTI/vtable check on `haloreach.dll`; engine-state pool inspector over the ~196 pools; free
  diagnostics (net latency overlay, AI debug draw, crash minidump path).
- Rules: prove with a read-only runtime probe before any write; no release/version bump/public push/tag/upload unless
  Tyler says "release it"; 128-player work ON HOLD until Tyler lifts it.

## PHASE-2 PROMPT - Stage 1 DONE 2026-11-15 11:35 ET; Stage 2 next (preparation, awaits Tyler)

- Report: `project-docs\STAGE1-STATUS-REPORT-2026-11-15.md`; network/security boundary:
  `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`.
- Repo state 11:35 ET: `main` = `wip-merge` = **`d636b31`** (dev test-harness fix; version still 0.19.103, no release).
- Pool inspector proven in-match (124 named pools + 8 banks). Heavy Firefight (ff_unearthed): page reference ranges
  85.7%, script node 80.2%, character properties cache 77.5% (31/40), object 71.9%, water physics 88.5%. Heavy Forge
  (forge_halo, 1,495 spawned objects): havok components 100% (1,500/1,500), water physics 100%, page reference ranges
  91%, object 76.5%, tag cache 74.4%.
- **Blocking finding:** Engine Limits (V16/V16P/Full) are force-disabled in this build (`Settings.cpp:253-261` reset
  + non-Forge forced Stock, upstream `4407101`); Stock game-state banks have only ~5-32 KB headroom in banks 1/3/4 ->
  count raising needs the Stage 3 fix + re-enable, and Firefight/multiplayer re-enable needs Claude's profile matching.
- Fixed the test harness (`d636b31`): the per-player tick was gated on a unit handle instead of the player index, so
  `-horde` / `-spawn-test` / dev cheats / cinematic gating were dead since 2026-11-09. Both gates built clean.
- Next: Stage 2 preparation only (reproduce the 5-player campaign crash and the 34-player MP stop, find the
  non-network causes, clamp UI rows to proven counts, put network blockers in the handoff note). Stage 3 black-screen
  fix before any count raising; 128-player still on Tyler's hold; no release without "release it".

## BUILD/DEPLOY QUEUE - Tyler's ordered bug list (started 2026-11-15 04:30 ET; v0.19.103 released 09:57 ET - see the v0.19.103 block below)

Rule from Tyler: work in this order, build + push to `main` after EACH completed fix; multiplayer is last.
Repo state when this queue was written: `main`=`wip-merge`=`c584c8a` (v0.19.102 was then the public release; all later fixes
are dev-only and the next release is held until the queue is done, per Tyler 06:05 ET). Commit-message style: subject + body paragraphs (no trailers).

1. [REMOVED 07:45] 3rd-person camera spazzing: Tyler reports another user already pushed a fix, so this is removed from
   the queue. No further work unless the issue comes back.
2. [DONE] Armory "scroll options misaligned" turned out to be SETTINGS > GAME OPTIONS camera sliders -> `ec585f0`
   (fixed value column 132*UI scale, labels ellipsized at the slider column). The FOV-scroll-wheel zooming the Armory
   model was fixed earlier in `3a3c98e` (FOV is match-only now; `CameraFov::Apply` no-op while `ReachArmory::IsMenuMode`).
   Evidence: `settings_camera_aligned.png`, `settings_camera_crop.png`, `fov_fixed_*.png`.
3. [DONE] Campaign/custom/firefight/lobby MODDED tabs: collapsed one-section-per-mod banners, click/Accept expands;
   expanded sections fold again via the FOLD action on the mod heading row; search still expands+filter all. `5f99735`
   + follow-up. Evidence `camp_modded_new.png`, `camp_mod_click.png`, `custom_modded_new.png`, `recheck_fold2_*.png`.
   Banner polish (chevrons removed, count/expand spacing) in `8f467e1`; evidence `banner_clean_camp.png`,
   `banner_clean_custom.png`.
4. [DONE] Campaign MAX PLAYERS (4/8/12/16) wired into HOST CO-OP; was hard-coded 16 with no UI. `5f99735`. Evidence
   `camp_players_click3.png`, `camp_players_pick16.png`.
5. [DONE] Expanded map/variants drawer now has an in-drawer **variant search** box and a **category cycler** (ALL MODES +
   the variant's forge/game-mode groups from left/right arrows or clicking the pill), with an "N OF M VARIANTS" count.
   Forge World AI Battlegrounds (353 variants) filters to e.g. "jump" -> 3 rows and "BIG TEAM" -> 11 rows. Evidence
   `drawer_filter4.png`, `drawer_search_jump.png`, `drawer_cat_1.png`. Test hooks `-drawer-filter-test <text>` /
   `-drawer-category-test <n>` prefill the filter for captures. The "?" placeholder was fixed earlier (`5f99735`).
6. [DONE] Custom Games cleanup: mod sections collapse/fold (`5f99735`, `e213497`), in-drawer search/category filter as
   above, ASCII drawer header and clamped campaign search. Variant drawer now usable at 353 entries.
7. [PARTIAL - awaiting Tyler retest] Campaign skulls row click verified with `-menu-click-at` (opens SKULLS screen);
   campaign search clipping fixed (`5f99735`). If Tyler still sees a hover/click failure, it is device/mode specific.
8. [DONE] Server browser Refresh/Favourite (`0efb619`): Refresh now shows "Refreshing server list..."; FAVOURITE/COPY
   INVITE pinned in a fixed footer and verified clickable (`sbfix_click_0/1.png`).
9. [DONE] "What's New" hint clickable (`0efb619`): clickable HintExtra opens the changelog (`main_hint_click.png`).
10. [DONE] Game Options rows fully clickable (`0efb619`): whole row is the click target; verified label click opens the
    options screen (`gameoptions_labelclick.png`).
11. [DONE] Controller toggle crouch (`0efb619`): latches the pad crouch button; settings capture `settings_toggle_rows.png`.
12. [DONE] M&K toggle zoom (`0efb619`): TOGGLE ZOOM setting latches third-person zoom; capture `settings_toggle_rows.png`.
13. [DONE core - 88d4937] Lobby share codes, public/private hosting and prominent "JOIN WITH CODE": 23-char NB1- codec
    (InviteCode.hpp/.cpp, self-test 0 failures), Main Menu entry, Server Browser JOIN WITH CODE modal, pre-game lobby
    HOSTING share-code panel with COPY CODE. Private lobbies stay unlisted but the same code joins them. Evidence:
    `join_code_main.png`, `join_code_modal2.png`, `join_code_lobby.png`, fake-code error capture. Follow-ups: Social screen quick action DONE (`ca770e2`, `social_join_code.png`); only the real two-PC
    host->code->join test remains. Plan: `project-docs\PLAN-LOBBY-CODES-2026-11-15.md`.
14. [IN PROGRESS - static RE done 07:10] ENGINE WORK: exact custom visor colours in matches. Found the model shader-param palette (`visor_tint` = slot 6, id 0x8018f), parameter init FUN_18032596c, colour application FUN_1803246b8/FUN_180324f90 (global appearance table DAT_182a03c58 + index*0x2ae). Notes/next tests: `project-docs\RE-VISOR-COLOR-2026-11-15.md`. Only five engine visor slots exist; extended
    Armory finishes currently map to the nearest stock slot. Goal: each player keeps their own custom finish in-game.
    Options documented in the handoff "Visor colour follow-up": scan the shared tag heap for the spartan_helmet_interior_visor
    / secondary visor rmsh materials and tint them, or locate/extend the engine visor colour table+lookup so per-player
    custom indices render correctly. Bigger RE job; schedule after the lobby-code feature.
15. [PROOF DONE 08:35 - next expose variants] Armor Effect colours: palette-reference patch → blue Legendary flames proven in-game (blueflame_0/1.png), rebuilt Forge World via tool.exe, dev hook `-armor-effect-test` committed (`64e6d22`); original tag/profile restored, blue variant saved as head_fire_blue.particle. Next: #15-B create variant effect + Armory entries. Original assets research: Armor Effect colours (fire + lightning/storm) and NEW colour variants: particle tags are palette-driven; fire palettes `fire.bitmap`/`covenant_fire.bitmap`, lightning/storm uses `plasma_blue_01.bitmap`/`blue_plasma.bitmap`; recipe = duplicate particle/effect tags + recolour palette gradients, ship in a mod, expose new entries (needs engine effect-table work for a selectable list). Notes: `project-docs\RE-ARMOR-FX-COLORS-2026-11-15.md`. Original investigation:
    `objects\characters\spartans\fx\armor_fx\` (armor_fx.effect events: legendary_flame/head_fire, eternal_flame,
    storm/thunder_cloud+lightning_flash, pestilence, heart_attack; particles are palette-driven) and expose a listing of
    new colour-changed effect options. Assets dump fine with `tools\runmb.ps1` + `tools\tagdump.py`; see plan doc above.
16. [LAST] Multiplayer: campaign 4 players -> Firefight already 16 -> optional Firefight AI spawn multiplier (2x/3x)
    -> multiplayer 16 -> 128 players. READ FIRST: `docs\limits-players*.md`, `knowledge\drafts\draft-session-networking.md`,
    `docs\limits-ai.md`, `knowledge\desktop-guides\Noble-AI-Limits-Report-2026-11-08.md`; code in
    `src\host\SlotPatch.*`, `SlotMessage.*`, `PoolResize.cpp`, `src\hooks\LobbyPatches.cpp` (`s_maxMultiplayerPlayers=128`),
    `tools\limits\*`. The 32-slot patches are implemented but built-not-tested per those docs.

Test hooks for the remaining items: `-menu-screen <name> -menu-tab <n>`, `-menu-ignore-input`, `-capture-at`,
`-quit-after`, `-menu-click-at <sec> <x> <y>` (repeatable), `-menu-debug-layout`, `-server-browser-demo`,
`-server-browser-style 1|2|3`, `-armory-focus/-armory-cycle`, `-fov-apply-test-at`, `-loading-roster`, `-wait-peers`.
Evidence captures live in `build\bin\Release\data\userdata\claude-tests-2026-11-11\`.

### Extra bug (Tyler, 2026-11-15 ~05:00): custom visor colour shows WHITE in game
- Cause found (Ghidra): only five real engine visor slots exist (0..4); extended Noble finishes were being written as
  invalid slots (e.g. blood_orange = 23) which render white. Armory shows them correctly because it patches rmsh shader
  materials in the *menu* map; the same scan matches **0 materials in campaign/multiplayer maps** (shared tag heap).
- Fixed in `96fd2c4`: every finish now writes a valid stock visor slot (nearest stock colour) so nothing renders white.
  Exact custom colours (e.g. blood orange) still only show in the Armory - getting them into matches needs a shared-heap
  material scan (map-wide tint affects every Spartan) or engine visor-table extension (per-player correct). Ask Tyler
  whether to schedule that engine work; it is bigger than the remaining queue UI fixes.
- Test hook added: `-visor-cam-test` (third-person camera in front of the local player) for future visual checks.

## Reference library (external machine paths - read before researching)

Full index: `project-docs\REFERENCE-LIBRARY-2026-11-15.md`. Key points:
- **Ghidra MCP is connected to `haloreach-copy.dll`** (base `0x180000000`, address = base + repo RVA). Use it for engine
  truth (examples already solved: visor item translator `0x18003B2F4`, patch site `0x2EDC5E`, profile enum map
  `0x1800393CC`).
- Decompile exports for fast text grep: `C:\Users\tyler\Desktop\Reach.exe\` (4,119 functions) and
  `C:\Users\tyler\Desktop\host-probe.exe\` (5,825 functions); check `decompile-index.json` target_sha256 before trusting.
- Repo `docs\` holds the 128/32-player multiplayer research, AI limits, render limits, mod-map mounting
  (`limits-players*.md`, `limits-ai.md`, `limits-render.md`, `mod-map-mount-findings.md`, `limits-players-*.json/trace`).
- `knowledge\drafts\draft-session-networking.md` explains player tables, masks, transport, handshake, join-in-progress.
- External asset/tag tools: `C:\Users\tyler\Desktop\ex\` (extracted tags/UI art), `so many tags\`, `spectre\`, `Pelican\`,
  `assembly\` (tag editor), `Forgeworks-Halo-3-main\`, `reach varient tool and editor\`, `Reach game modes and maps\`,
  `MVAR\` (mvar tool), `Noble-AI-Limits-Report-2026-11-08.md`. `Halo Reach (Jul 24 2010) (Omaha) (Cache Build)\` is
  actually Halo 3 cache XEX builds (mislabeled). `menumaps\` is empty.
- `C:\Users\tyler\Desktop\Compendium\` is a stale mirror of `knowledge\`: after editing repo knowledge docs, copy the
  changed files into the matching Compendium path so Tyler/future agents see the same text.

## REPO STATE AT 09:57 ET - v0.19.103 RELEASED

- Repo `D:\Project-Noble-Closed-Source`. **`main` and `wip-merge` both at `eea9868`** (pushed; working tree clean;
  version **0.19.103**). **Release v0.19.103 published and latest**:
  <https://github.com/O7OF/Project-Noble-/releases/tag/v0.19.103>
  - base `Project-Noble-v0.19.103-win64.zip` 357.1 MB, sha256 `81260682...`
  - optional `Noble-ForgeWorld-v0.19.103-win64.zip` 652.6 MB, sha256 `be693143...`
  - verified latest/draft=false, GitHub asset digests match local hashes, version.txt 0.19.103, menu/armour maps present.
- v0.19.103 contains: lobby share codes with COPY CODE / JOIN WITH CODE on Main Menu + Server Browser + Social,
  variant-drawer search/category filters, server-browser Refresh status + pinned FAVOURITE/COPY INVITE footer,
  Game Options full-row clicks, clickable What's New hint, controller toggle crouch and M&K toggle zoom.
- Visor fallback accepted by Tyler: valid stock slots, no white; exact per-player custom visor stays a separate
  engine task. Multiplayer 128 remains on hold. Task-4 (QA/release) complete.

## REPO STATE AT 05:35 ET - v0.19.102 RELEASED (historical)

- Repo `D:\Project-Noble-Closed-Source`. **`main` and `wip-merge` both at `8db1ce8`** (pushed; working tree clean;
  version **0.19.102**). **Release v0.19.102 published and latest**:
  <https://github.com/O7OF/Project-Noble-/releases/tag/v0.19.102>
  - base `Project-Noble-v0.19.102-win64.zip` 357.1 MB, sha256 `bc200798...`
  - optional `Noble-ForgeWorld-v0.19.102-win64.zip` 652.6 MB, sha256 `aca0b6e9...`
  - verified: elite_armor + noble_menu map, zero RR names, version.txt 0.19.102.
- v0.19.102 contains: details wrapping (`d422a74`), FOV guard (`3a3c98e`), mod sections + campaign MAX PLAYERS + search
  clamp + ASCII header + click hook (`5f99735`), banner polish + visor id mapping (`8f467e1`), Settings sliders
  (`ec585f0`), visor stock-slot fallback/no-white (`96fd2c4`), FOLD action + cleanup (`e213497`).
- `noble-dev` / `noble-client` were rebuilt from `8db1ce8`; tests on the AOC with `-NoFocus`.
- RELEASE POLICY (Tyler 06:05 ET): do NOT bump/publish until the whole queue is done - one final release at the end.
  Keep pushing commits to `main`+`wip-merge` after each item.
- Next queue: lobby share codes (13), visor engine colours (14), armour effect colours (15), multiplayer (16).
- Hex-number UI bug runtime check: DONE, theory not confirmed in solo MP/campaign; see
  `project-docs\NOTE-HEX-BUG-RUNTIME-CHECK-2026-11-15.md`. Needs an exact repro screen/mode.
  last (13). Optional bigger task: engine-level custom visor colours (shared tag heap / visor colour table).

## Done 2026-11-15 (this session, after the loading screen/server browser work)

- Campaign/Custom/Firefight/Lobby MODDED tabs: one collapsed section per mod, click/Accept expands; search expands all
  and filters (`5f99735`). Banner chevrons removed and count/"CLICK TO EXPAND" spacing fixed (`8f467e1`).
- Campaign MAX PLAYERS 4/8/12/16 wired to HOST CO-OP (`5f99735`); campaign search box clamped; drawer header "?"
  placeholder fixed (em-dash glyph missing -> ASCII "-") (`5f99735`).
- In-game visors match the Armory finish: shared `src\common\ArmoryVisor.hpp` resolver used by profile + appearance
  record paths (`8f467e1`); four finishes (violet/ice/crimson/onyx) were swapping due to Armory-vs-MCC catalog order.
  Engine truth documented via Ghidra (`0x18003B2F4`, patch `0x2EDC5E`, `0x1800393CC`) in the reference library.
- Settings > Game Options camera sliders aligned: fixed value column + label ellipsis (`ec585f0`).
- Camera FOV can no longer zoom the Armory model (`3a3c98e`); match FOV still applies on match start.
- New test hooks: `-menu-click-at <sec> <x> <y>` (repeatable, unfocused ImGui clicks), `-menu-debug-layout`,
  `-fov-apply-test-at`. Use them to reproduce the remaining click bugs.
- Release v0.19.101 published with both assets; zip verified (elite_armor + noble menu map, zero RR names).

## Done 2026-11-15 (early) - loading screen + server-browser options

- New loading screen (campaign / custom game / firefight / forge): top-left LOADING / WAITING FOR PLAYERS block with
  the lobby roster and progress bar, top-right mode label, complete (uncropped) map picture over a dim soft copy,
  Reach's "Auntie Dot" diamond grid behind the picture (drifting, travelling wave), Noble Team + UNSC emblems bottom
  corners. Logos/grid extracted from HREK via ManagedBlam and kept in `assets\ui\noble\loading` (not committed).
- Host waiting state: after the engine's loading screen ends, `BlamEngine` reports the remote-player membership wait and
  the screen stays up as WAITING FOR PLAYERS with the joined/expected count instead of a black frame.
- Lobby roster names flow to the screen via `-loading-roster` (host first); the mode is passed with
  `PauseMenu::SetLoadingSession`.
- Server browser: three layouts behind `-server-browser-style 1|2|3` (shipped amber tactical cards, large blue-glass
  cards, compact table) for Tyler to choose from; default remains 1.
- Merged upstream v0.19.99 (`45f365d`); only `BlamEngine.cpp` conflicted (membership-wait rework vs our waiting flag).

## REPO STATE AT 19:55 ET (historical)

- Then: `main`/`wip-merge` at `0e8ca81` (v0.19.89), upstream `origin/main` at `a576e43` (v0.19.90, unmerged), and
  v0.19.90 had no Forge World asset so the in-client download row errored. All resolved in the 20:00-20:35 wave.

## REPO STATE AT 10:45 ET (historical)

- Then: pushed tip `a3a4d50` (Trooper third person, untested), local-only merge `4da490a`, uncommitted dedicated-link
  fix + bank-3 guard. Merged-build early session end was solved (beam pools overflowed game-state bank 3; guard added).
  Trooper mode was later removed (7578b41).

## Done 2026-11-14 - late wave (20:00-20:40)

- Merged upstream v0.19.90 (`a576e43`/`8f7a5b8`) and v0.19.91 (`2ec2b49`) into `wip-merge`; rebuilt both gates.
- Theater saved-film playback verified end-to-end with `-film <saved film>`: film map/mode resolved from the film,
  playback ran 50 s with the film director overlay, mainmenu-cruiser bug not present. Captures
  `theater_film_m10_25s.png` / `theater_film_m10_40s.png`.
- Armory after the merge: scene-depth mask still picks 1920x1080; Marine preview in front of the lattice. Evidence
  `armory_after_v01991_42s.png`.
- Match after the merge: Sword Base, elite pack loaded (`relocated 67051 pointers`, extra cache
  `noble_elite_pack` returned 1, `game session started at frame 247`), capture at 45 s, clean quit. Evidence
  `match_swordbase_45s.png`.
- **Forge World download fixed**: (a) attached `Noble-ForgeWorld-*` assets to the v0.19.90 and v0.19.91 releases (latest
  release has the pack again); (b) code fix `aae1675` - `RecommendedMaps` now scans `releases?per_page=10` newest-first
  and uses the newest release that carries the pack, so a base-only release cannot disable the row again. Tested three
  ways with a local release JSON (fallback, direct, no-asset error) plus a real-GitHub selection check.
- Push: `main` and `wip-merge` both at `aae1675` (fast-forward from `2ec2b49`).

## Done 2026-11-14 - first wave (16:30-19:30)

- Trooper prototype REMOVED (CAMPAIGN back to VANILLA / MODDED) (7578b41); cutscene "recovery" opt-in only (b357a93).
- Release download split: Forge World optional asset, `mainmenu_all.map` retired, base zip 1260 -> 607 -> 356 MB;
  updater only accepts the exact client zip (v0.19.85/86).
- MODS > RECOMMENDED MAPS: in-client SHA256-verified Forge World download; tested against a local HTTP server.
- Armory: lattice depth fix (previews in front of the lattice, v0.19.87); Marine/Female Marine Armory without Marine
  mods; species switch instant except Elite settle; menu audio double-gain fix + 15% base music.
- Engine limits default to Stock outside Forge.
- **No RR-named files ship** (v0.19.89): `elite_armor.map` + Noble `NBRL` `.noblereloc`; other RR sidecars gone;
  embedded table `spartan_armor_parts.txt`; verified in a match (127,379 pointers relocated).
- Desktop tester zip: not shipped (Tyler: "we dont need 'new desktop tester pack'").
- Releases v0.19.84 - v0.19.89 published; pushes to main per Tyler's instruction.

## Done earlier today (morning - keep for reference)

- Bank-3 beam guard (v0.19.69 pools overflowed the game-state bank: 68,052,044 of 67,043,328); dedicated-server link
  fix; test-only launch/skip/dump instrumentation; Covenant airstrike palette in Forge World (Tyler confirmed).
- Test harness: tests launch without activation (`SW_SHOWNOACTIVATE`) and never take Tyler's mouse (`-NoFocus`); windows
  on the AOC; Forge World proven served from `data\maps\forge_halo.map`; elite pack 486 -> 286 MB proven A/B identical.

## Needs Tyler's decision / action

1. [DONE 07:55] **Server-browser layout**: style 2 (large blue-glass cards) is already the default in `BrowserStyle()`
   ("Tyler's pick, default"); styles 1/3 stay behind `-server-browser-style` for dev comparisons only. Captures
   `sb_style1/2/3.png`.
2. **Publish a release that includes the loading screen + the RecommendedMaps fallback fix** (`ea572ae`). Bump
   `Version.hpp` + release notes, build both gates, run `tools\publish-release.ps1` (builds base **and** Forge zip).
   Check GitHub first - O7OF publishes to main and releases roughly hourly; coordinate before overwriting a tag.
3. **Tell O7OF to publish with the Forge pack** (no `-NoForgePack`) or run the attach step after each release; the
   already-shipped v0.19.90/91 clients look at the latest release for the pack.
4. **Remaining RR provenance is inside `noble_menu.map`** (`rr_*` HaloScript variable names + RR-era menu-map base).
   Invisible to players/files; removing it = rename variables in HREK + rebuild/spot check (hours, no visible change).
5. [APPROVED 07:45 by Tyler] **Local cleanup (dev only, no user impact)**: stale `build\bin\Release\data\armor\noble_elite_pack.map`
   (286 MB), ~3 GB of `noble_menu.map.*` backups in `build\bin\Release\data\menumaps`, repo
   `data\menumaps\mainmenu_all.map` (285 MB dev fallback, no longer shipped).
6. Test-harness note: `match_elite.ps1 -NoF5` skips the key press but still prints "F5 sent at N s" - cosmetic only.

## Old bugs to double-check (hand checks, need Tyler)

- Forge on a Firefight canvas: fixes landed upstream (e119116). Custom-map missions: work landed (6afd742).
- [REMOVED 07:45] "Respawn in 00000001" (Tyler cannot reproduce it any more) and pause-not-freezing (Tyler says remove).

## Parked

- Flashlight (merged into HREK `spartans.biped`, L key; open value reset to 0 every update in multiplayer).
- Multiplayer two-PC test.
- Armory: knife chests, emblem textures (engine asks host slots 73/74; stubbed, paused by Tyler), undersuit colour,
  Elite shine/lights.
- Forge-palette "Option 2": inject Pelican/Spectre/Boathog/targeting laser into ANY map's Forge palette (like ElitePack)
  instead of shipping the whole 750 MB rebuilt Forge World.
- Marine player animation (model/biped graph): only relevant again if a Trooper-style mode returns through a mod.

### Queue progress (2026-11-15 04:45 ET, `main`=`wip-merge`=`5f99735`)

- DONE 2a: FOV no longer applies in front-end/Armory (`3a3c98e`); 2b armory scroll alignment still needs Tyler's exact spot.
- DONE 3: MODDED tabs (campaign/custom/firefight + lobby map select) show one collapsed section per mod, click expands.
- DONE 4: Campaign MAX PLAYERS row 4/8/12/16 wired into HOST CO-OP (was hard-coded 16, no UI).
- DONE part of 5/6: variant drawer header em dash -> ASCII "-" ("?" placeholder fixed); custom-game mod tabs now collapsed too.
- DONE part of 7: campaign search box clamped on-screen; skulls row click verified working in current build (needs Tyler retest).
- NEW TEST TOOLS: `-menu-click-at <sec> <x> <y>` (repeatable, injects real ImGui click unfocused) and `-menu-debug-layout`.
- NEXT: 5/6 in-drawer variant search + game-mode category filter; 8/9/10 click bugs (click harness ready); 11/12 bindings; 13 multiplayer last.
### Queue progress update (2026-11-15 04:55 ET, `main`=`wip-merge`=`8f467e1`)

- Collapsed-mod banner polish DONE: side chevrons removed; count and "CLICK TO EXPAND" are laid out from measured widths
  with a fixed gap (no overlap/underlap); expand hint hidden on very narrow rows.
- In-game visor mismatch FOUND + FIXED: the Armory catalog order differs from MCC's HR_Visor item order for four
  finishes (UI: violet 7, ice 8, crimson 9, onyx 10; MCC: ice 7, crimson 8, onyx 9, violet 10). ElitePack took the UI
  index straight to the appearance record while ReachHost mapped the profile by id, so those four rendered as the
  wrong finish in game. New `common/ArmoryVisor.hpp` is the single resolver (id -> MCC slot, legacy index remap, stock
  Black/Gold 3/4 swap) used by both paths. Verified mapping: violet->10, ice->7, crimson->8, onyx->9, black->4,
  gold->3; saved blood_orange still slot 23. ReachHost now logs the applied visor slot.
- Known limitation: extended Noble visor finishes (>=5) get their exact Armory RGB forced onto the LOCAL biped only;
  other players see the MCC slot colour (close but not identical). Replicating custom visor RGB to peers is future work.
### Queue progress update (2026-11-15 05:05 ET, `main`=`wip-merge`=`ec585f0`)

- Tyler clarified the "armory scroll options misaligned" bug is actually SETTINGS > GAME OPTIONS camera sliders. Fixed in
  `ec585f0`: the track was sized from the current value text width ("OUT 120 %" rows ended further left than "100 %"
  rows); it now uses a fixed value column (132 * UI scale), so all camera slider tracks share the same start/end. Long
  labels are ellipsized at the slider column instead of running underneath the track. Verified with a Game Options
  capture (all eight camera tracks x=664..968 in the crop, values share one right edge).
- Visor mismatch fixed in `8f467e1` (Armory catalog order vs MCC visor-item order: violet/ice/crimson/onyx now resolve
  by visor id; stock Black/Gold 3/4 swap kept; legacy saves remapped).
## Runtime-colour prototype - 2026-11-17 03:05

- Ghidra MCP headless bridge restored (127.0.0.1:8081); use direct Python MCP client if Harness client stays disconnected.
- Flame draw/palette identified via RenderDoc: PS 8701, VS 8693, palette 256x16 B8G8R8A8_SRGB, five billboard draws.
- ParticleTint module + -armor-fx-runtime-tint flag implemented: PSSetShaderResources hook (index 8), detects palette
  and substitutes tint SRV. Works on launcher context; engine rendering context not captured yet -> stock flames
  still orange. Next: find/hook engine device/context or particle submit, then per-player palette index.

- **M1 DONE 2026-11-17 04:03:** runtime palette override changes stock-map flames with no rebuild. Engine context captured from DXGI Present swapchain; PSSetShaderResources vtable index 8; palette 256x16 B8G8R8A8_UNORM_SRGB. M2 per-player selection still TODO; M3 peer sync TODO.

- M2 per-player palette: **blocked by missing owner/colour data in ParticleRenderVS CB**; VS uses vs_particle_state_buffer (stride 80) and PS palette*v1+v2. Options: patch particle state buffer + PS palette-atlas index, expose owner per draw, or interim local-player-only tint. Restrict M1 matching to particle shader hash (currently any 256x16 BGRA palette).

- Steps 1-3 done: slot-2 palette restriction, profile-driven runtime variant selection (stock LEGENDARY + palette), 30 variants restored in Armory, six missing colours selectable, gold/rust/pale/brick verified stock map. Remaining: Armory preview tint for six (menu path), thumbnails/photos, per-player M2, peer replication M3.
