# Fix double-armor-effect: make armor_fx permutation 0 a no-marker default, shift the six stock effect perms to
# indices 1..6, keep variants 0..24 on armor_fx and move variants 25..29 to armor_fx_pri perms 0..4.
import os,traceback
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
try:
    tf=load(r"objects\characters\spartans\spartans.render_model")
    regions=field(tf.Fields,'regions'); region=None; pri=None
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        nm=str(field(fs,'name').Data)
        if nm=='armor_fx': region=regions.Elements[i]
        if nm=='armor_fx_pri': pri=regions.Elements[i]
    perms=field(region.Fields,'permutations')
    L("armor_fx perms before=%d"%perms.Elements.Count)
    if perms.Elements.Count<32:
        while perms.Elements.Count<32: perms.AddElement()
    for i in range(32):
        nm='armor_fx_default' if i==0 else 'armor_fx_%02d'%(i-1)
        field(perms.Elements[i].Fields,'name').SetStringData(nm)
    L("armor_fx perms renamed")
    pperms=field(pri.Fields,'permutations')
    while pperms.Elements.Count<5: pperms.AddElement()
    for i in range(5):
        field(pperms.Elements[i].Fields,'name').SetStringData('armor_fx_pri_%02d'%i)
    L("armor_fx_pri perms=%d"%pperms.Elements.Count)
    # markers
    groups=field(tf.Fields,'marker groups')
    for i in range(groups.Elements.Count):
        gfs=groups.Elements[i].Fields
        nm=str(field(gfs,'name').Data)
        m=field(gfs,'markers')
        if not m or m.Elements.Count==0: continue
        mf=m.Elements[0].Fields
        if nm in ('armor_fx_00','armor_fx_01','armor_fx_02','armor_fx_03','armor_fx_04','armor_fx_05'):
            idx=int(nm[-2:]); field(mf,'region index').Data=0; field(mf,'permutation index').Data=1+idx
        elif nm.startswith('flame_pri_'):
            id=nm[len('flame_pri_'):]
            if id not in PRI: continue
            i2=PRI.index(id)
            if i2<25:
                field(mf,'region index').Data=0; field(mf,'permutation index').Data=7+i2
            else:
                field(mf,'region index').Data=15; field(mf,'permutation index').Data=i2-25
    L("markers patched")
    tf.Save(); tf.Dispose()
    # pmcg both lists
    rel=r"globals\player_model_customization.player_model_customization_globals"
    tf=load(rel)
    for listname in ('spartan armor effect selections','elite armor effect selections'):
        block=field(tf.Fields,listname)
        for i,id in enumerate(PRI):
            fs=block.Elements[7+i].Fields
            bits=field(fs,'customized bits').Elements[0].Fields
            if i<25:
                field(bits,'region name').SetStringData('armor_fx'); field(bits,'permutation name').SetStringData('armor_fx_%02d'%(i+6))
            else:
                field(bits,'region name').SetStringData('armor_fx_pri'); field(bits,'permutation name').SetStringData('armor_fx_pri_%02d'%(i-25))
        L("pmcg %s entries=%d"%(listname,block.Elements.Count))
    tf.Save(); tf.Dispose()
    mb_stop()
except Exception:
    out.append("ERROR "+traceback.format_exc())
open(os.path.join(OUT,'fix_default_perm.txt'),'w',encoding='utf-8').write("\n".join(out))
