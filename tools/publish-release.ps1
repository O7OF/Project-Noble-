param(
    [string]$Repo = 'O7OF/Project-Noble-',
    [string]$Version = '',
    [switch]$SkipBuild,
    [switch]$NoUpload,
    # Forge World is ~760 MB of uncompressed map data; it ships as its own optional asset so the base release stays
    # small. -NoForgePack skips that second asset.
    [switch]$NoForgePack
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
$excludeDirs = 'userdata','autosave','cache','sandbox','blocks','mods','menumaps','armor','armor_noble_parked'
$excludeFiles = 'appearance*','career*','settings*','preferences*','*.pncsold','menu-background-deck.txt','launch_block_1.bin','lb_m_c_1.bin','profile_1.bin','*.log','*.dmp','*.tmp','*.bmp','*.obj','*.mtl','*.PMGC','*.rdc','*.map','*.csv','*.bin','*.bak*','*.before*','*.old','*.map.*'
& robocopy (Join-Path $release 'data') (Join-Path $pkg 'data') /E /XD $excludeDirs /XF $excludeFiles /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -gt 7) { throw "robocopy data failed ($LASTEXITCODE)" }
New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'data\mods') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'data\limits') | Out-Null
# Engine limits are PINNED to Full (src/host/EngineLimits.cpp, limits::kShippedProfile): every install must run the
# patched layout, so the release has to carry the patch file. A missing .nlp would silently fall back to the stock DLL.
$pinnedNlp = Join-Path $pkg 'data\limits\full.nlp'
if (-not (Test-Path $pinnedNlp)) {
    throw "data\limits\full.nlp is missing from the package - the pinned engine limits cannot run without it. Generate/copy data\limits from the build folder (tools\limits\make_v16_particles.py) before packaging."
}

# Copy the armour pack (elite_armor.map + its .noblereloc relocation sidecar) into data\armor\.
# Noble's own naming/format: no Reach Renaissance file names ship in the package.
$srcArmor = Join-Path $root 'data\armor'
if (-not (Test-Path (Join-Path $srcArmor 'elite_armor.map'))) {
    $srcArmor = Join-Path $release 'data\armor'
}
New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'data\armor') | Out-Null
foreach ($name in 'elite_armor.map', 'elite_armor.map.noblereloc') {
    $src = Join-Path $srcArmor $name
    if (Test-Path $src) {
        Copy-Item $src (Join-Path $pkg ('data\armor\' + $name)) -Force
    } else {
        throw "data\armor\$name was not found to package"
    }
}

# Copy the Armory menu map and its string_id tables into data\menumaps\.
# Only noble_menu.map is needed: it is the rebuilt menu map (Armory scripts / GLOBALS tables / Spartan, Elite,
# Marine and Female Marine previews, with the campaign marine model and animation embedded). VFS serves it as
# haloreach\maps\mainmenu.map and prefers it over mainmenu_all.map. Tested 2026-11-14 with mainmenu_all.map absent:
# the Armory and the Marine tab work. mainmenu_all.map (285 MB) is no longer shipped.
$srcMenuMaps = Join-Path $root 'data\menumaps'
$relMenuMaps = Join-Path $release 'data\menumaps'
New-Item -ItemType Directory -Force -Path (Join-Path $pkg 'data\menumaps') | Out-Null
$copiedMaps = @()
foreach ($name in 'noble_menu.map') {
    $src = Join-Path $srcMenuMaps $name
    if (-not (Test-Path $src)) { $src = Join-Path $relMenuMaps $name }
    if (Test-Path $src) {
        Copy-Item $src (Join-Path $pkg ('data\menumaps\' + $name)) -Force
        $copiedMaps += $name
    }
}
if ($copiedMaps -notcontains 'noble_menu.map') {
    throw "missing noble_menu.map in data\menumaps (required for the Armory and the Marine previews in the release zip)"
}
# Loose string_id tables next to the maps (noble_menu.string_ids.csv is the one the Armory reads; the
# mainmenu_all tables are kept because they are small and older installs may still reference them).
foreach ($dir in $srcMenuMaps, $relMenuMaps) {
    if (-not (Test-Path $dir)) { continue }
    Get-ChildItem $dir -Filter '*.string_ids.csv' | ForEach-Object {
        Copy-Item $_.FullName (Join-Path $pkg ('data\menumaps\' + $_.Name)) -Force
    }
}

# Noble's Forge World (Pelican, Spectre, Boathog, targeting laser in the Forge palette) is a 760 MB uncompressed
# HREK map. It is NOT part of the base package: it ships as a separate optional asset (see the forge pack below) so
# players who never play Forge World do not download it. Only the map path is remembered here.
$srcForge = Join-Path $root 'data\maps\forge_halo.map'
if (-not (Test-Path $srcForge)) { $srcForge = Join-Path $release 'data\maps\forge_halo.map' }
if (-not (Test-Path $srcForge)) { $srcForge = $null; Write-Warning "data\maps\forge_halo.map (Noble Forge World) was not found to package" }

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
    'Forge World: the rebuilt Forge World map (Pelican, Spectre, Boathog, targeting',
    'laser) ships as a separate optional download ("ForgeWorld" zip). Extract its',
    'data folder over this installation to play/forge on that map; everyone in a',
    'Forge World lobby needs the same pack.',
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

# == Optional Forge World asset (the 760 MB map stays out of the base package) ==
$forgeZip = $null
$forgeHashFile = $null
$forgeAssetName = 'Noble-ForgeWorld-' + $tag + '-win64.zip'
if (-not $NoForgePack -and $srcForge) {
    Write-Host '== Creating the optional Forge World pack =='
    $forgePkg = Join-Path $env:TEMP 'noble-forge-world-pack'
    if (Test-Path $forgePkg) { Remove-Item $forgePkg -Recurse -Force }
    New-Item -ItemType Directory -Force -Path (Join-Path $forgePkg 'data\maps') | Out-Null
    Copy-Item $srcForge (Join-Path $forgePkg 'data\maps\forge_halo.map') -Force
    $specFsb = Join-Path $release 'data\fmod\pc\sfx.st_spectre.fsb'
    $specInfo = Join-Path $release 'data\fmod\pc\sfx.st_spectre.fsb.info'
    if (Test-Path $specFsb) {
        New-Item -ItemType Directory -Force -Path (Join-Path $forgePkg 'data\fmod\pc') | Out-Null
        Copy-Item $specFsb (Join-Path $forgePkg 'data\fmod\pc\sfx.st_spectre.fsb') -Force
        if (Test-Path $specInfo) { Copy-Item $specInfo (Join-Path $forgePkg 'data\fmod\pc\sfx.st_spectre.fsb.info') -Force }
    } else {
        Write-Warning 'Spectre sound bank missing from data\fmod\pc'
    }
    $forgeReadme = @(
        ('Project Noble ' + $Version + ' - optional Forge World pack'),
        '',
        'Copy the data folder in this zip over your Project Noble installation',
        '(the folder that contains Noble.exe). It adds:',
        '  data\maps\forge_halo.map        Noble Forge World (Pelican, Spectre, Boathog,',
        '                                  targeting laser in the Forge palette)',
        '  data\fmod\pc\sfx.st_spectre.fsb Spectre sound bank',
        '',
        ('Requires Project Noble ' + $Version + ' or newer. Everyone in a Forge World'),
        'lobby needs this same pack so the maps match.'
    )
    Set-Content -Path (Join-Path $forgePkg 'FORGE-WORLD-README.txt') -Value ($forgeReadme -join [Environment]::NewLine) -Encoding UTF8
    $forgeZip = Join-Path $dist $forgeAssetName
    if (Test-Path $forgeZip) { Remove-Item $forgeZip -Force }
    Push-Location $forgePkg
    & tar.exe -a -c -f $forgeZip 'data' 'FORGE-WORLD-README.txt'
    $forgeTarExit = $LASTEXITCODE
    Pop-Location
    if ($forgeTarExit -ne 0 -or -not (Test-Path $forgeZip)) {
        Compress-Archive -Path (Join-Path $forgePkg '*') -DestinationPath $forgeZip -CompressionLevel Fastest -Force
    }
    $forgeHash = (Get-FileHash -Path $forgeZip -Algorithm SHA256).Hash.ToLower()
    $forgeHashFile = $forgeZip + '.sha256'
    Set-Content -Path $forgeHashFile -Value ($forgeHash + '  ' + $forgeAssetName) -Encoding ASCII
    Write-Host ('Forge World pack: ' + $forgeZip + ' (' + [math]::Round((Get-Item $forgeZip).Length / 1MB, 1) + ' MB)')
}
Write-Host ('Zip: ' + $zip + ' (' + $sizeMb + ' MB)')
Write-Host ('SHA256: ' + $hash)

if ($NoUpload) {
    Write-Host '== NoUpload: packages are ready in dist/ =='
    if ($forgeZip) { Write-Host ('   base:  ' + $zip + ' (' + $sizeMb + ' MB)') ; Write-Host ('   forge: ' + $forgeZip) }
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

if ($forgeZip) {
    Write-Host '== Uploading the optional Forge World pack =='
    $forgeUploadUrl = 'https://uploads.github.com/repos/' + $Repo + '/releases/' + $releaseInfo.id + '/assets?name=' + [uri]::EscapeDataString($forgeAssetName)
    & curl.exe -sS -X POST -H ('Authorization: Bearer ' + $token) -H 'Content-Type: application/zip' --data-binary ('@' + $forgeZip) $forgeUploadUrl
    if ($LASTEXITCODE -ne 0) { throw 'forge pack upload failed' }
    $forgeChecksumUrl = 'https://uploads.github.com/repos/' + $Repo + '/releases/' + $releaseInfo.id + '/assets?name=' + [uri]::EscapeDataString($forgeAssetName + '.sha256')
    & curl.exe -sS -X POST -H ('Authorization: Bearer ' + $token) -H 'Content-Type: text/plain' --data-binary ('@' + $forgeHashFile) $forgeChecksumUrl
    if ($LASTEXITCODE -ne 0) { throw 'forge checksum upload failed' }
}

Write-Host '== Publishing =='
$notesAscii = $notes -replace '[^\x09\x0A\x0D\x20-\x7E]', '-'
$publishBody = @{ draft = $false; prerelease = $false; body = $notesAscii } | ConvertTo-Json -Depth 5
$published = Invoke-RestMethod -Method Patch -Uri ('https://api.github.com/repos/' + $Repo + '/releases/' + $releaseInfo.id) -Headers $headers -Body $publishBody -ContentType 'application/json' -TimeoutSec 60
Write-Host ('Release: ' + $published.html_url)

Write-Host '== Done =='
Write-Host ('Download: https://github.com/' + $Repo + '/releases/download/' + $tag + '/' + $assetName)
