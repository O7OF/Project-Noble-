# mainmenu.scenario_bipeds_resource: list top-level blocks, names, palette, placements. WRITE=1 adds the Marine
# (palette objects\characters\marine\marine, object name marine_armory, copy of chief_armory's placement, new unique id).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def v(f):
    for a in ('Data', 'Value'):
        try:
            x = getattr(f, a)
            try: return ','.join('%.3f' % y for y in x)
            except Exception: return str(x)
        except Exception: pass
    return '?'
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\resources\mainmenu', 'scenario_bipeds_resource'))
    for f in tf.Fields:
        cnt = f.Elements.Count if str(f.FieldType) == 'Block' else ''
        L('field %s [%s] %s' % (f.FieldName, f.FieldType, cnt))
    names = tf.SelectField('Block:names')
    nl = [str(e.SelectField('name').Data) for e in names.Elements]
    L('names: ' + ', '.join('%d=%s' % (i, n) for i, n in enumerate(nl)))
    pal = tf.SelectField('Block:biped palette')
    for i, e in enumerate(pal.Elements):
        r = e.SelectField('name'); L('palette %d: %s' % (i, r.Path.RelativePathWithExtension if r.Path else ''))
    objs = tf.SelectField('Block:bipeds')
    for i, e in enumerate(objs.Elements):
        L('object %d: type=%s name=%s' % (i, v(e.SelectField('type')), v(e.SelectField('name'))))
    if os.environ.get('WRITE') and 'marine_armory' not in nl:
        pal.AddElement(); pe = list(pal.Elements)[-1]
        pe.SelectField('name').Path = B.Tags.TagPath.FromPathAndExtension(r'objects\characters\marine\marine', 'biped')
        palIdx = pal.Elements.Count - 1
        ci = nl.index('chief_armory')
        names.DuplicateElement(ci); ne = list(names.Elements)[-1]; ne.SelectField('name').Data = 'marine_armory'
        nameIdx = names.Elements.Count - 1
        src = [i for i, e in enumerate(objs.Elements) if int(v(e.SelectField('name'))) == ci][0]
        objs.DuplicateElement(src); me = list(objs.Elements)[-1]
        me.SelectField('type').Value = palIdx; me.SelectField('name').Value = nameIdx
        od = list(me.SelectField('Struct:object data').Elements)[0]
        oid = list(od.SelectField('Struct:object id').Elements)[0]
        uids = []
        for e in objs.Elements:
            o2 = list(list(e.SelectField('Struct:object data').Elements)[0].SelectField('Struct:object id').Elements)[0]
            uids.append(int(v(o2.SelectField('unique id'))))
        oid.SelectField('unique id').Data = max(uids) + 1
        L('added marine: palette %d name %d object %d uid %d' % (palIdx, nameIdx, objs.Elements.Count - 1, max(uids) + 1))
        tf.Save(); L('SAVED')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'res_bipeds.txt')), 'w').write('\n'.join(out))
