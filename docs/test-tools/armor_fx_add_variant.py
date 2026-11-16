# Add the first armour-effect variant: legendary_flame_blue (blue flames, pmcg index 7, armor_fx_06).
# Edits HREK tags in place after writing .bak backups; prints a verification report.
import os, traceback, shutil
from mbstart import B, mb_stop
HT = r"D:\Steam\steamapps\common\HREK\tags"
OUT = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out=[]; L=out.append
def p(path): return os.path.join(HT, path)
def backup(path):
    src=p(path); dst=src+'.bak'
    if not os.path.exists(dst): shutil.copy2(src,dst); L("backup "+os.path.relpath(dst,HT))
def load(path):
    base, ext = os.path.splitext(path)
    tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
def setsid(f, s):
    f.SetStringData(s)
def setint(f, v):
    f.Data = v
def setref(f, relpath):
    base, ext = os.path.splitext(relpath)
    f.Path = B.Tags.TagPath.FromPathAndExtension(base, ext[1:])
def verify_effect(tf):
    loc=field(tf.Fields,'locations'); L(f"locations {loc.Elements.Count}: " + ", ".join(setsid_name(loc.Elements[i]) for i in range(loc.Elements.Count)))
    ev=field(tf.Fields,'events'); L(f"events {ev.Elements.Count}")
    for i in range(ev.Elements.Count):
        fs=ev.Elements[i].Fields
        name=str(field(fs,'event name').Data)
        ps=field(fs,'particle systems')
        extra=""
        if ps.Elements.Count:
            p0=ps.Elements[0].Fields
            extra=str(field(p0,'particle').Path.RelativePathWithExtension)+" loc="+str(field(p0,'location').Value)
        L(f"  #{i} {name} {extra}")
def setsid_name(el):
    for f in el.Fields:
        if str(f.FieldName) in ('marker name','event name','name','selection name','region name','permutation name'):
            return str(f.Data)
    return '?'
try:
    # ---------- effect tag ----------
    ep="objects\\characters\\spartans\\fx\\armor_fx\\armor_fx.effect"; backup(ep)
    tf=load(ep)
    loc=field(tf.Fields,'locations')
    if loc.Elements.Count < 7:
        el=loc.DuplicateElement(0)
        f=field(el.Fields,'marker name'); setsid(f,"armor_fx_06")
        L("effect: added location armor_fx_06")
    ev=field(tf.Fields,'events')
    # add event legendary_flame_blue if not present
    names=[str(field(ev.Elements[i].Fields,'event name').Data) for i in range(ev.Elements.Count)]
    if "legendary_flame_blue" not in names:
        el=ev.DuplicateElement(0)
        fs=el.Fields
        setsid(field(fs,'event name'),"legendary_flame_blue")
        ps=field(fs,'particle systems')
        p0=ps.Elements[0].Fields
        setref(field(p0,'particle'),"objects\\characters\\spartans\\fx\\armor_fx\\particles\\head_fire_blue.particle")
        setint(field(p0,'location'), 6)
        L("effect: added event legendary_flame_blue -> head_fire_blue.particle loc 6")
    verify_effect(tf)
    tf.Save(); tf.Dispose()
    # ---------- render_model: permutation + marker group ----------
    rp="objects\\characters\\spartans\\spartans.render_model"; backup(rp)
    tf=load(rp)
    regions=field(tf.Fields,'regions'); region_el=None
    for i in range(regions.Elements.Count):
        fs=regions.Elements[i].Fields
        if str(field(fs,'name').Data)=='armor_fx': region_el=regions.Elements[i]; break
    perms=field(region_el.Fields,'permutations')
    pnames=[str(field(perms.Elements[i].Fields,'name').Data) for i in range(perms.Elements.Count)]
    if "armor_fx_06" not in pnames:
        el=perms.DuplicateElement(0)
        setsid(field(el.Fields,'name'),"armor_fx_06")
        L("render_model: added permutation armor_fx_06")
    groups=field(tf.Fields,'marker groups')
    gnames=[str(field(groups.Elements[i].Fields,'name').Data) for i in range(groups.Elements.Count)]
    if "armor_fx_06" not in gnames:
        el=groups.DuplicateElement(0)
        setsid(field(el.Fields,'name'),"armor_fx_06")
        markers=field(el.Fields,'markers')
        setint(field(markers.Elements[0].Fields,'permutation index'), 6)
        L("render_model: added marker group armor_fx_06 (permutation index 6)")
    tf.Save(); tf.Dispose()
    # ---------- pmcg: spartan + elite effect lists ----------
    pp="globals\\player_model_customization.player_model_customization_globals"; backup(pp)
    tf=load(pp)
    for listname in ('spartan armor effect selections','elite armor effect selections'):
        block=field(tf.Fields,listname)
        names=[str(field(block.Elements[i].Fields,'selection name').Data) for i in range(block.Elements.Count)]
        if "legendary_flame_blue" in names:
            L(f"pmcg {listname}: already present"); continue
        el=block.DuplicateElement(1)   # copy legendary_flame entry
        fs=el.Fields
        setsid(field(fs,'selection name'),"legendary_flame_blue")
        bits=field(fs,'customized bits')
        b0=bits.Elements[0].Fields
        setsid(field(b0,'region name'),"armor_fx")
        setsid(field(b0,'permutation name'),"armor_fx_06")
        L(f"pmcg {listname}: added legendary_flame_blue -> armor_fx/armor_fx_06 (count {block.Elements.Count})")
    tf.Save(); tf.Dispose()
    # verification reload
    tf=load(pp)
    for listname in ('spartan armor effect selections','elite armor effect selections'):
        block=field(tf.Fields,listname)
        L(f"{listname} [{block.Elements.Count}]")
        for i in range(block.Elements.Count):
            fs=block.Elements[i].Fields
            nm=str(field(fs,'selection name').Data)
            bits=field(fs,'customized bits')
            ch=[]
            for k in range(bits.Elements.Count):
                cfs=bits.Elements[k].Fields
                ch.append(str(field(cfs,'region name').Data)+"/"+str(field(cfs,'permutation name').Data))
            L(f"  #{i} {nm} -> {ch}")
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','armor_fx_variant_edit.txt')),'w',encoding='utf-8').write("\n".join(out))
