# Per mesh object in elite_noble.blend: share of loop (split) normals pointing against the face normal, UV bounds,
# UV layer count. Read-only (blend not saved).
import bpy, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\normcheck.txt'
out = []
try:
    import numpy as np
    rows = []
    for o in bpy.data.objects:
        if o.type != 'MESH': continue
        me = o.data
        nl = len(me.loops)
        if nl == 0: continue
        ln = np.empty(nl * 3, np.float32)
        try:
            me.loops.foreach_get('normal', ln)
        except Exception:
            ln = np.array([c for l in me.corner_normals for c in l.vector], np.float32)
        ln = ln.reshape(-1, 3)
        pn = np.empty(len(me.polygons) * 3, np.float32); me.polygons.foreach_get('normal', pn); pn = pn.reshape(-1, 3)
        lp = np.empty(nl, np.int32)
        ls = np.empty(len(me.polygons), np.int32); me.polygons.foreach_get('loop_start', ls)
        lt = np.empty(len(me.polygons), np.int32); me.polygons.foreach_get('loop_total', lt)
        lp = np.repeat(np.arange(len(me.polygons)), lt)
        dots = (ln * pn[lp]).sum(1)
        inv = float((dots < 0).mean())
        uvs = me.uv_layers
        if uvs:
            uv = np.empty(nl * 2, np.float32); uvs.active.data.foreach_get('uv', uv); uv = uv.reshape(-1, 2)
            ub = f"u {uv[:,0].min():.2f}..{uv[:,0].max():.2f} v {uv[:,1].min():.2f}..{uv[:,1].max():.2f}"
        else: ub = 'no uv'
        mats = ",".join(sorted({m.name for m in me.materials if m}))[:60]
        rows.append((inv, o.name, len(me.polygons), len(uvs), ub, mats))
    rows.sort(reverse=True)
    out.append(f"objects {len(rows)}; mean inverted-loop share {np.mean([r[0] for r in rows]):.2f}")
    for r in rows[:25] + [('...',)] + rows[-8:]:
        out.append(" ".join(str(x) if not isinstance(x, float) else f"{x:.2f}" for x in r))
except Exception:
    out.append("ERROR\n" + traceback.format_exc())
open(LOG, 'w').write("\n".join(out))
