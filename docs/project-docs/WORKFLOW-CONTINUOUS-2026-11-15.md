# Continuous work workflow - Project Noble (2026-11-15 day session)

Purpose: keep the remaining queue moving without Tyler prompting each step, while staying safe (no focus/mouse theft),
keeping the repo releaseable, and leaving an audit trail every iteration.

Read this file + `WORKFLOW-LEDGER-2026-11-15.md` at the start of **every** iteration, then pick the first READY task from
the shared task board (`team_task_list`).

## Order of work (Tyler, 2026-11-15)

1. **Main queue** - #14 visor engine colours, #15 armour effect colours, then #16 multiplayer (on hold for now; only after
   the others, and only once Tyler lifts the hold).
2. **QA / release** - only after the main queue is as done as it can be: version bump, release notes, both gates, publish
   base + Forge pack, verify assets. Release is currently HELD by Tyler.
3. **Housekeeping** - Tyler approved: stale `noble_elite_pack.map`, menu-map backups, dev fallback `mainmenu_all.map`.
4. **Parked items** - only if everything above is finished: flashlight, two-PC multiplayer test, Armory polish (knife
   chests, emblem slots 73/74, undersuit, Elite shine/lights), Forge-palette Option 2, Marine animation.

## One iteration (30-45 min, then stop even if mid-investigation)

1. Read the ledger + task board. Pick the first READY task; claim it on the shared board (record the task id in the ledger).
2. Work it with a single clear hypothesis/experiment. Use the AOC test rules:
   - launch with `tools\launch_noble.ps1` (opens on the AOC, never takes focus/mouse);
   - no focus stealing, no cursor capture: `-NoFocus`, synthetic input only;
   - never kill a `noble-dev.exe`/`noble-client.exe` that Tyler started; check `Get-Process` before launching.
3. Verify with a capture/log; save evidence under
   `build\bin\Release\data\userdata\claude-tests-2026-11-11\<item>_<name>.png`.
4. If the iteration produced a real fix: build both gates, commit with subject + body paragraphs, push to `main` and
   `wip-merge`, and note commit + evidence in the ledger.
5. If it produced only research: write/append the note in `knowledge\project-docs\` (and mirror to Compendium), record
   the finding + next concrete step in the ledger, and either leave the task in progress or create a follow-up task.
6. Update the shared task (complete / keep in progress with comment in description) and sync changed `knowledge\` docs to
   `C:\Users\tyler\Desktop\Compendium\` (SHA256 check).
7. Trim temp hooks/experiments. Never leave diagnostics in a commit unless they are useful, documented test hooks.

## Rules that never change

- Release/bump only when the queue is done and Tyler says so.
- Multiplayer work stays on hold until Tyler lifts it.
- No RR-named files ship; `knowledge\` is git-excluded; never commit assets.
- Commit style: subject + body paragraphs, no trailers. Push both branches after every commit; `main` and `wip-merge`
  stay identical for now.
- Never take Tyler's mouse/keyboard: unfocused + synthetic input, and never test while he is playing.
- If an item is blocked on Tyler (colour list, two-PC test, release permission), record the blocker in the ledger and move
  to the next READY task; do not spin on it.

## Status vocabulary

READY / IN PROGRESS / BLOCKED (waiting on Tyler or another task) / DONE. Every ledger entry needs: item, hypothesis,
evidence path, commit (if any), next step.

## Today's known blockers

- #15 needs a colour list (default assumption: blue/cyan flames, purple lightning; use this unless Tyler changes it).
- #13 needs a real two-PC host -> code -> join test (needs Tyler/second machine).
- #14 needs the higher-level model path validated (RE still open).
- Release + multiplayer hold.
