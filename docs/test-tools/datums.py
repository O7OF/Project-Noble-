import struct, os, glob
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
PB = 0x50000000
out = []
cands = []
for root in [r"D:\Steam\steamapps\common", r"C:\Program Files (x86)\Steam\steamapps\common", r"E:\SteamLibrary\steamapps\common", r"D:\SteamLibrary\steamapps\common"]:
    cands += glob.glob(os.path.join(root, "Halo The Master Chief Collection", "haloreach", "maps", "20_sword_slayer.map"))
out.append(f"MCC sword: {cands}")
maps = {'rrpack': r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map",
        'menu': r"D:\Steam\steamapps\common\HREK\maps\mainmenu.map"}
if cands:
    d = os.path.dirname(cands[0])
    maps['sword'] = cands[0]; maps['mcc_menu'] = os.path.join(d, 'mainmenu.map'); maps['panopticon'] = os.path.join(d, '50_panopticon.map')
want = ['globals\\globals', 'objects\\characters\\elite\\elite', 'objects\\characters\\masterchief\\mp_masterchief\\mp_masterchief',
        'objects\\characters\\elite\\mp_elite\\mp_elite', 'globals\\player_model_customization', 'objects\\characters\\elite_noble\\elite_noble',
        'objects\\weapons\\rifle\\assault_rifle\\assault_rifle']
def tagnames(m):
    cnt, boff, bsz, ioff = struct.unpack_from('<IIII', m, 0x20)
    buf = m[boff:boff+bsz]
    idx = struct.unpack_from(f'<{cnt}i', m, ioff)
    return [buf[o:buf.index(b'\0', o)].decode('latin1') if o >= 0 else '' for o in idx]
for k, p in maps.items():
    if not os.path.exists(p): out.append(f"{k}: missing"); continue
    m = open(p, 'rb').read()
    toff, tsize = struct.unpack_from('<II', m, 0x10)
    vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
    sec = lambda a, n: m[toff + (a - vbase): toff + (a - vbase) + n]
    th = struct.unpack('<16I', sec(thdr, 64))
    out.append(f"--- {k} hdr10 {toff:X} {tsize:X} vbase {vbase:X} thdr {thdr:X} th " + " ".join(f"{x:X}" for x in th[:8]) + f" filesize {len(m):X} flags18 {struct.unpack_from('<I', m, 0x18)[0]:X}")
    if th[4] < 100: continue
    gcount, gaddr, icount, iaddr = th[0], th[2] | th[3] << 32, th[4], th[6] | th[7] << 32
    groups = [sec(gaddr + 16*g, 4)[::-1].decode('latin1') for g in range(gcount)]
    inst = struct.unpack(f'<{icount*2}I', sec(iaddr, 8*icount))
    names = tagnames(m)
    out.append(f"=== {k}: tags {icount} names {len(names)} strings {struct.unpack_from('<I', m, 0x30)[0]}")
    for i, n in enumerate(names):
        if n in want:
            g = inst[2*i] & 0xFFFF; salt = inst[2*i] >> 16
            out.append(f"  {n}.{groups[g] if g < len(groups) else '?'} datum {salt:04X}{i:04X}")
open(os.path.join(T, 'datums.txt'), 'w').write("\n".join(out))
