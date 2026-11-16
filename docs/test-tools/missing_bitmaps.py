# List bitmap parameters of every shader under the given tag folders that point at a bitmap tag that does not exist.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
TAGS = r"D:\Steam\steamapps\common\HREK\tags"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for folder in os.environ['DIRS'].split('|'):
        for root, _, files in os.walk(os.path.join(TAGS, folder)):
            for f in files:
                if not f.endswith('.shader'): continue
                rel = os.path.relpath(os.path.join(root, f[:-7]), TAGS)
                tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(rel, 'shader'))
                for e in tf.SelectField('Struct:render_method[0]/Block:parameters').Elements:
                    try:
                        bm = e.SelectField('bitmap')
                        if bm is None or not bm.Path: continue
                        p = str(bm.Path.RelativePathWithExtension)
                        if p and not os.path.exists(os.path.join(TAGS, p)):
                            L('%s | %s | %s' % (rel, e.SelectField('parameter name').Data, p))
                    except Exception: pass
                tf.Dispose()
    L('DONE')
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'missing_bitmaps.txt')), 'w').write('\n'.join(out))
