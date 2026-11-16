# Give a tool-built cache an (empty) load-fixup table like RR's pack has: header +0x97A8 = 1,
# +0x97AC = size before, +0x97B0 = table offset, +0x97B4 = table size. The engine's cache step reads it
# whenever the cache loads away from its own base (count at table +8 = 0 -> nothing to fix).
import struct, os
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
P = os.environ.get('PACK', r"D:\Project-Noble-Closed-Source\build\bin\Release\data\armor\noble_elite_pack.map")
out = []
with open(P, 'r+b') as f:
    f.seek(0x97A8); flag, before, off, size = struct.unpack('<IIII', f.read(16))
    if size:
        out.append(f"already has a fixup table: flag {flag} before {before:X} off {off:X} size {size:X}")
    else:
        f.seek(0, 2); end = f.tell()
        off = (end + 0xFFF) & ~0xFFF
        f.write(b'\0' * (off - end + 0x1000))
        f.seek(0x97A8); f.write(struct.pack('<IIII', 1, end, off, 0x10))
        out.append(f"added empty fixup table at {off:X} (file was {end:X}, now {off + 0x1000:X})")
open(os.path.join(T, 'add_fixup.txt'), 'w').write("\n".join(out))
