# Inspect model markers and effect event fields for armor_fx.
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
    # model markers
    tf=load(r"objects\characters\spartans\spartans.model"); L("#### spartans.model")
    for name in ('markers','attachments','regions'):
        f=field(tf.Fields,name)
        if not f: continue
        L(f"{name} [{f.Elements.Count}]")
        for i in range(f.Elements.Count):
            fs=f.Elements[i].Fields
            nm=sv(field(fs,'name')) if field(fs,'name') else ''
            if name=='markers' and 'armor_fx' not in nm: continue
            if name=='attachments': continue
            L(f"  #{i} name={nm}")
            if name=='markers': dumpfields(fs,'    ')
    tf.Dispose()
    # effect events full fields
    tf=load(r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"); L("\n#### armor_fx.effect events")
    f=field(tf.Fields,'events')
    for i in range(min(3,f.Elements.Count)):
        L(f"event #{i}")
        dumpfields(f.Elements[i].Fields,'  ')
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11", os.environ.get('OUT','armor_fx_inspect2.txt')),'w',encoding='utf-8').write("\n".join(out))
