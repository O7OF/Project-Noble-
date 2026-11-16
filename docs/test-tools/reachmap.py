import struct, sys, os
class Map:
    def __init__(s, d):
        r = lambda n: open(os.path.join(d, n), 'rb').read()
        s.h = r('header.bin'); s.tags = r('tags.bin')
        s.vbase = struct.unpack_from('<Q', s.h, 0x2E0)[0]
        s.idx = struct.unpack_from('<Q', s.h, 0x2E8)[0]
        # tag names
        fi = r('fileidx.bin'); ft = r('filetab.bin')
        s.names = [ft[o:ft.index(b'\0', o)].decode('latin1') if o >= 0 else '' for o in struct.unpack('<%di' % (len(fi)//4), fi)]
        si = r('stridx.bin'); st = r('strtab.bin')
        s.strs = [st[o:st.index(b'\0', o)].decode('latin1') if o >= 0 else '' for o in struct.unpack('<%di' % (len(si)//4), si)]
        s.ns = list(struct.unpack('<%di' % (len(r('nstab.bin'))//4), r('nstab.bin')))
    def off(s, addr): return addr - s.vbase
    def ptr(s, p32): return 0 if p32 == 0 else p32*4 + 0x50000000
    def u32(s, a, o=0): return struct.unpack_from('<I', s.tags, s.off(a)+o)[0]
    def raw(s, a, n): o = s.off(a); return s.tags[o:o+n]
    def sid(s, v):
        # 8 ns bits, 17 index bits (Reach)
        if v == 0: return ''
        ns = (v >> 17) & 0xFF; i = v & 0x1FFFF
        try:
            if ns == 0 and i >= s.ns[1]: return s.strs[i + sum(s.ns[1:]) - s.ns[1] + s.ns[0] - s.ns[0]] if False else s.strs[i - s.ns[1] + sum(s.ns)] if False else None
        except: pass
        base = sum(s.ns[:ns]) if ns > 0 else 0
        try: return s.strs[base + i]
        except: return '<sid %08X>' % v
    def load_index(s):
        o = s.off(s.idx); h = s.tags
        ng, ga, nt, ta = struct.unpack_from('<iiQ', h, o)[0], None, None, None
        vals = struct.unpack_from('<iIQiIQ', h, o)
        return o, h[o:o+0x60]
if __name__ == '__main__':
    m = Map(sys.argv[1])
    print(hex(m.vbase), hex(m.idx), len(m.names), len(m.strs), m.ns)
    o, b = m.load_index(); print(hex(o), b.hex(' ', 4))
