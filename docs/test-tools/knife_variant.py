# spartans.model knife fix, step-wise. STEPS="dup,clearregions,cleardefaultobjs,removevariantobjs" (comma list).
#  dup: duplicate variant "default" as "noble_knife" | clearregions: empty noble_knife's regions
#  cleardefaultobjs: remove the knife objects from "default"
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    steps = os.environ.get('STEPS', '').split(',')
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\spartans\spartans', 'model'))
    variants = tf.SelectField('Block:variants')
    names = [str(v.SelectField('name').Data) for v in variants.Elements]
    di = names.index('default')
    if 'dup' in steps and 'noble_knife' not in names:
        variants.DuplicateElement(di)
        nv = list(variants.Elements)[-1]
        L('dup name before rename: ' + str(nv.SelectField('name').Data))
        nv.SelectField('name').Data = 'noble_knife'
        L('dup ok')
    names = [str(v.SelectField('name').Data) for v in variants.Elements]
    if 'clearregions' in steps:
        nv = list(variants.Elements)[names.index('noble_knife')]
        nv.SelectField('Block:regions').RemoveAllElements(); L('regions cleared')
    if 'dropchesta' in steps:
        nv = list(variants.Elements)[names.index('noble_knife')]
        regs = nv.SelectField('Block:regions')
        for i, r in enumerate(list(regs.Elements)):
            rn = str(r.SelectField('region name').Data)
            if rn in ('flair_chest_a', 'flair_chest_b'):
                pass
        idx = [i for i, r in enumerate(regs.Elements) if str(r.SelectField('region name').Data) in ('flair_chest_a', 'flair_chest_b')]
        for i in sorted(idx, reverse=True): regs.RemoveElement(i)
        L('dropped chest regions from noble_knife: %d' % len(idx))
    if 'onlydecals' in steps:
        nv = list(variants.Elements)[names.index('noble_knife')]
        regs = nv.SelectField('Block:regions')
        idx = [i for i, r in enumerate(regs.Elements) if str(r.SelectField('region name').Data) != 'decals']
        for i in sorted(idx, reverse=True): regs.RemoveElement(i)
        L('noble_knife regions left: ' + ', '.join(str(r.SelectField('region name').Data) for r in regs.Elements))
    if 'blankall' in steps:
        nv = list(variants.Elements)[names.index('noble_knife')]
        for r in nv.SelectField('Block:regions').Elements:
            pb = None
            for f in r.Fields:
                if str(f.FieldType) == 'Block' and 'runtime' not in str(f.FieldName): pb = f
            n = pb.Elements.Count if pb is not None else -1
            if pb is not None: pb.RemoveAllElements()
            L('  %s: removed %d permutation entries' % (str(r.SelectField('region name').Data), n))
    if 'cleardefaultobjs' in steps:
        list(variants.Elements)[di].SelectField('Block:objects').RemoveAllElements(); L('default objects cleared')
    L('variants now: ' + ', '.join(str(v.SelectField('name').Data) for v in variants.Elements))
    tf.Save(); L('SAVED'); tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:800])
open(os.path.join(T, os.environ.get('OUT', 'knife_variant.txt')), 'w').write('\n'.join(out))
