import struct, os
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
PB = 0x50000000
maps = {'rr': (r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor_rr_parked\rr_elite_pack.map", 'objects\\characters\\elite\\rr_player_elite'),
        'noble': (r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\noble_elite_pack.map", 'objects\\characters\\elite_noble\\elite_noble')}
out = []
H = {}
for k, (p, bname) in maps.items():
    m = open(p, 'rb').read()
    H[k] = m[:0xA000]
    toff, tsize = struct.unpack_from('<II', m, 0x10)
    vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
    sec = lambda a, n: m[toff + (a - vbase): toff + (a - vbase) + n]
    th = struct.unpack('<16I', sec(thdr, 64))
    gcount, gaddr, icount, iaddr = th[0], th[2] | th[3] << 32, th[4], th[6] | th[7] << 32
    groups = [sec(gaddr + 16*g, 4)[::-1].decode('latin1') for g in range(gcount)]
    inst = struct.unpack(f'<{icount*2}I', sec(iaddr, 8*icount))
    cnt, boff, bsz, ioff = struct.unpack_from('<IIII', m, 0x20)
    buf = m[boff:boff+bsz]; idx = struct.unpack_from(f'<{cnt}i', m, ioff)
    names = [buf[o:buf.index(b'\0', o)].decode('latin1') if o >= 0 else '' for o in idx]
    def datum(i): return ((inst[2*i] >> 16) << 16) | i
    def find(n, g):
        for i in range(icount):
            if names[i] == n and groups[inst[2*i] & 0xFFFF] == g: return i
    def data(i, n): return sec((inst[2*i+1] << 2) + PB, n)
    b = find(bname, 'bipd')
    w = struct.unpack('<32I', data(b, 128))
    out.append(f"=== {k}: biped datum {datum(b):08X}; +0x70 ref: {w[0x70//4]:08X} {w[0x74//4]:08X} {w[0x78//4]:08X} {w[0x7C//4]:08X}")
    md = w[0x7C//4]
    out.append(f"  ref datum index {md & 0xFFFF:X} -> local tag {names[md & 0xFFFF] if (md & 0xFFFF) < icount else '?'} datum {datum(md & 0xFFFF):08X} grp {groups[inst[2*(md & 0xFFFF)] & 0xFFFF] if (md & 0xFFFF) < icount else '?'}")
    out.append(f"  matg datum {datum(find('globals\\globals','matg')):08X}  tag count {icount}")
diffs = [o for o in range(0, 0xA000, 4) if H['rr'][o:o+4] != H['noble'][o:o+4]]
out.append("header diffs: " + " ".join(f"{o:X}:{struct.unpack_from('<I', H['rr'], o)[0]:X}/{struct.unpack_from('<I', H['noble'], o)[0]:X}" for o in diffs[:80]))
open(os.path.join(T, 'refcheck.txt'), 'w').write("\n".join(out))
