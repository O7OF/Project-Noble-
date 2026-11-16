$H = 'D:\Steam\steamapps\common\HREK'
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry"
$MM = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\menumaps'
$A = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\armor'
Copy-Item "$H\data\objects\characters\elite_noble\elite_noble.blend" "$Bk\elite_noble.blend.before-phunflip" -Force
Remove-Item "$T\uvunflip_log.txt" -ErrorAction SilentlyContinue
$start = Get-Date
Start-Process -FilePath "$env:LOCALAPPDATA\Microsoft\WindowsApps\blender-launcher.exe" -ArgumentList @('--background', "$H\data\objects\characters\elite_noble\elite_noble.blend", '--python', "$T\tools\uvunflip_ph.py", '--', '--export') -WindowStyle Hidden | Out-Null
while (((Get-Date) - $start).TotalMinutes -lt 25) { if ((Test-Path "$T\uvunflip_log.txt") -and ((Get-Content "$T\uvunflip_log.txt") -contains 'DONE')) { break }; Start-Sleep 10 }
Get-Content "$T\uvunflip_log.txt"
Start-Sleep 20
Copy-Item "$Bk\elite_noble.model" "$H\tags\objects\characters\elite_noble\elite_noble.model" -Force; "model restored"
& "$T\tools\build_map.ps1" -Scenario 'levels\ui\mainmenu\mainmenu' -Log "$T\build_menu_phunflip.txt" | Select-Object -First 1
Copy-Item "$H\maps\mainmenu.map" "$MM\noble_menu.map" -Force
& "$T\run-armory-test.ps1" -Name phu -Species elite -At '24,32' -Quit 40
Copy-Item "$T\phu_capture_900001.jpg" "$T\cmp2_b_phfix.jpg" -Force
& "$T\tools\sheet.ps1" -Pattern 'cmp2_*.jpg' -Out 'cmp4_sheet.png' -Crop '0.62,0.12,0.28,0.80' -TileW 400
& "$T\tools\build_map.ps1" -Scenario 'levels\multi\20_sword_slayer\20_sword_slayer' -Log "$T\build_pack_phunflip.txt" | Select-Object -First 1
Copy-Item "$H\maps\20_sword_slayer.map" "$A\noble_elite_pack.map" -Force
& "$T\tools\runmb.ps1" -Script tagwalk.py -Out tagwalk_noble_ph.txt -Env "MODE=build;A64=8;REC=1;PACK=$A\noble_elite_pack.map" -Wait 1200 | Select-String -Pattern 'wrote|ERROR' | ForEach-Object { $_.Line }
"total " + [int]((Get-Date) - $start).TotalSeconds + " s"
