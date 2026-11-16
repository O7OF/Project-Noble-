# Notes p2_05 (second pass on chunk_05: #ripping 2025-26, #reclaimer-up 2021-25, #texturing-an 2021-22)
Channel tags: ripping / reclaimer-up / texturing (file label is "texturing-an"). All raw lines were re-read. No lines in this chunk discuss networking, anti-cheat, EAC, lightmap baking beyond one question, scripting/AI beyond Architect, or Honor Guard specifically.

## Missed facts

### Reach shine / spec formula and shader behaviour
- Reach final specular roughly = diffuse alpha (spec mask) x (diffuse x albedo_blend constant) x constant specular colour; PBR-style inputs cannot be fed in directly. To convert PBR to Reach: use the metal colour as the material's specular tint and the average roughness as one scalar roughness. (pointdevice, 2021-05, texturing)
- In Reach most "specular" textures are just a greyscale copy of the base texture stuffed in the diffuse alpha; base_map carries the visible look and specular only says which parts are shiny. (the_heavynator, 2021-05, texturing)
- Reach visors are deliberately blown out by specular intensity (hard to see except in dark areas such as inside Anchor 9) and are biased toward specular/fresnel. (chromacore, 2021-07, texturing)
- Soulburnin (2022-01) lost hours on wrong Reach Spartan specular colour; cause was detail-texture multiplier values set wrong. Check the detail multipliers when spec looks off. (soulburnin, 2022-01, texturing)
- The H3/Reach "spec mask" feeds the albedo_blend input (spec mask is not the spec colour). (chromacore, 2021-11, texturing)
- the_heavynator (2021-10-14) said he found descriptions of the material_texture channels inside H3 shader code, and (2021-07-18) that ODST's own textures seem to have their colour channels swapped. Details were not posted in this chunk, so the RGB meaning of material_texture is still not stated here. (the_heavynator, 2021-07/10, texturing)
- For Reach per-pixel roughness the key trap is material_texture_black_roughness: 0 gives NaN artefacts (dark/black pixels), so use 0.01. (the_heavynator, 2021-08, texturing)
- the_heavynator thinks the shader ignores diffuse alpha when an extra texture (material texture) is used (said about H4 weapons, but same behaviour stated for material texture). Relevant if a spec mask in diffuse alpha seems ignored. (the_heavynator, 2021-12, texturing)
- H2ASP/H2A SP uses diffuse + specular(alpha = smoothness) + normal; H3 does not support a coloured specular map the same way, so converting H2A assets to H3/Reach needs manual spec editing. (the_heavynator, 2022-04, texturing)
- Reach env "enviro spec" property set is large and hard to reproduce; ChromaCore's Blender shader is not complete versus all Assembly values. (chromacore, 2022-01, texturing)

### Colour change on Reach
- Lobbster hit "grenadier chest/Jorge won't take colour": some Reach assets seem not to accept colour at all; guess was an asset-level texturing quirk (no answer given). So a piece not tinting can simply mean its CC map channels are empty. (lobbster, 2021-04, texturing)
- fiycsf noted Reach has intentionally matte-black patches on torso/forearm/calf areas, meaning some zones are meant to be outside the R/G colour zones; he used the R and G zones to rough out the main metal. A black region is not always a bug. (fiycsf, 2021-05, texturing)
- The "channel packed" Blender fix: a Reach sniper shoulder CC map looked fully black; fix was setting the image node Alpha mode to Channel Packed. It also cured brightness problems in the colour part. This was asked by martyrthesaint about a SHOULDER piece, directly analogous to the Honor Guard issue. (martyrthesaint / chiefster_4014 / theishikawarin, 2021-07, texturing)
- The question about "varying values in CC maps near weathered areas" (crashlegacy14, 2021-07) was NOT answered in chat; pass 1 merged it with the channel-packed fix. (texturing)
- Someone asked what each Reach tint is for (headphones1999, 2021-08); the answer is the 2021-11-30 unordinal/soulburnin explanation (elements 0-3, fresnel and spec tints).
- Hijacking the Reach Blender shader to do colour change on H2A MP elite works for quick previews. (mesathepostalg, 2026-02, ripping)
- In the Bungie-style Blender H3 shader, inputs listed include diffuse, detail diffuse, diffuse bump (zbump into it), and several alphas (diffuse alpha, detail 1/2 alpha, colour-control alpha). The shader named "Halo Bungie Shader"; Chief's H3 shader is the recommended fallback. Chief's shader may not work with emblems. (plastered_crab 2021-09; koax117/chiefster_4014 2022-06, texturing)
- An H3 MP armour CC pipeline is "_d with alpha" + control + "_s"; H3 palette hex values were requested for testing CC maps. (soulburnin 2021-10; snapchipper 2022-03, texturing)

### Elite specific
- Corrupted portion in an Elite texture: possibly bad mip data; un_nonimus planned to inspect mipmaps (CEA elite). (un_nonimus, 2026-08, ripping)
- Infinite Elite blends came out bright purple (missing textures) and Elite face needed own texture; some Elite textures needed vertical UV flips. (young_gusano / unordinal, 2021-12, texturing)
- MP Elite shown to need different CC colour handling than Spartans in H2A: mask is in the diffuse alpha (Index, H2ASP). Scope is H2A SP, not Reach. (chiefster_4014, 2026-09, ripping)
- Elite helmets (Infinite Officer) have no emission channel; Infinite colour-3 and emission share a channel/file, green of mask 1 stores emission colour. Infinite only. (sierra060822 / crashlegacy14, 2021-12, texturing)
- Elite textures in H2A MP are visible via Index but map files go via Reclaimer (BLAM). (mesathepostalg / gameoverload, 2026-02, ripping)

### Tools, versions, commands
- Reclaimer 1.6.274 (2021-05): collada export writes vertex position W as vertex colour R; copy-path menu; Halo 2 bump palette fix; meta viewer plugin profiles; batch workers; auto update check. Reclaimer 1.6.294: Assimp Scale default = Halo scaling, `[0]` suffix for multi-bitmap names, fixes small A8Y8/Y8 deswizzle. (gravemind2401, 2021-05, reclaimer-up)
- Reclaimer 1.6.285: pak batch extraction, H5 bitmap fix. 1.7.307 also fixes vertex weights on some H1 models; 1.7.309 fixes H2 rigid mesh weighting and inside-out H1 meshes; 1.7.806 fixes MCC H1 internalised bitmaps. (gravemind2401, 2021-05..2022-08, reclaimer-up)
- Architect (Reclaimer.Architect, https://github.com/Gravemind2401/Reclaimer.Architect/releases): scenario editor; 1.4.178 added initial H4/H2AMP scenario editing, 1.4.186 squad indexes in hierarchy, 1.4.194 editing string/stringID fields such as variant names, 1.4.199 block index prefix on areas/spawn locations. This is the only scripting/AI/scenario-editing tool mentioned. Mirror: mediafire folder named Reclaimer_Downloads. (gravemind2401, 2021, reclaimer-up)
- Reclaimer 2.0.1506 (2024-11): Halo Infinite, scenario tags, RMF export, expanded CEA/H5; 2.0.1536 RMF extra colour channel for H4/H5 (position W), `Export Metadata`, decorator model support for Reach/H4. (gravemind2401, 2024-11, reclaimer-up)
- Array bitmaps (cubemap/array DDS) can be loaded in Blender as one image where colour space and alpha mode apply to all slices (so one setting fixes a CC map's alpha for every slice). (gameoverload, 2026-08, ripping)
- Zoephie's Baboon dev builds: load Campaign Evolved, choose "chimp" for an fmodel-like browser; extract .jms or .pskx. (zoephie, 2026-08, ripping)
- Tag editing aids useful for Reach: Baboon (byte-exact edit of Reach tags), Reclaimer `Export Metadata` JSON dump (to diff a working and a broken shader/biped tag), Guerilla for H3EK. (zoephie / gravemind2401 / chiefster_4014)
- Foundry requires Reach Editing Kit selected as project; it can build sky shaders from `.scenery`; `Setup as Asset` on re-import. A user later reported it initialised after following "the reach foundation guide". (iloveagoodcris / frankr2994, 2025-10..2026-03, ripping)
- Foundry advice on gamma: control it via the image colour profile on the texture node, not via node-group gamma. (iloveagoodcris, 2025-10, ripping)
- Pre-order of H3 tags into the HREK folder for Foundry: textures may be flaky. (iloveagoodcris, 2025-10)
- Pass-1 item "General-101 toolset" details: users also run the `Halo Tools Helper` addon for JMA. (exodst., 2025-11, ripping)
- Fonts/HUD icons: Lord Zedd's FontPackager, open `maps\fonts\font_package_icon.bin`; fixedsys-ui entry; repeat for x2/x3. (haloismylife65, 2026-05)
- Exporting Reach FP rig: Azarath__ could not find the first-person skeleton in Reclaimer (2026-09); adorabirb says the rig is the CE/Reach fp rig with different bone names; no answer on extraction.
- Another claim that Halo 3 cannot be ripped directly and levels must be moved into Reach (arraymonitor, 2025-12) contradicts Reclaimer H3 support; treat as stale.
- Reach assets via Reclaimer: "no option to rip specific parts of the Reach armor set" (moonify.sfm, 2026-08); answer in docs is that Reclaimer 1.7.305+ has export selected parts/bitmaps and markers included from 1.7.307.

### Shader/Blender misc
- Normals: if a normal map has no blue/colour, many Halo normals are two-channel (BC5-like) so Blender needs the blue reconstructed; asked unanswered. (average_userna, 2022-04, texturing)
- Gamma tip for too-bright specular in Blender: Multiply Mix between albedo and shader, factor 1; then a Gamma node at 2.2 on spec if still bright. (soulburnin, 2021-04, texturing)
- Reach coloured spec colour value example: steel is hex 646464. (chromacore, 2021-05, texturing)
- Texture import into Reach: Forerunner's tool converts DDS to `.tagc`, imported in Assembly under a custom name. (surasia, 2021-07, texturing)
- Reach uses a single shared "visor colour" system already in the engine, said to make adding visors easy. (williaso, 2021-05)

## Corrections to pass 1
- Pass 1 section 1 attributes "Halo 2 bump maps use right palette" to Reclaimer 1.6.249. Raw lines place it in 1.6.274 (2021-05). 1.6.249 has: H4 crash fix, material detection, AMF normal/detail maps, TGA export, U8V8, H2 beta render_model.
- Pass 1: "Alpha as 'channel packed' in Blender fixes black/dark CC map reads (chiefster_4014/theishikawarin 2021-07)" is correct but the surrounding sentence about "dark values near weathered areas used for variation" is only a QUESTION (crashlegacy14), never answered. Also the black map was a Reach sniper SHOULDER CC map.
- Pass 1 H4 control Type 2 alpha: "white = primary, grey = secondary, black = uncolourable" is the soulburnin/R93 version (2021-06, 2021-12). The original Waffles text (relayed by surasia 2021-07) says A = dual colour change, black secondary, white primary. See contradictions.
- Pass 1 "error importing JMA ... Blender 5 API change" is pass-1 inference; the chat only gave the 'fcurves' error text (tristangold, 2025-12).
- Pass 1 section 3 "Reach CC maps ... Secondary colour doesn't tint env map (false alarm)": correct, but unordinal's original grunt test only retracted after realising the room had a bluish tint.
- Pass 1 lists H4 control type 1 B as reflection mask and type 2 B as spec exponent; surasia's 2021-06 relay calls B "gloss". Same thing, but H4 type 1 and 2 "R" differ (type 2 R = spec/reflection).
- Pass 1 Infinite entry merges two incompatible descriptions (fiycsf vs surasia). See contradictions.
- Pass 1 flagged "per-pixel roughness (found 2021-07/08)". The first note was 2021-07-18; the recipe was posted 2021-08-29 (the first mention said he could not test further because he did not know how to import textures).
- Pass 1 source label "texturing" is the channel texturing-an.
- Pass 1 says H3 MP `_s` spec colour map exists (soulburnin 2021-10); heavynator (2022-04) says H3 does not support coloured spec maps. Not a pass-1 error but unflagged.

## Contradictions between speakers
- Is Reach PBR / does it use specular: surasia (2021-05) "Reach doesn't use specular / isn't PBR metallic-roughness" vs pointdevice "Reach is specular, Cook-Torrance, metallic is converted to specular" vs the_heavynator "microfacet but not PBR standard" vs lavadeeto/_gallium ("form of PBR"). Practical agreement: RGB diffuse + alpha spec mask, scalar roughness.
- Per-pixel roughness: pointdevice and chromacore (2021-05) say Reach has none; the_heavynator (2021-07/08) shows an alpha-roughness path via material_texture; pointdevice replies it is a leftover from the H3 beta, not used in retail.
- Spec tint source: chromacore (2021-11) believes spec tint is per material; unordinal says per-material tint is overridden by the biped Change Colors block when a CC map exists.
- H4 control alpha: Waffles (black = secondary, white = primary) vs soulburnin/R93 (white primary, grey secondary, black not coloured).
- H3 spec colour: soulburnin `_s` spec colour map in MP armour vs heavynator "H3 doesn't support coloured specular maps".
- Infinite masks: fiycsf (Control R/G/B = colour 1/colour 2/edgewear; ASG G = scratch, B = grunge) vs surasia (Control1 RGB = zones 1-3; Control2 R/G = zones 4/5, B = blast damage; ASG G = edge wear, B = dirt).
- Foundry xrgb vs sRGB: brooen says Foundry wrongly sets sRGB; Crisp says to manage gamma on the node colour profile (no confirmation either way).
- Reclaimer versus Foundry for Reach/H4 ripping: frankr2994 says use either; chiefster_4014 says Reclaimer is the main ripper and Foundry reads kit tags.

## Best leads for these problems
(1) Black or too-dark Honor Guard Elite shoulder
- First check the CC map alpha: Reach CC alpha masks element 3 (quaternary) which also tints specular; elite pieces often have small CC maps with non-white alpha. A zero alpha or alpha read as premultiplied will give black. In Blender use Alpha mode Channel Packed (confirmed for a black sniper shoulder CC map).
- Check biped/object `Change Colors`: element 2 tints fresnel, element 3 tints specular for materials with no explicit specular_tint; a black or near-black colour in the active variant darkens fresnel/spec, and the biped block overrides per-material tint. Element colours are masked by CC R/G/B/A; black variants colours on a channel will make that zone black.
- Shader floats that blacken: albedo_color tint (clamped 0-1), diffuse_coefficient 0, specular_coefficient 0, material_texture_black_roughness = 0 (NaN), analytical_roughness extremes, missing bitmap defaulting to grey_50_percent, wrong detail multiplier (soulburnin 2022-01).
- Compare the shoulder's shader tag against a working Elite piece using Reclaimer `Export Metadata` (JSON) or Baboon/Assembly; diff diffuse_coefficient, albedo_blend, tints, bitmap references.
- Verify bitmap format: Foundry may mark bitmaps sRGB instead of xrgb (brooen 2025-10); wrong gamma can make a CC/control map read dark. Also inspect mips (un_nonimus saw corrupted elite texture, possibly mips).
- Possibility: this CC zone is intentionally outside R/G/B/A (fiycsf: some Reach zones are meant to stay matte black).

(2) Elite shine/specular
- Spec = diffuse alpha (mask) x diffuse x albedo_blend x specular colour; tune albedo_blend, specular_coefficient, specular_tint, analytical_specular_contribution (brightness) and analytical_roughness (<0.5 matters), fresnel_color/fresnel_curve_steepness, environment_map_specular_contribution plus env_tint_color; area_specular_contribution looks like glow from indirect light. roughness and approximate_specular_type do nothing.
- Spec tint on CC-capable assets comes from Change Colors element 3 (masked by CC alpha); fresnel tint from element 2.
- Optional per-pixel roughness: analytical_roughness = 1, material_texture_black_roughness 0.01, use_material_texture on (float constraint, often index 0), roughness in the alpha of material_texture, needs a shader with all material inputs.
- If textures are derived from PBR sets, convert to greyscale spec in alpha with a scalar roughness; coloured spec maps are not a Reach feature.
- In Blender previews: two glossy nodes sharing spec colour/roughness, env reflection off; ChromaCore's or soulburnin's Reach shader, not plain Principled.

(3) Importing custom armour into Reach
- Foundry (Blender) with the Reach Editing Kit project is the actively supported path; drag and drop `.scenery`; `Setup as Asset` option; JMS/JMA importer fork from camden-smallwood exists for newer tools; animations/rigs from HaloAnimationRepository; Extract Import Info path must start `tags` or a drive.
- Textures: DDS to `.tagc` with Forerunner's tool and import via Assembly under a new name (surasia 2021-07); keep CC map alpha intact (use format that preserves alpha, xrgb for linear maps).
- Armour variants and colour channels live in the Change Colors block and permutations in the biped tag; edit with Baboon (Reach tags), Assembly or Guerilla (H3EK analog).
- Scale: Reclaimer-ripped rigs need 32.8084 scale; JMA imports can be about 33x too big (H2 case).

(4) Networking of custom armour for other players
- Nothing in this chunk. No mention of netcode, tag sync, anti-cheat (EAC) or what other clients see. Only indirect clue: armour colour comes from the player's biped Change Colors and CC maps, so a client that lacks your modified tags would show defaults; this is an inference, not stated by speakers.
