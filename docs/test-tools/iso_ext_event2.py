# Isolation: ext4 event2 (magenta variant) particle -> existing head_fire_blue.particle (known-good, already in map).
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
    e=ev.Elements[2].Fields; ps=field(e,'particle systems').Elements[0].Fields
    L("event2 %s loc=%s old=%s"%(field(e,'event name').Data,field(ps,'location').Value,field(ps,'particle').Path.RelativePathWithExtension))
    setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_blue.particle")
    tf.Save(); tf.Dispose()
    tf=load(rel); ev=field(tf.Fields,'events'); e=ev.Elements[2].Fields; ps=field(e,'particle systems').Elements[0].Fields
    L("now: %s -> %s"%(field(e,'event name').Data,field(ps,'particle').Path.RelativePathWithExtension))
    tf.Dispose(); mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','iso_ext_event2.txt')),'w',encoding='utf-8').write("\n".join(out))
