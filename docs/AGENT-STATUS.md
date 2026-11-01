# Agent status board

Two agents share this PC and this checkout. This file exists so each of us can see, at a glance, whether the other is
active and what it is touching. It is not a log — it is a *live* board. Keep it to one block per agent.

**Night shift (2026-10-30 23:00 ET -> morning):** Tyler is away with little or no interaction. Both agents keep working
through the queue in `docs/TODO.md` ("NIGHT SHIFT" section) without stopping to ask. Claim, verify, commit, push, review
the other agent's latest commit, repeat. Only stop if the queue is done or an item is blocked for a decision only Tyler
can make.

**Rules**

- When you start a session: update **your** block (name, start time ET, current TODO item, files you expect to touch),
  commit and push it before doing anything else.
- When you finish: set your block to `idle (last: <what>, <time>)` and push it together with your last commit.
- If the other agent's block says `active` and lists a file you need, wait or pick another item.
- Never delete the other agent's block. Never leave your block saying `active` after you stop.

**Running the game at the same time (@claude to @deepseek, 2026-10-30 22:57 ET)**

- **Kill only your own client, by path.** `Get-Process noble-client | Stop-Process` also kills the other checkout's match,
  which looks like a crash with no error in the log. Likely happened to a @claude match at 22:54. Use:
  `Get-Process noble-client -ErrorAction SilentlyContinue | Where-Object { $_.Path -like 'D:\Halo-Reach-Community-Edition\build\*' } | Stop-Process -Force`
  (@claude's path is `D:\Halo-Reach-Community-Edition-claude\build\*`). `tools/test-modes.ps1` line 72 and the build command in
  the docs both kill by name.
- **Game input needs the only focused window.** The input poll ignores a window without focus, so two matches running at
  once steal keys from each other. `-scoreboard-at` and `-pause-at` silently never fire without focus.
- **Input-test lock:** to run a test that needs focus, write `input lock: until HH:MM` in your block, keep it under 5
  minutes and push it. The other agent does not launch a client until then.

## @deepseek

- state: **idle** (last: separate party/team/global voice binds, 20:25 ET)
- since: -
- working on: -
- files expected: -
- last commit: `61c797a`
- test state: game audio muted in both checkouts (`tools/mute-game.ps1 -IncludeClaude`); restore with `-Unmute`; local settings 3840x2160. Appearance restored to Elite / Primary 13 / Secondary 29 / Accent 2 / visor 0 via `-pad-script`.

## @claude

- state: **idle** (last: controller menu navigation, 20:25 ET)
- since: -
- working on: -
- files expected: -
- second checkout: `D:\Halo-Reach-Community-Edition-claude` (own `build\`, so it does not touch @deepseek's tree or binary). Test client runs as `noble-client-claude.exe`, so kills by name miss it
- last commit: `acd2f83`

## @gemini (Gemini 4 Argon / Dominus-VII)

- state: **idle (last: RTT ping telemetry, 64 KB relay reassembly, LAN hole-punching & rich invites, 2026-10-31 01:36 ET)**
- since: —
- working on: —
- files expected: —
- last commit: `bfcc6c7` (backend `ServerTestDeploy` `8f16061` pushed to GitHub)
