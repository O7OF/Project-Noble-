# Notes p1_05 (chunk_05: #ripping 2025-26, #reclaimer-up 2021-25, #texturing 2021-22)
Tag format: (author, YYYY-MM, channel). Channels: ripping, reclaimer-up, texturing. Confidence marked where relevant.

## 1. Tools, versions, links
- Reclaimer (Gravemind2401) is the ripper for CE, H2, H3, ODST, Reach, H4, H2AMP. Releases: https://github.com/Gravemind2401/Reclaimer/releases ; wiki https://github.com/Gravemind2401/Reclaimer/wiki ; v2.1.1616 fixes some H1 custom-edition maps not opening. (chiefster_4014, 2025-08, ripping; plastered_crab, 2021-05, reclaimer-up)
- For CEA and H2ASP (Saber3D engine) use Index: https://github.com/Wildenhaus/IndexV2/releases . H2A multiplayer uses BLAM, so Reclaimer works for H2AMP, Index for H2ASP. (chiefster_4014, 2025-08; gameoverload, 2026-02, ripping) [confirmed]
- Reclaimer 2.0 (2024-08/11): needs .NET Desktop Runtime 7; new RMF model format with Blender/3ds Max importer addon; adds Halo Infinite, scenario tags, `Export Metadata` (right-click, JSON dump without opening tag), decorator model support for Reach and H4. v2.1 (2025-02): Halo 2 Vista and H2 MCC, map browser. (gravemind2401, 2024-08..2025-02, reclaimer-up)
- Reclaimer 1.x settings: Assimp Scale default matches Halo scale; batch extract workers setting (concurrent extraction, opt-in); Meta viewer plugin profiles can point at Assembly's plugin XML dir; since 1.7.808 (2022-09) MCC title updates usually need no Reclaimer update if map structure unchanged (works via MCC install folder structure; copied/moved maps may fail). (gravemind2401, 2021-05/2022-09, reclaimer-up)
- Reclaimer 1.6.249: H4 material detection improved (fewer cases where control map is used instead of diffuse); AMF export includes normal/detail maps; TGA bitmap export; Halo 2 bump maps use right palette. Reclaimer 1.7.305: "export selected bitmaps" option (all bitmaps used by visible meshes); DDS export includes mipmaps (H1/H2) and all frames of array bitmaps (H3+). Markers are included when exporting parts of a model (1.7.307). (gravemind2401, 2021, reclaimer-up)
- Reclaimer cannot extract shader code. For H2, the only source is MCC mod tools: look at `.shader_template` / `.shader_pass` tags; tags only hold assembly-level data. (chiefster_4014, 2026-08, ripping) [claimed]
- Reclaimer H3 ODST/Mombasa (h100.map) rip showing white/untextured sky/billboards even with shared/mainmenu/campaign.map present: unresolved in chat. (lomnk, 2025-08, ripping)
- Foundry (iloveagoodcris) Blender addon for Reach/H4/H2AMP editing kits: drag a sky `.scenery` tag into Blender and Foundry builds shaders automatically; re-import into the tools using the `Setup as Asset` option. (chiefster_4014, 2025-11, ripping)
- Foundry does not support H3/ODST natively: ManagedBlam for H3 can't be loaded in python. Workaround claimed: copy H3/ODST tags into the HREK tags folder (works well for Chief, textures can have issues). (iloveagoodcris, 2025-10/12, ripping) [claimed]
- Foundry/Reach xrgb issue: user reported Foundry sets bitmaps to sRGB instead of xrgb on a Reach map. Crisp prefers controlling gamma via the Blender image colour profile instead of nodegroup gamma. (brooen / iloveagoodcris, 2025-10, ripping)
- Foundry Reach guide install: https://github.com/ILoveAGoodCrisp/Foundry?tab=readme-ov-file#installation ; set project to Halo Reach Editing Kit. Error "Ensure your file is saved to your Omaha data directory" appears when dragging animation files outside the project data dir. (iloveagoodcris / tristangold, 2025-10/12, ripping)
- General-101 Halo Asset Blender Development Toolset: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset (CE/H2/H3/ODST JMS/JMA/ASS import/export). Fork with Reach+ work: camden-smallwood/Halo-Asset-Blender-Development-Toolset. Error importing JMA/JMS on newer Blender: "action object has no attribute 'fcurves'" (Blender 5 API change; use compatible addon version). (chiefster_4014 / zoephie / tristangold, 2025-10..2026-08, ripping)
- Reclaimer-ripped CE rigs need a scale of 32.8084 applied; H2 JMA animations imported via the toolset can come in ~33.33x too large (fix via import scale / scaling the rig). (chiefster_4014, 2025-11; exodst., 2025-11, ripping)
- Extract Import Info (Foundry/H3EK): path argument must be an editing-kit relative path starting with `tags` or a full system path, not starting with `H3EK`. If H3EK.bat extraction nests a same-named folder, move the inner folders out to the kit root. (iloveagoodcris / magmashark009, 2026-02, ripping)
- H3EK: run `H3EK.bat` to extract tags, then open them with `guerilla.exe` to read shader tag parameters. (chiefster_4014, 2025-11, ripping)
- Baboon (Zoephie): Rust GUI built on `blam-tags` for MCC editing-kit tags (H3, ODST, Reach, H4, H2A): byte-exact tag read/edit and asset extraction (bitmaps, geometry...). https://github.com/Zoephie/Baboon ; dev builds include Unreal-engine (Chimp) browser for CEA (jms/pskx/textures). (zoephie, 2026-08, ripping)
- Halo animation repository (ripped anims for MCC modding, incl. Reach fp weapons e.g. fp_magnum): https://github.com/77Mynameislol77/HaloAnimationRepository ; import with Blender toolset. (chiefster_4014, 2025-08/2026-04, ripping)
- Digsite's official CE assets ported to MCC tools: https://github.com/digsite/h1 . (chiefster_4014, 2025-10, ripping)
- CR4B tool (PlasteredCrab): https://github.com/PlasteredCrab/Halo-CR4B-Tool one-click game-accuracy shader tool for H3/ODST/Reach in Blender; works best on Blender 3.6, not 1:1 but a good baseboard. (koax117, 2026-10, ripping)
- Halo 1 CE Mod Tools can be used to read vanilla CE models; H2/H3 fonts/HUD icons: `tool export-bitmap-dds "<tag path>" "<out dir>"` then take blue channel (H2ASP HUD); H2AMP/Reach/H4 weapon icons live in `maps\fonts\font_package_icon.bin`, open with Lord Zedd's FontPackager (fixedsys-ui entry; repeat for x2 and x3 packages). (haloismylife65, 2026-05, ripping)
- Lightmap UVs/lightmaps from Reach BSP: Reclaimer doesn't write the secondary (lightmap) UV set, so lightmap bitmaps in tags are unusable externally. (un_nonimus, 2025-12, ripping) [open]
- Reach tool-side texture import: convert DDS to `.tagc` with Forerunner's tool and import into Assembly under a custom name. (surasia, 2021-07, texturing)
- Frame debugging (Nsight) is how pointdevice reverse engineers shaders (e.g. H3 area specular term, H2A vertex-colour location which became a Reclaimer feature). Assembly's shader dumps are raw shader binaries, not readable HLSL. (pointdevice, 2021-07, texturing)
- Gmod/Source ripping route (WorkshopDL, GWTool, SourceIO, Crowbar) - not Halo, skip.
- Halo 3 sounds: https://sounds.spriters-resource.com/xbox_360/halo3/ ; (koax117, 2026-07, ripping)

## 2. Reach shader / material parameters (Reach `shader`/material tag, via Assembly/Guerilla)
- Reach diffuse: RGB = base colour, A = specular mask; normal = DirectX (invert green for Blender, set Non-Color). (surasia, 2021-05, texturing) [confirmed]
- Reach specular model is Cook-Torrance but non-metallic-roughness: base_map carries most of the look, specular is shininess mask; roughness is a scalar not a per-pixel texture; metallic/height/roughness maps don't exist in the standard pipeline. (the_heavynator, pointdevice, 2021-05, texturing)
- Parameter meanings found by testing (the_heavynator, pointdevice, chromacore, 2021-05, texturing) [claimed/tested]:
  - albedo colour tints diffuse (clamped 0-1); diffuse_coefficient scales final diffuse; specular_coefficient scales all specular (direct, fresnel, env, area); specular_tint tints specular (clamped).
  - analytical_specular_contribution = brightness of direct highlight; analytical_roughness = roughness of direct highlight (Beckmann NDF; visible change mostly under 0.5).
  - albedo_blend = how much diffuse tints specular. area_specular_contribution = area specular (looks like specular glowing with indirect light colour). environment_map_specular_contribution = cubemap reflection strength; env_tint_color tints cubemap; env_roughness_offset coarse blur (mip level; 0-0.5 no visible change).
  - fresnel_color / fresnel_curve_steepness control fresnel; `roughness` and `approximate_specular_type` appear to do nothing.
- Per-pixel roughness DOES work in Reach (found 2021-07/08): set analytical_roughness = 1, material_texture_black_roughness = 0.01 (non-zero to avoid NaN), enable use_material_texture (float constraint, usually constraint 0), and put a texture with roughness in ALPHA into the material_texture input; needs a transparent-capable shader with all material inputs (e.g. cargo_truck_decals). Reportedly a leftover from H3 beta; used by ODST body/fp hands. Colour channels of the material texture still not decoded. (the_heavynator, 2021-07/08, texturing) [claimed; pointdevice disputes it is a retail feature: "leftover from H3 beta", not used in Reach retail shaders]
- Reach water is the H3 water shader but many wave-shaping values do nothing. (the_heavynator, 2021-12, texturing)
- `grey_50_percent` bitmap is used by almost every Reach shader (neutral default). Detail-texture multiplier values wrongly set can skew spec colour. (tpg / soulburnin, 2022-01, texturing)
- Reach has an entire "environment specular" property set that is hard to reproduce. Cubemaps are baked GI-reflection storage; H3 uses them to save probes more than Reach. (chromacore/pointdevice, 2022-01 & 2021-07, texturing)
- Avoid plain Principled BSDF for Reach: accurate Blender approach is two glossy nodes sharing spec colour/roughness + diffuse, with env reflection ideally disabled. Coloured-spec recipe: Principled #1 everything 0 except albedo; Principled #2 spec map to colour, specular=1, metallic=1, inverted gloss to roughness, normal on #2 only; Add Shader into output. (jadedqrow, 2022-05; the_heavynator 2022-06, texturing)
- All Halo games use coloured specular; UE's default shader doesn't, so Unreal needs shader/gbuffer edits. Heuristic for PBR conversion: if spec RGB channels are close -> dielectric (base = diffuse), else metallic (base = spec). (pointdevice, 2021-05/06, texturing)

## 3. Colour change (CC) – Reach, H3, H4, H5, Infinite
- Reach `Change Colors` block (biped/object tag): up to 4 elements (0-3), each has variants with name + two colours. Elements map to CC map channels: 0 primary (R), 1 secondary (G), 2 tertiary (B), 3 quaternary (A). Element 2 also tints fresnel and element 3 tints specular for assets without an explicit specular_tint parameter; those tints do not use the CC map. When element 3 is both spec tint and quaternary colour it's masked by CC alpha. (unordinal + soulburnin, 2021-11/12, texturing) [confirmed by tests on grunt]
- Reach visors have their own small CC map whose alpha isn't pure white; CC alpha masks the quaternary colour applied to visor diffuse. (soulburnin/unordinal, 2021-12, texturing)
- Reach CC maps: dark values near weathered areas are used for variation; pasted via 4 channels. Secondary colour doesn't tint env map (false alarm). Alpha as "channel packed" in Blender fixes black/dark CC map reads. (chiefster_4014/theishikawarin, 2021-07, texturing)
- H3 MP armour textures: `_d` CC map with alpha, control map (R metal, G AO, A gloss), `_s` spec colour map. H3 spec mask feeds albedo_blend. (soulburnin, 2021-10; chromacore, 2021-11, texturing)
- H3 ODST CC details: camo areas of fp arms and body thigh plates have colour masks in alpha of cc texture; ODST and fp hands use material textures. Hunter colouring needs `_cc` map. (err_0r 2021-04; chiefster_4014 2022-01, texturing)
- H4 control types (Waffles/R93_Sniper/soulburnin): Type 1 (weapons/props, suffix "multi"): R spec, G spec exponent, B reflection mask. Type 2 (colourable objects e.g. MP Spartans): R spec/reflection, G diffuse, B spec exponent, A colour change (white = primary, grey = secondary, black = uncolourable). Illumination is in diffuse alpha. H4 has no dedicated CC map. (surasia/soulburnin, 2021-06/12, texturing)
- H5: Control R = colour zone 1, G = zone 2, B = metallic, A = smoothness (helmet visors may have metallic in R). Colour image is a near-white base hued by control; multiply colour by zone mask. Normal = DirectX. (fiycsf, 2021-04..2022-01, texturing)
- H2A MP: albedo + control (metallic, AO, smoothness) + normal; H2ASP: diffuse + specular(alpha = smoothness) + normal; elite CC mask is in alpha of diffuse. (adorabirb 2021-04; the_heavynator 2022-04; chiefster_4014 2026-09)
- Infinite: ASG R=AO, G=edge wear/scratch, B=dirt/grunge; Control0 R/G/B = zones 1-3 (+ edgewear variants per source), Control1 R/G = zones 4/5, B = blast damage. Max ~6 colour layers from two masks + 2 from ASG. No albedo textures exist. Normals DirectX. Shaders: Grand Bacon (original), Surasia 2.0/2.3, Goblin/AverageTrapEnthusiast modular (https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources). R93 Unity: https://github.com/R93Sniper/HaloInfiniteVRCShader ; blender importer: https://github.com/Coreforge/blender-halo-infinite (dev branch). (fiycsf/surasia/r93_sniper/coarforge, 2021-12..2022-07, texturing)
- Making custom CC: pack masks into R/G channels, keep base colours near white/greyscale, multiply colour via mask; mask metal wear separately so it isn't tinted. (fiycsf, 2021-04, texturing)
- Reach/H3 "step" function: pointdevice suggests step() for mask thresholding in shader code. (pointdevice, 2021-05)
- Halo 1 CE: `shader_model` for all non-BSP objects, `shader_environment` for BSP; cubemap mask = blue channel of multipurpose map; specular reflection is tinted, not the cubemap itself. Bumped cubemap distorts with bump map, flat does not. (vuthakral, 2021-06, texturing)

## 4. Blender shader setup tips
- Normals: Non-Color, invert green (DirectX->OpenGL); roughness/masks Non-Color; diffuse sRGB. Blender default sRGB breaks normal vectors. (_gallium, fiycsf, 2021-04..2022-06, texturing) [confirmed]
- Shaders in Blender 3.x for Halo: swagster Reach shader, ChromaCore Reach shader (accurate, uses analytical spec/albedo blend, can bake to PBR coloured spec), soulburnin H2A/H3 shaders; `Shader to RGB` doesn't exist in Cycles (toon shaders Eevee only). (various, 2021, texturing)
- Make colours editable in one place: driver set to linear colour, copy RGBA channels as drivers into a custom property. (surasia, 2021-07, texturing)
- Missing X/Y scale in UI: right-click the uniform scale and choose show X / show Y. (kaboose1066, 2021-11)
- Illum maps: plug diffuse into alpha/emission, Cycles needed for some setups. (kaboose1066, 2021-11/12)
- Emission as visor: duplicate shader and plug colour 1 into emission. (surasia, 2021-12)
- FBX carries little shader data in Blender (needs manual material rebuild). (deskclaw, 2021-12)
- Some elite textures need UVs flipped vertically. (unordinal, 2021-12)
- Missing textures on Reach rips: Reclaimer-exported materials may be named odd; DDS/tga must be hand-assigned; some mesh/visor materials (e.g. `additive_gradient`) have no real texture (glow). (opr_23b, 2022-04)
- Hologram shaders: use Foundry's shader for accuracy (koax117, 2026-09, ripping); Reach shield/antigrav use noise_map_a/b and thinness values (unordinal, 2021-11). Reach portable shield: 4 textures, same template as H1 jackal shield (spartanjoe193, 2025-08).
- Reach/H4 relation: H4 engine derived from Reach; Reach assets still exist in H4. (k2.ev, 2021-05) [claimed]
- Bake-to-standard-PBR: bake shader output (BSDF/emit) for engines lacking custom shader support; set colour space Non-Color before bake. (gameoverload, 2022-05)

## 5. Animation / rig / import
- Reach fp rig is the same proportional rig as CE-reach fp; bone names differ. (adorabirb, 2026-08, ripping)
- Elite walk cycles: H3 and H2AMP walks exist; default is pistol combat walk. (iloveagoodcris, 2026-10)
- Importing Reach animations to Blender: repo above + toolset; Foundry can import a cinematic (e.g. last cinematic of m50) if Reach tags are extracted. (iloveagoodcris, 2025-12)
- ripped Elite with no vertex groups (fmodel uemodel export of CEA/Chimp model) – unresolved. (tom_76, 2026-09)

## 6. Misc modding facts
- H3 MP armour extractor_arm textures aren't in H3EK tags; take from H3 MP Spartana set. (koax117, 2026-06)
- Reach is the engine of CEA MP: CEA MP = Reach with anniversary map pack. (chiefster_4014, 2025-09)
- H2 cut-content MP maps fail in Reclaimer; unresolved. (zlitchufdux, 2025-07)
- Reach lightning gradient textures, armor ability pucks etc requested; unanswered.

## Open questions raised but unanswered
- How to extract Reach/H3 lightmap UV set and lightmap bitmaps for external lighting.
- Why Foundry sets xrgb to sRGB on Reach bitmaps (possible bug).
- Channel layout of the Reach material_texture colour channels (only alpha = roughness identified).
- H3 ODST Mombasa untextured sky/billboards in Reclaimer.
- Whether per-pixel roughness is usable in retail Reach shaders (disputed).
- Reach armour-ability puck/lightning gradient texture locations.
- Foundry/ManagedBlam support for H3/ODST.
