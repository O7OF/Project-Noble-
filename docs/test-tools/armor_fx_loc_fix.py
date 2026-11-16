# Location-block workaround: reuse unused location #4 (armor_fx_03_shoulders, no event uses it) by renaming
# its marker to armor_fx_06, and point the new legendary_flame_blue event at location 4.
import os, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
out=[]; L=out.append
def load(path):
    base, ext = os.path.splitext(path)
    tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
try:
    ep=r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"
    tf=load(ep)
    loc=field(tf.Fields,'locations')
    L("locations before: "+", ".join(str(field(loc.Elements[i].Fields,'marker name').Data) for i in range(loc.Elements.Count)))
    field(loc.Elements[4].Fields,'marker name').SetStringData("armor_fx_06")
    ev=field(tf.Fields,'events')
    e5=ev.Elements[5].Fields
    p0=field(e5,'particle systems').Elements[0].Fields
    field(p0,'location').Value=4
    tf.Save(); tf.Dispose()
    tf=load(ep)
    loc=field(tf.Fields,'locations')
    L("locations after: "+", ".join(str(field(loc.Elements[i].Fields,'marker name').Data) for i in range(loc.Elements.Count)))
    ev=field(tf.Fields,'events')
    for i in range(ev.Elements.Count):
        fs=ev.Elements[i].Fields
        ps=field(fs,'particle systems')
        p0=ps.Elements[0].Fields
        L(f"event {i} {str(field(fs,'event name').Data)} particle={str(field(p0,'particle').Path.RelativePathWithExtension)} loc={field(p0,'location').Value}")
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11", os.environ.get('OUT','armor_fx_loc_fix.txt')),'w',encoding='utf-8').write("\n".join(out))
