# APPLIED 2026-11-12 00:35 (blend saved) - do NOT run again, it would flip these pieces back.
# 2026-11-12: undo the V flip (uvflip_export.py) on faces whose material uses one of Noble's own 7 Prophets' Hand
# shaders (their textures were already right; Honor Guard shoulders glowed yellow after the flip). Reach-textured
# faces keep the fix. Saves the blend, exports with "Light Model (PRT)".
import bpy, sys, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\uvunflip_log.txt'
PH = ('default_ch_elite_armor_02_honor', 'default_ch_elite_honor_armor_01_illum', 'default_sh_elite_honor_armor_01',
      'elite_combat_armor', 'elite_council_helmet', 'elite_ossoona_helmet', 'honor_lights_ch_elite_armor_emissive')
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
try:
    import numpy as np
    total = 0; objs = []
    for me in {o.data for o in bpy.data.objects if o.type == 'MESH'}:
        phslots = {i for i, m in enumerate(me.materials) if m and any(p in m.name for p in PH)}
        if not phslots or not me.uv_layers: continue
        mi = np.empty(len(me.polygons), np.int32); me.polygons.foreach_get('material_index', mi)
        lt = np.empty(len(me.polygons), np.int32); me.polygons.foreach_get('loop_total', lt)
        sel = np.repeat(np.isin(mi, list(phslots)), lt)
        if not sel.any(): continue
        for uvl in me.uv_layers:
            uv = np.empty(len(me.loops) * 2, np.float32); uvl.data.foreach_get('uv', uv); uv = uv.reshape(-1, 2)
            uv[sel, 1] = 1.0 - uv[sel, 1]
            uvl.data.foreach_set('uv', uv.ravel())
        me.update(); total += int(sel.sum()); objs.append(me.name)
    L(f'unflipped {total} loops in {len(objs)} meshes: ' + ', '.join(objs))
    bpy.ops.wm.save_mainfile(); L('SAVED')
    bpy.context.scene.nwo_export.lightmap_structure = True
    if '--export' in sys.argv:
        L('EXPORT ' + str(bpy.ops.nwo.export_scene()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
