# Player-count expansion (top-10 item #3): first 32-player milestone + high-slot blocker

Date: 2026-11-15 14:20-14:35 ET. Owner: DeepSeek (lead). Version 0.19.103; no source changes yet for this item
(only test harness + docs) - the existing slot patches are activated by item #1's pinned profile.

Context: Tyler chose item #3 and answered "Lift hold, DeepSeek takes everything" - the **128-player hold is LIFTED**
and the session/membership/replication work that was Claude's lane is now DeepSeek's for this item (Claude unavailable).
Security stays out of scope. Current committed baseline: `8f0fe85` (pinned Full + 1024 actors; squad 512; objectives 125;
layout generation 1).

## What the existing code is

From the 2026-11-08/10 C12 work (docs/limits-players*.md, src/host/SlotPatch.*, SlotMessage.*):
- `SlotPatch` redirects 110 player/peer slot address sites past the stock arrays into Noble-owned shadow session
  objects, widens 23 peer/team masks to 32 bits, raises 139 loop bounds, grows the 0x396828 membership-message frame,
  and patches the new 32-player membership message (#32: payload 0xA300, wire counts 6 bits, player index 5 bits).
- It was gated three ways: needs an Extended profile (`SlotPatch: -slot-patch needs ENGINE LIMITS set to Extended`),
  needs `-slot-patch`, and `-slot-players 32` needs the dev override `NOBLE_SLOT_PLAYERS_UNSAFE=1`. Item #1's pinned
  Full profile removes the first gate; the other two remain dev-only.
- Pools follow only when 32-player mode installs (`PoolResize::kPlayerPools`): players, fire team, simulation view
  data array 16 -> 32.

## Tests this session (pinned build, AOC monitor, 0 crashes in every run)

1. **Single-window 32-player install - PASS.** `-slot-patch -slot-players 32` + `NOBLE_SLOT_PLAYERS_UNSAFE=1`:
   - `SlotPatch: 23 peer / team masks widened to 32 ... companion message #32 ... 110 slot address sites redirected`
   - `SlotPatch: 32-player mode (dev): 8 step loops, 139 loop bounds raised to 32, ... peer table +0x446C: 65 sites caved`
   - live pool report: `POOL|players|1|32|1168`, `POOL|fire team|0|32|212`, `POOL|simulation view data array|0|32|280`
   - Sword Base Slayer renders, 0 crash. (First time the install has been exercised since the profile pin.)

2. **Two-instance LAN join, normal slots - PASS at membership level.** Host `build\bin\Release`, client
   `build\bin\Release2` (exe refreshed to the current build), both with `-slot-patch -slot-players 32` + the unsafe
   override, direct 127.0.0.1:11774 connect:
   - host: `membership producer #1 ... 2 peers, 2 players`, session `pmask 00000003 peermask 00000003`,
     peer states `p0:8 p1:5` -> `p1:6` -> `p1:8` (guest registered as player 1);
   - host sends `msg 26 size 41728` (0xA300 = the extended 32-player membership message), guest decodes it
     (`RECV msg 26 ... decoded true`) and answers `msg 35`, host receives `msg 35` repeatedly;
   - `msg 44 size 53752` flows host -> guest every ~10 s for the whole run; both processes alive; 0 crash.
   - Evidence: `build\bin\Release\host-lan32-low.log`, `build\bin\Release2\client-lan32-low.log`.
   - **Engine confirmation:** at the join the host logs its game-player datum table: capacity `0x20 = 32`, used
     `2/2`, entries `0:EC70 f00000009 t0 | 1:EC71 f00008008 t1` - i.e. the guest really is game player index 1 in the
     host's simulation, with the player array at the raised 32 capacity.
   - **UI caveat (new follow-up):** a `-scoreboard-at` capture added to the LAN harness shows Noble's MATCH ROSTER with
     only the local player. Noble's scoreboard roster is fed from the master-server lobby roster, and a direct/LAN
     session has none, so the overlay shows 1 row even though the engine has 2 players. This is a Noble UI data-source
     gap for LAN/direct sessions, not a join failure; feed the session membership/roster into `PauseMenu` for LAN
     sessions as a small follow-up. Captures: `lan32_low_host_cap0.png` (Noble roster, 1 row), `lan32_low_client_cap0.png`.

3. **Two-instance LAN join, high slots (`NOBLE_SLOT_HIGH=1` -> peer 17 / player 16) - FAIL (known blocker).**
   - host: producer #1 `2 peers, 2 players`, SEND msg 26 41728, RECV msg 35 (guest ack), **immediately** producer
     #2 `1 peer, 1 player` -> the guest's high-slot player is removed; then `CLOSE channel reason 10 (state was 5)`
     and a repeating `CLOSE reason 4 (state 3)` loop;
   - guest: `CONNECT REFUSED: id 0x7..0xA reason 3 (hr+0x423E2E)` repeatedly, falls back to `map 0 (none)` (left the
     session); 0 crash (clean rejection).
   - Evidence: `host-lan32-high.log`, `client-lan32-high.log`.
   - Progress vs 2026-11-10: the guest now **does** send `msg 35` (then it never did); the remaining failure is the
     host-side removal of the high-slot player.

Harness notes: `build\bin\Release\probe-lan32.ps1` (test-only, untracked) starts host+client, sets
`NOBLE_WINDOW_SECONDARY=1` so both windows open on the AOC (never the Samsung), and writes low/high logs. Client
instance `build\bin\Release2` has its `noble-dev.exe` refreshed to the current build. `-trace-members` is not used.

## Next steps (in order)

1. **Diagnose the host-side high-slot removal** (the current blocker). Dev trace the player add/remove helpers
   (`0x39D620` add/update, `0x39D7F8` remove wrapper, `0x39D8A8` add, `0x39D950` remove, `0x39CA04..` peer removal)
   and the membership producer input (which player/peer table entry disappears), plus the peer promotion scans in
   `0x392DB0`. The 11-10 notes list uncaved `add@/not@` and `imul 0x56` peer-player-table sites; check which are caved
   now (SlotPatchTable/SlotMemberCaves) and cave the missing ones with byte checks. Re-run the high-slot probe.
2. **3-instance test** on the same harness (tools/lan-three-instance.ps1 pattern, host + 2 clients) for normal slots.
3. **34-player stop**: measure what actually stops above 32 - peer mask is a DWORD (32 peers), players/peer indices in
   message #32 are 6/5 bits; confirm the first refusal reason and document the protocol step needed for >32.
4. **Campaign co-op > 4**: the UI row is clamped to 4 (`b2065bd`); `BlamEngine` already creates a capacity-8 session
   when the roster is over 4 and `LobbyPatches` overrides the co-op insertion refusal/boot paths. Test 5 players
   (local multi-instance is heavy - may need Tyler/real peers), then unclamp the row with a safe maximum.
5. **32 -> 64 -> 128**: only after 32 is clean end-to-end (join, leave, host quit, save/film, split-screen). Protocol
   masks/ids widening per master A5; keep the pinned layout generation in mind.

## Open questions / needs Tyler

- ~~In-game confirmation of the low-slot guest~~ done: the host's engine game-player table shows 2/32 players; the
  Noble scoreboard overlay shows 1 row only because LAN sessions have no master-server roster (new UI follow-up above).
- A 5-player campaign test needs either real peers or 5 local instances (memory heavy). Tell me which to attempt.



## Campaign co-op >4 with 5 local instances (2026-11-15 14:45-15:20 ET)

Setup (all on the AOC, `NOBLE_WINDOW_SECONDARY=1`; no source changes): host `build\bin\Release`,
guests `build\bin\Release2..Release5` (own data folder with junctions, own unique device id, refreshed
`noble-dev.exe`), all five on the same **low-spec settings template**
(`build\bin\Release\low-spec-settings.json`: 1280x720 render, FXAA off, menu supersampling off, 30 fps cap, all
audio muted). Peak total working set 22.2 GB of 31.8 GB - workable. Test harness `build\bin\Release\probe-camp5.ps1`.

Launch path: host `-host -lan -private -max-players 5 -map <m10|m50> -mode campaign -difficulty 1 -wait-peers 4`;
guests `-connect 127.0.0.1:11774 -lan -map <same> -mode campaign -difficulty 1`.

What is proven working:
- Host logs **`BlamEngine: co-op roster is over 4; creating the session with capacity 8 (0xE8)`** - the >4 session
  path from v0.19.95 works.
- `LobbyPatches` co-op expansion is active: **5 `Co-op player cap gate` patches** (`hr+0x40954A/40955B/411B6E/411D71/397689`)
  and `Lobby expansion active (Target MP: 128, Co-op: 16, Active Detours/Patches: 17)`.
- **m10 (Winter Contingency): 4 players joined** - host + 3 guests, host session `pmask 0x0F`, engine game-player table
  `4/16` (`0:EC70 f00002001 t0 ... 3:EC73 f00002001 t3`); 0 crashes in all five logs. A 4-player campaign co-op
  session runs on the current build.
- m50 (Exodus) first attempt: 2 players joined (host + 1 guest), 0 crashes; the two earliest guests hit
  **`external_launch_individual_state_timeout`** (engine event reason 4) ~62 s after launch, and the last guest was
  still loading the map when the host quit.

Key finding - the limiting factor for the 5th seat is the launch window, not a hard cap:
- Campaign co-op has **no join-in-progress**: the host with `-wait-peers 4` waits in the pre-mission window and then
  **aborts the launch (`EngineEvent reason=2`, overall timeout ~85 s after engine start)** if the expected peers have
  not all arrived. A test that delayed the guests until the host was "in mission" failed by construction (host aborted,
  all guests timed out).
- With 5 instances on one PC the guest map load (especially m50/Exodus) takes long enough to race the wait window, so
  guests either miss it (`reason 2` on the host) or wait for a host that has already given up (`reason 4` on the guest).
- m10's lighter load let 3 guests in within the window; the 4th missed it.

Next steps for campaign co-op >4:
1. Test through the **pre-game lobby** flow instead of direct CLI launch (host lobby with 4 peers, everyone joins, then
   start the mission) - that is the intended path and removes the load-vs-window race. Needs a CLI/harness path for
   "host lobby, wait for peers, then start" (FrontEndMenu `-wait-peers` + lobby demo hooks exist; a small dev harness
   action is the cleanest).
2. Alternatively find and extend the pre-mission wait timeout (the reason-2 path) so slow loading guests can still join;
   identify the constant behind `external_launch_overall_timeout`.
3. Once all five are in, test the mission: insert/removal, respawns, save/checkpoint, performance; then unclamp the
   campaign MAX PLAYERS UI row (currently 4-only from `b2065bd`) with the proven maximum.

Final attempt (all four guests launched promptly into the wait window, 6 s apart):
- **All five players joined the same campaign co-op session on m50:** host session `pmask 0x1F` / `peermask 0x1F`,
  peer states `p0:8 p1:8 p2:8 p3:8 p4:8`, and each of the four guests decoded the membership message 15 times
  (`RECV msg 26 ... decoded true`) with **0 `external_launch_individual_state_timeout` and 0 crashes** in all logs.
  This is the first 5-player campaign session formation on the current build.
- **But the mission did not continue.** Right after the last peer reached state 8 (15:08:18), every instance closed its
  channels (`CLOSE reason 14`) and exited via `ReachHost [Slot 4 - EngineEvent]: reason=0` +
  `FreeLibraryAndExitThread`; no relaunched process and no new log appeared, and the host never left launch state 8
  (`LoadProgress 0.0%`). So 5-player **session formation is proven; 5-player in-mission on Exodus is not yet**.
- For comparison, m10 with 4 players reached a live session (host state 6, engine game-player table 4/16) and did not
  exit at the peer barrier.

Next investigation: the `EngineEvent reason=0` exit at the all-peers-ready point on m50. Candidates: the host's
external-launch/peer-barrier handoff (`BlamEngine::LaunchMap` peersToWaitFor / declared capacity 8 path) expecting a
container relaunch, or an m50-specific launch-state transition. Instrument the launch-state machine (Stage1/2/3 hooks
already exist) and compare a 4-player m50 run with the 5-player one; then re-test in-mission 5-player gameplay.



## Campaign join-in-progress probe (2026-11-15 15:16-15:20 ET)

Tyler asked to create a campaign JIP system and re-run the 5-player tests. Read-only probe first (no code changes):
host starts **m10 solo** (no `-wait-peers`, so no pre-mission barrier), then four guests are launched one at a time
with `-connect 127.0.0.1:11774`. Harness `build\bin\Release\probe-jip.ps1`.

**The JIP join path already exists and works** (upstream work in `LobbyPatches`: can_accept_join_request /
can_accept_player_join_request overrides, boot-peer keep, `evaluate_coop_insertion_point` override; `BlamEngine`:
host pre-allocated slots 1..15, guest campaign command 10/11 + host-simulation sync):
- All four guests joined the running mission one after another (`RECV msg 26` true), 0 crashes.
- The host's engine game-player table reached **5 players / capacity 16**:
  `0:EC70 f00000001 t0 | 1:EC75 f00008001 t1 | 2:EC72 f00000002 t2 | 3:EC73 f00000001 t3 | 4:EC74 f00000002 t1`.

**The blocker is team assignment, not the join:** the joiners are placed on **separate teams** (`t1/t2/t3/t1`) instead
of campaign's team 0, so the host's game engine sees `team count: alive 3 teams 3`, fires `Slot 6 - GameResults` on the
guests and the guests exit (`EngineEvent reason=0` ~1-2 s after joining). The host keeps the peers (boot suppression
works; `BOOT peer 1 reason 9` is logged and suppressed).
- Guest logs: joined at `session state 6` with `pmask` set, then `CLOSE channel reason 14`, `Slot 6 - GameResults`,
  `EngineEvent reason=0`, exit.
- So the missing piece for a usable campaign JIP is **forcing every mid-mission joiner onto the host's team (team 0)
  and making the game engine's team/game-over state follow**, plus suppressing the guest-side game-results exit.

Fix plan (next code change - engine-level, one change at a time):
1. Locate the MCC team-change path from the retail names in Phase 3 (game_engine_player_prepare_to_change_team /
   game_engine_player_changed_teams / player_set_team_by_configuration / set_team_for_object); match via Ghidra MCP
   using shared strings/constants, record the hr+RVA in `mcc-address-table.md`.
2. On campaign JIP (`s_campaignCoop` true), force the joining player's team to the host's team (0) as soon as its game
   player datum exists, using the engine's own team-change function (not a raw write) so team counts stay consistent.
3. If the guest still gets a game-results event from the join transition, suppress it for campaign JIP (guest side only
   while the JIP handshake is settling) - same pattern as the existing abort/boot suppressions.
4. Re-run `probe-jip.ps1` (m10, then m50/Exodus) with four guests: pass = all five in the engine player table with
   `t0`, no GameResults exit, guests stay for >=60 s, 0 crashes. Then re-run the 5-player campaign test and the
   4-player pre-mission comparison.


### Team fix implemented (2026-11-15 15:22-15:30 ET, uncommitted)

Change: `LobbyPatches::ForceCampaignPlayerTeams()` runs from the session->game player sync hook and the team-count hook
while `s_campaignCoop` is set; it walks the game-player datum array (TLS `hr+0xC17B18` -> block+0x18 -> arr, data
arr+0x50, stride arr+0x20) and forces **every present player's team (int8 at +0xAD) to 0**. Logs
`campaign JIP: forcing game player N from team T to 0`.

Test (host solo m10, 2 sequential mid-mission guests, `probe-jip.ps1 -Guests 2 -HostMax 16`):
- **Team fix works:** host table now `0:EC70 ... t0 | 1:EC75 ... t0 | 2:EC72 ... t0`, all players team 0; guests
  joined and **stayed 25 s** (previously they got GameResults and exited in 1-2 s), 0 crashes, 0 game-results/engine-exit
  on the guest that stayed in-session.
- **Remaining blocker:** the guest still leaves after ~30 s. Cause found in the guest logs: its first process runs its
  own campaign launch, then `AbortGame(reason=6) from hr+0x3AA35D`, autosaves, gets `PauseMenu: game over` and
  `Relaunch: new window up`; the **relaunched process starts its own host session** (`[HOST pid 2376]` in the guest's
  `noble-dev.log`) instead of joining the real host - i.e. the campaign JIP guest relaunch loses the join target
  (`Relaunch::SetCurrentGameArgs` / guest auto-return-to-lobby path does not carry `-connect/-mode` correctly).
- Host side: keeps the peer (BOOT reason 9 logged and suppressed), then the guest's own process exits and sends DISC
  (~15:27:01).

Next (continuing the same change series, in order):
1. Fix the guest JIP relaunch so the new process keeps the join target (and does not turn into a host): thread the
   connect address/session/join intent through `Relaunch::SetCurrentGameArgs` / `PauseMenu` guest auto-return path;
   re-run `probe-jip` - expected: guest joins and stays, host table x/N t0 stable for >=60 s.
2. Suppress the guest-side campaign game-over during the JIP handshake if it still fires.
3. Scale: host + 2 -> 4 -> 8 guests (memory: 5 instances peaked 22 GB of 31.8 GB; 16 local instances will not fit on
   this PC, so the 16-player goal needs a second machine/real peers - the code path for capacity 16 is
   `launchBlock[0xE8] = min(16, ...)` and the game-player array is 16 slots, peer ids 0-15 fit stock masks).
4. Keep the team fix as part of the campaign JIP change once the join is stable; it currently lives in the working tree
   (uncommitted) until the end-to-end join+stay test passes and both gates build.


### Guest abort root cause (15:36 ET, instrumented)

- Added a log-only hook on the external-launch readiness check `hr+0x41EB80` (`LobbyPatches: launch_ready`). Guest values
  at join: `src 0x... srcState 4 sess 0x... sessState 4 modeSet 1 modeVal 3` - all plausible.
- The check returns **1** for the first calls, then **0 at 15:36:06.157**, and the next line is
  `AbortGame(reason=6) from hr+0x3AA35D`. The inputs for the 1 and the 0 calls are identical, so the failing condition is
  deeper in the function: the branch that calls `FUN_1803c39fc()/FUN_1803c387c()` (external-launch data availability) and
  returns 0 when that data is not there.
- Also fixed a **harness bug**: `Release2..5` were still running an older `noble-dev.exe` (only the host was rebuilt), so
  earlier "guest" results were from a stale binary. `probe-jip.ps1` (and future harnesses) now copies the host binary to
  every instance before each run.

Conclusion: the campaign JIP join path works (guest joins, loads the mission, team fix puts everyone on team 0); the
remaining blocker is the **external-launch data handshake for a mid-mission joiner**. The host must publish the launch
data the guest's state machine expects (the `DAT_180c1a114` state machine / `FUN_1803c3xxx` helpers), or the guest must be
held in the join state until it arrives. Next code step:
1. Decompile `FUN_1803c39fc / FUN_1803c387c / FUN_1803c3e18` and the `DAT_180c1a114` writers in `hr_00000.c` to find what
   data the check reads and where it is populated (host-side external-launch publish).
2. Hook/populate that data from Noble's host path (the launch block is already built in BlamEngine and the engine start
   call receives it), then re-run `probe-jip.ps1` - expected: launch_ready stays 1, no AbortGame(6), guest stays.
3. Scale to more guests (toward the 16-player goal), noting the local-RAM limit (5 instances = 22 GB of 31.8 GB; a real
   16-player run needs a second machine/real peers).


### Launch-ready gate fully traced (2026-11-15 23:57 ET)

Instrumented the final helper `FUN_1804201dc` and logged the local-player state. Guest at the failure:
`launch_ready -> 0 (srcState 4, src+0x20 1, src+0x3C 0, sessState 4, modeSet 1, modeVal 3)` immediately followed by
`AbortGame(reason=6) from hr+0x3AA35D`.

- The identity match inside the check (`FUN_18041fbe8 -> FUN_1803a922c` murmur-fold of the 8-byte player id) **passes**:
  guest logs `identity_match -> 1 ... local 1133576688/16777216 slot 1 | p0=2C3776BB p1=C15E7E09`, host logs
  `identity_match -> 1 ... local 1723641663 ... | p0=2C3776BB`. Peer ids and slot mapping agree.
- The 0 comes from `FUN_1804201dc`: it returns 0 when `*(localPlayerManager+0x20) == 1`. That field is the local-player
  setup state; in the normal lobby→match relaunch flow the engine fills it from the launch data it receives, but a
  mid-mission direct-connect guest leaves it at 1, so the engine treats the launch as not ready and aborts the game.

Conclusion / next step (in order):
1. Find what writes `localPlayerManager+0x20` (search writers / the external-launch data apply path) and whether the
   normal lobby flow fills it from the launch block the host sends. This is the last unknown before a fix.
2. Two candidate fixes: (a) host publishes/forwards the launch data to a mid-mission joiner so the guest's player
   manager reaches the ready state; or (b) the guest applies the host's launch block before the check (client-side
   mapping fix). (a) is the clean fix and matches how the engine's external-launch state machine expects to work.
3. Note for 16 players: the engine's external-launch handshake has 60 s (individual) / 90 s (overall) timeouts; a
   16-player local test also needs more RAM than this PC has (5 instances = 22 GB of 31.8 GB), so the final 16-player
   acceptance needs a second machine or real peers.
4. Working tree still holds the campaign-JIP team fix + diagnostic hooks (uncommitted, ~200 lines in LobbyPatches.cpp);
   both gates build. Nothing released, version 0.19.103.


### Where true campaign drop-in stands (decision point, 2026-11-16 00:05 ET)

The abort trigger is the engine's **launch-data readiness** state: after a mid-mission direct-connect join the guest's
local-player manager stays at state `+0x20 == 1`, so `FUN_1804201DC` returns 0 and `FUN_1803AA268` does
`AbortGame(reason=6)`. Identity mapping, map load and session join all succeed; what is missing is the **external-launch
data the engine normally delivers from the host's mission start to the peers present at that moment**. A peer joining
later never receives it in our direct-connect flow, and the engine has no campaign drop-in support of its own (the
external-launch handshake has 60 s/90 s timeouts and is built for lobby launches).

Two paths forward:
1. **Deep engine path (true drop-in):** find the host-side external-launch data send (the `DAT_180c1a114` state
   machine / launch-data serialiser) and re-send it to each late joiner, then make the guest apply it (advancing
   `localPlayerManager+0x20`). This is original engine work with uncertain scope, but it is the only way to genuine
   turn-joined mid-mission campaign play.
2. **Standard Halo path (recommended for the 16-player goal):** all players join the host's **lobby/loading window**
   before the mission starts (Halo campaign co-op's model), with the host holding the lobby until all expected players
   are in. This is "join in progress" from the player's point of view (you can join while others are in the lobby/loading)
   and reuses the flow that already put **4 players in a live m10 session** and **5 players in one m50 session**. The
   remaining blocker for 5-16 is the all-peers-ready exit seen on m50 (every process got `EngineEvent reason=0` right
   after the 5th peer reached state 8), which is more tractable than the launch-data RE.
   - Capacity: the launch block already allows `launchBlock[0xE8] = min(16, ...)` and the game-player array is 16
     slots; peer ids 0-15 fit the stock masks, so 16-player co-op does not need the 32-slot shadow work.
   - Test logistics: 5 local instances peak 22 GB of 31.8 GB, so a full 16-player test needs a second PC or real
     players (planned as a 2-machine test).

Recommendation: take path 2 for the 16-player deliverable, and keep path 1 as a follow-up research item. Next concrete
steps for path 2: instrument the `-wait-peers` barrier on m50 (why reason=0/all-exit at all-peers-ready), fix it, then
raise the campaign co-op host capacity/expected-peers path to 16 and test 4 -> 6 -> 8 players locally before the
2-machine 16-player run.


### CAMPAIGN JIP BREAKTHROUGH - manager state gate (2026-11-16 00:20 ET)

Root cause fully identified with the write watch:
- The working pre-mission guest gets the local-player-manager state written to **4** (joined) from the network message
  apply path: `msg 37 (0x25) -> FUN_180429FC0 -> FUN_1803CDF38 -> ... -> FUN_1803C3E8C -> [manager+0x20] = 4`.
- The mid-mission guest never reaches 4: after receiving msg 37 it flaps 2<->3 (waiting) and then the launch-ready
  failure path writes **1** back (`FUN_1803C3E8C <- hr+0x3BF92E <- hr+0x3AA3B6`) -> `FUN_1804201DC` returns 0 ->
  `AbortGame(reason=6)` -> GameResults -> guest leaves.
- The state value semantics: 1 = not joined/reset, 2/3 = waiting, 4 = joined.

Experiment (env `NOBLE_JIP_STATE_HACK=1`, temporary): when a campaign guest is about to fail the launch-ready check
(state == 1), set the state to 4. Result:
- guest **stays in the mission**: `game session started`, LoadProgress 100%, `launch state 11`, frames ticking past
  frame 1500, **no AbortGame, no GameResults, no game over, 0 crashes** (previously left in ~12 s);
- host engine table: 2 players, both `t0`; host session stable, peer stays connected;
- capture of the host shows real campaign gameplay (m10, Noble HUD).

Still to verify/prove before calling it done:
1. The guest's local player is actually spawned/playable (the guest's capture-at did not produce a fresh screenshot; verify
   by dumping the guest's own game-player table after the state is forced, then by a guest screenshot/HUD).
2. Make the fix persistent and clean instead of re-forcing each tick: either write 4 once when the guest's player-add
   (msg 37) is applied, or understand why the apply path resets it and mirror the pre-mission apply.
3. Then re-run: 1 guest -> 2 -> 4 guests mid-mission (all should join and stay, host table N players t0), then push the
   capacity path to 16 and do the 2-machine 16-player test.
4. Harness now supports `-Premission` (known-good comparison flow), `-CaptureAt N` (engine screenshot), refreshes every
   instance's exe from the host build, and keeps per-run logs.


### JIP guest is in-mission but not yet seated as a player (2026-11-16 00:25 ET)

Verification of the state-fix guest: the guest's **own game-player table has only 1 entry** - index 0
`f00000008` (the host's player) - while the host's table has 2 players (both `t0`). So the guest's engine is running
the mission but **its own player 1 was never replicated into the guest simulation**: it is spectating.
Also: the guest process never arms `-capture-at` (no "FrameCapture: ... armed" line in its log), so screenshots are not a
usable verification path on guests right now - the game-player-table dump is the reliable check.

Next fix target: deliver/reconstruct the joining player's *player data* on the guest so `Hook_PlayerSync` can append the
guest's own player (the session player mask on the guest already includes both players; the game-player table does not).
Concretely:
1. Log the decoded payload of session **message 37** (0x25) - which player index it carries - on both host sends and
   guest receives, and log the guest's player-sync in/out (session mask vs table mask). That shows whether the host
   sends the peer's player-add only for player 0, or the guest drops/ignores the player-1 add.
2. If the host sends nothing for the joiner: make the host broadcast the joining player's data after the peer's channel
   is established (the same message the pre-mission flow sends at mission start).
3. Then re-verify guest table = 2 players (self included), HUD/spawn in the guest, then make the state fix persistent
   (write 4 once instead of per tick) and scale 1 -> 2 -> 4 guests, then 16.


### Player-seating diagnosis (2026-11-16 00:35 ET)

Target shape from the working pre-mission guest (dumped with `NOBLE_JIP_DUMP=1`): its table has **2 players, both
`f00000001 t0`** (host remote + itself). The JIP guest still has **1 player** (host, `f00000008`).

Captured from the running engine:
- `msg 37` (0x25) payloads are player-slot records: `00 00 00 00 FF FF FF FF …` / `01 00 00 00 FF FF FF FF …` /
  `02 00 00 00 00 00 00 00 01 00 00 00 3C 00 00 00 <16-byte identity>` / `03 …`. Host sends these, guest receives and
  echoes them, so the message link is up.
- The player-create gate `FUN_180420298` returns **1** on the guest (preconditions `b960=7, b7cc=5` are satisfied), yet
  the guest's game-player table is not updated.
- The guest's `player_sync` hook (`hr+0x41F510`) never fires (no log lines at all on the guest), so the guest's game
  player table is built by a different path than the host's - that path is what drops the guest's own player.

Next concrete step: find what adds the guest's own player in the working flow. The host-side sync (0x41F510) appends
game players from session players; on the guest it is never called, so the guest's own player must be created from the
`msg 37` apply path (`FUN_1803CDF38` chain) - trace that function's per-player loop for the player slot matching the
guest's own identity and why it does not append the local player; then make the join apply the same record (or call the
append path directly once) so the guest table becomes 2 players `f00000001`. After that: verify HUD/spawn, make the
state fix persistent, and scale to 2/4/16 players.


### Status after seating deep-dive (2026-11-16 00:40 ET)

- The msg-37 apply path (`FUN_1803CDF38` controller/player state machine) behaves the **same on host and guest**: it
  cycles player indices 0..3, applies/rejects with the same status values (1 = applied, 0xFFFFFF01 = rejected/skip).
  So the message apply itself is not guest-specific.
- The guest's game-player table (`DumpGamePlayers`) is still **1 entry**: `0:EC70 f00000008` = the *host's* player
  (same salt EC70 as host player 0). The guest's own player (host-side salt EC71) is absent. Working pre-mission guest
  has 2 entries, both `f00000001`.
- Visual check is unreliable right now: the two windows overlap on the same monitor position, so an external screen
  capture caught the same window twice ("STARTING GAME..." panel), not separate host/guest views. The in-engine
  player-count/table dumps remain the reliable evidence.
- Conclusion: the guest's local player datum is created during its own pre-join launch, then discarded when the engine
  re-initialises for the host's in-progress game. The remaining fix is to **re-create the local player on the guest
  after the join**: the likely entry point is `FUN_1803C3E18(* (param+8))` (called from the launch-ready path when the
  local player record is not found) plus the launch-block player configuration; trace that create path on the guest,
  feed it the host session's player/launch data, and verify the guest table becomes 2 players `f00000001`.
- After that: guest HUD/spawn verification, persistent state fix (write 4 once), then scale to 2 -> 4 -> 16 players.


### Working vs JIP guest controller/player mapping (2026-11-16 00:41 ET)

Dumped both flows with the controller-apply hook. The difference is in the mapping sequence, not the join itself:

- Working pre-mission guest (3 players): `controller_apply` applies player indices `p2=2,3,4,5` with `p3=0`, its object
  fields climb `f40 1->2->3->4, f44 0, f50 0->2->3->4->5`, and the player table goes from three entries
  `f00000008` to three entries **`f00000001`** (players become active) - that is the launch-completion sequence.
- JIP guest: applies `p2=2,3` (f40 1->2->3) then receives `p2=0, p3=-1` and `p2=1, p3=-1`, which are
  **rejected with status 0xFFFFFF01 and reset the mapping** (`f50 3->0, f44 0->-1`; one call shows `f3c 6` set). The
  sequence restarts (`p3 1`) and never reaches the `p2=5 / flags 0x1` completion; its table stays at one inactive
  entry `f00000008`.

So the JIP guest needs two things tied together:
1. finish the player activation sequence (the `0x8 -> 0x1` transition) after joining,
2. create its own local player datum (the working guest's table includes itself because the whole match launched from
   one shared launch block; the JIP guest's local player was dropped at re-init).

Next targets in order: (a) the `p2=0/1, p3=-1` reset source (which msg-37/apply call drives it and why the session id is
invalid on the JIP guest), (b) the activation completion path (what writes table flags 0x1 after `p2=5`), (c) local
player re-creation using the guest's own launch block player configuration; then re-test 1 guest, 2 guests, 4 guests,
and finally the 16-player capacity run.


## SESSION HANDOFF - campaign JIP (2026-11-16 00:47 ET)

State of the work (all uncommitted in the working tree, noble-dev builds clean):

1. **Campaign JIP session join is FIXED and proven**: host starts m10 solo (no -wait-peers), guests connect
   mid-mission; they join the session, load the map (100 %), reach `launch state 11`, stay connected (no crash, no
   channel close) - previously they were evicted after 1-2 s. Two pieces made this work:
   - `ForceCampaignPlayerTeams()` - campaign JIP players are forced onto team 0 (host and guest);
   - the launch-ready gate is satisfied for the guest by the local-player-manager state experiment.
2. **Guest is still NOT seated as a player** (the remaining work): its own game-player table has 1 entry (the host's
   player, flags `f00000008`); the working pre-mission guest has 2-3 entries, all `f00000001` (active). The guest is in
   the mission as an observer/spectator. Findings toward the fix:
   - the manager state must be 4 ("joined"); the JIP guest flaps 2<->3 and then the abort path resets it to 1
     (env `NOBLE_JIP_STATE_HACK=1` forces 4 and keeps the guest alive - experiment only);
   - the working flow's launch-completion sequence is visible in `controller_apply` (`FUN_1803CDF38`): it applies
     player slots p2=2,3,4,5 and then the table flags go 0x8 -> 0x1. The JIP guest gets p2=2,3 applied, then an invalid
     config (p2=0/1, no session player = -1) resets the mapping (`f50 3->0, f44 0->-1`) and it never completes;
   - the guest's `player_sync` (`hr+0x41F510`) never runs, so the guest's table is built by the msg-37 apply path
     (`FUN_1803CDF38` chain); `FUN_180420298` player-create gate passes (returns 1) and its preconditions
     `b960=7, b7cc=5` are met;
   - `FUN_1803C3E18(manager)` (called when the local player record is missing) clears player records and sets the
     manager state; it is the likely place to re-create the guest's own player from its own launch-block config.
   - Experiments tried (env-gated): `NOBLE_JIP_STATE_HACK=1` (keeps guest alive), `NOBLE_JIP_APPLY_HACK=1` (skip/reset
     suppression of the invalid controller config - not sufficient on its own).
3. **Verification tools**: `probe-jip.ps1` (JIP flow; `-Premission` comparison mode; `-CaptureAt`; refreshes all
   instance exes from the host build), `NOBLE_JIP_DUMP=1` periodic game-player-table dump, launch-ready / identity /
   manager-state / player-create / controller-apply hooks. Guest `-capture-at` does not arm FrameCapture (guest render
   path), and external screen capture is unreliable when windows overlap - use the table dumps.
4. **Scale-out plan**: after seating works for 1 guest, test 2 and 4, then capacity 16 (launch block `0xE8 `min(16,...)`,
   16 game-player slots, peer ids 0-15); the full 16-player acceptance needs a second machine (5 local instances peak
   22 GB of 31.8 GB).
5. **Flaky/experimental flags must be cleaned up before shipping**: remove the state/apply experiments or convert the
   state fix into the proper persistent write, keep the instrumentation log-only, then rebuild both gates and commit
   (currently only noble-dev built; nothing committed, version 0.19.103).


### Insertion probe (round N, 2026-11-16 00:57 ET)

- Added logging to the existing `evaluate_coop_insertion_point` override (`hr+0x39CE10`). During a JIP join the
  function is **never called** on either host or guest - the join path stops before the engine attempts to seat the
  joining player, which is consistent with the guest staying at one inactive game-player entry.
- Host table at the time of the dump was still 1 player (guest had not been added yet at that sample point); guest
  table dumps were not produced in this run (env inheritance/timing - to re-check).
- Next targets: (a) find what makes the host add/activate the joining player's game entry after membership accepts the
  peer (the working pre-mission flow activates all entries to flags `f00000001` and causes the manager state write 4
  through the network apply chain), (b) re-run the `msg-37` apply with the guest's valid player config once the host has
  emitted it (the JIP guest currently gets invalid/no-session-player records that reset the mapping), (c) trace the
  `evaluate_coop_insertion_point` caller (what event triggers insertion evaluation) so a mid-mission join can invoke it.


### Config-apply progress and next target (2026-11-16 01:07 ET)

- Working-vs-JIP msg-37 broadcast difference confirmed: working launch sends one clean round for controller slots
  **1..5** (second dword 0); a JIP peer receives repeated rounds with slots **0..3 only** and varying second dword.
- Guest-side experiment: skip invalid controller configs (slot 0, and slot 1 with no session player) on campaign guests.
  Result: the mapping **no longer resets** (`f50 3->3`, `f3c 0` stays clean) and settles at `f40 3 / f44 0`, but the
  launch-completion step (working flow reaches slot 5 then flips the game-player flags from `f00000008` to
  `f00000001`) never happens - the JIP host never broadcasts slots 4/5.
- Next target: watch the game-player flags word (`gamePlayerTable + i*0x490 + 4`) in the working pre-mission guest to
  find the function that performs the `0x8 -> 0x1` activation flip (the existing player-0 hardware watch can be reused
  for campaign co-op). Then either trigger that activation on the JIP guest or synthesize the missing slots 4/5 apply
  calls so the activation runs. After seating: remove the experiments, persistent state fix, 1/2/4 guest tests, 16-player
  capacity path, both gates, commit, docs.


### Player activation chain captured (round 1, 2026-11-16 01:11 ET)

Hardware watch on the game-player flags word in the working pre-mission flow shows the full activation sequence on
every process (host and guests), right after game start:
- `0x8 -> 0x9` at `hr+0x46FF9` (FUN_180046E6C),
- `0x9 -> 0x1` and `0x1 -> 0x2001` at `hr+0x4B402 / 0x4B433` (FUN_18004A9C8),
- final `0x1` at `hr+0x47AF8` (FUN_180047A3C), reached via `FUN_180048040 -> FUN_180047A3C -> FUN_180047050` from the
  per-player update loop (`FUN_180057434 / FUN_1800575AC`), i.e. the real player update/activation path.
After that the table entries are `f00000001` (active).

On the JIP guest the existing (host) player entry stays `f00000008` for minutes - its player update/activation loop
never performs those writes. Next target: instrument `FUN_18004A9C8` / `FUN_180047A3C` / `FUN_180057434` on the JIP guest
to see whether the loop runs for that entry and which precondition blocks it; then either drive the activation or
deliver the spawn/insertion data the loop needs. This is the last functional layer between "guest in mission" and
"guest playable".


### Round 2: simulator gate found (2026-11-16 01:23 ET)

- The per-player update loop (which drives the activation chain) is gated by `FUN_1803AAB08()`. Gate inputs on the
  working host: `engine+0x10=3, engine+0x20=4, ctx+4=-2147483648 ` -> gate 1. On the JIP guest:
  `engine+0x10=4, engine+0x20=3, ctx+4=-2147483648 ` -> gate 0. The guest's game-engine/player-manager pair is swapped
  relative to the host (3/4 vs 4/3), so the whole player update (and therefore activation) never runs on the guest.
- Experiment: force the gate open on campaign guests. Result: `player_update_loop` is then called (was 0 calls), but
  with `a1=0`, which early-returns in `FUN_180057434` (`if (param_1 < 1) return`). The call order (gate then a1=0) shows
  the engine passes a "local-player/update count" that is 0 on the guest.
- Next: find where the main loop's `param_1` for FUN_180057434 comes from (local var in FUN_180033E6C derived from the
  game-engine player count / "local players bound" state) and what should raise it on a guest after a JIP join
  (likely the same local-player-manager state that stays 0/non-4). Then make that state persistent (remove the
  `NOBLE_JIP_STATE_HACK` / `NOBLE_JIP_SIMGATE_HACK` experiments) and retest.


### Round 3: broadcast chain mapped, next lever (2026-11-16 01:28 ET)

- Host-side: after a JIP join the host's own game-player table does activate the guest's player (`1:EC71 f00000001`
  in earlier dumps), so the host simulation seats the joiner.
- Guest-side: the guest never builds its own game-player entry. The player-config broadcast chain was mapped from the
  host send stacks: `FUN_1803BF844 -> FUN_1803C1854 / FUN_1803C29CC -> FUN_1803CDD2C -> FUN_1803CDE4C ->
  FUN_1803F9620 -> FUN_180442134` (session send/encode of msg 37). The engine only broadcasts its **local** player
  configs; the joining guest's own player config is never delivered/built on the guest, and the local-player manager's
  record list is already empty at join time (the abort/re-init cleared it via `FUN_1803C3E18`).
- Next levers, in order of likelihood:
  1. Preserve/re-create the guest's local player records: hook `FUN_1803C3E18` (the "clear local players + set state"
     path called when the manager record is missing) and the local-player-list setters `FUN_1803BF63C / FUN_1803BF2C4`
     (called by the launch transition functions `FUN_1803C387C/FUN_1803C39FC`), then re-run the launch player-config
     apply for the guest's own block after the join.
  2. Or make the host re-broadcast the joined player's config (`FUN_1803BF844` chain) so the guest can apply it.
- Also to do when the seating path works: remove the env-gated experiments (`NOBLE_JIP_STATE_HACK`,
  `NOBLE_JIP_SIMGATE_HACK`), keep the real fix persistent, then 1/2/4-guest verification, 16-player capacity path,
  both gates, commit, docs.


### Round 4: missing launch messages identified (2026-11-16 01:36 ET)

Host SEND-message histograms, working launch vs JIP:
- working: msg 4 x15, 6, 7, 14 x2, 25, 26 x10, 34 x15, 37 x6, **39 x15**, **43 x2**;
- JIP: msg 4 x1, 6, 25, 26 x6, 34 x15, 37 x11 - **no 7, no 39, no 43**.

So a late joiner never receives the launch-time player-config/session messages:
- **msg 39** (player config apply; receive handler in FUN_180429FC0 branch 0x27, applies only when local game-engine +0x20 == 4)
  is sent from the per-player update path: `FUN_180057434 -> FUN_1800575AC -> FUN_1803AB59C -> FUN_1803C0804 ->
  FUN_1803CE42C -> session send`.
- **msg 43** (session state, sent x2 at launch) is sent from the launch-update path:
  `FUN_1803AA268 -> FUN_1803BF844 -> FUN_1803C1854 -> FUN_1803C1AE0 -> FUN_1803C0E50 -> session send`.
- msg 7 also missing.

These are precisely the messages that configure the guest's local players at launch. Next experiment: on the host, when a
remote player joins mid-mission, invoke the launch-update broadcast `FUN_1803BF844(DAT_184e0e908)` once (and/or drive the
per-player update path) so msg 43/39 are emitted for the late joiner; verify the guest's game-player table becomes
2 active entries. If that alone is not enough, hook the session send to target the new peer explicitly.


### Round 5: re-broadcast negative, MP comparison attempt (2026-11-16 01:39 ET)

- Experiment: call `FUN_1803BF844(DAT_184e0e908)` (the launch-update broadcaster) on the host when a late joiner's player
  is mirrored. It ran (logged twice) but emitted **no msg 39/43** - in mission state (launch 11) the function takes a
  non-broadcast branch (it only emits during launch states 3/4/5). So a simple re-run cannot replay the launch handshake.
- Tried a multiplayer mid-match join comparison (MP sessions support JIP) using `probe-jip.ps1 -Mode multiplayer`: the MP
  host crashed in the harness before reaching a mission (`0xC0000005 at haloreach +0x7E8B0`, minidump written), so no MP
  message comparison this round. The campaign path is unaffected (campaign JIP runs still work).
- Next: (a) retry the MP JIP comparison with the proven MP harness settings (probe-lan32 arguments, -slot-patch) to see
  what launch/join-sync messages an MP late joiner receives; use that as the template for campaign. (b) Alternatively
  find the engine's join-synchronize path (`c_network_session` state 9 / peer-establish messages 22/25/26/34/35/36/38`)
  and see which message carries the launch data so it can be targeted to the late peer.


### Round 6: MP comparison works - campaign gap isolated (2026-11-16 01:43 ET)

- Fixed a real hygiene bug: the campaign-JIP diagnostic hooks are now installed only with `NOBLE_JIP_PROBE=1`; with them
  gated out the reproducible MP crash at `haloreach+0x7E8B0` disappeared. Normal modes are no longer affected by the
  instrumentation (important for the eventual shippable change).
- MP mid-match join test (host 20_sword_slayer multiplayer, guest connects 25 s later):
  - host game-player table seats the joiner: `1:EC71 f00008008 t1` (MP), no crash;
  - the guest receives the **full msg-37 controller-config round for slots 1,2,3,4**, and the slot-2 payload carries the
    **guest's own identity** (e.g. `73 86 0D 37 ...`), i.e. the MP guest's own player config is in the session;
  - no msg 39/43 needed for MP JIP.
- Campaign JIP guest, by contrast, receives msg-37 rounds containing the **host's identity** (CC C6 AD 57 ...) and never
  its own config; its engine is the one that aborts/re-initialises on join (external-launch-ready gate) instead of taking
  the MP JIP path. So the remaining difference is the campaign session's join-in-progress behaviour, not messages 39/43.
- Next: find the session/JIP-mode switch the engine derives from the game mode (campaign vs multiplayer) - the MP path
  clearly supports late joins natively. Look for the session-description field / session-create parameters set from the
  launch block's mode (0x0C) and force the campaign host session to advertise join-in-progress, so the joining guest's
  engine follows the MP path (no abort, own player config exchanged).


### Round 7 decision point (2026-11-16 01:47 ET)

MP-engine-class experiment (`NOBLE_JIP_GUEST_MODE3=1`, guest launch block engine class 0x0C=3):
- **Abort eliminated without hacks**: the campaign guest starts, loads m10, reaches launch state 11, ticks frames and stays
  connected with **no AbortGame, no GameResults, no state hack needed**; host seats the joiner (table 2 players).
- Still **not seated on the guest**: its game-player table remains 1 entry (host player, f00000008). The guest receives
  msg-37 rounds carrying the host's config only; its own player config is never exchanged. sim_gate stays 0
  (engine+0x20 = 3), so the per-player update/activation never runs on the guest.

Blocker (persistent across rounds 3-7): the campaign/co-op session has **no join-in-progress player-config exchange**.
The engine's late-join machinery used by multiplayer (own config broadcast, join synchronisation, MP engine class)
does not run for campaign sessions; the guest's local player manager never receives its own player config, and there
is no engine entry point found to create that player from session/membership data. Tried and exhausted:
launch-message replay (no-op in mission state), launch-update re-broadcast, manager-state forcing, controller-config
reset suppression, launch-reset skip, external-launch-ready bypass, MP engine class. MP JIP works natively, so the
machinery exists but is gated by the campaign session/join path.

Two options for a decision:
1. **Ship path (recommended):** standard Halo co-op model - host holds the lobby/loading window until the expected
   players have joined (this already seats 2-3 active players in tests, and the 16-player capacity path is
   `launchBlock[0xE8] = min(16,...)` + 16 game-player slots / peer ids 0-15). Call this "join in progress" from the
   user's point of view (you can join while the host is in the lobby/loading). Verified path: `probe-jip.ps1 -Premission`.
2. **Continue deep RE:** reconstruct the campaign session's late-join player-config handshake (what MP does) - open-
   ended engine work; would need the session description/join-sync protocol and player-config exchange to be replicated.


### SHIP PATH progress (2026-11-16 03:37 ET)

Persistent fixes made (no env-gated behaviour):
1. **Campaign guests start their engine in the multiplayer engine class** (`block[0x0C] = 3` for campaign guests). The
   local block only selects the engine start class; session/game mode still comes from the host. Verified: campaign
   guests join a lobby/loading-window session cleanly and are seated as active players.
2. **Team-0 forcing** for campaign co-op players (already in the tree) stays as part of the change.
All `NOBLE_JIP_*_HACK` behaviour experiments were removed from the ship path; only log-only diagnostics remain and those
are installed only with `NOBLE_JIP_PROBE=1`.

Verified launches (m10 campaign, one machine, AOC, low-spec settings):
- 2 guests (3 players): all active `f00002001 t0` on host and guests, no aborts/GameResults.
- 3 guests (4 players): host game-player table `0..3 f00002001 t0`, `game session started`, no aborts.
- 4 guests (5 players): new readiness work (exact declared player count + wait for real network game-peer count) gets
  **all 5 participants ready** (engine peers 4/4, elapsed 25 s) and posts Command 11, but the **engine shuts the session
  down immediately after simulation start** (channels closed reason 14 via
  `0x3F82E0 <- 0x3F7501 <- 0x3AE3E7 <- 0x34DC1`; no AbortGame/SetGameEnded/GameResults hook fires). This is the
  campaign **game-start player cap (>4)** - the remaining blocker for 5-16 players.

Harness/engine improvements now in the tree (still env-gated for testing, to be made persistent when the cap is fixed):
`NOBLE_COOP_DECLARE_EXACT` (declare exactly host+expected peers instead of min 8), `NOBLE_COOP_WAIT_CHANNELS` (wait for
real game-peer count rather than the over-counting msg-27 counter), `NOBLE_COOP_WAIT_MS` (co-op wait window, default 45 s).

Next: locate and patch the campaign game-start player cap (the validation that shuts the session when >4 players are
present at Command 11); candidate paths are the launch state machine (`FUN_1803AA268` family) and the c_game_engine
start validation. Then: 5-player launch test, make the readiness options persistent, 16-player 2-machine plan, both
gates, commit, docs.


### Ship-path verification addendum (2026-11-16 03:40 ET)

Verified with the persistent fixes (no env behaviour hacks):
- 3 players `f00002001 t0` active on host and both guests (2-guest test);
- 4 players `0..3 f00002001 t0` active on host, `game session started` (3-guest test).
These are stock-Halo-sized campaign co-op sessions over the lobby/loading-window flow with the campaign-guest engine
class fix.

5 players: all participants ready, Command 11 posted, then the engine closes all channels (reason 14,
`0x3F82E0 <- 0x3F7501 <- 0x3AE3E7 <- 0x34DC1`) and exits with `EngineEvent reason=0` - no AbortGame/SetGameEnded/
GameResults hook fires and LobbyPatches logs no error, so this is the engine's own campaign co-op start validation
(game engine campaign player cap = 4), not our launch-barrier code. The session-validation function
`FUN_180408DDC` (which holds the already-patched gates at 0x40954A/0x40955B) is the place to look next for the start
time check; alternatively c_game_engine's campaign player-count gate.

Ship scope decision needed after that cap work: stock Halo campaign co-op is 4 players; >4 (the 16-player goal) needs
this engine cap lifted, which is the remaining deep-work item.


### SHIP CANDIDATE verified (2026-11-16 03:44 ET)

Persistent, no env-gated behaviour (behaviour experiments removed from the tree):
1. campaign guests start the engine in the multiplayer engine class (`block[0x0C] = 3`; session/game mode still from
   the host);
2. campaign co-op players forced onto team 0;
3. co-op launch declares exactly the expected roster (`0xE8 = min(16, 1 + expectedPeers)`) instead of widening to 8;
4. the launch barrier waits for the real network game-peer count (channels) instead of the over-counting msg-27 counter,
   with a late counter fallback at 20 s.

Regression (m10, campaign, one machine, AOC, low-spec settings, `NOBLE_JIP_DUMP=1` only):
- 2 guests / 3 players: host and client tables all `f00002001 t0` active, `game session started`;
- 3 guests / 4 players: host table `0..3 f00002001 t0`, `game session started`, no aborts.
These match stock Halo campaign co-op (4 players).

5+ players: all participants can now become ready and Command 11 is posted, but the engine closes all channels and exits
(`EngineEvent reason=0`) - the campaign game-engine player cap (stock campaign co-op is 4). Lifting that is the remaining
item for the 16-player goal; it lives in the engine's campaign start validation, not in our launch barrier or session
plumbing.

Both gates building clean (noble-dev + noble-client) with the persistent logic.


### Campaign cap characterized + ship config final (2026-11-16 03:47 ET)

Decisive experiment: with **5 players present but the launch block declaring 4** (stock roster), the host stays alive and
the game session starts: host game-player table shows 5 entries - `0..3 f00000001 t0` (active) and `4 f00000002` (the
extra joiner tolerated in membership as a non-active entry), player mask `001F`. Declaring 5 instead makes the engine
close the session at simulation start (`EngineEvent reason=0`). So the abort is the **campaign game engine's start-time
player cap of 4**, not the session/launch barrier.

Ship configuration therefore declares the stock campaign cap:
`kCampaignCoopEngineCap = 4` in BlamEngine (`declaredPlayers = min(4, 1 + expectedPeers)`) until the engine cap is
lifted. Everything else stays persistent: multiplayer engine class for campaign guests, team-0 forcing, real channel
readiness wait, exact roster declaration.

Verified with this config: 3 players and 4 players (stock campaign co-op) - all active `f00002001 t0`, `game session
started`, no aborts. 16-player campaign co-op remains gated on lifting the engine's campaign player cap (deeper engine
work: the cap lives in the campaign game-engine start validation; session plumbing already tolerates extra members).


### Spawn/cutscene hypothesis test (2026-11-16 03:54 ET)

User hypothesis: campaign co-op spawns and scripted cutscenes are authored for four players, so a fifth player has
nowhere to spawn/be shown and the engine aborts.

What the instrumentation shows:
- With four guests connected at Command 11 (declared roster clamped to the stock cap of 4), the host engine still exits
  (`EngineEvent reason=0`) - so the abort triggers when **more than four players are present at simulation start**,
  regardless of the declared count.
- The fifth guest resolves `evaluate_coop_insertion_point -> 4` (one beyond the stock four-player range); clamping that
  result to 3 on both host and guest (diagnostic `NOBLE_COOP_CLAMP_INSERTION=1`) did **not** prevent the host abort -
  the cap check happens before insertion/spawning.
- When only three players are present at Command 11 (declared 4), the engine starts fine and a later fifth joiner is
  tolerated in membership as a non-active entry (`4:EC74 f00000002`) - it is never activated.
- Guest five also hit `external_launch_individual_state_timeout` (ReachHost's transient-join grace keeps waiting).

So the engine's campaign **start-time player cap (4)** is the immediate blocker; the user's spawn/cutscene point is a
separate, real content constraint that would apply *after* that cap is lifted (campaign scenarios author four player
spawns and scripted scenes with four co-op players; a fifth player has no authored spawn/camera slot). Neither has been
proven to be the abort cause yet beyond the start-time cap evidence above.


### 16-player campaign plan takes shape (2026-11-16 04:06 ET)

New direction accepted by Tyler: extras do not need to appear in cutscenes. Plan: host starts the campaign mission with
at most the stock four players (or solo), the remaining players join **after** mission start, and they are spawned at the
mission-start landing area once the opening cinematic ends (they can be hidden until then).

Evidence collected:
- The engine's >4 abort only triggers when more than four players are present **at simulation start**; extra players
  joining after mission start are tolerated in session membership and appear in the host game-player table
  (earlier JIP runs show remote entries going `f00008001` -> `f00000001` - the host activates/spawns mid-mission
  joiners on its own side).
- So the remaining work is the **guest side**: a mid-mission guest still does not create its own game-player entry
  (its table only holds the host's remote player), and its engine stays a spectator; the guest-side player-sync path
  (`hr+0x41F510`) never runs because the engine-update gate is closed on campaign guests.
- Started a hardware probe on the game-player datum unit field (`+0x28`) to find the spawn code; first captures point at
  the per-player update chain (`hr+0x45987 <- 0x4AE39 <- 0x4730F <- 0x576F9 <- 0x5753A <- 0x34411`) but the field's
  meaning/value is not yet confirmed, so the spawn entry point is not identified yet.

Next: force/execute the guest's player-sync path (`FUN_18041F510`) with the session player set after a mid-mission join
so the guest builds its own game-player entry, then drive the engine's spawn (unit creation) for that player at the
mission-start location after the cinematic; validate with 5 players, then scale.


### 16-player campaign: host side accepts 5+ players (2026-11-16 04:40 ET)

Milestone evidence from a 5-machine test (host + 4 guests, m10 campaign, low-spec, probe build):
- With extras arriving after the host's simulation start, the host **stayed in mission with five game players**
  (table `0:EC70 f00000001 t0 ... 4:EC74 f00000001 t0`, mask `001F`) for over two minutes, no AbortGame, no
  EngineEvent, mission frames advancing (launch 11 running). So the campaign engine's start cap is a *start-time*
  condition; the running host tolerates more than four players.
- The engine's own `player_create` (hr+0x420298) tried to create the extra player datums and gave up after 20 quick
  attempts when a late joiner's channel state had not settled (`b960/b7cc` channel fields 0) - it is a *timing*
  condition, not a hard player cap. A queued-retry probe (`NOBLE_COOP_RETRY_CREATE`) was added to retry those calls
  after the channels settle.
- Guests joining during the loading window each build a four-player game-player table (`EC70..EC73, flags `f00000008`,
  teams t0..t3`) and now sit waiting instead of aborting (the committed class-3 guest-engine start plus the committed
  launch plumbing keep them alive). They still do not get their own player activated into the mission, and the pending
  extra players on the host need units - the guest-side activation/spawn path remains the open problem.

Probe hooks added this round (all env-gated diagnostics): player-update broadcast chain capture
(`player_update_send` hr+0x3AB59C, `player_config_send` hr+0x3C0804), guest state dump, engine-state promotion,
create retry, pending-player activation. None of this is committed; the shippable increment stays f7a2c74.


### 16-player campaign: player-create gate proven, post-create stall remains (2026-11-16 04:52 ET)

- The engine's `player_create` gate is proven: forcing the per-slot state pair (DAT_182d1b960 / DAT_182d1b7cc,
  stride 0x18D0) from `8/3` (or `9/2`) to `7/5` immediately before the call makes it return success for the fifth
  player slot (`forced slot 3 states 8/3 -> 7/5` then `player_create -> 1`).
- But the launch still does not complete: with the forced states, the engine keeps re-issuing creates and every instance
  (host and guests) ends up with four pending game players (`flags f00000008, teams t0..t3`), no activation, no player
  update — a stalled launch. The forced writes perturb the launch state machine, so this shortcut is diagnostic only.
- Separately, the hard >4 start cap is reproducible in seconds: strict channel wait (all four guest channels ready) plus
  full-roster declaration (5) makes the host engine begin shutdown ~28 ms **before** Command 11 returns, load
  `mainmenu.map` and exit (`EngineEvent reason=0`) with no player_create calls at all. This is the engine-side
  rejection of a >4-player campaign launch that must be found and patched.

Next: trace the engine's return-to-menu decision in the strict-channel reproduction (the launch state reading map 0 /
mainmenu load), and separately characterise what normally moves a pending player (flags 8 -> active) so the late-join
path can complete without perturbing the state machine.


### 16-player campaign: launch sequence and late-joiner slots (2026-11-16 05:12 ET)

New instrumentation results:
- The external-launch slot state machine is captured: each joining player's slot runs `3 -> 4 -> 5 -> 6 -> 7`
  (setter hr+0x3F7B40, called from the launch/apply stack hr+0x39541A / 0x39BE9F / 0x39A932 / 0x3AE6FC), and
  `player_create` only succeeds for a slot at 7 (plus the 5 in the second state word). A mid-mission joiner is already at
  8/3 (in-game states) so the launch sequence never runs for its slot - the engine then gives up and disbands.
- The launch barrier now has a slot-readiness condition (all expected peers' slots at 7/5) plus a configurable wait window
  (`NOBLE_COOP_WAIT_MS`, default 45 s). With the full roster declared and a 120 s window, the host started the mission
  cleanly with three guests seated (4 players active) - no abort - but the fourth guest's instance connected after
  Command 11 (its first msg-37 arrived with the launch broadcast), so only three remote player datums were created.
- So the remaining blocker for a clean 5-player launch is timing: the last guest must be launch-ready (slot at 7/5)
  before Command 11. The one-machine harness is flaky at four guest instances (one instance can take tens of seconds
  longer to boot), which masks the real condition.

Probe state: slot-state setter hook, launch-slot readiness list, configurable wait window, roster-declared launch are all
env-gated diagnostics; the shippable increment stays f7a2c74.


### 16-player campaign: engine shutdown initiator identified (2026-11-16 05:16 ET)

- Captured the engine teardown initiator stack in the >4 launch failure:
  `stage 0x3873C(stage=3) <- FUN_1800A3C74 (0xA3E7C) <- main loop (0x34D3A)`.
- `FUN_1800A3C74` is the engine's "game finish" routine: it calls `FUN_1800A456C("game finish")` and shuts the engine
  down when `FUN_1800586C4()` reports the finish flag. That predicate reads the TLS game context:
  `ctx[0x1EAB0] != 0 && ctx[0] == 0` then returns `ctx[1] != 0`; the flag is set by `FUN_180058708`.
- The frame function that sets it is `FUN_1800A411C`: on the else-branch of `FUN_180015034(...)` it calls
  `FUN_180058708()` ("game finish"/quit flag) and logs "game start". So the remaining question is what makes the engine
  believe the launch/game is finished in the >4-player case (likely a game-engine/launch validation returning failure).
- Practical note: the one-machine 5-instance harness remains flaky (one guest instance can boot tens of seconds late and
  the engine then takes the disband path); the create-success wait added this round did not get a clean shot because the
  engine exited before any player datums, so the next run needs the slow-instance issue controlled (staggered cold boot,
  or the 2-machine test).

Probe additions this round: slot-state setter transitions, player-create success counter (`GetPlayerCreateSuccessCount`),
create-count launch barrier, configurable wait window, stage-initiated teardown stack capture.


### 16-player campaign: launch-timeout insight and block validation (2026-11-16 05:19 ET)

- The engine has an `external_launch_overall_timeout` / `individual_state_timeout`: delaying Command 11 for tens of
  seconds while waiting for slow guests makes the engine take the "game finish"/teardown path (initiator stack
  `stage 0x3873C <- FUN_1800A3C74 "game finish" <- main loop`). So the launch window is short (~25-30 s) and cannot be
  stretched to wait for cold-booting guest instances.
- Harness updated with `-GuestsFirst` (pre-warmed guests start first, then the host): this removes most of the join-latency
  flakiness, but the run now shows a different failure: with the roster declared 5, the engine exits at boot
  (`engine main loop returned 1 after 579 ms`, `EngineEvent reason=0`), i.e. the launch block itself is rejected when it
  declares more than four players - the declared-count cap is enforced in block/launch validation, not only in the
  session join path.
- Working runs with declared 4 still boot fine, so the next step is to find the block validation that limits the declared
  player count (0xE8) to four, or encode the extra players a way the loader accepts.

Harness change is in the untracked probe script; probe code unchanged from last round in the repo. Ship increment stays
f7a2c74.


### 16-player campaign: declared-5 block is rejected at engine boot (2026-11-16 05:21 ET)

- With guests pre-warmed (they start a few seconds before the host) the join-latency issue is gone, but the host engine
  now consistently unloads ~3-4 s after boot when the launch block declares 5 players:
  `engine main loop returned 1 after 516 ms` then `EngineEvent reason=0`, before any guest connects; the later
  Command 11 goes to a dead engine. Runs whose guests start after the host (host-first order) sometimes get further,
  so this is likely engine-side block validation when the declared player count exceeds the stock campaign maximum.
- The harness now supports `-GuestsFirst` with a 4 s stagger so every instance is booted when the host starts its load
  window (guests otherwise time out waiting for the host after ~30 s).
- Working baseline (declared 4) remains stable; all probes and barriers from earlier rounds still in place.

Next: confirm the early unload is block validation (instrument BlamEngine's start/exit handling and retry), then either
patch the declared-count validation or encode the extra players without exceeding it at launch.


### 16-player campaign: abort/quit path identified (2026-11-16 05:33 ET)

- Instrumented the engine's error/quit reporter `FUN_18000D648(code, reason)` (hr+0xD648). In the failing 5-player launch it
  fires once: `code 0, reason null`, caller `hr+0xA30C5 <- 0xA2E6A <- 0x33AD2 <- 0x33F4C`, ~28 s after the host starts,
  immediately followed by `EngineEvent reason=0` and the engine unloading.
- That call site is inside `FUN_1800A2E80` (the engine's main/frame path that also calls `FUN_1800A3C74` "game finish"):
  it takes the abort branch when the launch parameter byte is 4/null, computes a code from the launch-mode globals
  (`DAT_182a1fd40/48`, engine mode 6/7) and calls the reporter; in our case the computed code is 0.
- Solo forced-declare tests (1/4/5 players, no peers) show the block itself is NOT rejected: declared 4 and 5 both sit in
  launch state 8 waiting for players; only declared 1 launches. So the engine accepts the declared count and waits; the
  quit happens in the launch/player-acceptance path once the >4 session is present.
- Harness timing insight: the engine's launch window is ~28 s in practice (solo waits get aborted around then when no
  players arrive), so all guests must join inside that window; guests-first breaks the host boot (host engine unloads
  ~3 s after boot), so the harness keeps host-first with tight guest stagger.

Next: trace the `FUN_1800A2E80` abort branch inputs (launch parameter pointer/byte and the computed code) during the
5-player launch, and map the launch-mode globals to the actual failure (player count > 4 in the launch/acceptance path).


### 16-player campaign: launch-frame comparison pins the failure (2026-11-16 05:37 ET)

- Hooked the engine's frame/launch function `FUN_1800A2E80` and compared a working 4-player launch with the failing 5-player
  launch:
  - working (3 guests): `launch_frame block b0 = 1, bC = 2` once, no engine error, mission starts normally;
  - failing (4 guests, all slots reached 7, session pmask 0x1F = 5 players): `launch_frame block b0 = 4, bC = 199` then
    `ENGINE ERROR code 0 (reason null) caller hr+0xA30C5 <- 0xA2E6A <- 0x33AD2` and the engine finishes/unloads.
- In `FUN_1800A2E80` the abort branch is taken when the launch-parameter byte is 4: it computes an error code from the
  launch-mode globals (mode 6/7 paths; in our case 0) and reports it, then the main loop runs the "game finish" routine
  (`FUN_1800A3C74` at 0xA3E7C) and unloads. So the launch parameter is a *quit/failure* descriptor created upstream in the
  main loop - the actual >4 rejection happens before this frame, when the engine decides the external launch failed.
- The 5-player launch does reach the point where every guest slot is launch-ready (7) and all five players are in the
  session; the engine then rejects the external launch and returns to menus rather than seating five players. This is the
  concrete remaining cap: the engine's external-launch validation (game-engine start) for campaign with >4 players.

Next: find the code in the main loop that builds the failure launch descriptor (caller chain 0x33AD2/0x33F4C) - that is
where the >4 campaign launch is refused.


### 16-player campaign: launch failure is an engine "restart/return-to-menu" path (2026-11-16 05:39 ET)

- The failing launch takes `FUN_1800A3FFC`: it calls the "game finish" routine, builds a launch context, then calls
  `FUN_1800A4330` which unconditionally sets `ctx[0] = 4` ("restart/quit" mode), and finally `FUN_1800A2E80(ctx)`, whose
  `param[0] == 4` branch reports an engine error (code 0) to the host and lets the main loop finish/unload the engine.
  A working launch never enters that path (its frame call has `ctx[0] = 1` and no error report).
- So the >4 launch is refused *before* this point: the engine decides the external launch failed (most likely the c_game_engine
  start/validation for campaign with more than four players) and tears the session down via the restart path rather than
  seating five players. All four guest slots do reach launch-ready (7) and the session holds all five players before the
  refusal - the plumbing is correct, the engine's own launch validation is what rejects it.
- Practical next avenue: research existing Halo MCC/Reach campaign >4 mods/tools for the known patch targets, then verify
  the candidate offsets against this build; that is faster than continuing to walk the menu/restart control flow.

Probe code (uncommitted): launch-frame hook, engine error reporter hook, slot-state setter, player-create counter/barrier,
configurable wait window, stage-initiator stack capture.


### 16-player campaign: restart-context mechanism mapped (2026-11-16 05:43 ET)

- The engine's restart/return-to-menu mechanism is `FUN_1800A2CE4(ctx)`: it copies a 0x1EAA0-byte game context into
  `DAT_1827CD400`, sets the pending-restart flag `DAT_1827CD3EF = 1`, and the next frame `FUN_1800A2D50` passes that
  context to the frame function `FUN_1800A2E80`. If `ctx[0] == 4` the frame takes the error/quit branch that reports
  engine error code 0 and lets the main loop finish the session.
- Contexts are built by `FUN_18000D2AC` + `FUN_1800A4330` (which sets `ctx[0] = 4` and fills the campaign launch
  template) and copied by callers in `FUN_1800A445C` / `FUN_1800A450C` before the restart.
- Working 4-player launch: the frame call carries `ctx[0] = 1` (normal in-game frame), no error report.
  Failing 5-player launch: the restart context with `ctx[0] = 4` is installed and the frame reports error code 0 and
  shuts the engine down. So the failing launch ends up in the engine's "restart with campaign context" path instead of
  the in-game path - the launch acceptance decision (which path to take) happens upstream.
- Reference point: Project Reclaimer (recompiled native engine with overhauled networking, source not published) ships
  16-player Reach campaign co-op with mid-mission joins and cinematic handling, confirming the target is achievable but
  requiring engine-source-level work rather than a single runtime patch.

Next: hook `FUN_1800A2CE4` to log caller stacks and `ctx[0]` for every restart-context installation in working vs
failing launches - the caller that appears only in the failing run is the launch-rejection site.


### 16-player campaign: launch variability and 4-vs-5 player difference (2026-11-16 05:48 ET)

- Comparing working (3 guests) and 5-player (4 guests) launches with the restart-context hook: in both runs the engine
  installed the restart context from the same caller stack (hr+0x40B490 <- 0x40A0D4 <- 0x40739D <- 0x3AE716, the game
  engine start path) with ctx[0]=1, and the frame ran normally.
- The 5-player run did not abort the engine this time: it reached "game session started", but only three guests were
  seated before the start (game-player table 4 entries: host + 3 pending), and the fourth guest joining late triggered a
  session disband. So the same root condition keeps producing different outcomes depending on when the last guest joins:
  - all four guests present at launch acceptance -> engine aborts via the restart/quit path (previous runs);
  - only three guests present -> engine launches with 4 players; the fourth disbands the session.
- This confirms the engine's campaign launch acceptance caps the launch at four players total (host + 3 guests); the
  fifth player cannot be part of the launch no matter how the guests are timed. That is the exact check to locate and
  patch next.

Next: hook the launch-acceptance path itself (the game engine start caller 0x40739D / 0x40A0D4 / 0x40B490 region) and
find the comparison that rejects five players, then patch it.


### 16-player campaign: failure chain pinned to the launch-data vtable decision (2026-11-16 05:52 ET)

- In the failing 5-player launch the failure context (ctx[0]=4) is installed from this exact stack:
  `FUN_1800A445C (builds ctx + ctx[0]=4, installs restart) <- hr+0x40C482 <- hr+0x407480 <- hr+0x407305`.
  hr+0x407480 is inside `FUN_180407438` (the launch reset/stage function) and hr+0x407305 inside
  `FUN_18040719C` (the launch tick). So the launch tick takes its failure branch and tears the launch down.
- The launch tick's success/failure decision is the vtable call in `FUN_18040719C`:
  `plVar1 = *(param_1 + *param_1*2 + 2); iVar4 = (**(code**)(*plVar1 + 8))(plVar1, param_1[1]);`
  iVar4 == 0 -> failure cleanup branch (FUN_18044F8B4 x3 + FUN_180407438 reset -> FUN_1800A445C failure ctx),
  iVar4 == 1 -> success branch (FUN_1804076CC next stage). Our probe on `FUN_180407C84` shows it always returns 1, so the
  actual accept/reject value comes from that launch-data object's vtable method.
- Next: capture the dynamic vtable method address from the launch-data object during the launch tick, hook it, and log its
  inputs/result in working vs 5-player launches. That method's false return is the >4-player rejection.

Probe additions: launch decision/proceed/reset hooks, restart-context caller stacks (failure path captured with ctx[0]=4).
Ship increment still f7a2c74.


### 16-player campaign: acceptance vtable ruled out; failure branch is in FUN_180407438 (2026-11-16 05:55 ET)

- Added a runtime dynamic detour on the launch-data object's accept/reject method (vtable+8). It resolves to hr+0x72640,
  a generic method shared by several engine objects; during the launch tick it is called from hr+0x407362 and always
  returns 1 (20/20 samples, including the failing-flow frames), so the launch acceptance decision is NOT this value.
- The 5-player failure context (ctx[0]=4) is installed from `FUN_1800A445C <- hr+0x40C482 <- hr+0x407480`; 0x407480 is
  inside `FUN_180407438` (the launch stage/reset function called by the launch tick). That function branches on the
  session states `*(int*)(session+0x60EF8)` (3/5/7 -> proceed, 8/9 -> extra flags, 0 -> reset, else fail) and calls
  `FUN_1804073E8(param_1, stage, param_1[1], a4, a5)` for each transition; the failure context comes from that call path
  via the function at ~hr+0x40C482.
- Next: decompile the function at hr+0x40C482 (the caller into FUN_1800A445C) and instrument `FUN_1804073E8` arguments in
  a failing launch - that pins which branch/stage value produces the failure context for five players.

Probe additions: dynamic launch-data method detour mechanism (reusable), launch decision/proceed/reset hooks.
Ship increment still f7a2c74; docs mirrored.


### 16-player campaign: probe-hook bug found and fixed - earlier launch failures partly self-inflicted (2026-11-16 06:03 ET)

- Critical: the launch-stage detour `FUN_180407438` was declared with one argument but the engine function takes five
  (context, newState, newSubState, a4, a5). The detour's logging clobbered RDX/R8/R9 and the forward call passed garbage,
  corrupting every launch-state transition while the probe was installed. Several "engine refuses >4" observations were
  likely exaggerated by this. Fixed: the detour now takes and forwards all five arguments.
- First run with the corrected detour (4 guests, declared roster 5, create-count barrier): transitions
  `cur 0 -> state 1 sub 2`, then `1 -> 2 sub 3`, `2 -> 3 sub 4`, `game session started`, no ENGINE ERROR, three remote
  player datums created (the 4th guest still joined too late and the mission ran with four players).
- So the engine launch path is cleaner than previously measured. Next: re-run the strict 5-player launch (all four guests
  ready inside the launch window) on the corrected build and confirm whether five players now pass launch acceptance.

Probe fix only; shippable increment remains f7a2c74.


### 16-player campaign: launch state machine mapped; cap detector identified (2026-11-16 06:07 ET)

- Launch state-machine object table captured (context+state*8+8 -> object -> vtable):
  state 1 m30 hr+0x409A90, state 2 m30 hr+0x40A008, state 3 m30 hr+0x40BBC0, state 4 m8 hr+0x40C36C / m10 hr+0x40C3DC /
  m18 hr+0x40C470. Failing sequence in the 5-player launch: `3 -> 4 sub 5` then, as soon as the fourth guest connects,
  `4 -> 1 sub 6` and the engine error/teardown.
- State 4's per-frame update `hr+0x40C36C` is the cap detector: when a flag at context+0x5C bit 10 is set it reads the
  launch-data object's status via `FUN_18044F8B4` (byte 0) and its error code via `FUN_18044F8DC` (dword +4). Expected
  status is 4; anything else triggers the reset transition with that error code (we observed 6). With five players the
  status becomes non-4 and the engine aborts the launch.
- FUN_18044F8B4/FUN_18044F8DC read from the object at `*(*(ctx+0x30)+0xC0)` via its vtable+0x90 accessor: status byte at
  +0, error dword at +4. So the next target is whatever sets that status to non-4 when a fifth player is present
  (candidate: host session launch apply with a four-player validation).

Next: hook FUN_18044F8B4 to capture the launch-data/status object, then hardware-watch the status byte/dword during the
5-player launch to capture the writer stack that sets the failure code.


### 16-player campaign: rejection is a membership-vs-launch-data comparison (2026-11-16 06:10 ET)

- With the launch status probe the failing 5-player sequence is exact: status goes 0 -> 1 -> 2 -> 3 -> 4 (ready, state 4)
  and then, right after the fourth guest appears, the state-4 update calls the status setter back to 1 from
  `hr+0x40C200` (`FUN_18040C0C0`), which flips the launch to the failure path (error code 6, reset state 1 -> 1 sub 6).
- The two rejection guards in that function are:
  1. session state in 6..9 AND `FUN_18044F6E0(session)` false-or-true AND context flag bit 10 AND
     `FUN_18039EBF0(session+0x30, 0, 0)` false -> status = 1 (start rejected);
  2. a launch timeout branch (`DAT_180D1FC84` seconds) that checks the local peer record bit 13 and sets status = 1.
- `FUN_18039EBF0` is the concrete mismatch: it walks the session's peer mask (0x11 peers), and for every present peer
  compares `recordField == *(uint*)(launchData+4)` and a per-peer bit in a launch bitmask; the first mismatch returns 0.
  So the failure is "session membership does not match the launch data the engine is holding" - with five players the
  fifth member has no matching entry in that launch data, and the launch is rejected.
- Next: instrument FUN_18039EBF0 to log the per-peer compared values and the launch-data dword/mask during the 5-player
  launch, to identify which side is only four players (the launch data source to extend).

Probe additions: launch status setter hook with caller stacks; launch state object/method table.
Ship increment still f7a2c74; docs mirrored.


### 16-player campaign: guests-first clean baseline + 4th guest timeout (2026-11-16 06:16 ET)

- With the detour bug fixed, the guests-first harness now works: host boots normally, launch states 0->1->2->3, three remote
  player datums created, game session starts with **four active players** (`0..3 f00000001 t0`), no engine error.
- The fourth guest, however, fails on its own side with `EngineEvent reason=4 external_launch_individual_state_timeout`
  before the host start - the same four-player boundary appears client-side: the fourth client does not receive valid
  external-launch state in time (it would be the fifth player).
- So the 5-player rejection now presents consistently: three remote guests launch cleanly; the fourth cannot complete its
  external launch. Next experiments should capture the fourth guest's launch state machine (state object transitions and
  status values on the guest) instead of only the host side.

Probe state: launch state objects/transitions, launch status setter, membership-vs-launch-data compare hook, engine
error/restart hooks, corrected multi-arg detours. Ship increment still f7a2c74.


### 16-player campaign: true 5-player rejection captured in detail (2026-11-16 06:19 ET)

- Reproduced the real 5-player rejection with the fixed probes: host session reaches `peermask 0x1F` (five players), then
  the launch aborts (`ENGINE ERROR code 0`, state 4 -> 1 sub 6, error code 6).
- The membership-vs-launch-data comparison (`FUN_18039EBF0` / session_compare) during that window:
  `mode 0 bit 0 -> 0 peers 5` with per-peer record values `A=00000005` for all five peers and handshake bits
  `B=1` appearing progressively; once all five peers' bits are 1 the comparison returns `1` (success) at 06:18:57.078.
- Immediately at that moment the launch status is set to `1` from `hr+0x40C200` (`FUN_18040C0C0`, the
  "all players ready -> start session" branch), and the state-4 update then treats status != 4 as failure and resets
  with error code 6 -> engine aborts. For a 4-player launch the flow advances past state 4 before this session-start
  branch fires, so status stays 4 and the direct launch path succeeds.
- Interpretation: with five players the engine sits in launch state 4 long enough for the network session-start path to
  engage; that path does not complete under our host emulation and the state-4 update turns it into the abort.
- Next candidate patch: suppress the `hr+0x40C200` status=1 transition (or the state-4 abort check
  `hr+0x40C36C`'s status requirement) for campaign launch with >4 players and see whether the direct launch path
  continues and creates the fifth player datum.

Probe state: launch status/state-object hooks, session compare hook, corrected multi-arg detours. Ship increment f7a2c74.


### 16-player campaign: two candidate patches tested - neither seats five (2026-11-16 06:26 ET)

1. Suppress the "all players ready -> session-start" status transition (status 1 from hr+0x40C200):
   - the launch no longer aborts immediately and the membership comparison passes (session_compare -> 1 with five peers),
   - but the launch then sits in launch state 4 with no player datums created; guests time out (~60 s) and the session
     disbands. So suppressing the transition only hides the abort.
2. Skip the whole session-start routine (hr+0x40C0C0) for campaign hosts:
   - the direct launch path advances (state 4 reached, then "game session started" at +33 s),
   - but no player_create calls happen, no player table exists, and the engine then fails with
     `ENGINE ERROR code 3 _map_state_none`; the staged Command 11 arrives after the engine already failed/disbanded.

Interpretation: when more than four players are present the engine intentionally switches from the direct campaign launch
path to a **network session start path** (context bit 0x400 + launch status 1). That path is the correct one for >4, but it
does not complete under our host emulation/transport. The remaining work is to find what that session-start path expects
from the host/transport (messages, synchronisation) and provide it - or to re-create the direct launch path for five
players by understanding why the engine chooses session start only when >4 players are present.

Probe additions this round: session-start tick hook (skippable), status suppression, detailed session_compare logging.
Ship increment still f7a2c74.

### CONSOLIDATED STATE / NEXT STEPS (2026-11-16 06:30 ET)

**Shipped increment (commit f7a2c74):** campaign co-op joins are stable at the stock four-player size. Verified 3- and
4-player launches on m10 (all players active, team 0, no aborts). Both gates build clean.

**Working (research) tree (commit 16a5ee3):** env-gated diagnostics only; default behavior unchanged. This includes the
launch state-machine/status/comparison instrumentation, player-create counters, wait barriers and experiment switches.

**Blocking condition for 16 players:** the engine switches from the direct campaign launch path to a network session-start
mode as soon as more than four players are present (launch states 3->4, context bit 0x400, launch status 1, session-start
routine hr+0x40C0C0). That mode does not complete under the current host emulation/transport: the state-4 update aborts
with error 6, or (with the mode suppressed) the launch stalls and the session disbands. Four-player launches never enter
that mode and stay on the proven path.

**Next steps (in order):**
1. Determine what the session-start path expects from the host/transport (messages, synchronisation) and provide it; or
2. Find the check that makes the engine choose session-start only when >4 players are present (state-3 decision
   hr+0x40BBC0, membership comparison hr+0x39EBF0 with bit mask 0x18) and patch it so the direct launch path serves 5+.
3. Then implement the extra-player spawn-in (after the cinematic, mission-start area) as scoped by Tyler.
4. Reference point: Project Reclaimer ships 16-player Reach campaign co-op, but on a recompiled native engine with a
   completely overhauled network stack (source unpublished); that confirms feasibility but not a small-patch path.

**Harness notes:** probe-jip.ps1 has -Premission (host waits peers), -GuestsFirst (pre-warmed guests; the host boot
 tolerates it after the detour-corruption fix), staggered starts, and per-guest logs. One-machine 5-instance runs are
 flaky: up to four machines connect, the last one often misses the join window. The documented 2-machine 16-player test
 plan remains the realistic acceptance route.


### 16-player campaign: session-state gate + state-4 probes; harness variance dominates (2026-11-16 07:15 ET)

- Finding: the campaign session state is already 6 at engine boot (not a late transition), so the state-3 session-start
  decision (hr+0x40BBC0) sees state 6..9 in every launch; what differs with five players is that its readiness comparison
  (hr+0x39EBF0, bit mask 0x18) comes back true, which routes the launch into status 4 / session-start mode.
- Added a session-state hold gate (clamp session state >=6 to 5 while the launch is still in states 0..3,
  env NOBLE_COOP_HOLD_SESSION_STATE) - it did not fire because the session is already 6 before launch starts.
- Added a launch-context field watch (env NOBLE_COOP_CTX_WATCH, watches DAT_1827CD400+0xEC) and a transition blocker
  (env NOBLE_COOP_BLOCK_STATE4). The hardware watch only arms the thread that installs it, so it missed the launch
  thread writes; the blocker did not fire in the runs because they only had 3 guests connected (no state-4 transition).
- One-machine four-guest reliability is now the main experimental obstacle: only one run in the last several reached the
  real five-player condition (peermask 0x1F). The engine-side blocker remains as characterized: >4 players -> status 4 /
  session-start mode -> abort with error 6.
- Next: (a) hook the explicit transition requester FUN_1804073E8 to capture who requests launch state 4 (with args/stack),
  and (b) improve four-guest connection reliability (stagger/boot order) so the five-player condition is reproducible.

Ship increment f7a2c74; diagnostics committed in 16a5ee3; current probe changes uncommitted.


### 16-player campaign: bypassing the session-start mode works - launch now starts but peers time out (2026-11-16 07:53 ET)

- Found the exact mode-switch decision: launch state-3 update `FUN_18040A648` requests the session-start state 4 whenever the
  launch status byte is not 2/3/0x15 (status 4 or 1 both do it). The state-4 transition then aborts with error 6.
- Experiment: for campaign hosts with the >4-player condition latched (session player mask popcount >= 5, sticky), report
  the launch status as 3 (env NOBLE_COOP_FAKE_STATUS3). Result across three consecutive runs:
  - no state-4 request, no session-start abort;
  - launch proceeds: states 1->2 sub2, 2->3 sub2, then `game session started` (~34 s);
  - but no remote player datums (`player_create` count 0), and ~26 s later the engine aborts with
    `ENGINE ERROR code 4 external_launch_individual_state_timeout`.
- So the session-start mode is now successfully bypassed; the remaining failure is the individual external-launch state
  timeout (a peer/launch-data handshake does not complete without the real status-4 announcement path). Next: compare what
  the host sends/applies when status is 4 vs 3 (launch data broadcasts and the peer state machine), and satisfy that
  handshake while keeping the direct path.

Probes: peer-gated sticky status fake; transition requester capture (state 4 requester = hr+0x40A6BD in FUN_18040A648).
Ship increment f7a2c74; diagnostics committed in 16a5ee3.


### 16-player campaign: blocking the state-4 request avoids abort but launch stalls (2026-11-16 08:00 ET)

- Cleaner probe: instead of faking the launch status globally, block only the engine's state-4 transition request
  (`FUN_1804073E8` with state 4, env NOBLE_COOP_BLOCK_STATE4_REQ) once the five-player condition is latched.
- Result across runs (five-player condition reached in two attempts):
  - the state-4 session-start transition is refused (10 request blocks observed);
  - the engine does not abort with error 6 and `game session started` is reached (~33 s);
  - but still **no player datums** are created and the engine later fails with
    `ENGINE ERROR code 3 _map_state_none` (the mission never actually loads for the participants).
- Combined with the earlier fake-status result (`external_launch_individual_state_timeout`), this shows that merely
  bypassing the session-start mode cannot work: the engine's direct launch path does not create player datums for the
  five-player case; the launch-data/mission-start handshake happens inside the session-start path.
- Conclusion for the next step: the session-start path itself must be made to complete under our host emulation. The
  status-4 -> state-4 transition sets context bit 0x400 and calls the session-start routine (hr+0x40C0C0) which starts the
  network session (per-peer session-start helpers, launch-data comparison) and sets launch status 1; the state-4 update
  then aborts because status != 4. The open question is what the session-start handshake waits for (peer acknowledgements /
  session-start messages) and why it never completes in the headless transport.

Ship increment f7a2c74; diagnostics commit 16a5ee3; latest probes uncommitted. Next: capture message traffic and engine
callbacks inside the session-start path (without 4-player shortcuts) and find the missing acknowledgement.


### 16-player campaign: session-start mode closes/renegotiates the network session (2026-11-16 08:02 ET)

- Captured the natural (no-shortcut) five-player launch window with full message tracing:
  - launch reaches state 3, sets launch status 4, requests state 4 (hr+0x40A6BD) and enters state 4;
  - at that same moment the engine sends `msg 7` (12 bytes) to peers and **closes every channel with reason 10**
    (state was 5, caller hr+0x429BEE), then sets launch slots to 9 and runs the membership comparison repeatedly
    (`session_compare mode 0 bit 0` with five peers).
- So the >4 launch path is not just "another launch mode": entering state 4 makes the engine **renegotiate the network
  session** (leave/start handshake, msg 7 + channel closes reason 10). Our host emulation/transport does not complete
  that renegotiation, so peers drop and the launch fails (engine error; no player datums created).
- This is the concrete missing capability behind the 16-player campaign cap: support the network session-start
  renegotiation (or make the engine skip it while still creating the extra player datums - the direct path does not).
- Next: decode the session-start renegotiation (msg 7 semantics, close reason 10 path at hr+0x429BEE, and what the engine
  expects back from peers) and implement/simulate it in the transport+host interface so the five-player session can
  complete its start handshake.

Ship increment f7a2c74; diagnostics commit 16a5ee3; latest probes uncommitted. Docs mirrored.


### 16-player campaign: session-start mode renegotiates the session; slot-count init is the lead (2026-11-16 08:10 ET)

- Natural five-player trace: entering launch state 4 sends `msg 7` to peers and closes every channel with reason 10
  (caller hr+0x429BEE), then sets slots to 9 and re-runs the membership comparison. So the >4 path is a **network session
  renegotiation**, not just a different launch mode.
- Bypassing it (status fake or state-4 request block) avoids the error-6 abort but the direct path never creates player
  datums and the engine ultimately fails (`_map_state_none` / `external_launch_individual_state_timeout`).
- Lead for the root cause: the campaign session's slot counts are never initialised in the headless flow. The
  `set_slot_counts` SigScanner pattern no longer matches this DLL build (silent miss, no hook installed), so the session
  keeps its default four-player settings while the launch data declares five - a settings mismatch is the plausible
  trigger for the engine's session restart.
- Scanning the DLL for the old pattern gives no exact match; seven loose prefix candidates exist
  (rva 0x13624, 0x105940, 0x216E50, 0x3164C8, 0x3DB104, 0x428AAC, 0x4FAD7C). The PC function must be re-identified
  (decompile-based: a function taking (this, privateSlots, publicSlots, ranked) that stores the slot counts) and then the
  session slot counts initialised to 16 before peers join.

Next: identify the PC set_slot_counts address (or the membership slot-count fields) and initialise the campaign session
for 16 slots before the five-player launch; if the engine then keeps the direct launch path, player datums should be
created for all five. Otherwise, implement the session renegotiation handshake in the transport.

Commits: f7a2c74 (ship increment), 16a5ee3 + a1c3e72 (env-gated diagnostics, default behavior unchanged). Docs mirrored.


### 16-player campaign: launch-ready dispatch probe narrows the trigger to session settings (2026-11-16 08:22 ET)

- Instrumented the two launch-readiness dispatchers that FUN_18040A008 (launch-data apply) feeds into:
  - four-player launch: dispatcher A (hr+0x40B67C) returns 1, dispatcher B (hr+0x40A81C) returns 1, the launch clears and
    proceeds on the direct path - no launch status 4, players created directly.
  - five-player case: forcing dispatcher B to return 1 (env NOBLE_COOP_FORCE_READY_B) still results in the status-4
    transition and the state-4 session-start request, so dispatcher B is not the trigger.
- Remaining trigger inside FUN_18040A008 is the launch-data "settings changed" flag (local variable cVar5 from
  FUN_18044FFE0 / the applied-launch-data comparison). That matches the session-settings (max players) mismatch theory:
  the session still has four-player settings while the launch data has five players, so the engine marks the session as
  needing a restart and enters the network session-start mode.
- Consequence: the lead fix remains initialising the campaign session slot counts to 16 before the launch. The
  set_slot_counts SigScanner pattern no longer matches this DLL build, and the PC decompiled files carry no symbol names;
  the x360 symbol index places set_slot_counts between network_configuration_notify_changed, reset_membership, add_peer
  and remove_peer, which is the localization hint for its PC counterpart.

Commits: f7a2c74 (ship increment), 16a5ee3 + a1c3e72 + 7d2791c (env-gated diagnostics). Both gates clean. Docs mirrored.


### 16-player campaign: PC set_slot_counts recovered; campaign slot hook fixed and verified (2026-11-16 08:44 ET)

- The `c_network_session_membership::set_slot_counts` hook had silently never installed on the current haloreach.dll build
  (its SigScanner pattern no longer matches). Identified the PC function from the retail symbol's body shape:
  **FUN_18039C5C8 (hr+0x39C5C8)** - stores privateSlots at membership+0x14, publicSlots at +0x18, ranked byte at +0x1C,
  calls `managed_session_modify_slot_counts` (FUN_180400A50) and bumps the membership counters. The hook now falls back to
  that proven RVA when the pattern misses.
- Fixed the campaign branch of the expansion: campaign sessions are capped at 16 slots (they were being routed into the
  128-slot multiplayer branch because the engine passes publicSlots=16), and the engine's zero-public-slot join lock during
  a launch is preserved. Without the fix the setter churned 1000+ times per launch, the launcher stalled in launch state 1
  and the engine failed with error 2.
- Observed natural campaign slot traffic: the engine sets public slots 16 at session setup and 0 when the launch begins
  (join lock). It does not appear to ask for four.
- Regression verified: standard 4-player campaign launch (host + 3 guests) still works - 4 players active, game session
  started, no engine errors, exactly two slot calls (16, then lock 0). Both gates build clean.
- Five-player runs still take the state-4 session-start path and abort (same as before), so slot counts alone are not the
  discriminator; the campaign launch state machine's >4 decision is elsewhere (state-3 readiness/launch-data status).

Commit 1a6846b (functional slot-count correction). Ship increment f7a2c74. Docs mirrored.


### 16-player campaign: five-player discriminator is the machine-identity/launch-data check (2026-11-16 08:54 ET)

- Refreshed dispatcher traces in the same run now give a precise five-player discriminator:
  - four-player launch: dispatcher A (hr+0x40B67C) -> 1, dispatcher B (hr+0x40A81C) -> 1, direct launch proceeds.
  - five-player launch: dispatcher A -> 1, dispatcher B -> **0**; B -> 0 sets launch status 4 (session-start mode).
- Forcing B to 1 does not help by itself: the status-4 setter is then reached from **FUN_18040B4C8**
  (call site hr+0x40B65B via hr+0x40A121). That function compares a local machine identity (TLS block +0x10/+0x1C/+0x2C)
  against the launch data returned by the session object's vtable+0x90 getter; on mismatch it calls
  FUN_1803951AC and sets launch status 4, returning -1. Four-player launches match this identity; five-player
  launches mismatch, which starts the network-session restart.
- So the remaining discriminator is the identity/launch-data comparison (not slot counts): either the local identity
  fields differ in our five-player setup, or the launch-data getter returns a different player/peer descriptor once a
  fifth player is present.

Next: instrument the identity comparison inputs (local TLS block fields vs the launch-data descriptor bytes) in 4- and
5-player runs, then align the field that differs (likely the host machine id / player descriptor at block+0x2F8 or a
TLS-side identity field).

Commits: f7a2c74 (ship increment), 1a6846b (slot-count fix), db4f3f3 (probes). Both gates clean. Docs mirrored.

## Files / evidence

- Test harness: `build\bin\Release\probe-lan32.ps1` (new, test-only), refreshed `build\bin\Release2\noble-dev.exe`.
- Logs: `build\bin\Release\host-lan32-low.log` / `-high.log`, `build\bin\Release2\client-lan32-low.log` / `-high.log`,
  single-window probe `probe-slot32.log`.
- No source commits for item #3 yet; repo still at `8f0fe85`, version 0.19.103, no release.

## 2026-11-16 — 5-player campaign co-op: dispatcher-B and launch-info caps lifted (engine path)

Starting point: a five-player campaign launch aborts because the direct-launch validator (dispatcher B, `hr+0x40A81C`,
called by the launch-data apply `hr+0x40A008` after dispatcher A `hr+0x40B67C` returns 1) returns 0; the apply then sets
launch status 4, the launch state machine enters network session-start mode (state 4), which closes every peer channel
with reason 10 (`hr+0x429BEE`) and shuts the engine down.

Two distinct hard four-player caps were found inside dispatcher B's path. Both are now lifted by 3-5 byte in-memory
patches in `LobbyPatches.cpp` (haloreach.dll is never modified on disk):

1. `hr+0x40AD63` — the mode-1 (campaign co-op) branch of dispatcher B loads the session player count
   (`ctx+0x1738`) and executes `cmp edx,4 / movzx eax,bl / mov edi,esi / cmovle edi,eax`: the routine's overall verdict
   (EDI) is only kept when the count is <= 4. Patch `hr+0x40AD6B` `0F 4E F8` -> `8B F8 90`
   (cmovle edi,eax -> mov edi,eax ; nop) makes the campaign branch accept any count; 1-4 players behave exactly as
   before.
2. `hr+0x57C90` — `FUN_180057C90(info)` is the launch-info validator dispatcher B calls before it starts the session.
   It has two separate limits: `hr+0x57D7F` `mov ecx,4 / cmp eax,ecx / jg` on the info's `+0x18` field (raised to 16),
   and the decisive per-mode limit at `hr+0x5804A`: `mov ecx,4 / cmovnz r14d,ecx` keeps 8 for mode 2 (Firefight) and 4
   for mode 1 (campaign co-op), after which `cmp playerCount,r14d` rejects the fifth player's launch data. Patch
   `hr+0x5804A` `B9 04 00 00 00` -> `B9 10 00 00 00` raises the campaign limit to the engine's 16-player buffer
   capacity.

Runtime evidence (five players in the session, `peermask/pmask = 0000001F` at launch state 3): `probe dispatcher A -> 1`,
dispatcher B input `mode 1 count 5 mask 0000001F`, `launch_info_validate -> 0 ... mask254 0000001F` (stock), forced to 1
for the probe, `probe dispatcher B -> 1`. With B returning 1 the host then closes all four peer channels with reason 10
about 0.13 s later (`hr+0x429BEE <- hr+0x405CAE <- hr+0x42379D <- hr+0x3AEA27 <- hr+0x34F0A <- hr+0x2FA43 <-
hr+0x3002D <- hr+0x31357 <- hr+0x141E7 <- hr+0xA38A8 <- hr+0xA313B <- hr+0xA2E6A`): the next wall is the network
session-start path itself, not the launch-data validation. Guests see `ENGINE ERROR code 0` and exit; the host proceeds
alone into the mission.

New probe instrumentation (all env-gated behind `NOBLE_JIP_PROBE`, default off): dispatcher B input dump
(`NOBLE_COOP_DUMP_B`: mode, session count/mask, launch-block fields, every vtable+0x90 accessor with ok flags),
launch-info validator logging and a local check replication (`launch_info_validate ->`, `launch_info_checks`: header
bytes, player mask, pairwise 6-byte player-data compare, slot flags/datum bytes), plus probe switches
`NOBLE_COOP_FAKE_COUNT4` (temporarily fake the session count to 4 around dispatcher B) and
`NOBLE_COOP_FORCE_VALIDATE` (accept launch info after the stock validator rejects it).

Harness work: `build\bin\Release\probe-camp5.ps1` gained `-Guests` (3 = four players, default 4 = five) and
`-GuestsFirst` (pre-warm guests before the host so the launch window sees the full roster); both probe scripts got a
focus guard so launched test windows cannot steal the desktop foreground (the user's mouse stays on their own screen).

Next: follow the session-start close path from `hr+0xA2E6A` / `hr+0x141E7` to find the >4 refusal on the
network-session side (player join/seat creation), then the campaign spawn/cutscene handling for extra players (spawn at
mission start / beach, invisible during the cinematic, per the user's approved design).
