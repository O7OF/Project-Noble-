
import os
maps = r"C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps"
mp = ['20_sword_slayer','30_settlement','70_boneyard']
ff = ['cex_ff_halo','ff20_courtyard','ff30_waterfront']
camp = ['m05']
N = 0x8000
def head(n):
    with open(os.path.join(maps, n + '.map'), 'rb') as f:
        return f.read(N)
mph = [head(n) for n in mp]
ffh = [head(n) for n in ff]
ch = head(camp[0])
hits = []
for i in range(N):
    mv = mph[0][i]
    if any(h[i] != mv for h in mph): continue
    fv = ffh[0][i]
    if any(h[i] != fv for h in ffh): continue
    if mv == fv: continue
    hits.append((i, mv, fv, ch[i]))
print("candidates:", len(hits))
for i, mv, fv, cv in hits[:40]:
    print(f"0x{i:04X}: MP=0x{mv:02X} FF=0x{fv:02X} campaign=0x{cv:02X}")
