# C12: 128 real players — research note (2026-11-08, DeepSeek)

Status: **research only, nothing patched.** Companion to `LIMITS-TODO.md` C12 and `LIMITS-HANDOFF.md`.
Every RVA is for stock `haloreach.dll` 1.3528.0.0 (file offset = RVA − 0xC00 in `.text`).

## 1. What "16 players" actually is today

| Thing | Capacity | Record | Where |
|---|---|---|---|
| `players` data array | **16** | 0x490 (1,168 B) | created by `data_array_new` (0x12FCC) at **RVA 0x43474**; count is `lea edx,[r9+0x10]` (immediate 0x10) at **0x43470**, record size `mov r8d,0x490` at **0x43465**, name string at 0x43469 |
| `fire team` data array | **16** | 0xD4 (212 B) | created at **RVA 0x122E58**; count `lea edx,[r9+0x10]` at 0x122E54 |
| `simulation view data array` | **16** | 0x118 (280 B) | in the pool report; creation call site not yet located (not within 64 B of a 0x12FCC call) |
| `players` per-player HUD/scoreboard rows | 16 | – | engine CHUD widgets + Noble's own scoreboard (Noble's UI is already dynamic: `lobbyRoster.size()`) |
| Noble lobby slots | **128** | – | `LobbyPatches::s_maxMultiplayerPlayers = 128` (`src/hooks/LobbyPatches.cpp:17`); co-op 16 |

The lobby already accepts 128 and the session code is patched to insert more peers; the **match** can still only
create 16 players because the `players` array (and everything indexed by player index) is 16.

## 2. Engine limits and the code that has to change

### 2.1 Player data arrays (must grow together)
| RVA | What |
|---|---|
| 0x43474 | `players` array creation — count immediate at 0x43470, size 0x490 |
| 0x122E58 | `fire team` creation — count immediate at 0x122E54, size 0xD4 |
| – | `simulation view data array` (16 × 0x118): find its constructor (pool report name) |
| `data_array_new` 0x12FCC | every one of the above is a named pool, so `PoolResize` can raise them at runtime **once the match accepts more players** |

### 2.2 Session / network membership (Noble already patches some of this)
| RVA / global | What (from `src/hooks/LobbyPatches.*`) |
|---|---|
| 0x3973BC | lobby player-slot function; stack args 5, 6, 7, 9 control insertion |
| 0x39CE10 | `c_network_session_membership::evaluate_coop_insertion_point` — refuses co-op joins with 0x1B when saves differ |
| 0x425368 | "join refused" logger (arg 3 → {u64 session, u32 reason}) |
| 0x42459C | "connect refused" logger |
| 0x44EBA0 | `c_network_session::abort` |
| 0x429AA4 | out-of-band session message handler (type dispatch) |
| 0x39BDCC | session update path (queue mode + session state) |
| 0x2A49308 | queue-mode global; 0x2A1FE08 / FE10 / FE18 = mode session pointers; 0x2A1FD40 = mode-set byte; session +0x60EF8 = state (6 = in game) |

The session membership and replication wire formats are the second wall: player indices/masks in packets are
16-bit (`0xFFFF`), and the roster is packed per message. Raising them means changing packet layouts on **both**
host and client, i.e. a network protocol version bump, not just a pool size.

### 2.3 Per-player static tables and 16-bit masks
Not yet enumerated. Method that works (already used for the AI audit): with
`python tools/limits/analyze_ai_frames.py --range …` for stack frames, plus a scan for `imul reg, reg, 0x490`
(player record) and for 16-bit masks (`cmp …, 0xFFFF` / `or …, 0xFFFF`) near player-array use
(`movzx …, word ptr [player*…]`). Candidate tables to check first: player spawn/insertion, per-player update
priorities, killfeed, scoreboard, voting, team bitmasks, and `chud widgets[0..3]` (4 widgets × 4 players each).

### 2.4 Spawning
Players are inserted through the session membership path (0x39CE10 for co-op). For 32+ players the spawn point
allocator, respawn queue and team balancing must all stop assuming 16; these live with the `players` record and
the `fire team` array (16 × 0xD4).

### 2.5 HUD / scoreboard / killfeed
Noble's scoreboard is dynamic (`PauseMenu.cpp` sizes rows from the roster, 128 already "usable"), so the work is
engine-side: CHUD widgets are per-player and fixed at 4 widget slots (pool report `chud widgets[0..3]`), and the
engine killfeed/scoreboard draws from the 16-slot player array.

## 3. Milestones

| Milestone | Work | Effort |
|---|---|---|
| **16 → 32** | Raise `players` (0x43470 / 0x43465), `fire team` (0x122E54), simulation views; find and raise 16-slot per-player tables; spawn/insertion smoke test. Network masks still fit in 32 bits. | **2-4 days** (DeepSeek: 1 day for the pools + tables, rest for spawn/HUD fixes) |
| **32 → 64** | 16→64-bit player masks in session/replication code; message size growth; roster packing; join/connect refusal paths (0x42459C/0x425368); scoreboard/killfeed; team balancing. Protocol bump. | **1-2 weeks** |
| **64 → 128** | Session membership limits, peer ids, packet size limits, dedicated server config, UI at 128, performance (player update budget) | **2-4 weeks** |

Total for 128 real players in a match: **~4-7 weeks** of focused engine work, with the first playable 32-player
build possible in under a week if the 16-slot tables are the only hard wall.

## 4. Practical first step (recommended)

1. Change the two count immediates to 0x20 (32) in a test profile: 0x43470 (`41 8D 51 10` → 0x20) and 0x122E54,
   then run a local match with `-max-players 32` and see what breaks (crash RVA names the next table; the
   STACK OVERRUN/gs hook already reports the function).
2. Keep `players` at 16 until the network path accepts 32 — the engine will otherwise render/update stale slots.
3. `docs/limits-ai.md`'s tooling and the `-pool-trace` / `-ai-pools` dev options are the fastest way to find
   the next fixed table (pool names are already printed).
