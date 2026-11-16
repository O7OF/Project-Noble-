# Re-save Blender-written TIFFs as plain 32bpp uncompressed TIFF (GDI+) and reimport them with tool.
$H = 'D:\Steam\steamapps\common\HREK'
$D = "$H\data\objects\characters\elite_noble\bitmaps"
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry\blender_tifs"
$names = 'cc_full_primary','ch_elite_armor_02_cc','ch_elite_honor_armor_01_cc','ch_elite_honor_armor_01_grey','ch_helmet_councilor_01_diff_cc','elite_ossoona_helmet_armor_diffuse_4k_cc'
Add-Type -AssemblyName System.Drawing
New-Item -ItemType Directory -Force $Bk | Out-Null
$codec = [Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() | Where-Object { $_.MimeType -eq 'image/tiff' }
$ep = New-Object Drawing.Imaging.EncoderParameters 1
$ep.Param[0] = New-Object Drawing.Imaging.EncoderParameter ([Drawing.Imaging.Encoder]::Compression), ([long][Drawing.Imaging.EncoderValue]::CompressionNone)
foreach ($n in $names) {
    $src = "$D\$n.tif"
    if (-not (Test-Path "$Bk\$n.tif")) { Copy-Item $src "$Bk\$n.tif" }
    $b = [Drawing.Bitmap]::FromFile("$Bk\$n.tif")
    $c = New-Object Drawing.Bitmap $b.Width, $b.Height, ([Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [Drawing.Graphics]::FromImage($c); $g.DrawImage($b, 0, 0, $b.Width, $b.Height); $g.Dispose(); $b.Dispose()
    $c.Save($src, $codec, $ep); $c.Dispose()
    "{0}: re-saved {1} bytes" -f $n, (Get-Item $src).Length
}
Set-Location $H
foreach ($n in $names) {
    $p = Start-Process -FilePath .\tool.exe -ArgumentList @('reimport-bitmaps-single', "objects\characters\elite_noble\bitmaps\$n") -NoNewWindow -PassThru -RedirectStandardOutput "$env:TEMP\ri_$n.txt" -RedirectStandardError "$env:TEMP\ri_$n.err"
    $p.WaitForExit(300000) | Out-Null
    "{0}: tool exit {1} | {2}" -f $n, $p.ExitCode, ((Get-Content "$env:TEMP\ri_$n.txt","$env:TEMP\ri_$n.err" | Select-String -Pattern 'error|fail|warn' | Select-Object -First 2 | ForEach-Object { $_.Line.Trim() }) -join ' / ')
}
