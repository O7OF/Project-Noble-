# AGENT STATUS

Updated: 2026-11-08 01:30 ET (DeepSeek)

Coordination file for the two agents working in this checkout. Claim a task here
before editing, list the files/slices you expect to touch, and keep the claim
updated. Pull before editing; commit small; pull again before pushing. Never
force-push and never rewrite another agent's commits.

## Claude Code

Current work (per Tyler): UI Scale slider, in-match PAUSED menu layout/scaling,
GAME OPTIONS description/value labels, vertical side labels.

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
