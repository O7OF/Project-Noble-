# noble_menu.string_ids.csv from the map-slim output of data\menumaps\noble_menu.map (see menu_sidcsv.ps1).
import sys, os, io, csv
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
sys.path.insert(0, os.path.join(T, 'tools'))
from sidcsv import rows
out = io.StringIO(); w = csv.writer(out, lineterminator='\r\n'); w.writerow(['Index', 'Total', 'ID', 'String'])
for r in rows(os.path.join(T, 'slim_noble_menu')): w.writerow(r)
open(r"D:\Project-Noble-Closed-Source\build\bin\Release\data\menumaps\noble_menu.string_ids.csv", 'w', encoding='utf-8', newline='').write(out.getvalue())
open(os.path.join(T, 'sidcsv_menu.txt'), 'w').write('ok')
