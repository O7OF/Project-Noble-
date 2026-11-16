# 2026-11-14 Trooper mode: every scenario field (2 block levels) whose name mentions player / representation / unit /
# spartan, plus references to bipeds. SCN env (default levels\solo\m35\m35). Read only.
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
KEYS = ('player', 'represent', 'spartan', 'biped', 'unit', 'character')
def walk(el, path, depth):
    for f in el.Fields:
        ft = str(f.FieldType); n = str(f.FieldName)
        hit = any(k in n.lower() for k in KEYS)
        if ft == 'Block':
            if hit: L('%s[%s] %d' % (path, n, f.Elements.Count))
            if depth < 2 and (hit or depth == 0) and f.Elements.Count:
                for i, e in enumerate(f.Elements):
                    if i >= 4: break
                    walk(e, '%s%s[%d].' % (path, n, i), depth + 1)
        elif ft == 'Struct':
            for s in f.Elements: walk(s, path, depth)
        elif ft not in ('Pad', 'Skip', 'Explanation', 'Custom', 'UselessPad'):
            v = val(f)
            if hit or (ft == 'Reference' and 'bipd' in v.lower() or v.endswith('.biped')):
                L('%s%s = %s' % (path, n, v))
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(os.environ.get('SCN', r'levels\solo\m35\m35'), 'scenario'))
    walk(tf, '', 0)
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_player.txt')), 'w').write('\n'.join(out))
