# Project Noble: handoff (2026-10-27, 05:05 ET)

This picks up a reverse-engineering and C++ effort on **Tyler's Windows PC**. All paths below are real paths on that machine.
Tyler (GitHub `WeabooTyler117`) works with a partner (GitHub `O7OF`) who owns the repo and writes most of the code.
Tyler mostly runs builds, tests, and does the reverse engineering; he is not a C++ developer, so give him exact commands
(one command per code block, so each can be pasted on its own).

**Goal:** `noble-client.exe` loads MCC's own `haloreach.dll` (Halo: Reach) and drives it **without MCC**, like the existing
community project Reach Renaissance (RR, installed at `D:\Reach`) does. Ownership is verified through Steam, nothing in the
MCC install is modified, and no RR code or data is redistributed. RR is used only as a reference for behaviour.

---

## 1. Where things are

| What | Path |
|---|---|
| **Repo (Noble)** | `D:\Halo-Reach-Community-Edition` (remote `https://github.com/O7OF/test-build-01`, branch `main`) |
| Built client | `D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.exe` |
| Run log / crash dump | `build\bin\Release\noble-client.log` (overwritten every launch), `noble-crash.dmp` |
| Noble data folder (git-ignored) | `build\bin\Release\data\` (`sandbox\`, `userdata\`, `mods\`, local test files) |
| Map-info sandbox (created at runtime) | `build\bin\Release\data\sandbox\haloreach\maps\info\` (the process `cwd` is switched to `data\sandbox`) |
| Repo docs | `docs\HANDOFF.md` (this file), `docs\hud-text-findings.md`, `docs\boot-investigation-2026-10-27.md`, `docs\architecture_plan.md` |
| **MCC (Steam)** | `D:\Steam\steamapps\common\Halo The Master Chief Collection\` |
| Engine DLL | `...\haloreach\haloreach.dll`, SHA-256 `738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894` (the only build RR supports; all RVAs below are for this build) |
| MCC game variants | `...\haloreach\game_variants\*.bin` (`slayer_054.bin`, `hr_ff_*.bin` for Firefight, ...) |
| MCC sound banks | `...\haloreach\fmod\pc\sfx.fsb`, `english.fsb` (found through host slot 48) |
| MCC binaries | `...\MCC\Binaries\Win64\` (ships `xaudio2_9redist.dll`, which `haloreach.dll` loads for sound) |
| Reach Renaissance install (reference only) | `D:\Reach` (`bin\host-probe.exe`, `bin\menu-overlay.exe`, `data\profile_1.bin`, `data\lb_m_c_1.bin`, `data\launch_block_1.bin`, `data\blocks\*.bin`, `data\menumaps\mainmenu_all.map`) |
| RR run logs | `C:\Users\tyler\AppData\Local\ReachRenaissance\runs\<timestamp>\` (`menu\`, `guest\`, `match\` host_probe logs, `*-args.txt`) |
| **RE workspace** | `D:\REACH REVERSE ENGINEER` (not a git repo) |
| RR analysis notes | `D:\REACH REVERSE ENGINEER\docs\rr-launch-flow.md`, `rr-tags.md` |
| RR pseudocode (Ghidra) | `D:\REACH REVERSE ENGINEER\docs\evidence\pseudocode\*.c` (`host-probe.exe__host__hit@0x14007c600.c`, `__main.c`, `__set_game_variant@0x1401c3450.c`, ...) |
| haloreach.dll index | `D:\REACH REVERSE ENGINEER\docs\evidence\haloreach\` (`haloreach_functions.csv`, `haloreach_strings.csv`, ...) |
| Omaha symbol tables | `D:\REACH REVERSE ENGINEER\docs\evidence\symbols\` (Xbox 360 addresses, names only) |
| Leaked 2010 build (source of Omaha symbols) | `D:\Halo Reach (Jul 24 2010) (Omaha) (Cache Build)`. Keep anything derived from it **out of the repo** |

**Tools installed:** Visual Studio 2022 Community + Desktop C++ (CMake bundled), Git, Node.js, **Ghidra 12.1.4** at
`C:\tools\ghidra_12.1.4_PUBLIC`, **JDK 21** at `C:\Program Files\Eclipse Adoptium\jdk-21.0.12.101-hotspot`
(`GHIDRA_INSTALL_DIR` and `JAVA_HOME` set). The partner's PC does **not** have Ghidra configured.

`host-probe.exe` (RR) still has Rust symbols: `llvm-nm -C` / `llvm-objdump -d -C` give real function names
(`host_probe::host::hit`, `host::hit2`, `host::load_profile`, `text::glyph`, `pad::fill`, `live::remap_keys`, `uicursor::*`, ...).
That is the fastest way to answer "what does RR do for slot N". `objdump` on `haloreach.dll` itself is also quick
(~2.5 M lines) for tracing engine code at an RVA.

## 2. Build and test (exact commands)

Use **Developer Command Prompt for VS 2022** (plain PowerShell has no `cmake`; in cmd use `cd /d` to change drive).

```
cd /d D:\Halo-Reach-Community-Edition
```
```
cmake --build build --config Release
```

* If `build\` is missing or CMakeLists changed a lot: `cmake -S . -B build -G "Visual Studio 17 2022" -A x64` first.
* The post-build step copies a top-level `data\` folder; if the repo has none the build "fails" after linking. Create it once:
  `if not exist D:\Halo-Reach-Community-Edition\data mkdir D:\Halo-Reach-Community-Edition\data` (the post-build copy should
  be wrapped in `if(EXISTS ...)` in CMakeLists).
* `LNK1104: cannot open file ...noble-client.exe` = the game is still running. Close it or `taskkill /IM noble-client.exe /F`.

Run (Steam must be running), from `build\bin\Release`:

| Command | What it does |
|---|---|
| `noble-client.exe` | Noble front-end menu (also for `-map mainmenu` / `-mode mainmenu`, which the C# launcher passes) |
| `noble-client.exe -map 20_sword_slayer -mode multiplayer` | Sword Base directly (most tested map) |
| `noble-client.exe -map m10 -mode campaign -difficulty 1` | Winter Contingency (0 easy .. 3 legendary) |
| `noble-client.exe -map ff10_prototype -mode firefight -difficulty 1` | Firefight (default game type Standard) |

Other args: `-variant <path to MCC game variant .bin>`, `-mvar <path>`, `-mouse-scale F` (mouse look per pixel, default 0.0008),
`-profile <file>` (load a captured profile instead of Noble's built-in one; relative paths are resolved against the exe folder).
Read `build\bin\Release\noble-client.log` after a run. Useful lines: `launch state N`, `Slot 4 - EngineEvent`, `slot N first call`,
`FATAL EXCEPTION`, `FrontEndMenu: PLAY ...`, `Slot 48 ... .map`.

## 3. Current status

| Milestone | State |
|---|---|
| Find MCC, Steam ownership check, load `haloreach.dll`, init, start | ✅ |
| Sword Base and other MP maps render and play | ✅ |
| Keyboard/mouse input (move, look, jump, crouch, sprint/ability, fire, weapon/grenade swap, Tab) | ✅ |
| Scoped aiming | ✅ (look settings block in profile) |
| Sound (gunfire, footsteps, damage) | ✅ (profile volumes + XAudio2 redist directory) |
| Mouse cursor in Reach menus (class/loadout pick) | ✅ (Windows arrow; RR uses it too) |
| Pause menu (Esc: Resume / Quit to Menu / Quit) | ✅ (ImGui overlay, base to extend) |
| Front-end menu (map + game type, Campaign, Firefight, old Servers/Armor/Workshop/Settings screens as tabs) | ✅ |
| Campaign missions (cutscene → gameplay) | ✅ tested Noble Actual (prologue, ends after cutscene) and Winter Contingency |
| Firefight | ✅ tested with Standard |
| Black screen when a game ends (engine reports results through host slot 6) | ❌ nothing handles game end yet |
| HUD / scoreboard text | ❌ mostly boxes; partner's `TextAtlas` in progress, see `docs\hud-text-findings.md` |
| HUD dim / dark blue | ❌ not gamma (RR leaves gamma at 1.0); suspect a profile setting, see findings doc |
| **Spire (`35_island`) crashes ~15 s into a match** (Slayer and SWAT) | ❌ see §6 |
| Servers / Armor Locker / Workshop Mods tabs | ⚠️ screens only; not wired to the engine |
| Engine main menu (`mainmenu.map`) | ❌ by design: MCC ships no `mainmenu.map` for Reach (RR ships its own modified 298 MB `mainmenu_all.map`, which Noble cannot) |

## 4. Repo state

Work from this session went up as separate branches/PRs, each building on the previous one:
`feat/input-slot36` → `feat/sound` → `feat/menu-cursor` → `feat/pause-menu` (merged with `main` on GitHub, includes the
partner's TextAtlas/GammaPass) → `fix/zoom-aim` → `feat/front-end-menu` (front-end menu, then campaign, Firefight and the
extra tabs). Check on GitHub which of these are merged into `main`. Uncommitted at the time of writing: `docs/hud-text-findings.md`
and this file, plus possibly the campaign/Firefight changes if they weren't committed (`git status` shows it).

Local test files that must **never** be committed (they're RR's data, all under the git-ignored `build\`):
`build\bin\Release\data\profile_1.bin` (copy of RR's captured profile) and `data\test_profile_*.bin` (bisection files). Delete
them when convenient; Noble no longer reads `profile_1.bin` automatically.

## 5. Engine facts (haloreach.dll build above)

**Exports / vtable**
| Item | Detail |
|---|---|
| `SetLibrarySettings(p)` | zeroed 0x1FE block, three UTF-16 locale strings |
| `CreateGameEngine(&engine)` | returns 0x46 |
| `Init` vtbl+0x08 | `(engine, ID3D11Device*, 0, IDXGISwapChain*, presenter=0)`; finds the HWND **of the calling thread** |
| `Start` vtbl+0x10 | `(engine, host, launch_block[0x2BF30])`; spawns the engine thread. The engine can't switch maps afterwards: RR and Noble restart the process (Noble: pause menu "Quit to Menu") |
| `PostCommand` vtbl+0x18 | `(engine, id, payload)`; 10 then 11 after ~300 ms |
| Present | done **by the engine thread**; host slot 1 runs first with `(swapchain, &flags)`. Host overlays (gamma pass, pause menu) draw there; never use the immediate context from another thread while the engine runs |

`haloreach.dll` loads `xaudio2_9redist.dll` by bare name during start-up: Noble keeps `SetDllDirectoryW(<MCC>\MCC\Binaries\Win64)`
set for the whole run (as RR does), otherwise there is no sound. Audio itself is FMOD inside the DLL.

**Launch state machine** `haloreach+0xC1A114`: 0…10 while launching, 11 = launched. Failure strings table `0x1803A2DCC`
(17 `no map selected`, 18 `no game selected`, 19 `map load failure`).

**Paths:** map list from `FindFirstFileW` over `haloreach\maps\info\*.*` relative to cwd (hence the sandbox); files through slot 48.

**Launch block (0x2BF30 bytes).** Noble builds every block itself (`BlamEngine::BuildLaunchBlock`). RR's captured blocks for
menu, MP, campaign and Firefight are byte-identical apart from these fields (layout used as reference only):

| Offset | MP | Campaign | Firefight | Menu |
|---|---|---|---|---|
| +0x0C | 3 | 1 | 5 | 1 |
| +0x10, +0x14, +0x1D5C0 | launch index 190–209 | 178–189 | 210–219 | -2 |
| +0x28 | 0 | `01 00 11 11` | 0 | 0 |
| +0x34 | 1 | difficulty 0–3 | difficulty 0–3 | 1 |
| +0x1D334, +0x1D5BC (map id) | MP map id (Sword Base 1000, Boardwalk 1035, ...) | 1035 | 1035 | 1035 |
| +0x300 game variant (decoded with `haloreach+0x373A0`) | chosen variant (default `slayer_054`) | slayer | **a Firefight variant** (`hr_ff_*.bin`; RR loads `game_variants\<mode>.bin`) | slayer |

Also: +0x18 / +0x1D5C4 `0x88880000`, +0x50 random 16-byte launch id, +0x1D308 map variant area (init `haloreach+0x6BFF8`).
Launch indices: campaign 178 m05 Noble Actual, 179 m10, 180 m20, 181 m30, 182 m35, 183 m45, 184 m50, 185 m52, 186 m60, 187 m70,
188 m70_a, 189 m70_bonus; Firefight 210 ff10_prototype, 211 ff45_corvette, 212 ff20_courtyard, 213 ff60_icecave, 214 ff70_holdout,
215 ff60_airview (verified: loads Outpost), 216 ff50_park, 217 ff30_waterfront, 218 ff_unearthed, 219 cex_ff_halo.

**Host interface** (`host[0]` = 160-slot vtable, `host[1]` = secondary interface; args `(host, a1, a2, a3, a4)`).
| Slot | Meaning | Noble |
|---|---|---|
| 1 | per-frame, before Present: `(swapchain, &flags)` | gamma pass, then pause menu |
| 4 | engine event `(reason, msg)` | logs |
| 6 | **game results** `(buffer, 0xD5A8, 0x14)`: called when a game ends (RR `results::on_game_results`) | stub, nothing reacts yet → black screen |
| 10 | return secondary interface | ✅ |
| 23 | load progress | ✅ |
| 32 / 33 | display settings / quality | ✅ / partial |
| 34 | **profile**: pointer to a 64 KB block (see Profile below) | ✅ built-in defaults |
| 36 | **input poll** `(userIndex, u8 state[0x130])`, users 0–3 every tick (`haloreach+0x23AE7`) | ✅ |
| 47 / 48 | user-data folder / file path resolver | ✅ |
| 51 | feature query | always true |
| 61–67 | text: prepare, glyph, atlas, has_icon, kerning, register_icon, metrics | partner's `TextAtlas`; see `docs\hud-text-findings.md` |
| 88 | identity `(u64* xuid, wchar_t* name, cap, userIndex)` → 1 | ✅ |
| 116 | controller map `(xuid)`; RR only logs | returns 0, writes nothing |
| iface2 147 | pointer to a 16-byte session GUID; every other iface2 slot returns 0 (RR does the same) | ✅ |

**Input state (slot 36 buffer, 0x130 bytes, same as RR byte for byte).** +0x000 u32 1 (keyboard/mouse; 0 = gamepad);
+0x004+VK u8 per key (`0x80` pressed < 500 ms, then `0x81..0xFF` held); +0x104/+0x108 f32 mouse look; +0x114 wheel;
+0x118 f32 1.0; +0x11C mouse buttons; +0x120.. gamepad (RR `pad::fill`, not done). The engine reads keys through a 104-entry VK
table (`haloreach+0x98FDF0`) but maps them to actions **only through the profile's binding table**.

**Profile (slot 34).** RR returns a zeroed 64 KB block, optionally filled from a captured profile (`--profile`, first 0xC00 bytes),
with a 4-char UTF-16 service tag at +0x1AC. Everything the engine reads from a blank profile is 0, which broke several things.
Noble's built-in defaults (`ReachHost.cpp`):
| Range | Content | Effect when missing |
|---|---|---|
| +0x1B4..+0x1DF | look/controller settings (sensitivity bytes, dead zones 0.13/0.12, 1.0 multipliers, option bytes) | aim frozen while scoped |
| +0x42C..+0xA5B | key bindings: 66 × 0x18 `{u32 action, u32 VK, 16 zero bytes}`, MCC default PC layout | no key does anything |
| +0xA5C / +0xA60 / +0xA64 | f32 volumes (game / music / other) | no sound |
Still zero: rest of +0x000..+0x42B and +0xA68..+0xBFF (suspected to hold the HUD brightness/opacity setting).

**UI cursor.** `haloreach+0x2D2018` runs only while a Reach screen that uses the pointer is up (RR `uicursor::install` hooks it,
prologue `48 89 5C 24 18`). Noble hooks it with MinHook; within 250 ms of a call the mouse is freed, the arrow shown and no
look delta sent. The engine reads the pointer itself (GetCursorPos/ScreenToClient).

## 6. Known problems and next steps (in order)

1. **Spire crash.** Two runs (Slayer and SWAT) crash at `haloreach+0x362FF` ~15 s into the match, right after a HUD icon is
   registered with `TextAtlas`. That function is the engine's whitespace check (binary search over a 12-entry table at
   `haloreach+0x9903C0`); it reads out of bounds for any character below 0x09, so the engine was laying out text containing a
   control character. Call chain `0x36347 ← 0x1D0371 ← 0x1D0B77 ← 0x1CFEDF ...` (text layout). Most likely bad glyph/icon data
   from the host text slots; fix §2 of `docs\hud-text-findings.md` first and re-test. To confirm, build a commit from before
   `TextAtlas` and run Spire.
2. **HUD text.** `docs\hud-text-findings.md`: glyph record has atlas X at +0x08 (not +0x02) and origin y/x swapped at +0x10/+0x12;
   verify `register_icon` (slot 66) arguments against RR `text::register_icon` (0x14028cad0).
3. **Dim HUD.** Run with `-profile D:\Reach\data\profile_1.bin`; if the HUD is bright, bisect the remaining zero profile regions
   (same method as the zoom fix: build test profiles = Noble defaults + one region of RR's, pass with `-profile <full path>`).
4. **End of game.** When slot 6 fires, go back to the front-end menu (restart Noble like "Quit to Menu"), or for campaign start
   the next mission (Noble Actual → Winter Contingency, etc.).
5. **Wire up the old tabs**: servers (P2P host/join), armor (profile/armory slots), Workshop mods (map loading through slot 48).
6. Mouse wheel (`WM_MOUSEWHEEL` → +0x114) and gamepad (XInput into +0x120..).
7. Remove the remaining debug-only bits when they're no longer useful (slot first-call logging is fine to keep).

## 7. Rules for this project

- Read-only toward MCC files. Writes go to `build\bin\Release\data\…`.
- Never commit or publish: RR binaries/data/decompiled code (including `profile_1.bin`, launch blocks, `mainmenu_all.map`),
  Omaha-derived files, `haloreach.dll` or dumps of it (e.g. dumpbin output), crash dumps, logs (they contain a player name/ID).
  The repo is **public**. GitHub rejects files over 100 MB.
- Keep the Steam ownership check. No anti-cheat bypassing, no official MCC online services.
- Small, separate PRs. The partner edits `ReachHost.cpp`/`BlamEngine.cpp` too, so tell him before large edits; when both sides
  change `HostSlot1_FrameTick`, keep the gamma pass first and the pause menu after it.
- Before overwriting a file, re-read the current version first, and re-check it a minute later (file sync has put stale copies
  back before).
- Commits, PRs and docs carry only the authors' names (no "generated with" or co-author lines).
