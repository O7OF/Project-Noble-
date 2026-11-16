import bpy
WANT = ('council', 'ossoona', 'combat_armor', 'honor', 'invalid')
out = []
for m in bpy.data.materials:
    n = m.nwo
    if any(w in m.name for w in WANT) or 'invalid' in (n.shader_path or ''):
        users = [o.name for o in bpy.data.objects if o.type == 'MESH' and any(s.material == m for s in o.material_slots)]
        out.append('%s | path=%s | nodes=%s | type=%s | users=%s' % (m.name, n.shader_path, n.uses_blender_nodes, n.shader_type, users[:4]))
props = [p.identifier for p in bpy.data.materials[0].nwo.bl_rna.properties][:60]
open(r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\inspect_mats.txt', 'w').write('\n'.join(out) + '\nPROPS ' + ' '.join(props) + '\nALL ' + ', '.join(m.name for m in bpy.data.materials))
print('INSPECT DONE', len(out))
