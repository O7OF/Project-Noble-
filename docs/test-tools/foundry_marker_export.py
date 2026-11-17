import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\foundry_marker_export_test.txt"
lines=[]
def L(s): lines.append(str(s))
try:
    sc = bpy.context.scene.nwo
    L("body-ish objects: %r"%[o.name for o in bpy.data.objects if 'body' in o.name.lower()][:20])
    if 'armor_fx_b' not in [e.name for e in sc.regions_table]:
        e=sc.regions_table.add(); e.name='armor_fx_b'; L("region table + armor_fx_b")
    if 'armor_fx_b_00' not in [e.name for e in sc.permutations_table]:
        e=sc.permutations_table.add(); e.name='armor_fx_b_00'; L("perm table + armor_fx_b_00")
    arm = next((o for o in bpy.data.objects if o.type=='ARMATURE'), None)
    L("arm=%r"%(arm.name if arm else None))
    nm='armor_fx_b_00 armor_fx_b'
    if nm not in bpy.data.objects:
        me=bpy.data.meshes.new(nm)
        me.from_pydata([(0,0,0),(0.0005,0,0),(0,0.0005,0)],[],[(0,1,2)])
        o=bpy.data.objects.new(nm, me)
        bpy.context.scene.collection.objects.link(o)
        if arm: o.parent=arm
        try: o.nwo.region_name='armor_fx_b'; o.nwo.permutation_name='armor_fx_b_00'
        except Exception as e: L("nwo mesh err %s"%e)
        L("mesh "+nm)
    src=bpy.data.objects.get('armor_fx_00')
    L("src marker=%r"%(src.name if src else None))
    if src:
        m=src.copy(); m.name='fx_test_marker_b'
        if src.data: m.data=src.data.copy()
        bpy.context.scene.collection.objects.link(m)
        n=m.nwo; n.marker_uses_regions=True; n.region_name='armor_fx_b'
        n.marker_permutation_type='include'; n.marker_permutations.clear(); n.marker_permutations.add().name='armor_fx_b_00'
        L("marker fx_test_marker_b")
    sc.asset_type='model'
    sc.sidecar_path=r"objects\characters\spartans\spartans_rt\spartans_rt.sidecar.xml"
    sc.template_render_model=r"objects\characters\spartans\spartans.render_model"
    sc.render_model_path=r"objects\characters\spartans\spartans.render_model"
    L("asset name=%r"%sc.asset_name)
    try:
        res=bpy.ops.nwo.export_scene(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_rt\spartans_rt.asset")
        L("export=%r"%str(res))
    except Exception as e:
        L("export EXC %s"%str(e)[:400])
    for p in (r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt\spartans_rt.render_model",
              r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt\spartans_rt.model",
              r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_rt\spartans_rt.sidecar.xml"):
        L("%s exists=%r size=%r"%(p,os.path.exists(p),(os.path.getsize(p) if os.path.exists(p) else 0)))
    bpy.ops.wm.save_as_mainfile(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx_test.blend")
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
