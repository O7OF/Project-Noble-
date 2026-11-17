# Isolate marker-group/permutation support: add a NEW permutation armor_fx_07 to the EXISTING armor_fx region and a NEW
# marker group test_ext_marker bound to it; pmcg id 241 -> armor_fx/armor_fx_07; ext0 event0 -> stock particle there.
import os, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out=[]; L=out.append
def load(rel):
    base, ext=os.path.splitext(rel)
    tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
def setref(f, rel):
    base, ext=os.path.splitext(rel); f.Path=B.Tags.TagPath.FromPathAndExtension(base, ext[1:])
try:
    # 1) render model: add permutation + marker group
    rel=r"objects\characters\spartans\spartans.render_model"
    tf=load(rel)
    regions=field(tf.Fields,'regions'); region=None
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        if str(field(fs,'name').Data)=='armor_fx': region=regions.Elements[i]; break
    perms=field(region.Fields,'permutations')
    names=[str(field(perms.Elements[i].Fields,'name').Data) for i in range(perms.Elements.Count)]
    if 'armor_fx_07' not in names:
        el=perms.DuplicateElement(0); field(el.Fields,'name').SetStringData('armor_fx_07'); L("added perm armor_fx_07")
    groups=field(tf.Fields,'marker groups')
    gnames=[str(field(groups.Elements[i].Fields,'name').Data) for i in range(groups.Elements.Count)]
    if 'test_ext_marker' not in gnames:
        el=groups.DuplicateElement(0); fs=el.Fields
        field(fs,'name').SetStringData('test_ext_marker')
        m=field(fs,'markers').Elements[0].Fields
        field(m,'region index').Data=0; field(m,'permutation index').Data=7
        L("added marker group test_ext_marker region0 perm7")
    tf.Save(); tf.Dispose()
    # 2) pmcg id241 entry (index8) -> armor_fx/armor_fx_07
    rel=r"globals\player_model_customization.player_model_customization_globals"
    tf=load(rel); block=field(tf.Fields,'spartan armor effect selections')
    fs=block.Elements[8].Fields
    L("pmcg idx8 selection=%s"%field(fs,'selection name').Data)
    bits=field(fs,'customized bits').Elements[0].Fields
    field(bits,'region name').SetStringData('armor_fx'); field(bits,'permutation name').SetStringData('armor_fx_07')
    tf.Save(); tf.Dispose(); L("pmcg idx8 -> armor_fx/armor_fx_07")
    # 3) ext0 event0/location0 -> stock particle at test_ext_marker
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect"
    tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    field(loc.Elements[0].Fields,'marker name').SetStringData('test_ext_marker')
    e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
    setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle")
    field(ps,'location').Value=0
    tf.Save(); tf.Dispose(); L("ext0 event0 -> stock particle at test_ext_marker")
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','existing_region_test.txt')),'w',encoding='utf-8').write("\n".join(out))
