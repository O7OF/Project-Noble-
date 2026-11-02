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

- state: **idle** (last: 0.9.16.35 restored Armory map after confirming the new 268 MB menu has no character scene; armor/visor/camera tested, 05:50 ET; release published)
- since:  -
- working on:  -
- files expected:  -
- last commit: `0.9.16.35` local (pending push)
- test state: local settings left at 1280x720 windowed (borderless applies live), audio unmuted; `noble-client` and `noble-dev` rebuilt at 05:48 ET. Release https://github.com/O7OF/Project-Noble-/releases/tag/v0.9.16.35. Test captures in `scratch/` (`old-armory-crop.png`, `scene_sweep_sheet.png`, `live-borderless.png`).

## @claude

- state: **idle** (last: 0.9.16.32 menu UI scaling at native 4K; 0.9.16.31 exclusive FULLSCREEN + Borderless default, 2026-11-02 ~04:15 ET)
- since:  -
- working on:  -
- files expected:  -
- test state: Tyler's settings.json restored (Borderless, no exclusiveFullscreen). Desktop is native 3840x2160 (it ran at 1920x1080 before 03:30 ET). Ignored data/profile_1.bin + data/menumaps/ copied in for blocks.rc.
- second checkout: `D:\Halo-Reach-Community-Edition-claude` (own `build\`, so it does not touch @deepseek's tree or binary). Test client runs as `noble-client-claude.exe`, so kills by name miss it
- last commit: 0.9.16.32 (see git log)

## @gemini (Gemini 4 Argon / Dominus-VII / @O7OF)

- state: **idle** (last: handed 0.9.16.23 main-menu/Armory camera work to @deepseek, 01:15 ET)
- since:  -
- working on:  -
- files expected:  -
- last commit: `0.9.16.22` Release x64 verified clean (0 errors, 0 warnings)
