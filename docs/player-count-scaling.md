# Scaling campaign co-op past 4 players (notes, not implemented)

Status: **research notes only**. Today the cap is 4 and that works. This file records what already scales, what
blocks 8/16, and a staged path if we ever want it.

## What already scales

- **Launch block player count**: `block[0xE8]` declares how many players the session has. We now set it to
  `1 + expectedPeers` for campaign/firefight hosts, so the engine creates a player slot per declared player. The field is a
  byte, so up to 255 structurally.
- **Player slot descriptors**: the launch block has **16** descriptors starting at `0x100`, stride `0x20`
  (`+0x100 + slot*0x20`). Our guest block already writes slot 0 (remote host) and slot 1 (local controller); the rest are
  `0xFF` (unassigned).
- **Engine player array**: MCC's results builder walks **16** player records (stride `0xD8`, `hr+0x28C82E0..0x28C9060`), so
  16 players is the engine's own structural ceiling.
- **Session membership**: `LobbyPatches::SetMaxMultiplayerPlayers` (16..64) and `SetMaxCoopPlayers` (4..16) expand the
  session slot arrays, and the "too many players for net coop" gates are already patched
  (`0x40954A`, `0x40955B`, `0x411B6E`, `0x411D71`, `0x397689`, `0x40958B`).
- **The co-op cap compare** is `cmp edi, [hr+0xD1FD90]` (max players, set once during globals init from a constant block at
  `0xA8C3F0`); the branch that uses it is already jumped over by the LobbyPatches gate patch.
- **Multiple local instances for testing**: `-data-dir <path>` gives each instance its own profile/userdata, so 4-6
  instances can run on one PC.

## What actually blocks >4 in campaign

1. **Scenario spawn points.** Campaign missions ship 4 co-op spawn locations. Players 5+ need a spawn fallback (e.g. spawn
   at the host's position with small offsets) or a patched spawn-point selector.
2. **Mission scripts.** Campaign scripts use `game_coop_player_count`, per-player objectives and player indices 0-3.
   Compiled into the map; changing them means editing the map's script tags (HREK can extract/compile tags).
3. **HUD.** Campaign HUD is built for 1-4 (HUD quadrants, killfeed, nav points). 5-16 needs a Noble-drawn overlay, like the
   planned scoreboard overlay.
4. **Lockstep determinism.** Campaign co-op is deterministic lockstep: every extra player adds an input stream and a
   desync surface, plus host CPU for simulation. 8 is plausible on LAN; 16 wants a strong host and LAN-class latency.
5. **Respawn/checkpoint per-player state.** Some arrays are sized for 4; needs verification before promising more.

## Staged path

| stage | work | effort |
|---|---|---|
| **6 players (experiment)** | declare 6, run 6 local instances with `-data-dir`, see what breaks first (spawns/HUD/scripts) | 1 session |
| **8 players** | spawn fallback for players 5-8 (reuse insertion point 0 / host position + offsets), cap patches to 8, minimal HUD overlay | days |
| **16 players** | + HUD/scoreboard redesign, mission-script accommodations (fork map scripts), bandwidth/determinism tuning, relay limits | weeks |

**Firefight is the easier target** than campaign: wave-based, no cutscenes or checkpoint insertion, and its spawn logic is
closer to multiplayer. If more-than-4 is ever wanted as a demo, Firefight at 8 is the shortest path.

## First experiments when the time comes

1. Run host + 5 guests locally (`-data-dir` per guest, `-wait-peers 5`). Check whether the host log shows
   `launch block declares 6 player(s)`, whether all 6 reach `game session started`, and where the first failure appears
   (spawn, HUD, script assert).
2. If spawns are the blocker: find the campaign spawn-point selector (the function that reads scenario spawns for a player
   index) and add a fallback for `index >= 4`.
3. If scripts are the blocker: extract `m10`'s script tags with HREK, look for `game_coop_player_count` and player-index
   loops, and decide whether to clamp (scripts assume 4) or extend.
