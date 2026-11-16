# Hex-number UI bug ("0000001c", "Respawn in 00000001") - runtime check result (2026-11-15)

Source: `decompile-exports\mcc-haloreach\hexbug_diag.cpp` + `project-docs\HANDOFF-LEAD-HEXBUG-HOOK.md`.
Owner: lead. Status: hook installed, run, **not confirmed**, hook removed again; repo clean.

## What was done

- Added the temporary `hexbugdiag` hook (`src/engine/HexBugDiag.cpp/.hpp`) and installed it from
  `EnsureUiCursorHook()` (first input poll: haloreach.dll loaded, `HookManager`/`MH_Initialize` already ran).
- Detour target hr+0x2D288C (`FUN_1802d288c`), signature confirmed against the Ghidra decompile:
  `ulonglong F(longlong ctx, int* stack, longlong p3, int stringId, wchar_t* out, longlong cap)`.
- Rebuilt `noble-dev`, ran with `-NoFocus` on the AOC:
  - multiplayer: `-map 20_sword_slayer -mode multiplayer` (~22-45 s runs, plus one with `-suicide-at 15`);
  - campaign: `-map m35 -mode campaign` (~40 s).
- The hook was removed afterwards and the tree returned to the last pushed commit; `noble-dev`/`noble-client` rebuilt
  clean at 0.19.102.

## What the log showed (`[hexbug]` lines)

- Hook installed once per run: `[hexbug] installed on hr+0x2D288C`.
- The expander was exercised (hundreds/thousands of calls in a run). Examples:
  - `call=1 stringId=20806 handle=-2111235188 hexRun=0 ret=... cap=1024 text="Respawn in 0" caller=hr+0x2D2592`
    (this is the respawn label string; repeated 4x at match start and again later in the run)
  - `call=6 stringId=20983 handle=-2110514281 hexRun=0 ret=1 cap=1024 text="armor lock" caller=hr+0x2D2592`
  - campaign m35: `call=1 stringId=52378 handle=-1432798827 hexRun=0 ... text="Respawn in 0" ... caller=hr+0x2D2592`
- **No LEAK line**: no expanded output started with, or contained, 8 hex digits (`hexRun=0` for every logged call).
- **No `ctx+0x180 == -1` call was observed** in these scenes; the observed handle values are large negative ints
  (e.g. -2111235188, -2110514281), not -1.
- Important caveat: the function's first action is a duplicate check on `param_2` (the per-context string-id stack). When
  the id is already in that list it returns 0 **without writing `out`**, so some logged `text=` heads are stale buffer
  contents, not expanded text. The first call for a string id is the one that matters; for those, no 8-hex leak appeared.
- `Respawn in 0` in the log is the beginning of the localized respawn string ("Respawn in ..." plus a countdown token);
  it is not itself a leak of 8 digits. The visible zero-padded number was not reproduced in these solo MP/campaign runs.

## Conclusion / next steps

- The runtime check **does not confirm** the static theory (that `ctx+0x180 == -1` causes the digits to be printed).
  In normal solo multiplayer and campaign the expander saw a valid handle, and no leaked digit run was produced.
  No fallback patch was applied.
- To continue, we need the exact user-facing repro: which screen and mode showed "0000001c" / "Respawn in 00000001"
  (e.g. engine scoreboard vs Noble's panel, Firefight/co-op result screens, a specific weapon's HUD, Forge).
- Alternative static follow-up (as the hand-off suggested): trace who creates the string context that reaches
  FUN_1802d288c and whether any caller can leave +0x180 == -1 (decompile callers around hr_00004.c:71443 and :71603 in
  the MCC export). If such a context exists, log the stringId at the failure path rather than only checking the output.
- The queued `-suicide-at` test helper used for this check was also removed; no diagnostic code remains in the repo.

## Evidence

- Log: `build\bin\Release\noble-dev.log` (search `[hexbug]`; lines at 2026-11-15 06:31-06:39).
- Captures while the hook was active: `build\bin\Release\data\userdata\claude-tests-2026-11-11\death_*.png`,
  `respawn_*.png` (solo MP loadout/spawn; no zero-padded HUD number visible).

## Scoreboard follow-up (2026-11-15 07:15, cancelled by Tyler)

Tyler asked for a second run holding the scoreboard (Tab) and a check of whether Noble hides the engine scoreboard widget
or draws over it. While setting that up he confirmed he no longer sees the old scoreboard in Noble and said never mind.

Result of the code check (recorded for future reference): Noble does **not draw over** Reach's scoreboard. In
`ReachHost::FillLocalInput` the scoreboard key is removed from the engine input state (`st[0x004 + boardVk] = 0`) and only
published to the overlay through `s_engineKeyMask`; Noble's own scoreboard panel reads that and draws its own UI
(`PauseMenu.cpp`, `scoreboardHeld`). The engine's screen therefore never opens. The temporary `-scoreboard-at` /
`-engine-scoreboard-test` / `-unfocused-input-test` diagnostics and the hexbug hook used for this follow-up were removed
again; tree clean and both gates rebuilt at 0.19.102. No new hexbug findings (the only added line was the hook install).
