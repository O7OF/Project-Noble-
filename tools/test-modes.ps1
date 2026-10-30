# Multiplayer smoke test for Project Noble.
#
# Loads every mode / game type / map combination the Custom Game screen allows and reports whether the engine
# reached its launch state without crashing or logging an error. A bare CLI launch stops in the "lobby waiting"
# state (LoadProgress 5%, launch state 11) by design - the host still has to press start - so the pass condition
# is "map + game type loaded, no crash, no [error] lines", not "gameplay reached".
#
#   .\test-modes.ps1                 # quick matrix, about 4 minutes
#   .\test-modes.ps1 -Full           # every supported map x game type, about 25 minutes
#   .\test-modes.ps1 -QuitAfter 30   # give slow maps longer to load
#
# Each run writes its own log slice to test-modes-<n>-<label>.log next to this script.

param(
    [switch]$Full,
    [int]$QuitAfter = 22,
    [int]$Limit = 0,          # only the first N cases (debugging)
    [string]$Exe = "D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.exe",
    [string]$Variants = "D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\game_variants"
)

$ErrorActionPreference = "Stop"
$exeDir = Split-Path -Parent $Exe
$log = Join-Path $exeDir "noble-client.log"

function Variant([string]$stem) {
    $path = Join-Path $Variants "$stem.bin"
    if (-not (Test-Path $path)) { return $null }
    return $path
}

# Map scenario -> display name, mirroring the menu's list.
$maps = [ordered]@{
    "20_sword_slayer"  = "Sword Base"
    "70_boneyard"      = "Boneyard"
    "35_island"        = "Spire"
    "52_ivory_tower"   = "Reflection"
    "30_settlement"    = "Powerhouse"
    "dlc_medium"       = "Tempest"
    "forge_halo"       = "Forge World"
    "dlc_invasion"     = "Breakpoint"
}

# Game types that work on every map, plus the two that only work where they ship.
$types = @(
    @{ label = "Slayer";     stem = "slayer_054" },
    @{ label = "CTF";        stem = "ctf_054" },
    @{ label = "Assault";    stem = "assault_054" },
    @{ label = "KOTH";       stem = "koth_054" },
    @{ label = "Oddball";    stem = "oddball_054" },
    @{ label = "Infection";  stem = "infection_054" },
    @{ label = "Invasion";   stem = "3nvasion_054"; only = @("70_boneyard", "35_island", "dlc_invasion", "forge_halo") }
)

$cases = @()
if ($Full) {
    foreach ($map in $maps.Keys) {
        foreach ($type in $types) {
            if ($type.only -and ($type.only -notcontains $map)) { continue }
            $cases += [pscustomobject]@{ Mode = "multiplayer"; Map = $map; Label = "$($maps[$map]) / $($type.label)"; Stem = $type.stem }
        }
    }
} else {
    $cases += [pscustomobject]@{ Mode = "multiplayer"; Map = "20_sword_slayer"; Label = "Sword Base / Slayer"; Stem = "slayer_054" }
    $cases += [pscustomobject]@{ Mode = "multiplayer"; Map = "20_sword_slayer"; Label = "Sword Base / CTF"; Stem = "ctf_054" }
    $cases += [pscustomobject]@{ Mode = "multiplayer"; Map = "70_boneyard"; Label = "Boneyard / Invasion"; Stem = "3nvasion_054" }
    $cases += [pscustomobject]@{ Mode = "multiplayer"; Map = "35_island"; Label = "Spire / Invasion"; Stem = "3nvasion_054" }
}
# Firefight, campaign and Forge, always included.
foreach ($ff in @("hr_ff_standard_1set", "hr_ff_gruntpocalypse_1round")) {
    $cases += [pscustomobject]@{ Mode = "firefight"; Map = "45_launch_station"; Label = "Firefight / $ff"; Stem = $ff }
}
$cases += [pscustomobject]@{ Mode = "campaign"; Map = "m10_b"; Label = "Campaign / Winter Contingency"; Stem = $null }
$cases += [pscustomobject]@{ Mode = "forge"; Map = "forge_halo"; Label = "Forge / Forge World"; Stem = $null }

if ($Limit -gt 0) { $cases = $cases | Select-Object -First $Limit }
$results = @()
$index = 0
foreach ($case in $cases) {
    $index++
    $args = @("-quit-after", "$QuitAfter", "-map", $case.Map, "-mode", $case.Mode)
    if ($case.Stem) {
        $variant = Variant $case.Stem
        if (-not $variant) {
            $results += [pscustomobject]@{ Case = $case.Label; Result = "SKIP (variant missing)"; Seconds = 0 }
            continue
        }
        $args += @("-variant", $variant)
    }
    if ($case.Mode -eq "multiplayer") { $args += @("-host", "-lan") }

    # The engine is a single instance per account: a process that is still shutting down makes the next launch
    # hang at PostCommand(11) and -quit-after never fires, so wait for a real exit before starting.
    Get-Process noble-client -ErrorAction SilentlyContinue | ForEach-Object {
        $_ | Stop-Process -Force
        $_.WaitForExit(15000)
    }
    Start-Sleep -Seconds 3
    if (Get-Process noble-client -ErrorAction SilentlyContinue) {
        Write-Host "a noble-client process is still running; stopping the run" -ForegroundColor Red
        exit 1
    }
    $before = 0
    if (Test-Path $log) { $before = (Get-Content $log | Measure-Object -Line).Lines }
    $started = Get-Date
    $proc = Start-Process -FilePath $Exe -ArgumentList $args -WorkingDirectory $exeDir -PassThru
    $finished = $proc.WaitForExit(($QuitAfter + 90) * 1000)
    if (-not $finished) {
        Get-Process noble-client -ErrorAction SilentlyContinue | ForEach-Object { $_ | Stop-Process -Force; $_.WaitForExit(15000) }
    }
    $seconds = [math]::Round(((Get-Date) - $started).TotalSeconds)

    $slice = @()
    if (Test-Path $log) { $slice = Get-Content $log | Select-Object -Skip $before }
    $slice | Set-Content (Join-Path $PSScriptRoot ("test-modes-{0:d2}.log" -f $index))
    $errors = ($slice | Select-String -Pattern '\[error\]').Count
    $crashed = ($slice | Select-String -Pattern 'FATAL EXCEPTION').Count
    $reachedLaunch = ($slice | Select-String -Pattern 'LoadProgress: 5\.0%|LoadProgress: 100|launch state 11').Count
    $result = if ($crashed -gt 0) { "CRASH" }
              elseif (-not $finished) { "HUNG" }
              elseif ($errors -gt 0) { "ERRORS ($errors)" }
              elseif ($reachedLaunch -eq 0) { "NO LAUNCH" }
              else { "ok" }
    $results += [pscustomobject]@{ Case = $case.Label; Result = $result; Seconds = $seconds }
    Write-Host ("[{0}/{1}] {2,-38} {3,-8} {4}s" -f $index, $cases.Count, $case.Label, $result, $seconds)
}

Get-Process noble-client -ErrorAction SilentlyContinue | Stop-Process -Force

Write-Host ""
$results | Format-Table -AutoSize
$failed = ($results | Where-Object { $_.Result -notlike "ok*" -and $_.Result -notlike "SKIP*" }).Count
Write-Host ("{0}/{1} passed ({2} skipped)" -f ($results.Count - $failed - ($results | Where-Object { $_.Result -like "SKIP*" }).Count), $results.Count, ($results | Where-Object { $_.Result -like "SKIP*" }).Count)
if ($failed -gt 0) {
    Write-Host "Failures:" -ForegroundColor Red
    $results | Where-Object { $_.Result -notlike "ok*" -and $_.Result -notlike "SKIP*" } | Format-Table -AutoSize
    exit 1
}
exit 0
