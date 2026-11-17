# List marker group names in the Foundry markers-only tag.
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
    tf=load(r"objects\characters\spartans\spartans_pri\spartans_pri.render_model")
    groups=field(tf.Fields,'marker groups'); L("groups=%d"%groups.Elements.Count)
    names=[]
    for i in range(groups.Elements.Count):
        nm=str(field(groups.Elements[i].Fields,'name').Data)
        if nm.startswith('flame_pri_') or nm.startswith('fx_'): names.append((i,nm))
    L("flame markers=%d"%len(names))
    for i,nm in names: L("  %d %r"%(i,nm))
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'pri_marker_names.txt'),'w',encoding='utf-8').write("\n".join(out))
