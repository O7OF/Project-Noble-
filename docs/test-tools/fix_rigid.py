import bpy, sys, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\fix_rigid_log.txt'
# rigid pieces in PH's AMF (vtype 2) and their real bone
RIGID = {
    'chest': ('b_torso_armor', ['minor', 'story_elite', 'zealot', 'honor_guard']),
    'helmet': ('b_head', ['general', 'officer', 'ossoona', 'spec_ops', 'ultra']),
    'left_shoulder': ('b_l_upperarm', ['combat', 'general', 'honor_guard', 'minor', 'officer', 'schism', 'space', 'spec_ops', 'story_elite', 'ultra', 'zealot']),
    'right_shoulder': ('b_r_upperarm', ['combat', 'general', 'honor_guard', 'minor', 'officer', 'schism', 'space', 'spec_ops', 'story_elite', 'ultra', 'zealot']),
}
NEW_SHADERS = ['elite_council_helmet', 'elite_ossoona_helmet', 'elite_combat_armor', 'default_ch_elite_armor_02_honor',
               'default_sh_elite_honor_armor_01', 'default_ch_elite_honor_armor_01_illum', 'honor_lights_ch_elite_armor_emissive']
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
try:
    fixed = 0
    for region, (bone, perms) in RIGID.items():
        for perm in perms:
            o = bpy.data.objects.get('%s %s' % (perm, region))
            if o is None:
                L('missing object %s %s' % (perm, region)); continue
            groups = {g.index: g.name for g in o.vertex_groups}
            used = set()
            for v in o.data.vertices:
                for g in v.groups:
                    if g.weight > 0: used.add(groups[g.group])
            for g in list(o.vertex_groups):
                o.vertex_groups.remove(g)
            vg = o.vertex_groups.new(name=bone)
            vg.add([v.index for v in o.data.vertices], 1.0, 'REPLACE')
            fixed += 1
            L('%s %s: %s -> %s' % (perm, region, sorted(used), bone))
    L('fixed %d objects' % fixed)
    for name in NEW_SHADERS:
        m = bpy.data.materials.get(name)
        if m: m.nwo.shader_path = 'objects\\characters\\elite_noble\\shaders\\%s.shader' % name
    bpy.ops.wm.save_mainfile(); L('SAVED')
    if '--export' in sys.argv:
        L('EXPORT ' + str(bpy.ops.nwo.export_scene()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
