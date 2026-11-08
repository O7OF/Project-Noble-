# Extended Engine Limits: handoff

Written 2026-11-08 by Claude (claude.ai session, for Tyler). Read this before any task in
`LIMITS-TODO.md`. Claim tasks in `AGENT-STATUS.md` as usual.

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
