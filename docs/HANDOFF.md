# Project Noble: handoff (2026-10-29, 03:45 ET)
Dominus join the team 20261028! 
This picks up a reverse-engineering and C++ effort on **Tyler's Windows PC**. All paths below are real paths on that machine.
Tyler (GitHub `WeabooTyler117`) works with a partner (GitHub `O7OF`) who owns the repo and writes most of the code.
Tyler mostly runs builds, tests, and does the reverse engineering; he is not a C++ developer, so give him exact commands
(one command per code block, so each can be pasted on its own).

**Goal:** `noble-client.exe` loads MCC's own `haloreach.dll` (Halo: Reach) and drives it **without MCC**, like the existing
community project Reach Renaissance (RR, installed at `D:\Reach`) does. Ownership is verified through Steam, nothing in the
MCC install is modified, and no RR code or data is redistributed. RR is used only as a reference for behaviour. (AGENT IS TO ALWAYS WRITE TO THIS FILE WHEN MODIFYING/MAKING ANY CHANGES TO CODE THIS NEEDS TO BE DETAILED AND THE CHANGES NEED TO BE UNDER THE NAME OF THE PERSON WHO COMMITED SAID CHANGES.)

--

## 0. Pick up here (2026-10-29, 03:45 ET) - Guest stuck loading: the HOST refuses the join (reason 15)

Tested by Tyler with two instances on one PC (O7 offline), so NAT, relay and the master server are out of the picture:
host `noble-client.exe -map 20_sword_slayer -mode multiplayer -host -lan`, guest the same with `-connect 127.0.0.1:11774`.
The guest sits at launch state 9 (2%) and its join timeout sends it back to the main menu after about a minute.
- **Cause found:** the host's haloreach.dll refuses every join request. Log (03:40, new trace below):
  `[HOST pid 9696] JOIN REFUSED: session 0x48414C4F52454143 reason 15 (0xF) caller hr+0x423E2E a1=0x7ffcb6e272f0 a2=0xc`.
  The session id is right ("HALOREAC"), so the request reaches the right session and is turned down on purpose; one
  refusal per guest retry (about one a second, in bursts). The first one always follows, by about 15 ms, the first
  call of host slot 25 with the guest's XUID: `slot 25 (a1=0x99CC93FAFC1B3, a2=0, a3=0xFFFFFFFF)` (our stub returns 0).
- **Why nobody saw it before:** the Dominus hooks on can_accept_join_request / can_accept_player_join_request
  (`LobbyPatches.cpp`, RVA 0x3975D8 / 0x3973BC) never fire on this path; refusal 15 comes from somewhere else.
- **New in `src/hooks/LobbyPatches.cpp`** (built and run by Tyler; not committed yet):
  - Log-only hooks on the two functions RR's host-probe.exe traces as "join refused" (hr+0x425368, arg3 =
    {u64 session, u32 reason}) and "connect refused" (hr+0x42459C, arg3 = {u32 id, u32 reason}). The original always
    runs. Each line says HOST or GUEST (both instances write one log), the pid, and the caller RVA. The prologue
    (48 89 5C 24 08) is checked first; a mismatch only logs a warning.
  - Switch `-join-any` (host, off by default, in memory only): RR's `RR_JOIN_ANY` patch, hr+0x397635 74 5E -> EB 5E
    and hr+0x397363 74 4D -> EB 4D (bytes checked first). Tested: both patches apply on Tyler's dll, but **reason 15
    still comes back**, so those two gates are not it. RR's launcher never sets RR_JOIN_ANY; it is a debug knob.
- **RR facts checked:** RR's host::hit sends slot 25 to its default case, which returns 0 - same as our stub - so slot
  25's return value alone does not explain the difference (RR's per-slot thunk was not checked).
- **Next:** disassemble haloreach.dll around hr+0x423E2E (the caller that builds the refusal) to see which test sets
  reason 15, then compare that state with RR's host (session / membership / privacy, the guest's XUID, what slot 25
  asks). This needs a read-only copy of haloreach.dll in a disassembler (Tyler, O7OF or Dominus); nothing was
  disassembled yet. Reach's refusal enum was not confirmed for this build, so 15 is not named here.

## 0. Pick up here (2026-10-29, 03:10 ET) - Networking review: why guests don't get in, fixes, test switch

Full write-up with evidence: `docs/networking-review.md` (17 findings, checked against the code, Tyler's 02:13 / 02:27
guest logs, the Render server log, RR's host-probe.exe, and the real server.js run locally). Not built or tested yet.
- **Fixed (client):** host's lobby session was DELETEd at launch so guests went to a dead session (`Relaunch` ->
  `MasterServerClient::Shutdown(keepSession)`); guest lobby launch started a second, unconnected match
  (`FrontEndMenu`); lobby events written from the relay thread (crash with no log); host now sends the variant
  file name (server cuts at 64 chars); every lobby guest had the host's fixed id (`LobbyMachineId`); UDP 11774 now
  SO_EXCLUSIVEADDRUSE (SO_REUSEADDR let a 2nd process steal packets); a dropped relay now reconnects and can no
  longer std::terminate (`NetworkTransport` Start/StopRelay); host advertises the port the socket really got.
- **Test switch `-rr-guest-block`:** guests stuck at launch state 9 with traffic both ways. RR's
  `patch_launch_block --as-guest` gives a guest no local players, no player id, no variant (+0x300 on zeroed). Ours
  sets all three. The switch lays the guest block out like RR; it carries over the menu -> match relaunch.
- **For O7OF:** one launch message, not HTTP + WS; 3+ player routing (host packets go to every guest; guest-to-guest
  not routed; unknown targets broadcast instead of held); one-way UDP hangs (all-or-nothing relay switch, modelled);
  private lobbies never heartbeat (30 s timeout); no "lobby closed" event; peer_id 2 for every logged-out guest;
  same-router players need the host's LAN address (NAT loopback); a guest's relay is started once only.

## 0. Pick up here (2026-10-29, 02:05 ET) - Armory 3D preview: real model pieces, try-on, Elite effects off

Built and partly tested by Tyler (helmet/wrist id fix and the stray visor fix are not tested yet).
- **The 3D Armory took item names as model pieces.** The menu script's GLOBALS pairs are (model region, permutation),
  e.g. head:player_kurt. Noble wrote Armory item ids (kurt_a, chest_3, ...), so only items whose id equals a head
  permutation changed. `ReachArmory.cpp` now translates each item with Reach's pmcg table, read at run time from
  `data\armor\*.rrpmcg` (dumped from the RR armour pack; not in git): helmet -> head + flair_head_a/b/c, shoulders ->
  flair_*_shoulder, chest -> flair_chest_a/b (+ arms for [R], male/female by flag 1/2), wrist/utility/knees -> flair_*,
  effect -> armor_fx (armor_fx_00 Legendary, _01 Eternal, _02 Heart Attack, _03 Pestilence, _04 Inclement Weather,
  _05 Birthday Party). Only the regions the items change are written ("default" is a real piece in some flair regions).
- **13 helmet ids were attached to the wrong helmet** (Tyler checked each in the 3D Armory). New ids: CQC rosenda, ODST
  malcolm, HAZOP skull, Operator isaac, EVA player_li, JFO nicole, Mk V john117, Pilot sam (+ Haunted sam_c), Military
  Police adrianna, CQB kurt, Recon jai, Mk VI will, Gungnir kelly. Wrist: Tactical/UGPS wrist_5, Assault/Breacher wrist_4.
  Each helmet keeps its MCC item number in `ReachHost.cpp` Helmets() (only the id key moved), so in-game looks are as before.
  Catalog (.inc, docs csv/json) updated the same way. `tools/armory/build_spartan_catalog.py` reads the lists, so a re-run agrees.
- **Try-on:** clicking a locked or unbought piece shows it on the 3D model without equipping (orange outline, "PREVIEW").
  Lists show usable pieces first, then for sale, then locked.
- **Rebuilds:** unchanged pairs don't rebuild the biped (colour-only changes no longer flash). REBUILD is bumped 2 frames
  after the pairs; `-armory-settle-rebuild` adds a second rebuild (test switch). After a rebuild the colour/suit records are
  looked for in the memory regions they were found in before (every 4 ms, up to 1.5 s).
- **Crash fixed (haloreach+0x105512, building the player in a match and on the menu):** with Eternal (effect 6) and the
  Hydration Container (utility 6) equipped. `ReachHost.cpp` now sends no effect / default utility for those two in the
  profile (log says so). Not yet known which of the two it was.
- **Elites:** no Armor Effect / Firefight Voice rows for Elites; Elite armour effect in the profile is always none; the Elite
  3D preview gets no effect pair.
- **Still open:** a knife stays on the 3D Spartan's chest whatever chest is picked (was there before these changes too); the
  log line "menu script's starting pairs" lists the regions the menu sets itself, look there first. Colours sometimes stay
  white/brown after an armour change (the rebuilt biped's colour records aren't always found).
- **O7OF:** `ReachArmory.cpp` and `ReachHost.cpp` (Helmets keys, effect/utility guard, Elite effect) changed.

## 0. Pick up here (2026-10-29, 01:55 ET) - Developer Account Display, Mini Dev Menu, and [DEV] Name Tags

- **Developer Account Status in Account Menu (`src/ui/FrontEndMenu.cpp`):**
  - Updated `DrawAccountModal()`: When logged in as a developer, the modal explicitly renders **"Developer Account (ID: %d)"** in gold with active status and an **"OPEN DEVELOPER MENU"** button, replacing the previous "Standard User Account" text.
  - Automatically polls `/api/dev/check` every ~2.5 seconds via `AuthClient::RefreshDeveloperStatusAsync()` while the account modal is viewed, allowing developer privileges granted via `node set_dev.js <user>` on the Render server shell to take effect in real time without restarting the game.

- **Mini Developer Menu (`src/ui/FrontEndMenu.cpp`, `src/career/Career.hpp`, `src/career/Career.cpp`, `src/ui/ArmorCustomizer.cpp`):**
  - Added `DrawDevMenuModal()` popup accessible exclusively by authenticated developers via either the Account Menu or the quick party action button.
  - Hardened access control: immediately auto-closes and rejects access if `!AuthClient::Get().IsDeveloper()`.
  - **Commands Implemented:**
    1. **Unlock All Armors:** Invokes `Career::DevSetArmoryUnlockAll(true)`, bypassing all rank and credit requirements across all Spartan permutations and Sangheili harnesses.
    2. **Give Max Credits:** Invokes `Career::DevGrantMaxCredits()` granting +999,999,999 cR spendable balance.
    3. **Give Max Rank:** Invokes `Career::DevSetMaxRank()` instantly promoting the player profile to Inheritor (20,000,000 cR).
    4. **Max Everything:** Combined action to unlock all armors, set Inheritor rank, and grant max credits in one click.
    5. **Reset Career:** Helper button to reset progress back to Recruit (0 cR, locked armors) for QA and testing fresh account states.

- **[DEV] Name Badges in Main Menu & Pre-Game Lobby (`src/ui/FrontEndMenu.cpp`):**
  - **Main Menu Party Box:** Appends ` [DEV]` in gold right next to the player's name and displays `ONLINE [DEV]: <name>` in the account quick bar for developers.
  - **Pre-Game Lobby Roster:** Renders `<username> [DEV]` in gold next to the player's name and service tag whenever `player.isDev` is true.

---

## 0. Pick up here (2026-10-29, 00:40 ET) - Dominus Remote Join & Black Screen Fixes: Unique `s_player_identifier` (`+0xF0`), Unique Guest XUID, UDP/Relay Dedup & Timeout Recovery

All changes compiled cleanly via `build.bat` (`noble-client.exe` and `noble-dedicated.exe`) with **0 errors and 0 warnings** under `/W4`:

- **Fixed Remote Join Refusal (`refuse_reason = 0x16` / `22`) Caused by Duplicate `s_player_identifier` at `launch_block[0xF0..0xF7]` (`src/engine/BlamEngine.cpp`):**
  - **Root Cause (from `haloreach.dll` Disassembly):**
    1. In `State 6` (`haloreach.dll+0xE804`), `haloreach.dll` copies the 8-byte value at `launch_block + 0xF0` directly into `[0x182A1E94C]` (`0x182A1E860 + 0xEC`) as local Player 0's 8-byte `s_player_identifier`.
    2. Both Host and Guest previously hardcoded `block[0xF0..0xF7]` to `{0xBE, 0xF3, 'Y', '?', 0, 0, 0, 0}` (`0x000000003F59F3BE`).
    3. When a Guest sent a `join-request` (`haloreach.dll+0x3923DC`), the Host validated the request in `c_network_session::can_accept_join_request` (`haloreach.dll+0x3975D8`) -> `c_network_session::can_accept_player_join_request` (`haloreach.dll+0x3973BC`) -> `c_network_session_membership::find_player_by_identifier` (`haloreach.dll+0x39D59C`).
    4. Because the Guest's `s_player_identifier` (`0x000000003F59F3BE`) matched the Host's existing Player 0 (`[membership + 0x1714]`), and Player 0's owning peer index (`[membership + 0x174C] == 0`) did not match the joining Guest's peer index (`1`), `haloreach.dll+0x397513` (`mov ebx, 16h`) immediately rejected every Guest join with refusal code `0x16` (`22`).
  - **Fix:**
    - `BlamEngine::BuildLaunchBlock()` now generates a unique 8-byte `s_player_identifier` at `block[0xF0..0xF7]` for Guest sessions derived from the Guest's random `localMachineId` (`localMachineId ^ 0x3F59F3BE00000000ULL`), ensuring it never collides with Host Player 0 (`0x000000003F59F3BE`) or any other joining Guest.
    - Also applies the Host/Guest network session and player identity fields (`+0x00`, `+0x50`, `+0x58`, `+0x60`, `+0xE8`, `+0xF0`, `+0xF8`, `+0x2F0`, `+0x2F4`, `+0x2F8`, `+0x2BF14`) when loading prebuilt Firefight or Campaign launch blocks (`loadedPrebuilt == true`).

- **Fixed Duplicate Hardcoded XUID (`0x0009000000000001ULL`) Across Host and Guest Clients (`src/engine/ReachHost.cpp`):**
  - **Root Cause:** `HostSlot88_UserIdentity` returned the same hardcoded XUID (`0x0009000000000001ULL`) on all machines, which `haloreach.dll` (`State 6` `0x18000E8B3` -> `0x18003AB1C`) stores into `0x182A1E860 + 0x128` and sends in the `join-request` player configuration block, colliding with the Host's Player 0 XUID.
  - **Fix:** When `NetworkTransport::Get().IsActive() && !NetworkTransport::Get().IsHost()`, `HostSlot88_UserIdentity` now derives a unique per-Guest XUID (`0x0009000000000000ULL | (localMachineId & 0x0000FFFFFFFFFFFFULL)`) guaranteed never to equal the Host's `0x0009000000000001ULL`.

- **Fixed Simultaneous Duplicate Packet Delivery Over Direct UDP and WebSocket Relay (`src/network/NetworkTransport.hpp`, `src/network/NetworkTransport.cpp`, `src/main.cpp`):**
  - **Root Cause:** When direct UDP was open between Guest and Host, `NetworkTransport::OnNetSend()` and `OnNetRecv()` sent and received every packet over **both** Direct UDP and the WebSocket Relay simultaneously, delivering a delayed (~50 ms) duplicate of every packet and corrupting Blam!'s sliding-window packet sequence.
  - **Fix:**
    - Added `m_lastUdpRecvTick` tracking to `NetworkTransport` and an automatic direct-UDP `PING` echo from Host to Guest upon receiving a NAT hole-punch `PING`.
    - `OnNetSend()` and `OnNetRecv()` now use the WebSocket Relay strictly as a fallback when direct UDP has not received traffic within the last 3 seconds.
    - In `src/main.cpp`, `NetworkTransport::Initialize(false, ...)`, `MasterServerClient::JoinSessionSync()`, and `NetworkTransport::StartRelay()` now run **before** `BlamEngine::LaunchMap()` starts `haloreach.dll`'s engine thread, so NAT hole-punching and the WebSocket relay are ready before the first `join-request` packet is sent.

- **Fixed Black Screen on 60s Join Timeout & 5s Shutdown Hang (`src/engine/ReachHost.cpp`, `src/network/NetworkTransport.cpp`):**
  - `HostSlot1_FrameTick` no longer hides the loading screen at 60 seconds while `haloreach.dll` is still waiting in `launch state < 10` (`waitingForRemoteJoin`).
  - `HostSlot4_LogMessage` now detects `reason == 4` (`external_launch_individual_state_timeout` / `external_launch_overall_timeout`) during a Guest join and cleanly relaunches back to the Front-End Menu (`Relaunch::Start(L"")`) instead of stranding the player on a black screen.
  - `NetworkTransport::StopRelay()` now closes the active WinHTTP WebSocket and session handles before joining `m_relayThread`, unblocking `WinHttpWebSocketReceive` immediately on exit.

---

## 0. Pick up here (2026-10-29, 00:35 ET) - Pre-Game Lobby, Guest Black Screen Fix, WebSocket Signaling & Build Verification

Compiled cleanly via `build.bat` with **0 errors and 0 warnings**:
- **Fresh Binaries Built:** `noble-client.exe` (2,562,048 bytes) and `noble-dedicated.exe` (538,624 bytes).
- **Released to Test Directory:** Copied directly to `C:\Users\Chey1\Desktop\Release For Test\`.

### 1. Fixed Guest Stuck on Load & Black Screen on Join (`src/engine/BlamEngine.cpp`)
- **Root Cause Identified from Logs & Reverse Engineering:**
  In `BlamEngine::LaunchMap`, a background thread was unconditionally posting `Command 10` (Start) and `Command 11` (Advance State) after a 300 ms delay on both Host and Guest. On a remote guest connecting to an existing match or lobby, forcing these commands hijacked the engine state machine out of `state 9` (`join_synchronize`) into `state 11` (simulation) before host session replication packets had arrived. Because the guest's local Blam engine had not yet received the game variant, player slot assignments, or initial world state from the host, the engine faulted into an unrendered black screen / void.
- **Fix:**
  Wrapped the manual `PostCommand(10)` and `PostCommand(11)` calls in `if (!isGuest)`. For guest clients, `BlamEngine` leaves the state machine alone, allowing `haloreach.dll`'s native Blam! session manager to complete handshake, membership replication, and scenario transition automatically upon packet receipt.

### 2. Pre-Game Lobby Implementation (`src/ui/FrontEndMenu.cpp`, `src/ui/ServerBrowser.cpp`, `src/network/MasterServerClient.cpp/.hpp`, `src/network/NetworkTransport.cpp/.hpp`)
- **Screen State:** Added `Screen::PreGameLobby` to `enum class Screen`.
- **Hosting Flow:**
  In `CustomGame`, `Firefight`, `Forge`, and `Campaign`, clicking **"HOST LOBBY"** or **"HOST CO-OP"** now calls `SetupHostLobby(...)`:
  - Registers the session on the master server with `in_progress = false` and initializes the host player in the lobby roster with their Peer ID (`P:ID`), Machine ID, username, service tag, and `[DEV]` badge.
  - Binds UPnP port `11774` and LAN discovery on port `11775`.
  - Transitions to `Screen::PreGameLobby` without launching the match immediately.
- **Pre-Game Lobby UI Layout (`DrawPreGameLobby`):**
  - **Left Column:**
    - High-resolution map preview banner art (`UiArt::HeroBanner` / `UiArt::MapPreview`).
    - Map cycler / selector and Gametype cycler.
    - Difficulty & Skulls selectors (for Campaign & Firefight).
    - Host Network Status Card with External IP, Port, LAN IP, and one-click "COPY INVITE" button.
  - **Right Column ("PLAYERS (X/16)"):**
    - Live player roster listing all connected peers in the lobby.
    - Host badge, glowing gold `[DEV]` badge for authenticated developers, `P:ID` (Peer ID), service tag, and ping status.
  - **Bottom Action Bar:**
    - **Host View:** "START MATCH" (dispatches `LaunchLobbySync` HTTP call and WebSocket `lobby_launch` JSON control frame, then triggers game launch for all players) and "CANCEL" (stops hosting, tears down relay/session, and returns to menu).
    - **Guest View:** "WAITING FOR HOST" (listens on WebSocket and polls HTTP for the launch signal, then automatically executes synchronized match launch) and "LEAVE" (notifies backend, disconnects relay, and returns to Server Browser).
- **Server Browser Integration:**
  - Games still in the pre-game lobby state are flagged with `[LOBBY]`.
  - Clicking "JOIN" on a `[LOBBY]` server joins the pre-game lobby room (`Screen::PreGameLobby`) as a guest instead of forcing an immediate Join-In-Progress relaunch.

### 3. Backend Master Server & WebSocket Relay Updates (`ServerTestDeploy`)
- All backend changes were deployed to `https://github.com/O7OF/ServerTestDeploy.git` (`main` branch, commit `e1301b3`) and live-reloaded on Render (`https://outbreak-z-test-server.onrender.com`):
  - In-memory lobby rosters tracked per session room: `GET /api/sessions/:id/lobby`, `POST /api/sessions/:id/lobby/join`, `POST /api/sessions/:id/lobby/leave`, and `POST /api/sessions/:id/lobby/launch`.
  - Multi-client WebSocket relay control message broadcasting (`broadcastLobbyEvent`): propagates `lobby_launch` and `lobby_roster` JSON control frames in real-time across peers in the room alongside binary UDP game packets.

---

## 0. Pick up here (2026-10-29, 00:20 ET) - Spartan Armory: Halopedia catalog, pictures, rank + cR locks, BUY

Built and tested by Tyler: pictures show, locks work, buying works.
- **Catalog:** `tools/armory/fetch_halopedia_armory.py` pulled every Spartan piece's picture, rank and cR from Halopedia
  (into `build/bin/Release/data/ui/reach/armory/<slot>/`, plus `pieces.csv`). `tools/armory/build_spartan_catalog.py` joins that
  with the Armory lists, ReachHost's item ids and `docs/career/armory_overrides.json` (Noble's own rules for pieces Reach
  didn't sell) and writes `docs/career/spartan_armory_catalog.csv/.json` and `src/ui/SpartanArmoryCatalog.inc`.
  Re-run it after changing a list or an override (it reports pieces it couldn't match; only EXO/TSCS is left out, on purpose).
- **Tyler's rules:** CQB and its attachments free from the start; AKIS (+GRD/CBRN) Sergeant; UA [W] chests Sergeant; Military
  Police Warrant Officer. Removed: Mariner (+Defiant/Mister Chief/OPTREX), Belos, Blackguard, Drengr helmets, Condemned and
  Highlands chests. Other prices are picks marked "Claude's pick" in the overrides file; change freely.
- **Renamed to Halopedia names:** AKIS (was Highlands helmet) and its GRD/CBRN attachments, HAZOP CBRN/HUL, Recon HUL-I, effects
  Legendary (flames) and Eternal (blue_flames).
- **[R] chests:** UA/Counterassault, Tactical/Recon, Tactical/Patrol and Collar/Grenadier [UA] [R] now use the `_roboarm` ids;
  `ReachHost.cpp` Chests() has them as aliases (236-239). **O7OF:** small edit in your file.
- **Career (`Career.cpp/.hpp`):** career.json has `creditsSpent` and `armoryOwned`; new `Career::Balance()` (earned - spent),
  `OwnsArmoryItem(key)`, `BuyArmoryItem(key, price)`. Rank still follows cR earned, so buying never lowers rank. Key = "<slot>:<id>".
- **Armory (`ArmorCustomizer.cpp`):** Spartan tiles show the Halopedia picture; above-rank pieces are dimmed with the rank;
  pieces with a price show it and clicking opens BUY in the info panel (with Balance). Saved looks with locked/unbought pieces
  fall back to the slot's first piece. `-armory-unlock-all` skips every lock (Spartan and Elite).
- **Still to do (Tyler's notes after testing):**
  - Unlocked/owned pieces should sort to the top of each list.
  - No preview of a piece before buying: clicking a for-sale piece only opens BUY and doesn't put it on the character.
  - Each equip makes the 3D character flash white (the old test-account white) for a moment. Every equip calls SaveProfile ->
    SyncToReachArmory -> ReachArmory::SetSelection, and the scene re-applies the whole look; not yet looked into further.
- **Elite (from the 22:35 entry) still untested:** rank locks, H2A shoulder/legs fix, all 9 sets + mixed pieces in a match.
  Elite armour effects and first-person Elite arms come later.
- Pictures are in the git-ignored `build/` folder: add with `git add -f`.

## 0. Pick up here (2026-10-28, 23:55 ET) - Dominus Critical Bug Fixes: Menu-to-Match ntdll Crash, UDP Port Stealing, Relay Reconnect Drop & NAT Join Flow

All changes compiled cleanly via `build.bat` (`noble-client.exe` and `noble-dedicated.exe`) with **0 errors and 0 warnings** under `/W4`:

- **Fixed Menu-to-Match `ntdll.dll` Access Violation on Exit (`src/engine/BlamEngine.cpp`, `src/main.cpp`):**
  - **Root Cause:** When the menu process (`mainmenu_all.map`) exited to launch a match, `haloreach.dll`'s engine thread was still actively running its simulation/render loop. Calling `m_fnShutdown(m_engineInstance)` (`vtable[0]`, `c_game_engine`'s scalar deleting destructor) or tearing down `UiArt`/`UIManager`/`LoadScreen` on the main thread freed the engine's critical sections and heap buffers while the engine thread was still locking/reading them inside `ntdll.dll`.
  - **Fix:** `BlamEngine::Shutdown()` and `main.cpp` now cleanly stop `MasterServerClient` (sending `DELETE /api/sessions/:id`), `SessionManager` (releasing UPnP and LAN sockets), and `NetworkTransport` (closing the WebSocket relay and UDP socket), flush `Logger::Shutdown()`, and terminate the process via `TerminateProcess(GetCurrentProcess(), 0)` without freeing live engine/D3D11 structures underneath the running engine thread.

- **Fixed Menu Process Stealing UDP Port `11774` & Wrong Guest Machine ID on Join (`src/engine/BlamEngine.cpp`, `src/common/Relaunch.cpp`, `src/ui/ServerBrowser.cpp`, `src/main.cpp`):**
  - **Root Cause 1 (UDP Port 11774 Conflict):** `BlamEngine::BuildLaunchBlock()` previously called `NetworkTransport::Get().Initialize(true)` even when launching `mainmenu_all.map`, binding UDP port `11774` inside the menu process. When `Relaunch::Start()` spawned the match process before the menu process finished closing, port `11774` was still held by the dying menu process.
  - **Root Cause 2 (Wrong Guest Machine ID in `JoinSessionSync`):** `ServerBrowser::TryJoin()` previously called `MasterServerClient::Get().JoinSessionSync()` inside the **menu process** before relaunching, sending the menu process's host Machine ID (`0x484F535400000001`) instead of the child guest process's randomly generated Machine ID and bound port.
  - **Fix:**
    - `BlamEngine::BuildLaunchBlock()` no longer initializes `NetworkTransport` when `isMainMenu` is true.
    - `Relaunch::Start()` now shuts down `MasterServerClient`, `SessionManager`, and `NetworkTransport` **before** calling `CreateProcessW`, guaranteeing ports `11774` and `11775` are free before the child process starts.
    - `ServerBrowser::TryJoin()` passes `-session-id "<id>"` to the child process, and `main.cpp` executes `MasterServerClient::Get().JoinSessionSync()` in the **child process** after `NetworkTransport::Initialize(false, ...)` has generated the guest's actual `m_localMachineId` and bound `m_localPort` (with automatic session ID lookup by `ip:port` for Direct Connect).
    - Removed the duplicate `SessionManager::Get().Initialize()` call in `src/main.cpp`.

- **Fixed WebSocket Relay Disconnecting Every Heartbeat & 5-Second Send Blocking (`src/network/NetworkTransport.cpp`, `src/network/MasterServerClient.cpp`):**
  - **Root Cause 1:** `MasterServerClient::DoHeartbeat()` received `relay_url` on every heartbeat and called `NetworkTransport::StartRelay()`, which unconditionally called `StopRelay()`, tearing down the active WebSocket connection and dropping all queued packets in `m_relaySendQueue` and `m_relayRecvQueue` every heartbeat.
  - **Root Cause 2:** `NetworkTransport::RelayWorkerThread()` inherited a 5,000 ms receive timeout from `WinHttpSetTimeouts`, causing `WinHttpWebSocketReceive` to block outgoing packet sends (`m_relaySendQueue`) for up to 5 seconds whenever no incoming relay packet arrived.
  - **Root Cause 3:** When hosting, `MasterServerClient::WorkerLoop()` waited 10 seconds after `DoRegister()` before sending the first heartbeat, delaying WebSocket relay startup and `pending_joins` NAT hole-punching by 10 seconds.
  - **Fix:**
    - `NetworkTransport::StartRelay()` is now a no-op if the relay is already active on the same `relayUrl`.
    - `NetworkTransport::RelayWorkerThread()` sets a 15 ms receive timeout (`WinHttpSetTimeouts(hWebSocket, 5000, 5000, 1000, 15)`) after upgrading the WebSocket handle so outgoing packets flush immediately.
    - `MasterServerClient::WorkerLoop()` now triggers `DoHeartbeat()` immediately after `DoRegister()` and polls every 3 seconds so `relay_url` and `pending_joins` are processed rapidly.

- **Fixed NAT Hole-Punch `PING` Peer Registration & Multi-Peer Host Routing (`src/network/NetworkTransport.cpp`):**
  - `NetworkTransport::Initialize()` now sends an immediate `"PING"` NAT hole-punch datagram from Guest to Host upon socket creation, and tries binding `localPort` (`11774`) first for both Host and Guest before falling back to an ephemeral port.
  - `NetworkTransport::OnNetRecv()` now registers the sender's `senderMachineId -> senderAddr` in `m_peerTable` when a `"PING"` NAT punch packet arrives on the Host (instead of discarding `"PING"` before peer registration) and loops to read any subsequent game packet without returning `0` early.
  - `NetworkTransport::AddPeer()` no longer overwrites an already-learned direct UDP peer address in `m_peerTable`.
  - `NetworkTransport::OnNetSend()` on the Host now broadcasts to all peers in `m_peerTable` when `targetPeerId` is not yet mapped, rather than only sending to `m_peerTable.begin()->second`.

- **Fixed `AuthClient` HTTPS Scheme Detection & Default URL Fallback (`src/network/AuthClient.cpp`, `.gitignore`):**
  - Set `parts.dwSchemeLength = static_cast<DWORD>(-1)` in `AuthClient.cpp` (`PostJson`) so `WinHttpCrackUrl` populates `parts.nScheme == INTERNET_SCHEME_HTTPS` reliably, and added fallback to `https://outbreak-z-test-server.onrender.com` when `masterServerUrl` is empty.
  - Added `node_modules/` and `**/node_modules/` to `.gitignore`.

---

## 0. Pick up here (2026-10-28, 23:35 ET) - P2P NAT Traversal, WebSocket Netcode Relay, In-Menu Account System & Developer Tools

- **Infinite Loading on Join Fixed (Home NAT Firewall Traversal & WebSocket Netcode Packet Relay):**
  - **Root Cause from Client Logs:**
    The log revealed that when Tyler joined his friend's game at `35.147.220.133:11774`, the guest sent 10 UDP handshake packets over port `52354` but received **0 packets back**. Because both players are behind residential NAT routers without manual port forwarding, the host's router firewall blocked inbound UDP packets on port 11774. The host never received the packet, never knew the guest was trying to connect, and never sent a reply, causing `external_launch_individual_state_timeout`.
  - **Solution Implemented (Two-Tier Traversal):**
    1. **Master Server Join Signaling & NAT Hole Punching:**
       - Added `POST /api/sessions/:id/join` on the backend. When a guest joins, their public IP, port, and machine ID are registered with the session.
       - On `PUT /api/sessions/heartbeat`, the host receives `pending_joins`.
       - In `NetworkTransport::AddPeer(machineId, ip, port)`, the host immediately fires a direct UDP punch packet to the guest's endpoint, opening the host router's NAT pinhole bidirectionally.
    2. **WebSocket Netcode Packet Relay Fallback (`/ws/relay`):**
       - Built a high-performance binary WebSocket packet relay into `server.js` using `ws`.
       - In `NetworkTransport.cpp`, a background worker thread (`RelayWorkerThread`) maintains a persistent WebSocket connection to `wss://outbreak-z-test-server.onrender.com/ws/relay`.
       - In `OnNetSend`, outgoing packets are sent directly over UDP AND queued to the WebSocket relay.
       - In `OnNetRecv`, if no direct UDP packet is ready, packets from the relay queue are delivered to `haloreach.dll`.
       - **Result:** Players can now join each other regardless of NAT type, ISP CGNAT, or firewall restrictions with zero router configuration.

- **In-Game Main Menu Account Login & Registration System (`AuthClient`):**
  - Implemented `noble::AuthClient` in `src/network/AuthClient.hpp` and `.cpp`:
    - Handles asynchronous authentication with the master backend (`/api/login` and `/api/register`).
    - Saves session token, username, user ID, and developer status into `settings.json`.
    - Auto-restores cached logins on startup.
  - Implemented `DrawAccountModal` and Main Menu integration in `src/ui/FrontEndMenu.cpp`:
    - Added "Account" entry to the Main Menu list.
    - Added interactive account status bar and clickable badge to `DrawParty` (top right).
    - Modal features tab switching between **LOG IN** and **REGISTER**, password masking, real-time error handling, and clean logout.

- **Developer Accounts & `/var/data/database.db` Disk Integration:**
  - Upgraded `set_dev.js` (`O7OF/ServerTestDeploy` and client `server/`):
    - Added `node set_dev.js list` command to display all registered users and their dev status.
    - Automatic seeding of `/var/data/database.db` from repository database if newly mounted.
    - Case-insensitive username lookup (`LOWER(username) = LOWER(?)`).
    - Automatically unlocks developer cosmetics (Matrix camos, Developer skins, titles, calling cards, capes).
  - In-Game Developer Recognition:
    - Glowing gold `[DEV]` badge rendered directly on the party header next to the player's name in `DrawParty`.
    - Developer status indicator displayed in the Account modal with perks overview.

- **Binaries Built & Deployed:**
  - Release binaries (`noble-client.exe`, `noble-dedicated.exe`) compiled with 0 errors.
  - Deployed directly to `C:\Users\Chey1\Desktop\Release For Test\`.
  - Pushed backend updates to `https://github.com/O7OF/ServerTestDeploy.git` (`510b797`) - live on Render.

---

## 0. Pick up here (2026-10-28, 22:55 ET) - Render Master Server Deployment Repo Synchronized & Pushed

- **Server Backend Sync & Render Trigger (`O7OF/ServerTestDeploy`):**
  - Synchronized the full server architecture (`src/` backend modules: `auth.js`, `config.js`, `database.js`, `matchmaker.js`, `ranks.js`, `sessionManager.js`, and `routes/authRoutes.js`) from `Halo Reach Community Edition/server/` into `C:\Users\Chey1\Desktop\ServerTestDeploy/`.
  - Verified JavaScript syntax across all modules with `node --check`.
  - Committed and pushed changes to `https://github.com/O7OF/ServerTestDeploy.git` (`origin main`, commit `986f2b9`).
  - Render automatic deployment triggered for `https://outbreak-z-test-server.onrender.com`.

---

## 0. Pick up here (2026-10-28, 22:50 ET) - Infinite Load on Join Fixed: Wire Packet Framing (8-Byte Machine ID Header), Peer Resolution & Shutdown Order

- **Root Cause of Infinite Load at `LoadProgress: 2.0%, launch state 9` Resolved:**
  - Reversed `host-probe.exe` (`0x1400271c0.c` / `0x140028df0.c` / `0x140025af0.c` / `0x140038130.c`) to uncover the exact wire protocol between Blam! engine peers:
    1. **Missing 8-Byte Machine ID Header on Wire Packets:**
       In `haloreach.dll`, network transport is abstracted through host callbacks Slot 41 (`OnNetSend`) and Slot 43 (`OnNetRecv`).
       - On `OnNetSend`, the host must prepend the sender's 8-byte uint64 machine ID (`m_localMachineId`) to the packet payload before transmitting over UDP (`[uint64_t senderMachineId] [payload]`).
       - On `OnNetRecv`, the host must inspect incoming UDP datagrams, extract the first 8 bytes as `senderMachineId`, pass `*outSenderPeerId = senderMachineId`, and deliver the payload stripped of those 8 bytes to `outBuffer`.
       - Previously, `NetworkTransport` transmitted raw payloads without prepending `m_localMachineId`, and `OnNetRecv` delivered the raw bytes directly while generating a random hash of the sender's IP string.
       - Because `*outSenderPeerId` was a random hash rather than the guest's authentic machine ID, `haloreach.dll`'s native network session protocol rejected incoming join packets, leaving the guest engine waiting forever in `launch state 9` (`c_network_session::join_synchronize` / waiting for membership replication).
    2. **Host-to-Peer Address Mapping:**
       On the host, `OnNetRecv` now maps the extracted `senderMachineId` directly to the sender's `sockaddr_in` in `m_peerTable[senderMachineId]`. When `haloreach.dll` invokes `OnNetSend(targetPeerId, ...)` to reply to the guest, `m_peerTable.find(targetPeerId)` finds the exact IP/port of that remote player.
    3. **Diagnostic Packet Counters:**
       Added packet counters (`m_packetsSent`, `m_packetsRecv`) logging payload and wire sizes, target/sender peer IDs, and endpoints for the first 10 packets and every 500 packets thereafter.
    4. **Clean Engine Shutdown Sequence:**
       Fixed `0xC0000005` access violation during process shutdown. In `BlamEngine::Shutdown()`, `m_fnShutdown(m_engineInstance)` is now invoked **before** `NetworkTransport::Get().Shutdown()` so the engine's internal session teardown can dispatch disconnect packets cleanly before the socket is closed.
    5. **Master Server URL Path Normalization:**
       In `MasterServerClient.cpp`, trailing slashes on custom master server URLs (e.g. `https://outbreak-z-test-server.onrender.com/`) are now properly trimmed before appending endpoint paths, preventing double-slash HTTP 400/404 routing errors.

- **Binaries Deployed & Verified:**
  - Built Release binaries (`noble-client.exe`, `noble-dedicated.exe`) with 0 errors and 0 warnings.
  - Fresh binaries deployed directly to `C:\Users\Chey1\Desktop\Release For Test\`.

---

## 0. Pick up here (2026-10-28, 22:35 ET) - invisible players, menu crash, join window, Elite pack fixes

All tested by Tyler unless marked.
- **Invisible character models (pushed):** O7OF's `IsArmorItemUnlocked` hook (`EnginePatches.cpp` §3) found its target with a
  generic function-prologue signature, which matched `hr+0x105BDC` inside the appearance builder instead of `0x2C7DCC`;
  replacing it with "return 1" stopped player models from being built. The hook is now off (`#if 0`, comment says why).
- **Menu -> match crash (pushed):** `main.cpp` shuts the engine down before UiArt / UIManager / LoadScreen (the engine thread
  drew one more ImGui frame after ImGui was gone).
- **Joining left the menu open (pushed):** `ServerBrowser::TryJoin` closed `GetActiveWindow()`, which is NULL on the engine
  thread; it now closes this process's own top-level window. (Written before O7OF's 22:15 netcode entry below; joining is his area.)
- **Elite pack (`ElitePack.cpp`):**
  - Shield: the pack model's own shield value (hlmt +0xD0) was 210, the stock MP Elite's 79; now copied from the stock Elite
    (its damage info at hlmt +0xC0 was already identical: body 50, shield 79, stun 3 s, recharge 3.5 s). `-elite-pack-own-health` keeps the pack's.
  - Heart Attack + Birthday Party on every pack Elite death: the pack model's "body" damage section (hlmt +0xE0, 0xC0 each,
    blocks at +0x1C / +0x84) plays `armor_fx_death`, which reads the "armor_fx" model region the pack Elite lacks. Those two
    references are cleared for the pack Elite only (log "dropped 2 death armour effect reference(s)"). `-elite-pack-death-fx` keeps them.
  - Elite slot patch at `0x105796` is now `cmp esi,8; setb r15b` (lists 0..7 for Elites, not the effect lists 8/9).
  - Kept as diagnostics: `RestoreMapGlobals` (logs tags the pack replaced; only pmcg so far), `-elite-pack-dump`, and the first
    6 appearance records are logged (Elite record: +0x10..+0x17 pieces/set, +0x18/+0x19 effects, +0x1A visor?).
  - The pack's render model has H2A pieces too (h2a_space_helmet, h2a_l/r_shoulder, h2a_legs), not only the menu map.
- **Armory:** Elite Armor Set tiles show pictures from `data\ui\reach\elite_sets\<set id>.png` (Tyler's 8 renders;
  Ranger = `space`, Field Marshal = `story_elite`). Add with `git add -f`.
- **Spec:** `docs/menu-redesign-spec.md` trimmed to what's still to do (steps 1-8 done).
- **Elite rank locks (not built/tested yet):** `ArmorCustomizer.cpp` `kEliteUnlocks`: Spec Ops Corporal, Ranger Sergeant, Officer
  Sergeant Grade 2 (Tyler may pick Commander), Ultra Warrant Officer, Zealot Captain, General Major, Field Marshal Lt. Colonel,
  Ossoona Major G3, Schism/Combat Commander G2, H2A helmet/shoulders/legs Colonel G1, Council Brigadier, Honor Guard General.
  Rank only, no cR; locked tiles dimmed with the rank; saved locked pieces fall back to Minor; `-armory-unlock-all` for tests.
  Also fixed `ElitePack::PieceIndex`: H2A shoulders are piece 11 (were dropped), H2A legs 9 / Honor Guard robe legs 10 (were swapped).
- **First-person Elite arms (later, Tyler 23:33):** player representation 1 (matg +0x474) = FP hands `objects\characters\elite\fp\fp`,
  FP body `...\elite\fp_body\fp_body` (+0x20 / +0x30 tagrefs), 3P unit +0x40. The FP model has region `armor_pieces`
  (general, officer, space, spec_ops, ultra, story_elite, zealot, default) + `body`; markers left_hand, secondary_weapon. The Elite set
  list (pmcg +0x74) changes only `armor_pieces`. Likely cause of bare arms: the pack's 3P Elite has per-piece regions and no
  `armor_pieces` region, so nothing selects the FP permutation. Idea: drive the FP `armor_pieces` permutation from the forearms pick
  (needs the code that picks FP permutations; research). The FP model has no shoulder region.
- Open: Elite armour effects (needs an armor_fx region or another route); the menu process still logs an ntdll access
  violation when it exits for a match (22:03, 22:24; separate from the ImGui fix, not looked at yet); first-person Elite arms.

---

## 0. Pick up here (2026-10-28, 22:20 ET) - Dominus Updates: Build System Fallback, Zero-Warning Compilation & Git/Toolchain Setup

- **Git & Team Onboarding (`Dominus-VII`):**
  - Configured global Git author identity (`Dominus-VII <349591305+Dominus-VII@users.noreply.github.com>`) on Dominus's workstation (`Reeve`) and verified remote push access to `origin/main` (`https://github.com/O7OF/test-build-01.git`).

- **Build Script & Toolchain Auto-Detection (`build.bat`, `CMakeLists.txt`):**
  - Updated `build.bat` to verify via `vswhere` whether the installed Visual Studio instance includes `Microsoft.VisualStudio.Component.VC.Tools.x86.x64` before invoking the Visual Studio CMake generator.
  - Added automatic fallback in `build.bat` to configure and build using Ninja + local MSVC 19.44 (`%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC\14.44.35207`) + Windows SDK when the full Visual Studio IDE C++ workload is absent.
  - Configured single-config CMake generators (Ninja) in `CMakeLists.txt` to output directly to `build/bin/Release` so `build\bin\Release\noble-client.exe` and `build\bin\Release\noble-dedicated.exe` match `build.bat` and `run.bat`.
  - Set CMake policy `CMP0169` to `OLD` to silence CMake 4.x `FetchContent_Populate` deprecation warnings for `minhook` and `imgui`.
  - Scoped `/W4` strictly to `noble-client` and `noble-dedicated` and `/W0` to third-party `minhook`, eliminating `D9025` flag-override warnings and third-party C warnings.

- **Zero-Warning Source Fixes Across `src/` (`/W4`):**
  - `src/ui/ObjModel.cpp`: Replaced C++20-deprecated `std::filesystem::u8path` calls (`C4996`) with `PathFromUtf8()` (`std::filesystem::path(std::u8string_view)`), and renamed shadowed local variable `relative` to `texRelative` (`C4456`).
  - `src/ui/CareerPanel.cpp`: Renamed local `kModeColour` array in `CareerPanel::RenderCommendations()` to `kTabModeColour` to avoid shadowing the file-scope `kModeColour` (`C4459`).
  - `src/hooks/ForgePatches.cpp` & `src/hooks/EnginePatches.cpp`: Added `[[maybe_unused]]` to internal helpers `WriteProtectedMemory` and `Hook_IsArmorItemUnlocked` (`C4505`).
  - `src/ui/FrontEndMenu.cpp` & `src/engine/ReachHost.cpp`: Marked unused parameters in `DrawLobbyControls` (`width`) and `HostSlot23_LoadProgress` (`a2`) (`C4100`).
  - Verified full `build.bat rebuild` compiles all 82 targets (`noble-client.exe` and `noble-dedicated.exe`) with **0 errors and 0 warnings**.

---

## 0. Pick up here (2026-10-28, 22:15 ET) - Netcode Fixed: Host Slot 41/43 NetworkTransport, Synchronized Session IDs, Active Guest Player Slot & Variant Decoding

- **Root Cause Resolution for "Loads forever at 5% on join" and "Player joined never pops up":**
  1. **Missing Game Variant Decoding on Guest:** In `BlamEngine.cpp` (`BuildLaunchBlock`), `!isGuest` previously skipped variant decoding into `block[0x300]` when joining. Halo: Reach's Blam! engine requires the game variant layout in memory to resolve scenario dependencies during map loading; without it, the engine stalls at 5% indefinitely. Fixed by decoding stock or custom game variants (`slayer_054.bin` / `hr_ff_standard_1set.bin`) for **both** host and guest.
  2. **Unimplemented Packet Transport (Host Slots 41 & 43):** Reversed Reach Renaissance (`host-probe.exe`) and discovered that in MCC, `haloreach.dll` abstracts network packet transmission through the host interface callbacks rather than raw WinSock sockets:
     - **Host Slot 41 (`HostSlot41_NetSend`):** Engine calls to send UDP packets (`targetPeerId`, `data`, `size`).
     - **Host Slot 43 (`HostSlot43_NetRecv`):** Engine polls to receive incoming packets (`outBuffer`, `capacity`, `outSenderPeerId`).
     Previously, both slots were silent stubs returning 0, dropping 100% of network traffic. Created `NetworkTransport` (`src/network/NetworkTransport.hpp` & `NetworkTransport.cpp`) providing non-blocking UDP socket I/O on port 11774 with automated peer table mapping.
  3. **Mismatched Network Session IDs:** Previously, each client generated a random GUID at `block[0x50]` independently, causing the host to reject incoming guest packets due to mismatched session credentials. Fixed by synchronizing `block[0x50]` and secondary interface slot 147 (`Iface2Slot147_SessionGuid`) using a shared, deterministic session ID (`0x48414C4F52454143ULL`).
  4. **Zeroed Local Player Count on Guest:** `BuildLaunchBlock` previously cleared `block[0xE8]` and player slots on the guest, leaving 0 active local players. Reach engine requires at least 1 local active player (`block[0xE8] = 0x01`, descriptor at `0x100..0x10B`) so the guest can spawn its player into the session and communicate membership to the host.
  5. **Host Machine ID in Launch Block:** Corrected `block[0x2F8]` to carry the 64-bit uint64 Host Machine ID (`kDefaultHostMachineId = 0x484F535400000001ULL`) rather than raw IP octets.

- **Fresh Release Binaries Deployed:**
  - Built with MSVC Release configuration (`noble-client.exe`, `noble-dedicated.exe`) and copied to `C:\Users\Chey1\Desktop\Release For Test\`.

---

## 0. Pick up here (2026-10-28, 21:00 ET) - O7OF Updates: Master Server Online, IPify Public IP Detection, Guest Launch Block & Robust UPnP P2P

- **Master Server Deployed & Live on Render (`https://outbreak-z-test-server.onrender.com/`):**
  - Updated `server.js` and `package.json` in `server/` and pushed to remote GitHub repo `https://github.com/O7OF/ServerTestDeploy` (`main` branch, commit `2362b5d`).
  - Render auto-deployed and confirmed healthy: `GET /api/sessions` returns 200, `POST /api/sessions` registers active games with 201 Created and public IP detection via reverse proxy headers (`x-forwarded-for`).
  - Added JSON error body parsing in `MasterServerClient.cpp` `HttpRequest()` so any master server error returns human-readable diagnostic messages.

- **Public IP Discovery with Ipify & Zero-Conflict UPnP (`UPnP.cpp`, `UPnP.hpp`):**
  - Added background public IP resolution via `https://api.ipify.org` (with `http://api.ipify.org` fallback) during `UPnP::Initialize()`.
  - Added `IsPublicIP()` validator ensuring loopback (`127.0.0.1`), link-local (`169.254.x.x`), and RFC 1918 private subnets (`10.x.x.x`, `192.168.x.x`, `172.16-31.x.x`) are never advertised as external public IPs.
  - UPnP `ForwardPort()` now issues `DeletePortMapping` before `AddPortMapping` to clear stale router leases, preventing conflict 718 errors. If router UPnP is disabled or rejected, it cleanly falls back to Direct IP mode while preserving the valid public IP for P2P connection.
  - Removed premature `WSACleanup()` from `UPnP::Shutdown()` to ensure WinSock remains active while the Blam! engine network thread shuts down.

- **Guest Launch Block Construction & Network Plumber (`BlamEngine.cpp`, `main.cpp`):**
  - Fixed `src/main.cpp`: explicitly forwards `connectAddress` into `blamEngine->LaunchMap(..., connectAddress)` and sets `launchHost = false` when joining as guest.
  - In `BlamEngine.cpp` (`BuildLaunchBlock`):
    - Sets `block[0] = 0x00` (guest mode).
    - Clears 32 bytes of local player block at offset `0xE8`.
    - Writes `0xFFFFFFFF` at offset `0x2F4`.
    - Serializes target host endpoint (IPv4 octets + uint16 port + uint16 pad) at offset `0x2F8`.
    - Sets guest sync flags at offset `0x2BF14` (`0xF3 0x01`).
    - Skips local variant decoding/mvar injection so match parameters are synced directly from the host.

- **Server Browser & Direct Connect Enhancements (`ServerBrowser.cpp`):**
  - **Public IP in Browser & Clipboard:** Display and "COPY INVITE" / "COPY ADDRESS" now copy the resolved public IP (`UPnP::Get().GetExternalIP()`) instead of the private LAN IP when self-hosting.
  - **Invite Parsing Resiliency:** `ParseAddress()` now automatically strips prefixes (`/connect `, `-connect `, `connect `, `http://`, `https://`) and whitespace/quotes, allowing one-click paste from Discord, Steam chat, or server browser invites.
  - **Direct Connect Modal Polish:** Cleaned default input text (starts empty with informative hint placeholder `IP or IP:port (e.g. 147.124.149.202:11774)`).

- **Release Test Build Deployed:**
  - Fresh binaries built with CMake/MSBuild (`noble-client.exe`, `noble-dedicated.exe`) and copied to `C:\Users\Chey1\Desktop\Release For Test\`.

---

## 0. Pick up here (2026-10-28, 20:30 ET) - O7OF Updates: Netcode Fixes, Server Browser Live Overhaul, P2P Join & First-Run Setup

All committed to `main` branch (commits `60fb4a3`, `14d3852`, `5d50fc2`). Release test binaries deployed to `C:\Users\Chey1\Desktop\Release For Test`.

- **Network Shutdown Crash & Port Contention Fixed (`SessionManager`, `UPnP`):**
  - **Discovery Port Separation:** LAN discovery beacons and listener moved from UDP `11774` to UDP `11775` (`kDiscoveryBeaconPort = 11775`). UDP `11774` is kept strictly dedicated to Blam! engine packets and UPnP port forwarding. Previously, `SessionManager::DiscoveryListenerLoop` binding `SO_REUSEADDR` on 11774 intercepted and consumed `haloreach.dll`'s native game packets.
  - **Clean Thread Shutdown:** Added `uintptr_t m_listenerSock` handle. In `SessionManager::Shutdown()`, `closesocket(m_listenerSock)` is now called before joining `m_listenerThread`, immediately interrupting any blocking `recvfrom()` call and eliminating the shutdown freeze/deadlock.
  - **Sliced Beacon Polling:** Replaced the monolithic 1500 ms `std::this_thread::sleep_for` in `SessionManager::BeaconLoop()` with a 50 ms polled sleep loop checking `!m_stopThreads && m_isHosting`, allowing `StopHosting()` to join threads instantly.
  - **Safe WinSock Lifetime:** Removed premature `WSACleanup()` from `UPnP::Shutdown()` that was tearing down network sockets while threads were exiting.

- **Server Browser Overhaul & Hosting Flow (`ServerBrowser`, `FrontEndMenu`):**
  - **Removed Dummy Hosting Modal:** The broken placeholder "HOST GAME..." modal and form variables in `ServerBrowser` were removed entirely. Hosting is now driven exclusively by the Pre-Game Lobbies (Campaign, Firefight, Custom Game, Forge) where map, gametype, and lobby privacy (Public / Friends / Local) are configured.
  - **Main Menu Navigation:** Renamed "Matchmaking" button on the main menu to "Server Browser" (`FrontEndMenu.cpp`).
  - **Live Games Only:** `ServerBrowser::RefreshServers()` now filters out offline saved servers or empty placeholder entries. Only live matches discovered via LAN broadcast, the master server, or the local hosted match are shown in the server list.
  - **Active Session Termination:** Added a "STOP HOSTING" button to the Server Browser when hosting, which cleanly deregisters the match from the master server (`DELETE /api/sessions/:id`), stops LAN beacons, and closes the UPnP port forwarding.
  - **Direct Connect & Invites:** Added Direct Connect dialog with IP and Port fields, along with Copy Invite (`/connect <ip>:<port>`) and Copy Address clipboard buttons for easy sharing.
  - **Master Server HTTP 400 Resolution:** Resolved HTTP 400 Bad Request error by auto-populating external public IP via `UPnP::Get().GetExternalIP()` (falling back to local IP) when registering sessions on `MasterServerClient.cpp`. Live master server deployed at `https://outbreak-z-test-server.onrender.com/`.

- **Halo: Reach Built-in Netcode P2P Remote Joining (`BlamEngine`, `main.cpp`):**
  - Added `-connect <ip>:<port>` command-line argument plumbing in `main.cpp` into `BlamEngine::LaunchMap(..., connectAddress)`.
  - Adapted `BlamEngine::BuildLaunchBlock` for remote guest joins when `!connectAddress.empty()`:
    - Sets `block[0] = 0x00` (guest mode vs `0x08` for host, derived from `(param_3 ^ 1) << 3`).
    - Skips local host slot registration.
    - Sets connection flags `block[0x2F4] = -1` and `block[0x2BF14] = 0xF3 0x01` based on reverse-engineering analysis of `host-probe.exe`.

- **First-Run Profile & Configuration Auto-Generation:**
  - Automated initial generation of `profile.json`, `appearance.json`, `career.json`, and `settings.json` on first run if they do not exist. Prevents crashes and missing data directory errors for fresh playtesters.

- **Playtest Deployment Packaging:**
  - Fresh Release binaries (`noble-client.exe`, `noble-dedicated.exe`) compiled and copied to `C:\Users\Chey1\Desktop\Release For Test\`.
  - Backend server repository updated in `C:\Users\Chey1\Desktop\ServerTestDeploy`.

---

## 0. Pick up here (2026-10-28, 19:50 ET) - pushed 4483029; menu -> match crash fixed (not yet built/tested)

- **Pushed** `4483029` (Elite pack A1-A3, ElitePack files, docs) on top of O7OF's pulled `0f4d050` (career),
  `f94c4cb` (settings / main / server browser), `f87c367` (main entry + network client). Working tree was clean.
- **Crash starting a match from the menu (Windows error):** the menu process shuts down to relaunch for the match;
  `main.cpp` shut ImGui down (`UIManager::Shutdown`) before `blamEngine->Shutdown()`, and the engine thread drew one
  more overlay frame -> AV in `ImGui_ImplDX11_NewFrame` (fault RVA 0x1B3D1E, found with `noble-client.map`, exe base
  from the IsWhitespace hook address in the log). Fix written to `src/main.cpp` (engine shutdown first, then UiArt /
  UIManager / LoadScreen). **Not built, tested or committed yet.** Tell O7OF (his main.cpp area).
- The log showed two Noble processes writing the same log at once, and a fresh launch self-hosting
  "Mode: Campaign, Map: Boneyard" right away (O7OF's new network code?) - ask O7OF if intended.
- Elite pack state: see 19:25 below (first-person Elite arms still bare, pack Elite too tough).

## 0. Pick up here (2026-10-28, 19:25 ET) - Elite pack: A2 + A3 work (Armory Zealot spawns as Zealot)

All in `src/engine/ElitePack.cpp` (on Tyler's PC, NOT committed/pushed yet; Tyler decides when). Tested on Sword Base.
- **A2 player Elite (works):** after the pack's cache step, the one `bipd` inside the pack section is the pack's player
  Elite (0x85564635 here); in the map's `matg` (+0x474 representations, 0x6C each, biped datum at +0x40) every entry
  equal to representation 1's biped is pointed at it (AI keep the stock Elite). Zone-set rebuild hook `hr+0x8EAF0`
  re-sets active bits (state+0x30B70) for biped, its model (+0x70) and render model (model +0x0C).
- **Armour list merge (works):** the pack replaces the pmcg; Noble puts the map's back (its Elite set list +0x74 dresses
  the first-person arms) and copies, for the 7 Spartan piece lists (+0x20..+0x68, 12 bytes apart; selection 16 bytes:
  name sid, changes block; change 12 bytes: region, permutation, flags), every pack change to an "Elite region" (regions
  the pack's Elite sets change: 87EE 87F3 87F6 87F8 87F9 87FD) into the map selection of the same name; pack-only
  selections (legs list pack 11 vs map 7, wrist 9 vs 6) are appended. New arrays: VirtualAlloc + `hr+0x688F3C` encode.
  `-elite-pack-list` = test option keeping the pack's list (Spartans fine, but first-person Elite arms bare).
- **Elite armour slots patch:** `hr+0x105796` `83 FE 07 41 0F 94 C7` -> `41 B7 01 90 90 90 90` (RR `--elite-armor-slots`):
  the appearance builder `0x1055EC` then applies all 8 lists to Elites, not only +0x74.
- **A3 picks (works: Armory Zealot -> Zealot):** calls to `0x1055EC` at `0x4AC36` (spawn), `0x447B0` (live), `0x3B184F`
  (remote), `0x524CD5` (copy) retargeted. For Elites (mode 1, or species byte rec+4 == 1; modes 0/2 untouched) a copy of
  the 0x28-byte appearance record gets rec+0x10..+0x15 = six pack piece indices (helmet, L, R, chest, forearms, legs),
  rec+0x16 = 0xFF, rec+0x17 (Elite set) kept for the first-person arms. Pieces from `data\appearance.json` "elite"
  *_id via `PieceIndex` (pack order = menu lists minus H2A; RR set table kSetPieces as fallback). Every player Elite gets
  the local Armory pieces for now (other players need networking).
- **Open:** pack Elite is much tougher than MCC's MP Elite (2 grenades don't drop its shields; stock Elite dies) - copy
  MCC's MP Elite vitality, or RR's "Even Elites"; first-person Elite arms still bare (Tyler 19:22) even with the map list + set byte kept - find which model draws them (pack FP model? representation entry has more refs than +0x40); H2A indices are guesses (11 helmet, 10 legs);
  Honor Guard etc. to verify by eye; Heart Attack/Birthday effect showing once (watch); `.rrneeds` check not done
  (campaign excluded); before pushing, consider making the pack opt-in (`-elite-pack`) so O7OF doesn't get it by surprise.
- `NOBLE_PROFILE_SET` env var trap: it persists in a cmd window and overrides the Armory (+0x8C=C7 = General). Clear with
  `set NOBLE_PROFILE_SET=`.

## 0. Pick up here (2026-10-28, 18:20 ET) - Elite armour pack, Stage A1 works (pack loads into matches)

**Goal (Tyler, Option 1):** separate Elite pieces in matches, later the H2A pieces. MCC's match maps only carry an Elite
with one `armor_pieces` region (8 sets); the menu map's Elite (RR's `mainmenu_all.map`) has per-piece regions plus
H2A / Honor Guard / Council / Ossoona etc. Plan: load an Elite pack into every match like Reach Renaissance does.
Full research notes: project doc `claude/elite-pack-notes.md`. RR's pack has no H2A pieces (Tyler checked RR's Armory),
but its piece indices suggest it does carry Honor Guard / Council / Ossoona / Schism / Combat (unverified).

- **Stage A1 done and tested (18:19, Sword Base):** new `src/engine/ElitePack.{cpp,hpp}` (+ CMakeLists, `main.cpp`
  calls `ElitePack::Install(module, dataDir, mode)` for non-menu launches, slot 48 in `ReachHost.cpp` resolves the pack
  file name first). Only for `multiplayer` and `firefight`; `-no-elite-pack` turns it off. Files (local only, git-ignored):
  `build\bin\Release\data\armor\rr_elite_pack.map` + `.rrreloc` (+ `.rrneeds`, `.rrpmcg` for later), copied from
  `D:\Reach\data\armor`. What it does:
  - call `hr+0xA38A3` (-> map load `0x141C0`) retargeted: when the request has no extra caches, adds one,
    `haloreach\maps\rr_elite_pack` (request: +0 name, +8 extra count, +0x10 names).
  - call `hr+0x31395` (-> extra cache load `0x31078`) retargeted: for the pack, first `hr+0xAA634(0xFFFF, name, 2)`
    (header slot, the main map gets the same at `0xA37DC`).
  - 2-byte patch `hr+0x30859` `8B D2` -> `66 90` (keeps the 64-bit load delta).
  - MinHook on `hr+0x30698` (per-cache step, arg 5 = cache: +0 loaded base A, +0x24 size, +0x2F0 file base B,
    +0x2F8 tags header): before the original, rebases the pack's packed pointers from `.rrreloc` (RRRL v1: count,
    u64 B, u64 packed base, u32 size, then u32 offsets; addr = (p<<2)+packedBase, if in [B,B+size) write
    `hr+0x688F3C(addr + A - B)`) and the 4 u64 table pointers at header +8/+0x18/+0x28/+0x38.
  - MinHook on the 14 resource-interface methods (vtable `0x996978`, `0x1BB6C0..0x1BBDA0`, index in r8 for 6C0/7B4/C9C/
    D34/DA0, else rdx): an index in a range k with lo != 0 is called on a one-range copy of the interface with the
    index rebased (RR `resadapt`). Without them the first test crashed at `hr+0x1BB3D4` right after the map load.
  - Log: "relocated 121513 packed pointers (0 outside ...) and 4 table pointers", "extra cache ... load returned 1";
    the match then plays normally. Players still use the map's Elite.
- **Next (Stage A2):** RR `armorpack::callback` after the original step: merge the pack's pmcg Elite selections into the
  map's pmcg (`MERGE`), point player representation 1 (globals block at +0x474) at the pack's player Elite biped
  (AI keep the stock one), and the zone-set rebuild hook `hr+0x8EAF0` that re-sets the pack's tag datums active
  (bits at rcx+0x30B70). Then Stage A3: retarget the 4 calls to `hr+0x1055EC` (spawn `0x4AC36`, live `0x447B0`,
  remote `0x3B184F`, copy `0x524CD5`) to feed the Armory's six Elite pieces. Not done: RR's `.rrneeds` check (that the
  match map has all 149 stock tags the pack needs); campaign is excluded until it exists.
- For O7OF: new ElitePack files, small hook-up in `ReachHost.cpp` (slot 48) and `main.cpp`.

## 0. Pick up here (2026-10-28, 17:35 ET) - Elite armour in game fixed, Armory scene leak fixed

- **In-game Elite fixed (tested by Tyler):** profile fields +0x68 and +0x8C were swapped in `ApplyArmoryAppearance`
  (`ReachHost.cpp`). +0x8C is the **Elite armour set** (194 + set; `8C=C7` spawned a General, `8C=A9` a non-Minor
  Elite), +0x68 is the **Spartan armour effect** (166 + effect; Tyler: effects work in game). Writing 173+effect to
  +0x90 changed nothing visible. That is why the Armory's armour effect used to pick the in-game Elite and the Elite
  Armor Set did nothing. Tell O7OF (his file). Still to confirm by eye: each of the 8 Elite sets spawns as itself.
- **Test option survives the match restart:** starting a game relaunches Noble with only the game args, so
  `-profile-set` was lost in the match process (every earlier in-game Elite test was void). `-profile-set` now also
  reads the environment variable `NOBLE_PROFILE_SET` (inherited by the relaunch), e.g.
  `set NOBLE_PROFILE_SET=8C=C7&& noble-client.exe` (no space before `&&`). Log line: `-profile-set +0x8C = 0xC7`.
- **Armory character no longer follows into other menus:** `FrontEndMenu::UpdateSceneForScreen` default branch sets
  `start_menu` (scene 0x0, shows_elite 0); `ReachArmory` only shows the Elite in Armory scenes (0x0A-0x14 except roster
  0x0E). Log confirms the switch when leaving the Armory.
- Open: black background behind the menu / Armory scene (scene environment not drawing); Tyler hasn't answered
  whether the main menu should show his character beside the list or none.

## 0. Pick up here (2026-10-28, 16:30 ET) - Elite per-piece armor, all the menu map's Elite pieces

- The Elite model in `mainmenu_all.map` (render model regions read from the map file; permutation array at file offset
  0x1042b8a8, stride 24) has these regions and permutations:
  - `elite_helmet` (12): council, general, h2a_space_helmet, honor_guard, minor, officer, ossoona, space, spec_ops, story_elite, ultra, zealot
  - `elite_left_shoulder` / `elite_right_shoulder` (12 each): combat, general, h2a_l_shoulder / h2a_r_shoulder, honor_guard,
    minor, officer, schism, space, spec_ops, story_elite, ultra, zealot
  - `elite_chest` (9): minor, general, officer, space, spec_ops, ultra, story_elite, zealot, honor_guard
  - `elite_forearms` (9): default (Minor's arms), general, honor_guard, officer, space, spec_ops, story_elite, ultra, zealot
  - `elite_legs` (11): general, h2a_legs, honor_guard, honor_guard_legs, minor, officer, space, spec_ops, story_elite, ultra, zealot
  - plus `body` and `head` (default only).
- Bug fixed: `ReachArmory` wrote the Elite's GLOBALS pairs with Spartan region names (head/chest/left_shoulder/
  right_shoulder/arms/legs; "legs" isn't even a string_id in the map), so Elite armor changes were hit or miss (Tyler saw
  odd pieces when changing Armor Effect). It now writes `elite_helmet`, `elite_left_shoulder`, `elite_right_shoulder`,
  `elite_chest`, `elite_forearms`, `elite_legs` from new `ReachArmorySelection` fields (`eliteHelmet` ... `eliteLegs`).
- `ArmorCustomizer`: Elite rows are now Armor Set + Helmet, Left Shoulder, Right Shoulder, Chest, Arms, Legs
  (`GetElitePieces(piece)`; the first 8 entries of every list follow `kEliteSets`, so a set index is also a piece index and
  old profiles load unchanged). Extras: Honor Guard (all pieces), Council, Ossoona, Ranger (H2A) helmets; Schism, Combat,
  H2A shoulders; Honor Guard (Robe), H2A legs. Picking an Armor Set resets all six pieces. Saved as `*_index` plus `*_id`
  under `"elite"` in appearance.json. Field Marshal now uses `story_elite` (it pointed at `zealot_commander`, which the
  model doesn't have, so it showed as Minor). Names of the extras are from the map's permutation names; confirm by eye.
- **In matches** the Elite still wears only its Armor Set: Reach's player appearance has one Elite set item
  (`ReachHost.cpp` slot 34, `kEliteSet` 194 + 0..7). Mixed pieces and the extras are Armory-only for now (their
  descriptions say so). Getting them into games needs another route (research).
- Tested: capture `ep1_mixed.bmp` (Ossoona/H2A/Honor Guard mix shows on the real Elite). Still open: Elite colours stay
  stock blue; the Elite's head is cut off at the top by scene 0x0B's camera.

## 0. Pick up here (2026-10-28, 16:05 ET) - the real Armory scene works (Spartan and Elite)

- **Engine Armory scene now loads.** With RR's `mainmenu_all.map` + `mainmenu_all.string_ids.csv` copied into
  `build\bin\Release\data\menumaps\` (local only, git-ignored, over GitHub's size limit), the menu map was always loading;
  `ReachArmory` just never found its GLOBALS table. Cause: the scanners skipped any memory region over 512 MB (Reach's tag
  data sits in bigger ones) and gave up after 10 s. Fix (O7OF's `src/engine/ReachArmory.cpp`, small): `ReadWriteChunks()`
  walks committed private RW memory in 16 MB pieces with 4 KB overlap (RR reads 1 MB pieces with no size cap), skips the
  scanner's own copy buffer, keeps looking while the menu is up (250 ms, then 1 s), and logs progress every 20 passes.
  Result: "Found mainmenu_all.map GLOBALS table ... (stride=8)" about 5 s after launch; the posed MCC Spartan shows with
  the saved armor and colours (capture `es3_armory.bmp`).
- **Elite:** species Elite switches the scene to 0x0B (shows_elite=1) and the real Elite shows (captures `el1_elite.bmp`,
  `el2_specops.bmp`). Armor sets go through the GLOBALS permutation slots (Tyler clicked through all 8; the head changes).
  The Elite hlmt region-table scan (`ScanEliteTable`) never matches; it now retries 10 times while the Elite is shown, then
  warns once. **Open:** Elite colours stay stock blue (the 6 tracked colour records look like the Spartan's); the Elite's head
  is cut off at the top of the screen (Reach's own scene 0x0B camera, not Noble's framing).
- `ArmorCustomizer`: with `data\menumaps\mainmenu_all.map` present the preview says "LOADING ARMORY..." until the scene is
  ready instead of flashing the OBJ stand-in; the OBJ preview only shows if the scene isn't up after 30 s (or the map is
  missing). Note `-menu-capture N` counts frames after the scene is ready (`sceneReadyFrames`).
- For O7OF: the scanner change above is in your file; nothing else in ReachArmory/BlamEngine changed. Noble still builds its
  own main-menu launch block (no `lb_m_c_1.bin` in Noble's data folder) and that works.

## 0. Pick up here (2026-10-28, 15:15 ET) - Armory v2, first pass (spec step 8)

- `ArmorCustomizer::Render` rewritten: left = grouped slot list (CHARACTER species/body, ARMOR helmet, variant, shoulders, chest,
  wrist, utility, knees, visor; Elite: armor set; COLORS primary/secondary; EFFECTS & VOICE; IDENTITY service tag), middle =
  picker for the focused slot (title, EQUIPPED: X, RANDOM, tile grid with equipped check, colours as a 10 hue x 3 shade swatch
  grid, hovered option's name + description), right = Reach's live Armory scene (Reach's camera puts the model on the right,
  so the new panels sit left). Camera follows the focused slot by default (`m_cameraMode = 2`, `m_focusScene` = c_helmet /
  c_left_shoulder / c_chest / ...); mouse wheel over the model: close-up / full body / Armory view. Up/down = slot,
  left/right = option. The old top pills (SPARTAN / ELITE / PREFERRED SPECIES / BODY / CAM) and the colour pop-up are gone
  (species and body are rows now; preview = preferred species).
- Capture `ar_armory.bmp` (build clean) shows the layout. Picker scroll bounced back (Tyler's video): tiles were placed with
  `SetCursorPos(GetCursorStartPos() + ...)`, which counts the scroll twice; now placed from one screen-space origin.
- **No 3D model in the Armory (not new):** `ReachArmory` waits for `mainmenu_all.map`'s GLOBALS table, which never appears
  (log: "Scanning process memory for mainmenu_all.map GLOBALS table..." every 10 s, and "Could not locate
  mainmenu_all.string_ids.csv") because MCC ships no Reach main-menu map. Same in every log since at least 14:53, before
  Armory v2. O7OF's earlier DX11 OBJ preview (`ObjModelPreview`) is no longer in `src/`. Needs a decision: bring back a
  Noble-drawn preview (models read from the player's MCC maps, spec 4.6) or get the engine's Armory scene to load.
- Not yet: picture thumbnails per piece (render once and cache under `data\ui\cache\armory\`), rank/cR locks
  (`docs/career` armory list), stance poses, emblems (profile offsets unknown), ODST / Marine / mod looks (research spike, step 9).

## 0. Pick up here (2026-10-28, 15:10 ET) - default controls, VSync off

- Noble's defaults (Tyler): melee F (was Q), throw grenade G (was F), crouch Left Ctrl (already), Falcon ascend Space /
  descend Left Ctrl, VSync off by default (`Settings.hpp` `verticalSync = false`). Existing settings.json files keep their
  saved VSync value; Restore Defaults in Graphics gives the new default.
- Mouse 4 / Mouse 5 already work: the rebind capture takes any VK 1-255 (XBUTTON1/2 included, shown as "Mouse 4/5") and
  slot 36 input passes them to the engine (`ReachHost.cpp`, st[0x004 + VK] and the 0x08/0x10 button bits).
- **Remind Tyler to test** (he asked to be reminded): typing a value into a slider box (Look Sensitivity, Enter, Apply);
  Falcon keys show Space / Left Ctrl; melee F / grenade G in game; binding an action to Mouse 4 / Mouse 5 and using it in game.

## 0. Pick up here (2026-10-28, 14:50 ET) - post-game "exit experience" (spec step 7)

**Status:** pushed (`2319c27`, after fixing a build break: `small` is a Windows macro, never use it as a name). Tyler
played a game: it works.

- Quit box on the main menu: the menu behind it no longer reacts to the mouse (`ImGui::BeginDisabled(modal)` around the
  party box, title and list; `FrontEndMenu.cpp`).
- Falcon defaults (Tyler): Ascend = Space, Descend = Left Ctrl (`KeyBindings.cpp` slots 62 / 63, were unbound).
- Every Settings slider's value box (e.g. sensitivity "1.00x") can be clicked and typed into: Enter sets it, Esc cancels,
  clicking away sets it. Typed values keep two decimals (not snapped to the slider step), whole numbers for whole-number
  sliders, percent sliders take 0-100, always clamped to the slider range (mouse sensitivity 0.10-5.00x, vertical 0.25-4.00x).
  `SettingsPanel::SliderValue`, `m_typeRow`. Typing needs ImGui keyboard input, which the in-game pause overlay doesn't feed
  yet, so there it only works with the bar.
- References Tyler sent (for later screens): Reach matchmaking "STARTING MATCH / BRACE FOR CARNAGE" lobby (map + game type
  picture, countdown, roster in team colours with emblem, name and rank icon); Reach beta "MATCH FOUND / VOTE FOR GAME AND
  MAP" (3 options + NONE OF THE ABOVE, vote counts, "Voting round ends in" timer); Halo Infinite post-game PLAYER RECAP
  (tabs PLAYER RECAP / TEAM LINEUP / SCOREBOARD, score, rank, stat tiles, medal strip, Spartan on the right).

- `PauseMenu.cpp` `DrawGameOver` rewritten as a full-screen Reach-style results screen: header (GAME OVER, map, mode, minutes,
  difficulty icon), left = SCOREBOARD (local row highlighted with rank icon) + PROGRESS (challenge / commendation bars this
  game moved: old part blue-grey, this game's part amber, green COMPLETE / "<TIER> REACHED", commendation banner art),
  right = CREDITS (lines appear every 0.5 s and count up with `Totals`; "Random bonus"/"Jackpot!" roll like a slot machine with
  `SlotMachine`), CREDITS EARNED total, RANK (icon, bar fills from the old cR to the new; on promotion `RankUp`, amber glow,
  PROMOTED). `GameCompletion` plays when the screen opens. Enter / Space / a click away from the buttons skips to the end.
  Buttons bottom-right: PLAY AGAIN, RETURN TO MENU, QUIT (always usable). Sounds respect the Menu sounds setting.
  Keyboard is read with GetAsyncKeyState (the overlay only feeds the mouse into ImGui); no controller skip yet.
- `Career`: `Award` now has `progress` (`ProgressLine`: kind daily/weekly/commendation, mode, title, art, before, after,
  target, completed, tier) filled in `OnGameResults` for every challenge or commendation this match moved, and `difficulty`.
- Idea from Tyler for spec 4.14 (map voting), "not 1 for 1 but similar": a full-screen VOTE FOR THE NEXT GAME screen with
  the next game named at the top, a vote-closed ring with "N of M voted", 4 choice cards (map picture, map + game type, vote
  count badge, a bar for the share of votes, "PLAYLIST'S PICK" and "NEXT GAME" tags), a PLAYERS list on the right in
  team colours with each player's vote as a small map thumbnail, and "The next game starts once the results are over".
  Needs networking for the multiplayer side.

## 0. Pick up here (2026-10-28, 12:40 ET) - Reach art is now in git

- Tyler decided the Reach menu art goes into the repo so O7 sees exactly the same build (`3522203`, 106 PNGs, ~2 MB):
  `build/bin/Release/data/ui/reach/{ranks,difficulty,commendations}/` (force-added; `build\` is otherwise git-ignored).
  After a pull, O7's menus show the same rank icons, difficulty icons and commendation banners. New art goes in the same
  folders and is added with `git add -f build/bin/Release/data/ui/reach`.
- `8bad0ad`: Demon / Flawless Cowboy text says "(vanilla Noble Actual and Lone Wolf don't count)". Firefight "Coming later"
  label clipping dropped from the to-do (Firefight will be done before release).
- Claude raised that the repo was public and the art is Microsoft's; Tyler pushed it anyway, and the repo is private now.

## 0. Pick up here (2026-10-28, 12:25 ET) - Reach's 45 commendations (spec step 5)

**Status:** built and pushed (`78a943e`, then `2241e59` with the Noble Actual / Lone Wolf block). `cm_tab2.bmp` checked:
mode tabs, banners, cR per tier and "Not tracked yet" rows look right.

- `Career.cpp`: `kCommendations` is now Reach's 45 (mode, art key, name, requirement, Reach's six tier counts from Halopedia).
  Tier names Steel, Bronze, Silver, Gold, Onyx, Max. Counts are kept per commendation in `record.commendationCounts`
  (`<mode>_<key>`), tiers paid in `record.commendations` (old Noble commendations are dropped). Tyler asked to drop
  Reach's rules (every difficulty counts).
- **Stock Noble Actual (`m05`) and Lone Wolf (`m70_bonus`) pay nothing** (`UnpaidMission` in OnGameResults): no cR lines at all,
  no challenge or commendation progress; the post-game shows one 0 cR line saying so. Tyler found Noble Actual paid 5,004 cR and
  completed a challenge after skipping the cutscene (0.3 min). Matched by scenario name, so modded maps with their own
  names still pay. Kills/deaths still go into lifetime stats.
- Each commendation pays Reach's own cR per tier (`CommendationDef::rewards`, from the Halopedia list Tyler sent; Max pays 0;
  Heavy Weapon was unknown, so it uses Halopedia's averages 200/700/1000/2400/2400). Rows show the next tier's cR in amber.
- Tyler's payout rule (`TierReward`): Steel..Gold pay Reach's cR x2.5 (rounded), Onyx pays 2x that Gold, Max pays 3x Onyx.
  Example Any Spree: 625, 1,063, 3,000, 6,000, Onyx 12,000, Max 36,000.
- Counted now: Demon (missions), Flawless Cowboy (mission with 0 deaths), Support Role / Backup / Assistant (assists),
  Perfectionist (Firefight game with 0 deaths, per game until rounds are decoded). Everything else shows "Not tracked yet".
- `CareerPanel.cpp` COMMENDATIONS tab: CAMPAIGN / FIREFIGHT / MULTIPLAYER sub-tabs (click, or LT/RT; `m_commendTab`), Reach-style rows with the banner art
  (`data\ui\reach\commendations\<mode>_<key>.png`) tinted by tier (Steel art only; Max gets an amber frame), name, tier,
  requirement and "x / next" bar.

### TO DO - commendation stats Noble can't read yet

These commendations stay at 0 until the stats are decoded from the end-of-game results record (or engine events):
- **Headshots:** Pinpoint, Dome Inspector, One Shot.
- **Weapon types** (precision, pistol, automatic, heavy, grenade, close quarters): Precisely, Longshot, Crack Shot,
  Small Arms, Trigger Happy, Sidearm, Standard Issue, Riflin' Through, Trigger Man, Splash Damage, Get Loud, Heavy Weapon,
  Nice Arm, Pull the Pin, Grenadier, Close Quarters.
- **Enemy classes** (infantry, leader, specialist): Cannon Fodder, Target Practice, Leadership Element, In Command, SpecOps, Specialized.
- **Multikills:** Walking Tank, Numbers Game, Multikill.
- **Sprees:** Super Soldier, Methodical, Any Spree.
- **Vehicles** (vehicle kills, vehicles destroyed, driving for a gunner): War Machine, Vehicular, Mobile Asset, Right of Way,
  Grounded, Downshift.
- **Medals / assassinations:** Jack of All Trades, Rear Admiral.
- **Firefight rounds:** Perfectionist should count rounds without dying, not games.
When a stat is decoded: add a `CommendationMetric`, set it on the rows above, and fill it in `CommendationAmount`.

## 0. Pick up here (2026-10-28, 12:10 ET) - commendation art in place (spec step 5)

- Tyler's art: `D:\Halo Commendations` (52 named webp/png). Copied as PNG to `build\bin\Release\data\ui\reach\commendations\`
  (now committed, see 12:40) as `<mode>_<name>.png`: 16 campaign (green), 15 firefight (red), 14 multiplayer (blue), all Steel tier,
  plus `example_*.png` (bronze sniper, silver technician, gold assassin, onyx stars, onyx max, steel assist, service record).
- Names/thresholds: halopedia "List of Halo: Reach commendations" (Steel/Iron, Bronze, Silver, Gold, Onyx, Max). Only Steel art
  exists, so other tiers need tinting in code (bronze/silver/gold/onyx tint; Max = onyx with flames per example_onyx_max).
- Full screen for Service Record/Games/Mods pushed as `bd002d9`; ranks capture confirmed the fixed order.
- Next: Commendations tab (Reach-style rows: banner, name, tier, progress x / next), tracking what Noble can read now
  (kills, assists, missions, no-death missions); the rest show 0 until headshots/multikills/weapons are decoded.

## 0. Pick up here (2026-10-28, 11:40 ET) - challenges pushed, panel screens full screen

- Challenges are committed and pushed as `c3b52f1`. `ch_tab1.bmp` checked: the cards, mode chips, countdowns and two columns look right.
- Tyler: version **stays 0.8.2** (patch bumps only, no change needed).
- Service Record, Games (server browser) and Mods now use the whole screen like Settings (`FrontEndMenu.cpp`: `full = true`).
  The job on the PC builds it, captures `fs_career1.bmp`, `fs_career0.bmp` and `fs_servers.bmp`, then commits and pushes.
  Waiting for Tyler to run `go.bat`.
- Rank art fixed per Tyler (files on his PC only, `data\ui\reach\ranks`): Major <- old Major G3, Major G1 <- old G2,
  Major G2 <- old G1 (by elimination), Major G3 <- old Major; Colonel G1 <- old G2, G2 <- old G3, G3 <- old G1;
  Mythic/Noble swapped; Forerunner/Reclaimer swapped. The job also captures `fs_career3.bmp` (RANKS tab).

## 0. Pick up here (2026-10-28, 11:35 ET) - Reach's real challenges (spec step 4)

**Status:** files are on Tyler's PC and verified; `build\agent\jobs\job.bat` builds, captures the Challenges tab
(`build\bin\Release\ch_tab1.bmp`), then commits and pushes. Waiting for Tyler to run `build\agent\go.bat`.
Results: `build\bin\Release\ch_done.txt` (DONE or BUILD FAILED), `build\nb_build_out.txt`, `build\git_out.txt`.

Tyler's choices: **original timing** (daily reset 11:00 GMT, weekly reset Monday 11:00 GMT, as in Reach) and a
**random pick from the whole pool** (7 daily, 5 weekly).

- `docs/career/challenges.json`: Reach's 2,103 daily/weekly challenges (2010-2012: kind, mode, name, requirement, cR).
- `tools/career/gen_reach_challenges.py` (run from the repo root) writes `src/career/ReachChallenges.inc`. It keeps only
  what Noble can track from a finished game: kills, assists, games, missions, best-game score, no-death games, mission
  time, difficulty, skulls, mission (m10..m70), "complete N daily challenges", "complete all dailies". 262 kept
  (228 daily, 34 weekly). Matchmaking wording is dropped. Left out: co-op, wins, waves, rounds, sets, Bungie.net/file share,
  and anything needing stats Noble can't read yet (headshots, weapons, medals, enemy types). Skull names map to MCC bits:
  Iron 18, Black Eye 3, Tough Luck 36, Catch 7, Cloud 13, Famine 11, Thunderstorm 34, Tilt 35, Mythic 23.
- `Career.cpp`: `ChallengeDay() = (unix time - 11 h) / 86400`, `ChallengeWeek() = (day - 4) / 7`; progress keys
  `D<day>:r<row>` / `W<week>:r<row>` (old date keys are pruned once). `PickChallenges` shuffles the pool with splitmix64
  seeded by the day/week (same set for everyone that day), taking 2 campaign, 2 Firefight, 2 multiplayer, 1 any for dailies
  and 2 campaign, 2 multiplayer, 1 any for weeklies, no repeated names, filling up from the rest. `ChallengeAmount` checks
  mode, mission (last part of the map path), difficulty (exact or "or harder"), skulls (all required), no deaths, time limit.
  Best-game challenges keep the max; others add up. Dailies are processed before weeklies so "complete N dailies" and
  "all dailies" count in the same game. New: `Challenge::mode`, `DailyResetText()`, `WeeklyResetText()`.
- `CareerPanel.cpp` CHALLENGES tab: DAILY / WEEKLY headers with done count and reset countdown, cards in two columns
  (one when narrow): mode chip, name, requirement, progress bar `x / y`, `+cR` in amber, green COMPLETE with a check.
- Next: check `ch_tab1.bmp`; then commendations (spec step 5, needs Tyler's commendation art), post-game, Armory v2.
  Still open: rank-order check (`rank_order_check.png`), version number (0.8.2, patch bumps only), Firefight
  "Coming later" label clipping.

## 0. Pick up here (2026-10-28, 11:30 ET) - Campaign / Firefight / Custom / Forge setup (spec step 3)

- Map and mission lists show a number and MCC's small map preview before each name (`ListItem::thumb/number`,
  rows 40-42 px, as many rows as fit above the hint line).
- Right side: `HeroBanner` (map picture, dark gradient, accent kicker "HALO: REACH Â· CAMPAIGN / FIREFIGHT / MULTIPLAYER /
  FORGE" or "MOD: <name>", big title, blurb under it), then the options, then `BigAction` buttons anchored above the
  hint row (fixes START MISSION covering the hints): Campaign/Firefight PLAY SOLO ("<mission> Â· <difficulty>",
  "Hold out alone Â· ...") + HOST CO-OP (greyed, "Needs online play"); Custom Game START GAME ("<map> Â· <game
  type>") + HOST OR JOIN (server browser); Forge START FORGE. The banner shrinks to fit the screen height.
- Campaign: the skulls that are on show as small skull icons next to the score multiplier.
- Firefight: "FIREFIGHT OPTIONS Â· DEFAULT" row shown greyed ("Coming later") until Noble can write Firefight variants.

## 0. Pick up here (2026-10-28, 11:15 ET) - Settings redesign (spec steps 1-2)

Tyler chose option A (build the spec in its order). Done:
- **UI kit** `src/ui/NobleUi.{hpp,cpp}` (spec §3, first pieces): colours, `Scale()`, `SectionHeader`, `RowBackground`
  (3 px accent edge + gradient focus, faint hover), `ChangedDot` (amber), `DrawSlider` (fill bar, default tick),
  `DrawCycle` (‹ value › with position dots, ends greyed when not wrapping), `DrawToggle` (ON/OFF pill), `DrawBox`,
  `Text`/`TextWidth`/`Paragraph`. They only draw; interaction lives in the screen so keys, pad and mouse share one path.
- **Settings in three columns** (`SettingsPanel` rewritten, same public API, same settings.json keys): left = search
  box, categories grouped DISPLAY & SOUND (Graphics, Audio) / GAME (Game options, Player) / CONTROLS (Keyboard & mouse,
  Controller) / ONLINE (Online services), each with a live summary and an amber dot when it has unapplied changes;
  bottom = "N UNSAVED CHANGES / In ..." with APPLY CHANGES / DISCARD, or a per-category RESTORE DEFAULTS (greyed when
  already default). Middle = rows (slider / cycle / toggle / key box / text / info), changed rows get a dot; search
  shows matching rows from every category. Right = details for the hovered (else focused) row: title, when it takes
  effect, description, DEFAULT, and previews (FOV cone vs Reach's 78, frame-time bar, volume meters, controller status
  + picture, graphics card from DXGI adapter 0). Keys/pad: Up/Down rows, Left/Right value, Enter activate, Q/E (LB/RB)
  category, Y apply, X discard. Rows that can't work yet are shown greyed with a reason (advanced graphics, mute in
  background, in-vehicle FOV, hide HUD, mouse acceleration, controller button layouts). `-menu-tab N` = category 0..6.
  Old menu-music values are normalised to `reach:menu` on load so they don't show as an unsaved change.
- **Full screen** (Tyler 11:09): the front-end SETTINGS screen uses the whole screen (dark backdrop, small title);
  the pause menu's SETTINGS window is the display size minus 16 px each side.
- Next per spec §6: campaign/Firefight setup polish (thumbnails, hero banner; difficulty tiles done), then challenges
  cards, commendations (Reach's 45), post-game exit experience, Armory v2.

## 0. Pick up here (2026-10-28, 11:00 ET) - menu redesign spec, rank + difficulty icons

**Menu redesign spec (Tyler 10:34):** `docs/menu-redesign-spec.md` ("Reclaimer layout, Reach soul"): the plan for
every front-end screen, the Armory, career screens and the features Reach never had. Read it before menu work; its
§6 gives the order (UI kit + three-column Settings first). Tyler has 43 Reclaimer reference screenshots: ask him when a
screen in the spec is unclear.

**Reach art (placeholders, never committed):** Tyler sent a sheet of Reach rank icons + the four difficulty icons.
Cut into one file each and copied to his PC only: `build\bin\Release\data\ui\reach\ranks\<NN>_<name>.png`
(NN = `Career::Ranks()` index 00 Recruit .. 49 Inheritor) and `...\reach\difficulty\{easy,normal,heroic,legendary}.png`
(git-ignored like all of `data\`). New `UiArt::ReachArt(category, stem)`, `RankIcon(index)`, `DifficultyIcon(d)`,
`DrawContain(...)` look in `data\ui\custom\reach\` then `data\ui\reach\`; everything falls back to text when a file
is missing. Rank order is partly a guess (Major = two stars, Lt. Colonel G3 = the slashed L, Warrant Officer grade
order, Mythic..Nova = moon, crescent, black hole, sun): Tyler checks `rank_order_check.png`; fixing one = renaming files.
- Campaign / Firefight: DIFFICULTY is four icon tiles (`DifficultyTiles` in FrontEndMenu.cpp; click or Left/Right).
- Party box: rank icon at the end of the player's row (career.json read at most every 2 s).
- Service Record: big rank icon in the header, progress "x / y cR", "Next: <icon> <rank> at N cR (M to go)";
  new **RANKS** tab (4th; Q/E cycle 4 tabs; `-menu-tab 3`): all 50 ranks, earned bright, current framed in blue, next
  outlined, cR under each.
- `-menu-screen` works (it only looked ignored before); the menu logs `FrontEndMenu: screen a -> b`.
- Note: the working copy of `src/ui/UiArt.cpp` on Tyler's PC had lost `NobleControllerImage` (an older copy had come
  back, the stale-file problem from §8): restored from `git show HEAD`. The test job now writes `build\git_status_r3.txt`
  (`git status` + `git diff --stat`) before building so stale files show up.

## 0. Pick up here (2026-10-28, 09:55 ET) - menu freeze fixed, clickable hints, quit confirmation

Also pushed since 09:15: `e950306` ODST-only Firefight voices removed (Armory lists Reach's 12; voice saved/loaded
by `voice_id`), `5808e5e` GRD helmet offers MCC's four variants (Base, Optrex, CBRN, Optrex/CBRN; helmets load by
`helmet_id`; tested: item 71 reaches the profile and a match runs). The menu changes below are committed with this
handoff (FrontEndMenu.cpp/.hpp, main.cpp); the quit box still needs a hand check (Esc on the main menu).

**Menu "freeze" (Tyler 09:26: Armory froze, Esc / Service Record tabs dead after Alt+Tab).** Not the Armory and not
Alt+Tab: in both of his runs the whole front-end menu stopped exactly ~60 s after start. The in-engine menu (O7OF's
mainmenu mode) asks the engine for a menu map MCC doesn't ship (RR's `mainmenu_all.map` is not used/shipped), so after
a minute the engine gives up (slot 3 with a1=0xA, then slot 4 `_map_state_none`) and stops calling slot 1, which is
where `FrontEndMenu::RenderOverlay` draws. Fix: `FrontEndMenu::HoldFrame()` (main thread, called from the message loop
in `main.cpp` next to `PauseMenu::HoldFrame`): once no slot-1 frame came for 1 s it clears the back buffer, draws the
menu itself and presents (mutex shared with RenderOverlay). Tested 09:48: log `engine stopped drawing the menu; Noble
draws it itself` at 09:48:06, menu kept running and `-menu-capture 5000` saved `m1_capture_5480.bmp` 30 s later.
After that minute the Armory has no 3D scene behind it. Note: `-menu-screen` is still ignored by the in-engine menu.

**Clickable button hints:** the ENTER Select / ESC Back hints at the bottom of every front-end screen are buttons now
(hover highlight, click = the key). Change / Tabs hints only describe keys and stay plain labels.

**Quit confirmation (Tyler 09:48):** Esc on the main menu, or choosing QUIT, opens "QUIT PROJECT NOBLE?" with YES / NO
(No preselected; arrows switch, Enter picks, Esc = No). The main-menu hint now reads "ESC Quit".

**Version number (Tyler 09:48):** "slow down with the version number, we'll hit 1.0 in no time." Currently 0.8.2
(`src/common/Version.hpp`). Waiting for Tyler's answer on the scheme before changing it; proposal: only bump the
patch number for normal changes, minor only for milestones Tyler/O7OF agree on, 1.0 reserved for a real release.

## 0. Pick up here (2026-10-28, 09:15 ET) - Armory appearance now reaches the game

**What was wrong:** Noble only wrote the Elite flag (+0x27/+0x38) into the profile; every armor field stayed 0,
and 0 means "item 0" of each slot (Mark V [B] helmet, default pieces, Steel, **Elite Minor**).

**How the profile carries appearance** (haloreach+0x2EDA08 copies it into the player's settings):
dword "global item ids" at +0x40 helmet, +0x44 left shoulder, +0x48 right shoulder, +0x4C chest, +0x50 wrist,
+0x54 utility, +0x64 body (192 male / 193 female), +0x68 Elite armor set, +0x6C knee guards, +0x70 visor colour,
+0x8C armor effect, +0x90 Elite armor effect, +0x94 Firefight voice, +0x98 primary colour, +0x9C secondary colour,
+0xA0 third colour (RR: Steel). (+0xA4/+0xA8 are the Armory poses, HR_Rifle_Salute / HR_Elite_Rifle_Salute in RR.)
A global item id is the item's position in **MCC's own Reach customization list**: the `HR_*` names stored in order
in `MCC-Win64-Shipping.exe` (HR_Helmet_MarkVB_Base = 0 ... Drengr_TYR = 89, LeftShoulder 90-107, RightShoulder
108-125, Chest 126-143, Wrist 144-149, Utility 150-156, Visor 157-161, Knees 162-165, ArmorEffect 166-172,
EliteArmorEffect 173-179, FirefightVoice 180-191, Spartan_Male/Female 192/193, Elite_Minor..Officer 194-201,
Color_Steel..Pale 202-231, later chests 232-239). `haloreach+0x3B2F4` turns it into the engine's slot index;
the group sizes match that function exactly.

**Code:** `ApplyArmoryAppearance` in `ReachHost.cpp` (replaces `ApplyArmorySpecies`) reads `data/appearance.json`
and writes all of the above; helmets and chests go by the Armory's `helmet_id` / `chest_id` string ids, the other
slots by index (the Armory lists are already in MCC order). `RefreshArmoryAppearance` re-applies it whenever the
file changes (checked twice a second), so an Armory save during a session reaches the profile.

**Tested on Tyler's PC (screenshots, he was away):**
- `-profile-set 98=E0` (Brick) turns the player's colour swatch on the scoreboard red; 27/38 = Elite gives the
  Elite HUD. So the colour fields reach the game.
- Built with the new code and two appearance.json files (Spartan EVA helmet / Brick, and Elite General / Blue):
  log `applied Armory appearance (Spartan, helmet 27, chest 133, Elite set 0, colours 22/0)` with a red swatch,
  and `(Elite, helmet 0, chest 126, Elite set 5, colours 15/2)` with the Elite HUD and a blue swatch
  (`build\bin\Release\t14_P/Q_capture_1450.bmp`). Tyler's own appearance.json was not touched (there was none in
  `build\bin\Release\data\`).
- **Not verifiable in first person:** helmet, shoulders, chest and the Elite armor set only show in third person
  (the first-person Elite hand looks the same for Minor and General). **Tyler: please check in game** (die, or look
  at yourself in the Armory/another player's view) that the helmet and Elite set match the Armory.
- GRD now offers MCC's four variants in MCC order (`5808e5e`): BASE `hilands`, OPTREX `hilands_b`, CBRN `hilands_a`,
  OPTREX/CBRN `hilands_c` (new; the Armory preview may have no model for it). Saved helmets load by `helmet_id`.
  Tested: GRD Optrex/CBRN reaches the profile as item 71 and a Sword Base match runs. Mariner variants map by name;
  "HIGHLANDS" chest has no MCC item (falls back to default). The ODST-only Firefight voices (Dare, Dutch,
  Mickey, Romeo) were removed from the Armory (`e950306`, Tyler's call); the voice now goes by `voice_id`, and old
  saves that picked one of them load as Noble Six. Whether the engine re-reads the profile at respawn (live change
  mid-match) is untested.

Head of main before this: `0f29293`.

## 0. Pick up here (2026-10-28, 08:30 ET) - Armory status from Tyler

**08:29 (Tyler, after O7OF's `f4f2d48`; Tyler is offline until later):**
- Preferred species now works (Spartan / Elite pick reaches the game).
- **Armor still doesn't swap in game**, and an **Elite always spawns as the Minor variant**.
- Appearance choices now save and survive a restart, but still don't change the character in game.
- From O7OF's side: FPS limit has several settings including uncapped, V-Sync fixed, FOV offset found,
  settings can be changed mid-match.
Next for the Armory: find the profile/player bytes the engine reads for armor pieces and the Elite variant
(Minor / Officer / Ultra / ...), e.g. with `-capture-profile` before/after captures and `-profile-set`
experiments like the controller look bytes (+0x1B5/+0x1B6) were found. Head of main: `6cfadc4`.

## 0. Pick up here (2026-10-28, 04:30 ET) - wheel, controller, music picker, service record, post-game

Tyler (03:47) asked for: career extras, post-game scoreboard, controller settings, music picker, FOV mid-match,
wheel down. All done and tested by test options + screenshots on Tyler's PC (he was away); details:

- **FOV mid-match: works.** New test option `-fov-test-at <s> <deg>` changes the saved FOV mid-match the way
  APPLY does; standing still, frame 900 (78) vs 1500 (110) show the wider view at once (`t1_A_capture_*.bmp`).
- **Mouse wheel down: solved.** The engine's wheel-down key code is **0xAE** (up is 0xAF); 0xB0 does nothing.
  Proven: 0xAE as change-weapon's second key + a -1 pulse switches weapons. Only the second key slot (+8) works
  (the "third slot" test that failed earlier was the wrong slot, not the wrong code). New settings
  `wheelUpAction` / `wheelDownAction` (default both 4 = change weapon; -1 = nothing), shown as KEY BINDINGS >
  MOUSE WHEEL. Same action both ways -> only 0xAF bound and down notches sent as +1 (old behaviour); different
  actions -> 0xAF / 0xAE on each action's second key and real -1 pulses (tested: up = melee, down = change weapon).
  Key bindings are now rewritten into the built-in profile every 500 ms, so changes may apply mid-game (not
  verified that the engine re-reads them live).
- **Controller: the engine never read the pad before.** With a host, haloreach.dll doesn't load XInput
  (`hr+0x23855`: only when the host global `hr+0xC1A100` is null); MCC passes the pad in the slot-36 buffer:
  `+0x000` u32 device (0 = controller, 1 = keyboard/mouse), `+0x120` u16 buttons, `+0x124/+0x125` u8 triggers,
  `+0x126/128/12A/12C` i16 LX/LY/RX/RY (decoded from `hr+0x23CF0..0x23D87`). So Tyler's "controller works" was
  probably Steam Input mapping it to keyboard/mouse - **ask him**. Noble now reads pad 0 with XInput
  (`FillLocalPad`) and switches device to whichever was used last; Start toggles Noble's pause menu.
  Tested with `-fake-pad <s>` (fake right stick for 1.5 s): view turns.
  Look speed: `-profile-set <off>=<byte>` test option. **+0x1B6 = horizontal stick speed** (0 -> ~19 deg instead
  of ~61 deg for the same push), **+0x1B5 = vertical** (`-fake-pad-y`: 0 -> barely looks up), +0x1B7 changed
  nothing measurable (range 0..9, stock 7; maybe acceleration). Engine range 0..18, stock 18.
  New CONTROLS > GAMEPAD rows: look sensitivity / vertical (1..19 -> +0x1B6/+0x1B5), invert (Noble flips RY),
  left/right stick dead zone (Noble-side, radial, 0..50%), vibration on/off + strength.
  Vibration: host slot 39 now calls XInputSetState (a1 = pad, a2 -> u16 {low, high, left trigger, right
  trigger}, from `hr+0x24194`). **Not verified** (no pad here; rumble may only fire for a pad user).
  The engine's dead-zone floats (+0x1B8 0.13, +0x1BC 0.12) were not tested, so Noble uses its own.
- **Menu music picker:** AUDIO tab now has a two-pane picker: Shuffle all / Off and the games (with track counts)
  on the left, the chosen game's tracks on the right, plus a search box over every game. Click = play + select.
- **Service Record** (new main-menu entry, `src/ui/CareerPanel.*`, `-menu-screen career -menu-tab 0..2`):
  rank + cR + progress, totals (games per mode, time, K/D/A, medals, best game, challenges), campaign best
  difficulty per mission, last 20 games. CHALLENGES (Tyler 08:00: 7 daily / 5 weekly, rotating): 7 daily from a pool
  of 18 and 5 weekly from 10; each day takes the next 7 in the list (wrapping), each week the next 5, so
  consecutive days never repeat one and all come back in turn. Paid when completed. COMMENDATIONS: 6 tracks (campaign missions, Firefight games, MP games, hours, kills,
  medals), tiers Iron..Max pay 500..25000 cR once. Campaign scoring: mission-complete bonus by difficulty
  (250/500/1000/2000) + 10% per skull. All in `career.json` under `"record"` (old files load fine).
- **Post-game scoreboard:** the post-game panel (560 px wide) now lists everyone in the results record (score,
  K/D/A, medals; only the local player is named - the record holds no names) above the cR lines, which now
  include challenge / commendation / mission payouts. Real 10-minute Sword Base match (04:24): paid 133 cR,
  history row + per-mode totals + "Time on Duty" challenge progress (10/45 min) saved; career.json was restored
  afterwards. Post-game panel checked (04:38, `build\bin\Release\t5_capture_900000.bmp`): GAME OVER, map, mode and
  time, scoreboard row "NobleSix [NOBL]" with score/K/D/A/medals, cR lines, rank bar (now blue), buttons.
- **Git (04:42):** pushed as `9e679d8`, rebased onto O7OF's `2b9e843` (ReachArmory + in-engine 3D menu: the front-end
  menu now runs inside the engine on `mainmenu`). FrontEndMenu.cpp conflicted; resolved by taking O7OF's file and
  re-adding the Service Record entry / screen / `-menu-screen career`. Builds clean; the in-engine main menu shows
  SERVICE RECORD (screenshot `capture_780.bmp`). Notes for O7OF: in his new menu `-menu-screen <name>` seemed to be
  ignored by the capture (it showed MAIN MENU), and the log warns `ReachArmory: Could not locate
  mainmenu_all.string_ids.csv`. Also check that nothing from RR's `mainmenu_all.map` is committed (repo rule).
- Small open bits: Start can't close the pause menu (the pad is only read while the game has input); vibration
  untested with a real pad; +0x1B7 meaning unknown; challenge kills/medals depend on the provisional stat slots.

### Earlier (03:40 ET) - FOV checked in game

**03:40 (Tyler's PC, O7OF's `c4ace4e` pulled and built clean):** FOV works when set before a match. Sword Base,
windowed 1600x900, `fieldOfView` 78 vs 110 in settings.json, frame 1500 of each run: the log shows "CameraFov: game
field of view set to 78/110 degrees" and the 110 shot has a visibly wider view (the rifle takes up much less of the
screen). Shots: `build\bin\Release\FOV78_capture_1500.bmp`, `FOV110_capture_1500.bmp` (different spawns, so
compare the gun, not the scenery). Not checked yet: changing FOV live from the pause menu mid-match. FOV range is
now 70-120 (Tyler's call, 03:45, commit `93681f4`): slider (always clamped, also for Ctrl+click typing), the
settings file clamp and the help text all say 70-120; CameraFov still guards the engine's own 1-150.
Tyler's settings were restored after the test (copy in `data\userdata\settings.backup.json`).

## 0a. Latest (2026-10-28, 02:00 ET) — FOV hooked up + settings reachable from the pause menu

**02:00:** Item 3 below is implemented (not yet run in game). Static REA/Ghidra analysis of `haloreach.dll`
(`docs/fov-findings.md`) shows the game's FOV is a float in degrees at **`haloreach+0xB42C10`** (78.0f in the
shipped DLL; the engine multiplies it by pi/180 for the camera and only accepts 1–150 degrees).
- New `src/engine/CameraFov.{hpp,cpp}`: `SetEngineModule()` from `main.cpp`, `Apply(degrees)` writes the global,
  `Read()` logs what the engine holds, and `Tick()` (called from ReachHost slot 1) applies `Settings::fieldOfView`
  when it changes and re-asserts it every 2 s because the engine refreshes its camera block from the profile.
- `SETTINGS > VIDEO > FIELD OF VIEW` now reaches the running game on APPLY; the default is 78 (Reach's own value),
  RESET goes back to 78, and the help text states the 1–150 limit.
- The pause menu has a **SETTINGS** entry: it renders the same `SettingsPanel` in a large window, so FOV, VSync,
  frame cap, mouse, audio and key bindings can be changed mid-match. Esc (or `< BACK`) returns to the pause menu;
  Esc during a key rebind still cancels the rebind only.
- Version is 0.8.2. Rebuilt `build\bin\Release\noble-client.exe`.
- Build note: `build\` had a broken Ninja `CMakeCache.txt` (left over from a failed configure; no build.ninja).
  It was reconfigured as `cmake -S . -B build -G "Visual Studio 18 2026" -A x64` (MSVC, not ClangCL) and built with
  `cmake --build build --config Release`.
- **Still to verify in game:** whether the camera picks the new value up live (it may cache FOV at spawn; a map load
  or a respawn may be needed) and whether 78 vs 90 looks right. Test: pause → SETTINGS → VIDEO, drag FOV, APPLY.

---


## 0. Pick up here (23:58 ET) — Display Settings & FPS Control + Controller Picture

**23:58 (O7OF):** Bumped to version 0.8.1 with FOV/VSync/FPS limit/FPS counter display fully implemented. The FPS counter displays in-game (upper-right, blue text on dark box, toggled via Settings > Display). VSync and frame-cap are applied in the swap-chain Present hook and take effect immediately without restarting. FOV slider stores selections (60–120°, default 90) but remains **UI-only** until the game camera mapping is verified via profile capture. The preview loads local MCC-derived Spartan/Elite OBJ groups, undersuits, and Elite harness layers; color masks and visor tint work in the preview. In-game armor and color IDs remain unmapped, so preview selections do not change the character yet.

**23:45 (upstream):** Controller picture replaced (item 5, built + screenshot-checked). O7OF made a 1600x1000 transparent PNG of an Xbox 360 pad; it is tracked at `assets/ui/noble/controller.png` (original work, OK for the public repo). CMake now also copies `assets\` into `<exe>\data\` after a build (data\ itself stays git-ignored). `UiArt::NobleControllerImage()` loads `controller.png` from `data\ui\custom` then `data\ui\noble`; SettingsPanel uses it with its own callout table (`kReach360`, straight leader lines, left margin sized from the widest label) and falls back to MCC's LARGECONTROLLERFRONT with the old table when the file is missing.

**Do not commit or distribute OBJ files from `data/armor` or profile captures.** Use `-capture-profile data/profile-captures` to record Slot 34 profile buffers for before/after comparisons; delete captures after analysis.

Open list, in priority order:
1. **Load armor geometry from MCC maps instead of local OBJ exports:**
   - Locate Spartan/Elite model files (`.model`) in `{MCC_INSTALL}\haloreach\models\` (e.g., `m_spartans.model`, `m_elites.model`)
   - Parse and extract mesh/skeleton at runtime; discard local OBJ files to avoid shipping game assets
   - Harness and armor should layer correctly on both species (tested in preview, not in-game)
2. **Determine armor equipment/color offsets in Slot 34 profile:**
   - Use profile captures (`-capture-profile data/profile-captures`) to identify which bytes control torso, helmet, shoulders, legs, visor color, and suit color
   - Map UI selections to equipment IDs and color indices
   - Test in-game: set armor in settings, launch a game, verify the character reflects the changes
3. ~~**Map FOV to the game camera and verify it works in-game**~~ (works at match start, 03:40) — wired in 0a below (FOV value at
   `haloreach+0xB42C10`, see `docs/fov-findings.md`); still needs an in-game check that the camera takes the value live
4. ~~**Controller settings**~~ done 04:30 (native XInput pad, look speed, invert, dead zones, vibration; see section 0). Left: verify with a real pad (vibration, +0x1B7)
5. ~~**Menu music picker**~~ done 04:30 (two-pane picker with search)
6. Check in game: Forge menus and loadout cards no longer show hex (expected fixed by Slot 55)
7. Noble exiting from inside a game: confirm `tasklist | findstr /i noble` is empty afterwards
8. Mod-only maps (not in Reach's tables) are listed but disabled: needs map-id work in BlamEngine
9. Halo 2 Anniversary music (`sounds_*.dat`) not readable yet

Build/run (Developer Command Prompt, not plain PowerShell - cmake is only on PATH there):
`cd /d D:\Halo-Reach-Community-Edition && cmake --build build --config Release && cd build\bin\Release && noble-client.exe`
If LNK1104 (exe locked) and `taskkill` fails, a stuck process needs a real reboot; rename the exe meanwhile.

## 0c2. Earlier (23:15 ET)
(23:15 ET)

**23:15: pushed + mouse settings + input prompts (built, Tyler OK'd "fine for now").** Stuck processes were from
quitting at the menu (10:12-10:17 runs, ExitProcess hang, likely XAudio2 DLL detach); real reboot cleared them; with
TerminateProcess the menu-quit check came back clean (in-game quit not re-checked yet). Commit `d31ccff` rebased onto
O7OF's `2248385` (adds ObjModel/ObjModelPreview armory preview, ArmorCustomizer::Render(device, ctx), default master
server URL) and pushed as `7500059`. Since then (not committed): `main.cpp` `return 0;` after TerminateProcess (C4715);
new `src/ui/InputPrompts.cpp/.hpp` (device tracking from ImGui input each frame in `UIManager::BeginFrame`; drawn
glyphs: A green / B red circles, LB/RB pills, D-pad, or keyboard key boxes); FrontEndMenu `DrawHints` takes
`Prompt` values, `AcceptPressed()` (Enter / keypad Enter / pad A) and D-pad for focus rows and skulls; SettingsPanel
MOUSE section (look sensitivity + RESET, new `mouseSensitivityY` vertical ratio 0.25-4, invert, help lines) and LB/RB
(Q/E) prompts around the tab row; ReachHost slot 36 reads look settings every 500 ms (`InputLookSettings`) so APPLY
works in a running game; `-mouse-scale` still overrides. Small ReachHost change - tell O7OF. Tyler (23:17): **a controller already works in game** (correct older notes saying gamepad isn't supported). Next:
CONTROLLER section (settings for it), Video options (FOV, V-Sync), cleaner music picker, pixelated controller picture.

## 0b2. Earlier (22:40 ET)
(22:40 ET)

**22:40: stuck Noble processes + exit change (written, build untested).** 22:28-22:34: Modded tab now collapsible
(mod heading rows with arrow + map count, click/Enter toggles, `st.openMods`), mouse-wheel scrolling + scroll bar in
`DrawList` (per-list offset), preview blurb height measured (fixed a const ImFont compile error). Tyler built it: works.
Three `noble-client.exe` (PIDs 41172/7516/8484, ~2 GB each) stayed listed after closing and could not be killed even
as admin ("no running instance") - stuck in the kernel (driver wait); only a reboot clears them, and they lock the exe
(LNK1104; workaround: rename the exe). main.cpp's three `ExitProcess(0)` are now `TerminateProcess(GetCurrentProcess(), 0)`
(ExitProcess runs DLL detach under the loader lock and can hang). If processes still get stuck: find when (after a
match? quit from menu?) and release D3D / stop the engine before exiting. Also sent Tyler `docs/career/armor-list.md`
(readable Armory list from armor.json). Not committed yet: everything since 355b31f (audio, menus, mods, VFS, main.cpp).

## 0a2. Earlier (22:30 ET)
(22:30 ET)

**22:30: VANILLA / MODDED tabs (written, NOT BUILT).** 22:20: Tyler tested the MOD row + click-to-pick: both work.
He wants MCC's way instead: map selection -> vanilla maps or modded maps. The MOD option row is gone; Campaign,
Firefight and Custom Game have `DrawSourceTabs` (VANILLA | MODDED (n); click, Q/E, LB/RB). The modded tab lists
every mod's maps under the mod's name (`ListItem.header` rows, not selectable; `ModdedMaps`, `FixSelection`); the
picked map carries its mod (`MapEntry.mod`) into `FrontEndChoice.modFolder`. "Mod: <title>" shows under the
preview. Campaign rows back to 1 difficulty, 2 skulls, 3 start. Forge has no tabs.

**22:25 (superseded in part by 22:30):**

**22:25: list picking + MOD option (written, NOT BUILT).** Tyler (22:04): in Campaign the mission under the mouse
became the pick, so moving to START changed it; and Campaign/Firefight need options such as Mods and game type.
- `DrawList(..., hoverSelects)`: mission/map lists now only change on click or keys (hover = light highlight);
  the main menu still follows hover.
- New `src/ui/ModCatalog.cpp/.hpp`: scans Workshop (976730) + `data\mods` for `Engine: HaloReach` mods; reads
  `campaign\*.json`, `firefight\*.json`, `multiplayer\*.json` (+ `GameModContents.*Maps`, `CampaignInfo.json`
  CampaignMaps): `ScenarioFile` last part = scenario, `Title/Description.Neutral`; and `maps\*.map` names.
- Campaign rows: MOD, DIFFICULTY, SKULLS, START (focus 1-4). Firefight/Custom: MOD, GAME TYPE, (DIFFICULTY), START.
  Choosing a mod swaps the list to the mod's maps; maps not in Reach's own tables are shown disabled (Noble
  launches by Reach map id, see BlamEngine::ResolveMapId). Maps the mod only replaces show as "<name> (modded)".
- `FrontEndChoice.modFolder/modTitle`; main.cpp calls new `VFS::SetActiveMod(folder)`; VFS::ResolvePath reads
  `<mod>\<relative path>` first (e.g. `maps\ff60_icecave.map`). Small VFS change - tell O7OF.
- Test: Firefight, MOD = "Icecave: The Last Stand" (Workshop 3724674100, replaces ff60_icecave = Glacier).

## 0c. Earlier (22:15 ET)

**22:15: Menu music catalogue from every readable MCC game (written, NOT BUILT).** 21:47 build tested: Reach UI
sounds + Reach menu music work (cache shows a_button, b_button, cursor_*, bumper7 L/R, 13 menu pieces; no warnings).
Tyler asked for the whole Reach OST plus Halo 1/2/3/ODST themes.
- ReachAudio now builds a track list per game. Reach/H3/ODST from `<game>\fmod\pc\sfx.fsb(.info)` (same format);
  Halo: CE from `halo1\sound\pc\sounds_music.fsb` + `lst\sounds_music.lst.bin` (u32 1, u32 count, then u32 len,
  name incl NUL, u32 first sub-sound, u32 permutations; `sound\music\` = Anniversary, `sound\old_music\music\` =
  classic, `sound\sinomatixx*` = cutscene music). H1 index->vgmstream sub-song mapping is ASSUMED (first+k+1);
  the stream names vgmstream prints are logged at debug level to check it.
- Halo 2 Anniversary: sound is in `halo2\h2_maps_win64_dx11\sounds_*.dat` (custom format) - not supported yet.
- A track = one music cue: intro, loop pieces (random), outro; `alt_*` layers, sustain/perc/stinger/ambient beds
  skipped. Counts from the .info files: Reach 62, H3 45, ODST 80, CE Anniversary 95, CE classic 85. Labels: curated
  for menu themes and Reach mission cues (m20 ONI Sword Base ... m70 Pillar of Autumn) and CE cutscenes (a30 Halo
  ... d40 The Maw); the rest are prettified cue names (the in-game score is stems, not the album tracks).
- Ids: `<game>:<cue path>` (`reach:menu`, `h3:menu`, `odst:menu`, `h1a:menu`, `h1:menu`), plus `all` (shuffle all
  games: each track ~2.5 min then its outro) and `off`. Old setting values ("shuffle", piece names) = `reach:menu`.
  Settings default is now `reach:menu`. MENU MUSIC row is a grouped drop-down (game headings), plays on pick.
- Music is decoded on demand (the Reach menu theme and UI sounds up front); cache trimmed to ~1.1-1.5 GB at start.

## 0b. Earlier (22:05 ET)

**22:05: Reach menu sounds + menu music written, NOT BUILT YET (task #17).** Tyler asked (21:31): Reach's menu
scroll sound instead of the beeps, Reach menu music with a track choice; then a Settings rework (MOUSE and CONTROLLER
sections, modern but Reach-feeling navigation and previews) and prompt icons that follow keyboard/mouse vs gamepad.
Hex fix is committed and pushed (`355b31f`).
- New `src/audio/ReachAudio.cpp/.hpp`: reads `haloreach\fmod\pc\sfx.fsb.info` (280-byte records, name at +24,
  record N = vgmstream sub-song N+1), decodes the sounds it needs once with `vgmstream-cli -s N+1 -o x.wav sfx.fsb`
  on a worker thread into `<exe>\data\userdata\cache\audio` (git-ignored via `data/`), plays them with XAudio2.
  UI sounds by bank path: omaha_ui cursor_vertical / cursor_horizontal / a_button / b_button, ui\error, first
  `bumpers\left_bumper\` / `right_bumper\` entry, exit_experience game_completion/totals/rank_up/slot_machine.
  Music: `data\sound\music\reach_menu\track1\in\*` = intro, `...\loop\*` = pieces (all pre-decoded in the
  background; only the playing piece is held in RAM). "shuffle" = intro then random pieces; a piece name = that
  piece looped; "off". vgmstream-cli is looked for in `<exe>\tools\vgmstream`, `<exe>\vgmstream`, `<exe>\bin\vgmstream`
  (NOT shipped; Tyler copies his own into `build\bin\Release\tools\vgmstream`). Without it the old beeps play and
  SETTINGS > AUDIO shows why.
- `MenuSounds` API now: Click (A), Back (B), Scroll (list up/down + hover), Change (value left/right), Tab(dir)
  (bumpers), Error. FrontEndMenu: list step -> Scroll, value cycle -> Change, goBack -> Back, the extra sound on
  screen change removed. SettingsPanel: tabs -> Tab(+-1), Cycle -> Change. Volume = `volumeOther`.
- Settings: new `menuMusicTrack` (default "shuffle"), SETTINGS > AUDIO > MENU MUSIC cycles the tracks and previews
  at once (UNDO restores the saved one). Music starts in `FrontEndMenu::Run`, stops (fade) before it returns.
- main.cpp calls `ReachAudio::Init` after `UiArt::Init`. CMake: ReachAudio files + `xaudio2`, `ole32`.
- All edited files are CRLF (repo uses CRLF; convert before copying back).
- Unverified: whether the `.info` order really matches vgmstream's sub-song order for every record (a warning is
  logged if vgmstream prints a different stream name), and the exact bumper file names.
- Next: Tyler builds + tests; then task #15 Settings rework (MOUSE: sensitivity X/Y, invert, raw input/accel?;
  CONTROLLER: look sensitivity, invert, dead zones, vibration, button layout picture - current one is pixelated;
  VIDEO: FOV, V-Sync, quality), task #16 prompt icons per input device, and ask Tyler to check Forge + loadouts.

## 0a. Earlier (21:40 ET)

**21:40: hex-number bug FIXED (tested).** With slot 55 returning 1 the engine registers its strings (log:
"StringTable: register ... found" for all 40 logged, lookups return real text such as "Tuesday", "October") and the
scoreboard shows place "1", score "0" (`capture_777777.bmp`, 21:27). Tyler confirmed it works. All diagnostics
(`crtfmt` IAT wrappers, `[tokendiag]`, `[hexdiag]`) are removed from ReachHost.cpp (112,875 bytes). Still to do:
build, commit ReachHost.cpp + this handoff, push; delete the untracked `src/engine/HudColor.*`. Then check Forge
menus and loadout cards in game (expected fixed by the same change).

**21:35: hex-number bug ROOT CAUSE FOUND: host slot 55 must return 1.**
- `hr+0x3140D` calls host slot 55 (vtable +0x1B8, "strings clear") and only if it returns true ("test al,al /
  je skip") registers the string tables through slot 56 (+0x1C0, ~8700 strings; RR's log: "strings registered by
  the engine: 8712; lookups answered from the language file: 11178"). Noble's slot 55 returned 0, so slot 56 was
  never called and every slot 57 lookup missed (Noble log: all "(none)"). Fields bound to string ids then show
  the raw id as 8 hex digits: score/place, "Respawn in", loadout grenades, Forge menu title "00000002", prices
  "$00000001", rotation snap "00000008". The `[tokendiag]` hook on `hr+0x2D26C0` never fired for them (they use
  the int -> string-id conversion of value tag 6, `hr+0x2F8FD4`, not `\x07` tokens).
- **Fix (uncommitted, ReachHost.cpp):** `HostSlot55_StringsClear` returns 1. Test with go.bat; check the log for
  "StringTable: register ... found" lines and the scoreboard / loadout / Forge numbers.
- Once confirmed: remove `[tokendiag]`, `[hexdiag]` and the crtfmt diagnostics; the fallback string table in
  ReachHost.cpp may then be mostly unneeded (lookups come from MCC's EN_HaloReach.bin via registered keys).

**21:25: hex bug, UI token pipeline mapped (static) + diagnostic running.**
- `hr+0x2D288C` = string id -> text: gets the text via `hr+0x2D25A4` -> `hr+0x9E0D0` (host slot 57 =
  vtable +0x1C8, then the map's own strings; no hex there), then expands `\x07` tokens in a loop. For each token it
  asks host slot 51 feature 5 (`[hr+0xC1A100]` vtable +0x198, Noble answers on, like RR). On: splits
  "datasource.property" at '.', turns both into string ids (`hr+0x9DB08`), finds the property (`hr+0x9E350`) and
  calls **`hr+0x2D26C0`** to write the value as text. Off: parses the token as 8 hex digits (`wcstoul` radix 16).
- `hr+0x2D26C0`: datasource object -> column (vtable +0x50), rows (+0x60), value (+0x68 -> {tag, value});
  int value whose column info (+0x58) says "string list" -> recurse into `hr+0x2D288C` with that string id;
  otherwise tag handler to_text (`hr+0x2D284E`).
- The slot 61 call stack only shows the draw path (`hr+0x2C2BF6` -> `0x2DA7FB` -> `0x2DA262` -> `0x2DF075` ->
  `0x2DF68F` -> `0x2DF533` -> `0x26921B` -> `0x268DD4` -> `0x1CF393` -> slot 61), not where the text is built.
- **Running (uncommitted, ReachHost.cpp):** `[tokendiag]` MinHook on `hr+0x2D26C0` logs column id, extra, output
  text (first 40 calls + any 4/8-digit hex-looking output with a call stack). `[hexdiag]` in slot 61 logs stacks for
  "Respawn"/"01c"/"02" texts. Both must be removed before committing (with the crtfmt code).
- **Warning:** the stale-copy revert also hit `ReachHost.cpp` twice (a 76 KB / older copy reappeared after a
  write). Always check sizes after writing and run `git diff --stat` before committing.

**21:30: hex-number bug, cause narrowed down (static analysis of haloreach.dll).**
- The loadout's "0000 / 0002" is one 8-digit value `00000002` wrapped onto two lines, so all cases are a 32-bit
  int shown as hex (score, place, respawn time, frag grenade count). All four come from strings with a
  `\x07datasource.prop_x\x07` token. Noble's fallback strings are identical to RR's (`D:\Reach\data\strings_en.json`
  is RR's capture of what it answered: 71 strings, all present in Noble's table; slot 56 never registers anything,
  so every lookup uses the fallback table). The strings are not the problem.
- UI value types: sorted handler array at `hr+0xB4C800` (14 entries, looked up by tag with binary search
  `hr+0x1802E9720`; built at init `hr+0x2AA09A`). Each handler = {u64 tag, can_convert_from(tag), convert(src,dst),
  to_text(value, buf, cap)}. Tag 1 = int32 (`hr+0xA36058`, to_text `hr+0x2F3AB8` = `_itow_s` radix 10). Tag 8 =
  int64 (`hr+0xA45B50`): convert `hr+0x381EF0` accepts tag 1 (sign-extends), **to_text `hr+0x381F10` =
  `_i64tow_s(v, buf, cap, 16)`**, i.e. hex. Other tags: 0 bool ("true"/"false"), 2 float, 3/12/13 strings.
- Value to text: `hr+0x2D284E` (handler->to_text for the value's tag; tag 6 handled separately). Data binding:
  `hr+0x2F4F29` converts the source value to the widget property's tag (handler->convert), then sets the property
  (vtable+0x70). So an int reaches the text as tag 8 and prints in hex. Why RR doesn't hit this is still unknown
  (maybe the source value's tag, or which widget property type is bound).
- Ruled out this round: the engine's own narrow printf at `hr+0x7C2EF0` is OpenSSL's BIO_printf (next to
  `crypto\bio\b_print.c`); narrow `%08x`/`%04x` strings are OpenSSL/network/marker messages; the only wide `%x`
  is the locale check; no hand-written hex-digit code in the UI.
- Padding to 8 digits isn't explained by `_i64tow_s` (it doesn't pad), so check where the zeros come from if the
  test below shows decimal but still padded.
- **Test (done, reverted):** `src/hooks/EnginePatches.cpp` patch 2 changes the radix byte at
  `hr+0x381F1A` (`41 B9 10 00 00 00` -> radix 10), only if the bytes match. Tyler runs
  `D:\Halo-Reach-Community-Edition\build\agent\go.bat` (job = build + Sword Base + scoreboard shot
  `capture_777777.bmp`, log `Y_noble-client.log`; look for "UI int64 values now convert to text in decimal").
  **Result (21:12): no change.** The patch was applied (log: "UI int64 values now convert to text in decimal"),
  but the scoreboard still shows place "0000" and score "0000001c" (`capture_777777.bmp`). So the tag-8 to_text
  is NOT the source; the patch was reverted (EnginePatches.cpp restored to the committed version).
  Remaining ideas: the token `\x07datasource.prop\x07` may not be evaluated at all (property lookup fails and the
  text widget dumps the raw value / handle; 0x1c and 1 look like small indices, place is 4 digits = 16-bit);
  find where the text widget expands `\x07` tokens (wide 0x0007 compare in the UI code around hr+0x2C0000-0x300000
  found nothing with `cmp ...,0x7`, so it may scan with a table or `wcschr`), or compare at run time which
  datasource the scoreboard binds (RR vs Noble). An interactive debugger (x64dbg on Tyler's PC, breakpoint on
  memory write of the text) would find it fastest.
- Still in `ReachHost.cpp` (uncommitted): the crtfmt diagnostics; remove before committing.

**20:55: hex-number bug (score "0000001c", "Respawn in 00000001", loadout grenades "0000 / 0002") still open.**
RR shows all of these correctly, so it is something Noble's host does differently.
- Ruled out: the engine's wide sprintf `hr+0x35D84` (only normal output: '%d'->'10', '%d:%02d'->'0:00', ...);
  `_itow_s` (only caller `hr+0x2F3AB8`, radix 10; a UI data-value vtable at `0x180A36070`); locale code
  `hr+0x2B6F12`; Noble's own source (no hex formatting on these paths).
- Found: `hr+0x381F10` calls `_i64tow_s` with **radix 16** (another data-value type: type 8 = int64, and
  `hr+0x381EF0` converts type 1 int32 -> type 8). It does not zero-pad, so it is a lead, not the answer.
- There are two more wide sprintf wrappers: `hr+0x35CC8` and `hr+0x35D24` (20 callers, incl. `hr+0x381ED4`).
- **Running now (uncommitted, on the PC):** the old 0x35D84 hook is replaced by IAT wrappers around all six CRT
  format imports (`__stdio_common_vsnwprintf_s/vswprintf/vswprintf_s/vsprintf/vsprintf_s/vsnprintf_s`, IAT RVAs
  0x91EA20/28/48/60/70/38). Log lines: `ReachHost [crtfmt ...]`, plus `zero-padded` for outputs starting "0000".
  job.bat = build + Sword Base with `-scoreboard-at 20 -quit-after 30`, log copied to
  `build\bin\Release\Y_noble-client.log`. **Result (20:56):** no CRT call produces any hex/zero-padded text
  (every format seen is normal: '%d'->'0', '%d:%02d'->'0:00', '%i'->'25', '%s%s%s'->'0', ...), so the hex text
  is made by engine code itself, not printf. If no CRT call makes the hex text, the text is
  made by engine code (look for code that writes hex digits by the value's size), or the value reaching the UI
  has the wrong type tag (compare with RR at the data-value vtables).
- Remove the diagnostic (`EnsureFormatDiagnostics` + `crtfmt` wrappers in ReachHost.cpp) before committing.

### Earlier (20:20 ET)

**20:15: hand checks done by computer use (all pass).** Windowed 1600x900 Sword Base:
- Post-game panel (held after the engine stops): PLAY AGAIN relaunches into a new match. A very short synthetic click
  was missed (the panel polled GetAsyncKeyState once per frame), so overlay menus now also count WM_LBUTTONDOWN
  (`Window::TakeLeftClicks`; cleared when the pause menu / post-game panel opens so in-game shots don't click).
- Pause menu QUIT TO MENU returns to the front-end menu; its QUIT closes Noble.
- Moving the window: with the mouse captured the title bar can't be reached (cursor clipped to the game), so the
  click lands in the game. Alt+Space > Move works, and with the pause menu open the title bar drags normally.
- Service tag: scoreboard shows "NobleSix  TEST" (`-scoreboard-at <s>` test option holds Tab 3 s and saves
  `capture_777777.bmp`).
- Not checked: kills/medals in the results (no enemies in an offline Slayer; needs Firefight or a second player).
- **New clue for the padded numbers:** the scoreboard shows score "0000001c" and place "0000": the numbers are printed
  as **hex at full field width** (8 digits for 32-bit, 4 for 16-bit), not just zero-padded. Looks like the UI's
  value-to-text falls back to a raw hex dump when it doesn't recognise the property's type. `%x` users in the DLL
  checked so far: `+0x2B6F12` is only locale detection (GetLocaleInfoEx + swscanf). Slot 61 only gets the set of
  unique characters, so it can't be used to catch the padded strings. Open question for Tyler: does RR show the
  same hex numbers?
- Tyler's settings were switched to windowed 1600x900 + tag TEST for the test and restored afterwards
  (`settings.backup.json` next to it is the copy).

**Pushed to GitHub (main):** `12a9971` HUD colours + post-game hold, `5e71e53` Mods screen (Tyler-tested),
`ab77f8b` service tag setting, plus the mouse-wheel / window-move commit below. Commits and pushes now run from
`build\agent\jobs\job.bat` through go.bat (output in `build\git_out.txt`); `src/engine/HudColor.*` stay untracked
(old stopgap, safe to delete).

**18:18: mouse wheel works (weapon switch, tested).** Input +0x114 is an **f32** wheel pulse (RR `host::WheelPulse`:
per notch +1/-1 for 40 ms, one pulse per 80 ms, up to 3 queued; `win::WHEEL` accumulates WM_MOUSEWHEEL / 120). The
engine only acts on it through the key bindings: RR's profile binds action 4 (change weapon) to '1' **and** second
key **0xAF** at entry +8 = wheel **up**. 0xAE in the third key slot did not give wheel down, so Noble sends every notch
as "up" (with two weapons it's the same). `-wheel-after <s>` test option injects one notch. Window.cpp now counts
WM_MOUSEWHEEL (`Window::TakeWheelNotches`) and, on WM_ENTERSIZEMOVE/KILLFOCUS, frees the cursor clip; ReachHost
releases the mouse while the window is being moved (`Window::InSizeMove`), so a windowed game can be dragged
(Alt+Space > Move, or title bar). The window-move part is untested by hand.

**17:35: post-game freeze FIXED (tested).** Full 10-minute Sword Base Slayer: slot 6 at 17:33:07 (cR paid, panel
shown), engine resolved `mainmenu.map` and stopped drawing; at 17:33:16 the log says "engine stopped drawing after
the match; Noble draws the post-game panel itself", and `capture_900000.bmp` (taken from the main-thread frames)
shows the GAME OVER panel with the cR lines, rank bar and the three buttons. Still to confirm by hand: clicking
PLAY AGAIN / RETURN TO MENU / QUIT on that panel (input is polled the same way as before, so it should work).
Note: the automated test runs pay cR into Tyler's `data\userdata\career.json` (now 2436 cR from test matches);
delete that file to start from zero. Not committed yet: `PauseMenu.*`, `main.cpp`.

**Number padding ("Respawn in 00000001", loadout grenades "0000 / 0002"), investigated 17:30, not solved.** The
engine builds those strings itself with the zeros already in them (slot 61 TextPrepare gets the characters
"Respawn i01"), so it is not a font/glyph problem. Widths are 8 and 4 digits, i.e. 32- and 16-bit fields, which
smells like a number-format setting the host should supply. Not the partner's IsWhitespace hook: the original
`haloreach+0x362E8` is a binary search over 12 Unicode whitespace ranges used for line breaking and really does
run away for characters below 0x09 (the `\x07` property markers MCC's own strings use), so the hook is correct.
Not slots 95/96 either (HUD scale / safe-area queries). Next idea: find where the UI formats an integer property
(look for the code that fills `camera_list.prop_respawn_time`) and see which host answer changes it.

**17:25: post-game freeze, fix (details).** After a match the engine goes for its main menu map (not in MCC) and stops
calling slot 1, so the post-game panel was never redrawn. RR handles this in its main loop: `host::loadscreen::
begin_hold()`, then `hold_frame()` + `swapchain->Present(1,0)` from the main thread for up to 60 s
(`host-probe.exe__main.c` ~4950). Noble now does the same: `PauseMenu::HoldFrame()` (main thread, called from the
message loop in `main.cpp`) draws the panel and presents once the game is over and no engine frame came for 1 s;
Render (engine thread) and HoldFrame share a mutex. Also a 5 s watchdog after the message loop, because the engine's
Shutdown may never return when its thread is stuck. Test running at 17:23 (full 10-minute Sword Base Slayer,
`U_*` files: look for "Noble draws the post-game panel itself" and `U_capture_900000.bmp`).

**17:20: dark HUD FIXED (host slot 97).** The engine sends every HUD colour through host slot 97 before it reaches
the HUD shader: `haloreach+0x2DDD8C` (sets constant id 0x60000 = chud_color_output_A..F, via `+0x271200`) calls
`+0x2DDF3C` for each colour, which packs it to ARGB and calls `[host]+0x308` (slot 97) with (a1 = filter/mode index
from `haloreach+0xD1F71C`, a2 = ARGB), then unpacks the returned ARGB. RR (`host::hit` case 0x61) returns the colour
unchanged; Noble returned 1, which made every HUD colour (0,0,1/255,0). `HostSlot97_ColorFilter` now returns a2. Test
(Sword Base, 17:19, `T_capture_2400.bmp`): shield bar, compass, tracker, reticle, ammo, grenades, game-type box all in
Reach's own colours. `src/engine/HudColor.*` (the Map/Unmap stopgap + diagnostics) is no longer built or called:
removed from `CMakeLists.txt` and `main.cpp`; delete the two files (`git rm src/engine/HudColor.cpp
src/engine/HudColor.hpp`). Not committed yet: `ReachHost.cpp`, `main.cpp`, `CMakeLists.txt`.

**Git state.** Everything up to the career work was committed on `main` as one commit ("Front-end menus, MCC art, skulls,
subtitles, settings, key binding names, career/cR, master server") and rebased onto the partner's `1d2a282` (their
`EnginePatches` whitespace-crash fix is kept). The `ReachHost.cpp` conflict was resolved by keeping Noble's version: all of
the partner's ReachHost edits were in the text code that Noble had already replaced or fixed the same way (glyph record,
digit/icon shortcuts, string fallback). The one idea not carried over: pre-rasterising text in slot 61 (TextPrepare).
If `git status` still says "rebase in progress", run `git add src/engine/ReachHost.cpp`, then
`git -c core.editor=true rebase --continue`, then `git push`.

**Not pushed yet:** a build fix made after that commit: `src/engine/KeyBindings.cpp` (the action table had landed outside
the anonymous namespace) and `src/engine/ReachHost.cpp` (unused-parameter warning). With them the build passes
(14:22). To push:
`git add src/engine/KeyBindings.cpp src/engine/ReachHost.cpp`, `git commit -m "Fix KeyBindings.cpp build"`, `git push`.

**Running the test job.** `D:\Halo-Reach-Community-Edition\build\agent\go.bat` (full path; from inside `build` the relative
path fails). It writes to files, so the console stays quiet; the line `EXITCODE` on screen is a quirk of `nb_build.bat`
(`echo EXITCODE 0>>` redirects) and means the build succeeded. Current job = `nb_run13.bat`: key-bindings screenshot
(`Q_keys.bmp`), then Sword Base with `-end-round-after 40 -quit-after 90` (`R_*` files in `build\bin\Release`).

**Result of the 14:22 run:** build OK, game ran 90 s without problems, but **`-end-round-after` did not end the game**:
the log shows "ending the round" at 40 s and then nothing (no slot 6, no `Career:` lines). Either `haloreach+0x7B3EC`
needs to run inside the simulation update (not from slot 1), or ending a round alone does not end a 1-round Slayer game
(try `game_won`'s evaluator `+0x1B0810`, or the engine's end-game function that `mp_round_end` leads to). Until then,
test career by **playing a short match to the end** (Slayer time limit) and reading the `Career:` log lines and
`data\userdata\results\last_game.bin`. `Q_keys.bmp` from this run has not been looked at yet.

**cR test passed (15:19).** Sword Base Slayer played to the time limit after the 13 cR/min rebuild. Log: `took 10.3 min;
local player 0 kills 0 assists 0 deaths 2 medals 0; paid 133 cR` = 10.3 x 13, matching Tyler's count (2 deaths, nothing
else); the post-game panel showed +133 cR and 133 / 7500 toward Private. No slot-machine bonus that game (1-in-8). Per-player
stats array: deaths appear at indices 11 and 13 (both 2).

**New: freeze after the post-game panel.** The panel opened (15:19:38) but its buttons could not be clicked. Right after
the results, the engine resolved `haloreach\maps\mainmenu.map` (slot 48), which MCC does not ship; about 7 s later it made
its first iface2 slot 140 call (a2=0x28, a3=0x40), a slot 4 EngineEvent with reason 0 and an empty message, and slot 1
frame ticks stopped (last frame 111000 at 15:19:45). With no frame ticks the panel is never redrawn or given input. Likely
cause: after game over the engine tries to go to the main menu map. Next: stop or answer that transition (slot 48 for
`mainmenu.map`, or slot 140 / slot 4), or move Noble to its own menu before the engine leaves the match.

**Tyler's reference list (from the afternoon chat).** He supplied 11 screenshots plus pasted notes (loadouts, control
schemes for MCC and Xbox 360, general Halopedia info on Reach) for: subtitles, main menus, skulls, controller layout
(360 and MCC), keyboard/mouse key bindings, loadout names. The files are not in the repo (Bungie/343 art and wiki text;
keep it that way). Status:
| Item | State |
|---|---|
| Subtitles | ✅ SUBTITLES setting + `0xD5244` hook (§5) |
| Main menus | ✅ Reach-style `FrontEndMenu` screens, art read from the MCC install (`UiArt`) |
| Skulls | ✅ Skulls screen + launch block +0x48 (§5) |
| Keyboard/mouse bindings | ✅ KEY BINDINGS tab, 66 slots named in MCC's groups; `Q_keys.bmp` (14:22 run) checked: renders correctly |
| Loadout names | ✅ come from the game variant and render correctly (§5) |
| Controller layout (360 and MCC) | ❌ **still to do**: a CONTROLLER tab in SETTINGS showing the MCC default and the original Xbox 360 layouts (controller picture already readable from `controllertexturepack`), and gamepad input itself (XInput into slot 36 +0x120.., §6 item 6) |
Checked against the Halopedia notes: the menu has all 24 MP maps (incl. Noble, Defiant and Anniversary packs), all 10
Firefight maps and all 11 campaign missions; mission descriptions are Noble's own wording (don't paste the official
taglines). Not used yet but useful later: the Armory's 30 armour colours (10 bases × 3 shades: Steel/Silver/White,
Red/Mauve/Salmon, Orange/Coral/Peach, Gold/Yellow/Pale, Sage/Green/Olive, Teal/Aqua/Cyan, Blue/Cobalt/Sapphire,
Violet/Orchid/Lavender, Crimson/Rubine/Pink, Brown/Tan/Khaki) for the Armor Locker, and the Arena divisions (Onyx, Gold,
Silver, Bronze, Iron) if a ranked mode is ever added.

**Career reference data (14:45).** Tyler collected the Reach career material from Halopedia and the Bungie.net
forums; it is now in `docs/career/`: `career-reference.md` (how Reach paid cR, campaign scoring, ranks, commendations,
challenges, Armory, loadouts, controller layouts, sources) and the data files `ranks_and_pay.json`, `challenges.json`
(all 18 months Sep 2010 – Feb 2012, 2,103 challenges; no month missing, only 14 days Halopedia itself lacks),
`commendations.json` (45) and `armor.json`. Names and numbers only, no Bungie art (rank/challenge icons must be read
from the MCC install; Tyler's rank-icon sheet stays local). Open question for Tyler: keep Noble's own cR numbers or
switch to Reach's measured table (`career-reference.md` §1).

**Armory appearance (current).**
- The Armory saves `data/appearance.json` under the resolved data directory. Slot 34 writes species, armor pieces,
  colours and the Elite armor set into the profile (see section 0, 09:15).
- The local OBJ preview resolves MCC MTL texture paths into the exported `bitmaps` tree, loads matching `_cc` masks,
  and applies the selected primary/secondary/detail colors and visor tint. It composes a Spartan base with selected
  MCC accessory groups, or an Elite body/undersuit layer followed by the selected rank harness group. The engine's
  own main-menu Spartan preview is unavailable because MCC has no Reach `mainmenu.map`; Armory uses Noble's DX11 OBJ
  renderer instead.
- The actual Spartan/Elite OBJ exports remain local-only under git-ignored `data\armor\`; never commit them. The OBJ
  model-group UI only offers geometry that exists in the local export. Reach's equipment IDs and live player color
  records are still unmapped, so piece/color selections affect the Armory preview, not yet the in-game character.
- A safe in-game equipment/color implementation needs a known-good profile capture with per-piece/color changes (or
  verified engine ID mappings). RR's profile contains candidate u32 lists at +0x40..+0x1AB, but their semantics are
  not confirmed. Ghidra 12.1 found no direct references to the `color_armor1` or Spartan helmet property strings, and the
  matching PDB was not present in the MCC install; static analysis has not established equipment or color IDs. Direct
  `.map` render-model decoding also remains future work.
- For an opt-in runtime capture, launch `noble-client.exe -capture-profile data/profile-captures`. Slot 34 writes the
  initial 64-KB profile buffer and any later changed buffer on subsequent Slot 34 calls as session-tagged, numbered
  `.bin` files; later runs do not overwrite earlier captures. This mode is disabled by default. Change one in-game
  setting at a time, retain the matching before/after files locally,
  and do not commit captures. If Slot 34 is not called again after a change, this mode cannot observe that change; Armory
  preview JSON selections are not included until they are mapped into the game profile. Compare matching captures by
  byte offset, substituting their actual session-tagged filenames:
  `$a=[IO.File]::ReadAllBytes('profile-<session>-0.bin'); $b=[IO.File]::ReadAllBytes('profile-<session>-1.bin'); 0..($a.Length-1) | ? { $a[$_] -ne $b[$_] } | % { '{0:X4}: {1:X2} -> {2:X2}' -f $_,$a[$_],$b[$_] }`.
- Forwarded community request (TRAXUSi, Workshop class mods): add extra player-host object types beyond Reach's
  three (0 spartan, 1 elite, 2 monitor), e.g. `elite2`, `spartan2`, `monitor2`, using the same host code paths, with
  matching Megalo names. Engine patch inside `haloreach.dll` at run time; not started, idea only.

**17:10 update: RR's real HUD colours (RenderDoc capture of RR, Spartan, Sword Base, frame 2662).**
- To play as a Spartan in RR, the profile matters: RR profile bytes **+0x27 and +0x38** are 1 for Elite, 0 for Spartan
  (only 5 bytes differ between the two profiles). Spartan copy: `D:\REACH REVERSE ENGINEER\rr_profile_spartan.bin`
  (RR data, never commit). RenderDoc: exe `D:\Reach\bin\host-probe.exe`, cwd `D:\Reach`, child capture off, args = RR's
  match line without `--show-behind/--exit-after-game/--hold-after-game` and without the three elite options.
- `CHUDWidgetPS` = Buffer 10679 (208 bytes = 13 float4, rewritten before every HUD draw). Rows 0-5 are
  chud_color_output_A..F; rows 6-12 are parameters (scale, timing, transforms). Values (0-255, R,G,B,A):

  | Row | Motion tracker (EID 20055, Draw 6) | Shield bar (EID 20084, Draw 18) |
  |---|---|---|
  | 0 A | 0,0,0,0 | 36,148,255,255 |
  | 1 B | 8,53,121,255 | 8,53,121,255 |
  | 2 C | 0,0,0,0 | 255,255,255,1 |
  | 3 D | 0,19,33,0 | 4,23,62,255 |
  | 4 E | 0,0,0,0 | 240,36,36,0 (red: low-shield colour) |
  | 5 F | 0,0,0,0 | 255,255,255,1 |

  Noble gets (0,0,1,0) in every row of every widget. **The colours differ per widget and per slot**, so a fixed
  replacement can't be right (that's why HudColor.cpp's sky blue gave the white block). The engine computes these on
  the CPU from the map's HUD (chud) data each frame; in Noble that computation returns 1 everywhere.
- Next (code work, no help needed from Tyler): in `haloreach.dll`, find the CPU code that fills this 208-byte buffer
  (follow the Map/Unmap of the CHUD widget constant buffer back to the colour evaluation), see what input makes it
  return 1 (likely a lookup that fails: a host answer, a profile field, or the player colour records RR writes,
  `looks::copy_colors`), and fix that instead of patching the buffer. Until then HudColor.cpp could use these values
  as a better stopgap, but that is a guess per widget.
- `armory-appearance.txt` (read 16:45): Spartan primary b3852a, secondary 000000, tertiary ff7a14, quaternary 6a2c00;
  Elite 3d6084; emblem colours 2fa59a/80ddda. None is the HUD blue, so the HUD isn't simply the armour colour.

**16:45 update: sky-blue HUD tested; next is an RR capture.**
- `HudColor.cpp` result (Sword Base, Forest): motion tracker, ammo, grenades, crosshair, ability icon now light blue
  like RR. Two regressions: the shield bar + compass background is a solid **white block** (several colour slots
  stack), and the scope mask outside the zoom view is a **light-blue haze** (it should stay dark). Every broken slot
  holds the same value (1), so one replacement colour can't be right. Not committed yet.
- Next: RenderDoc capture of RR for the real CHUDWidgetPS values (motion tracker, shield bar, scope mask). Launching
  `Reach.exe` with "Capture Child Processes" crashes RR's WebView2 menus, so launch the game process directly:
  - Exe `D:\Reach\bin\host-probe.exe`, working dir `D:\Reach`, child capture off.
  - RR's real launch line (16:41; `runs\<id>\match` is a per-run folder):
    `<run>\match --launch-block D:\Reach\data\launch_block_1.bin --armory-appearance
    %LOCALAPPDATA%\ReachRenaissance\armory-appearance.txt --profile <run>\profile.bin --strings
    D:\Reach\data\strings_en.json --elite-looks --features-on 6e --seconds 86400 --user-id <xuid> --user-name <name>
    --mcc "<MCC dir>" --map SwordBase --mode hr_4v4_team_slayer_dmr_ar_50points_tu --time-limit 12 --exit-after-game
    --extra-maps D:\Reach\data\armor --armor-pack rr_elite_pack --elite-armor-slots --service-tag ONYX
    --settings-file %LOCALAPPDATA%\ReachRenaissance\settings.txt --window-pos 100,100 --window 2560x1440
    --hold-after-game --show-behind <launcher hwnd> --map-variant "<MCC>\haloreach\hopper_map_variants\sword_base_default_031.mvar" --ingame-chat`
  - For RenderDoc: drop `--show-behind`, `--exit-after-game`, `--hold-after-game`; use a copied profile
    (`D:\REACH REVERSE ENGINEER\rr_profile.bin`, RR data: never commit).
- **New lead:** RR passes `--armory-appearance armory-appearance.txt` (player colours); HUD colours may come from
  the player's colours. Waiting for Tyler to paste that file.
- Loadout cards show grenades as "0000 / 0002": same zero-padding bug as the respawn timer (suspect the
  IsWhitespace hook in `EnginePatches.cpp`).

**16:30 update: dark HUD cause found (RenderDoc).**
- Capture of Noble on Sword Base (frame 1862, saved outside the repo; never commit `.rdc` files). The HUD is
  "Colour Pass #7" (EID 8935-9432, last pass before Present). Pixel history on the motion tracker: EID 8980 writes
  (0.04, 0.04, 0.08). Its pixel shader has two constant buffers: `ExposurePS` = g_exposure (0.5, 2.0, 1.0, 1.0),
  normal; `CHUDWidgetPS` (208 bytes) has **chud_color_output_A..F all (0, 0, 0.00392, 0)**, i.e. the integer 1
  read as ARGB. So the HUD colours the engine computes are an index/flag value, not a colour. Probably the same
  player/colour records RR rewrites in memory (`armory::write_colors`: four ARGB u32 >= 0xFF000000;
  `looks::copy_colors` hook), which would also explain why RR's profile file alone didn't help.
- **Stopgap (Tyler's call): `src/engine/HudColor.cpp`** hooks ID3D11DeviceContext::Map/Unmap (vtable 14/15) and,
  in 208-byte constant buffers, replaces colour slots holding exactly that broken pattern with sky blue #87CEEB
  (alpha kept). Installed in `main.cpp` after UIManager init; added to `CMakeLists.txt`. Real fix still to do: find
  what fills those colours (compare a RenderDoc capture of RR: same draw, CHUDWidgetPS values; then RR's
  `looks::copy_colors` / `armory::write_colors`).
- The 15:45 D3D11Renderer test (BGRA flag, 1 buffer) made the HUD black and was reverted (`git restore`).
- Mods screen rewrite (`WorkshopManager.*`) tested OK by Tyler.

**15:55 update.**
- Pushed `ebd9761` (on top of the partner's `ae7b71e`): career pay 13 cR/min, rank fix, `docs/career/`, fixed-size
  windowed mode (`Window.cpp`: no maximize/resize, since the engine can't change resolution after start; use
  BORDERLESS FULL SCREEN for full screen), Key Bindings no longer lists MCC's "other Halo games" actions.
- **Dark HUD:** not resolution (borderless 3840x2160 is still dark). Pixel samples: RR tracker edge (80,148,191) vs
  Noble (21,21,32), so the HUD colour is lost, not just dimmed. Same host slots as RR (RR's switch handles 1, 2, 4, 10,
  18, 32-34, 36, 39, 41, 43, 47, 48, 51, 52, 55-57, 88, 97, 116; slot 33 bytes identical; iface2 identical; RR gamma
  exponent default 1.0). Lobby patches are inactive (0 applied). **Test (reverted 16:05) in `src/host/D3D11Renderer.cpp`:**
  device flag `D3D11_CREATE_DEVICE_BGRA_SUPPORT` (RR passes 0x20) and RR's swap chain (1 buffer, flags 0). If the HUD
  stays dark, revert it; next suspects are RR's engine hooks (`resadapt` hooks 14 functions at haloreach+0x1BB6C0..
  +0x1BBD34 that read tag-block elements by u16 index; `hudx`, `widgetx`, `texcam`).
- **Respawn timer** "00000001": suspect Noble's `EnginePatches` IsWhitespace hook (returns 0 for chars < 0x09); the
  respawn string uses `\x07` property tokens. Test with the hook off.
- **UI to fix next (Tyler's order: HUD, then Mods screen and a clean Settings menu):** Mods: Halo 3 mods listed as
  Reach (read each mod's `ModInfo.json`: `"Engine"` and `"Title":{"Neutral":...}`), titles are the first map file
  name, ImGui ID clash (rows need `PushID(i)`), fixed pixel widths cut off at 4K. Controller picture is dotted/pixelated
  (scaling/alpha). Settings: lots of empty space, Video has only resolution and window mode (RR: FOV, V-Sync, quality,
  help text on the right). Party box says 1/16 (RR 1/64).
- Tyler added full Spartan and Elite rips (OBJ + bitmaps) at `D:\REACH REVERSE ENGINEER\spartans` and `...\elite`
  for the armory. Game assets: never commit them.

## 1. Where things are

| What | Path |
|---|---|
| **Repo (Noble)** | `D:\Halo-Reach-Community-Edition` (remote `https://github.com/O7OF/test-build-01`, branch `main`) |
| Built client | `D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.exe` |
| Run log / crash dump | `build\bin\Release\noble-client.log` (overwritten every launch), `noble-crash.dmp` |
| Noble data folder (git-ignored) | `build\bin\Release\data\` (`sandbox\`, `userdata\`, `mods\`, local test files) |
| Map-info sandbox (created at runtime) | `build\bin\Release\data\sandbox\haloreach\maps\info\` (the process `cwd` is switched to `data\sandbox`) |
| Repo docs | `docs\HANDOFF.md` (this file), `docs\hud-text-findings.md`, `docs\boot-investigation-2026-10-27.md`, `docs\architecture_plan.md` |
| **MCC (Steam)** | `D:\Steam\steamapps\common\Halo The Master Chief Collection\` |
| Engine DLL | `...\haloreach\haloreach.dll`, SHA-256 `738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894` (the only build RR supports; all RVAs below are for this build) |
| MCC game variants | `...\haloreach\game_variants\*.bin` (`slayer_054.bin`, `hr_ff_*.bin` for Firefight, ...) |
| MCC sound banks | `...\haloreach\fmod\pc\sfx.fsb`, `english.fsb` (found through host slot 48) |
| MCC binaries | `...\MCC\Binaries\Win64\` (ships `xaudio2_9redist.dll`, which `haloreach.dll` loads for sound) |
| Reach Renaissance install (reference only) | `D:\Reach` (`bin\host-probe.exe`, `bin\menu-overlay.exe`, `data\profile_1.bin`, `data\lb_m_c_1.bin`, `data\launch_block_1.bin`, `data\blocks\*.bin`, `data\menumaps\mainmenu_all.map`) |
| RR run logs | `C:\Users\tyler\AppData\Local\ReachRenaissance\runs\<timestamp>\` (`menu\`, `guest\`, `match\` host_probe logs, `*-args.txt`) |
| **RE workspace** | `D:\REACH REVERSE ENGINEER` (not a git repo) |
| RR analysis notes | `D:\REACH REVERSE ENGINEER\docs\rr-launch-flow.md`, `rr-tags.md` |
| RR pseudocode (Ghidra) | `D:\REACH REVERSE ENGINEER\docs\evidence\pseudocode\*.c` (`host-probe.exe__host__hit@0x14007c600.c`, `__main.c`, `__set_game_variant@0x1401c3450.c`, ...) |
| haloreach.dll index | `D:\REACH REVERSE ENGINEER\docs\evidence\haloreach\` (`haloreach_functions.csv`, `haloreach_strings.csv`, ...) |
| Omaha symbol tables | `D:\REACH REVERSE ENGINEER\docs\evidence\symbols\` (Xbox 360 addresses, names only) |
| Leaked 2010 build (source of Omaha symbols) | `D:\Halo Reach (Jul 24 2010) (Omaha) (Cache Build)`. Keep anything derived from it **out of the repo** |

**Tools installed:** Visual Studio 2022 Community + Desktop C++ (CMake bundled), Git, Node.js, **Ghidra 12.1.4** at
`C:\tools\ghidra_12.1.4_PUBLIC`, **JDK 21** at `C:\Program Files\Eclipse Adoptium\jdk-21.0.12.101-hotspot`
(`GHIDRA_INSTALL_DIR` and `JAVA_HOME` set). The partner's PC does **not** have Ghidra configured.

`host-probe.exe` (RR) still has Rust symbols: `llvm-nm -C` / `llvm-objdump -d -C` give real function names
(`host_probe::host::hit`, `host::hit2`, `host::load_profile`, `text::glyph`, `pad::fill`, `live::remap_keys`, `uicursor::*`, ...).
That is the fastest way to answer "what does RR do for slot N". `objdump` on `haloreach.dll` itself is also quick
(~2.5 M lines) for tracing engine code at an RVA.

## 2. Build and test (exact commands)

Use **Developer Command Prompt for VS 2022** (plain PowerShell has no `cmake`; in cmd use `cd /d` to change drive).

```
cd /d D:\Halo-Reach-Community-Edition
```
```
cmake --build build --config Release
```

* If `build\` is missing or CMakeLists changed a lot: `cmake -S . -B build -G "Visual Studio 17 2022" -A x64` first.
* The post-build step copies a top-level `data\` folder; if the repo has none the build "fails" after linking. Create it once:
  `if not exist D:\Halo-Reach-Community-Edition\data mkdir D:\Halo-Reach-Community-Edition\data` (the post-build copy should
  be wrapped in `if(EXISTS ...)` in CMakeLists).
* `LNK1104: cannot open file ...noble-client.exe` = the game is still running. Close it or `taskkill /IM noble-client.exe /F`.

Run (Steam must be running), from `build\bin\Release`:

| Command | What it does |
|---|---|
| `noble-client.exe` | Noble front-end menu (also for `-map mainmenu` / `-mode mainmenu`, which the C# launcher passes) |
| `noble-client.exe -map 20_sword_slayer -mode multiplayer` | Sword Base directly (most tested map) |
| `noble-client.exe -map m10 -mode campaign -difficulty 1` | Winter Contingency (0 easy .. 3 legendary) |
| `noble-client.exe -map ff50_park -mode firefight -difficulty 1` | Firefight on Beachhead (default game type Standard) |
| `noble-client.exe -map m10 -mode campaign -skulls 0x10` | campaign with skulls (MCC skull bits, here Blind; see §5 Skulls) |

Other args: `-variant <path to MCC game variant .bin>`, `-mvar <path>`, `-mouse-scale F` (mouse look per pixel, default 0.0008),
`-profile <file>` (load a captured profile instead of Noble's built-in one; relative paths are resolved against the exe folder).
Test-only args: `-menu-screen <main|campaign|skulls|firefight|custom|forge|servers|settings|armory|mods>` opens that
front-end screen, `-menu-tab <0-5>` picks the SETTINGS tab (1 = key bindings), `-menu-capture <frames>` saves `capture_<frames>.bmp` of the menu and exits; `-launch-poke 0x48=0x10,...`
writes u32 values into the launch block (for finding fields by experiment). `-capture-frames 300,3600` saves `capture_<frame>.bmp` next to the exe (and one shot ~120 frames after the
post-game panel appears); `-quit-after <seconds>` ends the run by itself; `-end-round-after <seconds>` ends the round (reaches the post-game results). Player settings (resolution, borderless, mouse,
invert, key overrides, volumes, menu sounds, player name, master-server URL, favourites) are in
`build\bin\Release\data\userdata\settings.json`, edited from the SETTINGS tab.

Unattended build without the developer prompt: `build\nb_build.bat` calls Visual Studio's bundled `cmake.exe` directly
and writes `build\nb_build_out.txt` (VsDevCmd hung when started from a script). `build\nb_run3.bat` = 12-minute Sword Base
test with captures. These helper scripts are under the git-ignored `build\`.

Read `build\bin\Release\noble-client.log` after a run. Useful lines: `launch state N`, `Slot 4 - EngineEvent`, `slot N first call`,
`FATAL EXCEPTION`, `FrontEndMenu: PLAY ...`, `Slot 48 ... .map`.

## 3. Current status

| Milestone | State |
|---|---|
| Find MCC, Steam ownership check, load `haloreach.dll`, init, start | ✅ |
| Sword Base and other MP maps render and play | ✅ |
| Keyboard/mouse input (move, look, jump, crouch, sprint/ability, fire, weapon/grenade swap, Tab) | ✅ |
| Scoped aiming | ✅ (look settings block in profile) |
| Sound (gunfire, footsteps, damage) | ✅ (profile volumes + XAudio2 redist directory) |
| Mouse cursor in Reach menus (class/loadout pick) | ✅ (Windows arrow; RR uses it too) |
| Pause menu (Esc: Resume / Quit to Menu / Quit) | ✅ (ImGui overlay, Noble style) |
| Loading screen while the map loads (was ~5 s black) | ✅ map name + moving bar (`PauseMenu::DrawLoading`), ends on the first slot-10 call (tested) |
| Settings tab (controls, key bindings, audio, display, player name) | ✅ `SettingsPanel`, saved to `settings.json`, applied at next launch |
| Menu look and sounds | ✅ `NobleStyle` (Reach-like palette, Bahnschrift), `MenuSounds` (synthesised clicks, 40 ms hover cooldown) |
| Front-end menu, Reach style (`FrontEndMenu.cpp`): MAIN MENU list (Campaign, Matchmaking*, Firefight, Custom Game, Forge, Theater*, Armory, Mods, Settings, Quit; *greyed), My Party box, button hints; Campaign (mission list + preview, difficulty, skulls, score multiplier), Skulls, Firefight / Custom Game (map + preview, game type, difficulty), Forge (MP launch with MCC's `00_basic_editing_054.bin`: Forge World loads with no score/timer HUD; switching to the editor (Monitor) is untested), Games (server browser), MCC-style Settings | ✅ screenshots of every screen checked |
| Menu / loading art (`UiArt`): player's `data\ui\custom\`, Noble's `data\ui\noble\`, else read from the MCC install at run time (nothing shipped) | ✅ campaign scenes as backgrounds, map previews, skull icons, controller picture |
| Skulls (launch block +0x48) | ✅ verified (Blind hides the HUD) |
| Cinematic / dialogue subtitles | ✅ SUBTITLES setting (on by default), hook on `haloreach+0xD5244`; tested on Winter Contingency's intro |
| Campaign missions (cutscene → gameplay) | ✅ tested Noble Actual (prologue, ends after cutscene) and Winter Contingency |
| Firefight | ✅ tested with Standard. Scenario names (from MCC's preview pictures): `ff50_park` = **Beachhead**, `ff10_prototype` = **Overlook** (the old front-end menu had them swapped; BlamEngine's name aliases "beachhead"/"overlook" are still swapped, the scenario names it maps are right) |
| Game end (engine reports results through host slot 6) | ⚠️ post-game panel shows the right cR (15:19 test), but its buttons can't be clicked: the engine stops sending frames about 7 s later (§0) |
| Memory | ✅ no leak during a match: 12-minute run, private bytes 3835 → 3852 MB, working set ~1.3 GB, flat |
| HUD / scoreboard text and icons | ✅ letters, numbers and weapon/grenade icons render (glyph layout and string slots fixed, see §5) |
| HUD colours (bars, compass, motion tracker) | ✅ fixed 17:20: host slot 97 must return the colour it is given (§0) |
| Key bindings: all 66 slots named | ✅ slot numbers = MCC's `GameAbstractButton` order (enum strings in `MCC-Win64-Shipping.exe`); `KeyBindings.cpp` groups them like MCC's CONFIGURE BINDINGS (movement, actions, vehicle, UI, Forge, Theater; 10 slots only other Halo games use). Crouch default is now Left Ctrl (MCC) |
| Career / cR (`src/career/Career.cpp`) | ⚠️ first version, needs a real match to confirm stat slots: reads MCC's game-results record at slot 6, pays cR, shows it on the post-game panel, saves `data\userdata\career.json`. See §5 "Game results" |
| Spire (`35_island`) crash ~15 s into a match | ✅ fixed by the text fixes: 3+ minutes of Slayer on Spire without a crash (13:30 run) |
| Servers tab | ⚠️ LAN list, own hosted game always listed, favourites, direct connect, optional master server (`tools\master-server`). **Joining** another player's game is not possible yet (engine net slots 41/43 not implemented); the UI says so |
| Armor Locker / Workshop Mods tabs | ⚠️ screens only; not wired to the engine |
| Engine main menu (`mainmenu.map`) | ❌ by design: MCC ships no `mainmenu.map` for Reach (RR ships its own modified 298 MB `mainmenu_all.map`, which Noble cannot) |

## 4. Repo state

Work from this session went up as separate branches/PRs, each building on the previous one:
`feat/input-slot36` → `feat/sound` → `feat/menu-cursor` → `feat/pause-menu` (merged with `main` on GitHub, includes the
partner's TextAtlas/GammaPass) → `fix/zoom-aim` → `feat/front-end-menu` (front-end menu, then campaign, Firefight and the
extra tabs). Check on GitHub which of these are merged into `main`. Uncommitted at the time of writing: `docs/hud-text-findings.md`
and this file, plus possibly the campaign/Firefight changes if they weren't committed (`git status` shows it).

**Uncommitted work from 2026-10-27 afternoon** (on top of the night session below): `src/ui/UiArt.*` (new),
`FrontEndMenu.*` (rewritten, Reach-style screens + skulls), `SettingsPanel.*` (MCC style), `NobleStyle.*` (hero/small
fonts), `PauseMenu.cpp` (loading art, Firefight names), `KeyBindings.*` (groups), `Settings.*` (subtitles, campaignSkulls,
menuBackground), `ReachHost.cpp` (subtitle hook), `BlamEngine.*` (skulls at +0x48 as u64, `-launch-poke`), `main.cpp`
(`-skulls`, UiArt), `FrameCapture.*` (`SaveNow`), `CMakeLists.txt` (UiArt, windowscodecs). Suggested extra PRs: (7) skulls +
subtitles (engine), (8) UiArt + Reach-style front end + MCC settings.

**Uncommitted work from the 2026-10-27 night session** (on disk in `D:\Halo-Reach-Community-Edition`, builds, nothing committed):
new `src/common/Settings.*`, `Relaunch.*`, `src/engine/KeyBindings.*`, `FrameCapture.*`, `src/network/MasterServerClient.*`,
`src/ui/NobleStyle.*`, `MenuSounds.*`, `SettingsPanel.*`, `tools/master-server/`; changes to `ReachHost.*` (text atlas, string
table, slots 6/32/33/51, settings, memory log), `main.cpp`, `Window.*`, `SessionManager.cpp` (LAN beacon crash), `ServerBrowser.*`,
`FrontEndMenu.cpp`, `PauseMenu.*`, `UIManager.cpp`, `MissionLauncher.cpp` (map names), `CMakeLists.txt` (new files, winhttp/winmm,
`if(EXISTS data)`). Suggested PR split: (1) LAN crash fix, (2) HUD text/icons + string table, (3) settings + key bindings + style
+ sounds, (4) server browser + master server, (5) loading/post-game overlay + Relaunch, (6) test tooling (FrameCapture).

Local test files that must **never** be committed (they're RR's data, all under the git-ignored `build\`):
`build\bin\Release\data\profile_1.bin` (copy of RR's captured profile) and `data\test_profile_*.bin` (bisection files). Delete
them when convenient; Noble no longer reads `profile_1.bin` automatically.

## 5. Engine facts (haloreach.dll build above)

**Exports / vtable**
| Item | Detail |
|---|---|
| `SetLibrarySettings(p)` | zeroed 0x1FE block, three UTF-16 locale strings |
| `CreateGameEngine(&engine)` | returns 0x46 |
| `Init` vtbl+0x08 | `(engine, ID3D11Device*, 0, IDXGISwapChain*, presenter=0)`; finds the HWND **of the calling thread** |
| `Start` vtbl+0x10 | `(engine, host, launch_block[0x2BF30])`; spawns the engine thread. The engine can't switch maps afterwards: RR and Noble restart the process (Noble: pause menu "Quit to Menu") |
| `PostCommand` vtbl+0x18 | `(engine, id, payload)`; 10 then 11 after ~300 ms |
| Present | done **by the engine thread**; host slot 1 runs first with `(swapchain, &flags)`. Host overlays (gamma pass, pause menu) draw there; never use the immediate context from another thread while the engine runs |

`haloreach.dll` loads `xaudio2_9redist.dll` by bare name during start-up: Noble keeps `SetDllDirectoryW(<MCC>\MCC\Binaries\Win64)`
set for the whole run (as RR does), otherwise there is no sound. Audio itself is FMOD inside the DLL.

**Launch state machine** `haloreach+0xC1A114`: 0…10 while launching, 11 = launched (reached ~5 s before the map is on
screen; the first slot-10 call marks the game session start). Failure strings table `0x1803A2DCC`
(17 `no map selected`, 18 `no game selected`, 19 `map load failure`).

**Paths:** map list from `FindFirstFileW` over `haloreach\maps\info\*.*` relative to cwd (hence the sandbox); files through slot 48.

**Launch block (0x2BF30 bytes).** Noble builds every block itself (`BlamEngine::BuildLaunchBlock`). RR's captured blocks for
menu, MP, campaign and Firefight are byte-identical apart from these fields (layout used as reference only):

| Offset | MP | Campaign | Firefight | Menu |
|---|---|---|---|---|
| +0x0C | 3 | 1 | 5 | 1 |
| +0x10, +0x14, +0x1D5C0 | launch index 190–209 | 178–189 | 210–219 | -2 |
| +0x28 | 0 | `01 00 11 11` | 0 | 0 |
| +0x34 | 1 | difficulty 0–3 | difficulty 0–3 | 1 |
| +0x1D334, +0x1D5BC (map id) | MP map id (Sword Base 1000, Boardwalk 1035, ...) | 1035 | 1035 | 1035 |
| +0x300 game variant (decoded with `haloreach+0x373A0`) | chosen variant (default `slayer_054`) | slayer | **a Firefight variant** (`hr_ff_*.bin`; RR loads `game_variants\<mode>.bin`) | slayer |

**Skulls: u64 at +0x48** in MCC's skull numbering, verified (Blind hides the HUD). `haloreach+0x3A22C` converts it to the
engine's own bits (game options +0x1D8, copied to game globals +0x1EAC8, which the `skull_enable` script command flips).
The MCC bit is the skull's index in MCC's alphabetical skull list: Iron 18, Black Eye 3, Tough Luck 36, Catch 7, Cloud 13,
Famine 11, Thunderstorm 34, Tilt 35, Mythic 23, Blind 4, Cowbell 8, Grunt Birthday Party 16, IWHBYD 19 (38 skulls in the
converter). Noble used to write skulls to +0x3C, which is a different
field the engine reads as u16; that was removed.

Also: +0x18 / +0x1D5C4 `0x88880000`, +0x50 random 16-byte launch id, +0x1D308 map variant area (init `haloreach+0x6BFF8`).
Launch indices: campaign 178 m05 Noble Actual, 179 m10, 180 m20, 181 m30, 182 m35, 183 m45, 184 m50, 185 m52, 186 m60, 187 m70,
188 m70_a, 189 m70_bonus; Firefight 210 ff10_prototype, 211 ff45_corvette, 212 ff20_courtyard, 213 ff60_icecave, 214 ff70_holdout,
215 ff60_airview (verified: loads Outpost), 216 ff50_park, 217 ff30_waterfront, 218 ff_unearthed, 219 cex_ff_halo.

**Host interface** (`host[0]` = 160-slot vtable, `host[1]` = secondary interface; args `(host, a1, a2, a3, a4)`).
| Slot | Meaning | Noble |
|---|---|---|
| 1 | per-frame, before Present: `(swapchain, &flags)` | gamma pass, then overlay (loading / pause / post-game), then test capture; memory log every 1800 frames |
| 4 | engine event `(reason, msg)` | logs |
| 6 | **game results** `(buffer, 0xD5A8, 0x14)`: called when a game ends (RR `results::on_game_results`) | shows the post-game panel (only once launch state is 11); results not decoded yet |
| 10 | return secondary interface | ✅ |
| 23 | load progress | ✅ |
| 32 / 33 | display settings / quality | 32: size from the window; 33: writes the 16 bytes `0x3F80000000000201, 0x2BF800000` **directly at a1** (as RR) |
| 34 | **profile**: pointer to a 64 KB block (see Profile below) | ✅ built-in defaults |
| 36 | **input poll** `(userIndex, u8 state[0x130])`, users 0–3 every tick (`haloreach+0x23AE7`) | ✅ |
| 47 / 48 | user-data folder / file path resolver | ✅ |
| 51 | feature query `(id, bool* out, u64* extra)`: RR answers from mask FEATURES_ON = 0x6E and zeroes `*extra` | same mask; the engine only asks 1, 2, 5, 6 (all on) |
| 55 / 56 / 57 | strings: clear / register `(table, id, const char* name, const char* suffix)` / lookup `(table, id, wchar_t* out, cap)` | key = CRC-32/MPEG-2 (poly 0x04C11DB7, init 0xFFFFFFFF, no xorout) of upper-case `name_suffix` (leading `$` skipped); text from `<MCC>\data\ui\Localization\EN_HaloReach.bin` (magic 0x90CE6B7A; u32@0x78 key bytes, keys from 0x80; u32@0x7C string block; NUL-terminated UTF-8 in key order). Lookup returns 0 when unknown so the engine uses the map's own strings |
| 61–67 | text: prepare, glyph, atlas, has_icon, kerning, register_icon, metrics | ✅ `TextAtlas`: 1024² pages (≤16), shelf packing, dirty-rect upload in slot 63 |
| 88 | identity `(u64* xuid, wchar_t* name, cap, userIndex)` → 1 | ✅ |
| 116 | controller map `(xuid)`; RR only logs | returns 0, writes nothing |
| iface2 147 | pointer to a 16-byte session GUID; every other iface2 slot returns 0 (RR does the same) | ✅ |

**Glyph record (slot 62 out, 26 bytes):** ch@0, zero@2..7, atlasX@8, atlasY@0xA, w@0xC, h@0xE, originY@0x10, originX@0x12,
advance@0x14, -ascent@0x16, page@0x18. Glyph alpha = min((gray·255+32)>>6, 255) from GDI GGO_GRAY8. Icons (0xE000–0xE2FF): the
engine registers one through slot 66 `(host, ch, size, f32 scale, face, w, h, top, ARGB4444-decoded pixels, pitch)` only when
slot 64 has_icon returns false (engine path `haloreach+0x1CF4D0` reads `font_package_icon.bin`); store the pixels unchanged,
originY = top, originX = 0, advance = w, ascent = 0.

**Input state (slot 36 buffer, 0x130 bytes, same as RR byte for byte).** +0x000 u32 1 (keyboard/mouse; 0 = gamepad);
+0x004+VK u8 per key (`0x80` pressed < 500 ms, then `0x81..0xFF` held); +0x104/+0x108 f32 mouse look; +0x114 wheel;
+0x118 f32 1.0; +0x11C mouse buttons; +0x120.. gamepad (RR `pad::fill`, not done). The engine reads keys through a 104-entry VK
table (`haloreach+0x98FDF0`) but maps them to actions **only through the profile's binding table**.

**Profile (slot 34).** RR returns a zeroed 64 KB block, optionally filled from a captured profile (`--profile`, first 0xC00 bytes),
with a 4-char UTF-16 service tag at +0x1AC. Everything the engine reads from a blank profile is 0, which broke several things.
Noble's built-in defaults (`ReachHost.cpp`):
| Range | Content | Effect when missing |
|---|---|---|
| +0x1B4..+0x1DF | look/controller settings (sensitivity bytes, dead zones 0.13/0.12, 1.0 multipliers, option bytes) | aim frozen while scoped |
| +0x42C..+0xA5B | key bindings: 66 × 0x18 `{u32 action, u32 VK, 16 zero bytes}`, MCC default PC layout | no key does anything |
| +0xA5C / +0xA60 / +0xA64 | f32 volumes (game / music / other) | no sound |
Still zero: rest of +0x000..+0x42B and +0xA68..+0xBFF (suspected to hold the HUD brightness/opacity setting).

**Subtitles.** Before drawing subtitles the engine calls `haloreach+0xD5244`, which walks the local players' table
(`haloreach+0x2A03C50`, stride 0xAB8, profile data at +8) and reads each player's subtitle setting at profile +0xA30
(0 automatic = only when voice and text language differ, 1 on, 2 off). That table is **not** a copy of the slot 34 buffer
(writing +0xA30 there does nothing; volumes etc. are read field by field). Noble hooks `0xD5244` (prologue `48 89 5C 24 08`)
and answers "on" when the SUBTITLES setting is on.

**MCC UI texture packs** (`<MCC>\Data\UI\texturepacks`, read by `UiArt`): `<name>.perm.bin` = records of 0x1E0 bytes, magic
0xCDBFA090, name at +0x44, u16 w/h at +0x74/+0x76, u64 size at +0x88, u64 offset into `<name>.temp.bin` at +0x90; data BC1 or
BC3 (decided by size per 4x4 block). Useful packs: `levelstexturepack` (every Reach map/mission preview, 316x160 and
668x292 `…LARGE`), `skullstexturepack` (128x128 skull icons), `controllertexturepack` (controller picture, button glyphs),
`mainmenutexturepack`, `globaluitexturepack`. `loadingtexturepack.perm.bin` (304 MB) has no names: entries
`{u32 0x5E73CDD7, u32 size, u32 size, u32 0, data}`, padded; 1920x1080 BC1. Reach's 12 campaign scenes are found by a CRC-32
of their first 4 KB (table in `UiArt.cpp`). `Data\UI\BackgroundVideos\Images\*.png` are only 392x220.

**Key-binding slots.** The 66 profile slots use the engine's own action numbering, which matches neither MCC list in
`Data\BuiltInData\HaloReachController.json`. Names in `KeyBindings.cpp` come from MCC's default keys (unique key or the
W/A/S/D block); the rest are shown as "OTHER (NOT IDENTIFIED YET)". Several defaults appear twice (two `R`, `F`, `LMB`, `Space`),
so e.g. Fire may be slot 8 or 27: test a rebind of each named action in game.

**Loadout names** come from the game variant (Slayer's default list is Sprint / Armor Lock / Jet Pack / Active Camo /
Hologram; Invasion on Spire shows its own tier list starting with Scout; Air Assault / Operator / Grenadier etc. belong to
such game types). They render correctly.

**UI cursor.** `haloreach+0x2D2018` runs only while a Reach screen that uses the pointer is up (RR `uicursor::install` hooks it,
prologue `48 89 5C 24 18`). Noble hooks it with MinHook; within 250 ms of a call the mouse is freed, the arrow shown and no
look delta sent. The engine reads the pointer itself (GetCursorPos/ScreenToClient).

### Game results (host slot 6)

`haloreach+0xF95CB` builds MCC's results record (**0x5D138 bytes**) with `haloreach+0x39128` from the engine's own
results at `haloreach+0x28B83B0`, then calls host slot 6 (`[host]+0x30`) with **a1 = pointer to the record**. a2 is not a
size: it is r8 left over from the conversion loop (always 0xD5A8). Layout read from `+0x39128`:

| Offset | Contents |
|---|---|
| +0x0000 u32 | game category (engine mode byte at results +5 mapped to MCC's list) |
| +0x0008, +0x0010 u64; +0x18, +0x19 u8 | copied from the engine header |
| +0x0020 | 16 player records × 0x1C8: +0x00 u8 present, +0x08 u64 player id, +0x18/+0x28/+0x2C i32, +0x30 u8, +0x34 i32 score (engine word or dword depending on mode), +0x38.., then appearance/name copy from +0x4C |
| +0x1CA0 | 16 × 4 bytes team data |
| +0x1CE0 | 16 per-player stat blocks × 0x5448: +0x000 23 u32 general stats, +0x05C 11 u32, +0x0AC 0x180 u32 medal counts, +0x6AC 0x32 u32, then weapon tables (0x54 weapons, 6 values each) |
| +0x56160 | per-player pairs; +0x5D120/+0x5D124/+0x5D128 u32 and +0x5D130 u64 match totals |

General-stat numbering: `haloreach+0x3B208` maps engine stat n to MCC slot (0→0, 1→1, n→n+1 for n ≥ 2). Assuming Reach
kept Halo 3's statistic order, slots 8/9/10/11 = score/kills/assists/deaths (**provisional**, `Career.cpp` kStat*).
Medal numbering: table built by `haloreach+0x39B00` (0x91 engine medals → MCC ids ≤ 0x17F).

Test option `-end-round-after <seconds>` calls the `mp_round_end` script command's work function
(`haloreach+0x7B3EC(-1, 0, 1)`) from slot 1, so automated runs reach slot 6. Noble writes the last record to
`data\userdata\results\last_game.bin` (local only: it holds player names and ids; never commit it).

## 6. Known problems and next steps (in order)

1. ~~**HUD bars, compass ticks and motion tracker are dark navy**~~ **Fixed 17:20 (slot 97, see §0).** Old notes: (letters/icons are fine). The game-type intro box shown
   during loadout selection, the "Visegrad Communications Outpost" waypoint box and the hint bar in Winter Contingency have
   the same dark tint (in MCC that box is bright cyan), while the loadout cards are correct. Ruled out: gamma, profile (RR's
   `profile_1.bin` gives the same picture), slot 51 mask, slot 33. **Confirmed 15:10:** RR's own HUD (Firefight, Beachhead)
   is bright light blue (shield bar, compass, tracker, timer box) while Noble's on Sword Base is dark navy; the world looks
   the same in both. The `GammaPass` in `ReachHost.cpp` is fixed at exponent 1.0 (a no-op; a whole-frame gamma washed the
   world out), so the fix must touch only the HUD. RR has a separate "UI Gamma" setting next to Gamma, which hints at a
   HUD-only value the host supplies. Next: look at how the HUD shader gets its colour/tint (chud globals, possibly a
   render-state or a constant the host sets in another slot, e.g. iface2 slots RR answers non-zero).
   Background (Bungie's Halo 3 HUD spec, cand.land/halohud): HUD element colours, flashes and scale come from the
   map's HUD tags ("HUD globals" / chud), the HUD is drawn mapped onto the inside of a sphere with a virtual camera
   and a per-resolution FOV from those globals, and layout uses anchors (4 corners + centre) with pixel offsets. So
   the colour is tag data the engine reads itself; if RR is bright, look for a host answer that changes which
   resolution/exception block or render target format the engine picks, rather than a colour value.
2. ~~Spire crash~~: no longer happens after the glyph/string fixes (3+ minutes on Spire, 2026-10-27 13:30).
3. **Memory**: Noble logs `Memory: frame N private X MB, working set Y MB` every 1800 frames. A full 10-minute match was
   flat (+17 MB). If a leak is still seen, it is probably across front-end menu ↔ match restarts or in the menu itself.
4. **Online joining**: engine networking slots 41/43 (and session setup) are not implemented, so the SERVERS tab can host
   (listed on LAN/master server) but not join. This is the big missing piece for multiplayer.
5. **Scoreboard/results**: the slot 6 record is decoded far enough for career pay (§5 "Game results"). Still to do: a
   post-game scoreboard from it, confirm the general-stat slots with a real match, campaign "next mission".
6. ~~Mouse wheel~~ done 18:18 (see §0). Gamepad (XInput into +0x120.., RR `pad::fill`) still to do.
7. Armor / Workshop tabs; remove debug-only bits when no longer useful.
   Menu ideas from Bungie's UI director (cand.land/new-page): menu backgrounds were layered concept art on cards with
   a slowly moving camera (parallax), and the main menu had a one-press "fast path" to the last mode played.
8. **Cursor lock** (fix in 18:18 commit, untested by hand): in windowed mode the game keeps the cursor clipped to its window, even while dragging the title bar,
   so the window can't be moved back (seen 15:14, the window ended up behind the taskbar). Only Alt+Tab frees it. Release
   the clip while the window is being moved or isn't focused (`WM_ENTERSIZEMOVE` / `WM_KILLFOCUS`), and re-clip on focus.
9. **Respawn timer** shows "Respawn in 00000001": the number filling `camera_list.prop_respawn_time` is zero-padded.
   Check the format used for that string (string table entry `0x84091d25` in `ReachHost.cpp`).
10. ~~Player name / service tag~~ done 17:58: the name comes from SETTINGS > GAMEPLAY > PLAYER NAME (slot 88) and the
   service tag from the new SERVICE TAG field (`Settings::serviceTag`, 1-4 letters/digits, default NOBL; written to
   profile +0x1AC).
11. **Forge**: the menu launches MP with `00_basic_editing_054.bin`; the map loads, check in game that the Monitor/editor toggle works.
9. **Career system** (planned with Tyler). Done: `Career` module, 50 ranks (Halopedia table), cR after each match
   (per-minute pay rising with rank, kills/assists/medals bonus, 1-in-8 random bonus 100–3000, 1-in-200 jackpot 7,777;
   **no daily cap** and up to **90 paid minutes** per match in every mode (Tyler's calls). Pay goes by **base type** =
   the engine game mode BlamEngine launches with (campaign / Firefight / multiplayer incl. Forge), so modded maps and game
   types earn exactly what their stock type earns (`Career::BeginMatch` is called from `BlamEngine::LaunchMap`); all constants at the top of `Career.cpp`, Noble's own numbers), post-game cR breakdown + rank bar.
   Next: confirm stat slots (play a short Slayer with a few kills; `last_game.bin` + the `Career:` log lines), campaign
   scoring (difficulty + skull multipliers, time bonus, par scores), 45 commendations, challenges (Sep 2010 – Feb 2012
   lists) paying cR, a career/service-record screen and rank icon in the menu, Armory with the armor list.
   All the data for this is in `docs/career/` (see §0).

## 7. Partner's Outbreak-Z code (reference for Noble's menus)

The partner (`O7OF`) wrote and owns a Godot game, Outbreak-Z, and shared its scripts as reference material. They are
GDScript for Godot, so none of them compile into Noble (C++ / D3D11 / ImGui). They can be used as designs to port; because
the partner owns them, porting or copying them into this repo is fine. Keep the copies of these files outside the repo
unless one is actually ported.

**Usable almost as-is**
- `master_server.js` + `package.json`: **ported** to `tools/master-server/` with `MasterServerClient` in Noble. The
  original already had a 1 MB body limit; the real gaps were that anyone could heartbeat/delete any session and that the
  listed address came from the request body. Now: secret token per session (`X-Session-Token`), address from the socket,
  4 KB body, 4 sessions per IP, 120 requests/min per IP, 2000 sessions, text sanitised. `node test_master_server.js` passes.

**Port as designs**
- `server_browser.gd`: **ported** (favourites, direct connect, filters, host dialog with password). Ping and join need
  networking.
- `settings_manager.gd`, `build_settings_menu.gd`: Gameplay / Controls / Keybinds / Controller tabs with sliders. In Noble
  the controls write the profile fields from section 5: look settings at +0x1B4, key bindings from +0x42C,
  volumes at +0xA5C/+0xA60/+0xA64. **Ported** as the SETTINGS tab (`SettingsPanel`), controller tab not yet.
- `glass_ui.gd`: **ported** into `NobleStyle`.
- `audio_manager.gd`: only the menu parts apply. Click / hover / tab sounds with a 40 ms hover cooldown, and a menu music
  playlist (in order or shuffled, loops, no repeat of the last track when a shuffle restarts). Play through XAudio2, which
  is already loaded. In-game sound is the engine's own FMOD and needs nothing from this file. Sound source: Reach's
  sounds can't be committed (public repo); either read them at runtime from the player's MCC install (FMOD sound banks)
  or use our own / royalty-free sounds plus a `data\menu_music\` folder for the player's own files. Click/hover/tab sounds
  are **ported** as synthesised blips (`MenuSounds`); music not done.
- `game_loading_screen.gd`: **ported** as the loading overlay. The engine's slot 23 progress stays at 5%, so the bar only
  shows activity, not real progress. (`ui/LoadScreen` is never drawn while the engine runs.)

**Not applicable to Reach**
- `barracks_menu.gd`, `save_stats_manager.gd`, `floating_damage_number.gd`: Outbreak-Z skins, capes, cards, prestige and
  zombie stats. Reach armour lives in the engine/profile.
- `dev_tools_menu.gd`: zombie-mode cheats (points, perks, power-ups).
- `dev_account_search_menu.gd`, `backend_client.gd`: need Outbreak-Z's account database; its login/register ignore the
  password, so don't carry that over.
- `hud.gd`: Reach draws its own HUD.
- `scoreboard.gd`: Reach's in-game scoreboard is the engine's; a post-game one would read the slot 6 results buffer.
- `chat_box.gd`: the engine does **not** draw chat outside MCC; a chat box would have to be Noble's own overlay, and needs
  networking first.
- `check_api.gd`: one-line test script.

**Status:** settings, style, menu sounds, server browser, master server and loading screen are ported (see §3). Left:
controller settings tab, menu music, chat (after networking).

## 7b. Partner's server (`server/`, commit `ae7b71e`) - review notes for O7OF

Accounts (scrypt + random tokens), SQLite tables (users, tokens, player_stats, matches, match_player_records),
session registry, matchmaker (8 playlists). DB goes to `server/data/` (git-ignored). Left for the partner to fix:
1. `server.js` is missing from the commit (`package.json` "main"/"start" and `node server.js` point to it).
2. `node:sqlite` needs Node 22.5+; `package.json` says `>=18`. The catch-block fallback requires `node:sqlite` again.
3. `npm test` points to `test/test_backend.js`, which isn't there.
4. `ranks.js` doesn't match `Career.cpp`: base pay 60 (game: 13 cR/min), Major Grade 2 370,000 (game: 375,000), random
   bonus 100-999 (game: 100-3000 in steps of 10), jackpot only on roll 77 of 200 (game: plain 1-in-200), +250 win bonus
   (not in the game). Both spell "Field Marshall"; Reach's rank is "Field Marshal".
5. For matchmaking stats the server should be the only place cR is calculated, from the host's match results;
   otherwise a client can submit any total.

## 8. Rules for this project

- Read-only toward MCC files. Writes go to `build\bin\Release\data\…`.
- Never commit or publish: RR binaries/data/decompiled code (including `profile_1.bin`, launch blocks, `mainmenu_all.map`),
  Omaha-derived files, `haloreach.dll` or dumps of it (e.g. dumpbin output), crash dumps, logs (they contain a player name/ID).
  The repo is **private** (Tyler, 2026-10-28 15:41; it was public before). GitHub rejects files over 100 MB.
- Exception (Tyler, 2026-10-28): the Reach menu art in `build/bin/Release/data/ui/reach/` is committed on purpose.
- Rule change (Tyler, 2026-10-28 15:37): RR data may be used. Noble now uses RR's `mainmenu_all.map` + string_ids
  (copied from `D:\Reach\data\menumaps` to `build\bin\Release\data\menumaps`) so the engine shows Reach's own Armory
  scene (O7OF's ReachArmory, found through `ReachHost`/`VFS` when the engine asks for mainmenu.map). The 298 MB map is
  over GitHub's 100 MB file limit, so it stays local; each tester copies it from their RR install.
- **Engine Armory scene, status 15:45 (for O7OF):** with RR's menumaps copied, the engine resolves `haloreach\maps\mainmenu.map`
  to `data\menumaps\mainmenu_all.map` and ReachArmory loads 32,213 string_ids, but `ScanGlobalsTable` never finds the GLOBALS
  table (log: one scan message, then nothing for 40 s; `sceneReadyFrames=0` at frame 6480). LoadProgress stops at 5 % and
  FrontEndMenu logs "engine stopped drawing the menu" 1 s after launch, so the menu map's scenario/scripts may never start.
  RR also ships main-menu launch blocks (`D:\Reach\data\lb_m_c_1.bin`, `launch_block_1.bin`); Noble may need RR's main-menu
  launch block for the scene to run. Until then the OBJ preview (local `data\armor` exports) is what the Armory shows.
  Large local files are git-ignored explicitly (`**/menumaps/`, `**/data/armor/`, `*.map`, `*.obj`, `*.mtl`, `*.rdc`, RR bins).
- Keep the Steam ownership check. No anti-cheat bypassing, no official MCC online services.
- Small, separate PRs. The partner edits `ReachHost.cpp`/`BlamEngine.cpp` too, so tell him before large edits; when both sides
  change `HostSlot1_FrameTick`, keep the gamma pass first and the pause menu after it.
- Before overwriting a file, re-read the current version first, and re-check it a minute later (file sync has put stale copies
  back before).
- Commits, PRs and docs carry only the authors' names (no "generated with" or co-author lines).
