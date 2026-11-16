# List categories and option names of shaders\shader.render_method_definition (CAT = only that category index).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'shaders\shader', 'render_method_definition'))
    cats = tf.SelectField('Block:categories')
    for ci, c in enumerate(cats.Elements):
        if os.environ.get('CAT') and str(ci) != os.environ['CAT']: continue
        names = []
        cname = ''
        for f in c.Fields:
            if 'name' in str(f.FieldName).lower() and not cname:
                try: cname = str(f.Data)
                except Exception: pass
        opts = c.SelectField('Block:options') if True else None
        for oi, o in enumerate(opts.Elements):
            on = ''
            for f in o.Fields:
                if 'name' in str(f.FieldName).lower():
                    try: on = str(f.Data); break
                    except Exception: pass
            names.append('%d=%s' % (oi, on))
        L('%d %s: %s' % (ci, cname, ', '.join(names)))
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'rmdef_opts.txt')), 'w').write('\n'.join(out))
