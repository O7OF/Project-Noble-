# Elite originals vs installed - full compare 2026-11-12 ~09:25 ET
(Copy of claude.ai project doc.)

Originals: D:\Project-Noble-ORIGINAL\data\menumaps\objects\characters\elite\bitmaps (164 PNGs). Script: tools\cmp_orig.ps1 -> cmp_orig.csv (compares each PNG with HREK data tiff and ph-export tif of the same name: size, per-channel means, alpha, mean abs diff).

## Results
- 138 ph-export textures: ALL identical to the originals.
- 137 HREK data tiffs: same sizes; all diffuse maps ~0.03 brighter than the originals; cc/illum ~0.01 different. Small uniform offset, not a bug.
- Real differences:
  - ch_elite_armor_02_nm / ch_elite_honor_armor_01_nm: our Z (blue) rebuilt, originals B=0 (earlier fix; ours is fine).
  - mp_newelite_diffuse (Combat): spec alpha original 0.466, ours 0.28 (lowered on purpose).
  - cov_hex_detail_grey: ours 0.57 vs original 0.53 (minor).
- 25 originals with NO installed counterpart: Honor Guard / armor_02 / Council / Ossoona rr files, the Ranger space helmet set (no ranger tags exist in HREK tags at all), and rr3 Spec Ops / Ultra chest, helmet, shoulder diffuse+cc.
- rr3 Ultra pieces are much darker than stock (chest 0.124 vs 0.274, helmet 0.134 vs 0.257, shoulder 0.150 vs 0.312) with their own cc masks. Likely the intended dark Reach Ultra/Spec Ops look.

## Not done / options
- Install rr3 Spec Ops/Ultra pieces (swap diffuse+cc for those shaders).
- Ranger space helmet: no tag exists; would need new bitmap+shader+render model part.
