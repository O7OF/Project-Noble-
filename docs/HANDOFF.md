# Project Noble: handoff (2026-10-27, 15:25 ET)

This picks up a reverse-engineering and C++ effort on **Tyler's Windows PC**. All paths below are real paths on that machine.
Tyler (GitHub `WeabooTyler117`) works with a partner (GitHub `O7OF`) who owns the repo and writes most of the code.
Tyler mostly runs builds, tests, and does the reverse engineering; he is not a C++ developer, so give him exact commands
(one command per code block, so each can be pasted on its own).

**Goal:** `noble-client.exe` loads MCC's own `haloreach.dll` (Halo: Reach) and drives it **without MCC**, like the existing
community project Reach Renaissance (RR, installed at `D:\Reach`) does. Ownership is verified through Steam, nothing in the
MCC install is modified, and no RR code or data is redistributed. RR is used only as a reference for behaviour.

---

## 0. Pick up here (15:25 ET)

**Git state.** Everything up to the career work was committed on `main` as one commit ("Front-end menus, MCC art, skulls,
subtitles, settings, key binding names, career/cR, master server") and rebased onto the partner's `1d2a282` (their
`EnginePatches` whitespace-crash fix is kept). The `ReachHost.cpp` conflict was resolved by keeping Noble's version: all of
the partner's ReachHost edits were in the text code that Noble had already replaced or fixed the same way (glyph record,
digit/icon shortcuts, string fallback). The one idea not carried over: pre-rasterising text in slot 61 (TextPrepare).
If `git status` still says "rebase in progress", run `git add src/engine/ReachHost.cpp`, then
`git -c core.editor=true rebase --continue`, then `git push`.

**Not pushed yet:** a build fix made after that commit: `src/engine/KeyBindings.cpp` (the action table had landed outside
the anonymous namespace) and `src/engine/ReachHost.cpp` (unused-parameter warning). With them the build passes
(14:22). To push:
`git add src/engine/KeyBindings.cpp src/engine/ReachHost.cpp`, `git commit -m "Fix KeyBindings.cpp build"`, `git push`.

**Running the test job.** `D:\Halo-Reach-Community-Edition\build\agent\go.bat` (full path; from inside `build` the relative
path fails). It writes to files, so the console stays quiet; the line `EXITCODE` on screen is a quirk of `nb_build.bat`
(`echo EXITCODE 0>>` redirects) and means the build succeeded. Current job = `nb_run13.bat`: key-bindings screenshot
(`Q_keys.bmp`), then Sword Base with `-end-round-after 40 -quit-after 90` (`R_*` files in `build\bin\Release`).

**Result of the 14:22 run:** build OK, game ran 90 s without problems, but **`-end-round-after` did not end the game**:
the log shows "ending the round" at 40 s and then nothing (no slot 6, no `Career:` lines). Either `haloreach+0x7B3EC`
needs to run inside the simulation update (not from slot 1), or ending a round alone does not end a 1-round Slayer game
(try `game_won`'s evaluator `+0x1B0810`, or the engine's end-game function that `mp_round_end` leads to). Until then,
test career by **playing a short match to the end** (Slayer time limit) and reading the `Career:` log lines and
`data\userdata\results\last_game.bin`. `Q_keys.bmp` from this run has not been looked at yet.

**cR test passed (15:19).** Sword Base Slayer played to the time limit after the 13 cR/min rebuild. Log: `took 10.3 min;
local player 0 kills 0 assists 0 deaths 2 medals 0; paid 133 cR` = 10.3 x 13, matching Tyler's count (2 deaths, nothing
else); the post-game panel showed +133 cR and 133 / 7500 toward Private. No slot-machine bonus that game (1-in-8). Per-player
stats array: deaths appear at indices 11 and 13 (both 2).

**New: freeze after the post-game panel.** The panel opened (15:19:38) but its buttons could not be clicked. Right after
the results, the engine resolved `haloreach\maps\mainmenu.map` (slot 48), which MCC does not ship; about 7 s later it made
its first iface2 slot 140 call (a2=0x28, a3=0x40), a slot 4 EngineEvent with reason 0 and an empty message, and slot 1
frame ticks stopped (last frame 111000 at 15:19:45). With no frame ticks the panel is never redrawn or given input. Likely
cause: after game over the engine tries to go to the main menu map. Next: stop or answer that transition (slot 48 for
`mainmenu.map`, or slot 140 / slot 4), or move Noble to its own menu before the engine leaves the match.

**Tyler's reference list (from the afternoon chat).** He supplied 11 screenshots plus pasted notes (loadouts, control
schemes for MCC and Xbox 360, general Halopedia info on Reach) for: subtitles, main menus, skulls, controller layout
(360 and MCC), keyboard/mouse key bindings, loadout names. The files are not in the repo (Bungie/343 art and wiki text;
keep it that way). Status:
| Item | State |
|---|---|
| Subtitles | ✅ SUBTITLES setting + `0xD5244` hook (§5) |
| Main menus | ✅ Reach-style `FrontEndMenu` screens, art read from the MCC install (`UiArt`) |
| Skulls | ✅ Skulls screen + launch block +0x48 (§5) |
| Keyboard/mouse bindings | ✅ KEY BINDINGS tab, 66 slots named in MCC's groups; `Q_keys.bmp` (14:22 run) checked: renders correctly |
| Loadout names | ✅ come from the game variant and render correctly (§5) |
| Controller layout (360 and MCC) | ❌ **still to do**: a CONTROLLER tab in SETTINGS showing the MCC default and the original Xbox 360 layouts (controller picture already readable from `controllertexturepack`), and gamepad input itself (XInput into slot 36 +0x120.., §6 item 6) |
Checked against the Halopedia notes: the menu has all 24 MP maps (incl. Noble, Defiant and Anniversary packs), all 10
Firefight maps and all 11 campaign missions; mission descriptions are Noble's own wording (don't paste the official
taglines). Not used yet but useful later: the Armory's 30 armour colours (10 bases × 3 shades: Steel/Silver/White,
Red/Mauve/Salmon, Orange/Coral/Peach, Gold/Yellow/Pale, Sage/Green/Olive, Teal/Aqua/Cyan, Blue/Cobalt/Sapphire,
Violet/Orchid/Lavender, Crimson/Rubine/Pink, Brown/Tan/Khaki) for the Armor Locker, and the Arena divisions (Onyx, Gold,
Silver, Bronze, Iron) if a ranked mode is ever added.

**Career reference data (14:45).** Tyler collected the Reach career material from Halopedia and the Bungie.net
forums; it is now in `docs/career/`: `career-reference.md` (how Reach paid cR, campaign scoring, ranks, commendations,
challenges, Armory, loadouts, controller layouts, sources) and the data files `ranks_and_pay.json`, `challenges.json`
(all 18 months Sep 2010 – Feb 2012, 2,103 challenges; no month missing, only 14 days Halopedia itself lacks),
`commendations.json` (45) and `armor.json`. Names and numbers only, no Bungie art (rank/challenge icons must be read
from the MCC install; Tyler's rank-icon sheet stays local). Open question for Tyler: keep Noble's own cR numbers or
switch to Reach's measured table (`career-reference.md` §1).

**Partner's Armory work (in progress on his PC, not pushed yet; 15:00).**
- Today `ArmorCustomizer` only writes `data/appearance.json`; nothing gives it to the engine. Slot 34 still sends only
  bindings, volumes, look settings and the hardcoded tag `NOBL`. The UI's armour names are not engine IDs.
- The engine's own main-menu Spartan preview is not possible (no Reach `mainmenu.map` in MCC; RR ships its own).
- RR applies looks by scanning engine memory for colour records (four ARGB u32s) and suit records by string id
  (`armory::write_colors`, `scan_suit`, `looks::*` hook on a colour-copy routine). The partner chose not to copy that.
- Lead to test: RR's captured profile has u32 id lists separated by `0xFFFFFFFF` at +0x40..+0x1AB (e.g.
  `0x2F 0x6B 0x7D 0x8A 0x93 0x9B`, `0xC0 0xA6 0xA5 0x9F`), possibly armour/appearance selections. Test like the zoom
  fix: Noble defaults + only that region from RR's profile, pass with `-profile`, look at the Spartan in game.
- Plan: Noble's own DX11 renderer for the Armory preview. For development he exported the Spartan and Elite render
  models to OBJ (`data\armor\spartans\spartans.obj` 329 groups incl. 51 MCC Series 8, `data\armor\elite\elite.obj`
  128 groups: default, general, officer, space, spec_ops, storyelite, ultra, zealot). **Those OBJ files are game
  assets: keep them under the git-ignored `data\`, never commit them.** The shipped version has to read the models
  from the player's MCC maps at run time. Next he is matching the Armory names to the model's group names
  (e.g. `dlc2_helmet_cndmnd`, `s8_helmet_drengr_tyr`), which can be added to `docs/career/armor.json`.
- Forwarded community request (TRAXUSi, Workshop class mods): add extra player-host object types beyond Reach's
  three (0 spartan, 1 elite, 2 monitor), e.g. `elite2`, `spartan2`, `monitor2`, using the same host code paths, with
  matching Megalo names. Engine patch inside `haloreach.dll` at run time; not started, idea only.

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
| `noble-client.exe -map ff50_park -mode firefight -difficulty 1` | Firefight on Beachhead (default game type Standard) |
| `noble-client.exe -map m10 -mode campaign -skulls 0x10` | campaign with skulls (MCC skull bits, here Blind; see §5 Skulls) |

Other args: `-variant <path to MCC game variant .bin>`, `-mvar <path>`, `-mouse-scale F` (mouse look per pixel, default 0.0008),
`-profile <file>` (load a captured profile instead of Noble's built-in one; relative paths are resolved against the exe folder).
Test-only args: `-menu-screen <main|campaign|skulls|firefight|custom|forge|servers|settings|armory|mods>` opens that
front-end screen, `-menu-tab <0-5>` picks the SETTINGS tab (1 = key bindings), `-menu-capture <frames>` saves `capture_<frames>.bmp` of the menu and exits; `-launch-poke 0x48=0x10,...`
writes u32 values into the launch block (for finding fields by experiment). `-capture-frames 300,3600` saves `capture_<frame>.bmp` next to the exe (and one shot ~120 frames after the
post-game panel appears); `-quit-after <seconds>` ends the run by itself; `-end-round-after <seconds>` ends the round (reaches the post-game results). Player settings (resolution, borderless, mouse,
invert, key overrides, volumes, menu sounds, player name, master-server URL, favourites) are in
`build\bin\Release\data\userdata\settings.json`, edited from the SETTINGS tab.

Unattended build without the developer prompt: `build\nb_build.bat` calls Visual Studio's bundled `cmake.exe` directly
and writes `build\nb_build_out.txt` (VsDevCmd hung when started from a script). `build\nb_run3.bat` = 12-minute Sword Base
test with captures. These helper scripts are under the git-ignored `build\`.

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
| Pause menu (Esc: Resume / Quit to Menu / Quit) | ✅ (ImGui overlay, Noble style) |
| Loading screen while the map loads (was ~5 s black) | ✅ map name + moving bar (`PauseMenu::DrawLoading`), ends on the first slot-10 call (tested) |
| Settings tab (controls, key bindings, audio, display, player name) | ✅ `SettingsPanel`, saved to `settings.json`, applied at next launch |
| Menu look and sounds | ✅ `NobleStyle` (Reach-like palette, Bahnschrift), `MenuSounds` (synthesised clicks, 40 ms hover cooldown) |
| Front-end menu, Reach style (`FrontEndMenu.cpp`): MAIN MENU list (Campaign, Matchmaking*, Firefight, Custom Game, Forge, Theater*, Armory, Mods, Settings, Quit; *greyed), My Party box, button hints; Campaign (mission list + preview, difficulty, skulls, score multiplier), Skulls, Firefight / Custom Game (map + preview, game type, difficulty), Forge (MP launch with MCC's `00_basic_editing_054.bin`: Forge World loads with no score/timer HUD; switching to the editor (Monitor) is untested), Games (server browser), MCC-style Settings | ✅ screenshots of every screen checked |
| Menu / loading art (`UiArt`): player's `data\ui\custom\`, Noble's `data\ui\noble\`, else read from the MCC install at run time (nothing shipped) | ✅ campaign scenes as backgrounds, map previews, skull icons, controller picture |
| Skulls (launch block +0x48) | ✅ verified (Blind hides the HUD) |
| Cinematic / dialogue subtitles | ✅ SUBTITLES setting (on by default), hook on `haloreach+0xD5244`; tested on Winter Contingency's intro |
| Campaign missions (cutscene → gameplay) | ✅ tested Noble Actual (prologue, ends after cutscene) and Winter Contingency |
| Firefight | ✅ tested with Standard. Scenario names (from MCC's preview pictures): `ff50_park` = **Beachhead**, `ff10_prototype` = **Overlook** (the old front-end menu had them swapped; BlamEngine's name aliases "beachhead"/"overlook" are still swapped, the scenario names it maps are right) |
| Game end (engine reports results through host slot 6) | ⚠️ post-game panel shows the right cR (15:19 test), but its buttons can't be clicked: the engine stops sending frames about 7 s later (§0) |
| Memory | ✅ no leak during a match: 12-minute run, private bytes 3835 → 3852 MB, working set ~1.3 GB, flat |
| HUD / scoreboard text and icons | ✅ letters, numbers and weapon/grenade icons render (glyph layout and string slots fixed, see §5) |
| HUD bars / compass / motion tracker dark navy | ❌ not gamma, not the profile (same with RR's profile), not slot 51/33; see §6 |
| Key bindings: all 66 slots named | ✅ slot numbers = MCC's `GameAbstractButton` order (enum strings in `MCC-Win64-Shipping.exe`); `KeyBindings.cpp` groups them like MCC's CONFIGURE BINDINGS (movement, actions, vehicle, UI, Forge, Theater; 10 slots only other Halo games use). Crouch default is now Left Ctrl (MCC) |
| Career / cR (`src/career/Career.cpp`) | ⚠️ first version, needs a real match to confirm stat slots: reads MCC's game-results record at slot 6, pays cR, shows it on the post-game panel, saves `data\userdata\career.json`. See §5 "Game results" |
| Spire (`35_island`) crash ~15 s into a match | ✅ fixed by the text fixes: 3+ minutes of Slayer on Spire without a crash (13:30 run) |
| Servers tab | ⚠️ LAN list, own hosted game always listed, favourites, direct connect, optional master server (`tools\master-server`). **Joining** another player's game is not possible yet (engine net slots 41/43 not implemented); the UI says so |
| Armor Locker / Workshop Mods tabs | ⚠️ screens only; not wired to the engine |
| Engine main menu (`mainmenu.map`) | ❌ by design: MCC ships no `mainmenu.map` for Reach (RR ships its own modified 298 MB `mainmenu_all.map`, which Noble cannot) |

## 4. Repo state

Work from this session went up as separate branches/PRs, each building on the previous one:
`feat/input-slot36` → `feat/sound` → `feat/menu-cursor` → `feat/pause-menu` (merged with `main` on GitHub, includes the
partner's TextAtlas/GammaPass) → `fix/zoom-aim` → `feat/front-end-menu` (front-end menu, then campaign, Firefight and the
extra tabs). Check on GitHub which of these are merged into `main`. Uncommitted at the time of writing: `docs/hud-text-findings.md`
and this file, plus possibly the campaign/Firefight changes if they weren't committed (`git status` shows it).

**Uncommitted work from 2026-10-27 afternoon** (on top of the night session below): `src/ui/UiArt.*` (new),
`FrontEndMenu.*` (rewritten, Reach-style screens + skulls), `SettingsPanel.*` (MCC style), `NobleStyle.*` (hero/small
fonts), `PauseMenu.cpp` (loading art, Firefight names), `KeyBindings.*` (groups), `Settings.*` (subtitles, campaignSkulls,
menuBackground), `ReachHost.cpp` (subtitle hook), `BlamEngine.*` (skulls at +0x48 as u64, `-launch-poke`), `main.cpp`
(`-skulls`, UiArt), `FrameCapture.*` (`SaveNow`), `CMakeLists.txt` (UiArt, windowscodecs). Suggested extra PRs: (7) skulls +
subtitles (engine), (8) UiArt + Reach-style front end + MCC settings.

**Uncommitted work from the 2026-10-27 night session** (on disk in `D:\Halo-Reach-Community-Edition`, builds, nothing committed):
new `src/common/Settings.*`, `Relaunch.*`, `src/engine/KeyBindings.*`, `FrameCapture.*`, `src/network/MasterServerClient.*`,
`src/ui/NobleStyle.*`, `MenuSounds.*`, `SettingsPanel.*`, `tools/master-server/`; changes to `ReachHost.*` (text atlas, string
table, slots 6/32/33/51, settings, memory log), `main.cpp`, `Window.*`, `SessionManager.cpp` (LAN beacon crash), `ServerBrowser.*`,
`FrontEndMenu.cpp`, `PauseMenu.*`, `UIManager.cpp`, `MissionLauncher.cpp` (map names), `CMakeLists.txt` (new files, winhttp/winmm,
`if(EXISTS data)`). Suggested PR split: (1) LAN crash fix, (2) HUD text/icons + string table, (3) settings + key bindings + style
+ sounds, (4) server browser + master server, (5) loading/post-game overlay + Relaunch, (6) test tooling (FrameCapture).

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

**Launch state machine** `haloreach+0xC1A114`: 0…10 while launching, 11 = launched (reached ~5 s before the map is on
screen; the first slot-10 call marks the game session start). Failure strings table `0x1803A2DCC`
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

**Skulls: u64 at +0x48** in MCC's skull numbering, verified (Blind hides the HUD). `haloreach+0x3A22C` converts it to the
engine's own bits (game options +0x1D8, copied to game globals +0x1EAC8, which the `skull_enable` script command flips).
The MCC bit is the skull's index in MCC's alphabetical skull list: Iron 18, Black Eye 3, Tough Luck 36, Catch 7, Cloud 13,
Famine 11, Thunderstorm 34, Tilt 35, Mythic 23, Blind 4, Cowbell 8, Grunt Birthday Party 16, IWHBYD 19 (38 skulls in the
converter). Noble used to write skulls to +0x3C, which is a different
field the engine reads as u16; that was removed.

Also: +0x18 / +0x1D5C4 `0x88880000`, +0x50 random 16-byte launch id, +0x1D308 map variant area (init `haloreach+0x6BFF8`).
Launch indices: campaign 178 m05 Noble Actual, 179 m10, 180 m20, 181 m30, 182 m35, 183 m45, 184 m50, 185 m52, 186 m60, 187 m70,
188 m70_a, 189 m70_bonus; Firefight 210 ff10_prototype, 211 ff45_corvette, 212 ff20_courtyard, 213 ff60_icecave, 214 ff70_holdout,
215 ff60_airview (verified: loads Outpost), 216 ff50_park, 217 ff30_waterfront, 218 ff_unearthed, 219 cex_ff_halo.

**Host interface** (`host[0]` = 160-slot vtable, `host[1]` = secondary interface; args `(host, a1, a2, a3, a4)`).
| Slot | Meaning | Noble |
|---|---|---|
| 1 | per-frame, before Present: `(swapchain, &flags)` | gamma pass, then overlay (loading / pause / post-game), then test capture; memory log every 1800 frames |
| 4 | engine event `(reason, msg)` | logs |
| 6 | **game results** `(buffer, 0xD5A8, 0x14)`: called when a game ends (RR `results::on_game_results`) | shows the post-game panel (only once launch state is 11); results not decoded yet |
| 10 | return secondary interface | ✅ |
| 23 | load progress | ✅ |
| 32 / 33 | display settings / quality | 32: size from the window; 33: writes the 16 bytes `0x3F80000000000201, 0x2BF800000` **directly at a1** (as RR) |
| 34 | **profile**: pointer to a 64 KB block (see Profile below) | ✅ built-in defaults |
| 36 | **input poll** `(userIndex, u8 state[0x130])`, users 0–3 every tick (`haloreach+0x23AE7`) | ✅ |
| 47 / 48 | user-data folder / file path resolver | ✅ |
| 51 | feature query `(id, bool* out, u64* extra)`: RR answers from mask FEATURES_ON = 0x6E and zeroes `*extra` | same mask; the engine only asks 1, 2, 5, 6 (all on) |
| 55 / 56 / 57 | strings: clear / register `(table, id, const char* name, const char* suffix)` / lookup `(table, id, wchar_t* out, cap)` | key = CRC-32/MPEG-2 (poly 0x04C11DB7, init 0xFFFFFFFF, no xorout) of upper-case `name_suffix` (leading `$` skipped); text from `<MCC>\data\ui\Localization\EN_HaloReach.bin` (magic 0x90CE6B7A; u32@0x78 key bytes, keys from 0x80; u32@0x7C string block; NUL-terminated UTF-8 in key order). Lookup returns 0 when unknown so the engine uses the map's own strings |
| 61–67 | text: prepare, glyph, atlas, has_icon, kerning, register_icon, metrics | ✅ `TextAtlas`: 1024² pages (≤16), shelf packing, dirty-rect upload in slot 63 |
| 88 | identity `(u64* xuid, wchar_t* name, cap, userIndex)` → 1 | ✅ |
| 116 | controller map `(xuid)`; RR only logs | returns 0, writes nothing |
| iface2 147 | pointer to a 16-byte session GUID; every other iface2 slot returns 0 (RR does the same) | ✅ |

**Glyph record (slot 62 out, 26 bytes):** ch@0, zero@2..7, atlasX@8, atlasY@0xA, w@0xC, h@0xE, originY@0x10, originX@0x12,
advance@0x14, -ascent@0x16, page@0x18. Glyph alpha = min((gray·255+32)>>6, 255) from GDI GGO_GRAY8. Icons (0xE000–0xE2FF): the
engine registers one through slot 66 `(host, ch, size, f32 scale, face, w, h, top, ARGB4444-decoded pixels, pitch)` only when
slot 64 has_icon returns false (engine path `haloreach+0x1CF4D0` reads `font_package_icon.bin`); store the pixels unchanged,
originY = top, originX = 0, advance = w, ascent = 0.

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

**Subtitles.** Before drawing subtitles the engine calls `haloreach+0xD5244`, which walks the local players' table
(`haloreach+0x2A03C50`, stride 0xAB8, profile data at +8) and reads each player's subtitle setting at profile +0xA30
(0 automatic = only when voice and text language differ, 1 on, 2 off). That table is **not** a copy of the slot 34 buffer
(writing +0xA30 there does nothing; volumes etc. are read field by field). Noble hooks `0xD5244` (prologue `48 89 5C 24 08`)
and answers "on" when the SUBTITLES setting is on.

**MCC UI texture packs** (`<MCC>\Data\UI\texturepacks`, read by `UiArt`): `<name>.perm.bin` = records of 0x1E0 bytes, magic
0xCDBFA090, name at +0x44, u16 w/h at +0x74/+0x76, u64 size at +0x88, u64 offset into `<name>.temp.bin` at +0x90; data BC1 or
BC3 (decided by size per 4x4 block). Useful packs: `levelstexturepack` (every Reach map/mission preview, 316x160 and
668x292 `…LARGE`), `skullstexturepack` (128x128 skull icons), `controllertexturepack` (controller picture, button glyphs),
`mainmenutexturepack`, `globaluitexturepack`. `loadingtexturepack.perm.bin` (304 MB) has no names: entries
`{u32 0x5E73CDD7, u32 size, u32 size, u32 0, data}`, padded; 1920x1080 BC1. Reach's 12 campaign scenes are found by a CRC-32
of their first 4 KB (table in `UiArt.cpp`). `Data\UI\BackgroundVideos\Images\*.png` are only 392x220.

**Key-binding slots.** The 66 profile slots use the engine's own action numbering, which matches neither MCC list in
`Data\BuiltInData\HaloReachController.json`. Names in `KeyBindings.cpp` come from MCC's default keys (unique key or the
W/A/S/D block); the rest are shown as "OTHER (NOT IDENTIFIED YET)". Several defaults appear twice (two `R`, `F`, `LMB`, `Space`),
so e.g. Fire may be slot 8 or 27: test a rebind of each named action in game.

**Loadout names** come from the game variant (Slayer's default list is Sprint / Armor Lock / Jet Pack / Active Camo /
Hologram; Invasion on Spire shows its own tier list starting with Scout; Air Assault / Operator / Grenadier etc. belong to
such game types). They render correctly.

**UI cursor.** `haloreach+0x2D2018` runs only while a Reach screen that uses the pointer is up (RR `uicursor::install` hooks it,
prologue `48 89 5C 24 18`). Noble hooks it with MinHook; within 250 ms of a call the mouse is freed, the arrow shown and no
look delta sent. The engine reads the pointer itself (GetCursorPos/ScreenToClient).

### Game results (host slot 6)

`haloreach+0xF95CB` builds MCC's results record (**0x5D138 bytes**) with `haloreach+0x39128` from the engine's own
results at `haloreach+0x28B83B0`, then calls host slot 6 (`[host]+0x30`) with **a1 = pointer to the record**. a2 is not a
size: it is r8 left over from the conversion loop (always 0xD5A8). Layout read from `+0x39128`:

| Offset | Contents |
|---|---|
| +0x0000 u32 | game category (engine mode byte at results +5 mapped to MCC's list) |
| +0x0008, +0x0010 u64; +0x18, +0x19 u8 | copied from the engine header |
| +0x0020 | 16 player records × 0x1C8: +0x00 u8 present, +0x08 u64 player id, +0x18/+0x28/+0x2C i32, +0x30 u8, +0x34 i32 score (engine word or dword depending on mode), +0x38.., then appearance/name copy from +0x4C |
| +0x1CA0 | 16 × 4 bytes team data |
| +0x1CE0 | 16 per-player stat blocks × 0x5448: +0x000 23 u32 general stats, +0x05C 11 u32, +0x0AC 0x180 u32 medal counts, +0x6AC 0x32 u32, then weapon tables (0x54 weapons, 6 values each) |
| +0x56160 | per-player pairs; +0x5D120/+0x5D124/+0x5D128 u32 and +0x5D130 u64 match totals |

General-stat numbering: `haloreach+0x3B208` maps engine stat n to MCC slot (0→0, 1→1, n→n+1 for n ≥ 2). Assuming Reach
kept Halo 3's statistic order, slots 8/9/10/11 = score/kills/assists/deaths (**provisional**, `Career.cpp` kStat*).
Medal numbering: table built by `haloreach+0x39B00` (0x91 engine medals → MCC ids ≤ 0x17F).

Test option `-end-round-after <seconds>` calls the `mp_round_end` script command's work function
(`haloreach+0x7B3EC(-1, 0, 1)`) from slot 1, so automated runs reach slot 6. Noble writes the last record to
`data\userdata\results\last_game.bin` (local only: it holds player names and ids; never commit it).

## 6. Known problems and next steps (in order)

1. **HUD bars, compass ticks and motion tracker are dark navy** (letters/icons are fine). The game-type intro box shown
   during loadout selection, the "Visegrad Communications Outpost" waypoint box and the hint bar in Winter Contingency have
   the same dark tint (in MCC that box is bright cyan), while the loadout cards are correct. Ruled out: gamma, profile (RR's
   `profile_1.bin` gives the same picture), slot 51 mask, slot 33. **Confirmed 15:10:** RR's own HUD (Firefight, Beachhead)
   is bright light blue (shield bar, compass, tracker, timer box) while Noble's on Sword Base is dark navy; the world looks
   the same in both. The `GammaPass` in `ReachHost.cpp` is fixed at exponent 1.0 (a no-op; a whole-frame gamma washed the
   world out), so the fix must touch only the HUD. RR has a separate "UI Gamma" setting next to Gamma, which hints at a
   HUD-only value the host supplies. Next: look at how the HUD shader gets its colour/tint (chud globals, possibly a
   render-state or a constant the host sets in another slot, e.g. iface2 slots RR answers non-zero).
   Background (Bungie's Halo 3 HUD spec, cand.land/halohud): HUD element colours, flashes and scale come from the
   map's HUD tags ("HUD globals" / chud), the HUD is drawn mapped onto the inside of a sphere with a virtual camera
   and a per-resolution FOV from those globals, and layout uses anchors (4 corners + centre) with pixel offsets. So
   the colour is tag data the engine reads itself; if RR is bright, look for a host answer that changes which
   resolution/exception block or render target format the engine picks, rather than a colour value.
2. ~~Spire crash~~: no longer happens after the glyph/string fixes (3+ minutes on Spire, 2026-10-27 13:30).
3. **Memory**: Noble logs `Memory: frame N private X MB, working set Y MB` every 1800 frames. A full 10-minute match was
   flat (+17 MB). If a leak is still seen, it is probably across front-end menu ↔ match restarts or in the menu itself.
4. **Online joining**: engine networking slots 41/43 (and session setup) are not implemented, so the SERVERS tab can host
   (listed on LAN/master server) but not join. This is the big missing piece for multiplayer.
5. **Scoreboard/results**: the slot 6 record is decoded far enough for career pay (§5 "Game results"). Still to do: a
   post-game scoreboard from it, confirm the general-stat slots with a real match, campaign "next mission".
6. Mouse wheel (`WM_MOUSEWHEEL` → +0x114) and gamepad (XInput into +0x120..).
7. Armor / Workshop tabs; remove debug-only bits when no longer useful.
   Menu ideas from Bungie's UI director (cand.land/new-page): menu backgrounds were layered concept art on cards with
   a slowly moving camera (parallax), and the main menu had a one-press "fast path" to the last mode played.
8. **Cursor lock**: in windowed mode the game keeps the cursor clipped to its window, even while dragging the title bar,
   so the window can't be moved back (seen 15:14, the window ended up behind the taskbar). Only Alt+Tab frees it. Release
   the clip while the window is being moved or isn't focused (`WM_ENTERSIZEMOVE` / `WM_KILLFOCUS`), and re-clip on focus.
9. **Respawn timer** shows "Respawn in 00000001": the number filling `camera_list.prop_respawn_time` is zero-padded.
   Check the format used for that string (string table entry `0x84091d25` in `ReachHost.cpp`).
10. **Player name / service tag** in game are still the placeholders "NobleSix" / "NOBL"; they should come from the
   player's own settings.
11. **Forge**: the menu launches MP with `00_basic_editing_054.bin`; the map loads, check in game that the Monitor/editor toggle works.
9. **Career system** (planned with Tyler). Done: `Career` module, 50 ranks (Halopedia table), cR after each match
   (per-minute pay rising with rank, kills/assists/medals bonus, 1-in-8 random bonus 100–3000, 1-in-200 jackpot 7,777;
   **no daily cap** and up to **90 paid minutes** per match in every mode (Tyler's calls). Pay goes by **base type** =
   the engine game mode BlamEngine launches with (campaign / Firefight / multiplayer incl. Forge), so modded maps and game
   types earn exactly what their stock type earns (`Career::BeginMatch` is called from `BlamEngine::LaunchMap`); all constants at the top of `Career.cpp`, Noble's own numbers), post-game cR breakdown + rank bar.
   Next: confirm stat slots (play a short Slayer with a few kills; `last_game.bin` + the `Career:` log lines), campaign
   scoring (difficulty + skull multipliers, time bonus, par scores), 45 commendations, challenges (Sep 2010 – Feb 2012
   lists) paying cR, a career/service-record screen and rank icon in the menu, Armory with the armor list.
   All the data for this is in `docs/career/` (see §0).

## 7. Partner's Outbreak-Z code (reference for Noble's menus)

The partner (`O7OF`) wrote and owns a Godot game, Outbreak-Z, and shared its scripts as reference material. They are
GDScript for Godot, so none of them compile into Noble (C++ / D3D11 / ImGui). They can be used as designs to port; because
the partner owns them, porting or copying them into this repo is fine. Keep the copies of these files outside the repo
unless one is actually ported.

**Usable almost as-is**
- `master_server.js` + `package.json`: **ported** to `tools/master-server/` with `MasterServerClient` in Noble. The
  original already had a 1 MB body limit; the real gaps were that anyone could heartbeat/delete any session and that the
  listed address came from the request body. Now: secret token per session (`X-Session-Token`), address from the socket,
  4 KB body, 4 sessions per IP, 120 requests/min per IP, 2000 sessions, text sanitised. `node test_master_server.js` passes.

**Port as designs**
- `server_browser.gd`: **ported** (favourites, direct connect, filters, host dialog with password). Ping and join need
  networking.
- `settings_manager.gd`, `build_settings_menu.gd`: Gameplay / Controls / Keybinds / Controller tabs with sliders. In Noble
  the controls write the profile fields from section 5: look settings at +0x1B4, key bindings from +0x42C,
  volumes at +0xA5C/+0xA60/+0xA64. **Ported** as the SETTINGS tab (`SettingsPanel`), controller tab not yet.
- `glass_ui.gd`: **ported** into `NobleStyle`.
- `audio_manager.gd`: only the menu parts apply. Click / hover / tab sounds with a 40 ms hover cooldown, and a menu music
  playlist (in order or shuffled, loops, no repeat of the last track when a shuffle restarts). Play through XAudio2, which
  is already loaded. In-game sound is the engine's own FMOD and needs nothing from this file. Sound source: Reach's
  sounds can't be committed (public repo); either read them at runtime from the player's MCC install (FMOD sound banks)
  or use our own / royalty-free sounds plus a `data\menu_music\` folder for the player's own files. Click/hover/tab sounds
  are **ported** as synthesised blips (`MenuSounds`); music not done.
- `game_loading_screen.gd`: **ported** as the loading overlay. The engine's slot 23 progress stays at 5%, so the bar only
  shows activity, not real progress. (`ui/LoadScreen` is never drawn while the engine runs.)

**Not applicable to Reach**
- `barracks_menu.gd`, `save_stats_manager.gd`, `floating_damage_number.gd`: Outbreak-Z skins, capes, cards, prestige and
  zombie stats. Reach armour lives in the engine/profile.
- `dev_tools_menu.gd`: zombie-mode cheats (points, perks, power-ups).
- `dev_account_search_menu.gd`, `backend_client.gd`: need Outbreak-Z's account database; its login/register ignore the
  password, so don't carry that over.
- `hud.gd`: Reach draws its own HUD.
- `scoreboard.gd`: Reach's in-game scoreboard is the engine's; a post-game one would read the slot 6 results buffer.
- `chat_box.gd`: the engine does **not** draw chat outside MCC; a chat box would have to be Noble's own overlay, and needs
  networking first.
- `check_api.gd`: one-line test script.

**Status:** settings, style, menu sounds, server browser, master server and loading screen are ported (see §3). Left:
controller settings tab, menu music, chat (after networking).

## 8. Rules for this project

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
