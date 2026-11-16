# Diagnostic 4: point legacy blue particle's palette at fire_gold.bitmap (known-good particle path).
import os, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out=[]; L=out.append
state={"n":0}
def load(rel):
    base, ext=os.path.splitext(rel)
    tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def setref(f, rel):
    base, ext=os.path.splitext(rel); f.Path=B.Tags.TagPath.FromPathAndExtension(base, ext[1:])
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
            if rp.endswith(r"fx\particles\_gradients\plasma_blue_01.bitmap"):
                setref(f, r"fx\particles\fire\gradients\fire_gold.bitmap"); state["n"]+=1
try:
    tf=load(r"objects\characters\spartans\fx\armor_fx\particles\head_fire_blue.particle")
    walk(tf.Fields); tf.Save(); tf.Dispose(); L("palette refs changed=%d"%state["n"])
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','diag4.txt')),'w',encoding='utf-8').write("\n".join(out))
