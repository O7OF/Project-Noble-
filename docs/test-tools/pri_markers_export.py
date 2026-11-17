import bpy, os, traceback
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\pri_markers_export.txt"
PRI=["steel","silver","white","brown","tan","khaki","sage","olive","drab","forest","green","sea_foam","teal","aqua","cyan","blue","cobalt","ice","violet","orchid","lavender","maroon","brick","rose","rust","coral","peach","gold","yellow","pale"]
lines=[]
def L(s): lines.append(str(s))
try:
    sc = bpy.context.scene.nwo
    src = bpy.data.objects.get('armor_fx_00')
    L("src=%r collections=%r"%(src.name if src else None,[c.name for c in src.users_collection] if src else []))
    coll = src.users_collection[0]
    # clean old test markers
    for o in list(bpy.data.objects):
        if o.type=='EMPTY' and (o.name.startswith('flame_pri_') or o.name.startswith('fx_test_marker')):
            bpy.data.objects.remove(o, do_unlink=True)
    # region/perm table entries
    if 'armor_fx_pri' not in [e.name for e in sc.regions_table]:
        e=sc.regions_table.add(); e.name='armor_fx_pri'; L("region table + armor_fx_pri")
    perm_names=[e.name for e in sc.permutations_table]
    for i in range(26):
        nm='armor_fx_%02d'%(i+6)
        if nm not in perm_names:
            e=sc.permutations_table.add(); e.name=nm
    for i in range(4):
        nm='armor_fx_pri_%02d'%i
        if nm not in perm_names:
            e=sc.permutations_table.add(); e.name=nm
    L("perms table has armor_fx_06? %r armor_fx_pri_00? %r"%('armor_fx_06' in [e.name for e in sc.permutations_table],'armor_fx_pri_00' in [e.name for e in sc.permutations_table]))
    for i,id in enumerate(PRI):
        o = src.copy(); o.name='flame_pri_%s'%id; o.data=src.data.copy() if src.data else None
        for c in src.users_collection: c.objects.link(o)
        o.matrix_world = src.matrix_world.copy()
        n=o.nwo; n.marker_uses_regions=True; n.marker_permutation_type='include'; n.marker_permutations.clear()
        if i<26:
            n.region_name='armor_fx'; n.marker_permutations.add().name='armor_fx_%02d'%(i+6)
        else:
            n.region_name='armor_fx_pri'; n.marker_permutations.add().name='armor_fx_pri_%02d'%(i-26)
    L("added %d markers"%len(PRI))
    # markers-only export
    ex=bpy.context.scene.nwo_export
    ex.export_render=False; ex.export_collision=False; ex.export_physics=False
    ex.export_markers=True; ex.export_skeleton=False
    sc.asset_type='model'
    sc.sidecar_path=r"objects\characters\spartans\spartans_pri\spartans_pri.sidecar.xml"
    sc.template_render_model=r"objects\characters\spartans\spartans.render_model"
    sc.render_model_path=r"objects\characters\spartans\spartans.render_model"
    try:
        res=bpy.ops.nwo.export_scene(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_pri\spartans_pri.asset")
        L("export=%r"%str(res))
    except Exception as e:
        L("export EXC %s"%str(e)[:500])
    p=r"D:\Steam\steamapps\common\HREK\tags\objects\characters\spartans\spartans_pri\spartans_pri.render_model"
    L("output exists=%r size=%r"%(os.path.exists(p),(os.path.getsize(p) if os.path.exists(p) else 0)))
    bpy.ops.wm.save_as_mainfile(filepath=r"D:\Steam\steamapps\common\HREK\data\objects\characters\spartans\spartans_fx_test.blend")
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(OUT,'w',encoding='utf-8').write("\n".join(lines))
