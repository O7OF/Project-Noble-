# Mute (or unmute) the game for test runs.
#
# The engine takes its volumes from the player profile: ReachHost writes SettingsData.volumeGame to profile +0xA5C and
# volumeOther to +0xA64, so zeroing those silences gunfire/vehicles/dialogue in the engine itself. masterMute only silences
# Noble's own audio (menu music, menu sounds).
#
#   .\tools\mute-game.ps1                 # mute the main checkout
#   .\tools\mute-game.ps1 -Unmute         # restore the volumes from the backup taken at mute time
#   .\tools\mute-game.ps1 -IncludeClaude  # mute both checkouts (main + D:\Halo-Reach-Community-Edition-claude)
#
# Nothing is committed: both settings.json files live under build\bin\Release\data\userdata, which is not in git.

param(
    [switch]$Unmute,
    [switch]$IncludeClaude,
    [string[]]$ExtraSettings = @()
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$targets = @(Join-Path $repoRoot 'build\bin\Release\data\userdata\settings.json')
if ($IncludeClaude) {
    $targets += 'D:\Halo-Reach-Community-Edition-claude\build\bin\Release\data\userdata\settings.json'
}
$targets += $ExtraSettings

function Read-JsonNoBom([string]$path) {
    $text = [System.IO.File]::ReadAllText($path, [System.Text.Encoding]::UTF8)
    if ($text.Length -gt 0 -and $text[0] -eq [char]0xFEFF) { $text = $text.Substring(1) }
    return $text | ConvertFrom-Json
}

function Write-JsonNoBom([string]$path, $obj) {
    $json = $obj | ConvertTo-Json -Depth 8
    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($path, $json, $utf8NoBom)
}

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$backupExt = '.before-mute.json'

foreach ($path in $targets) {
    if (-not (Test-Path $path)) { Write-Host "skip (no settings): $path"; continue }
    $backup = $path + $backupExt

    if ($Unmute) {
        if (Test-Path $backup) {
            [System.IO.File]::WriteAllText($path, [System.IO.File]::ReadAllText($backup), $utf8NoBom)
            Remove-Item $backup -Force
            Write-Host "unmuted (restored backup): $path"
        } else {
            $s = Read-JsonNoBom $path
            $s.volumeGame = 1.0; $s.volumeMusic = 1.0; $s.volumeOther = 1.0; $s.masterMute = $false
            Write-JsonNoBom $path $s
            Write-Host "unmuted (no backup, set volumes to 1.0): $path"
        }
        continue
    }

    if (-not (Test-Path $backup)) { Copy-Item $path $backup -Force }
    $s = Read-JsonNoBom $path
    $s.volumeGame = 0.0
    $s.volumeMusic = 0.0
    $s.volumeOther = 0.0
    $s.masterMute = $true
    Write-JsonNoBom $path $s
    Write-Host ("muted: {0}  (volumeGame/Music/Other=0, masterMute=true; backup {1})" -f $path, (Split-Path -Leaf $backup))
}
