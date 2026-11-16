# List marker groups matching NAME in a render_model with region/permutation of each marker. RM=tag path (no ext), NAME=knife
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(os.environ['RM'], 'render_model'))
    regions = []
    for r in tf.SelectField('Block:regions').Elements:
        rn = str(r.SelectField('name').Data)
        perms = [str(p.SelectField('name').Data) for p in r.SelectField('Block:permutations').Elements]
        regions.append((rn, perms))
    want = os.environ.get('NAME', 'knife')
    for g in tf.SelectField('Block:marker groups').Elements:
        gn = str(g.SelectField('name').Data)
        if want not in gn: continue
        L('group ' + gn)
        for m in g.SelectField('Block:markers').Elements:
            vals = {}
            for f in m.Fields:
                try: vals[str(f.FieldName)] = f.Data
                except Exception:
                    try: vals[str(f.FieldName)] = f.Value
                    except Exception: pass
            ri = int(vals.get('region index', -1)); pi = int(vals.get('permutation index', -1))
            rn = regions[ri][0] if 0 <= ri < len(regions) else '(any)'
            pn = regions[ri][1][pi] if 0 <= ri < len(regions) and 0 <= pi < len(regions[ri][1]) else '(any)'
            tr = vals.get('translation'); rot = vals.get('rotation')
            fmt = lambda v: ','.join('%.4f' % x for x in v) if v is not None else '?'
            L('   region %s perm %s node %s pos %s rot %s' % (rn, pn, vals.get('node index'), fmt(tr), fmt(rot)))
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'rm_markers.txt')), 'w').write('\n'.join(out))
