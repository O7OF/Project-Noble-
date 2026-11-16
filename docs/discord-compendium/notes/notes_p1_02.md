# Notes p1_02 (Discord #general-help, 2024-01 to 2026-03)
Source: chunk_02.txt (892 lines, fully read). This slice is mostly Blender/Spartan-pack user help, so Reach tag-modding content is thin. Tags: (author, YYYY-MM, channel). Channel is general-help throughout. Confidence tags: confirmed / claimed / disputed.

## 1. Tool.exe / editing-kit commands and import pipeline
- To get an H3 render_model's rig and markers back out of the H3EK, run `tool extract-import-info`. For most assets it writes an .ass you can open in Blender. Use that rig and those markers when porting a model (claimed) (chiefster_4014, 2024-02)
- Porting an H2A or foreign mesh into H3 works best by exporting the target's own render_model from the mod tools, then reusing its skeleton and markers. Bone names must match exactly (rename if needed) and the scale must match H3. This is the fix for "matching node can't be found" in first person and for the "no animation graph" error on a ported biped (claimed) (chiefster_4014, 2024-02)
- .JMS cannot be extracted from Reach, H4 or H2AMP because their import pipeline changed. For these games use Reclaimer on compiled .map files, or the Foundry Blender addon when working from tags (confirmed) (chiefster_4014, 2024-11)
- Tool had no model-export command before Halo 2, so CE .gbxmodel cannot be exported with Tool. Import the .gbxmodel directly with General-101's Halo-Asset-Blender-Development-Toolset (claimed) (chiefster_4014, 2025-11)
- Models extracted by Reclaimer are poor for re-importing into the game because the geometry differs from the original import. Extract from the H3EK data folder (original .ass/.jms) instead (claimed) (chiefster_4014, 2026-01). Someone reported the same problem with H2A and H3 assets going into Reach (rdt4, 2026-01)
- For a CE-style .jms: open the data folder next to the .model (not the .model itself) and import the .jms with General-101's toolset (chiefster_4014, 2024-10)
- Official mod tools contain shader info and shader code for CE through H2AMP. For H5 shader details you must use Reclaimer (chiefster_4014, 2025-06)
- Foundry Blender addon builds shaders and colours automatically when importing H4 content, but you need the Halo 4 mod tools installed (chiefster_4014, 2025-10)
- Foundry can import Reach tags (as above). In Reach tags the unsupported scenario-to-Blender direction for H3 maps (.ass back to .scenario) was asked about but not answered (nikolaas4430, 2024-01)
- Rebuilding FMOD sound banks (H3): `tool export-fmod-banks "reports\reports_00\sounds_report_sizes.csv" pc sfx -bank:h3`. It worked once, then stopped producing sfx.sfb after more sounds were added. Custom sounds worked in Sapien/Guerilla but not in MCC after building the level and fsb banks. Unresolved (ohwhen6442, 2025-06)
- Lightmapping: the Reclaimer-site instructions call `python calc_lm_farm_local.py` from the H3EK folder. It failed with "python isn't found", so Python must be installed and on PATH even though the script ships with the H3EK (ohwhen6442, 2025-06; no confirmed answer)

## 2. Reclaimer / Bonobo / map extraction
- Reclaimer: open via File > Open Map. campaign.map, multiplayer.map and shared.map cannot be opened as targets. To extract from other maps you also need mainmenu.map, shared.map and campaign.map in the same folder (confirmed) (chiefster_4014, 2024-05 and 2024-11, 2025-10)
- 360 .map files: https://haloce3.com/xmf (chiefster_4014, 2024-11). CE map filenames per level: https://www.halopedia.org/Map_file (chiefster_4014, 2025-10)
- If CE exclusive-map textures are missing, copy the shared maps from the custom_edition folder into the maps folder (claimed) (chiefster_4014, 2025-10)
- Reclaimer error `FileNotFoundException: Could not find shared.map` while loading a Halo 2 scenario_structure_bsp from a leaked/uncut build means the shared.map must sit in the same maps folder as the other .map files (zlitchufdux, 2025-07)
- Reclaimer "doesn't accept the file" for a Halo Online map from the Steam Workshop (lexy.shirosaki, 2024-09). Not answered
- Reach campaign map would not open in Reclaimer (vor4dorsraven, 2024-09). Answer given later: need mainmenu, shared and campaign maps present
- Export formats: AMF does not work on current Blender and is outdated. Export RMF from Reclaimer and use the RMF Blender plugin (confirmed) (chiefster_4014, 2025-07). Someone asked why Reach maps export as .rmf rather than .amf; this is the current behaviour (krazyzark, 2024-11)
- Reclaimer v2.0.1536 (https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.1536): RMF exports include a colour channel for H4/H5 meshes using position W. Affected models must be re-exported and the Blender import script v1.0.1 is required (gameoverload, 2025-01)
- Old JMA import error from Reach Jackal: "could not convert string to float: '0,9999999'". Cause is a comma decimal separator from system locale; fix is to run with a period decimal locale (claimed, from the evident message) (juliociraptor, 2024-07). Reach animations extracted with Reclaimer/Bonobo as JMA were then imported with the Blender toolset
- Reclaimer error extracting the Reach brute chieftain (zad_47, 2025-03) and extracting H3 ODST custom maps (dg5312, 2024-05) were raised without answers
- A modified Reclaimer exists that keeps the original lightmap UV channel in RMF output (so H2 baked lighting can be reproduced). Lightmap bitmaps had to be grabbed at runtime with NinjaRipper (tim_irl, 2025-10)
- Stock H2 lightmap bitmaps can also be pulled from editing kits with General-101's toolset, which can write lightmap UVs and .bitmap tags for CE and H2 MCC kits (chiefster_4014, 2025-10)
- Anniversary/MCC Reach map collision: only vehicle and prop collision models were found; map collision is a bsp question not answered here (sosa758, 2025-03)

## 3. Blender toolsets, rigs, import/export
- General-101 Halo-Asset-Blender-Development-Toolset: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset (imports/exports JMS, JMA and ASS for CE/H2/H3/ODST; cannot read .map files). The zip is not a normal addon-zip install in some versions; check the README (inversefrog768, 2024-05). It also imports every animation in the repository below (chiefster_4014, 2025-12)
- Animation repository CE through H4: https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main (chiefster_4014, 2025-12)
- Digsite crew tags/assets: https://github.com/digsite (chiefster_4014, 2025-07)
- Assembly edits .map data but does not extract models. Reclaimer extracts. Toolset is for the editing-kit formats (chiefster_4014, 2025-10)
- CR4B (cr3b/cr4b) map-porting tool: best support for H3/ODST, minimal for Reach, "mega broken" for Infinite. As of 2024-02 CR4B did not work correctly for Reach and no equivalent existed for CE/H2 (chiefster_4014, 2024-02; gameoverload, 2024-05)
- Alternative to ripping: Architect plugin is outdated (bojackdaggah, 2025-01)
- Bone/animation fixes
  - H2A Elite with H2 JMAs stretches because skeleton scale differs: scale the H2A elite to exactly match the H2 classic elite (confirmed) (chiefster_4014, 2024-08)
  - Weight painting checklist: vertex groups must match bone names exactly, armature modifier must target the rig, and the group must have non-zero weights (purplmunkii, 2024-11)
  - Swapping a prosthetic arm: separate Reach left-arm bones, parent the arm, scale/move it, join to the target rig, delete the target's left-arm bones (chiefster_4014, 2024-02)
  - H3 Elite has no chest bone; adding one needs manual bone plus weighting (monokkii, 2025-10, unanswered)
  - Joining meshes breaks Spartan pack rigs (armature modifier/vertex groups lost) (moistsaltine, 2024-08)
  - Reach Spartan rig bones looked wrong on import for marines (.crackonos, 2024-10, unanswered); Blender 4 broke older Reach FP arms (beeboponthetop, 2024-08)
  - H3 device_machine elevator: animation plays on render model but physics model stays at start in Sapien. Cause not found (bongcloset, 2024-10)
  - Game rig vs IK/FK rigs: use the game rig for animations taken straight from H3 (spartanatic, 2025-01)
- 3ds Max Reclaimer import script creates Physical material in newer Max even when Standard is requested; maxscript StandardMaterial() was suggested (dg5312, 2024-04, unresolved)
- Blender 4.0 broke older Infinite shader packs; Blender 4.2 slow loads with "max shader compilation" message (squeak_grenade, 2024-01; _r.e.d., 2024-07)
- Cycles error `Invalid handle in cuModuleGetGlobal_v2 ... kernel_params` on GPU with ODST kitbashes; CPU works. Also `Cycles: out of SVM stack space, shader too big` and Optix failures from shader complexity (matt161, 2024-05; iexaminar, 2025-04; backslashmedia, 2025-06). No fix posted beyond CPU or simplifying shader

## 4. Colour-change (CC) maps and zones
- H4 Spartans: diffuse is in the green channel of the control map; the alpha channel holds the colour-change zones. White = primary colour, grey = secondary (confirmed) (chiefster_4014, 2025-01). Matching Unity shader: https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity (same author: Reach one at https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity, 2024-02)
- Reach/H3 era visors: texture named visor_cc is only the colour-change mask used for all visors; visor normal and diffuse sit with the helmet textures (chiefster_4014, 2025-10)
- Reach emblem channel logic (Blender): emblem textures are masks. Mask 1 and mask 2 drive two mix nodes in series: first mix starts black, then the first mask colour, chained into the second mix with the second colour (gameoverload, 2024-01)
- Reach/CC masks in Infinite control maps: a mask controlling where colour goes appears to be in the alpha channel of the control maps (claimed) (chiefster_4014, 2024-05)
- Infinite packs also need mask 0 and mask 1 for scratch and colour paths (gr1mzter, 2024-02). In a black/white zone mask, white = colour shows, black = none (splutteryelm, 2026-01)
- Infinite armour: 6 colour channels plus scratch and dirt; each material layer node has top, mid and bottom colour. Not every piece uses the same material layer, so identical colours can still look different (gameoverload, 2026-01). Sets of 7 zones per piece, each needing all 3 colour inputs changed (zattlebone, 2025-05)
- Decals: Infinite shader 3.0+ has a decal input at the bottom (and emblem inputs separate from the ASG); feed Colour and Alpha of the image texture and set a UV node to a duplicated UV map (chiefster_4014, 2024-03; purplmunkii, 2024-08). Do not plug emblems into the ASG (chiefster_4014, 2024-03)
- Tint on a transparent base texture (Captain Lasky model) tints everything including gold; need to mask the tint (lt.pelekito, 2024-02, unanswered)
- Reach hex colours for armour/emblems: pointed to #lavadeeto-resources channel (aegis_wolf, 2025-05); no code listed here. Screenshot colour-pick is the fallback (wraith0912, 2024-03)

## 5. Shader and material behaviour (normal maps, spec, env)
- Reach/H3 normal maps: invert the green channel (confirmed in several answers) (chiefster_4014, 2025-08). Reach-style normals are XY with Z filled with 1; recompute Z in Unity/Unreal (gameoverload, 2026-01)
- Non-colour data textures (control/normal/ORM) must be set to Non-Color (gameoverload, 2024-06)
- Infinite packing: R occlusion, G roughness, B metallic (unused in head shader), A subsurface. Clothing uses a simple opaque shader, heads use subsurface profile (gameoverload, 2024-06)
- Seam in the middle of a normal-mapped model: model geometry/custom split normals; clear custom split normals on a duplicate before baking (matta_klark, 2024-02; chiefster_4014, 2024-05)
- Metallic armour showing SSS: turn down the subsurface section on the Infinite shader (masterj2001, 2024-08)
- Infinite shaders broke after Blender 4.0: open the node group (Tab), replace the Principled BSDF inside with a new one, copy inputs across. Setting IOR to 1.45 also fixes some (purplmunkii, 2024-07; soulburnin, 2024-09). Shaders older than 4.0 need this update (thoro.blend, 2024-09)
- Alpha issues: disconnect the Alpha input and set Alpha Blend to Opaque (chiefster_4014, 2024-06). For transparency use alpha hashed or a mix shader with a Transparent BSDF (squeak_grenade, 2024-11)
- Reach visor look: IOR/roughness-style settings of 8 or 10 were used in the Blender pack (squeak_grenade, 2024-12, claimed)
- Glow lights lose colour and go white when emission is raised: use a colour/mix before emission, and rely on emission colour rather than strength (unanswered, 2024-06)
- Reflections: CE-style cubemaps. Images are DXT compressed; run through the ESRGAN model "1x DXTDecompressor Source V3" (https://openmodeldb.info/models/1x-DXTDecompressor-Source-V3) before cropping faces or stitching a cross. chaiNNer helps with colour shift (tjsauce, 2025-12). In Blender use an Environment Texture with the Reflection output of Texture Coordinate, not an HDRI (solipsol_studi, 2026-01, unresolved)
- CE shader use: no standard setup because CE shaders have many toggles (gameoverload, 2024-09). H2 vs CE spec question unanswered (dustyoo10, 2025-11)
- Skyboxes: Halo skies are meshes (domes, planes) not cubemaps/HDRIs, so Reach HDRIs must be built (gameoverload, 2025-10; chiefster_4014, 2025-07). Community HDRIs of the Reach corvette and a Reach sky pack by Draken exist in the server (koax117, 2026-02)
- Shader languages: DX11 HLSL, DX12 DXIL/SPIRV, Vulkan SPIRV/GLSL; HLSL docs https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl (gameoverload, 2024-04/06)
- CE pixel-shader cubemap define: `FIRST_MAP_IS_CUBE` toggles Texture2D vs TextureCube in CE .psh when porting to GLSL (spartanjoe193, 2024-07, unresolved GLSL port)
- Procedural vs baked: modular layered shaders can share one material; atlas-style sheets with UV selection are more performant (gameoverload, 2025-12)

## 6. Baking, export to Unity/Unreal/VRChat/SFM
- Bake recipe for Infinite/Reach node-group shaders (confirmed repeatedly): add an Image Texture node with a blank image per material; plug each shader output (Colour, Metallic, Roughness...) into Material Output Surface; bake type Emit; samples can be 1; Cycles engine; non-colour for data maps (chiefster_4014, 2024-02, 2025-02, 2025-10; thefinalfronti, 2025-11)
- Normals: bake from Normal type using the BSDF output, on a duplicate with custom split normals cleared; Blender 3.6+ needs shade smooth rather than auto smooth when baking (chiefster_4014, 2024-02, 2024-05)
- Baked colour darker than viewport: unresolved; likely colour space/view transform (muchacho5894, 2025-11)
- Black bakes mean there is no colour data from the shader output (only normals bake from the shader itself) (gameoverload, 2025-02)
- Fiy Reach shader for Unity: https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity. FBX export: tick Limit To Visible Objects (chiefster_4014, 2024-03)
- Infinite shader did not support Blender baking in some pack versions (gameoverload, 2024-10)
- 8-bit vs 16-bit saves look different in viewers; export 8-bit/sRGB for consistent colour (blue.nasca, 2024-02/2025-03)

## 7. Gameplay / tags / AI (H3 and Reach)
- H3/ODST grunts do not suicide: kamikaze value was set but they only flee; unresolved (rougegamer, 2024-07)
- Use Reach Sapien (supports player simulation) for Reach work (spartanjoe193, 2024-07). Reach tag-group articles are mostly empty (spartanjoe193, 2024-07)
- Foundation permutation question (put a helmet on a marine): answer was not given (pendragone01, 2024-02)
- Cinematic events in Blender-to-Halo exporters (dialogue/particles/script events): design questions between timeline markers vs custom panel (iloveagoodcris, 2025-03, unanswered)

## 8. Networking, anti-cheat
- Reading memory in custom multiplayer for overlays with EAC disabled: asked, no answer (lehefe, 2024-02)
- Flyable pelican in MP with anti-cheat on: asked, no answer (die_rise., 2024-09)
- Replacing Reach voice lines: use Halo Reach mod tools; modding guidance on Halo Mods Discord (chiefster_4014, 2025-10). Reach FSB sounds live under haloreach/fmod/pc; site: https://www.sounds-resource.com/xbox_360/haloreach/ (brooen, 2024-12)

## 9. Links worth keeping
- Cylix guide (Infinite parts/coating IDs): https://cylix.guide/ (gameoverload, 2026-01)
- Reach SFM/Blender resource sheet: https://docs.google.com/spreadsheets/d/1N860qHHlzj7zL2FHCnz4VcGCRSGOxnxa63pKZDwf3Sc/edit (zepumpkineater, 2024-05)
- ambientCG PBR textures: https://ambientcg.com/view?id=Tiles072 (gameoverload, 2024-02)
- Armor Watcher emblem retrieval: https://www.armorwatcher.com/armory (x_crimsongalax, 2025-07)
- Plastered Crab tutorials: https://www.youtube.com/watch?v=y_lRfuCg1NA and https://www.youtube.com/watch?v=Jm39-vYl1zo (spartanatic, 2024-05)
- Animation ripping/porting tutorial: https://www.youtube.com/watch?v=oIIHsOifz60 (juliociraptor, 2024-07)
- Decal technique: https://www.youtube.com/watch?v=MBy62M7TcgY, https://www.youtube.com/watch?v=rUP8aAG4AQ8 (chiefster_4014, 2024-12; halostrophe, 2025-10)
- Licence caution: community node setups may forbid commercial use; textures belong to 343 (frostbitecinem, 2024-01)

## Open questions raised but unanswered
- How to fix `could not convert string to float` on Reach JMA import (locale)?
- How to export H3 maps from Blender back to .scenario via .ass for Sapien?
- Why H3 elevator device_machine physics model stays at rest in Sapien?
- How to make AI grunts suicide in H3/Reach Sapien?
- Why custom sounds work in Sapien/Guerilla but not in MCC after fsb build, and why export-fmod-banks stops writing sfx.sfb?
- Python requirement for calc_lm_farm_local.py (lightmapping)
- Whether memory reading for custom-game overlays is feasible with EAC off
- How Reach handles map collision (sbsp) and how to import sbsps into Reach (makeshift.h2, 2025-12; the1updude, 2025-06 asked for Blender-to-tag vehicle workflow)
- Reach animation formats/compression references (superintendent, 2026-01)
- How to colour Reach elites like MP spartans with Reach rigs (x_crimsongalax, 2025-07)
