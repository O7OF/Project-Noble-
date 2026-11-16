# Crop a capture at full resolution: crop.ps1 -In x.jpg -Out y.png -Crop 'x,y,w,h' (fractions) [-Scale 2]
param([string]$In, [string]$Out, [string]$Crop = '0.72,0.18,0.18,0.30', [double]$Scale = 1)
Add-Type -AssemblyName System.Drawing
$Dir = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$c = $Crop -split ',' | ForEach-Object { [double]$_ }
$b = [Drawing.Bitmap]::FromFile("$Dir\$In")
$x = [int]($b.Width*$c[0]); $y = [int]($b.Height*$c[1]); $w = [int]($b.Width*$c[2]); $h = [int]($b.Height*$c[3])
$o = New-Object Drawing.Bitmap ([int]($w*$Scale)), ([int]($h*$Scale))
$g = [Drawing.Graphics]::FromImage($o); $g.InterpolationMode = 'HighQualityBicubic'
$g.DrawImage($b, (New-Object Drawing.Rectangle 0,0,$o.Width,$o.Height), (New-Object Drawing.Rectangle $x,$y,$w,$h), 'Pixel')
$o.Save("$Dir\$Out", [Drawing.Imaging.ImageFormat]::Png); $g.Dispose(); $o.Dispose(); $b.Dispose()
"saved $Out $($o.Width)x$($o.Height) from $($b.Width)x$($b.Height)"
