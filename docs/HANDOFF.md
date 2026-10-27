# Project Noble: agent handoff (2026-10-27, 02:25 ET)

You are picking up a reverse-engineering and C++ session on **Tyler's Windows PC**. All paths below are real paths on that machine.
Tyler (GitHub `WeabooTyler117`) works with a partner (GitHub `O7OF`) who owns the repo and writes most of the code.
Tyler mostly runs builds, tests, and does the reverse engineering; he is not a C++ developer, so give him exact commands.

**Goal:** `noble-client.exe` loads MCC's own `haloreach.dll` (Halo: Reach) and drives it **without MCC**, like the existing
community project Reach Renaissance (RR, installed at `D:\Reach`) does. Ownership is verified through Steam, nothing in the MCC install is modified, and
no RR code or data is redistributed. RR is used only as a reference for behaviour.

---

## 1. Where things are

| What | Path |
|---|---|
| **Repo (Noble)** | `D:\Halo-Reach-Community-Edition` (remote `https://github.com/O7OF/test-build-01`, branch `main`) |
| Built client | `D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.exe` |
| Run log / crash dump | written to the **launch working directory**: `build\bin\Release\noble-client.log`, `noble-crash.dmp` |
| Map-info sandbox (created at runtime) | `build\bin\Release\data\sandbox\haloreach\maps\info\` (the process `cwd` is switched to `data\sandbox`) |
| Repo docs | `docs\boot-investigation-2026-10-27.md` (earlier write-up), `docs\architecture_plan.md`, this file |
| **MCC (Steam)** | `D:\Steam\steamapps\common\Halo The Master Chief Collection\` |
| Engine DLL | `...\haloreach\haloreach.dll`, SHA-256 `738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894` (the only build RR supports; all RVAs below are for this build) |
| Reach Renaissance install (reference only) | `D:\Reach` (`Reach.exe`, `bin\host-probe.exe`, `bin\menu-overlay.exe`, `data\lb_m_c_1.bin`, `data\launch_block_1.bin`, …) |
| RR run logs | `%LOCALAPPDATA%\ReachRenaissance\runs\<timestamp>\` |
| **RE workspace** | `D:\REACH REVERSE ENGINEER` |
| RR analysis notes | `D:\REACH REVERSE ENGINEER\docs\rr-launch-flow.md`, `rr-tags.md` |
| RR pseudocode (Ghidra) | `D:\REACH REVERSE ENGINEER\docs\evidence\pseudocode\*.c` (`host-probe.exe__host__hit@0x14007c600.c`, `__main.c`, `__set_map…`, `__set_game_variant…`, …) |
| haloreach.dll index | `D:\REACH REVERSE ENGINEER\docs\evidence\haloreach\` (`haloreach_functions.csv` 27,709 rows, `haloreach_strings.csv`, `haloreach_imports_exports.csv`, `haloreach_name_matches.csv`) |
| Omaha symbol tables | `D:\REACH REVERSE ENGINEER\docs\evidence\symbols\` (release CSV, debug ZIP, boot/engine subset); Xbox 360 addresses, names only |
| Partial host-probe decompile | `D:\REACH REVERSE ENGINEER\docs\decompiled\host-probe.exe\functions\` (~5,800 files; the run was stopped) |
| Leaked 2010 build (source of Omaha symbols) | `D:\Halo Reach (Jul 24 2010) (Omaha) (Cache Build)`. Keep anything derived from it **out of the repo** |

**Tools installed:** Visual Studio 2022 Community + Desktop C++ (CMake bundled), Git, Node.js, Claude Code with **REA**
(`npx rea-agents`, registered at user scope), **Ghidra 12.1.4** at `C:\tools\ghidra_12.1.4_PUBLIC`,
**JDK 21** at `C:\Program Files\Eclipse Adoptium\jdk-21.0.12.101-hotspot` (`GHIDRA_INSTALL_DIR` and `JAVA_HOME` set).
The partner's PC does **not** have Ghidra configured, so REA there cannot decompile.

`host-probe.exe` (RR) still has Rust symbols: `llvm-nm -C` / `llvm-objdump -d -C` give real function names
(`host_probe::host::hit`, `host::hit2`, `host::new_host`, `text::prepare`, …). That is the fastest way to answer "what does RR do for slot N".

## 2. Build and test (exact commands)

Use **Developer Command Prompt for VS 2022** (plain PowerShell has no `cmake`; in cmd use `cd /d` to change drive):

```
cd /d D:\Halo-Reach-Community-Edition
cmake --build build --config Release
```
(If `build\` is missing or CMakeLists changed a lot: `cmake -S . -B build -G "Visual Studio 17 2022" -A x64` first.)

Tests (Steam must be running):
```
cd build\bin\Release
noble-client.exe                                          # main menu (launch index -2, map id 1035): still stuck at 4%
noble-client.exe -map 20_sword_slayer -mode multiplayer   # Sword Base: currently the most useful test
```
Other args: `-mouse-scale F` (mouse look per pixel, default 0.0008), `-profile <path>` (load a captured
profile instead of Noble's built-in key bindings, like RR's `--profile`), `-difficulty N`, `-mvar <path>`, `-variant <path>`. Let it run ~60 s (the engine has 60 s / 90 s launch timeouts), close it, read
`build\bin\Release\noble-client.log`. Useful lines: `launch state N`, `Slot 4 - EngineEvent`, `iface2 slot N first call`, `slot N first call`, `FATAL EXCEPTION`.

## 3. Current status

| Milestone | State |
|---|---|
| Find MCC on any Steam library, Steam ownership check | ✅ |
| Load `haloreach.dll`, `SetLibrarySettings`, `CreateGameEngine`, `Init`, `Start`, commands 10/11 | ✅ |
| Original crash `haloreach+0x2744F2` | ✅ fixed (swap chain needs `DXGI_USAGE_SHADER_INPUT`) |
| Engine launch state machine reaches **11 (launched)** | ✅ (after the text metrics fix) |
| Engine finds maps (`no map selected` fixed by the map-info sandbox) | ✅ |
| Sword Base: engine opens `20_sword_slayer.map`, load progress **5%** | ✅ |
| Crash `haloreach+0x7924D` after `iface2 slot 147` returned 0 | ✅ fixed (slot 147 → session GUID) |
| **Sword Base renders in-game** (world, HUD compass, motion tracker, AR 32 rounds) | ✅ merged to `main` in PR #3 |
| HUD is dim/dark blue, no HUD text | ❌ (text slots 62–66 are stubs; RR slot 1 `gamma::frame` / overlay not done) |
| Keyboard/mouse input | ✅ move, look, jump, crouch, sprint/ability, fire, weapon + grenade swap, Tab scoreboard (slot 36 + key bindings in the slot 34 profile; cursor hidden). Uncommitted, see §4 |
| Sound | ❌ none at all (not investigated yet) |
| Scoreboard / HUD text | ❌ squares instead of text (text slots 62–66 are stubs) |
| UI mouse cursor (class/loadout pick) | ❌ no cursor drawn; RR has `uicursor::*` for this |
| Main menu (`noble-client.exe` with no args) | ❌ stuck at 4%: engine still asks for `haloreach\maps\.map` (blank scenario) even with map id 1035, so the menu needs different launch-block fields than MP |

**First thing to do:** if the input branch isn't pushed yet, confirm the last test (built-in bindings, no `-profile`)
worked and commit it (§4). Then pick from §6.

## 4. Repo state

Everything up to "Sword Base renders" is on `origin/main` (PR #3, merge commit `003ff30`). Local `main` was pulled and builds.

**Uncommitted on Tyler's PC (input work, tested 2026-10-27):** commit on branch `feat/input-slot36`:
- `src/engine/ReachHost.cpp`: slot 36 input poll; slot 34 profile now matches RR (zeroed, service tag at +0x1AC,
  no writes into a1/a2) **plus Noble's built-in default key-binding table**; optional `-profile <file>`; slot 116 no
  longer writes 0xFF through a2.
- `src/host/Window.cpp`: `WM_SETCURSOR` hides the cursor over the client area while focused.
- `docs/HANDOFF.md`: this file.

`build\bin\Release\data\profile_1.bin` is a copy of RR's captured profile used for one test. It is git-ignored and
no longer loaded automatically; never commit it.

Already merged: swap chain `SHADER_INPUT`, SafeGuard/atom hooks removed, `SetLibrarySettings` = 3 × `L"en-US"`
(wchar offsets 0x00/0x55/0xAA), engine-decoded game variant (`haloreach+0x373A0`) from MCC `game_variants\slayer_054.bin` into
launch block `+0x300`, random launch GUID at `+0x50`, slot 10 → secondary interface, slot 32 display-settings fill, slot 88 rewrite,
clean exit via `ExitProcess`, text slots 61/67, slot 4 decode, launch-state logging, 160-slot iface2 with slot 147, real MP map
table and map ids at `+0x1D334/+0x1D5BC`, map-info sandbox in `main.cpp`.

## 5. Engine facts (haloreach.dll build above)

**Exports / vtable**
| Item | Detail |
|---|---|
| `SetLibrarySettings(p)` | zeroed 0x1FE block, three UTF-16 locale strings |
| `CreateGameEngine(&engine)` | returns 0x46 |
| `Init` vtbl+0x08 | `(engine, ID3D11Device*, 0, IDXGISwapChain*, presenter=0)`; stores device `0x180C1A0F8`, swap chain `0x180C1A0F0`; finds the HWND **of the calling thread** |
| `Start` vtbl+0x10 | `(engine, host, launch_block[0x2BF30])`; copies the block to `0x1824971A0`, host to `0x180C1A100`, spawns engine thread `0x18000DC78` |
| `PostCommand` vtbl+0x18 | `(engine, id, const void* payload)`; RR posts 10 then 11 after ~300 ms. Command 14 = rebuild render surfaces |
| Present | done **by the engine thread** (`0x18025113C`): host slot 1 is called first with `(swapchain, &flags)`, then `swapchain->Present`. The host must not present or use the immediate context from its own thread while the engine runs |

**Renderer requirements:** `ID3D11Device1`, `ID3D11DeviceContext1`, `IDXGIDevice1`, `IDXGIFactory1`; swap chain `R8G8B8A8_UNORM`,
`BufferUsage = SHADER_INPUT | RENDER_TARGET_OUTPUT`. Rasterizer surface/pool table `0x180C8E4B0` (101 × 0x70; +0x60 atom array, element 0x88).

**Launch state machine** `haloreach+0xC1A114`: 0…10 while launching, 11 = launched. State 10 waits on the session layer (`0x1802A40E0`).
Timeouts: 60 s `external_launch_individual_state_timeout`, 90 s `external_launch_overall_timeout` (sent to host slot 4, reason 4).
Failure strings table `0x1803A2DCC` (code 17 = `no map selected`, 18 `no game selected`, 19 `map load failure`, …).

**Paths:** engine root prefix `haloreach\` (RVA 0x985D28); maps dir `%smaps\` → `haloreach\maps\`; map list built by
**FindFirstFileW over `haloreach\maps\info\*.*` relative to cwd** (hence the sandbox). Map files are requested through host slot 48.

**Launch block (0x2BF30 bytes)**
| Offset | Meaning |
|---|---|
| +0x0C | 1 = menu, 3 = match |
| +0x10, +0x14, +0x1D5C0 | launch index: menu `-2`; MP 190–209 (Boardwalk 190 … Sword Base 196 … Forge World 209) |
| +0x18 / +0x1D5C4 | `0x88880000` |
| +0x50 | 16-byte random launch id |
| +0x300 | decoded game variant (0xFC04), via `haloreach+0x373A0(file, out, &status)` |
| +0x1D308 | map variant area (init `haloreach+0x6BFF8`, parse `0x17AF00`) |
| +0x1D334, +0x1D5BC | **map id** (Boardwalk 1035, Sword Base 1000, Forge World 3006; the menu uses 1035) |
Campaign (178–189) and Firefight (210–219) map ids are **not verified yet**. RR uses captured blocks for those modes.

**Host interface** (`host[0]` = 160-slot vtable, `host[1]` = secondary interface with its own 160-slot vtable).
Args are `(host, a1, a2, a3, a4)`; float args arrive in XMM registers (slot 61 arg3, slot 67 arg2).
| Slot | Meaning (RR `host::hit` / `hit2`) | Noble |
|---|---|---|
| 1 | per-frame, before Present: `(swapchain, &presentFlags)` | logs + launch state |
| 2 | resize: RR calls `swapchain->ResizeBuffers(0, w, h, 0, 0)` | log only |
| 4 | engine event `(reason, const char* msg)` | ✅ logs |
| 10 | return secondary interface | ✅ |
| 23 | load progress | ✅ |
| 36 | **input poll** `(userIndex, u8 state[0x130])`; the engine calls it for users 0–3 every tick (`haloreach+0x23AE7`); see "Input state" below | ✅ |
| 32 | display settings: zero `a2` bytes at `a1`; `{u32 w, u32 h}`, then `01 02 02 02 02 02 01 01 | 00 01 01 01 00 02 00 00`; return 1 | ✅ |
| 33 | display quality (RR writes `0x3F80000000000201`, `0x2BF800000`, and an audio device name from a `--settings-file`) | partial |
| 34 | **profile**: return pointer to a 64 KB block (RR: zeroed, 4-char UTF-16 service tag at +0x1AC; RR's `--profile` copies ≤0xC00 bytes of a captured profile first). **Key bindings live here**, see "Key bindings" below | ✅ |
| 35 | called a few times at match start with `(1, engine struct, 0)`; RR has no case for it (returns 0) | stub |
| 47 / 48 | user-data folder / game-file path resolver | ✅ |
| 51 | feature query `(id, bool* out, u64* out2)`; RR `--features-on 6e` | always true |
| 57 | string lookup | stub (returns empty string) |
| 61–67 | text: prepare, glyph, atlas, has_icon, kerning, register_icon, metrics | **61, 67 done; 62–66 stubs** |
| 72 | emblem draw | stub |
| 88 | identity `(u64* xuid, wchar_t* name, cap, userIndex)` → 1 | ✅ |
| 97 | capability passthrough | Noble has a handler |
| 116 | controller map `(xuid)`; RR `pad::controller_map` only logs its pad layout | returns 0, writes nothing |
| iface2 147 | return pointer to a 16-byte session GUID (RR: zeros); all other iface2 slots return 0 | ✅ verified (Sword Base renders) |

**Input state (slot 36 buffer, 0x130 bytes, from RR `host::hit` case 0x24).** User index 1 (split-screen) is zeroed.
| Offset | Meaning |
|---|---|
| +0x000 | u32 `1` = local user present |
| +0x004 + VK | u8 per Windows VK code: `0` up, `0x80` pressed (< 500 ms), then `(min((ms-500)/33, 0x7E) - 0x7F)` while held. E.g. `W` = +0x5B, `R` = +0x56, Tab = +0x0D, Esc = +0x1F |
| +0x104, +0x108 | f32 mouse look dx, dy (raw delta × scale; RR default scale 0.0008 × sensitivity/100) |
| +0x114 | i32 wheel pulse (not done yet) |
| +0x118 | f32 `1.0` |
| +0x11C | u32 mouse buttons: L 1, M 2, R 4, X1 8, X2 0x10 |
Noble only fills it while its window is in the foreground. It clips the cursor and re-centres it each tick; the window
proc hides it (`WM_SETCURSOR`). RR's `pad::fill` writes the gamepad section at `+0x120` (u16 buttons, +0x124/+0x125
triggers, +0x126 sticks) and keeps `+0x000 = 1` for keyboard/mouse (0 when a pad is in use); Noble has no pad yet.
RR's `live::remap_keys` only swaps keys for MCC rebinds; not needed for defaults.

**Key bindings (profile, slot 34).** The engine reads key states from the slot 36 buffer through a 104-entry VK table
(`haloreach+0x98FDF0`), but maps them to actions only through the profile's binding table: 66 entries × 0x18 bytes from
**+0x42C**, each `{u32 action, u32 VK, 16 zero bytes}`, actions 0–65 (W=16, S=17, A=18, D=19, Tab=20, Space=22,
LMB=8, RMB=10, R=3, …). With a blank profile every key does nothing and only mouse look works. Noble writes MCC's
default PC layout itself (`kDefaultKeyBindings` in `ReachHost.cpp`).

## 6. Next steps (in order)

1. **Commit the input work** (§4) on `feat/input-slot36` and open a PR.
2. **Sound.** No audio at all in Sword Base. Start from RR `host::hit` and the audio-related host slots (slot 33 carries
   an audio device name in RR), and `src/audio/VgmAudioHook.cpp`.
3. **UI cursor.** The class/loadout picker needs a visible mouse cursor; RR has `uicursor::{active, screen_update}` and
   `win::Input::{free_pointer, release_cursor}`. While a UI cursor is active RR skips mouse-look deltas.
4. **HUD text and brightness.** Scoreboard shows squares. Implement text slots 62 (`text::glyph`) and 63 (`text::atlas`): GDI-rendered glyphs in a D3D texture atlas.
   Slot 62 first call was `(a1=0x31 '1', a2=ptr, a3=0x12 size 18)`. Check RR slot 1 (`gamma::frame`) for the dim/blue HUD.
5. **Main menu.** Even with map id 1035 the engine requests `haloreach\maps\.map`. Diff Noble's menu block against RR's captured menu block
   (`D:\Reach\data\lb_m_c_1.bin`, layout only) around `+0x0C`, `+0x10`, `+0x1D334`, `+0x1D5B8..+0x1D5F8`.
6. Campaign/Firefight: work out map ids for `+0x1D334/+0x1D5BC` (compare RR captured blocks `D:\Reach\data\blocks\*.bin` for the layout only; don't ship them).
7. Mouse wheel (needs `WM_MOUSEWHEEL` from the window proc) and gamepad (XInput into the `pad::fill` area).

## 7. Rules for this project

- Read-only toward MCC files. Writes go to `build\bin\Release\data\…` (VFS redirects writes).
- Never commit or publish: RR binaries/data/decompiled code, Omaha-derived files, `haloreach.dll`, dumps, logs (they contain a player name/ID).
  The repo is **public**.
- Keep the Steam ownership check. No anti-cheat bypassing, no official MCC online services.
- Small, separate PRs. The partner edits `ReachHost.cpp`/`BlamEngine.cpp` too, so tell him before large edits.
- When editing files from a remote session, re-read the current file before writing (a stale copy of `ReachHost.hpp` once dropped a line).
