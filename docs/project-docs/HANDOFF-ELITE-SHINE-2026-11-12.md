# Elite shine / Ossoona pale-silver - findings 2026-11-12 (updated ~08:00 ET)
(Copy of claude.ai project doc.)

## Root cause found
Shader parameters of Ossoona, Council and Combat are identical (dumped with shader_vals.py), so the shine was in the bitmaps:
1. elite_ossoona_helmet_armor_normal_2k.bitmap had Usage = "Diffuse Map" (a normal map imported as colour -> wrong shading/very shiny, pale). Set Usage to 19 "Normal Map (aka zbump)" (tools\bm_usage.py with SET_USAGE=19 SET_ONLY=<path>), reimported.
2. All PH normal maps (ossoona normal_2k, ch_helmet_councilor_01_nm, newelite_zbump, ch_elite_armor_02_nm, ch_elite_honor_armor_01_nm) had curve mode "force PRETTY" + usage override; stock normals use "choose best". Reset with bm_curve.py (CURVE=0) and reimported. newelite_zbump usage (18 ZBrush Bump) already matches stock elite_minor_chest_zbump.
3. Specular mask (diffuse alpha): Council 0.68, Combat 0.47, armor_02 0.37 vs stock ~0.25-0.32. Lowered to 0.28 mean with `darken_ph.ps1 -Apply -AlphaOnly -Only ...`.
Result in the Armory (T\sh2_sheet.png): Ossoona shows red secondary, much less white; Council helmet less blown out.

## 06:40: Honor Guard normal maps had NO Z channel (fixed, but NOT the cause of the hot spot)
ch_elite_armor_02_nm and ch_elite_honor_armor_01_nm had Blue = 0.00. FIX: `fix_nm_z.ps1 -Apply -Only ch_elite_armor_02_nm,ch_elite_honor_armor_01_nm` rebuilt Z (B mean 0.97). Originals: HREK\noble_mods\elite_noble_backup_before_foundry\ph_orig\<name>_nm_orig.tif. fix_nm_z.ps1 only rewrites maps whose B mean < 0.5.

## 07:15-07:25: WHITE PELVIS HOT SPOT = specular mask (alpha) of ch_elite_armor_02 (FIXED)
- Swapping only default_ch_elite_armor_02_honor's base_map to stock removed the spot -> culprit = ch_elite_armor_02 (material 79). (tools\set_shader_bitmap.py, env SHADER/PARAM/BITMAP.)
- ch_elite_armor_02 alpha: mean 0.25 but ~27% of pixels in 160-191. Scaled to mean 0.08 (`$env:ALPHA_AT='0.08'; $env:T_LUTDIR=$T; darken_ph.ps1 -Apply -AlphaOnly -Only 'ch_elite_armor_02'`) -> spot gone. (Later the rr texture install set it to 0.10.)

## 07:30-07:50: HONOR GUARD GLOW = works, follows the PLAYER'S COLOUR
- Glow maps (ch_elite_honor_armor_01_em, ch_elite_armor_emissive) were Usage 0 Diffuse / curve mode 2; stock illum maps are Usage 5 Self-Illum Map, curve 0. Fixed (bm_curve.py CURVE=0 + bm_usage.py SET_USAGE=5). tools\bm_props.py = read-only bitmap import settings.
- Illum shaders structurally identical to stock elite_illum_cards.
- Blue glow colour (0,0.29,0.682) x red player colour = black (invisible). Set to stock General gold/orange c0 (0.918,0.373,0) c1 (0.996,0.529,0.012) via tools\set_illum_color.py -> panels appear. Pure WHITE glow test: panels still RED -> glow is multiplied by the PLAYER'S colour. So fin lighting IS player-changeable via normal colour picks.
- Tyler's reference pictures (Halo 2/3/Reach Ultra/Honor Guard, gold fins on dark red armour) = target look.

## Rebuild recipes (at that time)
- Match pack: build_map.ps1 -Scenario levels\multi\20_sword_slayer\20_sword_slayer -> copy HREK\maps\20_sword_slayer.map to data\armor\noble_elite_pack.map -> runmb.ps1 -Script tagwalk.py -Env "MODE=build;A64=8;REC=1;PACK=<pack>" -Wait 1200. (Superseded 2026-11-14 by the noble_pack scenario.)
- Menu map: build_map.ps1 -Scenario levels\ui\mainmenu\mainmenu (~50 s), copy to data\menumaps\noble_menu.map.
- Armory capture: run-quiet.ps1 -Name <n> -Species elite -Cycle elitepieces -Delay 8000 -At '19,27,43' -Quit 46, then sheet.ps1.

## Render model facts (Honor Guard)
elite_noble: 102 materials; 79 = default_ch_elite_armor_02_honor, 80 = default_sh_elite_honor_armor_01 (body), 100 = default_ch_elite_honor_armor_01_illum, 101 = honor_lights_ch_elite_armor_emissive. Tools: rm_mats.py, rm_parts_use.py, shader_vals.py, shader_opts.py, set_shader_bitmap.py, set_illum_color.py, bm_props.py.
