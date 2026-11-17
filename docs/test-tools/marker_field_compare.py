# Compare stock armor_fx_00 marker element with a merged flame marker element (all fields).
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
def sv(f):
    try:
        t=str(f.FieldType)
        if hasattr(f,'Data'):
            d=f.Data
            try: return ",".join(f"{x:g}" if isinstance(x,float) else str(x) for x in d)
            except TypeError: return str(d)
        if hasattr(f,'Value'): return str(f.Value)
        if hasattr(f,'RawValue'): return hex(f.RawValue)
    except Exception: pass
    return ''
try:
    tf=load(r"objects\characters\spartans\spartans.render_model")
    groups=field(tf.Fields,'marker groups')
    for want in ('armor_fx_00','flame_pri_steel','flame_pri_brick'):
        for i in range(groups.Elements.Count):
            fs=groups.Elements[i].Fields
            if str(field(fs,'name').Data)==want:
                L("#### "+want)
                m=field(fs,'markers')
                L("markers=%d"%m.Elements.Count)
                for j in range(m.Elements.Count):
                    mf=m.Elements[j].Fields
                    L(" marker %d fields:"%j)
                    for f in mf: L("   %s (%s) = %s"%(f.FieldName,f.FieldType,sv(f)))
                break
    tf.Dispose(); mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'marker_field_compare.txt'),'w',encoding='utf-8').write("\n".join(out))
