# Dump spartans.model variants "default" and "noble_noknife": regions + permutation entries + objects (read only).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\spartans\spartans', 'model'))
    for v in tf.SelectField('Block:variants').Elements:
        name = str(v.SelectField('name').Data)
        if name not in ('default', 'noble_noknife'): continue
        L('== variant ' + name)
        for r in v.SelectField('Block:regions').Elements:
            rn = str(r.SelectField('region name').Data)
            perms = []
            for f in r.Fields:
                if str(f.FieldType) == 'Block':
                    for e in f.Elements:
                        try: perms.append(str(e.SelectField('permutation name').Data))
                        except Exception: perms.append('?')
            L('  region %s: %s' % (rn, ','.join(perms)))
        for o in v.SelectField('Block:objects').Elements:
            L('  object ' + ' | '.join('%s=%s' % (f.FieldName, getattr(f, 'Data', '')) for f in o.Fields if hasattr(f, 'Data'))[:200])
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:800])
open(os.path.join(T, os.environ.get('OUT', 'variant_dump.txt')), 'w').write('\n'.join(out))
