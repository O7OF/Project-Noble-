# Elite original ("rr") textures installed - 2026-11-12 ~09:00 ET
(Copy of claude.ai project doc.)

Source: C:\Users\tyler\Pictures\halo (original Reach texture set). Names map 1:1 to our tags.

## Findings vs what we had
- HG glow map (_em_rr_gray) == ours (corr 0.995).
- HG albedo: reference dark parts ~0.067 (ours ~0.003 = pure black), spec alpha 0.25 there; plates ours 0.82 vs ref 0.65.
- HG cc: reference = red-only mask on the colourable plates (~18% of UV). Ours was a derived red/green mask over most of the UV.
- Council helmet: reference albedo mean 0.15 (ours 0.52, the pale shine); ref alpha 0.68.
- armor_02: ref alpha 0.37 (we had 0.08 for the pelvis hot spot).

## Installed (tools\apply_rr.ps1, backups in HREK\noble_mods\elite_noble_backup_before_foundry\before_rr)
- ch_elite_honor_armor_01_grey <- rr3_diff, ch_elite_honor_armor_01_cc <- rr3_cc, ch_elite_honor_armor_01_em <- em_rr_gray
- ch_elite_armor_02 <- rr_diff with alpha scaled to 0.10, ch_elite_armor_02_cc <- rr_cc
- ch_helmet_councilor_01_diff <- rr_diff with alpha 0.28, _diff_cc <- rr_cc
- Reimported (tool.exe reimport-bitmaps-single), menu map rebuilt.
- WARNING: darken_ph.ps1 would overwrite these if re-run.

## Result (rr1_sheet.png)
HG: dark armour with red glowing fins (colour follows player/Lights colour). Council helmet crest still bright silver.
Also: Lights colour row added to the Elite Armory (ArmorCustomizer.cpp).
