import bpy
OUT = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\inspect_markers.txt'
out = []
ms = [o for o in bpy.data.objects if o.type == 'EMPTY']
out.append('empties %d' % len(ms))
for o in ms:
    if 'armor_fx' in o.name or 'space' in o.name or o is ms[0]:
        n = o.nwo
        out.append('%s parent=%s bone=%s uses_regions=%s region=%s perm_type=%s perms=%s marker_type=%s' % (
            o.name, o.parent.name if o.parent else None, o.parent_bone, getattr(n, 'marker_uses_regions', '?'), getattr(n, 'region_name', '?'),
            getattr(n, 'marker_permutation_type', '?'),
            [getattr(p, 'name', str(p)) for p in getattr(n, 'marker_permutations', [])], getattr(n, 'marker_type', '?')))
p = ms[0].nwo
out.append('PROPS ' + ' '.join(x.identifier for x in p.bl_rna.properties if 'marker' in x.identifier or 'region' in x.identifier or 'perm' in x.identifier))
sc = bpy.context.scene.nwo
out.append('regions table: ' + ', '.join(e.name for e in sc.regions_table))
out.append('region entry props: ' + ' '.join(x.identifier for x in sc.regions_table[0].bl_rna.properties))
open(OUT, 'w').write('\n'.join(out))
