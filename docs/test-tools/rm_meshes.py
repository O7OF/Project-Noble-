# For each region/permutation of the marine render model: mesh index and mesh count (-1 / 0 = empty).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    p = os.environ.get('RM', r'objects\characters\marine\marine')
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'render_model'))
    for r in tf.SelectField('Block:regions').Elements:
        parts = []
        for e in r.SelectField('Block:permutations').Elements:
            mi = mc = '?'
            for f in e.Fields:
                n = str(f.FieldName)
                try:
                    if n == 'mesh index': mi = str(f.Value if hasattr(f, 'Value') else f.Data)
                    if n == 'mesh count': mc = str(f.Data)
                except Exception: pass
            parts.append('%s(m%s x%s)' % (e.SelectField('name').Data, mi, mc))
        L('%s: %s' % (r.SelectField('name').Data, ', '.join(parts)))
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'rm_meshes.txt')), 'w').write('\n'.join(out))
