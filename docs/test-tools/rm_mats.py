# Lists materials of elite_noble.render_model whose shader name matches FILTER (comma list), with region/permutation usage.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\elite_noble\elite_noble', 'render_model'))
    mats = tf.SelectField('Block:materials')
    flt = os.environ.get('FILTER', 'honor').split(',')
    L('materials: %d' % mats.Elements.Count)
    for i, e in enumerate(mats.Elements):
        p = ''
        for f in e.Fields:
            if str(f.FieldType) == 'Reference':
                try: p = str(f.Path.RelativePathWithExtension) if f.Path else ''
                except Exception: pass
                break
        if i >= 90 or any(x in p for x in flt):
            L('%3d %s' % (i, p))
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'rm_mats.txt')), 'w').write('\n'.join(out))
