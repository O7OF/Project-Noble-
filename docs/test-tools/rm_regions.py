# List regions + permutations of render models, and variants of models (read only). RMS="path|path" (no ext), MODELS="path|path"
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for p in [x for x in os.environ.get('RMS', '').split('|') if x]:
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'render_model'))
        L('== render_model ' + p)
        for r in tf.SelectField('Block:regions').Elements:
            perms = [str(e.SelectField('name').Data) for e in r.SelectField('Block:permutations').Elements]
            L('  %s (%d): %s' % (r.SelectField('name').Data, len(perms), ', '.join(perms)))
        tf.Dispose()
    for p in [x for x in os.environ.get('MODELS', '').split('|') if x]:
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'model'))
        L('== model ' + p)
        L('  variants: ' + ', '.join(str(v.SelectField('name').Data) for v in tf.SelectField('Block:variants').Elements))
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1200])
open(os.path.join(T, os.environ.get('OUT', 'rm_regions.txt')), 'w').write('\n'.join(out))
