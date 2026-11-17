# Clean up after the region/marker diagnostic: attachments back to 9, pmcg idx8 back to ext mapping, remove test
# marker group + test permutation, ext0 event0 back to its custom particle/marker.
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
    # biped
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("biped before=%d"%att.Elements.Count)
    while att.Elements.Count>9: att.RemoveElement(att.Elements.Count-1)
    tf.Save(); tf.Dispose(); L("biped -> 9")
    # pmcg idx8 back
    rel=r"globals\player_model_customization.player_model_customization_globals"
    tf=load(rel); block=field(tf.Fields,'spartan armor effect selections')
    bits=field(block.Elements[8].Fields,'customized bits').Elements[0].Fields
    field(bits,'region name').SetStringData('armor_fx_ext'); field(bits,'permutation name').SetStringData('armor_fx_ext_01')
    tf.Save(); tf.Dispose(); L("pmcg idx8 -> armor_fx_ext/armor_fx_ext_01")
    # render model remove test marker group + test perm
    rel=r"objects\characters\spartans\spartans.render_model"
    tf=load(rel); regions=field(tf.Fields,'regions'); region=None
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        if str(field(fs,'name').Data)=='armor_fx': region=regions.Elements[i]; break
    perms=field(region.Fields,'permutations')
    for i in range(perms.Elements.Count-1,-1,-1):
        nm=str(field(perms.Elements[i].Fields,'name').Data)
        if nm=='armor_fx_07': perms.RemoveElement(i); L("removed perm armor_fx_07")
    groups=field(tf.Fields,'marker groups')
    for i in range(groups.Elements.Count-1,-1,-1):
        if str(field(groups.Elements[i].Fields,'name').Data)=='test_ext_marker':
            groups.RemoveElement(i); L("removed marker group test_ext_marker")
    tf.Save(); tf.Dispose()
    # ext0 event0 back
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect"
    tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    field(loc.Elements[0].Fields,'marker name').SetStringData('armor_fx_ext_t0_0')
    e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
    setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_default.particle")
    field(ps,'location').Value=0
    tf.Save(); tf.Dispose(); L("ext0 restored")
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','cleanup_region_test.txt')),'w',encoding='utf-8').write("\n".join(out))
