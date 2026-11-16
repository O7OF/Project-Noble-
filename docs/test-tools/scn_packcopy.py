# 2026-11-14: Elite pack shrink test. Copies levels\test\box\box.scenario to levels\test\noble_pack\noble_pack.scenario
# (new tag, box untouched) and sets its type to multiplayer so the build pulls in the multiplayer globals' Elite content.
# MODE=read only reports box's type fields. Writes OUT (default scn_packcopy.txt).
import os, shutil, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
H = r"D:\Steam\steamapps\common\HREK\tags"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    mode = os.environ.get('MODE', 'read')
    src = os.path.join(H, r'levels\test\box\box.scenario')
    dst_dir = os.path.join(H, r'levels\test\noble_pack')
    dst = os.path.join(dst_dir, 'noble_pack.scenario')
    scn = os.environ.get('SCN', r'levels\test\box\box')
    if mode == 'write':
        os.makedirs(dst_dir, exist_ok=True)
        shutil.copyfile(src, dst)
        L('copied box.scenario -> ' + dst)
        scn = r'levels\test\noble_pack\noble_pack'
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(scn, 'scenario'))
    for f in tf.Fields:
        ft = str(f.FieldType)
        if ft in ('Block', 'Struct', 'Pad', 'Skip', 'Explanation', 'Custom', 'UselessPad'): continue
        n = str(f.FieldName)
        if any(k in n.lower() for k in ('type', 'flags', 'map id', 'campaign')):
            try:
                if ft == 'ShortEnum' or ft == 'CharEnum' or ft == 'LongEnum':
                    L('%s (%s) = %s  options: %s' % (n, ft, f.Value, ', '.join(str(o) for o in f.Items)))
                else:
                    v = (str(f.Path.RelativePathWithExtension) if f.Path else '') if ft == 'Reference' else (f.Data if hasattr(f, 'Data') else getattr(f, 'Value', '?'))
                    L('%s (%s) = %s' % (n, ft, v))
            except Exception as e:
                L('%s (%s) ?: %s' % (n, ft, e))
    if mode == 'write':
        f = tf.SelectField('CharEnum:type')
        before = f.Value
        f.Value = int(os.environ.get('TYPE', '1'))
        L('type %s -> %s' % (before, f.Value))
        tf.Save()
        L('saved')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:2000])
open(os.path.join(T, os.environ.get('OUT', 'scn_packcopy.txt')), 'w').write('\n'.join(out))
