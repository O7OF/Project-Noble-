import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\elite_blend_inspect.txt"
lines=[]
def L(s): lines.append(str(s))
try:
    sc = bpy.context.scene.nwo
    L("asset_type=%r" % getattr(sc,'asset_type','?'))
    for prop in ('render_model_path','template_render_model','is_child_asset','sidecar_path','parent_asset'):
        try: L("%s=%r"%(prop,getattr(sc,prop)))
        except Exception as e: L("%s ERR %s"%(prop,e))
    L("regions=%s"%[e.name for e in sc.regions_table])
    L("perms=%s"%[e.name for e in sc.permutations_table])
    L("objects=%d meshes=%d empties=%d"%(len(bpy.data.objects), len([o for o in bpy.data.objects if o.type=='MESH']), len([o for o in bpy.data.objects if o.type=='EMPTY'])))
    for o in list(bpy.data.objects)[:80]:
        L("obj %s type=%s parent=%s"%(o.name,o.type,o.parent.name if o.parent else None))
    L("blend=%s"%bpy.data.filepath)
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
