<#
.SYNOPSIS
    One-command check that Noble's extended engine limits are really in effect.

.DESCRIPTION
    Starts noble-dev.exe once per profile (Stock, then Extended), lets it reach the main menu, reads the
    engine's live pool sizes from the log (src/host/PoolReport.cpp), closes it, and prints a PASS/FAIL table.
    Your ENGINE LIMITS setting is restored afterwards.

    Run from the repository root after building noble-dev:
        powershell -ExecutionPolicy Bypass -File tools\limits\test-limits.ps1

.PARAMETER Seconds
    How long each run waits before reading the pools (default 20).

.PARAMETER Profiles
    Profile numbers to test: 0 = Stock, 1 = Extended (V16), 2 = Extended + Particles. Default: 0, 1.
#>
param(
    [int]$Seconds = 20,
    [int[]]$Profiles = @(0, 1)
)

$ErrorActionPreference = 'Stop'
$repo = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$bin = Join-Path $repo 'build\bin\Release'
$exe = Join-Path $bin 'noble-dev.exe'
$settings = Join-Path $bin 'data\userdata\settings.json'

if (-not (Test-Path $exe)) { throw "noble-dev.exe not found in $bin. Build it first (msbuild build\noble-dev.vcxproj ...)." }
if (-not (Test-Path $settings)) { throw "settings.json not found at $settings. Start Noble once first." }
foreach ($p in $Profiles) {
    $nlp = @{ 1 = 'v16.nlp'; 2 = 'v16p.nlp' }[$p]
    if ($nlp -and -not (Test-Path (Join-Path $bin "data\limits\$nlp"))) {
        throw "data\limits\$nlp is missing next to the exe. Run: robocopy data build\bin\Release\data /E /XO /XD userdata autosave mods cache sandbox"
    }
}
if (Get-Process -Name 'noble-client', 'noble-dev' -ErrorAction SilentlyContinue) {
    throw "Close Noble before running this test."
}

$original = [System.IO.File]::ReadAllText($settings)
if ($original -notmatch '"extendedLimits"\s*:\s*\d+') { throw "settings.json has no extendedLimits entry yet. Open SETTINGS once in this build." }
$names = @{ 0 = 'Stock'; 1 = 'Extended (V16)'; 2 = 'Extended + Particles' }
$results = @{}

try {
    foreach ($p in $Profiles) {
        Write-Host ("Testing {0}..." -f $names[$p]) -ForegroundColor Cyan
        $patched = [regex]::Replace($original, '"extendedLimits"\s*:\s*\d+', "`"extendedLimits`": $p")
        [System.IO.File]::WriteAllText($settings, $patched)

        $log = Join-Path $bin "limits-test-$p.log"
        Remove-Item $log -ErrorAction SilentlyContinue
        $proc = Start-Process -FilePath $exe -ArgumentList @('-log', "limits-test-$p.log", '-pool-report-exit', "$Seconds") `
                              -WorkingDirectory $bin -PassThru
        if (-not $proc.WaitForExit(($Seconds + 90) * 1000)) {
            Write-Warning "Run did not finish in time; closing it."
            Stop-Process -Id $proc.Id -Force
        }
        Get-Process -Name 'noble-dev' -ErrorAction SilentlyContinue | Stop-Process -Force   # relaunched match process, if any

        $r = [ordered]@{ Requested = $names[$p]; Active = '?'; Reason = ''; Pools = @{}; Crashed = $false; Done = $false }
        if (Test-Path $log) {
            foreach ($line in Get-Content $log) {
                if ($line -match 'Engine limits: requested (.+?), active (.+?), reason (.*)$') { $r.Active = $Matches[2]; $r.Reason = $Matches[3] }
                elseif ($line -match 'POOL\|([^|]+)\|([^|]+)\|(\d+)\|') { $r.Pools[$Matches[1]] = @{ Used = $Matches[2]; Cap = [int]$Matches[3] } }
                elseif ($line -match 'FATAL EXCEPTION') { $r.Crashed = $true }
                elseif ($line -match 'POOL-REPORT-DONE') { $r.Done = $true }
            }
        }
        $results[$p] = $r
    }
}
finally {
    [System.IO.File]::WriteAllText($settings, $original)
    Write-Host "Your ENGINE LIMITS setting was restored." -ForegroundColor DarkGray
}

# ---- Report ----------------------------------------------------------------------------------------
$expectedObjects = @{ 0 = 2048; 1 = 32767; 2 = 32767 }
$allPass = $true
function Show([string]$label, [bool]$ok, [string]$detail) {
    $script:allPass = $script:allPass -and $ok
    $tag = if ($ok) { 'PASS' } else { 'FAIL' }
    $color = if ($ok) { 'Green' } else { 'Red' }
    Write-Host ("  [{0}] {1}  {2}" -f $tag, $label, $detail) -ForegroundColor $color
}

Write-Host ""
Write-Host "=== Engine limits test ===" -ForegroundColor White
foreach ($p in $Profiles) {
    $r = $results[$p]
    Write-Host ("{0}:" -f $names[$p]) -ForegroundColor White
    Show 'started without crashing' (-not $r.Crashed -and $r.Done) $(if ($r.Crashed) { 'crash in log' } elseif (-not $r.Done) { 'no pool report (did it reach the menu?)' } else { '' })
    Show 'profile active' ($r.Active -eq $names[$p]) ("active: {0} {1}" -f $r.Active, $(if ($r.Reason -and $r.Reason -ne 'none') { "($($r.Reason))" } else { '' }))
    $obj = $r.Pools['object']
    if ($obj) { Show 'object pool' ($obj.Cap -eq $expectedObjects[$p]) ("{0} in use / {1} (expected {2})" -f $obj.Used, $obj.Cap, $expectedObjects[$p]) }
    else { Show 'object pool' $false 'not found in the report' }
}

if ($results.Count -ge 2) {
    $a = $results[$Profiles[0]]; $b = $results[$Profiles[1]]
    $changed = @($a.Pools.Keys + $b.Pools.Keys | Sort-Object -Unique | Where-Object {
        $a.Pools[$_] -and $b.Pools[$_] -and $a.Pools[$_].Cap -ne $b.Pools[$_].Cap })
    Write-Host ""
    Write-Host ("Pools whose size differs between {0} and {1}:" -f $names[$Profiles[0]], $names[$Profiles[1]]) -ForegroundColor White
    if ($changed.Count -eq 0) { Write-Host "  (none)" }
    foreach ($n in $changed) { Write-Host ("  {0,-28} {1,8} -> {2,8}" -f $n, $a.Pools[$n].Cap, $b.Pools[$n].Cap) }
}

Write-Host ""
if ($allPass) { Write-Host "RESULT: PASS" -ForegroundColor Green; exit 0 }
else { Write-Host "RESULT: FAIL (logs: build\bin\Release\limits-test-*.log)" -ForegroundColor Red; exit 1 }
