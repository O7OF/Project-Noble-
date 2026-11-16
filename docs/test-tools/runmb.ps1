param([string]$Script, [string]$Out, [string]$Env = '', [int]$Wait = 1500)
# Like runpy.ps1 but WITHOUT --factory-startup so Foundry's pythonnet (clr) is available for ManagedBlam.
$Dir = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$OutPath = Join-Path $Dir $Out
if (Test-Path $OutPath) { Remove-Item $OutPath }
$env:WRAP_SCRIPT = $Script; $env:WRAP_OUT = $Out; $env:OUT = $Out
foreach ($kv in ($Env -split ';')) { if ($kv -match '^(\w+)=(.*)$') { Set-Item -Path ("env:" + $Matches[1]) -Value $Matches[2] } }
$Exe = Join-Path $env:LOCALAPPDATA 'Microsoft\WindowsApps\blender-launcher.exe'
$P = Start-Process -FilePath $Exe -ArgumentList @('--background','--python',(Join-Path $Dir 'tools\wrap.py')) -PassThru -WindowStyle Hidden
$start = Get-Date
while (-not (Test-Path $OutPath) -and ((Get-Date) - $start).TotalSeconds -lt $Wait) { Start-Sleep 2 }
Start-Sleep 1
"elapsed " + [int]((Get-Date) - $start).TotalSeconds + " s"
Get-Content $OutPath -TotalCount 400
