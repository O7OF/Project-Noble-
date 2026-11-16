# Hand-off for the lead agent: hex-number UI bug runtime check (from Claude, 2026-11-15)

Why: Static analysis of the full MCC haloreach.dll decompile (same build Noble loads) found the probable cause of the
"0000001c" / "Respawn in 00000001" text: `FUN_1802d288c` (hr+0x2D288C) expands `\a<8 hex digits>` placeholders. When
`*(int*)(ctx+0x180) == -1` (or the digits don't parse) it drops the `\a` but leaves the 8 digits in the output.
Read first: `project-docs\NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md`.

Task (lead owns src\ and running tests; Claude did NOT touch the repo):
1. Add `hexbug_diag.cpp` (this folder) next to the other diagnostic hooks (e.g. [tokendiag] in ReachHost.cpp) and call
   `hexbugdiag::Install()` after haloreach.dll loads and MH_Initialize() ran. Route LogLine to Noble's logger.
2. Build noble-dev, run a match (or whatever scene shows the number: scoreboard, "Respawn in ...") with `-NoFocus`,
   per the test rules (check `Get-Process noble-dev, noble-client` first; AOC monitor).
3. Grep the log for `[hexbug]`. Report: do LEAK lines appear, and is `ctx+0x180` -1 (INVALID) on them? Include stringId,
   text and caller (hr+0x...) for a few lines.
4. If confirmed: the real fix is to give that string context a valid handle at +0x180; fallback patch is to skip the 8
   digits on failure (set the read pointer to `pwVar7 + 9` once 8 hex digits were parsed). Ask Claude to trace the
   context creation in the decompile (`D:\MCC HALO REACH DECOMP\decompiled\hr_00004.c`, callers at lines ~71443 and ~71603).
5. Remove the hook before committing. Record the result in Recent results (AGENT-COORDINATION) and the TODO.
