# Diagnostic 2: ext4 event0 uses the STOCK head_fire.particle at stock marker armor_fx_00 -> tests whether the extra
# effect tag/attachment spawns when the particle asset is known-good.
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
    e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
    setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle")
    field(ps,'location').Value=0
    tf.Save(); tf.Dispose()
    tf=load(rel); ev=field(tf.Fields,'events')
    e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
    L("event0 %s loc=%s -> %s"%(field(e0,'event name').Data,field(ps,'location').Value,field(ps,'particle').Path.RelativePathWithExtension))
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','armor_fx_diag3.txt')),'w',encoding='utf-8').write("\n".join(out))
