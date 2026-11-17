import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\foundry_markers_only.txt"
lines=[]
def L(s): lines.append(str(s))
try:
    sc = bpy.context.scene.nwo
    ex = bpy.context.scene.nwo_export
    ex.export_render=False; ex.export_collision=False; ex.export_physics=False
    ex.export_markers=True; ex.export_skeleton=False
    try: ex.import_force=True
    except Exception: pass
    L("flags render=%r markers=%r skeleton=%r coll=%r phys=%r"%(ex.export_render,ex.export_markers,ex.export_skeleton,ex.export_collision,ex.export_physics))
    L("markers present: %r"%[o.name for o in bpy.data.objects if o.type=='EMPTY' and ('fx_test' in o.name or 'armor_fx_b' in o.name)])
    sc.asset_type='model'
    sc.sidecar_path=r"objects\characters\spartans\spartans_rt5\spartans_rt5.sidecar.xml"
    sc.template_render_model=r"objects\characters\spartans\spartans.render_model"
    sc.render_model_path=r"objects\characters\spartans\spartans.render_model"
    try:
        res=bpy.ops.nwo.export_scene(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_rt5\spartans_rt5.asset")
        L("export=%r"%str(res))
    except Exception as e:
        L("export EXC %s"%str(e)[:500])
    for p in (r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt5\spartans_rt5.render_model",
              r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_rt5\spartans_rt5.sidecar.xml"):
        L("%s exists=%r size=%r"%(p,os.path.exists(p),(os.path.getsize(p) if os.path.exists(p) else 0)))
    bpy.ops.wm.save_as_mainfile(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx_test.blend")
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
