# C12-L (d): three-instance LAN trace on one PC - host + 2 clients, all with -trace-members and the
# (dev, known-unsafe) 32-player mode, to find slot code paths nobody has reviewed yet (MEMBER-NEW| lines).
# Usage:  powershell -ExecutionPolicy Bypass -File tools\lan-three-instance.ps1 [-JoinDelay 90]
# Needs ENGINE LIMITS = Extended or Full in all three settings (Release, Release2, Release3); otherwise
# -slot-patch is ignored (the log says so).
# Folders: host build\bin\Release, client 1 build\bin\Release2 (existing), client 2 build\bin\Release3 (created
# here on first use: data folders junctioned to Release, userdata copied from Release2 with its own device id).
# The freshly built noble-dev.exe is copied into Release2 and Release3 every time.
# Known issue: -trace-members slows the game and the processes may stop pumping frames after a few minutes;
# MEMBER-NEW| lines are written within 5 s of being seen, so the logs stay useful.
param([int]$JoinDelay = 90)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$bin1 = Join-Path $repo 'build\bin\Release'
$bin2 = Join-Path $repo 'build\bin\Release2'
$bin3 = Join-Path $repo 'build\bin\Release3'
$exe = Join-Path $bin1 'noble-dev.exe'
if (-not (Test-Path $exe)) { throw "build first (build-dev.bat): $exe missing" }
if (-not (Test-Path $bin2)) { throw "second instance folder missing: $bin2 (see tools\lan-two-instance.ps1)" }

if (-not (Test-Path $bin3)) {
    Write-Host "Creating $bin3 ..."
    New-Item -ItemType Directory -Path (Join-Path $bin3 'data') | Out-Null
    Get-ChildItem (Join-Path $bin1 'data') -Directory | Where-Object { $_.Name -notin @('userdata', 'sandbox') } | ForEach-Object {
        New-Item -ItemType Junction -Path (Join-Path $bin3 "data\$($_.Name)") -Target $_.FullName | Out-Null
    }
    Get-ChildItem (Join-Path $bin1 'data') -File | Copy-Item -Destination (Join-Path $bin3 'data')
    robocopy (Join-Path $bin2 'data\userdata') (Join-Path $bin3 'data\userdata') /E /XD cache /NFL /NDL /NJH /NJS /NP | Out-Null
    robocopy (Join-Path $bin2 'data\sandbox') (Join-Path $bin3 'data\sandbox') /E /NFL /NDL /NJH /NJS /NP | Out-Null
    $idFile = Join-Path $bin3 'data\userdata\identity.json'
    if (Test-Path $idFile) {
        $id = Get-Content $idFile -Raw | ConvertFrom-Json
        $id.deviceId = -join ((1..16) | ForEach-Object { '{0:x}' -f (Get-Random -Maximum 16) })
        $id.playerName = 'NobleC2'
        $id.serviceTag = 'C2'
        $id | ConvertTo-Json | Set-Content $idFile -Encoding UTF8
    }
}
Get-Process noble-dev -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 3
Copy-Item $exe $bin2 -Force
Copy-Item $exe $bin3 -Force

$env:NOBLE_SLOT_PLAYERS_UNSAFE = '1'   # inherited by the three processes only
$common = @('-trace-members', '-slot-patch', '-slot-players', '32', '-map', '20_sword_slayer', '-mode', 'multiplayer')
$h = Start-Process -FilePath $exe -ArgumentList (@('-host', '-lan', '-private', '-max-players', '8') + $common + @('-log', 'host-trace3.log')) -WorkingDirectory $bin1 -PassThru
Write-Host "host     pid $($h.Id)  log $bin1\host-trace3.log"
Start-Sleep -Seconds 12
$c1 = Start-Process -FilePath (Join-Path $bin2 'noble-dev.exe') -ArgumentList (@('-lan', '-connect', '127.0.0.1:11774') + $common + @('-log', 'client1-trace3.log')) -WorkingDirectory $bin2 -PassThru
Write-Host "client 1 pid $($c1.Id)  log $bin2\client1-trace3.log"
Write-Host "Client 2 joins in $JoinDelay s (mid-game join). Meanwhile, play a little in client 1 (die / respawn)."
Start-Sleep -Seconds $JoinDelay
$c2 = Start-Process -FilePath (Join-Path $bin3 'noble-dev.exe') -ArgumentList (@('-lan', '-connect', '127.0.0.1:11774') + $common + @('-log', 'client2-trace3.log')) -WorkingDirectory $bin3 -PassThru
Write-Host "client 2 pid $($c2.Id)  log $bin3\client2-trace3.log"
Remove-Item Env:NOBLE_SLOT_PLAYERS_UNSAFE
Write-Host ''
Write-Host 'Then, about 30 s apart:'
Write-Host '  1. If the game type has teams: change team in a client (pause menu / scoreboard).'
Write-Host '  2. Player leave: close client 1 (pause menu > QUIT).'
Write-Host '  3. Lobby reset: on the host, end the game / return to the lobby (pause menu > QUIT TO MENU).'
Write-Host '  4. Close everything. Logs: host-trace3.log, client1-trace3.log, client2-trace3.log (look for MEMBER-NEW|).'
