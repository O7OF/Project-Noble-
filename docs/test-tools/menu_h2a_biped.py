# 2026-11-13: menu-only H2A Marine biped (objects\characters\h2a_marines\noble_menu_h2a.biped, a file copy of
# h2a_marines.biped made beforehand) with NO starting weapon; point the menu bipeds resource palette 2 at BIPED
# (env BIPED, default the H2A one; BIPED=objects\characters\marine\noble_menu_marine switches back to the Reach marine).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    biped = os.environ.get('BIPED', r'objects\characters\h2a_marines\noble_menu_h2a')
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(biped, 'biped'))
    def clear(el, depth):
        for f in el.Fields:
            if str(f.FieldType) == 'Block' and str(f.FieldName) == 'weapons':
                L('weapons block had %d' % f.Elements.Count); f.RemoveAllElements()
            elif str(f.FieldType) == 'Struct' and depth < 3:
                for s in f.Elements: clear(s, depth + 1)
    clear(tf, 0)
    tf.Save(); tf.Dispose(); L('biped saved ' + biped)
    rf = B.Tags.TagFile(); rf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\ui\mainmenu\resources\mainmenu', 'scenario_bipeds_resource'))
    pe = list(rf.SelectField('Block:biped palette').Elements)[2]
    L('palette 2 was ' + str(pe.SelectField('name').Path.RelativePathWithExtension))
    pe.SelectField('name').Path = B.Tags.TagPath.FromPathAndExtension(biped, 'biped')
    L('palette 2 -> ' + str(pe.SelectField('name').Path.RelativePathWithExtension))
    rf.Save(); rf.Dispose(); L('resource saved')
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'menu_h2a_biped.txt')), 'w').write('\n'.join(out))
