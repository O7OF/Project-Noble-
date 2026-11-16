# Fix 2026-11-12: elite_noble's UVs are V-flipped vs Reach's (tool export-render-model-mesh comparison: ours = (u, 1-v)
# of stock) -> every texture sampled upside down (dark, colours misplaced, normal maps wrong = "shiny"). Flip V on every
# mesh UV layer, SAVE the blend (PS script backs it up first), then export with "Light Model (PRT)" on.
import bpy, sys, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\uvflip_log.txt'
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
try:
    import numpy as np
    done = 0
    for me in {o.data for o in bpy.data.objects if o.type == 'MESH'}:
        for uvl in me.uv_layers:
            uv = np.empty(len(me.loops) * 2, np.float32); uvl.data.foreach_get('uv', uv)
            uv[1::2] = 1.0 - uv[1::2]
            uvl.data.foreach_set('uv', uv); done += 1
        me.update()
    L(f'flipped V on {done} uv layers')
    bpy.ops.wm.save_mainfile(); L('SAVED')
    ex = bpy.context.scene.nwo_export
    ex.lightmap_structure = True; L('lightmap_structure ' + str(ex.lightmap_structure))
    if '--export' in sys.argv:
        L('EXPORT ' + str(bpy.ops.nwo.export_scene()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
