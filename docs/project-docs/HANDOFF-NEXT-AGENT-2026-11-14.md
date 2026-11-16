# Handoff for the next agent - Project Noble (2026-11-15 05:10 ET)

This **replaces the 02:40 ET version of this file**. Morning content (Trooper mode work, merged-build debugging) is
history: Trooper mode was removed (7578b41) and is not coming back; details stay in `01-PROJECT-HISTORY.md` Phases
8-11 and `pc-handoffs\HANDOFF-ELITE-2026-11-12-NIGHT.md`.

Read order: this file, then **`project-docs\REFERENCE-LIBRARY-2026-11-15.md`** (every external reference folder on this
machine, Ghidra MCP usage, prior multiplayer/limit work, test hooks -- read this before any research so you do not
re-discover it), then `desktop-guides\TODO-MASTER-2026-11-14.md` (live to-do), then `02-LESSONS-AND-RULES.md` if you
have not worked this project before.

## Where things stand (05:35 ET) - v0.19.102 RELEASED

- Repo `D:\Project-Noble-Closed-Source`. **`main` and `wip-merge` are both at `8db1ce8`** (pushed; working tree clean),
  version **0.19.102**.
- **Release v0.19.102 is published** from `8db1ce8`: <https://github.com/O7OF/Project-Noble-/releases/tag/v0.19.102>
  - `Project-Noble-v0.19.102-win64.zip` 357.1 MB, sha256 `bc200798174bc6ff0995ff5b1453c09c846793617903ca3db6e20200408e683f`
  - `Noble-ForgeWorld-v0.19.102-win64.zip` 652.6 MB, sha256 `aca0b6e9b79cd8c2efc7b027e05f2548be795368677f1c7e93c51fb1f9ec5f0b`
  - zip verified: `data/armor` only `elite_armor.map` + `.noblereloc`, `data/menumaps` noble_menu.map + csvs, zero
    RR-named entries, `version.txt` = "Noble 0.19.102 release build".
- v0.19.102 includes every post-0.19.101 fix: `d422a74` (server-browser details wrapping), `3a3c98e` (FOV must not apply
  in the front-end/Armory), `5f99735` (campaign/custom collapsed mod sections, campaign MAX PLAYERS, campaign search
  clamp, ASCII drawer header, `-menu-click-at` test hook), `8f467e1` (mod-banner polish + visor id mapping), `ec585f0`
  (Settings > Game Options slider alignment), `96fd2c4` (visor stock-slot fallback/no white; engine cap documented),
  `e213497` (FOLD action on expanded mod sections + cleanup). `release-notes/v0.19.102.md` + the in-game changelog list
  the user-facing subset. The previous release v0.19.101 (from `ada9f5d`) had the loading screen + server browser +
  roster work.
- Release gates `noble-dev.exe` / `noble-client.exe` were rebuilt after every push. No game was running at the time of
  writing; tests were run on the AOC with `-NoFocus`.
- **Upstream is still moving fast**: v0.19.92 ... v0.19.100 landed during this session, roughly one big UI/netcode
  commit per hour, always straight to `main`. Always `git fetch` before starting and before pushing; expect to merge
  again. Their v0.19.99 conflicted in `src/engine/BlamEngine.cpp` (our remote-player waiting flag vs their
  membership-wait rework) - resolved by keeping their `waitStart`/elapsed logic and our instrumentation.
- Releases: v0.19.84-v0.19.89 are ours. v0.19.90/v0.19.91 base zips were published by upstream; we attached the missing
  `Noble-ForgeWorld-v0.19.90/91` assets so their download row works. v0.19.92+ are upstream's; v0.19.101 is ours.
  Our RecommendedMaps fallback fix (`aae1675`) is on main and included from v0.19.101 onwards.
- **Server browser is style 2 (blue-glass cards) by default** (Tyler's pick), styles 1/3 still behind
  `-server-browser-style`. Modded sessions show MOD / MOD - NOT INSTALLED, `MOD: <title>`, version warnings, and
  DOWNLOAD MOD & JOIN opens `ModDownloadPrompt`. The details pane warnings wrap under the server address.
- **Loading-screen roster is adaptive**: one column for small lobbies, 2 columns at 16, compact multi-column grid for
  32/64/128 (`1 OF N IN`; total = 1 + lobby guests). Captures `roster_16/32/64/128.png` in the claude-tests folder.
- `-server-browser-demo` returns a mixed list; its first row is the joinable details case (missing mod + older host).

## Loading screen redesign (2026-11-15 02:20-02:35 ET, pushed in `93f61dd` + merge `ea572ae`)

Built to Tyler's reference layout, for all four modes (campaign, custom game, firefight, forge):
- **Top-left block**: letter-spaced big title (`LOADING` / `WAITING FOR PLAYERS` / `WAITING FOR HOST`), status line
  (`<MAP> - LOADING <pct>%` or `<MAP> - <joined> OF <total> IN`), thin progress bar, then the lobby roster rows
  (host first, `(YOU)` on the local player, status `HOSTING` / `READY` / `WAITING...`) and `PRESS T FOR CHAT` when online.
- **Top-right**: mode label (`CAMPAIGN` / `CUSTOM GAME` / `FIREFIGHT` / `FORGE`).
- **Background**: map picture drawn **complete at its native aspect** (never cropped/stretched; the old full-screen
  `DrawCover` was what made the 668x292 Reach previews look zoomed in), over a dim soft copy of itself. Reach's
  "Auntie Dot" **diamond grid is behind the picture** and drifts slowly with a travelling bright wave; the extracted
  **Noble Team emblem bottom-left and UNSC emblem bottom-right**.
- **WAITING FOR PLAYERS now works after the engine's own loading screen ends**: `BlamEngine` publishes
  `IsWaitingForRemotePlayers()` / `WaitingRemotePlayersJoined()` / `WaitingRemotePlayersExpected()` around the
  remote-player membership barrier, and `PauseMenu` keeps drawing the screen (before this it was a black frame with
  only the FPS counter). Counts come from the engine membership count when available.
- **Art assets**: `grid.png`, `noble_logo.png`, `unsc_logo.png` in `assets\ui\noble\loading\` (copied to
  `build\...\data\ui\noble\loading\` at build; **not committed** - extracted from HREK, see below). The code degrades
  gracefully (no grid/logos) if they are missing. `UiArt::LoadingGrid/LoadingLogoNoble/LoadingLogoUnsc` read them.
- **How the art was extracted** (repeatable): ManagedBlam via `runmb.ps1` + `Bungie.Tags.GameBitmap` - script pattern:
  `GameBitmap(TagFile, 0, 0).GetBitmap().Save(path, Png)` with tag
  `objects\cinematics\auntie_dot\bitmaps\auntie_dot_noble_logo` and `..._unsc_oni_logo`; the grid is
  `img_auntiedot_background.bitmap` (already extracted at `C:\Users\tyler\Desktop\ex\ui\cui\common\`).
- **Session plumbing**: the lobby passes `-loading-roster "host|guest|..."` to the match process (host first);
  `PauseMenu::SetLoadingSession(mode, players, expectedPlayers, host, joiner)` carries mode/roster/counts to the screen.
- **Test hooks**: `-loading-roster`, plus the existing `-wait-peers N` (host waits for N remote players - use
  `-host -lan -private -max-players 4 -wait-peers 2` to see WAITING FOR PLAYERS without a network).

## Server browser (Tyler picked option 2, 03:05 ET)

`-server-browser-style 1|2|3` still exists; **2 is the default** (blue-glass cards, cyan chrome, 98 px cards, bigger map
art). 1 = the older amber tactical cards, 3 = compact one-line table; keep them for comparison captures only.
Modded sessions: cyan `MOD` badge when the Workshop item is installed, amber `MOD - NOT INSTALLED` when missing, and
`MOD: <title>` in the map/mode line. The details pane names the required Workshop item, says whether it is installed,
and the primary button is `DOWNLOAD MOD & JOIN` when missing - that opens `ModDownloadPrompt` (real item metadata and
DOWNLOAD & JOIN / CANCEL). The installed check (Workshop cache + data\mods) is cached for 2 s in `ModInstalledCached`.
`-server-browser-demo` now returns a mixed list with installed and missing mods for captures.

## This session's fixes after v0.19.101 (2026-11-15 03:00-05:10 ET; all pushed to main + wip-merge)

Tyler gave an ordered bug list (see TODO-MASTER "BUILD/DEPLOY QUEUE"). Work done so far, newest last:

1. **Server-browser details pane (d422a74)** - mod/version warning lines used `TextColored` (no wrapping) and sat at the
   bottom, so they ran under the scrollbar and were clipped; DOWNLOAD MOD & JOIN was below the fold. Fix: all warnings go
   through a wrapping helper (`PushTextWrapPos`), are placed under the server address, duplicate mismatch text removed,
   details photo capped to 118 px and telemetry rows to 19 px. Evidence: `sb_wrap2/3/4.png`.
2. **Camera FOV is match-only (3a3c98e)** - SETTINGS > VIDEO > APPLY wrote the match FOV into the engine while the menu
   was up; the Armory is rendered by the same engine camera, so changing FOV zoomed the Armory model (Tyler's report).
   `CameraFov::Apply()` now returns early while `ReachArmory::IsMenuMode()`; `CameraFov::Tick()` applies the saved value
   on match start (pause-menu changes in a match still apply live). Test hook `-fov-apply-test-at <sec> <deg>`; evidence
   `fov_fixed_0/1.png`, `armory_fov_after.png` + log lines "not applying the match FOV here".
3. **Campaign/Custom Game mod sections + campaign player count (5f99735)**
   - MODDED tabs (Campaign, Custom Game, Firefight, lobby map select) used to show every map of every mod as tiles.
     `MapTileGroup.sectionMapCount` now makes one collapsed banner per mod; click/Accept toggles
     `MenuState::openMods` (`OpenKey(mode, mod)`); searching still expands/filters all. Evidence:
     `camp_vanilla_new.png`, `camp_modded_new.png`, `camp_mod_click.png`, `custom_modded_new.png`.
   - Campaign screen got a `MAX PLAYERS` row (4/8/12/16, `MenuState::campaignMaxPlayers`) wired into HOST CO-OP
     (`SetupHostLobby` used to hard-code 16 with no UI). Keyboard focus order is Difficulty -> Max Players -> Skulls ->
     Play Solo -> Host Co-op. Evidence: `camp_players_click3.png`, `camp_players_pick16.png`.
   - Campaign search box clamped on-screen (was clipped on narrow UIs); expanded-map drawer header now uses ASCII `-`
     because the game font has no em-dash glyph (the "?" placeholder Tyler saw: "FORGE WORLD ? BASE MAP & VARIANTS").
   - New test-only input hook `-menu-click-at <sec> <x> <y>` (repeatable; injects ImGui mouse clicks at capture
     coordinates, unfocused) and `-menu-debug-layout` (logs row rects). Used to verify section click + MAX PLAYERS picker.
4. **Mod-banner polish + in-game visor mapping (8f467e1)**
   - Banners: side chevrons removed; count and "CLICK TO EXPAND" are laid out from measured widths with a fixed gap (no
     overlap/underlap); expand hint hidden on very narrow rows. Evidence: `banner_clean_camp.png`,
     `banner_clean_custom.png`.
   - Visors (superseded by `96fd2c4` - see the dedicated "Visor colour follow-up" section below): `8f467e1` fixed the
     Armory-catalog vs MCC-item order (violet/ice/crimson/onyx) by resolving the finish through the shared
     `src\common\ArmoryVisor.hpp` resolver, with legacy index remap and the stock Black/Gold 3/4 swap. `96fd2c4` then
     found the real in-game limitation: the engine only has five real visor slots, so every finish now maps to a valid
     stock slot (nearest colour) instead of an invalid extended index that rendered white. Engine truth: Ghidra
     `0x18003B2F4` (item -> slot translator), patch site `0x2EDC5E` inside `FUN_1802eda08` (`EnginePatches.cpp`),
     `FUN_180039128`/`0x1800393CC` (profile enum -> item ids). ReachHost logs the applied visor slot.
5. **Settings > Game Options sliders (ec585f0)** - Tyler clarified the "armory scroll options misaligned" report is
   really SETTINGS > GAME OPTIONS camera sliders: each track was sized from its current value text width, so rows with
   "OUT 120 %"/"RIGHT 100 %" ended further left than rows with "100 %" and the wheels looked offset. `SliderValue` now
   uses a fixed value column (132 * UI scale); row labels are ellipsized at the slider column so long labels
   (`1ST-PERSON VEHICLE HORIZONTAL OFFSET`) no longer run under the track. Evidence: `settings_camera_aligned.png`,
   `settings_camera_crop.png` (all eight camera tracks x=664..968 in the crop, values share one right edge).

**Other session results already described elsewhere in this file**: loading screen + WAITING FOR PLAYERS (93f61dd), server
browser blue-glass default + mod/Workshop download flow (8811f55), adaptive roster 16/32/64/128, v0.19.101 release
(ada9f5d).

### Visor colour follow-up (2026-11-15 05:20 ET, commit `96fd2c4`)

Tyler reports his "blood orange" visor shows **white** in game. Root cause found (Ghidra): haloreach.dll's pmcg visor
translator (`0x3B2F4`) only produces internal slots 0..6 and the stock game has **five** visor items (157..161). The
earlier "30 finishes" patch (`0x2EDC5E` -> item-157) therefore stored invalid engine slots (7..29) for the extended
Noble finishes; the renderer draws those as white/no visor. The exact Armory colours only exist as map shader patches in
the Armory menu map; in match maps (campaign and multiplayer) the scan matches **0 rmsh materials** because the Spartan
materials live in the shared tag heap, not the map-local tag table - so the old "match tint" never applied to gameplay.
Current state in `96fd2c4`: `ArmoryVisor.hpp::ArmoryVisorStockIndex` maps every finish to a valid stock slot (nearest
stock colour, Black/Gold swap kept) so extended finishes show a real stock colour instead of white; exact custom tints
remain Armory-only. Making custom visor colours render in game needs either a shared-tag-heap material scan (map-wide tint
affects every Spartan) or extending the engine visor colour table/lookup (per-player correct); both are engine-level
tasks - ask Tyler whether to schedule.

## Multiplayer (item 13 on Tyler''s list) - prior work exists, read it first

Do not start from scratch. The 16 -> 32/128 player investigation, patch designs, traces and open blockers are already
written down:
- `docs\limits-players.md` (128 research: player arrays, session membership, masks, spawning, HUD/scoreboard/killfeed),
  `docs\limits-players-audit.md`, `docs\limits-players-loops.md` (32-slot plan C built, not tested; pools/host list),
  `docs\limits-players-message.md` (membership message type `0x1A` design + `NOBLE_SLOT_HIGH=1` high-slot test),
  `docs\limits-players-regbounds.md` (register-held bounds, guest stuck at peer state 4), plus the
  `limits-players-bounds*.json` / `sites.json` / `rest.json` / `trace.txt` data.
- `knowledge\drafts\draft-session-networking.md` (two player tables, masks/bounds, transport framing, lobby handshake,
  join-in-progress, slot counts, dedicated server, master cluster).
- Code: `src\host\SlotPatch.*`, `SlotMessage.*` (`NOBLE_SLOT_HIGH`), `PoolResize.cpp` (per-player pools), 
  `src\hooks\LobbyPatches.cpp` (`s_maxMultiplayerPlayers = 128`), `tools\limits\*` generators + `test-limits.ps1`.
- Current state per those docs: the 32-slot patches are implemented but **built, not tested**; multiplayer is the LAST
  item on the current queue by Tyler's instruction. Also `docs\limits-ai.md` + `Noble-AI-Limits-Report-2026-11-08.md`
  cover Firefight AI spawn limits for the "2x/3x enemy spawns" sub-item.

Full external-reference index (Ghidra exports, Compendium mirror, asset folders, tag tools): see
**`project-docs\REFERENCE-LIBRARY-2026-11-15.md`**.


## Where things stood at 20:35 ET (older history, kept for asset/release context)

- Repo `D:\Project-Noble-Closed-Source`. **`main` and `wip-merge` are both at `aae1675`** - pushed to origin; working
  tree clean. The tree is upstream `2ec2b49` (v0.19.91) plus this session's RecommendedMaps fix (`aae1675`).
- Release gates `noble-dev.exe` / `noble-client.exe` were built 20:13-20:14 ET from exactly this content, and tested.
- **Upstream is moving fast.** O7OF is publishing release after release from their own agent:
  v0.19.90 at 19:23 ET, v0.19.91 at 20:11 ET - both **base zip only** (no optional pack). They may release again at any
  time. Always `git fetch` before starting and before pushing; always check the releases page before publishing.
- Their releases were missing the optional Forge World pack, which broke the in-client download row for every client
  that looks at the latest release. **Both missing assets are now attached** by us:
  `Noble-ForgeWorld-v0.19.90-win64.zip` and `Noble-ForgeWorld-v0.19.91-win64.zip` (652.6 MB each, sha256
  `32c04269...`, identical payload to the v0.19.89 pack). Latest release v0.19.91 therefore carries the pack again.
- Release assets we published earlier tonight: v0.19.84 - v0.19.89 (base zip 356.9 MB at v0.19.89). Upstream's v0.19.90
  and v0.19.91 base zips are 346.4 MB (their packaging differs slightly; not inspected in detail).

## Second wave (20:00-20:35 ET): upstream merge + Forge download fixed

1. **Merged upstream** `a576e43` (v0.19.90 theater saved-film fix), `8f7a5b8` (BlamEngine/host/lobby/pause-menu work) and
   `2ec2b49` (v0.19.91 pause-menu theater fix) into `wip-merge` (fast-forward each time, no conflicts), rebuilt both
   gates and retested. The merge touches `FrontEndMenu.cpp`, `BlamEngine.*`, `PauseMenu.cpp` (a large rewrite),
   `ReachHost.cpp`, `LobbyPatches.cpp`, `main.cpp`.
2. **Theater saved-film playback works** (upstream feature, verified here): launched `noble-dev.exe` with
   `-film <path>` on a real saved film; log resolved the map from the film (`-film inspect resolved map 'm10', mode
   'campaign'`, `Configured Theater Saved Film playback`), the engine ran the film for 50 s with the film director
   overlay and subtitles, and the game-end/abort events during playback were suppressed by `LobbyPatches`. Captures:
   `theater_film_m10_25s.png`, `theater_film_m10_40s.png` (in the claude-tests folder) show campaign footage, not the
   mainmenu cruiser.
3. **Armory still good after the merge**: `-menu-screen armory -armory-cycle marine` run; log still chooses
   `ArmoryDepth: ... -> using 1920x1080 cleared to 0` and the Marine preview renders in front of the lattice.
   Evidence `armory_after_v01991_42s.png`.
4. **A normal match still runs after the merge**: `match_elite.ps1 -Quit 55 -Frames '45' -NoF5 -NoFocus` on Sword Base -
   `ElitePack: relocated 67051 packed pointers`, `extra cache haloreach\maps\noble_elite_pack load returned 1`,
   `map load returned 1`, `game session started at frame 247`, capture at 45 s, clean quit at 55 s. Evidence
   `match_swordbase_45s.png`.
5. **Forge World download fixed two ways** (the live bug found at 19:55):
   a) **assets attached** to v0.19.90 and v0.19.91 (above), so already-shipped clients find the pack in the latest
      release again;
   b) **code fix** `aae1675`: `RecommendedMaps` now reads `/releases?per_page=10` (newest first) and takes the newest
      release that actually carries a `Noble-ForgeWorld-*.zip` asset (plus its `.sha256`), instead of only looking at
      `/releases/latest`. A single-release object is still accepted, and `NOBLE_RECOMMENDED_API_URL` is a new test hook
      next to the existing asset/install overrides. This means a base-only release can never disable the row again.
   Proof (local HTTP server + fake release JSONs, final binary): newest-release-without-pack -> picks pack from the
   older release, downloads, checksum ok, installs (marker written); newest-release-with-pack -> picks that one; no
   release with a pack -> clean error "no Noble-ForgeWorld-* asset in the latest releases". Against the real GitHub
   list the same selection resolves the v0.19.90 asset (that test ran before the v0.19.91 asset finished uploading).

The first wave (16:30-19:30) shipped: release download split (`mainmenu_all.map` retired, base zip 1260 -> 607 -> 356 MB),
Forge World as the optional asset + in-client MODS > RECOMMENDED MAPS download (tested), updater hardened to the exact
client zip, menu audio double-gain fix, Armory lattice/depth fix, Marine/Female Marine Armory without Marine mods,
Trooper prototype removed, engine limits stock outside Forge, and the de-RR pack rename (v0.19.89:
`elite_armor.map` + Noble's own `NBRL` `.noblereloc`, no RR-named files, verified in a match with 127,379 pointers).

## Queue additions from Tyler (2026-11-15 05:45 ET) - all scheduled, multiplayer stays LAST

1. **Lobby share codes + prominent join-with-code** - CORE DONE `88d4937` (codec/self-tests, Main Menu entry, Server Browser modal, lobby COPY CODE panel; captures `join_code_*.png`; Social action + two-PC test remain). Full plan:
   `project-docs\PLAN-LOBBY-CODES-2026-11-15.md`. Summary: public and private hosts both generate a copyable share code
   (self-contained Crockford-Base32 of session id + endpoint + privacy/build; session ids are 16 hex chars from the
   master server), a JOIN WITH CODE entry goes on the Main Menu + Server Browser (+ Direct Connect modal) and a
   "JOIN WITH CODE" action in the Social screen; incoming friend invites already exist (SocialClient `/api/invites/*`,
   toast + JOIN GAME cards) and stay. Private lobbies are already supported by `MasterAdvert::isPrivate`; the code must
   work for them even though they are unlisted. Files, join flow, tests and UI placement are in the plan doc.
2. **Engine-level custom visor colours in matches** (the follow-up described above): shared-tag-heap material scan or
   engine visor colour table extension; bigger RE task.
3. **Armor effect colours**: recolour/duplicate the flame + lightning/storm effects and expose new colour variants in the
   Armory. Recon already done: HREK assets live at `objects\characters\spartans\fx\armor_fx\`
   (`armor_fx.effect` events: legendary_flame -> `particles\head_fire.particle`, eternal_flame ->
   `head_fire_bungie.particle`, storm -> `thunder_cloud.particle` + `electric_crackle.particle` + `lightning_flash.particle`
   (plus a revenant plasma_bolt reference), pestilence -> bugs/gas/drip particles, heart_attack -> cupid_heart.particle).
   Particles are palette-driven (`palette_shift_amount`, palette bitmaps such as
   `fx\particles\fire\gradients\fire.bitmap`, `fx\particles\_gradients\plasma_blue_01.bitmap`), and the tags dump cleanly
   with `tools\runmb.ps1` + `tools\tagdump.py` (quick recipe in the plan doc). Recolouring an existing effect is a
   tag/bitmap job; exposing *new* effect ids needs extra slots in whatever engine/effect table the armor-effect item ids
   (kEffect 166..172) index - investigate before promising the listing.

## Open work, in priority order

1. **Tyler's ordered fix queue (live status in TODO-MASTER "BUILD/DEPLOY QUEUE")**. Done so far: FOV/Armory, campaign
   mod sections, campaign MAX PLAYERS, mod-banner polish, in-game visor mapping, Settings > Game Options sliders.
   Remaining, in order: (5/6) search + game-mode category filter inside expanded variant drawers (Forge World 353
   variants + custom-games mod variants); (8) server-browser Refresh/Favourite click bug; (9) "What's New (N)" hint
   click; (10) Game Options buttons in Custom/Firefight/Forge; (11) controller toggle crouch; (12) M&K toggle zoom;
   (13) multiplayer last (campaign 4 -> Firefight AI spawn multipliers -> 16 -> 128 players). Use `-menu-click-at` to
   reproduce the click bugs instead of guessing.
2. **Docketed, not scheduled**: one player reported 3rd-person camera spazzing left/right in milliseconds; four other
   players could not reproduce it after a settings reset on public + dev builds. Upstream v0.19.100 (in v0.19.101)
   reworked the third-person camera and may have fixed it - ask the affected player to retest before investigating.
3. **Publish a new release for the dev-only fixes** (`d422a74`, `3a3c98e`, `5f99735`, `8f467e1`, `ec585f0`): bump
   `Version.hpp` + release notes, build both gates, `tools\publish-release.ps1` (creates base + Forge pack zips).
   Coordinate with O7OF first - they release every ~hour; publishing the same tag races.
4. **If a release is published without the Forge pack**, attach one (`Noble-ForgeWorld-v<tag>-win64.zip` + `.sha256`,
   payload identical to the v0.19.101 pack) or un-updated clients' Recommended Maps row errors.
5. **Remaining RR provenance is inside `noble_menu.map`, not in files**: the map's HaloScript still uses `rr_*` variable
   names and the map is built from the RR-era menu map. Invisible to players and to the file list. Removing it means
   renaming script variables in HREK and rebuilding/spot-checking the menu (hours, no visible change) - Tyler's call.
6. **Optional local cleanup, dev only (ask Tyler first):** `build\bin\Release\data\armor\noble_elite_pack.map` (286 MB
   dev pack; loader prefers it when present - the released `elite_armor.map` is the same feature at 99 MB, proven);
   ~3 GB of `noble_menu.map.*` backups under `build\bin\Release\data\menumaps`; repo `data\menumaps\mainmenu_all.map`
   (285 MB, no longer shipped, dev fallback only - VFS order is noble -> mainmenu_all -> stock).
7. **Old bugs to hand-check (unchanged):** Forge on a Firefight canvas, custom-map missions, "Respawn in 00000001"
   padding, pause not freezing. All need Tyler at the keyboard.
8. **Parked:** flashlight, multiplayer two-PC test, other Armory items (knife chests, emblem, undersuit colour, Elite
   shine/lights), Forge-palette "Option 2" add-on pack (Pelican/Spectre/Boathog/targeting laser injected into any map's
   palette instead of the 750 MB rebuilt Forge World), Marine player animation (only matters if a Trooper-style mode
   comes back through a mod).

## Release procedure (how v0.19.84-89 were made)

1. `git fetch` + merge/`pull --ff-only`, keep the tree clean; check the releases page for upstream's latest.
2. Bump both strings in `src\common\Version.hpp` and add `release-notes\v<tag>.md`.
3. Build the gates: `noble-dev` + `noble-client` (the dedicated build does not block a push).
4. Run `tools\publish-release.ps1` (params: `-SkipBuild`, `-NoUpload`, `-NoForgePack`). It stages `Noble.exe`,
   `play.bat`, data, `data\menumaps\noble_menu.map` + `*.string_ids.csv`, `data\armor\elite_armor.map` +
   `.noblereloc`; creates the base zip **and the `Noble-ForgeWorld-*` zip** unless `-NoForgePack`; writes `.sha256`
   files; uploads to the GitHub release for the tag. It **throws** if the armour pack or the noble menu map is missing,
   so a mis-staged build fails loudly instead of shipping RR files.
5. **Check GitHub before publishing**: O7OF publishes too. Publishing the same tag races and can replace their release.
6. After publishing, list the zip entries (`data/armor`, `data/menumaps`), confirm the release has both
   `Project-Noble-*` and `Noble-ForgeWorld-*` assets, and run the client once to confirm the updater and RECOMMENDED
   MAPS rows resolve against the new release.

## Build and test

Build (from `D:\Project-Noble-Closed-Source`):

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build --config Release --target noble-dev noble-client
```

New `.cpp` files must be added to `CMakeLists.txt` (client and `DEDICATED_SOURCES` lists when used there). `cmake` is
not on PATH in this shell - use the full path above.

Menu (Armory, no focus taken):

```powershell
tools\launch_noble.ps1 -Exe noble-dev.exe -Arguments '-menu-screen armory -menu-ignore-input -armory-cycle marine -armory-cycle-delay 3000 -capture-at 30,42 -quit-after 55'
```

Theater saved-film playback (real film file; resolves map/mode from the film itself):

```powershell
tools\launch_noble.ps1 -Exe noble-dev.exe -Arguments '-film "D:\Project-Noble-Closed-Source\build\bin\Release\data\theater\films\<name>.film" -capture-at 25,40 -quit-after 50'
```

Match (Sword Base + Elite pack):

```powershell
match_elite.ps1 -Quit 55 -Frames '45' -NoF5 -NoFocus
```

Forge World:

```powershell
forge_test.ps1 -NoOverride
```

Recommended-maps download test (local server; override env vars, no real GitHub):

```powershell
$env:NOBLE_RECOMMENDED_API_URL='http://127.0.0.1:8765/releases-fallback.json'; $env:NOBLE_RECOMMENDED_INSTALL_OVERRIDE='<scratch dir>'; launch_noble.ps1 -Exe noble-dev.exe -Arguments '-menu-screen mods -menu-ignore-input -recommended-download forge_world -quit-after 20'
```

Captures land as `capture_*.bmp` in `build\bin\Release` (and PNG conversions in the test folder); logs are
`build\bin\Release\noble-dev.log`. Campaign: `camp_test.ps1 -Name X -Map m35 -Frames '160,190' -Quit 195 -Extra '-third-person' -NoFocus`.

Full tool list and PowerShell/tag-tool gotchas: `AGENT-BRIEF.md`; detailed day-by-day logs: `pc-handoffs\`.

## Rules (keep them)

- **Never test while Tyler is playing** (`Get-Process noble-dev, noble-client`). Do not close his game unless he says so.
- **Tests open on the AOC (second monitor), never the Samsung**: scripts set `NOBLE_WINDOW_SECONDARY=1`.
- **Do not take his mouse**: pass `-NoFocus`; F5 needs focus, so ask before using it. (`match_elite.ps1 -NoF5` skips the
  key but still prints "F5 sent" - that is just the message.)
- Back up/restore `build\bin\Release\data\appearance.dat` around every test (scripts do).
- Prove every cause with a one-variable test plus a capture/log before telling Tyler; check the capture yourself.
- Tyler is not a C++ developer: exact commands, one per code block.
- Keep game assets, MCC map files and RR data out of git; `knowledge\` is git-excluded (never committed).
- **HREK edits get backups. Never modify the installed `haloreach.dll` or MCC maps.**
- **Use the references before re-discovering them**: `project-docs\REFERENCE-LIBRARY-2026-11-15.md` (Ghidra MCP,
  decompile exports, docs\, drafts\, external asset folders). For engine truth use Ghidra MCP (`haloreach-copy.dll`,
  image base `0x180000000`); do not guess RVAs. For multiplayer read `docs\limits-players*.md` +
  `knowledge\drafts\draft-session-networking.md` first.
- **Docs convention**: commit messages are subject + body paragraphs (current tree has no Co-Authored-By trailers -
  match the latest style). After finishing work, update this handoff, `desktop-guides\TODO-MASTER-2026-11-14.md`,
  `AGENT-COORDINATION.md`, and sync the changed files into `C:\Users\tyler\Desktop\Compendium\` (same subfolders).
  Say WHAT changed, WHY, WHICH files/commits, HOW it was verified (evidence path), and what an agent should read next.
- Only view screenshots Tyler names.
