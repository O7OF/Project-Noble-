# Notes p2 chunk_00 (additions/corrections only; second full read of 1076 lines)

Channel is #general-help unless stated. Slice has NO direct text on Reach tool.exe/Sapien armour import, permutations, or networking of custom pieces; leads below are indirect.

## Missed facts

### CC / change-colors / control maps / spec / fresnel
- Starting belief stated by unordinal (2021-11): the four "Change Colors" blocks in a Reach biped tag map to albedo tint, specular tint, fresnel tint, environment tint (order forgotten). Two days later the same person observed slots 0-3 behave as primary/secondary/tertiary/quaternary (CC channels) AND sometimes also drive spec/fresnel/env tints; he noted the base shader's spec tint is blacked out and that Elite armour relies on those slots for spec and fresnel. So the real question (how the game decides colour-vs-tint) stayed open; the docs shader-input list does not cover per-biped overrides. (unordinal, 2021-11, general-help)
- Elite SpecOps: green fresnel equals the colour in tag slot 2 although the Elite CC map has no blue; Ranger elite: orange fresnel equals tertiary slot, while its self-illum colour comes from the illum_cards shader itself. Soulburnin first guessed emissive colour rather than fresnel, then pointed at greenish cubemap tint on SpecOps pieces. Nobody verified in game. (unordinal/soulburnin, 2021-11)
- Soulburnin: spec tint mix semantics: 0 = spec tint only, 1 = albedo-tinted spec; fresnel curve value 0 = fresnel colour fully overrides spec tint; elite globals spec tint D6D4DB, fresnel E8C6A4. This is the most concrete Elite-shine recipe in the slice.
- Brightness matters for shine in the Blender Reach pack: "brighten your primary colour, darker values won't shine" (chromacore, 2021-06). Related: sunliteflame could not get darker red from the wheel; fix was dragging the value slider toward black (plastered_crab, 2022-06).
- Reach pack shader (Chief's "Halo Reach Shader") complaints: everything glows too bright (bombwolf, 2022-01); chiefster advice: set the _blend texture to Non-Color, do not wire normal maps into the detail Strength sliders, and use the newer version that has detail intensity sliders. Chief's terrain shader mixing several blends produces glare/insane shine (bombwolf, 2022-01).
- Normal/shine hygiene list: flat shading or normal not Non-Color gives wrong highlights (soulburnin, 2021-08); also enable shade smooth, auto smooth/weighted normals, make sure normals are wired in the shader, texture folder beside the .blend (gameoverload, 2022-06); visor ugliness fixed by Non-Color on visor normal (silent1meow/gameoverload, 2022-08); visor = glass+glossy mix with the normal into both, glass colour tints the visor (gallium, 2022-01); a black visor often just means nothing to reflect, world strength/HDRI settings matter (incuriouslog/unordinal, 2022-06/07).
- Bungie Halo Shader (H3) looked overly reflective to a user; spec maps barely changed the result (xsoulstacex, 2021-12). Chief's H3 shader: plug the CC map in; diffuse alpha also goes to "diffuse alpha" input (sswampp, 2021-12).
- Colour-zone failures seen on Reach/H3 pieces (each fixed or not): Reach collar attachment would not recolour; advice was to change the white colour under the blue (mechacommando/williaso, 2021-05). Grenadier chest: set CC image Alpha to Channel Packed (soulburnin, 2021-08). Reach Sniper shoulder CC map reported bad/missing, fixed maps requested twice (.bluephoenix, 2021-12). Season-7 Elite armour CC maps are blank (aqua_himura, 2021-10). A Reach shoulder whose zone 5 turned blown-out white; cause was wrong colour slot plus colour space not Non-Color (etheox, 2022-07/08). Kneepad pack had two stacked shaders and edits hit the hidden one (koax117, 2022-06). Crotch piece/secondary colour stuck white and third colour white showing were asked without clear answers (2021-07, 2022-03, 2022-06).
- Source/SFM Reach port detail: use two colour attributes, but $phongtint and $envmaptint rather than $color (soulburnin, 2021-05). Brute armour in SFM stays chrome (kindledone, 2021-09, unresolved).
- To recolour a single piece that shares a material, duplicate the material and edit the copy (unordinal, 2021-12).

### Ripping / tools / versions
- Soulburnin warned not to rip Reach armour from the main menu / customisation screen; those models are broken in many ways (2021-07). Relevant to anyone trying to pull a custom Spartan/Elite from the menu.
- Reach MP .map location on Steam and Xbox-maps source already covered; extra: HW/H2A campaign maps do not work but H2A multiplayer maps do (plastered_crab, 2021-07); for Reach levels people used m05, m20 (2021-07); Reclaimer build .295 was the one reported refusing to extract (2021-05).
- Bonobo errors also reported when exporting H3 animations from maps other than the main menu map (aqua_himura, 2021-08).
- Assembly-side hint: open the .map that contains the skybox and look for the "rmsh - shader" entry in the sidebar to find a shader's textures (sswampp, 2021-10).
- "Don't use manual path edits": bluephoenix hit overwritten visor/techsuit shaders on save in H3 tool; no answer (2021-10).
- Brooen's 2021-06 video "How to Mod Halo #1" (youtu.be/UzZk5Wi2FQc) covers basic Reach MCC weapon tag editing in Assembly (AR firing rockets). Only Reach tag-edit tutorial in slice.
- Assembly texture import quirk in Reach: importing two different bitmaps one after another duplicated the first one (the_heavynator, 2021-08, unresolved).
- Reclaimer vertex-colour limitation also applies "to all games" (gravemind2401, 2024-02, Halo 5 Research).
- Brooen's H5 importer auto-imports control/normal from the colour map name; gravemind2401 says the 2024 MediaFire Reclaimer build only handled H3 and Reach so far (2024-02).

### Importing custom models (indirect)
- Several people could not import/export custom Reach Spartans: exported model becomes a white blob (therealafar, 2021-07), colour white plus odd pose (nooncall, 2022-01); someone repeatedly "messing up" importing a custom model into Reach and asked for a volunteer (Deleted User, 2021-10); a user wanted to follow "Reject's model importing video tutorial" to put a custom shotgun into Reach (nobleofdeath16, 2021-05). No reply gave the actual steps.
- Re-attaching a custom Reach Spartan to the Reach skeleton ripped by Reclaimer to reuse animations was asked (defultimate12k, 2022-04), unanswered. Map a foreign rig onto the H3 skeleton instead (r93_sniper/sswampp, 2021-10).
- H3 pipeline hint: sswampp points to General-101's Blender toolset (JMS/ASS) and says a rig already matching the H3 Spartan can skip re-mapping; a textured-cube import to H3 asked, no answer (2021-10).
- H3 tool facts: Sapien compiles shaders on open; tweak tiling in H3EK like in Blender (sswampp, 2021-10); lighting questions for H3EK indoor shadows unanswered (hamporker, 2022-08).

### Networking / multiplayer
- Nothing about custom armour replication, anti-cheat, EAC, or modded-lobby behaviour. Only tangents: Reach customs tweaks asked (no weapons on spawn, Forge object physics) with no answer (2021-07/08); someone said Assembly edits can "ruin multiplayer matches" in Halo 2 (peestain37, 2021-12); netcode wish (masterz1337, 2022-05).

## Corrections to pass 1
- Section 4 attributes causes of tool structure errors to extreme_narwha and kaboose1066; both only asked. The reply that Reach maps are built differently was from a Deleted User (2021-11). Scaling-up as a cause is the asker's guess.
- Section 2 "Resolution reached": overstated. Unordinal said "mostly makes sense" and still planned in-game verification; fresnel-vs-tertiary link remains unproven. Also Ranger elite had an orange fresnel matching tertiary; the illum_cards finding concerns the self-illum colour, not the fresnel.
- Section 2 H3: sswampp only gave R=primary and G=secondary. "B = tertiary" came from a different, later remark that tertiary is matched on H3 armours (Deleted User, 2022-06) and is not confirmed for mp_masterchief_cc.
- Section 2 HLSL note: unordinal's wording is ambiguous ("misread that it subtracted albedo from spec tint"); treat as unsettled which way the subtraction goes.
- Section 1 line on Adjutant/Reclaimer: plastered_crab's 2021-11 claim (Reclaimer = CE-ODST, Reach needs Adjutant 4.2.0) is contradicted by many 2021 users ripping Reach in Reclaimer (2021-05, 2021-07, 2021-12) and by gravemind2401's 2024 statement that his build handles Reach.
- Section 7 offline/clock-rollback tip (ember.cinder.a, 2021-07) is about an unnamed auth-tied tool, not MCC/EAC; do not use for Reach multiplayer.
- Section 6 rule "alpha=specular means RGB is colour control" (gameoverload, 2022-06) came in a thread that appears to be about a non-Halo game (ARM/NMA/ILM talk, later Aliens Fireteam Elite); not Reach evidence.
- Section 2 spec shortcut: fiycsf's "specular in the alpha of the colour texture" refers to Reclaimer-exported colour/spec textures, not the multipurpose control map whose spec channel is R (Xbox) or B (PC).
- Section 4 mainmenu.map: saltyace (2022-02) only noted MCC lacks it and he was told he did not need it; the error consequence appears in 2022-05.

## Contradictions between speakers
- Channel order of multipurpose map: Xbox R spec/G illum/B CC/A aux versus PC R aux/G illum/B spec/A CC (r93_sniper). Treat as format-dependent; MCC rips are PC order.
- Which tint carries Elite fresnel colour: soulburnin globals say fresnel E8C6A4 with curve 0 overriding spec tint; unordinal sees per-variant colours in slot 2 matching fresnel; soulburnin suggested emissive; unresolved.
- Reclaimer supports Reach or not (see above).
- fiycsf: Reach has no metal/rough, spec only; chromacore: Reach shading is technically more complex than Infinite; gallium doubts that (2021-12). Different meanings of "complex".
- Colour-space: sswampp argued a normal map set Non-Color is fine but diffuse must stay sRGB (2021-11); deskclaw agrees (base colour sRGB, rest Non-Color); the CC mask is Non-Color per chiefster.

## Best leads for these problems
1. Black / too-dark Honor Guard shoulder:
   - Check CC image node colour space = Non-Color and Alpha = Channel Packed (soulburnin 2021-08; chiefster 2021-09/10); blank or broken CC map for that piece (Sniper shoulder, S7 Elite maps reported); wrong colour slot/zone plus wrong colour space gave white blowout (etheox 2022-08).
   - In tags: specular tint is zero in the base shader, so biped Change Colors slots for spec/fresnel/env may be black or unset for a custom piece; compare with a working Elite piece slots 0-3 (unordinal 2021-11). Mixed spec = lerp between spec tint and CC-coloured albedo (soulburnin), so a black primary also kills spec.
   - Black can also be unset fresnel/cubemap tint or an alpha channel interpreted as mask; transparency vs black between mask regions changes channel read (keeeeeeeeen 2022-05).
2. Elite shine wrong: reproduce soulburnin's chain (diffuse*detail -> CC colour -> lerp with spec tint; fresnel curve 0 means fresnel colour wins); use globals D6D4DB / E8C6A4; colours in tags are linear; make primary bright; normals Non-Color and smooth; avoid stacking terrain-style bump mixes; cubemap tint on SpecOps is greenish.
3. Importing custom armour into Reach: slice has only pointers: Assembly texture import quirk (second bitmap duplicates first), Foundry (Blender add-on for Reach, with 2024 tag import) and General-101 toolset for H3; official Reach mod tools not released as of 2022 so values come via Assembly/TagTool; the better source is the project field guide docs.
4. Networking of custom pieces: no evidence in this slice; treat as unanswered.
