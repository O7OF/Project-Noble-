
import os
stock = r"C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps"
ws = r"C:\Program Files (x86)\Steam\steamapps\workshop\content\976730"
files = {
  'mod forge_halo (works)': os.path.join(ws, '2899383598', 'maps', 'forge_halo_normal.map'),
  'mod boneyard (works)':   os.path.join(ws, '2899771052', 'maps', '70_boneyard_normal.map'),
  'mod cex_ff_halo (fails)':os.path.join(ws, '2899728214', 'maps', 'cex_ff_halo_normal.map'),
  'mod icecave (fails)':    os.path.join(ws, '2902715603', 'maps', 'ff60_icecave_normal.map'),
  'stock cex_ff_halo':      os.path.join(stock, 'cex_ff_halo.map'),
  'stock 20_sword_slayer':  os.path.join(stock, '20_sword_slayer.map'),
}
N = 0x60
def head(p):
    with open(p, 'rb') as f:
        return f.read(N)
bufs = {k: head(p) for k, p in files.items()}
for k, b in bufs.items():
    print(f"{k:26s} " + " ".join(f"{x:02X}" for x in b))
print()
print("offset  " + " ".join(f"{i:>4X}" for i in range(0, 0x60, 4)))
