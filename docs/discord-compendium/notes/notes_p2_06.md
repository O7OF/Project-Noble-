# Notes p2_06 - second pass over chunk_06 (texturing channel, 2022-07..2024-11)

Scope warning: this chunk is a Blender/Unity shader-conversion channel. It has NOTHING on tag-level custom armour import (render_model, markers, tool import commands), NOTHING on networking/multiplayer sync/anti-cheat, and almost nothing on in-engine Reach tag values. Everything below is Blender-side or reading-tool-side. Tags: (author, YYYY-MM, texturing).

## Missed facts

Reach colour change / cc / mask handling
- The Noble Team model pack by CanadianDragon showed white armour except Jorge's shins; diffuse textures held only detailing (dark ridges, transparent elsewhere) and colour came from a separate input; materials using .tiff worked, ones using .png did not. Unresolved in-channel, but it is the same symptom as a cc/colour-change input not being wired or not loading. (_tobyr, 2024-01, texturing)
- Reach Spartans Blender pack: the visor colour is driven by the PRIMARY colour of the visor material; select the helmet, pick the visor material, and the three colours under Zone 1 control it. (zattlebone, 2024-10, texturing)
- Bright purple/pink armour with Reach Spartans pack = images not found, not a colour bug. Fix: File > External Data > Find Missing Files, point at the pack folder; Spartan pack link used was a Google Drive file named "Reach Spartans.zip". (purplmunkii 2023-06; urgedfutuer117 2024-10; chiefster_4014 2024-08, texturing)
- Reach cc texture symptom, user side: a cc texture whose alpha was 1 "should be zero" in the user's view; after editing, the file turned into a grey checkerboard and the material acted as if the cc slot were empty even though it said a texture was slotted. gameoverload only said to inspect what the shader does with that alpha. (caldoric, 2023-10, texturing)
- Reach grenadier chest cc (spartan_chest_joshua_cc.png) appears fully transparent and made colours wrong; answer was it is normal for that file: alpha is simply stored as 0, so either set image colour space to Non-Color in Blender or set alpha to white in an editor. (gwenwyn / chiefster_4014, 2024-10, texturing)
- For the HIMS-style multi-input camo setups: feeding ONE camo texture node into all three colour inputs breaks that shader version; give each colour input its own node (fixed for several users). Also the 2.9 release fixed one value feeding all three slots. (gameoverload 2023-06 and 2023-07; chunch 2023-07, texturing)
- To use separate RGB zones from a mask you need a separate-colour node per channel; Substance's own colour picker is poor, so split the mask in an external editor and import each channel. (soulburnin 2023-05; purplmunkii 2024-10; masterj2001 2024-10, texturing)
- Colour space discipline for packed Halo textures: control/ASG/mask, normal and roughness in Non-Color; only base colour in sRGB. When upscaling or editing packed maps, export RGB as sRGB but alpha data as linear, and do not upscale value maps by splitting channels carelessly (H5 grunt control broke this way). (tacohombre 2023-04; gameoverload 2023-12; chiefster_4014 2023-11, texturing)
- Chunch noted that in Substance each colour mask has to be separated and gamma-corrected manually when converting. (chunch, 2023-01, texturing)

Shine, specular, fresnel (Reach and relatives)
- Whitish rim glow on Reach assets in the community Cook-Torrance nodegroup ("HRCategory: cook torrance") comes from the glancing/fresnel colour input; soulburnin's mapping: bottom colour = specular tint, top = glancing, exponent = steepness. This was asked because the glow looked wrong, so turning down the glancing colour or tuning the exponent is the lever (inference, not stated outright). (cythecy_ / soulburnin, 2024-08, texturing)
- Practical way to fill those inputs: open the shader in Foundation, take the numbers listed under the Xenon entries, divide by 255. (chiefster_4014, 2024-08, texturing)
- Reach/H3 game formula for spec colour (as given): final spec = specular texture x (analytical spec coefficient x spec tint) x final tint; texture itself is coloured, not a packed channel. Open follow-up: how to author such a map was asked (just add colour to greyscale?) and not answered. (soulburnin / matta_klark, 2024-07, texturing)
- Blender-side shine fixes seen: lower roughness or add clearcoat (HIMS layer shader has no clearcoat); clamp every node leading into BSDF roughness inputs, which cured white dots on shiny parts of an H5 shader; suggested clamping roughness and metallic in HIMS because extreme swatch values cancel grime layers. (gameoverload 2023-04; chiefster_4014 2024-02; zoptic 2022-07, texturing)
- Silver visor turning into a mirror when colour set to grey: asked, unanswered; one tip was a Fresnel node mixed into a second BSDF. (matt161 2023-05; purplmunkii 2023-10, texturing)
- Reach energy sword not glowing was asked in 2024-04 with no answer; the emission-from-control-alpha tip (2023-02) was hedged and not confirmed to be about Reach.

Darkness / black pieces (non-tag causes seen)
- Texture appears much too dark in Blender when its alpha holds data: set image alpha mode to Channel Packed; removing alpha from a colour map and using a real metallic map also cured it. (chiefster_4014 2022-09; rafaleeater 2022-09, texturing)
- Reach Warthog turret metal rendered black in Cycles only: cause was Cycles light-path settings (glossy, transmission, volume bounces/visibility) unchecked. (trisckit, 2024-05, texturing)
- Reach Paris-class texture "too dark": possible causes given were colour space set to Linear, or game brightens it in engine; also try weaker lights and matching world colour. (powerhousegami / chiefster_4014 / khaalida, 2024-04, texturing)
- Spartans pack 3.5 version: edit mode strips shading to base colour only, and textures going all black after appending parts; 2.0 pack does not. Blender 4.0 also broke older shader nodegroups. (earnestbrick2 2023-04 and 2023-06; pmarc0011 2023-07; setriaphtfk 2023-11, texturing)
- Spartan all black whatever colour is chosen after following the CR4B/Plastered Crab tutorial: unresolved. (.danielshaver, 2023-11, texturing)
- Colour-ramp alpha: a black colour-ramp stop with alpha 1 does nothing for transparency; set that stop's alpha to 0. (chiefster_4014, 2024-03, texturing)
- H2A marines: chest packs and shoulders darker than other armour despite identical shader; owner of the old H2A shader said the shader is outdated; poster suspected the models. Not resolved. (josephfilmogra / gameoverload, 2023-08, texturing)
- Elite specifics: Elite head object carries a separate teeth material (easy to edit the wrong one); Elite General from the drive came pink (missing textures); H2A Elite armour on HI Elites pack showed primary colour stuck at white and the picker only moved toward black. (chalk33_8677 2022-11; nascarlaser1 2022-09; inversefrog768 2023-12, texturing)
- A shared Elite pack was released by Chiefster with the 2.0 shader; a 3.0/4.0 version was described as in progress (specular and colour working). (gameoverload / chromacore, 2022-09, texturing)

Tools, links, versions
- 405th forum "Procedural Armour Texture Setup for Blender" (version 2 from 2023-02-22): https://www.405th.com/forums/threads/procedural-armour-texture-setup-for-blender.53457/ (planetalexande, 2023-02)
- Reach Custom Spartan Creator tutorial: https://youtu.be/vVS9HJd73io (spencerbutler, 2023-02)
- Coating porting tutorial https://youtu.be/r_5suSLUucU; CR4B setup video https://www.youtube.com/watch?v=749xwvg-GK4 (chunch 2022-09; chiefster_4014 2023-08)
- The CE shader public release follows the engine source (jackofchaos); H4 and Reach community shaders differ in how close to game they are.
- Reclaimer: cubemaps via Flexify 2; Reclaimer brings TIFs along with AMF exports for Blender import (cythecy_, 2024-06).
- Diagnostic of a decoder error: Reach render_model "Found invalid data while decoding" was reported while maps sat inside Reclaimer's own Data/Halo Reach folder; no cause given. (waketheshark, 2022-11)
- Viewing HLSL for Infinite: shader code from current tools is DXBC converted to DXIL/SPIR-V, with DXIL giving register info and the Vulkan output the code; translations are mangled. (gameoverload / xansworkshop.c, 2024-02)
- Swapping MP chief shader on a duplicated H3 biped: expert mode (Guerilla/Foundation) lets you change shader names on the render model; Blender extract-and-reimport also works. This is the only render_model-level fix in the chunk and is relevant to custom armour variants. (zadeinator / lt_crunch236, 2024-06)
- H3EK can extract models directly. (autarchkatherl, 2024-07)

## Corrections to pass 1

1. Pass 1 section 4 says the skinned/rigid shader-group discussion is about "H3 Spartan blend". It is about Halo Infinite (HIRT, season coatings, layer system, urium86 and Season 5 layer 7 shader). Not H3. (gameoverload / urium86, 2023-10 to 2023-11)
2. Pass 1 says height maps become normal maps "in H3 and Reach". r93_sniper only said H2 and H3. Reach was not named. (r93_sniper, 2022-11)
3. Pass 1 says the Reach Blender shader got baking outputs "added later by soulburnin". Not supported: soulburnin only said he would look into it before release (2022-07), and in 2023-07 gameoverload still said the Reach shader may not be able to bake, so the bakeable outputs belong to the Infinite shader.
4. Pass 1 states the exponent can be read by clicking the specular colour as fact. Source (soulburnin, 2024-01) phrased it as "I think... should be called exponent or something".
5. Pass 1 heading "Reach fresnel visors: Fresnel into Color Ramp" is about getting a Reach/H5-style fresnel on INFINITE visors in HIMS, not about the Reach shader. (blue.nasca / chunch, 2022-12)
6. Pass 1 Infinite ASG "R=spec? G=rough" came from a user's belief (onemadwomble, 2023-04), not an answer; only the MaterialID 0/1 zone layout was confirmed (gameoverload, 2023-11).
7. Pass 1 "Reach energy sword emission" tip was from a single hedged answer with no game attribution; the Reach sword glow question later (2024-04) stayed unanswered.
8. The IOR/f0 remarks (below 1 no tint, 0.04) describe the Infinite metal-rough shader per gameoverload, and the 0.04 line is garbled; Reach's fresnel statement is the separate soulburnin line. Pass 1 flagged most of this; do not use IOR for Reach.
9. "Halo 4 helmet normal rectangular" is the H4 ODST helmet (scarabaeidaeee, 2023-03), not generic.

## Contradictions between speakers
- cc alpha: caldoric treats cc alpha=1 as wrong (should be 0), chiefster says alpha 0/transparent is just how the file is and to force white or use Non-Color. Different tools, same file family.
- Reach shader approach: chunch (2022-08) says just use a Reach-specific shader; tpg (2022-07) says Reach needs a custom shader in both Blender and Unity because its map system differs.
- Cook-Torrance: soulburnin says Reach is mainly Cook-Torrance with roughness; nihalrahman doubts Beckmann in a game that old; never reconciled.
- Specular texture: ODST skin tint: gameoverload guessed DXT artifacts, chiefster guessed shader tint, soulburnin said it is the real colour of the texture.
- Normal colour space: gameoverload said masterj2001's baked normal was sRGB, masterj said he bakes Non-Color (EXR); chiefster earlier called sRGB normals the DMR seam cause. (2024-04 and 2024-08)
- Guerilla shader viewing: fable_law_458 says Guerilla shows shader code in H2/H3/ODST; gregory0701 sees only parameter pages; gameoverload found Guerilla unhelpful for identifying the shader.

## Best leads for these problems
1. Black or too-dark Reach Elite shoulder: (a) check the cc image is actually loaded and set Non-Color; alpha of 0 is normal for Reach cc files (gwenwyn, 2024-10); (b) in HIMS-style setups use a separate node for each of three colour inputs (2023-06/07); (c) alpha mode Channel Packed (2022-09); (d) Cycles glossy/transmission/volume paths (2024-05); (e) normal map as Non-Color, with green inverted for Reach (2023-08, 2024-04); (f) missing image paths show pink, not black (2023-06). Nothing here pins the Honor Guard shoulder specifically; no tag-side cause (CC alpha, spec mask tint) is documented in this chunk.
2. Elite shine: Reach spec = coloured spec texture x analytical coefficient x tint x final tint; fresnel colour is blended over the tint-albedo mix; exponent from mod tools (soulburnin 2024-01, 2024-07, 2024-08). A too-white sheen suggests the glancing colour or its exponent (inference).
3. Importing custom armour to Reach: only one relevant item, expert mode in Guerilla/Foundation to rename shaders on a render_model (zadeinator, 2024-06). Shader source and parameters are readable in the MCC Reach mod tools (gameoverload, 2024-06, 2024-01). Nothing on markers, regions, tool commands.
4. Networking of custom armour for other players: no information in this chunk.
