# Object change colors + model/anim refs of bipeds (read only). BIPEDS="path|path" (no ext)
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    for p in [x for x in os.environ.get('BIPEDS', '').split('|') if x]:
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'biped'))
        L('== biped ' + p)
        def walk(el, ind, depth):
            for f in el.Fields:
                nm = str(f.FieldName); ft = str(f.FieldType)
                if ft == 'Reference' and nm in ('model', 'animation graph', 'collision model', 'physics model'):
                    try: L('  ' * ind + '%s -> %s' % (nm, f.Path.RelativePathWithExtension if f.Path else '')) 
                    except Exception: pass
                if ft == 'Block' and 'change color' in nm:
                    L('  ' * ind + '%s: %d' % (nm, f.Elements.Count))
                if ft == 'Struct' and depth < 3:
                    for e in f.Elements: walk(e, ind, depth + 1)
        walk(tf, 1, 0)
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1200])
open(os.path.join(T, os.environ.get('OUT', 'obj_cc.txt')), 'w').write('\n'.join(out))
