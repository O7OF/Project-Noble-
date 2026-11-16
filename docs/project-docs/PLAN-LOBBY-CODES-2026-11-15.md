# Plan: lobby share codes, public/private hosting and prominent "join with code" (2026-11-15)

Status: IMPLEMENTED (core) in `88d4937` on 2026-11-15. Codec + Main Menu entry + Server Browser modal + lobby share-code
panel are in and verified by captures; Social-screen quick action and a real two-PC end-to-end join test remain. The
format below was implemented as `NB1-XXXX-XXXX-XXXX-XXXX` (23 chars incl. prefix, CRC-8).

## What Tyler asked for

1. Players can host **public and private** lobbies.
2. **Both** kinds of host can generate a **code** they can post (e.g. Discord); another player can paste that code to
   join the match. (Private = not listed in the Server Browser, but the code works.)
3. Players can also receive **invites from their friends list** (this already exists - see below) and join from there.
4. The **"join friend with code"** option must be easy to spot.

## What already exists (do not rebuild)

- **Lobby privacy**: `MenuState::lobbyPrivacy` (0 public, 1 private) and `lobbyNetMode` (0 LAN, 1 online P2P) in
  `src/ui/FrontEndInternal.hpp`; the lobby screen draws the NETWORK ONLINE / LOBBY PUBLIC-Private controls.
  `MasterAdvert::isPrivate` is sent in `POST /api/sessions` (see `MasterServerClient.cpp` `DoRegister`) and in the
  heartbeat (`is_private`), so the backend already supports unlisted sessions.
- **Session id**: the master server returns `session_id` (16 hex chars, e.g. `37b843a50ae5d6e4`) and a `token`
  (`DoRegister` in `MasterServerClient.cpp`); `MasterServerClient::GetCurrentSessionId()` exposes it and
  `MenuState::lobbySessionId` holds it while hosting. `POST /api/sessions/<id>/lobby/join` plus
  `FetchLobbyStateSync()` are the join paths (relay/NAT signalling included).
- **Friends invites**: `SocialClient::SendInviteAsync(target, hostIp, port, cb, mode, map, sessionId)` ->
  backend `/api/invites/send`; the client polls `/api/invites/list`; incoming invites show as a toast
  (`DrawInviteToast` in `FrontEndSocial.cpp`) and as JOIN GAME cards at the top of the Social screen. The lobby and the
  friends list both have INVITE buttons (`FrontEndLobby.cpp` ~2484, `FrontEndSocial.cpp` ~600). The invite carries
  endpoint + session id; the toast JOIN relaunches into the match exactly like a direct connect.
- **Direct connect**: `ServerBrowser::TryJoin` / `-connect ip:port -session-id <id>` relaunch path; the
  server browser already has a DIRECT CONNECT modal (`FrontEndMenu`/`ServerBrowser.cpp`) with IP:port input.
- **Clipboard helpers**: `ImGui::SetClipboardText` used by the lobby "invite strip" (COPY INVITE copies `ip:port`).

## Share-code format (proposed, self-contained, backend-free)

Inputs available at the host: `sessionId` (8 bytes of hex), external IP + port, LAN IP, privacy flag, mode/map,
`noble::kBuildTag`. Payload:

```
version     1 byte    (1)
flags       1 byte    (bit0 public/private, bit1 LAN-only, bit2 has IPv4)
sessionId   8 bytes   (decoded from the 16-hex-char master id; zero for LAN-only)
ipv4        4 bytes   (external/public IP or LAN IP when LAN-only; zero if unknown)
port        2 bytes
crc32       4 bytes   (truncated to 2 bytes is enough for typo detection; keep 4 for simplicity)
```

Encode with Crockford Base32 (no `I`, `L`, `O`, `U`; case-insensitive), display with dashes:
`NB1-XXXXX-XXXXX-XXXXX-XXXXX-XXXXX-XXXXX` (~30 chars). Decoder accepts lower case, missing dashes and `0/O`, `1/I`
confusions. A `network/InviteCode.{hpp,cpp}` module should own encode/decode + validation (no ImGui, testable from a
console test hook like `-invite-code-test "<code>"`).

Join flow by code:

1. Decode; validate version byte (reject with "this invite is from an older build - ask them to update").
2. If it has a session id: call `MasterServerClient::FetchLobbyStateSync(id)` to validate build/limits/mod and get the
   canonical host IP/port (works for private sessions because they are stored, just unlisted). Then run the same
   `m_joinLobbyHandler` path the Server Browser uses (`FrontEndMenu` SetJoinLobbyHandler: version/limits checks, mod
   download prompt, EnterLobby). Fallback to the endpoint embedded in the code if the master call fails.
3. If LAN-only (no session id): relaunch with `-connect <ip>:<port>` (no `-session-id`), same as the direct connect path.
4. Show a small confirmation panel (host name, mode, map, public/private) before relaunching so a pasted wrong code does
   not kick the player out of the menu.

Encode/generate on the host when the lobby starts advertising (session id is known). LAN-only hosts generate a code
after the lobby knows its internal IP. Persist nothing; regenerate each lobby.

## UI placement (make it obvious)

- **Main menu**: add a highlighted `JOIN WITH CODE` row directly under `Server Browser` (or a large button next to it).
  It opens the join-code modal. This is the entry point Tyler wants "easy to spot".
- **Server Browser**: a `JOIN WITH CODE` action button in the filter row next to `DIRECT CONNECT`, and inside the Direct
  Connect modal a second field/tab for the code. The modal should explain "Paste the code a friend posted (e.g. in
  Discord)."
- **Lobby (host)**: an `INVITE / SHARE CODE` panel that always shows the code with a COPY CODE button, regardless of
  public/private, plus a one-line reminder ("Anyone with this code can join; private lobbies are not listed").
  Keep the existing IP:port COPY INVITE for friends. Both public and private hosts get the code.
- **Social screen**: keep incoming invite cards; add a `JOIN WITH CODE` action in the header so a friend who pasted a code
  in chat can enter it without leaving the Social screen. Friend invites already list JOIN GAME.
- Keyboard/pad: focusable like the other buttons; `Ctrl+V`/pad Y to paste is optional (input field already accepts paste).

## Files to touch

- `src/network/InviteCode.hpp/.cpp` (new; add to `CMakeLists.txt` in the client + `DEDICATED_SOURCES` if used).
- `src/ui/FrontEndMenu.cpp` - main-menu row, join-code modal, call the existing join-lobby handler; `FrontEndInternal.hpp`
  for the new `MenuState` fields (`joinCode`, modal flag) and any new `MainEntry`.
- `src/ui/ServerBrowser.cpp` - JOIN WITH CODE button + modal tab.
- `src/ui/FrontEndLobby.cpp` - host code panel + COPY CODE; generate code once the session id/endpoints are known.
- `src/ui/FrontEndSocial.cpp` - join-with-code action near invites; reuse the same modal.
- `src/ui/FrontEndInternal.hpp` - declarations/helpers.

## Tests (must be one-variable, on the AOC, `-NoFocus`)

- `-invite-code-test "<code>"` console/CLI hook: round-trip encode/decode for public, private, LAN and malformed codes
  (wrong CRC, wrong version, lowercase/no dashes).
- Host a private lobby with `-lobby-demo` / real host path, capture the SHARE CODE panel; decode the shown code in the
  test hook and check endpoint/session matches the host state.
- Join flow without a second machine: a code pointing at a local demo endpoint should reach the "connecting" stage and
  fail cleanly; with the second machine/LAN test, verify private lobby is not in the browser but the code joins.
- Main menu / Server Browser captures showing the JOIN WITH CODE entry is visible at 1080p and 4:3.

## Related queued work

- Custom **Armor Effect colours** (Tyler 05:40 ET): research/implement recoloured flame + lightning (Inclement Weather /
  storm) effects and expose new colour variants in the Armory. See the handoff "Armor effect colours" note; assets are
  under `objects\characters\spartans\fx\armor_fx\` in HREK and the tags dump cleanly with
  `tools\runmb.ps1` + `tools\tagdump.py` (`armor_fx.effect` events: legendary_flame -> head_fire.particle,
  eternal_flame -> head_fire_bungie.particle, storm -> thunder_cloud/electric_crackle/lightning_flash, pestilence,
  heart_attack). Particles are palette-driven (`palette_shift_amount`, palette bitmaps `fx\particles\fire\gradients\fire.bitmap`,
  `_gradients\plasma_blue_01.bitmap`), so recolours are a tag/bitmap job; exposing them as *new* Armory options needs
  extra effect-table slots (engine work similar in spirit to the visor table limitation).
