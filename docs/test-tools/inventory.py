import struct, os, collections
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
maps = {'rrpack': r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map",
        'oursword': r"D:\Steam\steamapps\common\HREK\maps\20_sword_slayer.map"}
out = []
for k, p in maps.items():
    m = open(p, 'rb').read()
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
    gc = collections.Counter()
    sel = []
    for i in range(icount):
        g = inst[2*i] & 0xFFFF
        if g == 0xFFFF: continue
        gn = groups[g]; gc[gn] += 1
        if gn in ('bipd', 'scnr', 'sbsp', 'pmcg', 'matg', 'mulg', 'zone', 'play'):
            sel.append(f"  {gn} {inst[2*i]>>16:04X}{i:04X} {names[i]}")
    out.append(f"=== {k}: size {len(m)} tags {icount} strings {struct.unpack_from('<I', m, 0x30)[0]} tagsec {tsize:X} vbase {vbase:X} type18 {struct.unpack_from('<I', m, 0x18)[0]:X}")
    out.append("  groups: " + ", ".join(f"{g}:{n}" for g, n in gc.most_common(30)))
    out += sel[:40]
    # partitions
    out.append("  sections 0x4DC: " + " ".join(f"{x:X}" for x in struct.unpack_from('<8I', m, 0x4DC)))
open(os.path.join(T, 'inventory.txt'), 'w').write("\n".join(out))
