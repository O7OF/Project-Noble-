import bpy, sys, traceback, collections
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\fix_perms_log.txt'
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
try:
    n = bpy.context.scene.nwo
    L('table before: ' + ', '.join(e.name for e in n.permutations_table))
    meshes = [o for o in bpy.data.objects if o.type == 'MESH']
    want = {}
    for o in meshes:
        nm = o.name.split('.')[0]
        perm, reg = nm.split(' ', 1) if ' ' in nm else ('default', nm)
        want[o.name] = (perm, reg)
    # make sure every permutation exists in the table WITHOUT renaming existing entries
    names = [e.name for e in n.permutations_table]
    for perm in sorted({p for p, _ in want.values()}):
        if perm not in names:
            e = n.permutations_table.add()
            L('  new entry starts as %r' % e.name)
            if e.name != perm:
                # direct assignment of the underlying string (avoid the rename callback when possible)
                try:
                    e['name'] = perm
                except Exception:
                    e.name = perm
            names = [x.name for x in n.permutations_table]
    # now assign objects directly
    for o in meshes:
        perm, reg = want[o.name]
        o.nwo.region_name = reg
        o.nwo.permutation_name = perm
    bad = [(o.name, o.nwo.permutation_name, o.nwo.region_name) for o in meshes if (o.nwo.permutation_name, o.nwo.region_name) != want[o.name]]
    L('table after: ' + ', '.join(e.name for e in n.permutations_table))
    L('mismatches after assign: %d %s' % (len(bad), bad[:6]))
    per = collections.defaultdict(list)
    for o in meshes: per[o.nwo.region_name].append(o.nwo.permutation_name)
    for r in sorted(per): L('  %s: %s' % (r, ', '.join(sorted(per[r]))))
    bpy.ops.wm.save_mainfile(); L('SAVED')
    if not bad and '--export' in sys.argv:
        L('EXPORT ' + str(bpy.ops.nwo.export_scene()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
