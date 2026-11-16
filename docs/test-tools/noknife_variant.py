# spartans.model: add variant "noble_noknife" = copy of "default" with no region permutations and NO objects
# (knife/sheath). Setting it after the pieces keeps the pieces and drops the knife.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\spartans\spartans', 'model'))
    variants = tf.SelectField('Block:variants')
    names = [str(v.SelectField('name').Data) for v in variants.Elements]
    if 'noble_noknife' in names:
        L('already there')
    else:
        variants.DuplicateElement(names.index('default'))
        nv = list(variants.Elements)[-1]
        nv.SelectField('name').Data = 'noble_noknife'
        for r in nv.SelectField('Block:regions').Elements:
            for f in r.Fields:
                if str(f.FieldType) == 'Block' and 'runtime' not in str(f.FieldName): f.RemoveAllElements()
        nv.SelectField('Block:objects').RemoveAllElements()
        L('objects left on noble_noknife: %d' % nv.SelectField('Block:objects').Elements.Count)
        L('objects on default: %d' % list(variants.Elements)[names.index('default')].SelectField('Block:objects').Elements.Count)
        tf.Save(); L('SAVED')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:800])
open(os.path.join(T, os.environ.get('OUT', 'noknife.txt')), 'w').write('\n'.join(out))
