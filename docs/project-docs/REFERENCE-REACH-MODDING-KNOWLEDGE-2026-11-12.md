# Halo Reach / MCC Modding Reference (compiled 2026-11-12, updated 11:30 ET)

Sources: Tyler's pasted Discord threads/links/attachments, pages fetched this session, and a Discord export of Halo Mods #materials filtered for "shader" (209 messages, 2022-04 to 2026-09; full digest in DISCORD-HALOMODS-MATERIALS-SHADER-DIGEST-2026-11-12.md). "(unverified)" = chat message, not docs. Not read (no URL in hand): Gibbo HREK guide, Reach Modding Guide slides, BLF repo, Blam Monitor Master, TagTool, c20 animation-data (fetch failed). Reclaimers #reach-help export failed ("forbidden": Tyler's account cannot read it).

## 0. Elite shading: status and what to test next (read this first)
RULED OUT (checked 2026-11-12 on the Honor Guard shoulder shaders vs the stock Ultra shoulder):
- Shader parameters: albedo_color, env_roughness_scale, use_material_texture, no_dynamic_lights identical (extra on HG: self_illum_color/intensity only).
- Shader options: identical option indices, same reference template elite_armor_template.shader.
- Compiled shader template: both use shaders\shader_templates\_10_2_0_1_1_2_0_0_0_0_0_0.render_method_template and it loads OK (the Discord "red template in postprocess" failure does not apply).
So the black HG shoulder wing is NOT a shader-setup problem. Remaining leads, in order:
1. Which mesh/material the black wing actually is (hide-piece test in the Armory with screenshots; tall wing may belong to helmet/forearm, big panel to legs).
2. Baked PRT/AO data on that mesh (re-import without PRT, or "precise").
3. The texture itself: wing UV island dark in diffuse and no cc coverage (cc is red-only mask on ~18% of the UV in the originals).
4. Specular mix / spec mask: the base_map alpha is the specular mask ("Specular mask from diffuse"); with env mapping on, an alpha of 0 or an odd value changes the look, and the spec mask controls how shiny something is even when the shader allows shine.
Reach is "very picky about shader properties" (Discord): when something misbehaves, copy a known-good stock shader (e.g. the marine armour) instead of building from scratch.

## 1. Pipeline (Reach HREK)
- .gr2 (Granny) replaces ASS/JMS. Flow: FBX -> `tool_fast fbx-to-gr2 <fbx> <json> <gr2>` -> `tool_fast import <x.sidecar.xml>`; levels then `python calc_lm_farm_local.py <scenario> <bsp|all> <quality>` (high, medium, low, direct_only, super_slow, draft, debug). `tool` and `tool_fast` behave the same; tool_fast is multithreaded with fewer error messages.
- FBX rules: transforms on skeleton nodes not meshes; material names must match `material_properties` keys exactly; node/mesh names are the JSON keys; regions via `bungie_face_region`; markers = nodes with object_type marker + marker_type.
- JSON: auto-created on first run, never updated after (edit then reconvert); keys need `bungie_` prefix; mesh-suffixed properties duplicated in nodes_properties and meshes_properties; material missing => `tags\shaders\invalid.shader`. Sidecar region definitions win over JSON; `default` region must be index 0; indices/order must match when replacing an existing model. Global material overrides only in JSON.
- shader_path in the JSON has no `.shader` extension and no stray backslash. If a shader tag sits in the model's own shaders folder with the right name, a re-import finds it automatically.
- Sidecar folder layout: models\ (FBX+JSON), export\models\ (GR2), bitmaps\; sidecar filename = model name; start from samples example_level / example_tentacle / example_health_pack.
- Can read `bungie_shader_path` from FBX custom props. Instance imposters and marker permutations supported. Stock tags lack import info.
- Known issues: no resource sharing; no single-threaded lightmapping (use multi-process with one client).
- H3 folders for comparison: data (FBX/ASS/JMS/TIFF/WAV), tags, maps; shaders folder = material shaders, rasterizer = global shaders. `script_doc` console command writes hs_doc.txt.

## 2. JSON parameter catalogue (c20 /hr/guides/json-parameters/; page has typos)
- Material: shader_type (override, shader, shader_cortana, shader_custom, shader_decal, shader_foliage, shader_fur, shader_fur_stencil, shader_glass, shader_halogram, shader_mux, shader_mux_material, shader_screen, shader_skin, shader_terrain, shader_water), shader_path.
- Face: face_type, face_mode (normal, render_only, collision_only, sphere_collision_only, shadow_only, lightmap_only, breakable), face_sides (one/two sided, +transparent), face_draw_distance, face_global_material, face_region, texcoord_usage, decal_offset, group_transparents_by_plane, no_shadow, precise_position, conveyor/ladder/slip_surface.
- Mesh: mesh_type, mesh_primitive_*, mesh_tessellation_density, mesh_additional_compression, mesh_poop_lighting (none/single_probe/per_pixel/per_vertex), mesh_poop_imposter_*, mesh_poop_precise_geometry, mesh_poop_excluded_from_lightprobe, portal/decorator/seam/water/fog properties.
- Lightmap: lightmap_type (per_pixel/per_vertex), resolution scale, chart group, bounce modifiers, translucency tint, lighting_from_both_sides.
- Material lighting: lighting_emissive_color/power/focus/quality/per_unit, lighting_attenuation_cutoff/falloff, lighting_bounce_ratio, lighting_use_shader_gel.
- Lights: light_type/shape/game_type, attenuation, hotspot, frustum, color, intensity, clipping sizes, gel/lens flare/shader references.
- Markers/physics/animation: marker_type list, physics_constraint_*, animation_control_*, animation_event_* (13 types incl. loop, sound, footstep, ik_active, ik_passive, frame, import).
- PRT is NOT mentioned anywhere on this page.

## 3. Shader docs (MS Learn H3, applies conceptually to Reach)
- **Albedo options**: default, detail_blend, constant_color, two_change_color, four_change_color, three_detail_blend, two_detail_overlay (undocumented), two_detail, color_mask (undocumented), two_detail_black_point (undocumented). base_map alpha can drive specular_mask, material_model, alpha_test, blend_mode. In `default`, albedo_color_alpha scales alpha grayscale; in detail_blend base alpha masks detail (black = full detail).
- **change_color_map**: two_change_color uses R (primary) and G (secondary); four_change_color uses R,G,B,A = primary, secondary, tertiary, quaternary. Colours set in the object tag.
- **Material model**: diffuse_only (no specular, default), cook_torrance (per-pixel specular/reflection/roughness/albedo blend; order3_area_specular, use_material_texture; needs env mapping or sides go black), glass, organism, single_lobe_phong (diffuse_coefficient, specular_coefficient). Per-pixel specular is in Specular Mask; env map source is in Environment Mapping (none = no reflections regardless of material model).
- Shader doc index: https://learn.microsoft.com/en-us/halo-master-chief-collection/h3/shaders/shadershome (about 50 option articles: albedo, bump, alpha test, specular mask, material model, environment mapping, self illumination, blend mode, parallax, rim, subsurface, templates, ...). All shader work is done in Guerilla/Foundation; picking a specific template is "no longer necessary" (legacy).
- Halo CE note: one change-color channel per shader_model; fake more with forced shader permutation (not relevant to Reach).

## 4. tool.exe tips from Discord (unverified, build-specific)
- Skip degenerate tri: Reach 0x1E69FD. Vertex weld/nodes: 0x2395EC. Cubemap full-mip: Reach 0xD5457 (NOP 12 bytes).
- Use "precise" for Reach models. PRT generation can fail on high-poly meshes.
- Colour change: R/G/B/A = primary..quaternary, black = none; quaternary only if cc has all 4 channels (Jackal note).
- "Black uniform no matter what the controls": cause was "specular mix".

## 5. Attached documents (Discord, 2026-11-12)
### 5a. H2 lightmapper scenery emissive fix (tool_fast.exe, H2 MCC)
- Symptom: emissive scenery light bakes ~2 world units too low. Cause: surface-light collector sub_499603 (build_lights) runs BEFORE process_scenery_object decompresses the [-1,1]-normalized vbuf; fix = decompress vbuf at collector entry (world_local = h*v + c from compression bbox at section+0x48), shared done-set with occluder decompress, gate on caller return address 0x4995EC (scenery only; poop verts also normalized and must not be touched). Key addresses: 0x499603, 0x49946B, 0x4AE696, 0x498962, 0x49DA6A, 0x5AFB7F/0x5AFC38, 0x4A2D56, 0x4A1F84, 0x4A1278. 196-byte working vertex: pos +0, UV +68/+72, normal +76. Method (order probe, caller-tagged log, world-origin log) reusable for other lightmapper bugs. Lesson: H2/H3 compressed verts must be decompressed before any consumer.
### 5b. Halo Reach MCC 65,535-particle hex patch (haloreach.dll 1.3528.0.0)
- Original size 13,229,016 bytes, SHA-256 738dd2d2...0894; patched 13,229,568 bytes, SHA-256 02239e6d...78f5. Raises GPU particles 7,168 -> 65,535, spawns/frame 576 -> 65,535, systems/locations 576 -> 4,608, CPU emitters 832 -> 6,656, CPU particles 2,048 -> 16,384, game state 0x1380000 -> 0x1BA0000. Seven steps: pool sizes, count-check branch flips + code redirects, memory-bank/checkpoint sizes, snapshot buffer displacement redirects (20), append 40 zero bytes + 512-byte code block at 0xC9DC00, new `.pbudget` PE section header at 0x388, PE header fields (sections 7->8, SizeOfImage, checksum, clear security dir). Apply to a COPY of haloreach.dll with original backed up; full tables in the source file (re-attach if needed). Not applied to Noble.
### 5c. forge_strings.txt (Reach Forge UI string table)
- ~1,138 lines of `string_id = "text"`: forge menu/help text, object palettes (weapons, vehicles, gadgets, structure, scenery, MCC additions "ff_thorage_*", "ff_cex_*"), team names, named-location strings per map. Useful as a string-id naming reference for Noble's menu strings / noble_menu.string_ids.csv. Full list in the source file.

## 6. Other leads from searches
- Nexus MCC mod 2481: Jackal bitmaps edited to use the D (alpha) channel for colour change in biped tags.
- General101 Halo Asset Blender Development Toolset (GitHub); c20 Discord discord.reclaimers.net.
- MS Learn Reach FBX->GR2 page: https://learn.microsoft.com/en-us/halo-master-chief-collection/hr/art/fbxtogr2 (read to char 100000 of 104905).
- c20 H3 shader template regeneration (also applies to Reach in practice): `tool dump-render-method-options`, then `tool generate-templates win shaders\<type>` (shaders\shader, particle, halogram, decal; about 30-60 min). Not required for rendering, only caches templates.

## 7. Project state
See HANDOFF-ELITE-RR-TEXTURES-2026-11-12.md and FINDINGS-ELITE-ORIGINALS-COMPARE-2026-11-12.md.

## 8. Halo Mods #materials "shader" digest: useful takeaways (full text in the DISCORD digest doc)
- **Shader breaks when a colour-change map is added** (2024-09-19): texture swaps to an unrelated one. Cause: the shader template in the postprocess block does not exist (Foundation: hold Alt when opening the shader; a RED template entry means missing). Fix: compile/generate the template. Reach ships ~2100 templates (H3 ~2500) but you often have to generate more. Also: "reach is very picky about the shader properties you use", so copy a stock armour shader (e.g. marine armour) and change only the bitmaps.
- **Colour channels**: R primary, G secondary, B tertiary; quaternary is only seen via a four_change_color shader and its bitmap.
- **Reach has no PBR shader** (no Bungie/343 one): only diffuse, specular (standalone, or packed in the diffuse alpha via "Specular mask from diffuse"), bump/normal, height (parallax). A community H3 PBR shader guide exists (https://c20.reclaimers.net/h3/guides/shaders/pbr-shader/); someone asked if it works for Reach, someone says they used it for Infinite marines in Reach.
- **Bitmaps**: diffuse alpha = specular map or transparency (alpha blend). `_diffuse` suffix is the default usage. Force format: DXT1 (alpha becomes 1-bit; do not use if alpha matters), DXT5 (keeps grey alpha), 24-bit colour + 8-bit alpha or best uncompressed for the highest quality. The specular mask controls how shiny, even if the shader would otherwise allow shine.
- **Metallic look in Reach**: turn on a material model (cook_torrance is a good start) and environment mapping; copy settings from an existing metal shader. Cubemap: set the shader's `environment_mapping` category to `per_pixel` and pick a cubemap bitmap (the assault rifle shader is an example).
- **Dynamic lights**: shaders have a "no dynamic lights" style checkbox; shadows still apply.
- **Finding a shader**: in Sapien `collision_debug 1` shows the shader name/path of a surface (disputed for objects); or import the BSP/model into Blender and read the material name, then open that shader in Guerilla/Foundation.
- **Foundation caveat** (2023): a user could not save shader changes in Foundation; use Guerilla if that happens.
- **Spec tint map tiling** is missing in the Bungie cook_torrance shader; adding it means editing material_models.fx / cook_torrance.fx (transform_texcoord).
- **Shader functions**: only float values animate in shader functions; periodic functions do nothing directly on frame index/colour values unless fed from an object function; evaluators take time as a parameter.
- **Wiki/tools mentioned**: Microsoft Learn shader docs, c20 compiling-shaders, c20 pbr-shader, Crabtool, "Foundry".

## 9. HREK pathfinding and AI on multiplayer maps (tested 2026-11-21, Phase 7)
Full log: `docs\PHASE7-AI-PATHS-PROGRESS.md`. All tested on Forge World with HREK's own tool.exe / ManagedBlam.
- **Bake:** `tool generate-pathfinding <scenario>` (no Sapien needed; ~70 s for Forge World). Writes the scenario's
  `ai pathfinding data` (per BSP: sectors, links, bsp2d refs / nodes, vertices, object refs, hints, instanced geometry
  refs, seams). The BSP tag's own pathfinding block stays empty on every map.
- **"no pathfinding" BSP flag:** `structure bsps[n].flags` bit 9 (raw 512). If set, the bake "succeeds" and writes
  nothing. Stock Forge World has it set; ff20 sets it only on non-playable BSPs.
- **"don't strip pathfinding" scenario flag:** top-level `flags` bit 1 (raw 2; bit 13 is "survival"). Without it
  `build-cache-file` strips the nav mesh from **multiplayer** scenarios. Building as `type = solo` also keeps it but
  drops every `globals\mp` tag (403 on Forge World): do not.
- **Firing positions:** tool.exe does not generate them (Sapien does); write them yourself, then bake again so the
  bake fills each one's sector / cluster (points just outside the ground are dropped).
- **Block limits** (scenario definition maxima): zones 128, firing positions 2,048 per zone, areas 128 per zone,
  objectives 64, tasks 100, task areas 64, squads 256, spawn points 128 per squad.
- **Objects added to Forge World's scenario do not survive the bake:** crates or scenery added with ManagedBlam
  (plus their palette entries) are gone after `generate-pathfinding`; other edits (flags, squads, zones) stay.
  Cause not found yet.
- **ManagedBlam gotchas:** one `ManagedBlamSystem.Start` per process (run each script in its own process); pass
  arrays to scripts with `& script.ps1`, not `powershell -File` (it joins them, and drops empty-string arguments);
  in PowerShell never write `$x = if (...) { $element }` with a tag element (it unrolls into its fields); set enums by
  option name (option lists differ between tag types). `tool get-tag-field-value` is slow and crash-prone.
- **`build-cache-file ... compress-most`** ran ~25 min then failed "hash mismatch, verification failed": not usable.
- **One command:** `tools\noble-map-ai-bake.ps1 -Tag <scenario> -Centre x,y,z` does all of the above for a map.

## 10. Things that load into Noble from MCC's folder; Forge pieces and AI (2026-11-22)
- **ReShade / proxy DLLs in `MCC\Binaries\Win64` load into Noble too.** Noble loads Reach from that folder, so a
  ReShade `dxgi.dll` there was injected into `noble-dev.exe` (ReShade.log named it). Tyler's ReShade 6.8 + RenoDX /
  DLSS5 feeder (installed 2026-10-17, also a `host64` folder) were moved to `D:\MCC-ReShade-backup-2026-11-22\`; copy
  them back to restore. When a Noble run behaves oddly on a player's PC, ask about ReShade / proxy DLLs in that folder.
- **Forge pieces and AI:** a player on a Forge piece resting on the ground is located on the ground sector below
  (AI can chase them); on a raised piece the engine finds no sector, because its ground finder ignores Forge crates.
  Details: `docs\REACH-PATHFINDING-DATA.md`.
- **Forge piece tags in a .map:** one piece name has several tags (crate, model, collision, render ...);
  `tools\research\map-tag-names.ps1 -Map <.map> -Pattern <regex>` lists them with their indices.
