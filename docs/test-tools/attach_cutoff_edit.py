# Empirical attachment cutoff: attach ext0/1/2 at slots 9/10/11, all with event0 at the stock marker armor_fx_00 and the
# stock head_fire.particle. Selecting stock flames then adds N copies of orange flames, revealing how many extras spawn.
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
    for t in range(3):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
        field(loc.Elements[0].Fields,'marker name').SetStringData('armor_fx_00')
        e0=ev.Elements[0].Fields; ps=field(e0,'particle systems').Elements[0].Fields
        setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle")
        field(ps,'location').Value=0
        tf.Save(); tf.Dispose(); L("ext%d event0 -> stock marker/particle"%t)
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel)
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments')
    L("attachments before=%d"%att.Elements.Count)
    tpl=5
    for t in range(3):
        el=att.DuplicateElement(tpl)
        setref(field(el.Fields,'type'), r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t)
    tf.Save(); tf.Dispose()
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields; att=field(obj,'attachments')
    L("attachments after=%d"%att.Elements.Count)
    for i in range(att.Elements.Count):
        ft=field(att.Elements[i].Fields,'type')
        try: L("  %d %s"%(i,ft.Path.RelativePathWithExtension))
        except Exception: pass
    tf.Dispose(); mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','attach_cutoff_edit.txt')),'w',encoding='utf-8').write("\n".join(out))
