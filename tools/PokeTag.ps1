# Assembly-style loose-tag field poker for the HREK Reach tag tree.
#
# Examples:
#   .\tools\PokeTag.ps1 -Tag 'o7of_tags\vehicles\pelican_o7\pelican_mp.vehicle' -Field 'unit/seats[0]/label' -Value 'wraith_d'
#   .\tools\PokeTag.ps1 -Tag 'objects\vehicles\covenant\phantom\phantom_mp.vehicle' -Field 'unit/seats[1]/label' -Value 'warthog_p' -Build
#
# -Build rebuilds levels\multi\forge_halo\forge_halo with HREK's tool.exe and installs the
# result into data\mods\noble-forge\maps\forge_halo.map (and the Release data copy).
param(
    [Parameter(Mandatory = $true)][string]$Tag,
    [Parameter(Mandatory = $true)][string]$Field,
    [Parameter(Mandatory = $true)][string]$Value,
    [switch]$Build,
    [string]$Hrek = 'C:\Program Files (x86)\Steam\steamapps\common\HREK',
    [string]$Repo = 'C:\Users\Chey1\Desktop\Halo Reach Community Edition'
)

$ErrorActionPreference = 'Stop'
$shellDir = Join-Path $Repo 'scratch\blam-tag-shell\blam-tag-shell-windows-x86_64'
$shell = Join-Path $shellDir 'blam-tag-shell.exe'
$src = Join-Path $Hrek "tags\$Tag"
if (-not (Test-Path $src)) { throw "tag not found: $src" }

$workDir = Join-Path $Repo 'scratch\hrek_work'
New-Item -ItemType Directory -Force -Path $workDir | Out-Null
$work = Join-Path $workDir (Split-Path $Tag -Leaf)
Copy-Item $src $work -Force

Push-Location $shellDir
try {
    & $shell --game haloreach_mcc set $work $Field $Value
    if ($LASTEXITCODE -ne 0) { throw "blam-tag-shell set failed ($LASTEXITCODE)" }
    & $shell --game haloreach_mcc get $work $Field
} finally {
    Pop-Location
}

Copy-Item $work $src -Force
Write-Host "wrote $Field = $Value to $Tag"

if ($Build) {
    Push-Location $Hrek
    try {
        & .\tool.exe build-cache-file 'levels\multi\forge_halo\forge_halo' pc
        if ($LASTEXITCODE -ne 0) { throw "tool.exe build failed ($LASTEXITCODE)" }
    } finally {
        Pop-Location
    }
    $map = Join-Path $Hrek 'maps\forge_halo.map'
    Copy-Item $map (Join-Path $Repo 'data\mods\noble-forge\maps\forge_halo.map') -Force
    Copy-Item $map (Join-Path $Repo 'build\bin\Release\data\mods\noble-forge\maps\forge_halo.map') -Force
    Write-Host "rebuilt and installed forge_halo.map"
}
