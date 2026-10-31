# Scaling campaign co-op past 4 players (notes, not implemented)

Status: **research notes only**. Today the cap is 4 and that works. This file records what already scales, what
blocks 8/16, how **Reach Renaissance (RR) already does it**, and a staged path if we ever want it.

## How Reach Renaissance does it (the working reference)

RR runs campaign above 4 players and Firefight above 8 today. Its `haloreach_copy.dll` in `D:\Reach` is **byte-identical**
to the Steam DLL (SHA-256 `738DD2D2…0894`), so all of it is **in-memory patching + host-interface stubs** by
`D:\Reach\bin\host-probe.exe` (11.7 MB, Rust, unstripped symbols).

Sources:

- `D:\REACH REVERSE ENGINEER\docs\rr-launch-flow.md` §6 "What host-probe hooks or patches after loading"
- `D:\REACH REVERSE ENGINEER\docs\rr-tags.md`
- strings/symbols in `D:\Reach\bin\host-probe.exe`

### RR's own words (`rr-launch-flow.md` §6, lines 361-365)

> Patches that the binary can apply depending on flags, mode and env [strings]: `armor pack` loading as an extra cache
> (`--armor-pack`, `+0x30859`), game-state arena growth, **32/64-player capacity work (`m64_*`, `p64_*`, `capacity`,
> `memberx`, `census`: session membership, simulation views, observer lists, score tables, HUD masks), Campaign up to N
> players (was 4), Firefight up to N players (was 8)**, AI guard (`FUN_18071f074`), netstats, entity rate gate,
> `RR_JOIN_ANY`, …

### The knobs and modules in `host-probe.exe`

| name | role |
|---|---|
| **`RR_CAMPAIGN_PLAYERS`** | campaign player count (native 4) — this is the single most important knob for us |
| **`RR_FF_PLAYERS`** | Firefight player count (native 8) |
| **`RR_PB` / `RR_PB64`** | "moved player block": relocates the player structs and gains 64 player slots. Strings: "64 player slots need the moved player block (RR_PB)", "64 machines need the moved player block and 64 player slots (RR_PB=1 RR_PB64=1)" |
| **`RR_P64_GROUPS` / `RR_M64_GROUPS`** | capacity-group patches (session membership, simulation views, observer lists, score tables, HUD masks). Strings: "off (RR_P64_GROUPS)", "off (RR_M64_GROUPS)" |
| **`capacity` module** | `CAPACITY`, `ORIG_NAMES`, `RESET_STUB`, `score_list`, **`spawn_list`**, **`hud_players`**, `megalo_sets`, `member_wait`, `census_fixes_32`, `check_ff32_profile(s)`, `WIRE_MODULE`, `engine_event_send`, `megalo_message` |
| **`coopx` module** | `PLAYER_LIST_MODULE`, `player_list`, **`starting_location` / `ORIG_STARTING_LOCATION`** (spawn placement beyond 4), **`objects_pool_growth` / `objects_pool_install`** (room for extra bipeds), `firefight_16`, `ff_message(s)`, `ff_received`, `score_summary`, `metagame_player/record`, `penalty_player/table`, `AI_GUARD_*`, `object_pool` |
| `net::patch_launch_block` | RR patches the launch block for networking before `CreateGameEngine` |
| `census` module | watches the main loop; `GAME_RANGE`, `tls_pointer`, `on_exception`, `REARM` — used to find/patch runtime structures |
| `m64` / `p64` profiles | `--m64-profile` (multiplayer), `--ff32-profile` (Firefight), `--max-players`, `--max-machines`, `--players-table`, `--max-listed-players`, `--lazy-commit` |

The `ff32` profile string spells out the shape of a large session:

> `ff32 experimental profile: players=32 machine admission=33 machine/player storage=64; AI=128; FF/scoring/spawn repairs, AI guard, upkeep/resync …`

### Patch-site strings RR itself prints (useful leads)

These are literal addresses/messages in `host-probe.exe`, i.e. RR's own patch targets in `haloreach.dll`:

- `--player-base: bytes at +0x39d69a are …`
- `machine table: bytes at +0x420161 are …`
- `guest/dedicated: Firefight zero-initial-player check installed (+0x58075)`
- `results writers: +0xfa6cc`, `results writers: +0xfa784`
- `FUN_18041f510 does not call FUN_18041f368 at +0x41f610` / `… FUN_1803a922c at +0x41f56b`
- `the call at +0x109c78 does not lead to capacity::engine_event_send` / `FUN_1801f1c8c is not hooked by capacity::megalo_message`
- `capacity::census_fixes_32's detour at …`

These confirm the profile of work: **player struct block relocation (PB/PB64) + per-subsystem capacity tables
(score/spawn/HUD/membership/observer/megalo) + co-op spawn & object-pool extensions**. Our launch-block `0xE8` fix is only
the "declare the session size" half of the story; RR adds the capacity tables and spawn fallback so the engine can actually
hold and place N players.

## What already scales in our build

- **Launch block player count**: `block[0xE8]` declares how many players the session has. We set it to `1 + expectedPeers`
  for campaign/firefight hosts; the engine creates a player slot per declared player. Byte field, up to 255 structurally.
- **Player slot descriptors**: the launch block has **16** descriptors at `0x100 + slot*0x20`.
- **Engine player array**: MCC's results builder walks **16** records (stride `0xD8`, `hr+0x28C82E0..0x28C9060`) — 16 is the
  engine's own structural ceiling before a "moved player block" is needed.
- **Session membership**: `LobbyPatches::SetMaxMultiplayerPlayers` (16..64) / `SetMaxCoopPlayers` (4..16) expand the slot
  arrays; the "too many players for net coop" gates are patched (`0x40954A`, `0x40955B`, `0x411B6E`, `0x411D71`,
  `0x397689`, `0x40958B`).
- **The co-op cap compare** is `cmp edi, [hr+0xD1FD90]` (globals-init constant), already bypassed by the gate patch.

## What actually blocks >4 in campaign

1. **Scenario spawn points**: 4 per campaign mission. RR's `coopx::starting_location` hook is exactly the fallback needed.
2. **Capacity tables**: score list, spawn list, HUD player masks, membership/observer lists are sized for 4/8/16; RR's
   `capacity` module + `P64_GROUPS` patches these.
3. **Object pools**: more bipeds need `coopx::objects_pool_growth` / `objects_pool_install`.
4. **Mission scripts** assume `game_coop_player_count` 1-4 and player indices 0-3 (compiled into the map; HREK can edit).
5. **HUD** is 1-4 player; RR patches `hud_players` masks, we would draw a Noble overlay.
6. **Lockstep cost**: each extra player is an input stream and desync surface; host CPU for simulation.

## Staged path for us (mirroring RR)

| stage | work | effort |
|---|---|---|
| **6 players (experiment)** | declare 6, run 6 local instances with `-data-dir`, see what breaks first | one session |
| **8 players** | `starting_location`-style spawn fallback for players 5-8, capacity-table patches (score/spawn/HUD), object-pool growth, cap patches to 8 | days |
| **16 players** | + moved player block (`RR_PB`-style), membership/observer/score tables to 64 slots, HUD overlay redesign, mission-script accommodation, determinism tuning | weeks |

**Firefight is the easier target**: RR's `firefight_16` and `ff32` profile show the path (wave-based, no cutscenes or
checkpoint insertion), and its spawn logic is closer to multiplayer. If more-than-4 is ever wanted as a demo, Firefight at
8-16 is the shortest path.

## Where to look in RR for exact patches

1. `D:\Reach\bin\host-probe.exe` is an unstripped Rust binary — Ghidra can recover `capacity::apply`, `capacity::*` patches,
   `coopx::install`, `coopx::starting_location`, `coopx::objects_pool_*`, `net::patch_launch_block` and the env-knob parsing.
   The `docs.zip` in `D:\REACH REVERSE ENGINEER` already contains pseudocode for `new_launch_block`, `host::hit`,
   `set_map`, `set_map_variant`, `main` — but not yet for `capacity`/`coopx`; that is the gap to fill.
2. `D:\REACH REVERSE ENGINEER\docs\rr-launch-flow.md` §5-§6 for the host interface and patch-pass order.
3. `D:\REACH REVERSE ENGINEER\docs\host-probe-functions-so-far.txt` (list of analysed RVAs in host-probe).

## First experiments when the time comes

1. Run host + 5 guests locally (`-data-dir` per guest, `-wait-peers 5`). Expect the launch block to declare 6 and all six to
   reach `game session started`; find the first failure (spawn, HUD, script assert).
2. If spawns are the blocker: implement a `starting_location`-style fallback (reuse insertion point 0 / host position +
   offsets) for `player_index >= 4`.
3. If capacity is the blocker: patch the score/spawn/HUD lists to 16 entries (RR's `capacity` module names map 1:1).
