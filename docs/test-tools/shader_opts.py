# Print shader template reference + option indices + all non-bitmap field summary. SHADERS = "path|path"
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for sp in os.environ['SHADERS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(sp, 'shader'))
        L('#### ' + sp)
        rm = tf.SelectField('Struct:render_method[0]')
        L('  struct members: ' + ','.join(x for x in dir(rm) if not x.startswith('_') and not x.startswith('get_') and not x.startswith('set_')))
        flds = list(rm.Elements)[0].Fields
        for f in flds:
            ft = str(f.FieldType); nm = str(f.FieldName)
            try:
                if ft == 'Reference':
                    L('  %-30s ref %s' % (nm, str(f.Path.RelativePathWithExtension) if f.Path else ''))
                elif ft == 'Block':
                    if nm in ('options',):
                        vals = []
                        for e in f.Elements:
                            try: vals.append(str(e.Fields[0].Data))
                            except Exception: vals.append('?')
                        L('  options: ' + ' '.join(vals))
                    else:
                        L('  %-30s block count=%d' % (nm, f.Elements.Count))
                else:
                    try: L('  %-30s %s' % (nm, str(f.Data)))
                    except Exception: L('  %-30s %s' % (nm, ft))
            except Exception as ex:
                L('  %-30s ERR %s' % (nm, str(ex)[:60]))
        # top-level fields of the shader itself
        for f in tf.Fields if hasattr(tf, 'Fields') else []:
            try: L('  [top] %s = %s' % (f.FieldName, f.Data))
            except Exception: pass
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'shader_opts.txt')), 'w').write('\n'.join(out))
