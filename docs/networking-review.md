# Networking review (2026-10-29, 02:45 ET)

Why a guest still can't get into a host's match, after the 2026-10-28/29 netcode work. Every finding below was
checked three times: (1) in Noble's code, (2) against the logs from Tyler joining O7OF (02:13 and 02:27) and the
Render server log, (3) against the working reference: Reach Renaissance's `host-probe.exe` (disassembly) and the
server code in `ServerTestDeploy-main`. Nothing from RR is copied; offsets and behaviour only.

## How a join works (MCC, RR and Noble)

- `haloreach.dll` doesn't open sockets. MCC (and RR, and Noble) give it two host-interface slots:
  **41 = send(peer id, bytes)** and **43 = receive(buffer) -> bytes + sender peer id**. RR's `host::hit` jump table
  sends only slots 41 and 43 to `net::handle`; no other slot touches networking (secondary interface: only 147,
  the session GUID). Noble matches this.
- Peer ids are opaque 64-bit numbers. RR's wire packet is `[8-byte sender id][payload]` (plus an optional
  `RR-BIG01` marker for oversize packets). Noble uses the same 8-byte header. Matches.
- Who is host, who is guest, the session id and the host's peer id all come from the **launch block**. RR writes
  them in `net::patch_launch_block` from its command line (`--as-guest`, `--net-self`, `--net-session`,
  `--net-host-id`, `--net-peer`). Noble writes them in `BlamEngine::BuildLaunchBlock`.
- Lobby, server list and relay are outside the engine: Noble's master server on Render (`server.js`) plus
  `MasterServerClient` / `NetworkTransport` (WebSocket relay) / `FrontEndMenu` (pre-game lobby).

## What the 02:27 log shows (guest side)

1. Guest relaunch -> `Launching ... as Guest connecting to 147.124.149.202:11774 (Session e382b06090788780)`.
2. Direct UDP works both ways: guest sends the 55-byte join request, the host answers (16-byte replies, then
   26-byte and 10-byte channel packets). No relay needed.
3. The guest then sits at **launch state 9, LoadProgress 2%** for the rest of the match. Both sides keep sending
   the same 10-byte packet (guest `08 00 5E 01 00 E0 ...`, host `08 00 00 01 EF 1F F8 ...`): the channel is up but
   the join never completes.
4. A **second Noble process** started on Tyler's PC at the same moment: a local "host" match on the same UDP
   port 11774, with the host's variant path cut to 64 characters. That's the "different lobby" Tyler saw.

## Findings

| # | Finding | Checked against | Status |
|---|---|---|---|
| 1 | **Guest launch block differs from RR's.** RR's `--as-guest`: +0x60 = 0; +0xE8..+0x10F zero with +0x100 = 0xFFFFFFFF and +0x108 = -1 (no local players, no player id); +0x2F0..+0x2FF zero with +0x2F4 = 0xFFFFFFFF; **everything from +0x300 zeroed** (no game variant; the host's comes over the session); +0x2BF14 = F3 01; +0x50 session, +0x58 self, +0x2F8 host. Noble's guest sets one local player (+0xE8 = 1), a player id at +0xF0, +0x100 = 0, +0x104 = 2, +0x108 = 0, and decodes a variant into +0x300. The stall at launch state 9 with traffic flowing fits a block the host's session won't accept. | RR `patch_launch_block` (read 3 times: string compares decoded, every store listed), `BlamEngine.cpp` guest branches (prebuilt and built), guest log | **Test switch added:** `-rr-guest-block` (guest only) lays the block out like RR after everything else. Default unchanged. |
| 2 | **Double launch on the guest.** The lobby screen relaunched into the match and also left `choice.play` set, so the menu's closing code started a second, unconnected match. | `FrontEndMenu.cpp` lobby + close paths, 02:27 log (two processes), Tyler's report | **Fixed** (lobby clears `choice.play`). |
| 3 | **SO_REUSEADDR on UDP 11774.** On Windows this lets a second process bind the same UDP port and receive datagrams meant for the first (the stray process in #2 did bind 11774 next to the joined guest). | `NetworkTransport::Initialize`, 02:27 log ("Initialized GUEST on UDP 11774" then "Initialized HOST on UDP 11774" in the same second) | **Fixed:** SO_EXCLUSIVEADDRUSE; a second process falls back to a free port. |
| 4 | **Host's lobby session deleted on launch.** The host's menu unregistered the session before the match process started, so the match registered a new one and guests were sent to a dead session. | `Relaunch::Start` -> `MasterServerClient::Shutdown`, server `DELETE /api/sessions/:id`, Render log (2a52->fa6e, d4af->ba43) | **Fixed** (keepSession when relaunching `-host ... -session-id`). Confirm in the server log: no new session id at launch. |
| 5 | **Lobby events on the relay thread** wrote menu state (strings, roster) while the menu read it; two launch events 0.1 s apart killed the guest with no crash log. | `NetworkTransport` relay worker, `FrontEndMenu` callbacks, 02:13 log | **Fixed** (queued behind a mutex; duplicate launch ignored). |
| 6 | **Launch sent twice** (HTTP `lobby/launch` and WS `lobby_launch`), and the server broadcasts both. | `FrontEndMenu` START MATCH, `server.js` | Client ignores the second. **O7OF:** send one. |
| 7 | **Variant path cut to 64 characters** by the server (`masterCleanText(..., 64)`), and it's a path on the host's PC. | `server.js` launch handlers, 02:27 log | **Fixed:** host sends the file name; guest uses its own copy. (With `-rr-guest-block` the guest uses no variant at all, like RR.) |
| 8 | **Every lobby guest had the same relay id** (the host's fixed id 0x484F535400000001, because the menu's transport isn't initialised). The relay keys guests by id, so a second guest replaces the first. | `MasterServerClient::JoinLobbySync`, relay URL in the 02:13/02:27 logs (`peerId=5210474914133311489`), `server.js` `room.guests.set(peerId)` | **Fixed:** random per-process lobby id. |
| 9 | **No guest-to-guest routing.** A guest sends every packet to the host whatever the target; the relay sends every host packet to every guest. RR has a star route (`net::star_wrap`, `route_peer`, `flush_held`: unknown routes are held, not broadcast). Fine for 2 players; 3+ will mis-deliver. | `OnNetSend`, `server.js` relay, RR `net::handle` call list | **O7OF:** needed before 3+ player tests. |
| 10 | **Host broadcasts to all peers when the target id is unknown** instead of holding the packet (RR holds). | `OnNetSend` host path, RR `flush_held` | **O7OF.** |
| 11 | **UDP-vs-relay switch is all-or-nothing:** any UDP packet in the last 3 s turns the relay off both ways, so one-way UDP (guest -> host works, host -> guest blocked) would hang. | `OnNetSend` / `OnNetRecv` | **O7OF** (track per direction / per peer). |
| 12 | **Private online lobbies are never advertised** by the match process (`hostOnline && hostPublic`), so no heartbeat: the session times out after 30 s and the host never joins the relay. | `main.cpp` host block, `server.js` `MASTER_SESSION_TIMEOUT_MS = 30000` | **O7OF.** |
| 13 | **Guests aren't told a session is gone** (DELETE / timeout); they wait for the engine's join timeout. | `server.js` | **O7OF** (broadcast `lobby_closed`). |
| 14 | Lobby `peer_id` is 2 for every guest who isn't logged in, and the roster de-duplicates by user name. | `FrontEndMenu` join handler, `server.js` lobby/join | **O7OF.** |

| 15 | **Same-network players.** The guest connects to the host's public address from the server. If both are behind the same router, that needs NAT loopback ("hairpin"), which many home routers don't do. | `FrontEndMenu` guest launch (`-connect` = server's `host_ip`), `server.js` `session.address` | **O7OF:** also send the host's LAN address and try it first. |

| 16 | **A dropped relay never came back, and a failed one could crash Noble.** When the server closed the WebSocket (Render redeploys, as in the 02:16 server log) or the handshake failed, the relay thread ended but stayed marked active, so the host's heartbeat never reconnected it. Worse, the next `StartRelay` assigned a new thread over the finished, unjoined one, which calls `std::terminate` (process ends with no crash log). | `NetworkTransport` StartRelay / StopRelay / relay worker | **Fixed:** the worker marks itself inactive when the server drops it, StopRelay always joins a finished thread, the host reconnects on its next heartbeat (3 s). A guest only starts its relay once, so a guest's dropped relay still doesn't come back (**O7OF**). |

| 17 | **Host advertised port 11774 even when the socket got another port** (11774 held by another program; the socket falls back to a free port). Guests would aim at the wrong port. | `main.cpp` host block (`cfg.port = 11774`), `NetworkTransport` bind fallback | **Fixed:** advertises `NetworkTransport::GetLocalPort()`. (The lobby's own advert, made by the menu before the match exists, still says 11774; only matters with two Nobles on one PC.) |

| 18 | **Stale launch.** When the host opens its next lobby from the same address and port, the server reuses the session and keeps the last `launch_data`; a guest's lobby poll treated "launch_data present" as "launch now", so a guest joining the next lobby went straight into a match that wasn't starting. | `server.js` re-register (`launchData: existing ? existing.launchData : null`), `FrontEndMenu` lobby poll; reproduced on the real server.js | **Fixed (client):** the poll launches only while the session is in progress. **O7OF:** clear `launchData` when a lobby re-registers with `in_progress: false`. |

Checked and fine: 8-byte wire header (matches RR); host +0x60 / +0xF8 = own id (matches RR); +0x2BF14 = F3 01
(matches RR); session id at +0x50 shared by both sides; host replies reach the guest by UDP (log); heartbeat
every 3 s (inside the 30 s server timeout); relay URL built from the server's response.

## Second round of checks (02:55 ET, three more passes)

1. **Compile check.** Every file changed tonight was compiled (syntax and types, with Noble's real ImGui and
   nlohmann headers, Windows APIs stubbed, and every log line's format string checked):
   `NetworkTransport.cpp`, `MasterServerClient.cpp`, `Relaunch.cpp`, `BlamEngine.cpp`, `ReachArmory.cpp`,
   `ArmorCustomizer.cpp`, `Career.cpp`: no errors. `FrontEndMenu.cpp` / `ReachHost.cpp`: the only errors are
   from the stubs (Direct3D and window functions), none in tonight's changes. The real check is still MSVC
   (`cmake --build build --config Release`).
2. **Logic re-check.** Host launch: the menu's relaunch arguments contain `-host` and `-session-id`, so the
   session is kept; the menu's second `Shutdown()` on exit does nothing (already stopped); the match process
   heartbeats within seconds, well inside the server's 30 s. Guest launch: one `Relaunch::Start`, `-connect`
   kept, test switches carried over. Exclusive UDP port: nothing else in Noble binds 11774 (discovery is
   11775). Lobby id: only used for the lobby relay; the match uses the transport's own guest id.
3. **RR re-read.** `patch_launch_block` read again: memset starts at +0x300 for (size - 0x300) bytes;
   +0x2BF14 is written after the memset; +0x50 comes from `--net-session`, +0x58 from `--net-self`, +0x2F8
   from `--net-peer`. `-rr-guest-block` writes the same fields in the same order. Noble's host and guest share
   the session id at +0x50 (fixed `HALOREAC`), so the switch keeps them matched.

## Third round (03:05 ET, five more checks)

1. **Ran the real server code.** `server.js` (syntax-checked with Node 22) was started locally with stand-ins for
   express/sqlite3/bcrypt and the real `ws` package, then driven the way Noble drives it. Results:
   - Host START: each guest got **2** `lobby_launch` events (#6 confirmed).
   - Variant over HTTP arrived as `c:/program files (x86)/steam/steamapps/common/Halo The Master Ch` (cut at 64),
     over WebSocket in full; a bare file name `slayer_054.bin` arrives intact (#7 confirmed, fix works).
   - Old host relaunch: DELETE -> the match's heartbeat gets **404** -> re-register returns a **new id** (#4
     confirmed). New flow (no DELETE): the match's heartbeat gets 200 and the same session's relay URL (fix works).
   - Two guests with the old shared id: the first guest got **0** launch events (#8 confirmed). With distinct ids
     both got it (fix works).
   - Relay: guest -> host packets arrive; a host packet reaches **every** guest; guest -> guest isn't routed
     (#9 confirmed).
   - No heartbeat for 36 s: the lobby returns 404 and the session's heartbeat 404 (#12 confirmed); guests got no
     "closed" message, only roster updates (#13 confirmed).
2. **Other launch paths.** Server browser / direct connect (`ServerBrowser::TryJoin`) relaunches once and closes
   its own window; no double launch there. `ReachHost` slot 4 (join timeout) relaunches to the menu with no
   arguments, as intended.
3. **Files on Tyler's PC.** Every change from tonight is present in the files on disk after the merges with
   O7OF's and Dominus's commits (checked by content, not size).
4. **Threads.** Lobby events (fixed earlier), peer table and relay queues are locked; the relay start/stop
   gap was not: finding #16, fixed.
5. **Rest of the socket setup.** Only the game socket used SO_REUSEADDR for a unicast port (fixed); LAN
   discovery on 11775 keeps it on purpose (several listeners). Wire sizes in the log match the 8-byte header
   (55-byte payload -> 63 bytes sent).

## Fourth round (03:10 ET, five more checks)

1. **Command line.** Every argument the lobby/server-browser relaunches pass (`-map -mode -difficulty -skulls
   -variant -connect -session-id -session-token -host -online/-lan -public/-private -max-players`) is parsed by
   `main.cpp`; `-rr-guest-block` and `-armory-unlock-all` are read from the command line where they're used and
   carried over by `Relaunch`.
2. **UDP/relay switch, modelled.** A step-by-step model of `OnNetSend`/`OnNetRecv` (10 s, 200 packets each way):
   UDP both ways 200/200 each; relay only 200/200; **guest->host UDP only: guest gets 0/200**; **host->guest UDP
   only: host gets 1/200**. Confirms #11: one-way UDP hangs the join.
3. **Server registration limits** (real server.js): re-registering from the same address and port reuses the
   session (so with the DELETE gone, even a fresh register keeps the lobby's id); a 4th game from one address on
   different ports gets `429 Too many games from this address` (rapid crash-and-retry testing can hit it; wait
   30 s). A join hands the host the guest's public IP with the guest's **local** port; the host's punch can miss
   behind a port-changing router, but the guest's PING then corrects the host's peer table.
4. **Ports.** Host advert port -> finding #17, fixed. LAN discovery (11775) unaffected.
5. **Docs.** `HANDOFF.md` has a 03:10 entry pointing here; tonight's Armory entry matches the code (settle
   rebuild off by default, try-on, pmcg table).

## Fifth round (03:20 ET, five more checks, run as programs)

1. **Transport run for real.** `NetworkTransport.cpp` (tonight's version) compiled on Linux against a thin
   socket shim, with AddressSanitizer and UndefinedBehaviorSanitizer, as two processes on one machine: the guest
   found 11774-equivalent taken and fell back to a free port; the host registered the guest from its PING; 5
   packets each way delivered with the right sender ids (host 0x484F535400000001, guest's random id); no
   sanitizer errors.
2. **Relay restart (#16) run for real.** With the WebSocket handshake failing, `StartRelay` five times in a row:
   the old code printed `terminate called without an active exception` on the second call (process killed); the
   new code survived all five.
3. **Lobby event queue (#5) under ThreadSanitizer.** The exact `PendingLobbyEvents` code from `FrontEndMenu.cpp`,
   with a relay thread sending 20,000 launch + roster events while the menu thread drains them: no races. The
   old pattern (relay thread writing the menu's strings directly) is flagged as a data race at once.
4. **`-rr-guest-block` byte check.** The block code from `BlamEngine.cpp` run on a 0x2BF30-byte block of random
   bytes, compared with RR's `patch_launch_block` steps rebuilt from the disassembly: 180,016 bytes compared,
   0 different; no out-of-range writes (AddressSanitizer).
5. **Lobby after a match (real server.js).** Found #18 (stale launch), fixed in the client.

## Next test (in this order)

1. Both sides on the new build (`git pull`, `cmake --build build --config Release`).
2. Host: normal launch, host a lobby. Guest: join, host presses START MATCH. Expect: one process on the guest,
   same session id in the server log before and after launch.
3. If the guest stops at launch state 9 again: guest relaunches Noble with `noble-client.exe -rr-guest-block`
   (the switch carries over to the match process) and joins again. Look for
   `BlamEngine: -rr-guest-block: guest block laid out like RR` in the guest log and whether the launch state
   passes 9.
4. Send both logs (host and guest). The first 60 packets each way are hex-dumped.
