# 2026-11-13: dump a scenario's "map variant palettes" (Forge palette): palette fields, entries and their variants.
# SCN env (default forge_halo). PAL env = only palettes whose name contains it. Read only.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def val(f):
    try:
        if str(f.FieldType) == 'Reference': return str(f.Path.RelativePathWithExtension) if f.Path else ''
    except Exception: pass
    for a in ('Data', 'Value'):
        try: return str(getattr(f, a))
        except Exception: pass
    return '?'
def dump(el, ind, depth):
    for f in el.Fields:
        ft = str(f.FieldType)
        if ft == 'Block':
            L('%s[%s] %d' % (ind, f.FieldName, f.Elements.Count))
            if depth < 3:
                for i, e in enumerate(f.Elements):
                    if i >= int(os.environ.get('N', '40')): L(ind + '  ...'); break
                    L('%s  #%d' % (ind, i)); dump(e, ind + '    ', depth + 1)
        elif ft == 'Struct':
            for s in f.Elements: dump(s, ind, depth)
        elif ft not in ('Pad', 'Skip', 'Explanation', 'Custom', 'UselessPad'):
            L('%s%s = %s' % (ind, f.FieldName, val(f)))
try:
    from mbstart import B, mb_stop
    scn = os.environ.get('SCN', r'levels\multi\forge_halo\forge_halo')
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(scn, 'scenario'))
    pals = tf.SelectField('Block:map variant palettes')
    for i, p in enumerate(pals.Elements):
        nm = ' '.join(val(f) for f in p.Fields if str(f.FieldType) not in ('Block', 'Struct'))
        if os.environ.get('PAL') and os.environ['PAL'] not in nm: L('palette %d: %s (skipped)' % (i, nm)); continue
        L('==== palette %d' % i); dump(p, '  ', 0)
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_mvp.txt')), 'w').write('\n'.join(out))
