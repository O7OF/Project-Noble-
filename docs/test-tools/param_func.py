# Dump every field of the animated parameters of shader parameters. SHADERS="a|b", ONLY="self_illum_color,..."
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def dump(el, ind):
    for f in el.Fields:
        ft = str(f.FieldType); nm = str(f.FieldName)
        if ft in ('Block', 'Struct', 'Array'):
            try:
                els = list(f.Elements)
            except Exception:
                els = []
            L('%s%s [%s] n=%d' % (ind, nm, ft, len(els)))
            for i, e in enumerate(els): L('%s [%d]' % (ind, i)); dump(e, ind + '    ')
        else:
            v = ''
            try: v = str(f.Data)
            except Exception:
                try: v = str(f.Value)
                except Exception: v = '?'
            if ft == 'Data':
                try: v = ' '.join('%02x' % b for b in list(f.GetData())[:48])
                except Exception: pass
            L('%s%s (%s) = %s' % (ind, nm, ft, v[:200]))
try:
    from mbstart import B, mb_stop
    for sp in os.environ['SHADERS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(sp, 'shader'))
        L('#### ' + sp)
        params = tf.SelectField('Struct:render_method[0]/Block:parameters')
        for e in params.Elements:
            name = str(e.SelectField('parameter name').Data)
            if os.environ.get('ONLY') and name not in os.environ['ONLY'].split(','): continue
            L('== ' + name); dump(e, '  ')
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'param_func.txt')), 'w').write('\n'.join(out))
