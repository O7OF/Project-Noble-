# Inspect pmcg entries 7..36, model marker groups and ext4 event particles for mapping consistency.
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
try:
    tf=load(r"globals\player_model_customization.player_model_customization_globals")
    block=field(tf.Fields,'spartan armor effect selections')
    L("pmcg entries=%d"%block.Elements.Count)
    for i in range(7,block.Elements.Count):
        fs=block.Elements[i].Fields
        nm=str(field(fs,'selection name').Data)
        bits=field(fs,'customized bits')
        ch=[]
        for k in range(bits.Elements.Count):
            cfs=bits.Elements[k].Fields
            ch.append(str(field(cfs,'region name').Data)+"/"+str(field(cfs,'permutation name').Data))
        L("  %d %s -> %s"%(i,nm,ch))
    tf.Dispose()
    tf=load(r"objects\characters\spartans\spartans.render_model")
    regions=field(tf.Fields,'regions'); region_idx=None
    for i in range(regions.Elements.Count):
        if str(field(regions.Elements[i].Fields,'name').Data)=='armor_fx_ext': region_idx=i
    groups=field(tf.Fields,'marker groups')
    L("\nregion ext idx=%s; marker groups:"%region_idx)
    for i in range(groups.Elements.Count):
        nm=str(field(groups.Elements[i].Fields,'name').Data)
        if nm.startswith('armor_fx_ext'):
            mk=field(groups.Elements[i].Fields,'markers').Elements[0].Fields
            L("  %s region=%s perm=%s"%(nm,field(mk,'region index').Data,field(mk,'permutation index').Data))
    tf.Dispose()
    tf=load(r"objects\characters\spartans\fx\armor_fx\armor_fx_ext4.effect")
    ev=field(tf.Fields,'events'); loc=field(tf.Fields,'locations')
    L("\next4 locations: "+", ".join(str(field(loc.Elements[i].Fields,'marker name').Data) for i in range(loc.Elements.Count)))
    for i in range(ev.Elements.Count):
        fs=ev.Elements[i].Fields; ps=field(fs,'particle systems').Elements[0].Fields
        L("  ev%d %s loc=%s -> %s"%(i,field(fs,'event name').Data,field(ps,'location').Value,field(ps,'particle').Path.RelativePathWithExtension))
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','mapping_check.txt')),'w',encoding='utf-8').write("\n".join(out))
