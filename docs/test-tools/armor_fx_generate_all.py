# Generate flame colour variants for 29 visor colours (blue stays as the existing legendary_flame_blue legacy variant).
# Strategy: 5 new effect tags (6 locations each) attached to spartans.biped, a new render_model region armor_fx_ext
# with 29 permutations + 29 marker groups, and 29 pmcg entries in both effect lists. Palettes are recoloured copies of
# fire.bitmap; particles are copies of head_fire.particle pointing at them.
import os, shutil, traceback, hashlib
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out=[]; L=out.append
VISORS=[
 ("default",(0.85,0.53,0.14)),("silver",(0.78,0.83,0.90)),("blue",(0.18,0.54,0.95)),("black",(0.11,0.13,0.16)),
 ("gold",(0.98,0.78,0.18)),("rose_gold",(0.93,0.58,0.55)),("emerald",(0.10,0.65,0.38)),("ice",(0.55,0.25,0.85)),
 ("crimson",(0.55,0.85,1.00)),("onyx",(0.85,0.12,0.18)),("violet",(0.05,0.05,0.06)),("amber",(0.95,0.48,0.08)),
 ("cyan",(0.08,0.85,0.92)),("ruby",(0.92,0.12,0.28)),("sapphire",(0.10,0.25,0.92)),("amethyst",(0.70,0.15,0.85)),
 ("bronze",(0.80,0.45,0.20)),("sunfire",(1.00,0.35,0.02)),("lime",(0.42,0.92,0.18)),("platinum",(0.92,0.94,0.98)),
 ("obsidian",(0.08,0.08,0.10)),("pearl",(0.95,0.92,0.88)),("electric_blue",(0.05,0.80,1.00)),("blood_orange",(0.95,0.22,0.05)),
 ("mint",(0.35,0.88,0.65)),("copper",(0.85,0.45,0.25)),("midnight",(0.05,0.08,0.25)),("magenta",(0.88,0.10,0.60)),
 ("aquamarine",(0.15,0.85,0.75)),("titanium",(0.70,0.72,0.75)),
]
NEW=[v for v in VISORS if v[0]!="blue"]   # blue uses the existing legendary_flame_blue variant
TAGS=5; PER=6
def pth(rel): return os.path.join(HT, rel)
def backup(rel):
    src=pth(rel); dst=src+".bak"
    if not os.path.exists(dst): shutil.copy2(src,dst); L("backup "+os.path.basename(rel))
def load(rel):
    base, ext=os.path.splitext(rel)
    tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
def data_field(tf):
    best=None
    def walk(fs):
        nonlocal best
        for f in fs:
            t=str(f.FieldType)
            if t=='Block':
                for el in f.Elements: walk(el.Fields)
            elif t in ('Struct','Array'):
                try:
                    for el in (f.Elements if t=='Array' else [f.Elements[0]]): walk(el.Fields)
                except Exception: pass
            elif t=='Data':
                try:
                    if f.ByteCount>1000 and (best is None or f.ByteCount>best.ByteCount): best=f
                except Exception: pass
    walk(tf.Fields); return best
def setref(f, rel):
    base, ext=os.path.splitext(rel); f.Path=B.Tags.TagPath.FromPathAndExtension(base, ext[1:])
def copy_tag(src_rel, dst_rel):
    s=pth(src_rel); d=pth(dst_rel)
    if not os.path.exists(d): shutil.copy2(s,d); L("copy "+os.path.basename(d))

PAL_SRC=r"fx\particles\fire\gradients\fire.bitmap"
PART_SRC=r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle"
EFFECT_SRC=r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"
MODEL_REL=r"objects\characters\spartans\spartans.render_model"
PMCG_REL=r"globals\player_model_customization.player_model_customization_globals"
BIPED_REL=r"objects\characters\spartans\spartans.biped"

def recolor(rgb, src_rel, dst_rel):
    copy_tag(src_rel, dst_rel)
    tf=load(dst_rel); f=data_field(tf); d=bytearray(f.GetData())
    tr,tg,tb=rgb
    for i in range(0,len(d)-3,4):
        Bv,Gv,Rv,A=d[i],d[i+1],d[i+2],d[i+3]
        m=max(Bv,Gv,Rv)/255.0
        hot=max(0.0,(m-0.75)/0.25)
        out_r=min(1.0, tr*(m**0.85)*(1-hot)+hot)
        out_g=min(1.0, tg*(m**0.85)*(1-hot)+hot)
        out_b=min(1.0, tb*(m**0.85)*(1-hot)+hot)
        d[i]=int(out_b*255); d[i+1]=int(out_g*255); d[i+2]=int(out_r*255); d[i+3]=A
    f.SetData(d); tf.Save(); tf.Dispose()

def make_particle(id, rel):
    copy_tag(PART_SRC, rel)
    tf=load(rel)
    changed=0
    def walk(fs):
        nonlocal changed
        for f in fs:
            t=str(f.FieldType)
            if t=='Block':
                for el in f.Elements: walk(el.Fields)
            elif t in ('Struct','Array'):
                try:
                    for el in (f.Elements if t=='Array' else [f.Elements[0]]): walk(el.Fields)
                except Exception: pass
            elif t=='Reference':
                try:
                    rp=str(f.Path.RelativePathWithExtension) if f.Path else ""
                except Exception: rp=""
                if rp.endswith(r"fx\particles\fire\gradients\fire.bitmap"):
                    setref(f, r"fx\particles\fire\gradients\fire_%s.bitmap"%id); changed+=1
    walk(tf.Fields); tf.Save(); tf.Dispose(); return changed

def build():
    # palettes + particles
    for id,rgb in NEW:
        recolor(rgb, PAL_SRC, r"fx\particles\fire\gradients\fire_%s.bitmap"%id)
        n=make_particle(id, r"objects\characters\spartans\fx\armor_fx\particles\head_fire_%s.particle"%id)
        L("particle %s: %d palette refs redirected"%(id,n))
    # effect tags (5 x up to 6)
    effect_tags=[]
    for t in range(TAGS):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        copy_tag(EFFECT_SRC, rel); effect_tags.append(rel)
        tf=load(rel)
        ev=field(tf.Fields,'events')
        while ev.Elements.Count>0: ev.RemoveElement(ev.Elements.Count-1)
        loc=field(tf.Fields,'locations')
        chunk=NEW[t*PER:(t+1)*PER]
        for i,(id,rgb) in enumerate(chunk):
            if i>=loc.Elements.Count: break
            field(loc.Elements[i].Fields,'marker name').SetStringData("armor_fx_ext_t%d_%d"%(t,i))
            el=ev.AddElement(); fs=el.Fields
            field(fs,'event name').SetStringData("legendary_flame_%s"%id)
            ps=field(fs,'particle systems')
            if ps.Elements.Count==0:
                # copy a particle system from a stock event before it was removed? we removed all -> duplicate from a template tag instead
                pass
            if ps.Elements.Count:
                p0=ps.Elements[0].Fields
                setref(field(p0,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_%s.particle"%id)
                field(p0,'location').Value=i
            else:
                L("WARN: empty particle systems for "+id)
        tf.Save(); tf.Dispose()
        L("effect tag %s: %d events, %d locations"%(rel,len(chunk),loc.Elements.Count))
    # model region + perms + marker groups
    backup(MODEL_REL); tf=load(MODEL_REL)
    regions=field(tf.Fields,'regions')
    region_idx=None
    for i in range(regions.Elements.Count):
        if str(field(regions.Elements[i].Fields,'name').Data)=='armor_fx_ext': region_idx=i; break
    if region_idx is None:
        region_idx=regions.Elements.Count
        rel_el=regions.DuplicateElement(0); fs=rel_el.Fields
        field(fs,'name').SetStringData('armor_fx_ext')
        perms=field(fs,'permutations')
        while perms.Elements.Count>0: perms.RemoveElement(perms.Elements.Count-1)
        for i in range(len(NEW)):
            el=perms.AddElement(); field(el.Fields,'name').SetStringData('armor_fx_ext_%02d'%i)
        L("model: added region armor_fx_ext at %d with %d perms"%(region_idx,len(NEW)))
    groups=field(tf.Fields,'marker groups')
    gnames=[str(field(groups.Elements[i].Fields,'name').Data) for i in range(groups.Elements.Count)]
    added=0
    for i,(id,rgb) in enumerate(NEW):
        gname='armor_fx_ext_%02d'%i
        if gname in gnames: continue
        el=groups.DuplicateElement(0); fs=el.Fields
        field(fs,'name').SetStringData(gname)
        markers=field(fs,'markers'); m=markers.Elements[0].Fields
        field(m,'region index').Data=region_idx
        field(m,'permutation index').Data=i
        added+=1
    tf.Save(); tf.Dispose(); L("model: added %d marker groups"%added)
    # pmcg entries
    backup(PMCG_REL); tf=load(PMCG_REL)
    for listname in ('spartan armor effect selections','elite armor effect selections'):
        block=field(tf.Fields,listname)
        names=[str(field(block.Elements[i].Fields,'selection name').Data) for i in range(block.Elements.Count)]
        for i,(id,rgb) in enumerate(NEW):
            nm='legendary_flame_%s'%id
            if nm in names: continue
            # find blue entry template (index 7) if present
            tpl=7 if block.Elements.Count>7 and str(field(block.Elements[7].Fields,'selection name').Data)=='legendary_flame_blue' else 1
            el=block.DuplicateElement(tpl); fs=el.Fields
            field(fs,'selection name').SetStringData(nm)
            bits=field(fs,'customized bits'); b0=bits.Elements[0].Fields
            field(b0,'region name').SetStringData('armor_fx_ext')
            field(b0,'permutation name').SetStringData('armor_fx_ext_%02d'%i)
        L("pmcg %s: %d entries"%(listname,block.Elements.Count))
    tf.Save(); tf.Dispose()
    # biped attachments
    backup(BIPED_REL); tf=load(BIPED_REL)
    unit=field(tf.Fields,'unit').Elements[0].Fields
    obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments')
    # find armor_fx attachment index
    tpl=None
    for i in range(att.Elements.Count):
        fs=att.Elements[i].Fields
        try:
            rp=str(field(fs,'type').Path.RelativePathWithExtension)
        except Exception: rp=''
        if rp.endswith(r"armor_fx\armor_fx.effect"): tpl=i; break
    if tpl is None: tpl=5
    added=0
    for t,rel in enumerate(effect_tags):
        el=att.DuplicateElement(tpl); fs=el.Fields
        setref(field(fs,'type'), rel); added+=1
    tf.Save(); tf.Dispose(); L("biped: added %d effect attachments (template %d, total %d)"%(added,tpl,att.Elements.Count))
    mb_stop()

try:
    build()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','armor_fx_generate_all.txt')),'w',encoding='utf-8').write("\n".join(out))
