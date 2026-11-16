import struct, os
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
maps = {
 'rrpack': r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map",
 'menu':   r"D:\Steam\steamapps\common\HREK\maps\mainmenu.map",
 'sword':  r"C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps\20_sword_slayer.map",
}
out = []
for k, p in maps.items():
    if not os.path.exists(p):
        out.append(f"{k}: missing {p}"); continue
    with open(p, 'rb') as f:
        h = f.read(0x800)
    out.append(f"=== {k} size {os.path.getsize(p)}")
    for o in range(0, 0x800, 4):
        v = struct.unpack_from('<I', h, o)[0]
        if v:
            out.append(f"{k} +{o:04X} {v:08X} {v}")
open(os.path.join(T, 'hdr3.txt'), 'w').write("\n".join(out))
