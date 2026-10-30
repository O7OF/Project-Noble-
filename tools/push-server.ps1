# Push server / netcode changes to the deployment repo
#
# The client repo carries a working copy of the Node server under `server/`, but the canonical, deployed copy lives in
# https://github.com/O7OF/ServerTestDeploy . Any change to netcode or server scripts has to end up in BOTH:
#
#   .\tools\push-server.ps1 -Message "lobby: keep relay room across re-register"
#
# What it does: clones the deployment repo next to this checkout (if missing), copies the server source files over it,
# commits, and pushes. Runtime state is deliberately NOT copied unless asked:
#   - node_modules/   (installed on the host, never source)
#   - database.db     (live player data; pass -IncludeDb only when you really mean to overwrite it)
#
# Files considered "server source":
#   server.js, grant_item.js, reset_db.js, set_dev.js, update_auth.js, package.json, package-lock.json, src/**

param(
    [string]$Message = "server sync from Project Noble",
    [string]$RepoPath = "",
    [switch]$IncludeDb,
    [switch]$DryRun
)

# git writes progress and line-ending warnings to stderr; with 'Stop' PowerShell would treat those as failures, so
# errors are checked explicitly through $LASTEXITCODE after each git call instead.
$ErrorActionPreference = 'Continue'

$repoRoot = Split-Path -Parent $PSScriptRoot
$serverDir = Join-Path $repoRoot 'server'
if (-not (Test-Path $serverDir)) { throw "no server folder at $serverDir" }

if (-not $RepoPath) { $RepoPath = Join-Path (Split-Path -Parent $repoRoot) 'ServerTestDeploy' }
if (-not (Test-Path (Join-Path $RepoPath '.git'))) {
    Write-Host "cloning deployment repo into $RepoPath"
    if (-not $DryRun) {
        git clone https://github.com/O7OF/ServerTestDeploy.git $RepoPath
        if ($LASTEXITCODE -ne 0) { throw "clone failed" }
    }
}

$files = @(
    'server.js', 'grant_item.js', 'reset_db.js', 'set_dev.js', 'update_auth.js',
    'package.json', 'package-lock.json'
)
if ($IncludeDb) { $files += 'database.db' }

foreach ($file in $files) {
    $src = Join-Path $serverDir $file
    if (-not (Test-Path $src)) { Write-Warning "missing in server/: $file"; continue }
    Write-Host "  copy $file"
    if (-not $DryRun) { Copy-Item $src (Join-Path $RepoPath $file) -Force }
}

# src/ is copied wholesale (no node_modules inside it)
if (Test-Path (Join-Path $serverDir 'src')) {
    Write-Host "  copy src/"
    if (-not $DryRun) {
        Copy-Item (Join-Path $serverDir 'src\*') (Join-Path $RepoPath 'src') -Recurse -Force
    }
}

if ($DryRun) { Write-Host "dry run - nothing committed"; exit 0 }

Push-Location $RepoPath
try {
    git add -A
    $staged = git diff --cached --name-only
    if (-not $staged) { Write-Host "nothing to sync - deployment repo already matches server/"; exit 0 }
    Write-Host "changes to commit:"; $staged | ForEach-Object { "  $_" }
    git -c core.editor=true commit -m $Message
    if ($LASTEXITCODE -ne 0) { throw "commit failed" }
    git push origin main
    if ($LASTEXITCODE -ne 0) { throw "push failed - check access to O7OF/ServerTestDeploy" }
    Write-Host "pushed to https://github.com/O7OF/ServerTestDeploy"
} finally {
    Pop-Location
}
