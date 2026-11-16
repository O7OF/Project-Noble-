# PROMPT - new DeepSeek agent: Project Noble multiplayer take-over (2026-11-16)

You are taking over the **multiplayer** workstream for Project Noble, a Halo Reach MCC community client. Tyler is
re-ordering the project: **multiplayer is now first**, campaign co-op beyond four players is parked at the end of the
list. You own multiplayer. Work autonomously, report concrete results, and do not stop for permission on internal steps.

## Repository and baseline

- Repo: `D:\Project-Noble-Closed-Source`, branch `wip-merge` (current HEAD at hand-off: `4bfe827`, version
  **0.19.103**, no release).
- Both gates must build clean: `noble-dev` and `noble-client`.
- Build command (full path matters):
  `"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build "D:\Project-Noble-Closed-Source\build" --config Release --target noble-dev noble-client`
- Engine truth: Ghidra MCP is attached to `haloreach-copy.dll` (sha256 `738dd2d2...0894`, image base `0x180000000`;
  address = base + repo RVA). PC decompile: `D:\MCC HALO REACH DECOMP\decompiled` (function index csv included).
  Retail-named decompile for symbols: `D:\ReachX360\ghidra_retail\decompiled_v3`.

## Mission

Deliver **working large-player multiplayer**, in this order:

1. **16-player multiplayer end-to-end** - real peers, match starts, gameplay, join/rejoin, clean exit. This is the
   acceptance bar for milestone 1.
2. **Fix the high-slot join failure** (peer 17 / player 16) that currently ends the match right after the join ack.
   This is the known blocker for anything above 16.
3. **32-player session** end-to-end using the already-installed 32-slot patch set; then plan 64/128 with Tyler.
4. Lobby/server-browser honesty for large counts (MAX PLAYERS picker, display of real counts), then QA + docs.

Campaign co-op >4 is **not** your task; it is parked at the end of the priority list (see the master TODO). Do not spend
time on campaign unless Tyler explicitly asks.

## Current state (facts from the campaign/engine session that just ended)

- **32-slot patch set installs fully** on the pinned build: 110 slot sites, 23 masks, 139 bounds, membership message #32
  (`0xA300`); pools live at 32 (players / fire team / simulation view).
- **Normal-slot two-instance LAN join works**: 2 peers / 2 players, pmask 3, guest decodes the `0xA300` membership
  message, acks msg 35, msg 44 stream, 0 crashes.
- **High-slot test fails**: with `NOBLE_SLOT_HIGH=1` (`SlotMessage::ApplyHighSlotTest`, free-slot searches start at
  peer 17 / player 16) the host **adds player 16 and removes it ~16 ms later** (membership producer goes 2 players -> 1),
  closes the session channel with **reason 10**, and the guest's rejoin gets **CONNECT REFUSED reason 3**. Earlier
  disassembly found player add/remove sites that reach the peer's player-slot table through address math with **no
  shadow-slot cave** yet, so peer 17 reads the wrong table (evidence: `docs\OVERNIGHT-2026-11-09.md`,
  `docs\limits-players-regbounds.md`, `docs\limits-players-message.md`, `docs\limits-players-review.md`).
- Engine limits profile is pinned to **Full + 1024 AI actors** (do not change it; it is a Tyler decision).
- Version stays **0.19.103**; releases/version bumps/tags/public pushes require Tyler's explicit "release it".

## Read these first (in this order)

1. `knowledge\desktop-guides\TODO-MASTER-2026-11-14.md` - top section has the new priority order.
2. `project-docs\TOP10-NEXT-UPGRADES-2026-11-15.md` - re-ranked list; multiplayer is #1.
3. `docs\limits-players.md`, `docs\limits-players-review.md`, `docs\limits-players-message.md`,
   `docs\limits-players-regbounds.md`, `docs\limits-players-audit.md`, `docs\limits-players-bounds*.md`.
4. `knowledge\drafts\draft-session-networking.md` - player tables, masks, transport, handshake, join-in-progress.
5. `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md` - join-path/backend gaps already identified.
6. `project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md` - latest large-session findings (incl. campaign context; read the
   multiplayer sections).
7. `project-docs\REFERENCE-LIBRARY-2026-11-15.md` - external tools, paths, Ghidra setup.
8. Code: `src\host\SlotPatch.*`, `src\host\SlotMessage.*`, `src\host\PoolResize.*`,
   `src\hooks\LobbyPatches.cpp` (`s_maxMultiplayerPlayers=128`, session slot counts `hr+0x39C5C8`), `tools\limits\*`.

## First 60-90 minutes (do this before editing anything)

1. Build both gates from current HEAD; confirm clean.
2. Baseline: run the 16-player-capable stock MP path with the local harness (host + as many instances as the machine can
   hold; low-spec settings, secondary monitor) and capture a clean 16-player session start if the hardware allows.
   Harness: `build\bin\Release\mp-jip-test.ps1`, `probe-lan32.ps1`, `probe-jip.ps1`, `qa-regression.ps1`;
   client dirs `build\bin\Release2..Release5` are copies of the host build.
3. Reproduce the high-slot failure with `NOBLE_SLOT_HIGH=1` and capture full host+guest logs. Add **read-only probes**
   (no behaviour changes) for: player add/remove path, player-slot table address computation, producer player counts,
   close reason and CONNECT REFUSED reason.
4. Write a short status block at the top of the TODO master (what works, what fails, exact logs) before attempting fixes.

## The high-slot fix (milestone 2 in practice)

- Find every site that indexes the 17-peer / 16-player structures by computed address and add the missing shadow-slot
  cave, or replace the address math so peer 17 indexes its own extended table entry. The two known free-slot searches
  are `0x3953BC` (peer) and `0x39D69A` (player); the earlier root-cause note in `docs\OVERNIGHT-2026-11-09.md`
  lists the uncaved add/remove sites. Verify with the existing high-slot probe after each site.
- Success criteria for the high-slot test: host producer player count stays >= 2 after the ack, guest record reaches
  state 6+, no CLOSE reason 10, no CONNECT REFUSED reason 3, match keeps simulating, guest can leave and rejoin.

## Working environment / test rig

- One Windows machine runs up to 5 instances (`Release` host + `Release2..Release5` guests) at 1280x720/30 fps/muted
  (low-spec template `low-spec-settings.json`); game windows are forced to the secondary monitor with
  `NOBLE_WINDOW_SECONDARY=1`.
- The probe harness scripts contain a **focus guard** that hands the foreground back to Tyler whenever a game window
  grabs it - keep that behaviour (his mouse must stay on his screen).
- Real multi-machine tests are the long-term acceptance path; one-machine 5-instance runs are the fast inner loop.
- Do not modify installed game files; all engine changes are runtime hooks/byte patches in Noble code.

## Reference: Reach Renaissance (RR) - local reference environment

RR is a separate large-player Reach project (multiplayer, Firefight and campaign with the Xbox 360 menus, playable over
the internet); their environment reportedly runs **88 players**. Tyler keeps a full installed RR build on this machine:

- **Installed reference: `C:\Users\tyler\Desktop\ReachRenaissance` - v0.1.50, commit `c9c8e3e7b`, built
  2026-11-16T00:01Z** (`VERSION.txt`). `bin\` has `host-probe.exe`, `menu-overlay.exe`, `rr-browser.exe`,
  `rr-master.exe`, `rr-voice.exe`, `WebView2Loader.dll`; `README.txt` documents the host/join/server-browser flow.
- Our own forensic archive: `build\rr-forensics\` (extracted `Reach Renaissance.exe` + v0.1.23 / v0.1.45 zips, sha256
  records). The Desktop install is newer and is the reference to read.

How to use it (read-only):

- Treat it as an **existence proof and UX/config reference**: study the lobby/session setup for large session sizes, the
  host/join/reconnect flow and the server-browser layout. Their 88-player environment shows the Reach engine can carry
  far more than 16 players once session/membership/replication are patched - the same layer our `SlotPatch` work targets.
- **Reverse-engineering their binaries for understanding is allowed** (Ghidra/decompile, strings, config files): learn
  the approach - how large sessions are shaped, how host/join/reconnect is handled, what the server browser exposes. Then
  implement it independently in Noble's own codebase. Do **not** paste decompiled code, copy data tables/constants
  wholesale, or lift assets/strings/names; write the implementation from our own engine RE and keep a short provenance
  note in the docs (what was learned from RR vs. what was written from our engine). This keeps the work a study of
  behaviour/ideas rather than a copy, and the shipped build must stay free of RR names/assets (it currently is).
- If you run RR to observe it, use a separate window/instance and not during our own test runs (both clients touch the
  same Reach data and ports); never point Noble at RR's files, and do not modify Tyler's RR install.
- If Tyler asks for a newer RR build, install/extract it separately, record version + sha256 in
  `project-docs\NOTE-PLAYER-COUNTS-2026-11-15.md`. RR is **not a blocker**: budget at most ~1-2 hours of reference reading
  before returning to the 16-player baseline and the high-slot repro.

## Rules

- No release, no version bump, no tag, no public push. Commits to `wip-merge`/private are fine and expected: commit
  finished work with a subject + body paragraphs (no trailers).
- Both gates clean before calling anything done; run the relevant QA/harness tests and keep evidence captures.
- Mirror doc changes into `knowledge\` (Compendium is a stale mirror of it) and keep the TODO master updated.
- Security / anti-cheat / DRM is out of scope.
- Ask Tyler before changing shipped user-facing caps or release behaviour; otherwise proceed autonomously.

## Definition of done - milestone 1

- 16-player multiplayer end-to-end on one machine, no crash, players can join/rejoin, match starts and ends cleanly.
- Evidence: host + guest logs showing 16 players in session, producer/consumer membership at 16, gameplay capture.
- Commits + docs updates + clean gates.

## Definition of done - milestone 2

- The high-slot (peer 17 / player 16) join completes and the match keeps running; the 32-player patch path is exercised
  end-to-end with evidence, and the next larger count (64/128) has a documented plan and Tyler's go-ahead status.
