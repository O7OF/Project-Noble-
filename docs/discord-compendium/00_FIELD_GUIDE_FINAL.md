# Halo Reach / Halo 3 Modding Field Guide (Elite armour focus) - FINAL SYNTHESIS

Source: ~12.5k messages of a Halo modding Discord (2021-03 to 2026-10), two extraction passes, then this synthesis. Pass-2 corrections override pass 1. Spot-checks against the raw chunk files are marked "raw-verified".

## 0. How to read this

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

## 1. Texture / control-map channel layouts per game

Authority: r93_sniper's #control-map-guide (2021-12), re-quoted by gameoverload (2022-11, 2022-12, 2023-07). "Control map" is the community word for the packed mask map (fiycsf, 2023-02).

### 1.1 Reach / H3 / ODST (all follow H2's set; details differ per shader)

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

### 1.2 Reach CC map: how many channels?  [D]
- Guide (r93_sniper, 2021-12): R/G only, B unused [C as the guide's statement].
- unordinal + soulburnin (2021-11/12): the biped `Change Colors` block has up to 4 elements that map to R/G/B/A of the CC map (section 3). Reach visors have their own small CC map whose alpha is not pure white. Elite SpecOps has no blue in its CC map yet a slot-2 colour.
- chiefster_4014 (2026-03): "H3 CC maps can use RGB but the sheet shows only RG; Reach CC maps can use RG but the sheet shows only R". Base H3 and Reach armour are RG; Raven sets are among the few using RGB.
- Practical reading [U]: MP armour = R/G; B and A are asset-specific (Elite, visors, Spec Ops). Open the actual `_cc` and look at each channel before assuming.

### 1.3 Multipurpose maps (CE, shown for comparison; format-dependent)
- Xbox / OG Xbox: R = specular/reflection mask, G = self-illum, B = primary CC, A = auxiliary. PC / Gearbox / MCC: R = auxiliary, G = self-illum, B = specular, A = primary CC (r93_sniper, 2021-06, 2021-10, 2021-12; raw-verified) [C]. If you rip from MCC you almost certainly get the PC layout. The format cannot be told from the file name, inspect channels. CE supports ONE CC colour.
- chiefster_4014 (2025-11, 2026-05) says CE meaning depends on shader tag settings and quotes Xbox as R detail mask, G self-illum, B reflection mask [D vs the guide's R=spec].

### 1.4 Other games (so you do not apply them to Reach)
- **Halo 2**: two changeable colours, spec in diffuse alpha, CC R/G (r93_sniper, 2021-12) [C].
- **Halo 4** control maps: Type 1 (weapons/props, suffix "multi") R = spec coefficient, G = spec exponent, B = reflection mask. Type 2 (MP armour, no separate diffuse) R = spec coefficient/reflection, G = DIFFUSE, B = spec exponent, A = colour change; Spartans' diffuse sits in the green channel (chiefster_4014, 2025-01). Alpha: white = primary, grey = secondary, black = not coloured (soulburnin/r93_sniper) vs "black secondary, white primary" (Waffles, relayed by surasia, 2021-07) [D]. This two-tone alpha is H4 ONLY. Do not apply it to Reach.
- **H2A MP** (PBR): control R = metalness, G = AO, B = self-illum, A = smoothness; CC R/G (r93_sniper, 2022-01) [S]. H2A SP Elite CC mask is in diffuse alpha (chiefster, 2026-09).
- **Halo 5**: MP Spartan control R = zone 1, G = zone 2, B = metallic, A = smoothness (fiycsf, confirmed by the Open Halo 5 shader author). The community believed blue = Spartan colour for a year (fiycsf, 2021-04): verify channels with a working shader.
- **Infinite**: ASG R = AO, G = scratch, B = grime; Mask0/Mask1 hold zones; speakers disagree on zone numbering [D]. Not Reach.

Colour space rule for ALL of the above [C] (err_0r 2021-03, sswampp 2021-11, deskclaw 2022-05, many): base/diffuse = sRGB; CC, control, normal, roughness, blend = Non-Color. Tint values stored in tags are linear (unordinal, 2021-11); convert for sRGB use.

---

## 2. Reach shading model

### 2.1 What it is
- Reach (and H3/ODST) use a Cook-Torrance-based specular model with a scalar roughness, coloured specular, a cubemap, and (Reach) area specular. Not metallic/roughness PBR (soulburnin, 2022-11; pointdevice, 2021-05; gallium, 2021-07) [C]. The exact normal distribution (Beckmann or other) was asked and never settled (nihalrahman/soulburnin, 2022-11) [D]. The tags describe the direct highlight as using a Beckmann NDF (the_heavynator, pointdevice, chromacore, 2021-05) [S].
- Whether this "is PBR" is a semantic argument: surasia "isn't PBR"; pointdevice "is Cook-Torrance"; the_heavynator "microfacet but not the PBR standard" (2021-05) [D]. Practical agreement: RGB diffuse + alpha spec mask + scalar roughness. Reach/H4 are "pseudo-PBR", H2A/H5 true PBR (adorabirb, 2021-04).
- All Halo games before Infinite use coloured specular (pointdevice, 2021-05/06). Rule of thumb for converting PBR sets: if spec RGB channels are close, dielectric (base = diffuse); else metallic (base = spec) [S].
- Real shader source: CE, H2, H3, ODST, Reach, H4, H2AMP shader code ships with the official MCC mod tools (chiefster_4014, 2025-12; gameoverload 2024-01/06) [C]. Procedure for Reach: open the material tag in Foundation, note the shader it references, open that shader with **Expert Mode enabled** to see the source (gameoverload, 2024-10) [S]. Assembly's shader dumps are raw binary, not readable HLSL (pointdevice, 2021-07). Guerilla shows parameter pages, not code, for H3 (gregory0701, 2022-08) [D vs fable_law_458].

### 2.2 Parameter meanings (tested, Reach `shader` / material tag)
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

### 2.3 Detail maps and constants
- **4.59479**: "the detail map's colour is multiplied by 4.59479 before being multiplied into the base map's colour" (chiefster_4014, 2025-04; raw-verified, chunk_04). Context: gregory0701's Unity recreation of the Reach **Covenant** shader came out much darker than the Blender one when the detail map was multiplied by albedo. [S] but the only concrete "too dark" explanation in the corpus. It was said for the Covenant (Elite-relevant) shader, not stated as universal to all Reach shaders [U on scope].
- gregory0701 (2025-04) saw gamma 2.0 in the game shader settings and wondered if the usual 2.2 assumption matters; chiefster did not rule on it. The "gamma 2.2 correction" from tacohombre (2024-01) concerned Halo 5 layered colours, NOT Reach (pass-2 correction) [D].
- Reach "height/detail" maps (`hdetm`) become normal maps in the shader path; r93_sniper (2022-11) said H2 and H3. Reach was NOT named [D/U].

### 2.4 Roughness texture recipe (per-pixel roughness)  [D]
the_heavynator (2021-08-29; raw-verified):
1. `analytical_roughness` = 1.
2. `material_texture_black_roughness` = 0.01 (NOT 0; zero gives NaN artefacts, i.e. black/dark pixels).
3. Enable `use_material_texture` (a float-constraint toggle, "usually constraint 0").
4. Put a texture with the roughness in its ALPHA into the `material_texture` input.
5. The shader must be a transparent-capable one exposing all material inputs (e.g. `cargo_truck_decals`).
Dispute: pointdevice (2021-07, raw-verified) says this is a leftover from the H3 beta and not used in retail Reach shaders; chromacore and pointdevice (2021-05) said Reach roughness is a flat scalar. The RGB meaning of the material_texture was never posted (the_heavynator said he found descriptions in H3 shader code, 2021-10). ODST body/fp hands reportedly use it. Treat as an experiment you must verify in game, not a retail pipeline [D].

### 2.5 Spec / fresnel / env / albedo-blend behaviour in one chain
Best-supported Elite/Skirmisher chain (soulburnin, 2021-11; raw-verified) [S]:
`diffuse (blended with detail) -> coloured by CC map -> mixed with specular tint -> final specular colour`
- Mix: 0 = specular tint only, 1 = full albedo-tinted spec.
- Fresnel curve value 0 = fresnel colour completely overrides the specular tint.
- Globals tag values for the Elite: specular tint `#D6D4DB`, fresnel colour `#E8C6A4` (raw-verified).
- Direction of the "subtract albedo" in the HLSL is unsettled: unordinal's own wording was ambiguous ("misread that it subtracted albedo from spec tint for the mix") [D].

### 2.6 Blender preview recipes (not engine truth)
- Avoid plain Principled for Reach. Use ChromaCore's or soulburnin's Reach shader, or two glossy nodes sharing spec colour/roughness plus diffuse with env reflection off (jadedqrow, 2022-05; the_heavynator, 2022-06). Coloured-spec recipe: Principled #1 = albedo only, everything else 0; Principled #2 = spec map to colour, specular 1, metallic 1, inverted gloss to roughness, normal on #2 only; Add Shader.
- No 1:1 Reach shader exists in Blender; the editor lacks light/render-pass data (gameoverload, 2026-09). chiefster's own attempt "messed up" (2026-09). jackofchaos ported H3/Reach shaders to his own renderer; the spec path = analytical specular + cubemap per material model (2024-04) [S]. Expect some shine mismatch.
- Cubemaps need cube-to-equirect: py360convert, Blender Cubemap Converter add-on, or Flexify 2 + crop (soulburnin, 2022-10; brooen 2023-08) [C]. Cubemap colour space and the cubemap alpha's purpose remain unanswered (gameoverload, 2023-09; theishikawarin, 2025-02) [open].

---

## 3. Change Colors and colour-change (CC) maps

### 3.1 Established
Reach object/biped tag `Change Colors` block (unordinal with soulburnin, 2021-11-30; raw-verified) [S but tested on a grunt; widely cited afterwards]:
- Up to 4 elements (0-3, asset dependent). Each holds a list of variants; each variant has a name and two colours.
- Element 0 = primary (CC R), 1 = secondary (G), 2 = tertiary (B), 3 = quaternary (A).
- The diffuse is mixed with these colours by the CC map.
- For assets with no explicit `specular_tint` in their shader, element 2 additionally tints the FRESNEL and element 3 tints the SPECULAR. Those tints do not use the CC map. When element 3 is both spec tint and quaternary colour it is masked by the CC alpha (soulburnin, 2021-12).
- A per-material spec tint is used when there is no CC map; with a CC map the biped's colour block seems to override it (unordinal, chromacore dispute, 2021-11) [D].
- Secondary does NOT tint the environment map (an early observation was a false alarm caused by a bluish room).
- Reach visors have their own tiny CC map; CC alpha masks the quaternary colour applied to visor diffuse (soulburnin/unordinal, 2021-12) [S; unordinal doubts it masks the specular itself].

### 3.2 NOT established (pass 1 overstated)
- The claim that the "tertiary-to-fresnel" link was "resolved" is wrong. unordinal said it "mostly makes sense" and planned to verify in game. Unverified.
- Elite SpecOps: green fresnel equals the colour in slot 2 although its CC has no blue. Ranger Elite: orange fresnel equals the tertiary slot; its orange self-illum comes from the `illum_cards` shader itself, not the colour block. soulburnin first guessed emissive, then a greenish cubemap tint on SpecOps pieces. Nobody verified in game [D].
- What decides colour-vs-tint (a flag?) was not found. The docs' shader-input list does not cover per-biped overrides (unordinal, 2021-11).
- unordinal's opening belief (four blocks = albedo/specular/fresnel/env tint) was replaced by the channel mapping above.
- No canonical table of Reach armour hex values exists in the corpus (aegis_wolf 2025-05; thoro.blend 2024-08 asked, nobody answered). Pointers: `#lavadeeto-resources`, Halo Archive video youtu.be/vVS9HJd73io, Cylix guide (Infinite). Reading exact campaign colours: biped tag in Assembly or ODST mod tools (brooen/chromacore/chiefster, 2022-2024). Reverse engineering Reach CC colour from the executable via IDA was attempted (walkmen, 2024-08), no result.

### 3.3 Colour space and alpha handling in Blender  [C unless noted]
- CC image colour space = **Non-Color**.
- Image **Alpha mode = Channel Packed** whenever alpha carries data (spec in diffuse alpha, CC alpha). With Straight, colour is multiplied by alpha and the shader can go black (soulburnin, chiefster, chromacore, 2021-08 to 2026-06). Concrete case: a Reach Sniper SHOULDER CC map looked fully black; Channel Packed fixed it (martyrthesaint/chiefster/theishikawarin, 2021-07).
- Some Reach `_cc` files store alpha = 0 and look transparent (e.g. `spartan_chest_joshua_cc`). This is normal for that file; set Non-Color or force alpha white in an editor (gwenwyn/chiefster_4014, 2024-10). caldoric (2023-10) saw alpha=1 as wrong and said the file turned into a grey checkerboard after editing: the slot read as empty [D].
- Editing hazards: GIMP Colors > Components > Decompose/Recompose kills alpha (chiefster, 2023-09); GIMP erase fills with the background colour (hydranaught444, 2023-01). Either erases a spec/CC alpha.

### 3.4 Making your own CC
- Base albedo for a CC item should be white/neutral (gameoverload, 2022-11; plastered_crab, 2023-08) [C]. Pack masks into R/G, keep wear/metal out of the tinted region (fiycsf, 2021-04).
- Manual node route: Mix (Multiply) with factor = primary mask, A = base, B = primary colour; second Mix with factor = secondary mask chained on the first (gameoverload, sswampp).
- The control map must match what is baked into the albedo; a mismatched CC "looks like hell" (gameoverload, 2022-11).

---

## 4. Elite / Sangheili specifics

- **Vertex colour**: the Reach Elite body is said to carry vertex colour used as a shader mask (the_heavynator, 2021-05; example "armorless" Elite) [S]. gravemind2401 (2021-05, 2024-11) recalls no render model with it and says position-W is not vertex colour and only one H2AMP bsp uses W. brooen (2022-12) says Longbow rocks should have it but export lacks it [D]. Reclaimer's AMF has no vertex colour; DAE and RMF can carry it. A piece going dark after a port could be a shader multiplying by an absent vertex mask [U].
- Reach Elite emissives vanish in Render but show in the tweak pane (x_crimsongalax, 2026-01), unresolved. Glowing bits guessed to be `base_illum_cards` (x_crimsongalax, 2025-07).
- CE Elite (not Reach): one CC colour, so CC tints the whole area; the real colour comes from the cubemap (r93_sniper, 2023-01). dustyoo10 (2025-02) found CE Elite looks accurate with base multiplied by the cubemap.

---

## 5. Diagnosing a black / dark / over-shiny armour piece (ranked checklist)

Nothing in the corpus pins the Honor Guard shoulder. This ranking merges tag-side facts with Blender-side facts, ordered by likelihood given what was documented. Each item says how to test. Items tagged [U] are inference.

### 5A. Black or too dark
1. **CC image read wrongly (most documented)**. Test: set CC node to Non-Color AND Alpha = Channel Packed; compare with a working piece. Documented on a Reach Sniper shoulder (martyrthesaint, 2021-07) and Straight alpha blackening (chiefster, 2026-06). Also check the CC file really loaded (caldoric 2023-10) and is not a checkerboard/blank (Sniper shoulder, Season-7 Elite maps blank, 2021).
2. **Colour-space wrong on other maps**: diffuse sRGB, everything else Non-Color; normal Non-Color (gives dark/odd shine; DMR seam case, chiefster, 2024-04). Texture saved as HDR with sRGB cured one fault (chocolateray, 2025-04).
3. **Tint input unset or black**: in Surasia's Reach node group, unset tint colours leave masked areas black [U]; the group already contains the BSDF, so a leftover Principled in the path makes tints do nothing (surasia/err_0r, 2021-03). In the engine, a black variant colour in `Change Colors` blackens that channel's zone; slots 2/3 also tint fresnel/spec (section 3). Test: diff the `Change Colors` block against a working Elite piece.
4. **Missing detail multiplier (4.59479)** on the Covenant shader: recreations came out much darker without it (section 2.3). Test: multiply detail colour by 4.59479, compare. [S, strongest numeric lead]
5. **Shader floats that darken**: `diffuse_coefficient` 0, `specular_coefficient` 0, `albedo_color` black (clamped), `material_texture_black_roughness` = 0 (NaN pixels), missing bitmap falling back to `grey_50_percent`, wrong detail multiplier. Test: compare against a working piece with Reclaimer's `Export Metadata` JSON, Baboon, Assembly, or Foundation. [mostly U as a list; each parameter is documented, the combination is inference]
6. **Piece has its own material slots / shader**. Edit every slot, Alt+H to unhide geometry; hidden second shader stacks (kneepads, koax117 2022-06); a global colour node can starve a piece, unlink it (koax117, 2026-09). Copy the node tree from a working piece and relink masks (exporush 2023-02; poet 2023-10; chiefster 2023-10).
7. **Zone switch / zone mix-up**: an off zone switch falls back to zone 1 colour (gameoverload 2023-06); three-input Infinite shader needs three separate nodes (not Reach). Wrong colour slot plus sRGB gave white blowout on a Reach shoulder (etheox, 2022-07/08).
8. **Reach armour darkened when diffuse replaced by the primary colour** in the damage node tree: re-link the original diffuse node (x_crimsongalax, 2025-07) [S].
9. **Metal-like behaviour with no environment**: in metallic engines high metallic blackens diffuse and needs env reflection (gameoverload, 2023-04; r93_sniper, 2022-08). Reach is not metallic, but a high `specular_coefficient` with no cubemap or a black World reads as dark mirror [U].
10. **Normals / geometry**: lost or sharp custom split normals after merging; use "Clear Custom Split Normals" (surasia, 2021-07; fiycsf, 2022-08/10). Overlapping/duplicate meshes (chiefster, 2025-09). Mirrored UVs (chiefster, 2026-08).
11. **Absent vertex-colour mask** on Elite mesh [U] (section 4).
12. **Render setup**: Cycles glossy/transmission/volume bounces off made Reach Warthog metal black (trisckit, 2024-05); Eevee vs Cycles; World lighting, exposure, view transform (koax117, 2026-05/08). Black visor often just means nothing to reflect (incuriouslog/unordinal, 2022-06).
13. **Intentional black**: matte-black zones outside R/G (fiycsf, 2021-05).
14. **Pink/purple is not black**: missing images; File > External Data > Find Missing Files (many). Black can persist after that fix (vexerart/_pandoras, 2026-08).
15. **Foundry bitmap format**: possibly sets Reach bitmaps to sRGB instead of xrgb, which would mis-read linear maps (brooen, 2025-10; Crisp suggests colour profile on the image node instead) [D/S].

### 5B. Over-shiny or wrong shine
1. Check normals: Non-Color, smooth shading, normal wired (soulburnin, 2021-08; gameoverload, 2022-06). Do not wire normal maps into detail strength sliders (chiefster, 2022-01).
2. Spec in diffuse alpha lost (alpha stripped/premultiplied), or alpha mode wrong.
3. Brightness of the primary colour matters in the Blender Reach pack: "brighten your primary, darker values won't shine" (chromacore, 2021-06).
4. Reproduce the chain in 2.5 with `D6D4DB` / `E8C6A4` if the Elite looks too white or wrong. A whitish rim glow = glancing colour/exponent of the cook-torrance group (soulburnin, 2024-08; lever is inference [U]).
5. Terrain-style stacked blends or bump mixes give "insane shine" in Chief's shader (bombwolf, 2022-01).
9. Blender 4.x: older packs render silver/white; replace the Principled BSDF inside the node group, IOR 1.45 (purplmunkii 2024-07; soulburnin 2024-09). Specular split in 4.0 (gameoverload 2023-12). Pack fix is version-specific; "HIMS 2.9 fixes it" is unsupported (pass-2 correction).

---

## 6. Import / export pipeline by game: commands, tools, versions, errors

### 6.1 Which tool for which game  [C unless noted]
| Game | Rip from .map | Edit tags / EK | Blender import |
|---|---|---|---|
| CE | Reclaimer (rig scale 32.8084 needed, chiefster/exodst 2025-11) | CE mod tools | General-101 toolset (.gbxmodel direct; Tool has no model export before H2) |
| H2 | Reclaimer | Tool extract commands | General-101 toolset (JMS/JMA/ASS). H2 has separate commands for render, physics, collision, structure |
| H3 / ODST | Reclaimer | Tool, Guerilla, Sapien, Foundation | `tool extract-import-info` then toolset (.ass) |
| Reach | Reclaimer (RMF) | Reach Editing Kit (Steam), Foundation, Assembly, TagTool, Baboon | **Foundry** (Crisp). JMS cannot be extracted from Reach/H4/H2AMP, pipeline changed (chiefster, 2024-11) |
| H4 | Reclaimer | H4 EK | Foundry |
| H2A MP | Reclaimer (BLAM) | H2AMP EK | Foundry. H2A SP: Index |
| H5 / Infinite | Reclaimer H5 branch; Ekur / HIMS for Infinite | no kit | Brooen H5 importers; not Reach relevant |

Steam IDs: H1 1532190, H2 1613450, H3 1695791, ODST 1695794, **Reach 1695793**, H4 1695792 (brooen/sswampp, 2023).

### 6.2 Reach-specific facts
- The Reach editing kit contains all retail tags (chiefster, 2025-03). Reach models cannot be pulled out through mod tools: render data not retained; Foundation "export mesh" gives unusable `.x` files; use Reclaimer (chiefster, 2022-10/2023-07/2025-03).
- The reverse route (custom model into Reach) has NO documented procedure in the corpus. Asked repeatedly: Blender-to-Reach tag tutorial (the1updude 2025-06), sbsp import (makeshift.h2 2025-12), CE model into Reach (khalghoush 2024-07), older-game models into Reach (_manchurian_ 2026-01). All unanswered. "No Reach import tutorial seen" (m0nkeyman117, 2023-08).
- Indirect pointers: H3 tutorials exist and Reach is "built off 3" (gameoverload 2023-05); Foundry with the Reach Editing Kit set as project; drag-and-drop `.scenery`; `Setup as Asset` option for re-import into tools (chiefster/iloveagoodcris, 2025-10/11); animation files must be saved inside the project data directory ("Ensure your file is saved to your Omaha data directory" otherwise; iloveagoodcris/tristangold, 2025-10/12).
- Export from the RENDER object, not a sharp-edged body copy (chiefster, 2023-09, Mauler example) [S]. Use the original source (.ass/.jms) from the EK data folder rather than a Reclaimer-extracted mesh; Reclaimer geometry differs from the original import (chiefster 2026-01; rdt4 2026-01 saw failures going H2A/H3 into Reach) [S].
- Port recipe (chiefster, 2024-02, 2026-08) [S]: export the target game's own render_model, reuse its skeleton and markers; bone names must match exactly; scale must match; rename vertex groups to target bones and merge weights. This is also the fix for "matching node can't be found" in first person and "no animation graph" on a ported biped.
- Textures into Reach via Assembly: DDS to `.tagc` with Forerunner's tool, import under a new name (surasia, 2021-07); importing two different bitmaps in a row duplicated the first (the_heavynator, 2021-08). Keep CC alpha intact [U].
- Cross-game tag copy trick: copy H3/ODST tags into the Reach/HREK tags folder so Foundry can import them; works for Chief, textures can have issues (koax117, iloveagoodcris, 2025-10 to 2026-06) [S]. Foundry does not support H3/ODST natively: ManagedBlam for H3 cannot be loaded in Python.
- Material/shader rename on a render_model: enable expert mode in Guerilla/Foundation and edit shader names on the render model directly (zadeinator/lt_crunch236, 2024-06; expert mode confirmed). Only render_model-level fix in the corpus; applicable to variant shader swaps.
- Compile gotcha (H3 tool): opening the scenario in Sapien compiles shaders; rebuild afterwards (r93_sniper, 2021-10). Do not hand-edit .shader text (corrupted in Guerilla, itzhouze 2021-11).

### 6.3 Commands seen
- `tool extract-import-info <path>` (H3/ODST; writes .ass, extracts, does not import). Path must be editing-kit relative starting with `tags`, or a full system path, NOT starting with `H3EK`; if `H3EK.bat` nests a same-named folder, move the inner folders to the kit root (iloveagoodcris/magmashark009, 2026-02; raw-verified).
- `H3EK.bat` extracts tags; open with `guerilla.exe` to read shader parameters (chiefster, 2025-11).
- Animation extraction (H3 syntax): `<full path>\map.map EditTag objects\vehicles\pelican\pelican.jmad ExtractAnimation all "<outdir>"` (r93_sniper, 2022-09/12). The raw line does not name the tool (Assembly or TagTool unattributed) [D/U].
- Lightmapping: Reclaimer site tells you to run `python calc_lm_farm_local.py` from the H3EK folder; "python isn't found" means Python must be installed and on PATH (ohwhen6442, 2025-06; no confirmed answer) [U].

### 6.4 Tools and versions
- **Reclaimer** (Gravemind2401): File > Open Map. `campaign.map`, `multiplayer.map`, `shared.map` cannot be targets, but `mainmenu.map`, `shared.map`, `campaign.map` must sit in the same folder as the target (chiefster, 2024-05/11, 2025-10) [C]. Set Materials root path to the bitmap export root to fix yellow textures (gallium 2021-05; soundflare 2022-08). 2.0 (2024-08/11): .NET Desktop Runtime 7, RMF format with Blender/3ds Max importers, `Export Metadata` (JSON dump of a tag, useful to diff a working and a broken shader/biped), decorator models for Reach/H4. 2.0.1536: RMF colour channel from position W (H4/H5; Blender import script v1.0.1). 2.1.1608 (2025-08): Reach workshop map fixes. **2.1.1624 (2026-04): extra H3 and Reach Alpha builds (bitmaps, render_models only)** (exodst., raw-verified). AMF is outdated on current Blender; use RMF (chiefster, 2025-07). AMF has no vertex colour or multi-UV; DAE does. Reclaimer cannot export shader code or the lightmap UV set (un_nonimus, 2025-12).

- Known Reclaimer errors: "valid .map but may not be a supported version" = old build; `FileNotFoundException ... campaign.map`/`shared.map` = companion maps missing from the folder; "Could not find ... mainmenu.map" on render_models (2022-05); `InvalidDataException: Found invalid data while decoding` on a Reach render_model with maps inside Reclaimer's Data folder (waketheshark, 2022-11, unresolved); `ArgumentOutOfRange` in `MccHaloReach.IndexItem.ReadMetadata` on Forge workshop maps (frankr2994, 2025-02, unresolved); `NullReferenceException` in `scenario_structure_bsp.ReadGeometry` after an MCC update (360 maps work better). Reach is supported (the old "needs Adjutant 4.2.0" claim is stale).
- **Foundry**: Blender add-on for Reach/H4/H2AMP tags (render/collision/physics import); builds shaders automatically; levels ~90% accurate, decals manual (koax117, 2026-09). Install errors: missing numpy; "Import completed in 33 milliseconds" with empty scene (2025-03, unresolved).
- **General-101 toolset**: CE/H2/H3/ODST JMS/JMA/ASS; cannot read .map; some versions need an unzipped `io_scene_halo` folder; "action object has no attribute 'fcurves'" on newer Blender (tristangold 2025-12). camden-smallwood fork has Reach+ work. Shaders only for CE/maybe H2 (chiefster, 2026-04).
- **Bonobo** (LaikaGlove, unmaintained): extracts jmad to JMA/JMM for CE-Reach Bungie-era maps; needs shared, campaign, mainmenu and a level map in one folder plus .NET. Errors: "unsupported game build" on current MCC Reach maps (2021-12, 2022-04); Reach fp_magnum "Value cannot be null (source)"; first person does not extract. Alternatives: **Baboon** (Zoephie; blam-tags GUI, byte-exact tag edit/extraction for H3/ODST/Reach/H4/H2A), HaloAnimationRepository (77Mynameislol77).
- JMA import: "no valid armature" / "Node 'camera_control' not found" = import the armature first (guhgunk, 2022-08). The "comma decimal locale" fix for "could not convert string to float" is inference [U].
- **CR4B Tool** (PlasteredCrab): Blender material builder for H3/ODST; Reach "minimal"/not working (2023-08 to 2024-02). Needs mod tools + tags.zip; best on Blender 3.6. Red nodes = missing `detail_map`. Writes tags only to the "halo 3" folder. Error `key 'shader' not found` on H3 MP Spartans (2024-01).
- **Architect** (Reclaimer.Architect): scenario editor (H4/H2AMP, string/stringID editing); outdated per bojackdaggah (2025-01). **Assembly** edits .map data, no model extraction. **TagTool** (dev branch) modifies tags in built caches.
- **Assembly** (Lord-Zedd): edits .map data, does not extract models. **TagTool** (TheGuardians dev branch): modify tags in built caches.
- Baking from node-group shaders (for engines outside Blender): image node (blank, selected) per material; plug the shader's own output into Material Output; bake type Emit; Cycles; 1 sample; Non-Color for data. Normal: bake type Normal from the BSDF on a duplicate with split normals cleared [C for Infinite; for Reach the shader has no bake outputs, so bake diffuse, spec colour, and a flat gloss using node-group outputs (chiefster, 2026-08)] [D: gameoverload says only normals bake from the shader output].

---

## 7. Rigging, skeleton, markers, animation

- **Scale**: Reclaimer-ripped CE rigs need 32.8084; H2 JMA imports can arrive ~33.33x too large (chiefster/exodst, 2025-11). Reach Spartans use a human skeleton at ~1.2x (frostbitecinem, 2024-03, unanswered exact value). Reach ripped bones can look tiny; check pack version (degweb, 2022-09). H2A Elite with H2 JMAs stretches because skeleton scale differs; scale H2A Elite to match H2 classic Elite (chiefster, 2024-08) [S, downgraded from confirmed].
- **Bone names**: must match the target skeleton exactly; vertex group names must equal bone names, the armature modifier must target the rig, weights non-zero (purplmunkii, 2024-11). H2/H3 rigs are near identical ("iirc", chiefster 2024-07 [S]) so weights port by renaming. Fix wrong weights by removing the verts from unwanted groups.
- **Markers**: included when exporting parts of a model from Reclaimer 1.7.307+. For tool import use the target game's own skeleton and markers from `extract-import-info` (chiefster, 2024-02). No documented list of Reach armour marker names, regions or permutations exists in the corpus.
- **Elite rig**: Reach Elite IK fails mainly because of horse-link (digitigrade) legs; Track To constraint feet vs master rotation; Elite hunch/kangaroo legs make human run cycles look wrong (xxpythonxx 2022-05; r93_sniper 2022-11). Elite leg bones follow the mesh (xxpythonxx, 2022-05; pass 1 mis-credited surdavow). Bone X axis must point along the bone for IK when porting Reach Elites to H3 (winnower, 2026-07). H3 Elite has no chest bone and no public rig; its head is a separate overlapping mesh that breaks auto-rig tools (kozmetic/monokkii, 2025). Auto-Rig Pro and ActorCore AutoRig rigged Elites with little effort (frankr2994, 2025). Mixamo recognises Reach Spartan and Elite rigs, Elite "wonky" (koax117, 2022-06). H2A Elite/Brute MP lack eyeball bones; Elites also lack face bones (denzeus 2023-03; pass-2 correction: Brutes lack eyeballs only).
- **Animation**: Bonobo/Baboon/Tool route (section 6). Reach fp rig is the CE-Reach fp rig with different bone names (adorabirb, 2026-08). Elite walk cycles exist for H3 and H2AMP (iloveagoodcris, 2026-10). Reach animation tag binary layout and compression asked, unanswered (superintendent, 2026-01). Armor-lock fall animation not found. Retargeting Reach to Infinite rigs with Mixamo/Rokoko/Mwni failed or exploded (.username_ladi, 2026-01); marine to other rig works when bones are lined up (koax117, 2026-08). Applying one character's animation to a second biped imports onto the first (rylietw, 2021-08, unresolved). Imported animations load into an animation layer.
- **Cinematic exporter**: iloveagoodcris is writing a Blender exporter for Reach and H4 cinematics; Foundry can import a cinematic (e.g. last m50 cinematic) from extracted Reach tags (2025-03/12).

---

## 8. Lightmaps, bsp, scenery, Sapien, scripting, AI

Little exists. What was found:
- **Lightmaps**: H3 tool lightmap command asked (raccnoon 2021-10), unanswered. `calc_lm_farm_local.py` (section 6.3). Reclaimer does not write the secondary UV set so Reach/H3 lightmap bitmaps are unusable externally (un_nonimus, 2025-12). A modified Reclaimer that keeps the lightmap UV channel in RMF exists (H2) (tim_irl, 2025-10). Reach lighting info can be imported to Blender by Brooen's Halo-Reach-Light-Importer (`.scenario_structure_lighting_info`).
- **BSP**: Reach geometry is built differently from the trilogy and gives "degenerate edges" in H3 Tool (reply from a Deleted User, 2021-11; the earlier "scaling up" cause was only the asker's guess). Typical H3 import failures: missing shaders, degenerate/duplicate triangles, bad collision edges. Reach map collision as sbsp: unanswered (sosa758 2025-03; makeshift.h2 2025-12). H3 sky is a `.render_model` tag in the scenario's sky folder (chiefster/jackofchaos); Halo skies are meshes, not HDRIs.
- **Campaign to MP conversion (H3)**: set map id and game type to multiplayer, one zone set, remove globals/scripts, `tool build-cache-file`; log still loaded global_scripts and the game gave "Fatal Error"; Sapien error "Failed to find a valid spawn location" until a respawn_point, respawn zone and slayer spawn were added. Final crash unresolved (itzhouze, 2021-10).
- **Sapien**: crash assertion `render_methods/render_method_definitions.cpp #3203, "result != NONE"` loading riverworld after the April 2022 MCC patch, no fix (tafurious). Sapien compiles shaders when opened. H3 device_machine elevator: animation plays on render model but physics model stays at rest in Sapien (bongcloset 2024-10, unresolved).

---

## 9. Networking, multiplayer, anti-cheat: what actually exists

Honest answer: **almost nothing.** No speaker explains how custom armour tags replicate to other players, whether stock clients accept modified tags, or what EAC does with mod tool sessions.

What exists:
- NinjaRipper on MCC: EAC raises an error but the planted DLL works; launch MCC with anti-cheat disabled afterwards (surasia/Jackal, 2021-07; confirmed process).
- Inference only [U]: armour colour comes from the player's biped `Change Colors` and CC maps, so a client without your modified tags would see defaults; custom content likely reaches others only in mod-enabled, anti-cheat-off sessions. No speaker said this.

---

## 10. Resources and authors to trust

### Authors
- **gravemind2401**: Reclaimer / Architect author. Authoritative on formats, vertex colour, versions.
- **iloveagoodcris / Crisp**: Foundry author. Authoritative on Reach/H4/H2AMP import.
- **chiefster_4014**: most frequent, knows Blender packs and mod tools; consistent but gives some single-source claims.
- **soulburnin**: Reach/H3 Blender shader author; fresnel/spec chain, globals values.
- **the_heavynator, pointdevice, chromacore**: shader reverse engineers (pointdevice uses frame capture; disagrees with heavynator on the roughness recipe).
- **r93_sniper**: #control-map-guide author. Carefully hedged by the author for H2A and Wars sections.
- **gameoverload**: shading theory, baking, node groups; occasionally speculative on engine internals.
- **unordinal**: Change Colors investigation. **plastered_crab**: CR4B. **zoephie**: Baboon. **fiycsf**: H5/H4/Reach Unity shaders. **general-101**: toolset. **chunch**: HIMS (Infinite).

### Links
- Reclaimer https://github.com/Gravemind2401/Reclaimer ; Architect .../Reclaimer.Architect ; releases pages for v2.0.1536, v2.1.1608, v2.1.1624.
- Foundry https://github.com/ILoveAGoodCrisp/Foundry
- General-101 toolset https://github.com/General-101/Halo-Asset-Blender-Development-Toolset ; fork camden-smallwood
- Assembly https://github.com/Lord-Zedd/Assembly ; TagTool https://github.com/TheGuardians/TagTool/tree/dev
- Baboon https://github.com/Zoephie/Baboon ; HaloAnimationRepository https://github.com/77Mynameislol77/HaloAnimationRepository
- CR4B https://github.com/PlasteredCrab/Halo-CR4B-Tool
- Reach Unity shader https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity (commented; could show how Reach CC/spec/masks combine [U]); H4 .../open-halo-4-spartan-shader-for-unity ; H5 .../open-halo-5-spartan-shader-for-unity
- Mjolnir-Forge-Editor https://github.com/Waffle1434/Mjolnir-Forge-Editor

---

## 11. Contradictions and unresolved questions

Contradictions
1. Multipurpose / CC channel order is format-dependent (Xbox vs PC vs Gearbox) - 1.3.
2. Reach CC: R/G only (guide) vs R/G/B/A elements (unordinal) vs "RG used, sheet shows R" (chiefster 2026) - 1.2.
3. Which tint carries Elite fresnel: slot 2 vs globals E8C6A4 vs emissive vs cubemap tint - 3.2.
4. Per-pixel roughness: retail feature (heavynator) vs H3-beta leftover (pointdevice) - 2.4.
5. Spec tint per material (chromacore) vs overridden by biped Change Colors (unordinal) - 3.1.
6. H3 coloured spec map exists (`_s`, soulburnin) vs H3 does not support it (heavynator).
7. Reach PBR or not; Beckmann or not (semantic and unresolved).
8. Baking: from BSDF fine (chromacore 2023-11) vs end-node outputs only (gameoverload 2022-12, 2025-02) vs Emit routing worked for Reach users (chiefster; Reach pack users 2025-11). Likely version dependent.
9. Normals: flip green in Blender (gameoverload) vs keep DirectX in a Halo pack's Normal Map node (chiefster). Depends on shader type.
10. Reach vertex colour: Elite body uses it (heavynator) vs gravemind recalls none.
11. Reclaimer supports Reach or needs Adjutant (stale claim) - resolved in favour of Reclaimer.
12. Gamma 2.0 (game param) vs 2.2 assumption.
13. Blender 4.0 fix: replace BSDF vs new pack version; both seen.
14. Guerilla shows shader code (fable_law_458) vs parameter pages only (gregory0701).

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

## 12. Corrections log (pass 1 errors, pass-2 version used)

- Tertiary-to-fresnel "resolved": no, still unverified (unordinal planned in-game test). Ranger `illum_cards` concerns self-illum colour, not fresnel. HLSL albedo-subtraction direction ambiguous.
- H3 "B = tertiary" for `mp_masterchief_cc`: sswampp gave only R/G; tertiary on H3 armours is a later unconfirmed remark (2022-06).
- Dropped: plastered_crab "Reach needs Adjutant 4.2.0" (contradicted); clock-rollback offline tip (not MCC/EAC); gameoverload "alpha = spec means RGB is control map" (non-Halo thread); Halo-4-labelled Vec4 packing list (game unnamed); "Reach banshee-style shine" and "R emissive, G spec power, B spec intensity" masks (not Reach/not Halo); Arbiter claim (was H2 and H5 Arbiters); Reach IOR 8/10 and emblem-mix chain (game unnamed); IOR/f0 remarks (Infinite only).
- Misattributed: "Reach got baking outputs from soulburnin" (Infinite); fresnel-into-ramp visors (Infinite); skinned/rigid shader groups (Infinite); height-to-normal "H3 and Reach" (r93_sniper said H2/H3); gamma 2.2 (tacohombre, Halo 5); fiycsf alpha-spec (Reclaimer-exported textures, not the control map); "Elite leg bones follow mesh" (xxpythonxx); structure-error causes (askers, not answerers); mainmenu.map consequence date.
- Downgraded to unverified: JMA locale fix; "Blender 5 fcurves"; HIMS 2.9 fixing Blender 4; Blender FBX reverse-engineered / units metres; "bake from BSDF confirmed"; H2A Elite scaling fix (one answer); rank-cubemap guess; Reclaimer vertex colour for Reach via position-W (effectively none); H2A Brutes lack eyeballs only.
- Scope fixes: H4 two-tone CC alpha is H4 only; normal blue rebuild is not Infinite-specific; Reclaimer 1.6.274 (not 1.6.249) fixed H2 bump palette.
