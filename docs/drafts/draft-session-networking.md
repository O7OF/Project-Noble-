# Session & networking — player tables, lobby handshake, membership messages, dedicated server, master cluster

Draft by cc, 2026-11-14. How a Noble session is represented in `haloreach.dll`, how joins are handshaken, and the
Noble transport/master-server layer around it.

**Confidence legend**: **proven** = one-variable test + capture/log; **observed** = seen in a live run/log but not a
controlled single-variable test; **community** = community guide; **unverified** = inferred from code/disassembly/docs.

**Sources** (paths relative to `knowledge\` unless stated):
- `repo-docs\root\HANDOFF.md` — 10-28 20:30..22:50 and 10-29 00:35..03:45 netcode sections; 11-07 20:30 cluster; §5
- `repo-docs\limits-players.md` (C12) — players pool, session RVAs, milestones
- `repo-docs\limits-players-audit.md` — session object layout + masks
- `repo-docs\limits-players-message.md` — membership-update wire format
- `src\hooks\LobbyPatches.cpp` (repo root) — join gates, slot counts, session RVA table
- `01-PROJECT-HISTORY.md` — Phases 2–3 summary

## 1. The two player tables

The engine keeps **session membership** (network view) and the **game player table** (simulation view) separately,
and syncs them each frame.

### 1a. Session membership (inline arrays in the session object)

| Offset | Contents | Source | Confidence |
|---|---|---|---|
| +0x0060 | 17 × 0x158 peer records (16 players + 1 host/local) | limits-players-audit | unverified (disasm); traced |
| +0x173C | 32-bit player-valid mask | limits-players-audit | unverified (disasm); traced |
| +0x1740 | 16 × 0xB0 player records | limits-players-audit | unverified (disasm); traced |
| +0x2270 | 17 × 0x158 peer records (second/transmitted copy) | limits-players-audit | unverified (disasm) |
| +0x394C | 32-bit player-valid mask (second copy) | limits-players-audit | unverified (disasm) |
| +0x3950 | 16 × 0xB0 player records (second copy) | limits-players-audit | unverified (disasm) |

Session player 0's 8-byte identifier is at `+0x1744`, its owning peer index at `+0x174C` (HANDOFF 10-29 00:40).
The `+0x173C` mask has exactly **one writer** (the constructor at RVA `0x3CF864`, vtable `0xA490D8`); the other
66 reads are in 32 functions, almost all in the `0x38xxxx–0x45xxxx` network code (limits-players-audit).

### 1b. Game player table + pools

| Thing | Value | Source | Confidence |
|---|---|---|---|
| Game player table singleton | `haloreach + 0x4E0E910`; mask at +0xA0, entries (16 × 0xB8) from +0xA8 | LobbyPatches.cpp `Hook_PlayerSync` | unverified (code); logged |
| Session→game sync | RVA `0x41F510` (compares 16-entry table with session players; emits events: 0 left, 1 moved, 3 joined, 4 changed) | LobbyPatches.cpp `Hook_PlayerSync` | unverified (code); logged |
| `players` data array pool | 16 × 0x490 (1,168 B), created by `data_array_new` at RVA `0x43474` (count imm `0x43470`, size `0x43465`) | limits-players.md | unverified (disasm) |
| `fire team` pool | 16 × 0xD4, created at RVA `0x122E58` | limits-players.md | unverified (disasm) |
| `simulation view data array` | 16 × 0x118 | limits-players.md | unverified |
| Local players table | `haloreach + 0x2A03C50`, stride 0xAB8 (subtitles read profile data at +8) | HANDOFF §5 | unverified (code) |

## 2. Masks and bounds

| What | Width | Where | Source |
|---|---|---|---|
| Session player-valid mask | 32-bit | session +0x173C (copy +0x394C) | limits-players-audit |
| Peer mask | 32-bit | session +0x5C | LobbyPatches `Hook_SessionUpdate` |
| Packet player indices / roster masks | 16-bit (`0xFFFF`) | session/replication wire format | limits-players.md §2.2 |

16 → 32 players needs only the 32-bit mask's loop bounds raised; 32 → 64/128 needs the mask widened to 64/128 bits
(67 read sites + writers) and a **protocol version bump** (limits-players.md §2.2, §3).

## 3. Transport framing and the lobby handshake

- **Wire framing.** Host slot 41 (`OnNetSend`) prepends an 8-byte machine-id header to every packet; slot 43
  (`OnNetRecv`) strips it and returns it as `outSenderPeerId`. Without it, the engine rejects every join packet
  (guest stuck at launch state 9). Source: HANDOFF 10-28 22:50. **observed** (fixed by Dominus).
- **Identity fields** that must be unique per machine (HANDOFF 10-29 00:40, 10-28 22:15):
  - `s_player_identifier` at launch block +0xF0 (guest value derived from `localMachineId ^ 0x3F59F3BE…`, so it
    never collides with host player 0).
  - XUID from host slot 88 (per-guest `0x0009000000000000 | (localMachineId & 0xFFFF…)`).
  - Session id at launch block +0x50 (deterministic: `PNCSLOB1` closed-source, `HALOREAC` open-source).
- **Join-request path (host).** join-request built at RVA `0x3923DC`; validated by
  `can_accept_join_request` (`0x3975D8`) → `can_accept_player_join_request` (`0x3973BC`) →
  `find_player_by_identifier` (`0x39D59C`). Refusals logged at `0x425368` (join) / `0x42459C` (connect).
  Source: HANDOFF 10-29 00:40; LobbyPatches. **observed**.

### Session message types

From the DLL's own table (`0x42609C`), names decoded in LobbyPatches.cpp:

| Type | Name | Type | Name |
|---|---|---|---|
| 8 | join-request | 19 | host-reestablish |
| 9 | peer-connect | 20 | host-decline |
| 10 | join-abort | 21 | peer-reestablish |
| 11 | join-refuse | 22 | peer-establish |
| 12 | leave-session | 23 | election |
| 13 | leave-acknowledge | 24 | election-refuse |
| 14 | session-disband | 25 | time-synchronize |
| 15 | session-boot | 26 | membership-update |
| 16 | host-handoff | 27 | (player joined; decode success counted) |
| 17 | peer-handoff | 36 | security |
| 18 | host-transition | 46 | end-game |

**Membership update (type 26)** — registered at RVA `0x427E8C` (message table +0x680), size 0x2550, encode
`0x42675C` / decode `0x427140`. Payload: +0x08/+0x0C/+0x10 ids, +0x14 peer count (i16), +0x16 player count (i16),
peers at +0x18 (17 × 0x180), players at +0x1998 (16 × 0xB8), tail +0x2518..+0x254F. Encoder field width 5 bits;
player index 4 bits. Source: limits-players-message.md. **unverified (disasm)**; the 32-player extension was
**LAN-tested** (3 players, `overnight_lan3.bat`).

## 4. Join-in-progress (Noble's overrides)

Noble forces ACCEPT on the engine's refusal paths so remote players can join an in-progress session:

| Patch | RVA | Effect | Source |
|---|---|---|---|
| `can_accept_join_request` detour | 0x3975D8 | any non-zero refusal → 0 | LobbyPatches.cpp |
| `can_accept_player_join_request` detour | 0x3973BC | any non-zero refusal → 0 | LobbyPatches.cpp |
| `acknowledge_join_request` detour | 0x397914 | non-zero reason → 0 | LobbyPatches.cpp |
| Join-Any gate (jnz→jmp) | 0x397635 / 0x397363 | host accepts while loading/in simulation | LobbyPatches.cpp |
| `evaluate_coop_insertion_point` | 0x39CE10 | negative (save mismatch) → 0 (mission start) | LobbyPatches.cpp |
| `c_network_session::abort` | 0x44EBA0 | suppress co-op incompatibility reasons | LobbyPatches.cpp |
| `boot_peer` | 0x395DE8 | keep guest in co-op (engine boots it otherwise) | LobbyPatches.cpp |
| co-op/Firefight cap bypass | 0x40954A / 0x40955B / 0x411B6E / 0x411D71 / 0x397689 | bypass caps + incompatibility aborts | LobbyPatches.cpp |
| cmovz NOP | 0x40958B | stop reason 0x1B from being loaded | LobbyPatches.cpp |
| `process_host_handoff` | 0x4035DC | keep host authority (clear candidates +0x60F04, target +0x60F18 = −1) | LobbyPatches.cpp |

Refusal codes seen and named: 15 (join-abort "abort ignored"), 0x16/22 (duplicate `s_player_identifier`), 0x1B
(co-op save mismatch), 6 (match simulation), 5 (campaign in-progress), 0x1C (peer disconnect), 0x20 (Firefight).
Session state lives at +0x60EF8 (6 = in game); the join queue mode global is `0x2A49308` (session pointers
`0x2A1FE08/10/18`, mode-set byte `0x2A1FD40`). Source: LobbyPatches; limits-players.md.

## 5. Slot counts

| Thing | Value | Source | Confidence |
|---|---|---|---|
| `s_maxMultiplayerPlayers` | **128** (Custom Games / Forge) | `LobbyPatches.cpp:21` | code |
| `s_maxCoopPlayers` | **16** (campaign/firefight) | `LobbyPatches.cpp:22` | code |
| `set_slot_counts` hook | expands `publicSlots` 16/8 → ≤128 (MP), co-op ≤16 | `LobbyPatches.cpp` | code |
| Session peer cap byte | patched to 128 | `LobbyPatches.cpp` (sig `…83 F8 10…`) | code |
| Engine session arrays | 16 players + 17 peers (hard-coded inline) | limits-players-audit | unverified (disasm) |
| Noble lobby roster | 128 slots (UI dynamic via `lobbyRoster.size()`); co-op 16 | limits-players.md | code |

The lobby accepts 128, but the **match** still can only build 16 players (the `players` pool and every
player-indexed table is 16). Source: limits-players.md §1.

## 6. Dedicated server

- `noble-dedicated.exe` is the headless host build. It has **no Noble P2P relay transport** — `NetworkTransport.cpp`
  is not in that target, and client-only code is compiled out with `#ifndef NOBLE_DEDICATED` (link failure fixed
  `5c2caa9`, 2026-11-13). Source: `pc-handoffs\HANDOFF-ELITE-2026-11-12-NIGHT.md` 13:10; LobbyPatches
  `Hook_ProcessHostHandoff`. **code**.
- Engine-limits flags: `noble-dedicated -limits <stock|v16|v16p|full>` and `-slot-players 32` (32-player work).
  Source: `desktop-guides\Noble-AI-Limits-Report-2026-11-08.md`; limits-players-message. **observed** (log lines).
- It advertises a `limits_id` in registration/heartbeat so clients can refuse to join a server whose engine limits
  differ (D3 join gating; wired, not end-to-end tested). Source: Noble-AI-Limits-Report D3.

## 7. Master-server cluster and P2P/relay transport

- **Cluster (3 Render nodes, federated):** `projectnobleeast` / `projectnoblewest` / `projectnobleuk`
  (`.onrender.com`). Each node merges `/api/browser` with the other two and `/api/health` reports the mesh.
  Backend = `ServerTestDeploy/server.js`. Source: HANDOFF 11-07 20:30. **observed**.
- **Build gate:** protocol `2`, headers `X-Noble-Build` / `X-Noble-Protocol`, `REQUIRED_BUILD_PREFIX=0.19.`,
  session magic `PNCSLOB1`; relay path `/ws/relay2?protocol=2`. Old builds get `403 client_outdated`.
  Source: HANDOFF 11-07. **observed** (gate proven in production).
- **Endpoints:** `POST /api/sessions` (register), `PUT /api/sessions/heartbeat`, `POST /api/sessions/:id/join`,
  `GET /api/sessions` / `GET /api/browser`, lobby endpoints (`GET …/lobby`, `POST …/lobby/join|leave|launch`),
  `DELETE /api/sessions/:id`. Source: HANDOFF 11-07, 10-29 00:35.
- **Transport:** game packets on UDP **11774**, LAN discovery beacons on **11775**; direct-UDP NAT hole-punch with
  a WebSocket relay fallback (used only after ~3 s without direct UDP, to avoid duplicate delivery). 8-byte machine-id
  header (§3). Source: HANDOFF 10-28 20:30/22:50/23:35/23:55. **observed**.

## 8. Open questions

- The third membership-message decode caller (`0x442394`, static buffer at RVA `0x4E3BB80`) was not verified for
  the 32-player (0xA300) message size — risk of overrun beyond 16 players (limits-players-message).
- The 32-player extension paths (>16 players / >17 peers) were never exercised; only the wire widths were
  LAN-tested with 3 players.
- Dedicated-server join gating (D3 `limits_id`) is untested end-to-end (backend does not yet store/return the field).
- Host slot 25's role (guest XUID) is still only inferred; its return value alone was ruled out as the refusal-15
  cause, but its exact contract was not disassembled.
