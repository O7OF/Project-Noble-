# Halo Reach / H3 Modding Field Guide (for Tyler, Claude and other AI agents)
Built 2026-11-12 from 12,581 Halo Archive Discord messages (2021-2026) + earlier Halo Mods #materials digest + MS Learn/H3 docs already in `REFERENCE-REACH-MODDING-KNOWLEDGE`.
Raw material: `D:\discord chat\compendium\01..11_*.md` (grep-able). This file = the distilled, usable part.

## 0. Honest assessment of the corpus
- The Halo Archive is a **rip / Blender / Unity / render** community. Most of the 12k messages are "how do I get a model out and make it look right outside the game". Only a minority are about in-engine Reach tag work.
- **Networking: essentially nothing useful.** The "networking" bucket was keyword noise (the word "multiplayer"). Real hits: one memory-reading question (custom-game stats overlay, 2024), MCC anti-cheat blocks injectors (NinjaRipper must be run with EAC off/offline), a HaloCE dedicated-server question. No netcode, replication, or tag-networking knowledge. For networking of Elite pieces use our own tests + Halo Mods/Reclaimers servers (not captured).
- **Gold nuggets** (below): control-map channel layouts per game, Reach Change Colors mechanics, Reach shader parameter meanings, shader/ripping tool list, hologram/H3 shader hints.

## 1. Texture channel layouts (r93_sniper control-map-guide)
| Game | Map | Channels |
|---|---|---|
| H2 | diffuse | RGB colour, A = spec (or opacity) |
| H2 | CC | R primary, G secondary, B unused |
| H3 / ODST / **Reach** | diffuse | RGB colour, **A = spec mask or opacity** |
| | normal | DirectX normal (flip G for Blender/Unity) |
| | CC | R primary, G secondary, B unused (as shipped in docs) |
| | illum | RGB = self-illum colour (not a mask) |
| | material (optional) | R specular coeff, G albedo blend, B environment contribution, A roughness. When present, diffuse alpha usually has no spec |
| H4 control type 2 (MP armor) | R spec coeff, G diffuse, B spec exponent, A colour change (white=primary, 50% grey=secondary, black=none) |
| H2A MP | C1: R metal, G AO, B illum mask, A smoothness; D(CC): R primary, G secondary, A wear |
| H5 MP spartan | R primary, G secondary, B metal, A smoothness |
**Reach caveat (chiefster, 2026-03):** "Reach CC maps can use RG but docs show only R"; the spec/blend/env/roughness layout is the H3/Reach cook_torrance model; 343's PBR shader (Infinite) is just spec+roughness. Our Elite uses four_change_color → R,G,B,A = primary..quaternary (we confirmed in-engine; A is NOT unused for four_change_color shaders).

## 2. Reach Change Colors (unordinal + soulburnin, 2021-11)  *** directly relevant to Elite shine/black parts ***
- Object/biped `Change Colors` block = 4 elements: 0 primary (CC R), 1 secondary (CC G), 2 tertiary (CC B), 3 quaternary (CC A).
- **Elements 2 and 3 double as tints**: element 2 = **fresnel tint**, element 3 = **specular tint** (when the shader has no explicit specular_tint param). Those tints do NOT use the CC map; element 3 is additionally masked by CC alpha when it acts as a colour.
- Final specular = (diffuse blended with detail, coloured by CC) mixed with specular tint. So **a black/very dark tertiary or quaternary colour in the permutation => black fresnel/specular => dark, matte "black" piece**, even if shader params are identical to stock.
- Elite armor specifically uses those slots for specular + fresnel tint. Visors have their own tiny CC with a non-white alpha.

## 3. Reach shader parameter meanings (the_heavynator / PointDevice / ChromaCore, 2021-05)
albedo_color tints albedo; diffuse_coefficient = final diffuse brightness; specular_coefficient = final spec brightness (direct+fresnel+env+area); specular_tint (clamped 0-1); fresnel_color, fresnel_curve_steepness (strength); analytical_specular_contribution = direct highlight brightness; analytical_roughness = highlight roughness (Beckmann; changes visible below ~0.5); area_specular_contribution (spec map "glows" with indirect light); environment_map_specular_contribution = cubemap brightness; env_tint_color; env_roughness_offset = blur via mip (0-0.5 no visible change); albedo_blend = how much diffuse tints spec; `roughness` and `approximate_specular_type` appear to do nothing.

## 4. Tools and sources worth remembering
- Ripping: Reclaimer (Gravemind2401; Reach/H3 maps, AMF export), Bonobo (animations, JMA), Adjutant/Assembly (shader/tag data, texture tiling), Foundry (Blender add-on to import tags/shaders), IndexV2 (H2A/CEA), General_101 Blender tools. Always need shared/campaign/ui .map files together.
- Blender Reach/H3 shaders: TheJudSub Reach nodegroup, chiefster Halo 3/ODST/Reach/H4 .blend, Foundry halogram shader. Unity: FiyCsf Open Halo Reach Spartan Shader v0.8, Lavadeeto H3 Spartan shader (needs lookup tables c78d78, cc0236, dd0236).
- To see exactly how a shader works: open the material's shader tag in Guerilla/Foundation (expert mode), read the shader code; Reach built on H3, so H3 shaders are valid reference.
- Anti-cheat: MCC EAC blocks injectors; run offline/EAC-off for tooling.

## 5. Lessons applied to our open Elite problems
1. **Honor Guard black shoulder wing** — shader params/options/template are identical to stock, so look at (a) the colour-change values (elements 2/3 = fresnel/spec tint) for that piece/permutation, (b) CC map B/A coverage on the wing UVs (if A=1 where it should be 0, quaternary dark colour darkens it), (c) diffuse-alpha spec mask, (d) baked lighting/PRT. Hide-piece test in Armory still needed to identify the mesh.
2. Council/Ossoona shine: check CC alpha + tertiary/quaternary defaults before editing shader.
3. Lights row: if a lights piece is dark, check that element 3 colour is not black.

## 6. How to use the raw files (agents)
- `grep -i "fresnel" 02_shaders-materials.md`, `grep -i "change color" 03_*.md`, etc. Entries: date, author, [channel], text.
- Authors worth trusting: r93_sniper (control maps), soulburnin, unordinal, chromacore, PointDevice, the_heavynator, chiefster_4014, FiyCsf, gameoverload (shader replication), Gravemind2401 (Reclaimer).
- Treat Discord as leads, not proof; verify in our ManagedBlam scripts (`shader_vals.py`, `shader_opts.py`, `shader_post.py`).
- Gaps: Halo Mods #mod-development / Halo Reach modding channels never exported; re-run with a fresh session if wanted.
