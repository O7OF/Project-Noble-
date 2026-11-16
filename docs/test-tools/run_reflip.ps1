# Backs up blend + render_model, runs uvreflip_hg.py (save + Foundry export), restores elite_noble.model, rebuilds the menu
# map, copies it to data\menumaps\noble_menu.map and captures the Armory (muted, second monitor).
$H = 'D:\Steam\steamapps\common\HREK'
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry"
$MM = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\menumaps'
$E = "$H\data\objects\characters\elite_noble"
Copy-Item "$E\elite_noble.blend" "$Bk\elite_noble.blend.before-hgreflip" -Force
Copy-Item "$H\tags\objects\characters\elite_noble\elite_noble.render_model" "$Bk\elite_noble.render_model.before-hgreflip" -Force
Remove-Item "$T\uvreflip_log.txt" -ErrorAction SilentlyContinue
$start = Get-Date
Start-Process -FilePath "$env:LOCALAPPDATA\Microsoft\WindowsApps\blender-launcher.exe" -ArgumentList @('--background', "$E\elite_noble.blend", '--python', "$T\tools\uvreflip_hg.py", '--', '--export') -WindowStyle Hidden | Out-Null
while (((Get-Date) - $start).TotalMinutes -lt 25) { if ((Test-Path "$T\uvreflip_log.txt") -and ((Get-Content "$T\uvreflip_log.txt") -contains 'DONE')) { break }; Start-Sleep 10 }
Get-Content "$T\uvreflip_log.txt"
Start-Sleep 20
Copy-Item "$Bk\elite_noble.model" "$H\tags\objects\characters\elite_noble\elite_noble.model" -Force; "model restored"
& "$T\tools\build_map.ps1" -Scenario 'levels\ui\mainmenu\mainmenu' -Log "$T\build_menu_hgreflip.txt" | Select-Object -First 1
Copy-Item "$H\maps\mainmenu.map" "$MM\noble_menu.map" -Force
& "$T\run-quiet.ps1" -Name $args[0] -Species elite
"total " + [int]((Get-Date) - $start).TotalSeconds + " s"
