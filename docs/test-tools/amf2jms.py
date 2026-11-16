"""Convert a Reclaimer AMF v2 render model to a JMS 8213 file (Halo 3 / Reach render source).

usage: amf2jms.py model.amf out.jms [--regions a,b,..] [--skip-regions a,b] [--rename old=new,..] [--map out.json]
Nodes: absolute transforms (JMS >= 8205). Materials: one per (shader, permutation, region).
"""
import sys, json, re, argparse
sys.path.insert(0, __import__('os').path.dirname(__file__))
from amf import AMF
from xform import absolute, qconj, qmul, qnorm

def clean(name):
    n = re.sub(r'\s*-\s*copy(\s*\(\d+\))?', '', name.strip(), flags=re.I)   # "x - copy (3) - copy" -> "x"
    n = re.sub(r'[^A-Za-z0-9_]+', '_', n).strip('_').lower()
    return n or 'unnamed'

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('amf'); ap.add_argument('jms')
    ap.add_argument('--regions', default=''); ap.add_argument('--skip-regions', default='')
    ap.add_argument('--rename', default='', help='region renames old=new,...')
    ap.add_argument('--map', default='')
    ap.add_argument('--node-prefix', default='', help='extra prefix written before every node name (HREK strips one leading "b_")')
    ap.add_argument('--collection', default='', help='"prefix:names.txt" - shaders listed in names.txt get "prefix " in front')
    a = ap.parse_args()
    m = AMF(a.amf)
    coll_prefix, coll_names = '', set()
    if a.collection:
        coll_prefix, f = a.collection.split(':', 1); coll_names = set(open(f).read().split())
    keep = set(filter(None, a.regions.split(','))); skip = set(filter(None, a.skip_regions.split(',')))
    ren = dict(x.split('=') for x in a.rename.split(',') if x)
    for i, n in enumerate(m.nodes):
        assert n['parent'] < i, 'node order: parent after child'
    absn = absolute(m.nodes)
    out = [';### VERSION ###', '8213', '']
    f6 = lambda *v: '\t'.join('%0.6f' % x for x in v)
    out += [';### NODES ###', str(len(m.nodes))]
    for i, (n, (p, q)) in enumerate(zip(m.nodes, absn)):
        out += [';NODE %d' % i, a.node_prefix + n['name'], str(n['parent']), f6(*q), f6(*p), '']
    # geometry first so materials are known
    mats, matidx, verts, tris, shader_names = [], {}, [], [], {}
    seen_mesh = {}
    for r in m.regions:
        if (keep and r['name'] not in keep) or r['name'] in skip: continue
        rname = ren.get(r['name'], r['name'])
        for p in r['perms']:
            if not p['vc']: continue
            if p['mtx'] is not None: raise SystemExit('permutation transform not handled: %s %s' % (r['name'], p['name']))
            v, t, subs, _ = m.mesh(p)
            base = len(verts)
            for pos, nrm, uv, inf in v:
                inf = [(n, w) for n, w in inf if w > 0] or [(0, 1.0)]
                s = sum(w for _, w in inf); inf = [(n, w / s) for n, w in inf]
                verts.append((pos, nrm, (uv[0], 1.0 - uv[1]), inf))
            for sh, start, count in subs:
                sname = m.shaders[sh]['name']; cn = clean(sname)
                shader_names.setdefault(cn, set()).add(sname)
                key = (cn, p['name'], rname)
                if key not in matidx:
                    matidx[key] = len(mats); mats.append(key)
                mi = matidx[key]
                for a0, a1, a2 in t[start:start + count]:
                    tris.append((mi, base + a0, base + a1, base + a2))
    out += [';### MATERIALS ###', str(len(mats))]
    order = sorted(range(len(mats)), key=lambda i: -(len(mats[i][1]) + len(mats[i][2])))
    slot = {i: k + 1 for k, i in enumerate(order)}
    for i, (cn, perm, reg) in enumerate(mats):
        d = '(%d) %s %s' % (slot[i], perm, reg)
        if len(d) > 31: raise SystemExit('material definition longer than 31 chars: ' + d)
        out += [';MATERIAL %d' % i, (coll_prefix + ' ' + cn) if cn in coll_names else cn, d, '']
    mk = {}
    for mm in m.markers:
        if mm['region'] != 255: continue          # region/permutation-specific markers dropped
        mk.setdefault((mm['name'].strip(), mm['node'], mm['pos'], mm['rot']), mm)
    out += [';### MARKERS ###', str(len(mk))]
    for i, (name, node, pos, rot) in enumerate(mk):
        out += [';MARKER %d' % i, name, str(node), f6(*qnorm(rot)), f6(*pos), f6(-1.0), '']
    out += [';### INSTANCE XREF PATHS ###', '0', ';### INSTANCE MARKERS ###', '0']
    out += [';### VERTICES ###', str(len(verts))]
    for i, (pos, nrm, uv, inf) in enumerate(verts):
        out += [';VERTEX %d' % i, f6(*pos), f6(*nrm), str(len(inf))]
        for n, w in inf: out += [str(n), '%0.6f' % w]
        out += ['1', f6(*uv), f6(0, 0, 0), '']
    out += [';### TRIANGLES ###', str(len(tris))]
    for i, (mi, x, y, z) in enumerate(tris):
        out += [';TRIANGLE %d' % i, str(mi), '%d\t%d\t%d' % (x, y, z), '']
    for sec in ('SPHERES', 'BOXES', 'CAPSULES', 'CONVEX SHAPES', 'RAGDOLLS', 'HINGES', 'CAR WHEELS',
                'POINT TO POINT', 'PRISMATIC', 'BOUNDING SPHERE', 'SKYLIGHT'):
        out += [';### %s ###' % sec, '0']
    open(a.jms, 'w', newline='\r\n').write('\n'.join(out) + '\n')
    if a.map:
        json.dump({k: sorted(v) for k, v in shader_names.items()}, open(a.map, 'w'), indent=1)
    print('nodes %d materials %d markers %d verts %d tris %d shaders %d' % (len(m.nodes), len(mats), len(mk), len(verts), len(tris), len(shader_names)))
main()
