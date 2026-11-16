# H2A Marine Armory preview capture (2026-11-13): dev cycle "h2a" (H2A model, Halo Wars colours), full-res capture.
# Tyler's appearance.dat is backed up and restored. Muted, second monitor.
param([string]$Name = 'h2a1', [int]$At = 22, [string]$List = '', [string]$Cycle = 'marine', [string]$Scene = '', [string]$Focus = '')
$Rel = 'D:\Project-Noble-Closed-Source\build\bin\Release'
$T = "$Rel\data\userdata\claude-tests-2026-11-11"
$settings = "$Rel\data\userdata\settings.json"
if (Get-Process noble-dev, noble-client -ErrorAction SilentlyContinue) { "game already running - not starting a test"; exit }
$bak = "$T\appearance.dat.before-h2a"
Copy-Item "$Rel\data\appearance.dat" $bak -Force
function Set-Vol([double]$g, [double]$o, [bool]$mm) {
    $text = [IO.File]::ReadAllText($settings, [Text.Encoding]::UTF8); if ($text.Length -gt 0 -and $text[0] -eq [char]0xFEFF) { $text = $text.Substring(1) }
    $o2 = $text | ConvertFrom-Json; $o2.volumeGame = $g; $o2.volumeOther = $o; $o2.masterMute = $mm
    [IO.File]::WriteAllText($settings, ($o2 | ConvertTo-Json -Depth 8), (New-Object Text.UTF8Encoding($false)))
}
$cur = (([IO.File]::ReadAllText($settings, [Text.Encoding]::UTF8)) -replace '^\uFEFF','') | ConvertFrom-Json
$og = [double]$cur.volumeGame; $oo = [double]$cur.volumeOther; $om = [bool]$cur.masterMute
try {
    Set-Vol 0.0 0.0 $true
    $env:NOBLE_WINDOW_SECONDARY = '1'
    Set-Location $Rel
    Get-ChildItem $Rel -Filter 'capture_9*.bmp' | Remove-Item -ErrorAction SilentlyContinue
    $a = @('-menu-screen', 'armory', '-menu-ignore-input', '-armory-cycle', $Cycle, '-armory-cycle-delay', '3000', '-capture-at', "$At", '-quit-after', "$($At + 3)")
    if ($Focus) { $a += @('-armory-focus', $Focus) }
    if ($Scene) { $a += @('-armory-scene', $Scene) }
    if ($List) { $a += @('-armory-cycle-list', $List) }
    $p = Start-Process -FilePath "$Rel\noble-dev.exe" -WorkingDirectory $Rel -ArgumentList $a -PassThru
    $p.WaitForExit(90000) | Out-Null
    Remove-Item (Join-Path $Rel "running\$($p.Id).flag") -ErrorAction SilentlyContinue
    Add-Type -AssemblyName System.Drawing
    Get-ChildItem $Rel -Filter 'capture_9*.bmp' | ForEach-Object { $b = [Drawing.Bitmap]::FromFile($_.FullName); $b.Save("$T\$Name.png", [Drawing.Imaging.ImageFormat]::Png); "$T\$Name.png $($b.Width)x$($b.Height)"; $b.Dispose() }
    Remove-Item "$Rel\capture_9*.bmp" -ErrorAction SilentlyContinue
} finally {
    Remove-Item Env:\NOBLE_WINDOW_SECONDARY -ErrorAction SilentlyContinue
    Set-Vol $og $oo $om
    Copy-Item $bak "$Rel\data\appearance.dat" -Force
    "appearance.dat restored"
}
