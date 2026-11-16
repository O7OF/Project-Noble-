# Print compiled image format/size of bitmap tags. BITMAPS="a|b" (no ext)
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for bp in os.environ['BITMAPS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(bp, 'bitmap'))
        L('#### ' + bp + '  usage=%s force=%s' % (tf.SelectField('Usage').Value, tf.SelectField('force bitmap format').Value))
        for e in tf.SelectField('Block:bitmaps').Elements:
            s = []
            for f in e.Fields:
                n = str(f.FieldName)
                if n in ('width', 'height', 'format', 'type', 'mipmap count', 'curve', 'flags', 'depth'):
                    try: v = f.Value
                    except Exception: v = '?'
                    s.append('%s=%s' % (n, v))
            L('  ' + ' '.join(s))
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'bm_compiled.txt')), 'w').write('\n'.join(out))
