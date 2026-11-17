# Test whether effect tag locations/events can be grown beyond 6 (duplicate last element twice, save, reload).
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
def dup_block(block):
    n=block.Elements.Count
    data=block.SerializeElement(n-1)
    block.AddElement()
    block.DeserializeElement(n,data)
try:
    tf=load(r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect")
    loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    L("before loc=%d ev=%d"%(loc.Elements.Count,ev.Elements.Count))
    for _ in range(2):
        dup_block(loc); dup_block(ev)
    L("after build loc=%d ev=%d"%(loc.Elements.Count,ev.Elements.Count))
    tf.Save(); tf.Dispose()
    tf=load(r"objects\characters\spartans\fx\armor_fx\armor_fx_ext0.effect")
    loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
    L("after reload loc=%d ev=%d"%(loc.Elements.Count,ev.Elements.Count))
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'grow_effect_test.txt'),'w',encoding='utf-8').write("\n".join(out))
