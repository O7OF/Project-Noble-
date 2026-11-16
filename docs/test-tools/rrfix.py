import struct, os, collections
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
P = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map"
m = open(P, 'rb').read()
toff, tsize = struct.unpack_from('<II', m, 0x10)
vbase = struct.unpack_from('<Q', m, 0x2E0)[0]
off, size = struct.unpack_from('<II', m, 0x97B0)
tab = m[off:off+size]
out = [f"table off {off:X} size {size:X} words0-7: " + " ".join(f"{x:08X}" for x in struct.unpack_from('<8I', tab, 0))]
n = (size - 16) // 16
kinds = collections.Counter(); hi = collections.Counter()
samples = []
for e in range(n):
    a, b, c, d = struct.unpack_from('<IIII', tab, 16 + 16*e)
    addr = c | (d << 32)
    kinds[(a, b)] += 1 if e < 300000 else 0
    if vbase <= addr < vbase + tsize:
        v = struct.unpack_from('<I', m, toff + addr - vbase)[0]
        hi[v >> 16 >= 0xE000 or (v >> 28) in (8, 9, 0xE, 0xF)] += 1
        if e < 12 or e % 20000 == 0:
            samples.append(f"  e{e} {a:08X} {b:08X} addr {addr:X} (+{addr - vbase:X}) value {v:08X}")
    else:
        hi['outside'] += 1
out.append(f"entries {n}; first-word kinds {kinds.most_common(6)}; datum-looking values {hi}")
out += samples
# where are RR's datum fix locations relative to our packed reloc list?
open(os.path.join(T, 'rrfix.txt'), 'w').write("\n".join(out))
