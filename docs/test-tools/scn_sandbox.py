# 2026-11-13: list a scenario's top-level blocks and its sandbox (Forge palette) blocks with a few entries. Read only.
# SCN env (default levels\multi\forge_halo\forge_halo).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def val(f):
    try:
        if str(f.FieldType) == 'Reference': return str(f.Path.RelativePathWithExtension) if f.Path else ''
    except Exception: pass
    for a in ('Data', 'Value'):
        try: return str(getattr(f, a))
        except Exception: pass
    return '?'
try:
    from mbstart import B, mb_stop
    scn = os.environ.get('SCN', r'levels\multi\forge_halo\forge_halo')
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(scn, 'scenario'))
    for f in tf.Fields:
        if str(f.FieldType) == 'Block':
            L('block %-40s %d' % (f.FieldName, f.Elements.Count))
            if 'sandbox' in str(f.FieldName).lower() or 'forge' in str(f.FieldName).lower():
                for i, e in enumerate(f.Elements):
                    if i >= int(os.environ.get('N', '6')): break
                    L('   %d: %s' % (i, ' | '.join('%s=%s' % (g.FieldName, val(g)) for g in e.Fields if str(g.FieldType) not in ('Block', 'Struct'))[:300]))
                    for g in e.Fields:
                        if str(g.FieldType) == 'Struct':
                            for s in g.Elements:
                                L('      %s: %s' % (g.FieldName, ' | '.join('%s=%s' % (h.FieldName, val(h)) for h in s.Fields if str(h.FieldType) not in ('Block', 'Struct'))[:300]))
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_sandbox.txt')), 'w').write('\n'.join(out))
