# Regenerate data\menumaps\noble_menu.string_ids.csv from the installed noble_menu.map. Run after EVERY menu rebuild:
# ReachArmory looks permutation/region names up in this CSV; a stale one sends wrong string ids.
$T = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$MM = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\menumaps'
& "$T\map-slim.ps1" -Map "$MM\noble_menu.map" -OutDir "$T\slim_noble_menu" | Out-Null
Copy-Item "$MM\noble_menu.string_ids.csv" "$MM\noble_menu.string_ids.csv.bak" -Force
& "$T\tools\runpy.ps1" -Script sidcsv_menu.py -Out sidcsv_menu.txt
"lines: " + (Get-Content "$MM\noble_menu.string_ids.csv").Count
Select-String -Path "$MM\noble_menu.string_ids.csv" -Pattern ',honor_guard$|,council$|,ossoona$|,schism$' | ForEach-Object { $_.Line }
