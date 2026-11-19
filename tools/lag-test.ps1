# Dev-only automatic lag test (2026-11-19). Runs an N-player one-PC game with the dev network simulator on every copy
# and prints a short PASS/FAIL summary from the logs, so networking can be re-checked after any change.
# Needs the dev build in build\bin\Release..ReleaseN and the multi-instance harness (probe-campN.ps1).
#
#   powershell -ExecutionPolicy Bypass -File tools\lag-test.ps1 -Players 4 -DelayMs 50 -JitterMs 10 -LossPercent 3
param(
  [int]$Players = 4,
  [int]$DelayMs = 50,
  [int]$JitterMs = 10,
  [double]$LossPercent = 2,
  [string]$Mode = 'campaign',
  [string]$Map = 'm10',
  [int]$Seconds = 240,
  [double]$MinFreeGb = 0.8,
  [string]$Harness = 'D:\noble-bisect\probe-campN.ps1'
)
$ErrorActionPreference = 'Continue'
$bin = Join-Path $PSScriptRoot '..\build\bin' | Resolve-Path
if (Get-Process noble-dev, noble-client -ErrorAction SilentlyContinue) { 'ABORT: Noble is already running'; exit 1 }
if (-not (Test-Path $Harness)) { "ABORT: harness not found: $Harness"; exit 1 }

$env:NOBLE_JIP_PROBE = '0'
$env:NOBLE_NETSIM = "delay=$DelayMs,jitter=$JitterMs,loss=$LossPercent"
$started = Get-Date
"Lag test: $Players players, $Mode $Map, $DelayMs ms +/- $JitterMs ms each way, $LossPercent% loss, $Seconds s"
& $Harness -Guests ($Players - 1) -StockLimits -MinFreeGb $MinFreeGb -Mode $Mode -Map $Map `
  -QuitAfter $Seconds -WaitSeconds ($Seconds + 40) *> (Join-Path $bin 'lag-test-harness.txt')
Remove-Item Env:NOBLE_NETSIM -ErrorAction SilentlyContinue

$logs = @(Join-Path $bin 'Release\host-camp5.log')
for ($i = 2; $i -le $Players; $i++) { $logs += Join-Path $bin "Release$i\client$i-camp5.log" }
$fail = $false
foreach ($f in $logs) {
  if (-not (Test-Path $f)) { "{0}: MISSING LOG" -f (Split-Path $f -Leaf); $fail = $true; continue }
  $s = Select-String $f -Pattern 'game session started' | Select-Object -First 1
  $bad = (Select-String $f -Pattern 'decoded false|ENGINE ERROR' -CaseSensitive).Count
  $copies = 0
  Select-String $f -Pattern 'last 30 s: (\d+) lost game packets replaced' | ForEach-Object { $copies += [int]$_.Matches[0].Groups[1].Value }
  $size = (Select-String $f -Pattern 'game packets sent, average (\d+) bytes, largest (\d+) bytes, (\d+) over 1200' | Select-Object -Last 1)
  $sizeText = if ($size) { "avg {0} B, max {1} B, {2} over 1200 B (last 30 s)" -f $size.Matches[0].Groups[1].Value, $size.Matches[0].Groups[2].Value, $size.Matches[0].Groups[3].Value } else { 'no size stats' }
  $ok = [bool]$s -and $bad -eq 0
  if (-not $ok) { $fail = $true }
  "{0}: {1} | started {2} | errors {3} | lost packets recovered {4} | {5}" -f (Split-Path $f -Leaf), $(if ($ok) { 'PASS' } else { 'FAIL' }), $(if ($s) { 'yes' } else { 'NO' }), $bad, $copies, $sizeText
}
$abort = Select-String (Join-Path $bin 'lag-test-harness.txt') -Pattern '^ABORT' -CaseSensitive | Select-Object -First 1
if ($abort) { "NOTE: $($abort.Line) (the PC ran low on memory; the run was cut short)" }
"RESULT: $(if ($fail) { 'FAIL' } else { 'PASS' }) ({0:N0} s)" -f ((Get-Date) - $started).TotalSeconds
