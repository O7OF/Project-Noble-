# Read or set "force bitmap format" of bitmap tags. BITMAPS="path|path" (no ext). LIST=1 prints enum options. SET=<index>
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for bp in os.environ['BITMAPS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(bp, 'bitmap'))
        f = tf.SelectField('force bitmap format')
        L('%s: force format = %s' % (bp, f.Value))
        if os.environ.get('LIST'):
            try:
                for i, it in enumerate(f.Items): L('  %d %s' % (i, it.EnumName if hasattr(it, 'EnumName') else it))
            except Exception as ex:
                L('  items err ' + str(ex)[:120])
                L('  members: ' + ','.join(x for x in dir(f) if not x.startswith('_'))[:600])
        if os.environ.get('SET'):
            f.Value = int(os.environ['SET']); tf.Save(); L('  set -> %s SAVED' % f.Value)
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'bm_format.txt')), 'w').write('\n'.join(out))
