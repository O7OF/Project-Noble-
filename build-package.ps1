param(
    [string]$Destination = (Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Noble Launcher')
)
$ErrorActionPreference = 'Stop'
$repo = $PSScriptRoot
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

# Runtime data: copy every content folder except player saves, caches and the huge mod tree.
$runtimeData = Join-Path $build 'bin\Release\data'
$excludeDirs = @('mods','userdata','autosave','sandbox','cache')
$destData = Join-Path $Destination 'data'
New-Item -ItemType Directory -Force -Path $destData | Out-Null
Get-ChildItem $runtimeData -Directory | Where-Object { $excludeDirs -notcontains $_.Name } | ForEach-Object {
    Copy-Item $_.FullName -Destination $destData -Recurse -Force
}
# Root runtime files: strings, launch blocks, default profile/appearance, palette.
Get-ChildItem $runtimeData -File | Copy-Item -Destination $destData -Force

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
