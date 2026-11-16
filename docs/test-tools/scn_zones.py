# Designer zones: which biped palette entries each zone holds. WRITE=1 adds biped palette 2 (marine) wherever palette 1 (spartans) is.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\mainmenu', 'scenario'))
    dz = tf.SelectField('Block:designer zones')
    L('designer zones: %d' % dz.Elements.Count)
    changed = False
    for z in dz.Elements:
        bl = z.SelectField('Block:biped')
        vals = []
        for e in bl.Elements:
            f = list(e.Fields)[0]
            vals.append(int(str(f.Value)))
        L('zone %s: bipeds %s' % (z.SelectField('name').Data, vals))
        if os.environ.get('WRITE') and 1 in vals and 2 not in vals:
            bl.AddElement(); list(list(bl.Elements)[-1].Fields)[0].Value = 2; changed = True
            L('  + marine palette 2')
    if changed: tf.Save(); L('SAVED')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_zones.txt')), 'w').write('\n'.join(out))
