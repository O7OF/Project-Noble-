# Layer-identification test (2026-11-12): point the base_map of stock Elite base shaders at solid test colours.
# -Apply: back up shaders, make test bitmaps, repoint.  -Restore: put the backed-up shaders back.
param([switch]$Apply, [switch]$Restore)
$H = 'D:\Steam\steamapps\common\HREK'
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry\before_layertest"
$SD = "$H\tags\objects\characters\elite\shaders\elite_base"
$map = [ordered]@{ body = 'ltest_magenta'; elite_base_upper = 'ltest_yellow'; elite_base_lower = 'ltest_white'; elite_base_lower_metal = 'ltest_white' }
$col = @{ ltest_magenta = @(255,0,255); ltest_yellow = @(255,255,0); ltest_white = @(255,255,255) }
if ($Restore) { foreach ($s in $map.Keys) { Copy-Item "$Bk\$s.shader" "$SD\$s.shader" -Force; "restored $s" }; return }
New-Item -ItemType Directory -Force $Bk | Out-Null
foreach ($s in $map.Keys) { if (-not (Test-Path "$Bk\$s.shader")) { Copy-Item "$SD\$s.shader" "$Bk\$s.shader" } }
Add-Type -AssemblyName System.Drawing
$BD = "$H\data\objects\characters\elite_noble\bitmaps"
Set-Location $H
foreach ($n in $col.Keys) {
  $c = $col[$n]; $b = New-Object Drawing.Bitmap 64,64
  $g = [Drawing.Graphics]::FromImage($b); $g.Clear([Drawing.Color]::FromArgb(255,$c[0],$c[1],$c[2])); $g.Dispose()
  $b.Save("$BD\$n.tif", [Drawing.Imaging.ImageFormat]::Tiff); $b.Dispose()
  & .\tool.exe reimport-bitmaps-single "objects\characters\elite_noble\bitmaps\$n" *> "$env:TEMP\lt_$n.txt"
  "$n import exit $LASTEXITCODE tag=" + (Test-Path "$H\tags\objects\characters\elite_noble\bitmaps\$n.bitmap")
}
foreach ($s in $map.Keys) {
  & "$T\tools\runmb.ps1" -Script set_shader_bitmap.py -Out "lt_$s.txt" -Env "SHADER=objects\characters\elite\shaders\elite_base\$s;PARAM=base_map;BITMAP=objects\characters\elite_noble\bitmaps\$($map[$s])" -Wait 120 | Select-String 'after|SAVED|ERROR|not found' | ForEach-Object { "$s : " + $_.Line }
}
