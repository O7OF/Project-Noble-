# Handoff: Elite Honor Guard shoulder wing + remaining Elite armor work (2026-11-12, 14:16 ET)
(Copy of claude.ai project doc.)

Read first: HANDOFF-ELITE-SHINE, HANDOFF-ELITE-RR-TEXTURES, HANDOFF-ELITE-PACK, HANDOFF-ELITE-CAMERA, HANDOFF-ARMORY,
FIX-SPARTAN-PIECES-PMCG-BOM, plus the guide parts (..\discord-compendium\00_ULTIMATE_GUIDE*).

## Task then
Fix the Honor Guard (HG) shoulder wing that renders black in the Armory. Then: Elite shine (Council/Ossoona), verify the
Lights row in game, rebuild the match pack ONCE after the shoulder fix, then Council helmet silver crest, HG helmet
silver, Spartan knife, trim the ~500 MB pack, networking of other players' Elite pieces.

## Findings
- Shader template facts: Elite non-metal template `elite_armor_template` options "10 2 0 1 1 2 0 0 0 0 0 -1"
  (albedo = four_change_color_applying_to_specular, detail bump, specular_mask_from_diffuse, cook_torrance,
  environment_mapping = dynamic). `elite_armor_template_metal` = "0 2 0 1 1 2 ..." (albedo default, no colour change).
  Templates exist for albedo options 0,1,3,6,7,8,10 only (no 4). Shaders store options -1 (inherit) plus a postprocess
  block whose "shader template" reference points at the compiled template. Spec mask = diffuse alpha. Illum shaders use
  cc_full_primary so glow follows player colour.
- HG and the stock Ultra use the same template with the same albedo (10) and env mapping (dynamic): NOT a shader-option mismatch.
- The wing panels are in material 80's UV (right half of `ch_elite_honor_armor_01_grey`): dark diffuse (~0.067), cc red
  channel empty there, spec alpha white. Mesh 22 (parts 79/80/100) for right_shoulder/honor_guard.
- The emissive map `ch_elite_honor_armor_01_em` has a bright cell pattern over ALL wing panels: the wing is meant to be the
  glowing fin; in game it shows black with a red glow diamond.
- Experiment: `default_sh_elite_honor_armor_01.shader` options[0] 10 -> 3 with template `_3_2_0_1_1_2_0_0_0_0_0_0`: wing
  unchanged. So "CC applying to specular" is not the cause. Backup in `HREK\noble_mods\elite_noble_backup_before_foundry\before_albedo3\`.
- Next test planned: give the wing islands colour-mask coverage (red channel of `ch_elite_honor_armor_01_cc`) and lift the
  wing diffuse (`T\tools\apply_rr.ps1`, lifted sources in `Pictures\halo\_work\`), rebuild, capture, compare.

## Tools (T = `D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11`)
`T\tools\`: `rm_all.py` (region/permutation -> mesh -> part material shader names for elite_noble.render_model),
`texview.ps1` (rgb/a/r/g/b previews into `T\view\`), `set_shader_option.py` (env SHADER, CAT, VAL, TEMPLATE, DRY=1),
`apply_rr.ps1`, `sheet.ps1` (contact sheets; `-Crop '0.66,0.03,0.34,0.95'`), `run-quiet.ps1` (muted Armory capture on the
second monitor), `build_map.ps1`, `runmb.ps1`, `tagwalk.py`, `mbstart.py`, `wrap.py`.
Menu map rebuild and capture: `build_map.ps1` then `run-quiet.ps1 -Name <n>`.
Match pack (old recipe; superseded by the noble_pack scenario on 2026-11-14): `build_map.ps1 -Scenario levels\multi\20_sword_slayer\20_sword_slayer`,
copy `HREK\maps\20_sword_slayer.map` to `data\armor\noble_elite_pack.map`, then
`runmb.ps1 -Script tagwalk.py -Out ... -Env "MODE=build;A64=8;REC=1;PACK=<pack>" -Wait 1200`.

## Reminders for Tyler (from then)
- Change the Discord password; delete run.log / run2.log.
