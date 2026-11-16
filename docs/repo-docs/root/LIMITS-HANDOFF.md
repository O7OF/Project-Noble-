# Extended Engine Limits: handoff

Written 2026-11-08 by Claude (claude.ai session, for Tyler). Read this before any task in
`LIMITS-TODO.md`. Claim tasks in `AGENT-STATUS.md` as usual.

## Status update 2026-11-09 overnight (Claude, Tyler asleep; read first)
- **2026-11-10 00:45 (static review, no test): next blocker found on paper.** Guest waits for its own peer state >= 6
  (0x3903C7) and has 4. The host promotes 5 -> 6 in 0x392DB0, whose peer scans use a register bound
  **0x392E12 `lea r13d,[rbx+0x11]` = 17, unpatched**; gen_bounds.py only raised `cmp reg, imm` bounds. A whole class of
  register-held 16/17 bounds was missed (9 near-certain, ~35 to review). List, evidence and fix bytes:
  `docs/limits-players-regbounds.md`. Do these as one batch, then one high-slot test.
- **2026-11-10 01:00: batch 1 written, NOT built / tested.** `kPeerMasks32` in SlotPatch.cpp + 3 register bounds
  (0x392E12 host join scans, 0x409898, 0x4139E5). Next: `overnight_quick_high.bat` (build + 2-minute high-slot test);
  pass = guest log shows local peer 17 record state >= 6 and `SEND msg 35`, host `RECV msg 27/35`. Five more
  register bounds need a cave or a bigger local first (0x39C922, 0x404E7C, 0x39BABA, 0x40938C, 0x4160F0), ~35 unreviewed:
  same doc. `SlotMemberCaves.inc` was changed 00:10 by the previous session after its last build (not reviewed here).
- **2026-11-10 00:44 test: batch 1 WORKS.** Host: p17 5 -> 6, guest: session state 2 -> 4, local peer 17 state 6, sends msg 22 /
  35 / 27 (first time a slot-17 guest got this far). Second guest (peer 18) the same.
  **New blocker: both guests crash ~40 ms later** in 0x3FF588(parameter at session +0x60A60, 16-byte value, local peer),
  called from 0x3907B4: it copies a 17 x 16-byte session parameter (0x110) to the stack and writes entry [peer]. Peer 17
  hits the stack cookie (STACK OVERRUN +0x3FF666), peer 18 the saved registers (AV at 0).
  **Stopgap written 02:10, NOT built / tested:** MinHook `peer_param_set_17` in SlotPatch.cpp skips the write for peer >= 17
  (32-player mode only, head bytes checked). Real fix: grow that parameter to 32 entries (storage, the 0x110 stack copies
  in 0x3FF588 and its getters, and its network encoding). Find the other users of that parameter before trusting it.
  Also fixed: `overnight_quick_high.bat` used `>/dev/null` (exited before the test); now `>nul`.
- **2026-11-10 02:10 test (stopgap in): no crash, guest joins, then the host's match ends again.** Host gets msg 22 / 35 / 27,
  p17 -> 7, then four msg 32 (encoder 0x427B70 / decoder 0x427BDC: 64-bit id + 2-bit user index = "peer drops its user n").
  The msg 32 handler at **0x42AB43** reads the peer player table `[session + (peer*0x56 + user)*4 + 0x1A4]` with an uncaved
  imul: for peer 17 it read inline memory (0), so it removed **player 0 (the host)**: pmask 0x10001 -> 0x10000, game over
  reason 0, guest drops back to its own session. Same class as the 22:13 fix, different site.
  **Written 02:30, NOT built / tested:** 3 caves appended by hand to `SlotMemberCaves.inc` (0x42AB43, plus 0x394C38 /
  0x394D8A which read the local peer's entry). `tools/limits/gen_member_caves.py` does not know them yet: add them there
  before regenerating. Still uncaved on this path: 0x395E2F / 0x395E6B / 0x395E85 (0x395DE8, read-only), 0x39CB0C, and the
  17 bounds in 0x39C6B4 (0x39C701 / 0x39C769 / 0x39C790) and 0x39C8DC (0x39CAF7 / 0x39CB69 / 0x39CB7D).
  Next test pass = host pmask stays 0x10001 after msg 32, guest stays in session state 4+ and loads the map.
- **2026-11-10 02:17 test (3 caves in; log: 106 sites redirected): host's player 0 SURVIVES, match keeps running, no crash.**
  Host: msg 22 / 35 / 27 + four msg 32, then pmask 0x10001 -> 0x00001 (it removed the *guest's* player 16, as asked) and
  p17 -> 8. Guest then gets msg 26 (1 peer, 2 players), `membership consume` returns 0 and it resets to its own session
  (local peer 1, CLOSE reason 14); host CLOSE reason 10. **Open question = why the guest sends msg 32 at all.**
  A normal-slot guest (ovn-client1.log) sends 22, 35, 27, **33** (136 bytes) and never 32. The high-slot guest sends 27 then
  four 32 (one per user 0..3) and no 33. Sender: 0x394BD8 (per-user update, called from 0x390D07 in the loop that ends
  ~0x390D87) -> remove-local-user code around 0x394D74 (host states remove directly at 0x394E03, a guest sends the
  message from 0x394E0A). So on the guest the local peer's 4-entry player table (peer +0x144; `[session + (peer*0x56 +
  user)*4 + 0x1A4]`) or the players it points to look wrong for peer 17. Check next, on paper first: (1) does the
  membership consumer 0x39E47C / the message extension fill the shadow peer's player table (if it stays 0 or -1 the guest
  drops its users); (2) every other reader of that table in 0x390xxx-0x395xxx (regex `\*4 \+ 0x1a4\]` and the `imul ..,0x56`
  before it) against SlotMemberCaves.inc; (3) the stopgap: the skipped 17-entry parameter (session +0x60A60) might be what
  the guest compares. A guest-side dev log of the 4 table entries for the local peer right before 0x394BD8 would settle it.
- **2026-11-10 02:55 (new session, on paper): cause of the guest's four msg 32 FOUND; fix written, NOT built / tested.**
  It is lead (2). The drop sender 0x394D48 has one caller, 0x38F77B, inside the per-frame local-user reconcile (function
  around 0x38F4xx, loop 0x38F67E..0x38F7E0 over the 4 local users). That function reads the LOCAL peer's player table from the
  membership base: `[rbp + (peer*0x56 + user)*4 + 0x174]`, rbp = session + 0x30, peer = `[rbp + 0x4434]`. Same table as
  `+ 0x1A4` from the session base, so the `+ 0x1a4` search never showed it. Its imuls **0x38F6E0, 0x38F723, 0x38F993** had no
  cave. For local peer 17 the read lands in inline memory (membership + 0x184C + 4*user = inside player 1's record, all 0),
  so every user "owns player 0": user 0 fails the id compare at 0x38F708 (player 0 is the host) and users 1..3 are not signed
  in but "have a player" -> 0x394D48 for each = four msg 32. A normal-slot guest reads its real entry (user 0 = own player,
  id matches; users 1..3 = -1) and falls through to 0x38F816 -> 0x390A04 -> 0x394BD8 = msg 33. Leads (1) and (3) are not
  needed to explain it: `member_consume` already showed pmask 0x10001 / local peer 17 on the guest, and 0x394D48 does not
  look at the stopgap parameter.
  **Written:** 4 caves appended to `SlotMemberCaves.inc` (the three above + 0x38F9A7 `imul rcx,rax,0xb0`, the player record
  read right after 0x38F993). `gen_member_caves.py` now lists them and the three 02:30 hand-added ones (SITES matches the
  .inc again; original bytes of the four new rows are assembled by hand from the listing, not read from the DLL - the load-time
  byte check will refuse the whole 32-player mode if one is wrong: look for that in the log first). Site count 106 -> 110.
  **Next test** (`overnight_quick_high.bat`): pass = guest sends 22 / 35 / 27 / **33** and no 32, host pmask stays 0x10001, guest
  stays in session state 4+ and loads the map. Still uncaved readers of this table, not on tonight's path (review before
  trusting): 0x395E6B (0x395DE8), 0x39752A, 0x39FD40 (`and [..], 0` = a write), 0x44B24C; and the 0x158-form ones not yet
  checked against the tables: 0x392631, 0x39FB62, 0x3ED5DC, 0x4091AD, 0x409492. The peer_param_set_17 stopgap is still in.
- **#32 membership update message implemented** (`src/host/SlotMessage.*`, `SlotMessageTable.inc` from
  `tools/limits/gen_message.py`; design + site list `docs/limits-players-message.md`). Builds. 0x396828 frame 0xC540 -> 0x10CC0.
- **10:51 LAN test PASSED with #32 on:** host sent message 26 at 0xA300 bytes, both clients decoded it, session ran normally.
  `NOBLE_SLOT_MSG_OFF=1` run (10:48) also joined. High-slot test mode added (`NOBLE_SLOT_HIGH=1`), see next item.
- **First 3-instance LAN test (10:39) FAILED the join (not reproduced since):** host received the join requests but never sent message 26 (membership
  update), so the clients never entered the session (no crash; all exited 0). Earlier runs (02:03 trace, 16-player mode) send 26
  right after the channel reaches state 4. Diagnosing: `NOBLE_SLOT_MSG_OFF=1` (leaves the #32 group out, frame still 0x10CC0) and
  producer logging (`SlotPatch: membership producer #n`). Tests: `overnight_lan3.bat`, `overnight_lan3_msgoff.bat` ->
  `tools\overnight-lan3.ps1` (logs `<tag>-host.log` / `-client1` / `-client2`, summary `build\overnight-lan3-<tag>.txt`).
- **High-slot test (11:01, `overnight_lan3_high.bat`, `NOBLE_SLOT_HIGH=1`): join FAILS, root cause narrowed.** The
  decoder fix (0x4273B0 owning-peer check <= 16 -> <= 31) is in and the client decodes every message 26. Host log: producer #1
  2 peers / 1 player, #2 2 peers / **2** players (guest player at slot 16 added), #3 **16 ms later** 2 peers / **1** player
  (player 16 removed again, no boot). The guest sees its player gone, leaves (msg 7), closes its channel to the host (reason 14,
  0x3F8A7F: channel no longer referenced by any session) and the rejoin is refused (reason 3). So the host removes a high slot
  player by itself. **Lead:** the player add/remove helpers use `add`-form slot addresses that have no shadow cave:
  0x39D8A8 player_add (`imul r8,rdi,0x158` 0x39D917 -> `[peer+0x174+slot*4]` write), 0x39D950 player_remove
  (0x39D97A), 0x39D7F8 remove wrapper (0x39D808 / 0x39D817), 0x39D620 add/update (peer-player table `imul r10,r14,0x56`
  0x39D63F + `[block+0x174]`), 0x39CA04..0x39CA75 (peer removal). With peer 17 these read/write inline memory past the 17
  peers (inside playersA), so a stale/garbage player index is found and removed. Sites file lists them as `add@0x39D8E8`,
  `add@0x39D91E`, `add@0x39D95E`, `add@0x39D988`, `not@0x39D64C`; none is in SlotPatchTable.inc. Next: cave these (and the
  other `add@` / `not@` / `imul 0x56` peer-player-table sites, from the sites JSON - not a new static scan), rebuild, rerun
  `overnight_lan3_high.bat`, then `overnight_lan3_hightrace.bat` for MEMBER-NEW sites.
- 17-bit peer masks (`0x1FFFF`, list in docs/limits-players-loops.md) still unpatched; several sit in the same membership code
  (0x39D1A3, 0x39F1B2, 0x39F203, 0x44C3AA) and will matter for peer 17+ next.
- **Evening 2026-11-09 (with Tyler): two fixes in, join still fails, root cause moved.**
  1. `src/host/SlotMemberCaves.inc` (`tools/limits/gen_member_caves.py`, 11 hand-reviewed caves in the player add/remove
     helpers 0x39D620 / 0x39D7F8 / 0x39D8A8 / 0x39D950 / 0x39C8DC; two dword-index `imul 0x56` sites use the new Site
     `scaleShift` = 2). Result: the host keeps the guest's player at slot 16 (producer #1: 2 peers, 2 players).
  2. `kPeerMasks32` in SlotPatch.cpp: the nine 17-bit `0x1FFFF` peer masks -> 0xFFFFFFFF and 0x408F10 `test si,si` ->
     `test esi,esi` (32-player mode only, byte-checked). No visible change in the join.
  - `overnight_lan3_hightrace.bat`: guest 0 MEMBER-NEW (no unreviewed slot-array access); host NEW sites all at slot 0.
  - **Real cause now:** in high-slot mode the host's running match ends ~10 ms after it receives the join request (game
    results written, post-game panel), before the membership update is even built. The guest then waits for a match
    that no longer exists and aborts after ~16 s (msg 10, reason 14). Normal-slot runs (Nov 8 13:15/13:22/15:22/15:25,
    Nov 9 01:58/10:39/10:48) all joined mid-match with the match still running - join-in-progress works; only a joiner
    in peer 17 / player 16 ends the match. Results stack: hr+0xF95E2 <- 0x7DA5D <- 0x57978 (game unload) <- 0x5753A <-
    0x34411 (main loop). Likely the in-game side (simulation world / per-peer views, the 0x18D0-stride observer arrays,
    in-game player tables) cannot take peer 17 / player 16 and the engine unloads the game. Next: hook the decision
    (log why the main loop unloads: game_finish / simulation abort reason) and fix those tables.
- **Later the same evening (21:10-21:42): the match-ending cause traced to its source; not fixed yet.**
  New dev traces in `LobbyPatches.cpp`: `game_finish` (0x59330), `game_over` (0x7BA58, logs the reason),
  `team_count` (0x133320, logs changes + dumps the game player datum array), and in high-slot mode a hardware write
  watch (Dr0) on game player 0's flags. Findings, in order:
  1. Game over reason 0 = "no teams with players" (0x7A61C). Team count drops from 1 to 0 at the join request.
  2. Game player array holds 32 (datum size 0x490). Only player 0 (the host) exists; at the join its flags go
     0x8 -> 0x2 ("left") at 0x527A5 / 0x527BC (0x52514 player-left), called from 0x418FF8 <- 0x3A4879 (decoder
     0x3A4648 of simulation event type 9, "game player event") <- 0x3C40D6 <- 0x3AB370.
  3. The type-9 events are built by 0x3AB05D -> **0x41F510**, which syncs session players into the game player table
     (16 x 0xA0 in the 0x4E0E910 singleton, 17 x 6 peer stack tables, bounds kept at 16 - the open "fixed tables" item
     in docs/limits-players-loops.md). With the joiner at peer 17 / player 16 this sync emits a "left" event for the
     host's player 0. **Fix needed: 32-player support in 0x41F510 (and 0x40A760): grow the singleton table to 32 x 0xA0,
     the stack tables to 32, raise its bounds and cave its session-player accesses.**
  - Kept in SlotPatch.cpp `kPeerMasks32` (all byte-checked, 32-player mode only): 0x1333A9 team count 15 -> 31, and the
    type-9 event player index 4 -> 5 bits (encoder 0x3C7CE0: 0x3C7D03/08/0C/16/23; decoder 0x3A46FE, bound 0x3A4735).
    Both are needed once a slot-16 player is really in a match; neither alone stops the match ending.
- **22:13 - host side FIXED: the host's match now keeps running when a slot-16 player joins.** Root cause (hardware
  watch on the session's player 0 id): shadow peers 17..31 started with their player-slot table (peer record +0x144, 4
  dwords) zeroed instead of -1, so peer 17 "owned" player 0; the add path 0x39D620 removed the host's player and wrote the
  joiner over slot 0 (0x39D8F0). Fix: `ResetShadowPeerPlayerTables` in SlotPatch.cpp (startup, mirrored clears, and
  copies from non-session blocks). The 41F510 "fixed table" theory above was a symptom, not the cause.
  **Next blocker (guest side):** the guest receives msg 26 + 34 but never answers with msg 35 and aborts after 16 s
  (reason 14). In normal-slot joins the guest sends 35 right after 26. Likely the guest cannot find its own peer 17 /
  player 16 in the decoded membership (local peer/player lookup, client-side shadow tables). Next: trace the guest's
  consumer path after msg 26 (hardware watch / logs on the guest like the host ones).
- **22:45-23:15 (Claude running the tests; windows now open on the non-primary monitor via NOBLE_WINDOW_SECONDARY=1,
  set by tools\overnight-lan3.ps1, handled in src/host/Window.cpp).** Guest side: new dev trace `member_consume`
  (0x39E47C) shows the guest applies the update correctly: peermask 0x20001, player mask 0x10001, **local peer 17**.
  So the guest knows itself. It then never starts the normal follow-up (a working guest sends 25 time-sync, 22
  peer-establish, 35, 27, 33 within 20 ms of 26+34) and aborts after 16 s. 0x411F38 ("local players known") is not
  even reached (trace `local_players` never fires); its 16-player loop was widened anyway (0x411F76/0x411FDE + cave
  0x411F8E). Next: find what drives the guest from "membership received" to "establish peers" (look for a check on the
  local peer index or a 17-entry per-peer table on the guest, e.g. the 17 x 0xC peer table / observer arrays).
- How to tell a LAN run really joined: host log has `SEND msg 26` and `RECV msg 27/35`; a run that only shows msgs 8/22/25 did
  not join (each instance just starts its own match).

## Status update 2026-11-09 01:00 ET (read this first, then the 2026-11-08 section below)

Written by the cloud Claude session for the next agent. Commits below are local, **not pushed**. Nothing in C12-L
step 3/4 has been run yet.

### How work is split now
- **Local Claude Code** (terminal in `D:\Project-Noble-Closed-Source`, effort high) owns code edits, tools and
  `build-dev.bat` builds. No Python on this PC: the `tools/limits/*.py` generators are the source of truth; a
  scratch Node port reproduces their `.inc` output byte for byte. Disassembly: `%TEMP%\stock_text.asm` (capstone
  listing of the stock DLL, not in git).
- **Cloud session** reviews diffs, reads logs, and drives tests on **monitor 1** (monitor 2 is Tyler's).
  It cannot run commands on the PC and only clicks in terminals.
- **Do not write a static "sort all imul sites" disassembly scanner.** Two Claude Code attempts and one cloud reply
  were stopped by Anthropic safeguards at that step. Find untraced sites at runtime instead (trace + guard below).

### C12-L (32 players) progress, 2026-11-09
| Commit | What |
|---|---|
| `ce932a7` | Step 1: last 11 pointer-stepping loops reviewed. Step caves for 0x44BDE3 and 0x44CCEF (read-only walks), bounds 0x44BDEA / 0x44CCF6 raised. 8 loops blocked by their own fixed tables (0x404B4A, 0x41FC64, 0x4314D3, 0x431B1E, 0x44CB51, 0x44CCA1, 0x44CD5F, 0x4509E8); 0x4215AE is not a session array. |
| `7d9e408` | Step 2 finding: most bounds were already raised by `d107dc0` (101/111 bit-scans, 26/31 C1, 6/8 C4). **169 `imul r,r,0xB0/0x158` sites in 0x380000-0x460000 have no cave** (the trace never ran them), e.g. 0x39D950's 0x39D957 / 0x39D97A. |
| `cba2c21` | Step 3: 0x390DB0 frame +0x100, its 16-qword name table moved to rbp+0x1F8 (32 entries), own unwind copy (block shared with 0x35B1E4), 0x38DA74 scan to 32. Its loop bounds stay 16 until its 6 uncaved imuls (0x390E85/F13/F44/FF5, 0x391342/3B2) are handled. |
| `e2aba3f` | 17 x 0xC table at block+0x4440 (#31) cannot grow in place; ~47 indexed sites need caves (list via grep in `docs/limits-players-loops.md`). Whole-block copy problem written up. |
| `272b096` | Copy/clear scope count: **8 sites + 1 compare** (0x39C4D5, 0x39C4F3, 0x39C71A, 0x39C7C4, 0x396A6F, 0x396A7F, 0x396CBE, 0x16FEA9; memcmp 0x396B3F). Copies with register sizes would not show up. |
| `f731b9f` | A+B: the 6 static-session copy/clear calls go through a Noble wrapper that mirrors the copy/clear onto the shadow slots 16-31. Non-static source (#1, network-decoded block from 0x391F0C's stack) clears the destination's slots 16-31 and logs (capped at 20). |
| `c941f23` | 0x396828 analysis: only one imul (0x396AB4, peer 0x158) addresses the stack copies S1/S2 (6 operands, fields +0xAC/+0xB0). No callee gets S1/S2. |
| `88d67ec` | C: 0x396828 frame 0x8F00 -> 0xC540 (probe size, lea rbp, epilogue, own unwind copy; GS cookie stays frame+0x8EF0). Side areas sideS1 rbp+0x8E00 / sideS2 rbp+0xB010 (0x2210 apart, 15 x 0x158 each). #5/#6 fill all 15 side records (shadow peers or zeros); memcmp extended. Cave at 0x396AB4: idx >= 17 -> rcx = 0x8EA8 + (idx-17)*0x158. **Safety gate:** `-slot-players 32` stays in 16-player mode unless `NOBLE_SLOT_PLAYERS_UNSAFE=1`, and logs the blockers. |

Existing hazard the gate covers: `d107dc0` already raised 0x396AA1 / 0x396B0A to 0x20 (peers 17-31 would have
written past the stack copies before `88d67ec`).

### Still blocking a real 32-player game
1. Untraced imul sites (169, incl. 6 in 0x390DB0): runtime route, see Next.
2. ~~17 x 0xC table at block+0x4440~~: caved 2026-11-09 (47 operands + 2 reset loops, guard counts non-session
   blocks; see `docs/limits-players-loops.md` "C12-L (c)"). Built: no, tested: no. 0x39C701 still 17.
3. Remaining step-3 tables (0x2C92050 16 x 0xA0, 17 x 6 at +0x34, 0x40A760 vtable object, the 8 blocked loops'
   tables, 17 x 0xC at session +0x61574 and 0x3FFC9C, stacks in 0x41E884 / 0x41F510 / 0x41FBE8, nine 0x1FFFF masks).
4. ~~Step 4~~: pools to 32 done in `a0f039f`; the 0xD6B8 host list stays 16 (Noble's slot 30 is a no-op,
   nothing reads it). Still open: whatever indexes the 32-record simulation views with a 16 bound.
5. Network encoding of slot counts/masks (protocol work; also #1's network-decoded source block).

### Next (in order)
1. ~~Smoke test~~ done 01:45: 32-player mode installs and runs 120 s clean with 1 player after fixing the GS
   handler byte in both frame tables (`docs/limits-players-loops.md` "Smoke test"). Original plan: `run_slot32_smoke.bat` (`f7f61b4`; ENGINE LIMITS = Extended) — one window, solo match, three runs, separate `-log` files: no slot options; `-slot-patch
   -slot-players 32` without the override (must log blockers and stay 16); same with `NOBLE_SLOT_PLAYERS_UNSAFE=1`
   (1 player, so slots >= 16 unused: checks the two grown frames and new caves install and don't crash). Ask
   Claude Code for a `.bat`; the cloud session can run it on monitor 1.
2. Step 4 (pools + host list), then the block+0x4440 caves.
3. (Replaced 2026-11-09 by the runtime trace: `-trace-members` now logs MEMBER-NEW for any unreviewed slot access, run
   `tools\lan-three-instance.ps1`; see `docs/limits-players-loops.md` "C12-L (d)". No disassembly scan.) First run 02:03: 26 real
   accesses -> 18 biased peer-table sites caved, player-count loop 0x2A38CF caved, 0x39DD4C reviewed (blocked by #32). Original: Dev-only guard in unsafe 32-player mode: log the RVA whenever slot >= 16 reaches an uncaved imul 0xB0/0x158 site
   in 0x380000-0x460000; extend `-trace-members` to log every site hit; trace sessions through player leave, lobby
   reset, team change, mid-game join (three-window LAN on one PC). Cave what they find.
4. Then network encoding, then the three-window 32-slot test.

### Parked: swarm zombie test
Renette's flood swarm tags are in HREK and build into a test `ff_unearthed.map`; spawning them in Firefight needs
a new squad (Sapien). Everything is in `docs/swarm-test-notes.md`.

### Rules (unchanged)
Never modify the installed haloreach.dll; never run `build.bat` (use `build-dev.bat`); never push without Tyler's
OK; never edit `D:\Project-Noble-ORIGINAL`; never commit `recipes/` or `.nlp`; commit trailers per
`docs/CLAUDE-CODE-START-C12.md`; no test without asking Tyler.

## Status at end of 2026-11-08 (previous status)

Updated 15:30 ET by Claude. Everything below this section is the original design; this section is what exists
now. Last commit: `ce37aa7` (local, not pushed). Uncommitted since then: C12 docs/tools (see below).

### Profiles and engine builds (all built by `tools/limits/make_v16_particles.py`, applied as `.nlp` to the
player's own stock DLL; the installed `haloreach.dll` is never modified)
| Profile | Setting label | `.nlp` | What it adds |
|---|---|---|---|
| 0 Stock | STOCK | – | nothing |
| 1 V16 | EXTENDED | `v16.nlp` | GreenKnight V16 (objects 32,767) + Noble extras below |
| 2 V16p | EXTENDED + PARTICLES | `v16p.nlp` | + GreenKnight particles (pools 8×, 65,535 GPU particles/frame) |
| 3 Full | FULL (V16 + PARTICLES + LIGHT VOLUMES) | `full.nlp` | + GreenKnight light volumes V4 (5 pools 160 → 65,535), ported by DeepSeek (`lv_port.py`) |
**Proof nothing else changed:** `--ai-bank-extra 0 --render-extra 0` reproduces GreenKnight's exact V16
`b8810c3b…` and V16p `47bc0723…`. Default builds: v16 `007a4dac…`, v16p `6403f044…`, full `d5b0122e…`.

### Noble extras in every Extended build
- **Game-state bank 1 (AI) +6 MB** (`--ai-bank-extra`, operand RVA 0x16155): stock 0x220000 had ~5 KB spare.
- **Render bank 3 +8 MB** (`--render-extra`, operand RVA 0x1619A): room for 16,384 cached object render states.
- **Main bank 4** sized from measured use in Full (0x3250000). Use the BANK diagnostic before changing banks.
- **Three per-actor stack buffers grown:** AI update scheduler 0x70ABBC (96 → 1,024 entries, stack-commit hook),
  spawn-candidate list 0x76EBF8 (512 → 1,024 + its 0x200 caps), search scratch 0x76E814 (+0x800, caps 0x400,
  stack-commit hook). Ops: `AI_FRAME_OPS`, `AI_FRAME2_OPS`/`AI_FRAME2_CAP_OPS`, `AI_FRAME3_OPS`/`_CAP_OPS`.
- **Effect list guard 0x1D4F85:** 4 static lists × 512 at RVA 0x477E700 had no bounds check (crashed FULL in an F6
  horde); a `.deltrac` cave now drops entries past 511. Emitted only when extras are on.

### Runtime pieces (Noble code)
- `PoolResize.cpp` (hooks data_array_new 0x12FCC): `-ai-actors N` / SETTINGS > ENGINE > AI ACTORS (96/256/512/1024):
  actor + prop/prop_ref/prop_search scaled, command scripts, object list header, dynamic firing points, swarm,
  script threads (det hs thread 1,024, non-det 32, list object reference 512); `-render-states N` (render bank
  budget). Budgets read from the loaded engine; never grows past the added room.
- `PoolReport.cpp`: `POOL|…` and **`BANK|n|used|capacity`** (bank table RVA 0x27BF270, stride 0x18F0, +0xA0 cap,
  +0xA8 used); warns on overflow. `-pool-trace`, `-ai-pools a,b`.
- Stack-overrun reporter (`gs_failure_report`, Extended): logs `STACK OVERRUN in haloreach+0x…` with frames before a
  fail-fast (0xC0000409) kills the process. Windows Event Log is the fallback.
- `SchedulerGuard.cpp`: waits for free jobs; ceiling is 16,384 jobs (a dense 5,000-object grid asks ~74k → crash
  0x68B21F). Known limit, inline arrays at scheduler body +0x230010/+0x260038/+0x290038; not fixed.
- `SpawnTest.cpp` (dev): F9 objects, F8 128-body crowd, F7 all squads, **F6 horde**, `-horde N`, `-horde-team`,
  `-horde-no-teleport`, `-horde-delay`; waits for the real cinematic flag (game-state root +0x130).
- Join check (D3): hosts send `limits_id` (= `limits::LocalNetworkId()`), joiners refuse a mismatch. **Inert until the
  private backend stores/returns `limits_id`.**
- Unwind for Full's relocated LV routines registered at load (`RtlAddFunctionTable`).

### Results (tested)
| Limit | Stock | Now | Evidence |
|---|---|---|---|
| Objects | 2,048 | 32,767 | V16; ~8,800 moving objects before the scheduler ceiling |
| AI actors | 96 | 1,024 | 973-982 live at wave end, 5 min no crash (`-horde 1000 -horde-team 0 -horde-no-teleport`); ~18-36 fps |
| Render states | 4,096 | 16,384 | 3,800 objects 5+ min; visible-past-4,096 check still open (V16 draw list, docs/limits-render.md) |
| Particles | 2,048 | 16,384 pool / 65,535 per frame | V16p |
| Light volumes | 160 | 65,535 | Full, 5-min plasma Firefight + F6 horde, no crash |
| Script threads | 320 | 1,024 | pool report |
| Players | 16 | in progress | see C12 below |

### C12: 128 real players (option 1, chosen by Tyler)
Keep the engine's own netcode and authority; move player/peer slots ≥ 16 into Noble-owned overflow storage; steps
32 → 64 → 128; 64+ expected to need a real dedicated host (the current `noble-dedicated` is a stub: no engine session).
- **Session objects:** 4 static membership objects, RVA 0x2B07418 + k × 0x61668 (ctor 0x3CF864). Inline arrays:
  peers 17 × 0x158 at +0x60 and +0x2270, players 16 × 0xB0 at +0x1740 and +0x3950; scalar fields (counts, 32-bit
  mask +0x173C, ids at +0x4464…) do not move.
- **Trace:** `src/host/MemberTrace.*` (`-trace-members`, dev only; stalls the game after a few minutes, known).
  Runs 1-3 (menu, Slayer start, two-instance LAN join) → **417 slot-record accesses from 136 address sites**
  (`docs/limits-players-sites.json`, `tools/limits/slot_rewrite.py`).
- **Patch design:** overflow blocks at the same 0x61668 stride, so each redirect is `dst += C` with one constant per
  array; slots 0..15 keep their exact addresses. 65 imul sites generated automatically, ~70 hand-reviewed. Caves
  live in Noble-allocated executable memory near the engine, patched at load with byte checks (no room left in
  the DLL). Details: `docs/limits-players-audit.md`, `docs/limits-players.md`.
- **Test rig (one PC):** `tools\lan-two-instance.ps1` starts a host (build\bin\Release) and a client
  (build\bin\Release2, second data folder) joined over 127.0.0.1:11774.
- **Step 2 (15:30 ET, Claude, written but not built yet):** `-slot-players 32` adds 6 pointer-stepping loop caves
  (`gen_steps.py` → `SlotStepTable.inc`) and 137 bound/mask edits (`gen_bounds.py` → `SlotBoundsTable.inc`), byte-checked,
  all or nothing; shadow slots are now built with the engine's record ctors. **Peers stop at 32** (DWORD peer mask).
  Still open: the fixed 16/17-entry tables (41 bounds kept at 16/17 because of them), nine more `0x1FFFF` peer masks,
  whole-block copies (A ↔ B, messages) that skip the shadow slots, the network encoding of slot counts/masks, and the
  players pool itself. Full list and test steps: `docs/limits-players-loops.md`.
- **Next:** build + the two regression runs in `docs/limits-players-loops.md` (16-player mode, then `-slot-players 32`
  with 2 players); then block copies + network encoding (the real protocol work), then the tables.

### Other open items
- C11 part 2: V16's 4,096 visible draw list (needs eyes on 4,096+ visible objects).
- Scheduler ceiling (16,384 jobs), effect lists capacity (4 × 512 → `.hkwork` if effects go missing).
- Headless dedicated host (real engine session) for 64-128 players.
- Backend: store/return `limits_id` so the join check works.

## Goal

Raise Halo: Reach's engine limits inside Noble so that 64-player lobbies render every player, Forge
and custom maps can hold far more objects, and a Flood horde / swarm mode becomes possible. More
particles and light volumes come later.

Hard rules from Tyler:

- **Never modify the player's installed `haloreach.dll`.** Noble builds a patched copy in its own
  data folder from the player's stock DLL and loads that copy. MCC keeps working untouched.
- Players must not have to hex edit anything. It is a Noble setting.
- If anything doesn't match (another DLL build, a bad patch file), Noble falls back to the stock DLL and
  logs why. It never refuses to start because of this feature.

## Where the research came from

The local recipe files are in `tools/limits/recipes/` (gitignored, see "Files" below).

| Source | What it is |
|---|---|
| GreenKnightOP, **V16** (`recipes/v16/`) | Object pool 2,048 → 32,767 plus the rendering, physics, scheduler and state-bank changes that go with it. 642 overwrite rows plus a 969,768-byte append. `REACH_MODIFIED_DLL.dll` that Tyler has is exactly V16 (SHA-256 `b8810c3b…0a5f24`, verified). |
| GreenKnightOP, **particles** (`recipes/particles/`) | GPU particles 7,168 → 65,535, CPU particles 2,048 → 16,384, emitters 832 → 6,656. 66 rows plus a 512-byte append. |
| GreenKnightOP, **light volumes V4** (`recipes/light_volumes/`) | Light volumes 160 → 65,535, transparent draw queue 1,024 → 16,384, effect-message queue 512 → 65,536. 95 rows plus a 351,784-byte append (hash verified). |
| Renette, Sapien work (`recipes/sapien_reference/`) | Sapien-exe patch (render-packet arena, cached render states) and her `migrationpooltrick.hsc` swarm script. **Not portable to the DLL**: different binary. Useful as a list of which limits matter for swarms. |
| Chat log (`recipes/sapien_reference/chat_log.txt`) | Discord context. Key points below. |

All three DLL recipes are for **`haloreach.dll` 1.3528.0.0**, stock SHA-256
`738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894`, length 13,229,016 bytes.

### Key points from the chat

- Renette: with stock limits **only about 34 Spartans render**, so Noble's 64-player lobbies
  (`LobbyPatches`) already hit this. The object pool (2,048) can also run out with 64 Spartans plus map
  objects. V16's visible render list (1,024 → 4,096) plus the object pool fix both. GreenKnight showed
  3,025 bipeds + 3,027 weapons (6,064 objects) rendering with no flicker.
- MtnDewIt: raising limits breaks **campaign saves** (checkpoint size changes). **Films are a non-issue**:
  the team decided to drop film compatibility.
- Renette's swarm limits: actors 96 (she uses 93), command scripts 40, active object lists 48,
  references per map 128, script threads 320, syntax nodes ~6,457, dynamic firing points 16. In Sapien
  she crashes on a static array past 987 Flood or 384 Spartan models.
- Her script works around the engine updating only about 8 actors per squad each tick by migrating actors across 12
  pool squads (`ai_migrate` with a "siphon" unit).

## Verified findings (Claude, 2026-11-08)

1. **The three recipes cannot be applied together as written.** All three:
   - add their section in the same header slot (file `0x388`) at the same RVA `0x04EDA000`;
   - rewrite the same game-state size operands (render bank, main bank, checkpoint, total) to
     different values;
   - redirect the same 20 checkpoint-snapshot references to different addresses;
   - replace the PE exception directory (LV and V16 each with their own full copy of `.pdata`).
   Outside that shared infrastructure, no edits overlap.
2. **Hidden conflict, particles × light volumes.** LV V4 relocates the "effect and particle
   initialization" routine (stock file `0x114C88`, 1,302 bytes). Three particle edits sit inside it
   (`0x114F85`, `0x114FD4`, `0x115003`) and would be dead code. In V4's relocated copy the same operands
   are at file **`0xC9E10B`** (systems/locations 576), **`0xC9E15A`** (CPU particles 2,048) and
   **`0xC9E189`** (CPU emitters 832).
3. **Game-state sizes follow fixed formulas** (checked against all three recipes):
   - `checkpoint = render_bank + main_bank + 0x260000`
   - `total = checkpoint + 2 * main_bank`
   - The recipes grow the banks for different pools, so the increases add up:
     render_bank +0x820000 (P), +0x2A70000 (LV), +0x1F0000 (V16); main_bank +0xA60000 (LV),
     +0x2340000 (V16). Stock: render 0x370000, main 0x490000.
   - All three combined: render `0x37F0000`, main `0x3230000`, checkpoint `0x6C80000` (108 MiB),
     total `0xD0E0000` (209 MiB). V16 alone: checkpoint 48 MiB, total 127 MiB.
4. **No collisions with Noble's existing hooks.** Every haloreach RVA referenced in `src/` was compared
   against every patched byte (±16) in all three recipes. Nothing is close.
5. **AI and script pools located** (stock file offsets, untouched by V16). All are created by the
   data-array allocator at RVA `0x12FCC` (`rcx` = name, `edx` = count, `r8d` = record size):

   | Pool | Count | Record size | Count instruction | File offset |
   |---|---:|---:|---|---|
   | actor | 96 | 0xD00 | `8D 56 5C` lea edx,[rsi+0x5C], rsi = 4 | `0x68CCAC` |
   | command scripts | 40 | 0x1EC | `8D 55 27` lea edx,[rbp+0x27], rbp = 1 | `0x68D27E` |
   | object list header | 48 | 0xC | `8D 57 24` lea edx,[rdi+0x24], rdi = 0xC | `0x80C7A` |
   | swarm | 32 | 0x34 | `41 8B D6` mov edx,r14d, r14 = 0x20 | `0x68CD87` area |
   | prop | 256 | 0xB0 | `BD 00 01 00 00` mov ebp,0x100 | `0x68CE85` |
   | squad | 256 | 0xEC | `8B D5` mov edx,ebp | `0x68CFE8` area |
   | clump | 20 | 0x10C | `8D 7E 12` lea edi,[rsi+0x12] | `0x68D0BB` |

   Allocator function for the AI pools: RVA `0x68D75C`–`0x68E028`. Raising a count needs more room than
   the 3-byte `lea` (disp8 tops out at +127), so it needs a small trampoline. Other AI code very likely has static
   arrays sized to 96 actors: find those **before** raising the count (this is probably Renette's
   "static array crash").
6. **Not found yet:** script thread limit (320), per-map reference limit (128), syntax nodes,
   dynamic firing points. Nothing in any recipe touches lightmaps or shadows.
7. Renette's GitHub notes (`github.com/haloreachfan200-collab/test`) could not be fetched from the
   claude.ai session (robots / access). Worth reading by hand.

## Architecture

```
SteamLocator → stock haloreach.dll path
        │
        ▼
limits::PrepareEngineDll(stockPath, requestedProfile)          (src/host/EngineLimits.*, Claude)
   1. requested == Stock → return stockPath
   2. SHA-256 stockPath == 738dd2d2… ? else → stockPath + reason "unsupported build"
   3. cached copy exists at %LOCALAPPDATA%\Noble\engine\<profile>-<outhash8>\haloreach.dll
      and its hash matches the expected output → return it
   4. else apply the recipe (overwrite rows with stock-byte checks + append), verify output hash,
      write atomically (temp file + rename) → return it
   5. any failure → stockPath + reason
        │
        ▼
Loader::LoadReachDll(dllToLoad, stockPath)   ← second argument = folder for dependency lookup
                                               (xaudio/MCC binaries path derivation must keep using
                                                the STOCK path's folder, not the copy's)
```

- The copy lives outside the MCC install, so MCC and Steam file verification never see it.
- Recipes ship inside Noble as a **compact diff** (overwrite rows + append block), never as a full DLL.
  The append blocks contain relocated copies of Reach's own code, so they only work on the player's own
  stock DLL. They must not be committed as part of a full DLL or any other game file.
  (Format: `LIMITS-TODO.md` C2.)
- Profiles (network id in brackets): `Stock` [0], `V16` [1], `V16Particles` [2] (done, `v16p.nlp`).
  Later: `Full` [3] (V16 + particles + light volumes), with its own rebased recipe.
- **Every player and server in a session must run the same profile.** Game-state layout differs, so
  a mismatched join would desync or crash. The host advertises its profile and clients that don't
  match are refused with a clear message (D3).
- Campaign saves from different profiles are incompatible: they go in separate folders (D4).

### Interface contract (so both agents can work in parallel)

Claude owns the implementation; DeepSeek codes against this header from day one.

```cpp
// src/host/EngineLimits.hpp
#pragma once
#include <cstdint>
#include <filesystem>
#include <string>

namespace noble::limits {

enum class Profile : uint32_t { Stock = 0, V16 = 1, V16Particles = 2 };

struct PrepareResult {
    std::filesystem::path dllPath;   // what Loader should load (stock path on any fallback)
    Profile active = Profile::Stock; // what is actually running
    std::string reason;              // empty on success; human-readable on fallback
};

// Never throws. Safe to call before the engine exists. May take ~1 s the first time (writes the copy).
PrepareResult PrepareEngineDll(const std::filesystem::path& stockDll, Profile requested);

Profile ActiveProfile();                  // valid after PrepareEngineDll
const char* ProfileName(Profile p);       // "Stock", "Extended (V16)"
uint32_t ProfileNetworkId(Profile p);     // value sent in lobby / server registration
std::string SaveFolderSuffix(Profile p);  // "" for Stock, "_ext_v16" for V16

} // namespace noble::limits
```

## Files

| Path | In git? | Notes |
|---|---|---|
| `LIMITS-HANDOFF.md`, `LIMITS-TODO.md` | yes | This plan. |
| `tools/limits/recipes/**` | **no** (gitignored) | Raw recipes from GreenKnight / Renette. Local reference only. |
| `tools/limits/build_recipe.py` | yes | (C2) Turns a recipe into Noble's compact diff. |
| `src/host/EngineLimits.*` | yes | (C1) Builder + profile API. |
| Generated diff `data/limits/*.nlp` | decide with Tyler | Contains Reach code bytes; ship alongside the client, not in the public repo. |

## Safety notes

- Writing the patched copy: temp file then `MoveFileExW(..., MOVEFILE_REPLACE_EXISTING)`. Never
  write next to the stock DLL.
- Check the **stock bytes of every overwrite row** before writing (the recipes give them), not just
  the file hash. Abort the whole patch on the first mismatch.
- Log: requested profile, active profile, reason, input hash, output hash, copy path.
- Memory: V16 reserves about 450 MB of virtual address space (`SizeOfImage` 0x1B0A2000). Fine on x64.
  Each campaign checkpoint copy is 48 MiB instead of 10 MiB, so expect slower saves.
- `noble-dedicated.exe` must load the same profile as its clients (D2).
