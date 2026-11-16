param([int]$Quit = 100, [string]$Frames = '1800,3600', [string]$Map = '20_sword_slayer')
$Rel = 'D:\Project-Noble-Closed-Source\build\bin\Release'
$env:NOBLE_WINDOW_SECONDARY = '1'   # open on the AOC (second monitor), never Tyler's Samsung (primary)
Set-Location $Rel
if (Get-Process noble-dev,noble-client -ErrorAction SilentlyContinue) { "game already running - not starting a test"; exit }
$Log = Join-Path $Rel 'noble-dev.log'
$mark = Get-Date
Get-ChildItem $Rel -Filter 'capture_*.bmp' | Remove-Item -ErrorAction SilentlyContinue
$P = Start-Process -FilePath (Join-Path $Rel 'noble-dev.exe') -ArgumentList @('-map',$Map,'-mode','multiplayer','-quit-after',"$Quit",'-capture-frames',$Frames) -PassThru
while (-not $P.HasExited -and ((Get-Date) - $mark).TotalSeconds -lt ($Quit + 150)) { Start-Sleep 3 }
if (-not $P.HasExited) { "still running -> stopping the test process"; Stop-Process -Id $P.Id -Force }
"exit code: " + $P.ExitCode
Start-Sleep 2
$stamp = $mark.ToString('yyyy-MM-dd HH:mm')
Get-Content $Log -Tail 6000 | Where-Object { $_ -ge "[$stamp" } | Select-String -Pattern 'ElitePack|FATAL|EXCEPTION|elitepicks|appearance builder' | Select-Object -First 70 | ForEach-Object { $_.Line }
"--- captures"
Get-ChildItem $Rel -Filter 'capture_*.bmp' | ForEach-Object { $_.Name }
