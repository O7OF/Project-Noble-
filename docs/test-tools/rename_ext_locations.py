# Rename ext effect tag locations to the real variant marker names.
import os,traceback
from mbstart import B, mb_stop
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
PRI=["steel","silver","white","brown","tan","khaki","sage","olive","drab","forest","green","sea_foam","teal","aqua","cyan","blue","cobalt","ice","violet","orchid","lavender","maroon","brick","rose","rust","coral","peach","gold","yellow","pale"]
out=[]
def L(s): out.append(str(s))
def load(rel):
    base,ext=os.path.splitext(rel); tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base,ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
try:
    for t in range(5):
        tf=load(r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t)
        loc=field(tf.Fields,'locations')
        for i in range(loc.Elements.Count):
            idx=t*6+i
            if idx<30:
                field(loc.Elements[i].Fields,'marker name').SetStringData('flame_pri_'+PRI[idx])
        tf.Save(); tf.Dispose(); L("ext%d renamed"%t)
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'rename_ext_locations.txt'),'w',encoding='utf-8').write("\n".join(out))
