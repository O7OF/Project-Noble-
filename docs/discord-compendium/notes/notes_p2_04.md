# Notes p2_04 (second pass over chunk_04; additions/corrections only)
Tags: (author, YYYY-MM, channel). Short channel names: ripping, control-map-guide, substance-su, lavadeeto-re. Marked [inference] where it is my reading, not a stated fact.
Overall: the chunk contains NO discussion of tool.exe import commands for Reach, Reach networking/anti-cheat, lightmap baking, scripting or AI. It is ripping/Blender/Unity-side. Armour-shading leads below are indirect.

## Missed facts

### Reach Elite / Covenant-shader specifics
- Reach Elite main body is one of the few Reach meshes said to carry vertex colour used as a shader mask (alongside some Forge objects, Sanctuary fog planes/waterfalls/skybox, H3 open containers). Heavynator suggested the "armorless" Elite as the most visible test case; vertex colour does not tint the mesh, it only masks shader effects, and the darker insides of Forge pieces come from it. (the_heavynator, 2021-05, ripping) [claimed; image HR_Elite_armorless.png was the example]
  - Reclaimer's AMF export has no vertex colour; DAE and RMF can carry it; Reclaimer only picks it up if stored in position-W. Gravemind later stressed vertex colour is not the same as position-W and he knew one H2AMP bsp using W. (gravemind2401, 2022-12 and 2024-11, ripping)
  - [inference] a Reach Elite piece that renders unexpectedly dark/black after a port could be a shader multiplying by an absent vertex-colour mask (default 0), not a CC problem.
- Direct "too dark" evidence for the Reach covenant shader: a Unity recreation came out much darker than the Blender one when the detail map was multiplied by albedo. Chiefster answered: the detail colour is multiplied by 4.59479 before being multiplied into the base colour. The user also noted game shader gamma param is 2.0 (not 2.2) and the Blender version shows the wrong detail scale. (gregory0701 + chiefster_4014, 2025-04, ripping) [strongest lead for darkness]
- Same thread: a custom "normalize" node that ignores blue is just recomputing Z in shader; almost all games do this. (chiefster_4014, 2025-04, ripping)
- Covenant stuff is coloured in-shader rather than baked in albedo. (tpg, 2022-03, ripping)
- Halo Online Elite rips on the drive have corrupted alpha channels, so using them as spec masks gives wrong shine; the "loved" appearance came from screenshots with broken/disabled illum. (vuthakral, 2021-04, ripping) [relevant to wrong Elite specular if those textures were used]
- Halo Online Elite armours fit the H3 "NuElite" body in MCC. (garfunkle3479, 2021-07, ripping)
- Standard/Honour-Guard Elite arms have to be jury-rigged; H2A Elite first-person textures not in the resources. (garfunkle3479 + markmcfuzz, 2021-10, ripping)
- Reach shoulder pieces on the Reach armour core are mispositioned in-game by 343 (complaint, no fix). (girldiseased, 2021-12, ripping)
- CE Elite (not Reach) colour change tints the whole area because CE has one CC colour; real colour comes from cubemap. Animation loaded separately until skeleton attached. (e.nine/r93_sniper, 2023-01, ripping)
- Reach Season 8 armours: same textures/models as H3 in the MCC viewer, with some colour-correction difference per r93_sniper; one user found the extracted Season 8 colour-control maps blank (unanswered); do not extract Reach armour from the menu (UVs broken). (.chevyj/r93_sniper/mehrunesdaegan/the_heavynator, 2021-10, ripping)

### Reach colour-change / tint wiring (Blender)
- Surasia's Reach Blender node group inputs: Diffuse, Diffuse Alpha, Tint color 01 = primary, Tint color 02 = secondary, TintMask = CC map, Normal. The group already contains the BSDF, so it must feed Material Output directly; a leftover Principled BSDF in the path is why tints showed nothing. If the tint colours are left unset/black, masked regions render black [inference]. (surasia + err_0r, 2021-03, ripping)
- Wrong-area tinting ("colour changes but not in right places") was reported with that shader; likely wrong channel (R primary, G secondary). (username056690, 2021-03, ripping) [inference, no fix posted]
- Exact Reach CC colour values live in game code (the tint fed to a material); easier to match by eye. (err_0r, 2021-03, ripping) [claimed]
- Specular maths in Blender: specTerm from Glossy node times specColor times alpha mask; plugging a scalar into a colour socket is just colour.rrr. The 0.7019608 spec colour in that thread is an example constant. (pointdevice, 2021-03, ripping)
- Linear for mask/normal/packed maps, sRGB only for diffuse and coloured spec; Reach spec is a "hybrid". (err_0r, 2021-03, ripping)
- Another user's "parts that should be black are white" with all shader settings tried: unresolved, game not stated. (cone_ram, 2022-11, ripping)
- Unity import dumps textures with no colour-map hookup; answer was only "get a shader set up for it". (burgereich/plastered_crab, 2023-03 and 2023-01, ripping)
- To get Reach material values: Reach .map files + Reach mod tools from Steam (read values per material) + Reclaimer for geometry; CR4B reads values from raw binary and needs a "level report" for Reach objects. (plastered_crab, 2023-03 and 2023-11, ripping)

### Tools, versions, commands
- Reclaimer v1.5.244 notes: Collada material URIs fix, H5 batch extract, WMA sounds H2 Xbox, some H3 bsps, H1 bsp batch fix. v1.5.245: latest MCC support, H4 cubemaps/node maps, H2 cubemap unwrapping, H2 LODs. (surasia, 2021-03/04, ripping)
- Newest Reclaimer cannot open reflection cubemap bitmaps or .shader tags that older builds could (cameraguy, 2023-06). H3 .shader tag crashed meta editor (surasia, 2021-05). Reclaimer plus Adjutant gave grey textures on Reach maps (sabsnake, 2022-01). No sound extraction option (doomman, 2023-03).
- Bonobo .NET: user reported one Microsoft download id (48130) works and another (53344) does not. (hobbes240, 2023-05, ripping)
- Normal format notes: DXN only in H3 normals; BC5 fine until alpha and a colour channel get swizzled. (gravemind2401 2021-04; _gallium 2022-01, ripping)
- Foundry import/CR4B/General-101 info: see pass 1; add that `extract-import-info` is a Tool command on H3/ODST that extracts, not imports. (chiefster_4014, 2025-04)
- No Reach import-to-game procedure appears: Bungie's import pipeline kept no data allowing models to be exported back out of Reach mod tools. (chiefster_4014, 2022-10, ripping)

## Corrections to pass 1
- Section 2 gamma/4.59479: the 4.59479 factor is Chiefster's; the "game gamma setting 2.0" observation is gregory0701's. The "gamma 2.2 correction" by tacohombre (2024-01) was about Halo 5 carbine/layered colour values, not Reach.
- Section 6: H2A Elites lack eyeballs and face bones; Brutes lack eyeballs only (denzeus said "elites and brutes have no eyeballs, or face bones in the elite's case"). Pass 1 said both lack face bones.
- Section 2 vertex colour: pass 1 implies Reclaimer supports Reach vertex colour via position-W. Gravemind's 2024-11 statement is that W is not vertex colour and only one H2AMP bsp uses it; Reach support is effectively none.
- Section 2 normal blue channel: not Infinite-specific; Chiefster says nearly all games recompute blue.
- Section 5: HW2 pm channel in guide is R gloss, G AO, B CC; plastered_crab's gloss summary "AO/roughness" is only a loose label.
- Bonobo: Plastered_crab (2022-01) says Reach is almost fully supported except first-person; later 2023 reports of "unsupported" are MCC maps; pass 1 merged these.
- The H4 two-tone CC (white/grey/black) is Halo 4 only; Reach CC maps are plain R/G masks with B unused. Do not apply H4 semantics to Reach.

## Contradictions between speakers
- Reach spec in alpha: err_0r says alpha is both smoothness and part of the spec colour product; pointdevice treats it as a plain mask; surasia says CE-Reach use alpha for spec while Sniper's guide says an optional Material texture takes over.
- Vertex colour: gravemind (2021-05) recalls no render models with it; heavynator says Reach Elite body and Forge pieces have it; brooen (2022-12) says Longbow rocks should have it but export lacks it.
- Reach vs H3 Season 8: sniper says both "legit the same" and "some colour correction differences".
- Gamma: 2.0 (game parameter) vs 2.2 assumption.
- H2A armour: metallic PBR (heavynator) vs shader accepts both and some sets textured "Reach-style" (heavynator, same thread).
- Is Foundation "export mesh" useful: chiefster says no; Reach models only via Reclaimer. R93_sniper says no tool Reach support unless Reclaimer.

## Best leads for these problems
1. Black/too-dark Reach Elite Honour Guard shoulder:
   - Check tint colours/tint mask wiring first (Surasia node inputs; unset tint = black, mask on wrong channel).
   - Check covenant detail-map multiply: factor 4.59479 and gamma 2.0; missing it makes the piece much darker.
   - Suspect an absent vertex-colour mask on Elite meshes (Reach Elite body is a named vertex-colour user). Re-export via DAE/RMF if colour is wanted.
   - Verify by disconnecting colour/spec maps and viewing pure base colour (_gallium, 2021-03).
   - Compare against Foundation: open the material then its shader (Expert Mode) to read the actual parameters (gameoverload, 2024-10).
2. Elite shine/spec wrong:
   - Reach diffuse alpha is the spec mask (not opacity, unless transparent); some pieces use a separate Material texture (R spec coefficient, G albedo blend, B env contribution, A roughness) and then alpha is empty. Cubemap colour matters (colour space unresolved).
   - Do not reuse Halo Online Elite alphas (known corrupted).
   - Spec/env behaviour is pseudo-PBR in Reach; true PBR is H2A/H5 only (adorabirb, 2021-04), so metallic conversions will look off.
3. Importing custom armour to Reach: nothing concrete in chunk. Only: Reach editing kit has all retail tags; Foundry (Crisp) imports Reach tags into Blender; reverse export from game data is not supported; Reach armour menu models have broken UVs. Look to other chunks for tool.exe model-import and JMS/render_model procedures.
4. Networking custom armour pieces to other players: no information in chunk.
