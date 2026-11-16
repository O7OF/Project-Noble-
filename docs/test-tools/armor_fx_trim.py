# Remove the stale 6th event from armor_fx_ext4 (only 5 variants in that tag).
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
    tf=load(rel); ev=field(tf.Fields,'events')
    L("before: %d events"%ev.Elements.Count)
    while ev.Elements.Count>5: ev.RemoveElement(ev.Elements.Count-1)
    tf.Save(); tf.Dispose()
    tf=load(rel); ev=field(tf.Fields,'events')
    L("after: %d events"%ev.Elements.Count)
    for i in range(ev.Elements.Count):
        fs=ev.Elements[i].Fields; ps=field(fs,'particle systems').Elements[0].Fields
        L("  %d %s loc=%s -> %s"%(i,field(fs,'event name').Data,field(ps,'location').Value,field(ps,'particle').Path.RelativePathWithExtension))
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','armor_fx_trim.txt')),'w',encoding='utf-8').write("\n".join(out))
