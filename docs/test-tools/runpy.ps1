param([string]$Script, [string]$Out, [string]$Env = '')
$Dir = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$OutPath = Join-Path $Dir $Out
if (Test-Path $OutPath) { Remove-Item $OutPath }
$env:WRAP_SCRIPT = $Script; $env:WRAP_OUT = $Out
foreach ($kv in ($Env -split ';')) { if ($kv -match '^(\w+)=(.*)$') { Set-Item -Path ("env:" + $Matches[1]) -Value $Matches[2] } }
$Exe = Join-Path $env:LOCALAPPDATA 'Microsoft\WindowsApps\blender-launcher.exe'
$P = Start-Process -FilePath $Exe -ArgumentList @('--background','--factory-startup','--python',(Join-Path $Dir 'tools\wrap.py')) -PassThru -WindowStyle Hidden
$start = Get-Date
while (-not (Test-Path $OutPath) -and ((Get-Date) - $start).TotalSeconds -lt 600) { Start-Sleep 2 }
Start-Sleep 1
Get-Content $OutPath
