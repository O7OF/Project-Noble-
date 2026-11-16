# Notes p2_03 (chunk_03.txt, all 1061 lines re-read; additions/corrections only)

Scope warning: this slice is Blender/Maya/Substance asset-pack chat. There is nothing on tool.exe, tag editing of shader/change-colour fields, lightmaps/bsp, scripting/AI, anti-cheat or networking of custom pieces. Leads below are therefore indirect.

## Missed facts

Black / unpaintable pieces (Blender packs)
- A user whose attachment OR base armour turned black suspected two "bake OV" maps in their data; no reply (shippudenleaf7, 2026-03, general-help).
- Two users reopened a Reach/other Spartan file and had pieces fully black that would not take a colour even after the purple missing-texture problem was fixed with koax117's help; the black stayed (vexerart 2026-08; _pandoras 2026-08; general-help). Unresolved. So "fix missing textures" is necessary but not sufficient.
- chiefster_4014 (2026-06, general-help): in the latest Spartan pack the console error spam is harmless, BUT if materials are fully black, very glossy or otherwise broken, those errors may be the actual cause. Also: unless painting new zones, just edit each zone's colour in the shader editor.
- Blender node-group version drift: a shader/pack can break across Blender versions; older node setups may exist for 4.0 (purplmunkii, 2026-06); a pack's node group gave grey output when routed through the group but colour when the image went straight to Material Output; suggested fix is to replace the shader nodes inside the group with the native ones from your own Blender build (the_titanfist, purplmunkii, 2026-07, general-help).
- sonic_senshi (2026-04, general-help): after editing all blue colours on every slot, leg and waist pieces still did not fully change; koax117 only repeated "check all material slots". Unresolved.
- draken_warlord (2026-04, general-help): in a pack shader's texture list, the middle texture drives the colour zone area; the others serve other purposes; the original (non-zone) colour is a separate texture. Matches the diffuse / normal / control-map trio (zophynoy asked what the "control" map does and saw no metal/spec channels, 2024-04, autodesk; unanswered).
- Pieces from a different pack can look wrong next to the base set: mak_74m could not make an Infinite "security" shoulder match the MK-V shoulder finish (2026-08) and butterchicken2 found Garita/Mk IV shoulder colouring differs from Mk VII (2026-07). Both unanswered; implies each shoulder carries its own shader/zone setup.
- tomatoatovt (2026-09): in her H3 pack the visor is left outside the primary/secondary group (H3 Classic had one visor colour); her Reach pack will expose all Reach visor colours. A user of an H3 pack found about 100 colour overrides in the shader; koax117 said detach the global colour node and edit three colour slots inside (2026-09). H3 pack also had a limb weighting bug to be fixed (tomatoatovt 2026-09).
- mslulu (2026-09): colour swap was fine but hair came out transparent; chiefster_4014 earlier: check whether the Infinite shader supports transparency, else use another shader (2026-06).
- doorsday (2026-09, general-help): Reach Spartans pack, render preview changes helmet colours and drops attachments vs viewport; unanswered.

Shine / specular / lighting (render-side)
- No accurate Reach/H3 Blender shader exists: gameoverload (2026-09) says Blender shader editor lacks light/render-pass data so Bungie custom lighting is approximated; Infinite is easy because it uses modern PBR. chiefster_4014's Reach shader attempt "messed up" (2026-09). Expect shine mismatch vs in-game as normal.
- Scene lighting matters for perceived shine: koax117 (2026-05, general-help) says the World shader supplies base lighting AND reflections; without an environment texture it looks flat. Appending an object into a new scene loses the world, so the look changes (echo219). Colour Management (Filmic/AgX), sun exposure, compositor exposure nodes and "Physical Atmosphere"/TrueSky-type addons can silently change exposure (koax117, 2026-08). Very bright lights wash colours out; about 50 was suggested for sun strength (tomatoatovt, 2026-08).
- nevermore's Reach Marine 2.0 visors were judged too shiny to match Reach, so one user swapped to purplmunkii's marine set (x_crimsongalax, 2026-09).
- jackofchaos (2024-04, substance-su) ported H3/Reach shaders to his own renderer; a stray multiplication made analytical specular too dark until fixed (visor glow came back); he lists "material models" (first 7 done). So the Bungie spec path = analytical specular + cubemap, per material model. He also needed spherical-harmonic coefficients (2024-03).
- file1431 (2023-10, substance-su): for Infinite he varies the specular from black to light grey with a cylindrical gradient and layer masks. Coating camouflage offset/scale mismatch is blamed on a missing translation value (chunch, 2022-08).
- Lost mesh normals after merging objects caused wrong shading in Substance iray (fiycsf, 2022-10); for soft-shaded rips clear custom normals (fiycsf, 2022-08). Relevant when a custom piece looks flat/shiny/dark.
- Maya cheap approximation: multiply the ASG-like map by a colour with a Stencil node in Hypershade (frostbitecinem, 2024-04).
- Infinite wear: ASG red channel = baked AO, green = subtract decals, blue = wear multiply (file1431/chunch).

Import / export / tools
- Cross-game tag copy trick: koax117 located H3 tags and copied them into the Reach tags folder so Foundry could import them (halogram work, 2026-06, general-help).
- chiefster_4014 (2026-04): the older Halo toolset addon imports/exports models CE-ODST but only does shaders for CE and maybe H2, not H3/ODST (this is the addon pass 1 listed under General101).
- Foundry is recommended for Reach custom maps into HREK (haloismylife65, 2026-03); the nihren line (2026-04) matches the older toolset (CE,2,3,ODST) vs Foundry (Reach,4,H2AMP).
- korrag (2026-03): "Open Halo Reach Shader for Unity" by Cameron "Fly" F did nothing with the pack's normal/diffuse/control; solved by Blender-to-Unity route (not detailed).
- Maya/Max: nobody had Reach materials working in Maya (sketch10121, 2022-06). Infinite skeleton is H4/H5-like with a few extra bones (r93_sniper, 2022-09); gameoverload planned a transfer via H4 rig. r93_sniper has a Max rig of Infinite Mk7 and Reach core using the in-game skeleton with his own skinning.
- Max FBX tip: 2016 imported Blender FBX fine, 2012 lost normals (ubald007, 2022-01). BMAX_Connector exists (https://github.com/TitusLVR/BMAX_Connector) but shaders do not translate.
- CE Campaign Evolved machinima mode reuses Reach animations; tutorial for no-weapon classic Reach pose: https://youtu.be/UQVcW13qIeM (strafeprime, 2026-07, general-help).
- CER Minor Elite: undersuit shared by all CER elites could not be found by the FModel user (starlight_ward, 2026-07).
- Substance import of normals: set Z output alpha to blue, convert DX to GL via levels/invert green, rotate/flip if needed (purplmunkii, 2024-04). Vertex colours before export allow ID-map bake by colour (purplmunkii, 2023-04).
- Halo 5 weapons look broken in Substance: export with modifiers applied; Reclaimer H5 branch (purplmunkii, r93_sniper 2024-07).

## Corrections to pass 1
- "Halo 3 and Halo 2 Arbiter armours ... not colourable" is wrong: winnower named the Halo 2 and Halo 5 Arbiters; H3 was not mentioned (2026-04).
- Ekur advice (koax117, 2026-08) answered an unspecified shader error, not the "only visor shows colour" report from opalrain3 (2026-04), who got no direct reply. Pass 1 merged them.
- "Units always metres" was r93_sniper asking whether it had been fixed, not an assertion; ubald007 could not confirm. "Blender FBX is reverse engineered, not SDK-based" is gameoverload's inference from XCOM 2 experience, and the "refuse the FBX API out of hatred" line is r93_sniper's opinion; both unverified.
- Pass 1 says H3 pack shader node group "unlink when mixing parts": koax117 said remove it from links to the other nodes (not delete); fine, but the group is a global colour node, not per-piece.
- chiefster_4014's port recipe says both CER and H3 "can use PBR textures"; treat as unverified for H3.
- Pass 1 file note "Cr4b Tool used for Ark pack" correct, but the unnamed "that addon" in the 2026-04 reply is not Cr4b.

## Contradictions between speakers
- Blender rig into Maya: kalvlidan (2023-11) said impossible to convert; r93_sniper (2024-03) said the skeleton should export fine (mesh+armature, DAE preferred); zophynoy (2024-04) failed and re-rigged; aegismind (2024-05) says not worth it.
- Textures with FBX: kalvlidan (2023-11) says possible with a newer FBX; gameoverload/chiefster_4014 say Blender FBX never carries these custom-shader textures (GLTF can).
- Blender FBX quality: ubald007 says Max 2025 imports fine; r93_sniper and frostbitecinem say still bad; gameoverload prefers the SDK-based Better FBX.
- Multi-material cost: r93_sniper called it "exponentially" worse, then "a linear increase"; ubald007 argued fewer maps render faster (2021-12).
- Hex colours: tpg says layer and lighting alter them (use Base Color view); r93_sniper blames sRGB vs linear (2022-05).
- Normal green flip: gameoverload (Blender, flip green/Normal Map node DirectX) vs purplmunkii (convert DX to GL) vs r93_sniper (Infinite is DX); consistent only if the target app's convention is stated.

## Best leads for these problems
1. Black/too-dark Honor Guard shoulder on Reach Elite: this slice only covers Blender packs. Transferable ideas: (a) piece may have its own material slots/shader unlike the base set; edit every slot, Alt+H to unhide; (b) a global colour node linking all pieces can override or starve a piece, unlink it when mixing sets; (c) missing/purple textures first, but black can persist (vexerart, _pandoras); (d) node-group/Blender version drift giving grey/black; (e) zones come from the control map (middle texture) so a missing or zero mask channel yields black; (f) Bungie env cubemap is never tinted, added before/after detail map depending on a tag flag (chiefster_4014 2026-06) so a dark piece may be a flag/ordering difference rather than colour; (g) lost custom normals after merges (fiycsf). Engine-side CC alpha, spec mask and fresnel-tint fields are NOT covered here; look in other chunks.
2. Elite shine/specular: expect approximation; check World/HDRI, Colour Management and exposure first; Bungie spec = analytical + cubemap per material model (jackofchaos); Reach shader not yet 1:1 in Blender (chiefster_4014, gameoverload 2026-09); visors in some packs are extra shiny (nevermore Marine 2.0).
3. Importing custom armour to Reach: Foundry (https://github.com/ILoveAGoodCrisp/Foundry) for Reach/H4/H2AMP into HREK; Reclaimers-library H3 guides partly apply; copy source-game tags into Reach tag folder to import (koax117); port-from-Infinite recipe: match proportions/default pose, rename weights to target bones, merge weights (fewer bones), bake PBR, export with addon (chiefster_4014, 2026-08); sky/material prefix such as +sky needs an export-time flag (Blender toolset) because Maya strips "+".
4. Networking of custom pieces: nothing in this slice. Only tangents: Mjolnir-Forge-Editor edits Reach Forge objects live in MCC (koax117, 2026-10), Ekur and Foundry are offline importers.
