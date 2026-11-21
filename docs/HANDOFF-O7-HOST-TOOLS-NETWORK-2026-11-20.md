# Handoff for O7: host tools network changes (v0.20.33, 2026-11-20; updated 2026-11-21 01:45)

Tyler and Claude built the host tools (Phase 1-2 and the start of Phase 3 of
`claude/TODO-HOST-TOOLS-AND-REMAINING-2026-11-20.md`). They touch your networking
code. This lists every change so you can review it, and the parts that need your call.

## New module

`src/network/HostModeration.{hpp,cpp}`: kick, ban (timed / permanent), host mute,
word filter, server-name filter, local text mute, anti-AFK, co-hosts and lobby access
(lock / invite only / password). The host enforces everything. Saved files in
`data\userdata\`: `host_bans.json` (bans, filter, anti-AFK, lobby password),
`host_access.json` (lobby access mode, admitted players, invites; kept across a match
relaunch), `host_notice.txt` (removal message from the match process to the menu).

## New control messages (NBCJ JSON, all via `HandleControlJson`)

| type | from -> to | what it does |
|---|---|---|
| `host_remove {target, kind, reason, minutes}` | host -> one guest | kick / ban / lobby refused. Guest leaves and shows the message. `kind`: kick, ban, locked, invite, password, wrong_password |
| `host_pause {paused}` | host -> guests | pause / resume for everyone (campaign). Resent every 1.5 s; guest times out after 6 s |
| `host_rules {afk, cohosts, filter, words, muted}` | host -> guests, every 5 s | anti-AFK limit, co-host ids, the host's chat filter and mute list |
| `cohost_action {action, target, name, from}` | co-host -> host | kick / mute / unmute request; host checks the sender is a co-host and never targets the host |
| `lobby_disband` | host -> guests | guests leave (RETURN TO LOBBY ALONE, post-game DISBAND). Before, only the relay's `lobby_closed` was handled, so LAN / direct games never heard a disband |
| `lobby_auth {from, mid, account, user, pw}` | guest -> host | join check after entering the lobby, every 2 s up to 60 s; `pw` is an FNV-1a hash, not the password |
| `lobby_admit {mid, account}` | host -> guests | stops that guest's `lobby_auth` |

Added after v0.20.33 (dev builds on the closed repo `main`: `5136669`, `b6a9cf9`, `d267e92`, `9f02b20`, `803761b`,
`99ccb43`; not released publicly yet - Tyler tests first):

| type | from -> to | what it does |
|---|---|---|
| `vote_request {kind, target, name, from}` | player -> host | ask for a vote: kind 0 kick, 1 end round, 2 end game (`src/network/HostVotes`) |
| `vote_start {id, kind, target, name, starter, starter_id, voters, need, seconds}` | host -> everyone | a vote opened (30 s); `voters` = match machine ids allowed to vote |
| `vote_cast {id, yes, from}` | player -> host | a voter's choice |
| `vote_state {id, yes, no}` / `vote_end {id, passed, vetoed, yes, no}` | host -> everyone | counts / result |
| `host_remove` kinds `no_jip`, `min_ping` | host -> guest | join in progress off; ping below the host's MIN PING |
| `host_rules` + `rules`, `lock_teams` | host -> guests | lobby rules text; LOCK TEAMS hides CHANGE TEAM for guests |

A passed END ROUND calls `mp_round_end` (haloreach+0x7B3EC) on the engine thread (`ReachHost::RequestRoundEnd`);
END GAME takes everyone back to the lobby; a passed kick is `HostModeration::Kick`.

Host-sender checks use the existing pattern
(`udpSenderPeerId == 0 || m_hostMachineId == 0 || udpSenderPeerId == m_hostMachineId`).

## Changed behaviour in NetworkTransport

- `SendLobbyControlJson`: `host_pause`, `lobby_auth`, `lobby_admit` added to the list
  that always also goes over the relay.
- Host relay of guest chat (NBCJ): drops lines from host-muted players, runs the word
  filter. Host relay of voice (NVOX): drops host-muted players.
- Chat JSON now carries `sid` (sender machine id). Receivers drop lines from players they
  text-muted, then apply `HostModeration::ApplyChatRules` (host: its own mute + filter;
  guest: the rules from `host_rules`). This covers lobby chat that went through the
  master-server relay and never passed the host.
- New API: `SendHostPause`, `HostPauseState`, `HostPauseAgeMs`, `ClearHostPause`,
  `SendHostRemove`, `TakeHostRemoval`.

## MasterServerClient / SocialClient

- New `SetAdvertPassword(bool)`; the heartbeat body now sends `has_password` (it was
  only in the first register before), so turning a password on after listing updates the
  LOCKED badge. **Check the master server accepts it on heartbeat.**
- `SocialClient::SendInviteAsync` records the invited username so it may join an
  invite-only or locked lobby.

## Needs your call

1. **Server-side lobby check.** Lock / invite / password are enforced by the host
   (guest joins, host sends `host_remove` after 6 s). `/api/sessions/<id>/lobby/join`
   checks nothing. A server-side password check and hiding locked lobbies from
   `/api/sessions` would be stronger.
2. **Join in progress.** A player who joins straight into a running match is not checked
   for lock / password (the lobby process is closed during the match). Bans and kick
   cooldowns are still enforced in the match.
3. **Machine ids.** Moderation matches players by account id (stable), else this
   session's machine id, else name. Lobby roster `machine_id` = `LobbyMachineId()` =
   the transport id when the transport is active; mutes and removals rely on that.
4. **Dedicated server** console: `players`, `kick <id|name>`, `ban <id|name> [minutes]`,
   `bans`, `unban <n>`, and the server loop now runs `HostModeration::Tick` over the
   connected peers.

5. **Host transfer (Phase 6, parked for you).** Proposal: a master-server call such as
   `POST /api/sessions/<id>/lobby/transfer {new_host_peer_id}` (host token) that makes another lobby member the
   host and tells everyone over the relay (e.g. `lobby_host_changed {peer_id}`). The new host then re-registers the
   advert from the same lobby settings and the others stay in the same session. Doing it client-only would mean the
   chosen player opens a new lobby and everyone else hunts for it in `/api/sessions` (breaks for private lobbies), so
   it waits for your server side. In-match host rows already follow whoever is host (`NetworkTransport::IsHost`).
6. **Mid-match host migration** stays parked (engine path + master server).

## No network changes in the last builds

`803761b` (Firefight wave options) and `99ccb43` (friendly fire / betrayal booting / team changing in GAME OPTIONS)
only edit the game variant before launch; the host's variant reaches guests through the session as before, so nothing
new goes over the wire. Phase 6 host transfer stays parked on item 5 above.

## Tested on one PC

Pause for everyone, return to lobby alone (disband), kick / ban message, restart
waits for everyone, no credits under 5 minutes, 2-player smoke match after all changes
(0 crashes). Not tested (needs two logged-in players): rejoin enforcement, lobby lock /
password, co-host requests over the relay, relay chat mute / filter. Since then: join in progress off (late guest removed
in 12 ms), END ROUND vote (round ended, post-game panel), MP restart with everyone (lobby countdown, 0 crashes).
