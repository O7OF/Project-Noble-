# Set one shader option (category index -> option value) and optionally the postprocess shader template reference.
# env: SHADER (tag path w/o ext), CAT (int), VAL (int), TEMPLATE (optional template tag path without extension), DRY=1 to only print
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(os.environ['SHADER'], 'shader'))
    rm = list(tf.SelectField('Struct:render_method[0]').Elements)[0]
    changed = False
    for f in rm.Fields:
        if str(f.FieldName) == 'options':
            els = list(f.Elements)
            L('options before: ' + ' '.join(str(list(e.Fields)[0].Data) for e in els))
            if 'CAT' in os.environ:
                fld = list(els[int(os.environ['CAT'])].Fields)[0]
                fld.Data = int(os.environ['VAL']); changed = True
            L('options after : ' + ' '.join(str(list(e.Fields)[0].Data) for e in els))
    if os.environ.get('TEMPLATE'):
        pp = tf.SelectField('Struct:render_method[0]/Block:postprocess')
        for e in pp.Elements:
            for f in e.Fields:
                if str(f.FieldName) == 'shader template':
                    L('template before: ' + (str(f.Path.RelativePathWithExtension) if f.Path else ''))
                    f.Path = B.Tags.TagPath.FromPathAndExtension(os.environ['TEMPLATE'], 'render_method_template')
                    L('template after : ' + str(f.Path.RelativePathWithExtension)); changed = True
    if changed and os.environ.get('DRY') != '1':
        tf.Save(); L('SAVED')
    else:
        L('not saved (dry or unchanged)')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'set_shader_option.txt')), 'w').write('\n'.join(out))
