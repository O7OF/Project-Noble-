# Processed size/format of every bitmap tag in a folder (DIR, relative to tags).
import os, glob, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
HT = r"D:\Steam\steamapps\common\HREK\tags"
out = []
try:
    from mbstart import B, mb_stop
    for p in sorted(glob.glob(os.path.join(HT, os.environ['DIR'], '*.bitmap'))):
        rel = os.path.splitext(os.path.relpath(p, HT))[0]
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(rel, 'bitmap'))
        bms = tf.SelectField('Block:bitmaps')
        if bms.Elements.Count:
            e = bms.Elements[0]
            w = e.SelectField('width').Data; h = e.SelectField('height').Data
            fmt = e.SelectField('format'); fs = str(fmt.Items[fmt.Value].EnumName)
            out.append(f"{os.path.basename(rel):48} {w}x{h} {fs}" + ("   <-- BROKEN" if w <= 1 else ""))
        else: out.append(f"{os.path.basename(rel):48} no bitmaps")
        tf.Dispose()
    mb_stop()
except Exception:
    out.append("ERROR\n" + traceback.format_exc())
open(os.path.join(T, 'bmsizes.txt'), 'w').write("\n".join(out))
