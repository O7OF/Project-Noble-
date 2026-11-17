# Regenerate only the five primary effect tags (stock armor_fx.effect has 5 events; clone from legendary_flame).
import os, shutil, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
PRI=["steel","silver","white","brown","tan","khaki","sage","olive","drab","forest","green","sea_foam","teal","aqua","cyan","blue","cobalt","ice","violet","orchid","lavender","maroon","brick","rose","rust","coral","peach","gold","yellow","pale"]
EFFECT_SRC=r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"
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
    for t in range(5):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        shutil.copy2(os.path.join(HT,EFFECT_SRC), os.path.join(HT,rel))
        tf=load(rel); ev=field(tf.Fields,'events'); loc=field(tf.Fields,'locations')
        orig=ev.Elements.Count; chunk=PRI[t*6:(t+1)*6]; n=len(chunk)
        for _ in range(n): ev.DuplicateElement(0)
        for _ in range(orig): ev.RemoveElement(0)
        L("ext%d orig=%d after clone=%d want=%d"%(t,orig,ev.Elements.Count,n))
        for i,id in enumerate(chunk):
            fs=ev.Elements[i].Fields
            field(fs,'event name').SetStringData("legendary_flame_pri_%s"%id)
            ps=field(fs,'particle systems').Elements[0].Fields
            setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_pri_%s.particle"%id)
            field(ps,'location').Value=i
            if i<loc.Elements.Count: field(loc.Elements[i].Fields,'marker name').SetStringData("flame_pri_t%d_%d"%(t,i))
        tf.Save(); tf.Dispose()
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'pri_effects.txt'),'w',encoding='utf-8').write("\n".join(out))
