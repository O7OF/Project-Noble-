#!/usr/bin/env python3
"""C12 step 2a: pointer-stepping loop caves -> src/host/SlotStepTable.inc.

Usage: python -I tools/limits/gen_steps.py <stock haloreach.dll> src/host/SlotStepTable.inc

Loops that walk a session slot array with `add ptr, 0xB0|0x158` cannot be fixed by the imul caves: past the last
inline record they run into the scalar fields (+0x1718 peers / +0x2240 players). Each site below replaces that
7-byte `add` with a jump to a cave that does the original step and, when the loop reaches the first slot that does
not exist in the original object (players 16, peers 17), moves the pointer into the shadow session:

    add  ptr, R            ; original step
    cmp  ctr, T            ; counter form: ctr already incremented for the next slot
    (or: cmp ptr, T*R      ; offset form: ptr is idx*R, added to the object base at the use)
    jne  back
    add  ptr, C            ; C = shadowDelta - T*R (same constant as the imul caves; filled in at runtime)
  back:
    jmp  <after the site>

Every site was reviewed by hand against the stock DLL (Claude, 2026-11-08): the stepped pointer is proven to be
inside a session membership object (base from the mask at +0x5C/+0x2C or from 0x398200's +0x1740 out-pointer),
the counter is incremented before the step, and the instruction after the site overwrites the flags.
Installed only in 32-player mode (they change the pointer after the last pass of a 16-slot loop).
"""
import sys

REGS = {r: i for i, r in enumerate('rax rcx rdx rbx rsp rbp rsi rdi r8 r9 r10 r11 r12 r13 r14 r15'.split())}
REGS32 = {r: i for i, r in enumerate('eax ecx edx ebx esp ebp esi edi r8d r9d r10d r11d r12d r13d r14d r15d'.split())}

# rva, stepped pointer, counter (32-bit reg) or None for the offset form, record, first shadow slot, evidence
STEPS = [
    (0x391B94, 'r8',  None,   0x158, 17, '3919E4: r8 = idx*0x158 used as [r8+r13+0x1a4], r13 = session (mask +0x5C)'),
    (0x39322C, 'r12', 'r14d', 0x158, 17, '392DB0: r12 = block+0x168, block mask at +0x2C'),
    (0x39EC61, 'r8',  'ecx',  0x158, 17, '39EBF0: r8 = block+0x14C, block mask at +0x2C'),
    (0x39ECD8, 'rbx', 'ecx',  0x158, 17, '39EC88: rbx = block+0x14C, block mask at +0x2C'),
    (0x408F2E, 'r15', 'r14d', 0xB0,  16, '408DDC: r15 = session+0x17B0 (mask +0x173C)'),
    (0x411986, 'rsi', 'r9d',  0xB0,  16, '4118C8: rsi = session+0x1780 (mask +0x173C)'),
    (0x44BDE3, 'r8',  'r10d', 0xB0,  16, '44BCE8: r8 = session+0x175C (r14 = session+0x30, mask [r14+0x170C]); read-only, r8 dead after exit'),
    (0x44CCEF, 'rdx', 'ecx',  0xB0,  16, '44C9BC: rdx = block+0x1750 (r15 = block, peer mask +0x2C); read-only count into [rbx+0x1DC], rdx reloaded at 0x44CD26'),
]

# Reviewed and verified as session-array steps, but NOT installed: the same loop also fills a fixed 16/17-entry
# table that is not in the session object, so raising its bound would overflow that table (C3 work first).
BLOCKED_BY_TABLES = [
    (0x00D8C9, 'r8',  'r9d',  0xB0,  16, 'D6B8: 16 x 0x20 player list at rbp-0x80 handed to the host interface [0xC1A100]+0xF0'),
    (0x39E212, 'rdx', 'r13d', 0xB0,  16, '39DD4C: function also walks the caller-stack 16 x 0x180 table (bounds review #32)'),
    (0x39E219, 'rbx', 'r13d', 0xB0,  16, '39DD4C: as above'),
    (0x3FFD7F, 'rsi', 'edi',  0x158, 17, '3FFC9C: 17 x 0xC table in the caller object at +0xC (rbx/rbp step 0xC)'),
    (0x41E9B5, 'r15', 'edi',  0x158, 17, '41E884: 17 x 6 stack table at rsp+0x20 (memset 0x66)'),
    (0x41F576, 'rsi', 'ebx',  0x158, 17, '41F510: 17 x 6 stack table at rsp+0x40'),
    (0x41F61E, 'rbp', 'esi',  0xB0,  16, '41F510: 16 x 0xA0 table in the 0x4E0E910 singleton at +0xA0'),
    (0x41F95B, 'rbx', 'esi',  0xB0,  16, '41F510: as above (rbp steps 0xA0)'),
    # Reviewed 2026-11-08 (C12-L step 1, the 11 remaining loops); two of them are installed above (0x44BDE3, 0x44CCEF).
    (0x404B4A, 'rbp', None,   0x158, 17, '404728: count-down loop (r13 = 0x11) that also walks a 17-dword stack table at rsp+0x30 (built at 0x40477A; frame 0x90, cookie at +0x80, no room for 32)'),
    (0x41FC64, 'r15', 'r14d', 0x158, 17, '41FBE8: r15 = stack argument [rbp+0x7F]; same loop fills a 17 x 6 stack table at rbp-0x61 (memset 0x66) read again by the 0x11 loop at 0x41FC84'),
    (0x4314D3, 'rdx', 'r8d',  0xB0,  16, '43134C: appends 0x35-byte entries to an output buffer [rdi+2+n*0x35] counted by byte [rdi+1] (sized for 16)'),
    (0x431B1E, 'r12', 'esi',  0xB0,  16, '431A1C: fills a 16 x 0x3A table at rbx+0x17 (r14/r15 step 0x3A) through call 0x431788'),
    (0x44CB51, 'r8',  'ecx',  0xB0,  16, '44C9BC: fills a 16 x 0x10 table in the r8-argument struct at rbx+0x14'),
    (0x44CCA1, 'r10', 'r9d',  0xB0,  16, '44C9BC: fills a 16 x 0x10 table at rbx+0x20 and a count-indexed 8-byte stack table at rsp+0x20'),
    (0x44CD5F, 'r14', 'r8d',  0xB0,  16, '44C9BC: fills a 16 x 0x24 table at rbx+0x1E8 (rbx = r8 argument, memset 0xA10 in 0x44BCE8)'),
    (0x4509E8, 'rdx', 'r9d',  0xB0,  16, '4505F0: reads a compact 4-byte table at rbp+0x990 (r8 += 4 per valid player, sized for 16)'),
]
# Not a session array (never caved): 0x4215AE (42136C) - r15 starts at -0xB00 and indexes the 64 x 0xB0 table at
# +0xF00 of the 0x4E0E908 object for player indices 16..79; indices < 16 read the players data array instead.
# 0x3809C9/0x38D24C/... and the other 0x4E0E908 object loops (64 x 0xB0 at +0xF00) are not session arrays either.

def rex(w, b): return 0x40 | (8 if w else 0) | (1 if b else 0)

def add_imm32(reg, imm):
    n = REGS[reg]
    return bytes([rex(1, n >> 3), 0x81, 0xC0 | (n & 7)]) + imm.to_bytes(4, 'little', signed=True)

def cmp_r32_imm8(reg, v):
    n = REGS32[reg]
    return (bytes([0x41]) if n >= 8 else b'') + bytes([0x83, 0xF8 | (n & 7), v])

def cmp_r64_imm32(reg, v):
    n = REGS[reg]
    return bytes([rex(1, n >> 3), 0x81, 0xF8 | (n & 7)]) + v.to_bytes(4, 'little')

def file_offset(pe, rva):
    # .text: VMA 0x1000 -> file 0x400 in the supported build
    return rva - 0x1000 + 0x400

def main():
    dll = open(sys.argv[1], 'rb').read()
    out = sys.argv[2]
    rows = []
    for rva, ptr, ctr, rec, T, note in STEPS:
        orig = add_imm32(ptr, rec)
        at = file_offset(dll, rva)
        if dll[at:at + 7] != orig:
            raise SystemExit(f'0x{rva:X}: expected {orig.hex()} got {dll[at:at+7].hex()}')
        c = bytearray(orig)
        c += cmp_r32_imm8(ctr, T) if ctr else cmp_r64_imm32(ptr, T * rec)
        c += b'\x75\x00'; jne_at = len(c) - 1
        cimm = len(c) + 3; c += add_imm32(ptr, 0)
        c[jne_at] = len(c) - (jne_at + 1)
        c += b'\xe9'; back = len(c); c += b'\0\0\0\0'
        rows.append((rva, orig, bytes(c), cimm, back, rec, T, note))
    with open(out, 'w', newline='\n') as f:
        f.write('// Generated by tools/limits/gen_steps.py (hand-reviewed list in the script). Do not edit.\n')
        f.write('// rva, original bytes (7), cave length, cave bytes, offset of C imm32, offset of jmp-back rel32, record, threshold\n')
        for rva, orig, cave, cimm, back, rec, T, note in rows:
            f.write(f'{{ 0x{rva:X}, {{ {", ".join(f"0x{x:02X}" for x in orig)} }}, {len(cave)}, '
                    f'{{ {", ".join(f"0x{x:02X}" for x in cave)} }}, {cimm}, {back}, 0x{rec:X}, {T} }},  // {note}\n')
    print(f'{len(rows)} step caves, max {max(len(r[2]) for r in rows)} bytes')

if __name__ == '__main__':
    main()
