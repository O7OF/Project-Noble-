import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\foundry_marker_export3.txt"
lines=[]
def L(s): lines.append(str(s))
try:
    sc = bpy.context.scene.nwo
    L("objects armor_fx_b? %r"%[o.name for o in bpy.data.objects if 'armor_fx_b' in o.name or 'fx_test' in o.name])
    sc.asset_type='model'
    sc.sidecar_path=r"objects\characters\spartans\spartans_rt3\spartans_rt3.sidecar.xml"
    sc.template_render_model=''
    sc.render_model_path=''
    L("asset name=%r template=%r"%(sc.asset_name,sc.template_render_model))
    try:
        res=bpy.ops.nwo.export_scene(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_rt3\spartans_rt3.asset")
        L("export=%r"%str(res))
    except Exception as e:
        L("export EXC %s"%str(e)[:500])
    for p in (r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt3\spartans_rt3.render_model",
              r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt3\spartans_rt3.model"):
        L("%s exists=%r size=%r"%(p,os.path.exists(p),(os.path.getsize(p) if os.path.exists(p) else 0)))
    bpy.ops.wm.save_as_mainfile(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx_test.blend")
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
