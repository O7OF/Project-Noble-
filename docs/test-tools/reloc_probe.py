import struct, os, collections
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
P = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\rr_elite_pack.map"
out = []
def log(s): out.append(str(s))
r = open(P + '.rrreloc', 'rb').read()
magic, ver, cnt, vb, pb, ss = struct.unpack_from('<4sIIQQI', r, 0)
log(f"reloc {magic} v{ver} count {cnt} vbase {vb:X} packedBase {pb:X} sectionSize {ss:X}")
offs = struct.unpack_from(f'<{cnt}I', r, 0x20)
rel = set(offs)
log(f"min {min(offs):X} max {max(offs):X} sorted? {list(offs)==sorted(offs)}")
m = open(P, 'rb').read()
toff, tsize = struct.unpack_from('<II', m, 0x10)
vbase, thdr = struct.unpack_from('<QQ', m, 0x2E0)
log(f"tags file off {toff:X} size {tsize:X} vbase {vbase:X} tags hdr {thdr:X} (rel {thdr-vbase:X})")
sec = m[toff:toff+tsize]
# what do the relocated words look like
vals = [struct.unpack_from('<I', sec, o)[0] for o in offs[:20]]
log("first offs/vals: " + ", ".join(f"{o:X}={v:08X}" for o, v in zip(offs[:20], vals)))
def addr(v): return (v << 2) + pb
inside = lambda v: v and vbase <= addr(v) < vbase + tsize
# check: all reloc'd values point inside section?
bad = sum(1 for o in offs if struct.unpack_from('<I', sec, o)[0] and not inside(struct.unpack_from('<I', sec, o)[0]))
zero = sum(1 for o in offs if struct.unpack_from('<I', sec, o)[0] == 0)
log(f"reloc vals outside {bad} zero {zero}")
# context histogram: (prev word small count?) pattern
ctx = collections.Counter()
for o in offs:
    prev = struct.unpack_from('<I', sec, o-4)[0] if o >= 4 else 0
    ctx['prev_count' if 0 < prev < 0x100000 else ('prev0' if prev == 0 else 'prev_other')] += 1
log(f"context {ctx}")
# heuristic H1: word inside & 4-aligned & prev word is count in 1..0xFFFFF
H = set()
for o in range(0, len(sec) - 4, 4):
    v = struct.unpack_from('<I', sec, o)[0]
    if inside(v):
        H.add(o)
log(f"H_any_inside {len(H)} ; reloc {len(rel)} ; both {len(H & rel)} ; reloc-not-H {len(rel - H)} ; H-not-reloc {len(H - rel)}")
H2 = {o for o in H if 0 < struct.unpack_from('<I', sec, o-4)[0] < 0x100000}
log(f"H2(prev count) {len(H2)} both {len(H2 & rel)} miss {len(rel - H2)} extra {len(H2 - rel)}")
miss = sorted(rel - H2)[:15]
for o in miss:
    w = struct.unpack_from('<8I', sec, o-16)
    log(f"miss {o:X}: " + " ".join(f"{x:08X}" for x in w))
ex = sorted(H - rel)[:15]
for o in ex:
    w = struct.unpack_from('<8I', sec, o-16)
    log(f"extra {o:X}: " + " ".join(f"{x:08X}" for x in w))
open(os.path.join(T, 'reloc_probe.txt'), 'w').write("\n".join(out))
