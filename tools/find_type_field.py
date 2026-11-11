
import os
stock = r"C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps"
ws = r"C:\Program Files (x86)\Steam\steamapps\workshop\content\976730"
def head(path, n):
    with open(path, 'rb') as f:
        return f.read(n)
N = 0x4000
groups = {
    'mp':  [os.path.join(stock, x + '.map') for x in ['20_sword_slayer','30_settlement','70_boneyard']],
    'ff':  [os.path.join(stock, x + '.map') for x in ['cex_ff_halo','ff20_courtyard','ff30_waterfront']],
    'sp':  [os.path.join(stock, x + '.map') for x in ['m05','m70_a','m70_bonus']],
}
data = {k: [head(p, N) for p in v] for k, v in groups.items()}
hits = []
for i in range(N):
    vals = {}
    ok = True
    for k, bufs in data.items():
        v = bufs[0][i]
        if any(b[i] != v for b in bufs):
            ok = False; break
        vals[k] = v
    if not ok: continue
    if len(set(vals.values())) != 3: continue
    hits.append((i, vals))
print("candidates:", len(hits))
for i, vals in hits[:30]:
    print(f"0x{i:04X}: mp=0x{vals['mp']:02X} ff=0x{vals['ff']:02X} sp=0x{vals['sp']:02X}")
# cross-check on the mod maps
mods = {
  'forge_halo (works)': os.path.join(ws, '2899383598', 'maps', 'forge_halo_normal.map'),
  '70_boneyard (works)': os.path.join(ws, '2899771052', 'maps', '70_boneyard_normal.map'),
  'cex_ff_halo (fails)': os.path.join(ws, '2899728214', 'maps', 'cex_ff_halo_normal.map'),
  'ff60_icecave (fails)': os.path.join(ws, '2902715603', 'maps', 'ff60_icecave_normal.map'),
}
modhead = {k: head(p, N) for k, p in mods.items()}
print("--- mod maps at the candidate offsets ---")
for i, vals in hits[:12]:
    line = f"0x{i:04X}: "
    for k, b in modhead.items():
        line += f"{k}=0x{b[i]:02X}  "
    print(line)
