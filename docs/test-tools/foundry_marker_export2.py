import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\foundry_marker_export2.txt"
lines=[]
def L(s): lines.append(str(s))
try:
    sc = bpy.context.scene.nwo
    src=bpy.data.objects.get('armor_fx_00'); L("src=%r collections=%r"%(src.name if src else None,[c.name for c in src.users_collection] if src else None))
    coll = src.users_collection[0]
    # clean previous test objects
    for nm in ('armor_fx_b_00 armor_fx_b','fx_test_marker_b'):
        o=bpy.data.objects.get(nm)
        if o: bpy.data.objects.remove(o, do_unlink=True); L("removed "+nm)
    if 'armor_fx_b' not in [e.name for e in sc.regions_table]:
        e=sc.regions_table.add(); e.name='armor_fx_b'
    if 'armor_fx_b_00' not in [e.name for e in sc.permutations_table]:
        e=sc.permutations_table.add(); e.name='armor_fx_b_00'
    me=bpy.data.meshes.new('armor_fx_b_00 armor_fx_b')
    me.from_pydata([(0,0,0),(0.0005,0,0),(0,0.0005,0)],[],[(0,1,2)])
    o=bpy.data.objects.new('armor_fx_b_00 armor_fx_b', me); coll.objects.link(o)
    o.parent=next((x for x in bpy.data.objects if x.type=='ARMATURE'),None)
    o.nwo.region_name='armor_fx_b'; o.nwo.permutation_name='armor_fx_b_00'
    L("mesh in %r"%coll.name)
    m=src.copy(); m.name='fx_test_marker_b'
    if src.data: m.data=src.data.copy()
    coll.objects.link(m)
    n=m.nwo; n.marker_uses_regions=True; n.region_name='armor_fx_b'
    n.marker_permutation_type='include'; n.marker_permutations.clear(); n.marker_permutations.add().name='armor_fx_b_00'
    L("marker in %r"%coll.name)
    sc.asset_type='model'
    sc.sidecar_path=r"objects\characters\spartans\spartans_rt2\spartans_rt2.sidecar.xml"
    sc.template_render_model=r"objects\characters\spartans\spartans.render_model"
    sc.render_model_path=r"objects\characters\spartans\spartans.render_model"
    L("asset=%r name=%r"%(sc.asset_type,sc.asset_name))
    try:
        res=bpy.ops.nwo.export_scene(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_rt2\spartans_rt2.asset")
        L("export=%r"%str(res))
    except Exception as e:
        L("export EXC %s"%str(e)[:400])
    for p in (r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt2\spartans_rt2.render_model",
              r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_rt2\spartans_rt2.model"):
        L("%s exists=%r size=%r"%(p,os.path.exists(p),(os.path.getsize(p) if os.path.exists(p) else 0)))
    bpy.ops.wm.save_as_mainfile(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx_test.blend")
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
