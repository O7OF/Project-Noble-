# Project Noble — to-do list

Single source of truth for open work. `[x]` = done (with the commit that did it), `[~]` = in progress, `[ ]` = open.
The chronological log stays in `docs/HANDOFF.md`; this file is just the list.

Last updated: 2026-10-31, after campaign co-op (`0b83b1c`), lobby identity (`b8fb2ad` / `663cae2`), the scoreboard
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
- [x] @claude 2026-10-30 22:34 - **Server browser polish** — display name in the details panel, vanilla `.map` names no longer flagged as mods, MOD badge - commit f3cbbf6
- [ ] **Scoreboard action bar** — ping column, Report, Vote to Kick, End Round, End Match (host only). Next for @claude once @deepseek's block no longer lists `src/ui/PauseMenu.cpp`
- [ ] **Post-game names** — remote players show `Player2`/`Player3` (needs engine player-record name path)
- [ ] **Skull collection gating** — lock until found, "All Skulls" toggle when 13/13
- [ ] **Readability pass** — run the capture harness over all 14 screens and fix remaining overlaps

## C. Engine features

- [~] @deepseek 2026-10-31 10:55 — **Third-person toggle does NOT work yet.** `CameraThirdPerson` MinHooks
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
