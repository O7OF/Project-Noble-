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
14. @claude - docs/screenshots upkeep, then help with the review queue
15. Either - housekeeping (CI artifact, handoff dedupe, stash) if a build/verify window frees up

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
- [ ] **Armory lighting** — unlit black chamber; only route left is data (lighting/material in `mainmenu_all.map`)
- [ ] **Controller presets + custom mapping** — biggest pure-UI item (6 presets, `SettingsData` storage, diagram already drawn)
- [x] @claude 2026-10-30 22:34 - **Server browser polish** — display name in the details panel, vanilla `.map` names no longer flagged as mods, MOD badge - commit f3cbbf6 — **reviewed by @deepseek: OK** (diff, 0/0 build, capture inspected)
- [ ] **Scoreboard action bar** — ping column, Report, Vote to Kick, End Round, End Match (host only). Next for @claude once @deepseek's block no longer lists `src/ui/PauseMenu.cpp` — free to claim, @deepseek released `PauseMenu.cpp`
- [ ] **Post-game names** — remote players show `Player2`/`Player3` (needs engine player-record name path)
- [ ] **Skull collection gating** — lock until found, "All Skulls" toggle when 13/13
- [ ] **Readability pass** — run the capture harness over all 14 screens and fix remaining overlaps

## C. Engine features

- [~] @deepseek 2026-10-30 22:17 — **Third-person toggle does NOT work yet.** `CameraThirdPerson` MinHooks
  `haloreach+0x48A910` and answers the check with `true`, and the detour does fire, but a clean gameplay capture still shows
  first person. Earlier "works" claim was a cutscene camera - corrected. Next: determine the check's real semantics (try
  returning `false`, and/or returning `original(...)` combined) and whether the engine needs the follow hook at
  `haloreach+0x1DCADC` too. Then distance/shoulder tuning, settings row, key rebinding, host/mode gate.
- [ ] **Real solo pause, CHANGE TEAM, vote-to-kick** (transport + roster UI)
- [ ] **Seamless transitions** — single-process map switch; research-only (RR also relaunches per match)

## D. Server / netcode

- [x] **Echo host identity in the lobby** — `host_rank_index`, `host_emblem_id`, `host_nameplate_id`, `host_team` now
  stored, echoed in `/api/sessions` and roster, guest identity accepted on join (`b8fb2ad`, client `663cae2`)
- [ ] **Invites carry mode/map** — so the toast can say "...inviting you to a Firefight match"
- [ ] **Vote-to-kick server endpoint + client UI** (pairs with C)

## E. Housekeeping

- [x] **Bump `VERSION_PATCH`** — now `0.8.4` / "0.8.4 (Co-op, Third Person & Menu Cleanup)" (`src/common/Version.hpp`)
- [ ] **GitHub Actions build artifact** — one-click exe so both PCs always run the same binary
- [ ] **Dedupe `docs/HANDOFF.md`** — O7's `dc349a5` appended ~27 identical entries; 245+ entries now
- [x] **Remove `patch_blam.js`** — O7's scratch rewrite script, superseded by `64dd74f`
- [ ] **Drop `stash@{0}`** once Tyler agrees (its useful half is already committed as `32cd0f1`)
