#!/usr/bin/env python3
"""Light-volume V4 port onto Noble's V16 (+particles) build (profile 3 "Full").

Data comes from tools/limits/recipes/light_volumes (GreenKnight V4). The V4 section (.lvlimit) cannot be
added here (all ten PE section slots are used), so the seven relocated routines and five small blocks are
placed in the free room at the end of V16's executable .deltrac section, and every relative reference is
recomputed for the new address. The storage buffers V4 puts in its virtual section (transparent records and
indices) go at the end of .hkwork instead.

Public API:
    code, addrs = build_code(recipes_dir, new_base, particle=True)
    rows = lv_rows(guide_path)
    apply_rows(img, rows, addrs, records_rva, indices_rva)
"""
from __future__ import annotations

import re
import struct
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_64, Cs

APPEND_NAME = "V4_append_at_00C9DBD8.bin"
GUIDE_NAME = "Halo_Reach_Light_Volume_V4_Hex_Guide.md"
LV_FILE0 = 0xC9DC00          # first byte of V4's section raw data (file offset)
LV_RVA0 = 0x4EDA000          # V4 section RVA

# (name, original RVA, V4 file offset, byte length)
ROUTINES = [
    ("effect_init", 0x115888, 0xC9DE00, 1302),
    ("msg_admit", 0x121AC4, 0xC9E320, 123),
    ("msg_reset", 0x121B3C, 0xC9E3A0, 183),
    ("msg_reloc", 0x121BE4, 0xC9E460, 347),
    ("msg_consume", 0x121D20, 0xC9E5C0, 194),
    ("msg_append", 0x122854, 0xC9E690, 172),
    ("msg_filter", 0x1228F4, 0xC9E740, 190),
]
BLOCKS = [
    ("sort_guard", 0xC9DC00, 31),   # 7+6+7+6+5 bytes
    ("lv_alloc5", 0xC9DD30, 10),
    ("gpu_row", 0xC9DD40, 10),
    ("gpu_param", 0xC9DD50, 10),
    ("parent_admit", 0xC9DD60, 16),   # 5+6+5 bytes
]
PLACEMENT = ["sort_guard", "lv_alloc5", "gpu_row", "gpu_param", "parent_admit",
             "effect_init", "msg_admit", "msg_reset", "msg_reloc", "msg_consume", "msg_append", "msg_filter"]

# The three particle-pool operands inside the relocated effect init: V4 file offset and the particle profile value.
EFFECT_INIT_PARTICLE_OPS = [
    (0xC9E10B, 0x1200),   # systems/locations: 576 -> 4,608
    (0xC9E15A, 0x4000),   # CPU particles: 2,048 -> 16,384
    (0xC9E189, 0x1A00),   # CPU emitters: 832 -> 6,656
]

BRANCHES = {"call", "jmp", "je", "jne", "jb", "jae", "jbe", "ja", "jl", "jge", "jle", "jg",
            "js", "jns", "jo", "jno", "jp", "jnp", "jcxz", "jecxz", "jrcxz"}
MD = Cs(CS_ARCH_X86, CS_MODE_64)
MD.detail = True


def _read_append(recipes: Path) -> bytes:
    bin_ = (recipes / "light_volumes" / APPEND_NAME).read_bytes()
    want = "e57941fb9ffe4e37bc1de08b79d74065070c6447f9f38b40a0abdb2c6c6edc2c"
    import hashlib
    if hashlib.sha256(bin_).hexdigest() != want:
        raise ValueError("V4 append file does not match the guide hash")
    return bin_


def _piece_bytes(bin_: bytes, file_off: int, length: int) -> bytes:
    start = file_off - 0xC9DBD8
    data = bin_[start:start + length]
    if len(data) != length:
        raise ValueError(f"append slice 0x{file_off:X} short")
    return data


def build_code(recipes: Path, new_base: int, particle: bool = True) -> tuple[bytes, dict[str, int]]:
    """Place all pieces at new_base (16-byte aligned) and relocate every branch / RIP-relative reference."""
    bin_ = _read_append(recipes)
    pieces = {}
    for name, _orig, foff, length in ROUTINES:
        pieces[name] = (LV_RVA0 + (foff - LV_FILE0), _piece_bytes(bin_, foff, length))
    for name, foff, length in BLOCKS:
        pieces[name] = (LV_RVA0 + (foff - LV_FILE0), _piece_bytes(bin_, foff, length))

    addrs: dict[str, int] = {}
    code = bytearray()
    for name in PLACEMENT:
        while len(code) % 16:
            code.append(0xCC)
        addrs[name] = new_base + len(code)
        code += pieces[name][1]

    # Map an old V4 code address to its new address (stock targets are returned unchanged).
    ranges = [(old, old + len(data), addrs[name]) for name, (old, data) in pieces.items()]

    def map_code_target(t: int) -> int:
        for old, end, new in ranges:
            if old <= t < end:
                return new + (t - old)
        return t

    for name in PLACEMENT:
        old_base, data = pieces[name]
        new_start = addrs[name]
        buf = bytearray(data)
        for insn in MD.disasm(bytes(data), old_base):
            off = insn.address - old_base
            # Branch: rel32 at the end of the instruction.
            if insn.mnemonic in BRANCHES and insn.operands and insn.operands[0].type == 2:
                target = map_code_target(insn.operands[0].imm)
                new_end = new_start + off + insn.size
                struct.pack_into("<i", buf, off + insn.imm_offset, target - new_end)
                continue
            # RIP-relative memory operand: disp32 at the end of the instruction.
            if insn.operands and insn.operands[0].type == 3 or (len(insn.operands) > 1 and insn.operands[1].type == 3):
                for op in insn.operands:
                    if op.type == 3 and op.mem.base == 41 and op.mem.disp != 0:
                        old_target = insn.address + insn.size + op.mem.disp
                        target = map_code_target(old_target)
                        new_end = new_start + off + insn.size
                        struct.pack_into("<i", buf, off + insn.disp_offset, target - new_end)
                        break
        data = bytes(buf)
        code[addrs[name] - new_base:addrs[name] - new_base + len(data)] = data

    if particle:
        for foff, value in EFFECT_INIT_PARTICLE_OPS:
            routine_off = foff - 0xC9DE00
            pos = addrs["effect_init"] - new_base + routine_off
            old = struct.unpack_from("<I", bytes(code), pos)[0]
            if old not in (0x240, 0x800, 0x340):   # V4 leaves the stock values here
                raise ValueError(f"effect-init particle operand at 0x{foff:X} is 0x{old:X}, expected stock 0x240/0x800/0x340")
            struct.pack_into("<I", code, pos, value)

    return bytes(code), addrs


def lv_rows(guide: Path):
    """(file offset, original bytes, replacement bytes, description) for every table row in the guide."""
    text = guide.read_text(encoding="utf-8")
    out = []
    for m in re.finditer(r"^\| \`([0-9A-F]{8})\` \| \`[0-9A-F]{8}\` \| ([^|]+)\| \`([0-9A-F ]+)\` \| \`([0-9A-F ]+)\` \|", text, re.M):
        off = int(m.group(1), 16)
        desc = m.group(2).strip()
        old = bytes.fromhex(m.group(3))
        new = bytes.fromhex(m.group(4))
        out.append((off, old, new, desc))
    return out


def _rva_from_desc(desc: str):
    m = re.search(r"RVA 0x([0-9a-fA-F]+) to 0x([0-9a-fA-F]+)", desc)
    if not m:
        return None
    return int(m.group(1), 16), int(m.group(2), 16)


LV_QUEUE_RVA0 = 0xCD90040   # V4's effect-message queue control block; stock refs point at 0x2497160..0x2497184


def apply_rows(img: bytearray, rows, addrs: dict[str, int], records_rva: int, indices_rva: int,
               queue_rva: int = 0) -> list[str]:
    """Apply every LV overwrite row that is not a PE-header / bank / snapshot row. Returns applied descriptions."""
    skip_kw = ("checkpoint", "game state", "game-state", "render bank", "main bank", "snapshot buffer")
    applied = []
    for off, old, new, desc in rows:
        if off < 0x400:
            continue
        if off == 0xBB7120:      # absolute pointer, handled at the end (needs the new records RVA)
            continue
        if any(k in desc.lower() for k in skip_kw):
            continue
        if "snapshot reference" in desc.lower():
            # V16 already repointed every reference to the two checkpoint buffers (STATE_BANKS); V4's copies of
            # those snapshots are not used. (Bisect note: patching these rows breaks the boot because the stock
            # bytes are no longer there.)
            continue
        cur = bytes(img[off:off + len(old)])
        if cur != old:
            raise ValueError(f"LV row 0x{off:X} ({desc}): image has {cur.hex(' ')}, guide expects {old.hex(' ')}")
        new = bytearray(new)
        pair = _rva_from_desc(desc)
        # Transparent draw array references: reseat the RIP displacement on our storage buffers.
        if pair and "draw array reference" not in desc.lower():
            old_target, v4_target = pair
            if LV_QUEUE_RVA0 <= v4_target < 0xCD91000 and queue_rva:
                target = queue_rva + (v4_target - LV_QUEUE_RVA0)
                insn_end = old_target - struct.unpack("<i", old)[0]
                struct.pack_into("<i", new, 0, target - insn_end)
                img[off:off + len(new)] = new
                applied.append(desc)
                continue
        if pair and "draw array reference" in desc.lower():
            old_target, v4_target = pair
            if v4_target == 0xD051000:
                target = indices_rva
            else:
                target = records_rva + (v4_target - 0xCD91000)
            if struct.unpack("<I", old)[0] == old_target:
                # Absolute 32-bit displacement inside a memory operand (e.g. [rsi+rax*2+base]): write the RVA.
                new = bytearray(struct.pack("<I", target))
            else:
                insn_end = old_target - struct.unpack("<i", old)[0]
                struct.pack_into("<i", new, 0, target - insn_end)
        # Branch rel32 at the start of the replacement (E8/E9 or 0F 8x): retarget into the new code.
        elif new[0] in (0xE8, 0xE9) and len(new) >= 5:
            insn_len = len(new)
            v4_target = off + 0xC00 + insn_len + struct.unpack("<i", new[1:5])[0]
            struct.pack_into("<i", new, 1, _map_code(v4_target, addrs) - (off + 0xC00 + insn_len))
        elif new[0] == 0x0F and 0x80 <= new[1] <= 0x8F and len(new) >= 6:
            insn_len = len(new)
            v4_target = off + 0xC00 + insn_len + struct.unpack("<i", new[2:6])[0]
            struct.pack_into("<i", new, 2, _map_code(v4_target, addrs) - (off + 0xC00 + insn_len))
        img[off:off + len(new)] = new
        applied.append(desc)
    # Absolute transparent-record pointer (.data, ASLR relocation entry stays on the instruction).
    off = 0xBB7120
    old = bytes.fromhex("C0 3B CC 80 01 00 00 00")
    if bytes(img[off:off + 8]) != old:
        raise ValueError("LV pointer row 0xBB7120: image bytes differ from stock")
    struct.pack_into("<Q", img, off, 0x180000000 + records_rva)
    applied.append("transparent record pointer")
    return applied


def _piece_len(name: str) -> bytes:
    for n, _o, _f, length in ROUTINES:
        if n == name:
            return bytes(length)
    for n, _f, length in BLOCKS:
        if n == name:
            return bytes(length)
    raise KeyError(name)


def _map_code(t: int, addrs: dict[str, int]) -> int:
    """V4 code RVA -> new RVA (pieces are contiguous in PLACEMENT order with 16-byte alignment)."""
    # Recompute the old V4 base of each placed piece from the known layout.
    off = 0
    for name in PLACEMENT:
        old = _piece_old_base(name)
        length = len(_piece_len(name))
        new = addrs[name]
        if old <= t < old + length:
            return new + (t - old)
        off += length
    return t


def _piece_old_base(name: str) -> int:
    for n, _orig, foff, _l in ROUTINES:
        if n == name:
            return LV_RVA0 + (foff - LV_FILE0)
    for n, foff, _l in BLOCKS:
        if n == name:
            return LV_RVA0 + (foff - LV_FILE0)
    raise KeyError(name)


def unwind_entries(stock_pe, routines=ROUTINES):
    """(original RVA, unwind-info RVA) from the stock .pdata for every relocated routine."""
    import pefile
    out = {}
    for name, orig, _f, length in routines:
        found = None
        for s in stock_pe.sections:
            if s.Name.rstrip(b"\x00") != b".pdata":
                continue
            raw = stock_pe.get_memory_mapped_image()[s.VirtualAddress:s.VirtualAddress + s.Misc_VirtualSize]
            for i in range(0, len(raw) - 12 + 1, 12):
                begin, end, unwind = struct.unpack_from("<III", raw, i)
                if begin == orig:
                    found = unwind
                    break
        if found is None:
            raise ValueError(f"no .pdata entry for {name} at 0x{orig:X}")
        out[name] = found
    return out
