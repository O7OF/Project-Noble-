# Notes p1_04 (chunk_04: substance-su, control-map-guide, lavadeeto-re, ripping)
Tags: (author, YYYY-MM, channel). Confidence: [confirmed]/[claimed]/[disputed]. Most content is community ripping/Blender/Unity-side, not Reach tool.exe; relevant mainly for understanding source textures/shader math. Reach MCC-specific points are marked REACH.

## 1. Control / CC / texture channel layouts (REACH first)
- REACH/H3/ODST texture set: Diffuse RGB=colour, A=spec mask OR opacity; Normal=DirectX-format (green inverted vs Blender/Unity); CC map R=primary colour mask, G=secondary colour mask, B=unused; Illum RGB = emissive COLOUR (not a mask); "Material" texture R=specular coefficient, G=albedo blend, B=environment contribution, A=roughness. When a Material texture is used the diffuse alpha usually holds no spec. Not every object has Illum or Material maps. [confirmed, author is the guide writer] (r93_sniper, 2021-12, control-map-guide)
- Halo 2: 2 changeable colours per material; spec stored in diffuse alpha (opacity moves to alpha only if transparent); CC R=primary, G=secondary. (r93_sniper, 2021-12, control-map-guide)
- Halo CE multipurpose map has two variants (no way to tell except by inspecting channels): OG Xbox R=spec mask, G=self-illum, B=primary CC, A=auxiliary; Gearbox R=aux, G=self-illum, B=spec, A=primary CC. CE only supports ONE CC colour. (r93_sniper, 2021-12, control-map-guide)
- Halo 4 control maps: Type 1 (weapons/props, has separate diffuse, "multi" suffix): R=spec coefficient, G=spec exponent, B=reflection mask. Type 2 (all MP armour, no diffuse): R=spec coefficient, G=diffuse, B=spec exponent, A=colour change; CC alpha is two-tone: white=primary, 50% grey=secondary, black=uncoloured. (r93_sniper, 2021-12 and 2021-06, control-map-guide/ripping)
- H2A Multiplayer (CA, Halo4-engine-based but PBR): C1 control R=metalness, G=AO, B=self-illum mask, A=smoothness; D (CC) R=primary, G=secondary, B=none, A=scratches/wear; S RGB=dielectric spec (author unsure); N=DirectX normal. [claimed - spec note flagged as assumption] (r93_sniper, 2022-01, control-map-guide)
- H2A MP armour shader accepts both metallic and spec inputs; 343/CA textured some sets "Reach style" while H2A is metallic PBR. Campaign H2A weapons are spec/gloss, MP ones metallic/smooth and need less work for Blender. (the_heavynator, 2021-11, ripping; r93_sniper, 2023-11, ripping)
- Halo 5: Color RGB=albedo; MP Spartan control R=primary CC, G=secondary CC, B=metalness, A=smoothness; weapon control R=smoothness, G=AO, B=metalness; helmet visor hex metalness is in R not B. (r93_sniper, 2022-01, control-map-guide)
- Halo 5 history: community believed control blue = Spartan colour for a year until a Unity shader disproved it - verify channels with a working shader. (fiycsf, 2021-04, ripping)
- Halo Infinite ASG: R=AO, G=scratches/wear, B=grime/dirt. Mask0 (CC1) R/G/B = zones 2/3/4, Mask1 (CC2) R/G/B = zones 5/6/7. Zone 1 is derived (invert the sum of the others). Infinite uses procedural materials ("ProcMats"/HIMS); layer shader masks 7 layers. (r93_sniper, 2022-01; gameoverload, 2023-04, control-map-guide/ripping)
- Halo Wars 2: al=albedo, em=emissive, nm=normal, sp=specular, pm packed R=gloss, G=AO, B=CC mask. HW1: df, sp (R=spec, B=gloss?), rm (rim), nm, xf (inverted CC mask); normal blue must be rebuilt/normalised. [HW1 section flagged as unsure by author] (r93_sniper, 2022-01, control-map-guide)
- CEA: diffuse alpha is typically spec mask (sometimes transparency); the "br" (brightener) map usually has emission mask in R, other mask = reflection (per shader code). (chiefster_4014, 2024-09, ripping)
- Infinite normal maps: blue channel often carries another packed texture, so rebuild Z in-shader (reconstruct from XY) rather than using blue; Unity's "Normal map" import type drops the 3rd channel. (bios_0307, 2021-12; chiefster_4014, 2025-04, ripping)
- Normal maps colour space: set Non-Color; Python: bpy.data.images['x'].colorspace_settings.name = "Non-Color". Halo (H3, Reach, ODST, H4, H5) is DirectX orientation, so invert green for Blender/Unity/OpenGL. If a normal looks inverted/bent in Painter, toggle DX/GL setting. (_gallium, 2021-03; sswampp, 2021-11; r93_sniper, 2025-06)
- Blender setup for flipping green: split RGB -> Invert on G -> Combine RGB -> Normal Map node. (sswampp, 2021-11, ripping)
- Diffuse alpha spec in Blender: set alpha to "Channel Packed" alpha mode, normals Non-Color. (surasia, 2021-03, ripping)
- Colour-change principle: CC mask selects regions; lerp/multiply tint colour onto those regions; use linear (non-sRGB) for mask/normal/packed maps, sRGB only for diffuse (and coloured spec). (err_0r, 2021-03, ripping)

## 2. Shader / lighting math (spec, fresnel, detail maps)
- REACH covenant detail map: detail colour multiplied by 4.59479 before being multiplied into base map colour; game shader gamma setting seen as 2.0 vs 2.2 assumed by people (chiefster_4014, 2025-04, ripping) [claimed from reading shader code]. Colour values from Halo's linear colour may need gamma 2.2 correction (tacohombre, 2024-01) [claimed].
- Reach/H4 are "pseudo-PBR" (lights interact with spec/env but not true PBR); H2A and H5 are true PBR. (adorabirb, 2021-04, ripping)
- Reach diffuse alpha in practice is both smoothness and a value multiplied with diffuse RGB into the specular input (err_0r, 2021-03) [claimed]. Quick-and-dirty Blender approximation: alpha*0.71 -> glossy colour, roughness ~0.3, plus diffuse, via Add shader. (pointdevice, 2021-03, ripping)
- CE Elite colour mostly comes from the cubemap (env) - get the cubemap, convert to panorama, plug into shader. (r93_sniper, 2023-01, ripping)
- H3/Reach cubemaps: needed colour mode was asked, not answered. H3 reflection cubemaps have an alpha channel (purpose asked, unanswered). (theishikawarin 2025-02; cameraguy 2023-02)
- Detail-normal blending mistakes make normals look flat in custom shaders (chunch, 2021-12).
- Reach vertex colour: only supported by Reclaimer when stored in position W; vertex-alpha used by H2A/H4 overlay meshes ("vertalpha", srf_ca_color_warp_vertmask). Vertex colour does not tint the mesh; used for shader masking (e.g. dark insides of Forge pieces, snow on rocks). (the_heavynator/pointdevice, 2021-05; gravemind2401, 2022-12 & 2024-11)
- H3/Reach use UV2 for water masks/main colour; H3+ water needs custom UV rebuilt after extraction. (the_heavynator 2021-05; chiefster_4014 2023-07)
- Halo 4 per-material shaders are highly individualised (many variants, each cut down separately), unlike an uber-shader approach. (gameoverload, 2023-05, 2025-02)

## 3. Shader code access / reading shader params
- Shader source: viewable via MCC mod tools for CE, H2, H3/ODST, Reach, H4, H2AMP; for CEA and H2ASP extractable from game; H5/Infinite only compiled bytecode. (chiefster_4014, 2024-09, ripping)
- Procedure for Reach: open the material in Foundation, note the shader it references, open that shader in Foundation (ENABLE Expert Mode) to see the shader code; learn HLSL docs at learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl. (gameoverload, 2024-10, ripping)
- Parameter names/values in Assembly/Guerilla/Foundation show what to match when recreating (sswampp 2021-11).
- Bytecode decompile: H5 via 3DMigoto; Infinite via dxil-spirv then spirv-cross (use dxc to keep debug info). (coarforge, 2024-10, ripping) NVIDIA Nsight frame capture can pull MCC PC DX11 shaders; dx9 only decompilable for Halo Online. (pointdevice, 2021-05)
- H4 has no shader templates (texture list only), unlike H3 shader templates that specify texture use. (gravemind2401, 2021-05)
- Material shader tag has a "source shader files" block with shader data (pixel + vertex entry points). (gravemind2401, 2021-05)
- Tool for decompiling MCC bytecode: HIRT build mentioned (urium86, 2023-05). Not Reach tooling.

## 4. Scale / units
- Halo uses world units across CE-H4. 1 wu = 100 units in 3ds Max (inches) = ~2.54 m in Blender/Unity per author's import convention. Chief model tallest vertex ~0.7036 wu (H1/H2), 0.7028 (H3), 0.7105 (H4). Scale factors to reach lore 2.18 m: Blender x1.0165 (CE) -> proposed universal 1.0165 (Blender) / 1.22 (Max). (lavadeeto, 2021-07, lavadeeto-re) [claimed, person-measured]
- Reach Spartans use human skeleton scaled ~1.2x (question on getting exact scale; unanswered) (frostbitecinem, 2024-03).
- Halo CE/H2 colour hex tables (CE: red fe0000, blue 0201e3, ... ; H2 armour: FDFEFF white, 535353 steel, BE2C2C red, F57A1F orange...) exist in lavadeeto-re 2021-07-08 for reference of player colours.

## 5. Ripping tools & versions
- Reclaimer (Gravemind2401): reads H3/ODST/Reach/H4/H2A MP maps; Reach support broken by MCC updates multiple times (2021-11, 2023-02) - get latest build; error "valid .map but may not be a supported version" = old Reclaimer; MCC Reach assets same as Xbox 360 for models. Reclaimer v1.5.244/245 releases; installer v1.7.309 seen. Latest Reclaimer2 (2025) has errors such as ArgumentOutOfRange in MccHaloReach.IndexItem.ReadMetadata for Forge workshop maps (frankr2994, 2025-02, ripping) - unresolved.
- Reclaimer exports AMF/DAE/RMF; AMF has no vertex colour or multi-UV; DAE has both but H2A bsps crash assimp; RMF supports multiple UVs (gravemind2401, 2021-05, 2024-06). Reach uses no multi-UV models known (gravemind2401, 2024-06).
- Reclaimer error "FileNotFoundException ... campaign.map" when ripping ODST render_model: shared/campaign .map files must sit in same folder (tacohombre, 2022-07). Some ODST/H3 bsps fail with NullReferenceException in scenario_structure_bsp.ReadGeometry after MCC update; 360 maps work better than MCC ones (gravemind2401, 2023-02).
- Foundry (Crisp's Blender addon: github.com/ILoveAGoodCrisp/Foundry) imports most Reach/H4/H2AMP tags directly into Blender; installation errors like missing numpy and "Import completed in 33 milliseconds" with empty scene seen, unresolved (shuttl_, un_nonimus, 2025-03). For CE use General-101 toolset (github.com/General-101/Halo-Asset-Blender-Development-Toolset) to import gbxmodel; H2-ODST: Tool commands extract JMS (H2 separate commands for render/physics/collision/structure; H3/ODST: `extract-import-info`). (chiefster_4014, 2025-04, ripping)
- Reach editing kit on Steam contains all retail tags; Reach model export via Foundation "export mesh" gives .x files not usable; Reach models cannot be pulled out through mod tools (render data not retained) - use Reclaimer. (chiefster_4014, 2025-03; 2022-10; 2023-07)
- Tool animation extraction (H3 syntax): `<map> EditTag <tag>.jmad ExtractAnimation all "<folder>"` e.g. EditTag objects\vehicles\pelican\pelican.jmad. (r93_sniper, 2022-12, ripping) [confirmed for H3].
- Bonobo rips animations from H3/ODST/Reach (Xbox/MCC partial); first-person animations won't rip; needs correct .NET; common failure "reach maps not supported" (plastered_crab 2022-01; hobbes240 2023-05; guga666555 2023-06). Importing JMA into Blender: errors "no valid armature" and "Node 'camera_control' not found in an existing armature" mean the armature/skeleton must be in scene first (guhgunk 2022-08). 77Mynameislol77/HaloAnimationRepository (GitHub) holds ripped anims.
- Halo 2 colour-plate extraction: github.com/SnowyMouse/halo2-color-plate-extractor; command form: halo2-color-plate-extractor <tags dir> <data dir> <bitmap tag> (zadeinator, 2024-03).
- Index tool is for H2A campaign; H2A multiplayer maps need Reclaimer; MP map files are `.map` only (no "anniversary graphics") (chiefster_4014 2024-09; gravemind2401 2025-02).
- CR4B tool (github.com/PlasteredCrab/Halo-CR4B-Tool): Blender one-click shader setup for H3/ODST/Reach (Reach shader setup not working as of 2023-11; needs level report). Error "bpy_prop_collection[key]: key 'shader' not found" for H3 MP Spartans (2024-01, unresolved). H3 gaps: water and halogram shaders. (plastered_crab 2023-03..2023-11)
- Halo Infinite: Ekur/HIRT extraction names files by hash; not Reach-relevant.
- UModel (UE version 4.21) for MCC menu assets (CE skins/PBR textures); Autodesk FBX SDK 2020.2.1 link shared (sswampp 2021-11).
- H3 retail has gray bitmaps; colour bitmaps in leaked H3 pre-betas; some H3 normals use palettised (u8v8-like) 256-colour format - Reclaimer got the palette from the exe (gravemind2401, 2021-04/05).
- Reach 360 vs MCC: Reach assets essentially identical; Reach menu armour model UVs broken - do not extract from menu (the_heavynator, 2021-10).
- Reach mesh normals are known-bad/mirrored (also in original game) vs H5 (pointdevice 2021-04); Spade truck rips with wrong normals even with import normals on (vinwarrior 2021-06).
- Elite Reach: single head type with helmets merged into armour; Spec-Ops mandibles/gap question unresolved (cythecy_, 2024-11).

## 6. Rigging / skeleton / animation
- H2A Elite/Brute MP lack eye and face bones; male bodysuit distortion on H2A MP AMF import unresolved (denzeus 2023-03; x_skel07 2025-05).
- Auto-rigging: ActorCore AutoRig worked for ripped Spartan and mostly for Elite (hunched pose needs tweaks) (frankr2994, 2025-04). IK rig for packs missing one: asked, unanswered.
- Reach ripped bones appear tiny (scale) - check pack version (degweb, 2022-09).
- Reach models with H2 classic animations: some work with effort; H3 animations usable on some HW2 units (sswampp 2021-08; aqua_himura 2021-10).
- Collision model for ripped weapon: asked, no answer (mas2terchi0ef 2022-10).
- Bone drift in Reach first-person legs observed (pointdevice 2021-05) - cause unknown.

## 7. Substance Painter workflow (art-side, low Reach-tool value)
- Painter loads only active UV set; multi-UV sets (2nd UV on single material) fail; workaround remove extra set (r93_sniper/purplmunkii 2025-07). Name materials differently if UVs overlap; AO bake: Ignore Backface=Always, Self Occlusion=Only Same Mesh Name (kuddlykraken, 2026-02).
- Triangulate before Painter to avoid topology mismatch on reimport (jedinick9 2025-05). Normal warping: use Path tool, perspective projection errors (purplmunkii 2025-05).
- ASG/mask creation: for finished assets plug game normal into Normal slot and untick normal baking so curvature derives from it (purplmunkii, 2026-05). Halo shaders for Painter previews were released by Jack (JackOfChaos) - links in server (r93_sniper 2024-11). RGB Mask Splitter tool (ArtStation, free; SP 11.3.2) (purplmunkii 2024-10).
- Gloss->rough by inverting; spec->metal via levels (purplmunkii 2025-06). Substance exports blank spec if the "spec" channel not enabled for that material (cwallen 2024-09).
- Visor multi-material masking: R/G/B per material (r93_sniper 2026-02).
- Normal "completer" tool in pins to rebuild blue (chromacore, 2025-06) [claimed imperfect].

## 8. Misc
- Halo CE weather mask (_W map) per channel to lerp base texture to weather texture (Infinite) (frostbitecinem, 2022-06).
- GDC talk on H2A MP tech: https://www.youtube.com/watch?v=072HkIJ_DMs (r93_sniper 2022-01).
- Halo Infinite Coating Tool 2.0: github.com/Surasia/Infinite-Coating-Tool/releases/tag/2.0 (HIMS 2.7) - Infinite only.
- Dielectric specular is greyscale off base colour vs coloured specular (gameoverload 2023-03).
- Epic-recommended packing: A/B (base+opacity), ORM, N; alpha-packed maps cost memory (gameoverload 2025-01).
- Reach DLC maps contain forerunner textures like s3d_edge; Reach m10.map has zbump normals (sswampp 2021-09; _gallium 2021-04).
- Reach Forge world exports via Reclaimer lack foliage/materials wrong (oragani 2021-07, unanswered); Reach foliage/decorators in "shared" map errors.

## Open questions raised but unanswered
- Correct colour space for H3/Reach cubemaps (sRGB vs linear) and purpose of the cubemap alpha.
- How to extract Reach exact armour CC colour values (IDA reverse attempt, walkmen 2024-08).
- Why Reach Forge maps fail in Reclaimer (ArgumentOutOfRange in MccHaloReach.IndexItem.ReadMetadata) and why Foundry imports nothing.
- Reach foliage/decorators extraction; campaign cutscene environment (Exodus intro) texture import from Reclaimer AMF (waketheshark 2022-11).
- Exact Reach Spartan skeleton scale values (Jorge).
- Bonobo failure on MCC Reach maps ("unsupported"); JMA import "no valid armature".
- Multi-UV/vertex-colour in Reach RMF export - no known Reach model uses them.
- Foundation "export mesh" .x files usability.
