# Diagnostic: point armor_fx_ext4 event0 at the stock location marker armor_fx_00 (existing model marker) to test
# whether the extra biped effect attachment is loaded/spawned at all. Select stock flames and see if copper + orange
# flames both appear (would prove attachments work; no rebuild of model markers needed).
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
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext4.effect"
    tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    field(loc.Elements[0].Fields,'marker name').SetStringData('armor_fx_00')
    e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
    field(ps,'location').Value=0
    tf.Save(); tf.Dispose()
    tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    L("locations: "+", ".join(str(field(loc.Elements[i].Fields,'marker name').Data) for i in range(loc.Elements.Count)))
    for i in range(min(2,ev.Elements.Count)):
        fs=ev.Elements[i].Fields; p0=field(fs,'particle systems').Elements[0].Fields
        L("%d %s loc=%s"%(i,field(fs,'event name').Data,field(p0,'location').Value))
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','armor_fx_diag.txt')),'w',encoding='utf-8').write("\n".join(out))
