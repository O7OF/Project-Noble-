# AI limits: fixed-table audit (2026-11-08, DeepSeek)

Companion to `AI-LIMITS-HANDOFF.md` and `LIMITS-TODO.md` (Phase 3b, TODO 3). Everything here is read off
the player's **stock** `haloreach.dll` 1.3528.0.0 (SHA-256 `738dd2d2…`); the file is never modified.
RVA = file offset + 0xC00 in `.text`.

## Method

`tools/limits/analyze_ai_frames.py` (new) uses the PE's `.pdata` table for exact function bounds and
capstone for decoding — no objdump needed:

```powershell
python tools\limits\analyze_ai_frames.py --function 0x76EBF8        # one function: full disassembly + summary
python tools\limits\analyze_ai_frames.py --range 0x690000 0x780000   # audit table (64 candidates)
```

For every function it reports the frame size (`__chkstk` immediate or `sub rsp, imm`), the stack cookie
slot, the lowest `rbp`-negative displacement (brackets the biggest local with the cookie), the 0x2C entry
stride, the 0xD00 actor-record iterator, and every small constant compare/move
(`0x60 / 0x80 / 0xC0 / 0x100 / 0x200 / 0x400`).

## What is already fixed

| RVA | Frame (stock → now) | Buffer | Entry | Capacity | Fix |
|---|---|---|---|---|---|
| 0x70ABBC | 0x540 → 0x30E8 | 96 × 12 at `rbp-0x70` … cookie | 12 B | 96 → 1,024 | `AI_FRAME_OPS` + `ai_update_stack` commit hook |
| 0x76EBF8 | 0x58E0 → 0xB0E0 | `rbp-0x40` … cookie | 0x2C | 512 → 1,024 | `AI_FRAME2_OPS` + `AI_FRAME2_CAP_OPS` (three 0x200 caps → 0x400) |

The 0x76EBF8 buffer holds spawn-point candidates (flags dword, position, tag), filled by the list builder
0x76EAE0 and by the volume search 0x76E814 → 0x76E628. Its count does **not** scale with actors; it is
capped at 0x200 in three places (0x76EA66, 0x76EB80, 0x76EC6A), all raised together with the frame. Details
in `AI-LIMITS-HANDOFF.md` ("Update 08:32 ET").

## Next candidates, highest risk first

These are the functions in the AI range whose stack frame is ≥ 0x400 bytes and that touch the actor record
(`imul …, 0xD00`) or walk 0x2C-byte entries. "Span" = cookie slot + lowest `rbp`-negative displacement,
i.e. an upper bound on the biggest local array; it over-counts register spills, so read the disassembly.

| RVA | Frame | Cookie | Span | Flags | Notes |
|---|---|---|---|---|---|
| `0x76E814` | 0x900 | rbp+0x7E0 | 0x860 | actor 0xD00, **cap 0x200 @0x76EA66** | **Clear risk.** 512-dword object scratch at `rbp-0x20` (0x800 B) and the chain-expansion loop writes `[rbp+idx*4-0x20]` with an index only bounded by the *initial* search result, then appends to the caller's list. No `__chkstk` → growing it needs a stack-commit hook. If `STACK OVERRUN … 0x76EAC0` shows up, this is the one. |
| `0x76D714` | 0x5A80 | rbp+0x58D0 | 0x5950 | actor 0xD00, entry 0x2C, `__chkstk` | Same shape as 0x76EBF8 (512 × 0x2C at `rbp+0xD0`, ends at the cookie). Grow exactly like `AI_FRAME2_OPS` + `AI_FRAME2_CAP_OPS` if a crash names its epilogue. |
| `0x76A358` | 0x5850 | - | - | actor 0xD00, entry 0x2C, `__chkstk` | 0x2C-entry list, frame just under 0x5A80; check its own 0x200 cap before growing. |
| `0x69AEE0` | 0x2E90 | rbp+0x2D80 | 0x2E00 | `__chkstk` | 11.6 KB frame with a table; no actor iterator found — likely squad/swarm data. |
| `0x6BA2E4` | 0x11060 | - | - | actor 0xD00, `__chkstk` | 68 KB frame; only 65 instructions, so the space is a big local array. Highest frame in the range — inspect first if a large-actor crash points near 0x6BA2E4. |
| `0x6D28C4` | 0x12DA0 | rbp+0x12C90 | 0x12D08 | actor 0xD00, `__chkstk` | 77 KB frame, same shape as above. |
| `0x7216CC` | 0x11260 | rbp+0x110B0 | 0x11130 | actor 0xD00, `__chkstk` | 70 KB frame. |
| `0x6B13C4`, `0x6B4F20`, `0x6C0218`, `0x6F515C`, `0x6F7A48`, `0x70267C`, `0x705570`, `0x768F7C`, `0x770814`, `0x773BE0` | 0x630 – 0x4300 | various | various | actor 0xD00 | Medium frames with actor access; none has a small constant cap, so they scale with the pools rather than a fixed table. |

Not candidates despite the old handoff note: **`0x76E800` is not a function start**. The `cmp …, 0x200`
checks at 0x76EA66 and 0x76EB80 belong to 0x76E814 and 0x76EAE0 (the 0x76EBF8 spawn list). The three
`cmp …, 0x60` sites in the range (0x69489B, 0x696D77, 0x6AF78E) are **not** actor limits: 0x69489B and
0x696D77 compare a script/AI parameter with 0x60, and 0x6AF78E compares a tag-class table entry (stride 0x20)
with 0x60.

## How to fix the next one (same recipe)

1. Crash log: `STACK OVERRUN in haloreach+0x…` → take that RVA.
2. `python tools\limits\analyze_ai_frames.py --function 0x…` gives the frame, cookie, caps and entry stride.
3. In `tools/limits/make_v16_particles.py` add an ops list like `AI_FRAME_OPS`:
   `lea rbp, [rsp-X]`, the `__chkstk` immediate, the cookie store/load, the epilogue `lea r11`,
   every stack-argument read at `rbp+…` that addresses the caller's stack, **and** every capacity compare /
   `mov reg, imm` that bounds the array.
4. If the function has no `__chkstk`, the grown frame jumps more than one page: add a MinHook on the
   function that calls `CommitStack()` (see `src/host/PoolResize.cpp`, `ai_update_stack`).
5. Regenerate both profiles and check `--ai-bank-extra 0` still gives V16 `b8810c3b…`.

## Appendix: full audit table (64 candidates, AI range 0x690000–0x780000)

Sorted by frame size. Regenerate with `--range 0x690000 0x780000`.

| RVA | Frame | Cookie | Span | Flags |
|---|---|---|---|---|
| 0x6D28C4 | 0x12DA0 | rbp+0x12C90 | 0x12D08 | actor 0xD00, __chkstk |
| 0x7216CC | 0x11260 | rbp+0x110B0 | 0x11130 | actor 0xD00, __chkstk |
| 0x6BA2E4 | 0x11060 | - | - | actor 0xD00, __chkstk |
| 0x76D714 | 0x5A80 | rbp+0x58D0 | 0x5950 | actor 0xD00, entry 0x2C, __chkstk |
| 0x76EBF8 | 0x58E0 | rbp+0x57C0 | 0x5840 | actor 0xD00, entry 0x2C, __chkstk |
| 0x76A358 | 0x5850 | - | - | actor 0xD00, entry 0x2C, __chkstk |
| 0x6BA090 | 0x4890 | rbp+0x4770 | - | actor 0xD00, __chkstk |
| 0x768F7C | 0x4300 | rbp+0x4170 | 0x41F0 | actor 0xD00, __chkstk |
| 0x6C0218 | 0x3370 | rbp+0x3250 | 0x32D0 | actor 0xD00, __chkstk |
| 0x69AEE0 | 0x2E90 | rbp+0x2D80 | 0x2E00 | __chkstk |
| 0x6F7A48 | 0x29C0 | rbp+0x28A0 | 0x2920 | actor 0xD00, __chkstk |
| 0x69FAD0 | 0x29B0 | rbp+0x28A0 | 0x28F0 | __chkstk |
| 0x6DBD04 | 0x21C0 | rbp+0x2090 | - | actor 0xD00, __chkstk |
| 0x764988 | 0x1FA0 | rbp+0x1E50 | 0x1ED0 | actor 0xD00, __chkstk |
| 0x6E1EE0 | 0x1EF0 | rbp+0x1DC0 | 0x1E38 | actor 0xD00, __chkstk |
| 0x6DC1D8 | 0x1EE0 | rbp+0x1DC0 | 0x1E38 | actor 0xD00, __chkstk |
| 0x6CD378 | 0x1ED0 | rbp+0x1DC0 | - | actor 0xD00, __chkstk |
| 0x6E5FC0 | 0x1E90 | rbp+0x1D70 | 0x1DEC | actor 0xD00, __chkstk |
| 0x6D1944 | 0x1E80 | rbp+0x1D60 | 0x1DE0 | actor 0xD00, __chkstk |
| 0x6CC140 | 0x1E30 | rbp+0x1D20 | 0x1D30 | actor 0xD00, __chkstk |
| 0x6CF9F8 | 0x1E30 | rbp+0x1D20 | 0x1D60 | actor 0xD00, __chkstk |
| 0x6D7720 | 0x1E30 | rbp+0x1D20 | 0x1D30 | actor 0xD00, __chkstk |
| 0x6E4368 | 0x1E30 | rbp+0x1D20 | 0x1D30 | actor 0xD00, __chkstk |
| 0x6CF190 | 0x1E20 | rbp+0x1D10 | 0x1D30 | actor 0xD00, __chkstk |
| 0x6D0F48 | 0x1E20 | rbp+0x1D10 | 0x1D30 | actor 0xD00, __chkstk |
| 0x6D9230 | 0x1E20 | rbp+0x1D10 | 0x1D30 | actor 0xD00, __chkstk |
| 0x6E05E4 | 0x1E20 | rbp+0x1D10 | 0x1D30 | actor 0xD00, __chkstk |
| 0x6E0BC8 | 0x1E20 | rbp+0x1D10 | 0x1D30 | actor 0xD00, __chkstk |
| 0x6DC770 | 0x1DF0 | rbp+0x1CA0 | 0x1D1B | actor 0xD00, __chkstk |
| 0x6DF258 | 0x1D90 | rbp+0x1C80 | 0x1CD8 | actor 0xD00, __chkstk |
| 0x6B13C4 | 0x1D20 | rbp+0x1C10 | 0x1C90 | actor 0xD00, __chkstk |
| 0x770814 | 0x1868 | rbp+0x16E0 | 0x1760 | actor 0xD00, __chkstk |
| 0x773BE0 | 0x1758 | rbp+0x15A0 | 0x1620 | actor 0xD00, __chkstk |
| 0x757630 | 0x1680 | rbp+0x14D0 | 0x1550 | actor 0xD00, __chkstk |
| 0x6DAB00 | 0x1578 | rbp+0x1410 | 0x1490 | actor 0xD00, __chkstk |
| 0x6B4F20 | 0x1500 | rbp+0x13D0 | 0x1450 | actor 0xD00, __chkstk |
| 0x70267C | 0x14B0 | rbp+0x1300 | 0x1378 | entry 0x2C, __chkstk |
| 0x6F515C | 0x11D0 | rbp+0x1020 | 0x10A0 | actor 0xD00, __chkstk |
| 0x7612F8 | 0xF88 | rbp+0xE10 | 0xE90 | actor 0xD00 |
| 0x733EA4 | 0xEB0 | rbp+0xD60 | 0xDC0 | actor 0xD00 |
| 0x734518 | 0xE60 | - | - | actor 0xD00 |
| 0x734258 | 0xE50 | - | - | actor 0xD00 |
| 0x6BA3E0 | 0x918 | rbp+0x7E0 | 0x860 | actor 0xD00 |
| 0x76E814 | 0x900 | rbp+0x7E0 | 0x860 | actor 0xD00 |
| 0x705570 | 0x8F0 | rbp+0x7D0 | 0x840 | actor 0xD00 |
| 0x760448 | 0x7D8 | rbp+0x6B0 | 0x730 | actor 0xD00 |
| 0x758A30 | 0x790 | rbp+0x610 | 0x690 | actor 0xD00, entry 0x2C |
| 0x7258CC | 0x710 | rbp+0x5A0 | 0x620 | actor 0xD00 |
| 0x7002D4 | 0x6C0 | rbp+0x5B0 | 0x630 | actor 0xD00, entry 0x2C |
| 0x7019C4 | 0x630 | rbp+0x4D0 | 0x550 | actor 0xD00 |
| 0x741C24 | 0x590 | - | - | actor 0xD00 |
| 0x6DF494 | 0x578 | rbp+0x450 | 0x4C4 | actor 0xD00 |
| 0x77880C | 0x558 | - | - | - |
| 0x70ABBC | 0x540 | rbp+0x410 | 0x484 | actor 0xD00 |
| 0x71897C | 0x500 | - | - | actor 0xD00 |
| 0x69D8AC | 0x4E0 | - | - | actor 0xD00 |
| 0x71BBA4 | 0x4B0 | - | - | actor 0xD00 |
| 0x71CA60 | 0x480 | - | - | actor 0xD00 |
| 0x6DB9A0 | 0x460 | - | - | actor 0xD00 |
| 0x6DD76C | 0x460 | - | - | actor 0xD00 |
| 0x71BF88 | 0x460 | - | - | actor 0xD00 |
| 0x71CB5C | 0x450 | - | - | actor 0xD00 |
| 0x71E2C8 | 0x450 | - | - | actor 0xD00 |
| 0x75DC4C | 0x440 | - | - | actor 0xD00 |

## C6 addendum (2026-11-08 09:10, DeepSeek): where the script-node pool is created

`script node` is **not** a `data_array_new` (0x12FCC) pool. Its constructor is a different growable-array
helper, `0x13148`, called from **0x81854**:

```text
0x817EC  mov rax, [rip+0xB0626E]     ; allocator function pointer
0x81819  mov edx, 0x1FF0             ; bytes for the header block
0x81821  call qword ptr [rax]        ; allocate it
0x8182B  lea rcx, [rax + 0x70]       ; argument (rsp+0x30)
0x8183A  lea rdx, [rip+0x91471F]     ; "script node"   (.rdata 0x995F60)
0x81846  mov r8d, 0xFC00             ; initial array bytes (64,512)
0x8184C  and dword [rsp+0x20], 0
0x81854  call 0x13148                ; (rcx, name, bytes, record size 0x18, ...)
```

Record size is `r9d = 0x18` (24 B, matches the pool report). 0xFC00 / 24 = **2,688 initial nodes**, but the live
pool report shows **16,280**: this array **grows** as scripts are compiled, so 16,280 is not a fixed cap — that is
why no constant compare was ever found. To raise it, either patch the initial size immediate at **0x81846**
(`41 B8 00 FC 00 00`; file offset 0x80C46) in the engine build, or hook 0x13148 and scale `r8` for
`name == "script node"`. Both change the initial game-state allocation, so they need the same bank-budget care as
the AI pools. Not patched in this batch — the PoolResize rule for `script node` stays inert.
