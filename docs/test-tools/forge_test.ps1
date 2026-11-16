# Forge World test (2026-11-13): HREK-built forge_halo.map via -map-override, Forge mode, captures at -Frames.
# Extra args via -Extra. Muted, second monitor.
param([string]$Name = 'fw', [string]$Map = 'D:\Steam\steamapps\common\HREK\maps\forge_halo.map', [string]$Frames = '45,60',
      [int]$Quit = 65, [string]$Extra = '', [switch]$NoOverride)
$Rel = 'D:\Project-Noble-Closed-Source\build\bin\Release'
$T = "$Rel\data\userdata\claude-tests-2026-11-11"
$settings = "$Rel\data\userdata\settings.json"
if (Get-Process noble-dev, noble-client -ErrorAction SilentlyContinue) { "game already running - not starting a test"; exit }
function Set-Vol([double]$g, [double]$o, [bool]$mm) {
    $text = [IO.File]::ReadAllText($settings, [Text.Encoding]::UTF8); if ($text.Length -gt 0 -and $text[0] -eq [char]0xFEFF) { $text = $text.Substring(1) }
    $o2 = $text | ConvertFrom-Json; $o2.volumeGame = $g; $o2.volumeOther = $o; $o2.masterMute = $mm
    [IO.File]::WriteAllText($settings, ($o2 | ConvertTo-Json -Depth 8), (New-Object Text.UTF8Encoding($false)))
}
$cur = (([IO.File]::ReadAllText($settings, [Text.Encoding]::UTF8)) -replace '^\uFEFF','') | ConvertFrom-Json
$og = [double]$cur.volumeGame; $oo = [double]$cur.volumeOther; $om = [bool]$cur.masterMute
Set-Location $Rel
Get-ChildItem $Rel -Filter 'capture_*.*' | Remove-Item -ErrorAction SilentlyContinue
$mark = Get-Date
try {
    Set-Vol 0.0 0.0 $true
    $env:NOBLE_WINDOW_SECONDARY = '1'
    $a = @('-map', 'forge_halo', '-mode', 'forge', '-quit-after', "$Quit", '-capture-at', $Frames)
    if (-not $NoOverride) { $a += @('-map-override', $Map) }
    if ($Extra) { $a += $Extra.Split(' ') }
    $P = Start-Process -FilePath "$Rel\noble-dev.exe" -ArgumentList $a -PassThru
    while (-not $P.HasExited -and ((Get-Date) - $mark).TotalSeconds -lt ($Quit + 120)) { Start-Sleep 3 }
    if (-not $P.HasExited) { Stop-Process -Id $P.Id -Force }
} finally {
    Remove-Item Env:\NOBLE_WINDOW_SECONDARY -ErrorAction SilentlyContinue
    Set-Vol $og $oo $om
}
Add-Type -AssemblyName System.Drawing
Get-ChildItem $Rel -Filter 'capture_*.bmp' | ForEach-Object {
    $b = [System.Drawing.Bitmap]::FromFile($_.FullName)
    $b.Save("$T\$($Name)_$($_.BaseName -replace 'capture_9000','').png", [System.Drawing.Imaging.ImageFormat]::Png); $b.Dispose()
}
Remove-Item "$Rel\capture_9*.bmp" -ErrorAction SilentlyContinue
Get-ChildItem $T -Filter "$($Name)_*.png" | ForEach-Object { $_.Name }
Get-Content "$Rel\noble-dev.log" -Tail 30000 | Where-Object { $_ -ge ("[" + $mark.ToString('yyyy-MM-dd HH:mm')) -and $_ -match 'FATAL|EXCEPTION|CRASH|session started|override|Forge|VFS: serving' } | Select-Object -First 15
