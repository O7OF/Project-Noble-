# Follow-up prompt for DeepSeek: feature work AFTER the engine prompt is finished (2026-11-15)
Use only when Phase 1-3 of `PROMPT-DEEPSEEK-ENGINE-PHASES-2026-11-15.md` are done (address table started, pool inspector working, team/spawn hooks proven). All rules in that file still apply (GIT AND RELEASE RULES, prove before writing, doc updates, mirror to `knowledge\`).
OUT OF SCOPE for you: networking (session, replication, host migration, netcode for higher player counts) and security. Tyler is giving those to Claude. If a task below needs a networking or security change, stop at that boundary, write down exactly what is needed in `project-docs\NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`, and move on.

## Stage 1 - Status report for Tyler
What from the engine prompt is proven, what is not, and the pool inspector's readings on a heavy Firefight map and a heavy Forge map (which pools are near full). Every stage below depends on those numbers.

## Stage 2 - Player counts (preparation only)
Tyler reports: multiplayer stopped at 34 players. Campaign co-op: the 4/8/12/16 MAX PLAYERS row exists in the UI, but an actual 5-player campaign attempt crashed or black-screened, so the row is NOT proof that more than 4 works. Treat campaign above 4 as broken until a real 5-player session loads and plays.
First reproduce the 5-player campaign failure (which map, at load or in play, crash or black screen, log and pool inspector readings; check whether it is the same black screen as Stage 3). If more than 4 cannot be made to work in this stage, clamp the row to the highest count proven to work so players cannot pick a crashing value.
Your part: find and document exactly what stops each one (which check, pool, array or UI limit; name -> hr+RVA), and fix the non-network causes (pools, UI rows, scoreboard widgets, spawn points, split-screen/render limits). Do NOT change session, membership or replication code; list those blockers in the network handoff note. The 128-player hold still stands until Tyler lifts it.

## Stage 3 - Black screen at max illumination / particle settings (regression, fix before any count raising)
Tyler reports: with Noble's illumination and particle effect options set to max, Firefight goes to a black screen after a short time; same in campaign, and he believes multiplayer too. Recent issue.
1. Reproduce on the AOC monitor with `-NoFocus`; record map, mode, settings, time to failure, and whether the game is still running (audio, HUD, log) or hung.
2. Bisect: illumination alone, particles alone, then each value step, to find the lowest setting that fails.
3. Read the pool inspector at the moment of failure. Suspects (UNPROVEN): the raised particle limit without matching GPU emitter rows/buffers, effect locations, effect message queue, light or light-volume pools, render target/exposure state (MASTER A2, A11, D).
4. Fix the cause. Until it is fixed, clamp the options to the highest value proven safe so players cannot select a crashing setting.
5. Done when: 30 minutes at max settings in Firefight, campaign and a multiplayer map with no black screen, and the result is written up with the cause.

## Stage 4 - Firefight and AI counts as player options (after the black-screen fix, so max-count tests are valid)
Goal: host options for enemies per wave/dropship, number of friendly AI, and wave pacing.
1. Find how Firefight builds waves and dropship loads (retail names to start from: survival_mode_*, squad/spawner code, `g_spawner_globals_allocator`, `g_squad_data_allocator`, `g_actor_data_allocator`, `g_bbsquad_*`; wave/squad definitions are expected to be tag data, VERIFY).
2. Use the pool inspector to find the real ceiling: actors, props, squads, objects, effects. Raise related pools together, never one alone.
3. Add game options in Noble's lobby UI (enemy multiplier, friendly AI count, wave size) with safe maximums taken from measurements, not guesses. Out-of-range values must clamp, not crash.
4. Done when: a 30-minute Firefight at the maximum settings runs with no crash, no pool exhaustion in the log, acceptable frame time; solo and hosted.

## Stage 5 - Marine mode built into Noble (no external mod pack)
1. Data first: marine character/squad/unit tags shipped in Noble's own map set; check `k_num_character_definition_cache_entries` and the squad/actor pools.
2. Friendly marine squads as a Firefight/custom option (ties into Stage A's friendly AI count), then playable-as-marine if Tyler wants it (ask him to define "marine mode" before building: AI allies, playable marines, or both).
3. Done when: works on stock maps without any third-party mod installed, and survives save/load and a hosted game.

## Stage 6 - Appearance: visor colours, effect colours, Elite lighting
1. Visors: exact per-player colour in matches (task-1). Use `RE-VISOR-COLOR-2026-11-15.md` plus the retail appearance code. Keep the stock-slot fallback until the new path is proven in a hosted match with two different colours visible at once.
2. Armour effect colour variants selectable in the Armory (task-6): profile effect index -> event StringId trace, per `RE-ARMOR-FX-COLORS-2026-11-15.md`.
3. Elite lighting/shine in Armory and in game (MASTER D1/D2).
4. Anything that must replicate to other players is a networking boundary: document it for Claude.

## Stage 7 - UI pass
Scoreboard and lobby layouts for larger counts, the new Firefight/marine options, hex-number bug if still open, parked UI items from task-8. Screenshot every changed screen (AOC monitor, `-NoFocus`).

## Stage 8 - Automatic AI pathfinding for custom/Forge maps (largest project; do last, in steps, report after each)
1. Research: read retail `path.obj`, `ai_sector_structures`, `path_influence_map`, objectives code in `decompiled_v3`; document the sector/nav data format the engine expects and where a map stores it. Check how Halo 4 handles it for comparison.
2. Proof: generate nav data offline for ONE simple Forge map and get one AI squad to walk it. No UI.
3. Tool: a "bake pathfinding" action that map makers run on their map; output stored with the map variant or a sidecar file.
4. Behaviour: default objectives so AI seeks enemies / an objective / a marked point without per-map scripting.
5. Only then consider doing it at runtime.
Be honest at step 1 if the format cannot be generated outside the official tools; that changes the plan and Tyler needs to know early.

## Rules for this prompt
At the start of every stage, before writing code: look up how Halo 3, ODST and Halo 4 (`D:\MCC_DECOMP`) handle the same system (limits, pools, checks) and record what they changed compared with Reach in the stage report. Use it as the reference for what is known to work.
One stage at a time, in this order (order set by Tyler 2026-11-15) unless he reorders. Each stage ends with: tests listed above passed, docs updated, a short report (proven / unproven / needs Tyler / needs Claude-networking). No release without "release it".
