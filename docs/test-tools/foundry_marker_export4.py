import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\foundry_marker_export4.txt"
lines=[]
def L(s): lines.append(str(s))
try:
    sc = bpy.context.scene.nwo
    ex = bpy.context.scene.nwo_export
    L("export flags render=%r markers=%r skeleton=%r force=%r lightmap_structure=%r"%(ex.export_render,ex.export_markers,ex.export_skeleton,ex.import_force,ex.lightmap_structure))
    ex.export_render=True; ex.export_markers=True; ex.export_skeleton=True; ex.import_force=True
    try: ex.lightmap_structure=True
    except Exception as e: L("lightmap_structure set err %s"%e)
    L("after flags force=%r lightmap_structure=%r"%(ex.import_force,ex.lightmap_structure))
    sc.asset_type='model'
    sc.sidecar_path=r"objects\characters\spartans\spartans_rt4\spartans_rt4.sidecar.xml"
    sc.template_render_model=r"objects\characters\spartans\spartans.render_model"
    sc.render_model_path=r"objects\characters\spartans\spartans.render_model"
    L("asset=%r"%sc.asset_name)
    try:
        res=bpy.ops.nwo.export_scene(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_rt4\spartans_rt4.asset")
        L("export=%r"%str(res))
    except Exception as e:
        L("export EXC %s"%str(e)[:500])
    for p in (r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt4\spartans_rt4.render_model",
              r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt4\spartans_rt4.model"):
        L("%s exists=%r size=%r"%(p,os.path.exists(p),(os.path.getsize(p) if os.path.exists(p) else 0)))
    bpy.ops.wm.save_as_mainfile(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx_test.blend")
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
