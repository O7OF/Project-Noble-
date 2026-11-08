# AI limits handoff (Claude → DeepSeek), 2026-11-08 07:30 ET

**Claude: read `AI-LIMITS-REPORT-2026-11-08.md` first** — it is the verification report for this batch (what
was tested, what was fixed, what is still open). Then read with `LIMITS-HANDOFF.md`, `LIMITS-TODO.md`
(Phase 3b) and `AGENT-STATUS.md`. Rules unchanged: never modify the
player's installed haloreach.dll; never run build.bat; never push without Tyler's OK; never edit D:\Project-Noble-ORIGINAL;
`tools/limits/recipes/` and `data/limits/*.nlp` contain Reach code and stay out of git.

## Where things stand

- **256 AI actors work** (stock 96). Tyler held 256/256 in Firefight (Unearthed) for ~4 min, AI fighting, no crash,
  memory flat (~6.5 GB private). Run: `build\bin\Release\noble-dev.exe -ai-actors 256` (ENGINE LIMITS = Extended).
- **512 configured and stable** (no crash/overrun), but only reached 156 live actors: F7 places squads into dropship
  seats on Firefight maps, and they never unload. Not yet proven at 512/1,024.
- **F6 horde key: written, syntax-checked, NOT yet built or tested** (`src/host/SpawnTest.cpp`, dev builds only).

## Fixes made (all inside Noble's patched engine copy; .nlp regenerated)

1. **AI game-state bank 1** (stock 0x220000, ~5 KB spare) holds all AI pools. Raising actors overran into bank 2
   (crash at 0x134D5 / 0x13850: datum_new with a null bitmap). `tools/limits/make_v16_particles.py` now builds both
   profiles (`--profile v16|v16p`) with bank 1 +6 MB (`--ai-bank-extra`, default 0x600000; operand at file offset
   0x15555 = RVA 0x16155, `mov edx,0x220000` in the bank setup at RVA 0x16024). Checkpoint = render + main + 0x260000
   + extra; total = checkpoint + 2·main; checkpoint buffers in .hkwork move with it. `--ai-bank-extra 0` reproduces
   GreenKnight's exact hashes (V16 b8810c3b…, V16p 47bc0723…), which proves nothing else changed.
   Rebuild: `python tools/limits/make_v16_particles.py <stock haloreach.dll> --profile v16` (and `--profile v16p`).
2. **AI update scheduler RVA 0x70ABBC** lists every live actor in a 96 × 12-byte stack array (no bounds check), sorts
   it (qsort 0x6898CC, comparator 0x70ABB0) and sets actor+0x89 update level. Actor 97 hit the stack cookie →
   `__report_gsfailure` 0x788A78 → fail-fast 0xC0000409 (no Noble crash log; Windows Event Log only). Frame grown
   0x568 → 0x30E8 (1,024 entries) via five operands (`AI_FRAME_OPS` in the script: lea rbp, sub rsp, cookie
   store/load, epilogue lea). Hook `ai_update_stack` (PoolResize.cpp) commits 16 KB of stack before each call.

## Code (uncommitted unless Tyler ran the commit in this chat)

- `src/host/PoolResize.cpp`: data_array_new 0x12FCC hook. `-ai-actors N` (env NOBLE_AI_ACTORS, Extended only):
  actor = N, prop/prop_ref/prop_search × N/96, command scripts / object list header ≥ 256, dynamic firing points
  ≥ 64, swarm ≥ 128. Reads bank-1 size from the loaded engine; never grows past the added room (minus 64 KB).
  `-pool-trace` logs every pool (`POOLNEW|name|count|size|header|allocator|data|bitmap`); `-ai-pools a,b` resizes
  only those (bisecting). Extended builds hook `gs_failure_report`: logs `STACK OVERRUN in haloreach+0x…` with frames.
- `src/host/SpawnTest.cpp` (dev): F9 +500 objects, F8 128-body crowd, F7 ai_place every squad, **F6 horde**:
  +100 actors per press; 5×/s places the next squad (round robin), finds new actor slots, orders seated units out
  (unit_exit_vehicle 0x4F9A74, mode 0), then teleports each to a spiral 10–30 m in front of the player
  (object_set_position 0x46CB38(obj,pos,0,0,0,0), refresh 0x46C8B4, actor_teleported 0x717F50). Player body: players
  pool root+0x18, record 0x490, unit handle +0x28. Actor record 0xD00: salt +0, unit handle +0x1C, position +0x398.
  Object data: parent object +0x14 (≠ -1 → seated), position +0x44, forward +0x50.
- Script command table entries (name ptr at +0x10, handler at +0x28): ai_vehicle_exit → 0x6C0ED4(ai,-1),
  ai_teleport → 0x6C1530(ai, point), player_get → players[…]+0x28.

## Next steps (in order)

1. Build (`cmake --build build --config Release --target noble-dev`) and test F6 in Firefight with `-ai-actors 512`.
   Check log: `SpawnTest: horde wave done | X placed, Y moved next to you`. If moved = 0, check parent offset (+0x14)
   and the exit call; if units snap back into seats, call the exit with mode 1/2 or wait longer before teleporting.
2. Push to 1,024 (`-ai-actors 1024`); any new fixed table shows as `STACK OVERRUN in …` or a FATAL with RVA.
   Map RVA → function with objdump on the generated DLL (`--out-dll`); file offset = RVA − 0xC00 in .text.
3. Measure frame time at 256/512/1,024 (main game thread bound).
4. Then make actor count a real setting + include it in ProfileNetworkId (host/clients must match) — C7d.
5. Commit (Tyler runs git; message ends with the Co-Authored-By / Claude-Session lines from AGENT-STATUS).

## Other open items (unchanged)
C11 render list 4,096 → 16,384 (flicker/HUD past ~4,096 visible objects), C12 engine players pool = 16 (real
32/64/128-player matches = big project, scope with Tyler), particles profile test after D6, C8 light volumes,
C6 script threads/references/syntax nodes.

## Update 07:55 ET: 512 and 1,024 tests

- **512: works.** F6 horde filled 512/512 and held ~7 min, no crash, memory flat. F6 moved ~63% of placed actors to
  the player (rest died/timed out). Frame rate dropped to ~36 fps (1,800 frames took 50 s vs ~18 s normally).
- **1,024: crashed at ~650 live actors** (last pool report 647). New stack overrun:
  `STACK OVERRUN in haloreach+0x76EE57` = function **0x76EBF8**. Frame 0x58E0 via __chkstk (0x86C2D0); buffer at
  `rbp-0x40` up to the cookie at `rbp+0x57C0` (0x5800 = 22,528 bytes), passed by address to callees
  (`lea rdx,[rbp-0x40]`). Next: find the per-entry size and the callee that fills it (look for the loop/bound in
  the callees given rbp-0x40), then grow the frame the same way as AI_FRAME_OPS (lea rbp disp, eax frame size
  before __chkstk at 0x76EC10, cookie store 0x76EC2F / load 0x76EE48, epilogue `lea r11,[rsp+0x58E0]` at
  0x76EE57, xmm6 save at [rsp+0x58D0], and the args read at rbp+0x5840..0x5858 move with rbp → recompute
  those displacements too since they address the caller's stack). Nearby function 0x76E800 (frame 0x900)
  compares counts against 0x200 (512) at 0x76EA66 / 0x76EB80: check whether it is the same subsystem.
- Practical limit today: **~640 actors**. Ship 512 as the safe value until 0x76EBF8 is fixed.

## Update 08:00 ET: second buffer grown (built, NOT yet tested)
`AI_FRAME2_OPS` in make_v16_particles.py doubles function 0x76EBF8's buffer (frame 0x58E0 -> 0xB0E0, cookie
and stack-arg displacements +0x5800; __chkstk probes it, no hook needed). Both .nlp files regenerated
(`--ai-bank-extra 0` still reproduces exact V16). Test: `noble-dev.exe -ai-actors 1024`, F6 repeatedly; if it
dies again, the log names the next function the same way.

## 08:05 ET: handed to DeepSeek
Tyler is away for a few hours. Start file: `DEEPSEEK-START-AI.md` (new session, TODO list, rules). Last run with fix 2:
peaked at 338 actors, no crash; F6 is too slow to reach 650+ (TODO 1).

## Update 08:32 ET (DeepSeek): TODO 2, fix 2 verified statically, and it needed the caps too

Function 0x76EBF8 is real (called from 0x70061C and 0x7608F2), frame 0x58E0 through `__chkstk` 0x86C2D0, locals:

| What | Where | Size |
|---|---|---|
| candidate list | `rbp-0x40` .. `rbp+0x57C0` | 0x5800 = **512 entries × 0x2C** |
| stack cookie | `rbp+0x57C0` | the list ends exactly at the cookie |

Entry = 0x2C bytes: flags dword (+0), position double at +4, dword at +0xC, float at +0x10, words at
+0x26/+0x28/+0x2A (constructor 0x7049C0 zeroes it and writes those three), rest reserved. It is a list of
**spawn points** (position + flags), not one entry per actor.

Who fills it:
1. **Initializer** 0x76EC6A `mov edi, 0x200` / loop 0x76ECA1 calls the entry constructor 0x7049C0 512 times.
2. **List builder 0x76EAE0** (`lea rdx,[rbp-0x40]`, `lea r9,[rsp+0x70]`): copies candidate entries out of a
   per-actor structure (`rdx = [rdx+0x48]` source entries, stride 0x2C, filtered by a bitmask `[rcx+4]` using
   the byte at source entry +0x24) and stops at `cmp rdi, 0x200` (0x76EB80). Returns the count in [rsp+0x70].
3. **Volume search 0x76E814** (called per actor at 0x76ED31 with the buffer as arg 5 and `&count` as arg 7):
   collects objects around the actor into its **own** 512-dword scratch at `rbp-0x20` (frame 0x900, cookie
   `rbp+0x7E0`, its own 0x200 caps at 0x76E8EE / 0x76E924) and appends each usable one to the caller's buffer
   through 0x76E628 (`buffer + count*0x2C`), stopping at `cmp esi, 0x200` (0x76EA66).

So the count does **not** scale with the actor count; it is the number of candidate positions for one spawn
call, capped at 0x200 in all three places. With the caps left at 512 the doubled frame only added unused
stack — **fix 2 could not have helped**. All three caps now grow with the buffer in
`AI_FRAME2_CAP_OPS` (make_v16_particles.py), so 1,024 entries × 0x2C = 0xB000 exactly fill the grown frame.
`--ai-bank-extra 0` still reproduces V16 `b8810c3b…` exactly (verified 08:31).

Open risk, for the next crash log: the buffer ends *exactly* at the cookie, and two sibling layouts have the
same shape and were not grown:
- 0x76E814's scratch (`rbp-0x20`, 0x200 dwords, cookie `rbp+0x7E0`, frame 0x900) — its chain-expansion loop
  writes at `[rbp+rax*4-0x20]` with an index that is only bounded by the *initial* search count, so a long
  object chain can walk past 0x200. If `STACK OVERRUN … 0x76EAC0` appears, this is it: grow its frame by
  0x800 and move the 0x200 caps the same way.
- 0x76D714 (one function, .pdata 0x76D714–0x76E628, frame 0x5A80, cookie `rbp+0x58D0`, 512 × 0x2C array at
  `rbp+0xD0` ending exactly at the cookie, `mov esi,0x200` at 0x76D90C) — the same AI spawn code for another
  object class. A crash there names its own epilogue. (.pdata has no entry for 0x76E314; that address is
  mid-instruction, and starting a disassembly there only looks like a prologue.)

The 0x200 compares the handoff attributed to "0x76E800" are the 0x76EA66 (append) and 0x76EB80 (builder)
checks above; 0x76E800 is not a function start.

Tooling: `tools/limits/analyze_ai_frames.py` (new) disassembles the stock DLL with capstone and prints the
frame/cookie/cap map for any RVA, and runs the fixed-table audit below over the AI range.

## Update 08:45 ET (DeepSeek): fix 2 + caps verified in a real match; 1,024 actors work

Verification report (for Claude and Tyler): `AI-LIMITS-REPORT-2026-11-08.md`.

`noble-dev.exe` can boot straight into a match (`-map` / `-mode`), so the horde tests ran unattended:

- `-map ff_unearthed -mode firefight -ai-actors 1024 -horde 1200`:
  `horde capped at the 1024 free actor slots`, then `1024 placed, 629 moved … actors 763 / 1024`, held for
  60 s (763 -> 667 live), **no STACK OVERRUN / FATAL**. That is past the old ~647-live crash point, three runs
  in a row (peaks 688 / 767 / 763).
- Stock pool run (`-horde 200`, no `-ai-actors`): 96 free slots, 96 placed, `actors 96 / 96`.
- Forge World: `horde needs AI squads; this map has none` (new warning), queue cleared.
- Frame rate at 1,024 configured / ~760 live: **~18 fps** (512 was ~36 fps) — C7c data.

The engine patch was also checked by disassembly of the cached DLL: frame 0xB0E0, cookie `rbp+0xAFC0`,
epilogue `lea r11,[rsp+0xB0E0]`, and all three caps `0x400` (`0x76EC6A`, `0x76EB80`, `0x76EA66`);
`--ai-bank-extra 0` still gives V16 `b8810c3b…`.

Still theoretical: `0x76E814`'s chain-expansion write past its 512-dword scratch (grow it if `0x76EAC0`
appears), and keeping 1,024 actors **alive** (combat attrition held live ~760; a passive map would prove it).

## Update 09:16 ET (DeepSeek, batch 2): third buffer grown, campaign hold tested, render states blocked

- **0x76E814 grown** (`AI_FRAME3_OPS` + `AI_FRAME3_CAP_OPS` in `make_v16_particles.py`): frame 0x900 -> 0x1100,
  cookie rbp+0x7E0 -> rbp+0xFE0, stack args 0x860/0x870 -> 0x1060/0x1070, epilogue rsp+0x900 -> 0x1100, and the
  two 0x200 scratch caps -> 0x400. `PoolResize.cpp` adds the `ai_spawn_search_stack` hook (CommitStack before
  the prologue; the function has no `__chkstk`). `--ai-bank-extra 0` still reproduces V16 `b8810c3b…` / V16p
  `47bc0723…`; new v16 `2a815ac4…`, v16p `7d807c95…`. Firefight 900 horde on the new build: 685 live, stable.
- **Mission 3 (m30) is not a hold map**: 1,000 placed, peak 482 live, mission scripting removed them to 11 in
  90 s; one run crashed after external F6 input (VCRUNTIME140, dump saved), another lost its process silently.
  Use Firefight Unearthed for 5-minute holds.
- **Cached object render states cannot be raised at runtime**: 16,384 crashes at haloreach+0x27B01F, 8,192 at
  haloreach+0x6E58E, both at map load. Rule is in PoolResize behind `-render-states N` (off by default). The
  V16 append-block/bank rebuild is the prerequisite.
- **script node constructor found** (0x13148 via 0x81854, initial bytes 0xFC00, records 0x18) — see
  `docs/limits-ai.md` C6 addendum.
