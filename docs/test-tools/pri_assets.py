# Generate 30 primary-palette flame assets and five 6-event effect tags (all events duplicated from legendary_flame so
# they keep correct third-person particle-system properties).
import os, shutil, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
PRI=[
 ("steel",0.349,0.349,0.349),("silver",0.608,0.608,0.608),("white",0.847,0.847,0.847),
 ("brown",0.427,0.310,0.220),("tan",0.620,0.494,0.376),("khaki",0.780,0.671,0.541),
 ("sage",0.345,0.408,0.302),("olive",0.490,0.569,0.400),("drab",0.643,0.725,0.533),
 ("forest",0.165,0.369,0.212),("green",0.267,0.545,0.314),("sea_foam",0.467,0.729,0.506),
 ("teal",0.165,0.431,0.459),("aqua",0.278,0.608,0.635),("cyan",0.471,0.788,0.808),
 ("blue",0.176,0.302,0.514),("cobalt",0.290,0.455,0.694),("ice",0.518,0.678,0.875),
 ("violet",0.310,0.227,0.498),("orchid",0.459,0.365,0.678),("lavender",0.651,0.565,0.855),
 ("maroon",0.467,0.161,0.200),("brick",0.678,0.251,0.278),("rose",0.863,0.478,0.494),
 ("rust",0.557,0.231,0.133),("coral",0.784,0.376,0.239),("peach",0.941,0.588,0.451),
 ("gold",0.639,0.439,0.141),("yellow",0.859,0.651,0.220),("pale",0.961,0.816,0.447),
]
PAL_SRC=r"fx\particles\fire\gradients\fire.bitmap"
PART_SRC=r"objects\characters\spartans\fx\armor_fx\particles\head_fire.particle"
EFFECT_SRC=r"objects\characters\spartans\fx\armor_fx\armor_fx.effect"
out=[]
def L(s): out.append(str(s))
def load(rel):
    base,ext=os.path.splitext(rel); tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base,ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
def setref(f, rel):
    base,ext=os.path.splitext(rel); f.Path=B.Tags.TagPath.FromPathAndExtension(base,ext[1:])
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
try:
    # palettes + particles
    for id,r,g,b in PRI:
        pal=r"fx\particles\fire\gradients\fire_pri_%s.bitmap"%id
        dst=os.path.join(HT,pal)
        shutil.copy2(os.path.join(HT,PAL_SRC), dst)
        tf=load(pal); f=data_field(tf); d=bytearray(f.GetData())
        for i in range(0,len(d)-3,4):
            Bv,Gv,Rv,A=d[i],d[i+1],d[i+2],d[i+3]
            m=max(Bv,Gv,Rv)/255.0; hot=max(0.0,(m-0.75)/0.25)
            oR=min(1.0, r*(m**0.85)*(1-hot)+hot); oG=min(1.0, g*(m**0.85)*(1-hot)+hot); oB=min(1.0, b*(m**0.85)*(1-hot)+hot)
            d[i]=int(oB*255); d[i+1]=int(oG*255); d[i+2]=int(oR*255); d[i+3]=A
        f.SetData(d); tf.Save(); tf.Dispose()
        # particle copy with palette refs replaced
        part=r"objects\characters\spartans\fx\armor_fx\particles\head_fire_pri_%s.particle"%id
        shutil.copy2(os.path.join(HT,PART_SRC), os.path.join(HT,part))
        tf=load(part); n=[0]
        def walk(fs):
            for f in fs:
                t=str(f.FieldType)
                if t=='Block':
                    for el in f.Elements: walk(el.Fields)
                elif t in ('Struct','Array'):
                    try:
                        for el in (f.Elements if t=='Array' else [f.Elements[0]]): walk(el.Fields)
                    except Exception: pass
                elif t=='Reference':
                    try: rp=str(f.Path.RelativePathWithExtension)
                    except Exception: rp=''
                    if rp.endswith(r"fx\particles\fire\gradients\fire.bitmap"):
                        setref(f, pal); n[0]+=1
        walk(tf.Fields); tf.Save(); tf.Dispose()
        L("assets %s palette refs=%d"%(id,n[0]))
    # five effect tags, all events cloned from legendary_flame
    for t in range(5):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        shutil.copy2(os.path.join(HT,EFFECT_SRC), os.path.join(HT,rel))
        tf=load(rel); ev=field(tf.Fields,'events'); loc=field(tf.Fields,'locations')
        chunk=PRI[t*6:(t+1)*6]; n=len(chunk)
        for _ in range(n): ev.DuplicateElement(0)
        for _ in range(6): ev.RemoveElement(0)
        for i,(id,r,g,b) in enumerate(chunk):
            fs=ev.Elements[i].Fields
            field(fs,'event name').SetStringData("legendary_flame_pri_%s"%id)
            ps=field(fs,'particle systems').Elements[0].Fields
            setref(field(ps,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_pri_%s.particle"%id)
            field(ps,'location').Value=i
            if i<loc.Elements.Count: field(loc.Elements[i].Fields,'marker name').SetStringData("flame_pri_t%d_%d"%(t,i))
        tf.Save(); tf.Dispose()
        L("effect ext%d events=%d"%(t,n))
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'pri_assets.txt'),'w',encoding='utf-8').write("\n".join(out))
