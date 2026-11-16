# Notes p1_07 (Halo modding Discord, chunk_07: texturing, shader-resources, tutorials-resources, animation, rigging channels; 2021-2026)
Caveat: this slice is mostly Blender-side (texturing/animation/rigging of ripped assets), little tool.exe/Sapien content. Confidence tags: confirmed / claimed / disputed.

## Where shader code and channel layouts come from (authoritative sources)
- For CE, H2, H3, ODST, Reach, H4 and H2AMP the real shader code (.fx) ships with the official mod tools; look under H2AMPEK/data/shaders/material_shaders/materials and open the .fx with the same name as the material-shader the material tag references. (chiefster_4014, 2024-12, texturing; confirmed). The same person says CEA and H2ASP shader code can be extracted with "Index"; other games (H5, Infinite) are educated guesses (2025-12).
- Steam editing kits exist for: Halo CE, Halo 2 (classic), Halo 3, ODST, Halo 4, H2A MP. Reach kit also exists (referenced in animation channel). (gameoverload, 2026-01)
- Practical advice for accurate previews: read the mod-tool shader code rather than guessing; the code states exactly how masks/specular/CC are combined. (gameoverload, 2025-04)
- CE shader_model tag docs: https://c20.reclaimers.net/h1/tags/shader/shader_model . Multipurpose map channel meaning depends on shader tag settings; Xbox order = R detail mask, G self-illum mask, B reflection mask. Different CE versions (Xbox vs Gearbox/PC) use different channel orders. (chiefster_4014, 2025-11 / 2026-05; confirmed)
- CE reflection setup in Blender: use an Environment Texture node (not Image Texture); feed Geometry node Incoming and Normal into a Vector Math "Reflect", output to the Env Texture vector. Split the multipurpose map with Separate Color. (chiefster_4014, 2026-05)
- CE elite question: whether CE elites have base colour or just cubemap was asked but not settled in chunk (dustyoo10, 2024-11; unanswered). Dustyoo10 notes CE marines seem not to use a cubemap, so multipurpose map role unclear (2025-02).
- CEA campaign GLT (Global Lighting Template) shader has no texture definitions, which makes it simpler; CEA uses _br and _hdetm textures (gameoverload, 2026-01; claimed).

## Halo 3 / Reach shading semantics (specular, CC, masks)
- H3/Reach: specular coefficient is a flat multiplier into the specular; specular exponent controls power of the blend between specular colour and fresnel colour; reflection mask only gates where cubemap reflections appear. (chiefster_4014, 2026-06; confirmed)
- Mapping to PBR for lookalike previews: use specular coefficient as spec colour and exponent as smoothness (gameoverload, 2025-05). H3 spartans mostly use a nearly static roughness; visors should not be super metallic to match vanilla H3, base visor w/o cubemap is rough; multiply visor colour x2 for brightness. A slight blue tint on armour diffuse helps recover the bluish specular (chunch/chiefster, 2026-06).
- CC (change colour) map channel usage (chiefster_4014 + gameoverload, 2026-03): H3 CC maps can use RGB but sheet shows RG; Reach CC maps can use RG; base H3 armours are all RG, Reach base armour is RG; Raven sets are among the few using RGB; some Halo Online/mythic armours use RB or RG. The "specular, blend, environment, roughness" layout is only for H3's original Cook-Torrance shader; 343's in-game PBR shader (H3 MCC) uses just specular and roughness. H3 "layer 1" map: chiefster says roughness and metallic order on the cheat sheet is swapped (disputed; gameoverload says it should be correct but will recheck).
- Reach marines: camo is baked into the change-colour map marine_cloth_cc; no separate camo map exists (chiefster_4014, 2026-07; claimed).
- Reach spartan banshee-style shine: a user says the metallic look was obtained by multiplying the banshee's specular-texture alpha channel with the secondary colour mask (spartanjoe193, 2025-05; claimed, Blender trick).
- CE-era trick: multipurpose blue channel can be used as mix factor for colouring the Spartan (bromanche, 2025-01).
- H2A uses 4 maps in ripped MCC assets: Diffuse, Specular, Normal, Emissive; normals ripped from MCC via NinjaRipper look yellow and must be colour-inverted; in Blender hit "Clear Custom Split Normals" on each mesh or you get odd black shading. (surasia quoting Jackal, 2021-07, tutorials)
- Reach Elite glowing parts visible in model pane but not in Render mode: unresolved (x_crimsongalax 2026-01).
- Reach godray/light-volume textures (e.g. fx_godray_a_alpha on m45) appear clipped in half in Reclaimer 1.7.808; latest at the time was 2.1.1608 (2025-08), update first. Release notes mention Reach workshop map fixes, H2V map fixes. https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1608 (manusanram, chiefster_4014, 2025-08)
- Nihren found a "broken" upscaled Reach texture was actually the menu (UI) texture's Color Control; game textures are higher res (many 2k) than menu textures (2025-10).
- Reach armour is often higher-res than expected, some 2k (chiefster_4014, 2025-10).
- Reach halo online plasma rifle reuse: Reach plasma rifle model was used for CEA, H2A SP&MP, Halo Online (chiefster_4014, 2024-12).

## Blender baking / colour-space rules for ripped Halo shaders
- Normal maps: Non-Color; Normal Map node set to DirectX (not OpenGL) for Halo packs; everything else Linear Rec.709/sRGB as appropriate (chiefster_4014, 2025-11, 2026-10). Colour textures sRGB, data Non-Color (haloismylife65 asked, 2025-04).
- Set image Alpha mode from Straight to Channel Packed on all textures, otherwise colour is multiplied by alpha and shader can go black (chiefster_4014, 2026-06; gameoverload 2025-07). Confirmed fix by users.
- Bake Halo Infinite/HIMS-style shaders: Bake Type Emit, plug the shader's own Bake outputs (Bake Color, Unity Mask Map, Unity Smoothness Map) into Material Output Surface; for normal, plug the full shader and bake type Normal. Samples 1, denoise off; resolution 2048-4096. Baking Diffuse type is wrong. (chiefster_4014, 2026-04/11)
- Baking tips: bake onto your own new image texture nodes, not the images already used by the shader (chiefster_4014, 2025-02); if multiple materials, add image node to each or separate meshes; typical baking addons break on Infinite shaders (2026-08).
- Reach shaders are not PBR so baking is tricky: bake diffuse, specular colour and a flat gloss map by using node-group outputs (chiefster_4014, 2026-08). Pack with accurate Reach shaders exist in the asset-packs channel (no URL given beyond Discord link).
- For UE/Unity/VRChat export: Infinite spartan uses procedural materials (procmat) and must be baked to albedo/rough/metal (chiefster_4014, 2025-10).
- Cycles shader-normal vs geometric-normal issue: Cycles uses pre-normal-map normals for light sampling, causing artifacts; masterj2001 patched a build and posted at devtalk.blender.org and projects.blender.org PR 135380 (2025-02/03).
- Specular/Roughness decode of "smoothness" maps with non-color: multiply x2 add -1, square root, then 1 minus to get roughness (gameoverload, 2025-05, for DOOM TDA but same degamma pattern).

## Infinite/Halo 5 channel layouts (general-purpose, may help Reach comparison)
- Infinite ASG = R occlusion, G scratch, B grime (not just metal/rough); Mask0 R/G/B = zone1/2/3 (black = zone 0); Mask1 R/G/B = zone 4/5/6 (6 often dust); normal RG with Z recomputed, DirectX. (gameoverload/chiefster, 2026-01)
- Infinite decals: shader has decal inputs at the bottom of node group; set image extension to Clip; use Mix Color Add with factor 1 for colour and Math Add for alpha to stack decals (chiefster_4014, 2026-06). Add own UV for decals.
- Fallout 4 / Division 2 layouts were posted for comparison but not Halo-relevant.
- Stahl's H3 colour mismatch: gameoverload/chiefster explained H3 lighting plus old H3 Blender pack shaders differ; exact hex codes will not look identical without H3 lighting (2025-09).
- Halo 2 E3 stencil shader lighting, H2 cortana shader: asked, unanswered.

## Existing shader packs / resources (URLs worth keeping)
- Unity: Open Halo 5 Spartan shader https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity ; also Open Halo 4 (V0.5) and Open Halo Reach (V0.8) Spartan shaders by FiyCsf on gitlab with same naming; weapon shader for H5 (fiycsf, 2021-06).
- Lavadeeto's H3 Spartan Unity shader (VRC capable), docs: https://docs.google.com/document/d/1kfCLkQWxZGDwP38FSXJa2svR3X_N5XYP-qcup4V7rXA/edit ; must apply lookup tables named c78d78, cc0236, dd0236 (2021-08).
- Halo Infinite VRC shader https://github.com/R93Sniper/HaloInfiniteVRCShader ; modular Infinite shader https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases (2022-03).
- Chief's Blender Halo shaders for H3/ODST/Reach/H4 (object, vehicle, WIP terrain) on Google Drive (chiefster_4014, 2021-07; link in channel: drive id 1bJJnAItSgoIgPvphSMVdX5QBVkJ-YUNC).
- Foundry (Blender addon for Reach, H4, H2A MP): https://github.com/ILoveAGoodCrisp/Foundry ; imports levels with shaders ~90% game-accurate, decals need manual work (koax117, 2026-09). Recommended over old Bonobo workflow for Reach+ assets, including fp animations (r93_sniper, 2025-12; autarchkatherl 2025-08).
- General-101 Halo-Asset-Blender-Development-Toolset (io_scene_halo): CE/H2/H3/ODST JMS/JMA/ASS; install by placing io_scene_halo folder in Blender scripts/addons (unzipped). https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
- Mjolnir Forge Editor (Reach Forge in MCC to Blender) https://github.com/Waffle1434/Mjolnir-Forge-Editor
- Assembly https://github.com/Lord-Zedd/Assembly ; TagTool https://github.com/TheGuardians/TagTool (plastered_crab, 2021-03)
- HaloAnimationRepository (ripped animations for MCC modding) https://github.com/77Mynameislol77/HaloAnimationRepository (r93_sniper, 2023-09)
- Doom 2016 graphics study (cubemap reflection reference) https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/ (gameoverload, 2025-12)
- Substance Painter ASG-map guide by TacoHombre: https://www.youtube.com/watch?v=8Szp4FM4y64 (2023-01)
- Alt-Tab Colorful palette addon https://superhivemarket.com/products/alt-tab-colorful (koax117, 2026-05)
- Reach Elite IK variants (elitecaden, 2022): built from Chief's Reach Elites blend; horse-link rig issue noted (hakenstudios 2021-12).

## Ripping and tool versions
- NinjaRipper recipe for H2A MCC (Jackal, 2021-07): target MCC\Binaries\Win64 x64 exe, inject mode D3D11, run; EAC blocks game with an error but DLL is planted; launch MCC anticheat-disabled afterwards; force-rip then wait about 10 minutes (game runs ~2 fps); browse results in Noesis; export OBJ. (confirmed process, anti-cheat must be off)
- Reclaimer (Gravemind2401) used for models; export AMF for Blender or JMS/AMF for Max. 2.1.1608 latest as of 2025-08; old versions break Reach workshop maps.
- Bonobo (LaikaGlove) extracts jmad source (.jma/.jmm) from Xbox-era CE/H2/H3/ODST/Reach .map files; does not work with Halo 4 (sswampp, 2022-01). Needs shared, campaign, mainmenu and a level map in same folder. Known failures: Reach fp_magnum jmad -> "Value cannot be null (source)", fp_spartan_laser -> "Index was out of range"; magnum cannot be extracted; Bonobo is unmaintained, source lost (mrchromed, 2021-07).
- Assembly CLI alternative for extracting animations (r93_sniper, 2022-09): `map.map EditTag objects\vehicles\pelican\pelican.jmad ExtractAnimation all "C:\out"` (full map path first, then EditTag, tag path, ExtractAnimation all, output folder).
- Blender import: AMF with scale set to 3DS Max option; then File > Import > Halo Jointed Model Animation (JMA/JMM), set the game version correctly (plastered_crab/R93_Sniper, 2021-07). JMM import is used for H3 first/third person (collinbishop 2025-03).
- General-101 addon also covers H3/ODST ASS export; Foundry covers Reach+ only (iloveagoodcris, 2024-01 confirmed).
- H4 JMA files exist in h4-packs but older toolsets do not import them (existential_se 2023-04, unanswered); H2A MP animations: Foundry can handle (r93_sniper, 2025-12) but asked unresolved.
- Max vs Blender gr2 animation differences: axis-angle from gr2 quaternion differs between Max and Blender scripts causing broken animation conversion (stumpyj, 2022-12; unresolved).
- Animation retargeting Reach -> Infinite rigs: manual and Rokoko/Mixamo/Mwni retargeting all gave exploded results; suggested bone-pair mapping script plus hand polishing (.username_ladi, 2026-01). Reach rigs are big/different from Infinite rigs. Retargeting to non-Halo skeleton is possible (r93_sniper); Foundry easier than old tutorial.
- Reach Jackal/Elite/marine rigs: Reach stock skeleton is awkward for VRChat (fiycsf, 2021-10 recommends his H5 skeleton); Reach marine female rig has bad shoulders/back weights (2022-01).
- Missing weights when kitbashing: delete rigs of appended parts, parent with automatic weights then paint leftovers (pmarc0011, 2023-07; question).
- Elite leg bones follow mesh; Reach Elite horselink armour overlaps feet (surdavow, 2021-05).
- SFM / Source: Blender recommended; Source lacks PBR and cloth sim (gingerjesus, 2023-06). GMod LVS vehicle seat animations asked, unanswered.

## Shader snippets that worked in Blender
- Fix for outdated Infinite shader on new Blender: replace the Principled BSDF inside the big node group with a new one and rewire (purplmunkii, 2025-01).
- Dielectric specular/metal conversion for _s textures (R dielectric spec, G roughness, B metal): set IOR 1000, Mix Color 1 (fac metallic, A diffuse, B black) -> Base Color; Mix Color 2 (fac metallic, A dielectric*0.08, B diffuse, mode Mix) -> Specular Tint; metallic input 0 (gameoverload, 2025-12; claimed).
- Godot export packing R=AO, G=roughness, B=metallic (gameoverload, 2024-11).
- Masks setup for a CE-style Reach/H3-type texture: R emissive, G spec power (gloss), B spec intensity (gameoverload, 2024-11; claimed for the specific asset).

## Open questions raised but unanswered
- Do CE elites have a real base colour or only a cubemap? How is CE multipurpose used on marines?
- How to add new colour zones to HIMS or fix a piece sharing a zone (H3/Reach UV zone sharing).
- Halo 2 cortana/E3 stencil shader recreations; H2 visor cubemap (more orange than CE) identity.
- Extract H2A MP and Halo 4 animations to Blender; Reach magnum fp jmad extraction failure; armour lock animation.
- Animation tag binary layout for Reach (superintendent, 2026-01).
- Light volume texture halving in Reclaimer for versions newer than 2.1.1608.
- Why Reach Elite emissives vanish in Render but show in tweak pane.
- Retargeting Reach animations to Infinite rigs without exploding meshes.
