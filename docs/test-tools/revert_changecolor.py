# Revert diagnostic: armor_fx attachment change color back to none.
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
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel)
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments')
    for i in range(att.Elements.Count):
        ft=field(att.Elements[i].Fields,'type')
        try: rp=str(ft.Path.RelativePathWithExtension)
        except Exception: rp=''
        if rp.endswith(r"armor_fx\armor_fx.effect"):
            cc=field(att.Elements[i].Fields,'change color'); L("before=%s"%cc.Value); cc.Value=0; break
    tf.Save(); tf.Dispose()
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields; att=field(obj,'attachments')
    for i in range(att.Elements.Count):
        ft=field(att.Elements[i].Fields,'type')
        try: rp=str(ft.Path.RelativePathWithExtension)
        except Exception: rp=''
        if rp.endswith(r"armor_fx\armor_fx.effect"):
            L("after=%s"%field(att.Elements[i].Fields,'change color').Value); break
    tf.Dispose(); mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','revert_changecolor.txt')),'w',encoding='utf-8').write("\n".join(out))
