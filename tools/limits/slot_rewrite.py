#!/usr/bin/env python3
"""C12 step 1: for every engine instruction MemberTrace saw touching a player/peer slot, walk backwards in its
function to the instruction that produced the address register, and classify that derivation.

Input: objdump -d -M intel text of stock haloreach.dll (--asm), the trace (--trace). Output: a table of
derivations grouped by kind, which is the patch inventory for moving the slot arrays."""
import argparse, bisect, collections, json, re

REG64 = {}
for r64, aliases in {
    'rax': 'eax ax al', 'rbx': 'ebx bx bl', 'rcx': 'ecx cx cl', 'rdx': 'edx dx dl', 'rsi': 'esi si sil',
    'rdi': 'edi di dil', 'rbp': 'ebp bp bpl', 'rsp': 'esp sp spl',
    **{f'r{n}': f'r{n}d r{n}w r{n}b' for n in range(8, 16)}}.items():
    REG64[r64] = r64
    for a in aliases.split(): REG64[a] = r64

def load_asm(path):
    out = []
    for line in open(path):
        p = line.split('\t')
        if len(p) < 2: continue
        a = p[0].strip().rstrip(':')
        try: out.append((int(a, 16) - 0x180000000, normalize(p[1].strip())))
        except ValueError: pass
    return out

def function_starts(ins):
    starts, prev = [], True
    for a, t in ins:
        if prev and t != 'int3': starts.append(a)
        prev = t == 'int3'
    return starts

MEM = re.compile(r'\[(\w+)(?:\+(\w+)\*(\d))?([+-]0x[0-9a-f]+)?\]')

def normalize(text):
    """Accept capstone-style text too: 'qword ptr [r8 + 0x25]' -> objdump style."""
    text = re.sub(r'\s*([+\-*])\s*', r'\1', text) if '[' in text else text
    return re.sub(r'\b(byte|word|dword|qword|xmmword) ptr\b', lambda m: m.group(1).upper() + ' PTR', text)

def dest(text):
    m = re.match(r'(\w+)\s+([^,]+)', text)
    if not m: return None, None
    op, d = m.group(1), m.group(2).strip()
    return op, REG64.get(d)

def classify(text, reg):
    """What wrote `reg` here."""
    if text.startswith('imul') and re.search(r',0x(b0|158)$', text): return 'index*record'
    if text.startswith('lea'): return 'lea ' + text.split(',', 1)[1]
    if text.startswith(('add', 'sub')): return text.split()[0] + ' ' + text.split(',', 1)[1]
    if text.startswith('mov') and '[' in text: return 'load pointer'
    if text.startswith('mov'): return 'copy'
    if text.startswith('call'): return 'return value'
    return 'other'

# Real slot arrays inside one membership object (offsets); everything else (counts, mask, ids) is a scalar field.
ARRAYS = {'peersA': (0x60, 0x60 + 17 * 0x158), 'playersA': (0x1740, 0x1740 + 16 * 0xB0),
          'peersB': (0x2270, 0x2270 + 17 * 0x158), 'playersB': (0x3950, 0x3950 + 16 * 0xB0)}

def site_key(kind, chain, rva):
    """Group accesses by the instruction that produced their address."""
    if chain: return f"{kind.split()[0]}@{hex(chain[-1][0])}"
    return f"{kind.split()[0]}@{hex(rva)}"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--asm', required=True); ap.add_argument('--trace', required=True); ap.add_argument('--out')
    a = ap.parse_args()
    ins = load_asm(a.asm); addr = [x[0] for x in ins]; starts = function_starts(ins)
    index = {x[0]: i for i, x in enumerate(ins)}
    traced = {}
    for line in open(a.trace):
        m = re.search(r'MEMBER\|0x([0-9A-F]+)\|(\w+)\|\w\|\d+\|0x([0-9A-F]+)-0x([0-9A-F]+)', line)
        if not m: continue
        lo, hi = int(m.group(3), 16), int(m.group(4), 16)
        hit = [n for n, (b, e) in ARRAYS.items() if lo < e and hi >= b]
        if hit: traced.setdefault(int(m.group(1), 16), set()).update(hit)   # real slot records only
    rows, kinds = [], collections.Counter()
    for rva, regions in sorted(traced.items()):
        i = index.get(rva)
        if i is None: continue
        text = ins[i][1]; m = MEM.search(text)
        if not m: rows.append((rva, sorted(regions), text, [], 'no memory operand')); continue
        base = REG64.get(m.group(1)); fstart = starts[bisect.bisect_right(starts, rva) - 1]
        chain, reg, j, kind = [], base, i - 1, 'not found in function'
        while j >= 0 and ins[j][0] >= fstart and len(chain) < 8:
            op, d = dest(ins[j][1])
            if d == reg and op not in ('cmp', 'test', 'push'):
                k = classify(ins[j][1], reg); chain.append((ins[j][0], ins[j][1]))
                if k == 'copy':                      # follow the source register
                    src = REG64.get(ins[j][1].split(',', 1)[1].strip())
                    if src: reg = src; j -= 1; continue
                kind = k; break
            j -= 1
        kinds[kind.split()[0] if kind else '?'] += 1
        rows.append((rva, sorted(regions), text, chain, kind))
    sites = {}
    for r, g, t, c, k in rows: sites.setdefault(site_key(k, c, r), []).append(hex(r))
    print('slot-record accesses:', len(rows), '| address derivation sites:', len(sites))
    for k, v in kinds.most_common(): print(f'  base register comes from {k}: {v}')
    if a.out:
        json.dump({'note': 'slot_rewrite.py: traced instructions whose offsets fall inside the real slot arrays '
                           '(ARRAYS), and the address derivation sites they come from.',
                   'slot_accesses': [{'rva': hex(r), 'regions': g, 'insn': t, 'kind': k,
                                      'chain': [[hex(x), y] for x, y in c]} for r, g, t, c, k in rows],
                   'sites': sites}, open(a.out, 'w'), indent=1)

if __name__ == '__main__':
    main()
