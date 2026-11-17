# Diagnostic: temporarily remove the stock armor_fx.effect attachment (slot 5) to see if it is the always-on flame.
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
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("before=%d"%att.Elements.Count)
    idx=None
    for i in range(att.Elements.Count):
        t=field(att.Elements[i].Fields,'type')
        try: rp=str(t.Path.RelativePathWithExtension)
        except Exception: rp=''
        if rp.endswith(r"armor_fx\armor_fx.effect"): idx=i; break
    L("stock armor_fx attachment idx=%r"%idx)
    if idx is not None: att.RemoveElement(idx)
    tf.Save(); tf.Dispose()
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    L("after=%d"%field(obj,'attachments').Elements.Count)
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'remove_stock_effect.txt'),'w',encoding='utf-8').write("\n".join(out))
