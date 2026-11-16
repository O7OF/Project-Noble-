# elite_noble render model now vs before tonight's UV/PRT changes (.before-prt): nearest exact-position vertex match,
# UVs same / v-flipped / other; report per height band so pieces can be told apart.
import struct, os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def load(path):
    b = open(path, 'rb').read(); o = 16; lists = []
    while o + 2 <= len(b):
        t = struct.unpack_from('<H', b, o)[0]; o += 2
        if t in (1, 2):
            c = struct.unpack_from('<I', b, o)[0]; o += 4 + c + (2 if t == 2 else 0)
        elif t == 3: o += 4
        elif t == 5: o += 16
        elif t in (6, 7):
            c = struct.unpack_from('<I', b, o)[0]
            if t == 7: lists.append(struct.unpack_from(f'<{c}f', b, o + 4))
            o += 4 + 4 * c
    return lists[0], lists[2]
try:
    import numpy as np
    pn, un = (np.array(x) for x in load(os.path.join(T, 'meshexp', 'now.mesh.x')))
    po, uo = (np.array(x) for x in load(os.path.join(T, 'meshexp', 'old.mesh.x')))
    pn = pn.reshape(-1, 3); un = un.reshape(-1, 2)[:len(pn)]; po = po.reshape(-1, 3); uo = uo.reshape(-1, 2)[:len(po)]
    L(f"now {len(pn)} verts, old {len(po)}")
    key = lambda p: [tuple(np.round(x, 4)) for x in p]
    idx = {}
    for i, k in enumerate(key(po)): idx.setdefault(k, []).append(i)
    res = []
    for i, k in enumerate(key(pn)):
        js = idx.get(k)
        if not js: res.append('nomatch'); continue
        best = 'other'
        for j in js:
            if np.abs(un[i] - uo[j]).max() < 1e-3: best = 'same'; break
            if abs(un[i][0] - uo[j][0]) < 1e-3 and abs(un[i][1] - (1 - uo[j][1])) < 1e-3: best = 'vflip'
        res.append(best)
    res = np.array(res)
    for r in ('same', 'vflip', 'other', 'nomatch'): L(f"{r}: {(res == r).sum()}")
    z = pn[:, 2] if pn[:, 2].ptp() > pn[:, 1].ptp() else pn[:, 1]
    L("height band (low->high): same/vflip/other")
    edges = np.linspace(z.min(), z.max(), 9)
    for a, b in zip(edges, edges[1:]):
        m = (z >= a) & (z <= b)
        L(f"  {a:6.2f}..{b:6.2f}: " + "/".join(str(((res == r) & m).sum()) for r in ('same', 'vflip', 'other')))
except Exception:
    L("ERROR\n" + traceback.format_exc())
open(os.path.join(T, 'xcompare_old.txt'), 'w').write("\n".join(out))
