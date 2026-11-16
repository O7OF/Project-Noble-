# Print each shader parameter's real value and animated-function values. SHADERS = "path|path" (tag paths without ext, relative to tags).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    first = True
    for sp in os.environ['SHADERS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(sp, 'shader'))
        L('#### ' + sp)
        params = tf.SelectField('Struct:render_method[0]/Block:parameters')
        for e in params.Elements:
            name = str(e.SelectField('parameter name').Data)
            if name in ('bump_map', 'material_texture', 'self_illum_detail_map'): continue
            ptype = str(e.SelectField('parameter type').Value)
            line = '  %-36s' % name
            try:
                bm = e.SelectField('bitmap')
                if bm is not None and bm.Path: line += ' bitmap=' + os.path.basename(str(bm.Path.RelativePathWithExtension))
            except Exception: pass
            try: line += ' real=%g' % e.SelectField('real').Data
            except Exception: pass
            anim = e.SelectField('animated parameters')
            for sub in anim.Elements:
                try:
                    ty = str(sub.SelectField('type').Value)
                    fv = sub.SelectField('animation function').Value
                    if first:
                        L('  [function value members] ' + ','.join(x for x in dir(fv) if not x.startswith('_'))); first = False
                    cols = []
                    for i in range(2):
                        try:
                            c = fv.GetColor(i); cols.append('c%d=(%.3f,%.3f,%.3f)' % (i, c.Red, c.Green, c.Blue))
                        except Exception as ex: cols.append('c%d?%s' % (i, str(ex)[:30]))
                    try: mm = 'min/max=%g/%g' % (fv.GetMin(), fv.GetMax())
                    except Exception: mm = ''
                    try: ty2 = 'funcType=' + str(fv.FunctionType)
                    except Exception: ty2 = ''
                    line += ' | [%s] %s %s %s' % (ty, ' '.join(cols), mm, ty2)
                except Exception as ex:
                    line += ' | anim?' + str(ex)[:40]
            L(line)
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'shader_vals.txt')), 'w', encoding='utf-8').write('\n'.join(out))
