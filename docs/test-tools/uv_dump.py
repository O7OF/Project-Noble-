# Dump per-object, per-material UV triangles of the Honor Guard materials from elite_noble.blend (read-only) -> uv_dump.json
import bpy, json, traceback
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
out = {}
log = []
try:
    for o in bpy.data.objects:
        if o.type != 'MESH': continue
        me = o.data
        if not me.uv_layers: continue
        uvl = me.uv_layers.active.data
        mats = [m.name if m else '' for m in me.materials]
        per = {}
        for p in me.polygons:
            mn = mats[p.material_index] if p.material_index < len(mats) else ''
            low = mn.lower()
            if 'honor' not in low and 'armor_02' not in low and 'ch_elite_armor' not in low: continue
            uv = [(uvl[li].uv[0], uvl[li].uv[1]) for li in p.loop_indices]
            per.setdefault(mn, []).append([c for t in uv for c in (round(t[0], 5), round(t[1], 5))])
        if per:
            out[o.name] = per
            log.append('%s: %s' % (o.name, {k: len(v) for k, v in per.items()}))
    json.dump(out, open(T + r'\uv_dump.json', 'w'))
except Exception:
    log.append(traceback.format_exc())
open(T + r'\uv_dump_log.txt', 'w').write('\n'.join(log))
