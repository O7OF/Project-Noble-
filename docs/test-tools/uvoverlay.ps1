# Draw UV polygons from uv_dump.json over a preview image. -Objects / -Mat filter. Output T\view\<Out>.png
param([string]$Base, [string]$Out, [string[]]$Objects, [string]$Mat = 'default_sh_elite_honor_armor_01', [switch]$Mask, [switch]$NoFlip)
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
Add-Type -AssemblyName System.Drawing
$j = Get-Content "$T\uv_dump.json" -Raw | ConvertFrom-Json
$img = [System.Drawing.Bitmap]::new($Base)
$S = $img.Width
if ($Mask) { $img = [System.Drawing.Bitmap]::new($S, $S) }
$g = [System.Drawing.Graphics]::FromImage($img)
if ($Mask) { $g.Clear([System.Drawing.Color]::Black) }
$cols = @([System.Drawing.Color]::FromArgb(110,255,0,0), [System.Drawing.Color]::FromArgb(110,0,255,0), [System.Drawing.Color]::FromArgb(110,0,128,255), [System.Drawing.Color]::FromArgb(110,255,255,0))
$i = 0
foreach ($o in $Objects) {
  $polys = $j.$o.$Mat
  $c = if ($Mask) { [System.Drawing.Color]::White } else { $cols[$i % 4] }
  $br = [System.Drawing.SolidBrush]::new($c)
  $n = 0
  foreach ($p in $polys) {
    $pts = for ($k = 0; $k -lt $p.Count; $k += 2) { [System.Drawing.PointF]::new([float]($p[$k] * $S), [float]($(if ($NoFlip) { $p[$k+1] } else { 1 - $p[$k+1] }) * $S)) }
    $g.FillPolygon($br, [System.Drawing.PointF[]]$pts); $n++
  }
  "$o : $n polys colour $i"
  $i++
}
$g.Dispose()
$img.Save("$T\view\$Out.png", [System.Drawing.Imaging.ImageFormat]::Png)
