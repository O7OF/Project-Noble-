# Repair variant mapping: region armor_fx_ext gets a default permutation first; marker groups are named after the
# effect locations (armor_fx_ext_t{t}_{loc}) and bound to permutation indices 1..29; pmcg entries point at ext_01..29.
import os, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
NEW=["default","silver","black","gold","rose_gold","emerald","ice","crimson","onyx","violet","amber","cyan","ruby",
     "sapphire","amethyst","bronze","sunfire","lime","platinum","obsidian","pearl","electric_blue","blood_orange","mint",
     "copper","midnight","magenta","aquamarine","titanium"]
out=[]; L=out.append
def pth(rel): return os.path.join(HT, rel)
def load(rel):
    base, ext=os.path.splitext(rel)
    tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
try:
    # --- model region + marker groups ---
    rel=r"objects\characters\spartans\spartans.render_model"
    tf=load(rel)
    regions=field(tf.Fields,'regions'); region_idx=None; region=None
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        if str(field(fs,'name').Data)=='armor_fx_ext': region_idx=i; region=regions.Elements[i]; break
    if region is None: raise RuntimeError("armor_fx_ext region missing")
    perms=field(region.Fields,'permutations')
    while perms.Elements.Count>0: perms.RemoveElement(perms.Elements.Count-1)
    el=perms.AddElement(); field(el.Fields,'name').SetStringData('armor_fx_ext_default')
    for i in range(len(NEW)):
        el=perms.AddElement(); field(el.Fields,'name').SetStringData('armor_fx_ext_%02d'%(i+1))
    L("model: region %d perms=%d (default + %d)"%(region_idx,perms.Elements.Count,len(NEW)))
    groups=field(tf.Fields,'marker groups')
    # remove existing ext marker groups then add correctly named ones
    for i in range(groups.Elements.Count-1,-1,-1):
        nm=str(field(groups.Elements[i].Fields,'name').Data)
        if nm=='armor_fx_ext_default' or (nm.startswith('armor_fx_ext_')):
            groups.RemoveElement(i)
    for i,vid in enumerate(NEW):
        t=i//6; loc=i%6
        el=groups.DuplicateElement(0); fs=el.Fields
        field(fs,'name').SetStringData('armor_fx_ext_t%d_%d'%(t,loc))
        m=field(fs,'markers').Elements[0].Fields
        field(m,'region index').Data=region_idx
        field(m,'permutation index').Data=i+1
    L("model: marker groups now=%d"%groups.Elements.Count)
    tf.Save(); tf.Dispose()
    # --- pmcg mapping ---
    rel=r"globals\player_model_customization.player_model_customization_globals"
    tf=load(rel)
    for listname in ('spartan armor effect selections','elite armor effect selections'):
        block=field(tf.Fields,listname)
        for i,vid in enumerate(NEW):
            nm='legendary_flame_%s'%vid
            found=False
            for j in range(block.Elements.Count):
                fs=block.Elements[j].Fields
                if str(field(fs,'selection name').Data)!=nm: continue
                bits=field(fs,'customized bits').Elements[0].Fields
                field(bits,'region name').SetStringData('armor_fx_ext')
                field(bits,'permutation name').SetStringData('armor_fx_ext_%02d'%(i+1))
                found=True
            if not found: L("WARN missing pmcg entry "+nm)
        L("pmcg %s: %d entries"%(listname,block.Elements.Count))
    tf.Save(); tf.Dispose()
    # --- verify marker names for first few ---
    tf=load(r"objects\characters\spartans\spartans.render_model"); groups=field(tf.Fields,'marker groups')
    names=[str(field(groups.Elements[i].Fields,'name').Data) for i in range(groups.Elements.Count)]
    L("marker sample: "+", ".join(n for n in names if n.startswith('armor_fx_ext'))[:400])
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','armor_fx_repair.txt')),'w',encoding='utf-8').write("\n".join(out))
