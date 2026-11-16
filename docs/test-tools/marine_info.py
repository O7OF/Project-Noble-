# Marine facts (read only): model variants (regions/perms per variant + objects), biped weapon/unit fields,
# change colour blocks of the biped (names/functions).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def val(f):
    for a in ('Data', 'Value'):
        try:
            x = getattr(f, a)
            try: return ','.join('%.3f' % y for y in x)
            except Exception: return str(x)
        except Exception: pass
    try: return str(f.Path.RelativePathWithExtension) if f.Path else ''
    except Exception: return '?'
def dump(el, ind, depth, maxd=4, only=None):
    for f in el.Fields:
        nm = str(f.FieldName); ft = str(f.FieldType)
        if ft == 'Block':
            L('  ' * ind + '%s [block %d]' % (nm, f.Elements.Count))
            if depth < maxd and (only is None or any(o in nm for o in only)):
                for i, e in enumerate(list(f.Elements)[:20]):
                    L('  ' * (ind + 1) + '#%d' % i); dump(e, ind + 2, depth + 1, maxd, None)
        elif ft == 'Struct':
            for e in f.Elements: dump(e, ind, depth + 1, maxd, only)
        elif ft in ('Reference', 'StringId', 'String', 'LongEnum', 'ShortEnum', 'CharEnum', 'ShortBlockIndex', 'LongBlockIndex', 'CharBlockIndex', 'RealRgbColor', 'ShortInteger', 'LongInteger', 'Real'):
            L('  ' * ind + '%s = %s' % (nm, val(f)))
try:
    from mbstart import B, mb_stop
    mode = os.environ.get('WHAT', 'model')
    if mode == 'model':
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\marine\marine', 'model'))
        vs = tf.SelectField('Block:variants')
        for v in list(vs.Elements)[:3]:
            L('== variant %s' % v.SelectField('name').Data); dump(v, 1, 0, 3)
    else:
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\marine\marine', 'biped'))
        dump(tf, 0, 0, 3, only=['change color', 'weapon', 'initial'])
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'marine_info.txt')), 'w').write('\n'.join(out))
