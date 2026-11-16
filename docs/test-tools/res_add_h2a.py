# 2026-11-13: mainmenu.scenario_bipeds_resource: palette 2 back to the Reach menu marine; add palette entry
# objects\characters\h2a_marines\noble_menu_h2a.biped, object name "h2a_armory", placement = copy of chief_armory's
# (same pedestal) with a new unique id. Idempotent.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
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
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\resources\mainmenu', 'scenario_bipeds_resource'))
    pal = tf.SelectField('Block:biped palette')
    p2 = list(pal.Elements)[2]
    p2.SelectField('name').Path = B.Tags.TagPath.FromPathAndExtension(r'objects\characters\marine\noble_menu_marine', 'biped')
    L('palette 2 -> ' + str(p2.SelectField('name').Path.RelativePathWithExtension))
    names = tf.SelectField('Block:names')
    nl = [str(e.SelectField('name').Data) for e in names.Elements]
    objs = tf.SelectField('Block:bipeds')
    if 'h2a_armory' not in nl:
        pal.AddElement(); pe = list(pal.Elements)[-1]
        pe.SelectField('name').Path = B.Tags.TagPath.FromPathAndExtension(r'objects\characters\h2a_marines\noble_menu_h2a', 'biped')
        palIdx = pal.Elements.Count - 1
        ci = nl.index('chief_armory')
        names.DuplicateElement(ci); ne = list(names.Elements)[-1]; ne.SelectField('name').Data = 'h2a_armory'
        nameIdx = names.Elements.Count - 1
        src = [i for i, e in enumerate(objs.Elements) if int(v(e.SelectField('name'))) == ci][0]
        uids = [int(v(uid_field(e))) for e in objs.Elements]
        objs.DuplicateElement(src); me = list(objs.Elements)[-1]
        me.SelectField('type').Value = palIdx; me.SelectField('name').Value = nameIdx
        uid_field(me).Data = max(uids) + 1
        L('added h2a: palette %d name %d object %d uid %d (uids before: %s)' % (palIdx, nameIdx, objs.Elements.Count - 1, max(uids) + 1, uids))
    else:
        L('h2a_armory already there')
    for i, e in enumerate(pal.Elements):
        L('palette %d: %s' % (i, e.SelectField('name').Path.RelativePathWithExtension))
    tf.Save(); L('SAVED'); tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'res_add_h2a.txt')), 'w').write('\n'.join(out))
