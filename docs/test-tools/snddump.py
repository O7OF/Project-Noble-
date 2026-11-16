# Dump cache snd! tags (+ blocks they point to) with RR truth markers (P = packed pointer, K0/1/2 = fixup kind).
import struct, os, collections
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
P = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map"
GROUP = os.environ.get('GROUP', 'snd!'); N = int(os.environ.get('N', '3')); WORDS = int(os.environ.get('WORDS', '16'))
out = []; L = out.append
m = open(P, 'rb').read()
toff, tsize = struct.unpack_from('<II', m, 0x10); vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
sec = m[toff:toff + tsize]; u = lambda o: struct.unpack_from('<I', sec, o)[0]
th = struct.unpack_from('<16I', sec, thdr - vbase)
gaddr = th[2] | th[3] << 32; icount = th[4]; iaddr = th[6] | th[7] << 32
groups = [struct.pack('>I', u(gaddr - vbase + 16 * i)).decode('latin1') for i in range(th[0])]
r = open(P + '.rrreloc', 'rb').read(); RS = set(struct.unpack_from(f'<{struct.unpack_from("<I", r, 8)[0]}I', r, 0x20))
fl, bf, fo, fs = struct.unpack_from('<IIII', m, 0x97A8); h = struct.unpack_from('<IIII', m, fo)
K = {a - vbase: k for k, z, a in (struct.unpack_from('<IIQ', m, fo + h[1] + 16 * i) for i in range(h[0]))}
def mark(o): return ('P' if o in RS else '') + (f'K{K[o]}' if o in K else '')
def row(a, n):
    return " ".join(f"{u(a + 4*j):08X}{('['+mark(a+4*j)+']') if mark(a+4*j) else ''}" for j in range(n))
tags = []
for i in range(icount):
    gi, salt, p = struct.unpack_from('<HHI', sec, iaddr - vbase + 8 * i)
    if p and gi < len(groups) and groups[gi] == GROUP: tags.append(p * 4 + 0x50000000 - vbase)
tags.sort()
L(f"{GROUP}: {len(tags)} tags; gaps between consecutive: {collections.Counter(b - a for a, b in zip(tags, tags[1:])).most_common(5)}")
for a in tags[:N]:
    L(f"tag @{a:X}: " + row(a, WORDS))
    for j in range(WORDS):
        if (a + 4*j) in RS:
            cnt = u(a + 4*j - 4); b = u(a + 4*j) * 4 + 0x50000000 - vbase
            L(f"   ptr at +{4*j}: count {cnt} -> @{b:X}: " + row(b, min(64, max(8, cnt * 4))))
open(os.path.join(T, os.environ.get('OUT', 'snddump.txt')), 'w').write("\n".join(out))
