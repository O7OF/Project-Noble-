# NOTE-MP16-AUDIT - Noble code audit for a 16-player multiplayer session (2026-11-16)

Scope: milestone 1 of the multiplayer workstream - **16-player multiplayer end-to-end** (real peers, match starts,
gameplay, join/rejoin, clean exit). This is a **read-only code audit**: no noble-dev/noble-client instance was
started, no source or build artifact was changed. Owner: mp16-audit (shared task task-4).

Baseline: repo `D:\Project-Noble-Closed-Source`, branch `wip-merge`, HEAD `4bfe827` ("Lift campaign co-op 4-player
launch caps and add >4 launch probes", committed 2026-11-16 10:33 -0500), version 0.19.103, no release.
Current binaries: `build\bin\Release\noble-dev.exe` built 2026-11-16 10:28; `Release2..Release5` still have the
**09:07** build (see the test-setup caveat at the end of section 3).

Engine truth: Ghidra MCP (`mcp__ghidra__*`) returned "fetch failed" for the whole audit window, so every engine-side
statement below comes from the repo's own decompile/RE notes and from existing logs; engine-side items that need
Ghidra are marked UNKNOWN and handed to the Ghidra task (task-1).

---

## 0. Verdict in one paragraph

**16 players is exactly the stock engine capacity** (16 game-player slots, 17 peer entries, peer/player masks fit a
single 16-bit mask, the stock membership message #26 is a fixed 0x2550 = 9552-byte payload), and **Noble has no
hard-coded cap that blocks 16**: the MAX PLAYERS picker offers 16 (default selection) and `-max-players 16` is
plumbed into the lobby advert. **But the current HEAD very likely breaks the normal-slot MP join path before player
count even matters**: commit `1a6846b` (08:43 today) restored the `set_slot_counts` hook that had silently stopped
pattern-matching, and its multiplayer branch expands **every** MP session from 16 to 128 public slots ~30 times/s.
The last known-good 2-peer MP run (`mp-host.log`/`mp-client.log`, 01:41, built before that commit) has **no**
`set_slot_counts` log line; every MP run after the restore shows the 16->128 churn, and the only post-restore 2-peer
MP runs (`base2` 10:51 and `base2s32` 10:53) fail at the join with `CLOSE channel reason 10` + guest
`CONNECT REFUSED reason 3`. That is the same failure signature the high-slot (peer 17/player 16) test shows.
**So: 16-player MP is not blocked by any 16-player limit in Noble, but the current tree's MP session slot-count
expansion is a strong suspect for gating even 2 players; fix/verify that first.** Everything else that limits a real
16-player run is test logistics: this PC can only hold ~5 instances (22.2 GB of 31.8 GB measured for 5), and there
is no headless or synthetic-peer mode to fake the other 11 peers.

---

## 1. How max-player counts flow (UI / command line -> engine session)

Two independent paths exist; only the second one reaches the engine session, and today it ignores the requested count.

**A. Noble's lobby/advert count (works, but is advisory only):**

1. CLI: `-max-players <n>` -> `hostMaxPlayers` (src/main.cpp:1128 default 16, parsed at src/main.cpp:1205-1206).
2. UI MAX PLAYERS picker -> `st.sessionMaxPlayers` (src/ui/FrontEndInternal.hpp:437-438, picker in
   src/ui/FrontEndPickers.cpp:3649-3678) -> `SetupHostLobby(..., maxPlayers, ...)` -> `st.lobbyMaxPlayers`
   (src/ui/FrontEndLobby.cpp:244-271, assign at :271) + `HostConfig.maxPlayers` (:293).
3. Lobby launch -> relaunch args `-max-players N` (src/ui/FrontEndMenu.cpp:7326) -> host process
   `hostMaxPlayers` (src/main.cpp:1205) -> `HostConfig.maxPlayers` (src/main.cpp:1966) ->
   `MasterAdvert.maxPlayers` (src/main.cpp:1978) -> master server (src/network/MasterServerClient.cpp:768-769).
4. The lobby roster count / expected peers also come from this path: `choice.expectedPeers` = number of non-host
   roster entries (src/ui/FrontEndLobby.cpp:1936-1940) -> `-wait-peers N` (src/ui/FrontEndMenu.cpp:7327-7328) ->
   `expectedPeers` (src/main.cpp:1207-1208) -> `BlamEngine::LaunchMap(..., expectedPeers)` (src/main.cpp:1947).
5. **`-max-players` does not gate admission anywhere.** It is not read by `NetworkTransport`, and there is no
   "session full" check outside the engine. 17 instances can try to connect to a `-max-players 16` host.

**B. The engine's session slot count (what actually decides capacity):**

- `LobbyPatches::s_maxMultiplayerPlayers` is a compiled-in **128** (src/hooks/LobbyPatches.cpp:59). The public
  setter exists (src/hooks/LobbyPatches.cpp:3372-3376, clamp 16..128) but **has no caller anywhere in the repo**
  (grep: only the declaration src/hooks/LobbyPatches.hpp:15 and definition LobbyPatches.cpp:3372), so it is
  effectively a constant.
- The engine calls `c_network_session_membership::set_slot_counts(private, public, ranked)`; Noble detours it at
  `hr+0x39C5C8` (src/hooks/LobbyPatches.cpp:2894-2913, pattern-first, proven-RVA fallback at :2900-2903).
- The detour (`Hook_SetSlotCounts`, src/hooks/LobbyPatches.cpp:67-95) rewrites the public slot count:
  - multiplayer/Forge (i.e. everything that is not campaign co-op): if the engine asks for 16 or 8 slots, it is
    rewritten to `min(s_maxMultiplayerPlayers, 128)` = **128** (:82-85);
  - campaign/Firefight: clamped to <=16 and the engine's zero-count launch lock is preserved (:72-81).
  `LobbyPatches::IsCampaignCoop()` is set from the launch mode (src/main.cpp:1839), so **MP and Forge always take
  the 128 branch**.
- Because the engine re-issues `set_slot_counts(16, ...)` continuously, the detour churns: the post-restore logs show
  ~30 expansions per second (3674 lines in a ~2-minute MP run, 1331 in the guest log; see section 3).
- Consequence for the 16-player goal: a 16-player session advertises **128** engine slots while the actual
  membership tables/serializers remain stock (17 peer entries / 16 player entries - see
  src/hooks/EnginePatches.cpp:800-813). For exactly 16 players that is *probably* harmless (the guest was accepted
  at 2 in the pre-restore build), but it is the behaviour that changed between the working and failing MP runs.

- Related: "validated network session peer capacity" patch (src/hooks/LobbyPatches.cpp:2915-2926) writes
  `static_cast<uint8_t>(s_maxMultiplayerPlayers)` over the `cmp eax, 0x10` immediate - i.e. it would write 0x80
  (-128 as a signed imm8). **It does not install on this build**: the pattern is not found, there is no
  else-branch log, and no log line "Patched validated network session peer capacity" exists in any current log.
  Treat it as a silent no-op / UNKNOWN, not as protection.

---

## 2. Findings table (file:line + verdict)

Verdicts: **GATES 16** = blocks/fails a 16-player attempt; **RISK** = could break it or invalidate a test;
**OK** = verified harmless for 16; **UNKNOWN** = needs engine truth or a run.

### 2.1 Session / max-player plumbing

| # | Finding | file:line | Verdict |
|---|---------|-----------|---------|
| 1 | `s_maxMultiplayerPlayers = 128`, baked in; `SetMaxMultiplayerPlayers()` has no callers | src/hooks/LobbyPatches.cpp:59, :3372-3376; src/hooks/LobbyPatches.hpp:15 | RISK: `-max-players` never reaches the engine |
| 2 | MP branch rewrites every 16-slot session to 128 slots (campaign branch clamps to 16) | src/hooks/LobbyPatches.cpp:67-95 (:82-85) | **GATES 16 (strong suspect)** |
| 3 | Detour installed via pattern + proven RVA fallback; restored today in commit `1a6846b` | src/hooks/LobbyPatches.cpp:2894-2913 | RISK (new behaviour vs known-good MP run) |
| 4 | Peer-capacity compare patch is a silent no-op on this build (pattern miss, no log) | src/hooks/LobbyPatches.cpp:2915-2926 | UNKNOWN / no-op |
| 5 | `-max-players` only feeds `HostConfig`/advert; no admission control in transport | src/main.cpp:1966-1978; src/network/SessionManager.cpp:107-120 | OK for 16 / RISK for honesty |
| 6 | Co-op launch-cap gates and campaign launch-info caps are byte-patched for every launch (intended campaign-only, applied unconditionally) | src/hooks/LobbyPatches.cpp:3152-3238, :3240-3253 | UNKNOWN (engine semantics); not needed by MP |
| 7 | Campaign co-op MAX PLAYERS row clamped to 4 (irrelevant to MP; do not unclamp for this milestone) | src/ui/FrontEndPickers.cpp:2581-2615 (`kCampCaps[]={4}` at :2586) | OK (not MP) |
| 8 | MP MAX PLAYERS picker = {4,8,12,16,24,32}, default index 3 = 16 | src/ui/FrontEndPickers.cpp:3649-3678 | OK: 16 selectable/default |
| 9 | `sessionMaxPlayers`/`lobbyMaxPlayers` defaults 16 (firefight 16, campaign 4) | src/ui/FrontEndInternal.hpp:391,436-439 | OK |
| 10 | `-wait-peers` wait loop deadline default 45 s, env-extendable 15..300 s; on timeout it starts the sim anyway (does not abort) | src/engine/BlamEngine.cpp:1991-2077 (:1996-2000, :2074-2077) | RISK for one-machine N: guests must fit the window |

### 2.2 Lobby / roster / server browser

| # | Finding | file:line | Verdict |
|---|---------|-----------|---------|
| 11 | Lobby roster entries are vectors/JSON, no fixed 4/16 array in Noble; roster parse is unbounded | src/ui/FrontEndInternal.hpp:441; src/ui/FrontEndLobby.cpp:24-43; src/network/MasterServerClient.cpp:1105-1146, :1332-1373; src/ui/FrontEndMenu.cpp:6915-6954 | OK |
| 12 | Lobby roster panel renders only 8 rows at a time (fixed panel height, scrolls; entries built dynamically, 128-cap) | src/ui/FrontEndLobby.cpp:2580-2581, :2602-2605, :2637-2671 | OK (UX: 16 players needs scrolling; screenshots may show only 8) |
| 13 | Lobby header count = `roster.size()/lobbyMaxPlayers`, slot count = `clamp(max(lobbyMaxPlayers, rosterCount), 1, 128)` | src/ui/FrontEndLobby.cpp:2591-2593, :2602-2604 | OK for 16 |
| 14 | Server-browser counts are plain ints (list `%d / %d`, fill bar, details row) | src/ui/ServerBrowser.cpp:1172-1185, :1399-1400 | OK for 16 |
| 15 | LAN discovery beacon hardcodes `players=1` (the field is a literal `1`), so a LAN browser shows 1/N regardless of real count | src/network/SessionManager.cpp:173-185 (:180) | RISK (honesty; not a join gate) |
| 16 | Online advert player_count is real: `GetGamePeerCount()+1` every 60 frames | src/engine/ReachHost.cpp:798-803 | OK |
| 17 | Master list read clamps `player_count`/`max_players` to 0..64 | src/network/MasterServerClient.cpp:680-681 | OK for 16 (would lie for 128 goal) |
| 18 | `-lobby-demo host -lobby-demo-players N` (N clamp 0..128) builds a synthetic lobby roster with no network - useful to verify a 16-row lobby UI on one instance | src/ui/FrontEndMenu.cpp:6449-6467, :6507-6515 | OK (UI-only, not a peer) |

### 2.3 Join / JIP / rejoin / leave / clean exit

| # | Finding | file:line | Verdict |
|---|---------|-----------|---------|
| 19 | Join gates are deliberately opened for every session: join-any branches patched by default (74->EB at `hr+0x397635` and `hr+0x397363`) | src/hooks/LobbyPatches.cpp:3071-3091 | OK for JIP; means host accepts join while loading/in-match |
| 20 | `can_accept_join_request` / `can_accept_player_join_request` refusals are forced to 0 (accept); `acknowledge_join_request` refusal forced to 0 | src/hooks/LobbyPatches.cpp:105-134, :251-264 | OK for JIP |
| 21 | Engine peer boots are suppressed while host/transport active ("keep peer") - stale/dead peers are not removed by the engine | src/hooks/LobbyPatches.cpp:469-501 (:485-499) | RISK at high N: hard-killed guests can linger |
| 22 | Session aborts are suppressed for host/transport sessions (reason 0x05..0x28) | src/hooks/LobbyPatches.cpp:236-248 | RISK: can mask real failures in logs |
| 23 | Host tells guests its simulation is running every 120/300 frames (`session_sync`); MP guests do not self-start Command 10/11, they wait for it | src/engine/ReachHost.cpp:753-763; src/engine/BlamEngine.cpp:2163-2165 | OK (this is what makes MP JIP work) |
| 24 | Guest transport always binds an **ephemeral** port; host binds 11774 - this is why N instances on one PC do not collide | src/network/NetworkTransport.cpp:396-433 (:401-407) | OK: one-machine multi-instance is supported |
| 25 | Peer tables are `unordered_map`/`unordered_set`, no fixed peer-count array in Noble; dedupe buffer 512; per-port receive queue cap 2048 packets, drain 512/call | src/network/NetworkTransport.hpp:196-225, :284-291; src/network/NetworkTransport.cpp:1959-1963, :1981-1983 | OK for 16; RISK if 15 guests burst simultaneously (queue drops) |
| 26 | Clean leave sends a DISC datagram; host removes the peer immediately on DISC | src/network/NetworkTransport.cpp:2024-2046 (:2041-2043); `ShutdownForExit` at :539-547 | OK for menu QUIT |
| 27 | `-quit-after` is a **hard kill**: `TerminateProcess(...,0)` right after flushing the log; no DISC, no session teardown | src/engine/FrameCapture.cpp:129-140, :151-181 (:177-181) | RISK for "clean exit" evidence: it proves no crash, not a graceful exit |
| 28 | Guest relaunch/return path keeps `-connect`/`-session-id` through `-lobby-return guest` args | src/ui/PauseMenu.cpp:191-250 (:242-248) | OK |
| 29 | In-match scoreboard roster for a **direct/LAN** session is only fed from the master-server lobby when `-session-id` exists; a raw `-connect` LAN session has no roster fetch | src/main.cpp:1584-1608 | RISK (UI only): use engine-table fallback (see 2.5) |

### 2.4 Engine patch inventory relevant to >4 players

| # | Finding | file:line | Verdict |
|---|---------|-----------|---------|
| 30 | Membership update decoder (#26, 0x2550-byte payload) is tightened to reject >17 peer / >16 player entries (stock allowed 34/32 into a 17/16-entry buffer). Exactly 16 players + <=17 peers still decodes | src/hooks/EnginePatches.cpp:798-856 (:803-813, :821, :837-840) | OK at exactly 16; **GATES >16** (intended; 32-slot work raises it) |
| 31 | Membership message is fixed-size stock: msg 26 payload 0x2550 = 9552 bytes (confirmed in logs at 1..4 players). 32-slot mode grows it to 0xA300 (41728) via SlotPatch/SlotMessage | src/hooks/EnginePatches.cpp:800-802; logs host-jip.log / mp-host.log | OK for 16 |
| 32 | Observer-channel flush only accelerates msg types 0x1F..0x30 and indexes channels <34 (>= peer capacity) | src/hooks/EnginePatches.cpp:236-262 (:249-252) | OK for 16 |
| 33 | SlotPatch is **opt-in** (`-slot-patch` / `NOBLE_SLOT_PATCH=1`), requires Extended/Full engine limits, and starts up in 16-player mode; without the flag `Install()` returns before patching anything | src/host/SlotPatch.hpp:9-19; src/host/SlotPatch.cpp:727-737, :830-842, :973-983; src/host/Loader.cpp:161 | OK: **16-player MP does NOT need it and it is inert without `-slot-patch`** |
| 34 | `-slot-players 32` additionally requires `NOBLE_SLOT_PLAYERS_UNSAFE=1` (refused otherwise) | src/host/SlotPatch.cpp:739-760, :843-855 | OK |
| 35 | Player pools ("players"/"fire team"/"simulation view data array") are only raised 16->32 when 32-player mode is active; otherwise stock 16 | src/host/PoolResize.cpp:234-276 (:262) | OK: 16 players use the stock 16-entry pools |
| 36 | Engine-limits pinned profile (Full + 1024 AI) is a Tyler decision and unrelated to the player count; pools affected are AI/render, not player tables | src/host/EngineLimits.cpp; knowledge/project-docs/NOTE-EXTENDED-LIMITS-REENABLE-2026-11-15.md | OK (do not change) |

### 2.5 Scoreboard / killfeed / HUD / teams

| # | Finding | file:line | Verdict |
|---|---------|-----------|---------|
| 37 | Noble scoreboard rows are a dynamic `std::vector<ScoreboardEntry>` seeded from the lobby roster (`PauseMenu::SetScoreboardRoster`); no 4/16 fixed row array | src/ui/PauseMenu.cpp:1013-1091 (:1068-1091) | OK |
| 38 | Engine live-read fallback is hard-coded to the stock 16-slot structures: game results table 16 slots stride 0xD8, in-game stats 16 slots stride 0x708, TLS players table accepted only when `maxP <= 16`; loops cap at 16 | src/ui/PauseMenu.cpp:1837-2003 (:1852-1867, :1872, :1934), :2017-2104 (:2022-2023, :2032, :2042) | OK at exactly 16 (at full capacity); GATES >16 |
| 39 | Scoreboard layout is dynamic: team columns derived from rows present, `visibleRows = min(maxVisible, max(14, tallest))`, 8 Reach team styles | src/ui/PauseMenu.cpp:2278-2418 (:2330-2416) | OK for 16 |
| 40 | No Noble killfeed at all (grep "killfeed" = 0 hits); the engine's own HUD/killfeed is untouched by Noble | - | OK |
| 41 | Team assignment: lobby deals players across up to 8 teams (`i % teams`), host pushes team list; launch passes `-lobby-teams` in roster order and re-issues `player_override_team <index> <team-1>` for 3 minutes | src/ui/FrontEndLobby.cpp:508-539; src/main.cpp:2031-2077 | OK for 16 (schedule is O(roster), unbounded) |
| 42 | Engine team/alive counts are clamped to >=1 to prevent false GAME OVER; logged on change | src/hooks/LobbyPatches.cpp:2555-2577 | OK |
| 43 | HUD itself is engine-owned; Noble only overlays loading/pause/scoreboard | src/host/ReachHost.cpp:693-728 | OK |

### 2.6 Test tooling / harnesses

| # | Finding | file:line | Verdict |
|---|---------|-----------|---------|
| 44 | `tools/mp-session-test.ps1` is the right N-instance MP harness: `-Players N` (total), auto-creates `Release2..N` dirs with data junctions, host `-wait-peers N-1`, staggered guests, RAM abort 26 GB, peak working-set sampling, evidence summary + JSON | tools/mp-session-test.ps1:1-20, :75-100, :136-145, :150-164, :166-253 | OK (use this) |
| 45 | **Harness bug**: `Ensure-InstanceDir` copies the exe **only when the instance dir does not already have one**; existing `Release2..5` keep stale binaries. This is exactly what made the 10:51/10:53 "MP broken" runs fail (host 10:28 vs guest 09:07) | tools/mp-session-test.ps1:76-81 (:78); probe-camp5.ps1:46-53,94; probe-jip.ps1:52-53 (this one *does* refresh) | **RISK / invalidates tests**: refresh all instance exes before every run |
| 46 | `build\bin\Release\mp-jip-test.ps1` = 2-instance JIP sanity test (copies host exe into `Release2` only, host without `-wait-peers`, guest joins 25 s later) | build\bin\Release\mp-jip-test.ps1:2-11, :20-23 | OK for 2 peers |
| 47 | `probe-jip.ps1` = campaign JIP probe; refreshes all instance exes, focus guard, `-Premission -GuestsFirst` pre-warm mode, expected-peer barrier wait | build\bin\Release\probe-jip.ps1:41-53, :61-103, :112-150 | OK reference for timing/focus |
| 48 | `probe-lan32.ps1` = 2-instance slot-patch test (host `Release`, client `Release2`, `-slot-patch -slot-players 32`, `NOBLE_SLOT_PLAYERS_UNSAFE=1`, optional `-High`) | build\bin\Release\probe-lan32.ps1:1-30 | OK (not needed for 16) |
| 49 | `qa-regression.ps1` / `qa-one.ps1` = single-instance menu/match QA (captures, timings, crash grep); qa-regression also runs `-lobby-demo` with 8 fake players | build\bin\Release\qa-regression.ps1:11-16, :28-75, :77-94; qa-one.ps1:1-34 | OK background QA; no multi-peer coverage |
| 50 | Focus guard: all harnesses hand the foreground back to the user's window whenever a game window grabs it; game windows open on the secondary monitor via `NOBLE_WINDOW_SECONDARY=1` | tools/mp-session-test.ps1:42-73; probe-jip.ps1:6-39; src/host/Window.cpp:19-32, :403-417 | OK (keep this behaviour) |
| 51 | Low-spec test template: 1280x720, borderless, FPS cap 30, FXAA off, menu supersampling off, master mute + all volumes 0, `throttleUnfocused=true`, `muteInBackground=true` | build\bin\Release\low-spec-settings.json; semantics at src/common/Settings.hpp:41 | OK |

### 2.7 Instance scaling (this PC)

| # | Finding | Source | Verdict |
|---|---------|--------|---------|
| 52 | 5 in-match instances measured **22.2 GB of 31.8 GB** total working set (campaign, low-spec) - the earlier note already concluded 16 local instances do not fit | knowledge/NOTE-PLAYER-COUNTS-2026-11-15.md:93, :207-209, :258, :286-287, :436-437 | **GATES 16 on one machine** |
| 53 | Per-instance in-match working set ~5.3-6.3 GB (MP/current build: 5877 MB host, 5453 MB private at frame 1800; slot32 MP 6274 MB; campaign guests 5.1-5.5 GB) | build\bin\Release\mp-host.log:1800, host-lan32-low.log:1800, host-camp5.log:1800 (Memory lines emitted by src/engine/ReachHost.cpp:814-820) | OK as data |
| 54 | 2-instance MP run peak 9.8 GB total working set (harness measurement) | build\bin\Release\base2-20261116-105147-result.json (`peakTotalGb: 9.8`) | OK as data |
| 55 | Menu-only instance is much smaller (private 3395 MB, working set 1070 MB at frame 1) - RAM cost is the match, not the process | mp-host.log frame 1 | OK |
| 56 | CPU = Ryzen 5 5600G (6C/12T), GPU = RTX 4070, RAM = 31.8 GB; each instance renders its own swapchain at 1280x720/30 fps | measured this session (Win32_Processor / Win32_ComputerSystem) | UNKNOWN: per-instance CPU/GPU/VRAM cost never measured |
| 57 | Audio per instance: voices are muted in the template and voice streams only open when a relay session exists; `VoiceChat::Start()` is called for every process | build\bin\Release\low-spec-settings.json; src/main.cpp:1833; src/audio/VoiceChat.cpp:620-640 | UNKNOWN: 16 simultaneous WASAPI opens untested |

---

## 3. Current MP join failure caught in existing logs (fix before any 16-player run)

Two runs exist on the current tree; they must be reconciled before trusting any 16-player result.

**Known-good normal-slot 2-peer MP (built before the slot hook was restored)** - build\bin\Release\mp-host.log,
build\bin\Release2\mp-client.log, 2026-11-16 01:41-01:42:

- host: `NetworkTransport: Initialized HOST on UDP port 11774 ...` (01:41:38.472); **no set_slot_counts line at
  all** and `LobbyPatches: Lobby expansion active (Target MP: 128, Co-op: 16, Active Detours/Patches: 17)`
  (01:41:37.067) - i.e. the set_slot_counts detour was not installed in that binary;
- guest: `NetworkTransport: Initialized GUEST on UDP port 59740 ...` (01:42:04.512),
  `RECV msg 26 (?) size 9552 decoded true` (01:42:05.524), `SEND msg 35 (?) size 6168` (01:42:05.543),
  `session ... state 4 local peer 1 (record state 8) ... peermask 00000003 pmask 00000003 ... peer states: p0:8
  p1:8` (01:42:05.629);
- host: `RECV msg 35` repeatedly, `SEND msg 44 ... size 53752`, `ReachHost [State]: ... map 2 (running)`
  (01:42:42), 0 crashes in both logs.

**Failing normal-slot 2-peer MP (current build, hook restored)** - build\bin\Release\host-base2-20261116-105147.log,
build\bin\Release2\client2-base2-20261116-105147.log, 2026-11-16 10:51 (from tools/mp-session-test.ps1 with
`-Players 2`):

- host starts and is in game (`game session started at frame 250`, 10:52:07.390);
- guest connects: `Initialized GUEST ...` (10:52:12.853), `RECV msg 26 (?) size 9552 decoded true` (10:52:13.958) -
  then **immediately** `CLOSE channel ... reason 14 (state was 5)` (10:52:13.970) on the guest and
  `CLOSE channel ... reason 10 (state was 5)` (10:52:13.978) on the host (right after `SEND msg 26 size 9552`),
  followed by a repeating host `CLOSE ... reason 4 (state was 3)` loop and 64x
  `CONNECT REFUSED: id 0x1 reason 3 (0x3) caller hr+0x423E2E` on the guest;
- both logs show the new churn: **3674** (host) / **1331** (guest) lines
  `LobbyPatches: Expanding multiplayer/forge session slot count from 16 to 128 (private=0, isRanked=false)`.

**Caveat found while auditing:** the failing run's host exe is 10:28 while `Release2`'s exe is **09:07**
(Get-ChildItem build\bin\*), because tools/mp-session-test.ps1 does not refresh existing instance binaries
(RISK in section 2.6). However the 10:29 campaign 5-instance run also mixed 10:28/09:07 binaries and **did** get
4 players joined (host-camp5.log: `3/3 remote player(s) in session`, `pmask 0x0F`), so a build mismatch alone does
not explain the MP failure; the campaign path clamps slots to 16 while MP expands to 128, which is the suspicious
difference. **Decisive test: host + guest with the exact same current exe.** If it still fails, bisect on the MP
slot expansion (section 6, fix 2).

Also note: the high-slot probe (NOBLE_SLOT_HIGH=1, peer 17/player 16) has the *same* failure signature per the
existing notes (host-lan32-high.log: host adds player 16, removes it ~16 ms later, `CLOSE reason 10`, guest
`CONNECT REFUSED reason 3`) - another reason to believe the 128-slot expansion is putting normal joins onto the
"high slot" code path.

---

## 4. Launch recipe

### 4.1 One machine, N instances (practical maximum on this PC: N = 5; N = 6 if RAM allows)

Use tools/mp-session-test.ps1 (do not hand-roll the ordering). **First fix the tooling precondition**: copy the
current build\bin\Release\noble-dev.exe over Release2..ReleaseN\noble-dev.exe yourself (the harness only copies into
newly created dirs; probe-jip.ps1:52-53 is the code that does it correctly, and mp-jip-test.ps1:5 does it for
Release2).

````powershell
# 0) Preconditions (all instances must run the SAME exe)
$R   = 'D:\Project-Noble-Closed-Source\build\bin\Release'
$exe = Join-Path $R 'noble-dev.exe'
foreach ($n in 2..5) { Copy-Item $exe "D:\Project-Noble-Closed-Source\build\bin\Release$n\noble-dev.exe" -Force }

# 1) Environment (never open test windows on the primary monitor)
$env:NOBLE_WINDOW_SECONDARY = '1'
$env:NOBLE_SLOT_PATCH = ''            # 16-player MP must NOT use the slot patch
$env:NOBLE_SLOT_HIGH  = ''            # must be unset for milestone 1
$env:NOBLE_COOP_WAIT_MS = '120000'    # only if you pass -wait-peers (15..300000 ms allowed)

# 2) Run: host + (Players-1) guests, 20_sword_slayer MP, 16 player cap
powershell -NoProfile -ExecutionPolicy Bypass -File D:\Project-Noble-Closed-Source\tools\mp-session-test.ps1 -Players 5 -MaxPlayers 16 -Map 20_sword_slayer -Mode multiplayer -Seconds 90 -CaptureAt 45 -ScoreboardAt 45
````

What the harness runs (tools/mp-session-test.ps1:136-164):

- host Release\noble-dev.exe with
  `-host -lan -private -max-players 16 -map 20_sword_slayer -mode multiplayer -log host-<run>.log -quit-after <...> -wait-peers 4`
- guests Release2..Release5\noble-dev.exe with
  `-connect 127.0.0.1:11774 -lan -map 20_sword_slayer -mode multiplayer -log clientK-<run>.log -quit-after <...>`

Notes and knobs:

- Guest instances bind ephemeral UDP ports (src/network/NetworkTransport.cpp:396-433), so no `-local-port` is
  needed for guests; only a *host* whose 11774 is taken falls back automatically.
- `-wait-peers 4` makes the host wait up to `NOBLE_COOP_WAIT_MS` before Command 11. On one machine with slow guest
  boots, pass `-NoWaitPeers` instead: the host starts solo (fast), then guests join in progress - the MP JIP path is
  the one proven at 01:41. Do not pass `-wait-peers 15` on this PC (15 guests cannot run).
- `-quit-after` kills each process with `TerminateProcess(0)` (no DISC). For a real leave/rejoin test use the
  pause-menu QUIT on one guest (sends DISC) or Stop-Process one guest, then start a new guest for the rejoin.
- `-capture-at` / `-scoreboard-at` write capture_*.bmp into each instance dir; overlapping windows make screenshots
  unreliable, prefer the log evidence (a 16-row scoreboard is only visible if the lobby roster is present; for
  direct LAN the engine-table fallback fills it).
- Expected peak total working set for N=5: ~22 GB (abort threshold in the harness is 26 GB,
  tools/mp-session-test.ps1:182).

### 4.2 Sixteen players (needs >1 machine or real peers)

On this PC, 16 in-match instances need ~70-95 GB (measured ~4.4-5.9 GB/instance, section 9) - impossible on
31.8 GB. Two workable shapes:

1. **LAN, 4 machines x 4 instances** (cheapest local equivalent): one host machine runs the host + 3 guests; each of
   3 other machines runs 4 guests. Every instance runs the same exe/version. Host:
   `noble-dev.exe -host -lan -private -max-players 16 -map 20_sword_slayer -mode multiplayer -log host.log -quit-after 600`
   (no `-wait-peers`; guests JIP). Guests on every machine:
   `noble-dev.exe -connect <host-lan-ip>:11774 -lan -map 20_sword_slayer -mode multiplayer -log g<N>.log -quit-after 540`.
   Stagger guests ~3-4 s; the host should reach `map 2 (running)` before the last guests connect.
2. **Real peers / lobby flow**: host from the UI lobby with MAX PLAYERS = 16, all 16 join the lobby (master-server
   roster), then host starts. This exercises the intended `-wait-peers 15` flow and the real lobby roster; it needs
   the backend relay and 16 real clients, not this PC.

Both shapes use the same evidence strings (section 5). One-machine 5-player runs are the fast inner loop for setup
and for the join/rejoin mechanics; the 16-player acceptance itself cannot be proven on this PC.

---

## 5. Expected log evidence per stage

All lines are in each process's `-log <file>` (build\bin log level is info; key warnings are always on). "pid"
prefixes are part of the real lines; trimmed here for width.

| Stage | Where | Expected evidence | Source |
|---|---|---|---|
| Lobby (host) | host | `FrontEndMenu: host lobby set up: map '20_sword_slayer' mode 'multiplayer' ...`; `Lobby start broadcast: session='...' map='...' mode='...' ... roster=16` | src/ui/FrontEndLobby.cpp:274-277, :1917-1920 |
| Lobby (guest) | guest | `EnterLobby: joining lobby '...' (<ip>:<port>) ...`; if online, lobby_roster callback -> `PauseMenu: scoreboard roster set (16 player(s), ...)` in match | src/ui/FrontEndLobby.cpp:165-166; src/ui/PauseMenu.cpp:1086-1089 |
| Lobby UI (offline proof) | 1 instance | `-lobby-demo host -lobby-demo-players 16 -capture-at 20`: roster panel shows `16/16`; `FrontEndMenu: -lobby-demo host=True players=16 mode=...` | src/ui/FrontEndMenu.cpp:6449-6467, :6584-6585 |
| Host session up | host | `NetworkTransport: Initialized HOST on UDP port 11774 ...`; `SessionManager: Self-hosting session ... on port 11774 (Mode: Local LAN)`; `Launching Halo: Reach Engine: Target Map='20_sword_slayer', Mode='multiplayer'` | NetworkTransport.cpp:453-456; SessionManager.cpp:118-119; main.cpp:2024 |
| Load/launch | host | `FrameCapture: -quit-after N s armed`; `ReachHost [State]: frame N launch 11 map 1 (loading_map)` -> `map 2 (running)`; `ReachHost [Slot 1 - FrameTick]: game session started at frame N, loading screen off` | FrameCapture.cpp:175; ReachHost.cpp:796; ReachHost.cpp:712 |
| Peer barrier (only with `-wait-peers`) | host | `BlamEngine: waiting for 15 remote player(s) to finish loading and join membership before starting simulation...`; then `BlamEngine: 15/15 remote player(s) in session (membership: X, engine peers: Y, elapsed Z ms); settling 1200 ms ...`; on shortage `BlamEngine: membership wait timed out with X/15 player(s); starting simulation anyway` | BlamEngine.cpp:2006-2007, :2062-2063, :2074-2076 |
| Simulation start | host | `BlamEngine: Posting Command 11 via vtbl+0x18...`; `PostCommand(11) returned: 0x0`; `map 2 (running)` continues | BlamEngine.cpp:2086-2088; ReachHost.cpp:796 |
| Guest connect | guest | `NetworkTransport: Initialized GUEST on UDP port <ephemeral> (Local Machine ID: 0x...)`; `Launching Halo: Reach Engine as Guest connecting to '127.0.0.1:11774' (Map: 20_sword_slayer, Mode: multiplayer, ...)` | NetworkTransport.cpp:453-456; main.cpp:1903-1906 |
| Guest membership | guest | `LobbyPatches: [GUEST ...] RECV msg 26 (?) size 9552 decoded true`; `SEND msg 35 (?) size 6168`; `session ... state 4 local peer K (record state 8) ... peermask 0000FFFF pmask 0000FFFF ... peer states: p0:8 p1:8 ... p15:8` | LobbyPatches.cpp:432-435, :384-387, :2772-2776 |
| N players in session (strongest) | host | `LobbyPatches: game players: hdr ... 00000010 ... 00000010 00000010 ... 0:... 1:...` ... 16 non-empty rows. Header carries player-array capacity `0x10` (16) and used count (two trailing words: `00000002 00000002` at 2 players, `00000004 00000004` at 4); expect `00000010 00000010` at 16 and 16 slot rows (`0:`..`15:`) with `f...` flags. Also `peer states: p0:8 ... p15:8` and `peermask 0000FFFF`/`pmask 0000FFFF` in the session line | LobbyPatches.cpp:2337-2361; :2772-2776 |
| N peers in session | host | `NetworkTransport: [HOST] Registered peer 0x... via NAT punch PING ... (echoed PONG)` once per guest; advert player count uses `GetGamePeerCount()+1` | NetworkTransport.cpp:2100-2110; ReachHost.cpp:800-802 |
| Gameplay | both | frame counters advance, `map 2 (running)` every 60 frames, `Memory: frame 1800 private ... working set ...`, msg 35/26/37/44 traffic continues, **no** `CLOSE channel reason 10`, **no** `CONNECT REFUSED`, no `FATAL EXCEPTION/0xC0000005/STACK OVERRUN` | ReachHost.cpp:787-803, :814-820; LobbyPatches.cpp:2785-2792 |
| Leave (clean) | guest->host | guest QUIT: `NetworkTransport: exit teardown (disconnect sent; UDP socket left to process exit).`; host: `NetworkTransport: [HOST] Remote peer 0x... sent DISC; removed from peer table immediately.` and `game players` count drops by one on the next change log | NetworkTransport.cpp:539-547, :2024-2043 |
| Leave (hard kill / stale) | host | no DISC line; peer keeps being pinged (`Registered peer ...`), engine boot suppressed: `LobbyPatches: keeping peer N in the session (engine boot reason ...)` | LobbyPatches.cpp:485-499 |
| Rejoin | guest+host | new guest `Initialized GUEST ...`; `RECV msg 26 ... decoded true`; `SEND msg 35`; host `game players` back to N / `peer states: p...:8`; no `CONNECT REFUSED reason 3` | LobbyPatches.cpp:432-435, :2772-2776 |
| Clean exit | both | `FrameCapture: -quit-after N s reached, exiting test run` + process exit code 0 + no `FATAL EXCEPTION`/`0xC0000005`/`crash dump written`; `running\<pid>.flag` removed (no "did not exit cleanly" on the next start) | FrameCapture.cpp:177-181; main.cpp:243-255 |
| Failure markers | both | host `CLOSE channel ... reason 10 (state was 5)` then repeated `reason 4 (state was 3)`; guest `CLOSE channel ... reason 14`; guest `CONNECT REFUSED: id 0x.. reason 3 (0x3) caller hr+0x423E2E`; host `BOOT peer N reason 8` loop (boot suppressed) | LobbyPatches.cpp:2785-2792, :278-288, :474-500 |

Quick greps used by tools/mp-session-test.ps1:220 already cover most of these
(`game players:`, `session state`, `SEND msg 26|35|44`, `CLOSE channel`, `CONNECT REFUSED`, `BOOT peer`, `FATAL`).

---

## 6. Top 5 things most likely to break a 16-player test (and how to see it in the logs)

1. **MP session slot expansion to 128 (current tree) - the prime suspect.**
   Detect: > 50 occurrences of `LobbyPatches: Expanding multiplayer/forge session slot count from 16 to 128`
   (a stock 16-player session should log this zero times once the hook is gated), plus `CLOSE channel ... reason 10
   (state was 5)` on the host immediately after `SEND msg 26`, guest `CLOSE ... reason 14`, guest
   `CONNECT REFUSED ... reason 3 (hr+0x423E2E)`. At 16 peers the engine membership tables remain 17/16-sized while
   the session claims 128 slots.
2. **Instance binary drift / wrong test setup.**
   Detect: guest gets `CONNECT REFUSED reason 3` while host loops `BOOT peer N reason 8`; compare
   `(Get-Item build\bin\Release\noble-dev.exe).LastWriteTime` against Release2..N. The harness does **not** refresh
   existing dirs (tools/mp-session-test.ps1:76-81). Always copy the exe and record its hash in the run summary.
3. **`-wait-peers` barrier vs slow one-machine boots / engine external-launch timeouts.**
   Detect: `BlamEngine: membership wait: X/15 ...` then `membership wait timed out with X/15 ... starting
   simulation anyway`; on guests `external_launch_individual_state_timeout` or
   `ReachHost [Slot 4 - EngineEvent]: reason=...` timeouts. The engine has ~60 s individual / ~90 s overall
   external-launch timeouts (knowledge/NOTE-PLAYER-COUNTS-2026-11-15.md:257-258, :816-819); a one-machine 15-guest
   cold boot will not fit, so use JIP (no `-wait-peers`) or pre-warmed guests.
4. **Membership decoder limit exactly at 16 / 17th entry.**
   Detect: host/guest `RECV msg 26 ... decoded false`, `membership consume (payload ... peers N players M)` with
   M > 16, a `game players: hdr` used count above `00000010`, or the host removing a player right after adding it
   (with `-slot-patch` this appears as `SlotPatch: membership producer #1 ... N peers, N players` -> `#2 ... N-1`).
   The decoder hard-caps at 17 peers / 16 players (src/hooks/EnginePatches.cpp:803-813), so a 17th player requires
   the high-slot work, not this milestone.
5. **Stale peers after a hard-killed guest (BootPeer suppression).**
   Detect: no `Remote peer ... sent DISC` for a guest that is gone, host keeps `Registered peer ... via NAT punch
   PING`, `LobbyPatches: keeping peer N in the session ...` lines, and `game players`/`peer states` never drop the
   slot. With 16 peers one or two stale entries push the peer mask/session over the stock 17-entry capacity. For
   leave/rejoin tests exit guests from the pause menu (DISC) rather than Stop-Process; or restart the host between
   test rounds.

Bonus (UI, not a gate): with a direct `-connect` LAN session there is no master-server lobby roster, so the
in-match scoreboard is filled from the *engine* table only (src/main.cpp:1584-1608 vs
src/ui/PauseMenu.cpp:2017-2104); a screenshot may show fewer/blank names early even though the engine has 16
players. `PauseMenu: scoreboard roster set (N player(s), ...)` is the line that only appears when a roster exists.

---

## 7. Ordered fix list (stop at the first item that is actually broken)

1. **Re-run the 2-peer MP baseline with identical binaries** (host + one guest both from the current 10:28 build;
   `tools/mp-session-test.ps1 -Players 2 -NoWaitPeers -Seconds 60` after copying the exe). Expected if fixed:
   guest `RECV msg 26 ... decoded true` + `SEND msg 35`, both sides `peer states: p0:8 p1:8`, msg 44 stream, host
   `map 2 (running)`, no `CLOSE reason 10` / `CONNECT REFUSED reason 3`. If it still fails, go to item 2.
2. **Gate the MP slot expansion** (one-line policy change in `Hook_SetSlotCounts`, finding 2.1 #2): only
   expand MP sessions when the requested maximum is really >16 (wire `-max-players`/`SetMaxMultiplayerPlayers`
   into the host, or add a test env such as `NOBLE_MP_SLOTS_STOCK=1` that forces 16). For a stock 16-player game the
   engine should keep its own 16 slots; expansion is only needed for the 32+ work (where SlotPatch already owns it).
   Then repeat item 1. Expected fix evidence: zero `Expanding multiplayer/forge ... 16 to 128` lines in a normal MP
   run and the 01:41-style join evidence.
3. **Fix the harness precondition**: refresh `noble-dev.exe` in every instance dir in
   tools/mp-session-test.ps1 / probe-camp5.ps1 (copy unconditionally before each run, or check size/hash);
   log the exe hash + version in the run summary. (Otherwise every "regression" can be a self-inflicted mismatch.)
4. **5-instance one-machine smoke** with `-Players 5 -MaxPlayers 16` and no `-wait-peers`: host solo start, four
   guests JIP one at a time. Evidence: host `game players` used count 2->3->4->5, `peer states p0..p4:8`,
   guest `msg 26/35`, `map 2 (running)`, no close/refuse lines for 60-90 s, no crash. Then Stop-Process one guest
   and relaunch it (rejoin) and check the host count drops/returns.
5. **Wire `-max-players` into `LobbyPatches::SetMaxMultiplayerPlayers`** so the engine slot count, Noble's advert
   and the UI picker all agree (today `-max-players` is advisory only, section 1). With the gate from item 2 this
   keeps 16 = stock and makes 32 explicit.
6. **Distributed 16-player run** (4 machines x 4 instances, or 16 real peers) using section 4.2; accept when host
   and at least one guest log `game players ... 00000010 00000010` with 16 slot rows, `pmask 0xFFFF`, no
   close/refuse for 120 s, a clean leave/rejoin, and clean exits.
7. **UI follow-ups (not gates)**: feed the direct-LAN scoreboard from transport peers/engine table even without
   `-session-id` (finding 2.5 #29); make the LAN beacon publish the real player count instead of `1`
   (src/network/SessionManager.cpp:180); give the lobby roster panel more than 8 visible rows or an explicit
   scroll-bar affordance for 16-player lobbies (finding 2.2 #12). Higher-count honesty (64/128 caps) is milestone 4.
8. Only after 16 is clean: continue the high-slot (peer 17/player 16) engine work for 17+ (task-1), then the
   32-slot path. Do not use `-slot-patch`/`-slot-players 32`/`NOBLE_SLOT_HIGH` for the 16-player acceptance run -
   they are not needed and the high-slot path is known broken.

---

## 8. Headless / synthetic-peer / load-test options (none can stand in for real peers)

| Option | What it actually does | Can it help a 16-player test? |
|---|---|---|
| `noble-dedicated.exe` (src/dedicated/*) | Loads haloreach.dll headless but **never starts a game/session**: `LobbyPatches::Apply` is commented out (src/dedicated/main.cpp:78-80), `DedicatedServer::Start` only forwards UPnP, opens RCON and starts a heartbeat thread whose "beacon" is an empty comment (src/dedicated/DedicatedServer.cpp:77-109, :166-177). Console commands are stubs (:134-164). | **No** - cannot host or join a real match today |
| Synthetic peers / fake players / bots | grep over src: no peer-injection path. `SlotMessage::ApplyHighSlotTest` only changes which real slot a joining peer gets; `-lobby-demo` creates a made-up roster in the UI with "no network" (src/ui/FrontEndMenu.cpp:6449-6452); AI spawn tests (`-spawn-test`, `NOBLE_HORDE`) spawn engine AI objects on one instance, not session players (src/host/SpawnTest.cpp:748-754) | **No** - only UI/roster demos and AI load |
| `-lobby-demo host -lobby-demo-players 16` | Synthetic 16-entry lobby roster, no network; lets you capture/verify a 16-player lobby UI, team dividers, count text on one instance | UI verification only |
| `-scoreboard-demo N` / `-scoreboard-demo-players N`, `-postgame-test`, `-postgame-role` | Draws the scoreboard/post-game panel with sample data (demo roster above 16 in src/ui/PauseMenu.cpp:2196-2246) without a session | UI verification only |
| `-test-map-gametypes` | Headless self-test of the map/gametype tables (no MCC, no window) - src/main.cpp:757-761; src/engine/MapGameTypesTest.cpp:9 | No |
| `-voice-test loopback/host/guest` | Headless voice path self-tests (src/audio/VoiceChat.cpp:940-1030) | No |
| tools/mp-session-test.ps1, probe-jip.ps1, probe-lan32.ps1, mp-jip-test.ps1 | The only way to put >1 real engine session on one PC (N <= ~5) | Yes - the one-machine inner loop |

Conclusion: there is **no** way in the current tree to exercise 16 real players with fewer than 16 real engine
sessions; a 16-player acceptance run needs 16 sessions (4 machines x 4 instances is the cheapest LAN shape here).

---

## 9. Instance-scaling numbers (this PC + measured data)

Machine: AMD Ryzen 5 5600G, 6 cores / 12 threads; 31.8 GB RAM; NVIDIA RTX 4070; Windows 11 Home (build 26300).

| Measurement | Value | Source |
|---|---|---|
| 5 in-match instances, total working set (peak) | **22.2 GB of 31.8 GB** | knowledge/NOTE-PLAYER-COUNTS-2026-11-15.md:93 (see also :207-209, :258, :286-287, :436-437) |
| MP host in match (current build, 2-instance run), frame 1800 | private 5453 MB, working set 5877 MB | build\bin\Release\mp-host.log (11/16 01:42) |
| MP host with `-slot-patch -slot-players 32`, frame 1800 | working set 6274 MB | host-lan32-low.log (11/15) |
| Campaign host / guests in match, frame 1800-5400 | WS 5.1-5.5 GB each | host-camp5.log, client2..5-camp5.log (11/16 10:30) |
| 2-instance MP harness peak total working set | 9.8 GB (=> ~4.9 GB/instance) | base2-20261116-105147-result.json (`peakTotalGb`) |
| Menu-only instance, frame 1 | private 3395 MB, working set 1070 MB | mp-host.log frame 1 (`Memory:` line, src/engine/ReachHost.cpp:814-820) |
| Implied max in-match instances on this PC | **~5** (5 = 22.2 GB; 6 ~= 26-29 GB, above the harness' 26 GB abort; 16 ~= 70-95 GB) | derived |
| Per-instance CPU / GPU / VRAM | **UNKNOWN** (never measured in existing logs) | - |

What can be changed test-only to squeeze more instances (do not change shipped behaviour):

- Keep the low-spec template (1280x720, 30 fps, FXAA off, muted) - already the baseline; the RAM cost is the loaded
  map/game state, not resolution, so lowering resolution further will not fix RAM (per-instance private memory
  ~3.4->5.5 GB comes from tags/game state).
- Pick the smallest MP canvas for load tests (empty Forge canvas vs full map) and measure - mapping is the unknown;
  run a 2-instance test first and compare `Memory: frame 1800` lines.
- Disable voice fully per instance in settings (`voiceEnabled=false`, `voiceMuteAll=true`) if WASAPI device limits
  bite at high instance counts (not yet observed; UNKNOWN).
- No per-instance audio/render switches beyond settings exist; each instance still creates its own D3D11 device and
  swapchain at 1280x720/30 fps.
- Windows may overlap all test windows on the secondary monitor - that is fine for CPU/RAM but makes `-capture-at`
  screenshots unreliable (knowledge/NOTE-PLAYER-COUNTS-2026-11-15.md:433-434); prefer log evidence and
  `-scoreboard-at` captures on one instance at a time.

---

## 10. Limitations / open items (hand-off)

- **Ghidra MCP was unavailable** during this audit ("fetch failed"), so the engine-side consumers of the membership
  slot counts (+0x14/+0x18 of `c_network_session_membership`) were not re-verified. The Ghidra task (task-1)
  should answer: who reads the public-slot count, and why a 128-slot MP session corrupts/rejects a normal 2-player
  join (`hr+0x423E2E` connect-refuse caller, `hr+0x405CAE` close-reason caller).
- No game instance was run (hard rule); the "current MP join is broken" claim rests on pre-existing logs
  (base2 10:51/10:53 runs) and the found exe mismatch. The decisive same-binary rerun is fix list item 1.
- CPU/GPU/VRAM scaling for >5 instances is unmeasured - treat instance-count advice as RAM-bound only.
- `-max-players` is not an admission control; if the milestone ever needs a hard cap, it must be added on the
  transport/session side, not just the advert.
- This note is additive; no existing docs were modified.

---

*Audit 2026-11-16 by mp16-audit (task-4). Read-only: no source, build, game instance or existing doc was changed.*
