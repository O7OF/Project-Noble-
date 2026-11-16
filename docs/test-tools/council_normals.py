# Read-only: find faces of the council helmet whose normal points toward the mesh centre (likely inverted), and faces
# that coincide with another face (duplicates). Writes council_normals.txt and council_inward_uv.json.
import bpy, json, traceback
from mathutils import Vector
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
log = []
try:
    for o in bpy.data.objects:
        if o.type != 'MESH' or o.name not in ('council helmet', 'ossoona helmet'): continue
        me = o.data
        mats = [m.name if m else '' for m in me.materials]
        cen = sum((v.co for v in me.vertices), Vector()) / len(me.vertices)
        inward = {}; uvs = []
        uvl = me.uv_layers.active.data
        keys = {}
        dup = 0
        for p in me.polygons:
            mn = mats[p.material_index]
            d = (p.center - cen).normalized()
            if p.normal.dot(d) < -0.3:
                inward[mn] = inward.get(mn, 0) + 1
                uvs.append([c for li in p.loop_indices for c in (round(uvl[li].uv[0], 4), round(uvl[li].uv[1], 4))])
            k = tuple(sorted(tuple(round(c, 3) for c in me.vertices[v].co) for v in p.vertices))
            if k in keys: dup += 1
            keys[k] = 1
        tot = {}
        for p in me.polygons: tot[mats[p.material_index]] = tot.get(mats[p.material_index], 0) + 1
        log.append('%s: faces per material %s; inward-facing %s; duplicate faces %d; custom normals %s' % (o.name, tot, inward, dup, me.has_custom_normals))
        if o.name == 'council helmet': json.dump(uvs, open(T + r'\council_inward_uv.json', 'w'))
except Exception:
    log.append(traceback.format_exc())
open(T + r'\council_normals.txt', 'w').write('\n'.join(log))
