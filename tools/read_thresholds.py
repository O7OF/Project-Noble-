
import struct
p = r"C:\temp\hr\haloreach.dll"
b = open(p, 'rb').read()
# .rdata: VA 0x91E000, Raw 0x91D200 -> delta 0xE00 ; .data: VA 0xAFA000, Raw 0xAF8600 -> delta 0x1A00
def read_at(rva, delta, fmt='<f'):
    off = rva - delta
    return struct.unpack_from(fmt, b, off)[0], off
for rva, delta, name in [(0xA8B5FC, 0xE00, 'state0 threshold'), (0xA8B618, 0xE00, 'state1 threshold')]:
    v, off = read_at(rva, delta)
    print(f"{name}: rva 0x{rva:X} off 0x{off:X} = {v}")
