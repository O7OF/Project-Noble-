# Notes p1_01 — #general-help, 2022-08 to 2024-01 (chunk_01, 996 lines, read in full)

Slice character: almost entirely Blender asset-pack support (Infinite/Reach/H3 Spartan packs, baking, Unity/VRC/UE ports). Little tool.exe/Sapien/scripting/networking content exists in this slice; only rendering/shader/texture knowledge is substantial. Confidence tags: confirmed = multiple people agree or a tool author states it; claimed = single person; disputed = noted.

## 1. Texture / control-map channel layouts (H2/H3/ODST/Reach family)
- H3, ODST and Reach follow H2's texture set; per-game extras exist (r93_sniper, 2021-12, control-map-guide; re-quoted by gameoverload 2022-11/2022-12/2023-07). Confirmed.
  - Diffuse: RGB = colour, A = specular OR opacity.
  - Normal: DirectX format, so flip the green channel in Blender/Unity.
  - CC (colour change) map: R = primary colour mask, G = secondary colour mask, B = unused. (The pack-3 viewer for some H3 pieces also treated B as unused.)
  - Illum: RGB = self-illumination colour (a colour image used directly, not a mask). Not every object has one.
  - Material map: R = specular coefficient, G = albedo blend, B = environment (cubemap) contribution, A = roughness. When a Material texture is used the diffuse alpha normally holds no specular.
- Reach/H3 "control" is the community term for the map that holds colour and material masks (fiycsf, 2023-02).
- Cubemap alpha: gameoverload (2023-09) asked what the alpha of older Halo cubemaps does; unanswered here.
- H5 texture packing order is documented in #control-map-guide (gameoverload, 2023-08). H5 shader internals (sampler names, constants) are not known; accurate fixes need decompiled shader code (gameoverload, 2022-10). Claimed.
- H5 armour sets don't reach full white/black (colour range clamped by 343); this also happens in game (apexrainbow163, 2023-10). Claimed.
- H5 normals are DirectX-style; Blender expects OpenGL, so invert green. H5 roughness map may need inverting to feed a Principled roughness input (zidiath/tacohombre, 2023-09). Claimed.
- H2AMP shaders srf_pbr / srf_pbr_forerunner pack data per channel differently (forerunner: x = roughness, y = AO, z = metallic; standard pbr: y = alpha despite same map combo). Channel meaning is per-shader, not per-texture; reading it needs the shader code (zidiath/gameoverload, 2023-11). Confirmed principle.
- Halo 4 shader packing (Vec4: spec RGB + gloss/intensity in alpha) examples: Diffuse Colour, Specular Gloss, Metallic, Clear Coat, Emissive, Diffuse Wrap, Damage, Damaged Blend, Damaged Colour, plus "Spec Gloss / Metallic / Clear Coat override" floats. These are a different game's (likely a non-Halo title's) layout; the useful general point is that a Vec4 = RGB colour + a scalar in A (gameoverload, 2023-04).
- "Metallic" in these engines is effectively spec colour = base colour and spec intensity = metallic slider; nothing renders true metal (gameoverload, 2023-04). Claimed.
- Blender 4.x Principled BSDF split Specular Level and Specular Color; old BSDFs get an auto multiply node. Rough equivalence: Specular Level 0.5 (about 0.04 F0), spec colour about albedo x 0.04 (gameoverload, 2023-12). This is why pre-4.0 Infinite packs go white/stuck colours in Blender 4.0 (backslashmedia/soundflare/doomman, 2023-11). Fix: stay on 3.6 or use updated pack shader (HIMS 2.9+); cannot be fixed by just changing colours. Confirmed.

## 2. Colour-change / zone logic (what it implies for CC work)
- Reach/H3 colour change is two mask channels (R primary, G secondary) multiplied against a white/grey base diffuse; to do manually: split channels, colour each, multiply (sswampp, 2022-09). Confirmed.
- Base albedo for a CC'd item should be entirely white/neutral so the tint applies cleanly (gameoverload, 2022-11).
- Control map in a replacement asset must match the materials baked into the albedo; mismatched CC masks "look like hell" (gameoverload, 2022-11).
- Flood Elites have no colour-change maps; need manual edit (purplmunkii, 2023-09). Claimed.
- Unused 7th "damage" zone exists in Infinite coating data; MP armour leaves it unused (video description quoted, wake9090, 2023-02).
- Infinite coating import path (Olympus): `M:\__chore\gen__\objects\characters\spartan_armor\coatings\reach\reach_spartan_style{ct}.materialstyles`; material path `M:\objects\characters\spartan_armor\coatings\reach\` (wake9090, 2023-02; unconfirmed whether correct). Game-accurate coatings require Infinite downgraded to Season 2 with Olympus (plastered_crab, 2023-04). Claimed.
- Infinite emblems are projected images that overwrite the colour layer in the default colour zones, not the edge/dirt/dust layers (chunch, 2022-09).
- Infinite-shader zones need three separate colour inputs (Top/Mid/Bot): feeding one node (or one colour ramp) to all three renders silver/wrong; use three separate ramp/image nodes (many, 2023-05 to 2023-12). Confirmed.
- Switch that enables a zone colour must be on or the zone falls back to zone 1; Zone 1 can be copied to Zone 6 by matching RGB (gameoverload, 2023).
- Textures used as masks must be Non-Color; pink = unlinked image; normal/roughness/metal also Non-Color (many, 2022-2023).

## 3. Specular / env / reflection behaviour
- Older Halo shaders sample a cubemap whose look dominates final shading; for game-accurate Blender work the cubemap must be converted to equirectangular (cube2equirect) (soulburnin/plastered_crab/brooen, 2023-07 to 2023-08). Confirmed.
- Tools: py360convert (https://github.com/sunset1995/py360convert); a Blender Cubemap Converter add-on (video https://www.youtube.com/watch?v=dpMQr59vSys); manual option: UV a cube to match the cubemap faces (nihalrahman, 2022-10). Cubemap faces in the H3 set have different layouts per file; dark_grey one is already aligned.
- sswampp's Halo CE Blender shader (HCEShader.blend, NodeTree group "Halo CE Shader") expects a multipurpose map and an equirect map; black them out if absent; copy values from H1 editing kit shader tags (2022-10).
- Rank/emblem colour images for Elites are stacked cubemaps in 4 ranks with differing layouts (chalk33, 2022-10).

## 4. Shaders, game-accuracy tooling, and where shader values live
- CR4B Tool (PlasteredCrab) = Blender add-on that auto-builds H3/ODST (Reach in progress) material node trees from tag data: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool (renamed Halo-CR4B-Tool; tutorial https://youtu.be/749xwvg-GK4).
  - Requires H3/ODST mod tools and the unzipped tags.zip; point the add-on at the tags folder and the shader .blend; choose Halo 3 or Halo 3 ODST in the top menu then Start (plastered_crab, 2023-07/2023-11).
  - Unconnected shader structures show as red nodes needing manual fix, usually a missing detail_map (plastered_crab, 2023-03/2023-07). Only .shader supported; .shader_halogram etc. missing (2023-03).
  - Reach support only began 2023-08; Reach shaders contributed by Chiefster and Soulburnin; tool needs a major rewrite for Reach (plastered_crab, 2023-07/2023-08); in 2023-11 still "extremely limited" (tacohombre). CE-to-Reach import via its tool part was not documented yet.
  - Material names must be right for any H3/ODST asset to work; it reads tag binary from mod tools.
  - Gotcha: evactouch (2023-10) Reach import froze Blender until the correct AMF importer was installed; colour-management set to Linear/sRGB only caused an AMF colour-space error; fixed by restoring colour-management options.
  - Gotcha: tool wrote tag folders only to the "halo 3" folder rather than 3/ODST/Reach (2023-07 question, unanswered).
- Shader values source of truth: open the shader tags in Guerilla/Foundation (Steam mod tools: H1 1532190, H2 1613450, H3 1695791, ODST 1695794, Reach 1695793 Foundation, H4 1695792) or Assembly (brooen 2023-05; sswampp 2023-07). Per-material values must be compared one by one (plastered_crab 2023-10).
- Rip tools (brooen 2023-05 summary): CE/CEA/H2A via IndexV2 (https://github.com/Wildenhaus/IndexV2/releases); H3/ODST/Reach/H4/H5 via Reclaimer (https://github.com/Gravemind2401/Reclaimer/releases). H5 shader data via a Reclaimer plugin. Index does not support H2A multiplayer maps, campaign only (chiefster_4014, 2022-10), though some MP bits appear (hydranaught444).
- Prefer exporting AMF over OBJ from Reclaimer: AMF preserves mesh normals; OBJ gives sharp-edged meshes (chiefster_4014, 2023-09). Reclaimer rips lacked custom split normals ("blocky mesh", gameoverload 2022-11). Reclaimer also includes vertex colour in Collada but meaning depends on shader (gravemind2401, 2022-10).
- Reach models: "Reclaimer itself is enough" to extract; Reach MP maps can be opened in Reclaimer after downloading map files (sswampp 2023-07; plastered_crab 2023-11).
- Some H3 Elite textures are swizzled and must be unswizzled before use (plastered_crab, 2023-07).
- Reach animation ripping exists via tutorial but was never finished, not all animations supported (plastered_crab, 2023-01).
- No way to recover source tags from compiled H3 maps (r93_sniper, 2023-11). Confirmed.
- .fxo compiled shader effect files cannot be fully decompiled (gameoverload, 2022-10); DXVK compiler + SPIR-V suggested but not applicable (coarforge).
- Reclaimer error when porting shaders from H3 construct.map: `ArgumentOutOfRangeException ... Index was out of range` for levels\solo\070_waste\shaders\waste_panel_pipe_blue.shader, then bsp and scenario (user, 2023-12). Unanswered; possibly missing bitmaps/bink/info beside the .map.
- H3 xbox render models failing to open in Reclaimer-type viewer: the xbox folder lacked two required maps (xscruffydasasq, 2022-10); H2x/H2v were fine.
- Sky/vehicles/animation: Halo Reach attack effects ("heart attack"), target locator etc. are compiled in-engine as vectors with tweakable parameters (purxfilms, 2023-12). Claimed.
- Reach Forge terrain/blend textures are very complicated in H3/Reach (plastered_crab, 2023-10); water also needs separate setup.
- Reach/H3 Forge-in-Blender tool exists: tutorial https://youtu.be/Q_LMjZkp1jo (plastered_crab, 2023-11).
- Reach scenery `.amf` import: the Blender AMF add-on from an old tutorial stopped working (olipog 2023-06); ensure the current AMF importer.

## 5. Baking and cross-engine export (Unity/UE/VRC)
- No export format carries Infinite-style shaders (r93_sniper 2023-08); bake textures or rebuild shader. Reach is "not plug and play" in Unity; needs custom shaders (gameoverload 2023-05). Confirmed.
- Reliable bake method (gameoverload 2022-12, 2023-01): create an unconnected Image Texture node with a new square image (e.g. 2k), keep it selected in every material's node tree, wire the value you want to bake into the Material Output Surface, bake with the Emit option, save the image, repeat per channel (diffuse, metallic, roughness). Bake normal with the Normal bake type.
  - Bake from the shader's own end-node outputs (e.g. the pack's "Metallic" output), not from the Principled BSDF. Chromacore (2023-11) says baking directly from the BSDF works for normals and the shader's normal output was a leftover debug output.
  - All-black bake results: usually missing the selected target image node or Cycles settings; use Cycles (Eevee differs).
- Material Preview uses a preview HDRI; Rendered view uses the World shader. Different looks between the two = World/HDRI setup (gameoverload 2023-10).
- Infinite camo/silver render bug: same image node into three colour inputs (see 2); also set Cycles not Eevee.
- Unity: import textures separately, albedo+metallic/smoothness works with Standard shader for simple props (r93_sniper: metallic 1, smoothness about 0.9 for single-material); H3 shader for Unity is in #shaders-resources; Reach shading is similar to H3 (r93_sniper 2023-08).
- UE: do not invert normals unless baked OpenGL; turn off sRGB on non-colour maps (gameoverload 2023-10).
- FBX carries mesh+armature+UVs+embedded textures; OBJ only mesh+one UV; glTF open alternative (gameoverload 2022-11).
- Textures going pink on import: missing HDRI or unfound paths; reassign manually in smaller batches to avoid Blender crash (gameoverload 2022-12).
- Hybrid-graphics laptops can break Blender display: force the dedicated GPU (coarforge/sswampp 2023-07).
- Mixamo + Reach FBX: knife detaches from the body; no fix given (matrixbearer 2023-03).

## 6. Rigging / skeleton / armour assembly (Blender packs)
- Reach pack helmet attach: delete the helmet's own rig, select helmet then rig, parent with "Empty Groups", or enter pose mode, select head bone, parent to Bone (setriaphtfk, 2023-12). Claimed.
- Chest-knife bones jump to origin in edit mode (Reach pack) (caldoric, 2023-09); Spartan Pack 3.5 shader breaks when tabbing into edit mode (deltahalocontr, 2023-07); Reach marine bones are hidden after append (thoro.blend 2023-06). Unresolved workarounds.
- Mixing skeletons from Reach/H3/Infinite is unlikely to sync unless all share a skeleton (gameoverload 2022-10).
- ODST models from H3 rips typically have broken skeletons (stacked horizontally) and bad weights (sp_220_hawkfir 2023-04).
- Infinite Elite rig: Hand IK bone does nothing; upper arm bone doesn't move the arm; IK-to-FK conversion export loses finger movement (pajamas007 2023-02, commanderprime 2023-07, backslashmedia 2023-01). Zang's elite rig mirrors poses (symmetry on) (nihilmancer 2023-03). Unresolved.
- Duplicated armour pieces share one colour group (evenmanatee887, 2023-04): make materials single-user. Copy shader nodes between pieces (exporush 2023-02).
- Vertex counts: H3 can handle ~36k verts per object, fewer in H2/CE; Reach and up can handle more (hydranaught444, 2022-10). Claimed.
- Quest-rated H3 Spartan is ~6000 tris for whole MkVI (chromacore 2023-04).

## 7. Importing custom assets into the engine (the little there is)
- Importing into Reach: tutorials exist for H3; no Reach import tutorial seen (m0nkeyman117, 2023-08). Reach/H3 Mauler example: a .blend used for a Reach export had an extra "Default Mauler Body" vs "render" object question (chiefster_4014, 2023-09); the exported mesh should come from the render object.
- Porting cross-game: sentinel/H3 attempts gave "close enough" CC by painting own CC map; H3 cannot do some Reach shader functions (hydranaught444, 2022-11).
- Sentinel beam shader is buggy when ported (hydranaught444, 2022-12). H3 thrust shaders use only illumination maps. Hologram effect = ordinary model + hologram shader (hydranaught444, 2022-11).
- MCC map restore: tag edits not appearing in compiled Reach maps (whatsdisc0rd 2022-12, unanswered). 
- Halo CE multiplayer gun sounds location and H1 dedicated-server/SAPP setup were asked, not answered (clericg 2023-01; .andythegreat 2023-06).
- Port 80085 used by _gallium for internal testing (2022-09), no Halo relevance.
- Halo 2: `halo2-color-plate-extractor` exists for batch bitmap extraction; no equivalent known for HCE/H3 (dgenpoly, 2023-11).

## 8. Reference, links, misc
- c20 wiki (H3 tag docs): https://c20.reclaimers.net/h3/
- Halopedia map file page: https://www.halopedia.org/Map_file
- Infinite shader releases (modular/material types 2.1): https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases (MaterialTypes2.1 tag redid roughness).
- HDRI sources: polyhaven, ambientCG SkyOnlyHDRI032 (gameoverload 2023-06).
- Reach armour hex codes from the Halo Archive resource (video https://www.youtube.com/watch?v=vVS9HJd73io); Cylix armour guide https://cylix.guide .
- Reach height maps in terrain: probably blend/detail data; must be read from tags (gameoverload 2023-07).
- gameoverload "Practical Shading Compendium" planned (2022-11) and offered XCOM2/Chivalry master-material replicas; not Halo.
- Zone-colour mask technique for new armour: mask 0/mask 1 RGB zones, ASG (AO/scratch/grime) texture; Substance Painter only needed for new masks (gameoverload 2022-12).
- Halo 4 "Storm" objects are alternate variants of Marine/Elite; reason unanswered (mechdragon108 2022-12).
- Corridor Digital "Halo: Cold Storage" seeking riggers/animators (2023-09, Rokoko mocap).

## Open questions raised but unanswered
- Reach cubemap colour/alpha: what does cubemap alpha do in H3/Reach shaders (gameoverload 2023-09)?
- Why do compiled Reach maps not reflect tag edits (whatsdisc0rd 2022-12)?
- Where are CE MP gun sounds in MCC; how to set up CE dedicated server + SAPP (2023-01, 2023-06)?
- Reclaimer ArgumentOutOfRangeException on H3 construct.map shader port (2023-12): cause?
- Reach import into engine tutorial (2023-08): none seen.
- Why CR4B writes tags only to the "halo 3" folder; exact Reach shader support state.
- Reach animation ripping: which animations unsupported?
- Fix for edit-mode shader break (Spartan Pack 3.5) and chest-knife bone jump (Reach pack).
- Halo 5 sampler/constant names for faithful shader; H4 shader internals.
- Reach coating path correctness for Olympus (wake9090 2023-02).
