# Dump the mainmenu scenario's biped palette, biped placements (name, palette, position, rotation, variant) and object names count (read only).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def val(f):
    for a in ('Data', 'Value'):
        try:
            v = getattr(f, a)
            try: return ','.join('%.3f' % x for x in v)
            except Exception: return str(v)
        except Exception: pass
    return '?'
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\mainmenu', 'scenario'))
    names = [str(e.SelectField('name').Data) for e in tf.SelectField('Block:object names').Elements]
    L('object names: %d: %s' % (len(names), ', '.join('%d=%s' % (i, n) for i, n in enumerate(names))))
    pal = tf.SelectField('Block:biped palette')
    for i, e in enumerate(pal.Elements):
        r = e.SelectField('name')
        L('palette %d: %s' % (i, r.Path.RelativePathWithExtension if r.Path else ''))
    bp = tf.SelectField('Block:bipeds')
    L('bipeds: %d' % bp.Elements.Count)
    for i, e in enumerate(bp.Elements):
        parts = []
        def walk(el, pre, depth):
            for f in el.Fields:
                nm = str(f.FieldName); ft = str(f.FieldType)
                if ft in ('Struct',) and depth < 3:
                    for s in f.Elements: walk(s, pre + nm + '/', depth + 1)
                elif nm in ('type', 'name', 'position', 'rotation', 'variant name', 'scale', 'bsp policy', 'manual bsp flags', 'editor folder', 'placement flags', 'origin bsp index'):
                    parts.append('%s%s=%s' % (pre, nm, val(f)))
        walk(e, '', 0)
        L('biped %d: %s' % (i, ' | '.join(parts)))
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_bipeds.txt')), 'w').write('\n'.join(out))
