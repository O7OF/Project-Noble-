param(
    [string]$Destination = (Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'Noble Launcher')
)
$ErrorActionPreference = 'Stop'
$repo = $PSScriptRoot
$build = Join-Path $repo 'build'
$cmake = $null
foreach ($cand in @(
    'C:\Program Files\CMake\bin\cmake.exe',
    'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
    'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
    "$env:LOCALAPPDATA\Microsoft\WinGet\Links\cmake.exe"
)) {
    if (Test-Path $cand) { $cmake = $cand; break }
}
if (-not $cmake) {
    $cmd = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmd) { $cmake = $cmd.Source }
}
if (-not $cmake) { throw 'cmake.exe not found' }

$useNinjaMsvc = $false
if ((Test-Path "$env:LOCALAPPDATA\Programs\MSVC\VC\Tools\MSVC") -and -not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    $msvcVer = (Get-ChildItem "$env:LOCALAPPDATA\Programs\MSVC\VC\Tools\MSVC" -Directory | Select-Object -Last 1).Name
    $msvcDir = "$env:LOCALAPPDATA\Programs\MSVC\VC\Tools\MSVC\$msvcVer"
    $sdkDir  = 'C:\Program Files\Microsoft Visual Studio\2022\Community\SDK\ScopeCppSDK\vc15\SDK'
    if (-not (Test-Path "$sdkDir\include\um\Windows.h")) {
        $altSdk = 'C:\Program Files\Microsoft Visual Studio\18\Community\SDK\ScopeCppSDK\vc15\SDK'
        if (Test-Path "$altSdk\include\um\Windows.h") { $sdkDir = $altSdk }
    }
    $ninjaDir = "$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
    if (-not (Test-Path "$ninjaDir\ninja.exe")) { $ninjaDir = "$env:LOCALAPPDATA\Microsoft\WinGet\Links" }
    if (Test-Path "$msvcDir\bin\Hostx64\x64\cl.exe") {
        $env:PATH    = "$msvcDir\bin\Hostx64\x64;$sdkDir\bin;$ninjaDir;$env:PATH"
        $env:INCLUDE = "$msvcDir\include;$sdkDir\include\ucrt;$sdkDir\include\um;$sdkDir\include\shared"
        $env:LIB     = "$msvcDir\lib\x64;$sdkDir\lib"
        $useNinjaMsvc = $true
    }
}

if (-not (Test-Path (Join-Path $build 'CMakeCache.txt'))) {
    Write-Host '== Configuring CMake project =='
    if ($useNinjaMsvc) {
        & $cmake -S $repo -B $build -G 'Ninja' -DCMAKE_BUILD_TYPE=Release '-DCMAKE_EXE_LINKER_FLAGS=/machine:x64 /MANIFEST:NO' '-DCMAKE_SHARED_LINKER_FLAGS=/machine:x64 /MANIFEST:NO'
    } else {
        & $cmake -S $repo -B $build -A x64
    }
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }
}

Write-Host '== Building release client =='
Stop-Process -Name 'Noble','noble-client' -Force -ErrorAction SilentlyContinue
& $cmake --build $build --config Release --target noble-client
if ($LASTEXITCODE -ne 0) { throw 'noble-client build failed' }

$release = Join-Path $build 'bin\Release'
if (-not (Test-Path (Join-Path $release 'noble-client.exe'))) { throw 'missing noble-client.exe' }

# Ensure repo data\, assets\ and tools\vgmstream\ are synced into build\bin\Release before packaging
$repoData = Join-Path $repo 'data'
if (Test-Path $repoData) {
    & robocopy $repoData (Join-Path $release 'data') /E /XO /XD userdata autosave mods cache sandbox /NFL /NDL /NJH /NJS /NP | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "robocopy data failed ($LASTEXITCODE)" }
}
$repoAssets = Join-Path $repo 'assets'
if (Test-Path $repoAssets) {
    & robocopy $repoAssets (Join-Path $release 'data') /E /XO /NFL /NDL /NJH /NJS /NP | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "robocopy assets failed ($LASTEXITCODE)" }
}
$repoVgm = Join-Path $repo 'tools\vgmstream'
if (Test-Path $repoVgm) {
    & robocopy $repoVgm (Join-Path $release 'tools\vgmstream') /E /XO /NFL /NDL /NJH /NJS /NP | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "robocopy vgmstream failed ($LASTEXITCODE)" }
}

Write-Host "== Packaging to $Destination =="
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
# Copy the complete working runtime folder. Selective copies kept missing a startup
# dependency that only the full build tree had. Exclude linker .map files (noble-*.map)
# without excluding data\menumaps\mainmenu_all.map.
& robocopy $release $Destination /E /XF *.log noble-*.map *.bmp *.dmp /NFL /NDL /NJH /NJS /NP
if ($LASTEXITCODE -ge 8) { throw "robocopy failed ($LASTEXITCODE)" }

Copy-Item (Join-Path $Destination 'noble-client.exe') (Join-Path $Destination 'Noble.exe') -Force
foreach ($extra in @('noble-client.exe','noble-dev.exe','noble-dedicated.exe','NobleTest.exe','dedicated_server.json')) {
    Remove-Item (Join-Path $Destination $extra) -Force -ErrorAction SilentlyContinue
}
if (Test-Path (Join-Path $Destination 'dedicated_data')) {
    Remove-Item (Join-Path $Destination 'dedicated_data') -Recurse -Force -ErrorAction SilentlyContinue
}

Set-Content (Join-Path $Destination 'play.bat') "@echo off`r`ncd /d `"%~dp0`"`r`nstart `"`" `"Noble.exe`"`r`n"
$versionMatch = Select-String -Path (Join-Path $repo 'src\common\Version.hpp') -Pattern 'VERSION_STRING = "([0-9]+\.[0-9]+\.[0-9]+) \(([^)]*)\)"'
$version = if ($versionMatch) { "v" + $versionMatch.Matches[0].Groups[1].Value + " (" + $versionMatch.Matches[0].Groups[2].Value + ")" } else { "dev" }
Set-Content (Join-Path $Destination 'version.txt') ("Noble " + $version + " playtest build " + (Get-Date -Format 'yyyy-MM-dd HH:mm'))
Write-Host '== Done =='
Write-Host "Executable: $(Join-Path $Destination 'Noble.exe')"
$global:LASTEXITCODE = 0
exit 0
