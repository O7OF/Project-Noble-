# Set one bitmap parameter of a shader: SHADER (tag path w/o ext), PARAM (parameter name), BITMAP (tag path w/o ext).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(os.environ['SHADER'], 'shader'))
    params = tf.SelectField('Struct:render_method[0]/Block:parameters')
    done = False
    for e in params.Elements:
        if str(e.SelectField('parameter name').Data) == os.environ['PARAM']:
            f = e.SelectField('bitmap')
            L('before: ' + str(f.Path.RelativePathWithExtension if f.Path else ''))
            f.Path = B.Tags.TagPath.FromPathAndExtension(os.environ['BITMAP'], 'bitmap')
            L('after: ' + str(f.Path.RelativePathWithExtension)); done = True
    if done: tf.Save(); L('SAVED')
    else: L('parameter not found')
    tf.Dispose(); mb_stop()
except Exception:
    L("ERROR\n" + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'set_shader_bitmap.txt')), 'w').write("\n".join(out))
