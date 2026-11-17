# Inspect merged marker groups: names, region/perm indices and transform consistency.
import os,traceback
from mbstart import B, mb_stop
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out=[]
def L(s): out.append(str(s))
def load(rel):
    base,ext=os.path.splitext(rel); tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base,ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
try:
    tf=load(r"objects\characters\spartans\spartans.render_model")
    groups=field(tf.Fields,'marker groups')
    L("groups=%d"%groups.Elements.Count)
    for i in range(groups.Elements.Count):
        fs=groups.Elements[i].Fields; nm=str(field(fs,'name').Data)
        if nm.startswith('flame_pri_'):
            m=field(fs,'markers')
            if m.Elements.Count==0: L("%s markers=0"%nm); continue
            mf=m.Elements[0].Fields
            L("%s region=%s perm=%s node=%s tx=%s"%(nm,field(mf,'region index').Data,field(mf,'permutation index').Data,field(mf,'node index').Data,field(mf,'translation').Data))
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'check_markers_centering.txt'),'w',encoding='utf-8').write("\n".join(out))
