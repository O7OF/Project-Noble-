# 2026-11-12: HREK lacks many stock Elite self-illum bitmap tags; the build then substitutes a default bitmap and the
# surfaces glow faintly (green hex in the Armory). Create black (no-glow) bitmaps at exactly the missing paths.
# Added files are listed in T\fill_missing_illum_added.txt so they can be deleted to undo.
$H = 'D:\Steam\steamapps\common\HREK'
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$names = @(
 'objects\characters\elite\bitmaps\elite_base\elite_base_upper_illum',
 'objects\characters\elite\bitmaps\elite_chest\elite_minor_chest_illum',
 'objects\characters\elite\bitmaps\elite_chest\elite_spec_ops_chest_illum',
 'objects\characters\elite\bitmaps\elite_chest\elite_ultra_chest_illum',
 'objects\characters\elite\bitmaps\elite_gauntlet\elite_officer_gauntlet_illum',
 'objects\characters\elite\bitmaps\elite_gauntlet\elite_spec_ops_gauntlet_illum',
 'objects\characters\elite\bitmaps\elite_gauntlet\elite_ultra_gauntlet_illum',
 'objects\characters\elite\bitmaps\elite_helmet\elite_officer_helmet_illum',
 'objects\characters\elite\bitmaps\elite_helmet\elite_spec_ops_helmet_illum',
 'objects\characters\elite\bitmaps\elite_helmet\elite_ultra_helmet_illum',
 'objects\characters\elite\bitmaps\elite_thigh\elite_minor_thigh_illum')
Add-Type -AssemblyName System.Drawing
Set-Location $H
foreach ($n in $names) {
  if (Test-Path "$H\tags\$n.bitmap") { "exists $n"; continue }
  $tif = "$H\data\$n.tif"
  New-Item -ItemType Directory -Force (Split-Path $tif) | Out-Null
  $b = New-Object Drawing.Bitmap 64,64; $g = [Drawing.Graphics]::FromImage($b); $g.Clear([Drawing.Color]::Black); $g.Dispose()
  $b.Save($tif, [Drawing.Imaging.ImageFormat]::Tiff); $b.Dispose()
  & .\tool.exe reimport-bitmaps-single $n *> "$env:TEMP\fmi.txt"
  $ok = Test-Path "$H\tags\$n.bitmap"
  "$n -> exit $LASTEXITCODE tag=$ok"
  if ($ok) { Add-Content "$T\fill_missing_illum_added.txt" "$H\tags\$n.bitmap`r`n$tif" }
}
