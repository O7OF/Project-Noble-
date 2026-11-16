# One-shot menu test: rebuild menu map, copy over noble_menu.map, capture ONE pinned Elite look (full res) and crop.
# Usage: menu_test.ps1 -Name x [-Look 2] [-At 24] [-Crop '0.72,0.18,0.20,0.30'] [-NoBuild] [-Colors]
#   Look: 1 Honor Guard, 2 Counselor (council/schism/HG chest), 3 Ossoona+Combat, 4 Minor. -Colors: White/Cobalt/Gold lights.
param([string]$Name, [int]$Look = 2, [int]$At = 24, [string]$Crop = '0.72,0.18,0.20,0.30', [switch]$NoBuild, [switch]$Colors, [string]$More = '')
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$H = 'D:\Steam\steamapps\common\HREK'
$Dst = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\menumaps\noble_menu.map'
Get-Process noble-dev -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep 2
if (-not $NoBuild) {
  & "$T\tools\build_map.ps1" -Scenario 'levels\ui\mainmenu\mainmenu' -Log "$T\build_$Name.log" | Select-Object -First 1
  Copy-Item "$H\maps\mainmenu.map" $Dst -Force
}
$extra = "-armory-look $Look"; if ($Colors) { $extra += ' -armory-test-colors' }; if ($More) { $extra += " $More" }
& "$T\run-quiet-full.ps1" -Name $Name -Species elite -Cycle elitepieces -Delay 4000 -At "$At" -Quit ($At + 3) -Extra $extra | Out-Null
& "$T\tools\crop.ps1" -In "$($Name)_capture_900000.jpg" -Out "$($Name)_crop.png" -Crop $Crop -Scale 1.5
