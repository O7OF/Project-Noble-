# 2026-11-14 Trooper mode test: a campaign mission, muted, on the AOC (second monitor), captures at -Frames (seconds).
# Tyler's appearance.dat is backed up and restored. Captures -> <Name>_NN.png in the test folder.
param([string]$Name = 'tr', [string]$Map = 'm10', [string]$Frames = '120,160', [int]$Quit = 170, [string]$Extra = '-trooper -third-person', [int]$F5At = 0)
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
Copy-Item "$Rel\data\appearance.dat" "$T\appearance.dat.bak_camp" -Force
Set-Location $Rel
Get-ChildItem $Rel -Filter 'capture_*.*' | Remove-Item -ErrorAction SilentlyContinue
$mark = Get-Date
try {
    Set-Vol 0.0 0.0 $true
    $env:NOBLE_WINDOW_SECONDARY = '1'
    $a = @('-map', $Map, '-mode', 'campaign', '-quit-after', "$Quit", '-capture-at', $Frames)
    if ($Extra) { $a += $Extra.Split(' ') }
    $P = Start-Process -FilePath "$Rel\noble-dev.exe" -ArgumentList $a -PassThru
    if ($F5At -gt 0) {
        Add-Type 'using System; using System.Runtime.InteropServices; public static class K3 { [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra); [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h); }' -ErrorAction SilentlyContinue
        Start-Sleep $F5At; $P.Refresh(); if ($P.MainWindowHandle -ne [IntPtr]::Zero) { [K3]::SetForegroundWindow($P.MainWindowHandle) | Out-Null }
        Start-Sleep -Milliseconds 400; [K3]::keybd_event(0x74, 0x3F, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 80; [K3]::keybd_event(0x74, 0x3F, 2, [UIntPtr]::Zero)
    }
    while (-not $P.HasExited -and ((Get-Date) - $mark).TotalSeconds -lt ($Quit + 150)) { Start-Sleep 3 }
    if (-not $P.HasExited) { Stop-Process -Id $P.Id -Force }
} finally {
    Remove-Item Env:\NOBLE_WINDOW_SECONDARY -ErrorAction SilentlyContinue
    Set-Vol $og $oo $om
    Copy-Item "$T\appearance.dat.bak_camp" "$Rel\data\appearance.dat" -Force
    "appearance.dat restored"
}
Add-Type -AssemblyName System.Drawing
Get-ChildItem $Rel -Filter 'capture_*.bmp' | ForEach-Object {
    $b = [System.Drawing.Bitmap]::FromFile($_.FullName)
    $b.Save("$T\$($Name)_$($_.BaseName -replace 'capture_9000','').png", [System.Drawing.Imaging.ImageFormat]::Png); $b.Dispose()
}
Remove-Item "$Rel\capture_9*.bmp" -ErrorAction SilentlyContinue
Get-ChildItem $T -Filter "$($Name)_*.png" | ForEach-Object { $_.Name }
Get-Content "$Rel\noble-dev.log" -Tail 40000 | Where-Object { $_ -ge ("[" + $mark.ToString('yyyy-MM-dd HH:mm:ss')) -and $_ -match 'TrooperMode|FATAL|EXCEPTION|CRASH' } | Select-Object -First 15
