# Print (and optionally set) the function type of a shader parameter's animation function.
# SHADERS="a|b", PARAM=self_illum_color, SET_FROM=<stock shader path> copies MasterType/ColorGraphType/colours setup from stock
# then sets colours C0/C1 ("r,g,b"). DRY=1 only prints.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def info(fv):
    s = []
    for n in ('MasterType', 'ColorGraphType', 'ColorCount', 'IsColorLocked', 'IsTypeLocked', 'IsRanged', 'CustomType'):
        try: s.append('%s=%s' % (n, getattr(fv, n)))
        except Exception as ex: s.append('%s=?' % n)
    try: s.append('possibleColors=%s' % fv.GetPossibleColorCount())
    except Exception: pass
    return ' '.join(s)
try:
    from mbstart import B, mb_stop
    pname = os.environ.get('PARAM', 'self_illum_color')
    for sp in os.environ['SHADERS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(sp, 'shader'))
        for e in tf.SelectField('Struct:render_method[0]/Block:parameters').Elements:
            if str(e.SelectField('parameter name').Data) != pname: continue
            for sub in e.SelectField('animated parameters').Elements:
                fv = sub.SelectField('animation function').Value
                L('%s: %s' % (sp.split('\\')[-1], info(fv)))
                for i in range(2):
                    try:
                        c = fv.GetColor(i); L('  color%d (%.3f,%.3f,%.3f)' % (i, c.Red, c.Green, c.Blue))
                    except Exception as ex: L('  color%d err %s' % (i, str(ex)[:80]))
                if os.environ.get('SETCGT') and not os.environ.get('DRY'):
                    try:
                        L('  SetPossibleColorCount sig: ' + str(fv.SetPossibleColorCount.__doc__)[:200])
                        fv.SetPossibleColorCount(tf, int(os.environ['SETCGT']))
                    except Exception as ex:
                        L('  SetPossibleColorCount err ' + str(ex)[:300])
                    for i, key in ((0, 'C0'), (1, 'C1')):
                        if os.environ.get(key):
                            c = fv.GetColor(i); c.Red, c.Green, c.Blue = [float(x) for x in os.environ[key].split(',')]; fv.SetColor(i, c)
                    L('  after: ' + info(fv))
                    for i in range(2):
                        c = fv.GetColor(i); L('  color%d (%.3f,%.3f,%.3f)' % (i, c.Red, c.Green, c.Blue))
                    try:
                        L('  CGT enum values: ' + ', '.join('%s=%d' % (n, int(v)) for n, v in zip(type(fv.ColorGraphType).GetNames(type(fv.ColorGraphType)), type(fv.ColorGraphType).GetValues(type(fv.ColorGraphType)))))
                    except Exception: pass
        if os.environ.get('SETCGT') and not os.environ.get('DRY'): tf.Save(); L('SAVED ' + sp)
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'func_type.txt')), 'w').write('\n'.join(out))
