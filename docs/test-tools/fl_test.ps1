# Flashlight test (2026-11-13): local Sword Base built from HREK (merged spartans.biped) via -map-override; presses L
# at -PressAt seconds; captures at -Frames. Muted, second monitor.
param([string]$Name = 'fl', [string]$Map = 'D:\Steam\steamapps\common\HREK\maps\20_sword_slayer.map', [int]$PressAt = 50,
      [string]$Frames = '46,56', [int]$Quit = 62)
$Rel = 'D:\Project-Noble-Closed-Source\build\bin\Release'
$T = "$Rel\data\userdata\claude-tests-2026-11-11"
$settings = "$Rel\data\userdata\settings.json"
if (Get-Process noble-dev, noble-client -ErrorAction SilentlyContinue) { "game already running - not starting a test"; exit }
Add-Type @'
using System; using System.Runtime.InteropServices;
public static class K3 {
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
}
'@
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
    $P = Start-Process -FilePath "$Rel\noble-dev.exe" -ArgumentList @('-map', '20_sword_slayer', '-mode', 'multiplayer', '-map-override', $Map, '-quit-after', "$Quit", '-capture-at', $Frames) -PassThru
    Start-Sleep $PressAt; $P.Refresh()
    foreach ($h in @($P.MainWindowHandle) + (Get-Process -Id $P.Id).MainWindowHandle) { if ($h -ne [IntPtr]::Zero) { [K3]::SetForegroundWindow($h) | Out-Null } }
    Start-Sleep -Milliseconds 400
    [K3]::keybd_event(0x4C, 0x26, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 120; [K3]::keybd_event(0x4C, 0x26, 2, [UIntPtr]::Zero)
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
Get-Content "$Rel\noble-dev.log" -Tail 30000 | Where-Object { $_ -ge ("[" + $mark.ToString('yyyy-MM-dd HH:mm')) -and $_ -match 'Flashlight|FATAL|EXCEPTION|session started|map-override|override' } | Select-Object -First 12
