# Per mesh: UVs in the current elite_noble.blend vs the .before-uvflip backup (same / v-flipped / other). Read-only.
import bpy, traceback
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
OLD = r'D:\Steam\steamapps\common\HREK\noble_mods\elite_noble_backup_before_foundry\elite_noble.blend.before-uvflip'
out = []
try:
    import numpy as np
    cur = {o.name: o.data for o in bpy.data.objects if o.type == 'MESH'}
    with bpy.data.libraries.load(OLD, link=False) as (src, dst):
        dst.objects = list(src.objects)
    import re
    old = {re.sub(r'\.\d{3}$', '', o.name): o.data for o in dst.objects if o and o.type == 'MESH'}
    for name in sorted(cur):
        a = cur[name]; b = old.get(name)
        if b is None or not a.uv_layers or not b.uv_layers or len(a.loops) != len(b.loops):
            out.append(f"{name}: not comparable"); continue
        ua = np.empty(len(a.loops) * 2, np.float32); a.uv_layers[0].data.foreach_get('uv', ua); ua = ua.reshape(-1, 2)
        ub = np.empty(len(b.loops) * 2, np.float32); b.uv_layers[0].data.foreach_get('uv', ub); ub = ub.reshape(-1, 2)
        same = (np.abs(ua - ub).max(1) < 1e-4).mean(); flip = (np.abs(ua[:, 1] - (1 - ub[:, 1])) < 1e-4).mean()
        mats = ",".join(sorted({m.name for m in a.materials if m}))[:70]
        out.append(f"{name:28} same {same:4.0%} flip {flip:4.0%}  {mats}")
except Exception:
    out.append("ERROR\n" + traceback.format_exc())
open(T + r'\blendcmp.txt', 'w').write("\n".join(out))
