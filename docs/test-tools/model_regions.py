# Inspect spartans.model region sort / variants / runtime regions to see whether a new render-model region must be mirrored.
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
def sv(f):
    try:
        t=str(f.FieldType)
        if t=='Reference': return 'ref:'+(str(f.Path.RelativePathWithExtension) if f.Path else '')
        if t in ('StringId','OldStringId','String','LongString'): return repr(str(f.Data))
        if hasattr(f,'Data'):
            d=f.Data
            try: return ",".join(f"{x:g}" if isinstance(x,float) else str(x) for x in d)
            except TypeError: return str(d)
        if hasattr(f,'Value'): return str(f.Value)
    except Exception: pass
    return ''
try:
    tf=load(r"objects\characters\spartans\spartans.model")
    for nm in ('region sort','variants','runtime regions'):
        f=field(tf.Fields,nm)
        if not f: L(nm+": absent"); continue
        L("\n%s [%d] max=%s"%(nm,f.Elements.Count,f.MaximumElementCount))
        for i in range(min(8,f.Elements.Count)):
            fs=f.Elements[i].Fields
            L("  #%d: %s"%(i, "; ".join("%s=%s"%(x.FieldName,sv(x)) for x in fs))[:600])
    tf.Dispose(); mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','model_regions.txt')),'w',encoding='utf-8').write("\n".join(out))
