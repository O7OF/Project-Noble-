# Start a two-instance LAN Slayer pair on one PC (C12 trace runs): host + client, both with -trace-members.
# Usage:  powershell -ExecutionPolicy Bypass -File tools\lan-two-instance.ps1
# The host runs from build\bin\Release, the client from build\bin\Release2 (second data folder: assets
# junctioned, userdata/sandbox copied). The client connects over loopback to 127.0.0.1:11774.
# Note: -trace-members slows the game and both processes stop pumping frames after a few minutes (known issue);
# grab the logs and restart with this script when that happens.
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$binHost = Join-Path $repo 'build\bin\Release'
$binClient = Join-Path $repo 'build\bin\Release2'
if (-not (Test-Path $binClient)) { throw "second instance folder missing: $binClient" }
Get-Process noble-dev -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 3
$hostLog = Join-Path $binHost 'host-lan.log'
$clientLog = Join-Path $binClient 'client-lan.log'
$h = Start-Process -FilePath (Join-Path $binHost 'noble-dev.exe') -ArgumentList @('-trace-members','-host','-lan','-private','-max-players','8','-map','20_sword_slayer','-mode','multiplayer','-log','host-lan.log') -WorkingDirectory $binHost -PassThru
Write-Host "host   pid $($h.Id)  log $hostLog"
Start-Sleep -Seconds 12
$c = Start-Process -FilePath (Join-Path $binClient 'noble-dev.exe') -ArgumentList @('-trace-members','-lan','-connect','127.0.0.1:11774','-map','20_sword_slayer','-mode','multiplayer','-log','client-lan.log') -WorkingDirectory $binClient -PassThru
Write-Host "client pid $($c.Id)  log $clientLog"
Write-Host 'Both windows are titled Project Noble; the client is the second one. Play the client by hand.'