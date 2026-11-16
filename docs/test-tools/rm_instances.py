# 2026-11-13: marine render model: per permutation all non-block fields (mesh index/count, instance masks...) and the
# instance placements block (names + mesh). Read only. RM env (default objects\characters\marine\marine).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def val(f):
    if 'instance mask' in str(f.FieldName):
        try:
            raw = bytes(bytearray(f.GetRawData()))
            v = int.from_bytes(raw[:4], 'little'); return '0x%08X' % v
        except Exception as ex: return 'raw?' + str(ex)[:40]
    for a in ('Data', 'Value'):
        try: return str(getattr(f, a))
        except Exception: pass
    return '?'
try:
    from mbstart import B, mb_stop
    rmp = os.environ.get('RM', r'objects\characters\marine\marine')
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(rmp, 'render_model'))
    for f in tf.Fields:
        if str(f.FieldType) == 'Block': L('block %s: %d' % (f.FieldName, f.Elements.Count))
    first = True
    for r in tf.SelectField('Block:regions').Elements:
        for p in r.SelectField('Block:permutations').Elements:
            parts = []
            for f in p.Fields:
                if str(f.FieldType) in ('Block', 'Struct'): continue
                parts.append('%s=%s' % (f.FieldName, val(f)))
            L('%s/%s: %s' % (r.SelectField('name').Data, val(p.SelectField('name')), ' '.join(parts[1:])))
    for blkname in ('instance placements', 'instance mesh', 'instanced geometry'):
        try:
            blk = tf.SelectField('Block:' + blkname)
            L('== %s (%d)' % (blkname, blk.Elements.Count))
            for i, e in enumerate(blk.Elements):
                L('  %d: %s' % (i, ' '.join('%s=%s' % (f.FieldName, val(f)) for f in e.Fields if str(f.FieldType) not in ('Block', 'Struct'))))
        except Exception as ex:
            L('(no block %s)' % blkname)
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'rm_instances.txt')), 'w').write('\n'.join(out))
