import struct, os
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
b = open(os.path.join(T, 'meshexp', 'ours.mesh.x'), 'rb').read()
out = [repr(b[:16])]
o = 16; n = 0
while o < len(b) and n < 400:
    t = struct.unpack_from('<H', b, o)[0]; s = f"{o:6X} t{t}"; o += 2
    if t in (1, 2):
        c = struct.unpack_from('<I', b, o)[0]; s += f" '{b[o+4:o+4+c].decode('latin1')}'"; o += 4 + c + (2 if t == 2 else 0)
    elif t == 3: s += f" {struct.unpack_from('<I', b, o)[0]}"; o += 4
    elif t == 5: o += 16
    elif t in (6, 7):
        c = struct.unpack_from('<I', b, o)[0]; s += f" list[{c}]"; o += 4 + 4 * c
    out.append(s); n += 1
open(os.path.join(T, 'xtok.txt'), 'w').write("\n".join(out))
