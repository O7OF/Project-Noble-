# What the Halo Reach Editing Kit actually ships (and what it unlocks)

Investigated 2026-10-30. `D:\Steam\steamapps\common\HREK` — the official **Halo Reach Editing Kit** (HREK), build
`2023.07.17.176677.1-QFE1`, project "Bulgogi" / display name "Omaha".

## The toolchain

| File | What it is |
|---|---|
| `Foundation.exe` | **"HR Foundation Tag Editor" v1.903** — Bungie's Bonobo tag framework (the thing the other tools sit on) |
| `bin\tools\bonobo\` | the framework: `BonoboInterfaces`, `Bungie.Blam`, `Bungie.Schema.Blam`, `TagService.exe`, `TagWatcher.exe`, `FoundationConsole.exe`, plugin folders (BlamPlugin, AssetPlugin, GuiPlugin, LibrarianPlugin, NormalPlugin, world) |
| `sapien.exe` | the **scenario/level editor** (place objects, lights, cinematics) |
| `tool.exe` / `tool_fast.exe` | the command-line build tool (compile tags, build maps, extract bitmaps) |
| `MegaloEdit.exe` | Megalo game-variant editor |
| `ProjectChooser.exe` | picks the project (`project.xml` -> tag datastore `bin\tools\tagDb.sdf`) |
| `reach_tag_play.exe` / `reach_tag_test.exe` | tag test harnesses |
| `HREK.7z` (6.5 GB) | **the content**: 63,569 entries, extracted by `HREK (Extract).bat` |

## The content (inside `HREK.7z`)

63,569 files, full tag tree — not a sample:

| Folder | tags |
|---|---:|
| `tags\objects` | 17,172 |
| `tags\levels` | 16,507 |
| `tags\sound` | 12,151 |
| `tags\cinematics` | 7,602 |
| `tags\shaders` | 4,285 |
| `tags\fx` | 1,586 |
| `tags\ui` | 1,318 |
| `tags\globals` | 263 |
| `tags\multiplayer` | 322 |
| `tags\firefight` | 196 |

Plus `data\` (HaloScript sources and Megalo sources, e.g. `data\multiplayer\megalo\simple\slayer`, `...\ctf`), `maps\info`,
`maps\fonts`.

## The three things this unlocks for Noble

### 1. The Armory's black chamber — the real fix is now possible

`tags\levels\ui\mainmenu\` holds 148 tags (27.6 MB), including exactly what was missing:

- `mainmenu.scenario` (0.43 MB)
- `mainmenu_000.scenario_structure_bsp` (0.24 MB)
- **`mainmenu_000.scenario_structure_lighting_info`** — the lighting for the chamber the armory stands in
- `mainmenu_lobbies.scenario_structure_bsp`, `mainmenu.structure_seams`
- `bitmaps\armory_cubemap.bitmap`, `armory_lobby_cubemap.bitmap`
- `post_fx\menu.area_screen_effect`

So the "light or replace the chamber in `mainmenu_all.map`" route is no longer blocked: open the scenario in **Sapien**
(or tweak the lighting_info in Foundation), then rebuild the menu map with `tool.exe`.

### 2. Emblems — the original background/foreground library is in the tags

- `tags\globals\emblems\global.emblem_library` (7.2 KB, 57,396 bytes on disk) — its structure strings are visible in the
  file: `emblem bitmaps`, `emblem bitmaps hi rez`, `emblem_bitmap_list` block with `name`, `string id`, `bitmap index`,
  and `shapes` with `bitmap`, `address mode x/y`, `transform` (`scale`), `gradient size`, `terminator X`. That is Reach's
  own **shape (background) + symbol (foreground) library**, not MCC's precomposed pairs.
- `tags\ui\cui\common\emblems\emblems.bitmap` (1.9 MB atlas) and `tags\ui\cui\common\emblems.bitmap` (3.1 MB).
- UI pieces: `supply_depot_emblem.cui_screen`, `emblem_selection*.bitmap`, `emblem_shader_*.bitmap`.

Extracted to `C:\Users\tyler\Documents\deepseek-harness\default-workspace\hrek-extract\` for inspection. Next step is
opening `global.emblem_library` in Foundation/Guerrilla (or parsing it with the tag layout) to enumerate the shapes and
symbols, then extracting the atlas with `tool bitmap`.

### 3. Forge / Megalo

`MegaloEdit.exe` plus `data\multiplayer\megalo\` sources (slayer, ctf, koth, juggernaut, ...) mean the Forge variant
`00_basic_editing_054.bin` can be opened as a Megalo variant, which is the cleanest way to see whether Forge carries a
session/editing flag that explains the netplay behaviour in `docs/FORGE-NETPLAY-OFFSETS.md`.

## Practical notes

- The tags are **inside `HREK.7z`**; run `"HREK (Extract).bat"` once to unpack them (~6.5 GB compressed, expect a few
  times that on disk). `bin\x64\7zr.exe` can extract selectively:
  `7zr.exe x HREK.7z -o<dir> -y "tags\levels\ui\mainmenu\*"`.
- Sapien and tool need the project selected (`ProjectChooser.exe`) and the tag datastore service (`TagService.exe` /
  `FoundationConsole.exe`) running; `README_MODDING.txt` says the installer deliberately does not extract to avoid
  overwriting user content.
- MCC's own install stays read-only: everything here is a **source** tree. Rebuilt maps land in HREK's own `maps` folder
  and can then be copied into Noble's `data\menumaps` as a mod, exactly like the current `mainmenu_all.map`.
