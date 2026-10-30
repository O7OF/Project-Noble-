# Co-op test protocol — two agents, two machines

This is how an agent on one PC (host) and an agent on another (guest) run a campaign co-op test **without a human relaying
messages in real time**. The repository is the shared bus: each side commits a small, filtered artefact, and the other side
pulls it.

Agents cannot talk to each other directly, so read this literally — every handshake goes through a file under `coop/`.

---

## 0. Preconditions (both sides, every test)

1. **Same commit.** `git -C <repo> rev-parse --short HEAD` must be identical on both machines, and both clients must be
   built from it (`cmake --build build --config Release`). A mismatched build fails co-op for its own reasons.
2. **Client closed** before building or launching (`taskkill /IM noble-client.exe /F`).
3. The guest reaches the host's **UDP port**. If both machines are behind NAT, either use the relay (`-Relay <url>`) or
   forward the port; the endpoint the host publishes is only useful if the guest can actually reach it.
4. Nothing else is playing: no second client instance, no MCC launcher.

## 1. The bundle under `coop/`

| File | Written by | Meaning |
|---|---|---|
| `request.json` | who starts the test | commit, map, mode, player count, timestamp |
| `endpoint.txt` | host, automatically | `ip:port` the guest should dial |
| `host-<stamp>.log` | host, automatically | filtered host log (barrier, commands, aborts) |
| `guest-<stamp>.log` | guest, automatically | filtered guest log |
| `result-<role>-<stamp>.json` | both, automatically | status, commit, endpoint, log path |
| `REPORT.md` | whoever analyses | the conclusion, with the evidence lines quoted |

Nothing else is committed: full `noble-client.log` files are megabytes and stay on the machine that produced them.

## 2. The handshake

**Test requester** (either side) writes `coop/request.json` and commits:

```json
{ "commit": "f4d247e", "map": "m10", "mode": "campaign", "players": 2, "requestedBy": "O7", "at": "2026-10-31T12:00:00-04:00" }
```

**Host agent** pulls, then runs:

```powershell
.\tools\coop-test.ps1 -Role host -Peers 1 -PublicIp <host-public-ip> -Seconds 180
```

It launches `-map m10 -mode campaign -host -online -wait-peers 1`, waits for the transport's UDP port, writes
`coop/endpoint.txt`, runs for three minutes, then writes `coop/host-<stamp>.log` and `coop/result-host-<stamp>.json`.
Commit and push `coop/`.

**Guest agent** pulls, reads `coop/endpoint.txt`, then runs:

```powershell
.\tools\coop-test.ps1 -Role guest -Connect <ip:port> -Seconds 180
```

It launches `-map m10 -mode campaign -connect <ip:port>`, runs three minutes, writes `coop/guest-<stamp>.log` and
`coop/result-guest-<stamp>.json`. Commit and push `coop/`.

**Analysis** (whoever is next): compare the two filtered logs, write `coop/REPORT.md`, then either close the bug or make
the next change. The pair of logs is the entire evidence base — no screenshots needed unless the *camera* is the subject.

## 3. What "pass" looks like

Host log, in order:

```
BlamEngine: campaign/Forge co-op - holding the mission start until 0/1 engine peer(s) have joined...
BlamEngine: peer wait before start: 1/1 engine peer(s) connected
BlamEngine: 1 engine peer(s) joined before start; beginning the mission with them
BlamEngine: Posting Command 10 (Event: Start) ...
BlamEngine: Posting Command 11 ...
game session started at frame ...
```

Guest log, in order:

```
NetworkTransport: Configured guest target host <ip>:<port>
NetworkTransport: [GUEST] Sent initial NAT punch PING ...
game session started at frame ...
```

**Fail** indicators, with the meaning of each:

| Line | Meaning | Next step |
|---|---|---|
| `no guest engine traffic after 10 s` on the host | the session is not joinable before the mission advances | the fix must let the host start, then insert the guest at the first checkpoint (the opposite approach) |
| `evaluate_coop_insertion_point returned -1` | the engine refused the guest's insertion point; the hook overrides it to 0 | confirm the hook is enabled and that both builds include it |
| `abort called ... reason=27` (`0x1B`) | "An error occurred while playing co-op" — players incompatible | check version/commit parity, then the checkpoint sync path |
| `abort called ... reason=28` (`0x1C`) | co-op peer disconnect | network reachability, relay, or the guest's process died |
| `abort called ... reason=29` (`0x1D`) | co-op state sync timeout | the guest never received the mission state: check the guest's `game session started` line |
| `JOIN REFUSED` / `CONNECT REFUSED` | the session refused the join; the reason code is logged with the caller RVA | the accept hooks in `LobbyPatches` may need the new reason code |

## 4. Who fixes what

- The two agents must not both change the same file in the same test cycle. One side owns `src/engine/BlamEngine.cpp`
  (start sequencing, peer barrier) and the other owns `src/hooks/LobbyPatches.cpp` (session membership, insertion,
  aborts). The `coop/REPORT.md` should name the owner of the next change.
- Every test result must quote the deciding log lines verbatim — not a summary — so the other side can verify without the
  raw log.

## 5. Known limitation, stated plainly

Two agents can only do this if each has shell access to its own machine and a shared repository to push to. If the host's
machine cannot publish a reachable UDP endpoint, the direct `-connect` path cannot work and the test must go through the
master-server lobby instead (`-session-id`, `-session-token`, `-relay`), which needs a host who can see the lobby in the
Noble UI. That is a decision for whoever runs the machines, not something the agents can invent.
