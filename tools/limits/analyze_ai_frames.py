#!/usr/bin/env python3
"""Static audit of fixed-size stack tables in Halo: Reach's AI code (AI-LIMITS-HANDOFF.md / TODO 3).

Disassembles every function in the requested RVA range with capstone (no objdump needed), and reports:

* the frame size (__chkstk immediate, or a 'sub rsp, imm'),
* the stack cookie slot and the buffer that ends at it,
* every small constant compare (cmp reg, 0x60/0x80/0x100/0x200/0x400) and 'mov reg, imm32' of those values,
* whether the function indexes the 0xD00-byte actor record ('imul reg, reg, 0xD00') or walks 0x2C entries,
* whether it calls data_array_new (0x12FCC) with a fixed count.

Usage:
  python analyze_ai_frames.py [--function RVA] [--range LO HI] [--dll FILE]

With --function it prints the aligned disassembly of that function and a summary; without it, the audit table.
The default DLL is the player's installed stock haloreach.dll (read only, never modified).
"""
from __future__ import annotations

import argparse
import struct
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_64, Cs

STOCK_DLL = Path(r"d:\steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll")
CHKSTK = 0x86C2D0         # __chkstk
DATA_ARRAY_NEW = 0x12FCC  # data_array_new(name, count, recordSize, align, allocator)
SMALL_COMPARES = (0x60, 0x80, 0xC0, 0x100, 0x200, 0x400, 0x800)


class Image:
    def __init__(self, path: Path):
        self.pe = pefile.PE(str(path), fast_load=True)
        self.img = self.pe.get_memory_mapped_image()   # RVA-indexed
        self.md = Cs(CS_ARCH_X86, CS_MODE_64)
        self.md.detail = True
        self.functions = self._functions()

    def _functions(self):
        out = []
        for s in self.pe.sections:
            if s.Name.rstrip(b"\x00") != b".pdata":
                continue
            raw = self.img[s.VirtualAddress:s.VirtualAddress + s.Misc_VirtualSize]
            for off in range(0, len(raw) - 12 + 1, 12):
                begin, end, _unwind = struct.unpack_from("<III", raw, off)
                if begin:
                    out.append((begin, end))
        return sorted(set(out))

    def function_at(self, rva):
        for begin, end in self.functions:
            if begin <= rva < end:
                return begin, end
        return None

    def disasm(self, begin, end):
        return list(self.md.disasm(bytes(self.img[begin:end]), begin))


def imm_values(insn):
    return [op.imm for op in insn.operands if op.type == 2]   # CS_OP_IMM


def analyze_function(img, begin, end):
    info = {"begin": begin, "end": end, "frame": 0, "cookie_off": None, "caps": [], "moves": [],
            "actor": False, "entry_2c": False, "array_new": False, "chkstk": False, "insns": 0,
            "low_disp": 0}
    insns = img.disasm(begin, end)
    info["insns"] = len(insns)
    for i, insn in enumerate(insns):
        ops = insn.op_str
        if insn.mnemonic == "call" and ops.startswith("0x"):
            target = int(ops, 16)
            if target == CHKSTK:
                info["chkstk"] = True
                for back in range(i - 1, max(i - 5, -1), -1):
                    if insns[back].mnemonic == "mov" and insns[back].op_str.startswith("eax, 0x"):
                        try:
                            info["frame"] = int(insns[back].op_str.split(", ")[1], 16)
                        except ValueError:
                            pass
                        break
            if target == DATA_ARRAY_NEW:
                info["array_new"] = True
        if insn.mnemonic == "sub" and ops.startswith("rsp, 0x"):
            try:
                info["frame"] = max(info["frame"], int(ops.split(", ")[1], 16))
            except ValueError:
                pass
        if insn.mnemonic == "imul" and ops.endswith("0xd00"):
            info["actor"] = True
        if insn.mnemonic == "imul" and ops.endswith("0x2c"):
            info["entry_2c"] = True
        if insn.mnemonic == "add" and ops.endswith("0x2c"):
            info["entry_2c"] = True
        if insn.mnemonic == "mov" and ("[rbp + 0x" in ops or "[rbp - 0x" in ops) and ops.endswith(", rax"):
            disp_text = ops.split("[rbp")[1].split("]")[0].replace(" ", "")
            try:
                disp = int(disp_text.replace("+", ""), 16)
                if disp > 0x400:
                    info["cookie_off"] = disp
            except ValueError:
                pass
        # lowest rbp-relative negative displacement: with the cookie slot it brackets the biggest local array
        if "[rbp - 0x" in ops:
            tag = ops.split("[rbp - 0x")[1].split("]")[0].split(" + ")[0].split(" * ")[0]
            try:
                info["low_disp"] = max(info["low_disp"], int(tag, 16))
            except ValueError:
                pass
        if insn.mnemonic == "cmp":
            for v in imm_values(insn):
                if v in SMALL_COMPARES:
                    info["caps"].append((insn.address, v))
        if insn.mnemonic == "mov" and ops.split(",")[0] in ("edi", "esi", "eax", "edx", "ecx"):
            for v in imm_values(insn):
                if v in SMALL_COMPARES:
                    info["moves"].append((insn.address, v))
    return info


def audit(img: Image, lo: int, hi: int, min_frame: int = 0x400):
    """Functions worth a look: a stack frame at least min_frame, and something that ties it to a table
    (a small constant compare/move, the 0x2C entry stride, or the actor record iterator)."""
    rows = []
    for begin, end in img.functions:
        if not (lo <= begin < hi):
            continue
        info = analyze_function(img, begin, end)
        if info["frame"] < min_frame:
            continue
        if not (info["caps"] or info["moves"] or info["actor"] or info["entry_2c"] or info["array_new"]):
            continue
        rows.append(info)
    return rows


def describe(info):
    caps = ", ".join(f"0x{v:X}@0x{a:X}" for a, v in info["caps"])
    moves = ", ".join(f"mov 0x{v:X}@0x{a:X}" for a, v in info["moves"])
    bits = []
    if info["frame"]:
        bits.append(f"frame 0x{info['frame']:X}")
    if info["cookie_off"] is not None:
        bits.append(f"cookie rbp+0x{info['cookie_off']:X}")
    if info["low_disp"] and info["cookie_off"] is not None:
        bits.append(f"span 0x{info['cookie_off'] + info['low_disp']:X}")
    if info["chkstk"]:
        bits.append("__chkstk")
    if info["actor"]:
        bits.append("actor(0xD00)")
    if info["entry_2c"]:
        bits.append("entry 0x2C")
    if info["array_new"]:
        bits.append("data_array_new")
    head = f"0x{info['begin']:X}-0x{info['end']:X} ({info['end'] - info['begin']} B, {info['insns']} insns) {', '.join(bits)}"
    return head + "\n    caps: " + (caps or "-") + "\n    moves: " + (moves or "-")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dll", type=Path, default=STOCK_DLL)
    ap.add_argument("--function", type=lambda v: int(v, 0), default=None)
    ap.add_argument("--range", nargs=2, type=lambda v: int(v, 0), default=(0x690000, 0x780000))
    ap.add_argument("--min-frame", type=lambda v: int(v, 0), default=0x400)
    a = ap.parse_args()

    img = Image(a.dll)
    print(f"{a.dll}: {len(img.functions)} functions in .pdata")
    if a.function is not None:
        hit = img.function_at(a.function)
        if not hit:
            print(f"0x{a.function:X} is not inside a known function")
            return 1
        begin, end = hit
        print(f"function 0x{begin:X}-0x{end:X}")
        for insn in img.disasm(begin, end):
            print(f"0x{insn.address:X}: {insn.bytes.hex():<22} {insn.mnemonic} {insn.op_str}")
        print()
        print(describe(analyze_function(img, begin, end)))
        return 0

    rows = audit(img, a.range[0], a.range[1], a.min_frame)
    print(f"\n{len(rows)} candidate functions in 0x{a.range[0]:X}-0x{a.range[1]:X}\n")
    for info in rows:
        print(describe(info))
        print()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
