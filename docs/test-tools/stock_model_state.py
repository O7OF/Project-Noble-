# Inspect stock spartans.model after Foundry round-trip to see if it is still a valid model tag.
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
    for rel in (r"objects\characters\spartans\spartans.model",
                r"objects\characters\spartans\spartans.render_model"):
        tf=load(rel); L("#### "+rel)
        rm=field(tf.Fields,'render model')
        if rm: L("render model ref=%s"%(rm.Path.RelativePathWithExtension if rm.Path else None))
        v=field(tf.Fields,'variants')
        if v: L("variants=%d"%v.Elements.Count)
        regions=field(tf.Fields,'regions')
        if regions: L("regions=%d"%regions.Elements.Count)
        groups=field(tf.Fields,'marker groups')
        if groups: L("marker groups=%d"%groups.Elements.Count)
        tf.Dispose()
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'stock_model_state.txt'),'w',encoding='utf-8').write("\n".join(out))
