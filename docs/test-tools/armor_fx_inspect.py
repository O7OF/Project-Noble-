# Focused inspect of armor_fx structures: render_model regions/perms/markers, effect locations/events, pmcg effect lists.
import os, traceback
from mbstart import B, mb_stop
HT = r"D:\Steam\steamapps\common\HREK\tags"
out=[]; L=out.append
def load(path):
    base, ext = os.path.splitext(path)
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def sval(f):
    try:
        ft=str(f.FieldType)
        if ft=='Reference': return str(f.Path.RelativePathWithExtension) if f.Path else ''
        if ft in ('StringId','OldStringId','String','LongString'): return str(f.Data)
        if hasattr(f,'Data'):
            d=f.Data
            try: return ",".join(f"{x:g}" if isinstance(x,float) else str(x) for x in d)
            except TypeError: return str(d)
        if hasattr(f,'RawValue'): return hex(f.RawValue)
        if hasattr(f,'Value'): return str(f.Value)
    except Exception as e: return '?'+str(e)[:40]
    return ''
def field(fields, name):
    for f in fields:
        if str(f.FieldName)==name: return f
    return None
def effects(tagpath):
    tf=load(tagpath); L("\n#### "+tagpath)
    f=field(tf.Fields,'locations')
    if f:
        L(f"locations [{f.Elements.Count}]")
        for i in range(f.Elements.Count):
            name=sval(field(f.Elements[i].Fields,'marker name'))
            L(f"  #{i} marker={name}")
    f=field(tf.Fields,'events')
    if f:
        L(f"events [{f.Elements.Count}]")
        for i in range(f.Elements.Count):
            fs=f.Elements[i].Fields
            L(f"  #{i} event={sval(field(fs,'event name'))} location={sval(field(fs,'location'))} rawloc={getattr(field(fs,'location'),'RawValue','?')} dur={sval(field(fs,'duration'))}")
    tf.Dispose()
def render_model(tagpath):
    tf=load(tagpath); L("\n#### "+tagpath)
    regions=field(tf.Fields,'regions')
    if not regions: L("no regions field"); tf.Dispose(); return
    L(f"regions [{regions.Elements.Count}]")
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        nm=sval(field(fs,'name'))
        if nm!='armor_fx': continue
        perms=field(fs,'permutations')
        L(f"region armor_fx at #{i}, permutations [{perms.Elements.Count if perms else 0}]")
        if perms:
            for j in range(perms.Elements.Count):
                pfs=perms.Elements[j].Fields
                pn=sval(field(pfs,'name'))
                mk=field(pfs,'markers')
                mnames=[]
                if mk:
                    for k in range(mk.Elements.Count):
                        mnames.append(sval(field(mk.Elements[k].Fields,'name')))
                L(f"  #{j} permutation={pn} markers={mnames}")
    tf.Dispose()
def pmcg(tagpath):
    tf=load(tagpath); L("\n#### "+tagpath)
    for listname in ('spartan armor effect selections','elite armor effect selections'):
        f=field(tf.Fields,listname)
        if not f: L(listname+" missing"); continue
        L(f"{listname} [{f.Elements.Count}]")
        for i in range(f.Elements.Count):
            fs=f.Elements[i].Fields
            nm=sval(field(fs,'selection name'))
            bits=field(fs,'customized bits')
            ch=[]
            if bits:
                for k in range(bits.Elements.Count):
                    cfs=bits.Elements[k].Fields
                    ch.append((sval(field(cfs,'region name')),sval(field(cfs,'permutation name'))))
            L(f"  #{i} {nm} -> {ch}")
    tf.Dispose()
try:
    effects(r"objects\characters\spartans\fx\armor_fx\armor_fx.effect")
    render_model(r"objects\characters\spartans\spartans.render_model")
    pmcg(r"globals\player_model_customization.player_model_customization_globals")
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11", os.environ.get('OUT','armor_fx_inspect.txt')),'w',encoding='utf-8').write("\n".join(out))
