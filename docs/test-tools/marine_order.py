# 2026-11-13: region order of objects\characters\marine: render model regions vs model variant regions (read only).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    rm = B.Tags.TagFile(); rm.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\marine\marine', 'render_model'))
    L('render_model regions: ' + ', '.join('%d=%s' % (i, r.SelectField('name').Data) for i, r in enumerate(rm.SelectField('Block:regions').Elements)))
    rm.Dispose()
    md = B.Tags.TagFile(); md.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\marine\marine', 'model'))
    for v in md.SelectField('Block:variants').Elements:
        regs = []
        for r in v.SelectField('Block:regions').Elements:
            nm = str(r.SelectField('region name').Data)
            perms = [str(p.SelectField('permutation name').Data) for p in r.SelectField('Block:permutations').Elements]
            regs.append('%s[%s]' % (nm, '/'.join(perms)))
        L('variant %s: %s' % (v.SelectField('name').Data, '; '.join(regs)))
    for f in md.Fields:
        if str(f.FieldType) == 'Block': L('model block %s: %d' % (f.FieldName, f.Elements.Count))
    md.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'marine_order.txt')), 'w').write('\n'.join(out))
