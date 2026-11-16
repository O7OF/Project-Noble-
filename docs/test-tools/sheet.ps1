param([string]$Pattern, [string]$Out, [string]$Crop = '0.55,0.12,0.40,0.80', [int]$TileW = 300)
# Contact sheet of JPG/PNG captures (crop = x,y,w,h as fractions), labelled with file names.
Add-Type -AssemblyName System.Drawing
$Dir = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$files = Get-ChildItem $Dir -Filter $Pattern | Sort-Object Name
$c = $Crop -split ',' | ForEach-Object { [double]$_ }
$first = [Drawing.Bitmap]::FromFile($files[0].FullName)
$cw = [int]($first.Width * $c[2]); $ch = [int]($first.Height * $c[3]); $first.Dispose()
$th = [int]($TileW * $ch / $cw)
$cols = [Math]::Min(6, $files.Count); $rows = [Math]::Ceiling($files.Count / $cols)
$sheet = New-Object Drawing.Bitmap ($cols * $TileW), ($rows * $th)
$g = [Drawing.Graphics]::FromImage($sheet); $g.Clear([Drawing.Color]::Black)
$font = New-Object Drawing.Font 'Arial', 11
for ($i = 0; $i -lt $files.Count; $i++) {
  $b = [Drawing.Bitmap]::FromFile($files[$i].FullName)
  $src = New-Object Drawing.Rectangle ([int]($b.Width * $c[0])), ([int]($b.Height * $c[1])), ([int]($b.Width * $c[2])), ([int]($b.Height * $c[3]))
  $dst = New-Object Drawing.Rectangle (($i % $cols) * $TileW), ([Math]::Floor($i / $cols) * $th), $TileW, $th
  $g.DrawImage($b, $dst, $src, [Drawing.GraphicsUnit]::Pixel)
  $g.DrawString($files[$i].BaseName, $font, [Drawing.Brushes]::Yellow, $dst.X + 3, $dst.Y + 3)
  $b.Dispose()
}
$sheet.Save((Join-Path $Dir $Out), [Drawing.Imaging.ImageFormat]::Png); $g.Dispose(); $sheet.Dispose()
"sheet: $Out ($($files.Count) images)"
