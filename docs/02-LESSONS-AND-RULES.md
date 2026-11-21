# Project Noble - rules, lessons, pitfalls, key facts
Read before doing anything. Sources: every handoff in this folder.

## Tyler's standing rules
- **Prove every cause** with a one-variable test and a capture/log before telling Tyler. Check captures at full
  resolution (images > ~3.7 MB: shrink or crop first). Only view screenshots Tyler names.
- Tyler is **not a C++ developer**: exact commands, **one per code block**. He uses PowerShell (`.\build-dev.bat`, with the dot).
- Tell Tyler when results are ready. Keep working without asking unnecessarily, but ask before irreversible choices.
- **Never run tests while Tyler is playing**: `Get-Process noble-dev, noble-client` first. Close his game only when he says so.
- Test windows on the **AOC (second monitor)**: `NOBLE_WINDOW_SECONDARY=1` (all tools set it). Never his Samsung (primary).
- Back up and restore `build\bin\Release\data\appearance.dat` around every test (tools do; check hashes).
- **Never modify** the installed haloreach.dll or MCC's map files. HREK edits get backups (`HREK\noble_mods\<name>\`).
- Keep game assets and RR data **out of the repo** (`*.map` gitignored). This knowledge folder is git-excluded.
- After every menu build: copy `HREK\maps\mainmenu.map` -> `build\bin\Release\data\menumaps\noble_menu.map`, run `tools\menu_sidcsv.ps1`.
- Branch: work on **wip-merge**. Tyler asked (11-14) that the merged build go into **main** once a match runs normally.
- **Commit, pull, push cadence (Tyler, 2026-11-14):** every time a *major feature is finished fully* (verified in-game
  or proven, not half-done), pull the latest, commit it, and push it. Do not let a finished feature sit uncommitted
  across sessions, and do not push half-finished work. `main` only on Tyler's explicit word (the 11-14 push was one).
- Commit messages, PRs and docs carry only the authors' names: no "Co-Authored-By: Claude", no
  "Claude-Session" line, no "generated with" line (Tyler 2026-11-20).
- Update the handoff + TODO-MASTER (and this folder) before finishing.

## How to work (what has worked)
- Build: `& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build --config Release --target noble-dev noble-client noble-dedicated`
  New .cpp files go into CMakeLists.txt (both source lists; dedicated list too when needed).
- Test scripts (`claude-tests-2026-11-11\tools`): camp_test.ps1 (campaign), match_elite.ps1 (-Extra -NoF5), match_marine.ps1,
  forge_test.ps1 (-NoOverride), armory_preview.ps1, run-quiet.ps1, sheet.ps1 (contact sheets), menu_test.ps1.
- Tag work: ManagedBlam via `runmb.ps1 -Script X.py -Out file -Env 'K=V;...'`; tool.exe for builds/reimports
  (`build_map.ps1 -Scenario <path>`). Compressed MCC maps: read with Reclaimer (`dotnet fsi rc_*.fsx`).
- Engine RE: `disasm.ps1 -Start <rva> -Stop <rva>`, `find_callers.ps1`; hardware write breakpoints (VEH, DR0) find who
  rewrites memory; minidumps parse in Python; noble-dev.map resolves Noble addresses (base 0x140000000).
- Device bridge: stage -> edit a copy in a FRESH outputs dir -> commit with expectedMtimeMs -> re-stage and compare.
  Keep CRLF. desktop-commander gives a real shell on Tyler's PC (allowed: the repo, Desktop, D:\Steam).
- Crash evidence: noble-dev.log, noble-dev-crash.dmp, WER `C:\ProgramData\Microsoft\Windows\WER\ReportArchive\AppCrash_*`.

## Lessons (mistakes not to repeat)
- Test with **obviously different** pieces/values (odst_light, not space_marine) and on Tyler's real loadout/camera, with quick clicks.
- Know who you are looking at: a third-person "Spartan" was Kat. Read the log line that names the player unit.
- Don't let a test script undo the thing under test (scripted F5 turned third person off).
- Hooks must fire where you think: the cache step +0x30698 only runs for extra caches.
- Writes from the render thread get overwritten by the sim->render memcpy (haloreach+0x1CD389): apply object state on
  the **simulation thread** (ReachArmory::NotifyMatchTick / FillLocalInput path).
- HREK builds read split RESOURCE tags (`*.scenario_cinematics_resource`, `*_bipeds_resource`, ...) over the scenario's
  own blocks, and compile the script stored in the hs_source TAG, not the data .hsc. Failing scripts are silently stripped.
- Duplicate unique ids / invalid objects in a scenario strip the scripts that use them (check build log "design:hs").
- Bitmap import settings matter more than pixels: curve mode, usage (Diffuse / Normal 19 / Self-Illum 5), forced format.
  Don't "fix" brightness by darkening textures.
- Never write into a menu-map GLOBALS table before its script has started (GPU driver crash).
- UTF-8 BOMs break line parsers (pmcg). Strip them.
- Scene numbers above 0x14 do nothing in the menu map.
- ManagedBlam: DuplicateElement, not AddElement, for new variants; never return a TagFieldBlock from a PowerShell function.
- PowerShell: `$t` == `$T`; `[IO.File]` relative paths resolve to System32; `R` is a built-in alias; never `@{u}` (hangs);
  wrap array concat in -ArgumentList with parentheses.
- MSVC: `near`/`far` are macros; `__try` functions can't hold objects with destructors (C2712) - split them.
- Re-using the same /mnt/user-data/outputs path can keep stale content: verify size/hash after staging.
- Merges: untracked files with the same names block a merge (move them aside first); dedicated build needs
  `#ifndef NOBLE_DEDICATED` around client-only calls (AuthClient, DevCheats, Career, PauseMenu).

## Key engine facts (haloreach.dll RVAs)
| What | Where |
|---|---|
| Live tag table | `*(haloreach+0xC1A600)` |
| Zone-set rebuild (Trooper hook) | +0x8EAF0 (prologue 48 8B C4 48 89 58 10) |
| Extra-cache step | +0x30698 |
| matg player representation block | +0x474 / +0x478, stride 0x6C, biped at +0x40 |
| Set permutation | +0x4729F4 (region lookup +0x1CBF0C, perm lookup +0x1CBF98, notify +0x4730CC) |
| object_set_variant | +0x472B70 (calls +0x472C20, +0x472C68, +0x479D08, tail +0x264A50) |
| SetChangeColor | +0x473858 |
| Player setup / variant enforcement | +0x6A4BA0 (player stride 0xD00) |
| Sim->render state memcpy | +0x1CD389 (inside +0x1CD2BC) |
| Emblem extern 16 | +0x261B00; host slots 73/74 (vtable +0x248/+0x250 of [haloreach+0xC1A100]) |
| Armory yaw write (RR workaround) | +0xBE5AD8 |
| Forge on Firefight crash (old) | +0x10DB1B |
| Collision walk stack overflow | +0x150B74 (fixed by CollisionWalkGuard) |
| Object's first dword | its definition datum |
- `.mapinfo` byte +0x42 = engine type (0 MP, 4 Firefight, 8 campaign). VFS serves `<scenario>_normal.map` for `<scenario>.map`.
- Menu GLOBALS markers rr_mark_a 0x03BB6FED / rr_mark_b 0x03BAB653; 16 string-id pairs at slots 4..35; 52 slots.
- Stock MCC maps are compressed (Noble's string-id lookup reads 0 names there).
- Monitors: Samsung LS32D80 = primary (Tyler); AOC 24E2QA = second (tests).

## Where things live
See `00-START-HERE.md` "Folder index" and "Things NOT copied".
