#!/usr/bin/env python3
"""Build the V16 (profile 1) or V16 + particles (profile 2) engine and compile it into an .nlp file.

Both profiles also get a bigger AI game-state bank (bank 1, stock 0x220000): the actor, prop and
other AI pools live there with only ~5 KB spare, so raising the actor pool without this overruns
into bank 2 and crashes. --ai-bank-extra sets how much is added (default 6 MB, enough for 1,024
actors). The checkpoint and total sizes grow with it.

Starts from stock haloreach.dll 1.3528.0.0, applies GreenKnight's V16 recipe, then ports GreenKnight's
65,535-particle recipe onto that layout:

* The particle patch's 512-byte block (budget table + three code pieces) goes into the spare room at
  the end of V16's executable `.deltrac` section (RVA 0x0AAE8200). V16 already uses all ten section
  header slots, so a new section is not possible.
* Every location-dependent reference in the block and its three redirects is recomputed for the new
  address (see BLOCK_FIXUPS / REDIRECTS).
* The particle patch's larger render bank (+0x820000) is added to V16's sizes. The checkpoint and
  total game-state sizes follow the formulas in LIMITS-HANDOFF.md, and V16's second checkpoint
  buffer and queue (at the end of .hkwork) move up to make room.
* The particle patch's own snapshot-buffer section is not needed: V16 already relocated those
  buffers, so they are simply made bigger.

Usage:  python make_v16_particles.py STOCK_DLL [--profile v16p|v16] [--ai-bank-extra 0x600000]
                                   [--out-dll FILE] [--nlp FILE]
"""
from __future__ import annotations

import argparse
import hashlib
import re
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import build_recipe  # noqa: E402

STOCK_SHA = "738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894"
V16_SHA = "b8810c3b9822f5d6563439fc6b8b16c2def6cd7a6f660dd3775c42edcd0a5f24"
TEXT_DELTA = 0xC00          # .text: RVA = file offset + 0xC00
ORIG_BLOCK_RVA = 0x04EDA000  # where GreenKnight's particle block was built to run

# V16 layout
DELTRAC_HDR, HKWORK_HDR = 0x3B0, 0x3D8
DELTRAC_RVA, DELTRAC_RAW, DELTRAC_SIZE = 0x0AA51000, 0x00CF3600, 0x00097200
HKWORK_RVA = 0x0AAE9000
NEW_BLOCK_RVA = DELTRAC_RVA + DELTRAC_SIZE           # 0x0AAE8200
NEW_BLOCK_RAW = DELTRAC_RAW + DELTRAC_SIZE           # 0x00D8A800 (V16 end of file)

V16_RENDER, V16_MAIN = 0x560000, 0x27D0000
PARTICLE_RENDER_EXTRA = 0x820000
V16_CHECKPOINT, V16_TOTAL = 0x2F90000, 0x7F30000
AI_BANK, AI_BANK_OP = 0x220000, 0x15555              # bank 1 size: `mov edx, 0x220000` at RVA 0x16154
DEFAULT_AI_BANK_EXTRA = 0x600000

# AI update scheduler (RVA 0x70ABBC) lists every live actor in a 96-entry, 12-byte stack array with no
# bounds check, then sorts it. Grow its frame so the array holds 1,024: the array and the stack cookie
# sit at rbp-0x70 .. rbp+0x410, so rbp and rsp move down by AI_FRAME_EXTRA and the cookie / frame-top
# offsets move up by the same amount. (file offset of the 32-bit field, original value, change)
AI_FRAME_EXTRA = 1024 * 12 - 96 * 12                # 0x2B80
AI_FRAME_OPS = [
    (0x70ABD7 - 0xC00, -0x468 & 0xFFFFFFFF, -AI_FRAME_EXTRA),   # lea rbp, [rax-0x468]
    (0x70ABDE - 0xC00, 0x540, AI_FRAME_EXTRA),                  # sub rsp, 0x540
    (0x70ABF8 - 0xC00, 0x410, AI_FRAME_EXTRA),                  # mov [rbp+0x410], cookie
    (0x70AFA7 - 0xC00, 0x410, AI_FRAME_EXTRA),                  # mov rcx, [rbp+0x410]
    (0x70AFB7 - 0xC00, 0x540, AI_FRAME_EXTRA),                  # lea r11, [rsp+0x540]
]

BUF1 = 0x15181000                                    # unchanged
V16_BUF2, V16_QUEUE = 0x18111020, 0x1B0A1040


class Layout:
    def __init__(self, particles: bool, ai_extra: int):
        if ai_extra % 0x10000:
            raise ValueError("--ai-bank-extra must be a multiple of 0x10000")
        self.render = V16_RENDER + (PARTICLE_RENDER_EXTRA if particles else 0)
        self.ai_bank = AI_BANK + ai_extra
        self.checkpoint = self.render + V16_MAIN + 0x260000 + ai_extra   # banks 0-5
        self.total = self.checkpoint + 2 * V16_MAIN
        self.buf2 = BUF1 + self.checkpoint + 0x20
        self.queue = self.buf2 + self.checkpoint + 0x20
        self.image_end = (self.queue + 0x40 + 0xFFF) & ~0xFFF

# Size operands (file offset of the 4-byte value) and their V16 values.
RENDER_OPS = [0x1559A]
CHECKPOINT_OPS = [0xB89C, 0x154BB, 0x15F1E, 0x1627D, 0x16568, 0x165C7, 0x165CF, 0x165EB, 0x3499F,
                  0x87649, 0x87B66, 0x87E5A, 0xA47E7, 0x1707DB, 0x170AC9, 0x170D39, 0x171048, 0x1715B4]
BUFSIZE_OPS = [0xB7FE]                               # checkpoint + 32
TOTAL_OPS = [0xB873, 0x153B7, 0x1552F]
# RIP-relative references to the static checkpoint buffers come from V16's STATE_BANKS.md table
# (operand file offset + V16 target RVA), so each instruction's end is exact: end = target - disp.

# Particle redirects in .text: (file offset of the E9, block offset it jumps to).
REDIRECTS = [(0x115540, 0x070), (0x1D48FD, 0x190), (0x27904B, 0x140)]
# Location-dependent fields inside the block: (block offset of the field, kind, value)
#   rel32: field is a rel32 whose instruction ends at block offset `end`, target is an absolute RVA.
#   imm:   field holds an RVA that must become new block RVA + value.
BLOCK_FIXUPS = [
    ("rel32", 0x073, 0x077, 0x04E30798),  # mov [rip+], rcx  -> effect-budget global (.data)
    ("imm",   0x090, None, 0x08D),        # sub r11, <RVA of 0x08D>  (self base)
    ("imm",   0x0CA, None, 0x000),        # mov edx, [r11+rax*4+<table RVA>]
    ("rel32", 0x127, 0x12B, 0x00116147),  # jmp back after the redirected mov
    ("rel32", 0x14E, 0x152, 0x00279CA9),  # jb  (final-row check)
    ("rel32", 0x153, 0x157, 0x00279C51),  # jmp
    ("rel32", 0x15D, 0x161, 0x00279CA9),  # jb
    ("rel32", 0x162, 0x166, 0x00279C51),  # jmp
    ("rel32", 0x1A0, 0x1A4, 0x001D5504),  # jmp back after the saturating add
]


def particle_rows(guide: Path):
    """Functional particle rows (capacity values and branch changes) from the guide's tables."""
    t = guide.read_text(encoding="utf-8")
    rows = []
    for m in re.finditer(r"^\| `([0-9A-F]{8})` \| ([^|]+)\| `([0-9A-F ]+)` \| `([0-9A-F ]+)` \|", t, re.M):
        off = int(m.group(1), 16)
        desc = m.group(2).strip()
        if off < 0x400:   # PE header rows: the V16 layout gets its own header changes
            continue
        if any(k in desc.lower() for k in ("snapshot", "checkpoint", "game state", "render bank", "redirect snapshot")):
            continue
        rows.append((off, bytes.fromhex(m.group(3)), bytes.fromhex(m.group(4)), desc))
    block = None
    for part in t.split("```text")[1:]:
        body = part.split("```")[0].split()
        if len(body) == 512:
            block = bytearray(bytes.fromhex("".join(body)))
    if block is None or len(rows) != 15:
        raise ValueError(f"particle guide parse failed ({len(rows)} rows)")
    return rows, block


def snapshot_refs(state_banks: Path):
    out = []
    for m in re.finditer(r"^\| `0x([0-9A-F]{8})` \| `0x([0-9A-F]{8})` \| `0x[0-9A-F]{8}` \| `0x([0-9A-F]{8})` \| `([0-9A-F ]+)` \|",
                         state_banks.read_text(encoding="utf-8"), re.M):
        insn_rva, off, target = int(m.group(1), 16), int(m.group(2), 16), int(m.group(3), 16)
        disp = struct.unpack("<i", bytes.fromhex(m.group(4)))[0]
        if insn_rva > off + TEXT_DELTA or target - disp - insn_rva not in range(5, 12):
            raise ValueError(f"STATE_BANKS row 0x{off:X} is inconsistent")
        out.append((off, target))
    return out


def u32(b, o): return struct.unpack_from("<I", b, o)[0]
def s32(b, o): return struct.unpack_from("<i", b, o)[0]
def put32(b, o, v): struct.pack_into("<I", b, o, v & 0xFFFFFFFF)


def expect32(img, off, value, what):
    if u32(img, off) != value:
        raise ValueError(f"{what} at 0x{off:X}: expected 0x{value:X}, found 0x{u32(img, off):X}")


def pe_checksum(img: bytearray, csum_off: int) -> int:
    total = 0
    for i in range(0, len(img) - (len(img) & 1), 2):
        if csum_off <= i < csum_off + 4:
            continue
        total += img[i] | (img[i + 1] << 8)
        total = (total & 0xFFFF) + (total >> 16)
    if len(img) & 1:
        total += img[-1]
    total = (total & 0xFFFF) + (total >> 16)
    total = (total & 0xFFFF) + (total >> 16)
    return (total + len(img)) & 0xFFFFFFFF


def build(stock: bytes, recipes: Path, particles: bool = True, ai_extra: int = DEFAULT_AI_BANK_EXTRA) -> bytes:
    L = Layout(particles, ai_extra)
    if hashlib.sha256(stock).hexdigest() != STOCK_SHA:
        raise ValueError("not the stock haloreach.dll 1.3528.0.0")
    rows, append, meta = build_recipe.load_v16(recipes)
    img = bytearray(build_recipe.apply(stock, rows, append))
    if hashlib.sha256(img).hexdigest() != V16_SHA:
        raise ValueError("V16 build mismatch")
    if len(img) != NEW_BLOCK_RAW:
        raise ValueError("unexpected V16 length")

    block = b""
    if particles:
        prow, block = particle_rows(recipes / "particles" / "particle_limit_hex_editing_guide.md")

        # 1. Functional particle rows (capacities, branch fixes). Redirect rows get recomputed targets.
        redirect_offs = {r[0] for r in REDIRECTS}
        for off, old, new, desc in prow:
            if img[off:off + len(old)] != old:
                raise ValueError(f"particle row 0x{off:X} ({desc}): V16 bytes are not stock")
            if off in redirect_offs:
                continue
            img[off:off + len(new)] = new
        for off, block_off in REDIRECTS:
            new = next(r[2] for r in prow if r[0] == off)
            insn_end = off + TEXT_DELTA + 5
            img[off:off + len(new)] = new                     # E9 xx xx xx xx + NOP padding
            put32(img, off + 1, NEW_BLOCK_RVA + block_off - insn_end)

        # 2. Block fixups, checked against the original values first.
        for kind, field, end, value in BLOCK_FIXUPS:
            if kind == "rel32":
                if ORIG_BLOCK_RVA + end + s32(block, field) != value:
                    raise ValueError(f"block rel32 at +0x{field:X} does not point at 0x{value:X}")
                put32(block, field, value - (NEW_BLOCK_RVA + end))
            else:
                if u32(block, field) != ORIG_BLOCK_RVA + value:
                    raise ValueError(f"block imm at +0x{field:X} is not the original block RVA + 0x{value:X}")
                put32(block, field, NEW_BLOCK_RVA + value)

        # 3. Append the block at the end of .deltrac and grow that section.
        img += block
        expect32(img, DELTRAC_HDR + 8, DELTRAC_SIZE, ".deltrac VirtualSize")
        expect32(img, DELTRAC_HDR + 12, DELTRAC_RVA, ".deltrac RVA")
        expect32(img, DELTRAC_HDR + 16, DELTRAC_SIZE, ".deltrac SizeOfRawData")
        put32(img, DELTRAC_HDR + 8, DELTRAC_SIZE + len(block))
        put32(img, DELTRAC_HDR + 16, DELTRAC_SIZE + len(block))
        if NEW_BLOCK_RVA + len(block) > HKWORK_RVA:
            raise ValueError("block does not fit before .hkwork")

    # 4. Game-state sizes.
    for off in RENDER_OPS:
        expect32(img, off, V16_RENDER, "render bank"); put32(img, off, L.render)
    expect32(img, AI_BANK_OP, AI_BANK, "AI bank size"); put32(img, AI_BANK_OP, L.ai_bank)
    if ai_extra:
        for off, old, delta in AI_FRAME_OPS:
            expect32(img, off, old, "AI update frame"); put32(img, off, old + delta)
    for off in CHECKPOINT_OPS:
        expect32(img, off, V16_CHECKPOINT, "checkpoint size"); put32(img, off, L.checkpoint)
    for off in BUFSIZE_OPS:
        expect32(img, off, V16_CHECKPOINT + 0x20, "checkpoint buffer size"); put32(img, off, L.checkpoint + 0x20)
    for off in TOTAL_OPS:
        expect32(img, off, V16_TOTAL, "total game state"); put32(img, off, L.total)

    # 5. Move the second checkpoint buffer and the queue up.
    refs = snapshot_refs(recipes / "v16" / "STATE_BANKS.md")
    if len(refs) != 20:
        raise ValueError(f"expected 20 checkpoint-buffer references, found {len(refs)}")
    for off, v16_target in refs:
        insn_end = v16_target - s32(img, off)
        if v16_target == BUF1:
            continue
        if V16_BUF2 <= v16_target < V16_BUF2 + V16_CHECKPOINT + 0x20:
            new_target = v16_target - V16_BUF2 + L.buf2
        elif V16_QUEUE <= v16_target < V16_QUEUE + 0x40:
            new_target = v16_target - V16_QUEUE + L.queue
        else:
            raise ValueError(f"checkpoint ref at 0x{off:X} points at unexpected 0x{v16_target:X}")
        put32(img, off, new_target - insn_end)

    # 6. Grow .hkwork (virtual only) and the image.
    expect32(img, HKWORK_HDR + 12, HKWORK_RVA, ".hkwork RVA")
    old_hk = u32(img, HKWORK_HDR + 8)
    new_hk = L.image_end - HKWORK_RVA
    put32(img, HKWORK_HDR + 8, new_hk)
    e = u32(img, 0x3C); opt = e + 24
    put32(img, opt + 4, u32(img, opt + 4) + len(block))          # SizeOfCode
    put32(img, opt + 12, u32(img, opt + 12) + (new_hk - old_hk))  # SizeOfUninitializedData
    put32(img, opt + 56, L.image_end)                               # SizeOfImage
    put32(img, opt + 64, 0)
    put32(img, opt + 64, pe_checksum(img, opt + 64))
    return bytes(img)


def diff_rows(stock: bytes, final: bytes, merge_gap=8):
    rows, i, n = [], 0, len(stock)
    while i < n:
        if stock[i] == final[i]:
            i += 1
            continue
        j = i
        while j < n:
            if stock[j] != final[j]:
                j += 1
                continue
            k = j
            while k < n and k - j < merge_gap and stock[k] == final[k]:
                k += 1
            if k < n and k - j < merge_gap and stock[k] != final[k]:
                j = k
                continue
            break
        rows.append((i, stock[i:j], final[i:j]))
        i = j
    return rows


def main() -> int:
    repo = HERE.parents[1]
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("stock", type=Path)
    ap.add_argument("--recipes", type=Path, default=HERE / "recipes")
    ap.add_argument("--out-dll", type=Path, default=None)
    ap.add_argument("--profile", choices=["v16p", "v16"], default="v16p")
    ap.add_argument("--ai-bank-extra", type=lambda v: int(v, 0), default=DEFAULT_AI_BANK_EXTRA)
    ap.add_argument("--nlp", type=Path, default=None)
    a = ap.parse_args()
    particles = a.profile == "v16p"
    nlp = a.nlp or repo / "data" / "limits" / f"{a.profile}.nlp"
    L = Layout(particles, a.ai_bank_extra)
    stock = a.stock.read_bytes()
    final = build(stock, a.recipes, particles, a.ai_bank_extra)
    rows = diff_rows(stock, final[:len(stock)])
    meta = {"stock_len": len(stock), "stock_sha": hashlib.sha256(stock).digest(),
            "out_len": len(final), "out_sha": hashlib.sha256(final).digest(), "append_off": len(stock)}
    append = final[len(stock):]
    build_recipe.validate(rows, append, meta)
    if build_recipe.apply(stock, rows, append) != final:
        raise SystemExit("diff does not round-trip")
    size = build_recipe.write_nlp(nlp, 2 if particles else 1, rows, append, meta)
    print(f"{a.profile}: {len(final):,} bytes, sha256 {hashlib.sha256(final).hexdigest()}")
    print(f"wrote {nlp} ({size:,} bytes, {len(rows)} rows)")
    print(f"render 0x{L.render:X} ai bank 0x{L.ai_bank:X} checkpoint 0x{L.checkpoint:X} total 0x{L.total:X} "
          f"buf2 0x{L.buf2:X} queue 0x{L.queue:X} image 0x{L.image_end:X}")
    if a.out_dll:
        a.out_dll.write_bytes(final)
        print(f"wrote {a.out_dll}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
