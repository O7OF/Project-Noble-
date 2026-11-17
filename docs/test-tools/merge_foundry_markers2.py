# Merge Foundry marker group into stock render model bound to the existing armor_fx_ext region (region count capped at 16),
# then wire pmcg id241 and ext0 for an end-to-end test.
import os,traceback
from mbstart import B, mb_stop
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out=[]
def L(s): out.append(str(s))
def load(rel):
    base,ext=os.path.splitext(rel); tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base,ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
def setref(f, rel):
    base,ext=os.path.splitext(rel); f.Path=B.Tags.TagPath.FromPathAndExtension(base,ext[1:])
try:
    # find stock armor_fx_ext region index
    tf=load(r"objects\characters\spartans\spartans.render_model")
    regions=field(tf.Fields,'regions'); ext_idx=None
    for i in range(regions.Elements.Count):
        if str(field(regions.Elements[i].Fields,'name').Data)=='armor_fx_ext': ext_idx=i
    L("stock regions=%d armor_fx_ext idx=%r"%(regions.Elements.Count,ext_idx))
    # copy marker group from rt5
    tfs=load(r"objects\characters\spartans\spartans_rt5\spartans_rt5.render_model")
    gs=field(tfs.Fields,'marker groups'); idx=None
    for i in range(gs.Elements.Count):
        if str(field(gs.Elements[i].Fields,'name').Data)=='fx_test_marker_b': idx=i; break
    L("rt5 marker idx=%r"%idx); gs.CopyElement(idx); tfs.Dispose()
    groups=field(tf.Fields,'marker groups')
    groups.PasteAppendElement(); L("stock marker groups=%d"%groups.Elements.Count)
    merged=groups.Elements[groups.Elements.Count-1].Fields
    m=field(merged,'markers').Elements[0].Fields
    L("merged name=%r region=%s perm=%s"%(str(field(merged,'name').Data),field(m,'region index').Data,field(m,'permutation index').Data))
    field(m,'region index').Data=ext_idx; field(m,'permutation index').Data=1
    tf.Save(); tf.Dispose(); L("patched merged marker to region %d perm 1"%ext_idx)
    # pmcg id241 -> armor_fx_ext/armor_fx_ext_01
    tf=load(r"globals\player_model_customization.player_model_customization_globals")
    block=field(tf.Fields,'spartan armor effect selections')
    bits=field(block.Elements[8].Fields,'customized bits').Elements[0].Fields
    field(bits,'region name').SetStringData('armor_fx_ext'); field(bits,'permutation name').SetStringData('armor_fx_ext_01')
    tf.Save(); tf.Dispose(); L("pmcg idx8 -> armor_fx_ext/armor_fx_ext_01")
    # ext0 event0 marker/particle
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect"
    tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    field(loc.Elements[0].Fields,'marker name').SetStringData('fx_test_marker_b')
    e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
    setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle")
    field(ps,'location').Value=0
    tf.Save(); tf.Dispose(); L("ext0 event0 -> stock particle at fx_test_marker_b")
    # biped ext0 attachment slot9
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("biped attachments before=%d"%att.Elements.Count)
    if att.Elements.Count==9:
        el=att.DuplicateElement(5); setref(field(el.Fields,'type'), r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect")
    tf.Save(); tf.Dispose(); L("biped attachments after edit")
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'merge_foundry_markers2.txt'),'w',encoding='utf-8').write("\n".join(out))
