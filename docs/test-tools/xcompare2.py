# Ours vs stock render model (tool export-render-model-mesh .x, one merged Mesh): nearest-vertex match on position,
# then compare UVs (same / v-flipped / u-flipped) and normals for matched vertices.
import struct, os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def load(path):
    b = open(path, 'rb').read(); o = 16; lists = []; names = []
    while o + 2 <= len(b):
        t = struct.unpack_from('<H', b, o)[0]; o += 2
        if t in (1, 2):
            c = struct.unpack_from('<I', b, o)[0]; names.append((len(lists), b[o+4:o+4+c].decode('latin1'))); o += 4 + c + (2 if t == 2 else 0)
        elif t == 3: o += 4
        elif t == 5: o += 16
        elif t in (6, 7):
            c = struct.unpack_from('<I', b, o)[0]
            lists.append((t, struct.unpack_from(f'<{c}{"I" if t == 6 else "f"}', b, o + 4))); o += 4 + 4 * c
    # sequence after 'Mesh': IL[n], FL[pos], IL[faces], (MeshNormals) IL, FL[nrm], IL[nfaces], (MeshTextureCoords) IL, FL[uv]
    fl = [v for t, v in lists if t == 7]
    return fl[0], fl[1], fl[2]
try:
    import numpy as np
    po, no, uo = (np.array(x) for x in load(os.path.join(T, 'meshexp', 'ours.mesh.x')))
    ps, ns, us = (np.array(x) for x in load(os.path.join(T, 'meshexp', 'stock.mesh.x')))
    po = po.reshape(-1, 3); no = no.reshape(-1, 3); uo = uo.reshape(-1, 2)
    ps = ps.reshape(-1, 3); ns = ns.reshape(-1, 3); us = us.reshape(-1, 2)
    L(f"ours {len(po)} verts ({len(no)} normals, {len(uo)} uvs); stock {len(ps)} ({len(ns)}, {len(us)})")
    L(f"ours pos range {po.min(0).round(2)}..{po.max(0).round(2)}; stock {ps.min(0).round(2)}..{ps.max(0).round(2)}")
    n = min(len(po), len(no), len(uo)); po, no, uo = po[:n], no[:n], uo[:n]
    m = min(len(ps), len(ns), len(us)); ps, ns, us = ps[:m], ns[:m], us[:m]
    dist = np.empty(n); J = np.empty(n, int)
    for s in range(0, n, 512):
        d = ((po[s:s+512, None, :] - ps[None, :, :]) ** 2).sum(2)
        J[s:s+512] = d.argmin(1); dist[s:s+512] = np.sqrt(d.min(1))
    for tol in (0.0005, 0.002, 0.01):
        L(f"matched within {tol}: {(dist < tol).mean():.0%}")
    c = dist < 0.002
    a = uo[c]; b = us[J[c]]
    tests = {'same': b, 'v-flip': np.c_[b[:, 0], 1 - b[:, 1]], 'u-flip': np.c_[1 - b[:, 0], b[:, 1]], 'both': 1 - b}
    for k, bb in tests.items():
        L(f" UV {k}: {(np.abs(a - bb).max(1) < 0.01).mean():.0%}")
    L(" sample ours/stock: " + " ".join(f"({a[k][0]:.3f},{a[k][1]:.3f})/({b[k][0]:.3f},{b[k][1]:.3f})" for k in range(0, min(len(a), 400), 50)))
    na = no[c]; nb = ns[J[c]]
    dots = (na * nb).sum(1) / (np.linalg.norm(na, axis=1) * np.linalg.norm(nb, axis=1) + 1e-9)
    L(f" normals: median dot {np.median(dots):.3f}; dot<0.9 {(dots < 0.9).mean():.0%}; dot<0 {(dots < 0).mean():.0%}")
    L(" normal sample ours/stock: " + " ".join(f"({na[k][0]:.2f},{na[k][1]:.2f},{na[k][2]:.2f})/({nb[k][0]:.2f},{nb[k][1]:.2f},{nb[k][2]:.2f})" for k in range(0, min(len(na), 400), 80)))
except Exception:
    L("ERROR\n" + traceback.format_exc())
open(os.path.join(T, 'xcompare2.txt'), 'w').write("\n".join(out))
