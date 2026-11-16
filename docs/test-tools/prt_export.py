# Re-export elite_noble with "Light Model (PRT)" on, so tool imports WITHOUT "draft" and bakes PRT ambient occlusion
# (the stock Elite has PRT Ambient; ours had none -> armour looked bright/shiny everywhere). The .blend is NOT saved.
import bpy, sys, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\prt_export_log.txt'
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
try:
    sc = bpy.context.scene
    ex = getattr(sc, 'nwo_export', None)
    if ex is None:
        for name in dir(sc):
            v = getattr(sc, name, None)
            if hasattr(v, 'lightmap_structure'): ex = v; L('export settings at scene.' + name); break
    L('lightmap_structure before: ' + str(getattr(ex, 'lightmap_structure', 'n/a')))
    ex.lightmap_structure = True
    L('lightmap_structure now: ' + str(ex.lightmap_structure))
    if '--export' in sys.argv:
        L('EXPORT ' + str(bpy.ops.nwo.export_scene()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
