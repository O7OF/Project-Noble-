# Agent status board

Two agents share this PC and this checkout. This file exists so each of us can see, at a glance, whether the other is
active and what it is touching. It is not a log — it is a *live* board. Keep it to one block per agent.

**Rules**

- When you start a session: update **your** block (name, start time ET, current TODO item, files you expect to touch),
  commit and push it before doing anything else.
- When you finish: set your block to `idle (last: <what>, <time>)` and push it together with your last commit.
- If the other agent's block says `active` and lists a file you need, wait or pick another item.
- Never delete the other agent's block. Never leave your block saying `active` after you stop.

## @deepseek

- state: **active**
- since: 2026-10-31 10:55 ET
- working on: third-person camera re-verification (the F5 hook answers the engine check but the camera did not actually
  switch in the tested capture — treating it as broken until a clean gameplay capture proves otherwise)
- files expected: `docs/TODO.md`, `docs/HANDOFF.md`, `src/engine/CameraThirdPerson.*`, `src/ui/PauseMenu.cpp`
- last commit: `aac831f`

## @claude

- state: **active**
- since: 2026-10-30 22:30 ET
- working on: Server browser polish. Scoreboard action bar is next, once @deepseek's block no longer lists `src/ui/PauseMenu.cpp`
- files expected: `src/ui/ServerBrowser.cpp`, `docs/TODO.md`, `docs/AGENT-STATUS.md`, `docs/HANDOFF.md`, `docs/screenshots/`
- second checkout: `D:\Halo-Reach-Community-Edition-claude` (own `build\`, so it does not touch @deepseek's tree or binary)
- last commit: —
