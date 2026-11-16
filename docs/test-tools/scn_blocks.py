# 2026-11-14: Elite pack shrink. Lists a scenario's non-empty top-level blocks with their references (2 levels).
# MODE=clear CLEAR=a|b|c empties those blocks (by field name) and saves. SCN env (default levels\test\noble_pack\noble_pack).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def refs(el, depth, acc):
    for f in el.Fields:
        ft = str(f.FieldType)
        if ft == 'Reference':
            try:
                if f.Path: acc.add(str(f.Path.RelativePathWithExtension))
            except Exception: pass
        elif ft == 'Block' and depth < 3:
            for e in f.Elements: refs(e, depth + 1, acc)
        elif ft == 'Struct':
            for s in f.Elements: refs(s, depth, acc)
try:
    from mbstart import B, mb_stop
    scn = os.environ.get('SCN', r'levels\test\noble_pack\noble_pack')
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(scn, 'scenario'))
    clear = [c for c in os.environ.get('CLEAR', '').split('|') if c]
    for f in tf.Fields:
        if str(f.FieldType) != 'Block' or f.Elements.Count == 0: continue
        name = str(f.FieldName)
        if os.environ.get('MODE') == 'clear' and name in clear:
            f.RemoveAllElements(); L('cleared %s' % name); continue
        acc = set()
        for e in f.Elements: refs(e, 0, acc)
        L('[%s] %d  %s' % (name, f.Elements.Count, '; '.join(sorted(acc))[:400]))
    if os.environ.get('MODE') == 'clear':
        tf.Save(); L('saved')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:2000])
open(os.path.join(T, os.environ.get('OUT', 'scn_blocks.txt')), 'w').write('\n'.join(out))
