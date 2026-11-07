param(
    [string]$OutDir = [Environment]::GetFolderPath('Desktop'),
    [string]$Name = 'Noble-Armory-Pack'
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$release = Join-Path $repo 'build\bin\Release'
$client = Join-Path $release 'noble-client.exe'
$mapMod = Join-Path $release 'data\menumaps\mainmenu_all.map'
$pack = Join-Path $release 'data\armor\rr_elite_pack.map'
foreach ($required in @($client, $mapMod, $pack)) {
    if (-not (Test-Path $required)) { throw "Missing required file: $required" }
}

$pkg = Join-Path $env:TEMP $Name
if (Test-Path $pkg) { Remove-Item $pkg -Recurse -Force }
New-Item -ItemType Directory -Force -Path $pkg | Out-Null

Write-Host '== Copying the release client =='
Copy-Item $client (Join-Path $pkg 'Noble.exe') -Force

Write-Host '== Copying data (map mod + armour pack included; user saves excluded) =='
$destData = Join-Path $pkg 'data'
New-Item -ItemType Directory -Force -Path $destData | Out-Null
& robocopy (Join-Path $release 'data') $destData /E /XD userdata autosave cache sandbox /XF appearance.dat settings.json career.json launch_block_1.bin lb_m_c_1.bin profile_1.bin *.log *.dmp *.tmp /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -gt 7) { throw "robocopy failed ($LASTEXITCODE)" }

foreach ($check in @('data\menumaps\mainmenu_all.map', 'data\armor\rr_elite_pack.map')) {
    if (-not (Test-Path (Join-Path $pkg $check))) { throw "Package is missing $check" }
}

$zip = Join-Path $OutDir "$Name.zip"
if (Test-Path $zip) { Remove-Item $zip -Force }
Write-Host "== Compressing to $zip (this takes a few minutes for the 400 MB map/pack) =="
Compress-Archive -Path (Join-Path $pkg '*') -DestinationPath $zip -CompressionLevel Fastest

$size = (Get-Item $zip).Length
Write-Host ("== Done: {0} ({1:N1} MB) ==" -f $zip, ($size / 1MB))
Write-Host 'Upload this zip as a GitHub Release asset (GitHub repo files are limited to 100 MB).'
