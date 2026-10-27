# Boot investigation: 2026-10-26/27 night session

Tested on Tyler's PC: MCC on `D:\Steam`, haloreach.dll SHA-256 `738dd2d2…0894` (the build Reach Renaissance supports).
All findings are static analysis of our own haloreach.dll plus comparison with Reach Renaissance's
(RR) host-probe behaviour. No RR code or data is copied into Noble.

## Where we are

| | Before | Now |
|---|---|---|
| Crash at `haloreach.dll+0x2744F2` in game globals init | yes (masked by SafeGuard hooks) | **fixed** at the root, hooks removed |
| Engine runs frames | yes | yes, 6,000+ frames, no crash |
| Crash when closing the window | yes (ntdll AV) | **fixed** |
| Load progress | stuck at 4% | **still stuck at 4%** |

## Changes (6 files, uncommitted on Tyler's PC)

1. **`src/host/D3D11Renderer.cpp`: swap chain `BufferUsage` now `RENDER_TARGET_OUTPUT | SHADER_INPUT`.**
   This was the real cause of the `0x2744F2` crash. During globals init (`FUN_1800335cc` → `0x18025103c`/surface setup
   `0x180266f90`) the engine builds rasterizer surface pool **1 = the backbuffer**: `swapchain->GetBuffer`, then
   `device->CreateRenderTargetView`, then `device->CreateShaderResourceView`. Without `DXGI_USAGE_SHADER_INPUT` the SRV fails,
   pool 1's atom array (`table 0x180C8E4B0 + 1*0x70 + 0x60`) stays null, and `0x274488` faults on it.
   The engine's own swap-chain path (`0x180250e1e`) uses `BufferUsage = 0x30`, format `0x1C` (R8G8B8A8_UNORM).
2. **`src/engine/ReachHost.cpp`: slot 32 = display settings fill, not an allocator.** RR `host::hit` case 0x20: `a1` = engine buffer,
   `a2` = its size (0x88 or 0x98 seen). Zero it, write `{u32 width, u32 height}` at +0, quality bytes
   `01 02 02 02 02 02 01 01 | 00 01 01 01 00 02 00 00` at +8, return 1.
3. **`src/engine/ReachHost.cpp`: slot 10 returns the secondary interface** (RR returns `SECOND`), not `1`.
4. **`src/engine/ReachHost.cpp`: slot 88 rewritten.** Args are `a1 = u64* out XUID, a2 = wchar_t* out name, a3 = name capacity,
   a4 = local user index`. The old code dereferenced whatever was in `*a1` and wrote the XUID there (random stack write).
   Users other than 0 now return 0.
5. **`src/engine/BlamEngine.cpp`:**
   - `SetLibrarySettings`: three `L"en-US"` strings at wchar offsets 0x00 / 0x55 / 0xAA (RR's layout), not paths.
   - SafeGuard hooks and fallback atom arrays behind `kUsePoolSafeGuards = false` (they pre-filled the pool table and hid the crash).
   - Launch block: the game variant at +0x300 is now **decoded by the engine** (`haloreach+0x373A0`,
     `ok = decode(fileBytes, out[0xFC04], &status)`) from the player's own `game_variants\slayer_054.bin`.
     The old code memcpy'd raw `.bin` bytes, which is not the in-memory layout. A random 16-byte launch ID now goes at +0x50.
6. **`src/main.cpp`: exit** calls engine Shutdown, then `ExitProcess(0)`; it no longer unhooks or unloads the DLL under live engine threads.
7. `src/common/Types.hpp`: `ReachLibrarySettings` = 3 × `wchar_t[85]`.

## Engine facts found (for a shared offsets header later)

| Item | Value |
|---|---|
| Init (vtbl+0x08) | `Init(engine, ID3D11Device*, 0, IDXGISwapChain*, extra)`, stores to `0x180C1A0F8` / `0x180C1A0F0`. Also finds the HWND of the **calling thread** (must be the window's thread). |
| Start (vtbl+0x10) | copies the 0x2BF30 launch block to `0x1824971A0`, stores host at `0x180C1A100`, spawns the engine thread `0x18000DC78` (4 MB stack) |
| PostCommand (vtbl+0x18) | `(engine, id, const void* payload)`. Ids ≥ 0x10 read a payload. RR posts 10 then 11. |
| Command 14 | rebuild rasterizer surfaces (calls host slot 2, then `0x180266f90`) |
| Renderer interfaces required | `ID3D11Device1`, `ID3D11DeviceContext1`, `IDXGIDevice1`, `IDXGIFactory1` |
| Rasterizer surface/pool table | `0x180C8E4B0`, 101 entries × 0x70; +0x10 flags, +0x58 count, +0x60 atom array (elements 0x88) |
| Game variant decoder | `0x1800373A0` |
| Map variant init / parse | `0x18006BFF8` / `0x18017AF00` |

## Open problem: load stuck at 4%

The order of host calls is the same every run, ending with: slot 88 → slot 34 → slots 118, 99 → load 1% → `mainmenu.map` → load 4%
→ (commands 10, 11) → slot 11 → nothing more, frames keep ticking.

Leads, most promising first:

1. **Text/font slots are stubs.** RR replaces vtable slots 0x3D–0x43 (61–67) with its `text::{prepare, glyph, atlas, has_icon,
   kerning, register_icon, metrics}` and slot 0x48 (72) with `emblem::draw` (`host::new_host @ 0x1400828E0`).
   Noble's 61 and 67 are called at startup and return 0. A font pipeline that never gets glyph metrics
   could block the main-menu load. Next: decompile RR's `text::prepare` and `text::metrics` and implement them.
2. **`haloreach\maps\.map` request** (empty scenario name), right after the first frame. Header fields match RR's captured menu block.
   Next: dump Noble's final launch block to a file and diff it against a known-good block region by region
   (player slots 0x100–0x2E0, 0x1CF00–0x1D0D4, 0x1D5B8–0x1D5F8, 0x1E600+).
3. Slots 98, 99, 118, 11 are stubs (RR returns 0 for these too, so lower priority).

## Coordination

Ghidra isn't configured on the other dev machine (`GHIDRA_INSTALL_DIR` unset), so REA there can't decompile.
Until it is, do the static analysis on Tyler's PC.
