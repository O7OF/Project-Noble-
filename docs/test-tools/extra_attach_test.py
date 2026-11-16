# Diagnostic: add one extra attachment pointing at the stock armor_fx.effect (known-good tag) to test whether
# engine attachments beyond the original 9 are instantiated at all.
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
def setref(f, rel):
    base, ext=os.path.splitext(rel); f.Path=B.Tags.TagPath.FromPathAndExtension(base, ext[1:])
try:
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel)
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments')
    L("attachments before=%d"%att.Elements.Count)
    # find stock armor_fx attachment
    tpl=None
    for i in range(att.Elements.Count):
        ft=field(att.Elements[i].Fields,'type')
        try: rp=str(ft.Path.RelativePathWithExtension)
        except Exception: rp=''
        if rp.endswith(r"armor_fx\armor_fx.effect"): tpl=i; break
    L("stock armor_fx attachment index=%s"%tpl)
    el=att.DuplicateElement(tpl)
    # ensure type is the stock effect (duplicate does not change)
    tf.Save(); tf.Dispose()
    tf=load(rel)
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("attachments after=%d"%att.Elements.Count)
    for i in range(att.Elements.Count):
        ft=field(att.Elements[i].Fields,'type')
        try: L("  %d %s"%(i,ft.Path.RelativePathWithExtension))
        except Exception: L("  %d <?>"%i)
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','extra_attach_test.txt')),'w',encoding='utf-8').write("\n".join(out))
