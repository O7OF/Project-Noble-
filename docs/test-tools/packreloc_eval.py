import os, sys, struct, collections
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
sys.path.insert(0, os.path.join(T, 'tools'))
os.environ['OUT'] = 'packreloc_dummy.txt'
import packreloc as pr
H, owner, gname, (lo, hi), W = pr.analyse(pr.P)
r = open(pr.P + '.rrreloc', 'rb').read()
cnt = struct.unpack_from('<I', r, 8)[0]
rel = set(struct.unpack_from(f'<{cnt}I', r, 0x20))
out = []
def grp(o):
    t = owner(o); return gname(t[1]) if t else 'pre'
A = {o for o in H if W[o//4 + 1] == 0}
# header-table entries are in H unconditionally; keep them
first = min(rel)
extra0 = set()
for i in range(first//4, len(W) - 3):
    v = W[i]
    if lo <= v < hi and W[i+1] == 0 and W[i+2] == 0:
        extra0.add(i*4)
B = A | {o for o in extra0 if grp(o) in ('snd!', 'hcfd')}
C = A | extra0
Hh = H | {o for o in rel if False}
def rep(name, S):
    e = S - rel; mi = rel - S
    out.append(f"{name}: {len(S)} recall-miss {len(mi)} extra {len(e)} extra-by-group {collections.Counter(grp(o) for o in e).most_common(10)} miss-by-group {collections.Counter(grp(o) for o in mi).most_common(6)}")
rep('H', H); rep('A next0', A); rep('B +snd/hcfd ptr00', B); rep('C +any ptr00', C)
# header-table entries lost by A?
open(os.path.join(T, 'packreloc_eval.txt'), 'w').write("\n".join(out))
