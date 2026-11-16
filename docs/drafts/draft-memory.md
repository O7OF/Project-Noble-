# Halo engine memory model — tag table, game-state banks, bump allocator, pool budgets

Draft by cc, 2026-11-14. Covers how the engine (MCC `haloreach.dll` 1.3528.0.0, SHA-256 `738dd2d2…`) holds
live tags and game-state memory, and the budget rules Noble's engine-limits work must obey.

**Confidence legend** (project convention): **proven** = one-variable test + capture/log; **observed** = seen in a
live run/log/pool report but not a controlled single-variable test; **community** = community guide; **unverified** =
inferred from code/disassembly/docs, not runtime-verified.

**Sources** (paths relative to `knowledge\` unless stated):
- `repo-docs\root\AI-LIMITS-REPORT-2026-11-08.md` — "C8 follow-up" BANK diagnostic (bank table layout + live values)
- `repo-docs\root\HANDOFF.md` — §5 Engine facts (tag table)
- `src\host\PoolResize.cpp` (repo root) — bank RVAs, budget rules, beam guard
- `00-START-HERE.md`, `AGENT-BRIEF.md`, `02-LESSONS-AND-RULES.md` — bank summary + current state
- `repo-docs\limits-ai.md`, `repo-docs\limits-render.md` — AI/render bank accounting
- `desktop-guides\Noble-AI-Limits-Report-2026-11-08.md` — Full-profile render/main bank sizes

## 1. Live tag table

| Fact | Value | Source | Confidence |
|---|---|---|---|
| Pointer to the live tag table | `*(haloreach + 0xC1A600)` (8-byte entry layout) | HANDOFF §5 (`UpdateTagShaderConstantsSEH`); TrooperMode reads it | observed (Noble dereferences it live) |
| Tag-page pointer bases (16 entries) | `haloreach + 0x4E39F20` | HANDOFF §5 | unverified (code) |

The tag table is the engine's runtime index of loaded tags; Noble reads it to repoint game globals (Trooper mode)
and to walk tag memory. The pointer itself is dereferenced at runtime; its exact entry layout is code-derived.

## 2. Game-state banks

Game-state memory is carved from **8 banks**, described by a static table. The table is the same record
`PoolReport` logs as `BANK|n|used|capacity`.

| Fact | Value | Source | Confidence |
|---|---|---|---|
| Bank table | static array at RVA `0x27BF270` | AI-LIMITS-REPORT C8 follow-up; `PoolResize.cpp:102` | unverified (code), values observed |
| Records | 8, one per bank | AI-LIMITS-REPORT C8 follow-up | observed |
| Record stride | `0x18F0` | AI-LIMITS-REPORT C8 follow-up; `PoolResize.cpp:103` | unverified (code) |
| +0xA0 | capacity (u64) | AI-LIMITS-REPORT C8 follow-up; `PoolResize.cpp` `BankRoomFor` | unverified (code), values observed |
| +0xA8 | bytes handed out by the bump allocator (u64) | same | unverified (code), values observed |
| Allocator | bump allocator with **no bounds check** | `AGENT-BRIEF.md`; `PoolResize.cpp` header comment | unverified (code); consequence observed (beam overflow) |

Live BANK diagnostic (Full profile, Forge World — pre-fix, from AI-LIMITS-REPORT C8 follow-up):

| Bank | Used | Capacity | Capacity (hex) | Known role |
|---|---|---|---|---|
| 0 | 125,992 | 131,072 | 0x20000 | unknown (small) |
| 1 | 2,222,752 | 8,519,680 | 0x820000 | **AI bank** (stock 0x220000) |
| 2 | 50,740 | 65,536 | 0x10000 | unknown (small) |
| 3 | 58,613,580 | 67,043,328 | 0x3FF0000 | **render bank** (stock 0x560000) |
| 4 | 51,649,416 | 41,746,432 | 0x27D0000 | **main bank** (over by 9.9 MB here; fixed to 0x3250000) |
| 5 | 5,124 | 65,536 | 0x10000 | unknown (small) |
| 6 | 338,488 | 1,572,864 | 0x180000 | unknown |
| 7 | 0 | 589,824 | 0x90000 | unknown |

Bank 1 (AI) and bank 3 (render) are the two Noble budget rules care about; bank 4 (main) is the third large bank
(raised from 0x27D0000 to 0x3250000 in the Full profile). Roles of 0/2/5/6/7 are not established.

## 3. The bump allocator and `data_array_new`

- Named pools (the `POOL|name|…|live` report) are created through **`data_array_new`** at RVA `0x12FCC`
  (signature `0x64407440`). Source: `repo-docs\limits-render.md`; `PoolResize.cpp:20`.
- Memory cost of a pool = `count × recordSize + ⌈count/32⌉ × 4` (one bitmap bit per 32 records). Source:
  `PoolResize.cpp:92` (`ArrayBytes`).
- The allocator hands out bytes from a bank and only advances the +0xA8 cursor; it has **no bounds check**, so a
  pool that does not fit silently corrupts the bank instead of failing. Source: `PoolResize.cpp` header comment;
  `AGENT-BRIEF.md`. Consequence was observed in the 2026-11-14 beam overflow (§5).

## 4. Bank budget rules

| Rule | Bank | Formula | Source | Confidence |
|---|---|---|---|---|
| AI pool budget | bank 1 | `budget = bankSize − 0x220000 (stock) − 0x10000 (safety)`; a resize that doesn't fit is trimmed to what fits | `PoolResize.cpp:28-32, 437-447` | unverified (code); "AI memory +6080 KB" observed |
| Render-state budget | bank 3 | `s_renderBudget = renderBank − 0x560000 (stock) − 0x10000` | `PoolResize.cpp:450-458` | unverified (code) |
| Later-pool reserve | bank 3 | `kLaterPoolsReserve = 9 MB` + `0x10000` safety kept back from any beam expansion | `PoolResize.cpp:105, 115` | unverified (code); fix proven |

`data_array_new` is hooked (`Hook_DataArrayNew`); every pool resize is charged against its bank's remaining room
and falls back to the stock count (or a trimmed count) when it does not fit. Source: `PoolResize.cpp`.

## 5. Worked example — 2026-11-14 beam-pool overflow (bank 3)

- **What happened.** The merged build (origin/main `0.19.69`) expanded three runtime beam pools —
  `beam_system` (2,809 KB), `beam_location` (3,064 KB), `beam` (3,319 KB) — ~9.2 MB total in bank 3. Bank 3 went
  from **58,613,580 → 68,052,044 bytes of 67,043,328**. With no bounds check, every session that overflowed ended
  inside its **first frame**. Source: `00-START-HERE.md` "Current state"; `AGENT-COORDINATION.md` 10:52 entry.
- **Guard.** `src\host\PoolResize.cpp` — `BankRoomFor(extra)` reads bank 3's `+0xA0`/`+0xA8` and only expands a
  beam pool when `extra + 9 MB reserve + 64 KB safety ≤ capacity − used`; otherwise it keeps the stock pool.
  Source: `PoolResize.cpp:107-119` (the beam branch at :269-284).
- **Proof (proven).** After the guard: `BANK|3` back to **58.6 MB**; Sword Base ran **70 s** with captures at
  40/60 s and no GAME OVER. Source: `00-START-HERE.md`.

## 6. Proven vs inferred — summary

- **Proven by test:** the beam-overflow fix (bank 3 restored to ~58.6 MB; 70 s Sword Base run clean).
- **Observed (runtime):** the BANK diagnostic values, the pool-report sizes, the "AI memory +6080 KB" line.
- **Inferred from code/docs (unverified):** the bank-table offsets (`0x27BF270`, stride `0x18F0`, `+0xA0`/`+0xA8`),
  the "bump allocator, no bounds check" claim, the bank 1 / bank 3 role assignments and their stock sizes
  (`0x220000` / `0x560000`), and the 9 MB reserve value.

## 7. Open questions

- What do banks 0, 2, 5, 6, 7 hold? Only capacities are known; roles unverified.
- Is the "no bounds check" a property of the allocator for *all* pools, or only the bank carve-out path?
- Exact stock render-bank size (0x560000 is the offline-patch threshold in `PoolResize`, not necessarily the
  stock value — it may already include a V16-era growth).
