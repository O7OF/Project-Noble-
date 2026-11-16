import sys, os
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
sys.path.insert(0, os.path.join(T, 'tools'))
from amf import AMF
m = AMF(os.path.join(T, r'ph-export\objects\characters\elite_ai_ultimate\elite_ai_ultimate_player.amf'))
out = []
for r in m.regions:
    for p in r['perms']:
        node = m.nodes[p['node']]['name'] if p['node'] != 255 else '-'
        out.append('%s %s vtype=%d node=%s vc=%d mtx=%s' % (r['name'], p['name'], p['vtype'], node, p['vc'], 'yes' if p['mtx'] else 'no'))
open(os.path.join(T, 'amf_perm_types.txt'), 'w').write('\n'.join(out))
print('WROTE')
