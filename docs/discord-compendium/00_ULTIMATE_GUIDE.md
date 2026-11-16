# Halo Reach / H3 Modding - ULTIMATE GUIDE (Elite armour focus)
Merged 2026-11-12 from: (B) the Discord field guide (12.5k Halo Archive messages, read twice + synthesis) and (C) the Reach modding reference (Microsoft Learn H3/HR docs, c20.reclaimers.net JSON parameters, Halo Mods #materials digest, Tyler's three attachments, tool.exe tips).
Section numbers inside Part B text refer to Part B; inside Part C to Part C.

## Contents
- **Part A - Start here**: current status and ordered test plan for the black Honor Guard (HG) shoulder wing and Elite shine; what is ruled out
- **Part B - Field guide from Discord** (B0-B12): channel layouts, Reach shading model, Change Colors, Elite specifics, black/dark/shiny checklist, import/export tools and commands, rigging, lightmaps, networking (almost nothing), links, contradictions, corrections
- **Part C - Official docs and project references** (C1-C8): HREK FBX->GR2 pipeline, JSON parameter catalogue, MS Learn shader options, tool.exe tips, particle-limit patch, Forge strings, Halo Mods takeaways
- **Part D - Reconciliation and gaps**: where A/B/C agree or conflict, what has NOT been read
- **Part N - Networking** (N1-N12): how Halo 3, ODST and Reach network (Bungie GDC talk), bandwidth numbers, MCC changes, player checklist, ideas to improve; supersedes B9
- **Part E - Other Halo games** (CE, H2, H4, H5, Infinite, Wars, H2A): kept fully separate; not for Reach/H3/ODST work

Confidence tags: [C] confirmed, [S] claimed (single source), [D] disputed, [U] unverified inference. "(unverified)" in Part C = chat message, not documentation.

---

# PART A - START HERE

## A1. Ruled out on the Honor Guard shoulder (checked 2026-11-12 against the stock Ultra shoulder)
- Shader parameters: albedo_color, env_roughness_scale, use_material_texture, no_dynamic_lights identical (HG only adds self_illum_color/intensity).
- Shader options: identical option indices; same reference template elite_armor_template.shader.
- Compiled template: both use shaders\shader_templates\_10_2_0_1_1_2_0_0_0_0_0_0.render_method_template; it loads (the "red template in postprocess" failure from Discord does not apply).
=> The black wing is probably NOT a shader-parameter problem. BUT two shader-level checks are still open (A2 steps 1-2): the albedo OPTION (two_ vs four_change_color) was never printed, and the environment-mapping/cubemap category was not compared.

## A2. Ordered test plan (cheapest first)
1. **Albedo option** [new, unchecked]: run shader_opts.py on the HG shoulder and the stock Ultra shoulder and print the albedo category. two_change_color reads only cc R and G; four_change_color reads R,G,B,A = primary..quaternary. If HG is two_change_color and the wing is painted in cc blue/alpha, that zone does nothing (Part C3; the shader dump so far printed parameters, not option selections).
2. **Environment mapping** [new, unchecked; top shader-side test]: with no env map a cook_torrance surface goes black on its sides (MS Learn material model page). Compare HG vs stock Ultra for `environment_mapping` category (per_pixel + cubemap bitmap), `environment_map_specular_contribution`, `env_tint_color`, and the material model. A cubemap-less shader with a high specular_coefficient also reads as a dark mirror [U].
3. **Which mesh is the wing**: hide-piece test in the Armory with screenshots (a tall wing may belong to helmet/forearm; a big panel to legs). Decide which material/shader/texture actually applies before changing anything.
4. **Texture read**: wing UV island dark in diffuse? cc coverage (cc is a red-only mask on ~18% of the UV in the originals)? Open each channel of the _cc and diffuse alpha (B1.2). Spec mask = diffuse alpha ("Specular mask from diffuse"); alpha 0 or odd values change the look, and the spec mask gates shine even if the shader allows it. Reach _cc files with alpha 0 are normal (B3.3).
5. **Specular mix / tint chain**: Discord's one concrete cause of "black uniform no matter what the controls" was "specular mix". Specular = lerp(spec tint, CC-coloured albedo) (B2.5): a black primary also kills spec, and unset Change Colors slots may leave tints black (B3; the slot-2/3 = fresnel/spec link is UNVERIFIED). Compare the biped Change Colors block (slots 0-3 for the HG permutation) with a working Elite piece using Reclaimer's Export Metadata JSON or Assembly.
6. **Detail multiplier**: the Reach Covenant shader multiplies the detail-map colour by 4.59479 before it multiplies the base colour; recreations without it came out much darker (chiefster_4014, 2025-04) [S, Covenant shader only]. Check the detail map path of the Elite shader.
7. **Baked PRT/AO**: re-import the mesh without PRT or with "precise". (c20's JSON catalogue never mentions PRT, so there is no documented switch; PRT generation can fail on high-poly meshes.)
8. **Last resort**: copy a known-good stock shader (e.g. marine armour or the stock Ultra shoulder) and change only the bitmaps. "Reach is very picky about the shader properties you use" (Discord).
Also: if a shader breaks when a cc map is added (texture swaps to an unrelated one), the template in the postprocess block is missing: hold Alt when opening the shader in Foundation (RED template = missing) and generate templates (C4, C8).

## A3. Elite shine reference values
Chain: diffuse x detail -> coloured by CC -> mixed with specular tint (0 = spec tint only, 1 = albedo-tinted) ; fresnel curve value 0 = fresnel colour overrides spec tint. Elite globals: specular tint #D6D4DB, fresnel colour #E8C6A4. Darker colour values do not shine: brighten the primary. Full parameter table in B2.2. Shine/washout of Ossoona and extra sets in the Armory is plausibly spec/env, still to compare against Council (works).

## A4. Other open items (project)
Rebuild the match pack once after the shoulder fix; Lights row in-game check; Council helmet silver crest / HG helmet silver; Spartan knife; trim 500 MB pack; networking of other players' Elite pieces (no documentation exists, B9). See HANDOFF-ELITE-RR-TEXTURES, FINDINGS-ELITE-ORIGINALS-COMPARE, HANDOFF-ELITE-SHINE.

---

# PART B - FIELD GUIDE FROM DISCORD
Source: ~12.5k messages of a Halo modding Discord (2021-03 to 2026-10), two extraction passes, then this synthesis. Pass-2 corrections override pass 1. Spot-checks against the raw chunk files are marked "raw-verified".

## B0. How to read this

Citation form: (author, YYYY-MM). All names are Discord handles.

Confidence legend:
- **[C] Confirmed**: stated by a tool/shader author, or by two or more independent people, or raw-verified and uncontested.
- **[S] Claimed**: one source. Treat as a lead to test.
- **[D] Disputed**: speakers disagree. Both sides are given.
- **[U] Unverified**: inference by an extraction pass or by this synthesis, never said by a speaker. Marked so you do not mistake it for fact.

Scope warnings (important):
- The corpus is overwhelmingly Blender / ripping / porting help. Direct tool.exe, Sapien, Foundation tag-editing for Reach is thin. Nothing in it mentions "Honor Guard" specifically.
- There is almost nothing on networking, anti-cheat or how other players see custom armour (section 9).
- Many "Reach" statements come from Blender-pack users and describe how a *Blender approximation* behaves, not the engine. Where that matters it is flagged.

---

## B1. Texture / control-map channel layouts (Reach / H3 / ODST)

Authority: r93_sniper's #control-map-guide (2021-12), re-quoted by gameoverload (2022-11, 2022-12, 2023-07). "Control map" is the community word for the packed mask map (fiycsf, 2023-02).

### B1.1 Reach / H3 / ODST (shared layout; details differ per shader)

| Texture | Channels | Conf |
|---|---|---|
| Diffuse | RGB = base colour; A = specular mask OR opacity (transparent shaders). When a Material texture is used the diffuse alpha normally holds no spec | [C] (r93_sniper, 2021-12; surasia, 2021-05) |
| Normal | DirectX format, green inverted vs Blender/Unity. Often XY only with Z rebuilt in shader (BC5/DXN style); nearly all games rebuild blue | [C] (many; chiefster_4014, 2025-04) |
| CC (colour change, `_cc`) | Guide: R = primary mask, G = secondary mask, B = unused. See 1.2 for the Reach 4-channel evidence | [C] for R/G; [D] for B/A |
| Illum | RGB = emissive COLOUR used directly (not a mask). Not every object has one | [C] (r93_sniper, 2021-12) |
| Material (optional) | R = specular coefficient, G = albedo blend, B = environment (cubemap) contribution, A = roughness | [S] (guide author only; raw-verified text). Only A=roughness is independently tested (the_heavynator, 2021-08) |

Notes:
- Not every object has Illum or Material maps. Which channel means what is decided by the shader, not the texture (zidiath/gameoverload, 2023-11; [C] principle).
- Texture names seen on Reach: `mpmask` = colour-change mask, `hdetm` = height/detail map mask (chiefster_4014, 2023-10) [S]. `visor_cc` is only the CC mask for all visors; visor diffuse/normal live with the helmet textures (chiefster_4014, 2025-10) [S]. Marines: camo is baked into `marine_cloth_cc`, no separate camo map (chiefster_4014, 2026-07; raw-verified) [S].
- Reach textures are often 2k; menu/customisation textures are lower resolution and their UVs are broken. Do not extract armour from the menu (soulburnin, 2021-07; the_heavynator, 2021-10; chiefster_4014, 2025-10) [C].

### B1.2 Reach CC map: how many channels?  [D]
- Guide (r93_sniper, 2021-12): R/G only, B unused [C as the guide's statement].
- unordinal + soulburnin (2021-11/12): the biped `Change Colors` block has up to 4 elements that map to R/G/B/A of the CC map (section 3). Reach visors have their own small CC map whose alpha is not pure white. Elite SpecOps has no blue in its CC map yet a slot-2 colour.
- chiefster_4014 (2026-03): "H3 CC maps can use RGB but the sheet shows only RG; Reach CC maps can use RG but the sheet shows only R". Base H3 and Reach armour are RG; Raven sets are among the few using RGB.
- Practical reading [U]: MP armour = R/G; B and A are asset-specific (Elite, visors, Spec Ops). Open the actual `_cc` and look at each channel before assuming.

Colour space rule for ALL of the above [C] (err_0r 2021-03, sswampp 2021-11, deskclaw 2022-05, many): base/diffuse = sRGB; CC, control, normal, roughness, blend = Non-Color. Tint values stored in tags are linear (unordinal, 2021-11); convert for sRGB use.

---

## B2. Reach shading model

### B2.1 What it is
- Reach (and H3/ODST) use a Cook-Torrance-based specular model with a scalar roughness, coloured specular, a cubemap, and (Reach) area specular. Not metallic/roughness PBR (soulburnin, 2022-11; pointdevice, 2021-05; gallium, 2021-07) [C]. The exact normal distribution (Beckmann or other) was asked and never settled (nihalrahman/soulburnin, 2022-11) [D]. The tags describe the direct highlight as using a Beckmann NDF (the_heavynator, pointdevice, chromacore, 2021-05) [S].
- Whether this "is PBR" is a semantic argument: surasia "isn't PBR"; pointdevice "is Cook-Torrance"; the_heavynator "microfacet but not the PBR standard" (2021-05) [D]. Practical agreement: RGB diffuse + alpha spec mask + scalar roughness. Reach is "pseudo-PBR" (adorabirb, 2021-04).
- Reach/H3/ODST use coloured specular (pointdevice, 2021-05/06).
- Real shader source: Reach, H3 and ODST shader code ships with the official MCC mod tools (chiefster_4014, 2025-12; gameoverload 2024-01/06) [C]. Procedure for Reach: open the material tag in Foundation, note the shader it references, open that shader with **Expert Mode enabled** to see the source (gameoverload, 2024-10) [S]. Assembly's shader dumps are raw binary, not readable HLSL (pointdevice, 2021-07). Guerilla shows parameter pages, not code, for H3 (gregory0701, 2022-08) [D vs fable_law_458].

### B2.2 Parameter meanings (tested, Reach `shader` / material tag)
Source: the_heavynator, pointdevice, chromacore (2021-05-24; raw-verified), by poking values [S, tested by three people]:

| Parameter | Effect |
|---|---|
| `albedo_color` | Tints the albedo; clamped 0-1 |
| `diffuse_coefficient` | Brightness of final diffuse |
| `specular_coefficient` | Brightness of ALL specular (direct, fresnel, env, area) |
| `specular_tint` | Tints specular; clamped 0-1 |
| `analytical_specular_contribution` | Brightness of the direct highlight |
| `analytical_roughness` | Roughness of the direct highlight; visible change mostly below 0.5 |
| `albedo_blend` | How much the diffuse tints the specular |
| `fresnel_color` | Colour of the fresnel (glancing) specular |
| `fresnel_curve_steepness` | Strength/steepness of the fresnel curve |
| `area_specular_contribution` | Area specular strength; looks like the spec map glowing with indirect diffuse light colour |
| `environment_map_specular_contribution` | Cubemap reflection strength |
| `env_tint_color` | Tints the cubemap reflection |
| `env_roughness_offset` | Very coarse cubemap blur (mip level); 0 to 0.5 shows no change |
| `roughness`, `approximate_specular_type` | Appear to do nothing |

Later refinements:
- Specular colour formula as given by soulburnin (2024-07): final spec = specular texture x (analytical specular coefficient x specular tint) x final tint. The texture itself is coloured (ODST skin's blue spec is the real texture colour; pointdevice 2023-07 agrees: spec is a colourising mask) [S]. pointdevice (2021-05): spec ~ diffuse alpha x (diffuse x albedo_blend) x constant spec colour [S]. These two are not identical statements [U].
- H3/Reach spec exponent controls the power of the mix between specular colour and fresnel colour; spec coefficient is a flat multiplier; the reflection mask only gates where cubemap reflections appear (chiefster_4014, 2026-06; raw-verified) [S, from someone who has read the shader code].
- Fresnel in the community "HRCategory: cook torrance" Blender node group: bottom colour = specular tint, top colour = glancing/fresnel colour, mixed on top of the (specular tint - albedo blend) result; "exponent" = steepness of the curve (soulburnin, 2024-08) [C, by the node-group author]. soulburnin (2024-01) thinks the exponent can be read by clicking the specular colour in the mod tools, but phrased it as a guess ("should be called exponent or something") [S, weak].
- To fill those inputs from Foundation: use the RGB numbers listed under the "Xenon" entries and divide by 255 (chiefster_4014, 2024-08) [S].
- Detail multiplier trap: wrongly set detail-texture multiplier values skewed Reach Spartan spec colour for hours (soulburnin/tpg, 2022-01) [S]. `grey_50_percent` is the neutral default bitmap in almost every Reach shader.
- Reach has a large "environment specular" property set that is hard to reproduce; ChromaCore's Blender shader is not complete vs all Assembly values (chromacore, 2022-01) [S]. Cubemaps are baked GI-reflection storage; cubemap look dominates final shading on older Halo shaders (soulburnin, 2023-07).
- The env/cubemap reflection is never tinted by the CC; it is added to diffuse either before or after the detail map is applied depending on a flag in the shader tag (chiefster_4014, 2026-06) [S].

### B2.3 Detail maps and constants
- **4.59479**: "the detail map's colour is multiplied by 4.59479 before being multiplied into the base map's colour" (chiefster_4014, 2025-04; raw-verified, chunk_04). Context: gregory0701's Unity recreation of the Reach **Covenant** shader came out much darker than the Blender one when the detail map was multiplied by albedo. [S] but the only concrete "too dark" explanation in the corpus. It was said for the Covenant (Elite-relevant) shader, not stated as universal to all Reach shaders [U on scope].
- gregory0701 (2025-04) saw gamma 2.0 in the game shader settings and wondered if the usual 2.2 assumption matters; chiefster did not rule on it.
- Reach "height/detail" maps (`hdetm`) become normal maps in the shader path; r93_sniper (2022-11) described this for earlier titles; Reach was NOT named [D/U].

### B2.4 Roughness texture recipe (per-pixel roughness)  [D]
the_heavynator (2021-08-29; raw-verified):
1. `analytical_roughness` = 1.
2. `material_texture_black_roughness` = 0.01 (NOT 0; zero gives NaN artefacts, i.e. black/dark pixels).
3. Enable `use_material_texture` (a float-constraint toggle, "usually constraint 0").
4. Put a texture with the roughness in its ALPHA into the `material_texture` input.
5. The shader must be a transparent-capable one exposing all material inputs (e.g. `cargo_truck_decals`).
Dispute: pointdevice (2021-07, raw-verified) says this is a leftover from the H3 beta and not used in retail Reach shaders; chromacore and pointdevice (2021-05) said Reach roughness is a flat scalar. The RGB meaning of the material_texture was never posted (the_heavynator said he found descriptions in H3 shader code, 2021-10). ODST body/fp hands reportedly use it. Treat as an experiment you must verify in game, not a retail pipeline [D].

### B2.5 Spec / fresnel / env / albedo-blend behaviour in one chain
Best-supported Elite/Skirmisher chain (soulburnin, 2021-11; raw-verified) [S]:
`diffuse (blended with detail) -> coloured by CC map -> mixed with specular tint -> final specular colour`
- Mix: 0 = specular tint only, 1 = full albedo-tinted spec.
- Fresnel curve value 0 = fresnel colour completely overrides the specular tint.
- Globals tag values for the Elite: specular tint `#D6D4DB`, fresnel colour `#E8C6A4` (raw-verified).
- Direction of the "subtract albedo" in the HLSL is unsettled: unordinal's own wording was ambiguous ("misread that it subtracted albedo from spec tint for the mix") [D].

### B2.6 Blender preview recipes (not engine truth)
- Avoid plain Principled for Reach. Use ChromaCore's or soulburnin's Reach shader, or two glossy nodes sharing spec colour/roughness plus diffuse with env reflection off (jadedqrow, 2022-05; the_heavynator, 2022-06). Coloured-spec recipe: Principled #1 = albedo only, everything else 0; Principled #2 = spec map to colour, specular 1, metallic 1, inverted gloss to roughness, normal on #2 only; Add Shader.
- No 1:1 Reach shader exists in Blender; the editor lacks light/render-pass data (gameoverload, 2026-09). chiefster's own attempt "messed up" (2026-09). jackofchaos ported H3/Reach shaders to his own renderer; the spec path = analytical specular + cubemap per material model (2024-04) [S]. Expect some shine mismatch.
- Cubemaps need cube-to-equirect: py360convert, Blender Cubemap Converter add-on, or Flexify 2 + crop (soulburnin, 2022-10; brooen 2023-08) [C]. Cubemap colour space and the cubemap alpha's purpose remain unanswered (gameoverload, 2023-09; theishikawarin, 2025-02) [open].

---

## B3. Change Colors and colour-change (CC) maps

### B3.1 Established
Reach object/biped tag `Change Colors` block (unordinal with soulburnin, 2021-11-30; raw-verified) [S but tested on a grunt; widely cited afterwards]:
- Up to 4 elements (0-3, asset dependent). Each holds a list of variants; each variant has a name and two colours.
- Element 0 = primary (CC R), 1 = secondary (G), 2 = tertiary (B), 3 = quaternary (A).
- The diffuse is mixed with these colours by the CC map.
- For assets with no explicit `specular_tint` in their shader, element 2 additionally tints the FRESNEL and element 3 tints the SPECULAR. Those tints do not use the CC map. When element 3 is both spec tint and quaternary colour it is masked by the CC alpha (soulburnin, 2021-12).
- A per-material spec tint is used when there is no CC map; with a CC map the biped's colour block seems to override it (unordinal, chromacore dispute, 2021-11) [D].
- Secondary does NOT tint the environment map (an early observation was a false alarm caused by a bluish room).
- Reach visors have their own tiny CC map; CC alpha masks the quaternary colour applied to visor diffuse (soulburnin/unordinal, 2021-12) [S; unordinal doubts it masks the specular itself].

### B3.2 NOT established (pass 1 overstated)
- The claim that the "tertiary-to-fresnel" link was "resolved" is wrong. unordinal said it "mostly makes sense" and planned to verify in game. Unverified.
- Elite SpecOps: green fresnel equals the colour in slot 2 although its CC has no blue. Ranger Elite: orange fresnel equals the tertiary slot; its orange self-illum comes from the `illum_cards` shader itself, not the colour block. soulburnin first guessed emissive, then a greenish cubemap tint on SpecOps pieces. Nobody verified in game [D].
- What decides colour-vs-tint (a flag?) was not found. The docs' shader-input list does not cover per-biped overrides (unordinal, 2021-11).
- unordinal's opening belief (four blocks = albedo/specular/fresnel/env tint) was replaced by the channel mapping above.
- No canonical table of Reach armour hex values exists in the corpus (aegis_wolf 2025-05; thoro.blend 2024-08 asked, nobody answered). Pointers: `#lavadeeto-resources`, Halo Archive video youtu.be/vVS9HJd73io. Reading exact campaign colours: biped tag in Assembly or ODST mod tools (brooen/chromacore/chiefster, 2022-2024). Reverse engineering Reach CC colour from the executable via IDA was attempted (walkmen, 2024-08), no result.

### B3.3 Colour space and alpha handling in Blender  [C unless noted]
- CC image colour space = **Non-Color**.
- Image **Alpha mode = Channel Packed** whenever alpha carries data (spec in diffuse alpha, CC alpha). With Straight, colour is multiplied by alpha and the shader can go black (soulburnin, chiefster, chromacore, 2021-08 to 2026-06). Concrete case: a Reach Sniper SHOULDER CC map looked fully black; Channel Packed fixed it (martyrthesaint/chiefster/theishikawarin, 2021-07).
- Some Reach `_cc` files store alpha = 0 and look transparent (e.g. `spartan_chest_joshua_cc`). This is normal for that file; set Non-Color or force alpha white in an editor (gwenwyn/chiefster_4014, 2024-10). caldoric (2023-10) saw alpha=1 as wrong and said the file turned into a grey checkerboard after editing: the slot read as empty [D].
- Editing hazards: GIMP Colors > Components > Decompose/Recompose kills alpha (chiefster, 2023-09); GIMP erase fills with the background colour (hydranaught444, 2023-01). Either erases a spec/CC alpha.

### B3.4 Making your own CC
- Base albedo for a CC item should be white/neutral (gameoverload, 2022-11; plastered_crab, 2023-08) [C]. Pack masks into R/G, keep wear/metal out of the tinted region (fiycsf, 2021-04).
- Manual node route: Mix (Multiply) with factor = primary mask, A = base, B = primary colour; second Mix with factor = secondary mask chained on the first (gameoverload, sswampp).
- The control map must match what is baked into the albedo; a mismatched CC "looks like hell" (gameoverload, 2022-11).

---

## B4. Elite / Sangheili specifics

- **Vertex colour**: the Reach Elite body is said to carry vertex colour used as a shader mask (the_heavynator, 2021-05; example "armorless" Elite) [S]. gravemind2401 (2021-05, 2024-11) recalls no render model with it and says position-W is not vertex colour. brooen (2022-12) says Longbow rocks should have it but export lacks it [D]. Reclaimer's AMF has no vertex colour; DAE and RMF can carry it. A piece going dark after a port could be a shader multiplying by an absent vertex mask [U].
- Reach Elite emissives vanish in Render but show in the tweak pane (x_crimsongalax, 2026-01), unresolved. Glowing bits guessed to be `base_illum_cards` (x_crimsongalax, 2025-07).

---

## B5. Diagnosing a black / dark / over-shiny armour piece (ranked checklist)

Nothing in the corpus pins the Honor Guard shoulder. This ranking merges tag-side facts with Blender-side facts, ordered by likelihood given what was documented. Each item says how to test. Items tagged [U] are inference.

### B5A. Black or too dark
1. **CC image read wrongly (most documented)**. Test: set CC node to Non-Color AND Alpha = Channel Packed; compare with a working piece. Documented on a Reach Sniper shoulder (martyrthesaint, 2021-07) and Straight alpha blackening (chiefster, 2026-06). Also check the CC file really loaded (caldoric 2023-10) and is not a checkerboard/blank (Sniper shoulder, Season-7 Elite maps blank, 2021).
2. **Colour-space wrong on other maps**: diffuse sRGB, everything else Non-Color; normal Non-Color (gives dark/odd shine; DMR seam case, chiefster, 2024-04). Texture saved as HDR with sRGB cured one fault (chocolateray, 2025-04).
3. **Tint input unset or black**: in Surasia's Reach node group, unset tint colours leave masked areas black [U]; the group already contains the BSDF, so a leftover Principled in the path makes tints do nothing (surasia/err_0r, 2021-03). In the engine, a black variant colour in `Change Colors` blackens that channel's zone; slots 2/3 also tint fresnel/spec (section 3). Test: diff the `Change Colors` block against a working Elite piece.
4. **Missing detail multiplier (4.59479)** on the Covenant shader: recreations came out much darker without it (section 2.3). Test: multiply detail colour by 4.59479, compare. [S, strongest numeric lead]
5. **Shader floats that darken**: `diffuse_coefficient` 0, `specular_coefficient` 0, `albedo_color` black (clamped), `material_texture_black_roughness` = 0 (NaN pixels), missing bitmap falling back to `grey_50_percent`, wrong detail multiplier. Test: compare against a working piece with Reclaimer's `Export Metadata` JSON, Baboon, Assembly, or Foundation. [mostly U as a list; each parameter is documented, the combination is inference]
6. **Piece has its own material slots / shader**. Edit every slot, Alt+H to unhide geometry; hidden second shader stacks (kneepads, koax117 2022-06); a global colour node can starve a piece, unlink it (koax117, 2026-09). Copy the node tree from a working piece and relink masks (exporush 2023-02; poet 2023-10; chiefster 2023-10).
7. **Zone switch / zone mix-up**: an off zone switch falls back to zone 1 colour (gameoverload 2023-06). Wrong colour slot plus sRGB gave white blowout on a Reach shoulder (etheox, 2022-07/08).
8. **Reach armour darkened when diffuse replaced by the primary colour** in the damage node tree: re-link the original diffuse node (x_crimsongalax, 2025-07) [S].
9. **Metal-like behaviour with no environment**: in metallic engines high metallic blackens diffuse and needs env reflection (gameoverload, 2023-04; r93_sniper, 2022-08). Reach is not metallic, but a high `specular_coefficient` with no cubemap or a black World reads as dark mirror [U].
10. **Normals / geometry**: lost or sharp custom split normals after merging; use "Clear Custom Split Normals" (surasia, 2021-07; fiycsf, 2022-08/10). Overlapping/duplicate meshes (chiefster, 2025-09). Mirrored UVs (chiefster, 2026-08).
11. **Absent vertex-colour mask** on Elite mesh [U] (section 4).
12. **Render setup**: Cycles glossy/transmission/volume bounces off made Reach Warthog metal black (trisckit, 2024-05); Eevee vs Cycles; World lighting, exposure, view transform (koax117, 2026-05/08). Black visor often just means nothing to reflect (incuriouslog/unordinal, 2022-06).
13. **Intentional black**: matte-black zones outside R/G (fiycsf, 2021-05).
14. **Pink/purple is not black**: missing images; File > External Data > Find Missing Files (many). Black can persist after that fix (vexerart/_pandoras, 2026-08).
15. **Foundry bitmap format**: possibly sets Reach bitmaps to sRGB instead of xrgb, which would mis-read linear maps (brooen, 2025-10; Crisp suggests colour profile on the image node instead) [D/S].

### B5B. Over-shiny or wrong shine
1. Check normals: Non-Color, smooth shading, normal wired (soulburnin, 2021-08; gameoverload, 2022-06). Do not wire normal maps into detail strength sliders (chiefster, 2022-01).
2. Spec in diffuse alpha lost (alpha stripped/premultiplied), or alpha mode wrong.
3. Brightness of the primary colour matters in the Blender Reach pack: "brighten your primary, darker values won't shine" (chromacore, 2021-06).
4. Reproduce the chain in 2.5 with `D6D4DB` / `E8C6A4` if the Elite looks too white or wrong. A whitish rim glow = glancing colour/exponent of the cook-torrance group (soulburnin, 2024-08; lever is inference [U]).
5. Terrain-style stacked blends or bump mixes give "insane shine" in Chief's shader (bombwolf, 2022-01).
6. Blender 4.x: older packs render silver/white; replace the Principled BSDF inside the node group, IOR 1.45 (purplmunkii 2024-07; soulburnin 2024-09). Specular split in 4.0 (gameoverload 2023-12). Pack fix is version-specific; "HIMS 2.9 fixes it" is unsupported (pass-2 correction).

---

## B6. Import / export pipeline (Reach / H3 / ODST): commands, tools, versions, errors

### B6.1 Which tool for which game  [C unless noted]
| Game | Rip from .map | Edit tags / EK | Blender import |
|---|---|---|---|
| H3 / ODST | Reclaimer | Tool, Guerilla, Sapien, Foundation | `tool extract-import-info` then toolset (.ass) |
| Reach | Reclaimer (RMF) | Reach Editing Kit (Steam), Foundation, Assembly, TagTool, Baboon | **Foundry** (Crisp). JMS cannot be extracted from Reach, pipeline changed (chiefster, 2024-11) |

Steam IDs: H3 1695791, ODST 1695794, **Reach 1695793** (brooen/sswampp, 2023).

### B6.2 Reach-specific facts
- The Reach editing kit contains all retail tags (chiefster, 2025-03). Reach models cannot be pulled out through mod tools: render data not retained; Foundation "export mesh" gives unusable `.x` files; use Reclaimer (chiefster, 2022-10/2023-07/2025-03).
- The reverse route (custom model into Reach) has NO documented procedure in the corpus. Asked repeatedly: Blender-to-Reach tag tutorial (the1updude 2025-06), sbsp import (makeshift.h2 2025-12), CE model into Reach (khalghoush 2024-07), older-game models into Reach (_manchurian_ 2026-01). All unanswered. "No Reach import tutorial seen" (m0nkeyman117, 2023-08).
- Indirect pointers: H3 tutorials exist and Reach is "built off 3" (gameoverload 2023-05); Foundry with the Reach Editing Kit set as project; drag-and-drop `.scenery`; `Setup as Asset` option for re-import into tools (chiefster/iloveagoodcris, 2025-10/11); animation files must be saved inside the project data directory ("Ensure your file is saved to your Omaha data directory" otherwise; iloveagoodcris/tristangold, 2025-10/12).
- Export from the RENDER object, not a sharp-edged body copy (chiefster, 2023-09, Mauler example) [S]. Use the original source (.ass/.jms) from the EK data folder rather than a Reclaimer-extracted mesh; Reclaimer geometry differs from the original import (chiefster 2026-01; rdt4 2026-01 saw failures going H2A/H3 into Reach) [S].
- Port recipe (chiefster, 2024-02, 2026-08) [S]: export the target game's own render_model, reuse its skeleton and markers; bone names must match exactly; scale must match; rename vertex groups to target bones and merge weights. This is also the fix for "matching node can't be found" in first person and "no animation graph" on a ported biped.
- Textures into Reach via Assembly: DDS to `.tagc` with Forerunner's tool, import under a new name (surasia, 2021-07); importing two different bitmaps in a row duplicated the first (the_heavynator, 2021-08). Keep CC alpha intact [U].
- Cross-game tag copy trick: copy H3/ODST tags into the Reach/HREK tags folder so Foundry can import them; works for Chief, textures can have issues (koax117, iloveagoodcris, 2025-10 to 2026-06) [S]. Foundry does not support H3/ODST natively: ManagedBlam for H3 cannot be loaded in Python.
- Material/shader rename on a render_model: enable expert mode in Guerilla/Foundation and edit shader names on the render model directly (zadeinator/lt_crunch236, 2024-06; expert mode confirmed). Only render_model-level fix in the corpus; applicable to variant shader swaps.
- Compile gotcha (H3 tool): opening the scenario in Sapien compiles shaders; rebuild afterwards (r93_sniper, 2021-10). Do not hand-edit .shader text (corrupted in Guerilla, itzhouze 2021-11).

### B6.3 Commands seen
- `tool extract-import-info <path>` (H3/ODST; writes .ass, extracts, does not import). Path must be editing-kit relative starting with `tags`, or a full system path, NOT starting with `H3EK`; if `H3EK.bat` nests a same-named folder, move the inner folders to the kit root (iloveagoodcris/magmashark009, 2026-02; raw-verified).
- `H3EK.bat` extracts tags; open with `guerilla.exe` to read shader parameters (chiefster, 2025-11).
- Animation extraction (H3 syntax): `<full path>\map.map EditTag objects\vehicles\pelican\pelican.jmad ExtractAnimation all "<outdir>"` (r93_sniper, 2022-09/12). The raw line does not name the tool (Assembly or TagTool unattributed) [D/U].
- Lightmapping: Reclaimer site tells you to run `python calc_lm_farm_local.py` from the H3EK folder; "python isn't found" means Python must be installed and on PATH (ohwhen6442, 2025-06; no confirmed answer) [U].

### B6.4 Tools and versions
- **Reclaimer** (Gravemind2401): File > Open Map. `campaign.map`, `multiplayer.map`, `shared.map` cannot be targets, but `mainmenu.map`, `shared.map`, `campaign.map` must sit in the same folder as the target (chiefster, 2024-05/11, 2025-10) [C]. Set Materials root path to the bitmap export root to fix yellow textures (gallium 2021-05; soundflare 2022-08). 2.0 (2024-08/11): .NET Desktop Runtime 7, RMF format with Blender/3ds Max importers, `Export Metadata` (JSON dump of a tag, useful to diff a working and a broken shader/biped), decorator models for Reach. 2.1.1608 (2025-08): Reach workshop map fixes. **2.1.1624 (2026-04): extra H3 and Reach Alpha builds (bitmaps, render_models only)** (exodst., raw-verified). AMF is outdated on current Blender; use RMF (chiefster, 2025-07). AMF has no vertex colour or multi-UV; DAE does. Reclaimer cannot export shader code or the lightmap UV set (un_nonimus, 2025-12).

- Known Reclaimer errors: "valid .map but may not be a supported version" = old build; `FileNotFoundException ... campaign.map`/`shared.map` = companion maps missing from the folder; "Could not find ... mainmenu.map" on render_models (2022-05); `InvalidDataException: Found invalid data while decoding` on a Reach render_model with maps inside Reclaimer's Data folder (waketheshark, 2022-11, unresolved); `ArgumentOutOfRange` in `MccHaloReach.IndexItem.ReadMetadata` on Forge workshop maps (frankr2994, 2025-02, unresolved); `NullReferenceException` in `scenario_structure_bsp.ReadGeometry` after an MCC update (360 maps work better). Reach is supported (the old "needs Adjutant 4.2.0" claim is stale).
- **Foundry**: Blender add-on for Reach tags (render/collision/physics import); builds shaders automatically; levels ~90% accurate, decals manual (koax117, 2026-09). Install errors: missing numpy; "Import completed in 33 milliseconds" with empty scene (2025-03, unresolved).
- **General-101 toolset**: H3/ODST JMS/JMA/ASS (also CE/H2, see Part E); cannot read .map; some versions need an unzipped `io_scene_halo` folder; "action object has no attribute 'fcurves'" on newer Blender (tristangold 2025-12). camden-smallwood fork has Reach+ work.
- **Bonobo** (LaikaGlove, unmaintained): extracts jmad to JMA/JMM for H3/ODST/Reach maps; needs shared, campaign, mainmenu and a level map in one folder plus .NET. Errors: "unsupported game build" on current MCC Reach maps (2021-12, 2022-04); Reach fp_magnum "Value cannot be null (source)"; first person does not extract. Alternatives: **Baboon** (Zoephie; blam-tags GUI, byte-exact tag edit/extraction for H3/ODST/Reach), HaloAnimationRepository (77Mynameislol77).
- JMA import: "no valid armature" / "Node 'camera_control' not found" = import the armature first (guhgunk, 2022-08). The "comma decimal locale" fix for "could not convert string to float" is inference [U].
- **CR4B Tool** (PlasteredCrab): Blender material builder for H3/ODST; Reach "minimal"/not working (2023-08 to 2024-02). Needs mod tools + tags.zip; best on Blender 3.6. Red nodes = missing `detail_map`. Writes tags only to the "halo 3" folder. Error `key 'shader' not found` on H3 MP Spartans (2024-01).
- **Assembly** edits .map data, no model extraction. **TagTool** (dev branch) modifies tags in built caches.
- Baking from node-group shaders (for engines outside Blender): image node (blank, selected) per material; plug the shader's own output into Material Output; bake type Emit; Cycles; 1 sample; Non-Color for data. Normal: bake type Normal from the BSDF on a duplicate with split normals cleared [Reach: the Blender shader has no bake outputs, so bake diffuse, spec colour and a flat gloss using node-group outputs (chiefster, 2026-08)] [D: gameoverload says only normals bake from the shader output].

---

## B7. Rigging, skeleton, markers, animation

- **Scale**: Reach Spartans use a human skeleton at ~1.2x (frostbitecinem, 2024-03, exact value unanswered). Reach ripped bones can look tiny; check the pack version (degweb, 2022-09).
- **Bone names**: must match the target skeleton exactly; vertex group names must equal bone names, the armature modifier must target the rig, weights non-zero (purplmunkii, 2024-11). H2/H3 rigs are near identical ("iirc", chiefster 2024-07 [S]) so weights port by renaming. Fix wrong weights by removing the verts from unwanted groups.
- **Markers**: included when exporting parts of a model from Reclaimer 1.7.307+. For tool import use the target game's own skeleton and markers from `extract-import-info` (chiefster, 2024-02). No documented list of Reach armour marker names, regions or permutations exists in the corpus.
- **Elite rig**: Reach Elite IK fails mainly because of horse-link (digitigrade) legs; Track To constraint feet vs master rotation; Elite hunch/kangaroo legs make human run cycles look wrong (xxpythonxx 2022-05; r93_sniper 2022-11). Elite leg bones follow the mesh (xxpythonxx, 2022-05; pass 1 mis-credited surdavow). Bone X axis must point along the bone for IK when porting Reach Elites to H3 (winnower, 2026-07). H3 Elite has no chest bone and no public rig; its head is a separate overlapping mesh that breaks auto-rig tools (kozmetic/monokkii, 2025). Auto-Rig Pro and ActorCore AutoRig rigged Elites with little effort (frankr2994, 2025). Mixamo recognises Reach Spartan and Elite rigs, Elite "wonky" (koax117, 2022-06).
- **Animation**: Bonobo/Baboon/Tool route (section 6). Reach fp rig uses different bone names from earlier fp rigs (adorabirb, 2026-08). Elite walk cycles exist for H3 (iloveagoodcris, 2026-10). Reach animation tag binary layout and compression asked, unanswered (superintendent, 2026-01). Armor-lock fall animation not found. Retargeting Reach to Infinite rigs with Mixamo/Rokoko/Mwni failed or exploded (.username_ladi, 2026-01); marine to other rig works when bones are lined up (koax117, 2026-08). Applying one character's animation to a second biped imports onto the first (rylietw, 2021-08, unresolved). Imported animations load into an animation layer.
- **Cinematic exporter**: iloveagoodcris is writing a Blender exporter for Reach cinematics; Foundry can import a cinematic (e.g. last m50 cinematic) from extracted Reach tags (2025-03/12).

---

## B8. Lightmaps, bsp, scenery, Sapien, scripting, AI

Little exists. What was found:
- **Lightmaps**: H3 tool lightmap command asked (raccnoon 2021-10), unanswered. `calc_lm_farm_local.py` (section 6.3). Reclaimer does not write the secondary UV set so Reach/H3 lightmap bitmaps are unusable externally (un_nonimus, 2025-12). Reach lighting info can be imported to Blender by Brooen's Halo-Reach-Light-Importer (`.scenario_structure_lighting_info`).
- **BSP**: Reach geometry is built differently from the trilogy and gives "degenerate edges" in H3 Tool (reply from a Deleted User, 2021-11; the earlier "scaling up" cause was only the asker's guess). Typical H3 import failures: missing shaders, degenerate/duplicate triangles, bad collision edges. Reach map collision as sbsp: unanswered (sosa758 2025-03; makeshift.h2 2025-12). H3 sky is a `.render_model` tag in the scenario's sky folder (chiefster/jackofchaos); Halo skies are meshes, not HDRIs.
- **Campaign to MP conversion (H3)**: set map id and game type to multiplayer, one zone set, remove globals/scripts, `tool build-cache-file`; log still loaded global_scripts and the game gave "Fatal Error"; Sapien error "Failed to find a valid spawn location" until a respawn_point, respawn zone and slayer spawn were added. Final crash unresolved (itzhouze, 2021-10).
- **Sapien**: crash assertion `render_methods/render_method_definitions.cpp #3203, "result != NONE"` loading riverworld after the April 2022 MCC patch, no fix (tafurious). Sapien compiles shaders when opened. H3 device_machine elevator: animation plays on render model but physics model stays at rest in Sapien (bongcloset 2024-10, unresolved).

---

## B9. Networking, multiplayer, anti-cheat: what the Discord corpus has (see PART N for the full researched section)

Honest answer: **almost nothing.** No speaker explains how custom armour tags replicate to other players, whether stock clients accept modified tags, or what EAC does with mod tool sessions.

What exists:
- NinjaRipper on MCC: EAC raises an error but the planted DLL works; launch MCC with anti-cheat disabled afterwards (surasia/Jackal, 2021-07; confirmed process).
- Inference only [U]: armour colour comes from the player's biped `Change Colors` and CC maps, so a client without your modified tags would see defaults; custom content likely reaches others only in mod-enabled, anti-cheat-off sessions. No speaker said this.

---

## B10. Resources and authors to trust

### Authors
- **gravemind2401**: Reclaimer / Architect author. Authoritative on formats, vertex colour, versions.
- **iloveagoodcris / Crisp**: Foundry author. Authoritative on Reach import.
- **chiefster_4014**: most frequent, knows Blender packs and mod tools; consistent but gives some single-source claims.
- **soulburnin**: Reach/H3 Blender shader author; fresnel/spec chain, globals values.
- **the_heavynator, pointdevice, chromacore**: shader reverse engineers (pointdevice uses frame capture; disagrees with heavynator on the roughness recipe).
- **r93_sniper**: #control-map-guide author. Carefully hedged by the author.
- **gameoverload**: shading theory, baking, node groups; occasionally speculative on engine internals.
- **unordinal**: Change Colors investigation. **plastered_crab**: CR4B. **zoephie**: Baboon. **fiycsf**: Reach Unity shader. **general-101**: toolset.

### Links
- Reclaimer https://github.com/Gravemind2401/Reclaimer ; Architect .../Reclaimer.Architect ; releases pages for v2.0.1536, v2.1.1608, v2.1.1624.
- Foundry https://github.com/ILoveAGoodCrisp/Foundry
- General-101 toolset https://github.com/General-101/Halo-Asset-Blender-Development-Toolset ; fork camden-smallwood
- Assembly https://github.com/Lord-Zedd/Assembly ; TagTool https://github.com/TheGuardians/TagTool/tree/dev
- Baboon https://github.com/Zoephie/Baboon ; HaloAnimationRepository https://github.com/77Mynameislol77/HaloAnimationRepository
- CR4B https://github.com/PlasteredCrab/Halo-CR4B-Tool
- Reach Unity shader https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity (commented; could show how Reach CC/spec/masks combine [U])
- Mjolnir-Forge-Editor https://github.com/Waffle1434/Mjolnir-Forge-Editor

---

## B11. Contradictions and unresolved questions

Contradictions
1. Reach CC: R/G only (guide) vs R/G/B/A elements (unordinal) vs "RG used, sheet shows R" (chiefster 2026) - 1.2.
2. Which tint carries Elite fresnel: slot 2 vs globals E8C6A4 vs emissive vs cubemap tint - 3.2.
3. Per-pixel roughness: retail feature (heavynator) vs H3-beta leftover (pointdevice) - 2.4.
4. Spec tint per material (chromacore) vs overridden by biped Change Colors (unordinal) - 3.1.
5. H3 coloured spec map exists (`_s`, soulburnin) vs H3 does not support it (heavynator).
6. Reach PBR or not; Beckmann or not (semantic and unresolved).
7. Baking: from BSDF fine (chromacore 2023-11) vs end-node outputs only (gameoverload 2022-12, 2025-02) vs Emit routing worked for Reach users (chiefster; Reach pack users 2025-11). Likely version dependent.
8. Normals: flip green in Blender (gameoverload) vs keep DirectX in a Halo pack's Normal Map node (chiefster). Depends on shader type.
9. Reach vertex colour: Elite body uses it (heavynator) vs gravemind recalls none.
10. Reclaimer supports Reach or needs Adjutant (stale claim) - resolved in favour of Reclaimer.
11. Gamma 2.0 (game param) vs 2.2 assumption (chiefster/gregory0701, 2025-04; undecided).
12. Blender 4.0 fix: replace BSDF vs new pack version; both seen.
13. Guerilla shows shader code (fable_law_458) vs parameter pages only (gregory0701).

Unresolved
- What mechanism decides whether Change Colors element 2/3 act as fresnel/spec tint (flag?).
- RGB meaning of the Reach `material_texture`; actual retail use.
- Cubemap colour space and cubemap alpha purpose.
- Custom model/armour import into Reach (tool.exe or Foundation) - no documented procedure.
- Reach permutations/regions/marker names - undocumented here.
- Why Bonobo rejects MCC Reach maps; Reach fp animation extraction.
- Lightmap bitmap/UV extraction from Reach/H3.
- Reach Forge map failures in Reclaimer; Foundry empty-scene imports.
- Foundry xrgb vs sRGB bitmap handling.
- How to extract exact Reach armour CC colour values.

---

## B12. Corrections log (Reach/H3/ODST items; pass-2 versions used)

- Tertiary-to-fresnel "resolved": no, still unverified (unordinal planned an in-game test). Ranger `illum_cards` concerns self-illum colour, not fresnel. HLSL albedo-subtraction direction is ambiguous.
- H3 "B = tertiary" for `mp_masterchief_cc`: sswampp gave only R/G; tertiary on H3 armours is a later unconfirmed remark (2022-06).
- Dropped: "Reach needs Adjutant 4.2.0" (contradicted by many Reach rips in Reclaimer); a clock-rollback offline tip (not MCC/EAC); a "banshee-style shine" claim (not Reach); an "IOR 8/10 and emblem-mix" chain (game unnamed).
- Misattributed/overstated: height-to-normal for "H3 and Reach" (the speaker said earlier titles only); fiycsf "alpha-spec" (about Reclaimer-exported textures, not the control map); "Elite leg bones follow mesh" (xxpythonxx, not another speaker); structure-error causes (askers, not answerers); mainmenu.map consequence date.
- Downgraded to unverified: JMA locale fix; Reclaimer vertex colour for Reach via position-W (effectively none); "bake from BSDF confirmed"; rank-cubemap guess.
- Other-game corrections are in Part E.

---

# PART C - OFFICIAL DOCS AND PROJECT REFERENCES (Reach HREK, H3 docs, c20, Halo Mods)
Sources: Microsoft Learn (halo-master-chief-collection), c20.reclaimers.net guides, Halo Mods #materials export (209 messages, 2022-04 to 2026-09; digest in DISCORD-HALOMODS-MATERIALS-SHADER-DIGEST), Tyler's attachments. "(unverified)" = chat message, not docs.

## C1. Pipeline (Reach HREK)
- .gr2 (Granny) replaces ASS/JMS. Flow: FBX -> `tool_fast fbx-to-gr2 <fbx> <json> <gr2>` -> `tool_fast import <x.sidecar.xml>`. Levels then: `python calc_lm_farm_local.py <scenario> <bsp|all> <quality>` (quality: high, medium, low, direct_only, super_slow, draft, debug). `tool` and `tool_fast` behave the same; tool_fast is multithreaded with fewer error messages.
- FBX rules: transforms on skeleton nodes, not meshes; material names must match `material_properties` keys exactly; node/mesh names are the JSON keys; regions via `bungie_face_region`; markers = nodes with object_type marker + marker_type.
- JSON: auto-created on the first run and NEVER updated afterwards (edit it, then reconvert); keys need the `bungie_` prefix; mesh-suffixed properties are duplicated in nodes_properties and meshes_properties; a material missing from material_properties silently becomes `tags\shaders\invalid.shader`. Sidecar region definitions win over JSON; the `default` region must be index 0; indices/order must match when replacing an existing model. Global material overrides only in JSON.
- shader_path in the JSON has no `.shader` extension and no stray backslash. If a shader tag sits in the model's own shaders folder with the right name, a re-import finds it automatically.
- Sidecar folder layout: models\ (FBX+JSON), export\models\ (GR2), bitmaps\; sidecar filename = model name; start from the samples example_level / example_tentacle / example_health_pack.
- Can read `bungie_shader_path` from FBX custom properties. Instance imposters and marker permutations supported. Stock tags lack import info.
- Known issues: no resource sharing; no single-threaded lightmapping (use multi-process with one client).
- H3 folders for comparison: data (FBX/ASS/JMS/TIFF/WAV), tags, maps; shaders folder = material shaders, rasterizer = global shaders. The `script_doc` console command writes hs_doc.txt.
- Page: https://learn.microsoft.com/en-us/halo-master-chief-collection/hr/art/fbxtogr2 (previously read to char 100,000 of 104,905; the last ~5 KB is unread).

## C2. JSON parameter catalogue (c20 /hr/guides/json-parameters/; the page has typos)
- Material: shader_type (override, shader, shader_cortana, shader_custom, shader_decal, shader_foliage, shader_fur, shader_fur_stencil, shader_glass, shader_halogram, shader_mux, shader_mux_material, shader_screen, shader_skin, shader_terrain, shader_water), shader_path.
- Face: face_type, face_mode (normal, render_only, collision_only, sphere_collision_only, shadow_only, lightmap_only, breakable), face_sides (one/two sided, +transparent), face_draw_distance, face_global_material, face_region, texcoord_usage, decal_offset, group_transparents_by_plane, no_shadow, precise_position, conveyor/ladder/slip_surface.
- Mesh: mesh_type, mesh_primitive_*, mesh_tessellation_density, mesh_additional_compression, mesh_poop_lighting (none/single_probe/per_pixel/per_vertex), mesh_poop_imposter_*, mesh_poop_precise_geometry, mesh_poop_excluded_from_lightprobe, portal/decorator/seam/water/fog properties.
- Lightmap: lightmap_type (per_pixel/per_vertex), resolution scale, chart group, bounce modifiers, translucency tint, lighting_from_both_sides.
- Material lighting: lighting_emissive_color/power/focus/quality/per_unit, lighting_attenuation_cutoff/falloff, lighting_bounce_ratio, lighting_use_shader_gel.
- Lights: light_type/shape/game_type, attenuation, hotspot, frustum, color, intensity, clipping sizes, gel/lens flare/shader references.
- Markers/physics/animation: marker_type list, physics_constraint_*, animation_control_*, animation_event_* (13 types incl. loop, sound, footstep, ik_active, ik_passive, frame, import).
- PRT is NOT mentioned anywhere on this page (read in full).

## C3. Shader docs (MS Learn H3; applies conceptually to Reach)
- Index: https://learn.microsoft.com/en-us/halo-master-chief-collection/h3/shaders/shadershome (about 50 option articles: albedo, bump, alpha test, specular mask, material model, environment mapping, self illumination, blend mode, parallax, rim, subsurface, templates...). All shader work is done in Guerilla/Foundation; choosing a specific template is "no longer necessary" (legacy). The H3 albedo page documents seven of its ten options; the HR-specific shader pages were not found, so H3 notes are only a guide for Reach.
- **Albedo options**: default, detail_blend, constant_color, two_change_color, four_change_color, three_detail_blend, two_detail_overlay (undocumented), two_detail, color_mask (undocumented), two_detail_black_point (undocumented). base_map alpha can drive specular_mask, material_model, alpha_test, blend_mode. In `default`, albedo_color_alpha scales the alpha grayscale; in detail_blend the base alpha masks the detail (black = full detail).
- **change_color_map**: two_change_color uses R (primary) and G (secondary); four_change_color uses R,G,B,A = primary, secondary, tertiary, quaternary. Colours are set in the object tag.
- **Material model**: diffuse_only (no specular; default), cook_torrance (per-pixel specular/reflection/roughness/albedo blend; options order3_area_specular, use_material_texture; NEEDS environment mapping or the sides of the model go black), glass, organism, single_lobe_phong (diffuse_coefficient, specular_coefficient). Per-pixel specular is set in Specular Mask; the env-map source is set in Environment Mapping (none = no reflections regardless of material model).

## C4. tool.exe hex tips from Discord (unverified, build-specific)
- Skip degenerate tri: Reach 0x1E69FD. Vertex weld/nodes: 0x2395EC. Cubemap full-mip: Reach 0xD5457 (NOP 12 bytes).
- Use "precise" for Reach models. PRT generation can fail on high-poly meshes.
- Colour change: R/G/B/A = primary..quaternary, black = none; the quaternary is only available if the cc has all 4 channels (Jackal note). Nexus MCC mod 2481 edited Jackal bitmaps to use the D (alpha) channel for colour change in biped tags.
- "Black uniform no matter what the controls": cause was "specular mix".
- Template regeneration (c20 H3 guide, applies to Reach in practice; not required for rendering, it only caches templates): `tool dump-render-method-options`, then `tool generate-templates win shaders\<type>` (types: shaders\shader, particle, halogram, decal; about 30-60 min).

## C5. Tyler's attachments (summaries; re-attach originals for the full tables)
### C5a. Particle limit: Halo Reach MCC 65,535-particle hex patch (haloreach.dll 1.3528.0.0)
- Original size 13,229,016 bytes, SHA-256 738dd2d2...0894; patched 13,229,568 bytes, SHA-256 02239e6d...78f5. Raises GPU particles 7,168 -> 65,535, spawns/frame 576 -> 65,535, systems/locations 576 -> 4,608, CPU emitters 832 -> 6,656, CPU particles 2,048 -> 16,384, game state 0x1380000 -> 0x1BA0000. Seven steps: pool sizes; count-check branch flips + code redirects; memory-bank/checkpoint sizes; snapshot buffer displacement redirects (20); append 40 zero bytes + a 512-byte code block at 0xC9DC00; new `.pbudget` PE section header at 0x388; PE header fields (sections 7->8, SizeOfImage, checksum, clear security dir). Apply to a COPY of haloreach.dll with the original backed up. Not applied to Noble.
### C5b. forge_strings.txt (Reach Forge UI string table)
- ~1,138 lines of `string_id = "text"`: forge menu/help text, object palettes (weapons, vehicles, gadgets, structure, scenery, MCC additions "ff_thorage_*", "ff_cex_*"), team names, named-location strings per map. Useful as a string-id naming reference for Noble's menu strings / noble_menu.string_ids.csv.

## C6. Other leads
- General101 Halo Asset Blender Development Toolset (GitHub); c20 Discord discord.reclaimers.net.

## C7. Halo Mods #materials "shader" digest: takeaways (Reach)
- **Shader breaks when a colour-change map is added** (2024-09-19): the texture swaps to an unrelated one. Cause: the shader template in the postprocess block does not exist (Foundation: hold Alt when opening the shader; a RED template entry = missing). Fix: compile/generate the template. Reach ships ~2100 templates (H3 ~2500) but you often must generate more. "Reach is very picky about shader properties": copy a stock armour shader (e.g. marine armour) and change only the bitmaps.
- **Colour channels**: R primary, G secondary, B tertiary; quaternary is only seen via a four_change_color shader and its bitmap.
- **Reach has no PBR shader** (no Bungie/343 one): only diffuse, specular (standalone, or packed in the diffuse alpha via "Specular mask from diffuse"), bump/normal, height (parallax). A community H3 PBR shader guide exists (https://c20.reclaimers.net/h3/guides/shaders/pbr-shader/); one person asked if it works for Reach.
- **Bitmaps**: diffuse alpha = specular map or transparency (alpha blend). `_diffuse` suffix is the default usage. Force format: DXT1 (alpha becomes 1-bit; do not use if alpha matters), DXT5 (keeps grey alpha), 24-bit colour + 8-bit alpha or best uncompressed for the highest quality. The specular mask controls how shiny something is even if the shader would otherwise allow shine.
- **Metallic look in Reach**: enable a material model (cook_torrance is a good start) and environment mapping; copy settings from an existing metal shader. Cubemap: set the shader's `environment_mapping` category to `per_pixel` and pick a cubemap bitmap (the assault rifle shader is an example).
- **Dynamic lights**: shaders have a "no dynamic lights" style checkbox; shadows still apply.
- **Finding a shader**: in Sapien, `collision_debug 1` shows the shader name/path of a surface (disputed for objects); or import the BSP/model into Blender and read the material name, then open that shader in Guerilla/Foundation.
- **Foundation caveat** (2023): a user could not save shader changes in Foundation; use Guerilla then.
- **Spec tint map tiling** is missing in the Bungie cook_torrance shader; adding it means editing material_models.fx / cook_torrance.fx (transform_texcoord).
- **Shader functions**: only float values animate in shader functions; periodic functions do nothing directly on frame index/colour values unless fed from an object function; evaluators take time as a parameter.
- **Wiki/tools mentioned**: Microsoft Learn shader docs, c20 compiling-shaders, c20 pbr-shader, Crabtool, "Foundry".

## C8. Project pointers
HANDOFF-ELITE-RR-TEXTURES-2026-11-12.md and FINDINGS-ELITE-ORIGINALS-COMPARE-2026-11-12.md (project state); DISCORD-HALOMODS-MATERIALS-SHADER-DIGEST-2026-11-12.md (full #materials text).

---

# PART D - RECONCILIATION AND GAPS (Reach / H3 / ODST only)

## D1. Where Parts A, B and C agree
- Four_change_color = R,G,B,A (primary..quaternary); two_change_color = R,G only (C3, Halo Mods digest, B3.1). Quaternary needs a 4-channel cc.
- Reach has no PBR shader: coloured specular + scalar roughness, cook_torrance; the spec mask is the diffuse alpha (C7, B2.1).
- Cook_torrance needs an environment map or the model goes dark/black (C3); B5A item 9 and A2 step 2 follow from it.
- Copy a stock shader when something misbehaves (C7 "Reach is very picky", A2 step 8).

## D2. Where they need care
- The Discord "R primary, G secondary, B tertiary, quaternary via four_change_color" (C7) matches the MS Learn option text; the Discord Archive guide says "B unused" (B1.1/B1.2). Resolution: B/A are used only by shaders using four_change_color; check the shader's albedo option first (A2 step 1).
- "A CC alpha of 0 is normal for Reach _cc files" (B3.3) means a zero alpha only matters if the shader reads the alpha (four_change_color). It does not by itself explain a black wing.
- The slot-2/3 = fresnel/spec tint link (B3) is UNVERIFIED; C only confirms that the biped's colours come from the object tag.
- tool.exe hex offsets (C4) are single-source and build-specific; test on a copy.
- Per-pixel roughness recipe (B2.4) is disputed; the H3 material-model docs (C3) list use_material_texture as a cook_torrance option, which supports that the option exists but not that retail Reach assets use it.

## D3. Not read / not found (so do not assume)
- Not read: the Gibbo HREK guide, the Reach Modding Guide slides, BLF, Blam Monitor Master, TagTool, the c20 animation-data page (fetch failed), the last ~5 KB of the FBX->GR2 page, the undocumented H3 albedo options (three of ten), the HR-specific MS Learn shader pages (not found), Megalo and the H3 Quick Start pages (not captured in this guide: the reference document available to this merge does not contain them).
- Halo Mods mod-development channels and Reclaimers #reach-help were never exported (permissions/disconnect).
- No source anywhere documents: importing custom armour into Reach as a tag workflow beyond the HREK FBX->GR2 pipeline in C1, Reach region/marker/permutation names for Elite armour, or networking/replication of custom armour pieces to other players.

---

# PART N - NETWORKING (Halo 3, ODST, Reach, and MCC)
Written 2026-11-12 from web research. This part SUPERSEDES section B9 ("almost nothing"), which only covers what the Discord corpus said. Tags: [C] primary source (Bungie talk/slides, 343 posts), [S] secondary summary or fan guide, [U] my inference, not stated by any source.
Primary source: David Aldridge (Bungie lead networking engineer), "I Shot You First: Gameplay Networking in Halo: Reach", GDC 2011. Read via slide transcriptions (slidetodoc, studylib) and summaries (Wolfire, Edgegap, UCL). Original slide file not read directly.

## N1. Architecture (Halo 3, ODST multiplayer, Reach)
- No dedicated servers in the originals. One console is the host ("physics host"); the others are clients. Host is authoritative over damage and object creation/destruction. [C]
- Design lineage: "The Tribes Engine Networking Model" (Frohnmayer and Gift, GDC 1998/99): host/client, cheat-resilient, semi-reliable delivery, persistent state plus transient events, bandwidth that scales to what is available. [C]
- Rejected alternatives (per the deck): lockstep/deterministic input passing, reliable transport (TCP or homegrown), sending all state as one atomic blob (Quake 3 model). [C]
- Bandwidth need grows as O(N squared) with player count; Halo targets 16 players, vehicles, hundreds of replicated objects (Edgegap: designed for 2,000+ synced entities). [C]
- Stack, bottom to top: transport (sockets) -> channel manager / flow and congestion control -> three replication protocols -> prioritization -> game interface (extract/apply). [C]
- Halo 3 differs from Halo 2 by allowing "limited client trust": if latency stays under a threshold, a client's action completes, even outside normal auto-aim range. A fan guide (Bungie.net forum v1.7, May 2007, written during the beta) says so; treat as secondhand. [S]
- ODST/Halo 3 campaign: Firefight and campaign co-op use a synchronous LOCKSTEP model (host collects every player's input, bundles it, everyone advances one frame). ODST multiplayer and Forge use the distributed host/client model. Bungie said ODST's co-op reuses the Halo 3 campaign model. (Bungie Weekly Update, 2009.) [C]
  - Lockstep consequences: any input needs a round trip to the host before the game reacts; packet loss stalls everyone (inputs are ordered, so the whole game waits for retransmission). Bungie's advice: do not share the connection, stop background downloads, wired or interference-free wireless, play with nearby players.
- Reach has two models: PvP host/client, and a separate one for Campaign/Firefight (GDC 2015 Destiny talk). Reach Firefight was notorious for lag; for MCC, 343 said Firefight matchmaking moved to dedicated servers with a new asynchronous model. [C/S]

## N2. The three replication protocols (Reach) [C]
| Protocol | Direction | Guarantee | Examples |
|---|---|---|---|
| State data | host to client only | latest value eventually arrives; intermediate values may be skipped | position, health, capture timer, ~150 more properties |
| Events | both directions | none (unreliable notifications) | fire request, weapon fired, projectile detonated, ~50 more; they explain why, state carries the truth |
| Control data | both ways | best effort, high frequency | host to client: all players' analog stick values; client to host: own biped position; ~15 more; ~20 bits per player |
- Guarantees are mixed per message type: guaranteed (game-over notice, grenade-throw request), ordered only (positions), none (sparks, bullet decals). (Wolfire)
- Replication is never fully reliable; every packet is filled in priority order because there is always more data than fits.

## N3. Prioritization [C]
- Priority is computed per object, per client: distance and direction first, then size and speed; shooting and damage add boosts; special cases (thrown grenades rank above what threat alone would give; dead bodies get near-max relevance for several seconds; idle grenades update ~3 Hz).
- Output per object: final priority, relevance, desired update period in ms. Low priority means slower updates, never a stall.
- Flow control ensures low-priority traffic never starves high-priority traffic.

## N4. Hiding lag: how Bungie designed mechanics [C]
Four rules: (1) decide what needs a single authority, (2) always ask where the lag is hidden, (3) change game mechanics if it helps networking, (4) reserve time to iterate.
- Grenade throw: client predicts the animation but not the release; the host creates the real grenade at the release frame and the predicted one is deleted. Lag lands on grenade appearance.
- Armor Lock: shipped version shows animation/FX immediately and the host starts invulnerability after a delay (3 frames plus round trip).
- Assassination: strict obey-the-host for all peers, including participants; the predict-locally version worked in playtests but failed in the public beta.
- Melee: needed post-ship fixes in Halo 2 and 3; rare host-ignored client strikes in the Reach beta.
- Shots are sent target-relative ("at this player's head"), so the host judges against the target's position at that time. (Halo 3 Bungie post: client sends who it aimed at and how far ahead; the same hit happens on the host two ticks later.)
- Prediction shown instantly: muzzle flash, sound, animation, aim, movement. The host confirms damage, spawns, grenades.

## N5. Bandwidth numbers [C]
- Halo 3 breakdown: 50% positions/velocities/orientations, 20% player control data, 20% weapons/bullets/damage, 10% other. Host to client control replication alone was 22% of host upstream.
- Reach cut bandwidth by over 80% (to about 20% of Halo 3). Four of five major wins were gameplay changes, not encoding:
  - Control replication: removed duplicate/unneeded data, 60% less (14% overall).
  - Equipment priority bug: dropped grenades inherited an equipment boost via a shared parent class; limiting the boost to ACTIVE equipment freed about 20%.
  - Ragdolls: no longer networked (only initial state); bullets and grenades pass through them; about 10-12%.
  - Item friction: constant artificial friction replaced by fake friction.
  - High-rate-of-fire weapons: some damage effects client-predicted; statistics traffic capped at 10% of a packet or less.
  - Encoding tricks (fixed-point positions, limited ranges) were a false start; a bucketing scheme saved 1% and cost 30+ hours of debugging.
- Reach figures from the deck: 250 kbit/s minimum total upstream for the host of a solid 16-player game; 675 kbit/s maximum from any single peer; 45 kbit/s maximum host to one client; ~1 kbit/s to replicate one combat biped to one client; 10 Hz minimum packet rate for solid play; latency limits 100 ms (tournament) / 200 ms (casual) close quarters and 133 / 300 ms ranged. (Edgegap adds: 384 kbps design goal; naive send-everything would be ~20 Mbps; playtesters noticed nothing up to ~150 ms.)
- Encoding rules: raw bits for rare values, limited range for common values, floats to fixed point, exploit temporal coherence; default to a high packet rate and lower it when the simulation gets rich.

## N6. Host selection, migration, tooling [C]
- Connection quality is recorded per player and matchmaking picks good hosts. Host migration moves the game to a better host if the host drops; the deck says it is hard to add late. (A fan source says a late-joining player with a better connection does not take over unless the host lags out. [S])
- Tools Bungie built: bandwidth/priority profilers; profile data spliced into "films" (deterministic replays) so network use can be analysed after the fact; monthly network playtests under traffic shaping; a controller button for "report lag"; full-time network testers.
- "Measure first, big picture over encoding, work with designers."

## N7. Master Chief Collection [C unless noted]
- MCC uses dedicated servers for matchmade games and the custom game browser, running the Unified Dedicated Server (UDS), a trusted authority that cannot be tampered with like PC clients (Halo Waypoint, 2022 incident post). Configuration failure falls back to a peer host, the way custom games work. Custom games and campaign co-op historically peer-hosted (2014 statement).
- Tick rate: Halo 3 clients sent 15 packets/s and the server 30/s; the 2019 update made both 60/s, including peer-hosted games. Reports list Halo 2, Halo 2 Anniversary, Halo 3 (one lists Halo 4 as well); none name Reach, so Reach's MCC packet rate is UNVERIFIED.
- Side effect of a higher tick rate: each tick covers less time, so per-tick projectile reach changed and weapon feel shifted (SteamDB patch notes about Halo 2: tick went from 1/30 s to 1/60 s).
- Reach in MCC: matchmaking relays can be disabled in Network Settings if matchmaking misbehaves (Windows Central, Steam threads). Campaign co-op still shows input lag; Halo 3 campaign co-op is reported worse unless the frame rate is locked to 60 (community reports [S]); 343 said fixing co-op latency "would require rewriting netcode" (screenshot quoted by players [S]).
- Halo Infinite favours the shooter with server-side rewind (latency compensation) per 343, a design choice to compare against Halo 3's host authority.

## N8. Practical connection checklist for players (stable connection, fewer drops) [mixed]
1. Wired Ethernet; wireless mesh will cost packet loss no matter what (Steam MCC troubleshooting). Packet loss is the biggest single killer in Bungie's own advice.
2. Nothing else using the line: downloads, streaming, VPNs (VPN raised packet loss in Infinite fixes lists).
3. NAT type open/moderate. Ports frequently quoted for MCC/Xbox services: UDP 88, 500, 3074, 3075, 3544, 4500 (and TCP/UDP 3074). Steam-listed ranges: TCP 3074, 27015-27030, 27036-27037; UDP 3074, 3544, 4380, 4500, 27000-27031, 27036. [S, community]; Halo Support's "no ping to datacenters" article says to open UDP 3075 plus Xbox network ports.
4. Allow MCC through the firewall; IPv6 on; no drawing tablets plugged in (Microsoft Answers).
5. If matchmaking/finding games fails: switch off relay in MCC network settings.
6. Prefer a good host for custom games: pick the player with the best upstream (Bungie recorded connection quality; MCC custom games are peer-hosted).
7. Cap frame rate at 60 for Halo 3 co-op (community-reported).

## N9. What this means for Noble and mod content [U unless stated]
1. The host is authoritative; clients simulate predicted copies. A client that lacks your modified tags cannot render or simulate your content. Every player in a session likely needs identical tag/map data. (Cf. Project Reclaimer: "every player in a game needs the same version".) No source documents how stock MCC clients handle custom tags; test with two PCs, do not assume.
2. Armour LOOK (render model, shader, change colours) is local cosmetic state; state that others must see must be replicated by the player's biped/armour data. Keep new armour cosmetic-only; avoid adding new always-on replicated properties.
3. Spend like Bungie: new networked objects, effects, vehicles and particles cost bandwidth. The 65,535-particle patch (C5a) raises client limits but nothing in this research suggests it is network-safe; test.
4. Avoid: always-on per-player data, many small physics objects, ragdoll-like simulated props, high rate-of-fire projectiles with per-bullet damage. Bungie's biggest savings came from removing exactly these.
5. Add a version/hash check: refuse to join when pack versions differ (Project Reclaimer does exactly this).
6. Measure: use traffic shaping to simulate 25-100 ms and 1-3% loss; use packet capture or the engine's own network debug (the engine has network statistics; names unverified) to count bytes per player with and without the mod; watch the 45 kbit/s per-client and 250 kbit/s host budgets as sanity checks for 16 players (Reach era; MCC at 60 Hz will use more).

## N10. Ideas to improve networking after 12 years
Sourced ideas (existing community engine work and 343 statements):
- Project Reclaimer Engine (Halo 3 recompiled to Rust, community project): rebuilt network layer; encrypted authenticated UDP plus a reliable stream for chat/votes/downloads; separate send thread; "enhanced networking" prioritises objects by how wrong they would look on each client's screen (in view update more, zoomed scope more, behind less); remote vehicles blend toward updates; small position jumps smoothed over ~100 ms instead of snapping; own-vehicle prediction window grows with speed and ping (host-limited); numbered events so duplicate packets cannot apply twice; host migration; STUN plus masters for NAT punch-through (up to four masters, no port forwarding in most cases, symmetric-NAT pairs excepted). Its own benchmark (64 clients, 25 ms, 1% loss): worst 1% aircraft position error 6.4 to 0.74 mrad; data downloaded by a watcher 13.9 to 6.6 MB; kinks 28 per minute to 0.5; Classic (Halo 3 model) remains selectable. It states bad connections stay bad: it "still predicts the way Halo 3 does".
- 343 for MCC: dedicated servers for matchmaking, 60 Hz tick, asynchronous model for Reach Firefight.
- Bungie's own ideas: connection-quality-aware host selection and migration; target-relative shot data.

My ideas, labelled inference (none claimed by sources; each needs testing):
1. Priority tuning: bias update rates by what each viewer can see (the same idea Reclaimer uses). For Reach, priority lives in game code/tags; nothing documented is exposed to tag editing, so this means engine work, not a mod.
2. Smooth, don't snap, remote players and vehicles: client-side smoothing over ~100 ms (as Reclaimer does) would hide corrections on high-jitter links.
3. Relay/hosting: a relay or small VPS acting as a dedicated host (the UDS model) removes the weak-host problem and NAT trouble, but MCC does not let mods run a UDS, and anti-cheat blocks tampering; feasible only for an offline/anti-cheat-off community mode.
4. Lobby-side quality checks: measure RTT/loss between all players before the game and pick the lowest-RTT host (what Bungie did in matchmaking). A custom lobby could do this at the community level (Discord bot, mod menu) even if not in the engine.
5. Version pinning and hash checks to prevent desyncs from mismatched content.
6. Players: prefer wired, 60 fps lock for co-op, low-latency DNS/router QoS (SQM/fq_codel) to prevent bufferbloat under load [general networking practice, not Halo-specific].
7. Test harness: a repeatable emulated-network profile (25 ms/1% loss as a baseline, 100 ms/3% as a bad day) and a "report lag" key, as Bungie did.

## N11. Gaps (not found)
- Reach MCC packet rate, tick rate (30 Hz originally per community; MCC rate unverified).
- Whether stock MCC clients accept modified tags, and what Easy Anti-Cheat does in modded sessions (mod launch already requires anti-cheat off per the Discord corpus, B9).
- Direct reading of the GDC slides (only transcriptions/summaries read).
- The Bungie.net network guides by "One One Seven" (v2.1.1, 2009) exist at halo.bungie.org/misc/networkguide.html; only a snippet was read (a "latency window" of about 300 ms in Halo 3 for host acceptance of client actions, stated secondhand).

## N12. Sources
Wolfire GDC summary (wolfire.com/blog/2011/03/GDC-Session-Summary-Halo-networking/); Edgegap deep dive; UCL summary; slidetodoc and studylib slide transcriptions; Bungie Weekly Update 6/20/08 and the ODST update (halo.bungie.org/bwu item 182 and 249); fan guides (Bungie.net forum post 344898; halo.bungie.org/misc/networkguide.html); Halo Waypoint "MCC Server Incident Summary"; GameSpot and Gamepur on the 60 Hz update; Windows Central/PC Gamer on Reach Firefight dedicated servers; GDC 2015 Destiny talk (Justin Truman); Project Reclaimer engine page (projectreclaimer.dev/engine); 343 "Closer Look at the Halo Infinite Online Experience"; Steam MCC troubleshooting threads; Halo Support articles.

---

# PART E - OTHER HALO GAMES (NOT Reach / H3 / ODST)
Everything here is deliberately kept apart. Do NOT apply it to Reach, H3 or ODST work. Covers Halo CE, Halo 2 (Vista/Anniversary), Halo 4, Halo 5, Infinite, Halo Wars and the H2A/H2AMP remasters.

### E1. Multipurpose maps (CE, shown for comparison; format-dependent)
- Xbox / OG Xbox: R = specular/reflection mask, G = self-illum, B = primary CC, A = auxiliary. PC / Gearbox / MCC: R = auxiliary, G = self-illum, B = specular, A = primary CC (r93_sniper, 2021-06, 2021-10, 2021-12; raw-verified) [C]. If you rip from MCC you almost certainly get the PC layout. The format cannot be told from the file name, inspect channels. CE supports ONE CC colour.
- chiefster_4014 (2025-11, 2026-05) says CE meaning depends on shader tag settings and quotes Xbox as R detail mask, G self-illum, B reflection mask [D vs the guide's R=spec].


### E2. Other games (so you do not apply them to Reach)
- **Halo 2**: two changeable colours, spec in diffuse alpha, CC R/G (r93_sniper, 2021-12) [C].
- **Halo 4** control maps: Type 1 (weapons/props, suffix "multi") R = spec coefficient, G = spec exponent, B = reflection mask. Type 2 (MP armour, no separate diffuse) R = spec coefficient/reflection, G = DIFFUSE, B = spec exponent, A = colour change; Spartans' diffuse sits in the green channel (chiefster_4014, 2025-01). Alpha: white = primary, grey = secondary, black = not coloured (soulburnin/r93_sniper) vs "black secondary, white primary" (Waffles, relayed by surasia, 2021-07) [D]. This two-tone alpha is H4 ONLY. Do not apply it to Reach.
- **H2A MP** (PBR): control R = metalness, G = AO, B = self-illum, A = smoothness; CC R/G (r93_sniper, 2022-01) [S]. H2A SP Elite CC mask is in diffuse alpha (chiefster, 2026-09).
- **Halo 5**: MP Spartan control R = zone 1, G = zone 2, B = metallic, A = smoothness (fiycsf, confirmed by the Open Halo 5 shader author). The community believed blue = Spartan colour for a year (fiycsf, 2021-04): verify channels with a working shader.
- **Infinite**: ASG R = AO, G = scratch, B = grime; Mask0/Mask1 hold zones; speakers disagree on zone numbering [D]. Not Reach.


### E3. Halo CE Elite
- CE Elite (not Reach): one CC colour, so CC tints the whole area; the real colour comes from the cubemap (r93_sniper, 2023-01). dustyoo10 (2025-02) found CE Elite looks accurate with base multiplied by the cubemap.

### E4. Tool routes per game (non-Reach)
| Game | Rip from .map | Edit tags / EK | Blender import |
|---|---|---|---|
| CE | Reclaimer (rig scale 32.8084 needed, chiefster/exodst 2025-11) | CE mod tools | General-101 toolset (.gbxmodel direct; Tool has no model export before H2) |
| H2 | Reclaimer | Tool extract commands | General-101 toolset (JMS/JMA/ASS). H2 has separate commands for render, physics, collision, structure |
| H4 | Reclaimer | H4 EK | Foundry |
| H2A MP | Reclaimer (BLAM) | H2AMP EK | Foundry. H2A SP: Index |
| H5 / Infinite | Reclaimer H5 branch; Ekur / HIMS for Infinite | no kit | Brooen H5 importers; not Reach relevant |

### E5. Rig scale in other games
- Reclaimer-ripped CE rigs need 32.8084; H2 JMA imports can arrive ~33.33x too large (chiefster/exodst, 2025-11). H2A Elite with H2 JMAs stretches because the skeleton scale differs; scale the H2A Elite to match the H2 classic Elite (chiefster, 2024-08) [S].

### E6. Other-game items removed from Part B
- Shader source: CE, H2, H4 and H2AMP shader code also ships with the MCC mod tools (chiefster_4014, 2025-12); H2ASP/CEA shader code can be extracted from the game; H5 and Infinite shader code is not obtainable (chiefster_4014, 2024-09).
- PBR conversion rule of thumb: if spec RGB channels are close it is dielectric (base = diffuse); otherwise metallic (base = spec) [S]. All Halo games before Infinite use coloured specular (pointdevice, 2021-05/06). H2A and H5 are true PBR; H4 is pseudo-PBR like Reach (adorabirb, 2021-04).
- Gamma: the "gamma 2.2 correction" from tacohombre (2024-01) concerned Halo 5 layered colours, not Reach.
- Foundry also imports H4 and H2AMP tags; JMS cannot be extracted from H4/H2AMP either (chiefster, 2024-11). General-101 toolset also handles CE (.gbxmodel direct; Tool has no model export before H2) and H2 (JMS/JMA/ASS; separate commands for render, physics, collision, structure).
- Architect (Reclaimer.Architect): scenario editor for H4/H2AMP, string/stringID editing; outdated per bojackdaggah (2025-01). Elite walk cycles also exist for H2AMP (iloveagoodcris, 2026-10). Only one H2AMP bsp uses position-W vertex data (gravemind2401).
- Reclaimer 2.0.1536 added an RMF colour channel from position W for H4/H5 models (Blender import script v1.0.1). A modified Reclaimer that keeps the lightmap UV channel in RMF exists for H2 (tim_irl, 2025-10). H2A Elite/Brute MP lack eyeball bones; Elites also lack face bones (denzeus 2023-03; Brutes lack eyeballs only). General-101 shaders exist only for CE/maybe H2 (chiefster, 2026-04). Reclaimer also adds decorator models for H4.
- Steam IDs: H1 1532190, H2 1613450, H4 1695792.
- Authors: fiycsf also wrote the H5/H4 Unity shaders; chunch made HIMS (Infinite). r93_sniper hedged the H2A and Wars sections of the control-map guide. Links: H4 https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity ; H5 https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity.
- Contradiction (CE): multipurpose/CC channel order is format-dependent (Xbox vs PC vs Gearbox), see E1.

### E7. H2 MCC lightmapper: scenery emissive fix (tool_fast.exe, H2)
- Symptom: emissive scenery light bakes ~2 world units too low. Cause: the surface-light collector sub_499603 (build_lights) runs BEFORE process_scenery_object decompresses the [-1,1]-normalized vbuf; fix = decompress the vbuf at collector entry (world_local = h*v + c from the compression bbox at section+0x48), shared done-set with the occluder decompress, gated on caller return address 0x4995EC (scenery only; poop verts are also normalized and must not be touched). Key addresses: 0x499603, 0x49946B, 0x4AE696, 0x498962, 0x49DA6A, 0x5AFB7F/0x5AFC38, 0x4A2D56, 0x4A1F84, 0x4A1278. 196-byte working vertex: pos +0, UV +68/+72, normal +76. Method (order probe, caller-tagged log, world-origin log) is reusable for other lightmapper bugs. Lesson: compressed verts must be decompressed before any consumer (H3 too, as a principle).

### E8. Halo CE shader note
- CE has one change-colour channel per shader_model; fake more with a forced shader permutation (not relevant to Reach).

### E9. Corrections that concerned other games
- Dropped/relabelled: gameoverload "alpha = spec means RGB is control map" (non-Halo thread); a Halo-4-labelled Vec4 packing list (game unnamed); "R emissive, G spec power, B spec intensity" masks (not Halo); Arbiter uncolourable claim (was H2 and H5 Arbiters, not H3); IOR/f0 remarks (Infinite only).
- Misattributed: "Reach got baking outputs from soulburnin" (Infinite); fresnel-into-ramp visors (Infinite); skinned/rigid shader groups (Infinite); gamma 2.2 (Halo 5).
- Downgraded: "Blender 5 fcurves"; HIMS 2.9 fixing Blender 4; Blender FBX reverse-engineered / metres; H2A Elite scaling fix (one answer); H2A Brutes lack eyeballs only.
- Scope fixes: the H4 two-tone CC alpha is H4 only; the normal-blue rebuild is not Infinite-specific; Reclaimer 1.6.274 (not 1.6.249) fixed the H2 bump palette.
