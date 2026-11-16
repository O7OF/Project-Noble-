# Inspect render_model/model top-level fields, markers, and effect particle-system fields.
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
def dumpfields(fs,pre='  ',maxdepth=3):
    for f in fs:
        t=str(f.FieldType)
        if t=='Block': L(pre+str(f.FieldName)+f" [{f.Elements.Count}]")
        elif t in ('Struct','Array'): L(pre+str(f.FieldName)+f" {{{t}}}")
        else: L(pre+str(f.FieldName)+" = "+sv(f))
try:
    for tp,label in ((r"objects\characters\spartans\spartans.render_model","render_model"),
                     (r"objects\characters\spartans\spartans.model","model")):
        tf=load(tp); L("\n#### "+label)
        L("top fields: " + ", ".join(str(f.FieldName)+":"+str(f.FieldType) for f in tf.Fields)[:1500])
        for name in ('markers','regions','attachments'):
            f=field(tf.Fields,name)
            if not f: L(name+": absent"); continue
            L(f"{name} [{f.Elements.Count}]")
            shown=0
            for i in range(f.Elements.Count):
                fs=f.Elements[i].Fields
                nm=sv(field(fs,'name')) if field(fs,'name') else ''
                if name=='markers' and 'armor_fx' not in nm: continue
                L(f"  #{i} name={nm}")
                dumpfields(fs,'    ')
                shown+=1
                if shown>=8: break
        tf.Dispose()
    tf=load(r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"); L("\n#### effect event particle system fields")
    events=field(tf.Fields,'events')
    ps=field(events.Elements[0].Fields,'particle systems')
    L("particle system #0 fields:")
    dumpfields(ps.Elements[0].Fields,'  ')
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11", os.environ.get('OUT','armor_fx_inspect3.txt')),'w',encoding='utf-8').write("\n".join(out))
