#!/usr/bin/env python3
"""Dev tool: list every game address (haloreach.dll RVA) Noble's source uses and check them against a DLL.

After an MCC update, run it with the old and the new haloreach.dll to see which addresses still point at the same
bytes and which need to be found again:

    python tools/check-addresses.py --dll OLD\\haloreach.dll --new-dll NEW\\haloreach.dll

With only --dll it checks that every address is inside the DLL and says what it points at (code / function start /
read-only data / writable data). Output stays on your PC; never paste it into a release or public file.
"""
import argparse
import os
import re
import struct
import sys
from collections import defaultdict

SRC_EXT = (".cpp", ".hpp", ".h", ".inl")
# Hex literal on a line that talks about the game module: named RVA constants, base + 0x..., hr+0x..., module checks.
NAMED = re.compile(r"\b(k\w+)\s*=\s*0x([0-9A-Fa-f]{4,8})\b")
BASED = re.compile(r"(?:base|Base|module|Module|hReach|reach|Reach|hr|mod|image|Image)\w*\)?\s*\+\s*0x([0-9A-Fa-f]{4,8})\b")
HRREF = re.compile(r"\bhr\s*\+\s*0x([0-9A-Fa-f]{4,8})\b")
SIZECHK = re.compile(r"\b0x([0-9A-Fa-f]{4,8})\s*(?:\+\s*sizeof\([^)]*\)\s*)?<=?\s*modInfo\.SizeOfImage")
NOT_RVA_NAMES = re.compile(r"(Offset|Size|Stride|Reserve|Mask|Magic|Flag|Count|Max|Min|Bytes|Len|Tag)$")


class Pe:
    def __init__(self, path):
        with open(path, "rb") as f:
            self.data = f.read()
        d = self.data
        pe = struct.unpack_from("<I", d, 0x3C)[0]
        if d[pe:pe + 4] != b"PE\0\0":
            raise ValueError(f"{path}: not a PE file")
        nsec, = struct.unpack_from("<H", d, pe + 6)
        self.timestamp, = struct.unpack_from("<I", d, pe + 8)
        optsize, = struct.unpack_from("<H", d, pe + 20)
        opt = pe + 24
        magic, = struct.unpack_from("<H", d, opt)
        if magic != 0x20B:
            raise ValueError(f"{path}: expected a 64-bit DLL")
        self.size_of_image, = struct.unpack_from("<I", d, opt + 56)
        exc_rva, exc_size = struct.unpack_from("<II", d, opt + 112 + 3 * 8)
        self.sections = []
        sec = opt + optsize
        for i in range(nsec):
            name = d[sec + 40 * i:sec + 40 * i + 8].rstrip(b"\0").decode("ascii", "replace")
            vsize, va, rawsize, rawptr = struct.unpack_from("<IIII", d, sec + 40 * i + 8)
            chars, = struct.unpack_from("<I", d, sec + 40 * i + 36)
            self.sections.append((name, va, max(vsize, rawsize), rawptr, rawsize, chars))
        self.func_starts = set()
        if exc_rva and exc_size:
            off = self.rva_to_off(exc_rva)
            if off is not None:
                for k in range(exc_size // 12):
                    b, = struct.unpack_from("<I", d, off + 12 * k)
                    self.func_starts.add(b)

    def section_of(self, rva):
        for s in self.sections:
            if s[1] <= rva < s[1] + s[2]:
                return s
        return None

    def rva_to_off(self, rva):
        s = self.section_of(rva)
        if not s or rva - s[1] >= s[4]:
            return None
        return s[3] + (rva - s[1])

    def bytes_at(self, rva, n=16):
        off = self.rva_to_off(rva)
        if off is None:
            return None
        return self.data[off:off + n]

    def kind(self, rva):
        s = self.section_of(rva)
        if not s:
            return "OUTSIDE"
        name, chars = s[0], s[5]
        if chars & 0x20000000:  # executable
            return "function start" if rva in self.func_starts else "code"
        if chars & 0x80000000:  # writable
            return "writable data"
        return "read-only data"


def scan_sources(src_root, image_size):
    found = defaultdict(list)  # rva -> [(file, line, name)]
    for root, _, files in os.walk(src_root):
        for fn in files:
            if not fn.endswith(SRC_EXT):
                continue
            path = os.path.join(root, fn)
            rel = os.path.relpath(path, os.path.dirname(src_root))
            with open(path, "r", encoding="utf-8", errors="replace") as f:
                for ln, line in enumerate(f, 1):
                    hits = []
                    for m in NAMED.finditer(line):
                        name = m.group(1)
                        if NOT_RVA_NAMES.search(name) and "Rva" not in name:
                            continue
                        hits.append((int(m.group(2), 16), name))
                    for rx in (BASED, HRREF, SIZECHK):
                        for m in rx.finditer(line):
                            hits.append((int(m.group(1), 16), ""))
                    for rva, name in hits:
                        if 0x1000 <= rva < image_size:
                            found[rva].append((rel, ln, name))
    return found


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dll", required=True, help="haloreach.dll Noble was made for")
    ap.add_argument("--new-dll", help="haloreach.dll from an MCC update, to compare against")
    ap.add_argument("--src", default=os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src"))
    ap.add_argument("--bytes", type=int, default=16, help="bytes compared per address (default 16)")
    args = ap.parse_args()

    old = Pe(args.dll)
    new = Pe(args.new_dll) if args.new_dll else None
    found = scan_sources(os.path.abspath(args.src), old.size_of_image)

    print(f"DLL: {args.dll}  (timestamp {old.timestamp:08X}, image size {old.size_of_image:X})")
    if new:
        print(f"New DLL: {args.new_dll}  (timestamp {new.timestamp:08X}, image size {new.size_of_image:X})")
        if new.timestamp == old.timestamp and new.data == old.data:
            print("The two DLLs are identical: every address is still valid.")
    print(f"{len(found)} distinct game addresses found in the source.\n")

    counts = defaultdict(int)
    rows = []
    for rva in sorted(found):
        kind = old.kind(rva)
        status = kind
        if new is not None:
            if kind in ("writable data",):
                status = "data (check by hand)"
            else:
                a, b = old.bytes_at(rva, args.bytes), new.bytes_at(rva, args.bytes)
                if a is None or b is None:
                    status = "CHANGED (not in file)"
                else:
                    status = "same" if a == b else "CHANGED"
        counts[status] += 1
        where = "; ".join(f"{f}:{l}{(' ' + n) if n else ''}" for f, l, n in found[rva][:4])
        more = f" (+{len(found[rva]) - 4} more)" if len(found[rva]) > 4 else ""
        rows.append((status, rva, kind, where + more))

    order = {"CHANGED": 0, "CHANGED (not in file)": 0, "OUTSIDE": 0, "data (check by hand)": 1}
    rows.sort(key=lambda r: (order.get(r[0], 2), r[1]))
    for status, rva, kind, where in rows:
        print(f"{status:<22} hr+0x{rva:07X}  {kind:<15} {where}")
    print("\nSummary: " + ", ".join(f"{k}: {v}" for k, v in sorted(counts.items())))
    if new is not None and any(k.startswith("CHANGED") for k in counts):
        print("Addresses marked CHANGED must be found again in the new DLL before a release.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
