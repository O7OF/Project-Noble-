# Phase 7 step 4: one command that makes a map AI-capable (nav mesh, AI squads, whole-map zones) and builds it.
#   powershell -ExecutionPolicy Bypass -Command "& tools\noble-map-ai-bake.ps1 -Tag levels\multi\forge_halo\forge_halo -Centre 38.4,84.5,5.5"
# Steps (each logged to HREK\noble_mods\ai_bake_<map>_<time>\bake.log; stops at the first failure):
#   1. back up the scenario tag and the current HREK\maps\<map>.map into that folder;
#   2. clear "no pathfinding" (structure bsps[*].flags bit 9) on every BSP;
#   3. set the scenario's top-level "don't strip pathfinding" flag (bit 1), or the builder drops the nav mesh from
#      multiplayer maps;
#   4. add the Noble AI characters and test squads around -Centre (tools\noble-forge-ai-squads.ps1; existing squads
#      are kept; spawn heights are corrected at runtime from the nav mesh);
#   5. tool generate-pathfinding;
#   6. whole-map zones / objectives from the nav mesh (tools\noble-ai-zones-from-navmesh.ps1, BSP -Bsp);
#   7. tool generate-pathfinding again (fills the new firing positions' sectors);
#   8. tool build-cache-file (skip with -SkipBuild). The built map stays in HREK\maps; copying it into Noble's
#      data\maps is a separate, deliberate step.
# Host side: MAP AI (lobby option) places the squads 15 s into a custom game / Forge session on any map built this way.
# Limits: zones are generated for one BSP (-Bsp, default 0); Forge pieces placed in the scenario do not survive
# generate-pathfinding on Forge World (see docs\PHASE7-AI-PATHS-PROGRESS.md, step 3).
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [Parameter(Mandatory = $true)][string]$Tag,               # scenario path without extension, e.g. levels\multi\x\x
    [Parameter(Mandatory = $true)][double[]]$Centre,          # x,y,z of the squads' centre (dry land on the nav mesh)
    [int]$Bsp = 0,
    [switch]$SkipBuild,
    [switch]$SkipSquads
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$map = Split-Path $Tag -Leaf
$dir = Join-Path $Hrek ("noble_mods\ai_bake_{0}_{1}" -f $map, (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Force $dir | Out-Null
$log = Join-Path $dir 'bake.log'
function Log([string]$m) { $line = (Get-Date -Format 'HH:mm:ss') + ' ' + $m; $line | Out-File $log -Append -Encoding utf8; Write-Host $line }
# Each ManagedBlam script runs in its own process (ManagedBlam starts once per process).
function Blam([string]$script, [string]$argText) {
    Log "> $script $argText"
    $ErrorActionPreference = 'Continue'   # ManagedBlam / tool.exe write harmless lines to stderr
    $o = powershell -ExecutionPolicy Bypass -Command "& '$repo\tools\$script' $argText" 2>$null
    $o | Out-File $log -Append -Encoding utf8
    if ($o -match 'ERROR|CRASH') { throw "$script failed (see $log)" }
    return $o
}
function Tool([string]$name, [string[]]$a) {
    Log "> tool $($a -join ' ')"
    $t0 = Get-Date
    $ErrorActionPreference = 'Continue'
    Push-Location $Hrek
    try { & .\tool.exe @a *> (Join-Path $dir "$name.log"); $code = $LASTEXITCODE } finally { Pop-Location }
    Log ("  exit {0} in {1:N0} s (log {2}.log)" -f $code, ((Get-Date) - $t0).TotalSeconds, $name)
    if ($code -ne 0) { throw "tool $($a[0]) failed with $code" }
}

try {
    $scen = Join-Path $Hrek "tags\$Tag.scenario"
    if (-not (Test-Path $scen)) { throw "no scenario $scen" }
    Log "AI bake: $Tag, centre $($Centre -join ','), BSP $Bsp"

    # 1. backups
    Copy-Item $scen (Join-Path $dir "$map.scenario") -Force
    $built = Join-Path $Hrek "maps\$map.map"
    if (Test-Path $built) { Copy-Item $built (Join-Path $dir "$map.map") -Force }
    Log "1 backups in $dir"

    # 2. clear "no pathfinding" on every BSP (raw 512)
    $flags = Blam 'noble-tag-flags.ps1' "-Tag '$Tag' -Block 'structure bsps'"
    $count = @($flags | Where-Object { $_ -match '^\[\d+\]' } | ForEach-Object { ($_ -split ' ')[0] } | Select-Object -Unique).Count
    if ($count -lt 1) { $count = 1 }
    for ($i = 0; $i -lt $count; $i++) { [void](Blam 'noble-tag-set-flags.ps1' "-Tag '$Tag' -Block 'structure bsps' -Index $i -Field flags -Clear 512 -Save") }
    Log "2 'no pathfinding' cleared on $count BSP(s)"

    # 3. "don't strip pathfinding" (top-level flags bit 1 = raw 2)
    [void](Blam 'noble-tag-set-flags.ps1' "-Tag '$Tag' -Block '' -Field flags -Set 2 -Save")
    Log "3 'don't strip pathfinding' set"

    # 4. characters + squads
    if (-not $SkipSquads) {
        [void](Blam 'noble-forge-ai-squads.ps1' "-Tag '$Tag' -Centre @($($Centre -join ',')) -Save")
        Log "4 squads around $($Centre -join ',')"
    } else { Log '4 squads skipped' }

    # 5-7. bake, zones, bake again
    Tool 'genpath_1' @('generate-pathfinding', $Tag)
    $z = Blam 'noble-ai-zones-from-navmesh.ps1' "-Tag '$Tag' -Bsp $Bsp -Save"
    Log ('6 zones: ' + (($z | Where-Object { $_ -match 'zones|firing' } | Select-Object -Last 2) -join ' | '))
    Tool 'genpath_2' @('generate-pathfinding', $Tag)

    # 8. build
    if (-not $SkipBuild) {
        Tool 'build' @('build-cache-file', $Tag)
        $m = Get-Item $built
        Log ("8 built {0} ({1:N0} MB)" -f $m.FullName, ($m.Length / 1MB))
    } else { Log '8 build skipped' }
    Log 'DONE'
} catch { Log ('FAILED: ' + $_.Exception.Message); exit 1 }
