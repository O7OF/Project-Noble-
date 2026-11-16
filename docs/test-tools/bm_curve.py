# Make a bitmap tag use stock-style import settings: curve mode = 0 (choose best), clear the usage override block (which forces sRGB 2.2).
# BITMAPS = "tag path (no ext)|..." relative to tags. Then re-run `tool reimport-bitmaps-single` for each.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for p in os.environ['BITMAPS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'bitmap'))
        cm = tf.SelectField('LongEnum:curve mode') if False else None
        done = []
        for f in tf.Fields:
            if str(f.FieldName) == 'curve mode':
                done.append('curve mode %s -> %s' % (f.Value, os.environ.get('CURVE','0'))); f.Value = int(os.environ.get('CURVE','0'))
            if str(f.FieldName) == 'usage override':
                done.append('usage override [%d] -> cleared' % f.Elements.Count); f.RemoveAllElements()
        tf.Save(); tf.Dispose(); L(p + ': ' + '; '.join(done))
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'bm_curve.txt')), 'w').write('\n'.join(out))
