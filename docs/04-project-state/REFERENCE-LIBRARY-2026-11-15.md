# Noble reference library -- what exists on this machine and when to use it

Written 2026-11-15 for the next agent. Tyler asked that every agent know these references up front so nothing gets
re-discovered. **Read this before research**: the answers to many engine/UI/mod questions are already in these folders.
Paths are absolute because most of them are outside the repo.

The repo itself: `D:\Project-Noble-Closed-Source` (branch `wip-merge`, `main` = `wip-merge` = `eea9868`, v0.19.103
released, when this was updated 2026-11-15 10:50 ET). `knowledge/` is the working docs tree (git-excluded, local);
`docs/` in the repo is tracked and holds the
older engine work.

---

## 1. Ghidra MCP (PRIMARY source of engine truth -- use it, Tyler insists)

A live Ghidra project is connected to DSH:

| Fact | Value |
|---|---|
| Program | `haloreach-copy.dll` |
| Image base | `0x180000000` (Ghidra address = `0x180000000 + RVA`) |
| Functions | 27,709 |
| Format | PE x86-64 |
| sha256 | `738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894` |

Tool: `mcp__ghidra__*` (decompile_function_by_address, disassemble_function, get_function_xrefs, list_strings,
search_functions_by_name, rename_function/rename_variable, set_function_prototype, read_memory, get_xrefs_to/from).
`get_program_info` first, as the MCP server instructions say.

**Address convention**: code comments in this repo use RVAs (e.g. `0x2EDC5E`). In Ghidra pass
`0x180000000 + RVA` and decompile the containing function. `decompile_function_by_address` accepts the site address
(e.g. `0x1802EDC5E`) and returns the containing function.

Examples already used (see 5a/5b below): decompiled `0x18003B2F4` (global item -> engine slot translator), inspected
`0x1800393CC` inside `FUN_180039128` (profile settings -> item id mapping), and read the 7-byte patch at `0x2EDC5E`
from `EnginePatches.cpp`. The translator switch in `FUN_18003b2f4` is large; search its body for the `0x9d`..`0xba`
cases to recover item -> engine-slot tables.

**When to use**: any question about engine internals, hooks, RVAs, item tables, camera/player limits, network message
layouts, or verifying that a patch site still matches the installed `haloreach.dll`. Do not guess an RVA -- decompile it.

---

## 2. Ghidra decompile exports (fast text search, no MCP round-trips)

Two folders on the Desktop contain a batch export made with `rea decompile_all` (Ghidra 12.1.4) -- every function as a
`.c` file with address-named files:

### `C:\Users\tyler\Desktop\Reach.exe\`
- `functions\*.c` -- 4,119 decompiled functions, names/addresses like `0x140001000.c` (image base `0x140000000`).
- `decompile-index.json` (~9 MB) -- metadata per function: address, name, status, relative path, sha256, body ranges.
- `decompile-journal.jsonl` (~8 MB) -- export journal.
- `target_sha256`: `2dc38eba3733936d466076b45513819c99d6541a1ca5ce36c84ff8a242bcb892`.
- Function bodies mention `haloreach` (e.g. `0x140026010.c`), so this is a build that loads/drives haloreach.dll.

### `C:\Users\tyler\Desktop\host-probe.exe\`
- Same layout: `functions\*.c` (5,825 functions), `decompile-index.json` (~20 MB), `decompile-journal.jsonl`.
- `target_sha256`: `38e0fad3b75026f36b909a5f5b369ad149484d80a2d09a36d1adf3cc3d9413b3`.
- Function bodies mention `haloreach` / `HaloReach` paths (e.g. `0x140199210.c`, `0x140294c80.c`).

**How to use**: `rg -n "pattern" "C:\Users\tyler\Desktop\Reach.exe\functions"` (or PowerShell `Select-String`).
Grep the pseudocode for constants/strings to locate logic, then use the index to get the exact address, then confirm in
live Ghidra MCP. These exports are *pseudocode*, not source and not the same build as `haloreach.dll`; treat them as a
fast index. **Identify the target first** by matching the `target_sha256` against a candidate binary if identity matters.

---

## 2b. FULL decompiles of haloreach.dll + Xbox 360 Reach (made 2026-11-15) -- the ones to use for haloreach.dll

Unlike the Desktop exports above (Noble's own exes), these decompile the game engine itself. Index, scripts and
function lists: `Compendium\decompile-exports\README.md`. Raw text on Tyler's PC:
- **MCC haloreach.dll** (same build Noble loads, SHA-256 738dd2d2...0894): `D:\MCC HALO REACH DECOMP\decompiled\hr_*.c`,
  27,578 functions, names FUN_*, `// ==== FUN_1802d288c  rva 0x2d288c ====` headers (RVA = address - 0x180000000).
- **Xbox 360 retail Reach, with real function names** (from the leaked Omaha linker map): `D:\ReachX360\ghidra_retail\decompiled_v3\hr_*.c` (**v3 only**),
  33,787 functions, 99.8% named (e.g. `cui_property_editor_get`, `c_number_formatter_component::format_number`).
- Xbox 360 dev-kit beta (FUN_* names): `D:\ReachX360\ghidra\decompiled\`.
- **Other MCC engines** (same MCC build as Noble, all finished): `D:\MCC_DECOMP\{halo1,halo2,halo3,halo3odst,halo4}\decompiled\`
  - Halo CE **79,637 fns, finished 2026-11-15 10:19 ET, 2 failed**; Halo 2 39,362; Halo 3 20,444; ODST 21,484; Halo 4
  36,876. x64, FUN_* names. Use for cross-game comparison (T8 in the DeepSeek briefing); counts/indexes:
  `Compendium\decompile-exports\README.md`.
Use the 360 names to understand MCC `FUN_*` functions (same engine, shared structure/strings). Confirmed matches so far are
in `project-docs\NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md`. First result: the "0000001c" / "Respawn in 00000001" UI bug is the
`\a<8 hex>` expander `hr+0x2D288C` leaking its digits - static theory ONLY; a solo MP/campaign read-only runtime
check never saw it fire, see `project-docs\NOTE-HEX-BUG-RUNTIME-CHECK-2026-11-15.md` (exact repro still unknown).

---

## 3. Repo `docs\` -- prior engine/limits/multiplayer work (READ BEFORE ITEM 13)

`docs\` (tracked in git) is the previous agents' engineering log. Multiplayer/C12 work Tyler called out:

| File | What it holds |
|---|---|
| `docs\limits-players.md` | The 128-player research note: what "16 players" is (player data arrays, session membership, masks, spawning, HUD/scoreboard/killfeed), milestones. Start here for item 13. |
| `docs\limits-players-audit.md` | Audit of fixed-16 indexing in the players pool; 16-slot mask in session membership object; slot-rewrite inventory and 32-slot patch design. |
| `docs\limits-players-loops.md` | 32-player loops/bounds/tables; plan C implemented (built, not tested); pools + host player list; the long list of known open sites. |
| `docs\limits-players-message.md` | Membership update message (type `0x1A`) design for 32 players, RVAs, high-slot test (`NOBLE_SLOT_HIGH=1`). |
| `docs\limits-players-regbounds.md` | Register-held loop bounds (the guest stuck at peer state 4), batch-1 `kPeerMasks32` note. |
| `docs\limits-players-review.md`, `limits-players-bounds*.json`, `limits-players-sites.json`, `limits-players-trace.txt`, `limits-players-rest.json` | Supporting review/trace/site data (large JSON/trace). |
| `docs\limits-ai.md` | AI fixed-table audit (`0x690000`-`0x780000`), script-node pool recipe -- relevant to Firefight spawn multipliers. |
| `docs\limits-render.md` | Render list 4,096 -> 16,384 research. |
| `docs\collision-walk-crash.md` | Collision-walk crash notes. |
| `docs\mod-map-mount-findings.md` | How mod maps mount through stock slots. |
| `docs\HANDOFF-CONTENT-2026-11-09.md` | Older content handoff (megapack/Forge assets). |
| `docs\pelican.md`, `docs\megapack.md`, `docs\swarm-test-notes.md`, `docs\OVERNIGHT-2026-11-09.md`, `docs\RR-mainmenu-decompiled.txt`, `docs\engine.rar` | Asset/tool notes; `RR-mainmenu-decompiled.txt` is decompiled main-menu script text (never ship RR names). |

Also in `docs\`: `limits-players-*` is the 128-player chain of custody. Code side: `src\host\SlotPatch.*`,
`src\host\SlotMessage.*`, `src\host\PoolResize.cpp`, `src\hooks\LobbyPatches.cpp`
(`s_maxMultiplayerPlayers = 128`), `tools\limits\*` (generators: `gen_caves.py`, `gen_sites.py`, `gen_message.py`,
`gen_steps.py`, `slot_rewrite.py`, `test-limits.ps1`, `recipes\`). The C12-L 32-slot patches are present but
**built, not tested** per those docs; multiplayer is the LAST item on Tyler's current list.

`knowledge\repo-docs\` mirrors most of `docs\` (stale copies from 2026-11-14). Prefer the tracked `docs\` files.

---

## 4. Repo `knowledge\` vs Desktop `Compendium` (docs mirror policy)

`knowledge\` (repo, git-excluded) is the live documentation tree:

- `00-START-HERE.md`, `01-PROJECT-HISTORY.md` (phase list), `02-LESSONS-AND-RULES.md`, `AGENT-BRIEF.md`,
  `AGENT-COORDINATION.md` (recent results), `PROJECT-NOBLE-COMPENDIUM.md`.
- `project-docs\` -- dated handoffs (HANDOFF-NEXT-AGENT-2026-11-14.md is the current one) plus focused findings
  (armory, menu map, elite pack, spartan pieces, mod bugs...).
- `desktop-guides\` -- TODO-MASTER-2026-11-14.md (live queue), `Noble-AI-Limits-Report-2026-11-08.md`,
  `halopedia_armor_pipeline\`, `hex_editor_guide_v16\`.
- `repo-docs\` -- copies of repo `docs\`.
- `pc-handoffs\` -- elite/wing handoffs used by the armory work.
- `drafts\` -- research drafts; `draft-session-networking.md` (11 KB) explains the two player tables, masks/bounds,
  transport framing, lobby handshake, join-in-progress, slot counts, dedicated server, master cluster -- read it before
  touching netcode. `draft-tools-resources.md`, `draft-forge-packs.md`, `draft-memory.md`, `draft-model-animation.md`
  are also useful.
- `test-tools\` -- 100+ helper scripts (tag/anim/bitmap tooling, `armory_preview.ps1`, etc.).

`C:\Users\tyler\Desktop\Compendium\` is a **hand-off mirror** of `knowledge\` (same subfolder layout, plus the top-level
START-HERE/HISTORY/LESSONS). It was stale on 2026-11-14 (did not contain the 11-15 fixes), so on 2026-11-15 05:15 the
following files were re-synced byte-identically: `00-START-HERE.md`, `01-PROJECT-HISTORY.md`, `AGENT-COORDINATION.md`,
`project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md`, `project-docs\REFERENCE-LIBRARY-2026-11-15.md`,
`desktop-guides\TODO-MASTER-2026-11-14.md`. **Policy**: edit `knowledge\` in the repo, then copy the changed files into
the matching Compendium path (verify with `Get-FileHash`) so every future agent/Tyler sees the same text. Do not edit
only the mirror.

---

## 5. Source-of-truth notes from this session (engine facts worth keeping)

### 5a. Visor mapping and the five-slot engine cap (corrected 2026-11-15; commits `8f467e1` + `96fd2c4`)
- MCC's 30 visor items are contiguous: `HR_Visor_*` item id = `157 + slot` (`0x9D + slot`), in the order now stored in
  `src\common\ArmoryVisor.hpp` (`default, silver, blue, black, gold, rose_gold, emerald, ice, crimson, onyx, violet,
  amber, ...`). The Armory UI catalog lists `violet` before `ice/crimson/onyx`, which is why index-based mapping showed
  the wrong finish in game.
- `haloreach!0x3B2F4` (Ghidra `0x18003B2F4`) is the pmcg item -> internal-slot translator. For visor items its stock
  cases are: item 157->0, 158->1, 159->2, 160->4, 161->3 (i.e. **engine internal 3 = Gold, 4 = Black**; MCC item order
  is Default/Silver/Blue/Black/Gold).
- `haloreach!0x2EDC5E` is patched by `EnginePatches.cpp` (7 bytes, `lea edx,[rcx-0x9D]`) to bypass that translator for
  visors: engine internal slot = `itemId - 157`. This enables all 30 finishes.
- `FUN_180039128` around `0x1800393CC` maps the profile settings visor enum to item ids: 0->157, 1->158, 2->159,
  3->161, 4->160 (confirms the Black/Gold swap). ReachHost's profile write uses `kVisor = 157` plus the resolved slot.
- Result: the profile path and the appearance-record path (`ElitePack`) must both use `ArmoryVisorEngineIndex`.
  ReachHost now logs `visor slot N` with the applied appearance.
- **Engine cap found 2026-11-15**: only five real visor slots exist (engine 0..4 = Default/Silver/Blue/Gold/Black; MCC
  items 157..161). The "30 finishes" patch (direct item-157) stores invalid slots 7..29 for the extended Noble finishes,
  which the renderer draws **white** (Tyler's blood-orange report). `ArmoryVisorStockIndex()` now maps every finish to a
  valid stock slot (nearest colour) for the profile + appearance record; exact custom tints only come from the rmsh
  shader-constant patch, and that scan matches materials in the Armory's menu map but **0 materials in campaign and
  multiplayer maps** (Spartan materials sit in the shared tag heap, not the map-local tag table). Extended custom visor
  colours are therefore still Armory-only in game; fixing that needs a shared-heap material scan (map-wide tint) or
  extending the engine visor colour table/lookup (per-player correct).

### 5b. Armory/profile appearance plumbing
- `haloreach!0x2EDA08` copies profile fields into the player's settings; profile item ids in `ReachHost.cpp`
  `namespace armory_ids` (`kVisor=157`, `kColor=202`, ...). Appearance record: `player+0xD8` (0x28 bytes),
  builder `0x1055EC`, record-set `0x2C837C`, apply-profile `0x2EDA08` (all documented in `ElitePack.cpp`).
- Extended visor finishes (UI index >= 5) are forced onto the **local** biped's visor material from
  `armory_colors.json` in `ReachArmory::SyncMatchLiveBipeds`; remote players see the engine slot colour. Replicating the
  exact RGB to peers is still future work.

### 5c. FOV and settings
- `CameraFov` writes `haloreach+0xB42C10`; `Apply()` now refuses to write while `ReachArmory::IsMenuMode()` (the
  front-end/Armory share the engine camera, and applying the match FOV in the menu zoomed the Armory model -- `3a3c98e`).
  `CameraFov::Tick()` applies the saved value when a match starts. Test hook: `-fov-apply-test-at <sec> <deg>` logs the
  front-end apply attempt.
- Settings rows are drawn by `SettingsPanel::Row/SliderValue`; the value column was made fixed width (132 * UI scale)
  and labels are ellipsized at the slider column (`ec585f0`).

### 5d. Front-end pickers
- `FrontEndPickers.cpp` `BuildMapTileGroups` + `DrawTacticalMapGrid` own the map/Forge/mod pickers for Campaign,
  Custom Game, Firefight, Forge and the lobby map select. Collapsed mod sections are `MapTileGroup.sectionMapCount`
  (click/Accept toggles `MenuState::openMods` via `OpenKey(mode, mod)`); the drawer header uses ASCII `-` because the
  game font has no em-dash glyph. Search: `-menu-screen campaign|custom|firefight|forge -menu-tab N`.

---

## 6. Test harness and hooks (use these instead of driving the UI by hand)

Harness lives in `build\bin\Release\data\userdata\claude-tests-2026-11-11\tools\`:
`launch_noble.ps1` (launches on the secondary monitor with `-NoFocus`), `camp_test.ps1`, `match_elite.ps1`,
`forge_test.ps1`, `runmb.ps1`. Captures are 1920x1080 BMPs written to `build\bin\Release\capture_*.bmp`; converted
copies/collections live in `build\bin\Release\data\userdata\claude-tests-2026-11-11\`.

Menu/UI hooks: `-menu-screen <name>`, `-menu-tab <n>`, `-menu-ignore-input`, `-menu-capture`, `-capture-at <secs>`,
`-quit-after <secs>`, `-menu-click-at <sec> <x> <y>` (repeatable; injects a real ImGui click unfocused -- added for the
click-bug items), `-menu-debug-layout` (logs row/layout rects), `-armory-focus <slot>`, `-armory-cycle <part>`,
`-server-browser-demo`, `-server-browser-style 1|2|3`, `-lobby-demo*`, `-fov-apply-test-at`, `-loading-roster`,
`-wait-peers`, `-max-players`, `-capture-frames`, `-connect`, `-host`, `-lan`, `-private`.

Rules Tyler has repeated: test on the **AOC (DISPLAY1, left, X=-1920)**, never his Samsung; never take his mouse/focus
(`-NoFocus`); never test while he plays. Kill only processes you launched; `noble-dev.exe` with no arguments is his.

---

## 7. Game data and asset tools (external, only when needed)

| Path | Contents / use |
|---|---|
| `C:\Users\tyler\Desktop\ex\` | Extracted Reach tags: `ui\` (cui/chud art used for the loading screen: `ex\ui\cui\common\img_auntiedot_background.png`), `objects\`, `globals\`, `levels\`, `fx\`, `shaders\`, `rasterizer\`, `test\`. Use for menu/UI art and tag inspection. |
| `C:\Users\tyler\Desktop\so many tags\` | Custom tag sets: `forge_halo_global`, `[HR] Boathog`, `LMG TAG`, `targetting laser tags`, H2A/HW marine & scorpion mix, `working flashlight`. Source material for mod maps/assets (never commit game assets). |
| `C:\Users\tyler\Desktop\spectre\` | `[HR] H2A T-46 Spectre` pack + `spectreblendreach`. |
| `C:\Users\tyler\Desktop\Pelican\` | Pelican asset (`data\`, `tags\`). |
| `C:\Users\tyler\Desktop\assembly\` | Assembly tag editor (`Assembly.exe` + `Plugins\` for Halo1/Halo2/Halo3/ODST/Reach/MCC). Use to open/inspect/view tag definitions (`.map`/tag files) when a tag question comes up. |
| `C:\Users\tyler\Desktop\Forgeworks-Halo-3-main\` | Halo 3 Forge/Mod Tools source (`src\`, `modules\`, `tags\objects`, `docs\`, `reports\`, `publish\`). Reference for Forge object/palette work (Halo 3 API, not Reach-native). |
| `C:\Users\tyler\Desktop\reach varient tool and editor\` | Qt-based Reach variant tool (game variants / map variant editing); Themes/styles/translations dirs, `help\`. Use for `.game_variants`/`.map_variants` workflows. |
| `C:\Users\tyler\Desktop\Reach game modes and maps\` | `game_variants\`, `map_variants\` collections + `INSTRUCTIONS.txt` + `SEMI-COMPLETE CUSTOM GAMES LIST.txt`. Source of shipped custom game/map presets (see `data\game_variants\` in the repo). |
| `C:\Users\tyler\Desktop\MVAR\` | `mvar_tool.cpp` (13 KB) + `mvar_oracle\` -- map-variant (`.mvar`) editing/inspection tool. Useful whenever variant parsing/re-serialisation comes up. |
| `C:\Users\tyler\Desktop\Noble-AI-Limits-Report-2026-11-08.md` | 40 KB AI-limits report (same file is in `knowledge\desktop-guides\`). Read before Firefight AI spawn-count work (item 13's AI multiplier). |
| `C:\Users\tyler\Desktop\Halo Reach (Jul 24 2010) (Omaha) (Cache Build)\` | **Misleading name**: contents are Halo 3 Omaha cache XEX builds (`halo3_cache_*.xex`, `.map`, debug dumps, `maps\20_sword_slayer.map` etc.) plus engine debug/cheats dumps. Halo 3 reference only (map/format/debug-menu archaeology), not Reach. |
| `C:\Users\tyler\Desktop\menumaps\` | Empty at the time of writing. Do not rely on it. |
| `C:\Users\tyler\Desktop\ReachRenaissance\` | Not in Tyler's reference list; noticed on the Desktop (modified 2026-11-14). Ask before assuming its role. |

Extraction reminders: Reach loading art was pulled with ManagedBlam (`mbstart.py` + `Bungie.Tags.GameBitmap`) -- recipe
in `HANDOFF-NEXT-AGENT-2026-11-14.md`. Map photos: MCC previews are 316x160/668x292; campaign missions have real
1920x1080 stills in `loadingtexturepack.perm.bin` matched by CRC-32 (`UiArt.cpp` `kCampaignScenes`). ffmpeg is available
through the bundled Python `imageio_ffmpeg.get_ffmpeg_exe()`.

---

## 8. Where the current answers live (documents to read in order)

1. `knowledge\00-START-HERE.md` -- orientation.
2. `knowledge\project-docs\HANDOFF-NEXT-AGENT-2026-11-14.md` -- current handoff: state, this session's fixes, next queue.
3. `knowledge\desktop-guides\TODO-MASTER-2026-11-14.md` -- the ordered queue and progress.
4. `knowledge\01-PROJECT-HISTORY.md` -- phase history (what was built when).
5. `knowledge\02-LESSONS-AND-RULES.md` -- hard-won rules (build/commit/test conventions).
6. This file -- reference paths above.
7. `knowledge\AGENT-COORDINATION.md` -- latest cross-agent results.