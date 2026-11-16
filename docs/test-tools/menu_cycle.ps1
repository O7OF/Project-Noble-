param([string]$Name)
# Rebuild + install the menu map, refresh the string id CSV, run the Elite piece cycle, make a contact sheet.
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
& "$T\tools\build_map.ps1" -Scenario 'levels\ui\mainmenu\mainmenu' -Log "$T\build_menu_$Name.txt" | Select-Object -First 1
Copy-Item 'D:\Steam\steamapps\common\HREK\maps\mainmenu.map' 'D:\Project-Noble-Closed-Source\build\bin\Release\data\menumaps\noble_menu.map' -Force
& "$T\tools\menu_sidcsv.ps1" | Select-Object -First 1
& "$T\run-armory-test.ps1" -Name $Name -Species elite -Cycle elitepieces -Delay 9000 -At '22,28,34,40' -Quit 46 | Out-Null
& "$T\tools\sheet.ps1" -Pattern "$($Name)_*.jpg" -Out "$($Name)_sheet.png" -Crop '0.62,0.12,0.28,0.80' -TileW 330
