# DeepSeek: new session start (AI limits), 2026-11-08 08:05 ET

**Tyler is away for a few hours.** Work unattended: no questions to Tyler, no in-game tests that need him.
Do what can be done from code and disassembly, write results into the files below, and leave a short
"ready for Tyler to test" list at the end of `AGENT-STATUS.md` (exact PowerShell commands, what to press,
what log line means pass/fail).

## Read first (in this order)
1. `AI-LIMITS-HANDOFF.md`: all AI findings, addresses, fixes, test history (256 ok, 512 ok, ~650 crash, fix 2 built).
2. `LIMITS-TODO.md`: Phase 3b table. 3. `LIMITS-HANDOFF.md`: V16 / .nlp / profile system. 4. `AGENT-STATUS.md`: claims + log.

## Hard rules
- Never modify the player's installed haloreach.dll. All engine changes go through `tools/limits/make_v16_particles.py`
  → `data/limits/v16.nlp` / `v16p.nlp` (applied to a copy at runtime). Always check `--ai-bank-extra 0` still
  reproduces V16 sha b8810c3b… / V16p 47bc0723….
- Never run build.bat (publishes a release). Never push. Commits only if Tyler said so; he hasn't for this batch.
- Never edit `D:\Project-Noble-ORIGINAL`. `tools/limits/recipes/` and `*.nlp` stay out of git.
- Claim tasks in `AGENT-STATUS.md` before editing; log every change there with time (ET).
- Build check: `cmake --build build --config Release --target noble-dev` (Developer PowerShell for VS 2022).

## TODO (in order)
1. **F6 horde is too slow** (Tyler: "the queue takes forever"). In `src/host/SpawnTest.cpp` `HordeStep()` runs every
   200 ms and places one squad per step. Make it place several squads per step (e.g. loop squads until ≥ 25 new
   actors or all squads tried), run every 100 ms, and cap pending at the free actor slots. Keep the teleport loop.
   Add `-horde N` (env NOBLE_HORDE) to auto-queue N actors 10 s after the match starts, so tests need no key spam.
2. **Verify fix 2 (function 0x76EBF8) statically**: find what its 0x5800-byte buffer holds (callees given
   `rbp-0x40`), per-entry size and whether its count scales with actors. If bounded by a constant (look at
   0x76E800's `cmp ..., 0x200` checks at 0x76EA66 / 0x76EB80), raise those too. Document in the handoff.
3. **Audit for the next fixed tables** before Tyler tests 1,024: search the AI code range (~0x690000–0x780000) for
   stack frames ≥ 0x400 bytes whose buffers are filled in actor loops (iterator 0x6AE194 / 0x6BE220), and for
   `cmp reg, 0x60` / `0x80` / `0x100` / `0x200` near actor-array use (`imul ..., 0xd00`). List candidates with RVA,
   buffer size, entry size, likely capacity in `docs/limits-ai.md`. Fix the clear ones the same way as
   `AI_FRAME_OPS` / `AI_FRAME2_OPS` (grow frame; patch every rbp/rsp displacement above the buffer; hook to
   commit stack only if the function has no __chkstk call).
4. **C7d**: turn `-ai-actors` into a Settings value (Extended only; 96/256/512 options; 1,024 marked experimental) and
   include it in `ProfileNetworkId` so host and clients must match. Coordinate with D1/D2 code you already own.
5. Update `LIMITS-TODO.md` + `AGENT-STATUS.md`, then write the "ready for Tyler" test list (1: `-ai-actors 1024 -horde 900`,
   expect no `STACK OVERRUN`/`FATAL`, pool report shows actor > 700; 2: frame rate notes).

## Useful facts
- Log: `build\bin\Release\noble-client.log`. Crash dump: `build\bin\Release\data\sandbox\noble-crash.dmp`.
  Fail-fast crashes (0xC0000409) only appear in the Windows Event Log; Extended builds now log
  `STACK OVERRUN in haloreach+0x…` first.
- File offset = RVA − 0xC00 in .text. Disassemble the generated DLL (`make_v16_particles.py … --out-dll x.dll`)
  with objdump `--start-address=0x180000000+RVA`.
- Current results: 256 stable (4 min), 512 stable (7 min, ~36 fps), 1,024 setting crashed at ~650 (fixed, untested;
  last run peaked at 338 without a crash).

## Part 2 (Tyler 08:07 ET): after TODO 1-5, or instead of TODO 3-4 if those stall: the next limits
Actors are "good enough" for now (512 stable). Work these in order; Claude takes over from your notes at ~10:05 ET,
so **log progress in AGENT-STATUS.md as you go** (what's done, what's half done, exact file/RVA you stopped at).

6. **C11 render list 4,096 → 16,384** (flicker + weapon HUD vanishing past ~4,096 visible objects). Find the
   per-frame visible-object/render list: look for pools/arrays sized 4,096 in the pool report ("cached object render
   states" 4,096 × 624 is a candidate) and code with `cmp ..., 0x1000` / `0xFFF` near object render submission.
   Raise via the PoolResize hook (if it's a data array) or an .nlp op (if it's a static array/constant), keeping
   memory bank sizes in mind (see bank table: bank sizes set at RVA 0x16024 via 0x15FD8). Repro for Tyler: F9 ×10
   in Forge (5,000 objects) and look at one spot.
7. **D6 + particles test**: finish profile 2 (V16+particles) in the ENGINE LIMITS setting UI, then a ready-for-Tyler
   test (stock vs V16p particle-heavy scene; pool report should show particles 65,535 class capacity).
8. **C8 light volumes**: recipe in `tools/limits/recipes/light_volumes/`. It conflicts with V16 / particles (same header
   slot 0x388, RVA 0x4EDA000, state-bank operands; LV relocates the particle init; particle operands sit inside the
   LV copy at 0xC9E10B / 0xC9E15A / 0xC9E189). Port it onto the V16p layout like make_v16_particles.py did for
   particles (new block at the end of .deltrac after the particle block, recompute every fixup), as profile 3 "Full".
9. **C6 script limits**: find script threads (det hs thread 320 × 1,324 is in the pool trace), per-map references (128),
   syntax nodes. Same method: name string → data_array_new caller → raise via PoolResize rules (Extended only).
10. **C12 research only** (no big code): engine `players` pool = 16 (1,168-byte records). List everything that would
   need to change for 32/64 real players (pool, network membership/session limits, HUD/scoreboard, spawn) into
   `docs/limits-players.md` with RVAs and an effort estimate. Tyler decides scope later.
