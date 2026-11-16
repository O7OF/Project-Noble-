# Network / security handoff to Claude (2026-11-15)

Written by DeepSeek (lead) as the boundary note required by `PROMPT-DEEPSEEK-PHASE2-FEATURES-2026-11-15.md`:
**networking (session, replication, host migration, netcode for higher player counts) and security are OUT OF SCOPE
for DeepSeek and are Tyler's assignment to Claude.** DeepSeek stops at this boundary, writes what it finds here, and
continues on the non-network parts of each stage.

Status: placeholder + Stage 1 findings. Extend this file whenever a stage hits a networking/security dependency; do not
implement networking/security changes in Noble from the DeepSeek side.

## Current known items for Claude

### 1. Player-count expansion (Stage 2 of the Phase-2 prompt; 128-player hold still stands)
- Tyler reports **multiplayer stopped at 34 players**, and **campaign co-op 5 players crashed or black-screened** (the
  4/8/12/16 MAX PLAYERS row is only UI - it is NOT proof that >4 works). Treat campaign >4 as broken until a real
  5-player session loads and plays.
- Background: 32-slot patches exist (`src\host\SlotPatch.*`, `SlotMessage.*`, `src\hooks\LobbyPatches.cpp` -
  `s_maxMultiplayerPlayers=128`, `src\host\PoolResize.cpp`) but are "built, not tested" per `docs\limits-players*.md`
  and `knowledge\drafts\draft-session-networking.md`. The session membership audit (fixed-16 indexing, 16-slot masks,
  membership update message type `0x1A`) is in `docs\limits-players-audit.md` / `-loops.md` / `-message.md`.
- Networking-owned pieces (NOT DeepSeek's work): session membership arrays and masks, join/invite handshake and
  join-refuse path, `c_network_session_membership::evaluate_coop_insertion_point` (co-op 5th player), per-player
  simulation entities/replication budgets, host election/migration, split-screen/window count where it is replicated
  session state, and the validation strings/refuse reasons (`invalid-player-count`, `session-size`,
  `big-party-count-exceeds-teams`, `k_*_refuse_reason_strings`).
- DeepSeek will do (when Stage 2 starts): reproduce the failures, name the check/pool/array/UI limit
  (name -> hr+RVA), fix non-network causes (UI rows, local pools, scoreboard/CHUD widgets, spawn points, render/
  split-screen limits), and clamp UI rows to counts proven to work. Network blockers go here.
- 128-player work stays ON HOLD until Tyler lifts it.

### 2. Extended engine limits vs the network (proven blocker, found in Stage 1)
- Current build **force-disables** the non-stock engine-limit profiles (V16 / V16+Particles / Full):
  `src\common\Settings.cpp:253-261` resets them to Stock on load (O7OF commit `4407101`, 2026-11-14), and
  `src\main.cpp:1708-1717` forces Stock for every non-Forge launch. The in-code reason: non-stock profiles "cause
  infinite blackscreen loads in Campaign and network kicks in Firefight/Multiplayer; only Forge mode functions".
- Why it is a networking item: the profiles change the **game-state layout** (pool sizes/banks), so peers must run the
  same profile or they desync. `src\host\EngineLimits.*` already has `ProfileNetworkId` (high byte = profile, low 16
  bits = AI actor count) and hosts can advertise it, but the join-time enforcement is unfinished - note B3 in
  `knowledge\repo-docs\root\AI-LIMITS-REPORT-2026-11-08.md` lists exactly what is missing: add a `"limits"` id to the
  master-server register/heartbeat + dedicated-server advert, parse it into `LobbyState` (including
  `FetchLobbyStateSync`/`JoinLobbySync`), compare before joining and refuse with a clear message; backend must pass the
  unknown field through. Two local instances could not be used unattended for that test (needs two Steam sessions).
- For Firefight/campaign (synchronous co-op), any replicated/deserialized size must match on all peers; saved films
  must be version-stamped when pool sizes change.
- DeepSeek can fix the client-only stability work (Stage 3 black screen, local pool handling) but Claude owns the
  profile advertisement/enforcement, kick/refuse path and the "network kicks" diagnosis.

### 3. Replicated appearance (Stage 6 boundary, if it comes to it)
- Exact per-player visor colour (and possibly selectable armour-effect variants) must reach other players to be correct
  in a hosted match. The stock-slot fallback (`96fd2c4`) needs no replication; an exact-colour path that sends per-player
  appearance data to peers is a networking change (replication bits / session join payload) -> Claude.
- DeepSeek will keep the local Armory/fallback path; if the engine exposes a per-profile appearance colour property that
  replicates for free, that is not a networking change - verify first.

### 4. Host migration / 128-player / session-size work
- Not started in any stage yet. When Tyler lifts the 128-player hold, the session membership -> players arrays ->
  per-player simulation entities/events -> scoreboard/UI order (master list A5) crosses the boundary after the
  "per-player simulation entities/events" step. All of it is Claude's; DeepSeek will document the local-only parts.

### 5. Security
- Out of scope entirely for DeepSeek: anti-cheat boundaries (MCC online uses EAC; Noble stays a separate community
  client), join/session token validation, auth, and anything that could let a modified client join a trusted session.
  If a Stage task touches these, it is recorded here and left to Claude.

### 6. Pinned engine-limit profile (2026-11-15, top-10 item #1) - what Claude needs
DeepSeek re-enabled the Extended limits by **pinning** them instead of offering a user setting:
`limits::kShippedProfile = Full (V16 + particles + light volumes)` and `kShippedAiActors = 1024` for every install.
Every peer should now advertise the same `limits_id` (0x0301_0400: profile 3, layout generation 1, AI actors 1024) and
the existing join refusal should only fire for old/Stock clients. The layout generation (bits 16-23) is bumped whenever
the pinned game-state layout changes, so builds with different pool sizes refuse to join even at the same profile/actor
count - backend passthrough of the field is what makes that effective. The client-side reason for the 11-14 disable was disproven: see
`project-docs\NOTE-EXTENDED-LIMITS-REENABLE-2026-11-15.md` (campaign black screen was the `145510a` launch-block bug,
fixed in v0.19.95 `dad10d2`).

Open items on the networking side:
1. **Backend passthrough of `limits_id` must be confirmed.** Noble sends it on register/heartbeat
   (`MasterServerClient.cpp` ~783/873), parses it (~1092/1323) and the server-browser join refuses a mismatch
   (`FrontEndMenu.cpp` ~6830-6854). If the private backend drops unknown fields, `peek.limitsId` stays -1 and
   "older hosts are let in" - i.e. a Stock/old client could join a pinned host. Verify with a real lobby.
2. **Join paths that never peek the host state** have no limits check: lobby share code (JOIN WITH CODE), direct
   connect, friend invite, campaign/Firefight co-op insertion. Decide: refuse a mismatched/unknown host with a clear
   message, or warn before joining. DeepSeek did not touch session/join code.
3. **Old clients must be refused** - a v0.19.103 client is Stock and its join would desync; version/build gating may
   already stop it, but `limits_id` is the authoritative check.
4. **Fallback layout:** if the pinned profile cannot be prepared (MCC DLL update invalidates `data\limits\full.nlp`),
   Noble logs an error and may run the in-memory runtime profile on the stock DLL, which is a different layout from
   peers. Decide whether online play should be blocked in that state.
5. **Films:** films recorded under the old Stock layout may not replay under the pinned layout; the team already
   dropped film compatibility, but a version stamp/refusal is the safe follow-up.

## References
- `project-docs\PROMPT-DEEPSEEK-PHASE2-FEATURES-2026-11-15.md` (`Out of scope + Stage 2/5/6/8 owner split)
- `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` (git/release rules)
- `project-docs\STAGE1-STATUS-REPORT-2026-11-15.md` (this session's proven/unproven + pool readings)
- `HANDOFF-FINAL-2026-11-15.md`, `docs\limits-players*.md`, `knowledge\drafts\draft-session-networking.md`
- `knowledge\repo-docs\root\AI-LIMITS-REPORT-2026-11-08.md` (B3 join gating notes)

## 7. 2026-11-15 14:20 ET: hold lifted, networking work reclaimed by DeepSeek for item #3
Tyler selected top-10 **item #3** (player counts 32 -> 64 -> 128 + campaign co-op > 4), **lifted the 128-player hold**,
and because Claude is unavailable asked DeepSeek to take **everything** for this item - including the
session/membership/replication work previously reserved for Claude. DeepSeek will still avoid security work and will
keep this note current with the network facts/blockers it finds. Item #1/#2 pinned-profile work is unaffected.

## Log (append-only)
- 2026-11-15 11:35 ET - created from Stage 1: player-count boundary (34-player MP stop, 5-player campaign crash),
  extended-limit profile/network mismatch, replicated-appearance boundary, 128-player hold, security out of scope.
- 2026-11-15 13:15 ET - pinned engine-limit profile (Full + 1024 AI actors) replaces the user setting; the join-path
  and backend-`limits_id` passthrough gaps above are handed to Claude. No networking/security code changed here.
- 2026-11-15 14:20 ET - Tyler lifted the 128-player hold and assigned item #3 to DeepSeek end-to-end (Claude
  unavailable). DeepSeek owns the session/membership/replication work for item #3; security stays out of scope.
