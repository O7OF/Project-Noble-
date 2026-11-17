# Biped: 4 stock attachments + all 5 ext tags = 9 total. Drops the 3p flashlight too.
import os, shutil, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
DROP=['night_vision.light','bungie_helmet.cheap_light','flashlight_1p.light','shield_down.light','flashlight_3p.light']
out=[]
def L(s): out.append(str(s))
def load(rel):
    base,ext=os.path.splitext(rel); tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base,ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
def setref(f, rel):
    base,ext=os.path.splitext(rel); f.Path=B.Tags.TagPath.FromPathAndExtension(base,ext[1:])
try:
    biped=os.path.join(HT, r"objects\characters\spartans\spartans.biped")
    shutil.copy2(biped+'.bak', biped); L("restored stock biped")
    tf=load(r"objects\characters\spartans\spartans.biped")
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments')
    for i in range(att.Elements.Count-1,-1,-1):
        rp=str(field(att.Elements[i].Fields,'type').Path.RelativePathWithExtension)
        if any(rp.endswith(d) for d in DROP): att.RemoveElement(i); L("drop "+rp)
    eff_idx=None
    for i in range(att.Elements.Count):
        rp=str(field(att.Elements[i].Fields,'type').Path.RelativePathWithExtension)
        if rp.endswith(r"armor_fx\armor_fx.effect"): eff_idx=i; break
    for t in range(5):
        el=att.DuplicateElement(eff_idx)
        setref(field(el.Fields,'type'), r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t)
    L("total=%d"%att.Elements.Count)
    for i in range(att.Elements.Count):
        L("  %d %s"%(i,str(field(att.Elements[i].Fields,'type').Path.RelativePathWithExtension)))
    tf.Save(); tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'build_9att_5ext.txt'),'w',encoding='utf-8').write("\n".join(out))
