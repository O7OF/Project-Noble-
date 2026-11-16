import sys, os, collections
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
sys.path.insert(0, os.path.join(T, 'tools'))
from amf import AMF
m = AMF(os.path.join(T, r'ph-export\objects\characters\elite_ai_ultimate\elite_ai_ultimate_player.amf'))
out = []
for r in m.regions:
    for p in r['perms']:
        if p['vtype'] != 2: continue
        v, t, sub, _ = m.mesh(p)
        c = collections.Counter(i[0][0] for *_, i in v if i)
        xs = [x[0] for x in v]
        mn = [min(a[k] for a in xs) for k in range(3)]; mx = [max(a[k] for a in xs) for k in range(3)]
        out.append('%s %s permnode=%d(%s) vertnodes=%s bbox=%s..%s' % (r['name'], p['name'], p['node'], m.nodes[p['node']]['name'],
            ', '.join('%d(%s)x%d' % (n, m.nodes[n]['name'] if n < len(m.nodes) else '?', k) for n, k in c.most_common(4)),
            [round(a, 3) for a in mn], [round(a, 3) for a in mx]))
# reference: a skinned helmet's bbox
for r in m.regions:
    for p in r['perms']:
        if r['name'] == 'helmet' and p['name'] in ('minor', 'honor_guard', 'council'):
            v, *_ = m.mesh(p); xs = [x[0] for x in v]
            out.append('REF helmet %s bbox=%s..%s' % (p['name'], [round(min(a[k] for a in xs), 3) for k in range(3)], [round(max(a[k] for a in xs), 3) for k in range(3)]))
open(os.path.join(T, 'amf_rigid_nodes.txt'), 'w').write('\n'.join(out))
print('WROTE')
