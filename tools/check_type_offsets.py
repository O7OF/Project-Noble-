
import os
ws = r"C:\Program Files (x86)\Steam\steamapps\workshop\content\976730"
stock = r"C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps"
mods = {
  'forge_halo (works)': os.path.join(ws, '2899383598', 'maps', 'forge_halo_normal.map'),
  '70_boneyard (works)': os.path.join(ws, '2899771052', 'maps', '70_boneyard_normal.map'),
  'cex_ff_halo (fails)': os.path.join(ws, '2899728214', 'maps', 'cex_ff_halo_normal.map'),
  'ff60_icecave (fails)': os.path.join(ws, '2902715603', 'maps', 'ff60_icecave_normal.map'),
}
refs = {
  'stock mp 20_sword_slayer': os.path.join(stock, '20_sword_slayer.map'),
  'stock ff cex_ff_halo': os.path.join(stock, 'cex_ff_halo.map'),
  'stock sp m05': os.path.join(stock, 'm05.map'),
}
offsets = [0x18, 0x1C, 0x1D, 0x1E, 0x70, 0x4D7, 0x50C, 0x50D]
def head(p, n=0x1000):
    with open(p, 'rb') as f: return f.read(n)
bufs = {k: head(p) for k, p in mods.items()}
refbufs = {k: head(p) for k, p in refs.items()}
for off in offsets:
    line = f"0x{off:04X}: "
    for k, b in refbufs.items():
        line += f"{k}=0x{b[off]:02X}  "
    print(line)
    line2 = "        "
    for k, b in bufs.items():
        line2 += f"{k}=0x{b[off]:02X}  "
    print(line2)
