# Multiplayer smoke test for Project Noble - every mode with every game type.
#
# A bare CLI launch cannot start a match: the client needs the account/profile arguments the menu adds on hand-off
# (`-player-name`, `-account-user`, `-account-token`, `-user-id`, ...). Without them the engine logs "no game selected"
# and never pumps a frame. So this harness drives the real launch path instead:
#
#   1. starts the client on a setup screen with `-auto-play` (the screen presses PLAY/HOST by itself after ~1.5 s),
#   2. waits for the hand-off line in the log and the new (child) process,
#   3. lets the match run for -PlaySeconds, kills it, and judges the log slice.
#
#   .\test-modes.ps1                    # the standard matrix, a few minutes
#   .\test-modes.ps1 -PlaySeconds 30    # let slow maps load longer
#   .\test-modes.ps1 -Only Slayer,CTF   # just these cases

param(
    [int]$PlaySeconds = 55,
    [string[]]$Only = @(),
    [switch]$Maps,           # add every vanilla map with Slayer, to check that each map loads at all
    [string]$Exe = "D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.exe"
)

$ErrorActionPreference = "Stop"
$exeDir = Split-Path -Parent $Exe
$log = Join-Path $exeDir "noble-client.log"
$outDir = $PSScriptRoot

# mode, screen, variant stem, label, and whether the game-type guard is expected to refuse the start.
$cases = @(
    @{ Screen = "custom"; Stem = "slayer_054";      Label = "Sword Base / Slayer" }
    @{ Screen = "custom"; Stem = "ctf_054";         Label = "Sword Base / CTF" }
    @{ Screen = "custom"; Stem = "assault_054";     Label = "Sword Base / Assault" }
    @{ Screen = "custom"; Stem = "koth_054";        Label = "Sword Base / King of the Hill" }
    @{ Screen = "custom"; Stem = "oddball_054";     Label = "Sword Base / Oddball" }
    @{ Screen = "custom"; Stem = "infection_054";   Label = "Sword Base / Infection" }
    @{ Screen = "custom"; Stem = "territories_054"; Label = "Sword Base / Territories" }
    @{ Screen = "custom"; Stem = "stockpile_054";   Label = "Sword Base / Stockpile" }
    @{ Screen = "custom"; Stem = "headhunter_054";  Label = "Sword Base / Headhunter" }
    @{ Screen = "custom"; Stem = "juggernaut_054";  Label = "Sword Base / Juggernaut" }
    @{ Screen = "custom"; Stem = "3nvasion";        Label = "Sword Base / Invasion (guard must refuse)"; ExpectBlocked = $true }
    @{ Screen = "firefight"; Stem = "hr_ff_standard_1set";       Label = "Firefight / Standard" }
    @{ Screen = "firefight"; Stem = "hr_ff_gruntpocalypse_1round"; Label = "Firefight / Gruntpocalypse" }
    @{ Screen = "campaign";  Stem = $null;                        Label = "Campaign / Winter Contingency" }
    @{ Screen = "forge";     Stem = $null;                        Label = "Forge / Forge World" }
)

if ($Maps) {
    $allMaps = @(
        @{ Scenario = "20_sword_slayer"; Name = "Sword Base" }, @{ Scenario = "50_panopticon"; Name = "Boardwalk" },
        @{ Scenario = "70_boneyard"; Name = "Boneyard" },       @{ Scenario = "45_launch_station"; Name = "Countdown" },
        @{ Scenario = "30_settlement"; Name = "Powerhouse" },   @{ Scenario = "52_ivory_tower"; Name = "Reflection" },
        @{ Scenario = "35_island"; Name = "Spire" },            @{ Scenario = "45_aftship"; Name = "Zealot" },
        @{ Scenario = "dlc_slayer"; Name = "Anchor 9" },        @{ Scenario = "dlc_invasion"; Name = "Breakpoint" },
        @{ Scenario = "dlc_medium"; Name = "Tempest" },         @{ Scenario = "condemned"; Name = "Condemned" },
        @{ Scenario = "trainingpreserve"; Name = "Highlands" }, @{ Scenario = "cex_beaver_creek"; Name = "Battle Canyon" },
        @{ Scenario = "cex_damnation"; Name = "Penance" },      @{ Scenario = "cex_timberland"; Name = "Ridgeline" },
        @{ Scenario = "cex_prisoner"; Name = "Solitary" },      @{ Scenario = "cex_hangemhigh"; Name = "High Noon" },
        @{ Scenario = "cex_headlong"; Name = "Breakneck" },     @{ Scenario = "forge_halo"; Name = "Forge World" }
    )
    foreach ($m in $allMaps) {
        $cases += [pscustomobject]@{ Screen = "custom"; Stem = "slayer_054"; Map = $m.Scenario; Label = ("{0} / Slayer" -f $m.Name) }
    }
}

if ($Only.Count -gt 0) {
    $cases = @($cases | Where-Object { $name = $_.Label; ($Only | Where-Object { $name -like "*$_*" }).Count -gt 0 })
}

$results = @()
$index = 0
foreach ($case in $cases) {
    $index++
    Get-Process noble-client -ErrorAction SilentlyContinue | ForEach-Object { $_ | Stop-Process -Force; $_.WaitForExit(15000) }
    Start-Sleep -Seconds 3
    if (Test-Path $log) { Remove-Item $log -Force }

    $clientArgs = @("-menu-screen", $case.Screen, "-auto-play")
    if ($case.Stem) { $clientArgs += @("-auto-play-variant", $case.Stem) }
    if ($case.Map) { $clientArgs += @("-auto-play-map", $case.Map) }

    $started = Get-Date
    $parent = Start-Process -FilePath $Exe -ArgumentList $clientArgs -WorkingDirectory $exeDir -PassThru
    $handedOff = $false
    # PowerShell 5.1 has no ternary operator.
    $startWindow = 75
    if ($case.ExpectBlocked) { $startWindow = 25 }
    $deadline = (Get-Date).AddSeconds($startWindow)
    while ((Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 500
        if (-not (Test-Path $log)) { continue }
        $text = Get-Content $log -Raw -ErrorAction SilentlyContinue
        if ($text -and $text -match "launching match via Relaunch") { $handedOff = $true; break }
        if ($case.ExpectBlocked -and $text -and $text -match "refused to start") { break }
        if ($parent.HasExited -and -not $text) { break }
    }

    $childRan = $false
    if ($handedOff) {
        $playUntil = (Get-Date).AddSeconds($PlaySeconds)
        while ((Get-Date) -lt $playUntil) {
            Start-Sleep -Milliseconds 500
            $child = Get-Process noble-client -ErrorAction SilentlyContinue | Where-Object { $_.Id -ne $parent.Id }
            if ($child) { $childRan = $true }
        }
    }
    $seconds = [math]::Round(((Get-Date) - $started).TotalSeconds)
    Get-Process noble-client -ErrorAction SilentlyContinue | ForEach-Object { $_ | Stop-Process -Force; $_.WaitForExit(15000) }
    Start-Sleep -Milliseconds 500

    $text = ""
    if (Test-Path $log) { $text = Get-Content $log -Raw -ErrorAction SilentlyContinue }
    $text | Set-Content (Join-Path $outDir ("test-modes-{0:d2}.log" -f $index))
    $errors = ([regex]::Matches($text, "\[error\]")).Count
    $crashed = ([regex]::Matches($text, "FATAL EXCEPTION")).Count
    $frames = ([regex]::Matches($text, "FrameTick")).Count
    # Gameplay marker: the HUD string lookups (slot 57) only happen once the engine is inside the match.
    $inGame = $text -match "Slot 57 - StringLookup" -or $text -match "LoadProgress: 100"

    $result =
        if ($crashed -gt 0) { "CRASH" }
        elseif ($case.ExpectBlocked) { if ($handedOff) { "STARTED (guard failed!)" } else { "blocked (expected)" } }
        elseif (-not $handedOff) { "NO START" }
        elseif (-not $childRan) { "NO MATCH" }
        elseif ($errors -gt 0) { "ERRORS ($errors)" }
        elseif (-not $inGame) { "LOADED, no gameplay ($frames frames)" }
        else { "ok, in game ($frames frames)" }

    $results += [pscustomobject]@{ Case = $case.Label; Result = $result; Seconds = $seconds }
    Write-Host ("[{0}/{1}] {2,-42} {3,-24} {4}s" -f $index, $cases.Count, $case.Label, $result, $seconds)
}

Get-Process noble-client -ErrorAction SilentlyContinue | ForEach-Object { $_ | Stop-Process -Force }

Write-Host ""
$bad = @($results | Where-Object { $_.Result -notlike "ok*" -and $_.Result -notlike "blocked*" })
Write-Host ("{0}/{1} passed" -f ($results.Count - $bad.Count), $results.Count)
if ($bad.Count -gt 0) {
    Write-Host "Failures:" -ForegroundColor Red
    foreach ($r in $bad) { Write-Host ("  {0}: {1}" -f $r.Case, $r.Result) -ForegroundColor Red }
    exit 1
}
exit 0
