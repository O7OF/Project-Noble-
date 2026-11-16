# DESIGN — Lobby / server-browser honesty for large player counts

Date: 2026-11-16 (ET). Author: lobby-design (subagent), shared task **task-6**.
Baseline: repo `D:\Project-Noble-Closed-Source`, branch `wip-merge`, HEAD **`e531160`** (parent `175637e`).
**Status: proposal only — read-only. No src/build edit was made, no game instance was started (the lead owns the rig).**
Every claim cites `file:line`; guesses are marked **UNCERTAIN**.

Inputs: [NOTE-RR-REFERENCE-2026-11-16.md](NOTE-RR-REFERENCE-2026-11-16.md) (RR UX reference),
[NOTE-MP16-AUDIT-2026-11-16.md](NOTE-MP16-AUDIT-2026-11-16.md),
[STATUS-MULTIPLAYER-2026-11-16.md](STATUS-MULTIPLAYER-2026-11-16.md).

> **Doc-currency warning.** The audit + status notes were written at `4bfe827`, *before* `175637e` (10:59 today) and
> `e531160`. Their "every MP session declares 128 slots" finding is **fixed**: the MP slot branch is now gated
> (`src/hooks/LobbyPatches.cpp:93-103`), and the current 5-player run logs exactly **one** `set_slot_counts public=16`
> call with the session left at 16 slots (evidence: `build/bin/Release/host-rejoin5-20261116-111256.log:732-733`).
> Use the audit for the *pre-fix* failure history only; use this note for today's totals.

---

## 0. TL;DR for the lead

1. **The honest number today is 16 seats** — engine stock 16 player slots / 17 peer records; the hook leaves MP sessions
   at the engine's own 16 (`LobbyPatches.cpp:93-113`), proven by the single `set_slot_counts private=0 public=16` line
   in the 5-player run whose target stayed 16.
2. **The only working large-ish multiplayer run on this HEAD is 5 players with the dev-only 32-slot patch installed**
   (`-slot-patch -slot-players 32` + `NOBLE_SLOT_PLAYERS_UNSAFE=1`; host args in
   `build/bin/Release/rejoin5-20261116-111256-summary.txt`; peak 24.5 GB, 5 players in the host table at 11:14:25).
   Its membership wire payload is the 32-slot variant (`RECV msg 26 size 41728` = 0xA300), so it does **not** prove the
   stock-16 message/table path end-to-end, and no post-`175637e` 2-player *stock* run exists yet (test T0 below).
3. **17-32 is a lie in three places today**: the picker offers 24/32 (`FrontEndPickers.cpp:3649`); the number reaches
   the advert/beacon/lobby header (`FrontEndLobby.cpp:271,293,309`; `SessionManager.cpp:181`) but the engine session
   stays 16 (`SetMaxMultiplayerPlayers` has **no callers**); and a 17th player cannot join (high-slot blocker: host
   removes player 16 ~84 ms after ack, CLOSE 10/14, CONNECT REFUSED 3 — STATUS F1). 64/128 has no seat support at all
   (32-peer DWORD-mask ceiling, STATUS B7).
4. **The 1-row roster is a data-source bug, not a slot bug.** Every roster surface sizes its rows from the *pre-game
   master-server roster*: the loading screen draws `wanted = max(expectedPlayers, players.size(), 1+engineExpected)`
   (`PauseMenu.cpp:4050`) and rebuilds from the local name when `-loading-roster` is absent (`main.cpp:1400-1411`);
   the in-match Noble scoreboard is seeded only when `-session-id` exists (`main.cpp:1584-1609`); the pre-game lobby
   panel starts at host-only and refreshes only from the master (`FrontEndLobby.cpp:361-399, 939-960`). In LAN/direct
   sessions the master roster does not exist, so all three show 1 row even though `GetGamePeerCount()` knows the peers
   (`PauseMenu.cpp:4046`; rejoin5 host log: 0 `scoreboard roster set` lines).
5. **Recommendation (needs Tyler approval, §6):** the picker shows 16 as the only cap above 12 today, with 24/32 greyed
   plus a one-line reason; every displayed cap (lobby header, advert, beacon, browser) uses the *effective engine seat
   count* (16 today, 32 only when the 32-slot mode is genuinely active); roster rows come from session membership, not
   the pre-game roster; `-max-players` is wired into `LobbyPatches::SetMaxMultiplayerPlayers` so declared capacity,
   advert and UI agree.

---

## 1. Evidence base (what was read/verified for this note)

| Source | What it establishes |
|---|---|
| Code at `e531160`: `FrontEndPickers.cpp`, `FrontEndLobby.cpp`, `FrontEndMenu.cpp`, `FrontEndInternal.hpp`, `PauseMenu.cpp`, `ServerBrowser.cpp/.hpp`, `LobbyPatches.cpp/.hpp`, `SessionManager.cpp`, `MasterServerClient.cpp/.hpp`, `main.cpp`, `ReachHost.cpp`, `BlamEngine.cpp`, `SlotPatch.cpp`, `Loader.cpp` | every line cited below |
| `git show 175637e` / `git log` (`e531160` -> `175637e` -> `4bfe827`) | MP slot policy is now stock-16; `e531160` added the harness + `NOBLE_SLOT_PROBE` diagnostics |
| Runs in `build/bin/Release`: `fix5-…105936` (5 players, slot32, peak 23.9 GB), `rejoin5-…111256` (5 players + rejoin, slot32, peak 24.5 GB), `s32decl-…111632` (2 players with `NOBLE_MP_SLOTS32=1`: declares 32, 3713 "Expanding … to 32" lines), `base2-…105147` (pre-fix 2-player failure) | what is actually proven today |
| Captures `data/userdata/claude-tests-2026-11-11/rejoin5-…-host-cap0.png` (Noble scoreboard shows 1 row: "NobleSix"), `…-guestlast-cap0.png` (loading screen shows only "NobleSix (YOU)") | the 1-row observation, plus its limits (see §7 U1) |
| `tools/mp-session-test.ps1` at HEAD | the only N-instance harness; it already syncs every guest exe to the host hash (`:100-114`), unlike the stale-binary bug in NOTE-MP16 §2.6 #45 |

**RR reference mapping** (from NOTE-RR-REFERENCE §3/§4): RR shows `players / max_players` straight from the host's
announced listing (browser.js:509,570 quoted there), verdicts Full / "Room for N of your M" / "Not heard lately"
(browser.js:303-320), freshness STALE=30 / DROP=120 / NOT_HEARD=75 s (browser.js:40-48), and the MAX PLAYERS number
selects a named engine profile (index.html:2065-2068). Noble currently has **none** of those three honesty mechanisms.

---

## 2. (a) Current behaviour — every place a player/max count is produced, clamped or displayed

### 2.1 The MAX PLAYERS number: picker -> lobby -> advert -> engine

| # | Where | file:line | Actual value/behaviour |
|---|---|---|---|
| A1 | **MP/Forge MAX PLAYERS picker** | `src/ui/FrontEndPickers.cpp:3645-3681` | options **{4,8,12,16,24,32}** (`:3649`), default index 3 = **16** (`:3652`). 24/32 are selectable but NOT supported. The comment admits 48+ is excluded only because "the 128-player work is on Tyler's hold", not that 17-32 fails. |
| A2 | Firefight MAX PLAYERS picker | `src/ui/FrontEndPickers.cpp:3682-3713` | **{4,8,12,16}**, default 16. Left alone by this design. |
| A3 | Campaign MAX PLAYERS picker | `src/ui/FrontEndPickers.cpp:2581-2615` | **{4}** only (deliberate honest clamp; comment: 5-player campaign crashes). |
| A4 | UI state defaults | `src/ui/FrontEndInternal.hpp:436-439` | `lobbyMaxPlayers=16`; `sessionMaxPlayers=16` (stale comment says "4..128"); `ffMaxPlayers=16`. |
| A5 | Picker value -> host lobby | `FrontEndPickers.cpp:3827-3834` -> `FrontEndLobby.cpp:244-297` | `st.lobbyMaxPlayers = maxPlayers` (`:271`); `HostConfig.maxPlayers = st.lobbyMaxPlayers` (`:293`); `SessionManager::StartHosting` (`:297`). |
| A6 | Host lobby -> advert | `FrontEndLobby.cpp:299-309` | advert started only when `lobbyNetMode==1 && MasterServerClient::Get().Enabled()` (`:299`); `adv.maxPlayers = cfg.maxPlayers` (`:309`). Local-LAN lobbies advertise nothing. |
| A7 | Lobby launch -> relaunch arg | `FrontEndLobby.cpp:1935` -> `FrontEndMenu.cpp:7326` | `choice.hostMaxPlayers = st.lobbyMaxPlayers` -> `-max-players N`. |
| A8 | CLI parse | `src/main.cpp:1128`, `:1205-1206`, `:1625` | default 16; `_wtoi` with **no range check** (e.g. `-max-players 500` accepted); re-passed to the child on relaunch. |
| A9 | CLI -> HostConfig / advert | `src/main.cpp:1966`, `:1978` | `cfg.maxPlayers = hostMaxPlayers`; `advert.maxPlayers = cfg.maxPlayers`. **Never calls `LobbyPatches::SetMaxMultiplayerPlayers`** (grep: only `LobbyPatches.hpp:15`, `LobbyPatches.cpp:3481-3485`). |
| A10 | Return-from-match restore / demo | `FrontEndMenu.cpp:6733-6743`; `:6449-6528` | `-lobby-max-players` (fallback 16) restores the host config; `-lobby-demo-players N` clamp 0..128 (`:6467`) is a synthetic roster (no network). |

**Net:** the picker number is carried as *display/advert metadata* only. It never reaches the session slot count.

### 2.2 Where the engine/network caps actually are

| # | Where | file:line | Actual value/behaviour |
|---|---|---|---|
| B1 | Engine session slot setter | `src/hooks/LobbyPatches.cpp:78-116` | `Hook_SetSlotCounts` rewrites `publicSlots`: campaign branch <=16 (`:86-92`); **MP branch expands only if `NOBLE_MP_SLOTS32=1` AND `slots::Players32Active()`, then min(expandedMp,32)** (`:93-103`); a fallback branch for the pre-`IsCampaignCoop` window keeps <=16 but logs the "campaign/firefight" label even for MP (`:104-111`) — log-honesty bug, see P1.5. |
| B2 | Baked target + setter | `src/hooks/LobbyPatches.cpp:59`, `:3481-3485`; `LobbyPatches.hpp:15-18` | `s_maxMultiplayerPlayers = 128` (baked); `SetMaxMultiplayerPlayers(n)` clamps 16..128 but **has no callers anywhere**. `Apply` still prints `Target MP: 128` (`:3476-3477`) while MP sessions actually get 16 — misleading log line. |
| B3 | Peer-capacity compare patch | `src/hooks/LobbyPatches.cpp:3006-3017` | writes `static_cast<uint8_t>(s_maxMultiplayerPlayers)` over the `cmp eax,0x10` immediate -> with 128 it writes **0x80** (signed -128). The pattern currently **does not match** this DLL (NOTE-MP16 §2.1 #4: silent no-op) — **UNCERTAIN** whether it ever installs; must be fixed before P4 raises the number. |
| B4 | 32-slot patch gates | `src/host/SlotPatch.cpp:727-737` (`-slot-patch`/`NOBLE_SLOT_PATCH`), `:741-760` (`-slot-players 32`/`NOBLE_SLOT_PLAYERS` + `NOBLE_SLOT_PLAYERS_UNSAFE`), `:843-855` (logs refusal unless unsafe), `:975-983` (16-player inert); install entry `src/host/Loader.cpp:161` | dev-only, all-or-nothing, logs "169 untraced imul sites"; 16-player mode is byte-identical to stock. |
| B5 | Session admission | `src/main.cpp:1966-2018`; `src/ui/ServerBrowser.cpp:1568-1578` | no "session full" check anywhere; `-max-players` does not gate joins (NOTE-MP16 §2.1 #5). A full browser row is still joinable. |
| B6 | Master list read clamps | `src/network/MasterServerClient.cpp:680-681` | `player_count` clamp 0..64; `max_players` clamp 1..64. |
| B7 | Lobby-state read | `src/network/MasterServerClient.cpp:1084-1085`; `MasterServerClient.hpp:56-58` | `playerCount/maxPlayers` defaults 1/16, **no clamp** on the server value. |
| B8 | LAN discovery beacon | `src/network/SessionManager.cpp:173-185` | wire payload `NOBL_BEACON|Name|Mode|Map|Gametype|Players|MaxPlayers|…`; **players is the literal `1`** (`:180`), `maxPlayers` from `m_activeConfig.maxPlayers` (`:181`). Parser accepts players 0..64 / max 1..64 (`:248-252`), ping -1 (`:267`), sessions pruned after 4 s (`:285-295`). |
| B9 | Server-browser own session row | `src/ui/ServerBrowser.cpp:360-390` | `e.playerCount = 1` hardcoded (`:375`), `e.maxPlayers = cfg.maxPlayers` (`:376`). |
| B10 | Advert player count (real) | `src/engine/ReachHost.cpp:798-803`; write `MasterServerClient.cpp:766-784`, heartbeat `:849-874` | `players = GetGamePeerCount()+1` every 60 frames; `max_players` comes from the host config (A9). Write side is unclamped. |
| B11 | Lobby start "expected peers" | `FrontEndLobby.cpp:1936-1940`; `BlamEngine.cpp:2003-2016, 2062-2076`; launch block `:1892-1940` | expectedPeers = non-host roster entries -> `-wait-peers` and the host barrier. Campaign/FF launch block declares players: stock cap 4 (`:1911,1922`), the `NOBLE_COOP_DECLARE_ROSTER` >4 probe can declare up to `min(16,1+expectedPeers)` (`:1917-1920`). MP has no declared-roster field. |
| B12 | Lobby roster size cap in UI | `FrontEndLobby.cpp:2602-2605` | `slotCount = clamp(max(lobbyMaxPlayers, rosterCount),1,128)`; 8 visible rows (`:2637`). |

### 2.3 Where counts are displayed (the surfaces that must become honest)

| # | Surface | file:line | Actual value/behaviour |
|---|---|---|---|
| C1 | Pre-game lobby header | `FrontEndLobby.cpp:2591-2598` | `"%zu/%d", roster.size(), lobbyMaxPlayers` — i.e. the picker value, not engine seats. Panel always reserves 8 row heights (`:2580-2581, :2637`). |
| C2 | Pre-game lobby roster **source** | `FrontEndLobby.cpp:361-399` (host only), `:939-960` + `:1852-1859` (refreshed only from master lobby JSON), relay variant `FrontEndMenu.cpp:6915-6954` | host LAN lobby = **1 row**; refresh gated on `lobbySessionId`/`Enabled()` (`:839-841`); local-LAN lobbies have no session id (A6) -> stays 1 row forever. |
| C3 | Loading-screen roster/count | `main.cpp:1400-1411` (build: `[local name]` when `-loading-roster` absent; total `1+expectedPeers` for host, 0 for joiner); `PauseMenu.cpp:4044-4057` (live `peers`/`engineJoined`/`engineExpected`), `:4089` ("N OF M IN"), `:4118-4184` (rows) | row grid size = `wanted = max(expectedPlayers, players.size(), 1+engineExpected)` (`:4050`) — **`peers` never raises `wanted`**, so a joiner with 4 peers can still draw 1 row. Names only from the lobby roster; unknown slots drawn as "PLAYER n". |
| C4 | In-match Noble scoreboard | seed `main.cpp:1584-1609` (only when `-session-id`); live fill `PauseMenu.cpp:2017-2124`; table reads `PauseMenu.cpp:1868-2003` (g_game_results 16 slots stride 0xD8; TLS players array accepted only when `maxP <= 16` at `:1934`) | direct-connect LAN launches pass no `-session-id` -> no seed; the engine fallback can fill rows but is hard-coded to 16 and its TLS path is disabled when the engine reports >16 players. rejoin5 host log: **0 lines** `PauseMenu: scoreboard roster set`; capture shows 1 row. |
| C5 | In-game server browser rows/details | `ServerBrowser.cpp:1171-1188` ("%d / %d" + capacity bar), `:1396-1400` (details "PLAYERS N / M"), sort by playerCount `:941-950` | honest formatting, dishonest inputs (B8/B9) and no freshness/verdict. |
| C6 | Browser filters / verdicts | `ServerBrowser.cpp:910-911` (`m_hideFull` = `playerCount>=maxPlayers`), `:1172`; version gate `:1556-1566, :1568-1578` | Full is only a *hide filter*; JOIN is never blocked when full; no "not heard lately"/age display; LAN rows vanish by the SessionManager prune (4 s), no age. |
| C7 | Membership/game-player evidence logs | `LobbyPatches.cpp:2595-2597` ("game players: hdr … | 0:… 1:…"), `:2864-2866` ("peer states: p0:8 …") | this is the authoritative in-session roster data the UI should read/publish; already proves 5 players in the rejoin5 run. |

**The lie in one sentence:** the number the player picks is shown to themselves (lobby header), to other players
(master advert / LAN beacon) and to the server browser, but it is never the number of seats the engine session has.

---

## 3. (b) Honest-capability model

### 3.1 What is actually supported TODAY

| Tier | Seats a session can really seat | Evidence | Gate |
|---|---|---|---|
| **Shipped path** | **16** (engine stock 16 player slots / 17 peer records; the MP slot branch now leaves 16) | `LobbyPatches.cpp:93-113`; rejoin5 log has one `set_slot_counts public=16` (`:732-733`); stock tables in NOTE-MP16 §2.4 #30 | none |
| **Proven end-to-end** | **5 players on one machine** — but only with the dev 32-slot patch installed (`-slot-patch -slot-players 32` + `NOBLE_SLOT_PLAYERS_UNSAFE=1`); wire payload 0xA300 | `fix5-…105936` (peak 23.9 GB, 0 crashes), `rejoin5-…111256` (peak 24.5 GB; host table 5 players at 11:14:25, 6 after rejoin); harness args `tools/mp-session-test.ps1:150-158` | dev flags |
| **16 players** | **Not proven** — needs >1 machine (~70-95 GB on this PC; NOTE-MP16 §4.2/§9); the 2-16 scale range is untested (STATUS §5 gap 2) | STATUS F3/F6 | test logistics |
| **17-32 players** | **Does not work.** High-slot join fails: host adds player 16 then removes it ~84 ms later, membership producer 2->1 players, CLOSE reason 10 / guest 14, CONNECT REFUSED reason 3 | STATUS F1 (11-15 14:29 logs); tasks task-1/task-5 (Ghidra) | `NOBLE_SLOT_HIGH=1` test mode only |
| **Declared 32 slots** | Technically possible (`NOBLE_MP_SLOTS32=1` + patch) — 2-player join works, but it **does not seat a 17th player** and churns `set_slot_counts` 3713 lines in a ~70 s run | `s32decl-…111632` host log: 3713 "Expanding … to 32", 0 CLOSE/refusal, peermask 3 both state 8 | dev env + unsafe |
| **33-128** | **Not supported** — the peer mask is a DWORD (32-peer protocol ceiling) | NOTE-PLAYER-COUNTS:71-72; STATUS B7 | none |

Consequences for the UI (this is the whole model):
- **One capability source**: a helper that reports the *effective seat count* the engine session will declare
  (`16` unless the 32-slot mode is both requested and active -> `min(requested,32)`). The picker, lobby header, advert,
  beacon, browser row and roster all read that number instead of the picker's raw value.
- **Never display a capacity larger than the session can seat.** If the user asks for 24 while the effective cap is 16,
  show 16 (and say why in one line); do not advertise 24.
- **A count without a source is shown as unknown, not invented** (RR uses "?" when the host did not announce — NOTE-RR
  §3). No placeholder "1/16" when the real count is 5.
- **Roster row count comes from session membership** (`GetGamePeerCount()` / `GetRemotePlayerAddCount()` /
  `engineExpected/Joined`), with the lobby roster used only for *names*. Unknown names are "PLAYER n", never fake names.
- **Experimental state is labelled**: any host listing >16 is badged "EXPERIMENTAL (17+ joins do not work yet)".

### 3.2 Recommended picker presentation

**By default today (shipped):**
- Options shown: **4 / 8 / 12 / 16**; 16 remains the default (matches the real seat cap and today's tested default).
- **24 and 32 are visible but disabled/greyed** with the reason: *"24/32 player sessions are not supported yet — the
  32-player engine mode is dev-only and a 17th player cannot join."* Arrow/nav stepping skips them; the popup shows them
  dimmed with the reason text (requires the `PickerRow` extension in P1).
- A small line under the row states the effective cap: *"This session will declare 16 seats."*

**When the 32-slot mode becomes real** (all of: high-slot join fixed, a 2-instance LAN run through player 17 passes, the
patch installed by default or explicitly selected):
- Enable **24 / 32** only on builds where the capability getter can report 32; keep **16 as the default**.
- Label 17-32 choices **"EXPERIMENTAL — 32-PLAYER MODE"** while the mode is opt-in; drop the label only when Tyler
  promotes it. Show the *effective seats* under the row exactly like today.
- Keep the value list explicit (4/8/12/16/24/32). RR's 1..N stepper (NOTE-RR §4.1) is their model, not ours; a stepper
  on a stock-16 engine would just invite invalid values.

---

## 4. (c) Ordered, minimal patch plan (no code here — precise instructions)

Order is UI-only first (no engine risk), engine-facing last. Each step ships independently and is revertable.

### P1 — Picker honesty + one capability source (UI only)

**Files:** `src/ui/FrontEndPickers.cpp` (MP picker block `:3649-3681`, summary use `:3827-3834`),
`src/ui/FrontEndMenu.cpp:2992-3099` (`PickerRow`) + `src/ui/FrontEndInternal.hpp:715-720` (decl),
`src/ui/FrontEndLobby.cpp:2591-2605` (header count); new getters in `src/hooks/LobbyPatches.cpp/.hpp` and
`src/host/SlotPatch.cpp/.hpp`.

1. **Add capability getters** (read-only, no engine writes):
   - `noble::slots::Players32ModeRequested()` in `SlotPatch.hpp/.cpp`: `Requested() && ArgPlayers32() && UnsafeOverride()`
     (reuse `SlotPatch.cpp:727-760`; those helpers are not declared in `SlotPatch.hpp` yet — add declarations).
   - `LobbyPatches::GetEffectiveMultiplayerCap()`: returns 16 unless `slots::Players32Active()` in the match process
     (or `Players32ModeRequested()` in the menu process, where the patch is not installed yet), then
     `min(s_maxMultiplayerPlayers,32)`. Also `LobbyPatches::GetRequestedMultiplayerCap()` = the parsed
     `-max-players` value (see P4).
2. **Picker caps**: build the choice list at runtime — base `{4,8,12,16}`; append `{24,32}` with a per-choice
   *disabled* flag when `GetEffectiveMultiplayerCap() < 32`. Extend `PickerRow` with one optional parameter
   (`const std::function<bool(int)>& enabled = {}` + a reason string for the disabled item) so disabled items render
   dimmed, are skipped by arrow navigation and cannot be selected. Keep the returned index space identical so call
   sites do not change. Clamp `st.sessionMaxPlayers` to the effective cap on screen entry and on every `picked`.
3. **Pass the effective cap** into `SetupHostLobby` (`FrontEndPickers.cpp:3829`): pass
   `min(sessionMaxPlayers, effective)`, and log
   `FrontEndPickers: max players effective 16 (requested 24 -> disabled: 32-player mode inactive)`.
4. **Lobby header/panel** (`FrontEndLobby.cpp:2591-2605`): count becomes `roster.size() / effectiveCap`; `slotCount`
   uses `effectiveCap`, not `lobbyMaxPlayers`; if `lobbyMaxPlayers > effectiveCap` add one muted line
   *"Requested N; this session seats M"* under the header.
5. **Log honesty** in the same change (no behaviour): the `Hook_SetSlotCounts` fallback branch
   (`LobbyPatches.cpp:104-111`) should say "multiplayer session slot count" when `!IsCampaignCoop()`, and `Apply`'s
   "Target MP: 128" (`:3476-3477`) should print the effective cap.

**Why safe:** touches only UI rendering/interaction and read-only getters; no engine memory writes, no network fields;
16-and-below behaviour is unchanged. Rollback = revert one UI commit.

### P2 — Real roster rows from session membership (UI/data-source only)

**Goal:** all three roster surfaces take their *row count* from the live session and their *names* from the best known
source; the pre-game master roster becomes a name source only.

1. **Loading screen** (`PauseMenu.cpp:4020-4184`):
   - Change `wanted` (`:4050`) to include live membership: `max(expectedPlayers, players.size(), 1+engineExpected,
     1+peers)` where `peers = max(GetGamePeerCount(), LobbyPatches::GetRemotePlayerAddCount())` (`:4046` already
     computes it). This alone stops "1 row while 5 are in".
   - Add `PauseMenu::SetLoadingLiveRoster(std::vector<std::string>)`; `DrawLoading` prefers live names, then lobby
     roster names, then "PLAYER n". `main.cpp:1400-1411` keeps its fallback for solo launches.
   - Fill live names from the engine TLS players datum array (same 0x490-stride table already read in
     `ReadRawScoresSEH`, `PauseMenu.cpp:1927-1976`), refreshed at ~2 Hz in `DrawLoading` behind SEH; "PLAYER n" until
     the engine creates the datum.
2. **In-match scoreboard** (`PauseMenu.cpp:2017-2130`):
   - Raise the TLS read gate `maxP <= 16` (`:1934`) to `maxP <= 32` only when `slots::Players32Active()`, looping
     `min(maxP,32)` entries; keep the g_game_results table at 16 slots (its growth for >16 is unverified — STATUS B4,
     **UNCERTAIN**).
   - Merge rows by peer/machine id (the row `machine` field already feeds pings, `:2113-2124`) instead of by name/slot,
     so a LAN/direct session shows every player the engine and the transport know.
   - Add `PauseMenu::SeedBoardFromSession()` called from the `ReachHost` frame tick next to `UpdateAdvert`
     (`ReachHost.cpp:798-803`): host row first + one row per transport peer (`GetAllPeerPingsMs()`,
     `NetworkTransport.hpp:114`) with an empty name / "PLAYER n" until the engine table fills it.
   - Log once: `PauseMenu: live roster N player(s) (transport X, membership Y)` for test evidence.
3. **Pre-game lobby panel** (`FrontEndLobby.cpp:361-399`): when `st.isLobbyHost && st.lobbyNetMode==0` (Local LAN,
   no master session), do not pretend the roster is a player list — append a muted status line
   *"LAN SESSION — players join in game; roster appears once they connect"* to the panel; keep the host row and OPEN
   seats. When a master lobby exists, behaviour is unchanged (that roster is complete).

**Why safe:** display/data-source only; no protocol, no engine writes; unknown names are visibly placeholders. The
pre-game online lobby keeps its current roster source, so nothing regresses for the master-server flow.

### P3 — Server browser: real counts, freshness and verdicts (UI/beacon only)

1. **Real LAN player count** (`SessionManager.cpp:173-185`): replace the literal `1` at `:180` with a live count.
   Add `SessionManager::SetBeaconPlayerCount(int)` (atomic) and call it from `ReachHost.cpp:798-803` where
   `players = GetGamePeerCount()+1` is already computed (and set 1 when not in a match). No wire-format change — the
   parser already allows 0..64 (`:248-252`).
2. **Real own-row count** (`ServerBrowser.cpp:375`): use the same live count instead of `1`.
3. **Freshness** (RR STALE=30 / NOT_HEARD=75 / DROP=120 s; NOTE-RR §3):
   - Add `int ageSeconds` + `bool heard` to `ServerEntry` (`ServerBrowser.hpp:10-33`).
   - In `RefreshServers` (`ServerBrowser.cpp:343-464`) keep the previous `m_servers` rows briefly: rows not re-seen keep
     their last-seen time and go `heard=false`; drop after DROP seconds. Set `age=0/heard=true` for rows seen this
     refresh; LAN rows also get `d.lastSeen` from `DiscoveredSession` (`SessionManager.cpp:268`).
   - Display: dim `heard=false` rows; details row "Last heard: N s ago" near `ServerBrowser.cpp:1392-1418`.
4. **Verdicts** (RR: Full / Room for N / Not heard lately; NOTE-RR §3):
   - `ServerEntryStatus(s)`: **FULL** (`playerCount>=maxPlayers`), **NOT HEARD LATELY** (`!heard && age>NOT_HEARD`),
     existing **VERSION MISMATCH** (`JoinBlockedByVersion`, `:1556-1566`), else **READY**.
   - Draw the status next to the row subtitle (`:1151-1165`) and in details (`:1421-1428`).
   - Block joins when FULL: add the check at the top of `AttemptJoin` (`:1568`) and switch the details button
     (`:1493-1499`) to a disabled "SERVER FULL (N/N)".
   - **"Room for N of your M" deferred**: Noble has no party-size concept (only the voice-chat party *channel*,
     `VoiceChat.cpp:337-339, 810`) — **UNCERTAIN / needs Tyler** whether to add one; until then only FULL is shown.
   - Optional badge for hosts advertising > effective cap: "EXPERIMENTAL HOST (N advertised seats)" — only if P1/P4 do
     not already clamp adverts to the effective cap.
5. **Do not invent totals**: the ONLINE/TOTAL counter (`ServerBrowser.cpp:640-648`) mixes master-server stats with
   summed rows; leave it, but label it "in listed games" if touched.

**Why safe:** the beacon value is an existing field with an accepted range; browser changes are display/join-gating only.

### P4 — Wire `-max-players` into `LobbyPatches::SetMaxMultiplayerPlayers` (engine-facing, do last)

1. **Clamp the parse**: `src/main.cpp:1205-1206` — clamp `hostMaxPlayers` to `[1,32]` (or `[16,32]` for MP); log a
   warning when clamped. Prevents absurd adverts from a typo.
2. **Set the engine target before `LaunchMap`** (in the host branch of both the in-process and relaunch paths, which
   converge on `main.cpp:1966-1978`): call
   `LobbyPatches::SetMaxMultiplayerPlayers(EffectiveHostSeats(hostMaxPlayers))` once, right after
   `LobbyPatches::Apply` (`:1838`) / before `LaunchMap` (`:1947`/`:2029`).
   - `EffectiveHostSeats(n)`: `16` unless `n>16 && slots::Players32Active() && NOBLE_MP_SLOTS32` -> `min(n,32)`.
   - Then set `hostMaxPlayers = effective` so `cfg.maxPlayers` (`:1966`) and `advert.maxPlayers` (`:1978`) match the
     engine; for the UI-lobby advert path apply the same clamp in `FrontEndLobby.cpp:293-309`.
   - Log: `LobbyPatches: host max players requested 24 -> session declares 16 (32-player mode inactive)`.
3. **Fix the peer-capacity patch** (`LobbyPatches.cpp:3006-3017`): write the *effective* cap (16 or 32), never the raw
   `s_maxMultiplayerPlayers` (128 -> 0x80). Verify on the current DLL whether the pattern installs at all
   (NOTE-MP16 §2.1 #4 says no) and, if not, log a warning instead of silently skipping.
4. **De-churn/limit the expansion log** (`LobbyPatches.cpp:102-103`): keep a `static` last target and log only when it
   changes (or every 100th call). The `s32decl` run already shows 3713 identical "Expanding … to 32" lines in a ~70 s run;
   the churn itself did not break the 2-player join (0 CLOSE/refusal) but it floods logs and needs re-testing with
   join evidence (T4a).
5. **Do not advertise what cannot be seated** (interim, until 32 is real): advert/beacon/lobby max = effective cap.
   This is the point that changes shipped behaviour -> §6 approval.

**Why safe:** at the default request (<=16) `expandedMp == publicSlots`, so `Hook_SetSlotCounts` takes none of the
expansion branches — identical to today's stock-16 path. The >16 path only activates behind the existing dev gates
(`NOBLE_MP_SLOTS32` + installed 32-slot patch) and is the same configuration already observed working for 2 players
(`s32decl`). Keep it behind the dev gate until 17+ works.

**Dependency order:** P1 -> P2 -> P3 -> P4. P1/P2/P3 are independent of P4, but P3's "experimental host" badge assumes
P4's advert clamp; implement P1+P2 first (they fix the visible dishonesty with zero engine risk).

---

## 5. (d) Test plan (exact runs; reuse `tools/mp-session-test.ps1`)

**Preconditions for every run** (the harness fixes the old stale-binary bug at `:100-114`; still confirm):
```powershell
$R = 'D:\\Project-Noble-Closed-Source\\build\\bin\\Release'
foreach ($n in 2..5) { Copy-Item "$R\\noble-dev.exe" "D:\\Project-Noble-Closed-Source\\build\\bin\\Release$n\\noble-dev.exe" -Force }
$env:NOBLE_WINDOW_SECONDARY = '1'; Remove-Item Env:\\NOBLE_SLOT_HIGH -ErrorAction SilentlyContinue
```
Do **not** run these concurrently with other game instances; the RAM ceiling is ~5 instances (~22-25 GB of 31.8 GB,
NOTE-MP16 §9). Record the exe hash/time from the run summary.

| # | Run (exact) | What it proves | Expected evidence |
|---|---|---|---|
| **T0** | `powershell -File tools\\mp-session-test.ps1 -Players 2 -NoWaitPeers -Seconds 60 -MaxPlayers 16` (NO `-Slot32`) | **stock-16 path end-to-end on this HEAD** (currently unproven after `175637e`) | host: exactly one `set_slot_counts private=0 public=16`; **no** `Expanding multiplayer/forge` lines; `RECV msg 26 size 9552` (stock payload) and guest `decoded true`; both `peermask 00000003`; guest `map 2 (running)`; 0 CLOSE/REFUSED/crash |
| **T1** | Single instance: `noble-dev.exe -lobby-demo host -lobby-demo-players 5 -capture-at 20` (plus a manual Custom Game setup capture) | picker + lobby header show honest caps | new log `FrontEndPickers: max players effective 16 (requested N -> …)`; lobby header `5/16`; setup screen shows 24/32 greyed with the reason; 16 default selected |
| **T2** | `tools\\mp-session-test.ps1 -Players 5 -NoWaitPeers -Seconds 90 -MaxPlayers 16 -CaptureAt 45 -ScoreboardAt 45` (dev slot patch as needed: add `-Slot32`) | roster rows come from membership | host log new `PauseMenu: live roster 5 player(s)`; loading screen `5 OF 5 IN` with 5 rows; `-scoreboard-at` capture shows 5 rows (not 1); `game players: hdr … 00000005` in the log |
| **T3** | Host `noble-dev.exe -host -lan -private -max-players 16 -map 20_sword_slayer -mode multiplayer -quit-after 120`; guest `-connect 127.0.0.1:11774 -lan … -quit-after 100`; browser instance `-capture-at 40` on the SERVERS tab | LAN beacon/browser real counts + freshness | new log `ServerBrowser: LAN row '<name>' players=2/16 age=1s`; browser row shows `2 / 16`; after the host exits the row becomes `NOT HEARD LATELY` for ~75 s then drops; FULL test: host `-max-players 2` with 2 players -> row shows FULL and JOIN is disabled |
| **T4a** | `-Players 2 -MaxPlayers 32 -Slot32` with `$env:NOBLE_MP_SLOTS32='1'` | declared-capacity wiring, dev path | one/few `LobbyPatches: Expanding multiplayer/forge session slot count from 16 to 32` (rate-limited), `Target MP: 32`; 2-player join evidence as `s32decl` (peermask 3, both state 8, 0 CLOSE/refusal) |
| **T4b** | `-Players 2 -MaxPlayers 24` with **no** slot patch / no env | honest fallback | log `-max-players 24 capped to 16 (32-player mode inactive)`; `Target MP: 16`; advert `max_players 16`; stock join evidence as T0 |
| **T5** | 16-row UI offline: `-lobby-demo host -lobby-demo-players 16 -capture-at 20`; and `-scoreboard-demo 16` | 16 players render without layout breaks (already OK per NOTE-MP16 §2.2 #12) | lobby `16/16`, 8 visible rows + scroll; scoreboard 16 rows |
| **T6** | High-slot regression check after P4: `-Players 2 -High -Slot32` + `NOBLE_MP_SLOTS32=1` | P4 must not change the known-broken path | still fails with the same signature (host removes player 16, CLOSE 10) — expected, and proves we only changed the declared count, not the seating |

Reading a pass: T0/T2/T4 use the harness summary + host/guest logs (`<tag>-summary.txt`, `-result.json`,
`mp-session-test.ps1:259-293`); screenshots go through the harness PNG conversion (`:240-257`). Never trust a capture
taken while windows overlap (NOTE-MP16 §4.1 note); prefer the log line.

---

## 6. (e) Needs Tyler approval (ships only after he says yes)

1. **Visible picker change** — 24/32 become greyed/disabled with a reason instead of selectable. This is a shipped
   user-facing cap change (§3.2). *Recommend: yes; interim: keep 24/32 visible-but-disabled.*
2. **Advert/beacon/lobby caps clamp to the effective seat count (16)** — a host who picks 32 will be *shown and
   advertised* as 16 until the 32 path works. It changes release behaviour (browser/other clients see 16). *Recommend:
   yes; it removes the current false advertisement.*
3. **Wiring `-max-players` into `LobbyPatches::SetMaxMultiplayerPlayers`** — changes the engine session's declared slot
   count for >16 requests and adds the 16->32 churn path (already seen logging 3713 lines in a ~70 s dev run). It
   touches the proven 16 config only at default values, but Tyler signs off on the engine-facing change and the test
   rig run (STATUS rule: engine work is his decision).
4. **Server-browser verdicts/JOIN blocking** — FULL rows become unjoinable and rows get dimmed/aged; a UX change from
   today's "everything is clickable".
5. **Default MAX PLAYERS stays 16** — keep; call it out explicitly (he may prefer 12 for the first release).
6. **Any changelog/version/release text** claiming large counts — none until T0/T2/T4 are green and 17+ works; the
   current changelog already over-claims loading-roster scaling for 16/32/64/128 (`src/ui/ChangelogData.hpp:85`).
7. **Promoting 17-32 from dev to shipped** — separate decision after the high-slot blocker (task-1/task-5) is fixed;
   until then 24/32 stay disabled even if the engine "declares" 32.

**Safe interim behaviour (no approval needed, no user-visible cap change):** ship only the *read-only* parts — the
capability getter, the log-honesty lines in `Hook_SetSlotCounts`/`Apply`, the real LAN beacon count (P3 item 1, if
considered internal) and roster row counts that never under-count (P2). Keep the picker list, adverts and engine wiring
untouched until Tyler approves; meanwhile *display* the effective cap next to the picker ("This session will declare 16
seats") — truthful without changing what is selectable.

---

## 7. UNCERTAIN / open questions

- **U1 — the exact "1-row with N players" observation.** The saved 25 s scoreboard capture
  (`rejoin5-…-host-cap0.png`) shows 1 row, but it was taken before the first guest joined (host start 11:13:02, first
  guest `RECV msg 26` 11:13:34). The *structural* 1-row bug (the loading screen `wanted` not counting peers,
  `PauseMenu.cpp:4050`; the scoreboard seed needing `-session-id`, `main.cpp:1584-1609`) is proven by code + the guest
  loading capture; a post-join capture in T2 should settle the observed overlay. **UNCERTAIN** which surface the
  lead's 2026-11-16 observation used.
- **U2 — stock-16 2-player path on this HEAD.** Not re-run after `175637e` (the fix's own verification used
  `-slot-patch -slot-players 32`). T0 is the missing baseline; do not claim "stock 16 works" until it passes.
- **U3 — >16 engine table reads.** `g_game_results` is a 16-slot table (stride 0xD8); whether the 32-slot patch grew it
  is not established (STATUS B4 lists 0x41F510/0x40A760 etc. as not grown). The P2 scoreboard plan only widens the TLS
  players array (known stride 0x490, pool raised to 32 in 32-mode — STATUS W3); `g_game_results` behaviour for 17+ is
  **UNCERTAIN**.
- **U4 — declared 16->32 churn safety.** `s32decl` (2 players) had 3713 re-expansions and no CLOSE/REFUSAL, but the old
  128 churn caused the join regression (fixed in `175637e`); whether 16->32 churn is harmless with >2 players is
  **UNCERTAIN** — T4a must re-run the 2-player join, and the log rate-limit is a mitigation, not a fix.
- **U5 — what the master backend enforces** for `max_players` (the client list read clamps 1..64, the write side is
  unclamped; NOTE-RR §5.3 saw the RR master validate "players is greater than max_players"). **UNCERTAIN** whether a
  >16 advertise is rejected or stored today.
- **U6 — "Room for N of your M"** needs a party size Noble does not have (only a voice-chat party channel). Deferred;
  needs Tyler to decide whether a party concept is in scope.
- **U7 — freshness thresholds**: RR uses STALE=30 / NOT_HEARD=75 / DROP=120 s; Noble's LAN beacons prune at 4 s and the
  browser refreshes at 2 s (`ServerBrowser.cpp:543-544`), so thresholds should be re-tuned for LAN (e.g. LAN 5/15/30)
  vs online (30/75/120). **UNCERTAIN** until tuned on a run.

---

## Appendix — evidence strings to grep in the test runs

- `LobbyPatches: Lobby expansion active (Target MP: 16, …)` — P1 changed from 128.
- `LobbyPatches: set_slot_counts private=0 public=16 ranked=false` — exactly once for a stock-16 MP session.
- `BlamEngine: N/N remote player(s) in session (membership: …, engine peers: …, elapsed …)` — join barrier
  (`BlamEngine.cpp:2062-2063`).
- `LobbyPatches: game players: hdr … 00000005 00000005 …` — used count 5 (P2 evidence, `LobbyPatches.cpp:2595-2597`).
- `PauseMenu: live roster 5 player(s) (transport 4, membership 4)` — new P2 line.
- `PauseMenu: scoreboard roster set (5 player(s), …)` — only exists with a master lobby (today absent in LAN runs).
- `ServerBrowser: LAN row '<name>' players=2/16 age=1s` — new P3 line.
- Harness summary: `build\\bin\\Release\\<tag>-summary.txt` and `<tag>-result.json`
  (`tools/mp-session-test.ps1:259-293`), including crash hits and per-instance peak working set.
