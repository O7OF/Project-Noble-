# Merge the Foundry markers-only render model's new marker group + region into the stock spartans.render_model,
# then wire pmcg id241 -> armor_fx_b/armor_fx_b_00 and ext0's event at fx_test_marker_b with the stock particle.
import os,traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
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
    # 1) copy marker group from rt5 into stock render model
    tfs=load(r"objects\characters\spartans\spartans_rt5\spartans_rt5.render_model")
    gs=field(tfs.Fields,'marker groups'); idx=None
    for i in range(gs.Elements.Count):
        if str(field(gs.Elements[i].Fields,'name').Data)=='fx_test_marker_b': idx=i; break
    L("rt5 marker idx=%r"%idx)
    gs.CopyElement(idx)
    tf=load(r"objects\characters\spartans\spartans.render_model")
    groups=field(tf.Fields,'marker groups')
    L("stock groups before=%d"%groups.Elements.Count)
    groups.PasteAppendElement()
    L("stock groups after=%d"%groups.Elements.Count)
    merged=groups.Elements[groups.Elements.Count-1].Fields
    L("merged name=%r"%str(field(merged,'name').Data))
    # 2) add region armor_fx_b (duplicate armor_fx region, rename) and first perm
    regions=field(tf.Fields,'regions')
    region_idx=regions.Elements.Count
    rel_el=regions.DuplicateElement(0); rfs=rel_el.Fields
    field(rfs,'name').SetStringData('armor_fx_b')
    perms=field(rfs,'permutations')
    L("dup region perms=%d"%perms.Elements.Count)
    for j in range(perms.Elements.Count):
        field(perms.Elements[j].Fields,'name').SetStringData('armor_fx_b_%02d'%j)
    L("added region armor_fx_b idx=%d"%region_idx)
    # 3) patch merged marker region/permutation indices
    m=field(merged,'markers').Elements[0].Fields
    L("marker before region=%s perm=%s"%(field(m,'region index').Data,field(m,'permutation index').Data))
    field(m,'region index').Data=region_idx; field(m,'permutation index').Data=0
    tf.Save(); tf.Dispose(); tfs.Dispose()
    # 4) pmcg id241 entry -> armor_fx_b/armor_fx_b_00
    rel=r"globals\player_model_customization.player_model_customization_globals"
    tf=load(rel); block=field(tf.Fields,'spartan armor effect selections')
    bits=field(block.Elements[8].Fields,'customized bits').Elements[0].Fields
    field(bits,'region name').SetStringData('armor_fx_b'); field(bits,'permutation name').SetStringData('armor_fx_b_00')
    tf.Save(); tf.Dispose(); L("pmcg idx8 -> armor_fx_b/armor_fx_b_00")
    # 5) ext0 event0 -> marker fx_test_marker_b + stock particle; keep location 0
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect"
    tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    field(loc.Elements[0].Fields,'marker name').SetStringData('fx_test_marker_b')
    e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
    setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle")
    field(ps,'location').Value=0
    tf.Save(); tf.Dispose(); L("ext0 event0 -> stock particle at fx_test_marker_b")
    # 6) add ext0 attachment at biped slot 9
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("biped attachments before=%d"%att.Elements.Count)
    if att.Elements.Count==9:
        el=att.DuplicateElement(5); setref(field(el.Fields,'type'), r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect")
        L("biped attachments after=%d"%att.Elements.Count)
    tf.Save(); tf.Dispose()
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'merge_foundry_markers.txt'),'w',encoding='utf-8').write("\n".join(out))
