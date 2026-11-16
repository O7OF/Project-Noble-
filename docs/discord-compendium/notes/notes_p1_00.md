# Notes p1 chunk_00 (Halo modding Discord, 2021-2026; mostly #general-help + tool-index channels)

Note: this slice is dominated by Blender/porting/render help and a tool index. Direct Reach tool.exe/Sapien/Foundation content is thin; the most valuable material is the Reach colour-change/shader breakdown (2021-11/12), channel layouts, and tool links.

## 1. Tools, add-ons, and links worth keeping
- Assembly (XboxChaos): multi-generation Blam tag research/editor; used to inspect shader bitmaps, tiling and scale to rebuild materials elsewhere. https://github.com/XboxChaos/Assembly (r93_sniper, 2024-08, Assembly; sswampp, 2021-11, general-help)
- Foundry (Blender add-on by Crisp/ILoveAGoodCrisp): asset creation for Reach, H4, H2A MP; later added render/collision/physics tag IMPORT into Blender. https://github.com/ILoveAGoodCrisp/Foundry (r93_sniper/iloveagoodcris, 2024-08, Foundry)
- Reclaimer (Gravemind2401): extracts models/textures/sounds; AMF/RMF importers for Blender and 3ds Max; v2.0.0-beta needs .NET Desktop Runtime 7 and has an RMF importer (read upgrade notes). https://github.com/Gravemind2401/Reclaimer ; add-on downloads on MediaFire "Reclaimer_Downloads" (gravemind2401, 2024-08, Reclaimer)
- Adjutant 4.2.0 was the version needed for Reach/H4 in the older toolchain; Reclaimer covers CE through ODST (plastered_crab, 2021-11, general-help) [claimed, old info; Reclaimer later added Reach].
- Bonobo 1.0.0.3: animation extractor for H2/H3/ODST/Reach (reads Bungie-era .map). https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003 (zad_47/rafaleeater, 2021-05/2022-06)
- General-101 Halo-Asset-Blender-Development-Toolset: CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. https://github.com/General-101/Halo-Asset-Blender-Development-Toolset (r93_sniper/sswampp)
- TagTool (TheGuardians, dev branch): modify tags in built caches. https://github.com/TheGuardians/TagTool/tree/dev (r93_sniper, 2024-08)
- Brooen add-ons: Halo-Reach-Light-Importer (.scenario_structure_lighting_info to Blender lights), Halo-4-Light-Importer, Halo-4-Material-Importer, Halo-5-Data-Importer (.material / structure_lights), Halo-LinearAnimator (animation helper). All under github.com/Brooen/ (brooen, 2024-2025)
- Halo 5 Animation Extractor v1.1 (Generalkidd/H5AnimExtractor): pushes H5 animations into Blender, integrates with Foundry, carries over flags like Disable Interpolation In/Out, Disable Mode IK. (green.knight, 2026-08)
- Index / IndexV2 (Wildenhaus): extracts CEA and H2A campaign content. https://github.com/Wildenhaus/IndexV2 (r93_sniper, 2024-08)
- Blender-DDS-Addon (matyalatte): DDS import/export with many DXGI formats. https://github.com/matyalatte/Blender-DDS-Addon (gameoverload, 2024-08)
- Blader (arkryl): generates empty H3 .shader files from FBX materials; does NOT fill in bitmap data. https://github.com/arkryl/Blader (surasia/itzhouze, 2021-11)
- h3-shader-compiler-fix (num0005): small DLL fixing Tool's use of the H3 shader compiler. https://github.com/num0005/h3-shader-compiler-fix (sswampp, 2021-10)
- Osoyoos launcher (HEK GUI launcher): https://github.com/num0005/Osoyoos-Launcher ; Invader/SixShooter and Mozz mek are H1 only.
- Reclaimers Library docs (H3 level creation hub): https://c20.reclaimers.net/h3/level-creation/ ; H1 shader tag docs https://c20.reclaimers.net/h1/tags/shader/ (dervish0/piratepenguin, 2021-11/2022-04)
- InfiniteCoatings JSON (Surasia) includes Reach armor coatings, e.g. armor/reach/reach____rch_mil_emile.json. https://github.com/Surasia/InfiniteCoatings (brianthefirelo, 2022-07)
- Halo Infinite shader resources (AverageTrapEnthusiast): material-type releases; R93Sniper/HaloInfiniteVRCShader (Amplify Unity shader); Open Halo 5 Spartan Shader for Unity gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity (various, 2021-2022)
- Vanity (veegie.com/Vanity): Reach player image generator, reference for armour permutations (richter3d, 2021-12)
- Where multiplayer map .map files live on Steam: steamapps/common/Halo The Master Chief Collection/halo reach/maps (dr.strange117, 2021-09, general-help)
- Old Xbox 360 Reach maps were sourced from haloce3.com/xmf; they need to sit in one directory for Bonobo/Reclaimer (plastered_crab, 2021-08) [site was reported down in 2022-08].
- Skull/umodel (UModel) has some higher-res Reach/H3 textures but with errors and missing detail maps (chromacore, 2021-09).

## 2. Reach colour change (CC), change-colors block, shader inputs
- Reach multipurpose/control map channel layouts, two variants: Xbox (360) maps R=Specular, G=Self-Illumination, B=Color Change, A=Auxiliary Mask. PC/MCC-format maps are R=Auxiliary Mask, G=Self-Illumination, B=Specular, A=Color Change mask. If you rip from MCC you most likely get the PC layout. (r93_sniper, 2021-06 and 2021-10, general-help) [confirmed by repeat statement]
- Reach tag-side breakdown: biped tags carry four "Change Colors" blocks; slots 0-3 correspond to primary, secondary, tertiary, quaternary colours, each mapped to a CC-map channel (R/G/B/A order of the CC map). (unordinal, 2021-11)
- Open observation: some variants have colours set in slots with no matching region in the CC map (e.g. Skirmisher major feathers on blue/slot 2; skirmisher/elite specops fresnel looks tied to tertiary colour). Resolution reached: apparent "tertiary-to-fresnel" links were really (a) CC map having a green region that was missed, (b) cubemap tint on specops pieces being greenish, (c) self-illum colour set directly in the illum shader (e.g. Ranger elite orange comes from the illum_cards shader), not from the colour block. [disputed then partially resolved] (unordinal/soulburnin, 2021-11, general-help)
- Skirmisher/Elite armour shader pipeline (soulburnin): diffuse blended with detail map -> coloured by CC map -> mixed with specular tint (0 = only specular tint, 1 = full albedo tint) = final specular colour. Fresnel curve value 0 = fresnel colour fully overrides specular tint. Globals tag values for elite: specular tint #D6D4DB, fresnel colour #E8C6A4. Per HLSL, the albedo is subtracted from the specular tint for the mix (unordinal re-read the HLSL). Colour values in tags are linear, so correct for sRGB in Blender. [claimed, from tags+HLSL reading] (soulburnin/unordinal, 2021-11, general-help)
- Reach "_material" textures on faces: unclear; looked like a mask between two skin-spec parameter sets, whitest where skin oil accumulates (gallium, 2021-04). Reach shading is specular-based (no metallic/smoothness), per frame-debugger inspection (fiycsf, 2021-04). [claimed]
- Reach models carry baked damage/wear in normal and spec maps, so Substance repainting looks like painting over dented armor; use H5 Reach-style parts for repaints (fiycsf, 2021-12).
- Reach spec/rough -> Blender shortcut: plug the specular (in the alpha of the colour/spec texture) into Principled specular (fiycsf, 2021-12). Not exact; game-accurate needs the Reach shader replicating Assembly values.
- Reach tag inspection is the reliable path: open the shader in Assembly/mod tools to read bitmaps, shading model, functions and tiling and mirror them in Blender (r93_sniper, 2022-04; sswampp 2021-11). Soulburnin held back a Reach Blender shader because it is intended to be used with Assembly and future official mod tools (soulburnin, 2022-08).
- Thinness params in Reach (hologram/thin shaders): pointdevice posted decompiled pixel shader asm (T1/T2 sample the same noise texture) as research; no conclusion given (2021-07).
- Reach holographic/additive lights have no alpha; they are additive shaders, so use a greyscale of the image as alpha or an Emission+Transparent mix (r93_sniper/gallium, 2021-07).
- H3 multiplayer armour CC: default armour CC map is mp_masterchief_cc; R = primary, G = secondary (B = tertiary on H3 armours) (sswampp, 2021-12).
- Colour-change texture import in Blender: set the _cc texture colour space to Non-Color; if a piece fails to recolour, set the CC image node Alpha mode to "Channel Packed" (soulburnin/chiefster_4014/chromacore, 2021-08 to 2021-10). Normal, roughness, blend maps must also be Non-Color (many people).
- Transparent/dithered CC bug: transparency (instead of black) between two CC stripes made Eevee and Cycles read different channels (keeeeeeeeen, 2022-05).
- Halo 3 detail maps: shader has a base map plus tiled detail and detail bump overlays; shrink the mapping node to ~0.1 for better detail (MGS Test App / killzone64, 2021-08).

## 3. Shaders, render states, HDRI/env quirks
- Magenta in Cycles = error colour from a bad environment texture (e.g. transparent film hiding it) (gallium, 2022-02); magenta in general = texture failed to read (2021-07). A cubemap input unsupported by Cycles also gives purple (r93_sniper, 2021-09).
- Reach visor: cubemap/HDRI reflection; to emulate in-game visor, supply an equirectangular map from a Reach cubemap (plastered_crab, 2022-07). Use a fresnel into Mix RGB for rim/visor tints (chromacore/fiycsf, 2022-06).
- Yellow textures everywhere in Reclaimer view: set Materials root path in Reclaimer import settings to the bitmap export root (the folder containing cinematics, fx, levels, multiplayer, objects, rasterizer, shaders) (gallium, 2021-05; soundflare, 2022-08 confirmed fix).
- Reach reclaimer limits: no vertex colours mapped for any H5 vertex formats, and in earlier games some formats pack colour into position W, which cannot be detected from tag data; water vertices missing on MCC Reach maps (gravemind2401, 2024-02, Halo 5 Research) [confirmed by author].
- Channel decoding rule of thumb for unknown control maps: if the colour map alpha holds specular, the RGB texture is a colour-control map; if alpha is roughness, the RGB is specular (gameoverload, 2022-06) [claimed].
- Halo 3 blue tint: most H3 armours and MP are blue-tinted; Bungie colour varied by level (chromacore, 2022-08).
- Terrain with several shader blends can blow out shiny in Chief's Blender terrain shader; use the lighter Bungie shader for trees/simple stuff (chiefster_4014/bombwolf, 2022-01).
- Reach decal files: Defender quarantined "Foxhole Decals"/Reach decals/Flash FX archives as false positives (bombwolf, 2022-01/05).

## 4. Importing / exporting / bsp / map porting (H3 toolchain, applies to Reach later)
- Reclaimer->bsp->back: exporting a bsp to AMF, editing in Blender, exporting ASS, then tool structure produced many errors (missing shaders, triangle errors) after 5 minutes (itzhouze, 2021-10). Typical causes: missing shader tags, degenerate/duplicate triangles and collision edges; scaling a map up can exaggerate them (extreme_narwha, 2021-10). Reach geometry is built differently from trilogy and gives "degenerate edges" in H3 Tool (kaboose1066, 2021-11).
- Shader compile trick: if map building hits shader compile trouble, open the scenario in Sapien so it compiles shaders, then rebuild (r93_sniper, 2021-10).
- Campaign->MP conversion attempt in H3 (itzhouze, 2021-10): changed map id to MP and game type to multiplayer, set zone set to one set, deleted Globals/scripting/source files/scripts, built with `tool build-cache-file levels\solo\010_jungle\010_jungle`; log still loaded global_scripts; game gave "Fatal Error". Sapien said "-ERROR- spawning: Failed to find a valid spawn location!" until a respawn_point, respawn zone and slayer spawn were added. Final crash unresolved in-slice. [unresolved]
- Sapien crash (MCC patch Apr 2022): assertion at render_methods/render_method_definitions.cpp #3203, "result != NONE", while loading riverworld (tafurious, 2022-04). No fix posted.
- Sapien sound: update added sound for H3EK/ODSTEK (yellowwolf1138, 2022-04) [claimed].
- H3 tool lightmap command asked (raccnoon, 2021-10), not answered in slice.
- Reach .map files contain every model used by that map (including Spartan/Elite) so extraction needs the right map (plastered_crab, 2021-04); main menu map (mainmenu.map) is absent in MCC Reach so Reclaimer cannot load some shared resources (saltyace, 2022-02) and errors like "Could not find file ...mainmenu.map" appear when loading render_models (2022-05).
- Mixed-up shader files: modifying .shader text by hand corrupted it in Guerilla (itzhouze, 2021-11); use the tool, not manual path edits.
- Halo 4 Light/Material importers and H5 importers read structure_lighting tags into Blender (brooen).
- GPU/Intel iGPU note: Intel iGPUs are weak; get Ryzen if no dedicated GPU (fiycsf, 2022-04).

## 5. Animation and rigs
- Bonobo errors: "Cache file is from an unsupported game build" opening Reach m52.map (zidiath, 2021-12; aflyingdorito, 2022-04). Bonobo works for H3/ODST but not for current MCC Reach builds. Reach first-person animations do not extract correctly with this approach (plastered_crab, 2021-08). Banshee and Guta animation extraction failed; Reach animations may be flagged as unsupported (plastered_crab, 2023-01, Extracting Hal).
- Bonobo reads Bungie-era .map; install .NET, put all .maps in one dir; reinstall maps from known good source if it will not open (plastered_crab, 2021-08).
- Applying a different character's animation to a second Elite/biped imports onto the first (rylietw, 2021-08); no fix given. Use separate skeletons/armatures with unique names.
- H3 first-person animation import fails or errors when stacking AR + arms (fesiug, 2021-08); the game handles FP via nodes (sswampp). Unresolved.
- Mixamo auto-recognises Reach Spartan and Elite rigs; elite is wonky but usable (koax117, 2022-06).
- Reach Spartan pack rigs: pelvis moving the entire model means IK rig/root issue; pack provides IK variant by XBXR (oodleberry, 2021-08). Knife handle weighted to spine; separate to unparent.
- Packs: Reach Spartans blend (Chief/Swagster) with armor toggles and primary/secondary colour nodes; elites pack "Reach Elites" with rig and shading. Pack download links on Google Drive (plastered_crab).
- Parenting weapons to rigs: bone-parent to the hand bone rather than object-parent (arionis question, 2022-03) [claimed].
- Mapping foreign rigs to H3 skeleton: map bones from original skeleton to H3 skeleton (r93_sniper, 2021-10).
- Reach energy sword/loop frame counts and spread angle of shotgun: unknown; official Reach mod tools were needed to read tag values (r93_sniper, 2022-06).

## 6. Ripping/versions and file formats
- Reclaimer errors seen: crash on save of OBJ when Architect not installed properly (xsoulstacex 2021-12); "Error loading model ... elite.render_model" needs the shared maps present; refusing to extract in build .295 (err_0r 2021-05); batch extract returns empty folder; Reach structure_bsp exports crash (yhormv 2022-03) and bsp viewed as a single pyramid (unordinal 2021-12). No definitive fixes except: update Reclaimer, set materials root, put maps in one folder.
- Extract bitmaps button sits next to export mesh in Reclaimer; bsp extraction button grabs most H3+ stuff (chromacore/sswampp, 2021-11).
- AMF imports with black/glossy materials on Reach props: need Reach shader node and tiff->dds/png alone does not fix (awsus 2022-04); working solution was a pre-textured prop pack.
- Vertex colour/multi-UV exporting from H5 maps via DAE was proposed (tacohombre/brooen, 2024-02).
- VMT/VTF are Source shader/image files (gameoverload 2022-08) [confirmed].
- Color space rules (deskclaw, 2022-05): base colour sRGB; everything else Non-Color; bake at 2k/4k in Non-Color.
- Baking: Cycles required; add an image texture node with a blank image selected on each material, bake Diffuse with only Color contribution enabled; baking needed a light to produce non-grey output in one case (zad_47, 2022-07).

## 7. Networking / anti-cheat / MCC runtime
- Offline testing: Reach/MCC on Xbox/Steam auth trick - disabling network and rolling back clock was suggested for auth-tied games (ember.cinder.a, 2021-07) [claimed, unrelated to EAC specifics].
- Mid-2022 MCC patch lowered frame rates in CE, Reach, 2A MP for some users (omgwtfun, 2022-04) [reports only].
- Infinite error "No ping to our datacenters detected" - no fix in thread (2022-01).

## 8. Misc useful behaviours
- Halo 4 compressed textures cause banding; add a Color Ramp (surasia, 2021-06).
- Colour ramp alpha output is white unless alpha is set on the stops (gallium, 2021-07).
- Alpha Hashed blend mode + diffuse alpha into shader Alpha for foliage/decals (plastered_crab, 2021-07).
- Appending Bungie Halo Shader group nodes: File > Append > NodeTree; double-click node top bar to see inside (xsoulstacex/theycallmespy, 2021-12). Blender will not accept opening a .blend for 3.0 pack unless Blender 3.0 is opened first, then File>Open (sierra060822, 2021-12).
- Unity port: textures must be baked out or shader math replicated; Halo 5 Unity shader links above (several).
- Draw-call optimisation: pack Reach Spartan down to ~3 materials for VRChat (fiycsf, 2021-11).

## Open questions raised but unanswered
- Exact slot order of the four Change Colors blocks vs albedo/spec/fresnel/environment tint (unordinal, 2021-11): partly answered (they map to CC channels) but the tint-override flag mechanism was not identified.
- How to fix "Fatal Error" when a campaign scenario is converted to an MP map in H3 (itzhouze, 2021-10).
- Why Bonobo rejects current MCC Reach maps ("unsupported game build") and whether any build supports it.
- Reach first-person and some creature animations (Banshee, Guta) failing to extract.
- Lightmap render command in H3 Tool (raccnoon, 2021-10).
- Whether _material face texture in Reach is a skin spec mask (gallium guess only).
- Automated bulk generation of filled .shader tags from Blender materials (Blader generates empty ones only).
- Sapien assertion render_method_definitions.cpp #3203 after the April 2022 MCC update.
