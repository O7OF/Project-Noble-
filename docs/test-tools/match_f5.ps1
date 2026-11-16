param([int]$Quit = 80, [string]$Frames = '3000,4200,5400,6600', [int]$F5At = 30, [string]$Map = '20_sword_slayer')
# Like match_test.ps1, but presses F5 (third person) $F5At seconds after launch; captures -> PNG (960x540).
$Rel = 'D:\Project-Noble-Closed-Source\build\bin\Release'
$env:NOBLE_WINDOW_SECONDARY = '1'   # open on the AOC (second monitor), never Tyler's Samsung (primary)
Set-Location $Rel
if (Get-Process noble-dev,noble-client -ErrorAction SilentlyContinue) { "game already running - not starting a test"; exit }
Add-Type @'
using System; using System.Runtime.InteropServices;
public static class K {
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
}
'@
Get-ChildItem $Rel -Filter 'capture_*.*' | Remove-Item -ErrorAction SilentlyContinue
$mark = Get-Date
$P = Start-Process -FilePath (Join-Path $Rel 'noble-dev.exe') -ArgumentList @('-map',$Map,'-mode','multiplayer','-quit-after',"$Quit",'-capture-frames',$Frames) -PassThru
Start-Sleep $F5At
$P.Refresh()
foreach ($h in @($P.MainWindowHandle) + (Get-Process -Id $P.Id).MainWindowHandle) { if ($h -ne [IntPtr]::Zero) { [K]::SetForegroundWindow($h) | Out-Null } }
Start-Sleep -Milliseconds 400
[K]::keybd_event(0x74, 0x3F, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 80; [K]::keybd_event(0x74, 0x3F, 2, [UIntPtr]::Zero)
"F5 sent at " + [int]((Get-Date) - $mark).TotalSeconds + " s"
while (-not $P.HasExited -and ((Get-Date) - $mark).TotalSeconds -lt ($Quit + 150)) { Start-Sleep 3 }
if (-not $P.HasExited) { Stop-Process -Id $P.Id -Force }
"exit code: " + $P.ExitCode
Add-Type -AssemblyName System.Drawing
Get-ChildItem $Rel -Filter 'capture_*.bmp' | ForEach-Object {
  $b = [System.Drawing.Bitmap]::FromFile($_.FullName); $s = New-Object System.Drawing.Bitmap $b, 960, 540
  $s.Save(($_.FullName -replace '\.bmp$', '.png'), [System.Drawing.Imaging.ImageFormat]::Png); $b.Dispose(); $s.Dispose(); $_.Name }
Get-Content (Join-Path $Rel 'noble-dev.log') -Tail 20000 | Where-Object { $_ -ge ("[" + $mark.ToString('yyyy-MM-dd HH:mm')) -and $_ -match 'ElitePack: (Noble|Elite dressed|Armory Elite)|FATAL|EXCEPTION' } | Select-Object -First 8
