# 2026-11-13: add the Marine to the menu scenario: biped palette objects\characters\marine\marine, object name
# "marine_armory", placement = copy of chief_armory's (same pedestal, same cameras). Idempotent.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\mainmenu', 'scenario'))
    names = tf.SelectField('Block:object names')
    nlist = [str(e.SelectField('name').Data) for e in names.Elements]
    if 'marine_armory' in nlist:
        L('already there'); tf.Dispose(); mb_stop()
    else:
        pal = tf.SelectField('Block:biped palette')
        pal.AddElement()
        pe = list(pal.Elements)[-1]
        pe.SelectField('name').Path = B.Tags.TagPath.FromPathAndExtension(r'objects\characters\marine\marine', 'biped')
        palIdx = pal.Elements.Count - 1
        L('palette index %d -> %s' % (palIdx, pe.SelectField('name').Path.RelativePathWithExtension))
        ci = nlist.index('chief_armory')
        names.DuplicateElement(ci)
        ne = list(names.Elements)[-1]
        ne.SelectField('name').Data = 'marine_armory'
        nameIdx = names.Elements.Count - 1
        L('object name index %d' % nameIdx)
        bp = tf.SelectField('Block:bipeds')
        src = None
        for i, e in enumerate(bp.Elements):
            if int(str(e.SelectField('name').Value)) == ci: src = i
        L('chief_armory placement %s' % src)
        bp.DuplicateElement(src)
        me = list(bp.Elements)[-1]
        me.SelectField('type').Value = palIdx
        me.SelectField('name').Value = nameIdx
        L('marine placement type=%s name=%s' % (me.SelectField('type').Value, me.SelectField('name').Value))
        tf.Save(); L('SAVED')
        tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_add_marine.txt')), 'w').write('\n'.join(out))
