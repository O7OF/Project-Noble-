# Read-only: print top-level import fields (usage, curve mode, format, bump height, etc.) of bitmaps. BITMAPS = "tag path (no ext)|..."
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for p in os.environ['BITMAPS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'bitmap'))
        L('#### ' + p)
        for f in tf.Fields:
            nm = str(f.FieldName); ty = str(f.FieldType)
            if ty in ('Block',):
                try: L('  %-28s block count=%d' % (nm, f.Elements.Count))
                except Exception: pass
                continue
            if ty in ('Struct', 'Data', 'ArrayStart', 'Pad', 'Skip', 'Explanation', 'Custom'): continue
            try: v = f.Value
            except Exception: v = '?'
            try:
                if ty.endswith('Enum') or ty.endswith('Flags'): v = '%s (%s)' % (v, f.Data)
            except Exception: pass
            L('  %-28s %s' % (nm, v))
        try:
            ov = tf.SelectField('Block:usage override')
            for e in ov.Elements:
                L('  [usage override] ' + ', '.join('%s=%s' % (str(x.FieldName), x.Value if hasattr(x, 'Value') else '') for x in e.Fields if str(x.FieldType) not in ('Pad', 'Skip')))
        except Exception as ex: L('  (usage override: %s)' % str(ex)[:60])
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'bm_props.txt')), 'w').write('\n'.join(out))
