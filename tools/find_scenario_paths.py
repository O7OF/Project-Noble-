
import os, re
stock = r"C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps"
ws = r"C:\Program Files (x86)\Steam\steamapps\workshop\content\976730"
files = {
  'stock cex_ff_halo': os.path.join(stock, 'cex_ff_halo.map'),
  'mod cex_ff_halo':  os.path.join(ws, '2899728214', 'maps', 'cex_ff_halo_normal.map'),
  'mod forge_halo':   os.path.join(ws, '2899383598', 'maps', 'forge_halo_normal.map'),
  'stock 20_sword_slayer': os.path.join(stock, '20_sword_slayer.map'),
}
N = 8 * 1024 * 1024
for name, path in files.items():
    with open(path, 'rb') as f:
        data = f.read(N)
    hits = set()
    for m in re.finditer(rb'(levels[\\/][A-Za-z0-9_\\/]{3,80})', data):
        s = m.group(1).decode('ascii', 'replace')
        hits.add(s)
    print(f"--- {name}: {len(hits)} scenario-path strings")
    for s in sorted(hits)[:12]:
        print("   ", s)
