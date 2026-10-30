# Project Noble — continuation guide (2026-10-30, ~07:15 ET)

Written for the next agent (Claude Opus 5.5) picking this up cold. Read this, then `docs/HANDOFF.md` (it is the
per-change log; **every change gets a new entry at its top**, see §7). This file is the map: where everything is, what
is done, what is broken, and what to do next.

Owner: **Tyler** (GitHub `WeabooTyler117`, Windows PC). Repo owner / main C++ author: **O7OF** (GitHub `O7OF`).
Tyler is **not a C++ developer**: give him one command per code block, say what to look for, and do the work yourself
when you can.

---

## 0. TL;DR

- Everything is in **`D:\Halo-Reach-Community-Edition`** (the repo). `origin` = `https://github.com/O7OF/test-build-01`,
  branch **`main`**. Last pushed commit when this was written: **`dc71e0b`**; working tree clean; `stash@{0}` still
  exists from 2026-10-29 (see §7, do not apply it blindly).
- Build with `cmake --build build --config Release` (see §2 for the full path to cmake). It builds
  `build\bin\Release\noble-client.exe` and `noble-dedicated.exe`, **0 errors / 0 warnings** as of `dc71e0b`.
- The client needs the player's **MCC install** at run time: `D:\Steam\steamapps\common\Halo The Master Chief Collection`
  (`haloreach\haloreach.dll`). Steam must be running. Nothing from MCC is copied into the repo.
- This session did the **menus** (layout, player rows, mod lists, account modal, server browser details, invite toast).
  It did **not** touch netcode, engine hooks, dedicated server or the social server.
- Next up (recommended, see §6): media player widget, controller presets/custom mapping, skull collection, new-scenario
  mod maps, post-game player names — all four last items need engine-side work or the RE folder in §8.

---

## 1. Paths on this machine

| What | Path |
|---|---|
| Repo / workspace | `D:\Halo-Reach-Community-Edition` |
| Built client | `D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.exe` |
| Client log | `D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.log` (appended across runs) |
| User data (settings, appearance) | `build\bin\Release\data\userdata\settings.json` |
| Nameplate / rank / armory art | `build\bin\Release\data\ui\...` (git-ignored except 298 tracked art files) |
| MCC install | `D:\Steam\steamapps\common\Halo The Master Chief Collection` |
| Steam Workshop (Reach mods) | `D:\Steam\steamapps\workshop\content\976730\<id>\ModInfo.json` |
| Reach Renaissance install (reference only) | `D:\Reach` |
| Reverse-engineering material | `D:\REACH REVERSE ENGINEER` (see §8) |
| Master/social server source (client copy) | `D:\Halo-Reach-Community-Edition\server\server.js` |
| Master/social server deployed copy | `D:\ServerTestDeploy-main` + Render: `https://outbreak-z-test-server.onrender.com` |
| Handoff log | `D:\Halo-Reach-Community-Edition\docs\HANDOFF.md` |
| Design spec | `D:\Halo-Reach-Community-Edition\docs\menu-redesign-spec.md` |
| Tyler's export copies | `C:\Users\tyler\Downloads\HANDOFF (2).md` (kept in sync from `docs/HANDOFF.md`) |

---

## 2. Build, run, test, screenshot

Build (works from any shell; `cmake` is not on PATH, use the full path):

```
cd /d D:\Halo-Reach-Community-Edition
```
```
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release
```

Full rebuild of the targets (keeps the CMake cache):

```
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release --clean-first
```

`build.bat` also works but **runs `git fetch` + `git pull origin main` first** - fine with a clean tree, dangerous with
uncommitted work. `build.bat rebuild` wipes the CMake cache (slow, re-fetches dependencies).

Run (single instance; the client refuses to start a second one for the same account):

```
cd /d D:\Halo-Reach-Community-Edition\build\bin\Release
```
```
.\noble-client.exe
```

Screenshot harness (drives a real menu screen, saves a BMP next to the exe, closes itself):

```
.\noble-client.exe -menu-screen firefight -menu-tab 1 -menu-capture 90
```

- `-menu-screen` accepts `main`, `campaign`, `skulls`, `firefight`, `custom`, `forge`, `servers`, `settings`, `armory`,
  `mods`, `career`, `lobby`/`pregame`, `social`.
- `-menu-tab N` = tab index for Settings / Career (category order in `SettingsPanel.cpp` `kCats`), and since 2026-10-30
  it also selects the VANILLA (0) / MODDED (1) source tab on Campaign / Firefight / Custom Game / Forge.
- Output: `capture_<frame>.bmp` in the exe folder. Convert with Python + Pillow if you need to look at it.
- `-menu-capture 90` waits until the engine scene has been ready for 90 frames (fallback: 480 extra frames), then
  saves and posts WM_CLOSE.
- **Gotcha:** a held pad button / key on the first frames can register as Accept and take the capture off the intended
  screen (seen with `-menu-screen main` landing on Campaign). Re-run with the pad released.

Tyler's own test recipes (from `docs/HANDOFF.md`):

```
.\noble-client.exe -map 20_sword_slayer -mode multiplayer -host -lan
```
```
.\noble-client.exe -connect 127.0.0.1:11774
```

---

## 3. Source map (only what matters for menus; the rest is unchanged and documented in HANDOFF)

```
src/
  main.cpp              process start, parses -map/-mode/... , loads haloreach.dll, hands off via Relaunch
  ui/
    FrontEndMenu.cpp    THE menu: every screen, layout helpers, PartyRow, lobby, modals, invite toast  (~3.7k lines)
    FrontEndMenu.hpp    FrontEndChoice (what the menu hands back to main.cpp)
    ServerBrowser.cpp   GAMES screen: table, filters, details panel with map photo
    UiArt.cpp/.hpp      all menu art: MCC texture packs, nameplates, emblems, map previews, MapPhoto()
    NobleStyle.cpp      fonts (Bahnschrift), colours, ImGui style
    NobleUi.cpp         small UI kit (headers, sliders, toggles)
    SettingsPanel.cpp   SETTINGS screen (3 columns, categories in kCats)
    ArmorCustomizer.cpp ARMORY (O7's area; also the emblem/nameplate pickers)
    CareerPanel.cpp     SERVICE RECORD
    PauseMenu.cpp       pause + post-game panel (post-game names are here, see §6.4)
    InputPrompts.cpp    A/B/X/Y glyphs or key boxes
    MenuSounds.cpp      UI clicks -> ReachAudio
  network/
    MasterServerClient.cpp/.hpp  sessions, lobby roster, adverts (identity fields live here now)
    SocialClient.cpp/.hpp        friends, requests, invites (GetInvites / PollInvitesAsync / ClearInviteAsync)
    AuthClient.cpp/.hpp          login/register/session token, IsDeveloper()
    NetworkTransport.cpp        UDP + WebSocket relay
    SessionManager.cpp          hosting/discovery
  engine/
    BlamEngine.cpp      launch block, ResolveMapId (map ids!), LaunchMap
    ReachHost.cpp       haloreach.dll host vtable (slots), input, profile
  common/Settings.cpp/.hpp  settings.json (every persisted option)
  hooks/                   engine patches (LobbyPatches, EnginePatches)
docs/                      HANDOFF.md (log), menu-redesign-spec.md (design), career/, armor-list.md
server/server.js           master + social server (deployed separately to O7's ServerTestDeploy repo)
tools/armory/              Halopedia art fetchers
build/                     build output; 298 art files tracked (git add -f needed for new ones)
```

### The menu's layout system (what this session built - do not regress it)

- `RightColumnLayout(ui)` returns `actions` / `strip` / `content` for the bottom-anchored right column of the setup
  screens. Rows flow top-down from the banner; `HeroBannerHeight()` fills the leftover space. **Never pin rows to
  `actionsY` again** - that was the original overlap bug (rows ended under the network strip).
- `BlurbBlockHeight()` + a 2-line clip in `HeroBanner()` mean a long workshop description can never move the layout.
- `PlayerRow(ui, pos, width, rankIndex, emblemIndex, nameplateIndex, team, serviceTag, name, dev, selected)` draws
  MCC's row: rank badge, emblem tile (joined), optional team strip, nameplate bar with `[TAG]` on top and the gamertag
  below, `[DEV]` in gold next to the tag for developer accounts only. Used by MY PARTY and the pre-game roster.
- `InputPrompts` owns every on-screen button glyph and the last-used-device switch. Use `InputPrompts::Draw(prompt)` for
  Accept/Back/Change/Tabs, `DrawButton(...)` for a specific pad button, `DrawKey(...)` for a keyboard key - never draw a
  glyph by hand, or it will not follow the device (that is what the lobby bar used to do).
- The main menu list is built per frame (`rows` vector + `kDeveloperEntry` above Quit for devs) and reserves a fixed
  **two-line** description so the list never jumps when the selection changes.

---

## 4. What is done (as of `dc71e0b`)

Menus (this session, all pushed):

| Commit | What |
|---|---|
| `8676b1b` | Right column no longer overlaps the network strip (Campaign/Firefight/Custom/Forge); lobby `BUNGIE` tag removed |
| `b0aac8b` | Mod headings clipped; workshop descriptions cleaned (`PlainDescription`) and capped to 2 lines; greyed mod maps selectable; dev menu moved above Quit; account modal rebuilt; server browser ESC -> main menu, map photo in GAME DETAILS, headers fit, first server auto-selected |
| `42f9c92` | Build fix for O7's `dc349a5` (duplicate `lockOption` lambdas made `origin/main` uncompilable) |
| `dc7f763` | `tools/armory/fetch_halopedia_icons.py` checked in |
| `6c48554` | Main menu no longer jumps; map blurb readable (dark band + shadow); `All Skulls` label; nameplates in party/lobby |
| `d9bffdb`, `60d0419` | Nameplate geometry (taller rows, 80 % width, scrim, frame) |
| `bf9d384` | MCC-style `PlayerRow` (rank / emblem / nameplate with tag+name), `MY GAME SESSION` header, empty lobby slot fix |
| `8059115` | **Emblems load again**: MCC pack stores `E_002_P_SML`, not `e_002_p` (upper case + `_SML`) - this also fixes O7's Armory emblem grid |
| `e93076f` | `[DEV]` as text, centred rank badge, one framed block, smaller name |
| `7d0d80b` | Main menu **Continue** fast path + design-doc status block |
| `a21ebce` | Menu **background picker**: `file:<name>` backgrounds, IMPORT via file dialog, preview panel; `Row()` trap documented |
| `5f26667` | Review pass: 6 bug fixes (preview title, stale picker cache, dead unsafe API, invite JOIN without host, stale mod on Continue, per-frame track list) |
| `5e512b7` | Halo: Reach **button layouts** (PadLayouts) applied in slot 36 + BUTTON LAYOUT row + layout-aware diagram |
| `7383c54` | Map/game-type guard (`MapGameTypes`) + `-test-map-gametypes` (280 combinations); armory backdrop attempt reverted |
| `b18cf29` | `tools/test-modes.ps1` mode/game-type smoke matrix |
| `31b7362` | Mouse acceleration + smoothing, controller stick layout (Southpaw); test script teardown hardened |
| `89a9509` | `MapNames` display names in the server browser + covered by `-test-map-gametypes` |
| `2e075a8` | `Settings > SYSTEM` (logs folder, diagnostics to clipboard, settings backup/restore); version 0.8.3 |
| `1f8d89b` | Master mute + mute in background (`ReachAudio::SetMuted`) |
| `c30659a` | Button-layout remap tests (`PadLayouts::SelfTest`), third suite under `-test-map-gametypes` |
| `ea55d4c` | Menu music playlist: per-track ON/OFF, honoured by SHUFFLE ALL GAMES |
| `04e6700` | Docs: armory backdrop clear-hook approach ruled out |
| `45a18b6` | Docs: all 21 armory scenes probed, none has a backdrop |
| `59154ac` | Docs: the PostCommand(11) stall investigation (conclusion corrected above) |
| `3a777bf` | `-auto-play` / `-auto-play-variant` hooks; harness drives the real launch path; 15/15 modes x game types reach gameplay |
| `1902aed` | GAME TYPE list filtered to what a vanilla map can host (mods unaffected) |
| `54aeecb` | Controller **per-action rebinding** (`padBindings` + BUTTON BINDINGS list); `-auto-play-map` and the 35/35 all-maps matrix |
| `2834507` | Server browser: sortable columns + `No password` / `Not started` filters |
| `a52884f` | Real **BRIGHTNESS** + **FXAA** (measured), dead advanced-graphics rows replaced with honest text, `InputTuning` extraction + 4th test suite, `tools/check-settings-wired.ps1` |
| `b8f667b` | Service record: mission/map names instead of scenario ids |
| `a848054` | Pause menu rebuilt (action list + scrollable daily/weekly challenge sidebar, solo/multiplayer header, RESTART MATCH); `-pause-at` test hook + flag forwarding |
| `2192054` | CONTINUE limited to campaign missions (recording, row and action) |
| (this commit) | "No limit" is genuinely uncapped (was 120); unfocused 30 FPS throttle is now a setting |
| `5f4da08` | Prompts follow the device everywhere; `InputPrompts` button glyphs in the CONTROLLER list |
| (this commit) | Main menu **Continue** fast path (spec 4.1/4.10) + a status block at the top of the design doc |
| `a8f412e` | Menu music player (`PauseMusic/ResumeMusic` + `DrawMusicPlayer`) |
| (this commit) | Prompts follow the device on every screen; `InputPrompts::DrawButton/DrawKey` glyphs in the CONTROLLER list |
| `aea6230` | Custom Game / Forge: GAME OPTIONS + PLAYERS rows, row plates for readability |
| (this commit) | Menu music player (`PauseMusic/ResumeMusic` + `DrawMusicPlayer`) |
| `ab4a9f4` | One service tag everywhere (Armory profile wins), `-menu-screen social` |
| (this commit) | Custom Game / Forge: GAME OPTIONS + PLAYERS rows (lobby cap 4-16 now real), row plates for readability |
| `f40ae67` | `docs/CONTINUATION.md` (this file) + the identity/invite handoff entry |
| `dc71e0b` | Lobby identity per player row (`rankIndex`/`emblemId`/`nameplateId`/`team` in `LobbyPlayer` + `MasterAdvert` + register/heartbeat JSON + roster parse) and **invite notifications** with a JOIN/DISMISS toast + `Settings > Online services > INVITE NOTIFICATIONS` |

Earlier work (netcode, engine, armory) is in `docs/HANDOFF.md` and is **not** summarised here - trust that file.

Known-good state: `cmake --build build --config Release` = 0 errors / 0 warnings. **Full sweep on 2026-10-30**: all 17
menu screens captured successfully (main, campaign +/- MODDED, skulls, firefight +/- MODDED, custom +/- MODDED, forge,
servers, settings, settings > Online services, career, armory, mods, social, lobby), every run exit code 0, no `[error]` /
`[warn]` in the log tail. The service tag now comes from the Armory profile everywhere (`PlayerServiceTag()`), because
settings.json and appearance.json could disagree (`RTRD` vs `N06`).

---

## 5. What is NOT done - prioritized backlog

### 5.1 Server side must catch up (2 small changes, O7 or whoever owns `server.js`)

The new lobby identity is **dropped by the deployed server** until it echoes the fields. Add to `POST /api/sessions`,
`PUT /api/sessions/heartbeat` and the lobby roster response in `server/server.js`
(`D:\ServerTestDeploy-main` + Render deploy):

```js
// on register / heartbeat (body):
host_rank_index, host_emblem_id, host_nameplate_id, host_team      // numbers
// inside the roster entry built for the host:
{ peerId, machineId, username, isHost, isDev, serviceTag, ping,
  rankIndex, emblemId, nameplateId, team }
```

The client already sends and parses exactly those keys (`MasterServerClient.cpp`, `{"host_rank_index", ...}` and
`item.value("rankIndex", -1)`); until the server passes them through, remote rows keep the placeholder badge and flat bar.

Also: invites carry `{senderUsername, hostIp, hostPort, timestamp}` only. To say *which* match ("... inviting you to a
**Firefight** match") the invite payload needs a `mode`/`map` field - small change in `SocialClient::SendInviteAsync`,
`/api/invites` on the server, and the toast in `DrawInviteToast()`.

### 5.2 Menu features still open (UI-only, fully testable)

1. ~~**Menu music player widget**~~ **DONE** (2026-10-30 08:15 entry): `ReachAudio::PauseMusic()/ResumeMusic()/
   IsMusicPaused()` added and `DrawMusicPlayer()` puts a now-playing strip with prev / play-pause / next / mute /
   shuffle on the main menu.
2. **Controller presets + custom mapping + extra buttons** - biggest item. The pad is remapped in `ReachHost.cpp`
   (slot 36 input path), so presets (Default / Southpaw / Boxer / Green Thumb / Bumper Jumper / Recon) are a mapping
   table there plus per-action storage in `SettingsData`; the CONTROLLER settings page already has the diagram and
   per-action placeholders.
3. **Skulls**: locking until found in the campaign, and an "All Skulls" toggle gated on having collected all 13. Needs
   campaign skull-collection data, which Noble does not read yet (engine research - §8). The label ("All Skulls") is
   already in `SkullSummary()` and the lobby row.
4. **Post-game names**: the report shows `Player2`/`Player3` instead of names for remote players
   (`Career::ScoreRow` falls back to "Player N" when the record name is empty). Needs the engine's player record name
   path researched (`ReachHost.cpp` slot 88 / player records; see the 2026-10-29 14:xx entries in HANDOFF).
5. **New-scenario modded maps** (Zenith, Eclipse, Sidereal, ...): `BlamEngine::ResolveMapId()` only knows Reach's own
   map tables and returns `0` for anything else, which boots the main menu. Real work: a launch index / map id for a
   scenario that is not in Reach's table. Partial option: a mod map whose *title* contains a Reach map name
   ("BREAKPOINT - (DKF)") could launch that base scenario through the VFS - un-greys those, not brand-new maps.
6. **Server browser leftovers**: show map *display* names instead of scenarios (`forge_halo` -> `Forge World`), a mod
   badge/photo once servers advertise mod ids, and Tyler's "anything else to add/remove" pass.
7. **Housekeeping**: `VERSION_STRING` (`src/common/Version.hpp`, still `0.8.2`) should be bumped so two builds can be
   told apart - the "other player isn't seeing the fixes" confusion came from that.

### 5.3 Known but unscheduled

- The account modal and lobby were rebuilt but modals cannot be screenshotted by `-menu-capture`; Tyler should eyeball
  them once.
- O7's `dc349a5` appended **27 identical copies** of his handoff entry to `docs/HANDOFF.md` (~2,100 duplicate lines).
  Left untouched (his text). Dedupe when he agrees.
- `stash@{0}` ("armory-menu-handoff attempt 2026-10-29") is still in `git stash list`; its `Relaunch::HandOff` half is
  already committed (`32cd0f1`), so it can be dropped once Tyler agrees.

---

## 6. Working agreement / traps this session hit

1. **`docs/HANDOFF.md` is the log and must be updated for every change**, newest entry at the top (see the rule block at
   the top of that file). Keep the `C:\Users\tyler\Downloads\HANDOFF (2).md` export in sync after each push.
2. **`build/` is git-ignored but 298 art files inside it are tracked.** New art needs `git add -f`. The exe is never in
   git: **a `git push` does not deliver the client** - the other player must `git pull` *and rebuild*.
3. `small` is a Windows macro (`#define small char`): `if (UiTexture small = ...)` fails with a confusing C2628.
4. MCC texture-pack names are **UPPER CASE with suffixes** (`E_002_P_SML`); `playercustomization.xml` uses
   `e_002_p`. Dump an index with Python: 0x1E0-byte records in `<pack>.perm.bin`, magic `0xCDBFA090`, name at `+0x44`.
5. Nameplates are **PNG files** (`MCC\Data\UI\NamePlates\Images\<name>.png`), emblems are **pack images** - different
   paths in `UiArt.cpp`.
6. The client is a single instance per account; `taskkill /IM noble-client.exe /F` before rebuilding (the linker cannot
   overwrite a running exe).
7. `Relaunch::HandOff()` ends the process on purpose; `Relaunch::Start()` is for callers that keep running.
8. **`SettingsPanel::Row()` always returns true** (it draws the row); action rows must check
   `m_rowHovered && ImGui::IsMouseClicked(0)` / `m_rowFocused && Accept`. Opening a dialog from its return value runs it
   every frame and hangs the client.
9. Do **not** load textures from the main-thread fallback draw path (`HoldFrame`). Warm art in
   `FrontEndMenu::Initialize` (`UiArt::WarmMenuBackground()` is the pattern).
10. The post-process (`GammaPass`) is the only place Noble can change the picture: brightness and FXAA live there. If you
    add a field to its constant buffer, **the sizes must match the HLSL cbuffer exactly** (a 16 vs 32 byte mismatch crashed
    the client the first time the pass ran), and never pass a null array to `PSSetSamplers`.
11. The headless suites under `-test-map-gametypes` cover game types, map names, button layouts and input tuning; run it after
    touching `MapGameTypes.cpp`, `MapNames.cpp` or `PadLayouts.cpp`. Engine model to remember: fire is analogue RT,
    grenades are the **B button**, melee **RB**, armour **LB**, zoom **RS click**.
11. **A bare CLI launch cannot start a match** - the client needs the `-account-user` / `-account-token` / `-user-id` /
    name arguments that the menu adds on hand-off. Use `-menu-screen <setup> -auto-play [-auto-play-variant <stem>]` and
    let it hand off (that is what `tools/test-modes.ps1` does); without identity the engine logs "no game selected" and
    never pumps a frame.
12. Game types: `MapGameTypes::Supports()` is the only place that decides whether a vanilla map may host a type; run
    `noble-client.exe -test-map-gametypes` after touching its tables. Modded maps are deliberately never checked.
11. `ReachAudio::MusicTracks()` builds the whole list - cache it if you call it per frame. Muting from the music widget
    is overridden while Settings > Audio is open (that page pushes its volume every frame).
11. `MenuState`/`OverlayContext` state is per-screen; `st.screen` changes are logged (`FrontEndMenu: screen X -> Y`),
   which is handy when a capture lands on the wrong screen.

---

## 7. Reverse-engineering resources on this machine

### 7.1 Tyler's RE drops in `C:\Users\tyler\Downloads`

These appeared during the 2026-10-30 session and are exactly what the engine-bound backlog (§5.2 items 3-5) needs:

| File | What it is |
|---|---|
| `haloreach.snap` (JSON, ~60 KB) | A **Ghidra evidence bundle**: keys `binding` (analysis profile: Ghidra 12.1 + digests), `entries`, `evidence_bundle`, `target`, `workflow_entries`. Provenance for the analysis below (which DLL build was decompiled). |
| `dec_fov_writer.json`, `dec_fov_wrapper.json`, `dec_fov_getter.json`, `dec_core_fov.json`, `dec_cam_a.json`, `dec_cam_a (1).json`, `dec_cam_b.json` | **Ghidra 12.1 decompilation exports**, one function each, with `evidence_id`, `subject` (haloreach.dll sha256 `738dd2d2...b0894`), `provider`, `raw_result`/`normalized_result`, `caller_input_path`, `locations`. These are the FOV writer/wrapper/getter and the two camera functions - the basis for the remaining **FOV / camera / viewmodel** work (spec §4.10) and for locating a per-player record if needed. |
| `globals_table.py`, `scan_globals.py` | PE scanners that walk haloreach.dll and print global/`GLOBALS` ranges (Python, `struct` only). |
| `find_refs.py`, `dump_q.py` | xref/pattern dumpers (find references to an address / dump a queue of hits). |
| `pe_probe.py` | PE header probe (sections, image base) - the first step before any RVA work. |
| `find_funcs.ps1`, `fix_lines.ps1`, `find_strings.ps1`, `parse_dmp.ps1` | PowerShell helpers: list functions, normalise/repair dumped listings, find strings, parse a minidump (`noble-crash.dmp`). |

**Caveat:** every one of those scripts has the DLL path hard-coded as
`C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll`, which is the
**old install location**. MCC now lives at `D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll`
- fix the `PATH` constant at the top before running them.

### 7.2 Everything else

- **`D:\REACH REVERSE ENGINEER`** - Tyler's RE material: `docs\` (incl. `host-probe-functions-so-far.txt`,
  `rr-launch-flow.md`, `rr-tags.md`), `spartans\`, `elite\`, `bothtpyes.zip`, `rr_profile.bin`,
  `rr_profile_spartan.bin`. Start here for map ids, player records and skull data.
- **`D:\Reach`** - the installed Reach Renaissance (reference behaviour only; no RR code or data may be redistributed).
- `docs/HANDOFF.md` - the engine history: RVAs, patch bytes, vtable slots, launch-block layout, why things are the way
  they are. `docs/networking-review.md`, `docs/armory-list.md`, `docs/career/`, `docs/fov-findings.md`,
  `docs/hud-text-findings.md`, `docs/elite-pack-notes.md`, `docs/boot-investigation-*.md`.
- `haloreach.dll` is at `D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll`
  (read-only; never copy it into the repo).

---

## 8. How to hand this back

When you stop: write a HANDOFF entry, bump/park anything in flight, push, sync the export, and tell Tyler
(a) what is now working, (b) the exact commands to build/run, (c) that any other player must pull *and rebuild*,
(d) what breaks if the server-side fields in §5.1 are missing.

Short version of the current state: **the menus are in good shape**; the remaining work is one server passthrough,
a music widget, controller mapping, and three engine-research items (skulls, new-map launch indexes, post-game names).
