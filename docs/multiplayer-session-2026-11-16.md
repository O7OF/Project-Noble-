# Multiplayer session - status 2026-11-16 (DeepSeek)

Multiplayer is the front item of Tyler's 2026-11-16 re-order. Repo `wip-merge`, HEAD `5b9d39f`, version 0.19.103,
no release. Both gates (`noble-dev`, `noble-client`) build clean. Test window: 2026-11-16 10:46-11:35 ET.

## What was found at hand-off

HEAD `4bfe827` could not complete **any** multiplayer join - not even two LAN instances. Commit `1a6846b` (campaign
session, same morning) restored the `c_network_session_membership::set_slot_counts` detour by RVA after its SigScanner
pattern stopped matching the installed dll. The hook had been silently uninstalled for weeks, and its multiplayer branch
expanded every multiplayer/forge session from the engine's 16 slots to `min(GetMaxMultiplayerPlayers(),128)` = 128
(`s_maxMultiplayerPlayers` is a constant 128 and nothing ever calls the setter). With the hook live the host declared
128 slots, closed the guest channel with reason 10 right after the first membership update, the guest closed with
reason 14, every rejoin got CONNECT REFUSED reason 3, and `set_slot_counts` churned 4607 times in a 75 s run.

## Fixes committed

| Commit | What |
|---|---|
| `175637e` | Multiplayer sessions keep the engine's stock 16 slots (expansion only under `NOBLE_MP_SLOTS32=1` + the 32-player slot patch, capped at 32); the campaign/firefight fallback no longer overrides `publicSlots == 0` (the engine's join lock). |
| `e531160` | `tools/mp-session-test.ps1` N-instance LAN harness (guest-exe sync, low-spec settings, unique identities, focus guard, RAM abort, evidence extraction) + env-gated log-only high-slot probes (`NOBLE_SLOT_PROBE=1`): membership producer payload/caller dump, membership consumer payload+state dump, player-remove primitive attribution. |
| `0e214e2` | Harness leave/rejoin mode and capture evidence. |
| `c33ef10` | Cave the message-33 player-properties handler (`0x44B24C`, generator + SlotMemberCaves.inc, 44 member caves / 111 slot sites total). This is what lets the host's shadow player 16 get its record +0x1C set, so the peer readiness test promotes peer 17 to **peer state 8**. |
| `5b9d39f` | Probe: dump the membership payload tail (flags 0x2518-0x254F, 0x2500-0x2550 hex) so a rejected delta can be attributed. |

## Verified end-to-end (stock, shipped 16-player path - no dev flags)

2 players (`stock2-20261116-112427`) and 5 players (`stock5-20261116-112548`) on one machine, 20_sword_slayer:
stock membership message size 9552, host `peermask 0x1F / pmask 0x1F` with peer states p0..p4 all 8, engine
game-player table 5/5 used, every guest at session state 4 with the msg 44 state stream, **0 CONNECT REFUSED,
0 crashes**. Join/leave/rejoin was proven separately in `rejoin5-20261116-111256` (guest hard-killed at t+40 s, host
drops to 4 peers/4 players, guest relaunched at t+52 s, re-seated as peer 4, rejoined guest resumes the msg 44 stream;
screenshots under `build/bin/Release/data/userdata/claude-tests-2026-11-11/rejoin5-*`).

## One-machine 16-player feasibility (measured 2026-11-16, Tyler approved the low-memory test profile)

Tyler approved experimenting with a dev-only lower-memory test profile, the lowest possible client resolution and the
cheapest window mode. Measurements on the pinned build (idle system baseline 12.8 GB used of 31.8 GB):

| Config (one instance) | Peak working set | Peak private commit | System used |
|---|---|---|---|
| Low-spec 1280x720, borderless, Full limits (shipping pin) | 5.8 GB | 5.1 GB | 18.3 GB |
| 640x360, windowed, Full limits | 6.1 GB | - | 19.1 GB |
| 640x360, borderless, Full limits | 5.5 GB | - | 18.3 GB |
| **640x360, borderless, Stock engine limits (low-mem profile)** | **5.0 GB** | **4.74 GB** | **18.8 GB** |

- Resolution and window mode do **not** reduce the engine's memory (world/geometry/tag data dominates; named engine
  pools total only 0.08 GB of capacity and the pinned AI actors add ~6 MB). The dev-only Stock-limits probe saves
  ~0.5-0.8 GB per instance (~10%), which is implemented as `tools/mp-session-test.ps1 -LowMem` (640x360 borderless
  settings template + `NOBLE_ENGINE_LIMITS_PROBE=0` + free-RAM guard `-MinFreeGb`), with templates
  `lowmem-640x360-windowed.json` / `lowmem-640x360-borderless.json` and system/private-memory accounting in the
  harness. The probe accepts Stock only in `NOBLE_DEV_BUILD`; the shipped pin stays Full + 1024.
- Multi-instance attempts with the low-mem profile: 8 instances aborted the 1.5 GB free-RAM guard at 30.5 GB system
  used (private commit 19.9 GB, later guests still loading); 6 instances aborted the 1 GB guard at 31.65 GB system
  used (private 23.98 GB, sixth guest still loading). The already-proven safe maximum on this machine is **5 players**
  (`fix5`/`stock5` runs).
- **Conclusion: 16 players on this one machine is not achievable with the current hardware** - at ~4.7-5.5 GB private
  commit per full client, 16 clients need roughly **76-88 GB**, against 31.8 GB installed (and ~19 GB realistically
  available next to the OS/apps). Page-file-only operation would thrash and is not a usable acceptance environment.
  Reaching the 16/32-player acceptance bar needs one of: (a) 3-4 PCs x 4-5 instances (recommended; the harness already
  supports `-connect <host-ip>` and per-machine identities), or (b) a RAM upgrade to 96 GB+ on this machine. The
  session/join layer itself is healthy and proven at 5 players on one machine.
## Tyler approvals 2026-11-16 11:45 ET (answers to the session report)

1. **Dev-only lower-memory test profile approved** - implemented (commit `a3bc5e4`): `tools/mp-session-test.ps1 -LowMem`
   selects a 640x360 borderless template, requests the Stock engine-limits layout through the dev-only
   `NOBLE_ENGINE_LIMITS_PROBE=0` (now accepted inside `NOBLE_DEV_BUILD`; shipped pin untouched) and adds a free-RAM
   guard plus system/private-memory accounting. **Measured result: 16 players on this machine is not achievable.**
   One instance costs 5.0-5.8 GB WS / 4.7-5.5 GB private (resolution and window mode do not matter; named pools total
   0.08 GB), so 16 clients need ~76-88 GB against 31.8 GB installed. 8 instances hit the RAM guard at 30.5 GB used,
   6 at 31.65 GB. Max proven on this machine stays 5 players. Next step needs Tyler: multi-PC acceptance (3-4 PCs x
   4-5 instances; harness supports `-connect <host-ip>`) or a RAM upgrade to 96 GB+.
2. **Lowest resolution + cheapest display mode** - tested: 640x360 windowed 6.1 GB, 640x360 borderless 5.5 GB,
   1280x720 borderless 5.8 GB, low-mem Stock 5.0 GB. Lower resolution/window mode does not reduce engine memory, so
   the dev-only profile uses 640x360 borderless + Stock limits purely as the cheapest measured option.
3. **Lobby/picker/server-browser honesty approved and implemented** (commit `a2c7125`): one seat-capability source
   (`LobbyPatches::EffectiveMultiplayerSeats()` = 16 stock, 32 only with the dev slot patch + `NOBLE_MP_SLOTS32=1`);
   `-max-players` is clamped and published as the session target; MAX PLAYERS keeps 4/8/12/16 (16 default) and greys
   24/32 with a reason; lobby header/adverts/beacon use the effective cap; LAN beacon shows the real player count;
   in-match scoreboard and loading roster size from the engine game-player table (verified: 2-player LAN run shows
   2 rows, `live roster 2 player(s)`, screenshot `lobbytest3-20261116-120707-host-cap0.png`); server browser shows
   real counts, FULL / STALE / NOT HEARD LATELY / 17+ UNSUPPORTED verdicts and blocks JOIN on a full row.
   Remaining: guest names on direct/LAN launches are placeholders (`PLAYER n`) until the msg-27 identity data is used.
## Remaining blockers

1. **High-slot (peer 17 / player 16) join, milestone 2 - membership layer FIXED, seat layer open.** Commit `48cd1d2`:
   the message-27 peer-properties handler used stock peer math (`0x39D23E`/`0x39F04E`/`0x39F0C0`), whose peer-17
   property copy overwrote player 0/1 records; player 0's slot became 4, the engine's 2-bit slot encoder leaked it
   into the peer field (host sent `peer 0 slot 4`, guest decoded `peer 1 slot 0`) and the guest rejected the delta.
   Three caves added (47 member caves) plus a producer-side invalid-slot sanitiser. Verified with `NOBLE_SLOT_PROBE=1`
   and `NOBLE_SLOT_HIGH=1`: peer 17 reaches state 8 on both sides and the guest **accepts** the delta (ret 1, no
   reset, msgSeq advances, peermask 0x20001 / pmask 0x10001, session state 6 local peer 17 record state 8).
   **Remaining (next step):** the host still calls `boot_peer(17, reason 9)` every frame after msg 27 (Noble
   suppresses it, `keeping peer 17`), i.e. the join-in-progress seat path still refuses the player; the guest channel
   closes with reason 14 ~19 s later and the guest returns cleanly to the lobby. Next fixes are the still-uncaved
   player-add/accept sites listed in the notes: `0x39740C`/`0x397503`/`0x39752A` (can_accept_player_join_request) and
   `0x39FB62`/`0x39FB7D` (queued player-add apply), then a real 32-player session room.
2. **16 players on one machine is not possible**: measured 5.2-5.9 GB working set per instance at 1280x720/30 fps/
   muted (23.9 GB for 5 of 31.8 GB). 16 instances need ~80-95 GB. Acceptance needs 3-4 PCs x 4-5 instances, or a
   Tyler-approved dev-only lower-memory profile for test instances. The harness already accepts `-connect <host-ip>`.
3. **32-player session (milestone 3)**: the declared-32 path joins cleanly with 2 players (`s32decl`) but the host
   churns 3713 set_slot_counts calls in 60 s (engine re-applies 16, hook re-expands to 32). Real 32 players is the same
   hardware problem at 2x the machines.
4. **Lobby/browser honesty (milestone 4)**: design ready in `project-docs/DESIGN-LOBBY-HONESTY-2026-11-16.md`; known
   visible bug from the 5-player evidence: Noble's in-match scoreboard and loading roster show only 1 row while 5
   players are in the session. Any visible picker/cap change needs Tyler's approval.

## Test rig quick reference

```powershell
# stock shipped path, 5 players on one machine
powershell -File tools/mp-session-test.ps1 -Players 5 -MaxPlayers 16 -NoWaitPeers -Seconds 90
# 32-player slot patch, normal slots
powershell -File tools/mp-session-test.ps1 -Players 2 -Slot32 -MaxPlayers 32 -NoWaitPeers -Seconds 60
# high-slot dev test (peer 17 / player 16)
powershell -File tools/mp-session-test.ps1 -Players 2 -Slot32 -High -MaxPlayers 32 -NoWaitPeers -Seconds 60
# leave/rejoin evidence
powershell -File tools/mp-session-test.ps1 -Players 5 -MaxPlayers 16 -NoWaitPeers -Seconds 110 -LeaveAt 40 -RejoinAt 52 -CaptureAt 20
```

`NOBLE_WINDOW_SECONDARY=1` keeps game windows on the secondary monitor; `NOBLE_SLOT_PROBE=1` enables the log-only
high-slot diagnostics; `NOBLE_MP_SLOTS32=1` (with the slot patch) declares 32 session slots.

Evidence logs are under `build/bin/Release*/host-*/client*-*.log` plus the matching `*-summary.txt` / `*-result.json`.
Design/analysis notes: `knowledge/project-docs/STATUS-MULTIPLAYER-2026-11-16.md`,
`knowledge/project-docs/NOTE-HIGHSLOT-2026-11-16.md`, `knowledge/project-docs/NOTE-RR-REFERENCE-2026-11-16.md`,
`knowledge/project-docs/NOTE-MP16-AUDIT-2026-11-16.md`, `knowledge/project-docs/DESIGN-LOBBY-HONESTY-2026-11-16.md`.
