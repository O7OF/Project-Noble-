param(
    [string]$OutDir = [Environment]::GetFolderPath('Desktop')
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path $PSScriptRoot -Parent
$staging = Join-Path $env:TEMP 'Noble-Source'
if (Test-Path $staging) { Remove-Item $staging -Recurse -Force }
New-Item -ItemType Directory -Force -Path $staging | Out-Null
# The two large runtime mods stay out of the source zip; the full package script includes them.
& robocopy $repo $staging /E /XD .git build data\armor data\userdata data\sandbox data\autosave data\mods /XF mainmenu_all.map appearance.dat settings.json career.json *.log *.dmp *.tmp *.zip /NFL /NDL /NJH /NJS /NP | Out-Null
if ($LASTEXITCODE -gt 7) { throw "robocopy failed ($LASTEXITCODE)" }
$zip = Join-Path $OutDir 'Noble-Source.zip'
if (Test-Path $zip) { Remove-Item $zip -Force }
Compress-Archive -Path (Join-Path $staging '*') -DestinationPath $zip -CompressionLevel Optimal
Write-Host ("Source zip: {0} ({1:N1} MB) - under GitHub's 100 MB file limit" -f $zip, ((Get-Item $zip).Length / 1MB))
