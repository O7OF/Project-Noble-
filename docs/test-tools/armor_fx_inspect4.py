# Inspect render_model marker groups and armor_fx permutation fields.
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
        if hasattr(f,'RawValue'): return "0x%X"%f.RawValue
        if hasattr(f,'Value'): return str(f.Value)
    except Exception as e: return '?'+str(e)[:30]
    return ''
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
def dumpfields(fs,pre='  '):
    for f in fs:
        t=str(f.FieldType)
        if t=='Block': L(pre+str(f.FieldName)+f" [{f.Elements.Count}]")
        elif t in ('Struct','Array'): L(pre+str(f.FieldName)+f" {{{t}}}")
        else: L(pre+str(f.FieldName)+" = "+sv(f))
try:
    tf=load(r"objects\characters\spartans\spartans.render_model"); L("#### render_model")
    mg=field(tf.Fields,'marker groups')
    L(f"marker groups [{mg.Elements.Count}]")
    shown=0
    for i in range(mg.Elements.Count):
        fs=mg.Elements[i].Fields
        txt=" ".join(sv(f) for f in fs)
        if 'armor_fx' in txt or i<3:
            L(f"group #{i}")
            dumpfields(fs,'  ')
            shown+=1
            if shown>=8: break
    regions=field(tf.Fields,'regions')
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        if sv(field(fs,'name'))!='armor_fx': continue
        perms=field(fs,'permutations')
        L(f"region armor_fx permutations [{perms.Elements.Count}]")
        for j in range(min(2,perms.Elements.Count)):
            L(f" permutation #{j}")
            dumpfields(perms.Elements[j].Fields,'  ')
    tf.Dispose()
    tf=load(r"objects\characters\spartans\spartans.model"); L("\n#### model top fields")
    L(", ".join(str(f.FieldName)+":"+str(f.FieldType) for f in tf.Fields))
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11", os.environ.get('OUT','armor_fx_inspect4.txt')),'w',encoding='utf-8').write("\n".join(out))
