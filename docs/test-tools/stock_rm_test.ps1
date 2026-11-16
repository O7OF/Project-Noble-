# Armory shine comparison: render the STOCK elite.render_model through elite_noble.model in the same Armory scene,
# capture, then restore elite_noble's render model and the menu map. Backups: noble_menu.map.before-stocktest.
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$H = 'D:\Steam\steamapps\common\HREK'
$MM = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\menumaps'
$Blender = Join-Path $env:LOCALAPPDATA 'Microsoft\WindowsApps\blender-launcher.exe'
function SetRm([string]$rm) {
    $p = Start-Process -FilePath $Blender -ArgumentList @('--background', '--python', "$T\tools\set_model_rm.py", '--', $rm) -PassThru -WindowStyle Hidden
    $p.WaitForExit(300000) | Out-Null
    $start = Get-Date
    while (-not (Select-String -Path "$T\set_model_rm_log.txt" -Pattern 'DONE' -Quiet -ErrorAction SilentlyContinue) -and ((Get-Date) - $start).TotalSeconds -lt 120) { Start-Sleep 2 }
    Get-Content "$T\set_model_rm_log.txt"
}
function BuildMenu([string]$tag) {
    & "$T\tools\build_map.ps1" -Scenario 'levels\ui\mainmenu\mainmenu' -Log "$T\build_menu_$tag.txt" | Select-Object -First 1
    Copy-Item "$H\maps\mainmenu.map" "$MM\noble_menu.map" -Force
}
Copy-Item "$MM\noble_menu.map" "$MM\noble_menu.map.before-stocktest" -Force
Copy-Item "$H\tags\objects\characters\elite_noble\elite_noble.model" "$T\elite_noble.model.before-stocktest" -Force
"--- switch to stock render model"; Remove-Item "$T\set_model_rm_log.txt" -ErrorAction SilentlyContinue
SetRm 'objects\characters\elite\elite.render_model'
BuildMenu 'stock'
& "$T\run-armory-test.ps1" -Name stockrm -Species elite -At '24,32' -Quit 40
"--- restore"
Copy-Item "$T\elite_noble.model.before-stocktest" "$H\tags\objects\characters\elite_noble\elite_noble.model" -Force
Copy-Item "$MM\noble_menu.map.before-stocktest" "$MM\noble_menu.map" -Force
"restored model tag + menu map (noble_menu.map is the PRT build again)"
Copy-Item "$T\fxB_capture_900006.jpg" "$T\cmp2_b_ours_prt.jpg" -Force
Copy-Item "$T\stockrm_capture_900001.jpg" "$T\cmp2_c_stock.jpg" -Force -ErrorAction SilentlyContinue
& "$T\tools\sheet.ps1" -Pattern 'cmp2_*.jpg' -Out 'cmp2_sheet.png' -Crop '0.62,0.12,0.28,0.80' -TileW 520
