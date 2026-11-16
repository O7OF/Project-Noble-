# Tag-layout walker for Reach MCC cache files (Stage B).
# Walks every tag of a pack with HREK's tag definitions (ManagedBlam TagGroupType.GetDefinition -> exact cache
# field sizes) and collects: packed pointers (block +4, data +12), string id fields, tag references, resource /
# interop fields. MODE=validate compares with RR's pack (.rrreloc + appended load fixup table);
# MODE=build writes <pack>.rrreloc and the fixup table into <pack>.
import os, sys, struct, bisect, collections, time, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
OUTN = os.environ.get('OUT', 'tagwalk.txt')
out = []
L = out.append
def save(): open(os.path.join(T, OUTN), 'w').write("\n".join(out))
PB = 0x50000000
MODE = os.environ.get('MODE', 'validate')
PACK = os.environ.get('PACK', r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map")
REC = MODE == 'validate' or os.environ.get('REC')
SKIPREC = ('Real', 'Angle', 'String', 'LongString', 'Explanation', 'Custom', 'Terminator', 'Point2d', 'Rectangle2d',
           'RgbPixel32', 'ArgbPixel32', 'ShortInteger', 'CharInteger', 'ShortEnum', 'CharEnum', 'WordFlags', 'ByteFlags',
           'ShortBlockIndex', 'CharBlockIndex', 'ShortIntegerBounds', 'ByteInteger', 'WordInteger')

class Lay:
    __slots__ = ('id', 'size', 'items', 'fields', 'name', 'hot', 'align')
A1 = {'CharInteger','CharEnum','ByteFlags','CharBlockIndex','CharBlockIndexCustomSearch','ByteBlockFlags','ByteInteger','String','LongString','Pad','Skip','UselessPad','Custom','Explanation','Terminator'}
A2 = {'ShortInteger','ShortEnum','WordFlags','ShortBlockIndex','ShortBlockIndexCustomSearch','WordBlockFlags','WordInteger','Point2d','Rectangle2d','ShortIntegerBounds'}
A64 = int(os.environ.get('A64', '4'))
A8 = {'Int64Integer','QwordInteger'}
LAYS = {}; LAYLIST = []; MISMATCH = []
HIDDEN = collections.Counter()
PADTYPES = {int(x, 16) for x in os.environ.get('PADTYPES', '35,37').split(',')}
def _ip(x):
    from System.Runtime.Serialization import SerializationInfo, FormatterConverter, StreamingContext, StreamingContextStates, ISerializable
    import System
    si = SerializationInfo(System.Type.GetType('System.Reflection.Pointer'), FormatterConverter())
    ISerializable(x).GetObjectData(si, StreamingContext(StreamingContextStates.All))
    return si.GetValue('_ptr', System.Type.GetType('System.IntPtr')).ToInt64()
def _priv(o, n):
    from System.Reflection import BindingFlags as BF
    return o.GetType().GetField(n, BF.Public | BF.NonPublic | BF.Instance).GetValue(o)
def native_fields(ed):
    # managed fields in native order, with hidden native entries as (type, size) tuples
    from System.Runtime.InteropServices import Marshal
    from System import IntPtr
    man = {}
    for f in ed.Fields: man[_ip(_priv(f, 'm_field'))] = f
    d = _ip(_priv(ed, 'm_definition'))
    p = Marshal.ReadInt64(IntPtr(d + 48))
    res = []
    for k in range(4000):
        q = p + 32 * k
        if q in man: res.append(man.pop(q)); continue
        t = Marshal.ReadInt32(IntPtr(q))
        if t == 0x3E: break
        res.append((t, Marshal.ReadInt32(IntPtr(q + 16))))
    if man: MISMATCH.append(f"native walk lost {len(man)} managed fields")
    return res
def compile_lay(ed, label):
    lay = LAYS.get(ed)
    if lay is not None: return lay
    lay = Lay(); LAYS[ed] = lay; lay.id = len(LAYLIST); LAYLIST.append(lay)
    lay.size = ed.ElementSize; lay.items = []; lay.fields = []; lay.name = label; lay.hot = False
    off = 0; mal = 1
    for f in native_fields(ed):
        if isinstance(f, tuple):
            ht, hsz = f; HIDDEN[ht] += 1
            if ht in PADTYPES and hsz:
                lay.fields.append((off, hsz, 'Pad%X' % ht, 'pad', None, 0)); off += hsz
            continue
        ft = str(f.FieldType); sz = f.FieldSize; nm = str(f.RawFieldName).split('#')[0].split(':')[0][:40]
        sub = None; n = 0
        if ft in ('Struct', 'Array'):
            sub = compile_lay(f.Element, label + '/' + nm); al = sub.align
        elif ft in A1: al = 1
        elif ft in A2: al = 2
        elif ft in A8: al = A64
        else: al = 4
        if sz == 0: al = 1
        off = (off + al - 1) & ~(al - 1); mal = max(mal, al)
        if ft in ('Struct', 'Block', 'Array'):
            sub = compile_lay(f.Element, label + '/' + nm)
            if ft == 'Struct':
                if sub.size != sz: MISMATCH.append(f"struct {label}/{nm} field {sz} def {sub.size}")
                lay.items.append((off, 't', sub, 1))
            elif ft == 'Array':
                n = sz // sub.size if sub.size else 0
                if n * sub.size != sz: MISMATCH.append(f"array {label}/{nm} field {sz} elem {sub.size}")
                lay.items.append((off, 'a', sub, n))
            else:
                if sz != 12: MISMATCH.append(f"block {label}/{nm} size {sz}")
                lay.items.append((off, 'b', sub, 0))
        elif ft in ('StringId', 'OldStringId'): lay.items.append((off, 's', None, 0))
        elif ft == 'Reference': lay.items.append((off, 'r', None, 0))
        elif ft == 'Data': lay.items.append((off, 'd', None, 0))
        elif ft == 'Resource': lay.items.append((off, 'x', None, sz))
        elif ft == 'Interop': lay.items.append((off, 'i', None, sz))
        elif ft == 'VertexBuffer': lay.items.append((off, 'v', None, sz))
        elif ft in ('LongInteger', 'DwordInteger', 'LongBlockIndex', 'LongBlockIndexCustomSearch'): lay.items.append((off, 'l', None, 0))
        lay.fields.append((off, sz, ft, nm, sub, n))
        off += sz
    lay.align = mal
    off = (off + mal - 1) & ~(mal - 1)
    if off != lay.size: MISMATCH.append(f"sum {label}: fields {off} element {lay.size}: " + " ".join(f"{x[2]}{x[1]}@{x[0]}" for x in lay.fields)[:300])
    return lay

def set_hot():
    for l in LAYLIST: l.hot = any(it[1] in 'sdrxivl' or it[1] == 'b' for it in l.items)
    ch = True
    while ch:
        ch = False
        for l in LAYLIST:
            if not l.hot and any(it[2] is not None and it[2].hot for it in l.items): l.hot = True; ch = True

def resolve(lay, off, depth=0):
    for (fo, sz, ft, nm, sub, n) in lay.fields:
        if fo <= off < fo + sz and sz:
            if ft == 'Struct': return nm + '/' + resolve(sub, off - fo, depth + 1)
            if ft == 'Array' and sub.size: return f"{nm}[{(off-fo)//sub.size}]/" + resolve(sub, (off - fo) % sub.size, depth + 1)
            return f"{ft}:{nm}+{off-fo}"
    return f"?+{off}"

def walk_pack(path):
    m = open(path, 'rb').read()
    toff, tsize = struct.unpack_from('<II', m, 0x10)
    vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
    sec = m[toff:toff + tsize]
    nW = len(sec) // 4
    W = memoryview(sec[:nW * 4]).cast('I')
    def u32(o): return W[o >> 2] if not (o & 3) else struct.unpack_from('<I', sec, o)[0]
    rel = lambda a: a - vbase
    th = [W[rel(thdr) // 4 + k] for k in range(16)]
    gcount, gaddr = th[0], th[2] | (th[3] << 32)
    icount, iaddr = th[4], th[6] | (th[7] << 32)
    xcount, xaddr = th[12], th[14] | (th[15] << 32)
    L(f"{os.path.basename(path)}: file {len(m):X} tags off {toff:X} size {tsize:X} vbase {vbase:X} groups {gcount} tags {icount} interops {xcount}")
    groups = []
    for i in range(gcount):
        g = struct.unpack_from('<I', sec, rel(gaddr) + 16 * i)[0]
        groups.append(struct.pack('>I', g).decode('latin1'))
    hdr_ptrs = set()
    for i in range(icount):
        o = rel(iaddr) + 8 * i + 4
        if W[o // 4]: hdr_ptrs.add(o)
    lo, hi = (vbase - PB) >> 2, (vbase + tsize - PB) >> 2
    for i in range(xcount):
        o = rel(xaddr) + 8 * i
        if lo <= W[o // 4] < hi: hdr_ptrs.add(o)
    G = B.Tags.TagGroupType
    gl = {}
    for gi, g in enumerate(groups):
        try: gl[gi] = compile_lay(G.GetDefinition(g).ElementDefinition, g)
        except Exception as e: L(f"no definition for group {g!r}: {e}")
    # Cache-only layouts (the cache stores these groups in a compact runtime form, not the editing layout)
    def custom(size, items, name):
        lay = Lay(); lay.id = len(LAYLIST); LAYLIST.append(lay); lay.size = size; lay.items = items
        lay.fields = [(o, 12 if c == 'b' else (8 if c == 'x' else 4), {'x': 'Resource', 'b': 'Block', 's': 'StringId'}[c], name + '@%d' % o, None, 0) for (o, c, s_, n_) in items]
        lay.name = name; lay.hot = True; lay.align = 4
        return lay
    if 'snd!' in groups and not os.environ.get('NOSNDFIX'):
        sndsub = custom(220, [(32, 'x', None, 8)], 'snd!/sub220')
        gl[groups.index('snd!')] = custom(52, [(28, 'x', None, 8), (36, 'b', sndsub, 0), (48, 's', None, 0)], 'snd!')
    set_hot()
    R = dict(ptrs=set(), s=set(), r=set(), x=set(), i=set(), v=set(), dat=set(), bad=[])
    SALT = []
    for i in range(icount):
        gi_, salt_, p_ = struct.unpack_from('<HHI', sec, rel(iaddr) + 8 * i)
        SALT.append(salt_ if p_ else 0xFFFF)
    es = []  # (start, layid) of root/block elements (REC)
    DR = []
    seen_blocks = set()
    def walk(lay, a, root):
        if REC and root: es.append((a, lay.id))
        if not lay.hot and not REC: return
        for (fo, c, sub, n) in lay.items:
            o = a + fo
            if c == 's': R['s'].add(o)
            elif c == 'r': R['r'].add(o)
            elif c == 'b':
                cnt = u32(o); p = u32(o + 4)
                if p:
                    R['ptrs'].add(o + 4)
                    if not (lo <= p < hi): R['bad'].append(f"block ptr out of range at {o:X} ({lay.name})"); continue
                    if cnt > 0x100000: R['bad'].append(f"block count {cnt} at {o:X} ({lay.name})"); continue
                    base = p * 4 + PB - vbase
                    if (base, sub.id) in seen_blocks: continue
                    seen_blocks.add((base, sub.id))
                    if base + cnt * sub.size > tsize: R['bad'].append(f"block past end at {o:X} ({lay.name})"); continue
                    if sub.hot or REC:
                        for k in range(cnt): walk(sub, base + k * sub.size, True)
            elif c == 'd':
                p = u32(o + 12)
                if p:
                    R['ptrs'].add(o + 12)
                    if REC and lo <= p < hi: DR.append((p * 4 + PB - vbase, u32(o), lay.name))
            elif c == 't': walk(sub, o, False)
            elif c == 'a':
                for k in range(n): walk(sub, o + k * sub.size, False)
            elif c == 'x': R['x'].add(o)
            elif c == 'i':
                R['i'].add(o); p = u32(o)
                if lo <= p < hi: R['ptrs'].add(o)
            elif c == 'v': R['v'].add(o)
            elif c == 'l':
                v = u32(o); ix = v & 0xFFFF
                if v != 0xFFFFFFFF and ix < icount and SALT[ix] == (v >> 16) and SALT[ix] != 0xFFFF: R['dat'].add(o - 12)
    tags = []
    for i in range(icount):
        gi, salt, p = struct.unpack_from('<HHI', sec, rel(iaddr) + 8 * i)
        if not p or gi == 0xFFFF or gi not in gl: continue
        a = p * 4 + PB - vbase
        tags.append((a, gi))
        walk(gl[gi], a, True)
    return dict(SALT=SALT, DR=DR, m=m, sec=sec, W=W, u32=u32, vbase=vbase, tsize=tsize, toff=toff, R=R, hdr=hdr_ptrs, es=es, groups=groups, tags=tags)

def read_fixups(m):
    flag, before, off, size = struct.unpack_from('<IIII', m, 0x97A8)
    if not size: return None, (flag, before, off, size)
    h = struct.unpack_from('<IIII', m, off)
    ents = [struct.unpack_from('<IIQ', m, off + h[1] + 16 * k) for k in range(h[0])]
    return (h, ents), (flag, before, off, size)

def hist(lbls, n=15):
    return "  " + "\n  ".join(f"{c:7} {k}" for k, c in collections.Counter(lbls).most_common(n))

try:
    t0 = time.time()
    from mbstart import B, mb_stop
    P = walk_pack(PACK)
    R, vbase, u32 = P['R'], P['vbase'], P['u32']
    L(f"hidden native types: {dict(HIDDEN)}")
    L(f"walk {time.time()-t0:.0f} s, layouts {len(LAYLIST)}, tags walked {len(P['tags'])}, mismatches {len(MISMATCH)}")
    for x in MISMATCH[:int(os.environ.get('NMIS', '15'))]: L("  MISMATCH " + x)
    for x in R['bad'][:15]: L("  BAD " + x)
    L(f"bad {len(R['bad'])}  ptrs {len(R['ptrs'])} + hdr {len(P['hdr'])}  sids {len(R['s'])} refs {len(R['r'])} resources {len(R['x'])} interops {len(R['i'])} vbufs {len(R['v'])}")
    allp = R['ptrs'] | P['hdr']
    if REC:
        Wd = P['W']; vb = P['vbase']; lo_, hi_ = (vb - PB) >> 2, (vb + P['tsize'] - PB) >> 2
        es_ = sorted(P['es']); st_ = [e[0] for e in es_]
        def outside(o):
            k = bisect.bisect_right(st_, o) - 1
            return k < 0 or o >= es_[k][0] + LAYLIST[es_[k][1]].size
        hx = set()
        for i in range(3, len(Wd) - 2):
            v = Wd[i]
            if lo_ <= v < hi_ and Wd[i+1] == 0 and Wd[i-1] == 0 and Wd[i-2] == 0 and 0 < Wd[i-3] < 0x10000000 and (i*4) not in allp and outside(i*4): hx.add(i*4)
        L(f"data-ref pointers outside walked elements (heuristic): {len(hx)}")
        allp |= hx
    if REC:
        es = sorted(P['es']); starts = [e[0] for e in es]
        DRL = sorted(P['DR']); dstarts = [d[0] for d in DRL]; DRS = []
        if es: L(f"elements {len(es)} first {es[0][0]:X} data ranges {len(DRL)}")
        def label(o):
            k = bisect.bisect_right(starts, o) - 1
            for (ds, dn, dl) in DRS:
                pass
            j = bisect.bisect_right(dstarts, o) - 1
            if j >= 0 and o < DRL[j][0] + DRL[j][1]: return "data:" + DRL[j][2]
            if k < 0: return "(before)"
            a, lid = es[k]; lay = LAYLIST[lid]
            if o >= a + lay.size: return "(outside after " + lay.name + ")"
            return lay.name.split('/')[0] + ':' + resolve(lay, o - a)
    rr = PACK + '.rrreloc'
    if os.path.exists(rr) and MODE == 'validate':
        r = open(rr, 'rb').read(); cnt = struct.unpack_from('<I', r, 8)[0]
        RS = set(struct.unpack_from(f'<{cnt}I', r, 0x20))
        L(f"\n.rrreloc {len(RS)}: walker {len(allp)}, miss {len(RS-allp)}, extra {len(allp-RS)}")
        L("miss labels:\n" + hist([label(o) for o in sorted(RS - allp)]))
        ms = sorted(RS - allp); L("miss sample: " + " ".join(f"{o:X}" for o in ms[::max(1, len(ms)//16)][:16]))
        if ms:
            o = ms[0]; L("words around first miss: " + " ".join(f"{u32(o + 4*j):X}" for j in range(-6, 8)))
        L("extra labels:\n" + hist([label(o) for o in sorted(allp - RS)]))
    fx, fh = read_fixups(P['m'])
    L(f"\nfixup header {fh[0]} {fh[1]:X} {fh[2]:X} {fh[3]:X}")
    if fx and MODE == 'validate':
        h, ents = fx
        L(f"table header {[hex(x) for x in h]}; first entries " + " ".join(f"({k},{z},{a:X})" for k, z, a in ents[:6]))
        L(f"sorted by address: {all(ents[i][2] <= ents[i+1][2] for i in range(len(ents)-1))}; kinds {collections.Counter(e[0] for e in ents)}")
        K = collections.defaultdict(set)
        for k, z, a in ents: K[k].add(a - vbase)
        mine = {0: R['s'], 1: R['x'], 2: R['r'] | R['dat']}
        for k in sorted(K):
            S = K[k]; M = mine.get(k, set())
            L(f"\nkind {k}: rr {len(S)} walker {len(M)} miss {len(S-M)} extra {len(M-S)}")
            L(" rr labels:\n" + hist([label(o) for o in S], 12))
            if S - M: L(" miss labels:\n" + hist([label(o) for o in S - M]))
            if S - M and k == 2:
                L(" miss +12 labels:\n" + hist([label(o + 12) for o in S - M], 12))
                L(" miss values @+12: " + " ".join(f"{u32(o+12):X}" for o in sorted(S - M)[::1500][:14]))
                mz = [o for o in S - M if label(o).startswith('zone:')]
                SALT = P['SALT']; isd = lambda v: v != 0xFFFFFFFF and (v & 0xFFFF) < len(SALT) and SALT[v & 0xFFFF] == (v >> 16)
                vm = [o for o in S - M if isd(u32(o + 12))]
                L(f" misses whose +12 is a VALID tag datum: {len(vm)}" + (":\n" + hist([label(o + 12) for o in vm], 10) if vm else ""))
                if mz: L(" zone miss sample: " + " | ".join(f"{o:X}:" + " ".join(f"{u32(o + 4*j):X}" for j in range(5)) for o in sorted(mz)[:4]))
            if M - S:
                ex = M - S
                L(" extra labels:\n" + hist([label(o) for o in ex]))
                if k == 0: L(f" extra values zero: {sum(1 for o in ex if u32(o) == 0)} / {len(ex)}; rr values zero: {sum(1 for o in S if u32(o) == 0)}")
                if k == 2: L(f" extra datum -1: {sum(1 for o in ex if u32(o+12) == 0xFFFFFFFF)} / {len(ex)}; rr datum -1: {sum(1 for o in S if u32(o+12) == 0xFFFFFFFF)}")
            L(" rr sample: " + " ".join(f"{o:X}={u32(o):X}" for o in sorted(S)[:5]))
            outs = sorted(o for o in S - M if label(o).startswith('('))
            if outs: L(f" outside: {len(outs)} range {outs[0]:X}..{outs[-1]:X} sample " + " ".join(f"{o:X}" for o in outs[::max(1, len(outs)//12)][:12]))
    if os.environ.get('DIAG') and fx:
        RSx = RS if 'RS' in dir() else set()
        KK = {}
        for k, z, a in fx[1]: KK[a - vbase] = k
        def flat(lay, base, pre, depth=0):
            rows = []
            for (fo, sz, ft, nm, sub, n) in lay.fields:
                if ft == 'Struct' and depth < 3: rows += flat(sub, base + fo, pre + nm + '/', depth + 1)
                else: rows.append((base + fo, sz, ft, pre + nm))
            return rows
        for g in os.environ['DIAG'].split(','):
            gi = P['groups'].index(g) if g in P['groups'] else -1
            tl = [a for a, x in P['tags'] if x == gi]
            if not tl: continue
            a = tl[0]; lay = LAYS[B.Tags.TagGroupType.GetDefinition(g).ElementDefinition]
            L(f"\n=== {g} tag at {a:X} size {lay.size}")
            truth = sorted([(o - a, 'P') for o in RSx if a <= o < a + lay.size] + [(o - a, 'K%d' % k) for o, k in KK.items() if a <= o < a + lay.size])
            L(" truth: " + " ".join(f"{o}{t}" for o, t in truth))
            L(" fields: " + " ".join(f"{o}:{ft}{sz}:{nm[:22]}" for o, sz, ft, nm in flat(lay, 0, '') if ft not in ('Real', 'Explanation', 'Custom', 'Angle')))
    if MODE == 'build':
        # .rrreloc (same format as packreloc2)
        offs = sorted(allp)
        with open(PACK + '.rrreloc', 'wb') as f:
            f.write(b'RRRL' + struct.pack('<IIQQI', 1, len(offs), vbase, PB, P['tsize']))
            f.write(struct.pack(f'<{len(offs)}I', *offs))
        L(f"wrote {PACK}.rrreloc ({len(offs)})")
        u = P['u32']
        sids = [o for o in R['s'] if u(o) != 0] if os.environ.get('SKIPZERO') else list(R['s'])
        refs = [o for o in R['r'] if u(o + 12) != 0xFFFFFFFF] if os.environ.get('SKIPZERO') else list(R['r'])
        kind1 = sorted(R['x'])
        refs = sorted(set(refs) | R['dat'])
        ents = sorted([(vbase + o, 0) for o in sids] + [(vbase + o, 1) for o in kind1] + [(vbase + o, 2) for o in refs])
        n = len(ents)
        H4 = int(os.environ.get('H4', '0'))   # RR: {count, 0x10, 0 address fixups, 0}
        tbl = struct.pack('<IIII', n, 0x10, 0, H4) + b''.join(struct.pack('<IIQ', k, 0, a) for a, k in ents)
        flag, before, foff, fsize = fh
        with open(PACK, 'r+b') as f:
            if fsize:
                f.truncate(before); end = before
            else:
                f.seek(0, 2); end = f.tell()
            foff = (end + 0xFFF) & ~0xFFF
            f.seek(end); f.write(b'\0' * (foff - end)); f.write(tbl)
            pad = (-len(tbl)) & 0xFFF; f.write(b'\0' * pad)
            f.seek(0x97A8); f.write(struct.pack('<IIII', 1, end, foff, len(tbl)))
        L(f"wrote fixup table: {n} entries (sid {len(sids)}, kind1 {len(kind1)}, refs {len(refs)}) at {foff:X}, size {len(tbl):X}, file was {end:X}")
    L(f"total {time.time()-t0:.0f} s")
    mb_stop()
except Exception:
    L("ERROR\n" + traceback.format_exc())
save()
