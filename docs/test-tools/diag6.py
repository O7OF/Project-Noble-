# Diagnostic 6: ext4 event1 (midnight, location armor_fx_ext_t4_1) with the STOCK particle. Select the midnight variant
# (custom item 266) after rebuild: if orange flames appear, the ext region/marker path works.
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
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext4.effect"
    tf=load(rel); ev=field(tf.Fields,'events')
    e1=ev.Elements[1].Fields; ps=field(e1,'particle systems').Elements[0].Fields
    L("event1 name=%s loc=%s"%(field(e1,'event name').Data,field(ps,'location').Value))
    setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle")
    tf.Save(); tf.Dispose()
    L("set event1 particle to stock head_fire")
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','diag6.txt')),'w',encoding='utf-8').write("\n".join(out))
