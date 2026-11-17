# Inspect markers-only exported render model.
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
try:
    tf=load(r"objects\characters\spartans\spartans_rt5\spartans_rt5.render_model")
    regions=field(tf.Fields,'regions'); L("regions=%d"%regions.Elements.Count)
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields; nm=str(field(fs,'name').Data)
        perms=field(fs,'permutations')
        L("  %s perms=%d"%(nm,perms.Elements.Count if perms else 0))
    groups=field(tf.Fields,'marker groups'); L("marker groups=%d"%groups.Elements.Count)
    for i in range(groups.Elements.Count):
        fs=groups.Elements[i].Fields; nm=str(field(fs,'name').Data)
        if 'test' in nm or nm in ('armor_fx_00','armor_fx_06'):
            m=field(fs,'markers'); L("  group %s markers=%d"%(nm,m.Elements.Count if m else 0))
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'inspect_rt5_rm.txt'),'w',encoding='utf-8').write("\n".join(out))
