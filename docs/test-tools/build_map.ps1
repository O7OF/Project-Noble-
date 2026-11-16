param([string]$Scenario, [string]$Log)
Set-Location D:\Steam\steamapps\common\HREK
$start = Get-Date
& .\tool.exe build-cache-file $Scenario pc english *> $Log
"exit $LASTEXITCODE secs " + [int]((Get-Date) - $start).TotalSeconds
Get-Content $Log | Select-String -Pattern 'error|warning|failed|string_id' | Select-Object -Last 15 | ForEach-Object { $_.Line }
Get-ChildItem .\maps\*.map | Sort-Object LastWriteTime -Descending | Select-Object -First 2 | ForEach-Object { "{0} {1} {2}" -f $_.Name, $_.Length, $_.LastWriteTime }
