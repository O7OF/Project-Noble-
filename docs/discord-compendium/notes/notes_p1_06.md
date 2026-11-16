# Notes p1_06 - texturing/shader channel (2022-07 to 2024-11), Blender-side texturing and shader reverse-engineering

Source: all lines of chunk_06 (1029 lines, one channel: texturing). Mostly Blender/Unity shader talk for ripped assets; the Reach-relevant and engine-level facts are collected first. Tag = (author, YYYY-MM, channel). Channel abbreviated "texturing".

## 1. Reach shader math and specular/fresnel (most useful for Reach)
- Reach's specular model is primarily Cook-Torrance based and uses roughness; the exact distribution (Beckmann or other) was asked about and never confirmed. (soulburnin/nihalrahman, 2022-11, texturing) - claimed / unresolved
- Reach fresnel function acts as the mix between the "specular tint - albedo blend" and the fresnel (glancing) colour. In the Blender HRCategory "cook torrance" nodegroup: bottom colour = specular tint, top colour = glancing (fresnel colour), mixed on top of the tint/albedo blend; the "exponent" value is the steepness of the fresnel curve. (soulburnin, 2024-01 and 2024-08, texturing) - confirmed by author of the group
- The exponent value can be read from the mod tools by clicking the specular colour in the shader/material (called "exponent" or similar). (soulburnin, 2024-01, texturing)
- To fill the Cook-Torrance colour inputs from Foundation: click into the colour boxes next to the function, use the RGB values listed under the "Xenon" entries, and divide by 255. (chiefster_4014, 2024-08, texturing)
- Reach/H3 "specular" is a colorised specular mask, not a PBR metallic map: specular texture x (analytical specular coefficient x specular tint) x final tint. So the ODST skin specular with a blue tint is the real colour of the spec texture, not packing. (soulburnin, 2024-07, texturing; pointdevice 2023-07 agrees H3/Reach spec is a colourisation mask)
- Metallic vs specular rule of thumb: if the spec colour is just derived from base colour + env reflection it is "metallic"; if the shader supplies its own spec colour on top of env reflection it is "specular" (H2A front plate shows darker green reflection over diffuse and cubemap). (gameoverload, 2023-01, texturing)
- True metals have black albedo (no diffuse), all reflectance comes from spec. Infinite's "metal mess" input darkens albedo and puts albedo colour into spec. (soulburnin/chunch, 2023-01, texturing)
- Halo 4 uses "old school" specular; a fully game-accurate PBR port is not possible. H4 MP visor shader does not use the hex pattern in gloss. (nihalrahman, soulburnin, 2022-11 / 2023-01, texturing)
- Fresnel Schlick across Reach/H4: albedo-tint-spec, no explicit IOR seen; gameoverload suggested verifying with IRTV. IOR below 1 does not apply specular tint; above 1 f0 is multiplied by tint (the Infinite metal-rough model, not Reach). (gameoverload, 2024-01, texturing) - claimed
- In H3 and Reach (and per r93_sniper H2) height/detail maps become normal maps; whether the game converts them or Reclaimer does was left unanswered. (r93_sniper, 2022-11, texturing) - disputed vs a claim they don't
- Reach glass shader is phong-based, hard to match exactly. (soulburnin, 2022-12, texturing)
- Reach/Covenant ship dark paint: Reach gets away with it via an iridescent shader for a chitinous look. (r93_sniper, 2022-10, texturing)

## 2. Where to read real shader source and parameters
- Reach mod tools (free, MCC) still ship shader source: path from material -> referenced material shader -> HLSL source files; text can be copy-pasted out. Use them as the reference for exact Reach shading and to read tiling values for terrain/blend textures. (r93_sniper 2022-11; gameoverload 2024-06 and 2024-01, texturing)
- CE: shader code is under the main tools folder "shaders" folder (fx.bin text); key files for the base model shader are fx/model_common.h and vsh/model.vsh. (fable_law_458 2022-08; jackofchaos 2022-11, texturing)
- CE console trick: open a level in Sapien or the standalone and run `debug_camera true` to show the shader name/path of whatever is looked at. Most CE transparent stuff is transparent_generic. (fable_law_458, 2022-11, texturing)
- Guerilla in H3 shows a shader tag as parameter pages (no code); .psh/.fx files are listed in Sapien. Opening the .shader gave hex for one user (H3). (gregory0701/gameoverload, 2022-08, texturing)
- H2A: texture-definition JSON has a "preset" field naming a shader; look it up in Shader Definitions in the Index rip, HLSL is polymorphic with a base shader plus a "common" file of math; mtl_blend handles material blending. JSON parsing is inconsistent (dict/array/string). (hausdev, 2022-11, texturing)
- Tool for source textures of H2 tags: SnowyMouse halo2-color-plate-extractor (https://github.com/SnowyMouse/halo2-color-plate-extractor); Reclaimer only exports post-processed textures. (zadeinator, 2024-06, texturing)
- H3 "ghost" render model shader lock: to give a duplicated biped (MP Chief) a different shader, either extract the model, rename shaders in Blender and reimport, or enable Guerilla/Foundation expert mode and edit shader names directly on the render model. Expert mode confirmed working. (zadeinator/lt_crunch236, 2024-06, texturing)
- Reclaimer error "System.IO.InvalidDataException: Found invalid data while decoding" opening Reach render_model: user had the maps in a custom folder under Reclaimer's Data/Halo Reach; cause not resolved in-channel. (waketheshark, 2022-11, texturing) - unresolved
- Cubemaps from Reclaimer: run through Flexify 2 and crop borders to get equirect. (soulburnin, 2022-10, texturing)
- Reach extracted textures can be corrupt/unreadable in some rips (jetpack, ammo pickups) - no fix given. (mikuspartan, 2024-01)

## 3. Channel layouts (colour change, control, masks)
- Reach colour-change texture (_cc) uses RGB-style zones; one user reported chief textures like spartan_chest_*_cc appear transparent: the colour is still there, set the colour space to Non-Color in Blender or force alpha to white in an image editor. (chiefster_4014, 2024-10; also 2023-10 cc alpha broke case, texturing)
- Reach cc/MP masks: "mpmask" is the colour-change mask; "hdetm" is height/detail map mask. (chiefster_4014, 2023-10, texturing)
- H4 spartan armor: base colour/diffuse sits in the GREEN channel of the _control texture; separate green in GIMP/Photoshop to recover it. H4 "RGB in control" can be dropped and alpha converted for Infinite conversion. (chiefster_4014 2024-11; spartanatic 2023-03, texturing)
- H5 control map (confirmed by author of Open Halo 5 shader): R = colour zone 1, G = colour zone 2, B = metallic, A = smoothness. "Fake third zone" is just the inverse of R and G. Conversion to Unity Standard: use zones as masks, move metallic to red channel for Unity. White fringes around zones come from strong compression plus mipmaps, not the shader. (fiycsf, 2023-06 and 2024-02, texturing)
- Some H5 models set control textures sRGB (Assault Rifle, Phantom), others Non-Color (Carbine, Spartans, Pelicans); H5 AMF extraction of MP Spartans crashed Reclaimer or output 445 KB; RMF worked. (theishikawarin, 2024-01, texturing)
- Infinite armor: Base Occlusion (ASG: R=spec? G=rough, plus grime mask), MaterialID 0 (zones 1-3), MaterialID 1 (zones 4-6), Base Normal. Zone 0 is the black base under all zones. Scratch intensity is one game value, divided per zone in the Blender shader. Seven colour zones from mask colours. (gameoverload, 2023-11 and 2024-02; r93_sniper 2022-09, texturing)
- Infinite gradientmask RGB: R white-on-black mask, G solid colour, B black-on-white mask; three colours per zone (Bot/Mid/Top) use those channels. (gameoverload/silent1meow, 2022-08 / 2023-01, texturing)
- Layout shorthand used for older color-zone masks: R, G, B, C, M, Y zones plus black and white for scratch/dirt. (gameoverload, 2022-07, texturing)
- Normal maps: always Non-Color; DirectX normals need the green channel inverted (Separate RGB, Invert G, Combine). Reach normals need the invert too. Blue channel of some Halo normals must be recalculated (node group in uploads, or fill blue white as a cheap fix). (many; soulburnin 2023-11; omnipotentdeba 2023-08, texturing)
- If a normal map is sRGB: shiny seams, e.g. Reach DMR seam fixed by setting Non-Color. (chiefster_4014, 2024-04, texturing)
- Alpha handling in Blender: set image alpha mode to "Channel Packed" when alpha stores data; otherwise textures look too dark. (chiefster_4014, 2022-09 / 2024-06, texturing)
- Halo 4 helmet normal texture rectangular - Substance squishes to square. (scarabaeidaeee, 2023-03)
- Infinite procedural: no albedo/rough/metal exist, only masks; bake from shader. (chiefster_4014, 2024-06 and 2024-10, texturing)

## 4. Colour-change / swatch colour reading
- Colours for campaign characters can be read from Assembly (biped tag) or the mod tools (ODST biped tag); IRTV or a material palette extractor for Infinite. (brooen, chromacore, chiefster_4014, 2022-07 to 2024-06, texturing)
- Infinite coatings JSON collection with Reach entries: https://github.com/Surasia/InfiniteCoatings (e.g. armor/reach/reach____rch_mil_jorge.json). Coatings past season 3 are very hard to pull. (surasia/scarabaeidaeee, 2023-01 / 2024-03, texturing)
- H3 Spartan blend: skinned vs rigid shader groups differ; only skinned versions have layer inputs for the coating system, so in-game objects switch rigid -> skinned. Infinite material layer shader reference not found for newer seasons. (gameoverload/urium86, 2023-10, texturing) - claimed
- Infinite MP armor changed from earlier black-detail look to single colour as shader versions changed (older shader chest detail differs). (kazmodeus, 2023-02)

## 5. Blender shader packs/versions and baking
- Halo Infinite Modular Shader (HIMS, chunch/AverageTrapEnthusiast): https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources releases MaterialTypes2.1, ModularShader2.3 (adds converters for older Halo games), 2.9 (fixed one value in all three colour slots). Older versions have no bake outputs; 3.0+ has them; 2.8 lacks bake. Upgrading shader version requires re-setup of material nodes: append new nodegroup, copy values and connections. (chunch, scarabaeidaeee, 2022-08 to 2024-03, texturing)
- Baking from HIMS: plug the desired baking output (colour, metallic, roughness...) into the Material Output surface, set Bake type = Emit for everything except normal; for normal plug the BSDF output and bake Normal. Add an unconnected, selected image node per material before bake (otherwise Blender overwrites another image node). One sample is enough unless using combined bake. Avoid the SimpleBake addon for nodegroup shaders. (chiefster_4014, gameoverload, chunch, 2022-07 to 2024-03, texturing)
- Baking issues: UV overlap/UVs outside 0-1 produce black or wrong bakes; make new UV map first. "Circular dependency" error on ASG input for meshes with two materials -> black bake (unresolved). (matta_klark, 2022-12)
- Reach Blender shader has no baking outputs (added later by soulburnin/gameoverload offering). Reach nodegroup credits: Soul (Falcon asset), JubSub. (gameoverload 2022-07, cythecy_ 2024-06)
- CR4B Tool (Completely Ready 4 Blender, PlasteredCrab) is the current importer for game-accurate H3/ODST/Reach materials: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool; Chief's old shader tools are outdated. (chiefster_4014, 2023-08, texturing)
- Blender 4.x changes: AgX default view transform vs Filmic changes colours; Blender 4.0 broke some older shaders. Set view transform back to Filmic to match older looks. (nihalrahman 2023-12; ianstl 2023-11)
- Missing textures after opening a pack: File > External Data > Find Missing Files, then reset colour textures to sRGB. (purplmunkii/chiefster_4014, 2023-06 / 2024-08)
- Color combinations: multiply nodes per mask: Mix 1 factor = primary mask, A = base colour, B = primary; Mix 2 factor = secondary mask, A = result of Mix 1, B = secondary. Each colour input needs its own camo texture node, not one for all three. (gameoverload, 2022-07 / 2024-02 / 2023-06, texturing)
- Infinite shader is heavy; Unity version limited to ~4 zones due to register limits; standalone bake app by xansworkshop.c pre-merges zones. (xansworkshop.c, chunch, 2024-02 / 2024-04, texturing)
- Unity/Unreal: HaloInfiniteVRCShader (https://github.com/R93Sniper/HaloInfiniteVRCShader, modified AGPL); Open Halo 5 shader for Unity; a UE5 project with Infinite shader pinned in #unreal-engine; Substance converter https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter. (various, 2022-09 to 2023-04)
- Infinite normals recalculated inside shader; do not need to recombine. (chunch, 2023-04)
- Reach fresnel visors: Fresnel node into Color Ramp into colour inputs of the HIMS shader. (chunch, 2022-12, texturing)
- Reach energy sword emission: plug control alpha into emission strength, colour into emission, multiply by ~10-20. (oodleberry, 2023-02, texturing)
- Additive transparency (Reach/CE style): add shader + transparent shader combined with the main shader. (soulburnin, 2023-05)
- Alpha cutout: Math node "Greater Than" on alpha feeding mix factor with transparent BSDF. (soulburnin, 2024-07)

## 6. CE / H2A notes
- CE shaders: diffuse only + cubemap with cubemap mask as Fac in a mixRGB; no real specular. (chromacore, 2022-11, texturing)
- CE shader by jackofchaos is public and follows source code; game-accurate H4 shader not feasible (see section 1).

## Open questions raised but unanswered
- Exact specular distribution/exponent used by Reach (Beckmann or not) and fresnel f0 handling.
- Do H2/H3 convert height maps to normals in-engine, or only Reclaimer?
- Cause of Reclaimer "Found invalid data while decoding" on Reach render_model from custom folder.
- Circular dependency error baking Infinite shader on two-material meshes.
- Where Infinite character skin shader and layer-7 shader references are stored (hex IDs).
- Reach terrain_blend tiling values via Assembly (answer: use Reach mod tools).
- Corrupt Reach jetpack/ammo texture rips.
