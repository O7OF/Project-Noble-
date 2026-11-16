# Dump object id fields of every biped placement; WRITE=1 gives placement 9 (marine_armory) a unique id (max+1).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def idfields(e):
    od = e.SelectField('Struct:object data')
    oid = list(od.Elements)[0].SelectField('Struct:object id')
    return list(list(oid.Elements)[0].Fields)
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\mainmenu', 'scenario'))
    bp = list(tf.SelectField('Block:bipeds').Elements)
    uids = []
    for i, e in enumerate(bp):
        parts = []
        for f in idfields(e):
            v = '?'
            for a in ('Data', 'Value'):
                try: v = str(getattr(f, a)); break
                except Exception: pass
            parts.append('%s=%s' % (f.FieldName, v))
            if 'unique' in str(f.FieldName).lower():
                try: uids.append(int(v))
                except Exception: pass
        L('biped %d: %s' % (i, ' | '.join(parts)))
    if os.environ.get('WRITE'):
        # every unique id used by any scenario object, so the new one doesn't collide
        new = max(uids) + 1
        for f in idfields(bp[9]):
            if 'unique' in str(f.FieldName).lower():
                f.Data = new; L('placement 9 unique id -> %d' % new)
        tf.Save(); L('SAVED')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_objid.txt')), 'w').write('\n'.join(out))
