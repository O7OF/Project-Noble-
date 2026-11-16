import bpy, sys, traceback, collections
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\armor_fx_log.txt'
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
FX = ['armor_fx_%02d' % i for i in range(6)]
try:
    sc = bpy.context.scene.nwo
    meshes0 = [o for o in bpy.data.objects if o.type == 'MESH']
    def want_of(o):
        nm = o.name.split('.')[0]
        return nm.split(' ', 1) if ' ' in nm else ('default', nm)
    # 1. table entries (adding can trigger Foundry's rename callbacks; objects are re-assigned at the end)
    if 'armor_fx' not in [e.name for e in sc.regions_table]:
        e = sc.regions_table.add(); L('new region entry starts as %r' % e.name); e.name = 'armor_fx'
    for p in FX:
        if p not in [e.name for e in sc.permutations_table]:
            e = sc.permutations_table.add(); e.name = p
    # 2. one tiny hidden triangle per armor_fx permutation, rigged to b_pelvis like the body
    ref = bpy.data.objects['default body']
    arm = ref.parent
    pel = arm.matrix_world @ arm.data.bones['b_pelvis'].head_local
    for p in FX:
        name = '%s armor_fx' % p
        if name in bpy.data.objects: continue
        me = bpy.data.meshes.new(name)
        d = 0.0005
        me.from_pydata([(pel.x, pel.y, pel.z), (pel.x + d, pel.y, pel.z), (pel.x, pel.y + d, pel.z)], [], [(0, 1, 2)])
        me.materials.append(ref.material_slots[0].material)
        o = bpy.data.objects.new(name, me)
        for c in ref.users_collection: c.objects.link(o)
        o.parent = arm
        vg = o.vertex_groups.new(name='b_pelvis'); vg.add([0, 1, 2], 1.0, 'REPLACE')
        mod = o.modifiers.new('Armature', 'ARMATURE'); mod.object = arm
        L('made ' + name)
    # 3. markers: only present with their own armour effect
    for o in bpy.data.objects:
        if o.type != 'EMPTY': continue
        base = o.name.split('.')[0]
        if base.startswith('armor_fx_'):
            perm = base[:11]
            if perm in FX:
                n = o.nwo
                n.marker_uses_regions = True; n.region_name = 'armor_fx'
                n.marker_permutation_type = 'include'
                n.marker_permutations.clear(); n.marker_permutations.add().name = perm
                L('marker %s -> armor_fx/%s' % (o.name, perm))
        elif base == 'fx_space_helmet_light':
            n = o.nwo; n.marker_uses_regions = True; n.region_name = 'helmet'
            n.marker_permutation_type = 'include'; n.marker_permutations.clear(); n.marker_permutations.add().name = 'space'
            L('marker %s -> helmet/space' % o.name)
    # 4. re-assign every mesh from its name (guards against rename callbacks)
    for o in [o for o in bpy.data.objects if o.type == 'MESH']:
        perm, reg = want_of(o)
        o.nwo.region_name = reg; o.nwo.permutation_name = perm
    per = collections.defaultdict(list)
    for o in [o for o in bpy.data.objects if o.type == 'MESH']: per[o.nwo.region_name].append(o.nwo.permutation_name)
    for r in sorted(per): L('  %s: %s' % (r, ', '.join(sorted(per[r]))))
    L('regions: ' + ', '.join(e.name for e in sc.regions_table)); L('perms: ' + ', '.join(e.name for e in sc.permutations_table))
    bpy.ops.wm.save_mainfile(); L('SAVED')
    if '--export' in sys.argv:
        L('EXPORT ' + str(bpy.ops.nwo.export_scene()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
