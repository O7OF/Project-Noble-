# Notes p1_03 (chunk_03.txt, lines 1-1062 fully read)

Source mix: #general-help (2026), #autodesk (2021-2025, Max/Maya), #substance-su (2022-2024). Nearly all Blender/DCC-side. Almost nothing on tool.exe, Sapien, scripting, AI, networking, lightmapping, or Reach tag editing appears in this slice. Confidence tags: confirmed = stated with a first-hand fix; claimed = single-person assertion; disputed = people disagreed.

## Ripping / importing tools, versions, links
- Reclaimer release v2.1.1624 adds support for extra H3 and Reach Alpha builds (bitmaps and render_models only) and fixes some H2 "bitmap format not supported" errors. https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1624 (exodst., 2026-04, general-help) [confirmed]
- Reclaimer needs its H5 branch to pull Halo 5 weapon meshes correctly (r93_sniper, 2024-07, substance-su) [claimed].
- Tool-per-game list: HIMS and HIME for Infinite; "Index" for H2A; Reclaimer plus official Mod Tools for the other games; no CEA tool as of 2022 (gameoverload, 2022-09, autodesk).
- Halo Animation Repository (ripped MCC animations): https://github.com/77Mynameislol77/HaloAnimationRepository . Baboon (Rust GUI built on blam-tags, extracts animations): https://github.com/Zoephie/Baboon . Retargeting Reach marine anims to another rig works if armatures and bones are lined up (koax117, 2026-08, general-help).
- Foundry (Blender addon, Crisp): https://github.com/ILoveAGoodCrisp/Foundry . Per #general-help it covers Reach, H4, H2AMP; the Reclaimers-library guides cover H3. (nihren / haloismylife65, 2026-03/04)
- Mjolnir-Forge-Editor (Blender tool to manipulate Reach Forge objects in MCC; one user used it to rip a map live): https://github.com/Waffle1434/Mjolnir-Forge-Editor (koax117, 2026-10, general-help). Unknown whether Foundry supports Forge.
- Cr4b Tool (H3 shader setup addon) was used for the H3 Ark pack. Known bug: running it on the Ark messes up the material of H3 bsp_050 "structure_catwalk_control_lift" shader "panel_glass" even though UVs and textures are unchanged (koax117, 2026-04). Unresolved. A different addon (General101's) broke over time; its author named a variable `this_is_a_horrible_hack`; no fix known, ask in Halo Mods / Halo Modding Reclaimers servers (chiefster_4014, 2026-06).
- Ekur (Infinite importer): https://github.com/TheHaloArchive/ekur . Install multiplayer first, then the campaign: campaign needs its own data folder and the "Is Campaign" addon preference ticked, and Forge import is then unavailable. Material names must remain the numeric strings, since Ekur identifies shaders by name; shader errors usually mean renamed or broken shader names. If only the visor shows colour, check the campaign/multiplayer setup and the full deploy steps (koax117, scalemaster117, 2026-04 to 2026-09). Coatings imported via Ekur may use Infinite shader v3.1.1 rather than 3.1.2 (spyder6300, 2026-10, unanswered).
- Halo CE Campaign Evolved (CER) rips via FModel: the elite body crashes FModel in one report; the Minor elite fails with "detex failed to decode texture" for armour/skeleton (2026-07, unanswered).
- Halo Reach audio rips: https://sounds.spriters-resource.com/xbox_360/haloreach/ ; H2: https://sounds.spriters-resource.com/xbox/halo2/ (koax117, 2026-06).

## Colour change / armour colouring (Blender packs)
- Reach, H3 and Infinite packs: colour zones are changed in Blender's Shading tab, per mesh, via the material's shader/node group. Meshes can have several material slots; each must be edited. If zones stay black, un-hide all geometry (Alt+H) and check every slot (koax117, chiefster_4014, 2026-04/06) [confirmed].
- H3 pack: a top-level colour node group with primary and secondary slots drives all armour pieces together. Tab into the group and use its colour wheels. Unlink it when mixing parts from different sets. Custom properties and drivers can be linked to colour sockets (gameoverload, koax117, tomatoatovt, squeak_grenade, 2026-09).
- The cubemap/env reflection in the Bungie shader is never tinted. It is added onto the diffuse either before or after the detail map is applied to the base map, depending on a flag in the shader tag. Master Chief's brown looks different in Blender if that order is wrong (chiefster_4014, 2026-06) [claimed by someone who has examined the tags].
- Infinite decals: the HIMS shader has two decal inputs at the bottom (colour and alpha) (chiefster_4014, 2026-08). In Substance, Infinite wear and decals come from the ASG texture: green channel for subtracting decals, blue for wear/multiply, red channel is baked AO (file1431, chunch, 2023).
- Halo 3 and Halo 2 Arbiter armours and Halo 5 Arbiter are not made colourable; custom work is needed (winnower, 2026-04).
- A Reach colour-code text file ships with one of the asset packs (koax117, 2026-08); no canonical table was given.
- Infinite: the control map red channel on the torso covers both the armour and the techsuit, so they cannot be separated by channel alone. Split them at the submesh level or make a new mask (ubald007, 2021-12, autodesk).
- Halo CE elite (Gearbox multipurpose map) armour and undersuit are two materials; mask or split them. The multipurpose alpha was tried as env mask without success (retrogasm2000, r93_sniper, 2024-07, unanswered).

## Shader behaviour, normals, textures
- Normal map artefacts in Blender: set the image to Non-Color and flip the green channel, or on Blender 5.1 use the Normal Map node with DirectX (gameoverload, 2026-04) [confirmed]. Bump node strength depends on model scale; textures must use the model's UV map, not Generated coordinates (chiefster_4014, 2026-08).
- Infinite normals are DirectX-formatted (r93_sniper, 2024-04, claimed). In Substance, set Normal Map Format to DirectX when importing them, then export as needed. To rebuild a missing blue channel in Substance, switch the Z output from alpha to blue.
- Several Infinite textures hold several tiles in one long strip; the shader works out U and V block counts. The texture JSONs give the block counts per texture (gameoverload, 2026-07).
- Blender cannot expose light/render pass data in the shader editor, which is why Bungie-era custom render functions can only be approximated. Blender 5.3/5.4 may improve this (gameoverload, 2026-09). A 1:1 Reach shader attempt by chiefster_4014 was still "messed up" (2026-09).
- H2 shaders: remake the shader templates and passes yourself (H2 shader code is hard to read), then read values from the .shader tags. H3 skies are .render_model tags in the scenario's sky folder (chiefster_4014, jackofchaos, 2024-03, 2026-10).
- Generating spherical-harmonic coefficients for an H3/Reach-style shader outside the engine was an open request (jackofchaos, 2024-03).
- Halogram shaders use heavily animated texture layers. Foundry has a Reach halogram shader. To find missing textures for an H3 object, open its render_model/shader in Guerilla (koax117, 2026-06).
- Ammo-counter digits in the Reach AR Blender shader are driven by a custom property on a bone through a driver (theonedutchguy, 2026-10).
- Looping animated textures: tutorial https://www.youtube.com/watch?v=0uMWdlz1ecc ; H3 assets animate two noise masks.
- Mac Blender has lower sampler limits, so not every texture loads in complex shaders (gameoverload, 2026-03).
- Cycles-only artefacts (stripe down ODST helmet, white armour, jaw edge lines) were reported but never explained here.

## Rigs, skeletons, animation, export to other DCCs
- Blender FBX never embeds the custom shaders or textures; GLTF can carry textures. For Maya/Unity/Unreal either bake textures from the shader or rebuild the node group (gameoverload, chiefster_4014, 2023-2026) [confirmed by many].
- Baking recipe: put an image node in each material, bake each pass (about 3-4 setup steps), export the mesh as FBX or DAE, rebuild materials in the target app (gameoverload, 2022-09).
- Blender-to-Maya exports commonly scale 1000x or 25400x; reduce joint radius and expect the root joint to turn into a locator (r93_sniper, frostbitecinem, 2022-06). Export armature and meshes only (no leaf bones, no IK poles); DAE was recommended over Blender FBX (r93_sniper, 2024-03). Better FBX addon, based on the official SDK, reads FBX 2014/2016/2018 and Unity files correctly where Blender's native one fails (gameoverload, 2024-03). Blender's native FBX is reverse-engineered, not SDK based. Units are always set to metres regardless of export settings (r93_sniper) [disputed: ubald007 reported newer Blender FBX imports cleanly into Max 2025].
- Whole-rig transfers between DCCs are usually not worth it; export mesh and skeleton only and re-rig (aegismind, kalvlidan, 2023-2024). Do not export both the IK and FK rig layers together (ubald007, 2024-04, claimed).
- AMF files need Max or Blender (r93_sniper, 2022-10). A Max AMF importer script exists but is janky; baconator650 got it working with Physical Materials (2022-08).
- Imported Halo animations load into an animation layer in Blender; in Max use the Animation Layer toolbar for non-CAT animations (baconator650, 2022-08).
- Retarget shoulder mismatches: combine upper arm with chest, merge by distance, reweight, then separate (surasia, 2022-03).
- Halo default rigs do not mirror cleanly (draken_warlord, 2026-09).
- Porting Infinite to H3 or Halo 4: reposition to the target proportions and default pose, rename weights to the target bone names, merge weights (H3 has far fewer bones), bake PBR textures, then export with an addon (chiefster_4014, 2026-08, claimed).
- Baking IK into exported Reach elite animations (idle feet drift) was asked; no answer (winnower, 2026-08).
- Finger posing in Blender: set pivot to Individual Origins and rotate selected bones together (jellybox., 2026-07).
- Maya strips "+" from names, so a "+sky" material cannot be named directly. The Blender toolset avoids this with a material flag that adds the prefix at export; a Maya workaround would be a rename-on-export script, or porting the Blender JMS exporter Python (r93_sniper, dgenpoly, 2023-05).
- FBX exports with missing faces or odd lines were fixed by cleaning stray edges and checking smoothing groups, tangents and binormals (sketch10121, 2022-03). Maya/Arnold flipped-normal patches come from flipped UV shells; fix by mixing inverted and normal maps with vertex-colour masks (theonedutchguy, 2022-01). Halo UVs are often in tiled space outside 0-1 after conversion, not flipped (r93_sniper).

## Substance Painter / UVs
- Many Halo models have UVs outside 0-1 or mirrored halves offset by 1 UV unit; for texturing, move islands back inside (-1 on U), then re-import via Edit > Project Configuration > select the FBX (fiycsf, r93_sniper, oodleberry, 2022-2024).
- For AO bakes on mirrored shells, offset the mirrored portion by exactly 1; check for overlap and clipping; adjust front/rear projection (r93_sniper, 2023-02, 2024-05).
- Use Infinite's shipped AO (red channel of ASG) instead of re-baking (fiycsf, chunch, 2023-02). Halo 5 MP armour has no AO and no recoverable normal maps for non-Spartan characters; clear mesh normals for soft-shaded rips (fiycsf, 2022-08).
- Export Blender meshes with modifiers applied when guns look broken in Substance (purplmunkii, 2024-07). Do not tick the UDIM option unless using UDIMs (gameoverload, 2022-09).
- HaloInfiniteToPBRMetalRough-SpecGloss converter (Substance file taking ASG, control 0/1, normal): https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter (2023-04).
- Colour hex values look wrong in Substance because of sRGB vs linear and other layers; check Base Color view (tpg, r93_sniper, 2022-05).
- Parallax does not work in VRChat (forward rendered); use detail normals or bump (gameoverload, 2023-03). Material count matters for VRChat draw calls.

## Misc
- Mixed-up channel notes: Infinite control maps and ASG layouts only; no Reach CC channel layout discussed in this slice.
- 3ds Max retargeting: avoid position constraints (r93_sniper, 2022-07). MayaCrasher (force-crash a frozen Maya to get a recovery file): https://github.com/carpikes/MayaCrasher
- #autodesk was retired on 2026-07-21; use #software-help and #software-resources.

## Open questions raised but unanswered
- How to remove or hide only the "UNSC" decal on the Reach RECON helmet while keeping the side triangles.
- Why Cycles shows a stripe/edge lines on some ODST/Sangheili meshes.
- Which Reach elite models map to which voice actor, if any tag links them.
- How to bake IK into exported Reach elite animations.
- FModel crashes and detex errors on Campaign Evolved elites.
- Cr4b Tool breaking H3 panel_glass material.
- Where to find H3 masterchief.shader values (roughness, specular strength).
- Env/cubemap reflection of CE colour variants on the Gearbox multipurpose map.
- Getting Reach/H3 terrain into height maps for World Machine.
- Ammo counter animation for shaders lacking driver support (answered only for the Reach AR).
