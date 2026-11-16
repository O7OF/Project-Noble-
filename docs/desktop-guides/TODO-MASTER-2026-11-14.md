# Project Noble - master to-do (updated 2026-11-16: multiplayer first, campaign last)
## PRIORITY ORDER - 2026-11-16 (Tyler): multiplayer first, campaign last

Tyler has re-ordered the remaining work. **Multiplayer is now the front item and is owned by a new DeepSeek session**
(prompt: `project-docs\PROMPT-MULTIPLAYER-AGENT-2026-11-16.md`). Campaign co-op beyond four players is **parked at the
end** of the list and must not be resumed unless Tyler asks. The campaign/engine session hands off with all of its work
committed (`4bfe827`, `f7a2c74`, `1a6846b`, `db4f3f3`) and documented in
`project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md`.

## MULTIPLAYER SESSION STATUS - 2026-11-16 (DeepSeek, live; newest first)

### TYLER APPROVALS 2026-11-16 11:45 ET - status

1. **Dev-only lower-memory profile: DONE + concluded impossible-for-16** (`a3bc5e4`; harness `-LowMem`: 640x360
   borderless + Stock limits + free-RAM guard + system/private accounting). Measured 5.0-5.8 GB WS / 4.7-5.5 GB
   private per instance regardless of resolution/window mode; 8 instances abort the guard at 30.5/31.8 GB, 6 at
   31.65 GB; max proven on this machine = 5 players. 16 needs ~76-88 GB vs 31.8 GB installed -> Tyler must choose
   multi-PC (3-4 PCs x 4-5 instances; harness supports `-connect <host-ip>`) or a RAM upgrade.
2. **Lowest resolution/cheapest display mode: tested** - 640x360 windowed 6.1 GB, 640x360 borderless 5.5 GB,
   1280x720 borderless 5.8 GB, low-mem Stock 5.0 GB; resolution/mode do not reduce engine memory.
3. **Lobby/picker/server-browser honesty: IMPLEMENTED + verified** (`a2c7125`): single seat-capability source
   (16 stock; 32 only dev slot patch + `NOBLE_MP_SLOTS32=1`), `-max-players` clamp + session target wiring, greyed
   24/32 with reason, honest lobby header/adverts/LAN beacon count, scoreboard/loading roster from the engine
   game-player table (2-player LAN shows 2 rows, `live roster 2 player(s)`), server-browser real counts + FULL /
   STALE / NOT HEARD LATELY / 17+ UNSUPPORTED verdicts and JOIN blocking. Remaining: guest display names on
   direct/LAN launches are `PLAYER n` placeholders until msg-27 identity data is used.
### MILESTONE STATUS at 2026-11-16 11:35 ET (5 commits on wip-merge, version stays 0.19.103, no release)

| Milestone | State | Evidence |
|---|---|---|
| 0. HEAD buildable | **DONE** | `175637e`..`5b9d39f`; both gates clean (`build\final-build-2026-11-16.log`) |
| 1. Multiplayer end-to-end (shipped 16-player path) | **WORKS at 2 and 5 players**; 16 real players blocked by one-machine RAM (see hardware ceiling) | stock mode (no dev flags, stock msg 26 = 9552): 2-player `stock2-20261116-112427`, 5-player `stock5-20261116-112548` - host+guests peermask/pmask 0x1F, peer states p0..p4 all 8, game-player table 5/5, msg 44 state stream to every guest, 0 CONNECT REFUSED, 0 crashes; join/leave/rejoin proven in `rejoin5-20261116-111256` (captures in `build\bin\Release\data\userdata\claude-tests-2026-11-11\rejoin5-*.png`) |
| 1b. Shipped-path regression found+fixed | **DONE** | `175637e`: campaign session had made every MP session declare 128 slots (host CLOSE reason 10, guest reason 14, CONNECT REFUSED 3, 4607 churn calls/75 s) |
| 2. High-slot (peer 17 / player 16) join | **IN PROGRESS** - peer now reaches state 8; guest still rejects the follow-up membership delta | `c33ef10` caves the msg-33 player-properties handler (0x44B24C) -> host p17:8; remaining: host delta entry for player 0 carries `peer 0 slot 4` and the guest decoder sees `peer 1 slot 0` -> consumer ret 0, membership reset (`ph5-20261116-112742`) |
| 3. Real 32-player session | **NOT STARTED / hardware-blocked** | 32-slot declare path tested with 2 players (`s32decl`): joins cleanly with 32 declared slots but host churns 3713 set_slot_counts calls/60 s; real 32 players needs 32 engine sessions (~160-190 GB on one machine) = multi-PC |
| 4. Lobby/server-browser honesty | **DESIGNED** | `project-docs\DESIGN-LOBBY-HONESTY-2026-11-16.md`; needs Tyler approval for visible picker/cap changes. Known bug: in-match scoreboard and loading roster list 1 row while 5 players are in the session |

**LOW-MEMORY PROFILE - measured and concluded 2026-11-16 (Tyler approved).** Dev-only profile implemented:
`tools/mp-session-test.ps1 -LowMem` (640x360 borderless template + `NOBLE_ENGINE_LIMITS_PROBE=0` Stock limits + free-RAM
guard + system/private memory accounting; Stock probe allowed only in NOBLE_DEV_BUILD, shipped pin untouched).
Measured one instance: Full 1280x720 borderless 5.8 GB WS / 5.1 GB private; 640x360 windowed 6.1; 640x360 borderless
5.5; **low-mem Stock 5.0 GB WS / 4.74 GB private** (~10% saving - resolution and window mode do not help, the engine's
world/tag data dominates; named pools total 0.08 GB). Multi-instance: 8 aborted the RAM guard at 30.5/31.8 GB used,
6 aborted at 31.65/31.8 GB (guests still loading); 5 players is the proven safe maximum on this machine.
**16 players on one machine needs ~76-88 GB private (16 x 4.7-5.5 GB) against 31.8 GB installed: not achievable on this
hardware.** Reaching 16/32 needs 3-4 PCs x 4-5 instances (harness supports `-connect <host-ip>`), or a RAM upgrade.
**Acceptance path for 16+ players (needs Tyler):** the session/join layer is healthy at 5 players on one machine but
16 full instances need ~80-95 GB working set (measured 5.2-5.9 GB/instance at 1280x720/30 fps/muted, 23.9 GB peak for
5). Options: (a) 3-4 PCs x 4-5 instances (recommended; the harness supports it with -connect to the host IP),
(b) a dev-only lower-memory engine profile for test instances, (c) use the 32-slot patch to seat 5 players in high
slots on one machine as the extended-slot proof, then do the true 16/32 runs across machines.



Repo `wip-merge` @ **`e531160`** (slot-count fix + diagnostics/harness), version 0.19.103, no release. Both gates build clean
(`noble-dev`, `noble-client`). Test rig: `tools\mp-session-test.ps1` (new; N instances, focus guard, low-spec
settings, guest-exe sync, RAM abort, log evidence). Detail: `project-docs\STATUS-MULTIPLAYER-2026-11-16.md`.

**Fixed today - HEAD was broken for every multiplayer join (regression from the campaign session):**
`4bfe827` could not complete even a 2-player LAN session. Commit `1a6846b` restored the
`c_network_session_membership::set_slot_counts` detour by RVA after its signature stopped matching; the hook had
been silently uninstalled, and its multiplayer branch expanded every MP/Forge session from the engine's 16 slots to
`min(GetMaxMultiplayerPlayers(),128)` = 128 (that value is a constant 128 and the UI never sets it). With the hook
live, the host declared 128 slots, closed the guest channel with **reason 10** right after the membership update, the
guest closed with reason 14, every rejoin got **CONNECT REFUSED reason 3**, and the setter churned 4607 times in 75 s.
Fix `175637e`: MP sessions keep the stock 16 slots (expansion only under `NOBLE_MP_SLOTS32=1` + the 32-player slot
patch, capped at 32); the campaign/firefight fallback no longer overrides `publicSlots == 0` (the engine's join lock).
**Verified 2-player:** set_slot_counts 1x, host+guest peermask 0x3 / pmask 0x3, both peers state 8, msg 44 stream,
no CLOSE/REFUSED/BOOT, 0 crashes.

**Verified now - 5-instance LAN multiplayer on the pinned Full+1024 build** (host `build\bin\Release` + guests
`Release2..5`, `20_sword_slayer`, `-slot-patch -slot-players 32`, `tools/mp-session-test.ps1 -Players 5`):
host `pmask 0x1F peermask 0x1F` with peer states p0..p4 all 8, every guest `session state 4 local peer 1..4
pmask 0x1F peermask 0x1F`, host game-player table 5/5 used (rows 0..4, datums EC70..EC74), guest msg 44 state
stream, **0 CLOSE, 0 REFUSED, 0 BOOT, 0 crashes**, all 5 processes alive to the end. Peak working set 23.9 GB.

**Join / leave / rejoin verified at 5 players on one machine** (`rejoin5-20261116-111256`, same build): host
peermask grows 0x1 -> 0x3 -> 0x7 -> 0xF -> 0x1F as the four guests join (each peer 5 -> 6 -> 8), game-player table
reaches 5/5; the fifth guest is hard-killed at t+40 s and the host drops to 4 peers/4 players (peermask 0x0F,
game-player row 4 released), the guest is relaunched at t+52 s, sends a fresh join-request, the host re-seats it
(peer p4:8, pmask 0x1F, game-player table 6/6 with the new datum) and the rejoined guest reaches session state 4
local peer 4 / peermask 0x1F / pmask 0x1F and resumes the msg 44 state stream - **no crash, no CONNECT REFUSED**.
Screenshots: `build\bin\Release\data\userdata\claude-tests-2026-11-11\rejoin5-20261116-111256-*.png`.
**Known UI bug seen in that evidence:** Noble's in-match scoreboard overlay lists only "1 NobleSix" and the loading
roster only "NobleSix (YOU)" while 5 players are in the session - the roster needs to come from the session
membership, not the pre-game local roster (milestone 4 / lobby honesty, design in progress).

**Hardware ceiling for the 16-player goal (measured):** each instance at 1280x720/30 fps/muted peaks at
**5.2-5.9 GB** working set, so 5 instances already use 23.9 GB of the machine's 31.8 GB. 16 full instances need roughly
80-95 GB - **16 players on one machine is not possible on this hardware**. `noble-dedicated` is a stub (no engine
session) and there is no headless/synthetic-peer mode. Milestone 1's "16 players on one machine" therefore needs either
(a) Tyler's multi-PC test (2-3 machines x 5 instances), or (b) a Tyler decision to allow a dev-only lower-memory profile
for test instances. The session layer itself is proven healthy at 5 peers/5 players on one machine.

**High-slot (peer 17 / player 16) root cause narrowed on `175637e`** (`NOBLE_SLOT_PROBE=1` run,
`host-p2high-20261116-110821.log` / `client2-p2high-*`): the remaining failure is a **peer-state promotion gap**, not a
missing slot cave. Normal-slot peer 1 goes state 5 -> 6 -> **8** and the membership delta is accepted by the guest.
With `NOBLE_SLOT_HIGH=1` peer 17 goes 5 -> 6 -> **7** and never 8; the host then sends a membership delta that carries
peer state 7 (delta header 0x1A / base 0x16, sequence check passes), and the guest's membership consumer **rejects** it
(`ret 0`) and **resets its own membership** (peermask 0x2, pmask 0, local peer 1), sends msg 7 (connect-closed) and
closes its channel with reason 14; the host closes reason 10 and later rejoins get CONNECT REFUSED reason 3. The host
join-processing promotion in hr+0x392DB0 (0x392F6E..0x392F84) chooses state 7 vs 6 with `CMP ESI,[block+0x10]`
(session+0x40) before calling set_peer_state hr+0x39C668, so either the 7 -> 8 promotion (0x39349D / 0x39528D /
0x403D46) fails for peer 17 or that state field is wrong. Ghidra task-5 is resolving the exact 7 -> 8 condition and the
patch. **32-slot declared session groundwork tested (`s32decl-20261116-111632`):** with `NOBLE_MP_SLOTS32=1` + the slot
patch, a 2-player LAN session is declared with **32 session slots** (host hook expands 16 -> 32) and still joins
cleanly: host peermask 0x3/pmask 0x3, both peers state 8, guest session state 4 local peer 1 peermask 0x3/pmask 0x3,
msg 44 stream, no CLOSE/REFUSED/BOOT, 0 crashes. Caveat: the host calls set_slot_counts ~62 times/s (3713 in 60 s,
engine re-applies 16, hook re-expands to 32); the guest settles at 32. A real 32-player run needs the engine's own
session-creation slot count raised (or a one-shot latch) to stop the churn - join works today, so this is a
performance/robustness item for milestone 3, not a blocker.

The earlier "host removes player 16 / producer goes 2 -> 1" reading was a red herring: a healthy session's
producer also prints 1/1 because the sender builds two payloads per tick for two sessions. Log-only probes are committed
in `e531160` (`NOBLE_SLOT_PROBE=1`).

**High-slot membership layer FIXED (12:2x) - commit `48cd1d2`.** Found the corruption that made the guest reject the
delta: msg-27 peer-properties used stock peer math (`0x39D23E`/`0x39F04E`/`0x39F0C0`); for peer 17 its 0x120-byte
property copy overwrote player 0/1 records and set player 0's slot dword to 4. The membership wire has a 2-bit slot
field and the engine encoder does not mask it, so slot 4 leaked into the preceding 6-bit peer field (host sent
`peer 0 slot 4`, guest decoded `peer 1 slot 0`), and the checksum mismatch made the consumer reset the session. Added
the three missing caves (47 member caves total) + a producer-side invalid-slot sanitiser (SlotMessage.cpp). Verified
with NOBLE_SLOT_PROBE=1 + NOBLE_SLOT_HIGH=1: peer 17 reaches state 8 on both sides and the guest **accepts** the delta
(ret 1, no reset, msgSeq advances, peermask 0x20001/pmask 0x10001, session state 6 local peer 17 record state 8).
**Next blocker:** the host still boots peer 17 with reason 9 every frame after msg 27 (suppressed by Noble), i.e. the
join-in-progress SEAT path still refuses the player; the guest channel closes reason 14 ~19 s later and the guest
returns to the lobby. Next fixes: cave the still-uncached player-add/accept sites `0x39740C`/`0x397503`/`0x39752A`
(can_accept_player_join_request) and `0x39FB62`/`0x39FB7D` (queued player-add apply) per NOTE-HIGHSLOT sections P2/P3.
**High-slot progress (11:2x) - commit `c33ef10`:** found and caved the missing engine site: the state 7 -> 8 peer
readiness test (FUN_180392DB0 0x393191-0x39349D) requires every player mapped to the peer to have player record +0x1C
set, and the player-properties message (type 33) handler FUN_18044B1DC resolved the owning peer's player table with
stock math at 0x44B24C (imul rcx,rsi,0x56), so the host's shadow player 16 never got +0x1C. After the new cave
(SlotMemberCaves.inc 44 sites, total 111 slot sites) **peer 17 reaches state 8** (host log `p0:8 p17:8`).
**Remaining blocker:** the guest's membership consumer still **rejects the follow-up delta** - payload is now
`1 peers, 2 players`, the sequence check passes (payload+0x0C == block msgSeq 0x16), but the consumer returns 0 and
resets its own membership (peermask 0x2, pmask 0, local peer 1), then sends msg 7 (connect-closed); the host closes
reason 10. The residual reject is the delta's tail consistency/checksum check (probe `5b9d39f` dumps payload+0x2518..
+0x254F). Note: `CONNECT REFUSED reason 3` in our logs means **this process SENT a refuse** (the hook is on the encode
side) - it is the guest refusing later reconnect ids after its session died, i.e. aftermath, not the cause.

Revised priority order:

1. **Multiplayer large sessions** - 16-player end-to-end first, then fix the high-slot (peer 17 / player 16) join
   failure, then 32 players, then plan 64/128 (128 still needs Tyler's explicit go-ahead). New session owns this.
2. Firefight AI/wave options.
3. Native team/spawn hooks + documented mod API.
4. Pathfinding/navmesh baking.
5. Exact per-player visor colour in matches.
6. Selectable armour-effect colour variants.
7. Elite lighting/shine.
8. Forge++ / map-variant / Megalo tooling.
9. **Campaign co-op > 4 (16-player campaign)** - moved to the END, paused. Do not resume without Tyler.



Live list. Newest state at the top. **Read `project-docs\REFERENCE-LIBRARY-2026-11-15.md` before researching anything**
(Ghidra MCP, decompile exports, repo `docs` multiplayer work, external tag/asset folders, test hooks). Detail:
`project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` + `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md`
(engine-upgrade phase); earlier state in `project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md` (05:10 ET) and
`01-PROJECT-HISTORY.md` Phases 8-13.

## CROSS-GAME COMPARISON + TOP-10 - DONE 2026-11-15 12:55 ET (nothing started)

- Cross-game comparison finished for the remaining systems: `project-docs\CROSS-GAME-COMPARISON-2026-11-15.md`;
  raw pool-constructor table `decompile-exports\topics\crossgame-pools-2026-11-15.csv` (391 rows: Reach 110 pools,
  H3 83, ODST 86, H4 112).
- Key results: H3/ODST raised actor 96->256, squad 256->335/512, objectives 64->100/125; Reach's proven pool hook can
  do the same. H4 is the closest sibling (same allocator, 112 pools) and kept Reach's counts but grew per-entity
  records (players 0x490 -> 0xF18, actor 0xD00 -> 0x1380). **No later MCC engine has >16 player slots.** Reach
  already equals H4 / exceeds H3/ODST for particles, effects, decals, lights and cached render states - raising those
  (A2/A11/A12/D10) is not supported by later-game evidence.
- Ranked list written: `project-docs\TOP10-NEXT-UPGRADES-2026-11-15.md` - 1) stabilize/re-enable Extended limits (L,
  High), 2) cross-game-justified pool raises (S-M, Med), 3) player counts >16/co-op >4 (XL, High, Claude+Tyler hold),
  4) Firefight AI/wave options (L, Med), 5) native team/spawn hooks + mod API (L, Med), 6) navmesh baking (XL, High),
  7) exact visor colour (M-L, High), 8) selectable armour-effect variants (M, Med), 9) Elite lighting (M-L, Med),
  10) Forge++/variant/Megalo tooling (L-XL, Med). Honourable: alloc-failure logging (S, Low - fold into #1).
- **Nothing on the list has been started**; waiting for Tyler's choice. No code changes, no builds, no release.
- **Next:** work is resuming on the top-10 list; Tyler will provide the item number(s)/order, then the next session
  restates plan/size/risk and starts with the read-only probe. Handoff section 0 explains the read order.

## DOCUMENTATION PASS - DONE 2026-11-15 12:40 ET (feature work still paused)

- Modder guide: `project-docs\MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md` - all 68 engine hooks with MCC RVAs (measured
  from a live startup log) + what/when/example/limits, the host tooling modules (PoolReport/PoolResize/EngineLimits/
  SpawnTest/MvarDump/FrameCapture), launch/test flags (detailed + an auto-generated appendix of all 216 flag literals),
  the 38-rule game-variant editor and its JSON format, appearance/profile data, and an explicit 'could not verify' table.
- Player guide: `project-docs\PLAYER-GUIDE-FIREFIGHT-MARINE-APPEARANCE-2026-11-15.md` - Firefight setup (10 maps, 7
  game types, MAX PLAYERS 4/8/12/16 unverified beyond 4, difficulty, 38-rule GAME OPTIONS, no skulls/teams in Firefight),
  the Armory Marine tab + 9 Marine/ODST slots, appearance (30 visor finishes -> 5 stock slots in match, 7 selectable
  stock armour effects + 12 Firefight voices, Elite sets, colours incl. custom wheel, nameplates/emblems, rank gates).
- Master upgrade list: every one of the 68 items now marks **PROVEN / FAILED / NOT STARTED** with the MCC address where
  known (7 PROVEN, 3 FAILED, 58 NOT STARTED) - `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`.
- Address table: new 'Proven MCC-only addresses' section (pool init `hr+0x12FCC`, bank table `hr+0x27BF270`, per-player
  tick `hr+0x61A38`, ai_place `hr+0x6C2B10`, object spawn `hr+0x49CC24`/`hr+0x46D6E0`, horde teleport/team/blind hooks,
  saved-film monitor `hr+0x3ABE70`) + startup-patch table - `decompile-exports\topics\mcc-address-table.md`.
- Doc errors fixed while documenting: the earlier player-guide draft claimed Firefight had skulls and that armour
  effects were not selectable; the code inventory shows Firefight forces skulls 0 and the 7 stock armour effects ARE
  selectable (only new recoloured variants are dev-proof). Corrected in the final player guide.
- **Unverified (flagged in the guides):** horde-full-lod/infantry paths, Alert-Carry behaviour, tag_get diagnostics,
  engine-side 128/16 gates, extended profiles, -engine-cmd, individual settings rows, Firefight variant-rule
  behaviour, Armory in-match visuals beyond visor/effects, mod packaging/.mvar round-trip, Forge editor monitor.
- No code changes were needed (no code errors found that the docs depend on); repo still `b2065bd`, version 0.19.103.

## STABILIZATION PASS - DONE 2026-11-15 12:05 ET (feature work paused)

- Scope: full regression of everything since the v0.19.103 baseline; record in QA-VERIFY; fix regressions only;
  clamp/hide crash-capable options; draft release notes + known issues. Docs:
  `project-docs\STABILIZATION-REPORT-2026-11-15.md`, `project-docs\RELEASE-DRAFT-AFTER-0.19.103-2026-11-15.md`,
  `project-docs\QA-VERIFY-2026-11-15.md` (new section).
- Changes since the v0.19.103 release: exactly one commit, `d636b31` (test harness tick gate fix; dev-only). The
  v0.19.103 feature delta (drawer filters, server-browser footer, click fixes, lobby share codes, social button,
  test hooks) was regressed too; per-commit test-proof table is in the stabilization report.
- QA: 16 scenarios, **0 real crashes** - campaign m10 (checkpoint autosave written), Firefight ff_unearthed,
  multiplayer 20_sword_slayer (local host), Forge forge_halo, Armory, server browser + codec self-test (0 failures),
  social, lobby demo with NB1 share code, saved-film playback; plus the same four modes with extendedLimits=3 +
  aiActors=1024 pre-set (build resets to Stock, no black screen, no crash). Captures `qa2_*.png`, post-fix `qa3_*.png`.
- Clamps added, commit **`b2065bd`** (main = wip-merge, pushed; **version still 0.19.103, no release**): campaign MAX
  PLAYERS = 4 only (5-player co-op crashes/unproven); multiplayer MAX PLAYERS picker capped at 32 (16 default; 128
  hold); ENGINE LIMITS information-only Stock (profiles are force-disabled); AI ACTORS shows 96 (STOCK).
- Not covered (needs Tyler/second PC): real two-PC lobby-code join; multiplayer with real peers; campaign >4 repro;
  full save LOAD/resume; Forge editor monitor toggle; Tyler's skulls-row/toggle-crouch retests.
- Recommendation: **hold the release** - no user-facing feature content since v0.19.103; only ship a small stability
  release if Tyler wants the crashable options removed from users' builds. Then bump/publish only on "release it".

## ENGINE-UPGRADE PHASE - DeepSeek (opened 2026-11-15 10:40 ET)

- Read first: `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` (git/release rules + phases 1-6) and
  `project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` (briefing, tasks T1-T10). Pointers at the top of
  `00-START-HERE.md`; candidate work in `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`.
- Done 10:50 (docs only, no `src` change): O7's three requests recorded (`project-docs\NOTE-O7-REQUESTS-2026-11-15.md`);
  `decompile-exports\topics\mcc-address-table.md` started with the 5 proven property-editor matches; Halo CE decompile
  completion (79,637 functions, 2 failed, 2026-11-15 10:19 ET) recorded in `decompile-exports\README.md` + this list.
- Next (phase 1): RTTI/vtable check on `haloreach.dll`; engine-state pool inspector over the ~196 pools; free
  diagnostics (net latency overlay, AI debug draw, crash minidump path).
- Rules: prove with a read-only runtime probe before any write; no release/version bump/public push/tag/upload unless
  Tyler says "release it"; 128-player work ON HOLD until Tyler lifts it.

## PHASE-2 PROMPT - Stage 1 DONE 2026-11-15 11:35 ET; Stage 2 next (preparation, awaits Tyler)

- Report: `project-docs\STAGE1-STATUS-REPORT-2026-11-15.md`; network/security boundary:
  `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`.
- Repo state 11:35 ET: `main` = `wip-merge` = **`d636b31`** (dev test-harness fix; version still 0.19.103, no release).
- Pool inspector proven in-match (124 named pools + 8 banks). Heavy Firefight (ff_unearthed): page reference ranges
  85.7%, script node 80.2%, character properties cache 77.5% (31/40), object 71.9%, water physics 88.5%. Heavy Forge
  (forge_halo, 1,495 spawned objects): havok components 100% (1,500/1,500), water physics 100%, page reference ranges
  91%, object 76.5%, tag cache 74.4%.
- **Blocking finding:** Engine Limits (V16/V16P/Full) are force-disabled in this build (`Settings.cpp:253-261` reset
  + non-Forge forced Stock, upstream `4407101`); Stock game-state banks have only ~5-32 KB headroom in banks 1/3/4 ->
  count raising needs the Stage 3 fix + re-enable, and Firefight/multiplayer re-enable needs Claude's profile matching.
- Fixed the test harness (`d636b31`): the per-player tick was gated on a unit handle instead of the player index, so
  `-horde` / `-spawn-test` / dev cheats / cinematic gating were dead since 2026-11-09. Both gates built clean.
- Next: Stage 2 preparation only (reproduce the 5-player campaign crash and the 34-player MP stop, find the
  non-network causes, clamp UI rows to proven counts, put network blockers in the handoff note). Stage 3 black-screen
  fix before any count raising; 128-player still on Tyler's hold; no release without "release it".

## BUILD/DEPLOY QUEUE - Tyler's ordered bug list (started 2026-11-15 04:30 ET; v0.19.103 released 09:57 ET - see the v0.19.103 block below)

Rule from Tyler: work in this order, build + push to `main` after EACH completed fix; multiplayer is last.
Repo state when this queue was written: `main`=`wip-merge`=`c584c8a` (v0.19.102 was then the public release; all later fixes
are dev-only and the next release is held until the queue is done, per Tyler 06:05 ET). Commit-message style: subject + body paragraphs (no trailers).

1. [REMOVED 07:45] 3rd-person camera spazzing: Tyler reports another user already pushed a fix, so this is removed from
   the queue. No further work unless the issue comes back.
2. [DONE] Armory "scroll options misaligned" turned out to be SETTINGS > GAME OPTIONS camera sliders -> `ec585f0`
   (fixed value column 132*UI scale, labels ellipsized at the slider column). The FOV-scroll-wheel zooming the Armory
   model was fixed earlier in `3a3c98e` (FOV is match-only now; `CameraFov::Apply` no-op while `ReachArmory::IsMenuMode`).
   Evidence: `settings_camera_aligned.png`, `settings_camera_crop.png`, `fov_fixed_*.png`.
3. [DONE] Campaign/custom/firefight/lobby MODDED tabs: collapsed one-section-per-mod banners, click/Accept expands;
   expanded sections fold again via the FOLD action on the mod heading row; search still expands+filter all. `5f99735`
   + follow-up. Evidence `camp_modded_new.png`, `camp_mod_click.png`, `custom_modded_new.png`, `recheck_fold2_*.png`.
   Banner polish (chevrons removed, count/expand spacing) in `8f467e1`; evidence `banner_clean_camp.png`,
   `banner_clean_custom.png`.
4. [DONE] Campaign MAX PLAYERS (4/8/12/16) wired into HOST CO-OP; was hard-coded 16 with no UI. `5f99735`. Evidence
   `camp_players_click3.png`, `camp_players_pick16.png`.
5. [DONE] Expanded map/variants drawer now has an in-drawer **variant search** box and a **category cycler** (ALL MODES +
   the variant's forge/game-mode groups from left/right arrows or clicking the pill), with an "N OF M VARIANTS" count.
   Forge World AI Battlegrounds (353 variants) filters to e.g. "jump" -> 3 rows and "BIG TEAM" -> 11 rows. Evidence
   `drawer_filter4.png`, `drawer_search_jump.png`, `drawer_cat_1.png`. Test hooks `-drawer-filter-test <text>` /
   `-drawer-category-test <n>` prefill the filter for captures. The "?" placeholder was fixed earlier (`5f99735`).
6. [DONE] Custom Games cleanup: mod sections collapse/fold (`5f99735`, `e213497`), in-drawer search/category filter as
   above, ASCII drawer header and clamped campaign search. Variant drawer now usable at 353 entries.
7. [PARTIAL - awaiting Tyler retest] Campaign skulls row click verified with `-menu-click-at` (opens SKULLS screen);
   campaign search clipping fixed (`5f99735`). If Tyler still sees a hover/click failure, it is device/mode specific.
8. [DONE] Server browser Refresh/Favourite (`0efb619`): Refresh now shows "Refreshing server list..."; FAVOURITE/COPY
   INVITE pinned in a fixed footer and verified clickable (`sbfix_click_0/1.png`).
9. [DONE] "What's New" hint clickable (`0efb619`): clickable HintExtra opens the changelog (`main_hint_click.png`).
10. [DONE] Game Options rows fully clickable (`0efb619`): whole row is the click target; verified label click opens the
    options screen (`gameoptions_labelclick.png`).
11. [DONE] Controller toggle crouch (`0efb619`): latches the pad crouch button; settings capture `settings_toggle_rows.png`.
12. [DONE] M&K toggle zoom (`0efb619`): TOGGLE ZOOM setting latches third-person zoom; capture `settings_toggle_rows.png`.
13. [DONE core - 88d4937] Lobby share codes, public/private hosting and prominent "JOIN WITH CODE": 23-char NB1- codec
    (InviteCode.hpp/.cpp, self-test 0 failures), Main Menu entry, Server Browser JOIN WITH CODE modal, pre-game lobby
    HOSTING share-code panel with COPY CODE. Private lobbies stay unlisted but the same code joins them. Evidence:
    `join_code_main.png`, `join_code_modal2.png`, `join_code_lobby.png`, fake-code error capture. Follow-ups: Social screen quick action DONE (`ca770e2`, `social_join_code.png`); only the real two-PC
    host->code->join test remains. Plan: `project-docs\PLAN-LOBBY-CODES-2026-11-15.md`.
14. [IN PROGRESS - static RE done 07:10] ENGINE WORK: exact custom visor colours in matches. Found the model shader-param palette (`visor_tint` = slot 6, id 0x8018f), parameter init FUN_18032596c, colour application FUN_1803246b8/FUN_180324f90 (global appearance table DAT_182a03c58 + index*0x2ae). Notes/next tests: `project-docs\RE-VISOR-COLOR-2026-11-15.md`. Only five engine visor slots exist; extended
    Armory finishes currently map to the nearest stock slot. Goal: each player keeps their own custom finish in-game.
    Options documented in the handoff "Visor colour follow-up": scan the shared tag heap for the spartan_helmet_interior_visor
    / secondary visor rmsh materials and tint them, or locate/extend the engine visor colour table+lookup so per-player
    custom indices render correctly. Bigger RE job; schedule after the lobby-code feature.
15. [PROOF DONE 08:35 - next expose variants] Armor Effect colours: palette-reference patch → blue Legendary flames proven in-game (blueflame_0/1.png), rebuilt Forge World via tool.exe, dev hook `-armor-effect-test` committed (`64e6d22`); original tag/profile restored, blue variant saved as head_fire_blue.particle. Next: #15-B create variant effect + Armory entries. Original assets research: Armor Effect colours (fire + lightning/storm) and NEW colour variants: particle tags are palette-driven; fire palettes `fire.bitmap`/`covenant_fire.bitmap`, lightning/storm uses `plasma_blue_01.bitmap`/`blue_plasma.bitmap`; recipe = duplicate particle/effect tags + recolour palette gradients, ship in a mod, expose new entries (needs engine effect-table work for a selectable list). Notes: `project-docs\RE-ARMOR-FX-COLORS-2026-11-15.md`. Original investigation:
    `objects\characters\spartans\fx\armor_fx` (armor_fx.effect events: legendary_flame/head_fire, eternal_flame,
    storm/thunder_cloud+lightning_flash, pestilence, heart_attack; particles are palette-driven) and expose a listing of
    new colour-changed effect options. Assets dump fine with `tools\runmb.ps1` + `tools\tagdump.py`; see plan doc above.
16. [LAST] Multiplayer: campaign 4 players -> Firefight already 16 -> optional Firefight AI spawn multiplier (2x/3x)
    -> multiplayer 16 -> 128 players. READ FIRST: `docs\limits-players*.md`, `knowledge\drafts\draft-session-networking.md`,
    `docs\limits-ai.md`, `knowledge\desktop-guides\Noble-AI-Limits-Report-2026-11-08.md`; code in
    `src\host\SlotPatch.*`, `SlotMessage.*`, `PoolResize.cpp`, `src\hooks\LobbyPatches.cpp` (`s_maxMultiplayerPlayers=128`),
    `tools\limits\*`. The 32-slot patches are implemented but built-not-tested per those docs.
17. [NEW 2026-11-15 13:10 ET, Tyler] **Pause menu scroll wheel does nothing**: in the pause/match-end menus the mouse
    wheel does not move the selection/list (it should scroll the visible list / move the highlighted row). Reproduce
    with an in-match pause capture on the AOC, *-NoFocus*; likely the pause overlay does not forward the wheel delta to
    its ImGui list or the list is not scrollable. Fix + capture evidence.
18. [NEW 2026-11-15 13:10 ET, Tyler] **ESC pause does not pause the match**: opening the pause menu with ESC leaves the
    game running behind it (no local simulation pause). Expected: ESC pause freezes the local simulation (or clearly
    labels "NOT PAUSED" for modes where it cannot pause, e.g. a live session). Find whether the engine local-simulation
    pause path is skipped or overwritten; hosted sessions may intentionally keep running. Fix the single-player/local
    cases + document the hosted case; capture proof.

Test hooks for the remaining items: `-menu-screen <name> -menu-tab <n>`, `-menu-ignore-input`, `-capture-at`,
`-quit-after`, `-menu-click-at <sec> <x> <y>` (repeatable), `-menu-debug-layout`, `-server-browser-demo`,
`-server-browser-style 1|2|3`, `-armory-focus/-armory-cycle`, `-fov-apply-test-at`, `-loading-roster`, `-wait-peers`.
Evidence captures live in `build\bin\Release\data\userdata\claude-tests-2026-11-11`.

### Extra bug (Tyler, 2026-11-15 ~05:00): custom visor colour shows WHITE in game
- Cause found (Ghidra): only five real engine visor slots exist (0..4); extended Noble finishes were being written as
  invalid slots (e.g. blood_orange = 23) which render white. Armory shows them correctly because it patches rmsh shader
  materials in the *menu* map; the same scan matches **0 materials in campaign/multiplayer maps** (shared tag heap).
- Fixed in `96fd2c4`: every finish now writes a valid stock visor slot (nearest stock colour) so nothing renders white.
  Exact custom colours (e.g. blood orange) still only show in the Armory - getting them into matches needs a shared-heap
  material scan (map-wide tint affects every Spartan) or engine visor-table extension (per-player correct). Ask Tyler
  whether to schedule that engine work; it is bigger than the remaining queue UI fixes.
- Test hook added: `-visor-cam-test` (third-person camera in front of the local player) for future visual checks.

## Reference library (external machine paths - read before researching)

Full index: `project-docs\REFERENCE-LIBRARY-2026-11-15.md`. Key points:
- **Ghidra MCP is connected to `haloreach-copy.dll`** (base `0x180000000`, address = base + repo RVA). Use it for engine
  truth (examples already solved: visor item translator `0x18003B2F4`, patch site `0x2EDC5E`, profile enum map
  `0x1800393CC`).
- Decompile exports for fast text grep: `C:\Users\tyler\Desktop\Reach.exe` (4,119 functions) and
  `C:\Users\tyler\Desktop\host-probe.exe` (5,825 functions); check `decompile-index.json` target_sha256 before trusting.
- Repo `docs` holds the 128/32-player multiplayer research, AI limits, render limits, mod-map mounting
  (`limits-players*.md`, `limits-ai.md`, `limits-render.md`, `mod-map-mount-findings.md`, `limits-players-*.json/trace`).
- `knowledge\drafts\draft-session-networking.md` explains player tables, masks, transport, handshake, join-in-progress.
- External asset/tag tools: `C:\Users\tyler\Desktop\ex` (extracted tags/UI art), `so many tags`, `spectre`, `Pelican`,
  `assembly` (tag editor), `Forgeworks-Halo-3-main`, `reach varient tool and editor`, `Reach game modes and maps`,
  `MVAR` (mvar tool), `Noble-AI-Limits-Report-2026-11-08.md`. `Halo Reach (Jul 24 2010) (Omaha) (Cache Build)` is
  actually Halo 3 cache XEX builds (mislabeled). `menumaps` is empty.
- `C:\Users\tyler\Desktop\Compendium` is a stale mirror of `knowledge`: after editing repo knowledge docs, copy the
  changed files into the matching Compendium path so Tyler/future agents see the same text.

## REPO STATE AT 09:57 ET - v0.19.103 RELEASED

- Repo `D:\Project-Noble-Closed-Source`. **`main` and `wip-merge` both at `eea9868`** (pushed; working tree clean;
  version **0.19.103**). **Release v0.19.103 published and latest**:
  <https://github.com/O7OF/Project-Noble-/releases/tag/v0.19.103>
  - base `Project-Noble-v0.19.103-win64.zip` 357.1 MB, sha256 `81260682...`
  - optional `Noble-ForgeWorld-v0.19.103-win64.zip` 652.6 MB, sha256 `be693143...`
  - verified latest/draft=false, GitHub asset digests match local hashes, version.txt 0.19.103, menu/armour maps present.
- v0.19.103 contains: lobby share codes with COPY CODE / JOIN WITH CODE on Main Menu + Server Browser + Social,
  variant-drawer search/category filters, server-browser Refresh status + pinned FAVOURITE/COPY INVITE footer,
  Game Options full-row clicks, clickable What's New hint, controller toggle crouch and M&K toggle zoom.
- Visor fallback accepted by Tyler: valid stock slots, no white; exact per-player custom visor stays a separate
  engine task. Multiplayer 128 remains on hold. Task-4 (QA/release) complete.

## REPO STATE AT 05:35 ET - v0.19.102 RELEASED (historical)

- Repo `D:\Project-Noble-Closed-Source`. **`main` and `wip-merge` both at `8db1ce8`** (pushed; working tree clean;
  version **0.19.102**). **Release v0.19.102 published and latest**:
  <https://github.com/O7OF/Project-Noble-/releases/tag/v0.19.102>
  - base `Project-Noble-v0.19.102-win64.zip` 357.1 MB, sha256 `bc200798...`
  - optional `Noble-ForgeWorld-v0.19.102-win64.zip` 652.6 MB, sha256 `aca0b6e9...`
  - verified: elite_armor + noble_menu map, zero RR names, version.txt 0.19.102.
- v0.19.102 contains: details wrapping (`d422a74`), FOV guard (`3a3c98e`), mod sections + campaign MAX PLAYERS + search
  clamp + ASCII header + click hook (`5f99735`), banner polish + visor id mapping (`8f467e1`), Settings sliders
  (`ec585f0`), visor stock-slot fallback/no-white (`96fd2c4`), FOLD action + cleanup (`e213497`).
- `noble-dev` / `noble-client` were rebuilt from `8db1ce8`; tests on the AOC with `-NoFocus`.
- RELEASE POLICY (Tyler 06:05 ET): do NOT bump/publish until the whole queue is done - one final release at the end.
  Keep pushing commits to `main`+`wip-merge` after each item.
- Next queue: lobby share codes (13), visor engine colours (14), armour effect colours (15), multiplayer (16).
- Hex-number UI bug runtime check: DONE, theory not confirmed in solo MP/campaign; see
  `project-docs\NOTE-HEX-BUG-RUNTIME-CHECK-2026-11-15.md`. Needs an exact repro screen/mode.
  last (13). Optional bigger task: engine-level custom visor colours (shared tag heap / visor colour table).

## Done 2026-11-15 (this session, after the loading screen/server browser work)

- Campaign/Custom/Firefight/Lobby MODDED tabs: one collapsed section per mod, click/Accept expands; search expands all
  and filters (`5f99735`). Banner chevrons removed and count/"CLICK TO EXPAND" spacing fixed (`8f467e1`).
- Campaign MAX PLAYERS 4/8/12/16 wired to HOST CO-OP (`5f99735`); campaign search box clamped; drawer header "?"
  placeholder fixed (em-dash glyph missing -> ASCII "-") (`5f99735`).
- In-game visors match the Armory finish: shared `src\common\ArmoryVisor.hpp` resolver used by profile + appearance
  record paths (`8f467e1`); four finishes (violet/ice/crimson/onyx) were swapping due to Armory-vs-MCC catalog order.
  Engine truth documented via Ghidra (`0x18003B2F4`, patch `0x2EDC5E`, `0x1800393CC`) in the reference library.
- Settings > Game Options camera sliders aligned: fixed value column + label ellipsis (`ec585f0`).
- Camera FOV can no longer zoom the Armory model (`3a3c98e`); match FOV still applies on match start.
- New test hooks: `-menu-click-at <sec> <x> <y>` (repeatable, unfocused ImGui clicks), `-menu-debug-layout`,
  `-fov-apply-test-at`. Use them to reproduce the remaining click bugs.
- Release v0.19.101 published with both assets; zip verified (elite_armor + noble menu map, zero RR names).

## Done 2026-11-15 (early) - loading screen + server-browser options

- New loading screen (campaign / custom game / firefight / forge): top-left LOADING / WAITING FOR PLAYERS block with
  the lobby roster and progress bar, top-right mode label, complete (uncropped) map picture over a dim soft copy,
  Reach's "Auntie Dot" diamond grid behind the picture (drifting, travelling wave), Noble Team + UNSC emblems bottom
  corners. Logos/grid extracted from HREK via ManagedBlam and kept in `assets\ui\noble\loading` (not committed).
- Host waiting state: after the engine's loading screen ends, `BlamEngine` reports the remote-player membership wait and
  the screen stays up as WAITING FOR PLAYERS with the joined/expected count instead of a black frame.
- Lobby roster names flow to the screen via `-loading-roster` (host first); the mode is passed with
  `PauseMenu::SetLoadingSession`.
- Server browser: three layouts behind `-server-browser-style 1|2|3` (shipped amber tactical cards, large blue-glass
  cards, compact table) for Tyler to choose from; default remains 1.
- Merged upstream v0.19.99 (`45f365d`); only `BlamEngine.cpp` conflicted (membership-wait rework vs our waiting flag).

## REPO STATE AT 19:55 ET (historical)

- Then: `main`/`wip-merge` at `0e8ca81` (v0.19.89), upstream `origin/main` at `a576e43` (v0.19.90, unmerged), and
  v0.19.90 had no Forge World asset so the in-client download row errored. All resolved in the 20:00-20:35 wave.

## REPO STATE AT 10:45 ET (historical)

- Then: pushed tip `a3a4d50` (Trooper third person, untested), local-only merge `4da490a`, uncommitted dedicated-link
  fix + bank-3 guard. Merged-build early session end was solved (beam pools overflowed game-state bank 3; guard added).
  Trooper mode was later removed (7578b41).

## Done 2026-11-14 - late wave (20:00-20:40)

- Merged upstream v0.19.90 (`a576e43`/`8f7a5b8`) and v0.19.91 (`2ec2b49`) into `wip-merge`; rebuilt both gates.
- Theater saved-film playback verified end-to-end with `-film <saved film>`: film map/mode resolved from the film,
  playback ran 50 s with the film director overlay, mainmenu-cruiser bug not present. Captures
  `theater_film_m10_25s.png` / `theater_film_m10_40s.png`.
- Armory after the merge: scene-depth mask still picks 1920x1080; Marine preview in front of the lattice. Evidence
  `armory_after_v01991_42s.png`.
- Match after the merge: Sword Base, elite pack loaded (`relocated 67051 pointers`, extra cache
  `noble_elite_pack` returned 1, `game session started at frame 247`), capture at 45 s, clean quit. Evidence
  `match_swordbase_45s.png`.
- **Forge World download fixed**: (a) attached `Noble-ForgeWorld-*` assets to the v0.19.90 and v0.19.91 releases (latest
  release has the pack again); (b) code fix `aae1675` - `RecommendedMaps` now scans `releases?per_page=10` newest-first
  and uses the newest release that carries the pack, so a base-only release cannot disable the row again. Tested three
  ways with a local release JSON (fallback, direct, no-asset error) plus a real-GitHub selection check.
- Push: `main` and `wip-merge` both at `aae1675` (fast-forward from `2ec2b49`).

## Done 2026-11-14 - first wave (16:30-19:30)

- Trooper prototype REMOVED (CAMPAIGN back to VANILLA / MODDED) (7578b41); cutscene "recovery" opt-in only (b357a93).
- Release download split: Forge World optional asset, `mainmenu_all.map` retired, base zip 1260 -> 607 -> 356 MB;
  updater only accepts the exact client zip (v0.19.85/86).
- MODS > RECOMMENDED MAPS: in-client SHA256-verified Forge World download; tested against a local HTTP server.
- Armory: lattice depth fix (previews in front of the lattice, v0.19.87); Marine/Female Marine Armory without Marine
  mods; species switch instant except Elite settle; menu audio double-gain fix + 15% base music.
- Engine limits default to Stock outside Forge.
- **No RR-named files ship** (v0.19.89): `elite_armor.map` + Noble `NBRL` `.noblereloc`; other RR sidecars gone;
  embedded table `spartan_armor_parts.txt`; verified in a match (127,379 pointers relocated).
- Desktop tester zip: not shipped (Tyler: "we dont need 'new desktop tester pack'").
- Releases v0.19.84 - v0.19.89 published; pushes to main per Tyler's instruction.

## Done earlier today (morning - keep for reference)

- Bank-3 beam guard (v0.19.69 pools overflowed the game-state bank: 68,052,044 of 67,043,328); dedicated-server link
  fix; test-only launch/skip/dump instrumentation; Covenant airstrike palette in Forge World (Tyler confirmed).
- Test harness: tests launch without activation (`SW_SHOWNOACTIVATE`) and never take Tyler's mouse (`-NoFocus`); windows
  on the AOC; Forge World proven served from `data\maps\forge_halo.map`; elite pack 486 -> 286 MB proven A/B identical.

## Needs Tyler's decision / action

1. [DONE 07:55] **Server-browser layout**: style 2 (large blue-glass cards) is already the default in `BrowserStyle()`
   ("Tyler's pick, default"); styles 1/3 stay behind `-server-browser-style` for dev comparisons only. Captures
   `sb_style1/2/3.png`.
2. **Publish a release that includes the loading screen + the RecommendedMaps fallback fix** (`ea572ae`). Bump
   `Version.hpp` + release notes, build both gates, run `tools\publish-release.ps1` (builds base **and** Forge zip).
   Check GitHub first - O7OF publishes to main and releases roughly hourly; coordinate before overwriting a tag.
3. **Tell O7OF to publish with the Forge pack** (no `-NoForgePack`) or run the attach step after each release; the
   already-shipped v0.19.90/91 clients look at the latest release for the pack.
4. **Remaining RR provenance is inside `noble_menu.map`** (`rr_*` HaloScript variable names + RR-era menu-map base).
   Invisible to players/files; removing it = rename variables in HREK + rebuild/spot check (hours, no visible change).
5. [APPROVED 07:45 by Tyler] **Local cleanup (dev only, no user impact)**: stale `build\bin\Release\data\armor\noble_elite_pack.map`
   (286 MB), ~3 GB of `noble_menu.map.*` backups in `build\bin\Release\data\menumaps`, repo
   `data\menumaps\mainmenu_all.map` (285 MB dev fallback, no longer shipped).
6. Test-harness note: `match_elite.ps1 -NoF5` skips the key press but still prints "F5 sent at N s" - cosmetic only.

## Old bugs to double-check (hand checks, need Tyler)

- Forge on a Firefight canvas: fixes landed upstream (e119116). Custom-map missions: work landed (6afd742).
- [REMOVED 07:45] "Respawn in 00000001" (Tyler cannot reproduce it any more) and pause-not-freezing (Tyler says remove).

## Parked

- Flashlight (merged into HREK `spartans.biped`, L key; open value reset to 0 every update in multiplayer).
- Multiplayer two-PC test.
- Armory: knife chests, emblem textures (engine asks host slots 73/74; stubbed, paused by Tyler), undersuit colour,
  Elite shine/lights.
- Forge-palette "Option 2": inject Pelican/Spectre/Boathog/targeting laser into ANY map's Forge palette (like ElitePack)
  instead of shipping the whole 750 MB rebuilt Forge World.
- Marine player animation (model/biped graph): only relevant again if a Trooper-style mode returns through a mod.

### Queue progress (2026-11-15 04:45 ET, `main`=`wip-merge`=`5f99735`)

- DONE 2a: FOV no longer applies in front-end/Armory (`3a3c98e`); 2b armory scroll alignment still needs Tyler's exact spot.
- DONE 3: MODDED tabs (campaign/custom/firefight + lobby map select) show one collapsed section per mod, click expands.
- DONE 4: Campaign MAX PLAYERS row 4/8/12/16 wired into HOST CO-OP (was hard-coded 16, no UI).
- DONE part of 5/6: variant drawer header em dash -> ASCII "-" ("?" placeholder fixed); custom-game mod tabs now collapsed too.
- DONE part of 7: campaign search box clamped on-screen; skulls row click verified working in current build (needs Tyler retest).
- NEW TEST TOOLS: `-menu-click-at <sec> <x> <y>` (repeatable, injects real ImGui click unfocused) and `-menu-debug-layout`.
- NEXT: 5/6 in-drawer variant search + game-mode category filter; 8/9/10 click bugs (click harness ready); 11/12 bindings; 13 multiplayer last.
### Queue progress update (2026-11-15 04:55 ET, `main`=`wip-merge`=`8f467e1`)

- Collapsed-mod banner polish DONE: side chevrons removed; count and "CLICK TO EXPAND" are laid out from measured widths
  with a fixed gap (no overlap/underlap); expand hint hidden on very narrow rows.
- In-game visor mismatch FOUND + FIXED: the Armory catalog order differs from MCC's HR_Visor item order for four
  finishes (UI: violet 7, ice 8, crimson 9, onyx 10; MCC: ice 7, crimson 8, onyx 9, violet 10). ElitePack took the UI
  index straight to the appearance record while ReachHost mapped the profile by id, so those four rendered as the
  wrong finish in game. New `common/ArmoryVisor.hpp` is the single resolver (id -> MCC slot, legacy index remap, stock
  Black/Gold 3/4 swap) used by both paths. Verified mapping: violet->10, ice->7, crimson->8, onyx->9, black->4,
  gold->3; saved blood_orange still slot 23. ReachHost now logs the applied visor slot.
- Known limitation: extended Noble visor finishes (>=5) get their exact Armory RGB forced onto the LOCAL biped only;
  other players see the MCC slot colour (close but not identical). Replicating custom visor RGB to peers is future work.
### Queue progress update (2026-11-15 05:05 ET, `main`=`wip-merge`=`ec585f0`)

- Tyler clarified the "armory scroll options misaligned" bug is actually SETTINGS > GAME OPTIONS camera sliders. Fixed in
  `ec585f0`: the track was sized from the current value text width ("OUT 120 %" rows ended further left than "100 %"
  rows); it now uses a fixed value column (132 * UI scale), so all camera slider tracks share the same start/end. Long
  labels are ellipsized at the slider column instead of running underneath the track. Verified with a Game Options
  capture (all eight camera tracks x=664..968 in the crop, values share one right edge).
- Visor mismatch fixed in `8f467e1` (Armory catalog order vs MCC visor-item order: violet/ice/crimson/onyx now resolve
  by visor id; stock Black/Gold 3/4 swap kept; legacy saves remapped).

### Queue progress update (2026-11-15 13:15 ET, top-10 work on `b2065bd` + local pinned-profile change)

- **TOP-10 ITEM #1 IN PROGRESS.** Tyler chose #1 then #2. Read-only probe disproved the 11-14 "Extended limits cause
  campaign black screens / Firefight-MP kicks" claim (those were the `145510a` launch-block regression, fixed v0.19.95
  `dad10d2`). Probe: campaign m10 renders on Stock/V16/Full and on Full+1024 actors; Firefight Full+1024 and a local MP
  host render; 10-min Firefight soak clean. Evidence + tables: `project-docs\NOTE-EXTENDED-LIMITS-REENABLE-2026-11-15.md`.
- **Tyler's decision:** Extended is pinned for every install (**Full + 1024 AI actors**), ENGINE LIMITS / AI ACTORS
  pickers removed, per-profile campaign save folders (`HaloReach_ext_full_a1024`; old Stock saves untouched).
- Code change set landed as commit **`0848280`** (wip-merge = main, pushed private; version still 0.19.103, no
  release): EngineLimits pin + ActiveSaveSuffix, Settings pin clamp, main.cpp gate removed + fallback errors,
  SettingsPanel pickers removed, ReachHost save folder, PoolResize release pin, publish-release `full.nlp` guard,
  dev-only probe. Both gates built clean (client 13:20:13, dev 13:24:51) and **pinned-path end-to-end passed** in the
  main menu, campaign, Firefight, MP and Forge: Full active, AI actors 1024 (network id 0x03000400), saves under
  `HaloReach_ext_full_a1024`, 0 crashes, 0 fallback warnings.
- **TOP-10 ITEM #1 COMPLETE (Tyler, 13:55 ET)** - remaining QA items are follow-ups: campaign checkpoint save->load
  under the pin, 30-minute acceptance runs, Claude's join-path/backend gaps, and the old-save notice decision.
- New Tyler bugs queued as items **17** (pause-menu scroll wheel does nothing) and **18** (ESC pause does not pause).
  No release/version bump; 128-player hold untouched; join-path/backend gaps handed to Claude in the network note.
- **TOP-10 ITEM #2 COMPLETE (14:15 ET)** - commit `8f0fe85`: squad 256 -> 512, objectives 64 -> 125 via the
  proven pool hook (live campaign report squad 120/512, objectives 28/125; Firefight + Forge 0 crashes; gates clean).
  Layout generation 1 added to the profile network id (0x03010400) so different-layout builds refuse to join.
  Follow-ups: long heavy-Firefight soak, final save/film layout check.
- **TOP-10 ITEM #3 IN PROGRESS (started 14:20 ET, Tyler):** the 128-player HOLD is LIFTED and the lane is widened -
  DeepSeek owns the session/membership/replication work for this item (Claude unavailable). First milestone: the
  32-player patch **installs fully on the pinned build** (110 slot sites, 23 masks, 139 bounds, msg #32 0xA300) with
  pools live at 32 (players/fire team/simulation view); a 2-instance normal-slot LAN join succeeds (2 peers/2 players,
  pmask 3, guest decodes msg 26, acks msg 35, msg 44 stream, 0 crashes). High-slot (peer 17/player 16) still fails:
  host removes the high-slot player right after the ack, CLOSE reason 10, guest CONNECT REFUSED reason 3. Next:
  host-side player add/remove trace. Evidence: `project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md`; no release/version bump.
- **Campaign co-op >4 (same day, 5 local instances, low-spec settings 1280x720/FXAA off/30fps/muted):** the >4 path
  works (host: "co-op roster is over 4; creating the session with capacity 8"), a 4-player m10 session ran live, and on
  Exodus (m50) **all five players formed the same co-op session** (pmask 0x1F, peers state 8, 0 timeouts/crashes).
  Every instance then exited at the peer-ready point (EngineEvent reason=0) without starting the mission - next is the
  launch-state/peer-barrier investigation; in-mission 5-player still to prove.
- **Campaign >4 research (2026-11-16, commit 16a5ee3):** env-gated diagnostics only (default behavior unchanged). Engine
  switches to a network session-start mode for >4 players (launch state 3->4, status 1); that mode does not complete
  under current host emulation. Next: satisfy that path or patch the mode choice (hr+0x40BBC0 / hr+0x39EBF0), then
  extra-player spawn-in. 16-player campaign co-op is not yet working; 4-player ship increment remains f7a2c74.
- **Campaign co-op SHIP INCREMENT (2026-11-16 03:50 ET, commit f7a2c74):** standard campaign co-op joins over the
  lobby/loading window are stable - campaign guests start the engine in the multiplayer engine class (no more
  external-launch abort), players stay on team 0, launches declare the exact roster (stock cap 4) and the barrier waits
  for real network game peers. Verified: 3 players and 4 players all active (`f00002001`, team 0), no aborts.
  **16-player goal is gated by the campaign game-engine start cap (stock 4):** declaring 5+ makes the engine close the
  session at simulation start; declaring 4 keeps it running with the extra joiner tolerated as a non-active membership
  entry. Lifting that cap is the remaining engine work.
- **Campaign JOIN-IN-PROGRESS probe (15:20 ET):** the JIP join path already exists upstream and works - host starts m10
  solo, all four guests joined mid-mission (§RECV msg 26§ true), host game-player table reached 5/16, 0 crashes. Blocker
  found: **joiners land on separate teams** (t1/t2/t3 instead of campaign team 0) so team count 3 -> guests get
  GameResults and exit (~1-2 s). Fix = force mid-mission joiners onto the host's team via the engine team-change path
  (+ suppress guest-side game results during the JIP settle), then re-run probe-jip + 5-player campaign tests.