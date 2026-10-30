# Extract a readable co-op failure window from a noble-client.log
#
# The full log is 3-4 MB of engine chatter; the co-op evidence is ~250 lines. This produces the same filtered extract
# both sides can compare, with a timeline header.
#
#   .\tools\extract-coop-log.ps1 -Log build\bin\Release\noble-client.log `
#       -Start "2026-10-30 19:39:44" -End "2026-10-30 19:40:12" -Role guest `
#       -Out "C:\Users\tyler\Downloads\campaign-guest.log.txt"
#
# Useful windows are around the launch: search the log for "launching match via Relaunch" and take ~30 s before to
# ~20 s after the failure.

param(
    [Parameter(Mandatory = $true)][string]$Log,
    [Parameter(Mandatory = $true)][string]$Start,
    [Parameter(Mandatory = $true)][string]$End,
    [ValidateSet('host', 'guest')][string]$Role = 'guest',
    [string]$Out = "",
    [string]$Note = ""
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path $Log)) { throw "log not found: $Log" }
if (-not $Out) { $Out = Join-Path (Split-Path $Log) ("coop-" + $Role + "-" + ($Start -replace '[: ]', '-') + ".log.txt") }

$skip = 'StringLookup|TextMetrics|TextPrepare|FilePathResolver|Slot 48|Slot 67|Slot 61|Slot 51|MH_CreateHook|MH_EnableHook'
$keep = 'FrontEndMenu|Lobby launch|Guest launching|Lobby start broadcast|launching match|Relaunch|NetworkTransport: (Initialized|Configured|Starting|Stopped)|MasterServer|BlamEngine|game session started|LobbyPatches: \[|CLOSE channel|abort|boot|refus|EngineEvent|PauseMenu|error|warning|LoadProgress|Slot 6 - GameResults|Slot 1 - FrameTick|main blckd|net blckd|Career:|join queue'

$lines = Get-Content -LiteralPath $Log
$window = $lines | Where-Object {
    if ($_ -notmatch '^\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\.') { return $false }
    $t = $matches[1]
    return ($t -ge $Start -and $t -le $End)
} | Where-Object { $_ -match $keep -and $_ -notmatch $skip }

$pkt = @($window | Where-Object { $_ -match 'TX #|RX #' }).Count
$body = $window | Where-Object { $_ -notmatch 'TX #|RX #' }

$header = @(
    "Project Noble - co-op failure log ($Role side)",
    "==================================================",
    "Log      : $Log",
    "Window   : $Start .. $End",
    "Captured : $(Get-Date -Format 'yyyy-MM-dd HH:mm') ET",
    ("Note     : " + $(if ($Note) { $Note } else { "-" })),
    "",
    "How to read: search this file for 'game session started' (client reached the match), 'Slot 6 - GameResults'",
    "(match ended), 'abort called' (reason 27/0x1B players incompatible, 28/0x1C peer disconnect, 29/0x1D state sync",
    "timeout), 'boot_peer' (host removed a peer), 'join queue' (host-side join bookkeeping) and 'PauseMenu: game over'.",
    "",
    "--- filtered log ($($body.Count) lines, $pkt packet TX/RX lines omitted) ---"
)
Set-Content -LiteralPath $Out -Value ($header + $body) -Encoding UTF8
Write-Host "wrote $Out ($($body.Count) lines, $pkt packet lines omitted)"
