$Dir = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$Armor = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\armor'
$RrBak = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\armor_rr_parked'
New-Item -ItemType Directory -Force $RrBak | Out-Null
Get-ChildItem $Armor -Filter 'rr_elite_pack.map*' | Move-Item -Destination $RrBak -Force
Copy-Item 'D:\Steam\steamapps\common\HREK\maps\20_sword_slayer.map' (Join-Path $Armor 'noble_elite_pack.map') -Force
& (Join-Path $Dir 'tools\runpy.ps1') -Script packreloc2.py -Out packreloc_noble.txt -Env "PACK=$Armor\noble_elite_pack.map;WRITE=1;OUT=packreloc_noble.txt"
"--- armor folder"
Get-ChildItem $Armor | ForEach-Object { "{0} {1}" -f $_.Name, $_.Length }
"--- parked"
Get-ChildItem $RrBak | ForEach-Object { $_.Name }
