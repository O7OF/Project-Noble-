# Inspect armor_fx permutations and brick marker binding.
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
    tf=load(r"objects\characters\spartans\spartans.render_model")
    regions=field(tf.Fields,'regions')
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        if str(field(fs,'name').Data)=='armor_fx':
            p=field(fs,'permutations')
            L("armor_fx perms: "+", ".join(str(field(p.Elements[j].Fields,'name').Data) for j in range(p.Elements.Count)))
        if str(field(fs,'name').Data)=='armor_fx_pri':
            p=field(fs,'permutations')
            L("armor_fx_pri perms: "+", ".join(str(field(p.Elements[j].Fields,'name').Data) for j in range(p.Elements.Count)))
    groups=field(tf.Fields,'marker groups')
    for i in range(groups.Elements.Count):
        fs=groups.Elements[i].Fields
        if str(field(fs,'name').Data)=='flame_pri_brick':
            m=field(fs,'markers').Elements[0].Fields
            L("brick marker region=%s perm=%s"%(field(m,'region index').Data,field(m,'permutation index').Data))
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'brick_map_check.txt'),'w',encoding='utf-8').write("\n".join(out))
