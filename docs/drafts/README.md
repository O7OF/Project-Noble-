# cc drafts - scope and format

`cc` (Claude Code on DeepSeek, launched by Tyler or spawned by lead through
`tools\claude-deepseek-run.ps1`) writes draft chapters here. `lead` reviews, verifies and merges them into the
single-file compendium. Nothing here is committed: `knowledge\` is git-excluded.

## Rules for cc

- Read `knowledge\00-START-HERE.md` and `knowledge\02-LESSONS-AND-RULES.md` first, then `AGENT-COORDINATION.md`.
- Write only inside `knowledge\drafts\`. Never edit `src\`, `tools\`, `data\`, `build\` or the three master docs.
- Every factual claim carries provenance: source doc/session, engine RVA if relevant, and a confidence level
  (`proven` / `observed` / `community` / `unverified`).
- Prefer short, dense sections over prose. Tables for structures, offsets and RVA lists.
- Do not copy long passages from the guides; distil and link the source file.
- Mark open questions explicitly instead of guessing.

## Current assignment (2026-11-14, round 2)

3. `draft-model-animation.md` - bipeds and player representations (`matg +0x474`: count / pointer, stride 0x6C,
   biped at +0x40), the model/regions/permutations chain (`hlmt` -> `mode` -> regions), how Noble applies armour
   (Armory pmcg pairs, `MarineArmor`, `ElitePack`), Elite/Marine/Spartan specifics, animation graphs (`jmad`), and
   the open campaign-Trooper leg case (model folds at the hips while the unit shadow stands). Sources:
   `src\engine\ReachArmory.cpp`, `src\engine\TrooperMode.cpp`, `src\engine\ElitePack.cpp`, `discord-compendium\`
   (shaders/models/rigging, Epic fields), `01-PROJECT-HISTORY.md`, the 11-14 night handoff.
4. `draft-forge-packs.md` - the Forge and pack pipeline: Noble's own palette (`data\forge_palette.json`), the
   native in-map scenario palettes (`ff_weapons_human`, `ff_weapons_covenant`, ...), the ManagedBlam scripts
   (`scn_add_forge.py`, `scn_palettes.py`, `scn_mvp.py`), HREK `build_map.ps1` and VFS map serving, ElitePack
   build and shrink, Pelican/Spectre/Boathog tags, and the Option 2 plan (add-on pack vs 750 MB map). Sources:
   `repo-docs\pelican.md`, `repo-docs\megapack.md`, `project-docs\HANDOFF-ELITE-PACK-2026-11-12.md`,
   `src\engine\ElitePack.cpp`, `src\ui\ForgeMenu.cpp`, today's `scn_*` scripts.

## Current assignment (2026-11-14, round 3)

5. `draft-tools-resources.md` - the tools and reference appendix: GDC talks (Halo: Reach Effects Tech, Chris Tchou,
   Bungie, https://www.gdcvault.com/play/1014347/HALO-REACH-Effects), Microsoft Learn MCC docs
   (learn.microsoft.com/halo-master-chief-collection), c20.reclaimers.net, Assembly, Guerilla/Sapien/Foundation/
   MegaloEdit, HREK `tool.exe`/`tool_fast.exe`, ManagedBlam, Reclaimer 2.1.1645, Bonobo/Baboon, Blender/Foundry,
   Ghidra 12.x, and the Discord guides (Halo Archive, Halo Mods). For each: what it is, when Noble uses it, its
   limits, and whether it is installed here. Sources: `discord-compendium\10_tools-guides-links.md`,
   `07_ripping-extraction-tools.md`, `09_scripting-ai-pathfinding.md`, `00_ULTIMATE_GUIDE.md`. Installed paths
   (verified 2026-11-14): Assembly `C:\Users\tyler\Desktop\assembly\Assembly.exe`; HREK
   `D:\Steam\steamapps\common\HREK` (Foundation.exe, Sapien.exe, MegaloEdit.exe, tool.exe, tool_fast.exe);
   Reclaimer `D:\RECLAIMER\Reclaimer-2.1.1645`; Blender launcher in `%LOCALAPPDATA%\Microsoft\WindowsApps`;
   Ghidra `C:\Tools\ghidra_12.1.4_PUBLIC` and `ghidra_12.2_PUBLIC`. Descriptive and operational: purpose, use,
   limits, path, credit/licence note.

Acceptance: each draft names its sources, separates proven from unverified, and fits on one screen per section.
Round-1 drafts (`draft-memory.md`, `draft-session-networking.md`) are already merged into
`knowledge\PROJECT-NOBLE-COMPENDIUM.md` and are kept only for reference. Round-2 drafts
(`draft-model-animation.md`, `draft-forge-packs.md`) are merged as chapters 4 and 5.
