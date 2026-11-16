# Print the postprocess block (template references etc.) of shaders and test whether each referenced tag loads. SHADERS = "path|path"
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def tryload(path):
    try:
        ext = os.path.splitext(path)[1].lstrip('.')
        p = os.path.splitext(path)[0]
        t = B.Tags.TagFile(); t.Load(B.Tags.TagPath.FromPathAndExtension(p, ext)); t.Dispose(); return 'OK'
    except Exception as ex: return 'MISSING/ERR ' + str(ex)[:60]
try:
    from mbstart import B, mb_stop
    for sp in os.environ['SHADERS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(sp, 'shader'))
        L('#### ' + sp)
        pp = tf.SelectField('Struct:render_method[0]/Block:postprocess')
        for i, e in enumerate(pp.Elements):
            for f in e.Fields:
                ft = str(f.FieldType); nm = str(f.FieldName)
                try:
                    if ft == 'Reference':
                        rp = str(f.Path.RelativePathWithExtension) if f.Path else ''
                        L('  [%d] %-28s ref %s  -> %s' % (i, nm, rp, tryload(rp) if rp else '(empty)'))
                    elif ft == 'Block':
                        L('  [%d] %-28s block count=%d' % (i, nm, f.Elements.Count))
                    else:
                        L('  [%d] %-28s %s' % (i, nm, str(f.Data)))
                except Exception as ex:
                    L('  [%d] %-28s ERR %s' % (i, nm, str(ex)[:50]))
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'shader_post.txt')), 'w', encoding='utf-8').write('\n'.join(out))
