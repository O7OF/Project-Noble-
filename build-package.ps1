param(
    [string]$Destination = (Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Noble Launcher')
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$build = Join-Path $repo 'build'
$cmake = 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'

Write-Host '== Building release client =='
& $cmake --build $build --config Release --target noble-client
if ($LASTEXITCODE -ne 0) { throw 'noble-client build failed' }

$exe = Join-Path $build 'bin\Release\noble-client.exe'
if (-not (Test-Path $exe)) { throw "missing $exe" }

Write-Host "== Packaging to $Destination =="
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
Copy-Item $exe (Join-Path $Destination 'Noble.exe') -Force

# Runtime data: keep game content and UI, never the player's saves or the huge mod cache.
$dataDirs = @('menumaps','ui','armor','blocks','loadscreen','maps','game_variants','map_variants','sandbox')
foreach ($d in $dataDirs) {
    $src = Join-Path $repo "data\$d"
    if (Test-Path $src) {
        $dst = Join-Path $Destination "data\$d"
        New-Item -ItemType Directory -Force -Path $dst | Out-Null
        Copy-Item (Join-Path $src '*') $dst -Recurse -Force -ErrorAction SilentlyContinue
    }
}
foreach ($f in @('forge_palette.json')) {
    $src = Join-Path $repo "data\$f"
    if (Test-Path $src) { Copy-Item $src (Join-Path $Destination "data\$f") -Force }
}

# vgmstream is required for the MCC music path; keep it in a tool folder next to the exe.
$vg = Join-Path $build 'bin\Release\tools\vgmstream'
if (Test-Path $vg) {
    $dst = Join-Path $Destination 'tools\vgmstream'
    New-Item -ItemType Directory -Force -Path $dst | Out-Null
    Copy-Item (Join-Path $vg '*') $dst -Recurse -Force
}

Set-Content (Join-Path $Destination 'version.txt') ("Noble playtest build " + (Get-Date -Format 'yyyy-MM-dd HH:mm'))
Write-Host '== Done =='
Write-Host "Executable: $(Join-Path $Destination 'Noble.exe')"
