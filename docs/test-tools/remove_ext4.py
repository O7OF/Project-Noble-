# Remove ext4 attachment (test 4 extra attachments) from current 11-attachment biped.
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
    tf=load(r"objects\characters\spartans\spartans.biped")
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("before=%d"%att.Elements.Count)
    for i in range(att.Elements.Count-1,-1,-1):
        rp=str(field(att.Elements[i].Fields,'type').Path.RelativePathWithExtension)
        if rp.endswith('armor_fx_ext4.effect'): att.RemoveElement(i); L("removed ext4")
    L("after=%d"%att.Elements.Count)
    tf.Save(); tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'remove_ext4.txt'),'w',encoding='utf-8').write("\n".join(out))
