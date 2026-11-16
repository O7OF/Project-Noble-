# Regenerate all five ext effect tags so EVERY event is a duplicate of the legendary_flame event (correct particle-system
# properties for third person), then assign one visor-colour particle per event.
import os, shutil, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
NEW=["default","silver","black","gold","rose_gold","emerald","ice","crimson","onyx","violet","amber","cyan","ruby",
     "sapphire","amethyst","bronze","sunfire","lime","platinum","obsidian","pearl","electric_blue","blood_orange","mint",
     "copper","midnight","magenta","aquamarine","titanium"]
TAGS=5; PER=6
out=[]; L=out.append
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
    src=os.path.join(HT, r"objects\characters\spartans\fx\armor_fx\armor_fx.effect")
    for t in range(TAGS):
        rel=r"objects\characters\spartans\fx\armor_fx\armor_fx_ext%d.effect"%t
        shutil.copy2(src, os.path.join(HT, rel))
        tf=load(rel)
        ev=field(tf.Fields,'events'); loc=field(tf.Fields,'locations')
        chunk=NEW[t*PER:(t+1)*PER]; n=len(chunk)
        # duplicate the legendary_flame event (index 0) n times -> they append at indices 6..6+n-1
        for _ in range(n):
            ev.DuplicateElement(0)
        # remove the six original events (indices 5..0)
        for _ in range(6):
            ev.RemoveElement(0)
        L("tag %d: after template dup events=%d (want %d)"%(t,ev.Elements.Count,n))
        for i,vid in enumerate(chunk):
            if i>=ev.Elements.Count: break
            fs=ev.Elements[i].Fields
            field(fs,'event name').SetStringData("legendary_flame_%s"%vid)
            ps=field(fs,'particle systems')
            p0=ps.Elements[0].Fields
            setref(field(p0,'particle'), r"objects\characters\spartans\fx\armor_fx\particles\head_fire_%s.particle"%vid)
            field(p0,'location').Value=i
            if i<loc.Elements.Count:
                field(loc.Elements[i].Fields,'marker name').SetStringData("armor_fx_ext_t%d_%d"%(t,i))
        tf.Save(); tf.Dispose()
        # verify
        tf=load(rel); ev=field(tf.Fields,'events'); loc=field(tf.Fields,'locations')
        L("  saved events=%d locations=%d"%(ev.Elements.Count,loc.Elements.Count))
        for i in range(min(ev.Elements.Count,6)):
            fs=ev.Elements[i].Fields; ps=field(fs,'particle systems').Elements[0].Fields
            L("   %d %s loc=%s -> %s"%(i,field(fs,'event name').Data,field(ps,'location').Value,field(ps,'particle').Path.RelativePathWithExtension))
        tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','ext_regen_events.txt')),'w',encoding='utf-8').write("\n".join(out))
