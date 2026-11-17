# Step 1: add region armor_fx_b with a single permutation to stock render model using AddElement; save and verify.
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
try:
    rel=r"objects\characters\spartans\spartans.render_model"
    tf=load(rel); regions=field(tf.Fields,'regions')
    L("regions before=%d"%regions.Elements.Count)
    el=regions.AddElement(); rfs=el.Fields
    field(rfs,'name').SetStringData('armor_fx_b')
    perms=field(rfs,'permutations')
    pe=perms.AddElement(); field(pe.Fields,'name').SetStringData('armor_fx_b_00')
    L("added region idx=%d perms=%d"%(regions.Elements.Count-1,perms.Elements.Count))
    tf.Save(); tf.Dispose()
    tf=load(rel); regions=field(tf.Fields,'regions'); L("after reload regions=%d"%regions.Elements.Count)
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        nm=str(field(fs,'name').Data)
        if nm=='armor_fx_b':
            p=field(fs,'permutations'); L("armor_fx_b idx=%d perms=%r"%(i,[str(field(p.Elements[j].Fields,'name').Data) for j in range(p.Elements.Count)]))
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'add_region_probe.txt'),'w',encoding='utf-8').write("\n".join(out))
