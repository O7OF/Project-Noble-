# Print categories + option names of shaders\shader.render_method_definition, and the elite_armor_template option indices.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'shaders\shader', 'render_method_definition'))
    cats = tf.SelectField('Block:categories')
    for i, c in enumerate(cats.Elements):
        nm = ''
        for f in c.Fields:
            if str(f.FieldName) == 'category name':
                nm = str(f.Data); break
        opts = []
        for f in c.Fields:
            if str(f.FieldName) == 'options' or str(f.FieldName) == 'shader options':
                for j, o in enumerate(f.Elements):
                    on = ''
                    for g in o.Fields:
                        if str(g.FieldName) == 'option name':
                            on = str(g.Data); break
                    opts.append('%d=%s' % (j, on))
        L('%2d %-28s %s' % (i, nm, ' '.join(opts)))
    # template options
    t2 = B.Tags.TagFile(); t2.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\elite\shaders\elite_armor_template', 'shader'))
    rm = list(t2.SelectField('Struct:render_method[0]').Elements)[0]
    for f in rm.Fields:
        if str(f.FieldName) == 'options':
            L('template options: ' + ' '.join(str(list(e.Fields)[0].Data) for e in f.Elements))
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'rmdef.txt')), 'w').write('\n'.join(out))
