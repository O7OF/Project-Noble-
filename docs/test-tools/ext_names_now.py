# Check ext tag location names only.
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
    for t in range(5):
        tf=load(r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t)
        loc=field(tf.Fields,'locations')
        L("ext%d: "%t+", ".join(str(field(loc.Elements[i].Fields,'marker name').Data) for i in range(loc.Elements.Count)))
        tf.Dispose()
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'ext_names_now.txt'),'w',encoding='utf-8').write("\n".join(out))
