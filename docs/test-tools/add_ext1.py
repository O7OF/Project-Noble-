# DIY: add only ext1 back next to ext0 (both after original 8 attachments; stock effect stays removed).
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
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("before=%d"%att.Elements.Count)
    el=att.DuplicateElement(5 if False else att.Elements.Count-1)
    setref(field(el.Fields,'type'), r"objects\characters\spartans\fx\armor_fx\armor_fx_ext1.effect")
    L("after=%d"%att.Elements.Count)
    tf.Save(); tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'add_ext1.txt'),'w',encoding='utf-8').write("\n".join(out))
