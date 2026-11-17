# Distinct-particle cutoff test: ext0 -> blue flames, ext1 -> cupid hearts, ext2 -> head_fire, all at stock marker
# armor_fx_00. Selecting stock flames reveals exactly which extra attachment slots instantiate.
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
parts=[r"objects\characters\spartans\fx\armor_fx\particles\head_fire_blue.particle",
       r"objects\characters\spartans\fx\armor_fx\particles\cupid_heart.particle",
       r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle"]
try:
    for t in range(3):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
        field(loc.Elements[0].Fields,'marker name').SetStringData('armor_fx_00')
        e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
        setref(field(ps,'particle'), parts[t]); field(ps,'location').Value=0
        tf.Save(); tf.Dispose()
        tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events'); ps=field(ev.Elements[0].Fields,'particle systems').Elements[0].Fields
        L("ext%d loc0=%s particle=%s"%(t,field(loc.Elements[0].Fields,'marker name').Data,field(ps,'particle').Path.RelativePathWithExtension))
        tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','attach_cutoff_distinct.txt')),'w',encoding='utf-8').write("\n".join(out))
