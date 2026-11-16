# A/B: Armory piece cycle with the render model from BEFORE tonight's UV/PRT changes (.before-prt), then restore.
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$H = 'D:\Steam\steamapps\common\HREK'
$RM = "$H\tags\objects\characters\elite_noble\elite_noble.render_model"
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry"
$MM = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\menumaps'
function Menu([string]$tag) {
    & "$T\tools\build_map.ps1" -Scenario 'levels\ui\mainmenu\mainmenu' -Log "$T\build_menu_$tag.txt" | Select-Object -First 1
    Copy-Item "$H\maps\mainmenu.map" "$MM\noble_menu.map" -Force
    & "$T\tools\menu_sidcsv.ps1" | Select-Object -Last 4
}
function Cycle([string]$name) {
    & "$T\run-armory-test.ps1" -Name $name -Species elite -Cycle elitepieces -Delay 9000 -At '22,28,34,40,46,52,58' -Quit 62 | Out-Null
    & "$T\tools\sheet.ps1" -Pattern "$($name)_*.jpg" -Out "$($name)_sheet.png" -Crop '0.62,0.12,0.28,0.80' -TileW 300
}
Copy-Item $RM "$Bk\elite_noble.render_model.current-phunflip" -Force
Copy-Item "$Bk\elite_noble.render_model.before-prt" $RM -Force
"--- old render model"; Menu 'oldrm'; Cycle 'pold'
Copy-Item "$Bk\elite_noble.render_model.current-phunflip" $RM -Force
"--- current render model"; Menu 'currm'; Cycle 'pnew'
Get-Content 'D:\Project-Noble-Closed-Source\build\bin\Release\noble-dev.log' -Tail 3000 | Where-Object { $_ -match 'ArmoryCycle' } | Select-Object -Last 6
