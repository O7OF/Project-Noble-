# PROJECT NOBLE COMPENDIUM

### A working reference to the Blam engine, the client, and the multiplayer build-out

> **What this is.** A curated, single-file reference for engineers joining Project Noble. It keeps the durable technical knowledge the project has accumulated (engine memory model, session/networking internals, model and armour mechanics, Forge/pack pipeline, runbook, bug archaeology, external tooling) and states the project's current position as of the date stamp below. It is not a changelog, not a task board, and not a mod API.

> **How to read it.** Sections 1-4 are the current state and the multiplayer workstream; sections 5-6 are engine subsystem references; 7 is the day-to-day runbook; 8 is the failure archive and the genuinely open questions; the appendices are quick-reference tables. Claims carry a confidence tag; when two notes disagree the newer one wins and the discrepancy is called out.

> **Version stamp.** Project Noble **v0.20.10**, repo `D:\Project-Noble-Closed-Source`, branch `wip-merge`, HEAD **`8d16160`** (latest upstream merge `8d16160` brought in the v0.20.10 line; the work below it is the high-slot seat fix `0fdcf97`, mod-version diagnostics `2eec1b4`/`1e853ec`, and the 32/64/128 capacity/chunked-layout model + headless self-test `0ebd1a2`; this documentation commit sits on top). Both branches are pushed and identical at that commit. Engine reference: MCC `haloreach.dll` build 1.3528.0.0, SHA-256 `738dd2d2...0894`, image base `0x180000000`. Document built 2026-11-16. All RVAs are `haloreach.dll` RVAs (Ghidra address = `0x180000000` + RVA); a game update invalidates them.

**Confidence legend**

| Tag | Meaning |
|---|---|
| **proven** | a one-variable test plus a capture/log, reproduced |
| **observed** | seen in a live run, log or shipped release note, but not isolated by a controlled test |
| **code** | read from the source tree; behaviour not exercised |
| **community** | from public community documentation, not this project's tests |
| **unverified** | inferred from disassembly, docs or older notes; never runtime-checked |

**Rule of the house:** nothing is "fixed" until it is proven; a claim without an evidence path is a hypothesis.

---

## Contents

1. Project overview and current state
2. Engine memory model - tags, banks, allocator, budgets
3. Session and networking - player tables, messages, slots, transport
4. Multiplayer workstream (2026-11-16)
5. Models, armour and animation
6. Forge and the pack pipeline
7. Tooling and runbook
8. Bug archaeology and open questions
- Appendix A: RVA quick reference
- Appendix B: source and documentation map
- Appendix C: tools, engines and reference material

---

## 1. Project overview and current state

### 1.1 What Project Noble is

`noble-client.exe` and `noble-dev.exe` host Halo: Reach's own engine (MCC's `haloreach.dll`) without launching MCC. Noble provides its own ImGui front end, a 3D Armory with live model manipulation, custom games / Forge / Firefight / campaign hosting, a server browser, a master-server cluster, P2P plus WebSocket-relay netcode, mods served through a VFS, and runtime-extended engine limits. It is an unofficial fan project: no game assets ship, and it loads the user's own MCC install and maps. Partners historically: Tyler (builds, tests, reverse engineering; not a C++ developer), O7OF (upstream code / repo owner), Dominus.

The public, closed-source distribution line is `0.20.x`; the earlier open-source base is archived at 0.9.16.37 and receives nothing. The master-server cluster is build-gated so that only matching closed-source builds may register, join or relay.

### 1.2 Binaries and targets

| Target | Output | Purpose | Gate |
|---|---|---|---|
| `noble-client` | `build\bin\Release\noble-client.exe` | release client | **required** |
| `noble-dev` | `build\bin\Release\noble-dev.exe` | development client: runtime log, test switches, diagnostics | **required** |
| `noble-dedicated` | `build\bin\Release\noble-dedicated.exe` | dedicated host stub (config + sockets + advert; **no engine session** - see 3.8) | compiles, does not block |
| `mvar-tool` | `build\bin\Release\mvar-tool.exe` | map-variant (`.mvar`) inspection tool | tooling |

Only `noble-dev` and `noble-client` are release gates. The dedicated server must compile but does not gate a push.

### 1.3 Version, repo and branches

| Fact | Value |
|---|---|
| Version | **0.20.10** in `src/common/Version.hpp`; dev builds report `0.20.10-dev (Closed Source)` |
| Branch | `wip-merge` @ **`8d16160`** (tracked tree clean; large maps/packs/saves are not in git) |
| Merge | `8d16160` merged upstream `ed72784` (v0.20.10 post-match KBM UI focus fixes); the commits below it are Noble-side work (high-slot seat, mod-version diagnostics, capacity/chunked layout model) |
| Branch parity | `main` and `wip-merge` are pushed and identical at `8d16160` |
| Release tags | the `v0.20.*` line is published through the release repo; the local clone carries tags only up to the `v0.19.x` line |

### 1.4 The v0.20.0-v0.20.10 release line

Everything below is shipped in the merged `origin/main` line at HEAD. Sources: `release-notes\v0.20.*.md` and `src/ui/ChangelogData.hpp` (in-client changelog).

| Version | Headline content |
|---|---|
| 0.20.0 | Full Elite customisation (Honor Guard, Counselor, H2A Ranger, H2A Armor, Schism sets/pieces) in Armory and in-match; Elite armour abilities (jet pack, evade, drop shield, armour lock, camo, hologram, sprint) mount correctly; multiplayer customisation replication of model/species/permutations; custom visor colours in-match and networked on Spartans and Elites; authoritatively coordinated Forge AI spawning (non-host clones no longer duplicate/static); netcode hardening; loading-screen emblems restored; pause-menu overhaul |
| 0.20.1 | Career/challenge progress persistence hardening; service-record parity for challenges; pause-menu navigation/controller polish; host lifecycle and hook stability during match transitions |
| 0.20.2 | Settings save fix (`settingsVersion: 2`, per-process temp file + atomic `MoveFileExW` replace with in-place fallback and on-screen failure alert); controller A-button apply; end-match focus stability; unified host/non-host post-match settlement with duplicate-payout guard; non-host post-match crash fix; match watchdog for the summary screen |
| 0.20.3 | Release hardening: whole-program optimisation/COMDAT folding, leaner packages, minidump diagnostics, release logging compiled out; introduced the tracked documentation tree, `ANTI-REVERSING.md`, string-obfuscation helpers and the doc-pack SHA-256 manifest |
| 0.20.4 | Loading roster sent to every process (guests see the full lobby); scoreboard marks only the local player, per-player service tags, joiner ping; invite/join-by-code uses direct connect and shows the host's real name; server-browser version strings match the menu; AMD CPU power-throttling (EcoQoS) opt-out, above-normal process and render-thread priority |
| 0.20.5 | Firefight join-in-progress: host refusal reason 3 (join in progress) is dropped while a Noble session is active so the engine handshake can complete |
| 0.20.6 | Settings migration now first-run only (stale mirror no longer overwrites saved settings); Firefight JIP refusal-3 fix carried |
| 0.20.7 | Firefight JIP: host `AbortGame(reason=6)` suppressed while a Noble network transport is active (logged as `SUPPRESSED AbortGame(reason=6) for join-in-progress`); settings migration fix; changelog notes |
| 0.20.8 | Custom-game join-in-progress: a guest resolves the host's content mod on a stock map instead of dropping it (the modded-Custom-Game desync/abort path); join/connect refusal suppression now respects the engine's launch join lock (`publicSlots == 0`); version bump |
| 0.20.9 | Third-person camera engine support: engine-side third-person camera hooks and tuning (`src/engine/CameraThirdPerson.cpp`); changelog/version-history data (`src/ui/ChangelogData.hpp`); version bump |
| 0.20.10 | Post-match keyboard/mouse UI focus and click handling stabilized (post-game panel and pause-menu focus, picker/lobby click routing, shared NobleUi helpers); version bump |

The line also contains a substantial **front-end UI rework**: `FrontEndPickers.cpp`, `PauseMenu.cpp`, `SettingsPanel.cpp`, `FrontEndLobby.cpp` and the new shared drawing layer `NobleUi.cpp/.hpp` were rewritten or heavily reworked, and the ElitePack/ReachHost engine integrations were wired into the UI (commit `f28a98f`: 17 files, roughly +4.2k/-3.4k lines). *observed (diffstat)*.

### 1.5 Build, test and release entry points

| Task | Command |
|---|---|
| Configure | `cmake -S . -B build -A x64` (first configure fetches ImGui, spdlog, nlohmann/json, MinHook, Opus via FetchContent) |
| Build all gates | `cmake --build build --config Release --target noble-dev noble-client noble-dedicated` |
| One-shot release client | `build.bat` (locates CMake/VS, builds `noble-client`, copies `data`/`assets`/`tools\vgmstream` into `build\bin\Release`) |
| One-shot dev build | `build-dev.bat` (same, `noble-dev`) |
| Package for local install | `build-package.ps1` (rebuilds, stages to `Documents\Noble Launcher`) |
| Publish a release | `tools\publish-release.ps1` (see 7.4) |

`build.bat` never pulls git; pull manually first. New `.cpp` files must be added to `CMakeLists.txt` (both the client source list and `DEDICATED_SOURCES` when shared).

### 1.6 Pinned engine limits (shipped)

Engine-limit profiles are **no longer user-selectable**. The shipped pin is declared in `src/host/EngineLimits.hpp`:

| Constant | Value |
|---|---|
| `kShippedProfile` | `Profile::Full` - V16 + particles + light volumes |
| `kShippedAiActors` | **1024** AI actors |
| `kShippedLayoutGeneration` | 1 |
| Profile network id | `0x03010400` = `[profile 3][layout generation 1][AI actors 1024]` |

The profile network id is sent with the session so builds with a different game-state layout refuse to join. The release package **must** carry `data\limits\full.nlp`; `tools\publish-release.ps1` throws if it is missing, because a missing patch file would silently fall back to the stock engine layout.

History: upstream force-disabled the Extended profiles (commit `4407101`) after an alleged black-screen/kick. A 2026-11-15 read-only probe disproved that (the real cause was a launch-block regression fixed in v0.19.95), the profile was pinned and shipped, and the first cross-game pool raises landed with it (see 2.6). *proven*.

**Decision (Tyler, 2026-11-15):** pin Full + 1024 actors for every install; no pickers, per-profile save folders.

### 1.7 Status matrix

| Area | State | Confidence |
|---|---|---|
| Release line v0.20.0-v0.20.10 | shipped and pushed; `main` = `wip-merge` = `8d16160` | proven |
| Shipped multiplayer path (16-slot engine session) | works at **2 and 5 players** on one machine with stock engine limits; join/leave/rejoin proven | proven |
| 16-player multiplayer acceptance | not yet run - blocked by one-machine RAM, needs 3-4 PCs (see 4.5) | proven absence |
| High-slot (peer 17 / player 16) membership layer | fixed; peer 17 reaches peer state 8 and the membership delta is accepted | proven |
| High-slot join-in-progress **seat** path | **fixed** (`0fdcf97`): peer 17 stays seated, both sides enter gameplay, 0 boots/closes | proven (2-instance high-slot run) |
| 32-player mode | dev-gated; **120-site** patch installs, high-slot seat works (peer 17 plays), declared-32 session joins with 2 players; setter churn and real 17-32 players remain | proven at install + seat level |
| 64/128 players | capacity model + chunked-layout plan validated by the headless self-test (`0ebd1a2`); chunked install, wire widening, 128 message framing, pools and UI remain (see `docs/02-engine-reference/TOPIC-scaling-64-128.md`) | code + synthetic test |
| Lobby/picker/server-browser honesty | implemented (single effective-seat source, greyed 24/32, live roster, browser verdicts) | proven for the 2-player LAN check |
| Settings persistence after a match | fixed 2026-11-16: the menu process re-reads `settings.json` after the match child exits (and on a fresh SETTINGS open), so in-match changes are shown and never overwritten by the stale pre-match copy | proven (two-process reload test + 2-player smoke) |
| Campaign co-op | 4 players stock works; more than 4 parked (fails at the network session-start path) | proven |
| Forge World content (Pelican/Spectre/Boathog/targeting laser) | ships as a separate optional map asset | observed |
| Armour, visors, Elite sets, customisation replication | shipped in v0.20.0 | observed |

Working priority (Tyler, 2026-11-16): **multiplayer large sessions first**; campaign co-op beyond 4 parked at the end; exact per-player visor colour, armour-effect variants, Elite lighting, Forge/variant tooling follow.

---

## 2. Engine memory model

### 2.1 Live tag table

| Fact | Value | Confidence |
|---|---|---|
| Pointer to the live tag table | `*(haloreach + 0xC1A600)` | observed |
| Tag-page pointer bases (16 entries) | `haloreach + 0x4E39F20` | unverified (code) |

The tag table is the runtime index of loaded tags; `TagAt` maps a packed tag reference through the pointer bases. Armour work (Armory, ElitePack) reads it live.

### 2.2 Game-state banks

Game-state memory is carved from 8 banks described by a static record table. The same record powers `PoolResize.cpp`'s budget guard and `PoolReport.cpp`'s `BANK|n|used|capacity` log lines.

| Fact | Value | Confidence |
|---|---|---|
| Bank table | static array at RVA `0x27BF270` | proven (live `BANK|` lines) |
| Records | 8, one per bank (0-7) | observed |
| Record stride | `0x18F0` | code |
| `+0xA0` | capacity, u64 | proven (values observed) |
| `+0xA8` | bytes handed out by the bump allocator, u64 | proven (values observed) |
| Allocator | bump allocator with **no bounds check** | consequence proven (2.5) |

Observed values (campaign m35 and Sword Base, Full profile, 2026-11-14 10:52-11:09, after the beam guard):

| Bank | Used | Capacity | Capacity hex | Known role |
|---|---|---|---|---|
| 0 | 125,992 | 131,072 | 0x20000 | unknown (small) |
| 1 | 7,860,328 | 8,519,680 | 0x820000 | **AI bank** (stock `0x220000`) |
| 2 | 50,740 | 65,536 | 0x10000 | unknown (small) |
| 3 | 58,613,580 | 67,043,328 | 0x3FF0000 | **render bank** (offline-patch threshold `0x560000`) |
| 4 | 51,656,592 | 52,756,480 | 0x3250000 | **main bank** (raised from `0x27D0000` by the Full profile) |
| 5 | 5,124 | 65,536 | 0x10000 | unknown (small) |
| 6 | 338,488 | 1,572,864 | 0x180000 | unknown |
| 7 | 0 | 589,824 | 0x90000 | unknown |

Banks 1, 3 and 4 are the ones the limits work touches. Roles of 0/2/5/6/7 are not established.

### 2.3 Allocator and pool cost

- Named pools (`POOL|name|...|live`) are created through **`data_array_new`** at RVA `0x12FCC` (signature `0x64407440`). *proven: pool resizing by name works and the report sees the raised capacities.*
- Pool memory = `count x recordSize + ceil(count/32) x 4` (one bitmap bit per 32 records) - `ArrayBytes` in `PoolResize.cpp`. *code.*
- The allocator only advances the `+0xA8` cursor. A pool that does not fit corrupts the bank instead of failing. *consequence proven (2.5).*

### 2.4 Budget rules

| Rule | Bank | Formula | Confidence |
|---|---|---|---|
| AI pool budget | 1 | `budget = bankSize - 0x220000 (stock) - 0x10000 (safety)`; a resize that does not fit is trimmed | code; "AI memory +6080 KB" observed |
| Render-state budget | 3 | `renderBudget = renderBank - 0x560000 - 0x10000` | code |
| Later-pool reserve | 3 | beam expansion must leave `kLaterPoolsReserve = 9 MB` + `0x10000` safety | code; the fix is proven |

`Hook_DataArrayNew` charges every resize against its bank's remaining room and keeps the stock count when it does not fit. The per-player pools (`players`, `fire team`, `simulation view data array`) are separate: they come from the engine's own allocator object and fall back to 16 when it has no room.

### 2.5 Worked case - the 2026-11-14 beam overflow (proven)

- The merged build expanded three runtime beam pools under the Full profile: `beam_system` 2,809 KB, `beam_location` 3,064 KB, `beam` 3,319 KB - about 9.2 MB inside bank 3.
- Bank 3 went from **58,613,580 to 68,052,044 bytes of 67,043,328**. With no bounds check the overflow silently corrupted game state; every overflowing session ended in its first frame (campaign) or at ~25 s (Sword Base). Six of six sessions with the overflow died.
- Guard (`src/host/PoolResize.cpp`): `BankRoomFor(extra)` reads bank 3's `+0xA0`/`+0xA8` and expands a beam pool only when `extra + 9 MB + 64 KB <= capacity - used`; otherwise it keeps the stock pool and logs `Engine limits: <pool> ... does not fit; keeping stock 160`.
- Proof: after the guard `BANK|3` is 58,613,580 / 67,043,328 and the Sword Base match ran 70 s with captures at 40 s and 60 s, no `GAME OVER`, no `engine stopped drawing`.

Lesson: any new pool expansion must be charged against its bank *before* allocation. "It loaded" is not proof.

### 2.6 Shipped capacity raises

The pinned profile (1.6) ships these proven raises:

| Pool / knob | Stock | Shipped | Basis |
|---|---|---|---|
| AI actors | 96 | **1024** | already exceeded the proven pool hook's headroom; measured live |
| squad | 256 | **512** | ODST precedent (256 -> 335/512) |
| objectives | 64 | **125** | ODST precedent |
| player pools (`players`, `fire team`, `simulation view data array`) | 16 | 16 shipped; 32 only when the dev 32-player slot patch is active | `PoolResize::kPlayerPools` |

Cross-game context: no later MCC engine has more than 16 player slots (Halo 4 grew the per-player record 0x490 -> 0xF18 instead). Reach already matches Halo 4 for particles/effects/decals/lights and exceeds H3/ODST, so those pool raises are not supported by later-game evidence and were deliberately not prioritised. *community/observed via the cross-game decompile comparison.*

### 2.7 Bank rules for new work

1. Never expand a pool without a charge against its bank (see 2.4) and without a stock-size fallback.
2. Log the decision (`BANK|` and the `does not fit; keeping stock` line) so a bad build is visible in one run.
3. A pool that is grown for 32 players must only grow when the 32-player slot mode actually installed; normal sessions must stay byte-identical to stock.

---

## 3. Session and networking

### 3.1 Two player tables

The engine keeps session membership (network view) and the game player table (simulation view) separately and syncs them each frame. Confusing the two is the most common source of wrong conclusions in this project.

**Session membership** (inline arrays in the session object) - *unverified (disasm); addresses traced in runtime filters:*

| Offset | Contents |
|---|---|
| `+0x0060` | 17 x `0x158` peer records (16 players + local/host) |
| `+0x173C` | 32-bit player-valid mask |
| `+0x1740` | 16 x `0xB0` player records |
| `+0x2270` / `+0x394C` / `+0x3950` | second (transmitted) copies of the peer array, mask and player array |

Session player 0's 8-byte identifier is at `+0x1744`, owning-peer index at `+0x174C`. The `+0x173C` mask has a single writer (session constructor) and about 66 readers, almost all in the `0x38xxxx-0x45xxxx` network code.

**The membership wire block** is the same live memory viewed from `session + 0x38`: peer records at `block + 0x28 + i*0x158` (= `session + 0x60 + i*0x158`), player records at `block + 0x1708 + i*0xB0` (= `session + 0x1740 + i*0xB0`). The producer is `FUN_18039DD4C`, called twice per tick from `FUN_180396828` at `0x396A2D` (full message) and `0x396A5C` (delta). *code (verified in disassembly).*

**Game player table** - *code; logged live:*

| Thing | Value |
|---|---|
| Singleton | `haloreach + 0x4E0E910`; mask `+0xA0`, entries `16 x 0xB8` from `+0xA8` (capacity `0x20` when the 32-player patch is active) |
| Session->game sync | RVA `0x41F510`; emits events 0 left / 1 moved / 3 joined / 4 changed |
| `players` pool | `16 x 0x490`, created at RVA `0x43474` |
| `fire team` pool | `16 x 0xD4`, created at RVA `0x122E58` |
| `simulation view data array` | `16 x 0x118` |

Diagnostic from a healthy start (2026-11-14 07:44): `table mask 00000001 ... p0 id 0009000000000001`, `team count: alive 1 teams 1`. From a broken start (merged build, pre-fix): `table mask 00000000 ... id 0`, `team count: alive 0 teams 0`, then `GAME OVER reason 0` in the same frame - the engine ends a session with no valid players/teams. In the 2026-11-16 5-player run the host's game-player table read **capacity 0x20, used 5/5** with per-row datums `EC70..EC74`; that row dump is the reliable way to prove who is actually seated.

### 3.2 Masks and bounds

- Session player mask 32-bit (`+0x173C`), peer mask 32-bit (`+0x5C`), packet player indices/roster masks 16-bit.
- Stock capacity is exactly **16 players / 17 peer records**; 16 players needs no shadow work.
- 16 -> 32 players: keep the 16-entry arrays but redirect every player/peer slot address past them into Noble-owned shadow session objects, widen the 17-bit peer masks, raise loop bounds (see 4.3).
- 32 -> 64/128 needs the mask itself widened (67 read sites plus writers) and a **protocol version bump**. The peer mask being a 32-bit word is the hard 32-peer ceiling.

### 3.3 Framing and the lobby handshake

- **Wire framing:** host slot 41 (`OnNetSend`) prepends an 8-byte machine-id header; slot 43 (`OnNetRecv`) strips it and returns the sender peer id. Without it the engine rejects every join packet (guest stuck at launch state 9). *observed; fixed by Dominus.*
- **Must be unique per machine:** `s_player_identifier` at launch block `+0xF0`; XUID from host slot 88; session id at launch block `+0x50` (`PNCSLOB1` for the closed-source client, `HALOREAC` for the archived open build).
- **Host join path:** join request built at `0x3923DC`, validated by `can_accept_join_request` (`0x3975D8`) -> `can_accept_player_join_request` (`0x3973BC`) -> `find_player_by_identifier` (`0x39D59C`).

### 3.4 Session message types

From the DLL table at `0x42609C`, named in `LobbyPatches.cpp`: 8 join-request, 9 peer-connect, 10 join-abort, 11 join-refuse, 12 leave-session, 13 leave-acknowledge, 14 session-disband, 15 session-boot, 16 host-handoff, 17 peer-handoff, 18 host-transition, 19 host-reestablish, 20 host-decline, 21 peer-reestablish, 22 peer-establish, 23 election, 24 election-refuse, 25 time-synchronize, 26 membership-update, 27 peer-properties, 33 player-properties, 35 (membership ack), 36 security, 44 session state stream, 46 end-game.

**Membership update (wire type 26 = 0x1A):** registered at `0x427E8C`, stock size `0x2550` (9552 bytes), encode `0x42675C` / decode `0x427140`. Peer records at payload `+0x18` (17 x `0x180`), player records at `+0x1998` (16 x `0xB8`), tail `+0x2518..+0x254F`, block checksum at `+0x2548`. Encoder field widths: peer index 5 bits (Noble: 6), player index 4 bits (Noble: 5), peer state 5 bits, slot **2 bits**. The 32-player variant keeps the wire type but grows the payload to **0xA300** (41728 bytes): peers 17..31 at `+0x8100`/`0x180` and players 16..31 at `+0x1998`/`0xB8`; counts/indexes widen as above, the frame grows `0x10CC0`, and the registered size is fixed at runtime when needed. *code + observed (`SEND msg 26 size 41728`, `decoded true`).*

Peer states used on the wire: 5 = connected, 6 = peer-join accepted, 7 = properties received, 8 = player data confirmed ("ready") - see 4.2 for the promotion tests.

### 3.5 Join-in-progress overrides

Noble forces ACCEPT on the engine's refusal paths so players can join an in-progress session:

| Patch | RVA | Effect |
|---|---|---|
| `can_accept_join_request` detour | `0x3975D8` | non-zero refusal -> 0 |
| `can_accept_player_join_request` detour | `0x3973BC` | non-zero refusal -> 0 |
| `acknowledge_join_request` detour | `0x397914` | non-zero reason -> 0 |
| Join-Any gate | `0x397635` / `0x397363` | accept while loading / in simulation |
| `evaluate_coop_insertion_point` | `0x39CE10` | negative save mismatch -> 0 (mission start) |
| `c_network_session::abort` | `0x44EBA0` | suppress co-op incompatibility reasons |
| `boot_peer` | `0x395DE8` | keep guest in co-op |
| `process_host_handoff` | `0x4035DC` | keep host authority |

Refusal codes seen: 15 join-abort, 0x16/22 duplicate identifier, 0x1B co-op save mismatch, 6 match simulation, 5 campaign in-progress, 0x1C peer disconnect, 0x20 Firefight. Session state lives at `+0x60EF8` (6 = in game).

Three host-side fixes shipped in the 0.20.x line (release notes 0.20.5 / 0.20.7 / 0.20.8, implemented in `src/hooks/LobbyPatches.cpp`): the host no longer sends refusal reason 3 for an active Noble session, `AbortGame(reason=6)` is suppressed while a Noble network transport is active (Firefight JIP), and from 0.20.8 both overrides respect the engine's launch join lock (`publicSlots == 0`) so a guest is never forced into a session the engine has locked. *observed (shipped release note; not independently re-tested here).*

### 3.6 Close reasons and refusal semantics

These two log lines were repeatedly misread as causes; they are consequences:

| Log | Meaning | Source |
|---|---|---|
| `CLOSE channel ... reason 10` | host received the guest's out-of-band **connect-closed** (OOB type 7) and tore the channel down | `FUN_180429AA4` case 7 -> `FUN_18043B98C(channel, 10)` at `0x429BEE` |
| `CLOSE channel ... reason 14` (guest) | channel no longer referenced by a session | engine teardown |
| `CONNECT REFUSED: id N reason 3` | **this process sent** a connect-refuse (the hook is on the encoder at `0x42459C`), usually stale reconnect attempts after its session died | sender `FUN_1803F8C60` -> `0x3F8D6F` |
| `boot_peer(17, reason 9)` | host's own seat refusal; this was the high-slot blocker until the seat check was caved (`0fdcf97`, 4.2); the hook remains as a safety net | `src/hooks/LobbyPatches.cpp` |

Reading rule: fix the first divergence (state/table/payload), not the teardown that follows it.

### 3.7 Slot counts and the 16-slot decision

| Fact | Value | Status |
|---|---|---|
| `s_maxMultiplayerPlayers` | constant **128**; nothing ever calls the setter | code |
| Shipped policy (since 2026-11-16) | multiplayer sessions keep the engine's **stock 16 slots**; expansion only with `NOBLE_MP_SLOTS32=1` **and** the 32-player slot patch active, capped at 32 | proven |
| `s_maxCoopPlayers` | 16 (campaign/Firefight) | code |
| The 128 regression | a restored `set_slot_counts` detour expanded every MP/Forge session to 128, breaking *all* joins (host CLOSE 10, guest CLOSE 14, CONNECT REFUSED 3, 4607 setter calls in 75 s). Fixed by keeping stock 16 and no longer overriding `publicSlots == 0` (the engine's join lock). | proven |
| 32-slot dev mode | `-slot-patch -slot-players 32` + `NOBLE_SLOT_PLAYERS_UNSAFE=1`; 120 slot address sites (67 table + 53 member caves) / 23 masks / 139 bounds / membership payload 0xA300; declared-32 session joins with 2 players; the setter still churns ~62x/s (engine re-applies 16) | proven |
| 17-32 seats | usable in the dev high-slot proof: peer 17 / player 16 seats and plays (4.2); a real 17-32 player run is hardware-blocked | proven (dev seat) |
| 33-128 seats | not wired yet: 64 fits the 6-bit peer index and a 16-bit message length; 128 needs wire widening and message framing (`docs/02-engine-reference/TOPIC-scaling-64-128.md`) | code + self-test |

The slot-count hook must be located by a robust fallback (pattern scan first, then the proven RVA `0x39C5C8`): a silent pattern-miss once left the hook uninstalled for weeks, and the later restore caused the 128 regression. Log the resolved source at startup. *proven.*

### 3.8 Dedicated server

`noble-dedicated.exe` remains a **stub**: it loads a JSON config (server name, UDP port 11774, RCON port 11775, map, gametype, `maxPlayers` 32, tick rate, playlist, fast-download URL), speaks its own sockets and advert, and compiles client-only code out with `#ifndef NOBLE_DEDICATED` - but it has **no engine session and no P2P transport**. It therefore cannot stand in as a headless peer for large-session tests. *code + documented in the 2026-11-16 multiplayer status notes.*

### 3.9 Transport and master cluster

- Game packets UDP **11774**, LAN discovery **11775**; direct UDP hole-punch with WebSocket-relay fallback after ~3 s without direct UDP.
- Three federated Render nodes (`projectnobleeast` / `projectnoblewest` / `projectnobleuk`); each merges `/api/browser` with the other two. Endpoints: `POST /api/sessions`, `PUT /api/sessions/heartbeat`, `POST /api/sessions/:id/join`, `GET /api/sessions|/api/browser`, lobby get/join/leave/launch, `DELETE /api/sessions/:id`.
- Build gate: protocol **2**, headers `X-Noble-Build` / `X-Noble-Protocol`, session magic `PNCSLOB1`, relay path `/ws/relay2?protocol=2`; mismatched builds get `403 client_outdated`. The deployed build prefix must match the current `0.20.x` line - **unverified in this refresh**; check the Render env before publishing a release.
- The dedicated stub advertises a `limits_id` so clients can refuse a server whose engine layout differs. The end-to-end gating (backend storage + every join path peeking it) is **not** verified: lobby-code, direct-connect and friend joins never read the host's `limits_id`, so two peers must be told to run the same build manually. *code/unverified; this is a real risk for multi-machine tests.*

---

## 4. Multiplayer workstream (2026-11-16)

Multiplayer large-session support is the front item of the current work order. This section is the standing summary; the newest detailed status is `docs/multiplayer-session-2026-11-16.md`, and the reverse-engineering notes behind it are summarised under `docs/05-research-and-prompts/`.

### 4.1 What works

| Tier | What is proven | Evidence |
|---|---|---|
| Shipped 16-slot engine session | 2-player and **5-player** LAN runs on one machine: host peer mask / player mask `0x1F`, peer states p0..p4 all 8, engine game-player table 5/5, every guest at session state 4 receiving the type-44 state stream, 0 CONNECT REFUSED, 0 crashes | 2026-11-16 stock runs; membership payload is the stock 9552 bytes |
| Join / leave / rejoin | host peer mask grows 0x1 -> 0x3 -> 0x7 -> 0xF -> 0x1F as guests join; a guest killed at t+40 s drops out cleanly and is re-seated on relaunch at t+52 s (new datum, same peer slot), stream resumes | rejoin run + screenshots |
| In-match join (native engine JIP) | multiplayer mid-match join seats the joiner (host table entry with the joiner's flags, guest receives the full type-37 config round including its own identity) | 2026-11-16 diagnostics |
| Session slot hook | resolved by fallback RVA `0x39C5C8`; shipped sessions stay at 16 slots (one setter call in a 90 s run) | 5-player run log |
| 32-player install | all-or-nothing patch installs and runs in single-window smoke tests; declared-32 session joins with 2 players | 114 slot address sites (67 table + 47 member caves) / 23 masks / 139 bounds / 8 step loops / 65 peer-table sites / membership group 16 caves + 26 edits |
| Lobby honesty | implemented and checked in a 2-player LAN run | see 4.6 |

Two hard lessons from getting here:

1. **The 128 -> 16 regression fix is the foundation.** Restoring the `set_slot_counts` detour (after its pattern stopped matching) made every multiplayer/Forge session declare 128 slots, which broke join for *every* player count, not just large ones. The shipped policy now keeps the engine's 16.
2. **Read the direction of the arrow.** `CONNECT REFUSED ... reason 3` is emitted by *this* process; `CLOSE ... reason 10` is the host reacting to a guest that already gave up. Both are downstream of the first table or payload divergence.

### 4.2 High-slot join (peer 17 / player 16): fixed - seats and plays

The high-slot test uses `NOBLE_SLOT_HIGH=1` (dev only): free-slot searches start at peer 17 (`0x3953BC`) and player 16 (`0x39D69A`), and the membership decoder's owning-peer limit is raised 16 -> 31 (`0x4273B0`). This is the gate to real >16 seats.

**Engine state machine (code, verified in disassembly):**

| Step | Where | Condition |
|---|---|---|
| 5 -> 6 / 7 | `FUN_180392DB0`, `0x392F6E..0x392F84` | state 5 and matching id; `CMP ESI,[session+0x40]` selects 7 for the host's own peer, 6 for a joining peer |
| 6 -> 7 | peer-properties handler `FUN_18044ADF4`, `0x44AE7E` | message type 27; sets 7 after properties are received |
| 7 -> 8 | `FUN_180392DB0`, readiness test `0x393191..0x39349D` | every player mapped to the peer must be "ready": mapping == -1 **or** player record `+0x1C != -1` |
| 8 consumed | membership consumer `FUN_18039E47C` | applies the delta; final consistency/checksum check over the block vs payload `+0x2548`; mismatch = `ret 0` + membership reset |

**Fix 1 - membership data (`48cd1d2`, with `c33ef10`):** the type-27 peer-properties handler used stock peer math (`0x39D23E` / `0x39F04E` / `0x39F0C0`): for peer 17 its 0x120-byte property copy overwrote player 0/1 records and set player 0's slot dword to **4**. The wire slot field is 2 bits and the encoder does not mask it, so slot 4 leaked into the preceding 6-bit peer field (host sent `peer 0 slot 4`, guest decoded `peer 1 slot 0`) and the checksum diverged. Fixed with three caves plus a producer-side invalid-slot sanitiser; peer 17 then reached state 8 and the delta was accepted on both sides.

**Fix 2 - the seat path (`0fdcf97`):** the host still called `boot_peer(17, reason 9)` every frame. The culprit was `FUN_18044FC94`, the per-frame duplicate-user/seat check: its loop bounds had already been raised to 32 (SlotBounds `0x44FF31` / `0x44FF8B` / `0x44FFC4`) but its peer stride at `0x44FF46` was stock, so peers >= 17 read player-record memory and the check kept matching the joining peer. Six member caves were added: `0x44FF46` (seat check peer stride) plus player-add acceptance `0x397424` / `0x397503` / `0x39752A` and queued player-add apply `0x39FB62` / `0x39FB7D`. The patch set is now **53 member caves / 120 slots address sites**, all byte-checked against the stock DLL.

**Result (2-instance `NOBLE_SLOT_HIGH=1` run, 2026-11-16):** host `peermask 00020001 pmask 00010001`, peer states `p0:8 p17:8`, **0 BOOT lines**; membership producer sends peer 17 state 8 + `plr[0] idx 16`; guest consumer `ret 1` on every delta; host `game session started` 13:58:43 and guest 13:59:02; no CLOSE 10, no CONNECT REFUSED, no crash. The guest plays as peer 17 / player 16.

**Multi-high-slot probe (4 instances):** two high peers seat simultaneously (host peer mask `0x00060001`, states `p0:8 p17:8 p18:8`); the third guest fell back to a normal slot and channels closed. Whether that is the same-PC shared identity being treated as a duplicate user or a real limit is still open (retest with distinct accounts).

**Still open, carried from the inventory:** the identity of the stray slot writer (the producer sanitiser neutralises it), the 17-entry session parameter at `session+0x60A60` and a handful of fixed 16/17-entry tables (4.7); none of them block the proven single-seat path.

### 4.3 32, 64 and 128 players - plan status

**32 players (milestone 3) - dev-gated, hardware-blocked end-to-end.** The 32-player patch set is large and byte-checked all-or-nothing; a session can be *declared* with 32 slots and joins with 2 players. Two problems remain: (a) the high-slot seat path above, (b) the host calls `set_slot_counts` about 62 times per second because the engine re-applies 16 and the hook re-expands to 32 - this needs the engine's own session-creation slot count raised (or a one-shot latch) rather than post-hoc expansion. A real 32-player run also needs 32 engine sessions, which is far beyond one machine (4.5).

**64/128 players - capacity model done, wiring is the work.** A constexpr shadow-layout model plus a headless self-test (`-test-slot-capacity`, commits `1fbd952`/`0ebd1a2`) validates the three capacities without any engine or clients:

| capacity | shadow blocks/session | extension bytes | message size | mask words | peer index bits | player index bits |
|---|---|---|---|---|---|---|
| 32 (current) | 1 | 0x2200 | 0xA300 | 1 | 6 (patched) | 5 (patched) |
| 64 | 2 (A/B) | 0x6900 | 0xEA00 (fits 16-bit length) | 2 | 6 (at the field's ceiling) | 6 (needs widening) |
| 128 | 2 (A/B) | 0xF700 | 0x17800 - exceeds 64 KiB, needs framing | 4 | 7 (needs widening) | 7 (needs widening) |

Layout decision (validated by `-test-slot-capacity`): use the **chunked** shadow layout above 32 rather than per-site A/B block tagging (the generic helpers take a block pointer and run for both A and B, so one constant cannot point at two shadow blocks). Chunks keep the +0x2210 A/B relationship and every cave stays single-constant per index range; the self-test now also simulates the dispatch for every index on both block bases and checks chunk placement: 64 = 2 peer / 1 player chunk (shadow end 0xEAA4, worst cave 53 B), 128 = 5 peer / 3 player chunks (shadow end 0x21A14, worst cave 140 B). Cost: member-cave entry grows from 48 to >=160 bytes and the step loops/mirrors must hop chunks. What remains is the ordered list in `docs/02-engine-reference/TOPIC-scaling-64-128.md`: chunked-layout install, dev-gated capacity install, membership encoder/decoder widening (peer/player index bits, multi-word masks, game-player event index), the 128 message-size/framing question, pool/session-slot-count raises, setter-churn fix and the UI/harness generalization. 64 is the natural next ceiling because the current peer-index field tops out at 64; no later MCC engine has more than 16 player slots, so this remains original architecture work.

**Reach Renaissance (RR) reference - use the real numbers.** The external RR project (`v0.1.50` at the time of study) is an existence proof and a UX reference only; no RR code, data, assets, strings, profile names or ports may be copied. Facts established from a read-only study of its install and Tyler's existing run logs:

| Fact | Value |
|---|---|
| RR's practical maximum | **81 players** (the M81 profile, one player per copy of the game) - the UI exposes 1..81 |
| RR's 128 | an engine profile (`m128`, two players per machine / 64 machines) that exists in code but is **not exposed** above 81 |
| "88 players" | **not found anywhere in the install**; the reported 88 is most likely a misremembering of 81 (or a blend of 81 and 128). Do not repeat "88" as a fact |
| Named profiles | `default` (stock 16/32), `m64`, `m81`, `ff32` (Firefight 32), plus an unexposed `m128` |
| MAX PLAYERS semantics | a plain 1..N stepper where the number selects the named engine profile; the overlay holds the game to the count |
| Honesty mechanics | listings carry `players / max_players` as first-class fields, "?" when unknown, freshness thresholds (STALE 30 s / NOT HEARD 75 s / DROP 120 s), FULL verdicts, "Lock Game" |
| Caveat | RR's own changelog says the 80-player dedicated path "hasn't been played with a full lobby yet" - treat any count above 64 as unproven even for RR |

### 4.4 Harness and tools

`tools\mp-session-test.ps1` is the N-instance LAN harness used for every multiplayer proof. It launches the host from `build\bin\Release` and guests from `Release2..ReleaseN` (creating skeletons with junctions into `Release\data` when absent), syncs every guest exe to the host hash, applies low-spec settings (1280x720, 30 fps, muted) on the secondary monitor, installs a focus guard so test windows never take the user's input, aborts on a RAM ceiling, and writes `<tag>-summary.txt` plus a machine-readable `<tag>-result.json` next to the host log.

| Switch | Use |
|---|---|
| `-Players N` | total players including host |
| `-Slot32` | `-slot-patch -slot-players 32` + `NOBLE_SLOT_PLAYERS_UNSAFE=1` |
| `-High` | `NOBLE_SLOT_HIGH=1` (peer 17 / player 16 free-slot test) |
| `-MaxPlayers N` | session target (`-max-players`) |
| `-Map` / `-Mode` | default `20_sword_slayer` multiplayer |
| `-Seconds` / `-HostWait` / `-GuestStagger` | timing |
| `-LeaveAt` / `-RejoinAt` / `-LeaveGuest` | hard-kill + relaunch evidence for leave/rejoin |
| `-CaptureAt` / `-ScoreboardAt` | screenshot capture without taking focus |
| `-LowMem` | dev-only 640x360 borderless template + Stock-limits probe + free-RAM guard (measured 4.5) |
| `-RamAbortGb` / `-MinFreeGb` | RAM guards |

Other harnesses: `tools\lan-two-instance.ps1` / `tools\lan-three-instance.ps1` (older patterns), `tools\coop-test.ps1` (campaign), `tools\test-modes.ps1` (mode smoke tests), `tools\limits\test-limits.ps1` (pool/limits).

Environment variables that matter for multiplayer tests: `NOBLE_WINDOW_SECONDARY=1` (put game windows on the secondary monitor), `NOBLE_SLOT_PROBE=1` (log-only membership producer/consumer dumps), `NOBLE_SLOT_HIGH=1`, `NOBLE_SLOT_PATCH=1` / `NOBLE_SLOT_PLAYERS=32` / `NOBLE_SLOT_PLAYERS_UNSAFE=1`, `NOBLE_MP_SLOTS32=1`, `NOBLE_SLOT_MSG_OFF=1` (bisect: leave the 32-player message group out), `NOBLE_MACHINE_ID` (test identity), `NOBLE_TRACE_MEMBERS`. `NOBLE_ENGINE_LIMITS_PROBE=0` is accepted only in dev builds.

### 4.5 Hardware ceiling (measured)

| Configuration (one instance) | Peak working set | Peak private commit |
|---|---|---|
| Low-spec 1280x720 borderless, Full limits (shipping pin) | 5.8 GB | 5.1 GB |
| 640x360 windowed, Full limits | 6.1 GB | - |
| 640x360 borderless, Full limits | 5.5 GB | - |
| 640x360 borderless, Stock limits (dev low-mem probe) | 5.0 GB | 4.74 GB |

- Resolution and window mode do **not** reduce engine memory (world/geometry/tag data dominates; named engine pools total only 0.08 GB). The dev-only Stock-limits probe saves about 0.5-0.8 GB (~10%).
- Five in-match instances peak at **22-24 GB of 31.8 GB installed**. 8 instances hit the 1.5 GB free-RAM guard at 30.5 GB used; 6 hit the 1 GB guard at 31.65 GB.
- Therefore ~5 GB per client: **16 players need roughly 76-95 GB**, against 31.8 GB installed. On this machine the proven maximum is **5 players**. Page-file-only operation would thrash and is not a usable acceptance environment.
- Acceptance options (decision needed): (a) **3-4 PCs x 4-5 instances**, recommended - the harness already accepts `-connect <host-ip>`; (b) a RAM upgrade to 96 GB+ (128 GB comfortable); (c) use the 32-slot patch to seat 5 players in high slots as the extended-slot proof before doing true 16/32 runs across machines.

### 4.6 Lobby, picker and server-browser honesty (implemented)

Previously three surfaces lied: the picker offered 24/32 while the engine stayed at 16, the lobby header and adverts published the raw pick, and a 17th player could not join. The implemented model:

- **One capability source.** Effective multiplayer seats = **16** stock, or `min(requested, 32)` only when the dev 32-player slot patch is genuinely active. Every surface reads it.
- **Picker.** MAX PLAYERS keeps 4/8/12/16 (16 default); 24/32 stay visible but greyed with a one-line reason and are skipped by navigation; a line under the row states the seats the session will declare.
- **Lobby / adverts / beacon.** Header count, session advert and LAN beacon use the effective cap; the LAN beacon shows the real player count; `-max-players` is clamped and published as the session target.
- **Roster / scoreboard.** Row count now comes from the live engine game-player table (and transport peer count), not the pre-game master roster; a 2-player LAN run shows 2 rows and logs `live roster 2 player(s)`.
- **Server browser.** Real player counts, FULL / STALE / NOT HEARD LATELY / 17+ UNSUPPORTED verdicts, and JOIN is blocked on a full row.

Known remaining UI gap: guest display names on direct/LAN launches are synchronised display names only after the type-27 identity data is wired into the roster; until then they read `PLAYER n`. *code/observed.*

### 4.7 Remaining blockers and the next concrete steps

1. **Multi-high-slot validation.** Two high peers seat at once; the third guest still falls back/closes. Retest with distinct accounts/profiles (the multi-instance harness shares one identity) before treating it as a limit, and extend the forced-slot test to peers 18/19/20.
2. **Name the stray slot writer.** A hardware write watchpoint on `session+0x1750` (player 0 record `+0x10`) plus hooks on the player-add helpers (`0x39D8A8`, `0x39D620`, `0x39FB34`) names the writer; the producer sanitiser only neutralises the symptom.
3. **Grow the 17-entry session parameter** at `session+0x60A60` properly: storage, the `0x110`-byte stack copies in `0x3FF588`, getters and its network encoding; delete the "not set for peer 17" stopgap. Re-check the fixed 16/17-entry tables (`0x41F510`, `0x40A760`, `0x41E884`, `0x41FBE8`, `0x3C0248`) once the trace points at them.
4. **Stop the set-slot-count churn** (~62x/s): raise the engine's own session-creation slot count (or latch the expansion) instead of post-hoc re-expansion - needed before any 32/64/128 run.
5. **16-player real acceptance:** same build and layout id (`0x03010400`) on 3-4 PCs x 4-5 instances; verify the pinned-limit gating on the real join path first (3.9).
6. **64/128 capacity work:** follow `docs/02-engine-reference/TOPIC-scaling-64-128.md` (A/B site classification, dev-gated install, wire widening, 128 framing, pools, UI). Validate every step with `-test-slot-capacity`, install-level declared-capacity sessions and the forced high-slot tests until real clients are available.
7. **Dedicated server** is still a stub (no engine session), so headless large-session testing remains impossible without the multi-PC rig.

### 4.8 Pass / fail signatures

| Result | What to look for |
|---|---|
| Join worked (normal slot) | host `SEND msg 26` + `RECV msg 27/35`, producer 2 peers/2 players, both sides `game session started at frame N`, guest `launch 11 map 2 (running)`, type-44 stream |
| Join failed | only messages 8/22/25 and no 26/27/35 |
| High-slot pass | **achieved 2026-11-16**: host producer stays at 2 players after the ack, peer 17 state 8 on both sides, delta `ret 1`, no BOOT lines, no CLOSE 10 / REFUSED 3, both sides `game session started` (leave/rejoin at peer 17 not yet re-tested) |
| Fail signatures | `CLOSE ... reason 10`, `CONNECT REFUSED ... reason 3 (hr+0x423E2E)`, guest `map 0 (none)`, `FATAL EXCEPTION` / `0xC0000005` / crash dump |

---

## 5. Models, armour and animation

Armour customisation is Noble's highest-visibility feature and the reason most of the engine/model research exists. The shipped state as of v0.20.0: full Spartan and Elite customisation (including Honor Guard, Counselor, H2A Ranger, H2A Armor and Schism sets), armour abilities visibly mounted on Elites, custom visor colours in-match and networked, and customisation replicated to other clients. *observed (release notes + shipped code).*

### 5.1 Player representations (matg)

The globals tag holds one `0x6C`-byte record per player character. Noble rewrites this block live to swap the player's biped and to find the Elite's first-person models.

| Field | Offset | Confidence |
|---|---|---|
| representation count | `matg +0x474` (i32; 3 in campaign, >=2 in MP) | observed |
| representation pointer | `matg +0x478` (packed u32) | observed |
| record stride | `0x6C` | observed |
| first-person arms model name / datum | `+0x14` / `+0x20` | unverified (comments) / observed |
| first-person body model name / datum | `+0x24` / `+0x30` | unverified / observed |
| third-person biped name / datum | `+0x34` / `+0x40` | unverified / observed |

Swapping the representation's third-person biped datum (`+0x40`) is how the engine is made to render a different character; ElitePack matches representation 1's `+0x40`.

### 5.2 The model chain

| Tag | Offset | Field | Confidence |
|---|---|---|---|
| `bipd` | `+0x70` | model (`hlmt`) datum | observed |
| `hlmt` | `+0x0C` | render model (`mode`) datum | observed |
| `hlmt` | `+0x1C` | collision model datum | unverified |
| `hlmt` | `+0x20` / `+0x2C` | animation graph (`jmad`) reference / datum | observed |
| `hlmt` | `+0xC0` | "new damage info" block (element `0x140` B; `+0x28` body vitality, `+0x7C` shield vitality) | unverified; values observed |
| `hlmt` | `+0xD0` | model shield strength (float) | observed |
| `hlmt` | `+0xE0` | damage sections (`+0x00` section name string id, e.g. "body") | unverified |
| `mode` | `+0x0C` | regions block `{count, ptr, u32}` | observed |
| region | stride `0x10` | `+0x00` name string-id, `+0x04` permutations block | observed |
| permutation | stride `0x18` | `+0x00` name string-id, `+0x04..` mesh/flair data | observed |

### 5.3 How Noble applies armour

The engine primitive is `set_region_permutation(object, region string-id, permutation string-id, flags)` at RVA **`0x4729F4`** (region lookup `0x1CBF0C`, permutation lookup `0x1CBF98`, notify `0x4730CC`). Colours use **`0x473858`**; a variant reset uses **`0x472B70`**.

- **Armory (menu map):** an Armory item is not a permutation - the customisation globals (`pmcg`) translate it into `(region, permutation)` pairs. Noble loads that table from `data\armor\*.rrpmcg` (format: `L <list> <count>`, `S <item>`, `C <region> <permutation> <flags>`; embedded fallback `src/res/spartan.rrpmcg`). Lists: 20 helmet, 2c/38 shoulders, 44 chest, 50 wrist, 5c utility, 68 knees, 80 Spartan effects, 74 Elite sets, 8c Elite effects. The menu script consumes 16 pairs from its GLOBALS table and applies them to the pedestal biped.
- **In-match (Marine/Marine-style pieces):** the Armory Marine tab saves `{region, permutation}` pairs; the sync reads the scenario map file's own string tables for region/permutation ids, confirms the region exists on the unit's model (`0x1CBF0C`), then applies per pair. It runs for ~3 s after spawn or a pick change, skips sessions with peers (lockstep safety), and the pieces go on the **simulation thread** (`NotifyMatchTick`) because the render thread's write is overwritten by the per-frame sim->render copy at `0x1CD389` (inside `0x1CD2BC`). Stock MCC maps are compressed, so the string-id lookup finds 0 names there; ElitePack carries the ids.
- **ElitePack (pack injection):** the pack's pmcg has no Elite changes and the engine keeps the map's pmcg, so Noble appends the pack Elite's permutations onto the map's six armour lists. The pack Elite's render model has 9 regions (permutation counts `{6,1,9,9,1,11,11,9,11}`); the six piece slots map to region indices `{5,6,8,2,3,7}` (helmet, left/right shoulder, chest, forearms, legs). First-person arms/body are dressed separately: each fp model's `armor_pieces` region gets the wanted set's mesh copied into every permutation.

**Species specifics.** Elite (representation 1): pmcg Elite set order Minor, Spec Ops, Ranger, Ultra, Zealot, General, Field Marshal, Officer (plus Honor Guard / Council / Ossoona / Schism / Combat as extra pieces). First-person arms follow the Armory Arms piece, body follows shoulders; pieces with no fp version fall back to the Armor Set. Change-colour slots 2/3 are engine-fixed, not player picks. Marine: the campaign/Firefight biped is `objects\characters\marine\marine` with model `marine.mode` and animation graph `marine.jmad`; render-model regions are `helmet` (10 perms), `head` (6), shoulders L/R (4), `chest_a` (4), `chest_b`, `backpack` (11), `belt`, `neck`, `thighs`, `legs` (4), with 4 change colours; flair pieces are **instanced geometry** (placements with per-permutation masks), not merged meshes. `marine_female` has fewer pieces and is the only one with a `flair_helmet_a` region. Spartan (representation 0): default pieces through the pmcg lists; gender is a `body`/`arms` permutation switch (`BipedSetGender` `0x49BB84` for the live biped); the prosthetic arm maps to `kat` (female) or `player_john117` (male) - never the raw `robot` string (crash `0xC0000005` at `0x6950A9`).

**Appearance pipeline state.** Visor colour ships through a mapped stock-slot path (the engine exposes five visor slots; the shipped item -> slot translator is `0x3B2F4`, with direct patch `0x2EDC5E` and the shader-parameter slot route at `0x24334C` where `visor_tint` is slot 6 -> `0x8018f`). Exact per-player custom RGB visors visible to all peers are networked in v0.20.0; the remaining work is armory-side selectable armour-effect colour variants (recolour is proven - blue flame shipped - but selectable per-item variants need engine effect-table work) and Elite lighting/shine. *code/observed; see `docs/05-research-and-prompts/RE-VISOR-COLOR-2026-11-15.md` and `RE-ARMOR-FX-COLORS-2026-11-15.md`.*

### 5.4 Animation graphs

The animation tag is the model animation graph (`jmad`); each biped references its own at `hlmt +0x20`. Noble touched these only when installing the Pelican (replacing the `marine`, `spartans` and `spartans_ai` graphs in HREK). The community pipeline uses Bonobo/Baboon for extraction; the one concrete "no animation graph" fix is a **node-name** match (bone and vertex-group names must match the target skeleton). No public list of Reach region/permutation/marker names exists to cross-check Noble's string-id lists.

### 5.5 Historical: the campaign "Trooper" marine-legs investigation

An earlier campaign feature swapped the player's biped to a campaign marine; with it the Marine rendered folded at the hips with no legs while its shadow stood normally. The region/permutation hypothesis was rejected by forcing every region permutation to 0 (15/15 applied, `rc 0`); borrowing the Spartan animation graph killed the player 7 s after spawn; the community fix was custom uncompressed campaign maps carrying a player-capable marine biped (`marineplayer2`). That feature has since been **removed** - campaign is now vanilla/modded rather than a live biped swap - so the legs defect is closed as "feature removed", not fixed. Do not reopen it without a new player-capable marine animation graph. *observed/proven in the 2026-11-14 captures.*

---

## 6. Forge and the pack pipeline

### 6.1 Two palettes - do not confuse them

| Palette | Where | Edited by | Served by |
|---|---|---|---|
| Noble's own Forge menu | `data\forge_palette.json` | hand/script | `src\ui\ForgeMenu.cpp` (ImGui overlay, INSERT/F3) |
| Native in-map Forge palette | the scenario's **map variant palettes** block | ManagedBlam scenario scripts + HREK `tool.exe build-cache-file` | VFS, from `data\maps\forge_halo.map` |

Noble's menu is a 13-tab JSON list (`SPAWNS, GOALS, WEAPONS, VEHICLES, EQUIPMENT, SCENERY, BUILDING BLOCKS, GADGETS, LIGHTS, NATURAL, UNSC INFANTRY, COVENANT INFANTRY, SPECIAL & BOSSES`) with no airstrike entries; each item names a tag path plus `max`/`price`. The native palette is what Reach's real Forge menu reads: one palette per category (`ff_weapons_human`, `ff_weapons_covenant`, `ff_vehicles`, plus MCC's hidden `ff_thorage_*` / `ff_cex_*`), each entry `{name, maximum allowed, price per instance, variants[]}`, each variant `{display name, object tag path, variant name}`.

### 6.2 Pipeline

1. Edit the scenario with ManagedBlam: add/duplicate palette entries (boathog, human airstrike, Covenant airstrike), dump palettes read-only. Scripts must be **idempotent**: duplicate an existing entry with `DuplicateElement`, never `AddElement`.
2. Rebuild with HREK `tool.exe build-cache-file` for the target scenario; a Forge World rebuild takes about 2-4 minutes.
3. Install the built map into `build\bin\Release\data\maps\<scenario>.map` (plus any `fmod` bank such as `sfx.st_spectre.fsb`).
4. Verify: the log must show `VFS: serving Noble's '<map>' in place of MCC's`, then test Forge from `data\maps`.

Lesson from the Pelican install: a vehicle that is only in a map-variant palette builds, spawns and drives but renders **invisible** (its resources are not streamed) - it must also be in the scenario's vehicle palette. Append palette entries; map variants store palette indices, so inserting shifts them.

### 6.3 Worked case - the Covenant airstrike palette (proven)

A script added entry `target_laser_covenant` to `ff_weapons_covenant` (copied from `wep_plasmalauncher`; max 8, price 150) with seven variants: `corvette_bombing`, `seraph_run`, `spirit_ghost`, `spirit_wraith`, `glassing_single`, `halo_array`, `game_ender` - all under `custom\weapons\target_laser\variants\`. The rebuilt map was ~796.8 million bytes, installed, and confirmed in Forge by hand. Before this the Covenant weapons palette had no target-laser entry at all. *proven.*

### 6.4 ElitePack build and shrink

The Elite pack is an **extra cache** loaded beside the map by `ElitePack.cpp`: the map-load request's extra-cache list is rewritten to add the pack (loader `0xA38A3` / `0x141C0`, extra loader `0x31078`, header slot `0xAA634`, per-cache step `0x30698`), and a relocation sidecar (`.rrreloc`, magic `RRRL`, packaged as the Noble-named `.noblereloc`) rebases the pack's packed pointers after load.

Shrink (486 -> 284 MB, A/B proven): build from a minimal scenario (a copy of `levels\test\box`, type multiplayer, palettes/squads/scripts/HS cleared). Leaving palettes in carried civilian and marine bipeds, which made the shield heuristic bail ("shields too close"). Deflate was not the route (16% on the old pack on the old format).

Note on naming inside the closed-source repo: the shipped pack is `data\armor\elite_armor.map` (+ `elite_armor.map.noblereloc`); the `rr_*` names belong to the external reference project and do not ship. *code (publish-release.ps1).*

### 6.5 Pelican / Spectre / Boathog

All HREK-only content (never extracted from MCC), installed with SHA-256 manifests and backups under `HREK\noble_mods\<mod>\`.

| Item | Origin | Scenario edits |
|---|---|---|
| Pelican (flyable) | Pepper's Pelican Overhaul (PepperMan) | vehicle palette 11 `pelican_gear_scripted_doors`, 12 `pelican_door_scripted_doors`; `ff_vehicles` entry 9 `pelican`; replaces 3 animation graphs + 3 shaders |
| Spectre (H2A T-46) | Spartan Tanner | 101 files; vehicle palette 13; `ff_vehicles` entry 10 `spectre` (5 variants); sound bank `sfx.st_spectre.fsb` via VFS |
| Boathog | Spartan Tanner | `ff_vehicles` entry `boathog` (4 variants: default/gauss/rocket/troop) |

Pelican doors: **X** toggles scripted object function 3, global across all scripted Pelicans (online sync untested). This is the only known control that works all Pelicans at once.

### 6.6 Add-on pack instead of the 796 MB map (plan, unbuilt)

Current, proven route: the rebuilt Forge World ships as `data\maps\forge_halo.map` (about 760 MB uncompressed) as a **separate optional release asset** so players who never forge do not download it. The alternative route: a small add-on pack injected into any map's Forge palette the way ElitePack injects a cache, so the airstrike and vehicles come from a pack rather than a full map replacement. No design or code exists beyond the observation that ElitePack's extra-cache + relocation route may be reusable. *unverified.*

### 6.7 Forge shipping facts

- `noble_menu.map` is the rebuilt menu map that ships in the base package (Armory scripts, GLOBALS tables, Spartan/Elite/Marine/Female Marine previews; VFS serves it as `haloreach\maps\mainmenu.map`). The older `mainmenu_all.map` (~285 MB) is **no longer shipped**. *code (publish-release.ps1).*
- The rebuilt Forge World is a separate optional asset (`ForgeWorld` zip); everyone in a Forge World lobby must have the same pack.
- A vehicle or weapon added only to a map-variant palette will not stream; always add it to the real palette.

---

## 7. Tooling and runbook

### 7.1 Build

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --target noble-dev noble-client noble-dedicated
```

`build.bat` (release client) and `build-dev.bat` (dev client) wrap the configure/build, locate CMake/VS (including a Ninja + standalone-MSVC fallback), kill any running Noble process, copy `data`, `assets` and `tools\vgmstream` into `build\bin\Release`, and never pull git. New `.cpp` files must be added to `CMakeLists.txt` (client list, and `DEDICATED_SOURCES` when shared). Release gates are `noble-dev` + `noble-client`; `noble-dedicated` and `mvar-tool` build but do not gate.

### 7.2 Test harness

| Script | Use |
|---|---|
| `tools\mp-session-test.ps1` | N-instance multiplayer LAN runs, leave/rejoin, captures, RAM guards, evidence JSON (4.4) |
| `tools\lan-two-instance.ps1`, `tools\lan-three-instance.ps1` | older multi-instance LAN patterns |
| `tools\coop-test.ps1`, `tools\test-modes.ps1` | campaign co-op and mode smoke tests |
| `tools\limits\test-limits.ps1` | engine-limit / pool raise tests (pool report evidence) |
| dev test folder (`camp_test.ps1`, `match_elite.ps1`, `forge_test.ps1`, `armory_preview.ps1`, `menu_test.ps1`) | single-instance captures; not part of the repo |
| ManagedBlam + HREK (`runmb.ps1`, `build_map.ps1`, Reclaimer `rc_*.fsx`) | tag/scenario/palette work and map builds (6.2) |

### 7.3 Dev-only switches and probes

| Variable / flag | Effect |
|---|---|
| `NOBLE_WINDOW_SECONDARY=1` | game windows on the secondary monitor (all test scripts set it) |
| `NOBLE_SLOT_PATCH=1`, `-slot-patch` | install the 32-player slot patch (dev only) |
| `NOBLE_SLOT_PLAYERS=32`, `-slot-players 32` | request 32 declared slots; requires the unsafe override |
| `NOBLE_SLOT_PLAYERS_UNSAFE=1` | acknowledge that 32-player mode is unverified (mandatory today) |
| `NOBLE_MP_SLOTS32=1` | allow the session slot branch to expand to 32 instead of stock 16 |
| `NOBLE_SLOT_HIGH=1` | free-slot scans start at peer 17 / player 16 (high-slot test mode) |
| `NOBLE_SLOT_PROBE=1` | log-only membership producer/consumer and player-remove attribution dumps |
| `NOBLE_SLOT_MSG_OFF=1` | leave the 32-player membership message group out (bisection) |
| `NOBLE_ENGINE_LIMITS_PROBE=0` | dev builds only: run the Stock engine layout (used by `-LowMem` tests) |
| `NOBLE_MACHINE_ID` | override the test machine identity (unique per instance) |
| `NOBLE_TRACE_MEMBERS` | verbose membership tracing |
| `NOBLE_DEV_BUILD` | compile-time dev build (enables dev-only probes, log, `.map`) |

Historical/removed switches (do not rely on them): `NOBLE_MODEL_DUMP`, `NOBLE_TROOPER_*`, `NOBLE_TEST_SKIP_CINEMATICS` - these belong to the removed campaign Trooper feature and its capture tooling.

### 7.4 Release packaging (including the caveat)

`tools\publish-release.ps1` parses the version from `src/common/Version.hpp`, optionally rebuilds `noble-client`, stages a portable package and zips it as `Project-Noble-v<version>-win64.zip`. What it ships: `Noble.exe`; `data\limits\full.nlp` (**required** - the script throws if missing, because the pinned layout cannot run without it); `data\armor\elite_armor.map` + `.noblereloc` (**required**); `data\menumaps\noble_menu.map` plus its string-id tables (**required**); `tools\vgmstream`. It strips dev binaries, PDBs, userdata, saves, `appearance*`/`career*`/`settings*`, logs, dumps and all `.map` files.

Caveats:

- GitHub's 100 MB repository limit is why the large data is not in git: the required armour pack and menu map are staged **into the base release zip**, while the rebuilt Forge World (~760 MB uncompressed) travels as a **separate optional `ForgeWorld` asset**. A base-only release must still attach the Forge asset or Forge World breaks for players.
- `-NoForgePack` skips the Forge World asset deliberately; use it only for a base-only build.
- The version in `Version.hpp`, the release note in `release-notes\v<version>.md`, the in-client changelog (`src/ui/ChangelogData.hpp`) and the zip asset must all agree. Never publish `noble-dev`. Publishing needs an explicit decision; the release repo is `O7OF/Project-Noble-`.

### 7.5 Release build hardening

`noble-client` ships with no `.map`/`.pdb`, whole-program optimisation and COMDAT folding (`/GL /LTCG /OPT:REF /OPT:ICF`), RTTI removed (`/GR-`), no log file in release (`Logger.hpp` compiles log macros out), a minidump-only crash handler, and compile-time string obfuscation (`NOBLE_OBF` / `NOBLE_OBFW` in `src/common/ObfuscatedString.hpp`) for endpoints and engine literals. No packer or anti-debug is applied by design. Verification commands and the rationale are in `docs/ANTI-REVERSING.md` (run `noble-client.exe -crash-test` and confirm only `noble-crash.dmp` appears, no log, no symbols).

### 7.6 Test discipline (non-negotiable)

- Never test while Tyler is playing: check `Get-Process noble-dev, noble-client` first; one game instance at a time on the shared rig.
- Put windows on the secondary monitor (`NOBLE_WINDOW_SECONDARY=1`) and never take the user's mouse; scripts launch unfocused and the harness returns the foreground if a game grabs it. F5 needs focus - ask first.
- Back up and restore `build\bin\Release\data\appearance.dat` around every test (scripts do; check hashes).
- Prove every cause with a **one-variable** test plus a capture or log; then actually look at the capture. Shrink or crop images over ~3.7 MB.
- Make sure the test cannot undo the thing under test (a scripted F5 once turned third person off), and know who you are looking at (a third-person "Spartan" was Kat).
- Object writes from the render thread are overwritten by the sim->render copy (`0x1CD389`): apply state on the simulation thread.
- Never modify the installed `haloreach.dll` or MCC's maps. HREK edits get backups under `HREK\noble_mods\`.
- After every menu build, refresh the menu map in `data\menumaps` and its string-id CSV.

### 7.7 Diagnostics vocabulary

- `BANK|n|used|capacity` - game-state bank usage; a bank at capacity after a pool change is the beam-overflow signature (2.5).
- `POOL|name|...|live` - named pool report; use it to prove a capacity raise actually allocated.
- `SlotPatch:` lines - one per install group; the all-or-nothing byte check refuses 32-player mode and logs why.
- `LobbyPatches:` / `membership producer` / `membership consume` lines - message trace (3.4, 4.8).
- `ReachHost [State]:` - launch state, current map, frame number; `map 0 (none)` means the guest never loaded one.

### 7.8 Slot-patch table generation

The slot patch is generated, not hand-written: `tools\limits\gen_sites.py`, `gen_bounds.py`, `gen_caves.py`, `gen_member_caves.py`, `gen_member_known.py`, `gen_steps.py`, `gen_table44.py` emit the `.inc` tables consumed by `src/host/SlotPatch.cpp` / `SlotMessage.cpp` (the 32-player membership message group is generated by the same limits tooling). Every site is installed with a load-time byte check and the whole set is all-or-nothing (a wrong DLL build turns 32-player mode off instead of corrupting it); 16-player mode must stay byte-identical to stock. Regenerate and rebuild after any DLL update; never edit the `.inc` files by hand.

---

## 8. Bug archaeology and open questions

### 8.1 Worked cases

| Symptom | Cause | Fix / outcome |
|---|---|---|
| Campaign session ended in its first frame; Sword Base match ended ~25 s in; `GAME OVER reason 0` right after `game session started` | runtime beam pools added ~9.2 MB to bank 3 (58.6 -> 68.05 of 67.04 MB); unbounded bump allocator corrupted state | `BankRoomFor` guard in `PoolResize.cpp`; proven (2.5) |
| Every multiplayer join broke (including 2 LAN instances): host CLOSE reason 10, guest 14, rejoin REFUSED 3 | restored `set_slot_counts` detour expanded every MP/Forge session to 128 declared slots | keep stock 16; expansion only under the dev 32-slot mode; proven by the 2- and 5-player runs |
| High-slot guest rejected the membership delta (host sent `peer 0 slot 4`, guest decoded `peer 1 slot 0`) | type-27 peer-properties handler used stock peer math, writing into player 0/1 records and corrupting player 0's slot (2-bit wire field, unmasked encoder) | 3 caves + producer-side invalid-slot sanitiser; peer 17 now reaches state 8 and the delta is accepted (4.2) |
| High-slot peer booted with `reason 9` every frame; player 16 never kept its seat | `FUN_18044FC94` duplicate-user/seat check read the peer record with stock 0x158 math, so peer 17 read player-record memory and matched | 6 member caves (`0x44FF46` + player-add acceptance + queued apply); peer 17 seats and plays (4.2) |
| Scoreboard/loading roster showed 1 row while 5 players were in the session | rows were seeded from the pre-game master-server roster, absent in LAN/direct sessions | roster/loading surfaces now size from the live engine game-player table; 2-row LAN proof |
| Settings reset to defaults on launch | start-up migration copied the newest mirror (`Documents\Noble Launcher\data`, `%APPDATA%\ProjectNoble\data`) back over the saved file | first-run-only migration, atomic per-process save; shipped v0.20.2/v0.20.6 |
| Firefight join-in-progress dropped the joiner to the main menu | host sent refusal reason 3 and/or `AbortGame(reason=6)` 11 ms after the joiner's channel closed | refusal-3 drop for active Noble sessions (v0.20.5) and logged `AbortGame(reason=6)` suppression (v0.20.7) |
| AMD Ryzen systems far below Intel on the same GPU | Windows power throttling (EcoQoS) and default priorities | opt out of EcoQoS, above-normal process priority, raised render thread, keep-awake (v0.20.4) |
| Campaign co-op 5 players: session forms then every instance exits before the mission starts | engine takes the network session-start path and closes every peer channel with reason 10 after the launch-data/membership comparison | launch caps lifted, failure isolated; feature **parked** (do not resume without direction) |
| Marine pieces looked broken in Firefight | the test pieces (`space_marine`) looked like the default from behind | test with obviously different pieces (`odst_light`); proven working |
| Trooper mode: scripted F5 turned third person off | the test script toggled Noble's third person | `LockThirdPerson`; captures confirmed |
| Covenant weapons palette had no airstrike entry | only the human palette had target-laser variants | added `target_laser_covenant` + 7 variants, rebuilt the map; confirmed in Forge (6.3) |
| "Numbers shown as hex" UI strings (`0000001c`, `Respawn in 00000001`) | unresolved; static candidate `FUN_1802d288c` (`hr+0x2D288C`) leaks 8 hex digits on its failure path | open question; a read-only probe never saw the failure path fire |

### 8.2 Open questions

- Roles of game-state banks 0, 2, 5, 6, 7; whether "no bounds check" holds for every pool path or only the bank carve-out; the exact stock render-bank size (`0x560000` is the offline-patch threshold in `PoolResize`).
- The third membership-decode caller (`0x442394`, static buffer `0x4E3BB80`) is unverified for the `0xA300` (32-player) message size - possible overrun beyond 16 players if it runs in the 32-player path.
- Who writes player 0 record `+0x10 = 4` during the high-slot join (watchpoint plan in 4.7); which of the fixed 16/17-entry tables still gate the seat.
- Whether the 17-entry session parameter at `session+0x60A60` contributes to the seat refusal, and how its network encoding must grow.
- Dedicated-server `limits_id` join gating: backend passthrough and every join path that never peeks the host.
- The unreproduced "multiplayer stopped at 34 players" report: no test in the tree has ever had more than 5 players, and the code ceiling is 32 peers; treat 34 as unmeasured.
- The hex-number UI bug has no repro yet (exact screen/mode needed before the candidate can be tested).
- Which string list names the native Forge palette entries (stock Forge shows blank names for the added vehicles); whether rebuilt maps need a hash check before online play.
- Pelican seat prompts and door sync online (the X function is global and unsynced).
- Whether Option 2 (add-on palette pack) can reuse ElitePack's extra-cache + relocation path unchanged (6.6).
- Selectable armour-effect colour variants and Elite lighting/shine: both need engine effect/render-constant work; only the recolour proof exists today.
- Cross-game decompile comparison for navmesh baking: no usable later-engine evidence found; **unverified**.

---

## Appendix A - RVA quick reference

**Provenance:** all RVAs are for MCC `haloreach.dll` build 1.3528.0.0, SHA-256 `738dd2d2...0894`, image base `0x180000000`. Retail (Xbox 360) names come from the v3 retail decompile; retail -> MCC matches are maintained in `docs/02-engine-reference/mcc-address-table.md` and must be added there only after static matching.

### A.1 Memory, tags, game state

| What | RVA |
|---|---|
| Live tag table pointer | `*(haloreach+0xC1A600)` |
| Game-state bank table | `0x27BF270`, stride `0x18F0`, `+0xA0` capacity, `+0xA8` used |
| `data_array_new` (pool init) | `0x12FCC` |
| Zone-set rebuild | `0x8EAF0` |
| Extra-cache step | `0x30698` |
| Game over / game finish | `0x7BA58` / `0x59330` |

### A.2 Session, networking, membership

| What | RVA |
|---|---|
| Session object peer array / player records | `+0x60` (17 x `0x158`) / `+0x1740` (16 x `0xB0`) |
| Session player mask | `+0x173C` |
| Membership block base for the producer | `session+0x38` (peer +0x28, player +0x1708) |
| Membership producer / delta caller | `FUN_18039DD4C`; calls at `0x396A2D` (full) / `0x396A5C` (delta) |
| Membership encode / decode | `0x42675C` / `0x427140` (registered `0x427E8C`, size `0x2550`) |
| Membership consumer (delta apply + checksum) | `FUN_18039E47C`; block checksum vs payload `+0x2548` |
| Peer state setter | `FUN_18039C668` |
| Peer state promotions | 5->6/7 `0x392F6E..0x392F84`; 6->7 `0x44AE7E`; 7->8 readiness `0x393191..0x39349D` |
| Peer-properties apply (msg 27) | `FUN_18039D1F8`; peer math caved 2026-11-16 at `0x39D23E`, `0x39F04E`, `0x39F0C0` |
| Player-properties apply (msg 33) | `FUN_18044B1DC`; mapping read `0x44B24C` (caved) |
| Player add / remove primitives | add `0x39D8A8`; remove `0x39D950`; remove wrapper `0x39D7F8` |
| Join request / accept gates | `0x3923DC`; `can_accept_join_request` `0x3975D8`; `can_accept_player_join_request` `0x3973BC`; `acknowledge_join_request` `0x397914` |
| Find player by identifier | `0x39D59C` |
| `boot_peer` | `0x395DE8` |
| Session -> game player sync | `0x41F510` |
| Session slot count hook fallback | `0x39C5C8` |
| Close primitive / connect-closed handler | `FUN_18043B98C`; reason-10 call at `0x429BEE` |
| Connect-refuse encoder | `0x42459C` (the hook is on the encoder, so the log line means "sent") |
| High-slot test free-slot starts / decode limit | `0x3953BC` (peer 17), `0x39D69A` (player 16), `0x4273B0` (owning peer 16 -> 31) |

### A.3 Models, armour, objects

| What | RVA |
|---|---|
| `matg` player representations | `+0x474` count / `+0x478` pointer, stride `0x6C`, biped at `+0x40` |
| Set region permutation | `0x4729F4` (region lookup `0x1CBF0C`, permutation lookup `0x1CBF98`, notify `0x4730CC`) |
| Object variant reset | `0x472B70` |
| Set change colour | `0x473858` |
| Sim -> render state copy | `0x1CD389` (inside `0x1CD2BC`) |
| Biped gender switch | `0x49BB84` |
| Player record init (`0xD00` stride) | `0x6A4BA0` |
| Player unit lookup | `0x53EF8` |
| Camera mode check / unit third person | `0x48A910` / `0xC6948` |
| Visor item -> slot translator / direct patch | `0x3B2F4` / `0x2EDC5E` |

### A.4 Proven MCC-only hook targets

| What | RVA |
|---|---|
| Pool initialiser (`data_array_new`) | `0x12FCC` |
| Game-state bank table | `0x27BF270` |
| Per-player control tick | `0x61A38` |
| `ai_place` | `0x6C2B10`; `ai_set_team` `0x6C1C14`; `ai_set_blind`/`ai_set_deaf` `0x6BF204`/`0x6BF0EC` |
| `object_placement_data_new` / `object_new` | `0x49CC24` / `0x46D6E0` |
| `object_set_position` / `object_refresh` / `actor_teleported` | `0x46CB38` / `0x46C8B4` / `0x717F50` |
| `unit_exit_vehicle` | `0x4F9A74` |
| Saved-film manager session monitor | `0x3ABE70` |
| Armour-composite fallback / null-biped / whitespace crash patches | `0x105324`->`0x1055C1`, `0x99A0D`, `0x8262E8` |

---

## Appendix B - source and documentation map

### B.1 Tracked documentation (`docs\`)

| Path | What it is |
|---|---|
| `docs/00-README-START-HERE.md` | index of the 2026-11-15 documentation packet (its state blocks describe v0.19.103; superseded by this file) |
| `docs/multiplayer-session-2026-11-16.md` | multiplier status of the 2026-11-16 session: fixes, 5-player evidence, high-slot history (the seat blocker it describes was fixed in `0fdcf97`), harness quick reference |
| `docs/ANTI-REVERSING.md` | what the release build hides and how to verify it (7.5) |
| `docs/01-guides/` | modder guide (68 hooks with MCC RVAs, launch flags, tooling, variant rules) and player guide (Firefight, Armory, appearance) |
| `docs/02-engine-reference/` | retail -> MCC address table, master upgrade opportunity list (68 items), decompile index, TOPIC chapters: networking, AI/pathfinding/objectives, armour/appearance/armory, lighting/rendering, **scaling past 32 players (64/128 capacity plan)** |
| `docs/03-qa-and-release/` | QA verification log, stabilization report, stage-1 status (pool inspector readings), release draft |
| `docs/04-project-state/` | project start-here, master task list, session handoffs, workflow ledger, reference library |
| `docs/05-research-and-prompts/` | visor colour and armour-effect RE notes, hex-number bug root cause + runtime check, decompile notes, network boundary note, community requests |
| `docs/limits-*.md`, `docs/limits-players-*.json` | the original 2026-11-08/10 player-limit research (32-slot site lists, bounds review, message plan) - the source data behind `SlotPatch` |
| `docs/collision-walk-crash.md`, `docs/mod-map-mount-findings.md` | crash guard and VFS map-mount notes |
| `docs/MANIFEST-SHA256.txt` | SHA-256 manifest for the 2026-11-15 documentation packet |
| `release-notes/v0.20.*.md`, `src/ui/ChangelogData.hpp` | shipped release notes and the in-client changelog data |
| `BUILDING.md`, `README.md` | build prerequisites and repo/distribution overview |

### B.2 Source modules (`src\`)

| Module | What lives there |
|---|---|
| `src/main.cpp` | process entry, launch/parse paths, in-process vs relaunch host flow |
| `src/hooks/` | engine hooks: `LobbyPatches.cpp` (session/lobby/join/membership/roster), `EnginePatches.cpp`, `ForgePatches.cpp` |
| `src/host/` | `SlotPatch.*` + `Slot*Table.inc` (32-player slot patch), `SlotMessage.*`, `PoolResize.*`, `PoolReport.*`, `EngineLimits.*`, `SpawnTest.*`, `Window.cpp` |
| `src/engine/` | `ReachHost.cpp` (launch/state), `BlamEngine`, `ElitePack.cpp`, `ReachArmory.*`, `VariantRules` |
| `src/ui/` | front end: `FrontEndMenu/Pickers/Lobby/Social`, `PauseMenu`, `ServerBrowser`, `SettingsPanel`, `ArmorCustomizer`, `NobleUi`, `ChangelogData` |
| `src/network/` | `SessionManager`, `MasterServerClient`, `SocialClient`, transport/relay, `UPnP`, auth |
| `src/common/` | `Version.hpp` (single version source), `Settings`, `BuildIdentity`, `DeviceIdentity`, `ObfuscatedString`, `EngineModule`, `Logger` |
| `src/career/`, `src/audio/`, `src/vfs/`, `src/mvar/`, `src/launcher/`, `src/dedicated/`, `src/res/`, `src/core-math/` | career/progression, audio, VFS, map-variant tooling, launcher/updater, dedicated stub, resources (embedded pmcg), math |
| `data/` | `limits/{v16,v16p,full}.nlp`, `forge_palette.json`, profile/launch blocks, menu-map string-id tables; large maps and armour packs are distributed with releases, not git |
| `tools/` | `mp-session-test.ps1`, `publish-release.ps1`, `limits/gen_*.py` (slot tables), `limits/test-limits.ps1`, `test-modes.ps1`, `coop-test.ps1`, LAN harnesses, emblem/mvar helpers |

### B.3 Read order for a new engineer

This file -> `docs/multiplayer-session-2026-11-16.md` -> `docs/02-engine-reference/mcc-address-table.md` -> `docs/02-engine-reference/TOPIC-networking-multiplayer.md` -> `docs/01-guides/MODDER-GUIDE-HOOKS-OPTIONS-2026-11-15.md` -> `docs/04-project-state/` for the current task list. The 2026-11-15 packet under `docs/03-qa-and-release/` is the best record of what was actually tested before the multiplayer workstream.

---

## Appendix C - tools, engines and reference material

Reference only: Noble ships no content from any of these. Confidence tags as elsewhere.

| Resource | What it gives / use | Confidence |
|---|---|---|
| Halo: Reach Effects Tech (C. Tchou, GDC 2011) | Bungie's effects/render pipeline talk: cheap colliding particles, shell-extruded shield geometry, low-res transparency; behaviour reference only | community |
| I Shot You First: Gameplay Networking in Halo: Reach (D. Aldridge, GDC 2011) | Bungie's host/client netcode model; basis for the networking behaviour in section 3 | community |
| Bungie "Design in Detail" talks (sniper rate of fire 2010, plasma-rifle muzzle velocity 2011) | sandbox/AI balancing method; design reference only | community |
| Three Approaches to Halo-style Behavior Tree AI (GDC 2007 panel) | AI behaviour-tree design background | community |
| Microsoft Learn MCC docs (`hr/art/fbxtogr2`, H3 shader option pages) | official FBX->GR2 pipeline and shader option index; Reach-specific shader pages were not found | community |
| c20.reclaimers.net (Reclaimers wiki) | tag/JSON parameter catalogue, compiling shaders, PBR | community |
| HREK (`D:\Steam\steamapps\common\HREK`) - official Reach mod tools | `tool.exe`/`tool_fast.exe` cache builder (Forge World and pack maps), `Foundation.exe` tag editor, `Sapien.exe` scenario editor, `MegaloEdit.exe` game-mode scripting, `fbx-to-gr2` import; backups under `HREK\noble_mods\` | community/observed |
| Assembly (Lord-Zedd) | edits live MCC `.map` cache data in place; shader/tag reads | community |
| ManagedBlam (Blender Python) | read/write Blam tags; Noble's scenario/palette pipeline (duplicate, never add) | observed |
| Reclaimer 2.1.1645 (Gravemind2401) | Halo file explorer: compressed MCC maps, render models, bitmaps, regions/permutations; `campaign.map`/`shared.map` must sit beside the target; set the materials root or models render yellow; AMF is outdated - use RMF | observed |
| Bonobo (LaikaGlove), Baboon (Zoephie), Foundry (Crisp), Architect | animation extraction, byte-exact tag edit, Blender tag import, H3 Spartan builder - none used by Noble yet | community |
| Blender + community add-ons (General-101 toolset, CR4B material builder, BMAX) | DCC for community model/rig work; ManagedBlam runs through Blender's Python | community |
| Ghidra 12.x + GhidraMCP bridge | the engine RE source: `disasm.ps1`, `find_callers.ps1`, RVA/signature lookup, function decompile, xrefs; Noble's proven addresses come from here. Keep exactly one GhidraMCP extension copy installed | observed |
| The Halo Archive / Halo Mods / Reclaimers communities | ripping, shaders, rigging, Blender packs; treat as untrusted reference and re-verify against the engine | community |

Gaps: Megalo is installed but not integrated into Noble's tooling; the HREK `fbx-to-gr2` pipeline has never been run by Noble; Reach first-person animation extraction and why Bonobo rejects current MCC maps remain unresolved; Foundry's xrgb/sRGB handling and empty-scene imports are unresolved.

---

*Maintenance: this file is a reference, not a log. Change it only with evidence - add a row after the test, not before it; keep the confidence tags honest; when a note and this file disagree, the newer proven note wins and the stale claim should be removed rather than annotated.*
