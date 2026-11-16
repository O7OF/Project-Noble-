# Inspect a render_model marker group/marker's fields.
import os, traceback
from mbstart import B, mb_stop
out=[]; L=out.append
def load(path):
    base, ext = os.path.splitext(path)
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def sv(f):
    try:
        ft=str(f.FieldType)
        if ft=='Reference': return 'ref:'+(str(f.Path.RelativePathWithExtension) if f.Path else '')
        if ft in ('StringId','OldStringId','String','LongString'): return repr(str(f.Data))
        if hasattr(f,'Data'):
            d=f.Data
            try: return ",".join(f"{x:g}" if isinstance(x,float) else str(x) for x in d)
            except TypeError: return str(d)
        if hasattr(f,'Value'): return str(f.Value)
    except Exception as e: return '?'+str(e)[:30]
    return ''
try:
    tf=load(r"objects\characters\spartans\spartans.render_model")
    mg=[f for f in tf.Fields if str(f.FieldName)=='marker groups'][0]
    for i in range(3):
        g=mg.Elements[i]
        L(f"group #{i}")
        for f in g.Fields:
            L("  field "+str(f.FieldName)+" type="+str(f.FieldType)+" = "+sv(f))
        markers=[f for f in g.Fields if str(f.FieldName)=='markers'][0]
        for j in range(markers.Elements.Count):
            L(f"  marker #{j}")
            for f in markers.Elements[j].Fields:
                L("    "+str(f.FieldName)+" type="+str(f.FieldType)+" = "+sv(f))
    tf.Dispose()
    tf=load(r"objects\characters\spartans\spartans.render_model")
    regions=[f for f in tf.Fields if str(f.FieldName)=='regions'][0]
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        if str([x for x in fs if str(x.FieldName)=='name'][0].Data)!='armor_fx': continue
        perms=[f for f in fs if str(f.FieldName)=='permutations'][0]
        for j in range(min(2,perms.Elements.Count)):
            L(f"perm #{j}")
            for f in perms.Elements[j].Fields:
                L("  "+str(f.FieldName)+" type="+str(f.FieldType)+" = "+sv(f))
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11", os.environ.get('OUT','armor_fx_inspect5.txt')),'w',encoding='utf-8').write("\n".join(out))
