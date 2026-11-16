#!/usr/bin/env python3
"""C12-L: shadow caves for the player add / remove helpers (high-slot join failure 2026-11-09, see LIMITS-HANDOFF.md).
These sites are `add@` / `not@` entries in docs/limits-players-sites.json (the address is built by an imul whose result
is added to the block pointer later), so gen_sites.py skipped them. Reviewed by hand, one function at a time:
  0x39D620 add/update player, 0x39D7F8 remove player, 0x39D8A8 player_add, 0x39D950 player_remove, 0x39C8DC peer remove.
Same cave as gen_caves.py; the two imul-0x56 sites index the peer's dword player table (peer*0x56 + slot, scaled by 4),
so their constant is (delta >> 2) - 17*0x56 (scale shift 2; SlotPatch.cpp requires delta % 4 == 0 in 32-player mode).
Usage: gen_member_caves.py <stock haloreach.dll> <out.inc>
"""
import subprocess, sys
REGS = {r: i for i, r in enumerate('rax rcx rdx rbx rsp rbp rsi rdi r8 r9 r10 r11 r12 r13 r14 r15'.split())}
# rva, original bytes (7, may span two instructions), dst, index, record, threshold, scale shift, what
SITES = [
    (0x39D63F, '4d6bd6564963e8', 'r10', 'r14', 0x56, 17, 2, 'imul r10,r14,0x56 / movsxd rbp,r8d (peer player table)'),
    (0x39D659, '4869c7b0000000', 'rax', 'rdi', 0xB0, 16, 0, 'imul rax,rdi,0xb0'),
    (0x39D808, '4869cfb0000000', 'rcx', 'rdi', 0xB0, 16, 0, 'imul rcx,rdi,0xb0'),
    (0x39D817, '4c69c058010000', 'r8',  'rax', 0x158, 17, 0, 'imul r8,rax,0x158'),
    (0x39D8E1, '4c69d6b0000000', 'r10', 'rsi', 0xB0, 16, 0, 'imul r10,rsi,0xb0'),
    (0x39D917, '4c69c758010000', 'r8',  'rdi', 0x158, 17, 0, 'imul r8,rdi,0x158'),
    (0x39D957, '4d69d3b0000000', 'r10', 'r11', 0xB0, 16, 0, 'imul r10,r11,0xb0'),
    (0x39D97A, '4869d058010000', 'rdx', 'rax', 0x158, 17, 0, 'imul rdx,rax,0x158'),
    (0x39C910, '4c69fd58010000', 'r15', 'rbp', 0x158, 17, 0, 'imul r15,rbp,0x158'),
    (0x39C99C, '486bca564903ca', 'rcx', 'rdx', 0x56, 17, 2, 'imul rcx,rdx,0x56 / add rcx,r10 (peer player mask table)'),
    (0x39CA11, '4869c8b0000000', 'rcx', 'rax', 0xB0, 16, 0, 'imul rcx,rax,0xb0'),
    # 0x411F38 "local players present" check (guest join gate, 2026-11-09): loops players 0..15, bounds raised in
    # SlotPatch.cpp kPeerMasks32 (0x411F76 / 0x411FDE), record address via this imul.
    (0x411F8E, '4d69cab0000000', 'r9',  'r10', 0xB0, 16, 0, 'imul r9,r10,0xb0'),
    # 0x397048 guest "joined" check: local peer (+0x4464) record state (+0x38) must be 8 before the guest moves on
    # (time sync 25 / peer-establish 22 / 35 / 27 / 33). With the local peer in slot 17 this read inline memory.
    (0x3970BC, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul rcx,rax,0x158'),    # Batch 2026-11-10 (read from the disassembly, not traced yet): session-indexed peer / player record
    # accesses on the join path that had no cave. First group: indexed by the LOCAL peer (+0x4464) - wrong on a
    # guest whose own peer is >= 17. Second group: peer state (+8) checks and their player lookups.
    (0x394531, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul   rcx,rax,0x158'),
    (0x39B067, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul   rcx,rax,0x158'),
    (0x4030DA, '4c69d058010000', 'r10', 'rax', 0x158, 17, 0, 'imul   r10,rax,0x158'),
    (0x4030E5, '4c69d858010000', 'r11', 'rax', 0x158, 17, 0, 'imul   r11,rax,0x158'),
    (0x40AD2B, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul   rcx,rax,0x158'),
    (0x40C282, '4869ca58010000', 'rcx', 'rdx', 0x158, 17, 0, 'imul   rcx,rdx,0x158'),
    (0x410637, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul   rcx,rax,0x158'),
    (0x412551, '4869d158010000', 'rdx', 'rcx', 0x158, 17, 0, 'imul   rdx,rcx,0x158'),
    (0x414F7D, '4869ca58010000', 'rcx', 'rdx', 0x158, 17, 0, 'imul   rcx,rdx,0x158'),
    (0x44A79C, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul   rcx,rax,0x158'),
    (0x395F6E, '4869d758010000', 'rdx', 'rdi', 0x158, 17, 0, 'imul   rdx,rdi,0x158'),
    (0x3EB7AC, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul   rcx,rax,0x158'),
    (0x3EBA5C, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul   rcx,rax,0x158'),
    (0x3FFFF2, '4869c658010000', 'rax', 'rsi', 0x158, 17, 0, 'imul   rax,rsi,0x158'),
    (0x402A76, '4969c058010000', 'rax', 'r8', 0x158, 17, 0, 'imul   rax,r8,0x158'),
    (0x404FC4, '4969c558010000', 'rax', 'r13', 0x158, 17, 0, 'imul   rax,r13,0x158'),
    (0x40EE75, '4869c158010000', 'rax', 'rcx', 0x158, 17, 0, 'imul   rax,rcx,0x158'),
    (0x4315B4, '4869c858010000', 'rcx', 'rax', 0x158, 17, 0, 'imul   rcx,rax,0x158'),
    (0x449F4F, '4c69c258010000', 'r8', 'rdx', 0x158, 17, 0, 'imul   r8,rdx,0x158'),
    (0x3EB793, '4969c2b0000000', 'rax', 'r10', 0xB0, 16, 0, 'imul   rax,r10,0xb0'),
    (0x3EBA4D, '4969d6b0000000', 'rdx', 'r14', 0xB0, 16, 0, 'imul   rdx,r14,0xb0'),
    (0x40EE66, '4969c1b0000000', 'rax', 'r9', 0xB0, 16, 0, 'imul   rax,r9,0xb0'),
    (0x4315A5, '4d69d0b0000000', 'r10', 'r8', 0xB0, 16, 0, 'imul   r10,r8,0xb0'),
    # 2026-11-10 (first added by hand to SlotMemberCaves.inc): peer player table reads `(peer*0x56 + user)*4`.
    # 0x42AB43 host handler of msg 32; 0x394C38 / 0x394D8A local peer's entry in the user update / drop senders.
    (0x42AB43, '486bd0564963c1', 'rdx', 'rax', 0x56, 17, 2, 'imul rdx,rax,0x56 / movsxd rax,r9d (msg 32: peer player table)'),
    (0x394C38, '486bc8564803ce', 'rcx', 'rax', 0x56, 17, 2, 'imul rcx,rax,0x56 / add rcx,rsi (local peer player table)'),
    (0x394D8A, '486bc8564903ce', 'rcx', 'rax', 0x56, 17, 2, 'imul rcx,rax,0x56 / add rcx,r14 (local peer player table)'),
    # 2026-11-10 02:55: local-user reconcile 0x38F4xx reads the local peer's table from the membership base (+0x174);
    # uncaved, a slot-17 guest dropped its own users (four msg 32). 0x38F9A7 = that player's record (+0x17A8).
    (0x38F6E0, '486bc9564903cf', 'rcx', 'rcx', 0x56, 17, 2, 'imul rcx,rcx,0x56 / add rcx,r15 (local peer player table: user id check)'),
    (0x38F723, '486bc9564903cf', 'rcx', 'rcx', 0x56, 17, 2, 'imul rcx,rcx,0x56 / add rcx,r15 (local peer player table: drop-user decision)'),
    (0x38F993, '486bc8564903cf', 'rcx', 'rax', 0x56, 17, 2, 'imul rcx,rax,0x56 / add rcx,r15 (local peer player table)'),
    (0x38F9A7, '4869c8b0000000', 'rcx', 'rax', 0xB0, 16, 0, "imul rcx,rax,0xb0 (local user's player record)"),
    # 2026-11-16 (high-slot root cause, ghidra task-5 + runtime probe): message 33 (player properties) resolves the
    # owning peer's dword player table with stock math, so the host's shadow player 16 never gets record+0x1C set and
    # the state 7 -> 8 readiness test in FUN_180392DB0 rejects peer 17 (it stays at 7 and the membership delta carries
    # state 7, which the guest consumer rejects). Same 0x56 dword-index shape as the msg 32 handler.
    (0x44B24C, '486bce564803c8', 'rcx', 'rsi', 0x56, 17, 2, 'imul rcx,rsi,0x56 / add rcx,rax (msg 33 player properties: peer player table)'),
    # 2026-11-16 (high-slot delta corruption, ghidra-delta section J.4): message 27 (peer properties) resolves the
    # owning peer's record with stock math. For peer 17 the property block then lands on session+0x1730 and its
    # 0x120-byte copy overwrites player 0/1 records (player-record+0x10 became 4 = invalid slot, which the wire's
    # 2-bit slot field cannot carry; the unmasked encoder then corrupted the peer field and the guest rejected the
    # membership delta). These three sites are the peer record bases on that path.
    (0x39D23E, '4969d758010000', 'rdx', 'r15', 0x158, 17, 0, 'imul rdx,r15,0x158 (msg 27 peer props: peer record base)'),
    (0x39F04E, '4969c658010000', 'rax', 'r14', 0x158, 17, 0, 'imul rax,r14,0x158 (msg 27 props apply: peer record base)'),
    (0x39F0C0, '4c69df58010000', 'r11', 'rdi', 0x158, 17, 0, 'imul r11,rdi,0x158 (msg 27 props apply: second peer base)'),
]
stock, out = sys.argv[1], sys.argv[2]
data = open(stock, 'rb').read()
import struct
pe = struct.unpack_from('<I', data, 0x3C)[0]
nsec = struct.unpack_from('<H', data, pe + 6)[0]; opt = struct.unpack_from('<H', data, pe + 20)[0]
secs = [struct.unpack_from('<8sIIII', data, pe + 24 + opt + i * 40) for i in range(nsec)]
def file_bytes(rva, n):
    for name, vsize, va, rsize, raw in secs:
        if va <= rva < va + vsize: return data[raw + rva - va: raw + rva - va + n]
    raise SystemExit(f'rva 0x{rva:X} not in a section')
def cmp_imm8(reg, v): n = REGS[reg]; return bytes([0x48 | (n >> 3), 0x83, 0xF8 | (n & 7), v])
def add_imm32(reg):   n = REGS[reg]; return bytes([0x48 | (n >> 3), 0x81, 0xC0 | (n & 7)])
rows = []
for rva, hexb, dst, idx, rec, T, shift, what in SITES:
    orig = bytes.fromhex(hexb)
    if file_bytes(rva, 7) != orig: raise SystemExit(f'stock bytes differ at 0x{rva:X}')
    c = bytearray(cmp_imm8(idx, T)) + b'\x0f\x83'; jae = len(c); c += b'\0' * 4
    c += orig + b'\xe9'; b1 = len(c); c += b'\0' * 4
    c[jae:jae + 4] = (len(c) - (jae + 4)).to_bytes(4, 'little', signed=True)
    c += orig + add_imm32(dst); cpos = len(c); c += b'\0' * 4
    c += b'\xe9'; b2 = len(c); c += b'\0' * 4
    rows.append((rva, orig, bytes(c), cpos, b1, b2, rec, T, shift, what))
with open(out, 'w', newline='\r\n') as f:
    f.write('// Generated by tools/limits/gen_member_caves.py (hand-reviewed player add/remove sites). Do not edit.\n')
    f.write('// rva, original bytes (7), cave bytes, offset of C imm32, offsets of the two jmp-back rel32, record, threshold, scale shift\n')
    for rva, orig, cave, cpos, b1, b2, rec, T, shift, what in rows:
        f.write(f'{{ 0x{rva:X}, {{ {", ".join(f"0x{x:02X}" for x in orig)} }}, {len(cave)}, '
                f'{{ {", ".join(f"0x{x:02X}" for x in cave)} }}, {cpos}, {b1}, {b2}, 0x{rec:X}, {T}, {shift} }},  // {what}\n')
print(f'{len(rows)} caves')
