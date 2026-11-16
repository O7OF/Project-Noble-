import struct, os
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
maps = {'rrpack': r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor_rr_parked\rr_elite_pack.map",
        'noble': r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\noble_elite_pack.map"}
out = []
for k, p in maps.items():
    with open(p, 'rb') as f: h = f.read(0xA000)
    out.append(f"=== {k}")
    for o in range(0x500, 0xA000, 4):
        v = struct.unpack_from('<I', h, o)[0]
        if v: out.append(f"{k} +{o:04X} {v:08X}")
    off, size = struct.unpack_from('<II', h, 0x97B0)
    out.append(f"{k} fixup? off {off:X} size {size:X}")
    if size:
        with open(p, 'rb') as f:
            f.seek(off); d = f.read(min(size, 64))
        out.append(f"{k} fixup head: " + " ".join(f"{x:08X}" for x in struct.unpack_from(f'<{len(d)//4}I', d)))
open(os.path.join(T, 'hdrtail.txt'), 'w').write("\n".join(out[:400]))
