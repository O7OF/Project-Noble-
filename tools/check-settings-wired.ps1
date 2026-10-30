# Proves which settings actually do something.
#
# For every field in SettingsData, check whether it is consumed anywhere outside the settings plumbing itself
# (Settings.cpp reads/writes it, SettingsPanel.cpp edits it). A field with no other consumer is a setting that
# cannot possibly change the game: it is stored and displayed, but nothing acts on it.
#
#   .\check-settings-wired.ps1            # report
#   .\check-settings-wired.ps1 -FailEmpty # exit 1 when an unconsumed field is found (for CI)

param(
    [switch]$FailEmpty,
    [string]$Repo = "D:\Halo-Reach-Community-Edition"
)

$ErrorActionPreference = "Stop"
$header = Join-Path $Repo "src\common\Settings.hpp"
$src = Join-Path $Repo "src"

# Fields declared in SettingsData (skip methods and constants).
$fields = @()
$inStruct = $false
foreach ($line in Get-Content $header) {
    if ($line -match '^\s*struct\s+SettingsData') { $inStruct = $true; continue }
    if ($inStruct -and $line -match '^\};') { break }
    if (-not $inStruct) { continue }
    if ($line -match '^\s*(?:std::[A-Za-z_:<>, ]+|bool|int|float|unsigned long long)\s+([A-Za-z_][A-Za-z0-9_]*)\s*(?:=|;)') {
        $fields += $Matches[1]
    }
}

# Everything that is allowed to mention a field without acting on it.
$plumbing = @(
    (Join-Path $Repo "src\common\Settings.cpp"),
    (Join-Path $Repo "src\common\Settings.hpp"),
    (Join-Path $Repo "src\ui\SettingsPanel.cpp"),
    (Join-Path $Repo "src\ui\SettingsPanel.hpp")
)

$sources = Get-ChildItem (Join-Path $src "*.cpp"), (Join-Path $src "*.hpp"), (Join-Path $src "*\*.cpp"), (Join-Path $src "*\*.hpp") -ErrorAction SilentlyContinue |
    Where-Object { $plumbing -notcontains $_.FullName }

$rows = @()
foreach ($field in $fields) {
    $consumers = @()
    foreach ($file in $sources) {
        $hits = Select-String -Path $file.FullName -Pattern ("\b$field\b") -ErrorAction SilentlyContinue
        foreach ($hit in $hits) {
            $rel = $hit.Path.Replace($Repo + "\", "")
            $consumers += ("{0}:{1}" -f $rel, $hit.LineNumber)
        }
    }
    $rows += [pscustomobject]@{
        Setting  = $field
        Wired    = ($consumers.Count -gt 0)
        Consumers = (($consumers | Select-Object -First 3) -join ", ")
    }
}

$rows | Sort-Object Wired, Setting | Format-Table -AutoSize
$dead = @($rows | Where-Object { -not $_.Wired })
Write-Host ("{0} settings, {1} wired, {2} stored but never used" -f $rows.Count, ($rows.Count - $dead.Count), $dead.Count)
if ($dead.Count -gt 0) {
    Write-Host "Never used by anything but the settings UI:" -ForegroundColor Yellow
    foreach ($d in $dead) { Write-Host ("  " + $d.Setting) -ForegroundColor Yellow }
    if ($FailEmpty) { exit 1 }
}
exit 0
