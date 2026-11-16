# Restore the working blue variant after diagnostics: blue particle palette -> plasma_blue_01, legacy event -> blue
# particle, and remove the diagnostic duplicate stock armor_fx attachment.
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
    # 1) blue particle palette back to plasma blue (2 refs to fire_gold)
    rel=r"objects\characters\spartans\fx\armor_fx\particles\head_fire_blue.particle"
    tf=load(rel)
    n=[0]
    def walk(fs):
        for f in fs:
            t=str(f.FieldType)
            if t=='Block':
                for el in f.Elements: walk(el.Fields)
            elif t in ('Struct','Array'):
                try:
                    for el in (f.Elements if t=='Array' else [f.Elements[0]]): walk(el.Fields)
                except Exception: pass
            elif t=='Reference':
                try: rp=str(f.Path.RelativePathWithExtension)
                except Exception: rp=''
                if rp.endswith(r"fx\particles\fire\gradients\fire_gold.bitmap"):
                    setref(f, r"fx\particles\_gradients\plasma_blue_01.bitmap"); n[0]+=1
    walk(tf.Fields); tf.Save(); tf.Dispose(); L("blue particle palette refs restored=%d"%n[0])
    # 2) legacy event back to blue particle
    rel=r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"
    tf=load(rel); ev=field(tf.Fields,'events')
    for i in range(ev.Elements.Count):
        fs=ev.Elements[i].Fields
        nm=str(field(fs,'event name').Data) if field(fs,'event name') else ''
        if nm=='legendary_flame_blue':
            ps=field(fs,'particle systems').Elements[0].Fields
            setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_blue.particle")
            L("legacy blue event particle restored")
    tf.Save(); tf.Dispose()
    # 3) remove duplicate stock armor_fx attachment (last one)
    rel=r"objects\characters\spartans\spartans.biped"
    tf=load(rel)
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments')
    L("attachments before=%d"%att.Elements.Count)
    while att.Elements.Count>14: att.RemoveElement(att.Elements.Count-1)
    tf.Save(); tf.Dispose()
    tf=load(rel); unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields; att=field(obj,'attachments')
    L("attachments after=%d"%att.Elements.Count)
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','restore_blue.txt')),'w',encoding='utf-8').write("\n".join(out))
