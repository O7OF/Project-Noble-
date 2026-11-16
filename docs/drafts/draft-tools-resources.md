# Tools — reference appendix (draft, round 3)

Written 2026-11-14 by cc. Input to `PROJECT-NOBLE-COMPENDIUM.md` (chapter 6 "Tooling and runbook"); lead
merges and owns the final wording. Sources: `discord-compendium\10_tools-guides-links.md`,
`07_ripping-extraction-tools.md`, `09_scripting-ai-pathfinding.md`, `00_ULTIMATE_GUIDE.md` (Parts B/C/N),
`00_INDEX.md`, plus the repo's own tooling facts (`02-LESSONS-AND-RULES.md`, `AGENT-BRIEF.md`, compendium
chapters 4–5). Installed paths verified 2026-11-14 by lead.

Confidence legend (same as the compendium): **proven** = one-variable test + capture/log; **observed** = seen
live but not isolated; **community** = from the Discord compendium / community guides; **unverified** = inferred
from code/docs, not runtime-checked. Rule of the house: a tool claim without a source is a hypothesis.

---

## 1. Official references and talks (behaviour reference only)

Not installed; consulted for engine behaviour, tag semantics and netcode. Noble ships no content from these.

| Reference | What it is | When Noble uses it | Limits | Conf. |
|---|---|---|---|---|
| **"Halo: Reach Effects Tech"** — Chris Tchou, Bungie · gdcvault.com/play/1014347/HALO-REACH-Effects | Bungie's own talk on the Reach effects/render pipeline | behaviour reference for the render/effects code (same role as Reach Renaissance: behaviour only, no data) | no code/data to copy; high-level talk | community |
| **"I Shot You First: Gameplay Networking in Halo: Reach"** — David Aldridge (Bungie lead networking engineer), GDC 2011 | primary source for Reach host/client netcode | basis of compendium Part N (session/networking behaviour) | read via slide transcriptions/summaries; original slide file not read | community |
| **Microsoft Learn MCC docs** · learn.microsoft.com/halo-master-chief-collection | official H3/HR mod-tool docs: FBX→GR2 page (`hr/art/fbxtogr2`), shader-option index (`h3/shaders/shadershome`) | source of the HREK FBX→GR2 pipeline + shader semantics in Part C | HR-specific shader pages not found; H3 notes only a guide for Reach | community |
| **c20.reclaimers.net** (Reclaimers wiki) · Discord `discord.reclaimers.net` | community tag reference: JSON parameter catalogue, shader guides (compiling-shaders, pbr-shader), level-creation hub | JSON parameter catalogue and shader-guide references in Part C | community-authored; page typos noted; some pages (animation-data) failed to fetch | community |

---

## 2. Native editing kits (installed where noted)

### 2.1 HREK — Reach Editing Kit (Steam app **1695793**) — `D:\Steam\steamapps\common\HREK` — *installed*

The official Reach mod tools. Contains **all retail tags** (community). Ships `tool.exe`, `tool_fast.exe`,
`Foundation.exe`, `Sapien.exe`, `MegaloEdit.exe` (observed). HREK edits get backups under `HREK\noble_mods\<name>\`.

| Tool | Role | Noble use | Limits |
|---|---|---|---|
| **`tool.exe` / `tool_fast.exe`** | cache/map builder + tag import. `tool` and `tool_fast` behave the same; `tool_fast` is multithreaded with fewer error messages (community) | `tool.exe build-cache-file` via `build_map.ps1 -Scenario <path>` rebuilds Forge World / pack maps — **proven** (Covenant airstrike, ElitePack) | ~2–4 min per map build; split resource tags read over scenario blocks; bad ids silently strip scripts |
| **`Foundation.exe`** | Reach tag editor (shader/render-model/bitmap params) | reading tag/shader values; part of the tag chain (`hlmt`→`mode`→regions) dumps | cannot save shader changes in some versions (use Guerilla then); "export mesh" gives unusable `.x`; render data not retained for re-import (community) |
| **`Sapien.exe`** | scenario/world editor (place objects, spawns) | not documented as used by Noble; opens scenarios when needed | most documented behaviour is the H3/ODST kit; Reach usage thin in the corpus; may crash on assert after some MCC patches (community) |
| **`MegaloEdit.exe`** | Reach Megalo (multiplayer game-mode scripting) editor | not documented as used by Noble | Megalo scripting not covered in the compendium corpus (not captured) |
| **`tool_fast fbx-to-gr2 <fbx> <json> <gr2>` then `tool_fast import <x.sidecar.xml>`** | FBX→Granny→tag pipeline (`.gr2` replaces ASS/JMS) | **not yet documented as used by Noble** — Noble's Pelican/Spectre/Boathog came from mods, not FBX import | no resource sharing; no single-threaded lightmapping; JSON auto-created once and never auto-updated |

### 2.2 Guerilla (H3/ODST/H2 tag editor) — **not installed** (not in the Reach kit)

Guerilla is the H3/ODST/H2 equivalent of Foundation (Steam apps H3 1695791, ODST 1695794, H2 1613450 — community).
It is the shader/tag parameter viewer for those games, not Reach. When the compendium says "check in Guerilla/
Foundation", Reach work means Foundation. *community.*

---

## 3. Tag / map editors

| Tool | What it is | Noble use | Installed? | Conf. |
|---|---|---|---|---|
| **Assembly** (Lord-Zedd) | edits live MCC `.map`/`.mapinfo` cache data in place; reads tag/shader values; **no model extraction** | reading shader/tag data and hex-patch guidance (the repo's `hex_editor_guide_v16` is a haloreach.dll patch guide, complementary to Assembly's cache edits) | yes — `C:\Users\tyler\Desktop\assembly\Assembly.exe` | community (tool), observed (path) |
| **TagTool** (dev branch) | modifies tags inside built caches (Reach/H3/ODST) | not documented as used by Noble | unverified | community |
| **ManagedBlam** | Python library to read/write Blam tags, driven through Blender's Python | Noble's scenario/palette editing: `runmb.ps1 -Script scn_*.py -Out file -Env 'K=V;…'` — `scn_add_forge.py`, `scn_add_covenant.py`, `scn_palettes.py`, `scn_mvp.py`, `tagwalk.py` — **observed/proven** (Forge palettes, ElitePack) | `DuplicateElement` not `AddElement` for new variants; never return a `TagFieldBlock` from PowerShell; UTF-8 BOMs break parsers; H3 ManagedBlam cannot load in Blender Python (community) |

---

## 4. Extraction and viewing

| Tool | What it is | Noble use | Installed? | Conf. |
|---|---|---|---|---|
| **Reclaimer 2.1.1645** (Gravemind2401) | Halo file explorer: opens `.map`, exports render models / bitmaps / sounds, `Export Metadata` JSON | reading **compressed MCC maps** via `dotnet fsi rc_*.fsx` (stock MCC string-id lookups find 0 names); dumping `hlmt`/`mode`/region/permutation data for the Trooper-leg case | yes — `D:\RECLAIMER\Reclaimer-2.1.1645` | observed (use), community (tool) |
| **Bonobo** (LaikaGlove) | jmad→JMA/JMM animation extractor for H2/H3/ODST/Reach | animation reference; Noble's jmad work (Pelican install) used HREK files, not Bonobo | no | community |
| **Baboon** (Zoephie) | blam-tags GUI, byte-exact tag edit/extraction for H3/ODST/Reach | not documented as used by Noble | no | community |
| **Foundry** (Crisp) | Blender add-on to import Reach tags (render/collision/physics) and build shaders | not documented as used by Noble; the community's Reach Blender-import route | no (Blender add-on, not installed) | community |
| **Architect** (Gravemind2401) | Reclaimer companion (H3 spartan builder) | not used by Noble | no | community |

Reclaimer notes worth keeping: `campaign.map` / `multiplayer.map` / `shared.map` cannot be *targets*, but `mainmenu.map`,
`shared.map`, `campaign.map` must sit beside the target map; set the Materials root path to the bitmap export root or
models show yellow. AMF is outdated on current Blender — use RMF (community).

---

## 5. Blender / DCC pipeline (community)

| Tool | Role | Noble use | Conf. |
|---|---|---|---|
| **Blender** | hosts ManagedBlam (Noble's tag scripts run through Blender Python); DCC for models/rigs | ManagedBlam host via `runmb.ps1`; Armory pmcg/menu work is tag-side | installed — launcher in `%LOCALAPPDATA%\Microsoft\WindowsApps` (observed) |
| **Foundry** (also §4) | Reach tag import → Blender | not used by Noble | community |
| **General-101 Halo Asset Blender Development Toolset** | H3/ODST JMS/JMA/ASS import/export | not used by Noble | community |
| **CR4B Tool** (PlasteredCrab) | one-click H3/ODST material builder; Reach "minimal" | not used by Noble | community |
| **BMAX** | Blender↔3ds Max connector | not used by Noble | community |

---

## 6. Reverse engineering

| Tool | Role | Noble use | Installed? | Conf. |
|---|---|---|---|---|
| **Ghidra 12.x** | disassembler/decompiler for `haloreach.dll` | Noble's engine RE: `disasm.ps1 -Start <rva> -Stop <rva>`, `find_callers.ps1`, RVA/signature lookup — the RVA tables in `02-LESSONS-AND-RULES.md` come from this | yes — `C:\Tools\ghidra_12.1.4_PUBLIC` and `C:\Tools\ghidra_12.2_PUBLIC` | observed (use), observed (path) |

---

## 7. Community / Discord

| Server / resource | What it is | Where its content lives here |
|---|---|---|
| **The Halo Archive** (discord.gg/haloarchive) | primary community: ripping, shaders, rigging, Blender packs | `discord-compendium\01`–`11` (12,581 messages distilled) |
| **Halo Mods** | mod-dev community; `#materials` shader digest | `DISCORD-HALOMODS-MATERIALS-SHADER-DIGEST` (209 messages, 2022-04→2026-09) |
| **Reclaimers** (discord.reclaimers.net) | c20 wiki community | referenced in Part C/D |

Trusted authors (community): `gravemind2401` (Reclaimer/Architect, formats/versions), `iloveagoodcris`/Crisp
(Foundry, Reach import), `chiefster_4014` (Blender packs + mod tools), `soulburnin` (Reach/H3 Blender shader),
`r93_sniper` (control-map guide), `zoephie` (Baboon), `plastered_crab` (CR4B), `general-101` (toolset).

---

## 8. Installed-here summary (verified 2026-11-14)

| Tool | Path |
|---|---|
| Assembly | `C:\Users\tyler\Desktop\assembly\Assembly.exe` |
| HREK (Foundation, Sapien, MegaloEdit, tool.exe, tool_fast.exe) | `D:\Steam\steamapps\common\HREK` |
| Reclaimer 2.1.1645 | `D:\RECLAIMER\Reclaimer-2.1.1645` |
| Blender | launcher in `%LOCALAPPDATA%\Microsoft\WindowsApps` |
| Ghidra | `C:\Tools\ghidra_12.1.4_PUBLIC` and `C:\Tools\ghidra_12.2_PUBLIC` |
| ManagedBlam | with HREK, driven through Blender Python |

Not installed (community-only, referenced): Guerilla (H3/ODST/H2), TagTool, Bonobo, Baboon, Foundry, General-101
toolset, CR4B, BMAX, Architect.

---

## 9. Open questions / gaps

- **Megalo**: MegaloEdit is installed but no Megalo scripting knowledge is captured in the corpus (Part D3 "not
  captured"). Noble's own Megalo use is undocumented. *unverified.*
- **FBX→GR2 import into Reach**: the HREK `tool_fast fbx-to-gr2` / `import` pipeline is documented in the compendium
  but Noble has not run it; no source documents custom-armour import beyond that pipeline. *unverified for Noble.*
- **Bonobo vs MCC Reach**: why Bonobo rejects current MCC Reach maps, and Reach first-person animation extraction,
  are unresolved in the community (Part B11). *community/unresolved.*
- **Foundry xrgb vs sRGB** bitmap handling and empty-scene imports are unresolved in the community. *community/
  unresolved.*
- **Guerilla vs Foundation shader code**: whether Guerilla shows shader code or only parameter pages is disputed in
  the corpus (fable_law_458 vs gregory0701). *community/disputed.*
- Installed Reclaimer is **2.1.1645** (newer than the 2.1.1624 the compendium documents); its exact delta from
  2.1.1624 is not recorded here. *unverified.*

---

*Sources for this draft: `00_ULTIMATE_GUIDE.md` B6/B10/B11, C1–C8, D3, N1; `07_ripping-extraction-tools.md`;
`09_scripting-ai-pathfinding.md`; `10_tools-guides-links.md`; round-3 assignment (installed paths + GDC Effects
Tech link). Noble-usage claims are from `02-LESSONS-AND-RULES.md`, `AGENT-BRIEF.md` and compendium chapters 4–6.*
