# Inspect the biped armor_fx attachment's change-color enum items, and effect tag's colour-related blocks.
import os, traceback
from mbstart import B, mb_stop
HT=r"D:\Steam\steamapps\common\HREK\tags"
OUT=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out=[]; L=out.append
def load(rel):
    base, ext=os.path.splitext(rel)
    tf=B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:])); return tf
def field(fs,name):
    for f in fs:
        if str(f.FieldName)==name: return f
    return None
try:
    tf=load(r"objects\characters\spartans\spartans.biped")
    unit=field(tf.Fields,'unit').Elements[0].Fields; obj=field(unit,'object').Elements[0].Fields
    att=field(obj,'attachments')
    a5=att.Elements[5].Fields
    cc=field(a5,'change color')
    L("change color type=%s value=%s"%(type(cc), getattr(cc,'Value',None)))
    try:
        items=list(cc.Items); L("items=%d"%len(items))
        for i,it in enumerate(items): L("  %d %s"%(i,getattr(it,'EnumName',it)))
    except Exception as e: L("items err "+str(e)[:120])
    # print all fields of attachment 5
    for f in a5: L("  field %s type=%s value=%s"%(f.FieldName,f.FieldType,getattr(f,'Value',getattr(f,'Data',''))))
    tf.Dispose()
    tf=load(r"objects\characters\spartans\fx\armor_fx\armor_fx.effect")
    L("\neffect top-level field names:")
    for f in tf.Fields: L("  %s:%s"%(f.FieldName,f.FieldType))
    tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n"+traceback.format_exc())
open(os.path.join(OUT, os.environ.get('OUT','changecolor_probe.txt')),'w',encoding='utf-8').write("\n".join(out))
