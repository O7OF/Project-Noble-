# Print full bitmap paths of every parameter of shaders and whether the tag file exists. SHADERS="a|b"
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
HT = r"D:\Steam\steamapps\common\HREK\tags"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for sp in os.environ['SHADERS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(sp, 'shader'))
        L('#### ' + sp)
        for e in tf.SelectField('Struct:render_method[0]/Block:parameters').Elements:
            try:
                bm = e.SelectField('bitmap')
                if bm is not None and bm.Path:
                    p = str(bm.Path.RelativePathWithExtension)
                    L('  %-34s %s %s' % (str(e.SelectField('parameter name').Data), p, 'OK' if os.path.exists(os.path.join(HT, p)) else 'MISSING'))
            except Exception as ex:
                pass
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'shader_refs.txt')), 'w').write('\n'.join(out))
