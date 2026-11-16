# O7's requests - recorded 2026-11-15

Source: addendum A of `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` ("Requests from O7 (a community dev)").
O7 is a community developer. The addendum does not say whether this is the same person as repo owner O7OF - ask Tyler
if the distinction ever matters.

Recorded here so the requests survive outside the prompt. **Nothing below is implemented or measured yet** unless a
status line says so; statuses are as of 2026-11-15 10:45 ET. Nothing here is a commitment or a schedule.

Cross-references: `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md` (candidate work),
`project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` (phases, addendum A), and
`project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` (briefing, T1-T10).

## The three requests

### 1. More hooks, now that the new decompiles exist
Ask: expose more engine functions as hooks now that the MCC `haloreach.dll` decompile (27,578 functions) and the named
Xbox 360 retail Reach decompile (33,787 functions) exist. Phase 3 of the starter prompt lists the candidate groups:

- **Team/player:** `game_engine_player_prepare_to_change_team`, `game_engine_player_changed_teams`,
  `game_engine_teams_player_can_change_teams`, `is_change_teams_allowed`,
  `player_set_team_by_configuration`, `set_team_for_object`, `game_engine_set_player_fireteam_by_team`,
  `game_engine_get_change_colors`.
- **Spawn/round:** `game_engine_player_added`, `game_engine_player_rejoined`,
  `game_engine_player_setup_player_for_respawn`, `game_engine_player_find_best_player_respawn_object_index`,
  `game_engine_player_get_player_respawn_time_seconds`, `game_engine_player_reset_players_for_new_round`,
  `loadout_reset_player_loadouts_for_new_round`, `game_engine_spawn_influencers_build_collection_for_player`.
- **Results/session:** `game_results_set_final_team_score_and_standing`, `game_results_statistic_set`,
  `managed_session_players_add_complete`, `managed_session_players_remove_complete`.

Note: the names above are the phase plan's spelling of the retail candidates; confirm each exact symbol in the retail
`decompiled_v3` export before using it (the plan itself says to match by body, not by name alone).

Plan: phase 3 - retail name -> body in `decompiled_v3` -> find the MCC match by strings/constants/call shape -> record
`hr+RVA` in `decompile-exports\topics\mcc-address-table.md` -> prove with a read-only runtime probe -> implement hook ->
test -> document, then expose each as a documented mod API (phase 3 step 10).

Status: **not started** (2026-11-15). Same method as request 2; the two should be worked together.

### 2. Native hooks such as team change/swap
Ask: real native hooks for team change/swap - the prepare/changed pair plus the permission checks (the first subgroup in
request 1 above).

Why it matters: lets mods/scripts change teams mid-game safely instead of forcing values, and the permission predicates
are what the engine itself uses to decide whether a swap is allowed. Related Noble code already exists for lobby/slot
work, but this ask is about the engine-side team functions, not the web UI.

Plan: phase 3. Start with the two entry points (`prepare`/`changed`), then the permission predicate
(`..._can_change_teams` / `is_change_teams_allowed`), then the setters (`player_set_team_by_configuration`,
`set_team_for_object`) and the fireteam mapping (`game_engine_set_player_fireteam_by_team`). Record each proven
retail -> MCC match in `mcc-address-table.md`.

Status: **not started** (2026-11-15).

### 3. Player limit expansion (still ON HOLD)
Ask: expand the player limit (Noble's track is multiplayer 16 -> 32 -> 128).

Status: **ON HOLD by Tyler** - do not start until Tyler lifts the hold (same hold recorded in the starter prompt and in
`HANDOFF-FINAL-2026-11-15.md`).

When/if lifted: follow phase 5 of the starter prompt + master list A5. Preconditions: pool inspector works, team/spawn
hooks work, 32 slots stable. Order: session membership -> players arrays -> per-player simulation entities/events ->
scoreboard/UI widgets -> split-screen render windows -> validation checks (`invalid-player-count`, `session-size`,
`big-party-count-exceeds-teams`). Keep replicated/serialized/deterministic sizes identical on all peers; version-stamp
saved films. Test 16 -> 24 -> 32 -> 64 -> 128 with join/leave, host quit, save/load, saved film, Forge, campaign.

## O7's opinion (opinion, not measured)
O7 said (opinion, not measured) that the Blam engine is hard to mod because:
- HaloScript is a separate Lisp-style language compiled into the map and run by the engine's script threads;
- naming conventions / folder structure are strict;
- most data is baked into map tags.

These match what we see (recorded as context, not as new findings):
- Scripts can only call functions the engine exposes: `k_hs_external_global_count` is the number of script-exposed
  engine globals, so adding a new script function needs engine code (master list A9).
- Data-only changes (AI squads/tasks, marines, objectives) are cheaper than engine patches (master list E3/E8).

## Cross-references
- Phase plan and prerequisites: `project-docs\PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` (Phase 3, Phase 5, addendum A).
- Full briefing: `project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` (T1-T10, method, pitfalls).
- Candidate functions, limits and priorities: `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`
  (A5 player slots, A9 scripting, E3 objectives, E8 marines; networking in section C).
- Topic chapters: `decompile-exports\topics\TOPIC-networking-multiplayer.md`,
  `decompile-exports\topics\TOPIC-ai-pathfinding-objectives.md`.
- Proven retail -> MCC address matches: `decompile-exports\topics\mcc-address-table.md`.
- Holds: 128 players (Tyler); release / version bump / tag / public assets only on Tyler's "release it".

## Status log (append-only)
- 2026-11-15 10:45 ET - recorded from the prompt addendum; no work started on any of the three requests.