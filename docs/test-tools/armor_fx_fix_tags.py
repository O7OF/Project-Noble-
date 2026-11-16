# Regenerate the 5 armour_fx_ext effect tags correctly (reuse stock particle systems; rename locations).
import os, shutil, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
VISORS=["default","silver","black","gold","rose_gold","emerald","ice","crimson","onyx","violet","amber","cyan","ruby",
        "sapphire","amethyst","bronze","sunfire","lime","platinum","obsidian","pearl","electric_blue","blood_orange","mint",
        "copper","midnight","magenta","aquamarine","titanium"]
TAGS=5; PER=6
out=[]; L=out.append
def pth(rel): return os.path.join(HT, rel)
def load(rel):
    base, ext=os.path.splitext(rel)
    tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
def setref(f, rel):
    base, ext=os.path.splitext(rel); f.Path=B.Tags.TagPath.FromPathAndExtension(base, ext[1:])
try:
    src=pth(r"objects\characters\spartans\fx\armor_fx\armor_fx.effect")
    for t in range(TAGS):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        dst=pth(rel); shutil.copy2(src,dst)
        tf=load(rel)
        loc=field(tf.Fields,'locations'); ev=field(tf.Fields,'events')
        L("tag %d: locations=%d events=%d"%(t,loc.Elements.Count,ev.Elements.Count))
        chunk=VISORS[t*PER:(t+1)*PER]
        for i,vid in enumerate(chunk):
            if i>=loc.Elements.Count: break
            field(loc.Elements[i].Fields,'marker name').SetStringData("armor_fx_ext_t%d_%d"%(t,i))
            if i<ev.Elements.Count:
                fs=ev.Elements[i].Fields
            else:
                fs=ev.DuplicateElement(0).Fields
            field(fs,'event name').SetStringData("legendary_flame_%s"%vid)
            ps=field(fs,'particle systems')
            if ps.Elements.Count==0:
                L("  ERROR empty particle system for %s"%vid); continue
            p0=ps.Elements[0].Fields
            setref(field(p0,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_%s.particle"%vid)
            field(p0,'location').Value=i
        tf.Save(); tf.Dispose()
        # verify
        tf=load(rel); ev=field(tf.Fields,'events'); loc=field(tf.Fields,'locations')
        L("  saved: locations=%d events=%d"%(loc.Elements.Count,ev.Elements.Count))
        for i in range(ev.Elements.Count):
            fs=ev.Elements[i].Fields; ps=field(fs,'particle systems').Elements[0].Fields
            L("   %d %s loc=%s -> %s"%(i,field(fs,'event name').Data,field(ps,'location').Value,
                                       field(ps,'particle').Path.RelativePathWithExtension))
        tf.Dispose()
    # verify biped attachments
    tf=load(r"objects\characters\spartans\spartans.biped")
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments'); L("biped attachments=%d"%att.Elements.Count)
    for i in range(att.Elements.Count):
        ft=field(att.Elements[i].Fields,'type')
        try: L("  %d %s"%(i,ft.Path.RelativePathWithExtension))
        except Exception: L("  %d <?>"%i)
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','armor_fx_fix_tags.txt')),'w',encoding='utf-8').write("\n".join(out))
