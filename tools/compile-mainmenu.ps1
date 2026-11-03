# Compiles HREK's levels\ui\mainmenu scenario into haloreach\maps\mainmenu.map.
# HREK (Halo: Reach Editing Kit) is required; the output is your own build of the game data and is
# not redistributed with Noble.
param(
    [string]$HrekRoot = '',
    [string]$MccRoot = '',
    [string]$Language = 'english',
    [switch]$NoCopy
)
$ErrorActionPreference = 'Stop'

function Find-GameRoot([string]$name) {
    $steam = (Get-ItemProperty 'HKCU:\Software\Valve\Steam' -ErrorAction SilentlyContinue).SteamPath
    if (-not $steam) {
        $steam = (Get-ItemProperty 'HKLM:\SOFTWARE\WOW6432Node\Valve\Steam' -ErrorAction SilentlyContinue).InstallPath
    }
    $candidates = @()
    if ($steam) { $candidates += (Join-Path $steam "steamapps\common\$name") }
    foreach ($drive in (Get-PSDrive -PSProvider FileSystem | Select-Object -ExpandProperty Root)) {
        $candidates += (Join-Path $drive "SteamLibrary\steamapps\common\$name")
        $candidates += (Join-Path $drive "Program Files (x86)\Steam\steamapps\common\$name")
    }
    foreach ($c in $candidates) { if (Test-Path $c) { return $c } }
    return $null
}

if (-not $HrekRoot) { $HrekRoot = Find-GameRoot 'HREK' }
if (-not $HrekRoot) { throw 'HREK not found. Pass -HrekRoot "D:\SteamLibrary\steamapps\common\HREK".' }
$tool = Join-Path $HrekRoot 'tool.exe'
$scenario = Join-Path $HrekRoot 'tags\levels\ui\mainmenu\mainmenu.scenario'
if (-not (Test-Path $tool)) { throw "tool.exe not found: $tool" }
if (-not (Test-Path $scenario)) { throw "mainmenu scenario not found: $scenario" }

if (-not $MccRoot) { $MccRoot = Find-GameRoot 'Halo The Master Chief Collection' }
if (-not $NoCopy -and -not $MccRoot) { throw 'MCC not found. Pass -MccRoot or use -NoCopy.' }

$map = Join-Path $HrekRoot 'maps\mainmenu.map'
if (Test-Path $map) { Remove-Item $map -Force }

Write-Host "Compiling levels\ui\mainmenu\mainmenu with $tool ..."
Push-Location $HrekRoot
try {
    # The scenario tag path is the folder plus the tag name. This is the command HREK's own
    # bat\LevelsForModTools.txt entry uses and it produces maps\mainmenu.map.
    & $tool build-cache-file levels\ui\mainmenu\mainmenu pc $Language
} finally {
    Pop-Location
}

if (-not (Test-Path $map)) {
    throw "The build did not produce $map. Check the HREK tool output above."
}
$size = (Get-Item $map).Length
if ($size -lt 1MB) { throw "The built map is only $size bytes; the build looks incomplete." }
Write-Host "Built $map ($([math]::Round($size / 1MB)) MB)"

if ($NoCopy) { Write-Host 'NoCopy: leaving the map in HREK\maps.'; exit 0 }

$destDir = Join-Path $MccRoot 'haloreach\maps'
$dest = Join-Path $destDir 'mainmenu.map'
if (-not (Test-Path $destDir)) { throw "MCC maps folder not found: $destDir" }
if (Test-Path $dest) {
    $backup = "$dest.stock.bak"
    if (-not (Test-Path $backup)) { Copy-Item $dest $backup -Force; Write-Host "Backed up stock map to $backup" }
}
Copy-Item $map $dest -Force
Write-Host "Installed $dest ($([Math]::Round($size / 1MB)) MB)"
Write-Host 'Noble loads this map from haloreach\maps. The Armory scene must exist in the compiled tags.'
exit 0
