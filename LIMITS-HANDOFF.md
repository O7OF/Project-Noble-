# Extended Engine Limits: handoff

Written 2026-11-08 by Claude (claude.ai session, for Tyler). Read this before any task in
`LIMITS-TODO.md`. Claim tasks in `AGENT-STATUS.md` as usual.

## Status update 2026-11-09 01:00 ET (read this first, then the 2026-11-08 section below)

Written by the cloud Claude session for the next agent. Commits below are local, **not pushed**. Nothing in C12-L
step 3/4 has been run yet.

### How work is split now
- **Local Claude Code** (terminal in `D:\Project-Noble-Closed-Source`, effort high) owns code edits, tools and
  `build-dev.bat` builds. No Python on this PC: the `tools/limits/*.py` generators are the source of truth; a
  scratch Node port reproduces their `.inc` output byte for byte. Disassembly: `%TEMP%\stock_text.asm` (capstone
  listing of the stock DLL, not in git).
- **Cloud session** reviews diffs, reads logs, and drives tests on **monitor 1** (monitor 2 is Tyler's).
  It cannot run commands on the PC and only clicks in terminals.
- **Do not write a static "sort all imul sites" disassembly scanner.** Two Claude Code attempts and one cloud reply
  were stopped by Anthropic safeguards at that step. Find untraced sites at runtime instead (trace + guard below).

### C12-L (32 players) progress, 2026-11-09
| Commit | What |
|---|---|
| `ce932a7` | Step 1: last 11 pointer-stepping loops reviewed. Step caves for 0x44BDE3 and 0x44CCEF (read-only walks), bounds 0x44BDEA / 0x44CCF6 raised. 8 loops blocked by their own fixed tables (0x404B4A, 0x41FC64, 0x4314D3, 0x431B1E, 0x44CB51, 0x44CCA1, 0x44CD5F, 0x4509E8); 0x4215AE is not a session array. |
| `7d9e408` | Step 2 finding: most bounds were already raised by `d107dc0` (101/111 bit-scans, 26/31 C1, 6/8 C4). **169 `imul r,r,0xB0/0x158` sites in 0x380000-0x460000 have no cave** (the trace never ran them), e.g. 0x39D950's 0x39D957 / 0x39D97A. |
| `cba2c21` | Step 3: 0x390DB0 frame +0x100, its 16-qword name table moved to rbp+0x1F8 (32 entries), own unwind copy (block shared with 0x35B1E4), 0x38DA74 scan to 32. Its loop bounds stay 16 until its 6 uncaved imuls (0x390E85/F13/F44/FF5, 0x391342/3B2) are handled. |
| `e2aba3f` | 17 x 0xC table at block+0x4440 (#31) cannot grow in place; ~47 indexed sites need caves (list via grep in `docs/limits-players-loops.md`). Whole-block copy problem written up. |
| `272b096` | Copy/clear scope count: **8 sites + 1 compare** (0x39C4D5, 0x39C4F3, 0x39C71A, 0x39C7C4, 0x396A6F, 0x396A7F, 0x396CBE, 0x16FEA9; memcmp 0x396B3F). Copies with register sizes would not show up. |
| `f731b9f` | A+B: the 6 static-session copy/clear calls go through a Noble wrapper that mirrors the copy/clear onto the shadow slots 16-31. Non-static source (#1, network-decoded block from 0x391F0C's stack) clears the destination's slots 16-31 and logs (capped at 20). |
| `c941f23` | 0x396828 analysis: only one imul (0x396AB4, peer 0x158) addresses the stack copies S1/S2 (6 operands, fields +0xAC/+0xB0). No callee gets S1/S2. |
| `88d67ec` | C: 0x396828 frame 0x8F00 -> 0xC540 (probe size, lea rbp, epilogue, own unwind copy; GS cookie stays frame+0x8EF0). Side areas sideS1 rbp+0x8E00 / sideS2 rbp+0xB010 (0x2210 apart, 15 x 0x158 each). #5/#6 fill all 15 side records (shadow peers or zeros); memcmp extended. Cave at 0x396AB4: idx >= 17 -> rcx = 0x8EA8 + (idx-17)*0x158. **Safety gate:** `-slot-players 32` stays in 16-player mode unless `NOBLE_SLOT_PLAYERS_UNSAFE=1`, and logs the blockers. |

Existing hazard the gate covers: `d107dc0` already raised 0x396AA1 / 0x396B0A to 0x20 (peers 17-31 would have
written past the stack copies before `88d67ec`).

### Still blocking a real 32-player game
1. Untraced imul sites (169, incl. 6 in 0x390DB0): runtime route, see Next.
2. ~~17 x 0xC table at block+0x4440~~: caved 2026-11-09 (47 operands + 2 reset loops, guard counts non-session
   blocks; see `docs/limits-players-loops.md` "C12-L (c)"). Built: no, tested: no. 0x39C701 still 17.
3. Remaining step-3 tables (0x2C92050 16 x 0xA0, 17 x 6 at +0x34, 0x40A760 vtable object, the 8 blocked loops'
   tables, 17 x 0xC at session +0x61574 and 0x3FFC9C, stacks in 0x41E884 / 0x41F510 / 0x41FBE8, nine 0x1FFFF masks).
4. ~~Step 4~~: pools to 32 done in `a0f039f`; the 0xD6B8 host list stays 16 (Noble's slot 30 is a no-op,
   nothing reads it). Still open: whatever indexes the 32-record simulation views with a 16 bound.
5. Network encoding of slot counts/masks (protocol work; also #1's network-decoded source block).

### Next (in order)
1. ~~Smoke test~~ done 01:45: 32-player mode installs and runs 120 s clean with 1 player after fixing the GS
   handler byte in both frame tables (`docs/limits-players-loops.md` "Smoke test"). Original plan: `run_slot32_smoke.bat` (`f7f61b4`; ENGINE LIMITS = Extended) — one window, solo match, three runs, separate `-log` files: no slot options; `-slot-patch
   -slot-players 32` without the override (must log blockers and stay 16); same with `NOBLE_SLOT_PLAYERS_UNSAFE=1`
   (1 player, so slots >= 16 unused: checks the two grown frames and new caves install and don't crash). Ask
   Claude Code for a `.bat`; the cloud session can run it on monitor 1.
2. Step 4 (pools + host list), then the block+0x4440 caves.
3. Dev-only guard in unsafe 32-player mode: log the RVA whenever slot >= 16 reaches an uncaved imul 0xB0/0x158 site
   in 0x380000-0x460000; extend `-trace-members` to log every site hit; trace sessions through player leave, lobby
   reset, team change, mid-game join (three-window LAN on one PC). Cave what they find.
4. Then network encoding, then the three-window 32-slot test.

### Parked: swarm zombie test
Renette's flood swarm tags are in HREK and build into a test `ff_unearthed.map`; spawning them in Firefight needs
a new squad (Sapien). Everything is in `docs/swarm-test-notes.md`.

### Rules (unchanged)
Never modify the installed haloreach.dll; never run `build.bat` (use `build-dev.bat`); never push without Tyler's
OK; never edit `D:\Project-Noble-ORIGINAL`; never commit `recipes/` or `.nlp`; commit trailers per
`docs/CLAUDE-CODE-START-C12.md`; no test without asking Tyler.

## Status at end of 2026-11-08 (previous status)

Updated 15:30 ET by Claude. Everything below this section is the original design; this section is what exists
now. Last commit: `ce37aa7` (local, not pushed). Uncommitted since then: C12 docs/tools (see below).

### Profiles and engine builds (all built by `tools/limits/make_v16_particles.py`, applied as `.nlp` to the
player's own stock DLL; the installed `haloreach.dll` is never modified)
| Profile | Setting label | `.nlp` | What it adds |
|---|---|---|---|
| 0 Stock | STOCK | – | nothing |
| 1 V16 | EXTENDED | `v16.nlp` | GreenKnight V16 (objects 32,767) + Noble extras below |
| 2 V16p | EXTENDED + PARTICLES | `v16p.nlp` | + GreenKnight particles (pools 8×, 65,535 GPU particles/frame) |
| 3 Full | FULL (V16 + PARTICLES + LIGHT VOLUMES) | `full.nlp` | + GreenKnight light volumes V4 (5 pools 160 → 65,535), ported by DeepSeek (`lv_port.py`) |
**Proof nothing else changed:** `--ai-bank-extra 0 --render-extra 0` reproduces GreenKnight's exact V16
`b8810c3b…` and V16p `47bc0723…`. Default builds: v16 `007a4dac…`, v16p `6403f044…`, full `d5b0122e…`.

### Noble extras in every Extended build
- **Game-state bank 1 (AI) +6 MB** (`--ai-bank-extra`, operand RVA 0x16155): stock 0x220000 had ~5 KB spare.
- **Render bank 3 +8 MB** (`--render-extra`, operand RVA 0x1619A): room for 16,384 cached object render states.
- **Main bank 4** sized from measured use in Full (0x3250000). Use the BANK diagnostic before changing banks.
- **Three per-actor stack buffers grown:** AI update scheduler 0x70ABBC (96 → 1,024 entries, stack-commit hook),
  spawn-candidate list 0x76EBF8 (512 → 1,024 + its 0x200 caps), search scratch 0x76E814 (+0x800, caps 0x400,
  stack-commit hook). Ops: `AI_FRAME_OPS`, `AI_FRAME2_OPS`/`AI_FRAME2_CAP_OPS`, `AI_FRAME3_OPS`/`_CAP_OPS`.
- **Effect list guard 0x1D4F85:** 4 static lists × 512 at RVA 0x477E700 had no bounds check (crashed FULL in an F6
  horde); a `.deltrac` cave now drops entries past 511. Emitted only when extras are on.

### Runtime pieces (Noble code)
- `PoolResize.cpp` (hooks data_array_new 0x12FCC): `-ai-actors N` / SETTINGS > ENGINE > AI ACTORS (96/256/512/1024):
  actor + prop/prop_ref/prop_search scaled, command scripts, object list header, dynamic firing points, swarm,
  script threads (det hs thread 1,024, non-det 32, list object reference 512); `-render-states N` (render bank
  budget). Budgets read from the loaded engine; never grows past the added room.
- `PoolReport.cpp`: `POOL|…` and **`BANK|n|used|capacity`** (bank table RVA 0x27BF270, stride 0x18F0, +0xA0 cap,
  +0xA8 used); warns on overflow. `-pool-trace`, `-ai-pools a,b`.
- Stack-overrun reporter (`gs_failure_report`, Extended): logs `STACK OVERRUN in haloreach+0x…` with frames before a
  fail-fast (0xC0000409) kills the process. Windows Event Log is the fallback.
- `SchedulerGuard.cpp`: waits for free jobs; ceiling is 16,384 jobs (a dense 5,000-object grid asks ~74k → crash
  0x68B21F). Known limit, inline arrays at scheduler body +0x230010/+0x260038/+0x290038; not fixed.
- `SpawnTest.cpp` (dev): F9 objects, F8 128-body crowd, F7 all squads, **F6 horde**, `-horde N`, `-horde-team`,
  `-horde-no-teleport`, `-horde-delay`; waits for the real cinematic flag (game-state root +0x130).
- Join check (D3): hosts send `limits_id` (= `limits::LocalNetworkId()`), joiners refuse a mismatch. **Inert until the
  private backend stores/returns `limits_id`.**
- Unwind for Full's relocated LV routines registered at load (`RtlAddFunctionTable`).

### Results (tested)
| Limit | Stock | Now | Evidence |
|---|---|---|---|
| Objects | 2,048 | 32,767 | V16; ~8,800 moving objects before the scheduler ceiling |
| AI actors | 96 | 1,024 | 973-982 live at wave end, 5 min no crash (`-horde 1000 -horde-team 0 -horde-no-teleport`); ~18-36 fps |
| Render states | 4,096 | 16,384 | 3,800 objects 5+ min; visible-past-4,096 check still open (V16 draw list, docs/limits-render.md) |
| Particles | 2,048 | 16,384 pool / 65,535 per frame | V16p |
| Light volumes | 160 | 65,535 | Full, 5-min plasma Firefight + F6 horde, no crash |
| Script threads | 320 | 1,024 | pool report |
| Players | 16 | in progress | see C12 below |

### C12: 128 real players (option 1, chosen by Tyler)
Keep the engine's own netcode and authority; move player/peer slots ≥ 16 into Noble-owned overflow storage; steps
32 → 64 → 128; 64+ expected to need a real dedicated host (the current `noble-dedicated` is a stub: no engine session).
- **Session objects:** 4 static membership objects, RVA 0x2B07418 + k × 0x61668 (ctor 0x3CF864). Inline arrays:
  peers 17 × 0x158 at +0x60 and +0x2270, players 16 × 0xB0 at +0x1740 and +0x3950; scalar fields (counts, 32-bit
  mask +0x173C, ids at +0x4464…) do not move.
- **Trace:** `src/host/MemberTrace.*` (`-trace-members`, dev only; stalls the game after a few minutes, known).
  Runs 1-3 (menu, Slayer start, two-instance LAN join) → **417 slot-record accesses from 136 address sites**
  (`docs/limits-players-sites.json`, `tools/limits/slot_rewrite.py`).
- **Patch design:** overflow blocks at the same 0x61668 stride, so each redirect is `dst += C` with one constant per
  array; slots 0..15 keep their exact addresses. 65 imul sites generated automatically, ~70 hand-reviewed. Caves
  live in Noble-allocated executable memory near the engine, patched at load with byte checks (no room left in
  the DLL). Details: `docs/limits-players-audit.md`, `docs/limits-players.md`.
- **Test rig (one PC):** `tools\lan-two-instance.ps1` starts a host (build\bin\Release) and a client
  (build\bin\Release2, second data folder) joined over 127.0.0.1:11774.
- **Step 2 (15:30 ET, Claude, written but not built yet):** `-slot-players 32` adds 6 pointer-stepping loop caves
  (`gen_steps.py` → `SlotStepTable.inc`) and 137 bound/mask edits (`gen_bounds.py` → `SlotBoundsTable.inc`), byte-checked,
  all or nothing; shadow slots are now built with the engine's record ctors. **Peers stop at 32** (DWORD peer mask).
  Still open: the fixed 16/17-entry tables (41 bounds kept at 16/17 because of them), nine more `0x1FFFF` peer masks,
  whole-block copies (A ↔ B, messages) that skip the shadow slots, the network encoding of slot counts/masks, and the
  players pool itself. Full list and test steps: `docs/limits-players-loops.md`.
- **Next:** build + the two regression runs in `docs/limits-players-loops.md` (16-player mode, then `-slot-players 32`
  with 2 players); then block copies + network encoding (the real protocol work), then the tables.

### Other open items
- C11 part 2: V16's 4,096 visible draw list (needs eyes on 4,096+ visible objects).
- Scheduler ceiling (16,384 jobs), effect lists capacity (4 × 512 → `.hkwork` if effects go missing).
- Headless dedicated host (real engine session) for 64-128 players.
- Backend: store/return `limits_id` so the join check works.

## Goal

Raise Halo: Reach's engine limits inside Noble so that 64-player lobbies render every player, Forge
and custom maps can hold far more objects, and a Flood horde / swarm mode becomes possible. More
particles and light volumes come later.

Hard rules from Tyler:

- **Never modify the player's installed `haloreach.dll`.** Noble builds a patched copy in its own
  data folder from the player's stock DLL and loads that copy. MCC keeps working untouched.
- Players must not have to hex edit anything. It is a Noble setting.
- If anything doesn't match (another DLL build, a bad patch file), Noble falls back to the stock DLL and
  logs why. It never refuses to start because of this feature.

## Where the research came from

The local recipe files are in `tools/limits/recipes/` (gitignored, see "Files" below).

| Source | What it is |
|---|---|
| GreenKnightOP, **V16** (`recipes/v16/`) | Object pool 2,048 → 32,767 plus the rendering, physics, scheduler and state-bank changes that go with it. 642 overwrite rows plus a 969,768-byte append. `REACH_MODIFIED_DLL.dll` that Tyler has is exactly V16 (SHA-256 `b8810c3b…0a5f24`, verified). |
| GreenKnightOP, **particles** (`recipes/particles/`) | GPU particles 7,168 → 65,535, CPU particles 2,048 → 16,384, emitters 832 → 6,656. 66 rows plus a 512-byte append. |
| GreenKnightOP, **light volumes V4** (`recipes/light_volumes/`) | Light volumes 160 → 65,535, transparent draw queue 1,024 → 16,384, effect-message queue 512 → 65,536. 95 rows plus a 351,784-byte append (hash verified). |
| Renette, Sapien work (`recipes/sapien_reference/`) | Sapien-exe patch (render-packet arena, cached render states) and her `migrationpooltrick.hsc` swarm script. **Not portable to the DLL**: different binary. Useful as a list of which limits matter for swarms. |
| Chat log (`recipes/sapien_reference/chat_log.txt`) | Discord context. Key points below. |

All three DLL recipes are for **`haloreach.dll` 1.3528.0.0**, stock SHA-256
`738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894`, length 13,229,016 bytes.

### Key points from the chat

- Renette: with stock limits **only about 34 Spartans render**, so Noble's 64-player lobbies
  (`LobbyPatches`) already hit this. The object pool (2,048) can also run out with 64 Spartans plus map
  objects. V16's visible render list (1,024 → 4,096) plus the object pool fix both. GreenKnight showed
  3,025 bipeds + 3,027 weapons (6,064 objects) rendering with no flicker.
- MtnDewIt: raising limits breaks **campaign saves** (checkpoint size changes). **Films are a non-issue**:
  the team decided to drop film compatibility.
- Renette's swarm limits: actors 96 (she uses 93), command scripts 40, active object lists 48,
  references per map 128, script threads 320, syntax nodes ~6,457, dynamic firing points 16. In Sapien
  she crashes on a static array past 987 Flood or 384 Spartan models.
- Her script works around the engine updating only about 8 actors per squad each tick by migrating actors across 12
  pool squads (`ai_migrate` with a "siphon" unit).

## Verified findings (Claude, 2026-11-08)

1. **The three recipes cannot be applied together as written.** All three:
   - add their section in the same header slot (file `0x388`) at the same RVA `0x04EDA000`;
   - rewrite the same game-state size operands (render bank, main bank, checkpoint, total) to
     different values;
   - redirect the same 20 checkpoint-snapshot references to different addresses;
   - replace the PE exception directory (LV and V16 each with their own full copy of `.pdata`).
   Outside that shared infrastructure, no edits overlap.
2. **Hidden conflict, particles × light volumes.** LV V4 relocates the "effect and particle
   initialization" routine (stock file `0x114C88`, 1,302 bytes). Three particle edits sit inside it
   (`0x114F85`, `0x114FD4`, `0x115003`) and would be dead code. In V4's relocated copy the same operands
   are at file **`0xC9E10B`** (systems/locations 576), **`0xC9E15A`** (CPU particles 2,048) and
   **`0xC9E189`** (CPU emitters 832).
3. **Game-state sizes follow fixed formulas** (checked against all three recipes):
   - `checkpoint = render_bank + main_bank + 0x260000`
   - `total = checkpoint + 2 * main_bank`
   - The recipes grow the banks for different pools, so the increases add up:
     render_bank +0x820000 (P), +0x2A70000 (LV), +0x1F0000 (V16); main_bank +0xA60000 (LV),
     +0x2340000 (V16). Stock: render 0x370000, main 0x490000.
   - All three combined: render `0x37F0000`, main `0x3230000`, checkpoint `0x6C80000` (108 MiB),
     total `0xD0E0000` (209 MiB). V16 alone: checkpoint 48 MiB, total 127 MiB.
4. **No collisions with Noble's existing hooks.** Every haloreach RVA referenced in `src/` was compared
   against every patched byte (±16) in all three recipes. Nothing is close.
5. **AI and script pools located** (stock file offsets, untouched by V16). All are created by the
   data-array allocator at RVA `0x12FCC` (`rcx` = name, `edx` = count, `r8d` = record size):

   | Pool | Count | Record size | Count instruction | File offset |
   |---|---:|---:|---|---|
   | actor | 96 | 0xD00 | `8D 56 5C` lea edx,[rsi+0x5C], rsi = 4 | `0x68CCAC` |
   | command scripts | 40 | 0x1EC | `8D 55 27` lea edx,[rbp+0x27], rbp = 1 | `0x68D27E` |
   | object list header | 48 | 0xC | `8D 57 24` lea edx,[rdi+0x24], rdi = 0xC | `0x80C7A` |
   | swarm | 32 | 0x34 | `41 8B D6` mov edx,r14d, r14 = 0x20 | `0x68CD87` area |
   | prop | 256 | 0xB0 | `BD 00 01 00 00` mov ebp,0x100 | `0x68CE85` |
   | squad | 256 | 0xEC | `8B D5` mov edx,ebp | `0x68CFE8` area |
   | clump | 20 | 0x10C | `8D 7E 12` lea edi,[rsi+0x12] | `0x68D0BB` |

   Allocator function for the AI pools: RVA `0x68D75C`–`0x68E028`. Raising a count needs more room than
   the 3-byte `lea` (disp8 tops out at +127), so it needs a small trampoline. Other AI code very likely has static
   arrays sized to 96 actors: find those **before** raising the count (this is probably Renette's
   "static array crash").
6. **Not found yet:** script thread limit (320), per-map reference limit (128), syntax nodes,
   dynamic firing points. Nothing in any recipe touches lightmaps or shadows.
7. Renette's GitHub notes (`github.com/haloreachfan200-collab/test`) could not be fetched from the
   claude.ai session (robots / access). Worth reading by hand.

## Architecture

```
SteamLocator → stock haloreach.dll path
        │
        ▼
limits::PrepareEngineDll(stockPath, requestedProfile)          (src/host/EngineLimits.*, Claude)
   1. requested == Stock → return stockPath
   2. SHA-256 stockPath == 738dd2d2… ? else → stockPath + reason "unsupported build"
   3. cached copy exists at %LOCALAPPDATA%\Noble\engine\<profile>-<outhash8>\haloreach.dll
      and its hash matches the expected output → return it
   4. else apply the recipe (overwrite rows with stock-byte checks + append), verify output hash,
      write atomically (temp file + rename) → return it
   5. any failure → stockPath + reason
        │
        ▼
Loader::LoadReachDll(dllToLoad, stockPath)   ← second argument = folder for dependency lookup
                                               (xaudio/MCC binaries path derivation must keep using
                                                the STOCK path's folder, not the copy's)
```

- The copy lives outside the MCC install, so MCC and Steam file verification never see it.
- Recipes ship inside Noble as a **compact diff** (overwrite rows + append block), never as a full DLL.
  The append blocks contain relocated copies of Reach's own code, so they only work on the player's own
  stock DLL. They must not be committed as part of a full DLL or any other game file.
  (Format: `LIMITS-TODO.md` C2.)
- Profiles (network id in brackets): `Stock` [0], `V16` [1], `V16Particles` [2] (done, `v16p.nlp`).
  Later: `Full` [3] (V16 + particles + light volumes), with its own rebased recipe.
- **Every player and server in a session must run the same profile.** Game-state layout differs, so
  a mismatched join would desync or crash. The host advertises its profile and clients that don't
  match are refused with a clear message (D3).
- Campaign saves from different profiles are incompatible: they go in separate folders (D4).

### Interface contract (so both agents can work in parallel)

Claude owns the implementation; DeepSeek codes against this header from day one.

```cpp
// src/host/EngineLimits.hpp
#pragma once
#include <cstdint>
#include <filesystem>
#include <string>

namespace noble::limits {

enum class Profile : uint32_t { Stock = 0, V16 = 1, V16Particles = 2 };

struct PrepareResult {
    std::filesystem::path dllPath;   // what Loader should load (stock path on any fallback)
    Profile active = Profile::Stock; // what is actually running
    std::string reason;              // empty on success; human-readable on fallback
};

// Never throws. Safe to call before the engine exists. May take ~1 s the first time (writes the copy).
PrepareResult PrepareEngineDll(const std::filesystem::path& stockDll, Profile requested);

Profile ActiveProfile();                  // valid after PrepareEngineDll
const char* ProfileName(Profile p);       // "Stock", "Extended (V16)"
uint32_t ProfileNetworkId(Profile p);     // value sent in lobby / server registration
std::string SaveFolderSuffix(Profile p);  // "" for Stock, "_ext_v16" for V16

} // namespace noble::limits
```

## Files

| Path | In git? | Notes |
|---|---|---|
| `LIMITS-HANDOFF.md`, `LIMITS-TODO.md` | yes | This plan. |
| `tools/limits/recipes/**` | **no** (gitignored) | Raw recipes from GreenKnight / Renette. Local reference only. |
| `tools/limits/build_recipe.py` | yes | (C2) Turns a recipe into Noble's compact diff. |
| `src/host/EngineLimits.*` | yes | (C1) Builder + profile API. |
| Generated diff `data/limits/*.nlp` | decide with Tyler | Contains Reach code bytes; ship alongside the client, not in the public repo. |

## Safety notes

- Writing the patched copy: temp file then `MoveFileExW(..., MOVEFILE_REPLACE_EXISTING)`. Never
  write next to the stock DLL.
- Check the **stock bytes of every overwrite row** before writing (the recipes give them), not just
  the file hash. Abort the whole patch on the first mismatch.
- Log: requested profile, active profile, reason, input hash, output hash, copy path.
- Memory: V16 reserves about 450 MB of virtual address space (`SizeOfImage` 0x1B0A2000). Fine on x64.
  Each campaign checkpoint copy is 48 MiB instead of 10 MiB, so expect slower saves.
- `noble-dedicated.exe` must load the same profile as its clients (D2).
