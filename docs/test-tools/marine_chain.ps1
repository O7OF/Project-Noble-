# Marine chain test (2026-11-13): Armory MARINE tab saves picks (dev cycle "marine") -> Firefight mod match uses the
# SAVED picks (no env override) -> Tyler's appearance.dat restored. Muted, second monitor.
$Rel = 'D:\Project-Noble-Closed-Source\build\bin\Release'
$T = "$Rel\data\userdata\claude-tests-2026-11-11"
$settings = "$Rel\data\userdata\settings.json"
if (Get-Process noble-dev, noble-client -ErrorAction SilentlyContinue) { "game already running - not starting a test"; exit }
$bak = "$T\appearance.dat.before-marine-chain"
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
    $a = @('-menu-screen', 'armory', '-menu-ignore-input', '-armory-cycle', 'marine', '-armory-cycle-delay', '3000', '-capture-at', '22', '-quit-after', '25')
    $p = Start-Process -FilePath "$Rel\noble-dev.exe" -WorkingDirectory $Rel -ArgumentList $a -PassThru
    $p.WaitForExit(90000) | Out-Null
    Remove-Item (Join-Path $Rel "running\$($p.Id).flag") -ErrorAction SilentlyContinue
    Add-Type -AssemblyName System.Drawing
    Get-ChildItem $Rel -Filter 'capture_9*.bmp' | ForEach-Object { $b = [Drawing.Bitmap]::FromFile($_.FullName); $b.Save("$T\marUI.png", [Drawing.Imaging.ImageFormat]::Png); $b.Dispose() }
    Remove-Item "$Rel\capture_9*.bmp" -ErrorAction SilentlyContinue
    Remove-Item Env:\NOBLE_WINDOW_SECONDARY -ErrorAction SilentlyContinue
    Set-Vol $og $oo $om
    & "$T\tools\match_marine.ps1" -Name marC -Test ''
} finally {
    Remove-Item Env:\NOBLE_WINDOW_SECONDARY -ErrorAction SilentlyContinue
    Set-Vol $og $oo $om
    Copy-Item $bak "$Rel\data\appearance.dat" -Force
    "appearance.dat restored"
}
