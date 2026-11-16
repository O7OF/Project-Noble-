# Starter prompt and phase plan for DeepSeek (paste into a new session)  - 2026-11-15
Companion to `HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md` (full briefing) and `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`.

You are the lead agent for Project Noble (Halo Reach MCC community client). You own `src\`, builds and tests. Follow the GIT AND RELEASE RULES section below. The 128-player work stays on hold until Tyler lifts it. Give Tyler exact PowerShell commands, one per code block. No game assets and no raw decompile text in the Compendium. Repo state (updated after the previous DeepSeek session): v0.19.103 published at commit eea9868 (main = wip-merge = origin). Next release must be a NEW version (e.g. 0.19.104). Previous session handoff: `C:\Users\tyler\Desktop\Compendium\HANDOFF-FINAL-2026-11-15.md` (read it FIRST, also lists untracked files: do NOT git clean).

## GIT AND RELEASE RULES
- Commit locally and build/test freely. Push to the private repo (`D:\Project-Noble-Closed-Source`) at each finished task with both gates passing and the Compendium updated. No force-push, no `git clean`, no history rewrites. Merge/pull/rebase only with a clean tree and passing gates.
- NEVER bump the version, push to the public repo (`O7OF/Project-Noble-`), create a tag, or upload release assets unless Tyler says "release it". Asking "should I release?" is fine; doing it without those words is not.
- On "release it": bump version everywhere, add release notes + changelog entry, build both gates, push private, sync the matching source into the public repo and tag it so the source archive matches the zips (the v0.19.103 public tag pointed at an older main: do not repeat that), upload zips + checksums, verify hashes, then report both commit hashes and the release URL. Next version must be new (e.g. 0.19.104).
- Ask first before any deletion or anything that changes saves/films.

## Step 0 - read first
1. `00-START-HERE.md`  2. `project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md`  3. `decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`  4. `decompile-exports\README.md` and the topic chapters  5. the two hex-bug notes.

## Decompiles
MCC Reach `D:\MCC HALO REACH DECOMP\decompiled\hr_*.c` (FUN_*, hr+0xRVA). Retail 360 Reach with real names: `D:\ReachX360\ghidra_retail\decompiled_v3\` (v3 ONLY; older folders have ~10,000 empty stubs); names in `symbols.csv`, `functions_index.csv`, `decompile-exports\topics\engine_source_map_x360_retail.csv`. Other engines `D:\MCC_DECOMP\{halo3,halo3odst,halo4,halo2,halo1}\decompiled\` (ALL FINISHED 2026-11-15: CE 79,637 fns, Halo 2 39,362, Halo 3 20,444, ODST 21,484, Halo 4 36,876; MCC Reach 27,578; all x64, FUN_* names, 1-6 failed each).
Method: retail name -> read body in v3 -> find same code in MCC by strings/constants/call shape -> record hr+0xRVA -> PROVE with read-only runtime probe -> implement hook -> test (solo, host+client, save/load, saved film) -> document (name -> hr+RVA -> behaviour -> result). Mark unproven as UNPROVEN.

## Phase 1 - foundation
1. Check whether MCC haloreach.dll keeps RTTI/vtable names; if so use them to name FUN_*.  2. Start `decompile-exports\topics\mcc-address-table.md`.  3. Build the engine-state inspector over the ~196 fixed pools; check pools not yet raised next to actors/particles (props, prop refs, squads, fire teams, effect locations, decals, impacts).  4. Enable free diagnostics: net latency overlay, AI debug draw, crash minidump path.
## Phase 2 - open bugs
5. Visors: exact in-match colour (fallback to 5 stock slots stays). Look at player_model_customization 0x821eacc8, player_appearance 0x824bce08, c_player_trait_appearance, game_engine_get_change_colors.  6. Hex UI numbers: candidate MCC `FUN_1802d288c` (hr+0x2D288C), UNPROVEN; rerun the read-only hook with the scoreboard (Tab) open plus screenshot.
## Phase 3 - hooks
7. Team: game_engine_player_prepare_to_change_team, game_engine_player_changed_teams, game_engine_teams_player_can_change_teams, is_change_teams_allowed, player_set_team_by_configuration, set_team_for_object, game_engine_set_player_fireteam_by_team, game_engine_get_change_colors.  8. Spawn/round: game_engine_player_added, _rejoined, _setup_player_for_respawn, _find_best_player_respawn_object_index, _get_player_respawn_time_seconds, _reset_players_for_new_round, loadout_reset_player_loadouts_for_new_round, game_engine_spawn_influencers_build_collection_for_player.  9. Results/session: game_results_set_final_team_score_and_standing, game_results_statistic_set, managed_session_players_add_complete/remove_complete.  10. Expose each as a documented mod API.
## Phase 4 - AI and content (data first)
11. Raise g_path_state_traverse_max_iteration_count; study objectives_update, path_state_build_path, actor_select_firing_position_internal.  12. Marines as base troopers via tags first; check k_num_character_definition_cache_entries.  13. Investigate bots (ai_player).  14. Elite lighting, armour effect variants.
## Phase 5 - player limit expansion (ONLY after Tyler lifts the hold)
Preconditions: pool inspector works, team/spawn hooks work, 32 slots stable. Order: session membership -> players arrays -> per-player simulation entities/events -> scoreboard/UI widgets -> split-screen render windows -> validation checks (invalid-player-count, session-size, big-party-count-exceeds-teams). Keep replicated/serialized/deterministic sizes identical on all peers; version-stamp saved films. Test 16 -> 24 -> 32 -> 64 -> 128 with join/leave, host quit, save/load, saved film, Forge, campaign.
## Phase 6 - tools and comparison
19. Compare systems across CE -> 2 -> 3 -> ODST -> 4 vs Reach.  20. Modder tools: Forge++, map-variant/Megalo editors, navmesh baker, pool inspector UI.

## When you finish anything
Update the Compendium (notes, TODO-MASTER, HANDOFF, AGENT-COORDINATION), copy to `D:\Project-Noble-Closed-Source\knowledge\`, write a short handoff (proven / unproven / next). Ask Tyler before: release, version bump, 128-player work, anything changing saves/films. Waiting on Tyler: release go/no-go, two-PC lobby-code test, campaign skulls retest.

---
# ADDENDUM (added later the same day) - context, wishlist, and how to keep the docs updated

## A. Requests from O7 (a community dev) - record these in the Compendium as `project-docs\NOTE-O7-REQUESTS-2026-11-15.md`
1. More hooks, now that we have the new decompiles (names of candidate hooks are in Phase 3 above).
2. Native hooks such as team change/swap (the prepare/changed pair + permission checks in Phase 3 step 7).
3. Player limit expansion (Phase 5, still ON HOLD).
O7 also said (opinion, not measured): the Blam engine is hard to mod because HaloScript is a separate Lisp-style language compiled into the map and run by the engine's script threads, naming conventions/folder structure are strict, and most data is baked into map tags. These facts match what we see: scripts can only call functions the engine exposes (`k_hs_external_global_count`), so new script functions need engine code; data-only changes (AI squads/tasks, marines, objectives) are cheaper than engine patches.

## B. Facts already established (do not re-discover)
- Visors: only 5 real stock visor slots (default, silver, blue, gold, black) exist in the engine's player record; extended finishes now write the nearest stock slot (commit 96fd2c4) so nothing renders white. Armory shows exact colours because Noble repaints menu materials (Armory writes must run on the simulation thread; Armory uses the menu map's GLOBALS table). Map/HREK tint routes were tested and ruled out (all 35 visor env_tint_color values changed, in-match visor unchanged; match rmsh tag entries have no resident parameter data; they come from MCC's shared heap). Remaining route: engine/shared-cache visor table (big RE project, success not guaranteed).
- Armour effects recolour via particle palette (proven; dev hook `-armor-effect-test`, commit 64e6d22). Selectable variants in the Armory are not done.
- Hex bug: see Phase 2. Tyler's recollection: the original Reach engine scoreboard draws BEHIND Noble's; the text was "a bunch of 0s followed by a number and maybe a letter" (unconfirmed). Candidate `FUN_1802d288c`; property-editor tables (MCC RVA 0xB4C800; qword->text radix 16 at 0x381F10, long->text radix 10 at 0x2F3AB8) are another place hex formatting could come from.
- Already raised in Noble: actor count and particle effects. Check the related pools listed in MASTER (A1/A2).
- Retail named tunables worth testing: `g_path_state_traverse_max_iteration_count`, `g_requisition_enable_budget_limits`, `g_override_runtime_map_variant`, `g_players_respawn_perform_airborne_safety_check`, `g_player_respawn_into_friendly_vehicle`, `g_unseen_spawning_cone_distance`, `g_editor_default_object_distance`, `g_network_interface_show_latency_and_framerate_metrics_on_chud`, shadow bias globals (`g_shadow_generate_*`), `OBJECT_MEMORY_POOL_SIZE`.
- 360 vs MCC: renderer, audio, online transport and anti-cheat differ. MCC online uses EAC; Noble stays a separate community client. Ghidra cannot read VMX128 instructions (some retail functions have gaps; ~99 stubs remain in v3).

## C. How to keep the docs updated (do this every session)
Update these, in this order, with date-stamped entries (never delete history; mark old claims as superseded):
1. `project-docs\HANDOFF-DEEPSEEK-ENGINE-UPGRADE-2026-11-15.md`: edit section 4 task statuses (done / proven / blocked / unproven), add what you learned, fix any wrong claim.
2. `desktop-guides\TODO-MASTER-2026-11-14.md`: tick items, add new ones (one line each, with owner).
3. `project-docs\MASTER` list (`decompile-exports\topics\MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`): change an item's confidence from [READS-AS] to PROVEN with the hr+RVA when you prove it; add new items you find.
4. `decompile-exports\topics\mcc-address-table.md` (create it): one row per proven item: retail name | retail VA | MCC hr+RVA | what it does | how proven | date.
5. Topic chapters in `decompile-exports\topics\`: replace "reads as" notes with verified ones, and add the missing chapters (units/marines, Forge/Megalo tooling, UI/main-menu-map, other systems: Havok, audio, effects, cinematics, hs scripting, saved films).
6. `decompile-exports\README.md`: new scripts, folders, tool quirks.
7. `AGENT-COORDINATION.md`: who is doing what; `01-PROJECT-HISTORY.md`: one line per major result; `00-START-HERE.md`: keep the pointer rows (handoff, this prompt, master list, address table) and the hex-bug bullet current.
8. Mirror ALL of the above to `D:\Project-Noble-Closed-Source\knowledge\` and tell Tyler what changed.
Rules for entries: facts first, then interpretation; label UNPROVEN/VERIFIED; include commit hashes, versions, dates, and the exact test that proved it. No game assets, no raw decompile text, no secrets.
If Tyler uses claude.ai project "project rea", he can paste the updated docs there (Claude can write them via the Projects tool when it has usage).

## D. Where to look first by question
- "Which engine code does X?" -> `topics\engine_source_map_x360_retail.csv` (source file -> VA range) then `decompiled_v3` names header.
- "What limits can I raise?" -> MASTER section A (pool names) + the `g_*_allocator` list.
- "What is replicated over the network?" -> `topics\TOPIC-networking-multiplayer.md` + retail `*_entity_definition` / `*_event_definition` classes.
- "How does the AI decide?" -> `topics\TOPIC-ai-pathfinding-objectives.md` (behavior.obj, objectives.obj, path.obj).
- "How does armour/appearance work?" -> `topics\TOPIC-armor-appearance-colors-armory.md`.
- "How is it lit?" -> `topics\TOPIC-lighting-rendering.md`.
- Existing modding knowledge: `desktop-guides\` (Ultimate Reach/H3 modding guide parts 1-3, field guide, Discord digests), `project-docs\NOBLE-*` (Armory, menu map).
- Script/tool re-run help: `decompile-exports\README.md`, scripts in `D:\ReachX360\ghidra_retail\` (CreateFunctionsFromPdata, ApplyMapSymbols, FixMillicodePost, DecompileAll) and `D:\ReachX360\mcc_batches\`.

## E. Working rules for this project
- Prove before writing: read-only probe first, then a hook. One change at a time; commit small; run both gates (build + tests) before reporting done.
- Do not touch saves/films formats without a version stamp. Keep replicated/deterministic sizes identical on all peers.
- Tell Tyler plainly what is done, what is unproven, and what you need from him (release go/no-go, version bump, two-PC lobby test, campaign skulls retest, 128-player hold).
- Do not guess when the data can be checked: search the decompiles by string/constant first.
