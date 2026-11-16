import struct, os, collections, bisect
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
P = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map"
PB = 0x50000000
m = open(P, 'rb').read()
toff, tsize = struct.unpack_from('<II', m, 0x10)
vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
sec = m[toff:toff+tsize]
W = struct.unpack_from(f'<{len(sec)//4}I', sec, 0)
rel = lambda a: a - vbase
th = W[rel(thdr)//4: rel(thdr)//4 + 16]
gcount, gaddr, icount, iaddr = th[0], th[2] | th[3] << 32, th[4], th[6] | th[7] << 32
groups = [W[rel(gaddr)//4 + 4*g] for g in range(gcount)]
inst = W[rel(iaddr)//4: rel(iaddr)//4 + 2*icount]
gname = lambda g: struct.pack('>I', groups[g]).decode('latin1') if 0 <= g < gcount else '????'
tags = sorted((((inst[2*i+1] << 2) + PB) - vbase, inst[2*i] & 0xFFFF, i) for i in range(icount) if inst[2*i+1] and (inst[2*i] & 0xFFFF) != 0xFFFF)
starts = [t[0] for t in tags]
def owner(o):
    k = bisect.bisect_right(starts, o) - 1
    return gname(tags[k][1]) if k >= 0 else 'pre'
datums = {((inst[2*i] >> 16) << 16) | i: inst[2*i] & 0xFFFF for i in range(icount) if (inst[2*i] & 0xFFFF) != 0xFFFF}
magics = set(groups)
off, size = struct.unpack_from('<II', m, 0x97B0)
tab = m[off:off+size]
n = struct.unpack_from('<I', tab, 0)[0]
truth = collections.defaultdict(set)
for e in range(n):
    k, _, a = struct.unpack_from('<IIQ', tab, 16 + 16*e)
    truth[k].add(a - vbase)
out = []
for k in (0, 1, 2):
    c = collections.Counter(owner(o) for o in truth[k])
    out.append(f"kind {k}: {len(truth[k])} by owner {c.most_common(12)}")
    vals = collections.Counter()
    for o in list(truth[k])[:2000]:
        v = W[o//4]
        vals['ffff' if v == 0xFFFFFFFF else ('datum' if v in datums else ('magic' if v in magics else ('small' if v < icount else 'other')))] += 1
    out.append(f"   value kinds (2000 sample) {vals}")
# heuristics
H1 = set(); H2 = set()
for i in range(len(W) - 4):
    v = W[i]
    if v in datums: H1.add(i*4)
    if v in magics and W[i+3] in datums and datums[W[i+3]] < gcount and groups[datums[W[i+3]]] == v: H2.add(i*4)
    if v == 0xFFFFFFFF and W[i+3] == 0xFFFFFFFF and W[i+1] == 0 and W[i+2] == 0: H2.add(i*4)
for k, H in ((1, H1), (2, H2)):
    t = truth[k]
    out.append(f"heur kind {k}: {len(H)} vs truth {len(t)}: miss {len(t - H)} extra {len(H - t)}")
    out.append("   miss sample " + " | ".join(f"{o:X}:{owner(o)}:" + " ".join(f'{x:08X}' for x in W[o//4-1:o//4+4]) for o in sorted(t - H)[:6]))
    out.append("   extra sample " + " | ".join(f"{o:X}:{owner(o)}:" + " ".join(f'{x:08X}' for x in W[o//4-1:o//4+4]) for o in sorted(H - t)[:6]))
# kind 0 neighborhoods
s0 = sorted(truth[0])
runs = []; start = s0[0]; prev = s0[0]
for o in s0[1:]:
    if o != prev + 4 and o != prev + 2:
        runs.append((start, prev)); start = o
    prev = o
runs.append((start, prev))
out.append(f"kind0 runs {len(runs)}; biggest: " + ", ".join(f"{a:X}-{b:X}({owner(a)})" for a, b in sorted(runs, key=lambda r: r[0]-r[1])[:12]))
out.append("kind0 odd-aligned: " + str(sum(1 for o in s0 if o % 4)))
open(os.path.join(T, 'rrfix2.txt'), 'w').write("\n".join(out))
