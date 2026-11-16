# 2026-11-13: compare the "change colors" block of bipeds (BIPEDS="path|path"). COPY_FROM=<biped> COPY_TO="path|path"
# copies the whole block (element count + values) from one biped into the others.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def find_block(el, name, depth=0):
    for f in el.Fields:
        if str(f.FieldType) == 'Block' and str(f.FieldName) == name: return f
        if str(f.FieldType) == 'Struct' and depth < 3:
            for s in f.Elements:
                r = find_block(s, name, depth + 1)
                if r is not None: return r
    return None
try:
    from mbstart import B, mb_stop
    for p in [x for x in os.environ.get('BIPEDS', '').split('|') if x]:
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'biped'))
        blk = find_block(tf, 'change colors')
        L('%s: change colors = %s' % (p, blk.Elements.Count if blk is not None else 'NOT FOUND'))
        tf.Dispose()
    src = os.environ.get('COPY_FROM')
    if src:
        sf = B.Tags.TagFile(); sf.Load(B.Tags.TagPath.FromPathAndExtension(src, 'biped'))
        sblk = find_block(sf, 'change colors')
        for p in [x for x in os.environ.get('COPY_TO', '').split('|') if x]:
            tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'biped'))
            tblk = find_block(tf, 'change colors')
            tblk.RemoveAllElements()
            for i in range(sblk.Elements.Count):
                tblk.AddElement()
            # copy element data field by field (binary serialize of each element)
            for se, te in zip(sblk.Elements, tblk.Elements):
                te.Deserialize(se.Serialize()) if hasattr(te, 'Deserialize') else None
            tf.Save(); L('%s: change colors now %d (copied from %s)' % (p, tblk.Elements.Count, src)); tf.Dispose()
        sf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'biped_cc.txt')), 'w').write('\n'.join(out))
