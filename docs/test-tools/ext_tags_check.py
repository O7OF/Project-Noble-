# Inspect all five ext effect tags: location marker names and event particle/location.
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
    for t in range(5):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        tf=load(rel); loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
        L("#### ext%d locations=%d events=%d"%(t,loc.Elements.Count,ev.Elements.Count))
        L("  locations: "+", ".join(str(field(loc.Elements[i].Fields,'marker name').Data) for i in range(loc.Elements.Count)))
        for i in range(ev.Elements.Count):
            fs=ev.Elements[i].Fields
            ps=field(fs,'particle systems')
            if ps.Elements.Count==0: part='NONE'; lo='?'
            else:
                p0=ps.Elements[0].Fields
                part=str(field(p0,'particle').Path.RelativePathWithExtension) if field(p0,'particle').Path else ''
                lo=str(field(p0,'location').Value)
            L("  ev%d %s loc=%s -> %s"%(i,field(fs,'event name').Data,lo,part))
        tf.Dispose()
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'ext_tags_check.txt'),'w',encoding='utf-8').write("\n".join(out))
