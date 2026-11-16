# Prints usage-related fields of bitmap tags. BITMAPS = "path|path" (no ext). SET_USAGE=<enum index> + SET_ONLY=<path> optional.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for p in os.environ['BITMAPS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'bitmap'))
        L('#### ' + p)
        for f in tf.Fields:
            n = str(f.FieldName)
            if n in ('Usage', 'usage', 'curve mode', 'force bitmap format', 'bump height', 'source gamma', 'format'):
                try:
                    items = list(f.Items)
                    L('  %-20s = %s (%d)  options: %s' % (n, items[f.Value].EnumName if f.Value < len(items) else '?', f.Value, ', '.join('%d:%s' % (i, x.EnumName) for i, x in enumerate(items))[:900] if n == 'Usage' else ''))
                except Exception as ex:
                    L('  %-20s = %s' % (n, getattr(f, 'Value', '?')))
            if n == 'usage override':
                L('  usage override elements: %d' % f.Elements.Count)
        if os.environ.get('SET_USAGE') and p == os.environ.get('SET_ONLY'):
            for f in tf.Fields:
                if str(f.FieldName) == 'Usage':
                    f.Value = int(os.environ['SET_USAGE']); L('  >> usage set to %s' % os.environ['SET_USAGE'])
            tf.Save()
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'bm_usage.txt')), 'w').write('\n'.join(out))


