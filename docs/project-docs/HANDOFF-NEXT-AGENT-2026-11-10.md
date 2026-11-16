# Project Noble: handoff to the next agent (2026-11-10, evening)
(Copy of claude.ai project doc. Historical - version 0.19.46 era.)

`HANDOFF.md` is OLD (2026-10-28, old repo path `D:\Halo-Reach-Community-Edition`, version 0.8.x). Still useful as engine-fact reference (host slots, launch block, profile layout).

## Summary of that session
Tyler tested earlier mod-bug fixes: pause menu lag fixed; V voice hint works; menu art and loading flavour text work; 4K loading layout good; Civil Unrest m10 hangs on loading; Armory crash.

### 1. Armory crash
- Crash #1: WER Report.wer showed exit 0xC0000409 (FAST_FAIL_FATAL_APP_EXIT) in ucrtbase = abort()/std::terminate. Noble's SEH filter never sees that.
- Crash #2: caught access violation on the Armory scan thread inside `ScanColorRecordsIn`, reading a vector pointer overwritten by colour floats. Hypothesis: byte-pattern scanners matched inside a thread stack and colour writes corrupted live data.
- Fix written: `src/engine/ReachArmory.cpp` enumerates every thread's stack range (Toolhelp + NtQueryInformationThread TEB, StackBase TEB+0x08, DeallocationStack TEB+0x1478), skips them in `ReadWriteChunks`, drops candidates inside a stack (`DropStackAddresses`), re-checks at write time. (Real root cause later found: writing to the uninitialised GLOBALS table - see NOBLE-MENU-MAP-FINDINGS.)

### 2. Crash logging
`src/main.cpp`: SIGABRT, invalid-parameter and pure-call handlers -> `NobleTerminateHandler` (stack log + dump); run-flag marker `running\<pid>.flag`. `FrontEndMenu.cpp` `ContainerMatchEnded` logs `the match process CRASHED, exit code 0x...` with meanings for 0xC0000005 / 0xC0000409 / 0xC00000FD / 0xC0000374 / 0xD3D10001 / 3. Hard kills, driver resets, power loss: use WER (`C:\ProgramData\Microsoft\Windows\WER\ReportArchive\AppCrash_*\Report.wer`) and Event Viewer.
- Crash address resolution: `noble-dev.map` (base 0x140000000, .text RVA 0x1000) + `objdump -d -M intel noble-dev.exe`. Minidump parseable in Python (exception stream type 6).

### 3. Civil Unrest mod (workshop 3797186862)
- Campaign maps m10, m30, m35, m45, m50, m52, m60, m70, m70_bonus, 30_settlement, ff_unearthed. m10 from the mod hangs on load (known custom-map load failure).
- Facts: VFS serves `<scenario>_normal.map`; `.mapinfo` byte +0x42 = engine type (0 MP, 4 Firefight, 8 campaign); Forge on a Firefight canvas crashes at haloreach+0x10DB1B.

### 4. Other open items then
- Pause does not freeze the game (never has): `game_speed 0` via `BlamEngine::ExecuteCommand` (vfunc 9) has no effect.
- Friend's freeze on joining: unproven GPU driver hang (AMD). Mitigations: 60 FPS cap until session start, Armory scan paused during a match, device-removed handling in the Present hook.
- Multiplayer version gate (`BuildIdentity.hpp` CompareBuilds, ServerBrowser). A version bump needs the master server's REQUIRED_BUILD_PREFIX updated.

## Pending decision then
`CMakeLists.txt` referenced UNTRACKED 32-player files (`src/host/SlotMessage.cpp/.hpp`, `src/engine/PelicanControls.cpp/.hpp`, `SlotMessageTable.inc`, `SlotMemberCaves.inc`); pushing as-is breaks a fresh checkout. The 32-player work (LIMITS-HANDOFF.md) is paused.

## Device bridge workflow (then)
Stage the file from the PC, edit a copy (preserve CRLF; edit as bytes in Python), copy to `/mnt/user-data/outputs/...`, `device_commit_files` with `expectedMtimeMs`, re-stage, `cmp`. (Later sessions also had desktop-commander with a real shell.)

## Dev build facts
Dev builds write `noble-dev.log` and `noble-dev-crash.dmp`; the shipping client writes `noble-client.log` and `noble-crash.dmp`. Menu process (ImGui, FrontEndMenu.cpp) runs its own haloreach engine (mainmenu map, 3D Armory) and launches matches in a child process inside a container window.
