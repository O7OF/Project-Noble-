# Merge 30 Foundry markers using SerializeElement/DeserializeElement (avoids clipboard state), add perms/region,
# rewrite pmcg entries 7..36, and add five biped effect attachments.
import os, shutil, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
PRI=["steel","silver","white","brown","tan","khaki","sage","olive","drab","forest","green","sea_foam","teal","aqua","cyan","blue","cobalt","ice","violet","orchid","lavender","maroon","brick","rose","rust","coral","peach","gold","yellow","pale"]
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
def copy_values(src,dst):
    for name in ('node index','translation','rotation','scale','direction'):
        sf=field(src,name); df=field(dst,name)
        if sf is not None and df is not None:
            try: df.Data = sf.Data
            except Exception:
                try: df.Data=list(sf.Data)
                except Exception: pass
try:
    biped=os.path.join(HT, r"objects\characters\spartans\spartans.biped")
    if os.path.exists(biped+'.bak'): shutil.copy2(biped+'.bak',biped); L("biped restored")
    # effect location markers patch
    for t in range(5):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        tf=load(rel); loc=field(tf.Fields,'locations')
        for i in range(6):
            idx=t*6+i
            if idx<len(PRI) and i<loc.Elements.Count:
                field(loc.Elements[i].Fields,'marker name').SetStringData('flame_pri_%s'%PRI[idx])
        tf.Save(); tf.Dispose()
    L("effect markers patched")
    # stock render model
    tf=load(r"objects\characters\spartans\spartans.render_model")
    regions=field(tf.Fields,'regions'); L("regions=%d"%regions.Elements.Count)
    region_armor_fx=None
    for i in range(regions.Elements.Count):
        if str(field(regions.Elements[i].Fields,'name').Data)=='armor_fx': region_armor_fx=regions.Elements[i]
    perms=field(region_armor_fx.Fields,'permutations')
    pnames=[str(field(perms.Elements[i].Fields,'name').Data) for i in range(perms.Elements.Count)]
    for i in range(26):
        nm='armor_fx_%02d'%(i+6)
        if nm not in pnames: el=perms.AddElement(); field(el.Fields,'name').SetStringData(nm)
    L("armor_fx perms=%d"%perms.Elements.Count)
    pri_idx=None
    for i in range(regions.Elements.Count):
        if str(field(regions.Elements[i].Fields,'name').Data)=='armor_fx_pri': pri_idx=i
    if pri_idx is None:
        el=regions.AddElement(); rfs=el.Fields
        field(rfs,'name').SetStringData('armor_fx_pri')
        pp=field(rfs,'permutations')
        for i in range(4): pe=pp.AddElement(); field(pe.Fields,'name').SetStringData('armor_fx_pri_%02d'%i)
        pri_idx=regions.Elements.Count-1
    L("armor_fx_pri idx=%r"%pri_idx)
    # collect serialized markers
    tfs=load(r"objects\characters\spartans\spartans_pri\spartans_pri.render_model")
    gs=field(tfs.Fields,'marker groups')
    idxs={}
    for i in range(gs.Elements.Count):
        nm=str(field(gs.Elements[i].Fields,'name').Data)
        if nm.startswith('flame_pri_'): idxs[nm]=i
    groups=field(tf.Fields,'marker groups')
    for i in range(groups.Elements.Count-1,-1,-1):
        if str(field(groups.Elements[i].Fields,'name').Data)=='fx_test_marker_b': groups.RemoveElement(i)
    ref=None
    for i in range(groups.Elements.Count):
        if str(field(groups.Elements[i].Fields,'name').Data)=='armor_fx_00': ref=groups.Elements[i]; break
    ref_marker=field(ref.Fields,'markers').Elements[0].Fields
    missing=[]
    for i,id in enumerate(PRI):
        nm='flame_pri_%s'%id
        if nm not in idxs: missing.append(nm); continue
        data=gs.SerializeElement(idxs[nm])
        newidx=groups.Elements.Count
        groups.InsertElement(newidx)
        groups.DeserializeElement(newidx,data)
        fs=groups.Elements[newidx].Fields
        got=str(field(fs,'name').Data)
        if got!=nm: L("WARN i=%d got=%r want=%r"%(i,got,nm))
        m=field(fs,'markers').Elements[0].Fields
        if i<26: field(m,'region index').Data=0; field(m,'permutation index').Data=6+i
        else: field(m,'region index').Data=pri_idx; field(m,'permutation index').Data=i-26
        copy_values(ref_marker,m)
    L("markers missing=%r groups=%d"%(missing,groups.Elements.Count))
    tf.Save(); tf.Dispose(); tfs.Dispose()
    # pmcg
    tf=load(r"globals\player_model_customization.player_model_customization_globals")
    for listname in ('spartan armor effect selections','elite armor effect selections'):
        block=field(tf.Fields,listname)
        while block.Elements.Count>7: block.RemoveElement(block.Elements.Count-1)
        for i,id in enumerate(PRI):
            el=block.DuplicateElement(1); fs=el.Fields
            field(fs,'selection name').SetStringData('legendary_flame_pri_%s'%id)
            bits=field(fs,'customized bits').Elements[0].Fields
            if i<26: field(bits,'region name').SetStringData('armor_fx'); field(bits,'permutation name').SetStringData('armor_fx_%02d'%(i+6))
            else: field(bits,'region name').SetStringData('armor_fx_pri'); field(bits,'permutation name').SetStringData('armor_fx_pri_%02d'%(i-26))
        L("pmcg %s=%d"%(listname,block.Elements.Count))
    tf.Save(); tf.Dispose()
    # biped attachments
    tf=load(r"objects\characters\spartans\spartans.biped")
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("biped att before=%d"%att.Elements.Count)
    for t in range(5):
        el=att.DuplicateElement(5); setref(field(el.Fields,'type'), r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t)
    L("biped att after=%d"%att.Elements.Count)
    tf.Save(); tf.Dispose()
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'pri_merge2.txt'),'w',encoding='utf-8').write("\n".join(out))
