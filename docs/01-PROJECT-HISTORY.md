# Project Noble - condensed history (2026-10-27 .. 2026-11-14)
What was worked on, how, what failed and what fixed it. Each line points to the doc with the detail
(paths relative to this folder). Times are ET. "PROVEN" = one-variable test + capture/log.

## Phase 1 - Getting Reach running without MCC (to ~10-28)
- Goal: noble-client.exe loads MCC's haloreach.dll and drives it like Reach Renaissance (RR) does, ownership via Steam,
  MCC install untouched. Host interface = Noble's ReachHost object answering the engine's "host slots" (vtable calls).
- 10-28 (many sessions): FOV hook, settings reachable from the pause menu, controller/wheel, music picker, service record,
  post-game screen, Reach challenges + 45 commendations, menu redesign spec (Campaign/Firefight/Custom/Forge setup),
  Reach art moved into git. Detail: `repo-docs\root\HANDOFF.md` (sections 10-28 02:00 .. 15:15).
- Armory v1/v2: the real Armory scene (Spartan + Elite) from RR's mainmenu_all.map; per-piece Elite armour; Elite pack
  stages A1-A3 (pack loads into matches, Armory Zealot spawns as Zealot). Detail: HANDOFF.md 10-28 15:15 .. 19:25.

## Phase 2 - Netcode, master server, lobby (10-28 .. 10-29)
- Joins failed / infinite load: wire packet framing (8-byte machine id header), host slot 41/43 NetworkTransport, synced
  session ids, unique s_player_identifier (+0xF0) + unique guest XUID, UDP/relay dedup, NAT traversal + WebSocket relay,
  in-menu accounts, pre-game lobby, server browser overhaul. "Guest stuck loading" = HOST refuses the join (reason 15).
  Detail: HANDOFF.md 10-28 20:30 .. 10-29 03:45 (O7OF + Dominus work included).

## Phase 3 - Closed source, releases 0.19.x (11-07 .. 11-09)
- 11-07: open-source repo archived; private `O7OF/Project-Noble-Closed-Source`; 3-region master-server cluster
  (Render: projectnobleeast/west/uk); build gate so open-source builds can't join.
- 11-07 22:00 "Armory doesn't change armour" ROOT CAUSE: `data\menumaps\mainmenu_all.map` was missing from the run folder
  (release zip excludes *.map) -> stock menu biped ignores the GLOBALS pairs.
- 11-08: releases 0.19.19 .. 0.19.24 (auto-updater, AV false positives, cross-play dev/release, Forge AI battleground,
  native Forge palette replica, simulation-thread dispatch for object creation).
- 11-08 .. 11-10: **Extended engine limits** with DeepSeek in parallel (`repo-docs\root\AGENT-STATUS.md`,
  `LIMITS-HANDOFF.md`, `LIMITS-TODO.md`, `repo-docs\limits-*`): AI actor pool resized (1,024 actors held in swarm tests;
  512 last "proved"), render/particle profiles (.nlp), 32/128-player work (SlotPatch, register-bound caves, peer param
  stopgaps; got a slot-17 guest to session state 4 and the host's match to survive). **32-player work PAUSED** since 11-10.
- 11-09: v0.19.37 (container single-window launcher, crash fixes, Armory fixes, 8-team lobby). Community content:
  megapack custom games, FORGE MAPS tab, Spectre + Pelican plan, mod reviews, numpad dev spawn keys
  (`repo-docs\HANDOFF-CONTENT-2026-11-09.md`, `repo-docs\megapack.md`, `repo-docs\pelican.md`, `repo-docs\swarm-test-notes.md`).

## Phase 4 - Mod bugs and crash evidence (11-10)  [`project-docs\MOD-BUGS-*`, `HANDOFF-NEXT-AGENT-2026-11-10`]
- FIXED: mod weapon sounds (VFS also looks in `<mod>\haloreach\`); mod menu/loading art (copy image fields); 4K loading
  text overlap (scale with h/1080); pause-menu lag (Career::Challenges every frame); voice "no session" hint.
- FAILED / open: custom-map missions served under another scenario's slot fail (header rename doesn't help: engine checks
  map data); Trepanation (>2 GiB, suspected signed 32-bit offsets); pause never freezes (`game_speed` via ExecuteCommand
  has no effect); friend's AMD freeze (unproven driver hang).
- Crash logging added: SIGABRT/invalid-parameter/purecall handlers, run-flag marker, child exit-code meanings.

## Phase 5 - Noble's own menu map + Armory (11-11)  [`project-docs\NOBLE-MENU-MAP-*`, `HANDOFF-ARMORY-2026-11-11`]
- Decision: build our own menu map from Bungie's stock `levels\ui\mainmenu` in HREK (RR map = reference only).
- FAILED: editing data .hsc (build compiles the script stored in the TAG); adding cameras to mainmenu.scenario (build
  reads `mainmenu.scenario_cinematics_resource`); tool.exe patch-tag-field can't add elements -> ManagedBlam works.
- FIXED: camera points (cinematics resource), string ids (use OUR build's string_ids.csv -> noble_menu.string_ids.csv),
  `noble_keep` so the compiler keeps Noble's GLOBALS table, **Armory GPU-driver crash** = writing into the GLOBALS table
  before the map script initialised it (wait for rr_scene_done to be a real scene number).
- Armory polish: Auntie Dot lattice backdrop, depth-buffer masking, layered Bungie menu art, Reach-only music, species
  switch debounce, scrollbars. Elite colours didn't change (live object colours stick; look comes from shader/tag data).

## Phase 6 - Elite armour pack quality (11-12)  [`project-docs\HANDOFF-ELITE-*`, `FIX-SPARTAN-PIECES-PMCG-BOM`]
- Spartan pieces not changing: embedded `spartan.rrpmcg` had a **UTF-8 BOM** -> first list line not parsed. Fixed.
- Prophet's Hand Elite pieces grey/white: PH bitmaps imported with curve "force PRETTY" + sRGB override (stock = curve 0,
  no override). Fix bm_curve.py + reimport + original pixels. Darkening textures was the WRONG fix (reverted).
- Shine: Ossoona normal map imported as Diffuse usage; PH normals curve mode; specular masks (diffuse alpha) too high.
  Pelvis hot spot = ch_elite_armor_02 alpha. HG normals had no Z channel (fixed, not the cause).
- HG glow: glow maps imported as diffuse; glow colour multiplied by player colour (blue x red = black). Later (night):
  glow function was a 1-colour scalar -> compiled red; _em compiled to 1 channel -> forced DXT1; now fixed gold.
- Green seam lines = DXT turning near-black green -> ch_elite_armor_02 forced 24-bit+8a (format 17).
- Original "rr" textures installed (HG, armor_02, Council). Elite per-slot camera zoom FAILED (scenes > 0x14 ignored) and
  was reverted. Monitor option in Settings (pick display). Lights row hidden (doesn't drive glow).
- Knife on chest: knife is a model OBJECT of variant "default" (stock behaviour); fixes not proven -> reverted.
- Emblem ROOT CAUSE: engine asks the HOST for emblem texture/colours via host slots 73/74; Noble stubs return 0. Slots
  73/74 implemented in a dev build, then PAUSED by Tyler.

## Phase 7 - Marines (11-13)  [`pc-handoffs\HANDOFF-ELITE-2026-11-12-NIGHT.md` 02:00 .. 18:55]
- MARINE Armory tab (pieces + 4 colours: Primary/Secondary/Detail/Accent), Marine preview in the menu map (biped palette 2
  in the bipeds RESOURCE; duplicate unique ids strip scripts), female Marine, close-up cameras, menu-only biped without
  rifle. HREK's marine model lacks most flair meshes (empty permutations) - backpacks limited to TROOPER/HEAVY/RADIO.
- H2A Marines added then REMOVED at Tyler's request (2cd67a7). Flashlight merged into spartans.biped (L key; MP resets it).
- In-match Marine pieces (Trooper Firefight mod): long hunt (variant enforcement at +0x6A4BA0, hooks, refresh calls).
  Hardware write breakpoint showed the per-frame **sim->render game-state memcpy** (haloreach+0x1CD389) overwrote pieces
  written on the render thread -> apply on the SIMULATION thread (NotifyMatchTick). Still "looked broken" because the test
  pieces looked the same from behind.
- Forge World rebuilt in HREK with Pelican, Spectre, Boathog, targeting laser (scn_add_forge.py).

## Phase 8 - 11-14 (this session)  [`project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md`, `desktop-guides\TODO-MASTER-2026-11-14.md`]
- Forge World Option 1 PROVEN: VFS serves `build\bin\Release\data\maps\forge_halo.map` (+ sfx.st_spectre.fsb) on every
  normal Forge World launch; publish-release.ps1 fixed to ship it (it skipped *.map).
- Dev switches cleaned up (edce357, c92b451). Test windows forced onto the AOC monitor.
- Elite pack 486 -> 284/286 MB: build from stripped `levels\test\noble_pack\noble_pack` (box copy, MP type, palettes/
  squads/scripts cleared). First try kept civilian/marine bipeds -> ElitePack shield heuristic bailed. A/B identical.
- Marine pieces PROVEN with obviously different pieces (odst_light / odst_jetpack).
- **Trooper mode** (c9f2630): `-trooper` in campaign hooks the zone-set rebuild (+0x8EAF0), repoints matg player
  representation (+0x474 block, biped +0x40) from the Spartan to the mission's marine biped (indexes per mission from
  Reclaimer since stock maps are compressed). PROVEN on m35 in third person. Mistakes on the way: hooked the cache step
  (+0x30698, only runs for extra caches); thought a third-person "Spartan" was the player (it was Kat); scripted F5 turned
  third person OFF. a3a4d50 locks third person (UNTESTED).
- Pulled origin/main 0.19.67..0.19.78 into wip-merge (local 4da490a), fixed noble-dedicated link errors (uncommitted).
  Merged build's test match ended at ~25 s -> cause unknown; nothing pushed to main yet.
- Built this knowledge folder.

## Phase 9 - 11-14 afternoon/evening: releases, Armory, de-RR  [`project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md`, `desktop-guides\TODO-MASTER-2026-11-14.md`]
- RELEASE SPLIT: Forge World became an optional `Noble-ForgeWorld-*` asset; `mainmenu_all.map` (285 MB) retired so the
  base ships only `noble_menu.map`; base zip 1260 -> 607 -> 356 MB. Updater fixed to accept only the exact client zip
  (earlier it grabbed the Forge pack; update/reinstall looked broken). `tools\publish-release.ps1` now stages
  `noble_menu.map`, string-id CSVs and `elite_armor.map` + sidecar, and fails loudly if they are missing.
- MODS > RECOMMENDED MAPS: in-client download of the optional Forge World pack with SHA256 verification; tested against
  a local HTTP server (override env vars) - downloading/checksum ok/installed. Row sits above the scrolling list.
- ARMORY: previews rendered behind the lattice because the mask picked the 1600x1600 shadow-map depth buffer; fix picks
  the depth matching the swapchain aspect and keeps it across frames. Marine/Female Marine Armory works without Marine
  mods because the rebuilt menu map now ships. Species switch: Spartan<->Marine instant, Elite flips settle.
- TROOPER MODE REMOVED at Tyler's request: CAMPAIGN back to VANILLA/MODDED, `-trooper`, TROOPER tab, `TrooperMode.*` and
  test hooks gone. The earlier cutscene "recovery" that broke mission intros became opt-in (`NOBLE_CUTSCENE_RECOVERY=1`).
- AUDIO: double gain stage removed (session volume + engine mixer), MASTER MUTE menu-only, MUSIC/OTHER live mid-match,
  base music 15% with config migration. Engine limits default to Stock outside Forge.
- DE-RR (v0.19.89): `rr_elite_pack.map` -> `elite_armor.map`, `.rrreloc` -> Noble's own `NBRL` `.noblereloc` sidecar
  (loader and packer updated), `.rremotes`/`.rrneeds`/`.rrpmcg` deleted, embedded table -> `spartan_armor_parts.txt`.
  Proven in a multiplayer match: 127,379 packed pointers relocated, `extra cache haloreach\maps\elite_armor load
  returned 1`, pack Elite biped applied. Remaining `rr_*` names are HaloScript variables inside `noble_menu.map` only.
- Pushed v0.19.84-0.19.89 to `main` and `wip-merge` (both at 0e8ca81) and published six releases. Upstream then moved
  `origin/main` to a576e43 (v0.19.90, theater saved-film fix) and published it base-zip-only, which leaves the in-client
  Forge World download without an asset until fixed.

## Phase 10 - 11-14 late: upstream v0.19.90/91 merge, Forge download fixed  [`project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md`]
- Merged upstream `a576e43`/`8f7a5b8` (v0.19.90 theater saved-film + BlamEngine/host/lobby/pause-menu work) and
  `2ec2b49` (v0.19.91 pause-menu theater fix) into wip-merge (fast-forward, no conflicts) and rebuilt both gates.
- Theater saved-film playback PROVEN: `-film <path>` resolved map/mode from the film, engine played the film 50 s with
  the director overlay (`LobbyPatches` suppressed the playback game-end), captures `theater_film_m10_25s/40s.png`.
- Armory and a normal Sword Base match retested after the merge (depth mask still 1920x1080; elite pack loaded, session
  started, clean quit). Evidence `armory_after_v01991_42s.png`, `match_swordbase_45s.png`.
- Forge World download bug found at 19:55 fixed two ways: (a) attached `Noble-ForgeWorld-v0.19.90` and
  `-v0.19.91` assets to those releases (they were published base-zip-only; the pack payload is unchanged), so already
  shipped clients find it in `/releases/latest` again; (b) code fix in `RecommendedMaps` (aae1675): scan
  `releases?per_page=10` newest-first and use the newest release that carries the pack, so a base-only release cannot
  disable the row again. Tested with a local release JSON (fallback, direct, no-asset error) plus a real GitHub check.
- Pushed `main` and `wip-merge` to `aae1675`. Not yet in a published executable: publish a release when Tyler says so
  (upstream is releasing every ~hour - coordinate/check before publishing).

## Phase 11 - 11-15 early: loading screen redesign + server-browser options  [`project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md`]
- Tyler supplied the reference loading/waiting screen (MCC-style WAITING FOR PLAYERS top-left block) and asked for the
  same for Campaign / Custom Games / Forge / Firefight, the map photos corrected (they were stretched full screen from
  668x292 previews = the "zoomed" look), the Reach grid from a reference video behind the photo, and UNSC/Noble corner
  emblems; plus a few options for the server browser (the pushed workup vs the old flat one he was running).
- Built the new loading screen in `PauseMenu::DrawLoading`: top-left title/status/progress/roster/chat hint, top-right
  mode label, complete map picture over a dim soft copy, procedural Reach diamond grid behind it (drifting + travelling
  bright wave), Noble + UNSC emblems bottom corners. Added `BlamEngine::IsWaitingForRemotePlayers()` around the
  membership barrier so the host's WAITING FOR PLAYERS state no longer shows a black frame. Lobby roster names flow
  through a new `-loading-roster` argument and `PauseMenu::SetLoadingSession`.
- Extracted the real loading-screen art from HREK with ManagedBlam `Bungie.Tags.GameBitmap` (auntie_dot noble/unsc
  logos + auntie dot background grid); kept in `assets\ui\noble\loading` (not committed).
- Server browser: added `-server-browser-style 1|2|3` (shipped amber cards / large blue-glass cards / compact table) and
  captured all three with demo rows - waiting on Tyler's pick before making one the default.
- Merged upstream v0.19.99 (conflict in `BlamEngine.cpp`: kept their membership-wait logic + our waiting flag), rebuilt
  the gates and pushed `main`/`wip-merge` to `ea572ae`.

## Phase 12 - 11-15: loading-screen polish, release v0.19.101, server browser + roster
- Server browser: option 2 (blue-glass cards) chosen by Tyler and made the default (`8811f55`); styles 1/3 stay behind
  `-server-browser-style`. Modded sessions got MOD / MOD - NOT INSTALLED badges, `MOD: <title>`, a details-pane mod
  summary, version warnings and DOWNLOAD MOD & JOIN -> `ModDownloadPrompt` (verified with real Workshop item 3127073871).
  Details warnings were wrapped and moved under the server address (`d422a74`) so nothing is clipped under the scrollbar.
- Loading-screen roster adapts to lobby size: 1 column normally, 2 columns at 16, compact multi-column grid for
  32/64/128 with `+N MORE` fallback; count line `1 OF N IN` verified at 16/32/64/128 with synthetic `-wait-peers` +
  `-loading-roster` rosters.
- Merged upstream v0.19.100 (`7026ab1`, camera/match-end fixes) as a fast-forward (upstream had already merged our work),
  bumped to 0.19.101, wrote `release-notes/v0.19.101.md` + in-game changelog and published **v0.19.101** from `ada9f5d`
  with both assets (base 357.1 MB + Forge World 652.6 MB); zip verified (elite_armor pack, noble_menu.map, zero RR names).
- Pushed `main` and `wip-merge` to `ada9f5d`; dev build and public release build were in sync at that point.

## Phase 13 - 11-15 night: Tyler's ordered fix queue (in progress)  [`project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md`, `project-docs\REFERENCE-LIBRARY-2026-11-15.md`]
- Created `project-docs\REFERENCE-LIBRARY-2026-11-15.md`: Ghidra MCP facts (haloreach-copy.dll image base 0x180000000,
  module sha256, RVA = Ghidra - base), Desktop decompile exports (`Reach.exe\functions` 4,119 functions,
  `host-probe.exe\functions` 5,825), repo `docs\` multiplayer/limits/AI work, `knowledge\drafts\` networking draft,
  external tag/asset/tool folders, test harness + every test hook, and the Compendium mirror policy.
- Fixed FOV/Armory conflict (`3a3c98e`): applying SETTINGS > VIDEO FOV in the front-end zoomed the Armory model because
  the menu and Armory share the engine camera; `CameraFov::Apply()` is now a no-op in `ReachArmory::IsMenuMode()`.
- Campaign/Custom/Firefight/lobby MODDED tabs: one collapsed section per mod that expands on click (`5f99735`); banner
  chevrons removed and count/"CLICK TO EXPAND" laid out from measured widths (`8f467e1`).
- Campaign MAX PLAYERS 4/8/12/16 wired into HOST CO-OP (`5f99735`); campaign search box clamped on-screen; drawer header
  "?" fixed (em-dash glyph missing -> ASCII "-").
- In-game visor mismatch fixed (`8f467e1`): Armory catalog order vs MCC HR_Visor item order swapped violet/ice/crimson/onyx;
  new `src\common\ArmoryVisor.hpp` is the single id-based resolver used by both the profile path (ReachHost) and the
  appearance-record path (ElitePack), including the stock Black/Gold 3/4 swap. Engine truth from Ghidra: translator
  `0x18003B2F4`, patch site `0x2EDC5E` (direct item-157), profile enum map inside `FUN_180039128` at `0x1800393CC`.
- Settings > Game Options camera sliders aligned (`ec585f0`): fixed value-column width (132 * UI scale) so every track has
  the same start/end; row labels ellipsized at the slider column instead of overlapping the track.
- Added test-only hooks `-menu-click-at <sec> <x> <y>` (repeatable, unfocused ImGui click injection) and
  `-menu-debug-layout`, plus `-fov-apply-test-at`; used to verify section clicks, the MAX PLAYERS picker and the skulls row.
- Released nothing after these fixes yet; `main` = `wip-merge` = `ec585f0`, dev-only. Queue continues with in-drawer
  variant search/category filters, the four click bugs, toggle-crouch/zoom bindings, then multiplayer (last).
- Docketed but not scheduled: one player's 3rd-person camera left/right spazzing (unreproducible by 4 others; upstream
  v0.19.100 camera rework may have fixed it - ask for a retest on v0.19.101).

### Phase 13 correction (2026-11-15 05:25)
- Rechecking the visor work found the real in-game cap: haloreach.dll only has five real visor slots (engine 0..4); the
  extended Noble finishes were stored as invalid slots and rendered white (Tyler's blood_orange). `96fd2c4` maps every
  finish to a valid stock slot so nothing renders white; exact custom colours remain Armory-only until a shared-tag-heap
  material scan or engine visor-table extension is done. Also re-verified Settings > Game Options alignment (tracks
  855..1343 in the 1080p capture) and added a FOLD action on expanded mod-section headings (click the heading row).

- Released **v0.19.102** from `8db1ce8` (base 357.1 MB sha256 bc200798..., Forge pack 652.6 MB sha256 aca0b6e9...): ships the map-picker sections/fold, campaign MAX PLAYERS, visor stock-slot fallback, Settings Game Options alignment, FOV guard and FOLD cleanup. Zip verified (elite_armor + noble_menu, zero RR names, version.txt 0.19.102).

## Phase 14 - 11-15 late morning: v0.19.103 released, all decompiles finished, engine-upgrade phase opened
- RELEASE **v0.19.103** from `eea9868` (`main` = `wip-merge`): lobby share codes (23-char NB1 codec; COPY CODE in the
  lobby HOSTING panel, JOIN WITH CODE on Main Menu / Server Browser / Social), variant-drawer search + category filters
  with N OF M VARIANTS, server-browser Refresh status + pinned FAVOURITE/COPY INVITE footer, Game Options / What's New
  row clicks, controller toggle crouch + M&K toggle zoom. Base zip 374,494,102 B sha256 `81260682...`, Forge pack
  684,304,362 B sha256 `be693143...`; release published 2026-11-15T14:57:37Z, draft=false and latest (re-verified via
  GitHub API 10:40 ET); both gates report 0.19.103.0. Tree has pre-existing untracked asset/doc files only (do not
  `git clean`). Next release must be a NEW version (e.g. 0.19.104) and only on Tyler's "release it".
- DECOMPILES COMPLETE: Halo CE finished 2026-11-15 10:19 ET - **79,637 functions, 2 failed**
  (`D:\MCC_DECOMP\halo1\decompiled\`, 40 `hr_*.c`, `functions_index.csv` = 79,638 lines incl. header; `ghidra.log`
  "DONE: 79637 functions, 2 failed"). All six MCC engines are now decompiled: CE 79,637, Reach 27,578, Halo 2 39,362,
  Halo 3 20,444, ODST 21,484, Halo 4 36,876; plus 360 retail v3 33,787 named and 360 dev-kit beta 22,317.
- ENGINE-UPGRADE PHASE opened for DeepSeek (docs only so far - no `src\` change): read
  `project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` + `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md`
  + `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`; recorded O7's three requests
  (`project-docs\NOTE-O7-REQUESTS-2026-11-15.md`), added the packet pointer rows to `00-START-HERE.md`, started
  `decompile-exports\topics\mcc-address-table.md` (5 proven property-editor matches), and corrected the stale
  v2-decompile paths in the master list. 128-player work still ON HOLD; no release/version bump.