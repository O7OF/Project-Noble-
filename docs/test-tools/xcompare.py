# Compare our vs stock render-model meshes (tool export-render-model-mesh, binary DirectX .x): for faces whose material
# texture matches TEX, match each of our vertices to the nearest stock vertex and compare UVs (same / v-flipped / other)
# and normals.
import struct, os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
D = os.path.join(T, 'meshexp')
out = []; L = out.append
def tokens(b):
    o = 16; fsz = 4 if b[12:16] == b'0032' else 8; n = len(b)
    while o + 2 <= n:
        t = struct.unpack_from('<H', b, o)[0]; o += 2
        if t == 1:
            c = struct.unpack_from('<I', b, o)[0]; yield ('N', b[o+4:o+4+c].decode('latin1')); o += 4 + c
        elif t == 2:
            c = struct.unpack_from('<I', b, o)[0]; yield ('S', b[o+4:o+4+c].decode('latin1')); o += 4 + c + 2
        elif t == 3: yield ('I', struct.unpack_from('<I', b, o)[0]); o += 4
        elif t == 5: o += 16
        elif t == 6:
            c = struct.unpack_from('<I', b, o)[0]; yield ('IL', struct.unpack_from(f'<{c}I', b, o + 4)); o += 4 + 4 * c
        elif t == 7:
            c = struct.unpack_from('<I', b, o)[0]; yield ('FL', struct.unpack_from(f'<{c}{"f" if fsz == 4 else "d"}', b, o + 4)); o += 4 + fsz * c
        elif t == 10: yield ('{', None)
        elif t == 11: yield ('}', None)
        else: pass
def tree(b):
    root = {'t': 'root', 'kids': [], 'data': []}; stack = [root]; pending = []
    for k, v in tokens(b):
        if k == 'N': pending.append(v)
        elif k == '{':
            node = {'t': pending[0] if pending else '', 'name': pending[1] if len(pending) > 1 else '', 'kids': [], 'data': []}
            stack[-1]['kids'].append(node); stack.append(node); pending = []
        elif k == '}':
            if pending: stack[-1]['data'].append(('ref', pending[-1])); pending = []
            if len(stack) > 1: stack.pop()
        else:
            if pending and stack[-1]['t'] == 'root': pending = []
            stack[-1]['data'].append((k, v))
    return root
def walk(n):
    yield n
    for k in n['kids']: yield from walk(k)
def meshes(path, tex):
    r = tree(open(path, 'rb').read())
    P, U, N = [], [], []
    for m in walk(r):
        if m['t'] != 'Mesh': continue
        il = [v for k, v in m['data'] if k == 'IL']; fl = [v for k, v in m['data'] if k == 'FL']
        if not il or not fl: continue
        nv = il[0][0]; pos = fl[0]
        faces = []; f = il[1] if len(il) > 1 else (); i = 1
        while i < len(f):
            c = f[i]; faces.append(f[i+1:i+1+c]); i += 1 + c
        uv = None; nrm = None; mats = None; mnames = []
        for k in m['kids']:
            kil = [v for kk, v in k['data'] if kk == 'IL']; kfl = [v for kk, v in k['data'] if kk == 'FL']
            if k['t'] == 'MeshTextureCoords' and kfl: uv = kfl[0]
            if k['t'] == 'MeshNormals' and kfl: nrm = kfl[0]
            if k['t'] == 'MeshMaterialList':
                mats = kil[0][2:] if kil else None
                for mm in k['kids']:
                    tn = ''
                    for t2 in walk(mm):
                        if t2['t'] == 'TextureFilename':
                            tn = next((v for kk, v in t2['data'] if kk == 'S'), '')
                    mnames.append(mm.get('name', '') + '|' + tn)
                for kk, v in k['data']:
                    if kk == 'ref': mnames.append(v)
        if uv is None or mats is None: continue
        want = {i for i, nm in enumerate(mnames) if tex in nm}
        vs = set()
        for fi, fc in enumerate(faces):
            if fi < len(mats) and mats[fi] in want: vs.update(fc)
        for v in vs:
            P.append(pos[3*v:3*v+3]); U.append(uv[2*v:2*v+2])
            N.append(nrm[3*v:3*v+3] if nrm and len(nrm) >= 3*v+3 else (0, 0, 0))
    return P, U, N
try:
    import numpy as np, collections
    if os.environ.get('DEBUG', '1'):
        r = tree(open(os.path.join(D, 'ours.mesh.x'), 'rb').read())
        L("node types: " + str(collections.Counter(n['t'] for n in walk(r)).most_common(20)))
        for m in walk(r):
            if m['t'] == 'Mesh':
                L(f"first Mesh name {m['name']!r} data " + str([(k, (len(v) if isinstance(v, tuple) else v)) for k, v in m['data']][:8]))
                for k in m['kids']:
                    L(f"  kid {k['t']} {k['name']!r} data " + str([(kk, (len(v) if isinstance(v, tuple) else v)) for kk, v in k['data']][:6]) + f" kids {[c['t'] + ':' + c.get('name','') for c in k['kids']][:6]}")
                break
    for tex in os.environ.get('TEX', 'elite_minor_helmet_diffuse|elite_spec_ops_chest_diffuse').split('|'):
        po, uo, no = meshes(os.path.join(D, 'ours.mesh.x'), tex)
        ps, us, ns = meshes(os.path.join(D, 'stock.mesh.x'), tex)
        L(f"\n{tex}: ours {len(po)} verts, stock {len(ps)} verts")
        if not po or not ps: continue
        po, uo, ps, us = map(lambda a: np.array(a, np.float64), (po, uo, ps, us))
        L(f" ours pos range {po.min(0).round(3)} .. {po.max(0).round(3)}; stock {ps.min(0).round(3)} .. {ps.max(0).round(3)}")
        best = []
        for i in range(len(po)):
            d = ((ps - po[i]) ** 2).sum(1); j = int(d.argmin()); best.append((d[j] ** 0.5, j))
        dist = np.array([b[0] for b in best]); J = np.array([b[1] for b in best])
        close = dist < 0.002
        L(f" nearest-vertex distance median {np.median(dist):.5f}, matched (<0.002) {close.mean():.0%}")
        if close.any():
            a = uo[close]; b = us[J[close]]
            same = (np.abs(a - b).max(1) < 0.01).mean()
            vflip = (np.abs(a - np.c_[b[:, 0], 1 - b[:, 1]]).max(1) < 0.01).mean()
            L(f" UV same {same:.0%}, v-flipped {vflip:.0%}; sample ours/stock: " + " ".join(f"({a[k][0]:.3f},{a[k][1]:.3f})/({b[k][0]:.3f},{b[k][1]:.3f})" for k in range(0, min(len(a), 40), 8)))
            na = np.array(no)[close]; nb = np.array(ns)[J[close]]
            dots = (na * nb).sum(1) / (np.linalg.norm(na, axis=1) * np.linalg.norm(nb, axis=1) + 1e-9)
            L(f" normal agreement: median dot {np.median(dots):.3f}, share dot<0.9 {(dots < 0.9).mean():.0%}")
except Exception:
    L("ERROR\n" + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'xcompare.txt')), 'w').write("\n".join(out))
