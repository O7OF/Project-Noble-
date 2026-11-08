#!/usr/bin/env python3
"""C12 step 2: patch specs for the imul-shaped slot address sites (see docs/limits-players-audit.md)."""
import json, re, subprocess, sys
STOCK = sys.argv[1]; SITES = sys.argv[2]; OUT = sys.argv[3]
ARRAYS = {'peersA': (0x60, 0x158), 'playersA': (0x1740, 0xB0), 'peersB': (0x2270, 0x158), 'playersB': (0x3950, 0xB0)}
d = json.load(open(SITES))
acc = {r['rva']: r for r in d['slot_accesses']}
def dis(rva, n=16):
    o = subprocess.run(['objdump', '-d', '-M', 'intel', f'--start-address={0x180000000+rva:#x}',
                        f'--stop-address={0x180000000+rva+n:#x}', STOCK], capture_output=True, text=True).stdout
    for l in o.split('\n'):
        p = l.split('\t')
        if len(p) >= 3 and p[0].strip().rstrip(':') == f'{0x180000000+rva:x}':
            return bytes.fromhex(p[1].replace(' ', '')), p[2].strip()
    return None, None
specs, review = [], []
for key, users in d['sites'].items():
    kind, at = key.split('@')
    if kind != 'index*record': continue
    rva = int(at, 16); raw, text = dis(rva)
    m = re.match(r'imul\s+(\w+),(\w+),(0x[0-9a-f]+)$', text or '')
    if not m: review.append((key, 'not a 3-operand imul', text)); continue
    dst, src, rec = m.group(1), m.group(2), int(m.group(3), 16)
    arrays = sorted({g for u in users for g in acc[u]['regions']})
    fits = [a for a in arrays if ARRAYS[a][1] == rec]
    entry = {'rva': hex(rva), 'bytes': raw.hex(), 'len': len(raw), 'text': text, 'dst': dst, 'index': src,
             'record': hex(rec), 'arrays': fits, 'users': users}
    if len(fits) != 1 or len(raw) < 5 or not dst.startswith('r') or dst.endswith('d'):
        review.append((key, f'arrays={arrays} len={len(raw)}', text)); entry['review'] = True
    specs.append(entry)
json.dump({'auto': [s for s in specs if not s.get('review')], 'review': [s for s in specs if s.get('review')],
           'not_imul': review}, open(OUT, 'w'), indent=1)
auto = [s for s in specs if not s.get('review')]
print(f'imul sites: {len(specs)} | auto-patchable: {len(auto)} | need review: {len(specs)-len(auto)}')
from collections import Counter
print('by array:', Counter(s['arrays'][0] for s in auto)); print('instruction lengths:', Counter(s['len'] for s in auto))
for k, why, t in review[:10]: print('  review', k, why, t)
