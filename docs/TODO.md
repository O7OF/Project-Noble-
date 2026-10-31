# Project Noble — to-do list

Single source of truth for open work. `[x]` = done (with the commit that did it), `[~]` = in progress, `[ ]` = open.
The chronological log stays in `docs/HANDOFF.md`; this file is just the list.

Last updated: 2026-10-30, after campaign co-op (`0b83b1c`), lobby identity (`b8fb2ad` / `663cae2`), the scoreboard
overlay (`7cef78c`) and the third-person camera (`97989d1`).

**This file is the claim board.** Before starting an item, change its line to
`- [~] @yourname YYYY-MM-DD HH:MM - what you are doing`, commit and push that single line, then do the work. When done:
`- [x] @yourname ... - commit <hash>`. See `docs/TEAM-WORKFLOW.md`.

## Recently done (for context)

- [x] **Campaign co-op works** — launch block declares the co-op player count at `0xE8`; 2- and 3-player runs verified (`0b83b1c`)
- [x] **Forge online works** (relay heartbeat + LAN routing + LobbyPatches sequencing)
- [x] **Backend 1.3.0** — `/api/health`, `/api/relay/stats`, 25 s relay heartbeat, 90 s sessions (`4a520d1`, `df33af7`)
- [x] **Lobby identity pass-through** — rank / emblem / nameplate / team survive registration, heartbeat and roster (`b8fb2ad`, `663cae2`)
- [x] **Scoreboard overlay** — Noble panel over Reach's scoreboard, roster + team + rank (`7cef78c`); live K/D columns still missing
- [x] **RR >4-player reference captured** — `docs/player-count-scaling.md`

---



## NIGHT SHIFT — read this, both agents (Tyler away until morning)

Tyler is stepping away for the night and will have **little to no interaction** until morning. Both agents keep working.
Do not stop to ask; the repo is the coordination channel and this list is the queue.

**Operating loop, repeat until the queue is empty**

1. `git pull`. Check `docs/AGENT-STATUS.md` (who is active, which files) and `docs/TODO.md` (`[~]` lines).
2. Pick the **first unclaimed item in the queue order below** that does not depend on an unfinished item. If your own
   item is blocked (hardware, another agent's file, missing evidence), leave it `[~]` with `needs:` and take the next one.
3. Claim it (TODO line `[~] @you <ET time> - what`), update your AGENT-STATUS block (item + files), commit and push.
4. Work in a timebox of roughly 45-60 minutes. If it is not converging, write down what was tried in the handoff and move
   on - a half-finished item with notes beats a stalled night.
5. Verify (build 0/0; capture for UI; two-process run for netcode; `node --check` + endpoint for server), add the handoff
   entry (with `Self-review:`), mark `[x] @you ... - commit <hash>`, set AGENT-STATUS back to idle, commit and push.
6. **Review the other agent's latest landed commit** using `docs/TEAM-WORKFLOW.md` (diff, hash+handoff, 0/0 rebuild,
   evidence inspected). Write `REVIEW OK` or `REVIEW: re-opened` in a handoff entry.
7. Refresh the Downloads mirrors and, if docs changed materially, rebuild the Claude package.
8. Go to 1.

**Stop conditions (only these)**

- The whole queue is done, or every remaining item is blocked; then write a `## 0.` handoff entry titled
  "Night shift summary" listing what landed, what is blocked and why, and leave everything clean.
- Two consecutive failed attempts at the same item (record both attempts and move on).
- A change would require Tyler's decision (design choice, deleting his data, spending money) - record it as
  `BLOCKED (Tyler): <question>` in the TODO line and pick another item.

**Tyler's standing rules still apply:** always update the handoff, server changes only in `ServerTestDeploy`, close
`noble-client.exe` when done, never force-push, never commit build output or public IPs, leave the tree clean.

**Queue order for tonight**

1. @deepseek - third-person camera matrix + tuning (in progress)
2. @claude - scoreboard action bar + full cover of the engine panel
3. @deepseek - campaign co-op verification (skulls/difficulty, disconnect, respawn)
4. @claude - emblem backdrop + symbol picker
5. @deepseek - scoreboard live K/D API (engine player-record hook)
6. @claude - controller presets + custom mapping
7. @deepseek - campaign CONTINUE checkpoint (may need research; timebox it)
8. @claude - readability pass (14 screens)
9. @deepseek - vote-to-kick transport + server endpoint
10. @claude - server browser follow-ups
11. @deepseek - invites mode/map
12. @claude - post-game names UI (needs @deepseek's name path; swap if blocked)
13. @deepseek - 4+ players (timebox; RR reference in `docs/player-count-scaling.md`)
14. @deepseek - armory spotlight / chamber lighting (data-first, HREK lighting or engine light injection)
15. @claude - docs/screenshots upkeep, then help with the review queue
16. @deepseek - host/admin engine actions (kick/ban/end round/restart/shuffle) with @claude's panel
17. Either - housekeeping (CI artifact, handoff dedupe, stash) if a build/verify window frees up

## Ownership plan (who is doing what)

Agents may still claim anything unowned, but this is the intended split so we do not collide. **Dependencies are stated
explicitly** - a "needs" item must land before the dependent one can be verified.

### @deepseek (engine / netcode / server)

| order | item | notes |
|---|---|---|
| 1 (now) | **Third-person camera** | experiment matrix on `haloreach+0x48A910` (`-third-person-mode 0/1` vs baseline), then the follow hook `+0x1DCADC`, settings row, F5 rebind, host/mode gate |
| 2 | **Campaign co-op verification** | skulls + difficulty parity, host-disconnect behaviour, death/respawn/checkpoint semantics (two-process tests with `-data-dir`) |
| 3 | **Campaign CONTINUE → last checkpoint** | locate the checkpoint block in save data, pass it to the relaunched process |
| 4 | **Scoreboard live data** | engine player-record hook (0x490-stride objects / results record), expose K/D/score to the UI through a small API; **@claude owns the drawing** |
| 5 | **Vote-to-kick transport + server** | `ServerTestDeploy` endpoint + client kick message; **@claude draws the button/modal** |
| 6 | **Invites carry mode/map** | `server.js` invite payload + `SocialClient::SendInviteAsync` + toast text |
| 7 | **4+ players (later)** | RR `capacity`/`coopx` path in `docs/player-count-scaling.md`; Firefight first |
| 8 | **Seamless transitions (research)** | single-process map switch; may stay research |
| 9 | **Housekeeping** | GitHub Actions artifact, handoff dedupe, `stash@{0}` drop (with Tyler's ok) |

### @claude (menus / UI / docs)

| order | item | needs |
|---|---|---|
| 1 (now) | **Scoreboard action bar** | ping column, Report modal, Vote to Kick button, End Round / End Match host-only. UI + client-side actions now; wire to my endpoints when they land. **Cover the engine's scoreboard fully** (opaque panel over its layout) |
| 2 | **Emblem backdrop + symbol picker** | rows, storage, composition (`FrontEndMenu.cpp:861`, `:3660`); ~30 % done |
| 3 | **Controller presets + custom mapping** | 6 presets, `SettingsData`, existing diagram; mapping code lives in `ReachHost.cpp` - claim it |
| 4 | **Readability pass** | capture all 14 screens, fix overlaps; commit before/after captures |
| 5 | **Server browser follow-ups** | live-server rows, LAN/own-host rows, hover tooltip, non-1080p layouts |
| 6 | **Post-game names (UI half)** | display real names once I expose the engine player-record name path |
| 7 | **Docs/screenshots upkeep** | keep `PATHS.md`/screenshots current when UI changes |

### Shared rules for dependencies

- If an item needs the other agent's work, leave the TODO line `[~]` with `needs: @other` until the dependency lands, then verify and close it.
- Never edit `PauseMenu.cpp` (Claude's) or `CameraThirdPerson.*` (mine) without a claim change first.
- Review pairings: UI work is reviewed by @deepseek (diff + build + screenshot inspected); engine/netcode/server is reviewed by @claude (diff + build + logs; if it cannot run it, it says so and the item stays `[~]`).

## A. Campaign co-op follow-ups

- [x] @deepseek 2026-10-30 23:10 - **Skulls + difficulty parity** — the guest launch args already carry `-difficulty`/`-skulls` from the host's launch data; verified with a two-process campaign run at Legendary + skull 0x1: both logs show `Difficulty: 3 | Skulls: 0x1`, both in game, zero GameResults over 170 s. The in-game effect of an individual skull is not visually confirmed (only the data path)
- [ ] **CONTINUE → resume last checkpoint** — `kContinueEntry` still restarts the mission. Needs the save/checkpoint block located, then passed to the new process. *Biggest campaign item.*
- [ ] **Co-op death/respawn + checkpoint behaviour** — guest downed, respawn at host checkpoint, mission-fail conditions
- [x] @deepseek 2026-10-30 23:18 - **Host disconnect handling** — the guest used to freeze in the mission. Now `NetworkTransport::SecondsSinceRemotePacket()` feeds a CONNECTION LOST panel (8 s without host packets) with a Return to menu button → `Relaunch::HandOff`; verified by killing the host mid-campaign and capturing the panel (`docs/screenshots/connection-lost.png`). Gap: the engine's own PLAYERS/STARTING GAME strip still shows dimmed behind it
- [ ] **Rejoin mid-mission** — reconnect to a running campaign session
- [ ] **4+ players** — RR's `capacity`/`coopx` path documented in `docs/player-count-scaling.md`; Firefight first

## B. Menus / UI

- [x] @deepseek 2026-10-31 01:20 - **Workshop mod required: Download & Join** — a lobby host that picked a Steam Workshop
  mod advertises its published-file id (master server + LAN beacon); a joining player who does not have it gets the
  "The lobby you are trying to join currently has a mod installed from the Workshop that you do not have." popup with
  **DOWNLOAD & JOIN** / **CANCEL**. Download & Join subscribes through Steam (dynamic `steam_api64.dll`, with the
  Workshop page as fallback), polls the Workshop folder and continues the join automatically. Needs the
  `ServerTestDeploy` `mod_id`/`mod_title` fields. Files: `FrontEndMenu.*`, `ServerBrowser.*`, `ModCatalog.*`,
  `ModDownloadPrompt.*`, `SteamWorkshop.*`, `MasterServerClient.*`, `SessionManager.*`, `main.cpp`, `CMakeLists.txt`.
  Client commit `ff9cc79`; backend `ServerTestDeploy` commit `dfb2494` (**not pushed/deployed** in this session —
  Render must redeploy for live listings to carry `mod_id`). Screenshot: `docs/screenshots/mod-required-prompt.png`.
  Gap: no two-machine lobby join test yet.
- [x] **Scoreboard overlay** — done (`7cef78c`); gap: live K/D/score columns need the engine player-record hook
- [x] @claude 2026-10-30 23:22 - **Emblem backdrop + symbol picker** — **reviewed by @deepseek: OK** (capture shows the 105-background grid, equipped marker, details panel; build 0/0 on the merged tree). Real MCC backgrounds, composition, and the shown emblem now matches the picked one - commit 66e64a0. Follow-ups: other players don't see the background (needs: @deepseek server field), emblem colours, check nameplates for the same id bug
- [ ] **Armory lighting / spotlight** — unlit black chamber. The Armory scene in `mainmenu_all.map` has only faint volumetric
  haze (`HANDOFF.md` ~4922); the model sits in a void. Options, cheapest first: (a) add a real **spotlight** by editing the
  menumap's lighting (HREK: `tags\levels\ui\mainmenu\mainmenu_000.scenario_structure_lighting_info` + lightmap, then
  rebuild the menumap), (b) inject an engine light near the armory pedestal from the host (needs the light-object API), or
  (c) a Noble-owned rim/key light pass in the preview renderer if the engine exposure can be read. Keep the per-slot cameras
  working (they are). Owner: @deepseek (engine/data), can be split if the UI side needs a toggle
- [~] @claude 2026-10-30 23:28 - **Controller presets + custom mapping** — already built (`PadLayouts.cpp`, BUTTON BINDINGS, `ReachHost.cpp:1395`); stale row removed. **needs: Tyler** a hands-on test with a pad: pick a layout, Apply, play; rebind one action
- [x] @claude 2026-10-30 22:34 - **Server browser polish** — display name in the details panel, vanilla `.map` names no longer flagged as mods, MOD badge - commit f3cbbf6 — **reviewed by @deepseek: OK** (diff, 0/0 build, capture inspected)
- [~] @claude 2026-10-30 23:17 - **Scoreboard action bar + in-game scoreboard (spec 4.16)** — landed in commit 60f374b: engine
  scoreboard no longer opens, row select, Report (saved locally), ping column, buttons in place. **reviewed by @deepseek: OK** (merged build 0/0, capture on the merged tree). **needs: @deepseek** real ping
  values (the server's are placeholders), vote-to-kick transport, end round / end match actions, report endpoint if wanted.
  Then @claude wires the greyed buttons. Mute icons wait for chat/voice. Controller Back still opens the engine's scoreboard
- [x] @deepseek 2026-10-30 23:52 - **Emblem picker tiles: background art showed a black rectangle behind the symbol** — fixed: MCC's emblem art is BC1 white-on-black with no alpha, so the square was drawn as-is. Added a CPU BC1/BC3/RGBA decode at load time that turns luminance into alpha and uploads RGBA (`UiArt::LoadPackImageKeyed`, used by `EmblemByName`), so only the shape shows and the tint colours it. Verified with armory captures of both grids (`docs/screenshots/emblem-symbols-keyed.png`, `emblem-backgrounds-keyed.png`). Still open: real emblem colour picker (backgrounds are steel-tinted), `Nameplate(id)` position-vs-id check. Original report: — Tyler's screenshots of the Emblem and Emblem Background grids show the whole square of the background texture (black field included) instead of only its shape, so tiles look like white symbols on black boxes and the equipped previews look blocky. The MCC background/symbol art is black-on-transparent-in-theory; composite it with a black colour key (or multiply/screen against the tile) so black becomes transparent and only the shape tints. Apply the same fix to the large equipped preview, the nameplate and the roster rows. Owner: @claude (UI); filed 2026-10-30 23:40 ET.
- [ ] **Emblem background replication (other players see it)** — **data landed** (`b1656b4` server, client `MasterServerClient`/`FrontEndMenu`): `emblem_background_id` is registered, joined, stored and returned as `emblemBackgroundId`, and exposed as `host_emblem_background_id`. **needs: @claude** draw it in `PlayerRow` and the lobby rows. Roster entries previously only carried `emblemId`; add `emblem_background_id`/`emblemBackgroundId` to the server (registration, heartbeat, roster, lobby join) and to `MasterServerClient`, then draw it in `PlayerRow`/lobby rows. Split: server+client data @deepseek, drawing @claude. Related: `Nameplate(int id)` may have the same list-position-vs-id bug; the emblem id lookup was already removed.
- [ ] **Post-game names** — remote players show `Player2`/`Player3` (needs engine player-record name path)
- [ ] **Skull collection gating** — lock until found, "All Skulls" toggle when 13/13
- [x] @claude 2026-10-30 23:36 - **Readability pass** — 20 views captured at 1080p, no overlaps; fixed low-contrast text on Skulls and the setup-screen status line - commit b0b7cac


### Filed from @claude's status report (2026-10-31 00:00 ET)

- [ ] **Controller Back still opens Reach's old scoreboard** — our panel replaces the keyboard scoreboard key, but the pad's
  Back/Select button still reaches the engine. Suppress the pad button in the input poll (same place as `boardVk`) and feed
  it to the overlay instead. Owner: @deepseek (ReachHost input poll), UI already handles the panel.
- [ ] **Possible pause-menu bug: Esc to resume sometimes reopens the menu** — seen once by @claude, not reproduced. Repro:
  open the pause menu in a match, press Esc to resume, check whether it reopens immediately (likely the engine key state and
  our edge detection disagreeing). Owner: @claude (PauseMenu).
- [x] @gemini 2026-10-31 01:36 - **Real ping values & WebSocket Relay fragment reassembly / NAT hole-punching** — commit bfcc6c7 (`ServerTestDeploy` `8f16061`): `NetworkTransport` measures live per-peer RTT from `PING`/`PONG` and UDP packet exchanges, disables `SIO_UDP_CONNRESET`, reassembles fragmented WebSocket relay frames up to 64 KB with 2 ms poll latency, punches same-LAN guest addresses, and syncs `peer_pings` into roster rows via `DoHeartbeat()` / `FetchLobbyStateSync()`.
- [x] @deepseek 2026-10-31 05:00 - **Scoreboard backend actions** - server endpoints live and verified: report (`b645dbf`), vote-to-kick request/status (`655fb4d`), host-only End Round/End Match token-gated (`65ba17a`), plus 128-player session cap + 2 MB JSON body (`48dc168`, `a1196ba`). @claude can wire the three buttons; pad Back and real ping stay open in section C.
- [ ] **Scoreboard backend actions** — `POST /api/reports` (report endpoint; replaces the local-only `reports.jsonl`),
  vote-to-kick request/vote/result endpoint, and host actions for End Round / End Match. Server + transport @deepseek, then
  @claude un-greys the buttons and wires them.
- [ ] **Controller presets need a human test** — logic passes, but Tyler has to pick Southpaw or Bumper Jumper, press Apply,
  play a match, then rebind one button.

## C. Engine features

- [x] @deepseek 2026-10-30 23:05 - **Third-person camera works** (`haloreach+0x48A910` check hooked, F5 toggles,
  `-third-person` starts on) - verified in Firefight with a side-by-side capture (`docs/screenshots/third-person-vs-first.png`):
  same spot, first person vs Spartan from behind. Campaign looks unchanged only because the first minutes are cutscene and
  vehicle cameras, which ignore the player camera check. Commit pending with the tuning work below.
- [~] @deepseek 2026-10-30 23:20 - **Third-person tuning** - host/mode gate landed (guests keep first person); the follow hook at `haloreach+0x1DCADC` installs but is not called in this camera path (no scaled-distance log), so distance tuning is parked as research; still open: settings row + key rebind

- [ ] **Real solo pause, CHANGE TEAM, vote-to-kick** (transport + roster UI)
- [ ] **Host/admin controls (spec 4.15)** — kick / session-ban / permanent ban (dedicated), end round, restart round, restart
  match, shuffle teams, end game → back to the lobby with everyone, skip to next rotation entry; actions confirmed and
  announced in chat. Split: engine/session actions @deepseek, the host-only panel @claude
- [ ] **Seamless transitions** — single-process map switch; research-only (RR also relaunches per match)

## D. Server / netcode

- [x] **Echo host identity in the lobby** — `host_rank_index`, `host_emblem_id`, `host_nameplate_id`, `host_team` now
  stored, echoed in `/api/sessions` and roster, guest identity accepted on join (`b8fb2ad`, client `663cae2`)
- [x] @gemini 2026-10-31 01:36 - **Invites carry mode/map** — commit bfcc6c7 (`ServerTestDeploy` `8f16061`): `SocialClient::SendInviteAsync` and `/api/invites/send` carry `mode`, `map`, and `sessionId` (case-insensitive username matching), and `DrawInviteToast` displays `"is inviting you to a <Mode> match"` and forwards `-session-id` on JOIN.
- [ ] **Vote-to-kick server endpoint + client UI** (pairs with C)


## F. Design-spec backlog (from `docs/menu-redesign-spec.md`, not scheduled yet)

These come from the spec's §4.10-§4.15 and §6 order of work. They are **not claimed**; they enter the queue as the core
items above close. Keep the spec open when building any screen so the details land with it.

- **Armory leftovers (§4.6/§6.1)** — thumbnails, rank / cR locks, stances, randomise buttons, identity name row; per-character
  looks + LOOKS priority needs a research spike (one mod with ODST/Marine players)
- **Input (§4.10)** — mouse acceleration + smoothing, separate invert-flight, sensitivity hotkeys, release-cursor key, second
  key per action; controller: aim Modern/Classic, aim acceleration, stick layout, look dead zone, button presets
- **Game options engine research (§4.10)** — vehicle and third-person FOV, centred/crosshair size, hide HUD/visor, label fade,
  viewmodel offsets; advanced graphics (filtering, AA, shadows, motion blur, draw distance, LOD, decals, water, presets)
- **Audio / language / communication** — mute in background, per-song menu music, volume meters, language + subtitle files,
  content filters (chat/name/slur/custom words), game chat on/off, voice chat (devices, push-to-talk), Discord rich presence
- **Online services / folders / mods** — master-server list + query batch size, player ID + COPY, MCC/cache/download folder
  rows with disk-space bars, mods manager (ON/OFF, folder, delete, totals)
- **Server browser extras (§4.5/§4.10)** — favourites (star), search by server/host/map/mode/mod, mode chips with counts,
  filters (hide private, ping limit, not-started), VIEW PLAYERS, clickable mod badge → mod list, "Hosted by", cosmetic mods
- **Pickers** — favourite maps/game types + FAVORITES tab, OPEN FOLDER / REFRESH, "+N variants", search
- **Lobby / rotation / voting (§4.14)** — playlist editor (map + game type + options per entry), rotation stored in server
  config, current/next map in browser, after-game vote between 2-3 options ("keep playing"), ties to host; required-mods
  check before joining a modded server
- **Game options editor (§4.4)** — variant read/write first (`SAVE AS VARIANT`, search every setting, "Unchanged" values)
- **Theater (§4.9)** — recent games, save film, rename, delete, export MP4; needs engine film recording (keep entry disabled
  with "Coming later")
- **"Continue last game" fast path** — covered by the checkpoint item in A (CONTINUE currently restarts the mission)

- [ ] **Regression: menu -> start match -> return must not throw a Windows error** — the clean-shutdown fix is
  `982e32a` (2026-10-28): `BlamEngine::Shutdown()` deliberately does **not** call `m_fnShutdown`/vtable[0] while
  haloreach.dll's engine thread runs, then `main.cpp` calls `blamEngine->Shutdown()` and `TerminateProcess`. A
  `git log -S"blamEngine->Shutdown();"` search only finds the old `8213116` (Oct 26) because the call itself never moved —
  the fix was removing the destructor call, so that search is a false alarm. Verify by behaviour: launch a match from the
  menu, return to the menu, and confirm no `ntdll` dialog and `Network and host services shut down; exiting process cleanly.`
  in the log. Repeat for a Direct Connect guest.



## H. Controller UX and settings (Tyler, 2026-10-31 00:10) - @claude

- [x] @claude 2026-10-31 00:40 - commit dc8731b - **Settings tabs on D-pad / left stick**, not LB/RB; when a value row is focused the stick/D-pad adjusts the value, not the tab
- [x] @claude 2026-10-31 00:40 - commit dc8731b - **Apply/Discard controller prompts**: Y = Apply changes, X = Discard changes (icons + key equivalents), replacing the text-only unsaved box
- [x] @deepseek 2026-10-31 01:05 - **Controller input now reaches ImGui**: `src/ui/GamepadUi.cpp` reads XInput each frame
  and feeds `io.AddKeyEvent` for GamepadDpad*, GamepadFaceA/B/X/Y, Start/Back, LB/RB, LT/RT and both stick directions before
  `ImGui::NewFrame` (called from PauseMenu, FrontEndMenu and UIManager), and sets `NavEnableGamepad`. This is the root fix
  behind "presets don't work": SettingsPanel's existing gamepad checks were dead. @claude now wires the visible Y=Apply /
  X=Discard prompts and the preset/diagram row; Tyler retests with a pad.
- [x] @claude 2026-10-31 00:40 - commit dc8731b (real cause: the row dropped its step, for every device) - **Button layout row must be changeable with a controller** (D-pad/stick left-right does nothing today) and the layout diagram must update per preset
- [x] @claude 2026-10-31 00:40 - commit dc8731b - **One "Controller preset" item** (stick + buttons together); remove the separate stick-layout row; presets from `docs/menu-redesign-spec.md` / Halopedia (Default, Southpaw, Boxer, Bumper Jumper, Recon, ...)
- [~] @claude 2026-10-31 - commits dc8731b, 83e05e9; paddles work as keyboard/mouse keys sent by a tool like Steam Input; stick movement axes only via presets; **needs: Tyler** real pad test - **Remap every input**: A/B/X/Y, bumpers, triggers, both sticks (directions + click), D-pad, Start/Back, paddles 1-6; add missing actions: crouch, toggle zoom (default on, still exposed), team chat, scoreboard, push-to-talk (future), etc.
- [~] @claude 2026-10-31 - commit eb757b8, 2.00x = Reach stock, measured; **open:** raising the full-push maximum needs the engine's turn rate found (research); **needs: Tyler** to feel it in a match - **Look sensitivity baseline**: 19 is the current default; document the scale against Reach's 1-10 and expose horizontal/vertical
- [ ] **(Tyler test) Controller presets** - pick Southpaw or Bumper Jumper, Apply, play a match, rebind one button

- [x] @claude 2026-10-31 03:53 - commits f0fda8b, ec531b4; captures docs/screenshots/lobby-after-*.png and roster-*.png; no two-PC run yet - **Pre-game / Host Game lobby feels off - bring it flush with the other screens** (Tyler, 2026-10-31): apply the shared layout rules (header/rule, margins, panel widths, row height, button sizes, focus colours, controller prompt bar) used by Settings and the Armory; the lobby currently reads as a different design. Cover host and guest views, ready state, slot list, team columns, map/mode header and the start/back prompts. Before/after captures. - @claude

- [x] @deepseek 2026-10-31 04:25 - **Elite pipeline + colour system** (night block 1) - commit e7ab6c9: `docs/reference/elite-slots.md` (314 rrpmcg components, elite ranks) and `docs/reference/elite-m10.md` (cache sections, 79 elite shader + 117 bitmap paths, 57/60 sampled bitmaps already stock vs 51/60 shaders pack-only - the rank look is in the shaders). Next: shader colour parameters.

- [~] @claude 2026-10-31 03:53 - lobby ec531b4, scoreboard 4792746; captures docs/screenshots/roster-*.png, scoreboard-*.png - **Lobby/scoreboard: split teams into two columns** (Tyler, 2026-10-31): RED on the left, BLUE on the right, each
  column scrolling straight down through its own player list (independent scroll positions, sticky team headers, team
  colour/dot per row, current player pinned in view). Free-for-all falls back to one column; uneven teams leave the shorter
  column empty rather than reordering. Stress test at 64/64 and 100/28. - @claude
- [~] @claude 2026-10-31 03:53 - lobby ec531b4, scoreboard 4792746; **controller page-scroll on the scoreboard open**; needs: @deepseek server roster limits - **Large-lobby roster: scrollable player list up to 128** (Tyler, 2026-10-31): the pre-game lobby and the in-game
  scoreboard must render far more than Reach's 16 rows with scrolling (virtualised rows, sticky header, current player kept
  in view, controller page-scroll, 16/32/64/128 stress fixtures). - @claude
- [ ] **Server roster/limits for 128 entries** (Tyler, 2026-10-31): raise `MASTER_MAX_PLAYERS` and the lobby roster caps,
  make join/heartbeat/roster payloads bounded but 128-safe, and add a synthetic 128-player fixture for the UI tests. - @deepseek
- [ ] **Pad Back: `d27ab84` is a no-op** (@claude review 2026-10-31): line 1430 already strips Back, comment names removed `GamepadUi`. If Reach's scoreboard still opens on pad Back the cause is elsewhere. Remove the line or find the real path. - @deepseek
- [ ] **Lobby READY state** is local only; send it in the roster if it should mean anything. - needs: @deepseek
- [ ] **Friends panel INVITE and Social screen** need a controller path; Y opens Social, whose pad flow is untested. - @claude
- [ ] **Vote to kick is blocked** (@claude review 2026-10-31): server clamps peer ids to 1..64 so real voters fail, votes aren't authenticated, and nothing removes a player when a vote passes. Button stays grey until fixed. - @deepseek
- [ ] **End Round / End Match need the multiplayer end path**: `haloreach+0x7B3EC(-1, 0, 1)` tried twice, timer kept running. Same search as night block 3. Button stays grey until then. - needs: @deepseek
- [ ] **Roster should carry emblem colours** (`emblem_primary_color`, `emblem_secondary_color`, `emblem_background_color`, indexes 0-29) so other players see them. - needs: @deepseek
- [ ] **Scoreboard actions have no controller path**: selecting a player and REPORT need the mouse. - @claude
- [~] @claude 2026-10-31 - live ping wired, commit 4792746; **needs:** a two-process run to see a real value - **Scoreboard real ping**
- [ ] **First-start question** (spec 4.12): "New to Halo: Reach?" with a short setup; `firstStartDone`. - @claude
- [ ] **Crash / issue report button** (spec 4.12, approved): bundle the log with player names removed, open logs folder, settings backup and restore. - unowned
- [ ] **Import/export of key bindings and settings** (spec 4.12). - @claude
- [ ] **Scoreboard mute icons and Mute all** (spec 4.16). Voice API is ready (``VoiceChat::SetPeerMuted``, ``IsPeerTalking``). - @claude
- [ ] **Rotation voting** (spec 4.14): 2-3 options after each game, ties to the host. Waits for rotation. - @deepseek then @claude
- [ ] **Armory left vs right shoulder camera** (handoff guide 5.2): the map has one shoulder camera; needs a lateral camera offset from engine globals. - @deepseek
- [ ] **Career deaths look uncounted**: the card shows 62 kills / 0 deaths over 15 games on the test profile. Check the results parsing. - @claude
- [ ] **Hints and the full-screen panels don't line up**: on Settings and Social the prompt row starts further right than the panel. Cosmetic. - @claude
- [~] @claude 2026-10-31 14:58 - **Push-to-talk rebinding in Voice Chat**: separate rows for key or mouse and for controller.
- [ ] **RESEARCH: real match capacity beyond 16** (Tyler, 2026-10-31): Reach's network session is 16 players; the UI can
  list 128, but an actual 128-player match needs the `capacity`/`coopx`-class engine work in `docs/player-count-scaling.md`
  (RR patch path) plus bandwidth work. Keep 128 as the lobby/browser ceiling for now and file match capacity as research,
  not a promise.

- [x] @claude 2026-10-31 14:55 - **Voice transport phase 2**: capture, playback, Opus, relay `voice` frames, jitter buffer, per-peer mute and volume, push to talk, open mic, Settings (`accbd06`, server `4af063f`). Evidence in the top HANDOFF entry. Still needs a person: voice by ear, a pad button, and a two-PC session (also exercises the new relay sender thread). **reviewed by @deepseek: OK** (merged 0/0, evidence inspected: loopback WAV, 3-process and 2-client logs, jitter tests, settings captures).

- [~] @deepseek 2026-10-31 05:20 - **post-game + credits (night block 3)** follows voice phase 1: -postgame-test timing, multiplayer end path, exit_experience sounds, mission_complete split.

- [ ] **Text chat: team / all / party in lobbies and in-match** (Tyler, 2026-10-31): keys **T = team**, **Y = everyone**,
  **P = party** (rebindable, shown in the binding list; controller gets quick-chat later). Works in the pre-game lobby, the
  in-game lobby and mid-match; channel tabs/prefix, local echo, timestamps, scrollback with the same virtualised list style as
  the 128-player roster. Transport: a `chat` message type on the existing relay (server relays + rate-limits, length cap,
  content-filter hook, never stores by default); per-player text mute alongside the voice mute state. Split: transport/server
  @deepseek, UI + input @claude.

- [ ] **Voice/chat keybinds (keyboard+mouse and controller)** (Tyler, 2026-10-31): bindable actions for **push-to-talk**,
  **open chat**, **team chat**, **all chat** and **party chat**, shown in Settings > Controls and in the binding list with
  defaults T/Y/P for the channels and a pad button for PTT/quick-chat. Must work pre-game, in the lobby and in-match, and the
  chat binds must not fire while a menu owns input. Shared with the voice transport (PTT) and text chat (channels). -
  @claude (UI/binding table) + @deepseek (transport hooks)

- [x] @deepseek 2026-10-31 18:50 - **Text chat relay (server)** - `ServerTestDeploy` `5d46c13` + cleanup `d0ac871`: `chat` JSON frames handled before the role logic, channels team/all/party (party = lobby group for now), 400-char cap, team chat only to same-team guests+host. `tools/chat-relay-test.js`: team 1/1/0, all 1/1/1 = PASS. Client send/receive + T/Y/P binds next.

## I. Armory presentation, audio and data (Tyler, 2026-10-31 00:10)

- [x] @claude 2026-10-31 00:56 - commit dd37730 - **Armory framing**: Spartan/Elite centered under the horizontal rule below APPEARANCE, head no longer above the HALO: REACH line or beside the nameplate
- [x] @claude 2026-10-31 00:56 - commit dd37730 - **Nameplate in the Armory must match the main menu's look**
- [~] @claude 2026-10-31 00:56 - commit dd37730, soft additive UI key light replaces pure black; a real engine/data light stays open for @deepseek - **Armory spotlight**: stop the pure-black chamber (UI gradient/key light first; engine/data light if needed)
- [~] @claude 2026-10-31 00:56 - pans verified by capture and untouched; transition smoothness is the engine's and not measured - **Per-slot camera pan** to the body part being edited (verify + smooth transitions)
- [x] @claude 2026-10-31 00:56 - commit 1561e00, run with `-dev-commendation-tiers`; **needs: Tyler** to judge the art - **Commendation tier check**: temporarily force the first six commendations to steel/bronze/silver/gold/onyx/max to verify art + payout (dev-only, not shipped values)
- [ ] **Campaign completion gating (refined)**: quitting a mission must not award the **completion bonus**, but the
  **performance bonus still pays** (kills/difficulty/time as normal). Server result needs an explicit `mission_complete`
  flag and the credit breakdown split into performance vs completion, so a quit pays only the performance part.
- [ ] **Post-game slot machine** (Reach's credit slot machine; Halopedia reference) - @deepseek
- [ ] **Forge / custom-game defaults / Firefight game options** screens (blocked on variant work)
- [x] @claude 2026-10-31 00:56 - commit f1facf3 - **Menu background art**: shuffle the whole set (repeats today) and prefer the loaded map's art
- [x] @claude 2026-10-31 00:56 - commit ecf8f0e; Halo: CE has no lengths and keeps everything - **Music**: drop tracks <= 45 s from the menu playlist
- [~] @claude 2026-10-31 00:56 - preview on X landed, commit ecf8f0e; **needs: Tyler** to listen. In-game voice packs not verified - **Firefight voice packs**: verify working + Armory preview button (X on pad) that plays a sample line per character

- [ ] **Emblem colours (real three-colour picker)** — backgrounds are fixed steel today; Reach's real picker needs the
  black layers treated as masks. Nameplate audit done and fixed, commit 41c1c91. - @claude
- [~] @claude 2026-10-31 - commit 0662bcf, Reverse Southpaw / Reverse Bumper Jumper added as Noble names (layout + swapped sticks); Reach has none. **Still missing: Reach's real Legacy and Legacy Southpaw stick layouts** (Halopedia shows them only as images) - **Controller preset list completeness** — include Reach's reverse variants (Reverse Southpaw, Reverse Bumper Jumper)
  and every MCC scheme; the diagram must switch with the preset, and the preset must apply on Y without a relaunch. - @claude
- [ ] **Game-option surfaces, split up**: Forge map options (budget/permissions), custom-game defaults (a default variant
  shipped with Noble), Firefight options (waves/lives/skulls/time), and per-mode match settings from the variant plan. - @deepseek + @claude UI
- [ ] **Design-spec + modding-kit audit** — sweep `menu-redesign-spec.md` §4.10/§4.11/§4.16 and the HREK docs for items not
  yet in this list (voice preview, slot machine, theater entry, forge options were all in the spec) and file anything still
  missing. Repeat at the end of the night. - either
- [ ] **Theater setup** — film recording/playback in the engine is the prerequisite; the menu entry stays disabled with
  "Coming later" until then (already in F, restated here because Tyler asked for it explicitly). - research

- [~] @deepseek 2026-10-31 01:15 - **End-screen credit/XP sound from Reach**: HREK ships the exact exit-experience set at
  `tags\sound\game_sfx\ui\omaha_ui\exit_experience\` - `counter_loop` (the ticking count-up), `totals`, `slot_machine`,
  `rank_up`, `game_completion`, `commendations`, `daily_challenges`, `heat_bonus`, `hopper_bonus`, `invasion_bonus`,
  `time_spent`, `winning_bonus`, `achievements`. Extract with HREK `tool.exe` (sound tag -> sound file), add to Noble's data,
  and play `counter_loop` while the CR/XP counter runs with `totals` at the end (CR and XP are the same currency).
  `slot_machine` plays under the counter at about 70% of the counter's volume (Tyler's rule: a tad lower). Until the assets
  are extracted, use the current audio cue.

- [~] @deepseek 2026-10-31 18:58 - **Post-game screen shows Reach's old scoreboard for multiplayer** (panel verified via `-postgame-test` at 900 frames, capture `docs/screenshots/postgame-clean.png`; remaining: raise it on the real multiplayer end path) - `ShowGameOver()` is only called from
  `HostSlot6_GameResults` (launch state >= 11); nothing else raises it. Added a `-postgame-test` harness (forces the panel)
  but the first capture still showed the map/loading background, so the trigger fires too early or the panel is skipped while
  loading. Next: trigger after the scene is ready (like `-scoreboard-demo`'s 900 frames), capture, then find the real
  multiplayer end path and raise the panel there. - @deepseek

## G. Match settings, social, voice (plan: `docs/plans/match-settings-and-social-plan.md`)

- [ ] **Pause-menu Controls page** (live remap already applies every frame; this is the UI to change it mid-match) - @claude
- [ ] **Variant picker** (shipped `game_variants/*.bin` + stock Reach variants; lobby already transports `variant`) - @claude + list from @deepseek
- [ ] **Lobby player profile card** (rank, emblem, Spartan render, armory %, challenges, commendations; ADD FRIEND row disabled until the friends API exists) - @claude
- [ ] **Profile summary fields + privacy flags on the server** (roster profile object, `GET /api/sessions/:id/profile/:peerId`) - @deepseek
- [ ] **Friends subsystem** (requests/accept/list/presence, then join-from-friend + party invites) - @deepseek + @claude
- [ ] **Variant viewer / RE of `c_game_variant`** (field map from the already-decoded struct) - @deepseek
- [ ] **Match end -> lobby or playlist rotation** (session survives results; host continue / guest wait; rotation array + next map) - @deepseek + @claude results UI
- [x] **Voice chat transport**: done, see the 2026-10-31 @claude line. **Proximity voice** still open.
- [ ] **Voice UI** (devices, PTT mode, per-player volume, scoreboard mute icons) - @claude after transport
- [ ] **Variant editor (subset) + credit eligibility flag** and **server-side credit validation** - @deepseek
- [ ] Real Reach settings only: the pasted Halo Online list is not Reach; take the actual set from `game_variant_settings` tags + MegaloEdit, not the list

## E. Housekeeping

- [x] **Bump `VERSION_PATCH`** — now `0.8.4` / "0.8.4 (Co-op, Third Person & Menu Cleanup)" (`src/common/Version.hpp`)
- [x] @deepseek 2026-10-31 01:34 - **GitHub Actions build artifact** — one-click exe so both PCs always run the same binary — commit 7f19dbc
- [ ] **Dedupe `docs/HANDOFF.md`** — O7's `dc349a5` appended ~27 identical entries; 245+ entries now
- [x] **Remove `patch_blam.js`** — O7's scratch rewrite script, superseded by `64dd74f`
- [ ] **Drop `stash@{0}`** once Tyler agrees (its useful half is already committed as `32cd0f1`)
