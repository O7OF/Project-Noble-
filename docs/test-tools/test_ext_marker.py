# Test whether the new render-model region ext + its marker groups activate: ext0 event0 -> stock particle at
# location marker armor_fx_ext_t0_0, then select variant id 241 (pmcg entry 8 -> armor_fx_ext_01, marker ext_t0_0).
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
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect"
    tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    field(loc.Elements[0].Fields,'marker name').SetStringData('armor_fx_ext_t0_0')
    e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
    setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle")
    field(ps,'location').Value=0
    tf.Save(); tf.Dispose()
    tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events'); ps=field(ev.Elements[0].Fields,'particle systems').Elements[0].Fields
    L("ext0 loc0=%s event0 loc=%s particle=%s"%(field(loc.Elements[0].Fields,'marker name').Data,field(ps,'location').Value,field(ps,'particle').Path.RelativePathWithExtension))
    tf.Dispose(); mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','test_ext_marker.txt')),'w',encoding='utf-8').write("\n".join(out))
