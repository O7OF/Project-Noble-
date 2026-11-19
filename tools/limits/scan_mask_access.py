#!/usr/bin/env python3
"""Dev tool (64-player work): list every instruction that reads or writes the engine's 32-bit peer/player valid masks.

Clean-room: works only from our own MCC decompile text and the haloreach.dll it was made from.
1. Finds the functions that bit-test the masks in the decompile text:
     peer-valid mask   session + 0x5C   ( `+ 0x5c) >> (i & 0x1f) & 1` )
     player-valid mask block   + 0x170C ( and block B + 0x391C )
2. Disassembles those functions (capstone, function bounds from .pdata) and lists every instruction whose memory
   operand uses that displacement, with the access kind, plus the loop-bound compares in the same function.

    python tools/limits/scan_mask_access.py --dll <haloreach.dll> --decomp <decompiled dir> --out <report.md>

Needs: pip install capstone. The report contains engine addresses: private repo only, never public.
"""
import argparse
import glob
import os
import re
import struct
from collections import defaultdict

import capstone
from capstone import x86

BASE = 0x180000000
FUNC_HDR = re.compile(r"^// ==== FUN_([0-9a-f]+)")
PATTERNS = {
    "peer": re.compile(r"\+ 0x5c\) >> \(\w+ & 0x1f\) & 1"),
    "player": re.compile(r"\+ 0x(170c|391c)\) (>> \(\w+ & 0x1f\) & 1|[|&] |=)"),
}
DISPS = {"peer": {0x5C}, "player": {0x170C, 0x391C}}
# Network session / membership / simulation code. Functions outside it that use the same offsets belong to other
# structures (checked by hand 2026-11-19: hr+0x779634..0x77CA94 adjust a counter at +0x170C).
SESSION_CODE = (0x380000, 0x460000)
WRITE_MNEMS = {"mov", "or", "and", "xor", "bts", "btr", "btc", "add", "sub", "inc", "dec", "not", "lock or", "lock and"}
BIT_MNEMS = {"bt", "bts", "btr", "btc"}
SCAN_MNEMS = {"bsf", "bsr", "tzcnt", "lzcnt", "popcnt"}


def load_pe(path):
    d = open(path, "rb").read()
    pe = struct.unpack_from("<I", d, 0x3C)[0]
    nsec, = struct.unpack_from("<H", d, pe + 6)
    optsize, = struct.unpack_from("<H", d, pe + 20)
    opt = pe + 24
    exc_rva, exc_size = struct.unpack_from("<II", d, opt + 112 + 3 * 8)
    secs = []
    for i in range(nsec):
        o = opt + optsize + 40 * i
        vsize, va, rawsize, rawptr = struct.unpack_from("<IIII", d, o + 8)
        secs.append((va, max(vsize, rawsize), rawptr, rawsize))

    def off(rva):
        for va, sz, rp, rs in secs:
            if va <= rva < va + sz and rva - va < rs:
                return rp + rva - va
        return None

    funcs = {}
    eo = off(exc_rva)
    for k in range(exc_size // 12):
        b, e, _ = struct.unpack_from("<III", d, eo + 12 * k)
        funcs.setdefault(b, e)
    return d, off, funcs


def functions_from_decompile(decomp):
    found = defaultdict(set)  # kind -> {rva}
    for path in sorted(glob.glob(os.path.join(decomp, "hr_*.c"))):
        cur = None
        with open(path, "r", encoding="utf-8", errors="replace") as f:
            for line in f:
                m = FUNC_HDR.match(line)
                if m:
                    cur = int(m.group(1), 16) - BASE
                    continue
                if cur is None:
                    continue
                if not (SESSION_CODE[0] <= cur < SESSION_CODE[1]):
                    continue  # same offsets in unrelated structures elsewhere (e.g. a counter at +0x170C)
                for kind, rx in PATTERNS.items():
                    if rx.search(line):
                        found[kind].add(cur)
    return found


def access_kind(insn, op_index):
    m = insn.mnemonic
    if m in BIT_MNEMS:
        return "bit-test" if m == "bt" else "bit-write"
    if m in SCAN_MNEMS:
        return "bit-scan"
    if op_index == 0 and m in WRITE_MNEMS:
        return "write" if m == "mov" else "read-modify-write"
    if m in ("test", "cmp"):
        return "test"
    return "read"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dll", required=True)
    ap.add_argument("--decomp", required=True)
    ap.add_argument("--out", required=True)
    a = ap.parse_args()

    data, off, funcs = load_pe(a.dll)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
    md.detail = True
    kinds = functions_from_decompile(a.decomp)

    rows = []
    bounds = defaultdict(set)
    for kind, fset in sorted(kinds.items()):
        for fn in sorted(fset):
            end = funcs.get(fn)
            fo = off(fn)
            if end is None or fo is None:
                continue
            code = data[fo:fo + (end - fn)]
            for insn in md.disasm(code, BASE + fn):
                for i, op in enumerate(insn.operands):
                    if op.type == x86.X86_OP_MEM and op.mem.disp in DISPS[kind] and op.size in (4, 8):
                        rows.append((kind, fn, insn.address - BASE, insn.mnemonic, insn.op_str, access_kind(insn, i)))
                if insn.mnemonic == "cmp" and len(insn.operands) == 2 and insn.operands[1].type == x86.X86_OP_IMM:
                    v = insn.operands[1].imm
                    if v in (0x10, 0x11, 0x1F, 0x20, 0x21):
                        bounds[fn].add(v)

    by_kind = defaultdict(int)
    by_access = defaultdict(int)
    for r in rows:
        by_kind[r[0]] += 1
        by_access[(r[0], r[5])] += 1
    with open(a.out, "w", encoding="utf-8", newline="\r\n") as f:
        f.write("# Mask access inventory for 64 players (generated)\n\n")
        f.write("Generated by `tools/limits/scan_mask_access.py` from our own MCC decompile + haloreach.dll (clean-room).\n")
        f.write("Every instruction in the mask functions whose memory operand is the peer-valid mask (session +0x5C) or\n")
        f.write("the player-valid mask (block +0x170C / +0x391C). Private repo only: contains engine addresses.\n\n")
        f.write(f"Functions: peer {len(kinds.get('peer', ()))}, player {len(kinds.get('player', ()))}. ")
        f.write(f"Sites: peer {by_kind['peer']}, player {by_kind['player']}.\n\n")
        f.write("| mask | access | count |\n|---|---|---|\n")
        for (k, acc), n in sorted(by_access.items()):
            f.write(f"| {k} | {acc} | {n} |\n")
        f.write("\n| mask | function | site | instruction | access | loop bounds in function |\n|---|---|---|---|---|---|\n")
        for kind, fn, site, mn, ops, acc in rows:
            b = " ".join(f"0x{v:X}" for v in sorted(bounds.get(fn, ())))
            f.write(f"| {kind} | hr+0x{fn:X} | hr+0x{site:X} | `{mn} {ops}` | {acc} | {b} |\n")
    print(f"peer functions {len(kinds.get('peer', ()))}, player functions {len(kinds.get('player', ()))}; "
          f"sites: peer {by_kind['peer']}, player {by_kind['player']}")
    for (k, acc), n in sorted(by_access.items()):
        print(f"  {k:<6} {acc:<18} {n}")


if __name__ == "__main__":
    main()
