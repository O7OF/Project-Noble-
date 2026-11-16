# STATUS - Multiplayer (Project Noble, 2026-11-16)

> Consolidated current-state note for the lead taking over the **multiplayer** workstream. Read this instead of
> re-reading the 4000+ lines of history. Every claim cites the file+line (or doc section) it came from; anything not
> proven is marked **UNCERTAIN**.
>
> Confidence marks: **PROVEN** = run with logs/captures; **CODE** = in the source tree, not run; **DOC** = stated in a
> doc only; **UNCERTAIN** = not settled by the docs.
>
> Scope: **multiplayer large sessions**. Campaign co-op >4 is parked at the end of the list
> ([TODO-MASTER](../desktop-guides/TODO-MASTER-2026-11-14.md):21) and appears here only where it shares engine paths
> (network session-start mode, close reason 10). The campaign work is what HEAD and the newest logs are about.
>
> Source precedence (newest wins): HEAD commit `4bfe827` (2026-11-16 10:33 ET) > [NOTE-PLAYER-COUNTS-2026-11-15.md](../NOTE-PLAYER-COUNTS-2026-11-15.md) (root copy, mtime 11-16 10:29, 1323 lines) > [TODO-MASTER](../desktop-guides/TODO-MASTER-2026-11-14.md) / [TOP10](TOP10-NEXT-UPGRADES-2026-11-15.md) (mtime 11-16 10:41) > [PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md) (10:45) > 11-15 14:35 test note > [LIMITS-HANDOFF](../../LIMITS-HANDOFF.md) (11-10) > `docs/limits-players*` (11-08..10).

---

## 1. HEAD / build state

| Fact | Value | Evidence |
|---|---|---|
| Branch / HEAD | `wip-merge` @ **`4bfe827`** 2026-11-16 10:33:27 -0500 - "Lift campaign co-op 4-player launch caps and add >4 launch probes" | `git log -1`; `git show --stat HEAD` = `src/hooks/LobbyPatches.cpp` +447/-3 (campaign lane only) |
| Version | **0.19.103** (dev string `0.19.103-dev`), no bump/release/tag/public push | `src/common/Version.hpp:14-18`; task rules |
| Working tree | clean for tracked files; only untracked docs/tools (e.g. `?? docs/OVERNIGHT-2026-11-09.md`, `?? tools/limits/gen_message.py`, `?? knowledge.rar`) | `git status --short`, measured 11-16 10:52 ET |
| Last multiplayer code change | **`86d6b61` 2026-11-10** "Merge origin/main (0.19.46) + ... 32-player work (paused)". No `src/host/Slot*` commit after that; later LobbyPatches commits are campaign (last: `4bfe827`, `1a6846b` slot-count hook fix) | `git log --date=short -- src/host/Slot*` (table in Appendix B) |
| Last build | `build/bin/Release/noble-dev.exe` 2026-11-16 **10:28:58**, `noble-client.exe` **10:29:04** (both gates present) | file timestamps, measured 11-16 10:52 ET. No independent rebuild done here (hard rule: do not touch build/); whether the 10:28/10:29 binaries contain every byte of 4bfe827 is inferred - **UNCERTAIN** only in the 10:28-vs-10:33 ordering sense |
| Guest instance binaries | `Release2..Release5\noble-dev.exe` all **09:07:44** today = **stale** vs the host build | the 11-15 note says guests were refreshed for the LAN test (`NOTE-PLAYER-COUNTS:59-61`), but they are stale again now; **refresh before any LAN test** (probe-lan32.ps1 does not copy the exe: `build/bin/Release/probe-lan32.ps1:5-7`; `probe-jip.ps1` does: `NOTE-PLAYER-COUNTS:222-224`) |
| Last tests | **campaign probes 11-16 10:32** (`build/bin/Release/host-camp5.log`, `build/bin/Release2/client2-camp5.log` mtimes); **last multiplayer LAN32 test 11-15 14:36-14:38 (normal slots) / 14:29-14:30 (high slots)**; last single-window 32 install 11-15 14:21 (`probe-slot32.log`) | log mtimes; log contents in section 3 |
| Release state | none; version stays 0.19.103; HEAD's own body says the 4-player campaign launch was re-verified after the cap lift and the 5-player wall is now the network session-start path | `git show -s --format=%B HEAD` |

**Bottom line:** source is frozen at the 11-10 32-player patch set plus an 11-16 campaign-only `LobbyPatches.cpp`
delta; the last multiplayer evidence is 11-15 afternoon. Nothing multiplayer has been touched since.

---

## 2. What currently WORKS (with evidence)

| # | Works | Evidence |
|---|---|---|
| W1 | Pinned engine-limit profile for every install: **Full (V16+particles+light volumes) + 1024 AI actors**, network/layout id `0x03010400` (profile 3, layout gen 1, actors 1024). Required before the 32-slot patch can install at all | `NOTE-PLAYER-COUNTS:8-9`; [TOP10](TOP10-NEXT-UPGRADES-2026-11-15.md):18-32; `git log` `8f0fe85` |
| W2 | **32-player patch installs fully** on that pinned build: **110 slot sites, 23 masks, 139 loop bounds, 8 step loops, 65 peer-table sites + 3 step loops, 6 block copy/clear mirrors, membership message group (16 caves + 26 edits)**, pools raised to 32. Byte-checked all-or-nothing | 11-15 run log `build/bin/Release/host-lan32-high.log:129-135`; `probe-slot32.log:133`; `NOTE-PLAYER-COUNTS:25-29`; `src/host/SlotPatch.cpp:880-892,963-983` |
| W3 | **Pools live at 32**: `players`, `fire team`, `simulation view data array` = 32 records | `probe-slot32.log:1382,1421,1435` (`POOL\|fire team\|0\|32\|212\|live`, `POOL\|players\|0\|32\|1168\|live`, `POOL\|simulation view data array\|0\|32\|280\|live`); `NOTE-PLAYER-COUNTS:28` |
| W4 | **Membership message group runs at runtime**: registered send size is `0xA300` and the producer emits the 32-player payload | `host-lan32-high.log:1141` `SlotPatch: membership message registered size min 0xA300 max 0xA300 (want 0xA300)`; `:1142` `membership producer #1 ... 2 peers, 2 players`; `host-lan32-low.log:1206`; guest decodes it `client-lan32-low.log:780` |
| W5 | **Normal-slot 2-instance LAN join PASSES at membership/engine-table level** (both instances 32-player mode, direct connect `127.0.0.1:11774`): host sees 2 peers/2 players, pmask 3; guest decodes msg 26, acks msg 35; msg 44 stream every about 10 s; host engine game-player table **capacity 0x20 (32), used 2/2**, entries `0:EC70 f00000009 t0` / `1:EC71 f00008008 t1`; `game session started` frame 352 (host) / 380 (guest); 0 crashes; no CLOSE in the host run | host `build/bin/Release/host-lan32-low.log:1205-1206,1214,1235,982`; client `build/bin/Release2/client-lan32-low.log:780-787,1387,1407`; `NOTE-PLAYER-COUNTS:31-47`; status also in [TOP10](TOP10-NEXT-UPGRADES-2026-11-15.md):34-40 |
| W6 | **Single-window 32-player mode smoke test**: installs and runs 120 s clean (1 player; proves install/frames/caves, not 32 players) | `LIMITS-HANDOFF:184-188`; `docs/limits-players-loops.md:204-215`; 11-15 rerun `probe-slot32.log:133` |
| W7 | **MP join-in-progress exists natively** (multiplayer mid-match join seats the joiner: host table `1:EC71 f00008008 t1`; guest receives the full msg-37 config round including its own identity) | `NOTE-PLAYER-COUNTS:558-574` (11-16 01:39-01:43 diagnostics) |
| W8 | **Session slot-count hook works again**: `set_slot_counts` pattern no longer matches the DLL, so it falls back to proven RVA `hr+0x39C5C8` and expands session slots (MP 16->128, co-op capped 16) | `src/hooks/LobbyPatches.cpp:2894-2924`; `NOTE-PLAYER-COUNTS:1232-1250`; commit `1a6846b` |
| W9 | (background, parked) Campaign co-op stock 4 players end-to-end on one machine; campaign guests start in MP engine class, team-0 forcing, exact roster declaration | `NOTE-PLAYER-COUNTS:651-689`; commit `f7a2c74`; current HEAD lifts launch caps for >4 |

**What W5 does NOT prove:** gameplay with two real players (no gameplay capture; the Noble scoreboard overlay showed only
1 row because LAN/direct sessions have no master-server roster - `NOTE-PLAYER-COUNTS:43-47`). There is no log of any
multiplayer session with more than **2 peers** anywhere in the tree.

---

## 3. What currently FAILS (exact evidence)

### F1 - High-slot join (peer 17 / player 16): THE multiplayer blocker. **PROVEN, unresolved.**
Test 2026-11-15 14:29 ET, `build\bin\Release\probe-lan32.ps1 -High` (both instances `-slot-patch -slot-players 32`,
`NOBLE_SLOT_PLAYERS_UNSAFE=1`, `NOBLE_SLOT_HIGH=1`; host log `host-lan32-high.log`, guest `client-lan32-high.log`).

Host sequence (timestamps exact):
```
14:29:23.215  SlotPatch: membership producer #1 payload 0xc53d044a90: 2 peers, 2 players      host:1142
14:29:23.215  LobbyPatches: SEND msg 26 (?) size 41728                                        host:1143
14:29:23.253  LobbyPatches: RECV msg 35 (?) size 6168 decoded true                            host:1158
14:29:23.299  LobbyPatches: player sync in: ... +10 4 ... (changed event)                     host:1169
14:29:23.299  SlotPatch: membership producer #2 payload 0xc53d042540: 1 peers, 1 players      host:1171
14:29:23.299  LobbyPatches: session ... peer states: p0:8 p17:7                              host:1173
14:29:23.326  LobbyPatches: CLOSE channel 0x... reason 10 (state was 5) stack: hr+0x429BEE <- hr+0x405CAE <- hr+0x42379D <- hr+0x3AEA27 <- ...   host:1180
14:29:25.832+ CLOSE channel ... reason 4 (state was 3) repeating                               host:1226,1256,1283,...
```
Guest side:
```
14:29:23.222  RECV msg 26 (?) size 41728 decoded true                                          client:777
14:29:23.222  membership consume (payload 2 peers 2 players) ... after: peermask 00020001 pmask 00010001 local peer 17  client:778
14:29:23.245  SEND msg 35 (?) size 6168; session ... state 4 local peer 17 (record state 6)     client:783-784
14:29:23.253  SlotPatch: 17-entry session parameter ... not set for peer 17 (stopgap)           client:794
14:29:23.312  RECV msg 26 (?) size 41728 decoded true                                          client:810
14:29:23.312  membership consume (payload 1 peers 1 players) ret 0 ... after: peermask 00000002 pmask 00000000 local peer 1   client:811
14:29:23.322  CLOSE channel ... reason 14 (state was 5)                                        client:814
14:29:23.834+ CONNECT REFUSED: id 0x1 reason 3 (0x3) caller hr+0x423E2E (then id 0x2, ...)    client:828-829,839,854,...
```
Guest state while refused: `ReachHost [State]: frame 60 launch 11 map 0 (none)` (`client-lan32-high.log:833`) - it never
gets a map. Both processes stay alive (0 crash; clean rejection).

Doc version: `NOTE-PLAYER-COUNTS:49-57`; [TOP10](TOP10-NEXT-UPGRADES-2026-11-15.md):39-40; [TODO-MASTER](../desktop-guides/TODO-MASTER-2026-11-14.md):423-424; [PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):38-43.
Progress vs 2026-11-10: the guest now sends msg 25/22/35/27/33 and **no msg 32** (`client-lan32-high.log:781-795`;
the 11-10 "guest sends four msg 32 and is dropped" root cause was caved - `LIMITS-HANDOFF:36-67`); the remaining
failure is the **host-side removal of the high-slot player** (`NOTE-PLAYER-COUNTS:56-57`).
Semantics of close reason 10 / connect-refused reason 3 are **not named anywhere in the docs** - **UNCERTAIN** (the
`hr+0x429BEE` close path is the same channel-close used when the engine restarts/tears down a network session,
`NOTE-PLAYER-COUNTS:1171-1185`, but do not assume causation).

### F2 - 32-player session end-to-end: never run. **PROVEN absence.**
Only 2 instances have ever been in one session. The msg-#32 extension paths (`>16 players / >17 peers`) have never been
exercised; only the widened wire *widths* were LAN-tested with 3 players in 16-player mode (`docs/limits-players-message.md:38-42,65-69`;
`OVERNIGHT-2026-11-09.md:15-19`).

### F3 - 16-player multiplayer: never tested with real peers. **PROVEN absence.**
"Multiplayer with real peers" is listed as not covered (`TODO-MASTER:89`; `QA-VERIFY-2026-11-15.md:120-121`);
the "3-instance normal-slot test" is still an open next step (`NOTE-PLAYER-COUNTS:70`). The only MP LAN test is 2 peers (W5).

### F4 - (parked, background) Campaign co-op >4 still fails. **PROVEN.**
5 players: launch reaches "all players ready" then the engine takes the network session-start path and closes every peer
channel with reason 10 (`NOTE-PLAYER-COUNTS:1171-1185,1302-1308`); HEAD's body confirms after the dispatcher-B/launch-info
cap lift "the network session-start path still closes all peer channels (reason 10) at launch, which is the next target"
(`git show -s --format=%B 4bfe827`). Do not spend multiplayer lane time here.

### F5 - Scoreboard/roster UI shows 1 row in LAN/direct sessions. **PROVEN, cosmetic/data-source gap.**
Noble's MATCH ROSTER is fed from the master-server lobby roster; a LAN/direct session has none, so the overlay shows
only the local player even though the engine has 2 (`NOTE-PLAYER-COUNTS:43-47`, captures `lan32_low_host_cap0.png`).
Fix later by feeding session membership into `PauseMenu`.

### F6 - No real multi-machine join has ever been tested. **DOC/absence.**
Two-PC lobby-code join is untested (`HANDOFF-FINAL-2026-11-15.md:97,203`; `TODO-MASTER:89`; `QA-VERIFY-2026-11-15.md:24-25`).
All evidence in this note is one PC, two instances, direct connect.

### F7 - Pinned-limit join gating is incomplete for real peers (risk for multi-machine tests).
`limits_id` is sent/parsed, but backend passthrough is unconfirmed, and lobby-code/direct-connect/friend/campaign join
paths never peek the host's `limits_id`, so a mismatched (old/Stock) client can join a pinned host
([NOTE-NETWORK-HANDOFF-TO-CLAUDE.md](NOTE-NETWORK-HANDOFF-TO-CLAUDE.md):75-89). Two peers must run the same build+layout
(`NOTE-PLAYER-COUNTS:8-9`; `LIMITS-HANDOFF:407-410`).

### F8 - 32-player mode is still dev-gated and unsafe by design.
`-slot-players 32` logs the blockers and refuses unless `NOBLE_SLOT_PLAYERS_UNSAFE=1` (`src/host/SlotPatch.cpp:846-853`);
the log line lists "169 untraced imul sites incl. 6 in 0x390DB0" (`docs/limits-players-loops.md:75-90`). Not a user path.

### F9 - "multiplayer stopped at 34 players" is an unreproduced Tyler report. **UNCERTAIN.**
It is quoted in [NOTE-NETWORK-HANDOFF](NOTE-NETWORK-HANDOFF-TO-CLAUDE.md):14 and still listed as "measure what actually stops
above 32" (`NOTE-PLAYER-COUNTS:71-72`) but no test in the tree has had more than 2 peers; the code's current hard ceiling
is 32 peers because the peer mask is a DWORD (`docs/limits-players-loops.md:19-22`). Do not treat 34 as a measured fact.

---

## 4. The 32-slot patch inventory

Dev entry: `-slot-patch` (or `NOBLE_SLOT_PATCH=1`) + `-slot-players 32` (or `NOBLE_SLOT_PLAYERS=32`) +
`NOBLE_SLOT_PLAYERS_UNSAFE=1`; requires the Extended profile (now pinned Full). All-or-nothing: every byte check must
pass or 32-player mode turns itself off with a log line (`src/host/SlotPatch.cpp:830-892`).
One install log line summarises everything (`host-lan32-high.log:131-135`).

| Group | File / symbol | Count | What it does |
|---|---|---|---|
| Slot address caves | `SlotPatchTable.inc` -> `kSites` | **67** | Redirects `imul 0x158 / 0xB0`-derived player/peer slot addresses for players>=16 / peers>=17 to Noble-owned **shadow session objects** (same stride 0x61668, delta constant); slots below stay at stock addresses (`SlotPatch.hpp:6-10`; `SlotPatch.cpp:50-54`) |
| Player add/remove + peer-player-table caves | `SlotMemberCaves.inc` -> same `kSites` | **43** | Hand-reviewed `add@/not@` sites: player add/remove helpers (0x39D620, 0x39D7F8, 0x39D8A8, 0x39D950), 0x56-stride map tables, 0xB0/0x158 indexing; includes scale-shift forms (`SlotPatch.cpp:52-53`; `docs/limits-players-loops.md:80-83`) |
| **Total redirected sites** | `kSites` | **110** | what the log prints (`SlotPatch.cpp:973-974`) |
| Pointer-stepping loop caves | `SlotStepTable.inc` -> `kSteps` | **8** | `add ptr,record; cmp counter,first` loops: at the 17th/16th entry jump the pointer to the shadow array and run to 32; stock bound kept on non-session blocks (`src/host/SlotStepTable.inc:1-2`; `SlotPatch.cpp:56-58`) |
| Loop/scan bound edits | `SlotBoundsTable.inc` -> `kBounds` | **139** | `cmp reg,0x10/0x11` -> `0x20` byte edits (bit-scans, loops) (`SlotBoundsTable.inc:1-2`; `SlotPatch.cpp:60-62`) |
| Peer masks + register-held bounds | `kPeerMasks32` in `SlotPatch.cpp` | **23** | Nine 17-bit peer masks `0x1FFFF` -> `0xFFFFFFFF`, `0x408F10 test si,si`->`test esi,esi`, game-over team count 15->31 (0x1333A9), game-player event index 4->5 bits (encoder 0x3C7CE0 / decoder 0x3A46FE + bound), register-held bounds 0x392E12/0x409898/0x4139E5, 0x411F38 player scan bounds (`SlotPatch.cpp:64-109`) |
| Grown frames | `kFrame390` / `kFrame396` + unwind copies | 14+3 edits | `0x390DB0` frame +0x100 for a 32-entry name-pointer table (table moved to rbp+0x1F8); `0x396828` frame 0x8F00->0xC540 (plan C side areas) -> **0x10CC0** with the #32 payload extensions; unwind tables rebuilt, GS handler RVA 0x7937E0 (`SlotPatch.cpp:111-181`; `docs/limits-players-loops.md:204-215`) |
| Peer table at session+0x446C | `SlotTableSites.inc` -> `kTableSites` | **65** | Caves for the 17 x 0xC peer table accessed as `peer+0x5B0/0x5B4` with bias folded into the index; non-session blocks left stock and counted; 2 reset-loop step caves + 1 player-count step cave (`SlotPatch.cpp:240-258`; `LIMITS-HANDOFF:162-168`) |
| Whole-block copy/clear mirroring | `kCopySites` + `kStackCopySites` | **6 + 2 + 1** | Redirect memcpy/memset/memcmp call sites: static->static copies mirror shadow slots; network-decoded source clears the destination's shadow slots; `0x396828` S1/S2 side areas always filled/compared (`SlotPatch.cpp:195-216`) |
| Membership message #32 | `SlotMessageTable.inc` -> `SlotMessage.cpp` | **16 caves + 26 byte edits** | Wire type stays **0x1A (26)**; payload grows to **0xA300** (extension at +0x8100: peers 17..31 at +0x18/0x180, players 16..31 at +0x1998/0xB8), counts/peer indices 6 bits, player index 5 bits; frame 0x10CC0; producer call-site stubs build the extension; registered size fixed at runtime if needed (`docs/limits-players-message.md:8-55`; `SlotMessage.cpp:37-95,197-213`; runtime W4) |
| High-slot test mode | `NOBLE_MSG_TEST_CAVES` -> `ApplyHighSlotTest` | **2 caves** | `NOBLE_SLOT_HIGH=1` only: free-slot searches start at **peer 17** (0x3953BC) and **player 16** (0x39D69A), plus decoder owning-peer limit edit 0x4273B0 (16->31) (`SlotMessage.cpp:216-243`; `docs/limits-players-message.md:57-63`) |
| Pool raises | `PoolResize` `kPlayerPools` | 3 | players / fire team / simulation view data array 16 -> 32, only when 32-player mode is active (`NOTE-PLAYER-COUNTS:20-21`; W3) |
| **Stopgap** | `Hook_PeerParamSet` @ `0x3FF588` | 1 | 17 x 16-byte session parameter (session+0x60A60) is not grown; writes for peer>=17 are **skipped** (logs "17-entry session parameter ... not set for peer 17 (stopgap)"). Real fix = grow the parameter to 32 entries **including its stack copies and network encoding** (`SlotPatch.cpp:798-812`; `LIMITS-HANDOFF:22-24`; observed `client-lan32-high.log:794`) |

Gating/order details: 16-player mode is byte-identical to stock (inert) - `SlotPatch.hpp:9-10`, `SlotPatch.cpp:983`;
`NOBLE_SLOT_MSG_OFF=1` leaves the #32 group out for bisection (`SlotPatch.cpp:882-886,966`); high-slot mode is a
test-only add-on and off by default (`OVERNIGHT-2026-11-09.md:64-67`).

---

## 5. Milestone 1 - 16-player multiplayer end-to-end: gap analysis

Acceptance bar ([PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):129-133): 16 players, real peers, match
starts, gameplay, join/rejoin, clean exit; evidence = host+guest logs with 16 players / producer-consumer membership at 16,
plus a gameplay capture.

**What already exists and should carry 16 players without the 32-slot patch:**
- Stock engine capacity is exactly 16 players / 17 peer records; 16 players needs **no shadow work** (0x32 shadow path is
  only for >16) - `docs/limits-players.md:6-17`; `draft-session-networking.md:22-35`; for co-op the note explicitly says
  16 game-player slots / peer ids 0-15 fit the stock masks and "do not need the 32-slot shadow work" (`NOTE-PLAYER-COUNTS:284-285`).
- Lobby/session side is already expanded: `s_maxMultiplayerPlayers = 128` (`src/hooks/LobbyPatches.cpp:59`), session peer
  cap byte patched to 128 (`draft-session-networking.md:120-132`), set_slot_counts hook working (W8), join-in-progress
  overrides in place (`draft-session-networking.md:98-118`), and MP mid-match join proven natively (W7).
- The MAX PLAYERS picker can request 16 today (capped at 32, 16 default: `TODO-MASTER:86-88`).

**Gaps (in likely order of pain):**
1. **Hardware/peers.** One PC holds about 5 instances (5-instance peak 22.2 GB of 31.8 GB); a real 16-player run needs a second
   machine/real peers (`NOTE-PLAYER-COUNTS:92-93,206-209,257-259,284-287`). Current rig density: host + Release2..5.
2. **No scale test at all between 2 and 16.** The documented next step is a 3-instance normal-slot run
   (`NOTE-PLAYER-COUNTS:70`) and it has not been done - so "16 works" is an untested extrapolation (**UNCERTAIN**).
3. **No real multi-machine join path has ever been tested** (two-PC lobby/master-server, exact connect path) - F6.
4. **Same-build/layout gating must hold for every peer** (pinned Full + layout id 0x03010400): real peers must run the
   same 0.19.103 build or the join/desync risk is unverified - F7; `NOTE-PLAYER-COUNTS:8-9`.
5. **Roster/scoreboard UI** shows 1 row in LAN/direct sessions (F5); the in-engine table is correct, the overlay is not.
6. **Verification kit**: no harness currently starts 16 clients; `probe-lan32.ps1` is hardcoded to host+one client
   (`probe-lan32.ps1:1-30`); `mp-jip-test.ps1` exists for MP JIP (2-instance style, `build\bin\Release\mp-jip-test.ps1`).
   `tools\lan-three-instance.ps1` and `tools\overnight-lan3.ps1` are the 3-instance patterns to extend
   (`docs/limits-players-loops.md:227-232`; `OVERNIGHT-2026-11-09.md:52-56`).

**Docs on 16-player testing:** the only planning text is the hardware statement above plus the campaign 16-player plan
(parked, `NOTE-PLAYER-COUNTS:714-734,284-292`). There is **no** synthetic/headless-peer plan for MP; the closest is the
msg-#32 synthetic encode/decode self-test idea for the *wire format* (`docs/limits-players-message.md:65-69`).
`noble-dedicated.exe` is a stub with **no engine session and no P2P transport**, so it cannot stand in as a headless peer
(`draft-session-networking.md:134-143`; `LIMITS-HANDOFF:287`).

**Suggested M1 plan:** 2-PC real join first (same build, limits ids equal); then 4-5 local instances as the inner loop;
then a LAN/community 16-player run. No code blocker is known for 16; treat it as a verification + logistics milestone
with two small code fixes (roster data source, any 16-bound surprises found on the way).

---

## 6. Milestone 2 - high-slot fix + 32 players: gap analysis

Success criteria for the high-slot test ([PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):79-80):
host producer player count stays >= 2 after the ack; guest record reaches state 6+; no CLOSE reason 10; no CONNECT REFUSED
reason 3; match keeps simulating; guest can leave and rejoin.

**Step 0 - reproduce with read-only probes (not done since 11-15):** host-side player add/remove helpers
(0x39D620 add/update, 0x39D7F8 remove wrapper, 0x39D8A8 add, 0x39D950 remove, 0x39CA04.. peer removal), the membership
producer input (which entry disappears), peer promotion scans in 0x392DB0 (`NOTE-PLAYER-COUNTS:63-69`). The latest docs
give an ordered list; nothing has been instrumented since `NOTE-PLAYER-COUNTS` was written.

### Blocker inventory (every known remaining blocker)

| ID | Blocker | Type | Evidence / link | Status / next action |
|---|---|---|---|---|
| B1 | Host removes the high-slot player about 84 ms after the ack; root cause **not proven** (candidate: address math in the add/remove path still reading inline memory for peer 17) | verification -> code | [NOTE-PLAYER-COUNTS:49-57,63-69](../NOTE-PLAYER-COUNTS-2026-11-15.md); [OVERNIGHT-2026-11-09.md:23-29](../../docs/OVERNIGHT-2026-11-09.md); [LIMITS-HANDOFF:77-91](../../LIMITS-HANDOFF.md) | **CRITICAL PATH.** Add host-side read-only traces around the remove path; capture which player/peer table entry disappears |
| B2 | Uncaved player add/remove + peer-player-table sites: 0x395E2F / 0x395E6B / 0x395E85 (in 0x395DE8), 0x39CB0C, 0x39C701 (bound still 17), 0x39CAF7 / 0x39CB69 / 0x39CB7D, plus unreviewed readers 0x39752A, 0x39FD40, 0x44B24C; 169 untraced `imul 0xB0/0x158` sites in 0x380000-0x460000 (incl. 6 in 0x390DB0) | code/table | [LIMITS-HANDOFF:65-67,173-181](../../LIMITS-HANDOFF.md); [docs/limits-players-loops.md:75-90](../../docs/limits-players-loops.md); [docs/limits-players-regbounds.md:73-76](../../docs/limits-players-regbounds.md); warning at `src/host/SlotPatch.cpp:846-853` | Audit the 43 member caves vs the site list; add missing caves with byte checks; this is what keeps `NOBLE_SLOT_PLAYERS_UNSAFE` necessary |
| B3 | 17-entry session parameter at session+0x60A60 left at 17 by the **stopgap**: peer>=17 writes are skipped, and the parameter is also sent over the network | table/protocol | `src/host/SlotPatch.cpp:798-812`; [LIMITS-HANDOFF:22-24](../../LIMITS-HANDOFF.md); runtime `client-lan32-high.log:794` | Grow to 32 entries: storage, the 0x110-byte stack copies in 0x3FF588, getters, and its network encoding. Candidate contributor to B1 (**UNCERTAIN**) |
| B4 | Fixed 16/17-entry tables not grown: 0x41F510 / 0x40A760 16 x 0xA0 singleton; 17 x 6 stack tables 0x41E884 / 0x41F510 / 0x41FBE8; 17 x 0xC at 0x39C6B4 and ctor 0x61574; 0x3FFC9C; 0x3C0248 / 0x41E7C8 6-byte tables; 0xD6B8 host list stays 16 | table | [docs/limits-players-loops.md:41-53](../../docs/limits-players-loops.md); [LIMITS-HANDOFF:173-181](../../LIMITS-HANDOFF.md) | **UNCERTAIN which are on the current failure path**: the 11-09 22:13 note says the 0x41F510 "fixed table" theory was a *symptom* of the shadow peer-table reset bug (`LIMITS-HANDOFF:123-127`), but 0x41F510 is still listed as not grown. Re-verify after B1 probe before growing anything |
| B5 | Message #32 extension paths (>16 players / >17 peers) never exercised; third decode caller 0x442394 decodes into a static buffer (RVA 0x4E3BB80) whose size for 0xA300 is unverified | verification/protocol | [docs/limits-players-message.md:42,65-74](../../docs/limits-players-message.md) | Add the suggested synthetic self-test (fill 32/32 payload, encode+decode through engine wrappers 0x423D88/0x423E40, compare); check 0x442394 at runtime before any >16 test |
| B6 | 32-player end-to-end evidence is impossible on one PC: needs 32 peers; only about 5 instances fit, no headless peers, dedicated build is a stub | verification/infra | [NOTE-PLAYER-COUNTS:206-209,284-287](../NOTE-PLAYER-COUNTS-2026-11-15.md); `draft-session-networking.md:134-143`; `LIMITS-HANDOFF:287` | Plan: fix high-slot with 2 instances -> 3-instance LAN -> 32-player run on a LAN/community event or a synthetic-peer harness (does not exist yet; scoping it is a lead decision) |
| B7 | Hard 32-peer ceiling: peer mask is a DWORD, so 64/128 needs protocol widening (the #32 message already uses 6-bit counts / 5-bit player index = its maximum for 32) | protocol | [docs/limits-players-loops.md:19-22](../../docs/limits-players-loops.md); [docs/limits-players-message.md:32-33](../../docs/limits-players-message.md); `NOTE-PLAYER-COUNTS:71-72` | Out of M2 scope, but it defines "32 players" as the ceiling until a protocol version bump |
| B8 | High-slot join is exercised only by the dev switch `NOBLE_SLOT_HIGH=1` (free-slot scans start at shadow slots); shipping behaviour has no way to put a player in slot >= 16 yet | code | `src/host/SlotMessage.cpp:216-243`; [docs/limits-players-message.md:57-63](../../docs/limits-players-message.md) | Expected: once the join works, the high-slot test mode is how it gets exercised; real 33rd+ joins need a policy for slot assignment later |

Net: **B1+B2 (+B3) are the critical path**; B4 is a conditional; B5/B6 are evidence blockers that will bite immediately
after the join works.

---

## 7. Test rig / facts (from the docs + scripts)

- **Machine/instances:** one Windows PC; up to **5 instances**: host `build\bin\Release`, guests `build\bin\Release2..Release5`
  (copies with their own data folder/device id; exe refresh as in section 1). 1280x720, FXAA off, 30 fps, all audio muted
  via `build\bin\Release\low-spec-settings.json`; windows forced to the secondary AOC monitor; focus guard keeps the
  user's mouse free (`NOTE-PLAYER-COUNTS:87-93`; [PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):82-90).
  Peak RAM for 5 instances: 22.2 GB of 31.8 GB (`NOTE-PLAYER-COUNTS:93`).
- **Do not run the game here** - the lead owns the single rig (task rule; concurrent instances break each other).

| Harness | Where / args | Purpose |
|---|---|---|
| `probe-lan32.ps1` | `build/bin/Release`; `-High` (sets `NOBLE_SLOT_HIGH=1`), `-Capture`, `-Seconds 75`, `-MaxPlayers 32` | The current 32-player LAN probe: host then one client, direct `-connect 127.0.0.1:11774`, writes `host-lan32-<low\|high>.log` / `client-lan32-<low\|high>.log`; prints last matching lines and optional captures. **Does not copy the host exe to Release2** (`probe-lan32.ps1:1-57`) |
| `probe-jip.ps1` | `-Map m10 -Guests N -HostMax 16 -Premission -Mode campaign\|multiplayer -GuestsFirst -CaptureAt` | Multi-instance join probe (campaign JIP default; `-Mode multiplayer` does MP JIP); refreshes every instance exe from the host build (`probe-jip.ps1:1,35-100`; `NOTE-PLAYER-COUNTS:431-434`) |
| `probe-camp5.ps1` | `-Guests 3\|4 -GuestsFirst` | 5-instance campaign probe (parked lane) (`NOTE-PLAYER-COUNTS:1317-1319`) |
| `mp-jip-test.ps1` | `build/bin/Release` | MP join-in-progress test harness (2-instance pattern) |
| `qa-regression.ps1`, `qa-one.ps1`, `stage1-run.ps1`, `probe-limits.ps1` | `build/bin/Release` | General QA / pool / limits probes; not multiplayer-specific |
| `tools\overnight-lan3.ps1` (+ `.bat` wrappers in repo root) | host + Release2 + Release3 | 3-instance LAN test; join criterion = host `SEND msg 26` + `RECV msg 27/35` (`OVERNIGHT-2026-11-09.md:52-56`) |
| `tools\lan-two-instance.ps1`, `tools\lan-three-instance.ps1` | host/client copies | Older LAN rigs; the three-instance one is also the `-trace-members` driver (`docs/limits-players-loops.md:227-232`) |

- **Launch lines used by the 11-15 LAN test** (`probe-lan32.ps1:19,23`):
  host `noble-dev.exe -slot-patch -slot-players 32 -host -lan -private -max-players 32 -map 20_sword_slayer -mode multiplayer -log host-lan32-low.log`;
  guest `noble-dev.exe -slot-patch -slot-players 32 -lan -connect 127.0.0.1:11774 -map 20_sword_slayer -mode multiplayer -log client-lan32-low.log`.
  Campaign: host `-host -lan -private -max-players 5 -map m10\|m50 -mode campaign -difficulty 1 -wait-peers 4`,
  guests `-connect 127.0.0.1:11774 -lan -map <same> -mode campaign -difficulty 1` (`NOTE-PLAYER-COUNTS:95-96`).
- **Env knobs that matter** (all `NOBLE_*`, read at startup): `NOBLE_WINDOW_SECONDARY=1` (AOC monitor,
  `src/host/Window.cpp`), `NOBLE_SLOT_PATCH=1`, `NOBLE_SLOT_PLAYERS=32`, `NOBLE_SLOT_PLAYERS_UNSAFE=1`,
  `NOBLE_SLOT_HIGH=1`, `NOBLE_SLOT_MSG_OFF=1`, `NOBLE_MACHINE_ID`, `NOBLE_TRACE_MEMBERS`, `NOBLE_JIP_PROBE`,
  `NOBLE_JIP_DUMP`, and the `NOBLE_COOP_*` set (campaign launch/barrier probes). Source: `grep NOBLE_ src/`
  (hits in `src/host/SlotPatch.cpp`, `src/host/SlotMessage.cpp`, `src/hooks/LobbyPatches.cpp`, `src/engine/BlamEngine.cpp`).
- **Logs:** `-log <file>` writes next to the exe, e.g. `build\bin\Release\host-lan32-low.log`,
  `build\bin\Release2\client-lan32-low.log`, ...`-high.log`, `build\bin\Release\probe-slot32.log`,
  `host-jip.log`/`client2-jip.log`, `host-camp5.log`; default runtime log is `noble-dev.log` (overwritten per launch;
  the host `noble-dev.log` is already 108 MB - rotate before a long run). `-capture-at N` / `-scoreboard-at N` save
  `capture_*.bmp` next to the exe; captures are converted to `build\bin\Release\data\userdata\claude-tests-2026-11-11\lan32_*.png`
  (`probe-lan32.ps1:31-51`; `TODO-MASTER:189-192`).
- **How to read a result:**
  - *Join worked (normal slot):* host has `SEND msg 26` + `RECV msg 27/35`, producer shows 2 peers/2 players, guest
    `RECV msg 26 ... decoded true` + `SEND msg 35`, both `game session started at frame N` (frame >0), guest
    `launch 11 map 2 (running)`, msg 44 stream. Low log reference: W5.
  - *Join did not happen:* only msgs 8/22/25 and no 26/27/35 (`LIMITS-HANDOFF:140-141`).
  - *High-slot pass (new criteria):* host producer stays >= 2 players after the ack, guest record state >= 6, no CLOSE
    reason 10, no CONNECT REFUSED reason 3, match keeps simulating, guest can leave/rejoin
    ([PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):79-80).
  - *Fail signatures:* `CLOSE ... reason 10`, `CONNECT REFUSED ... reason 3 (hr+0x423E2E)`, `map 0 (none)` on the
    guest, `FATAL EXCEPTION`/`0xC0000005`/crash dump in the run folder.
- **Captures/artifacts from the 11-15 LAN test:** `NOTE-PLAYER-COUNTS:1273-1278` (host/client logs, probe-slot32.log).
- **Focus guard:** probe scripts return the foreground to the user whenever a game window grabs it
  (`NOTE-PLAYER-COUNTS:1317-1319`); keep that behaviour.

---

## 8. Contradictions / stale claims (know these before trusting a doc)

| # | Claim (where) | Newer / truth | Verdict |
|---|---|---|---|
| C1 | "membership message **#32**" ([TOP10](TOP10-NEXT-UPGRADES-2026-11-15.md):36; [PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):40) vs "msg 26 size **41728** (0xA300)" (`NOTE-PLAYER-COUNTS:36`; logs) | Both are right: the wire type is **0x1A = 26** (stock membership-update); "#32" is the project label for the 32-player variant; 0xA300 is the payload/send size. See `src/host/SlotMessage.hpp:5-7`; `docs/limits-players-message.md:11-14` | Clarify in future notes; do not search for a message type 32 |
| C2 | "32-slot patches are built, **not tested**" ([NOTE-NETWORK-HANDOFF](NOTE-NETWORK-HANDOFF-TO-CLAUDE.md):18, written 11-15 11:35) | The 11-15 14:35 tests install the full set and the 2-instance LAN join passes (section 2). The handoff note was not updated afterwards | Stale; ignore C2 |
| C3 | "host adds player 16 and removes it **16 ms later**" (`OVERNIGHT-2026-11-09.md:25`; `docs/limits-players-message.md:61`) | The 11-15 run: producer #1 14:29:23.215 -> producer #2 14:29:23.299 = **about 84 ms** (host log lines 1142/1171). Behaviour is the same; the "16 ms" figure is from the 11-09 run | Timing stale, symptom current |
| C4 | Networking is Claude's / out of scope for DeepSeek ([NOTE-NETWORK-HANDOFF](NOTE-NETWORK-HANDOFF-TO-CLAUDE.md):1-9) | Section 7 of the same file (11-15 14:20) lifts the 128 hold and assigns session/membership/replication to DeepSeek; the 11-16 prompt repeats "multiplayer first" | C4 superseded by its own section 7 |
| C5 | "Multiplayer 128 on Tyler's hold" ([`HANDOFF-FINAL-2026-11-15.md`](../HANDOFF-FINAL-2026-11-15.md):21,130; [WORKFLOW-LEDGER](WORKFLOW-LEDGER-2026-11-15.md):23) | Hold lifted 2026-11-15 14:20 (`NOTE-NETWORK-HANDOFF:98-102`); [TODO-MASTER](../desktop-guides/TODO-MASTER-2026-11-14.md):1-21 now puts multiplayer first | Stale |
| C6 | "Joining another player's game is not possible yet (engine net slots 41/43 not implemented)" (`HANDOFF.md:1944`) | Net slots 41/43 were implemented 2026-10-28 and LAN joins work (W5); that line is from the 2026-10-27 status table | Very stale; HANDOFF.md is a general Oct project handoff, not current MP state |
| C7 | `HANDOFF.md` §1 paths: repo `D:\Halo-Reach-Community-Edition`, RR install `D:\Reach` (`HANDOFF.md:1845,1856`) | Actual repo is `D:\Project-Noble-Closed-Source` (task brief) and RR install is `C:\Users\tyler\Desktop\ReachRenaissance` ([PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):97-101). HANDOFF §2's build commands also target the old checkout | Do not follow HANDOFF.md paths; use this note / the prompt |
| C8 | "Nothing above has been started" ([TOP10](TOP10-NEXT-UPGRADES-2026-11-15.md):106) | Contradicts the same file's status blocks (items #1/#2 complete, #3 in progress) and the revised order at the top: the last line is stale boilerplate | Ignore line 106 |
| C9 | "multiplayer stopped at **34 players**" ([NOTE-NETWORK-HANDOFF](NOTE-NETWORK-HANDOFF-TO-CLAUDE.md):14; `TOP10:55`) | Never reproduced in the tree; current code ceiling is 32 peers (DWORD mask) and no >2-peer test exists (F9). Might be an old rendering/slot report | **UNCERTAIN**; measure before believing |
| C10 | Two copies of the player-count note: `knowledge\NOTE-PLAYER-COUNTS-2026-11-15.md` (1323 lines, mtime 11-16 10:29) vs `knowledge\project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md` (1278 lines, 11-16 08:53) | The root copy has the extra final section "2026-11-16 - 5-player campaign co-op: dispatcher-B and launch-info caps lifted" (root lines 1280-1323); the project-docs copy stops at line 1278. **Read the root copy** | Docs out of sync; do not mirror the old one |
| C11 | Campaign 16-player "all five formed the same co-op session" (`TOP10:41-45`) vs "mission did not continue" (same paragraph) | Session formation only, not in-mission 5-player; the latest campaign work (HEAD) lifts two launch caps but the network session-start path still closes channels | Background; do not count as 16-player campaign working |

Also: the COMPENDIUM mirror (`C:\Users\tyler\Desktop\Compendium`) is documented as a stale mirror of `knowledge/`
(`TODO-MASTER:219-220`); this note does not touch it.

---

## 9. Where to look next (ranked for the lead)

1. **Re-run the high-slot repro on the current build and instrument it read-only.** Refresh `Release2..5` exes, run
   `probe-lan32.ps1 -High -Capture`, add host-side traces around the player add/remove helpers and membership producer
   (`NOTE-PLAYER-COUNTS:63-69`); capture which table entry disappears. This is the only path to M2; everything else is
   support work.
2. **Audit B2 caves against the site list** (`docs\limits-players-sites.json`, `docs\limits-players-review.md`; uncaved
   list in `docs/limits-players-regbounds.md:73-76` and `LIMITS-HANDOFF:65-67`); add the missing caves with byte checks
   (`src/host/SlotMemberCaves.inc` pattern) and re-run the high-slot probe after each batch.
3. **Grow the 17-entry session parameter (B3) properly** (session+0x60A60, stack copies at 0x3FF588, network encoding) and
   remove the stopgap; watch the "not set for peer 17" log disappear.
4. **Then re-check B4 tables** only if the probe points at them (0x41F510/0x40A760 first; the 11-09 note warns the earlier
   theory was a symptom).
5. **Build the msg-#32 extension self-test (B5)** before any >16-player attempt: synthetic 32/32 payload through the
   engine encode/decode wrappers (0x423D88/0x423E40), compare round-trip; then check the third decode caller 0x442394 /
   static buffer 0x4E3BB80 at runtime. This de-risks the one part of the wire protocol that has never run.
6. **Plan the real-peer evidence path for M1/M2** (B6): two machines first; document the exact join path (direct connect vs
   lobby code vs master server) and the required identical build/layout id; decide whether a synthetic-peer harness or a
   community LAN event is the route to 16/32 players.
7. **Verify the pinned-limit join gating on real peers** (F7): backend `limits_id` passthrough, and add the check to the
   join paths that never peek the host (lobby code, direct connect, friend invite) - a mismatch will desync
   ([NOTE-NETWORK-HANDOFF](NOTE-NETWORK-HANDOFF-TO-CLAUDE.md):75-89).
8. **Small UI fix: feed LAN session membership into the scoreboard roster** (F5; `NOTE-PLAYER-COUNTS:43-47`) so captures
   from real runs are honest.
9. **RR reference read (time-boxed):** Tyler keeps a full Reach Renaissance build v0.1.50 (claimed 88 players) as an
   existence proof/UX reference; RE read-only is allowed, do not copy code/assets
   ([PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):92-118). Use it for the session/join/reconnect
   design, then implement independently.
10. **Retail-named RE map:** use [TOPIC-networking-multiplayer.md](../decompile-exports/topics/TOPIC-networking-multiplayer.md)
    (245 source modules, retail names, join/leave + host-migration references) to name the PC RVAs via Ghidra MCP when
    the add/remove trace needs engine context. Also `knowledge/decompile-exports/topics/mcc-address-table.md` for proven
    addresses.
11. **Keep the lane clean:** 128-player hold/go-ahead and any release are Tyler decisions; campaign >4 stays parked; do not
    change the pinned limits profile.
    (`TODO-MASTER:1-21`; [PROMPT-MULTIPLAYER-AGENT](PROMPT-MULTIPLAYER-AGENT-2026-11-16.md):120-127)

---

## Appendix A - older engine-research docs, 2-3 lines each

- `docs\limits-players.md` (80 lines, 2026-11-08 DeepSeek): the original 16-slot inventory - `players` 16 x 0x490 at
  RVA 0x43474 (count imm 0x43470), `fire team` 16 x 0xD4 at 0x122E58, simulation view 16 x 0x118, session/network
  RVAs, and the 16->32 (2-4 days) ->64 (1-2 weeks) ->128 (2-4 weeks) milestone estimate (`:6-17,29-43,62-71`).
- `docs\limits-players-review.md` (91 lines): static review of the 58 remaining slot-address sites from the run-trace;
  26 class A (non-slot), 17 B (covered by imul caves), 15 C (pointer from arg/call; producers named), 0 D (no new shape).
  Conclusion: extend the existing loops, no new detour shape needed (`:12-16,77-90`).
- `docs\limits-players-audit.md` (241 lines): 426 functions load the players pool; 33 compare against 16/0xF; ranked
  candidate table (0x41ED54, 0x14F430, ...). The starting list for per-player fixed tables (`:1-45`).
- `docs\limits-players-bounds-review.md` (301 lines): reviews the 60 loop entries in `limits-players-bounds.json` -
  which loops only need the compare, static .data objects with shadow storage, the 6-byte tables at object+0x34 and the
  step-site formats used by the generated caves.
- `docs\limits-players-loops.md` (271 lines): the C12 step-2 design/status doc - 8 step loops installed, 16 blocked,
  137 bounds raised + 41 kept, the open fixed 16/17 tables table, the 169 untraced imul blocker, the one-window smoke test
  (32 mode installs, 120 s clean) and the 3-instance `-trace-members` result (26 real slot accesses, 504-known list)
  (`:19-90,204-246`). This is the best "what is still 16/17" reference.
- `docs\limits-players-message.md` (74 lines): the #32 membership-message design - wire type 0x1A, payload 0x2550 ->
  0xA300, extension at C=0x8100 (peers 17..31 @0x180, players 16..31 @0xB8), Dpeer 0x6768 / Dplayer 0x7268, 16 caves +
  26 edits, 0x396828 frame 0x10CC0; status "works with 3 players, extension paths never exercised"; risks incl. the
  third decode caller static buffer (`:8-74`).
- `docs\limits-players-regbounds.md` (100 lines): the missed class of **register-held** 16/17 bounds (the generator only
  patched `cmp reg,imm`); batch 1 (0x392E12, 0x409898, 0x4139E5) now in `kPeerMasks32`, five more listed as needing a
  cave/bigger local, and the uncaved join-path sites 0x391F0C/0x395DE8 (`:22-100`).
- `docs\OVERNIGHT-2026-11-09.md` (67 lines): the 11-09 overnight - #32 message LAN-tested OK with 3 players; high-slot
  join fails by host adding player 16 then removing it (about 16 ms; strong lead = uncaved add/remove sites); later the host
  match-end was traced and the host-side bug fixed via `ResetShadowPeerPlayerTables`; guest still never replied, later
  it did but the host removes the player (`:7-63`).
- `LIMITS-HANDOFF.md` (463 lines, root): the 2026-11-08/10 C12-L handoff with the whole step-by-step history of the
  32-slot work, the C12 design (shadow sessions at 0x2B07418 + k*0x61668, trace 417 accesses/136 sites), and the ordered
  blocker list ("still blocking a real 32-player game", `:173-181`). Read the top status section (`:6-141`) for the
  high-slot history.
- `LIMITS-TODO.md` (172 lines, root): the C12-L ordered next steps: batch 1 high-slot test, five more register bounds,
  uncaved join-path sites, failed-join cleanup, "real >16 test" and the final review; also states the engine player pool
  is 16 and Noble's lobby accepts 128 (`:9-16,83-84`).
- `HANDOFF.md` (2253 lines, root, last updated 11-14): the general Oct-2026 project handoff (build/run commands, engine
  facts, launch-block layout, older netcode fixes 10-28/29). Useful background but its paths/repo name are stale
  (C7) and its "can't join yet" line is ancient (C6); use §2/§5 only for old build/launch-block facts.
- `knowledge\drafts\draft-session-networking.md` (168 lines, 2026-11-14): the best single reference for the two player
  tables, 16/32/64/128 mask math, the membership message format, the wire framing/machine-id header, the join-request
  path and Noble's join-in-progress overrides, the dedicated-server gap, the master-server/relay transport and open
  questions (`:17-168`).

## Appendix B - Git ground truth (slot-patch history; all ancestors of HEAD)

`git log --date=short --pretty=format:"%h %ad %s" -- src/host/Slot*` (all in `wip-merge` @ 4bfe827):

| Commit | Date | Subject |
|---|---|---|
| `fb1aa73` | 2026-11-08 | C12 step 1: SlotPatch - 67 slot address sites redirect players >= 16 / peers >= 17 to shadow sessions (inert at 16; two-instance LAN regression passed) |
| `d107dc0` | 2026-11-08 | C12 step 2: loop step caves and 32-player bounds (-slot-players 32, dev) |
| `1d07952` | 2026-11-09 | C12-L step 1: review the last 11 step loops - 2 caves installed, 8 blocked by own fixed tables |
| `d797cb0` | 2026-11-09 | C12-L step 3: grow 0x390DB0's frame for a 32-entry name-pointer table |
| `45d66df` | 2026-11-09 | C12-L: mirror whole-block copies/clears onto the shadow slots |
| `cf10d81` | 2026-11-09 | C12-L plan C: 0x396828 S1/S2 peer side areas; -slot-players 32 gated behind NOBLE_SLOT_PLAYERS_UNSAFE=1 |
| `5642019` | 2026-11-09 | C12-L step 4: players / fire team / simulation view pools to 32; host list left at 16 |
| `49031f3` | 2026-11-09 | C12-L (c): cave the 17 x 0xC peer table at session+0x446C (47 operands + 2 reset loops), shadow entries, block copy/clear mirroring |
| `73f4f4e` | 2026-11-09 | C12-L: fix GS-handler RVA in both grown-frame unwind tables; smoke test 32-player mode installs and runs 120 s clean on Full |
| `a06e24c` | 2026-11-09 | C12-L (d) trace 1: cave 18 biased peer-table sites, step cave for the 0x2A3870 player count, 0x39DD4C reviewed (blocked by #32) |
| `a06a8da` | 2026-11-10 | WIP local changes before merging origin/main (do not push) |
| `86d6b61` | 2026-11-10 | Merge origin/main (0.19.46) + Armory scanner fix, crash logging, mod fixes, 32-player work (paused) |

After `86d6b61` no `src/host/Slot*` commit exists in HEAD. Later commits that touch the multiplayer lane indirectly are
`1a6846b` (2026-11-16 08:43, "Campaign: restore session slot-count hook and fix campaign slot expansion" - the
`set_slot_counts` fallback to hr+0x39C5C8 that W8 depends on) and `4bfe827` (HEAD; campaign caps only).
Branch state: `wip-merge` = HEAD; no tags/releases from this work; version 0.19.103.
