# Heuristic packed-pointer finder for a Reach MCC cache file (writes/validates an .rrreloc).
import struct, os, collections, bisect, sys
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
P = os.environ.get('PACK', r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map")
PB = 0x50000000
out = []
def log(s): out.append(str(s))

def load(path):
    m = open(path, 'rb').read()
    toff, tsize = struct.unpack_from('<II', m, 0x10)
    vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
    sec = m[toff:toff+tsize]
    W = struct.unpack_from(f'<{len(sec)//4}I', sec, 0)
    return m, sec, W, vbase, thdr, tsize

def analyse(path):
    m, sec, W, vbase, thdr, tsize = load(path)
    lo, hi = (vbase - PB) >> 2, (vbase + tsize - PB) >> 2
    rel = lambda a: a - vbase
    th = W[rel(thdr)//4: rel(thdr)//4 + 16]
    gcount, gaddr = th[0], th[2] | (th[3] << 32)
    icount, iaddr = th[4], th[6] | (th[7] << 32)
    xcount, xaddr = th[12], th[14] | (th[15] << 32)
    groups = [W[rel(gaddr)//4 + 4*g] for g in range(gcount)]
    gname = lambda g: struct.pack('>I', groups[g]).decode('latin1') if 0 <= g < len(groups) else '????'
    tags = []
    for i in range(icount):
        a, p = W[rel(iaddr)//4 + 2*i], W[rel(iaddr)//4 + 2*i + 1]
        g = a & 0xFFFF
        if g == 0xFFFF or not p: continue
        tags.append((((p << 2) + PB) - vbase, g, i))
    tags.sort()
    starts = [t[0] for t in tags]
    def owner(off):
        k = bisect.bisect_right(starts, off) - 1
        return tags[k] if k >= 0 else None
    H = set()
    # tables in the tags header
    for i in range(icount): 
        o = rel(iaddr) + 8*i + 4
        if W[o//4]: H.add(o)
    for i in range(xcount):
        o = rel(xaddr) + 8*i
        if lo <= W[o//4] < hi: H.add(o)
    first = starts[0] if starts else 0
    def ascii(v):
        b = struct.pack('<I', v)
        return all(32 <= c < 127 for c in b)
    skip = [(rel(iaddr), rel(iaddr) + 8*icount), (rel(xaddr), rel(xaddr) + 8*xcount),
            (rel(gaddr), rel(gaddr) + 16*gcount), (rel(thdr), rel(thdr) + 64)]
    log(f"first tags: " + ", ".join(f"{gname(t[1])}@{t[0]:X}" for t in tags[:6]) + f" | header tables {[(hex(a), hex(b)) for a, b in skip]}")
    def skipped(o): return any(a <= o < b for a, b in skip)
    for i in range(first//4, len(W) - 1):
        if skipped(i*4): continue
        v = W[i]
        if not (lo <= v < hi): continue
        p1, p2, p3 = W[i-1], W[i-2], W[i-3]
        if ascii(v) and (ascii(p1) or ascii(W[i+1])): continue
        if 0 < p1 < 0x100000 or (p1 == 0 and p2 == 0 and 0 < p3 < 0x10000000):
            H.add(i*4)
    return H, owner, gname, (lo, hi), W

H, owner, gname, rng, W = analyse(P)
log(f"{P}: candidates {len(H)}")
if os.path.exists(P + '.rrreloc'):
    r = open(P + '.rrreloc', 'rb').read()
    cnt = struct.unpack_from('<I', r, 8)[0]
    rel = set(struct.unpack_from(f'<{cnt}I', r, 0x20))
    miss, extra = sorted(rel - H), sorted(H - rel)
    log(f"reloc {len(rel)} both {len(rel & H)} miss {len(miss)} extra {len(extra)}")
    for name, lst in (('miss', miss), ('extra', extra)):
        c = collections.Counter()
        for o in lst:
            t = owner(o); c[gname(t[1]) if t else 'pre'] += 1
        log(f"{name} by group: {c.most_common(25)}")
        seen = collections.Counter(); pick = []
        for o in lst:
            t = owner(o); k = gname(t[1]) if t else 'pre'
            if seen[k] < 4: pick.append(o); seen[k] += 1
        for o in pick:
            t = owner(o)
            log(f"{name} {o:X} in {gname(t[1]) if t else '-'}+{(o - t[0]) if t else 0:X}: " + " ".join(f"{x:08X}" for x in W[o//4-5:o//4+3]))
if os.environ.get('WRITE'):
    m = open(P, 'rb').read()
    tsize = struct.unpack_from('<I', m, 0x14)[0]; vbase = struct.unpack_from('<Q', m, 0x2E0)[0]
    offs = sorted(H)
    with open(P + '.rrreloc', 'wb') as f:
        f.write(b'RRRL' + struct.pack('<IIQQI', 1, len(offs), vbase, PB, tsize))
        f.write(struct.pack(f'<{len(offs)}I', *offs))
    log(f"wrote {len(offs)} records")
open(os.path.join(T, os.environ.get('OUT', 'packreloc.txt')), 'w').write("\n".join(out))
