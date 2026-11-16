# Packed-pointer finder v2 for Reach MCC cache files -> .rrreloc. Validated against RR's pack.
# Rules: tags-header tables (instance addresses, interop addresses) + any in-range packed value V at a
# 4-aligned offset with the next word 0 and either (prev word = block count 1..0xFFFFF) or
# (data ref: prev two words 0 and size before) or (next two words 0: bare pointer arrays).
import struct, os, collections, sys
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
PB = 0x50000000

def find(path, log):
    m = open(path, 'rb').read()
    toff, tsize = struct.unpack_from('<II', m, 0x10)
    vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
    sec = m[toff:toff+tsize]
    W = struct.unpack_from(f'<{len(sec)//4}I', sec, 0)
    lo, hi = (vbase - PB) >> 2, (vbase + tsize - PB) >> 2
    rel = lambda a: a - vbase
    th = W[rel(thdr)//4: rel(thdr)//4 + 16]
    gcount, gaddr = th[0], th[2] | (th[3] << 32)
    icount, iaddr = th[4], th[6] | (th[7] << 32)
    xcount, xaddr = th[12], th[14] | (th[15] << 32)
    log(f"{os.path.basename(path)}: tags off {toff:X} size {tsize:X} vbase {vbase:X} groups {gcount} tags {icount} interops {xcount}")
    H = set()
    for i in range(icount):
        o = rel(iaddr) + 8*i + 4
        if W[o//4]: H.add(o)
    for i in range(xcount):
        o = rel(xaddr) + 8*i
        if lo <= W[o//4] < hi: H.add(o)
    skip = [(rel(iaddr), rel(iaddr) + 8*icount), (rel(xaddr), rel(xaddr) + 8*xcount),
            (rel(gaddr), rel(gaddr) + 16*gcount), (rel(thdr), rel(thdr) + 64)]
    def skipped(o): return any(a <= o < b for a, b in skip)
    n = len(W)
    for i in range(3, n - 3):
        v = W[i]
        if not (lo <= v < hi) or W[i+1] != 0: continue
        if skipped(i*4): continue
        p1, p2, p3 = W[i-1], W[i-2], W[i-3]
        if (0 < p1 < 0x100000) or (p1 == 0 and p2 == 0 and 0 < p3 < 0x10000000) or W[i+2] == 0:
            H.add(i*4)
    return H, vbase, tsize

def write(path, H, vbase, tsize):
    offs = sorted(H)
    with open(path + '.rrreloc', 'wb') as f:
        f.write(b'RRRL' + struct.pack('<IIQQI', 1, len(offs), vbase, PB, tsize))
        f.write(struct.pack(f'<{len(offs)}I', *offs))

if __name__ == '__main__' or True:
    out = []
    P = os.environ.get('PACK', r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map")
    H, vb, ts = find(P, out.append)
    out.append(f"candidates {len(H)}")
    if os.path.exists(P + '.rrreloc') and not os.environ.get('WRITE'):
        r = open(P + '.rrreloc', 'rb').read()
        cnt = struct.unpack_from('<I', r, 8)[0]
        rel = set(struct.unpack_from(f'<{cnt}I', r, 0x20))
        out.append(f"vs reloc {len(rel)}: miss {len(rel - H)} extra {len(H - rel)}")
        out.append("miss sample " + " ".join(f"{o:X}" for o in sorted(rel - H)[:10]))
        out.append("extra sample " + " ".join(f"{o:X}" for o in sorted(H - rel)[:40]))
    if os.environ.get('WRITE'):
        write(P, H, vb, ts); out.append(f"wrote {P}.rrreloc ({len(H)})")
    open(os.path.join(T, os.environ.get('OUT', 'packreloc2.txt')), 'w').write("\n".join(out))
