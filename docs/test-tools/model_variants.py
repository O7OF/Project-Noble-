# 2026-11-14: a model tag's variants: name + region -> permutation list. MODEL env (default objects\characters\marine\marine).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def v(f):
    for a in ('Data', 'Value'):
        try: return str(getattr(f, a))
        except Exception: pass
    return '?'
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(os.environ.get('MODEL', r'objects\characters\marine\marine'), 'model'))
    for var in tf.SelectField('Block:variants').Elements:
        parts = []
        for r in var.SelectField('Block:regions').Elements:
            perms = [v(p.SelectField('permutation name')) for p in r.SelectField('Block:permutations').Elements]
            parts.append('%s=%s' % (v(r.SelectField('region name')), '/'.join(perms)))
        L('%s: %s' % (v(var.SelectField('name')), '  '.join(parts)))
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'model_variants.txt')), 'w').write('\n'.join(out))
