# Copies the Halo: Reach menu map (and its matching string table, when present) into data\menumaps
# so a local build can embed it. The map and string tables are Microsoft game data: they are not
# committed and must not be redistributed. The project still compiles without them.
#
# Sources tried, in order:
#   1. -MapPath <file> (a mainmenu_all.map you already extracted)
#   2. the MCC install's haloreach\maps / haloreach folders
#   3. a Reach Renaissance extraction: <ReachRenaissance>\data\menumaps, D:\Reach\data\menumaps
param(
    [string]$MccRoot = '',
    [string]$MapPath = ''
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$dest = Join-Path $root 'data\menumaps'
New-Item -ItemType Directory -Force -Path $dest | Out-Null

function Find-MccRoot {
    if ($MccRoot) {
        if (Test-Path (Join-Path $MccRoot 'haloreach')) { return $MccRoot }
        throw "MccRoot does not contain haloreach: $MccRoot"
    }
    $candidates = @()
    $steam = (Get-ItemProperty 'HKCU:\Software\Valve\Steam' -ErrorAction SilentlyContinue).SteamPath
    if (-not $steam) {
        $steam = (Get-ItemProperty 'HKLM:\SOFTWARE\WOW6432Node\Valve\Steam' -ErrorAction SilentlyContinue).InstallPath
    }
    if ($steam) { $candidates += (Join-Path $steam 'steamapps\common\Halo The Master Chief Collection') }
    foreach ($drive in (Get-PSDrive -PSProvider FileSystem | Select-Object -ExpandProperty Root)) {
        $candidates += (Join-Path $drive 'SteamLibrary\steamapps\common\Halo The Master Chief Collection')
        $candidates += (Join-Path $drive 'Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection')
    }
    foreach ($c in $candidates) {
        if (Test-Path (Join-Path $c 'haloreach')) { return $c }
    }
    return $null
}

$map = $null
if ($MapPath) {
    if (-not (Test-Path $MapPath)) { throw "MapPath not found: $MapPath" }
    $map = Get-Item $MapPath
} else {
    $search = @()
    $mcc = Find-MccRoot
    if ($mcc) {
        $search += (Join-Path $mcc 'haloreach\maps')
        $search += (Join-Path $mcc 'haloreach')
    }
    $search += (Join-Path $root '..\ReachRenaissance\data\menumaps')
    $search += 'D:\Reach\data\menumaps'
    foreach ($dir in $search) {
        if (-not (Test-Path $dir)) { continue }
        $candidate = Get-ChildItem $dir -Filter 'mainmenu*.map' -ErrorAction SilentlyContinue |
                     Sort-Object Length -Descending | Select-Object -First 1
        if ($candidate) { $map = $candidate; break }
    }
}
if (-not $map) {
    throw 'No mainmenu map found. MCC does not always keep it in haloreach\maps; pass -MapPath with your extracted mainmenu_all.map (the Reach Renaissance extraction folder is a common source).'
}

$target = Join-Path $dest 'mainmenu.map'
if ($map.FullName -ne $target) { Copy-Item $map.FullName $target -Force }
Write-Host "Menu map: $($map.FullName) -> $target ($([math]::Round($map.Length / 1MB)) MB)"
Write-Host 'A stock map loads the front-end; the 3D Armory needs an HREK-compiled map with the Armory scene.'

$csvNames = @('mainmenu_all.string_ids.csv')
foreach ($csvName in $csvNames) {
    $csvTarget = Join-Path $dest $csvName
    if (Test-Path $csvTarget) { continue }
    $csvSource = Join-Path $map.DirectoryName $csvName
    if (Test-Path $csvSource) {
        Copy-Item $csvSource $csvTarget -Force
        Write-Host "String table: $csvSource -> $csvTarget"
    }
}
if (-not (Test-Path (Join-Path $dest 'mainmenu_all.string_ids.csv'))) {
    Write-Warning 'mainmenu_all.string_ids.csv not found next to the map. The build still succeeds without it; the Armory needs the matching string table at runtime.'
}
Write-Host 'Re-run CMake (or build.bat) to embed the data into Noble.exe.'
