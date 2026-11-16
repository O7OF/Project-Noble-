# 2026-11-13 fix: "noble_noknife" = exact copy of "default" (region permutations KEPT) minus the knife/sheath objects.
# v1 emptied the region lists; Reach then picks RANDOM permutations for those regions (head showed MK V instead of AKIS,
# flair_chest_b showed a knife).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\spartans\spartans', 'model'))
    variants = tf.SelectField('Block:variants')
    names = [str(v.SelectField('name').Data) for v in variants.Elements]
    if 'noble_noknife' in names:
        variants.RemoveElement(names.index('noble_noknife')); L('old noble_noknife removed')
    names = [str(v.SelectField('name').Data) for v in variants.Elements]
    variants.DuplicateElement(names.index('default'))
    nv = list(variants.Elements)[-1]
    nv.SelectField('name').Data = 'noble_noknife'
    nv.SelectField('Block:objects').RemoveAllElements()
    for r in nv.SelectField('Block:regions').Elements:
        perms = []
        for f in r.Fields:
            if str(f.FieldType) == 'Block':
                for e in f.Elements:
                    try: perms.append(str(e.SelectField('permutation name').Data))
                    except Exception: pass
        L('  %s: %s' % (r.SelectField('region name').Data, ','.join(perms)))
    L('objects: %d' % nv.SelectField('Block:objects').Elements.Count)
    tf.Save(); L('SAVED')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:800])
open(os.path.join(T, os.environ.get('OUT', 'noknife2.txt')), 'w').write('\n'.join(out))
