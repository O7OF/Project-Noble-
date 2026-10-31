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

- [ ] **Verify skulls + difficulty parity** — host launch data carries skulls; confirm the guest actually receives them in-game
- [ ] **CONTINUE → resume last checkpoint** — `kContinueEntry` still restarts the mission. Needs the save/checkpoint block located, then passed to the new process. *Biggest campaign item.*
- [ ] **Co-op death/respawn + checkpoint behaviour** — guest downed, respawn at host checkpoint, mission-fail conditions
- [ ] **Host disconnect handling** — what the guest sees and whether it returns to the menu cleanly
- [ ] **Rejoin mid-mission** — reconnect to a running campaign session
- [ ] **4+ players** — RR's `capacity`/`coopx` path documented in `docs/player-count-scaling.md`; Firefight first

## B. Menus / UI

- [x] **Scoreboard overlay** — done (`7cef78c`); gaps: live K/D/score columns need the engine player-record hook, and the
  engine's own scoreboard is still visible behind/around the Noble panel (see `docs/screenshots/third-person.png`) — panel
  needs to cover the engine layout fully or the engine scoreboard must be hidden while ours is up
- [ ] **Emblem backdrop + symbol picker** — ~30 % done; two identity rows, storage and composition remain (`FrontEndMenu.cpp:861`, `:3660`)
- [ ] **Armory lighting / spotlight** — unlit black chamber. The Armory scene in `mainmenu_all.map` has only faint volumetric
  haze (`HANDOFF.md` ~4922); the model sits in a void. Options, cheapest first: (a) add a real **spotlight** by editing the
  menumap's lighting (HREK: `tags\levels\ui\mainmenu\mainmenu_000.scenario_structure_lighting_info` + lightmap, then
  rebuild the menumap), (b) inject an engine light near the armory pedestal from the host (needs the light-object API), or
  (c) a Noble-owned rim/key light pass in the preview renderer if the engine exposure can be read. Keep the per-slot cameras
  working (they are). Owner: @deepseek (engine/data), can be split if the UI side needs a toggle
- [ ] **Controller presets + custom mapping** — biggest pure-UI item (6 presets, `SettingsData` storage, diagram already drawn)
- [x] @claude 2026-10-30 22:34 - **Server browser polish** — display name in the details panel, vanilla `.map` names no longer flagged as mods, MOD badge - commit f3cbbf6 — **reviewed by @deepseek: OK** (diff, 0/0 build, capture inspected)
- [~] @claude 2026-10-30 22:46 - **Scoreboard action bar + in-game scoreboard (spec 4.16)** — ping column, Report, Vote to Kick, End Round, End Match
  (host only), per-row mute icon and Mute-all (when chat/voice exists), rank icon/emblem/tag already in place; the Noble
  panel must fully cover the engine's own scoreboard. Spec: `menu-redesign-spec.md` §4.16. **@claude**, `PauseMenu.cpp`
  released
- [ ] **Post-game names** — remote players show `Player2`/`Player3` (needs engine player-record name path)
- [ ] **Skull collection gating** — lock until found, "All Skulls" toggle when 13/13
- [ ] **Readability pass** — run the capture harness over all 14 screens and fix remaining overlaps

## C. Engine features

- [x] @deepseek 2026-10-30 23:05 - **Third-person camera works** (`haloreach+0x48A910` check hooked, F5 toggles,
  `-third-person` starts on) - verified in Firefight with a side-by-side capture (`docs/screenshots/third-person-vs-first.png`):
  same spot, first person vs Spartan from behind. Campaign looks unchanged only because the first minutes are cutscene and
  vehicle cameras, which ignore the player camera check. Commit pending with the tuning work below.
- [~] @deepseek 2026-10-30 23:05 - **Third-person tuning** - pull the camera back (follow hook `haloreach+0x1DCADC`:
  call original, scale `[r9+0x28]`, optional shoulder sign at `[r9+0x14]`), settings row + key rebind, host/mode gate

- [ ] **Real solo pause, CHANGE TEAM, vote-to-kick** (transport + roster UI)
- [ ] **Host/admin controls (spec 4.15)** — kick / session-ban / permanent ban (dedicated), end round, restart round, restart
  match, shuffle teams, end game → back to the lobby with everyone, skip to next rotation entry; actions confirmed and
  announced in chat. Split: engine/session actions @deepseek, the host-only panel @claude
- [ ] **Seamless transitions** — single-process map switch; research-only (RR also relaunches per match)

## D. Server / netcode

- [x] **Echo host identity in the lobby** — `host_rank_index`, `host_emblem_id`, `host_nameplate_id`, `host_team` now
  stored, echoed in `/api/sessions` and roster, guest identity accepted on join (`b8fb2ad`, client `663cae2`)
- [ ] **Invites carry mode/map** — so the toast can say "...inviting you to a Firefight match"
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

## E. Housekeeping

- [x] **Bump `VERSION_PATCH`** — now `0.8.4` / "0.8.4 (Co-op, Third Person & Menu Cleanup)" (`src/common/Version.hpp`)
- [ ] **GitHub Actions build artifact** — one-click exe so both PCs always run the same binary
- [ ] **Dedupe `docs/HANDOFF.md`** — O7's `dc349a5` appended ~27 identical entries; 245+ entries now
- [x] **Remove `patch_blam.js`** — O7's scratch rewrite script, superseded by `64dd74f`
- [ ] **Drop `stash@{0}`** once Tyler agrees (its useful half is already committed as `32cd0f1`)
