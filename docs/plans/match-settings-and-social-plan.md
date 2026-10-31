# Match settings, lobbies, voice and the social layer — plan

Written 2026-10-31 after Tyler's feature dump. It answers the six asks, says what Reach actually supports, and phases
the work. Owners are the same split as `docs/TODO.md`: engine/netcode/server = @deepseek, menus/UI = @claude.

## 1. Can controls be changed in-game (campaign, firefight, forge, custom games)?

**Yes — the remap is live.** `ReachHost` reads the binding table every input poll and writes the resulting pad state into the
engine, so a preset or rebind takes effect on the next frame, in any mode the client hosts. What does **not** work yet:

- There is no way to open the controls page **during** a match; the pause menu has no settings section. Adding "Settings →
  Controls" to `PauseMenu` reuses `SettingsPanel`'s.Controls page and needs nothing from the engine.
- Reach's own in-game control settings screen (the one the engine would show) is MCC UI, not Reach UI — we do not use it.

Plan: expose the pause menu's Settings → Controls (and a "Controls" row in the lobby) with the live remap. Test: change
preset mid-match, confirm movement changes without a relaunch; Tyler should confirm with real buttons.

## 2. Pre-game lobby: inspect another player's profile

Reference: Reach's service-record card (rank, emblem, Spartan render, challenges, armory %, commendations).

What already exists: the lobby roster carries rank/emblem/nameplate/team/emblem background (server `b1656b4`), and
`CareerPanel` renders the local player's service record. What is missing: **a profile summary for remote players** and a
card UI.

Data model (server + client):

| field | source | privacy |
|---|---|---|
| rank index, credits rank | `Career::RankIndex` | public |
| emblem / background / nameplate / armour indices | `Appearance` | public |
| armory completion % | computed from owned armour pieces / total | public |
| daily + weekly challenges completed (counts) | `Career` | opt-in (default on for friends, off for strangers) |
| commendation completion % | `Career` | opt-in |
| recent games (optional, later) | local results | opt-in |

Server keeps a `profile` object on each roster entry (updated on register/heartbeat/join), and a new endpoint
`GET /api/sessions/:id/profile/:peerId` returns the summary. The card is drawn in `FrontEndMenu` when a lobby row is clicked
or hovered (the row already has a click target used for team/colour changes — move team/colour to a context menu).

Spartan render: `ArmorCustomizer` already renders a Spartan from an `Appearance` struct. Factor that render into a reusable
"preview any appearance" widget (camera, scene, lighting) and call it from the profile card. That is also the piece needed
for the armory's future "inspect player" and for the main-menu hero.

**ADD FRIEND:** needs a friends subsystem (server table + requests/accept + client list + presence). Phase it: (1) local
friends list + UI stubs, (2) server API + notifications, (3) join-from-friend and party invites reuse the existing invite
toast. Don't couple it to the profile card release; the card ships with a disabled "ADD FRIEND — coming later" row until (2)
lands, per the project rule about disabled rows with a reason.

## 3. Voice chat (pre-game, in-game, post-game)

Nothing exists today; no capture, no codec, no audio channel. Proposed architecture:

1. **Capture/playback:** WASAPI capture on the default communications device, 48 kHz mono, 20 ms frames; playback mixed into
   a dedicated WASAPI stream (never through Reach's XAudio, so voice survives map loads and menus).
2. **Codec:** Opus (BSD, ~3 KB per 20 ms frame at 24 kbps). Vendor `opus` via CMake; no patent/licence problem for a
   non-commercial community client (confirm with O7 before shipping).
3. **Transport:** reuse the existing relay WebSocket (`wss://…/ws/relay`) with a second message type (`voice`), binary
   payloads, per-session room. Server relays blobs to the other peers and enforces per-peer rate/size limits. Voice packets
   are independent of game packets so a lost frame cannot stall the match.
4. **Mixer:** jitter buffer (60–120 ms), per-peer volume/mute, duck game audio while someone speaks (optional), push-to-talk
   and open-mic modes. Mute icons in the scoreboard (§4.16 spec) become real once this exists.
5. **Channels:** lobby, team, proximity (needs player positions from the engine — we have the player-object chain), and
   post-game; "voice chat off/on" setting stays independent of the mic toggle.

Phasing: (a) local capture + loopback monitor, (b) relay transport + two-client test, (c) UI (volume, devices, PTT, mute
icons), (d) proximity + team channels, (e) post-game lobby. Estimate: (a)–(b) are a few days of work each; (c)–(e) after.

Owner: @deepseek (capture/transport/server), @claude (UI once the transport exists).

## 4. Match end: back to lobby, or playlist rotation

Current behaviour: a finished match drops to the main menu / results screen, and the host has to rebuild the lobby.

Target flow (host chooses per lobby setting):

- **Return to lobby (default):** after results, host's client relaunches into the same lobby session (server session stays
  alive while the host returns), guests reconnect to it; host can change map/mode/settings and start again. Everyone stays.
- **Playlist rotation:** host builds an ordered list (map + game type + variant + options per entry, any length, optionally
  tied to a mod set); after each match the host advances to the next entry automatically; browser shows current/next map.
  Voting is a follow-up (`menu-redesign-spec.md` §4.14): 2–3 options, ties to host.

Server needs: session lifecycle that survives a match (currently a session dies with the match process), a `rotation` array
on the lobby, a `next` index, and results→advance signalling. Client needs: results screen "Continue" (host) / "Waiting for
host" (guest), and a relaunch path that preserves the lobby session id. Dependencies: results hook is already there
(`last_game.bin`); the lobby launch API already carries map/mode/variant/difficulty/skulls.

## 5. Match settings for every game mode (campaign excluded)

### Reality check on the pasted list

That list is **Halo Online / Halo 4-style**: many entries (Spartan Charge, Ground Pound, Stabilizer, Clamber, Slide, Thruster
Pack, Smart-Link motion sensor, loadout tiers, Breakout, Battle Golf, Mini-Game, Race trait configurations) **do not exist in
Halo: Reach's engine**. Reach's custom games expose a smaller set: general (rounds, time, sudden death), player traits
(shields/health, damage, weapons/equipment/grenades, movement, sensors, appearance), respawn, team/friendly fire, map options
and per-mode scoring (Slayer, CTF, Assault, Oddball, KOTH, Infection, Invasion, Forge, Firefight options, Race). Reach's
game types are **Megalo scripts**: `game_variant_settings` tags define what the UI can expose, and the shipped
`data/multiplayer/megalo/*.txt` + HREK `MegaloEdit.exe` author them.

We should not hard-code the pasted list. The correct source of truth is the variant blob + `game_variant_settings` tags, which
also means custom game modes can add their own settings later.

### What the engine gives us

- `BlamEngine` already **decodes** a game variant with the engine call
  `decode(fileBytes, out[0xFC04], status)` (`haloreach+0x373A0`, same call RR's `set_game_variant` makes) and passes the
  decoded variant into the launch block. Variant resolution already works for `.bin`/`.mvar` in `game_variants\`.
- There is **no writer**: we can read what the engine decoded, but producing a variant file means writing the
  `c_game_variant` blob ourselves (its layout is what needs RE) or authoring with MegaloEdit and shipping the `.bin`.

### Phased delivery

1. **Variant picker + shipped variants (UI, no RE):** list `data/game_variants/*.bin` and Reach's stock variants, show
   name/mode/description, host picks one; lobby launch already transports `variant`. This gives hosts real choice on day one.
2. **Variant viewer:** parse the decoded struct fields we can identify (score to win, time limit, respawn, traits) and show
   them read-only in the lobby. That RE pass also gives the field map needed for (3).
3. **Variant editor (subset):** edit the identified fields in the decoded struct, re-encode with the engine's encoder if one
   is found, else write the blob field-by-field; validate by feeding it back through `decode`. Ship per-mode pages in the
   order: Slayer → CTF → Oddball/Assault → KOTH → Infection → Invasion → Firefight.
4. **Full editor / custom Megalo settings:** only after (3) works; custom settings that Reach's engine does not have stay out
   of scope (they would need engine hooks, not data).
5. **Host-only editing, replicated:** the host edits and launches; guests receive the variant through the existing lobby
   launch data (file transfer or the launch block), and the guest's engine decodes the same variant. Settings UI for guests is
   read-only.

Storage: `data/game_variants/<name>.bin` plus a JSON sidecar with display name, mode, description, author and a hash; the
hash goes in the lobby launch data so guests can verify they run the same variant.

### Credit farming: how we stop it

Credits/challenges must never be earnable by editing settings. Rules:

| activity | credits | challenges / commendations |
|---|---|---|
| Matchmade / server-verified multiplayer | full | full |
| Player-hosted custom game, **unmodified** settings | reduced (e.g. 25 %) | none |
| Player-hosted custom game, **modified** settings (any trait/round/score field) | none | none |
| Forge | none | none |
| Firefight (standard) | full | full |
| Firefight with custom settings | reduced | none |
| Campaign co-op / solo | full | full |
| Campaign with skulls | full (skull bonus applies) | full |

Implementation:

- **Eligibility bit:** the launch block/variant carries a "settings modified" flag computed by the host; the server also
  checks the variant hash against known-good stock variants. Credits are computed **server-side** from the results payload
  (`last_game.bin` → server), never trusted from the client.
- **Server-side results validation:** required fields present, sane ranges, duration ≥ minimum, score-per-minute caps, no
  impossible values; reject rather than clamp. The server stores the variant hash + settings class with the result.
- **Rate limits and caps:** per-account hourly credit cap, diminishing returns after N matches, no credits for matches under
  3 minutes, and challenge progress only from eligible matches.
- **Custom settings are still allowed** — they simply never earn. That is the anti-abuse design: the reward surface, not the
  feature, is restricted.

## 6. Ownership and order

| # | item | owner | depends on |
|---|---|---|---|
| 1 | Pause-menu Controls page + live remap verification | @claude (UI) + Tyler test | — |
| 2 | Variant picker (shipped variants) | @claude (UI) + @deepseek (list/validation) | — |
| 3 | Lobby profile card (rank/emblem/armory %) | @claude | server profile fields (@deepseek) |
| 4 | Profile privacy flags + challenge/commendation sharing | @deepseek | 3 |
| 5 | ADD FRIEND (friends subsystem) | @deepseek + @claude | 3 |
| 6 | Variant viewer / RE of `c_game_variant` | @deepseek | — |
| 7 | Match-end → lobby / rotation | @deepseek (server+relaunch) + @claude (results UI) | — |
| 8 | Voice chat transport | @deepseek | — |
| 9 | Voice UI (devices, PTT, mute icons) | @claude | 8 |
| 10 | Variant editor (subset) + credit eligibility | @deepseek | 6 |
| 11 | Credits server-side validation | @deepseek | 10 |

Campaign is explicitly out of scope for match settings (Tyler's rule).
