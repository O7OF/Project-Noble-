import struct, os, collections
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
P = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map"
out = []
def log(s): out.append(str(s))
r = open(P + '.rrreloc', 'rb').read()
magic, ver, cnt, vb, pb, ss = struct.unpack_from('<4sIIQQI', r, 0)
offs = struct.unpack_from(f'<{cnt}I', r, 0x20); rel = set(offs)
m = open(P, 'rb').read()
toff, tsize = struct.unpack_from('<II', m, 0x10)
vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
sec = m[toff:toff+tsize]
n = len(sec) // 4
W = struct.unpack_from(f'<{n}I', sec, 0)
lo, hi = (vbase - pb) >> 2, (vbase + tsize - pb) >> 2
# tags header
th = thdr - vbase
log("tags header words: " + " ".join(f"{x:08X}" for x in struct.unpack_from('<16I', sec, th)))
def ins(v): return lo <= v < hi
def classify(i):
    v = W[i]
    if not ins(v): return None
    p1 = W[i-1] if i >= 1 else 0
    p2 = W[i-2] if i >= 2 else 0
    p3 = W[i-3] if i >= 3 else 0
    if 0 < p1 < 0x100000: return 'block'
    if p1 == 0 and p2 == 0 and 0 < p3 < 0x10000000: return 'data'
    return 'other'
start = min(offs) // 4
C = collections.Counter(); hit = collections.Counter()
H = set()
for i in range(start, n):
    c = classify(i)
    if c in ('block', 'data'):
        H.add(i*4); C[c] += 1
        if i*4 in rel: hit[c] += 1
log(f"from {start*4:X}: H {len(H)} {dict(C)} hits {dict(hit)} ; miss {len(rel-H)} extra {len(H-rel)}")
for o in sorted(rel - H)[:25]:
    log(f"miss {o:X}: " + " ".join(f"{x:08X}" for x in W[o//4-5:o//4+3]))
for o in sorted(H - rel)[:25]:
    log(f"extra {o:X}: " + " ".join(f"{x:08X}" for x in W[o//4-5:o//4+3]))
mc = collections.Counter()
for o in rel - H:
    i = o//4
    mc[(W[i-1] == 0, W[i-2] == 0, W[i-3] == 0)] += 1
log(f"miss shapes (p1==0,p2==0,p3==0): {mc}")
open(os.path.join(T, 'reloc_probe2.txt'), 'w').write("\n".join(out))
