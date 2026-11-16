# C8 light volumes onto V16 + particles: port plan (Claude, 2026-11-08 10:15 ET)

Source: `tools/limits/recipes/light_volumes/` (GreenKnight V4, stock → V4, 95 overwrite rows, 351,784-byte append).
Goal: profile 3 "Full" = V16 + particles + light volumes, built by `make_v16_particles.py`, applied as .nlp.

## What V4 does (from the guide)
- Pools 160 → 65,535 (light_volume_system/location, light_volume, c_light_volume_gpu, ::s_row), GPU row queue
  4,096 → 65,535, parameter queue 256 → 8,192, parent effect-location pool 1,380 → 65,534 (2 × 32,767),
  effect-message queue 512 → 65,536 (bitmap 16 → 2,048 words, header 0x4C → 0x200C), transparent draw queue
  1,024 → 16,384.
- Seven routines are **relocated and rewritten** (effect/particle init 0x115888 [1,302 B], message queue
  admission/reset/relocation/consumer/append/filtering at 0x121AC4..0x1228F4 [~1.2 KB]), plus five small
  redirect blocks (~80 B) and a sort guard.
- Relocated storage: snapshots, transparent records (0x2C0000) and indices (0x8000) in zero-filled virtual space.
- Banks: render 0x370000 → 0x2DE0000, main 0x490000 → 0xEF0000 (stock numbers; V16 has its own main 0x27D0000).
- The bulk of the 351 KB append is a **full copy of the exception table** with entries for the relocated routines.

## Layout problem (measured on our v16p build)
| Section | RVA | VirtualSize | Raw | Notes |
|---|---|---|---|---|
| .objpool | 0x4EDA000 | 0x5B76080 | 0x55A00 | V16 code + object storage (RWX) |
| .deltrac | 0xAA51000 | 0x97400 | 0x97400 | V16 code, **V16's exception table copy at 0xAA93000 (0x551B8)**, particle block at 0xAAE8200 |
| .hkwork | 0xAAE9000 | 0x131F9000 | 0 | virtual only (checkpoint buffers, queue) |
- All 10 section-header slots are used (no new section like V4's `.lvlimit`).
- Free room: 0x5C bytes at the end of .deltrac raw + 0xC00 virtual gap before .hkwork ≈ **3.1 KB**.
- V4's relocated code is ~2.6 KB + blocks + alignment ≈ 2.8-3.0 KB: **fits only just**, and V4 also needs new
  exception entries (the relocated routines have prologues; unwinding through them without entries = crash).

## Recommended approach
1. **Do not copy V4's exception table.** Put the relocated routines in the .deltrac gap (grow .deltrac VirtualSize /
   SizeOfRawData by up to 0xC00; .hkwork start stays 0xAAE9000). Reuse each original routine's UNWIND_INFO RVA
   (same prologue) and register RUNTIME_FUNCTIONs for the new copies from Noble at load
   (`RtlAddFunctionTable` on a small table in Noble's memory, base = engine base), in `EngineLimits`/Loader only
   for profile 3. If the code does not fit in 0xC00, move the particle block + LV code together and grow
   .deltrac by moving .hkwork's start up after checking nothing references [0xAAE9000, new start) (scan
   rip-relative targets in .text/.objpool/.deltrac).
2. **Extract V4's added code** from `V4_append_at_00C9DBD8.bin` at the file offsets in guide §5/§6 (code starts at
   0xC9DC00 = RVA 0x4EDA000 in V4). Every rel32 inside it targets stock RVAs or V4-relative RVAs: recompute for the
   new location (same method as BLOCK_FIXUPS for particles). Note V4 RVA 0x4EDA000 collides with V16's .objpool,
   so every V4-relative target must be remapped.
3. **Overwrite rows (95):** apply the ones that hit stock code not touched by V16/particles; for rows that overlap,
   merge by hand. Known overlaps: PE header rows (skip, recompute), header slot 0x388 (skip), state-bank operands
   (replace with formulas below), the three particle operands inside V4's relocated effect-init copy at
   0xC9E10B / 0xC9E15A / 0xC9E189 (apply the particle values there), snapshot addresses (use V16's BUF1/BUF2/QUEUE).
4. **Bank formulas** (all in `Layout`): render = V16 render + particle extra + LV render extra (0x2DE0000 − 0x370000)
   + render extra; main = max(V16 main, LV main delta) → V16 main 0x27D0000 + (0xEF0000 − 0x490000);
   checkpoint/total follow. Expected combined (from DeepSeek's notes, before our AI/render extras): render
   0x37F0000, main 0x3230000, checkpoint 0x6C80000, total 0xD0E0000.
5. **Transparent storage** (records 0x2C0000, indices 0x8000) → place at the end of .hkwork (grow it), like QUEUE.
6. **Verify:** `--ai-bank-extra 0 --render-extra 0` for profiles 1/2 still reproduce V16/V16p; profile 3 loads in
   Noble; pool report shows light_volume* 65,535; plasma-heavy scene (Firefight Covenant waves) has no missing
   light volumes. Add `Profile::Full` (EngineLimits, settings label, network id, save-folder suffix `_ext_full`).

Effort: ~1-2 focused days. Highest-risk part: the relocated routines' rel32 fixups and unwind registration.
