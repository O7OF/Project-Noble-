# Priority 2 findings, session 2026-11-10 (companion to MOD-BUGS-2026-11-10.md and MOD-BUGS-FINDINGS-2026-11-10.md)
(Copy of claude.ai project doc.)

## #4 Pause menu
### Lag on open: FIXED (Tyler confirmed)
- Cause: `PauseMenu::DrawMenu` called `Career::Challenges()` every frame. Fix: list built on open, refreshed at most every 2 s; warmed on a background thread at launch.

### "Doesn't pause": NOT a mod problem; never froze the game, stock either. NOT fixed.
- `SetLocalSimulationPaused` sends `game_speed 0` through `BlamEngine::ExecuteCommand` (vtable slot 9). Dispatched, `Command status: 0x0`, no effect. Test `-engine-cmd "game_speed 0.2" -engine-cmd-at 25` in m10: no slow-down.
- Untried: (a) find the engine's real pause/game-time flag (RE, read-only); (b) engine `PostCommand` ids; (c) hold the engine's per-frame callback (slot 1) while the menu is open.

## #5 V does nothing: by design without a voice session
- Voice (`src/audio/VoiceChat.cpp`) only opens a session while the relay is connected. Added a "VOICE CHAT: NO SESSION" / "OFF IN SETTINGS" hint for 2.5 s. CONFIRMED.

## #6 Third-person and mods
- `SettingsPanel.cpp` THIRD-PERSON ZOOM note. `CameraThirdPerson.cpp` `NobleShapesCamera()`: when Noble's third person is off and a mod is active (`VFS::HasActiveMod()`), Noble's tuning is skipped.

## #7 Mods' loading/menu art: FIXED
- `ModMaps` in `FrontEndMenu.cpp` known-slot branch now copies `imageThumbnail`, `imageLarge`, `imageLoading` from the mod mission JSON. Loading flavour text via `PauseMenu::SetLoadingMapDescription`.

## #7b Loading screen text overlap at 4K/2K: FIXED (offsets scale with h/1080).

## Menu option rows (PickerRow/ClickRow), multiplayer version gate (`CompareBuilds`, ServerBrowser AttemptJoin/JoinBlockedByVersion) - written at the time.

## Friend's PC freeze on joining
- Not a version mismatch; everything stalled, power button needed. Suspect AMD GPU driver hang; unproven. Ask for Event Viewer: Display 4101, Kernel-Power 41, driver version. Mitigations: 60 FPS cap until `s_sessionStarted`; Armory scan waits while a match runs; match process ends on `DXGI_ERROR_DEVICE_REMOVED/RESET`.

## Civil Unrest custom-map load
- m10 from the mod: map served, engine hangs. Same as known custom-map load failure (upstream `docs/mod-map-mount-findings.md`).

## Armory crash + crash logging - see HANDOFF-NEXT-AGENT-2026-11-10.md.

## Merge notes (origin/main 64fd5c7)
- 8 conflicts in 6 files resolved: FrontEndMenu.cpp, VFS.cpp, Window.cpp, PauseMenu.cpp (local 4K loading layout), ServerBrowser.cpp (local gate + upstream join behaviour), DevMenu.cpp (local Numpad spawn buttons).
- Upstream (O7OF) mod-map work: VFS serves `<scenario>_normal.map` for `<scenario>.map`; Forge on a Firefight canvas crashes at haloreach+0x10DB1B (null game-engine globals).
