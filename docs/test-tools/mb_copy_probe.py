# Probe ManagedBlam block clipboard/serialization APIs for cross-tag marker-group copy.
import os,traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
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
    tfs=load(r"objects\characters\spartans\spartans_rt5\spartans_rt5.render_model")
    groups=field(tfs.Fields,'marker groups')
    idx=None
    for i in range(groups.Elements.Count):
        if str(field(groups.Elements[i].Fields,'name').Data)=='fx_test_marker_b': idx=i; break
    L("rt5 idx=%r"%idx)
    for m in ('SerializeElement','CopyElement','SerializeEntireBlock','DeserializeElement','PasteAppendElement','InsertElement','DuplicateElement'):
        fn=getattr(groups,m)
        L(m+" doc: "+str(getattr(fn,'__doc__',None)))
    try:
        b=groups.SerializeElement(idx); L("SerializeElement type=%r len=%r"%(type(b), len(b) if b is not None else None))
    except Exception as e: L("SerializeElement err %s"%str(e)[:200])
    try:
        groups.CopyElement(idx); L("CopyElement ok")
    except Exception as e: L("CopyElement err %s"%str(e)[:200])
    tfs.Dispose()
    # open stock and try paste/serialize
    tf=load(r"objects\characters\spartans\spartans.render_model")
    g2=field(tf.Fields,'marker groups')
    L("stock groups=%d before"%g2.Elements.Count)
    try:
        g2.PasteAppendElement(); L("PasteAppendElement ok -> %d"%g2.Elements.Count)
        if g2.Elements.Count:
            L("last name=%r"%str(field(g2.Elements[g2.Elements.Count-1].Fields,'name').Data))
    except Exception as e: L("PasteAppendElement err %s"%str(e)[:200])
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'mb_copy_probe.txt'),'w',encoding='utf-8').write("\n".join(out))
