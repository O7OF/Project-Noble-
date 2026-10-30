# Work with the Noble backend (netcode / server scripts)
#
# The server is now centralised in its own repository:
#
#   https://github.com/O7OF/ServerTestDeploy
#
# The client repo no longer carries `server/` (commit 19207fb removed it). So server work happens in a clone of that
# repo, not here:
#
#   .\tools\server-repo.ps1                 # clone next to this checkout (or report it is already there) and pull
#   .\tools\server-repo.ps1 -Path D:\ServerTestDeploy
#   .\tools\server-repo.ps1 -Pull           # fast-forward an existing clone
#   .\tools\server-repo.ps1 -Status         # branch, head, and whether it is behind origin
#
# After editing server.js / src/ in that clone: run `node --check server.js`, then `git commit` + `git push` from there.
# Nothing server-side should be committed to O7OF/test-build-01 any more.

param(
    [string]$Path = "",
    [switch]$Pull,
    [switch]$Status
)

$ErrorActionPreference = 'Continue'   # git writes progress/warnings to stderr; failures are checked via $LASTEXITCODE

$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $Path) {
    $parent = Split-Path -Parent $repoRoot
    $Path = Join-Path $parent 'ServerTestDeploy'
}

if (-not (Test-Path (Join-Path $Path '.git'))) {
    if ($Status) { Write-Host "no clone at $Path"; exit 0 }
    Write-Host "cloning ServerTestDeploy into $Path"
    git clone https://github.com/O7OF/ServerTestDeploy.git $Path
    if ($LASTEXITCODE -ne 0) { throw "clone failed - check access to O7OF/ServerTestDeploy" }
}

Push-Location $Path
try {
    git fetch origin | Out-Null
    $branch = git rev-parse --abbrev-ref HEAD
    $head = git rev-parse --short HEAD
    $remote = git rev-parse --short origin/main
    $behind = [int](git rev-list --count "HEAD..origin/main")
    $dirty = (git status --porcelain | Measure-Object).Count

    if ($Pull -and $behind -gt 0) {
        Write-Host "fast-forwarding $behind commit(s)"
        git -c core.editor=true pull --no-rebase --no-edit origin main | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "pull failed" }
        $head = git rev-parse --short HEAD
        $behind = [int](git rev-list --count "HEAD..origin/main")
    }

    Write-Host ""
    Write-Host "ServerTestDeploy"
    Write-Host "  path:    $Path"
    Write-Host "  branch:  $branch"
    Write-Host "  head:    $head   (origin/main $remote, behind $behind)"
    Write-Host "  changed: $dirty file(s)"
    if ($Status) { exit 0 }

    Write-Host ""
    Write-Host "Server work happens here:"
    Write-Host "  edit server.js / src/*, then:"
    Write-Host "    node --check server.js"
    Write-Host "    git add -A; git commit -m '…'; git push origin main"
    Write-Host ""
    Write-Host "Diagnostics after a failed join:"
    Write-Host "  GET /api/health      version, commit, relay counters"
    Write-Host "  GET /api/relay/stats live relay rooms (hasHost / guests) + packet counters"
} finally {
    Pop-Location
}
