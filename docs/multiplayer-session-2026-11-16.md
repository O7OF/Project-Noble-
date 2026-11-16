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

## Remaining blockers

1. **High-slot (peer 17 / player 16) join, milestone 2.** With `NOBLE_SLOT_HIGH=1` the host now promotes peer 17 to
   state 8 (`c33ef10`), but the follow-up membership **delta is rejected by the guest consumer** (ret 0 -> membership
   reset -> guest sends msg 7, host CLOSE reason 10). Probe evidence (`ph5-20261116-112742`): the host's delta carries
   player 0 with `peer 0 slot 4` while the guest decode shows `peer 1 slot 0` (slot 4 is out of the valid 0..3
   range), so the delta's data/consistency check fails. Next step: find why the player-0 delta entry carries slot 4
   and whether the producer/encoder fields for delta entries are consistent for high-slot peers (a further uncached
   address path in the producer 0x39DD4C or the player-record slot write is likely).
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
