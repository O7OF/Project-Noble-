# Diagnostic 5: point the known-good legacy blue EVENT at the custom head_fire_gold.particle. If gold flames render when
# item 240 is selected, custom particle tags are valid and the failure is in the ext effect tag/attachment copy path.
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
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"
    tf=load(rel); ev=field(tf.Fields,'events')
    target=None
    for i in range(ev.Elements.Count):
        fs=ev.Elements[i].Fields
        nm=str(field(fs,'event name').Data) if field(fs,'event name') else ''
        if nm=='legendary_flame_blue': target=fs; break
    if target is None: raise RuntimeError("legendary_flame_blue event missing")
    ps=field(target,'particle systems'); p0=ps.Elements[0].Fields
    setref(field(p0,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_gold.particle")
    tf.Save(); tf.Dispose()
    tf=load(rel); ev=field(tf.Fields,'events')
    for i in range(ev.Elements.Count):
        fs=ev.Elements[i].Fields
        nm=str(field(fs,'event name').Data) if field(fs,'event name') else ''
        if nm=='legendary_flame_blue':
            ps=field(fs,'particle systems').Elements[0].Fields
            L("event %d %s loc=%s -> %s"%(i,nm,field(ps,'location').Value,field(ps,'particle').Path.RelativePathWithExtension))
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','diag5.txt')),'w',encoding='utf-8').write("\n".join(out))
