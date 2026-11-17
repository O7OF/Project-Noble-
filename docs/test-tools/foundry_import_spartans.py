import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\foundry_import_test.txt"
lines=[]
def L(s): lines.append(str(s))
try:
    bpy.ops.preferences.addon_enable(module="bl_ext.iloveagoodcrisp_github_io.io_scene_foundry")
    from bl_ext.iloveagoodcrisp_github_io.io_scene_foundry import utils as fu
    L("project=%r valid=%r"%(fu.get_project_path(), fu.current_project_valid()))
    L("tags=%r data=%r"%(fu.get_tags_path(), fu.get_data_path()))
    tag = r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans.render_model"
    L("exists=%r"%os.path.exists(tag))
    res = bpy.ops.nwo.foundry_import(filepath=tag)
    L("import result=%r"%str(res))
    sc = bpy.context.scene.nwo
    for prop in ('asset_type','render_model_path','template_render_model','sidecar_path','template_model','is_child_asset'):
        try: L("%s=%r"%(prop,getattr(sc,prop)))
        except Exception as e: L("%s ERR %s"%(prop,e))
    L("objects=%d meshes=%d empties=%d armatures=%d"%(len(bpy.data.objects),len([o for o in bpy.data.objects if o.type=='MESH']),len([o for o in bpy.data.objects if o.type=='EMPTY']),len([o for o in bpy.data.objects if o.type=='ARMATURE'])))
    for o in list(bpy.data.objects)[:40]:
        L("obj %s type=%s parent=%s"%(o.name,o.type,o.parent.name if o.parent else None))
    L("markers=%r"%[o.name for o in bpy.data.objects if o.type=='EMPTY'][:40])
    L("regions=%s"%[e.name for e in sc.regions_table][:20])
    L("perms=%s"%[e.name for e in sc.permutations_table][:20])
    outblend = r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx_test.blend"
    os.makedirs(os.path.dirname(outblend), exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=outblend)
    L("saved %r"%outblend)
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
