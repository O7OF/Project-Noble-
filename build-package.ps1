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

$release = Join-Path $build 'bin\Release'
if (-not (Test-Path (Join-Path $release 'noble-client.exe'))) { throw 'missing noble-client.exe' }

Write-Host "== Packaging to $Destination =="
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
# Copy the complete working runtime folder. Selective copies kept missing a startup
# dependency that only the full build tree had.
& robocopy $release $Destination /E /XF *.log *.map *.bmp *.dmp /NFL /NDL /NJH /NJS /NP
if ($LASTEXITCODE -ge 8) { throw "robocopy failed ($LASTEXITCODE)" }

Copy-Item (Join-Path $Destination 'noble-client.exe') (Join-Path $Destination 'Noble.exe') -Force
foreach ($extra in @('noble-client.exe','noble-dev.exe','noble-dedicated.exe','NobleTest.exe','dedicated_server.json')) {
    Remove-Item (Join-Path $Destination $extra) -Force -ErrorAction SilentlyContinue
}
if (Test-Path (Join-Path $Destination 'dedicated_data')) {
    Remove-Item (Join-Path $Destination 'dedicated_data') -Recurse -Force -ErrorAction SilentlyContinue
}

Set-Content (Join-Path $Destination 'version.txt') ("Noble playtest build " + (Get-Date -Format 'yyyy-MM-dd HH:mm'))
Write-Host '== Done =='
Write-Host "Executable: $(Join-Path $Destination 'Noble.exe')"
