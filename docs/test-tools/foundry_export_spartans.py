import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\foundry_export_test.txt"
lines=[]
def L(s): lines.append(str(s))
try:
    sc = bpy.context.scene.nwo
    L("before: asset_type=%r sidecar=%r template=%r render_path=%r"%(sc.asset_type,sc.sidecar_path,sc.template_render_model,sc.render_model_path))
    sc.asset_type='model'
    sc.sidecar_path=r"objects\characters\spartans\spartans_fx.sidecar.xml"
    sc.template_render_model=r"objects\characters\spartans\spartans.render_model"
    sc.render_model_path=r"objects\characters\spartans\spartans.render_model"
    L("after: asset=%r name=%r"%(sc.asset_type,sc.asset_name))
    try:
        res=bpy.ops.nwo.export_scene(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx.asset")
        L("export result=%r"%str(res))
    except Exception as e:
        L("export EXC %s"%str(e)[:400])
    for p in (r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_fx.render_model",
              r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_fx.model",
              r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_fx.sidecar.xml",
              r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx.sidecar.xml"):
        L("%s exists=%r size=%r"%(p,os.path.exists(p),(os.path.getsize(p) if os.path.exists(p) else 0)))
    bpy.ops.wm.save_as_mainfile(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx_test.blend")
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
