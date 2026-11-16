# Monitor option + quiet test runs + shine analysis state (2026-11-12 06:35)
(Copy of claude.ai project doc.)

## Monitor option
Tyler asked for a Display option to pick the monitor and auto-detect its resolution.
- `common/Settings.hpp/.cpp`: `int monitor` (0 = Auto/primary, 1..N; saved as `"monitor"` in settings.json).
- `host/Window.cpp/.hpp`: `Window::SetMonitorSetting/MonitorCount/MonitorSize`. Displays numbered primary first, then left to right. Window created on the chosen display (reuses the `NOBLE_WINDOW_SECONDARY=1` block); `ApplyWindowSettings` moves it live. Falls back to primary.
- `main.cpp`: `SetMonitorSetting(startupSettings.monitor)` before `Create()`.
- `ui/SettingsPanel.cpp`: MONITOR row (SETTINGS > GRAPHICS > DISPLAY). Picking one resets RESOLUTION to Native.

## Test runs while Tyler plays
- His public build: `C:\Users\tyler\Documents\Noble Launcher\Noble.exe` (own data dir). Primary monitor (Samsung, 1920x1080) = his game; second monitor (AOC, x=-1920) = Claude/test windows.
- Test windows must NOT open on the primary monitor. Use `NOBLE_WINDOW_SECONDARY=1` (all test scripts set it now).
- `claude-tests-2026-11-11\run-quiet.ps1`: mutes the dev build via its own settings.json, runs the Armory test, restores volume/mute/monitor.
- Warning: `tools\mute-game.ps1 -Unmute` without a backup writes volumes 1.0.

## Shine analysis then
- Stock Minor and PH Honor Guard shaders share template `elite_armor_template` and the same parameters.
- PH bitmaps (mean): armor_02 RGB 0.28 / alpha 0.25; honor_armor_01_grey RGB 0.42 / alpha 0.04; council 0.54 / 0.28; cc masks R~0.75 G~0.25.
(Resolved later: see HANDOFF-ELITE-SHINE and HANDOFF-ELITE-RR-TEXTURES.)
