# 2026-11-13: list object attachments + object functions of bipeds (BIPEDS="path|path"; read only).
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
def find_block(el, name, depth=0):
    for f in el.Fields:
        if str(f.FieldType) == 'Block' and str(f.FieldName) == name: return f
        if str(f.FieldType) == 'Struct' and depth < 3:
            for s in f.Elements:
                r = find_block(s, name, depth + 1)
                if r is not None: return r
    return None
try:
    from mbstart import B, mb_stop
    for p in [x for x in os.environ.get('BIPEDS', '').split('|') if x]:
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(p, 'biped'))
        L('==== ' + p)
        for bn in ('attachments', 'functions', 'change colors', 'new hud interfaces', 'seats'):
            blk = find_block(tf, bn)
            if blk is None: L('  (no %s)' % bn); continue
            L('  %s: %d' % (bn, blk.Elements.Count))
            if bn in ('attachments', 'functions'):
                for i, e in enumerate(blk.Elements):
                    L('    %d: %s' % (i, ' | '.join('%s=%s' % (f.FieldName, val(f)) for f in e.Fields if str(f.FieldType) not in ('Block', 'Struct', 'Data'))[:400]))
        tf.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'biped_attach.txt')), 'w').write('\n'.join(out))
