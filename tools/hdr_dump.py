
import os
maps = r"C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps"
names = ['20_sword_slayer','cex_ff_halo','ff20_courtyard','m05','m70_a']
N = 0x40
for n in names:
    with open(os.path.join(maps, n + '.map'), 'rb') as f:
        b = f.read(N)
    print(f"{n:18s} " + " ".join(f"{x:02X}" for x in b))
