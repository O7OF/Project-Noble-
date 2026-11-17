# Diagnostic: keep only ext0 extra effect attachment (remove ext1..4), no stock effect attachment.
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
    keep=[]; removed=0
    for i in range(att.Elements.Count):
        t=field(att.Elements[i].Fields,'type')
        try: rp=str(t.Path.RelativePathWithExtension)
        except Exception: rp=''
        if 'armor_fx_ext' in rp:
            L("found %s"%rp)
            if 'armor_fx_ext0' not in rp:
                att.RemoveElement(i); removed+=1; break
    # remove any remaining ext1-4 one by one
    for i in range(att.Elements.Count-1,-1,-1):
        t=field(att.Elements[i].Fields,'type')
        try: rp=str(t.Path.RelativePathWithExtension)
        except Exception: rp=''
        if 'armor_fx_ext' in rp and 'armor_fx_ext0' not in rp:
            att.RemoveElement(i); removed+=1
    L("removed=%d after=%d"%(removed,att.Elements.Count))
    tf.Save(); tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'only_ext0.txt'),'w',encoding='utf-8').write("\n".join(out))
