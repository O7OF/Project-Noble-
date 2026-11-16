# List every region/permutation of the PH source AMF with its submesh shaders + face counts -> amf_hg.txt
import sys, os
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
sys.path.insert(0, os.path.join(T, 'tools'))
from amf import AMF
m = AMF(os.path.join(T, r'ph-export\objects\characters\elite_ai_ultimate\elite_ai_ultimate_player.amf'))
out = []
for r in m.regions:
    for p in r['perms']:
        verts, tris, sub, _ = m.mesh(p)
        parts = ['%s:%d' % (m.shaders[s]['name'] if 0 <= s < len(m.shaders) else s, c // 3 if False else c) for s, st, c in sub]
        out.append('%s / %s  faces=%d  %s' % (r['name'], p['name'], p['fc'], ' | '.join(parts)))
out.append('--- shaders')
for i, s in enumerate(m.shaders):
    out.append('%d %s %s' % (i, s['name'], [t[0] for t in s['tex'] if t]))
open(os.path.join(T, 'amf_hg.txt'), 'w').write('\n'.join(out))
