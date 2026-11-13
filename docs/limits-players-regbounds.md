# 32 players: loop bounds held in registers (static review, 2026-11-10 00:45 ET, Claude Fable 5.2)

Found by reading the 23:45 high-slot logs and the stock listing (`build\stock_text.asm`), no new test.

## Why the guest sticks at peer state 4
- Guest gate: 0x3903C7 `cmp [session + local_peer*0x158 + 0x68], 6` (imul 0x3903BB is caved, so it really reads 4).
- Peer state setter: 0x39C668 (block, peer, state); imul 0x39C66F caved. 12 callers. Promotions:
  2 -> 4 at 0x449F7F / 0x3955DB, 4 -> 5 at 0x3933F9, 5 -> 6 (7 for +0x10 peer) at 0x392F84, -> 8 at 0x39349D / 0x39528D / 0x403D46.
- 0x3933F9 and 0x392F84 are both in the host's join-processing function 0x392DB0. Its peer-mask scans use a bound kept
  in a register: **0x392E12 `lea r13d,[rbx+0x11]` (rbx = 0) = 17, never patched** (gen_bounds.py only handled
  `cmp reg, imm`). Nine scans (0x392EE4, 0x392F1F, 0x392F94, 0x392FA6, 0x393000, 0x39303B ...) stop at peer 16, so the
  host never promotes peer 17 past the 4 -> 5 step (that one uses `cmp eax,0x11`, already raised).
  Fix candidate: bytes `44 8D 6B 11` -> `44 8D 6B 20`, 32-player mode only, byte-checked. Check first that nothing in
  0x392DB0 indexes a 17-entry stack table with those loop counters (the 4-entry tables at rsp+0x60/0xE0/0xF0 are per
  player-slot-of-peer, fine).
- Host log supports it: producer #1 (2 peers, 2 players), #2 one second later (1 peer changed), then nothing more.

## The missed class
Register-held bounds in 0x380000-0x460000 where the register is later used as `cmp x, reg` (script: this session's
regbound.py; "near bit-test" = a `bt` on a mask within 6 instructions, i.e. almost certainly a slot-mask scan):

```
--- 17 (peers)
392E12 lea r13d, [rbx + 0x11]           bound-uses 9 (near bit-test 9) not patched
409898 lea r14d, [rdi + 0x11]           bound-uses 1 (near bit-test 1) not patched
39C922 mov esi, 0x11                    bound-uses 3 (near bit-test 3) not patched
3A015C mov r12d, 0x11                   bound-uses 3 (near bit-test 0) not patched
3C823C mov r13d, 0x11                   bound-uses 4 (near bit-test 0) not patched
3C8A6E mov ebp, 0x11                    bound-uses 2 (near bit-test 0) not patched
3FB8C5 mov ebx, 0x11                    bound-uses 1 (near bit-test 0) not patched
3FE945 mov r8d, 0x11                    bound-uses 1 (near bit-test 0) not patched
404E7C mov esi, 0x11                    bound-uses 7 (near bit-test 6) not patched
--- 16 (players)
39BABA lea r9d, [rdx + 0x10]            bound-uses 2 (near bit-test 2) not patched
3B96DD lea edi, [r10 + 0x10]            bound-uses 1 (near bit-test 0) not patched
40938C lea eax, [rbx + 0x10]            bound-uses 1 (near bit-test 1) not patched
4139E5 lea r12d, [rsi + 0x10]           bound-uses 5 (near bit-test 5) not patched
4160F0 lea edx, [rbp + 0x10]            bound-uses 1 (near bit-test 1) not patched
418501 lea r12d, [r13 + 0x10]           bound-uses 1 (near bit-test 0) not patched
451D21 lea r12d, [rdi + 0x10]           bound-uses 1 (near bit-test 0) not patched
38B87C mov r10d, 0x10                   bound-uses 1 (near bit-test 0) not patched
3A0CD3 mov ebx, 0x10                    bound-uses 2 (near bit-test 0) not patched
3A10DA mov ebx, 0x10                    bound-uses 1 (near bit-test 0) not patched
3A1250 mov r11d, 0x10                   bound-uses 2 (near bit-test 0) not patched
3A1620 mov r14d, 0x10                   bound-uses 2 (near bit-test 0) not patched
3C06E9 mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
3C9AAE mov edx, 0x10                    bound-uses 1 (near bit-test 0) not patched
3DBC73 mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
3FC63F mov r12d, 0x10                   bound-uses 5 (near bit-test 0) not patched
3FD53C mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
3FD72E mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
416156 mov r9d, 0x10                    bound-uses 2 (near bit-test 2) not patched
417FCA mov ebp, 0x10                    bound-uses 2 (near bit-test 0) not patched
4205DF mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
423DE9 mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
423F9A mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
4241FA mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
4251CA mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
429970 mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
430135 mov edx, 0x10                    bound-uses 2 (near bit-test 0) not patched
43B096 mov r9d, 0x10                    bound-uses 1 (near bit-test 0) not patched
43CB4E mov r14d, 0x10                   bound-uses 1 (near bit-test 0) not patched
455C7F mov r11d, 0x10                   bound-uses 2 (near bit-test 0) not patched
456AFA mov ebx, 0x10                    bound-uses 3 (near bit-test 0) not patched
458302 mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
45BE64 mov r13d, 0x10                   bound-uses 4 (near bit-test 0) not patched
45D298 mov r8d, 0x10                    bound-uses 1 (near bit-test 0) not patched
```
Near-certain slot loops (bit-test next to the bound): 0x392E12, 0x409898, 0x39C922 (in 0x39C8DC peer removal),
0x404E7C, 0x39BABA, 0x40938C, 0x4139E5, 0x4160F0, 0x416156. The rest need a look each (several are sizes, not slots).
Each raise needs the same check as before: no fixed 16/17-entry table indexed by that loop.

## Other join-path sites not in any table (0x391F0C, network-decoded block -> session)
0x3920F4 `imul rax,rdi,0x158` (writes peer +0x138 join id right after set-state), 0x39215D / 0x392571 (player 0xB0),
0x392631 (peer). Low indexes in today's test (guest is provisional peer 1 there), but wrong for a real 17+ lobby.
0x395E2F / 0x395E6B / 0x395E85 in 0x395DE8 (called from 0x39301E / 0x395FA1 with state 7) likewise.

## Not in LIMITS-HANDOFF.md yet
The handoff stops at 23:15. The 23:45 run and the "state 4, needs >= 6" finding were only in chat.
`SlotMemberCaves.inc` (00:10) is newer than the last build (23:52): unbuilt change on disk.

## Batch 1 (2026-11-10 01:00 ET): written to SlotPatch.cpp `kPeerMasks32`, NOT built, NOT tested
| RVA | Stock -> patched | Why safe |
|---|---|---|
| 0x392E12 | `44 8D 6B 11` -> `.. 20` (lea r13d,[rbx+0x11], rbx = 0) | 9 peer-mask scans in 0x392DB0; all imuls caved; r13 reloaded at 0x39313F |
| 0x409898 | `44 8D 77 11` -> `.. 21` (lea r14d,[rdi+0x11], rdi = -1, so 16 -> 32) | player-mask scans; imuls 0x4099B3 / 0x4099C2 caved |
| 0x4139E5 | `44 8D 66 10` -> `.. 1F` (lea r12d,[rsi+0x10], rsi = 1, so 17 -> 32) | peer-mask scans against a bitmask, no indexing |
Byte-checked like the rest of the table: a wrong stock byte turns 32-player mode off with a log line, nothing else.
Expected in the next high-slot run: host producer goes on past #2, guest log shows local peer 17 record state 5 -> 6+,
guest sends msg 25 / 22 / 35.

## Reviewed, NOT raised yet (need more than a bound)
- 0x39C922 `mov esi,0x11` (0x39C8DC peer removal, 3 scans): esi is also passed as a count to 0x1800FE8B0 on a local at
  rsp+0x30 (0x39C939). Raise only the scans (cave) or prove the local holds 32.
- 0x404E7C `mov esi,0x11` (7 scans): esi also goes to 0x180689724 as an element count with a callback (0x405205),
  i.e. a 17-element local array. Needs the array grown first.
- 0x39BABA `lea r9d,[rdx+0x10]` (rdx = 1, so 17): peer scan with an uncaved imul 0x39BAD8 (`[rax+r10+0x148]`). Cave + bound.
- 0x40938C `lea eax,[rbx+0x10]`: player scan with an uncaved imul 0x4093BB. Cave + bound.
- 0x4160F0 `lea edx,[rbp+0x10]` (rbp = 0): player scan, uncaved imul 0x416134, edx copied to r9d. Cave + bound.
- 0x416156 and the "near bit-test 0" rows above: not reviewed yet.
