# Set self_illum_color function colours of shaders. SHADERS="path|path", C0="r,g,b", C1="r,g,b" (0..1, linear as stored), DRY=1 to only print.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    c0v = [float(x) for x in os.environ['C0'].split(',')]
    c1v = [float(x) for x in os.environ.get('C1', os.environ['C0']).split(',')]
    for sp in os.environ['SHADERS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(sp, 'shader'))
        params = tf.SelectField('Struct:render_method[0]/Block:parameters')
        for e in params.Elements:
            if str(e.SelectField('parameter name').Data) != 'self_illum_color': continue
            for sub in e.SelectField('animated parameters').Elements:
                fv = sub.SelectField('animation function').Value
                for i, vals in ((0, c0v), (1, c1v)):
                    c = fv.GetColor(i)
                    L('%s color%d before (%.3f,%.3f,%.3f) type %s' % (sp.split('\\')[-1], i, c.Red, c.Green, c.Blue, type(c).__name__))
                    if os.environ.get('DRY'): 
                        L('  members: ' + ','.join(x for x in dir(c) if not x.startswith('_'))[:400]); continue
                    try:
                        c.Red, c.Green, c.Blue = vals
                    except Exception as ex:
                        L('  in-place set failed: ' + str(ex)[:80])
                    fv.SetColor(i, c)
                    c2 = fv.GetColor(i)
                    L('  after (%.3f,%.3f,%.3f)' % (c2.Red, c2.Green, c2.Blue))
        if not os.environ.get('DRY'): tf.Save(); L('SAVED ' + sp)
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'set_illum_color.txt')), 'w').write('\n'.join(out))
