# #32: membership update message for 32 players (design, 2026-11-09 overnight, Claude)

Goal: in `-slot-players 32` mode the membership update message carries up to **32 peers and 32 players**
(peer mask is a DWORD `bt`, so peers stop at 32). Stock: 17 peers / 16 players. Every machine in a 32-player
session runs the same build (`limits_id` join check), so the wire format may change in 32-player mode.
16-player mode stays byte-identical to stock (plus the existing security fix in EnginePatches section 5).

## Pieces (haloreach.dll 1.3528.0.0, RVAs)
| What | Where | Notes |
|---|---|---|
| Registration | 0x427E8C | message table entry at table+0x680 (table ptr at 0x4E38D28): +0x14 min size, +0x18 max size (both 0x2550), +0x20 encode 0x42675C, +0x28 decode 0x427140. Message type 0x1A. |
| Encode wrapper | 0x423D88 | writes type (6 bits) + **size (16 bits)**, then calls encode(stream, size, payload) |
| Decode wrapper | 0x423E40 | reads type + size, checks min <= size <= max, `memset(buffer, 0, size)`, calls decode |
| Receive buffers | 0x405BDC, 0x405EB0 | **0x10000-byte stack buffer** (frame 0x10050) -> any size <= 0xFFFF fits. Third caller 0x442529 not checked yet. |
| Sender | 0x396828 | two payloads on its stack: P1 = rbp+0x4350, P2 = rbp+0x68A0 (0x2550 apart; P2 ends at the GS cookie rbp+0x8DF0). Ctor loops 0x396865..0x3968EC (peer ctor 0x396E44 x17, player ctor 0x396E60 x16 per payload). Producer called with r9 = P2 (0x396A21) / P1 (0x396A4D). Send: `movl $0x2550, 0x20(%rsp)` at **0x396C54** -> 0x3971AC -> 0x3F9620. |
| Producer | 0x39DD4C(block, other?, ..., payload r9) | `memset(payload, 0, 0x2550)` at 0x39DD78; peer loop over 17 block peers (`cmp $0x11,%r12d` 0x39DED8; record pointers step `add %rcx,%rbp` / `add %rcx,%rbx`, rcx = 0x158); player loop (`cmp $0x10,%r13d` 0x39E228, `add $0xb0` 0x39E212/0x39E219). Entry address = count*0x180 (`lea (rdx,rdx,2)` + `shl 7`) + 0x18, count*0xB8 (`imul $0xb8`) + 0x1998. |
| Encoder | 0x42675C | inline bit writer. Width register `mov $0x5,%r12d` (0x4268BB) for 8 fields: peer count, player count, peer entry index, a peer-index field in each player entry (0x426C0A), 4 tail fields (0x426EB2, 0x426F31, 0x426FB4, 0x426FEC). Inline immediates: `shl $0x5,%rax` at 0x4268D1 0x42690A 0x42695A 0x426C1A 0x426EC5 0x426F44 0x426FC4 0x426FFC, plus `lea 0x5(%rcx)` at 0x426900 0x426EBB 0x426F3A (check each). Player index (4 bits) at 0x426B45..0x426B65 (`cmp $0x4` / `add $0x4,%ecx` / `addl $0x4,0x24(%rbx)` / `shl $0x4,%rax` / `mov $0x4,%r8d`). Loops: peers `add $0x180,%rbp` 0x426B04 (rbp = entry+0x1C), players `add $0xb8,%rbp` 0x426DC4 (rbp = entry+8). |
| Decoder | 0x427140 | width `mov $0x5,%r13d` (0x4271B9) for the same 8 fields; player index `mov $0x4,%edx` 0x42730F. Limits 0x4271D9 `cmpw $0x22,0x14(%rdi)` and 0x4271E8 `cmp %r15w,%ax` (r15 = 0x20) (EnginePatches security fix sets 0x11 / `cmp $0x10`). Index validity: 0x427226 `cmpl $0x10,-0x8(%r14)` (peer <= 16), 0x427333 `cmpl $0xf,-0x8(%r14)` (player <= 15). Loops: peers `add $0x180,%r14` 0x4272E7 (counter r15d already incremented), players `add $0xb8` 0x427458. |
| Consumer | 0x39E47C(block, payload) | player loop `add $0xb8,%rbp` 0x39E54A (counter r15d), peer loop `add $0x180,%r15` 0x39E5E8 (counter r12d), more peer loops ~0x39E6F2 / 0x39E805. Block indexing: imul 0x158 at 0x39E4C7 (caved), **0x39E57F (not caved)**, 0x39E6B5 (caved), **0x39E6CE (not caved)**, imul 0xB0 0x39E7AD (caved), 0x39E897 (caved). Masks: `bt` on block +0x2C (peers, DWORD) and +0x170C (players). |
| Payload layout | | +0x08/+0x0C/+0x10 ids, +0x14 i16 peer count, +0x16 i16 player count, peers at +0x18 (17 x 0x180), players at +0x1998 (16 x 0xB8), tail +0x2518..+0x254F (flags/values at +0x2530..+0x2540). |

## Design (32-player mode only)
- **Extension at a fixed offset C from every payload** (P1, P2, the 0x10000 receive buffer): peers 17..31 at
  `payload + C + (i-17)*0x180` (0x1680 bytes), players 16..31 at `payload + C + 0x1680 + (j-16)*0xB8` (0xB80).
  Extension size E = 0x2200. P1 and P2 are 0x2550 apart and E < 0x2550, so ext(P1) and ext(P2) never overlap.
- **C = 0x8100**: ext(P1) = rbp+0xC450..+0xE650, ext(P2) = rbp+0xE9A0..+0x10BA0. 0x396828's frame (now 0xC540
  after plan C) grows to cover rbp+0x10BA0 (+ pushes). Registered size and send size = C + E = **0xA300**
  (< 0xFFFF, < 0x10000 receive buffer).
- Entry address deltas once past stock: **Dpeer = C - 0x1998 = 0x6768**, **Dplayer = C + 0x1680 - 0x2518 = 0x7268**.
  - pointer-stepping loops (encoder, decoder, consumer): step cave `add ptr, rec; cmp counter, first; jne; add ptr, D`.
  - count-indexed writes in the producer: cave after the index scaling adds D when index >= first.
- **Wire**: width 5 -> 6 for the 8 r12/r13 fields (counts 0..32, peer indices 0..31), player index 4 -> 5 bits.
  Decoder limits: peer count <= 32, player count <= 32; index checks peer <= 31, player <= 31.
- Ext entries are constructed (peer ctor 0x396E44 / player ctor 0x396E60 on zeroed memory) by a Noble detour on the
  producer before it runs (it gets the payload pointer); the decoder side is zeroed by the decode wrapper's memset.
- Consumer: cave 0x39E57F and 0x39E6CE like the other imul 0x158 sites (peer >= 17 -> shadow session).

## Status
**Implemented 2026-11-09, built, LAN-tested 10:51 ET: works with 3 players.** Host + 2 clients (one PC, `overnight_lan3.bat`):
host `SEND msg 26 size 41728` (0xA300) x15, both clients `RECV msg 26 size 41728 decoded true`, session proceeds (27/34/35/37/44),
all exit 0. Registered size read back 0xA300 (the code patch at 0x427EA9 is in before registration; the runtime table fix-up in
`CheckRegisteredSize` stays as a safety net). Not yet exercised: the extension paths (> 16 players / > 17 peers); the high-slot test reaches them but fails earlier (below).
Earlier 10:39 run of the same build (without the producer log) did not join: host never sent 26. Not reproduced since; rerun a few
times to see if the join is flaky. Only active with
`-slot-patch -slot-players 32` + `NOBLE_SLOT_PLAYERS_UNSAFE=1`; part of the all-or-nothing 32-player checks.
- `src/host/SlotMessage.cpp/.hpp`, `src/host/SlotMessageTable.inc` (generated by `tools/limits/gen_message.py`, which also
  checks every original byte against a stock DLL and can print each cave's disassembly: `python gen_message.py <stock dll> <out.inc> verify`).
- 16 caves: 2 encoder + 2 decoder + 4 consumer step loops, 2 consumer imul sites (0x39E57F, 0x39E6CE -> shadow), 4 producer
  entry-address sites, 2 producer loop ends (block record pointers -> shadow at the first shadow slot, bound 32).
- 25 immediate edits: encoder/decoder widths (5 -> 6, player index 4 -> 5), decoder limits (count <= 32) and index checks
  (<= 31), sender size and registration size (0x2550 -> 0xA300). The two decoder limit sites accept the EnginePatches
  security-fix bytes as original too.
- Producer call sites 0x396A28 / 0x396A57 -> thunk -> `ProducerWithExtension` (zeroes the payload extension and runs the
  peer / player entry ctors on it), then the stock producer.
- 0x396828 frame 0xC540 -> 0x10CC0 (SlotPatch.cpp kFrame396 + unwind copy edits).

## High-slot test (2026-11-09 11:01)
`NOBLE_SLOT_HIGH=1` (SlotMessage `ApplyHighSlotTest`): free-slot searches start at peer 17 (0x3953BC) and player 16 (0x39D69A)
once the mask is non-empty, so a 3-instance LAN test uses shadow slots. Added edit 0x4273B0 (decoder owning-peer check
`mov $0x10` -> `0x1F`); before it the client decoded message 26 as false. After it every message 26 decodes, but the host
adds player 16 and removes it 16 ms later (producer #2 2 players, #3 1 player), the guest leaves and its rejoin is refused.
The message itself is not the failure; see LIMITS-HANDOFF.md (uncaved player add/remove sites 0x39D620 / 0x39D7F8 / 0x39D8A8 /
0x39D950).

## Risks / to verify
1. **Extension paths only run with > 16 players / > 17 peers.** A three-window LAN test exercises the new wire widths
   (encoder and decoder must agree, counts are 6 bits on the wire) but not the extension. Needs either a synthetic
   self-test (fill a payload with 32/32 entries, encode + decode through the engine wrappers 0x423D88 / 0x423E40, compare)
   or a session with > 16 machines.
2. Third decode caller **0x442394** decodes into a static buffer at RVA 0x4E3BB80 with its own collection (`rbx+0x28`). If
   that collection is the same table, a 0xA300 membership message would overrun it. Verify at runtime (log the type/size
   seen there) before any > 16 test.
3. 0x396828 now has a 0x10CC0 (68 KB) stack frame; fine on the main thread, check if it ever runs on a small-stack thread.
4. Fields the consumer reads from extension entries come only from the decoder (decode buffer is memset by the wrapper).
