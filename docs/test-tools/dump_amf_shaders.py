import sys, os
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
sys.path.insert(0, os.path.join(T, 'tools'))
from amf import AMF
m = AMF(os.path.join(T, r'ph-export\objects\characters\elite_ai_ultimate\elite_ai_ultimate_player.amf'))
with open(os.path.join(T, 'amf_shaders.txt'), 'w') as f:
    for i, s in enumerate(m.shaders):
        tex = ['%d:%s@%s' % (k, t[0], t[1]) for k, t in enumerate(s['tex']) if t]
        f.write('%d %s | tints %s | transp %d cc %d\n   %s\n' % (i, s['name'], s['tints'], s['transparent'], s['cc_only'], '\n   '.join(tex)))
print('WROTE', len(m.shaders))
