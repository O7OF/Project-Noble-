# Point object name "marine_armory" at its own biped placement (dump fields of names chief_armory / marine_armory).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def dump(el, pre=''):
    for f in el.Fields:
        v = '?'
        for a in ('Data', 'Value'):
            try: v = str(getattr(f, a)); break
            except Exception: pass
        L('  %s%s [%s] = %s' % (pre, f.FieldName, f.FieldType, v))
        if str(f.FieldType) == 'Struct':
            for s in f.Elements: dump(s, pre + str(f.FieldName) + '/')
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\mainmenu', 'scenario'))
    names = list(tf.SelectField('Block:object names').Elements)
    nl = [str(e.SelectField('name').Data) for e in names]
    ci, mi = nl.index('chief_armory'), nl.index('marine_armory')
    bp = list(tf.SelectField('Block:bipeds').Elements)
    placement = [i for i, e in enumerate(bp) if int(str(e.SelectField('name').Value)) == mi]
    L('marine placement index %s' % placement)
    for idx in (ci, mi):
        L('name %d %s' % (idx, nl[idx])); dump(names[idx])
    if os.environ.get('WRITE') and placement:
        ne = names[mi]
        for f in ne.Fields:
            if 'placement' in str(f.FieldName).lower() and 'index' in str(f.FieldName).lower():
                try: f.Data = placement[0]
                except Exception: f.Value = placement[0]
                L('set %s = %d' % (f.FieldName, placement[0]))
            if str(f.FieldType) == 'Struct':
                for s in f.Elements:
                    for g in s.Fields:
                        if 'placement' in str(g.FieldName).lower() and 'index' in str(g.FieldName).lower():
                            try: g.Data = placement[0]
                            except Exception: g.Value = placement[0]
                            L('set %s = %d' % (g.FieldName, placement[0]))
        tf.Save(); L('SAVED')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_fix_marine_name.txt')), 'w').write('\n'.join(out))
