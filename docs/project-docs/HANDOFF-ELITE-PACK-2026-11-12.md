# Elite pack handoff - update 2026-11-12 ~02:20 ET
(Copy of claude.ai project doc.) Menu/Armory verified only at the time; the match pack was rebuilt later (and shrunk to 286 MB on 2026-11-14).

## ROOT CAUSE of "PH pieces look grey/white instead of taking player colour"
Stock Reach diffuse bitmaps are imported with curve 3:linear (curve mode 0 "choose best", no usage override). The PH (Prophet's Hand) diffuse bitmaps (made through Foundry/tool) had curve mode 2 "force PRETTY" + a usage override = sRGB (gamma 2.2), so they rendered with the wrong gamma. Darkening them toward "stock 0.2 mean" was a wrong fix (reverted).
FIX = T\tools\bm_curve.py (BITMAPS=a|b, CURVE=n env; default sets curve mode 0 and clears the usage override), then `tool reimport-bitmaps-single` for each, then restore the ORIGINAL texture pixels (ph_orig backup).
Applied (menu verified): mp_newelite_diffuse (Combat shoulders), ch_helmet_councilor_01_diff, elite_ossoona_helmet_armor_diffuse_4k, ch_elite_armor_02, ch_elite_honor_armor_01 + _grey + _em. Combat shoulders, Council helmet and Honor Guard armour now take primary/secondary colour like stock Minor pieces.
Proof tests: swapping the Combat shader to the stock Minor shoulder texture+cc made it vivid; stock base + PH cc / PH base + stock cc showed the base texture was the culprit. Stock minor textures exported to T\stock_bm (tool export-bitmap-tga takes an output PREFIX, files end _00_00.tga).

## Still wrong then
1. Honor Guard glow: after the curve fix the lit panels showed the SECONDARY colour and the blue glow wasn't visible. Illum shaders have self_illum_color (0,17,110) (glow_fix2.py).
2. Ossoona helmet pale silver (not a brightness problem; compare shader/cc/normal with Council).
3. Generated cc masks put secondary on recesses; stock Minor chest/thigh/shoulder have no secondary area.
4. Rebuild the match pack and run match_f5.ps1.
5. Tyler's request: player-customisable glow colour for Honor Guard / General. Option 1: drive self_illum_color from a player change colour in the shader. Option 2: Armory slider + profile value + object function (needs UI + network sync).
6. Trim pack (done 2026-11-14), networking of other players' pieces, ReachArmory.cpp biped index (0x7E), damage info.

## Tools (T\tools)
- darken_ph.ps1: GDI+ texture processor with ph_orig backups (HREK\noble_mods\elite_noble_backup_before_foundry\ph_orig). -Report, -Apply, -Raw, -Only, -NoLut, -NoReimport. Never DrawImage (zeroes RGB under alpha 0).
- bm_curve.py, maskstats.py, lutmatch.py, shader_vals.py, cmp_px*, ccstats.ps1.
- PowerShell gotcha: $t and $T are the same variable.
- runpy.ps1 only waits for the file named by -Out.
