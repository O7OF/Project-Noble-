# Working together: Claude + DeepSeek on Project Noble

Two agents work this repo with Tyler in the loop. There is **no direct chat between them** — the repository is the
handoff channel. This file defines how to avoid collisions and how to tell what the other is doing.

## The two channels

| channel | purpose |
|---|---|
| `docs/TODO.md` | *what* is being worked on and *who* has it. This is the claim board. |
| `docs/HANDOFF.md` | *what happened*, newest entry first, with verification evidence. This is the log. |

Both agents must `git pull` before reading and before starting work.

## Claiming an item

1. `git pull`
2. Open `docs/TODO.md`, find the item, and change its line to claim it — one line, no separate file:

```
- [~] @claude 2026-10-31 10:30 — Third-person distance/shoulder tuning (follow hook at +0x1DCADC)
```

Initials are `@claude` or `@deepseek`. Move `[ ]` → `[~]`. Commit that one-line change **immediately** and push, so the
other agent sees the claim.
3. When done, rewrite the line once more:

```
- [x] @claude 2026-10-31 12:10 — Third-person distance/shoulder tuning (follow hook) — commit abc1234
```

Then add the handoff entry (below) and push.

**If an item is already `[~]` by the other agent, do not touch it.** Pick the next unclaimed item, or work on docs.

## Handoff entries

Every completed piece of work gets, at the very top of `docs/HANDOFF.md`:

```markdown
--

## 0. Pick up here (2026-10-31, 12:10 ET) - Third-person distance and shoulder tuning (@claude)

What changed, why, and how it was verified. Include the exact commands/flags used, the commit hash, and where any
screenshot lives (`docs/screenshots/...`). Mention anything the next agent must know (traps hit, flaky behaviour).
```

Rules: newest first, one entry per change, timestamps in **ET**, short headings, no secrets, no huge logs (link files
instead). Then `git commit` + `git push` and mirror the handoff to `C:\Users\tyler\Downloads\HANDOFF (2).md`.

## Commit and push discipline

- Small commits, message shape `area(scope): summary` — e.g. `feat(ui): centre the emblem picker rows`.
- The body explains **how it was verified** (build 0/0, capture file, local pair test, server endpoint).
- Push to `main` (solo project, no PRs). **Never force-push.**
- `git status` must be clean when you stop. No half-finished code in the tree.
- Never commit `build/`, `data/userdata/*.json` test artifacts, `database.db`, or `*.log`/`*.bmp` (except deliberate
  `docs/screenshots/`).

## Verification bar (do not report "done" without it)

| change | required evidence |
|---|---|
| C++ / engine | build **0 errors, 0 warnings**; relevant runtime log lines; capture for visible changes |
| UI / menus | a committed screenshot under `docs/screenshots/` (use the capture harness) |
| netcode / co-op | a two-process local run (`-data-dir` for the guest) plus the log lines showing both sides in game |
| server (`ServerTestDeploy`) | `node --check server.js`, a local run with real endpoint requests, then push; Render redeploys |

Close `noble-client.exe` when done testing.

## Ownership that avoids conflicts

These files are edited by both agents most often — claim the *area*, not just the file, before touching them:

- `src/ui/FrontEndMenu.cpp` (menus, lobby start path, launch composition)
- `src/ui/PauseMenu.cpp` (pause, post-game, scoreboard overlay, loading screen)
- `src/engine/BlamEngine.cpp` (launch block, host/guest sequence, campaign grace/barriers)
- `src/main.cpp` (CLI flags and their *placement before the host/guest split* — this has bitten us three times)
- `CMakeLists.txt` (source lists for client vs `noble-dedicated`)
- `server.js` in the **ServerTestDeploy** clone, not this repo

If you must touch a file another agent is actively editing, say so in the TODO claim line and keep your diff minimal.

## Suggested split (not binding)

| agent | strengths to use |
|---|---|
| **@claude** | menus/UI: emblem picker, controller presets, readability pass, post-game names, server browser polish, design-spec work, docs |
| **@deepseek** | engine/netcode: third-person follow tuning, campaign checkpoints/CONTINUE, 4+ players (RR capacity path), vote-to-kick transport, server scripts |

Either agent may take any unclaimed item — the table is guidance, not a rule.

## Tyler's standing instructions

- "Always update the handoff please."
- "Any netcode or server script changes need to be pushed to `ServerTestDeploy`."
- "Close the noble client when done, test everything."
- One shell command per code block when giving him something to run.
- He is not a C++ developer: explain the *what/why* briefly, give exact commands.

## When tokens run low

Nothing special: the repo is the memory. A fresh session starts by reading `docs/START-HERE.md` → `docs/TODO.md` →
top of `docs/HANDOFF.md`, then continues with the first unclaimed item. Do not re-litigate decisions recorded in the
handoff; if you disagree, write the entry and say why.
