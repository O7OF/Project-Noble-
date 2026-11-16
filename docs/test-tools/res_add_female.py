# 2026-11-13: female Marine on the Armory pedestal. objects\characters\marine_female\noble_menu_marine_female.biped
# (file copy of marine_female.biped made beforehand) gets its weapons emptied; mainmenu bipeds resource gets a palette
# entry for it, object name "marine_f_armory" and a placement copied from chief_armory's (new unique id). Idempotent.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
BIPED = r'objects\characters\marine_female\noble_menu_marine_female'
def v(f):
    for a in ('Data', 'Value'):
        try: return str(getattr(f, a))
        except Exception: pass
    return '?'
def uid_field(e):
    od = list(e.SelectField('Struct:object data').Elements)[0]
    oid = list(od.SelectField('Struct:object id').Elements)[0]
    return oid.SelectField('unique id')
try:
    from mbstart import B, mb_stop
    bf = B.Tags.TagFile(); bf.Load(B.Tags.TagPath.FromPathAndExtension(BIPED, 'biped'))
    def clear(el, depth):
        for f in el.Fields:
            if str(f.FieldType) == 'Block' and str(f.FieldName) == 'weapons':
                L('weapons block had %d' % f.Elements.Count); f.RemoveAllElements()
            elif str(f.FieldType) == 'Struct' and depth < 3:
                for s in f.Elements: clear(s, depth + 1)
    clear(bf, 0); bf.Save(); bf.Dispose(); L('biped saved')
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\resources\mainmenu', 'scenario_bipeds_resource'))
    pal = tf.SelectField('Block:biped palette'); names = tf.SelectField('Block:names'); objs = tf.SelectField('Block:bipeds')
    nl = [str(e.SelectField('name').Data) for e in names.Elements]
    if 'marine_f_armory' in nl:
        L('marine_f_armory already there')
    else:
        pal.AddElement(); pe = list(pal.Elements)[-1]
        pe.SelectField('name').Path = B.Tags.TagPath.FromPathAndExtension(BIPED, 'biped')
        palIdx = pal.Elements.Count - 1
        ci = nl.index('chief_armory')
        names.DuplicateElement(ci); ne = list(names.Elements)[-1]; ne.SelectField('name').Data = 'marine_f_armory'
        nameIdx = names.Elements.Count - 1
        src = [i for i, e in enumerate(objs.Elements) if int(v(e.SelectField('name'))) == ci][0]
        uids = [int(v(uid_field(e))) for e in objs.Elements]
        objs.DuplicateElement(src); me = list(objs.Elements)[-1]
        me.SelectField('type').Value = palIdx; me.SelectField('name').Value = nameIdx
        uid_field(me).Data = max(uids) + 1
        L('added: palette %d name %d object %d uid %d' % (palIdx, nameIdx, objs.Elements.Count - 1, max(uids) + 1))
        tf.Save(); L('SAVED')
    for i, e in enumerate(pal.Elements): L('palette %d: %s' % (i, e.SelectField('name').Path.RelativePathWithExtension))
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'res_add_female.txt')), 'w').write('\n'.join(out))
