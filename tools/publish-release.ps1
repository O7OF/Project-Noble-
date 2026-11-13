param(
    [string]$Repo = 'O7OF/Project-Noble-',
    [string]$Version = '',
    [switch]$SkipBuild,
    [switch]$NoUpload
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$release = Join-Path $root 'build\bin\Release'
$dist = Join-Path $root 'dist'
$pkg = Join-Path $dist 'Project-Noble'

function Get-Version {
    $line = Select-String -Path (Join-Path $root 'src\common\Version.hpp') -Pattern 'VERSION_STRING = "([0-9]+\.[0-9]+\.[0-9]+(?:\.[0-9]+)?)' | Select-Object -First 1
    if (-not $line) { throw 'cannot parse the version from src/common/Version.hpp' }
    return $line.Matches[0].Groups[1].Value
}

if (-not $Version) { $Version = Get-Version }
$tag = 'v' + $Version
$assetName = 'Project-Noble-' + $tag + '-win64.zip'
Write-Host "== Project Noble release $tag =="

if (-not $SkipBuild) {
    Write-Host '== Building noble-client =='
    & cmake --build (Join-Path $root 'build') --config Release --target noble-client --parallel 16
    if ($LASTEXITCODE -ne 0) { throw 'noble-client build failed' }
}

$exe = Join-Path $release 'noble-client.exe'
if (-not (Test-Path $exe)) { throw "missing $exe" }

Write-Host '== Staging the portable package =='
if (Test-Path $pkg) { Remove-Item $pkg -Recurse -Force }
New-Item -ItemType Directory -Force -Path $pkg | Out-Null
Copy-Item $exe (Join-Path $pkg 'Noble.exe') -Force

# Ship data\menumaps\mainmenu_all.map, data\armor, and tools\vgmstream inside the release zip
# so the 3D Armory and custom audio work out of the box. Never ship dev builds, userdata, save files, appearance.dat, or career.dat.
$excludeDirs = 'userdata','autosave','cache','sandbox','blocks','mods','menumaps','armor'
$excludeFiles = 'appearance*','career*','settings*','preferences*','*.pncsold','menu-background-deck.txt','launch_block_1.bin','lb_m_c_1.bin','profile_1.bin','*.log','*.dmp','*.tmp','*.bmp','*.obj','*.mtl','*.PMGC','*.rdc','*.map','*.csv','*.bin'
& robocopy (Join-Path $release 'data') (Join-Path $pkg 'data') /E /XD $excludeDirs /XF $excludeFiles /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -gt 7) { throw "robocopy data failed ($LASTEXITCODE)" }
New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'data\mods') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'data\limits') | Out-Null

# Copy the armor folder (rr_elite_pack.map, .rrreloc, etc.) into data\armor\
$srcArmor = Join-Path $root 'data\armor'
if (-not (Test-Path (Join-Path $srcArmor 'rr_elite_pack.map'))) {
    $srcArmor = Join-Path $release 'data\armor'
}
if (Test-Path (Join-Path $srcArmor 'rr_elite_pack.map')) {
    New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'data\armor') | Out-Null
    & robocopy $srcArmor (Join-Path $pkg 'data\armor') /E /NFL /NDL /NJH /NJS /NP | Out-Null
    if ($LASTEXITCODE -gt 7) { throw "robocopy armor failed ($LASTEXITCODE)" }
} else {
    Write-Warning "data\armor\rr_elite_pack.map was not found to package"
}

# Copy the public Armory menu map mod and its string_id tables into data\menumaps\
$srcMenuMaps = Join-Path $root 'data\menumaps'
if (-not (Test-Path (Join-Path $srcMenuMaps 'mainmenu_all.map'))) {
    $srcMenuMaps = Join-Path $release 'data\menumaps'
}
if (-not (Test-Path (Join-Path $srcMenuMaps 'mainmenu_all.map'))) {
    throw "missing mainmenu_all.map in data\menumaps (required for Armory in release zip)"
}
New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'data\menumaps') | Out-Null
& robocopy $srcMenuMaps (Join-Path $pkg 'data\menumaps') mainmenu_all.map *.string_ids.csv /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -gt 7) { throw "robocopy menumaps failed ($LASTEXITCODE)" }

# Guarantee zero dev binaries or user save/appearance/career files in the staged release directory
Get-ChildItem -Path $pkg -Recurse -Include "noble-dev*","noble-dedicated*","noble-client*","*.pdb","appearance*","career*","settings*","preferences*","*.pncsold","menu-background-deck.txt" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue

$srcVgm = Join-Path $root 'tools\vgmstream'
if (-not (Test-Path (Join-Path $srcVgm 'vgmstream-cli.exe'))) {
    $srcVgm = Join-Path $release 'tools\vgmstream'
}
if (Test-Path (Join-Path $srcVgm 'vgmstream-cli.exe')) {
    New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'tools\vgmstream') | Out-Null
    & robocopy $srcVgm (Join-Path $pkg 'tools\vgmstream') /E /NFL /NDL /NJH /NJS /NP | Out-Null
    if ($LASTEXITCODE -gt 7) { throw "robocopy vgmstream failed ($LASTEXITCODE)" }
} else {
    Write-Warning "tools\vgmstream\vgmstream-cli.exe was not found to package"
}

$playLines = @('@echo off', 'cd /d "%~dp0"', 'start "" "Noble.exe"')
Set-Content -Path (Join-Path $pkg 'play.bat') -Value ($playLines -join [Environment]::NewLine) -Encoding ASCII
Set-Content -Path (Join-Path $pkg 'version.txt') -Value ('Noble ' + $Version + ' release build ' + (Get-Date -Format 'yyyy-MM-dd HH:mm')) -Encoding ASCII
$readme = @(
    ('Project Noble ' + $Version),
    '',
    'A standalone Halo: Reach client for Halo: The Master Chief Collection on Steam.',
    '',
    'Requirements:',
    '  - Windows 10 or 11 (64-bit)',
    '  - Steam with Halo: The Master Chief Collection and the Halo: Reach DLC installed',
    '  - A Direct3D 11 graphics card',
    '',
    'Run Noble.exe (or play.bat). The client checks this release repository for updates',
    'when the menu opens; an UPDATE AVAILABLE panel appears when a newer build exists.',
    '',
    'Your settings and appearance live in data\userdata and are never replaced by',
    'an update.',
    '',
    'Releases: https://github.com/O7OF/Project-Noble-/releases'
)
Set-Content -Path (Join-Path $pkg 'README.txt') -Value ($readme -join [Environment]::NewLine) -Encoding UTF8

Write-Host '== Creating the zip =='
New-Item -ItemType Directory -Force -Path $dist | Out-Null
$zip = Join-Path $dist $assetName
if (Test-Path $zip) { Remove-Item $zip -Force }
Push-Location $dist
& tar.exe -a -c -f $zip 'Project-Noble'
$tarExit = $LASTEXITCODE
Pop-Location
if ($tarExit -ne 0 -or -not (Test-Path $zip)) {
    Write-Host 'tar failed; falling back to Compress-Archive'
    Compress-Archive -Path $pkg -DestinationPath $zip -CompressionLevel Optimal -Force
}
$sizeMb = [math]::Round((Get-Item $zip).Length / 1MB, 1)
$hash = (Get-FileHash -Path $zip -Algorithm SHA256).Hash.ToLower()
$hashFile = $zip + '.sha256'
Set-Content -Path $hashFile -Value ($hash + '  ' + $assetName) -Encoding ASCII
Write-Host ('Zip: ' + $zip + ' (' + $sizeMb + ' MB)')
Write-Host ('SHA256: ' + $hash)

if ($NoUpload) {
    Write-Host '== NoUpload: package is ready in dist/ =='
    exit 0
}

Write-Host '== Creating the GitHub release =='
$credentialRequest = @('protocol=https', 'host=github.com', '', '') -join [char]10
$rawCredential = $credentialRequest | git credential fill 2>$null
$token = ($rawCredential | Where-Object { $_ -like 'password=*' }) -replace '^password=', ''
if (-not $token) { $token = $env:GITHUB_TOKEN }
if (-not $token) { throw 'no GitHub credential (run git credential fill or set GITHUB_TOKEN)' }
$headers = @{ Authorization = 'Bearer ' + $token; 'User-Agent' = 'ProjectNoble-Release'; Accept = 'application/vnd.github+json' }

$notesFile = Join-Path $root ('release-notes\' + $tag + '.md')
$notes = if (Test-Path $notesFile) { Get-Content $notesFile -Raw } else { 'Project Noble ' + $Version + ' release.' }

$existing = $null
try {
    $existing = Invoke-RestMethod -Uri ('https://api.github.com/repos/' + $Repo + '/releases/tags/' + $tag) -Headers $headers -TimeoutSec 30
} catch {
    $existing = $null
}
if ($existing) {
    Write-Host ('Replacing existing release ' + $tag)
    Invoke-RestMethod -Method Delete -Uri ('https://api.github.com/repos/' + $Repo + '/releases/' + $existing.id) -Headers $headers -TimeoutSec 30 | Out-Null
}

# Create a draft first: assets can be uploaded and verified before the release goes public.
# The notes are sanitised to ASCII because GitHub's release PATCH rejects some mis-decoded
# non-ASCII text that Windows PowerShell 5.1 reads from the markdown file.
$body = @{
    tag_name = $tag
    target_commitish = 'main'
    name = 'Project Noble ' + $Version
    draft = $true
    prerelease = $false
} | ConvertTo-Json -Depth 5
$releaseInfo = Invoke-RestMethod -Method Post -Uri ('https://api.github.com/repos/' + $Repo + '/releases') -Headers $headers -Body $body -ContentType 'application/json' -TimeoutSec 60
Write-Host ('Draft: ' + $releaseInfo.html_url)

Write-Host '== Uploading the zip =='
$uploadUrl = 'https://uploads.github.com/repos/' + $Repo + '/releases/' + $releaseInfo.id + '/assets?name=' + [uri]::EscapeDataString($assetName)
& curl.exe -sS -X POST -H ('Authorization: Bearer ' + $token) -H 'Content-Type: application/zip' --data-binary ('@' + $zip) $uploadUrl
if ($LASTEXITCODE -ne 0) { throw 'zip upload failed' }

Write-Host '== Uploading the checksum =='
$checksumUrl = 'https://uploads.github.com/repos/' + $Repo + '/releases/' + $releaseInfo.id + '/assets?name=' + [uri]::EscapeDataString($assetName + '.sha256')
& curl.exe -sS -X POST -H ('Authorization: Bearer ' + $token) -H 'Content-Type: text/plain' --data-binary ('@' + $hashFile) $checksumUrl
if ($LASTEXITCODE -ne 0) { throw 'checksum upload failed' }

Write-Host '== Publishing =='
$notesAscii = $notes -replace '[^\x09\x0A\x0D\x20-\x7E]', '-'
$publishBody = @{ draft = $false; prerelease = $false; body = $notesAscii } | ConvertTo-Json -Depth 5
$published = Invoke-RestMethod -Method Patch -Uri ('https://api.github.com/repos/' + $Repo + '/releases/' + $releaseInfo.id) -Headers $headers -Body $publishBody -ContentType 'application/json' -TimeoutSec 60
Write-Host ('Release: ' + $published.html_url)

Write-Host '== Done =='
Write-Host ('Download: https://github.com/' + $Repo + '/releases/download/' + $tag + '/' + $assetName)
