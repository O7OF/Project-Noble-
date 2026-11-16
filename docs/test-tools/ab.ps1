# Claude A/B helper (2026-11-13): run a quiet Armory test with env switches, print the cycle log, crop the captures.
# ab.ps1 -Name x -Cycle helmet -List '19,21' -Chest 0 -Env 'NOBLE_LIVE_OLD,NOBLE_NO_EMBLEM' [-At '22,42'] [-Quit 45] [-Crop '0.62,0.08,0.22,0.35']
param([string]$Name, [string]$Cycle = 'helmet', [string]$List = '', [int]$Chest = -1, [string]$Env = '',
      [string]$At = '22,42', [int]$Quit = 45, [int]$Delay = 20000, [string]$Crop = '0.62,0.08,0.22,0.35', [string]$More = '')
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$names = @($Env -split ',' | Where-Object { $_ })
foreach ($n in $names) { Set-Item "Env:\$n" '1' }
$extra = $More
if ($List) { $extra += " -armory-cycle-list $List" }
if ($Chest -ge 0) { $extra += " -armory-cycle-chest $Chest" }
try { & "$T\run-quiet-full.ps1" -Name $Name -Species spartan -Cycle $Cycle -Delay $Delay -At $At -Quit $Quit -Extra $extra.Trim() | Out-Null }
finally { foreach ($n in $names) { Remove-Item "Env:\$n" -ErrorAction SilentlyContinue } }
Select-String -Path 'D:\Project-Noble-Closed-Source\build\bin\Release\noble-dev.log' -Pattern 'ArmoryCycle|changed live|Spartan pieces head|rebuild \d|capture-at .* reached' |
    Select-Object -Last 8 | ForEach-Object { $_.Line.Substring(0, [Math]::Min(150, $_.Line.Length)) }
Get-ChildItem $T -Filter "$($Name)_capture_9*.jpg" | ForEach-Object {
    & "$T\tools\crop.ps1" -In $_.Name -Out (($_.BaseName -replace '_capture_9000', '_') + '.png') -Crop $Crop | Out-Null
}
Get-ChildItem $T -Filter "$($Name)_0*.png" | ForEach-Object { $_.Name }
