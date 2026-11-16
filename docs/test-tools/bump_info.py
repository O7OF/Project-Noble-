# Lists bump_map / material_texture bitmaps of shaders and key import settings of those bitmaps. SHADERS = "path|path"
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def bminfo(path):
    try:
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(path.replace('.bitmap', ''), 'bitmap'))
        s = []
        for f in tf.Fields:
            n = str(f.FieldName)
            if n in ('usage', 'curve mode', 'format', 'bump height', 'sharpen mode', 'compress format', 'color plate type', 'force bitmap format'):
                try: s.append('%s=%s' % (n, f.Value if not hasattr(f, 'Items') else str(f.Items[f.Value].EnumName)))
                except Exception: s.append('%s=%s' % (n, getattr(f, 'Value', '?')))
        tf.Dispose(); return ' '.join(s)
    except Exception as ex: return 'ERR ' + str(ex)[:80]
try:
    from mbstart import B, mb_stop
    for sp in os.environ['SHADERS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(sp, 'shader'))
        L('#### ' + sp)
        params = tf.SelectField('Struct:render_method[0]/Block:parameters')
        for e in params.Elements:
            name = str(e.SelectField('parameter name').Data)
            try:
                bm = e.SelectField('bitmap')
                if bm is not None and bm.Path:
                    p = str(bm.Path.RelativePathWithExtension)
                    L('  %-22s %s  :: %s' % (name, p, bminfo(p)))
            except Exception: pass
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'bump_info.txt')), 'w').write('\n'.join(out))
