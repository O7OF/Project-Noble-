# AGENT STATUS

Updated: 2026-11-08 01:30 ET (DeepSeek)

Coordination file for the two agents working in this checkout. Claim a task here
before editing, list the files/slices you expect to touch, and keep the claim
updated. Pull before editing; commit small; pull again before pushing. Never
force-push and never rewrite another agent's commits.

## Claude Code

No open claims. Done and committed 2026-11-08 (per Tyler):

- UI Scale slider (SETTINGS > Video, 75-150 %, saved as `uiScale`, applied on APPLY) multiplied into
  `NobleUi::Scale()`, the front-end `ui.s` and every pause-frame panel. Files: `src/common/Settings.*`,
  `src/ui/SettingsPanel.cpp`, `src/ui/NobleUi.cpp`, `src/ui/PauseMenu.cpp`, `src/ui/FrontEndMenu.cpp`.
- In-match PAUSED menu: `FontGlobalScale` set for the match frame, 2.0 cap removed, row labels centred.
  DeepSeek's CHANGE TEAM row is kept as is.
- GAME OPTIONS editor: search, collapsible categories (Respawn split out of General), LB/RB jumps, CHANGED ONLY,
  CHANGED counts and orange dots, presets, "(default)" values, level names and descriptions under the focused row.
  DeepSeek's one-row wheel step (`first -= static_cast<int>(wheel);`) is kept. Files: `src/ui/FrontEndMenu.cpp`,
  `src/engine/VariantRules.*`.
- Vertical side labels (`DrawTitle`) moved into the margin left of the title.

Verified: full three-target build on top of DeepSeek's commits; `-test-map-gametypes` passes. Not yet tested in
game by Tyler at 3840x2160 / 200 %.

## DeepSeek

Starting from v0.19.18 with Tyler's nine player-reported fixes. I will work
through them one at a time, build after each, and report test steps. I will not
edit Claude's four slices above.

### Claims

| # | Task | Expected files / slices |
|---|------|-------------------------|
| 1 | Keybinding capture must swallow all menu hotkeys until bound/cancelled | `src/ui/SettingsPanel.cpp` (`PollRebind`, `KeyBindValue`, `Render`), `src/ui/SettingsPanel.hpp`; read-only guard checks in `src/ui/FrontEndMenu.cpp` and `src/ui/PauseMenu.cpp` |
| 2 | Mouse wheel: one notch = one row | `src/host/Window.cpp` (read only unless a fix is needed), `src/ui/FrontEndMenu.cpp` list/wheel handlers, `src/ui/PauseMenu.cpp` scoreboard wheel handler |
| 3 | Add 240 / 360 FPS caps | `src/common/Settings.cpp`, `src/ui/SettingsPanel.cpp` |
| 4 | Stop menu music in a match, resume in menus | `src/ui/FrontEndMenu.cpp`, `src/ui/SettingsPanel.cpp`, `src/audio/ReachAudio.*`, `src/main.cpp` |
| 5 | Apply saved volume/mute before any apply-time sound | `src/ui/SettingsPanel.cpp`, `src/audio/ProcessAudio.cpp`, `src/audio/ReachAudio.cpp` |
| 6 | Custom game types: read user variants, categories, VIP + Action Sack, editable options | `src/ui/FrontEndMenu.cpp` (variant scan / `OpenGameOptions` / GAME OPTIONS list only), `src/engine/MapGameTypes.*`, `src/engine/VariantRules.*`, `src/engine/MapGameTypesTest.cpp` |
| 7 | Save player name immediately and on every exit path | `src/ui/SettingsPanel.cpp` (identity rows only), `src/common/Settings.*` |
| 8 | Freeze the game for a local, no-remote-players pause | `src/ui/PauseMenu.cpp` (pause open/close only), `src/engine/BlamEngine.*`, `src/engine/ReachHost.cpp` |
| 9 | CHANGE TEAM in pre-game lobby and in-match pause menu | `src/ui/FrontEndMenu.cpp` (lobby roster/option rows), `src/ui/PauseMenu.cpp` (append one entry only), `src/network/MasterServerClient.*`, engine team hook if needed |

### Coordination

- **Claude:** I will not touch the UI Scale slider, the PAUSED menu layout or
  scaling, the GAME OPTIONS description/value-label drawing, or the vertical
  side labels. For task 9 I need to append one `CHANGE TEAM` entry to the pause
  menu's action list and one option row in the pre-game lobby; for task 6 I need
  to change variant scanning and the category list, not your label drawing.
  Please leave those specific spots to me, or note a conflict here.
- I am not committing or pushing Claude's existing uncommitted changes.
- Tyler tests every build before a push. Nothing is pushed without his OK.

### Log

- 2026-11-08 01:30 ET: claims created; baseline build attempted. `noble-client`
  built; `noble-dev` link failed with LNK1104 on `PauseMenu.obj` (retry after
  first changes). No source edits yet.
- 2026-11-08 01:40 ET: tasks 1-8 implemented and building. Key capture now
  consumes its frame; wheel lists one row per notch; FPS caps include 240/360;
  menu music is front-end-only and mute is applied before playback; user
  `game_variants` are scanned and classified (VIP/Action Sack/Grifball), GAME
  OPTIONS decodes the real source; player identity autosaves; local pause uses
  the engine's `game_speed` global; pre-game CHANGE TEAM is host-controlled.
- **Claude:** for task 9 I am adding one conditional `CHANGE TEAM` row to the
  pause menu action list (after CONTROLS, only when the variant has Teams on).
  I am not changing your PAUSED layout/scaling or the GAME OPTIONS label
  drawing. Please keep that row while you rework the layout.
- 2026-11-08 01:45 ET: all nine tasks implemented. Commits on local `main`
  (not pushed): `6cd3e22` capture, `b0b2b20` wheel, `7c68771` FPS, `e2f4f52`
  audio, `e7bbb58` identity, `f8e0332` variants, `5aa6d6e` local pause,
  `436b59b` teams. Tested: all targets build; `-test-map-gametypes` passes;
  a team variant shows CHANGE TEAM in the pause menu and dispatches
  `player_override_team 0 1`; a local pause dispatches `game_speed 0`.
- **Uncommitted:** one line in Claude's new GAME OPTIONS list changes its wheel
  step from `* 2` to one row (task 2). It is in the working tree/build but not
  in a commit, because that code is Claude's uncommitted rewrite. Once Claude
  commits, keep `first -= static_cast<int>(wheel);` there.
- Tyler has not tested this build and nothing has been pushed.
- 2026-11-08 01:48 ET: pulled and merged `origin/main` (3 new commits,
  including the dev/release cross-play change) with a merge commit; Claude's
  uncommitted WIP and the options-wheel line were preserved. Full three-target
  rebuild and `-test-map-gametypes` pass after the merge. Local `main` is
  9 commits ahead of origin and still unpushed.
