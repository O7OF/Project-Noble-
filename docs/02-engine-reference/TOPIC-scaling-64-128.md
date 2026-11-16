# TOPIC - Scaling multiplayer past 32 players (64 / 128)

Status: 2026-11-16. 32-player mode is dev-gated and works at install + 2-client level; the high-slot seat is
proven. 64/128 are planned, dev-gated, and are being built so they can be validated *without* fielding 64 or 128
clients. This note is the design reference for that work; the executable evidence is the headless self-test
`noble-dev.exe -test-slot-capacity`.

## 1. Where 32 players stands (2026-11-16)

- The 32-player patch set installs all-or-nothing and is byte-checked: **120 slot address sites** (67 table sites +
  53 member caves), 139 loop bounds, 23 peer/team masks, 8 step loops, the #32 membership message extension
  (payload 0xA300, extension at +0x8100), pools raised to 32 (players, fire team, simulation view arrays), and
  shadow sessions for peers >= 17 / players >= 16.
- **High-slot seat fixed (0fdcf97):** the join loop was not the player-add path but `FUN_18044FC94`, the per-frame
  duplicate-user/seat check. Its loop bounds had already been widened to 32, but its peer-record stride was stock, so
  peers >= 17 read player-record memory and the check booted the joining peer with reason 9 every frame. Caving
  `0x44FF46` (plus `0x397424` / `0x397503` / `0x39752A` player-add acceptance and `0x39FB62` / `0x39FB7D` queued
  player-add apply) makes the check read the real shadow record: peer 17 now reaches state 8, the delta is accepted on
  both sides, the host stops booting, and both clients enter gameplay (host `game session started` 13:58:43, guest
  13:59:02; 0 BOOT lines, 0 CLOSE, 0 crashes).
- Multi-high-slot probe (4 instances, forced slots): two high peers seat at once (host peer mask 0x00060001, states
  p0:8 p17:8 p18:8). The third guest fell back to a normal slot and channels closed; whether that is a real limit or
  the same-PC shared identity being treated as a duplicate user is still open (test with distinct accounts).
- 32 **real** players still needs 32 engine sessions (~160-190 GB) - see the hardware ceiling in the compendium 4.5.

## 2. Why 32 -> 64/128 is not just a threshold bump

`SlotPatch` hides overflow records in Noble-owned **shadow session objects**. Today one shadow block per static
session reuses the stock array offsets because 15 peer / 16 player / 15 table shadow records fit into the gaps of the
stock session layout:

| array | stock offset | record | first shadow slot | 32 shadow bytes |
|---|---|---|---|---|
| peers A | session+0x60 | 0x158 | 17 | 0x1478 |
| players A | session+0x1740 | 0xB0 | 16 | 0x0B00 |
| peers B | session+0x2270 | 0x158 | 17 | 0x1478 |
| players B | session+0x3950 | 0xB0 | 16 | 0x0B00 |
| 17 x 0xC peer table | session+0x446C | 0xC | 17 | 0xB4 |

At capacity C the peer array needs (C-17)*0x158 bytes: 0x3F28 at 64, 0x95B8 at 128. Those are larger than the space
below the stock player array, so the shadow object needs a new layout. Because a static session's A/B blocks are only
0x2210 apart, a single shadow delta can also no longer map both at those sizes: **64/128 need two shadow blocks per
static session** (one for block-A accesses, one for block-B) and therefore per-site A/B classification of every
cave/step site.

The constexpr model added in `src/host/SlotPatch.cpp` makes this explicit and validates it:

| capacity | blocks | peer shadow | player shadow | table shadow | biases A (peer/player/table) | biases B |
|---|---|---|---|---|---|---|
| 32 | 1 | 0x60 | 0x1740 | 0x446C | 0 / 0 / 0 (stock, runtime unchanged) | 0 / 0 / 0 |
| 64 | 2 | 0x60 | 0x3F90 | 0x6090 | 0 / 0x2850 / 0x1C24 | 0x5F458 / 0x61BE8 / 0x6107C |
| 128 | 2 | 0x60 | 0x9590 | 0xE290 | 0 / 0x7E50 / 0x9E24 | 0x5F458 / 0x672E8 / 0x6807C |

All three layouts are static_assert-ed to fit inside `kSessionStride = 0x61668` with no overlapping arrays.

### 2.1 Two-block vs chunked shadow layout (decision)

The two-block model above keeps each shadow array contiguous (good for the pointer-stepping loops and the whole-array
copy/clear mirrors) but needs every cave/step site tagged with the block it addresses. That tagging is ambiguous for
the generic helpers: e.g. `FUN_18039D620` (player add/update) takes a block pointer and its callers pass block-relative
bases, so the same caved site can run for block A and block B at different times. A single constant cannot point A and
B at two different shadow blocks.

The alternative is a chunked layout that keeps one constant per index range: split every shadow array into chunks that
fit below the 0x2210 A->B distance, and place chunk A exactly 0x2210 before chunk B. A site's cave then only needs a
piecewise `if index < boundary: add Ck` dispatch and works for either block base, because the +0x2210 relationship is
preserved inside each chunk pair. Chunk capacity: 25 peer records, 49 player records, 726 table entries per chunk.

| capacity | peer chunks | player chunks | table chunks | chunked shadow bytes | piecewise member-cave size (est.) |
|---|---|---|---|---|---|
| 32 (current, stock layout kept) | 1 | 1 | 1 | 0xCB88 (upper bound) | 24 B chunk dispatch; stock single-range cave stays 41 B |
| 64 | 2 | 1 | 1 | 0x10F38 | 53 B (worst kind: peer) |
| 128 | 5 | 3 | 1 | 0x261C8 | 140 B (worst kind: peer) |

The self-test also simulates the dispatch itself: for every record index it checks that the per-range constant maps
both the block-A base and the block-A base + 0x2210 (block B) to the intended chunk offset (`+ (index - first) *
record`), that chunk pairs end inside `kSessionStride`, and that chunks of one kind cannot overlap. Cost of the
chunked model: the member-cave entry must grow from 48 to >=160 bytes (worst measured piecewise cave is 140 B at
128; 53 sites -> ~8.5 KB of cave code, fine), and the pointer-stepping loops (9 step caves) and whole-array
copy/clear mirrors must hop chunk boundaries instead of assuming one contiguous array. The validated plan is to use
the chunked model for >32 capacity and keep the proven stock-relative single-block layout (biases 0) for 32.

## 3. Wire and message limits (measured/bounded, not yet fixed)

| limit | stock | 32 (current) | needed for 64 | needed for 128 |
|---|---|---|---|---|
| membership peer index field | 5 bits | **6 bits** (patched) | 6 bits (max 64 peers; 64 is the ceiling of this field) | 7 bits |
| membership player index field | 4 bits | **5 bits** (patched) | 6 bits | 7 bits |
| peer valid mask | 17-bit `0x1FFFF` checks | 32-bit DWORD patches | 64-bit (2 words) | 128-bit (4 words) |
| membership extension bytes | - | 0x2200 (15 peers x 0x180 + 16 players x 0xB8) | 0x6900 | 0xF700 |
| membership message size | 0x2550 | 0xA300 (base 0x8100 + ext) | 0xEA00 - fits a 16-bit length | 0x17800 - **exceeds 64 KiB**, needs message/framing work |
| game-player event index | 4 bits | 5 bits | 6 bits | 7 bits |
| team-count / game-over loops | 16 teams | 32 teams patched | 64 (team masks are 32-bit DWORDs in several places) | 128 (multi-word masks) |

The 16-bit message limit for 128 is the biggest open protocol question: the channel currently registers min/max
message sizes per type (`0x4E38D28` table, membership entry +0x14/+0x18); whether the network layer accepts a
message above 64 KiB or needs the membership update split into batches has to be established before 128 can work.
Reach Renaissance (81-player ceiling; its 128 profile is two players per machine and is not exposed) is the closest
external calibration.

## 4. Engine/pool and UI work still required

- Pools: players / fire team / simulation view arrays are raised to 32 today; 64/128 needs the same raises plus the
  AI-actor/particles/light-volume profile cost and the per-client memory budget (a 128-player host would hold 128
  peer + 128 player records plus the shadow copies).
- `set_slot_counts` churn: the host currently calls the setter ~62x/s because the engine re-applies 16 and the hook
  re-expands; the engine's own session-creation slot count (or a one-shot latch) must be fixed for 32/64/128 rather
  than post-hoc expansion.
- UI: EffectiveMultiplayerSeats() caps at 16 stock / 32 dev; picker, lobby roster, loading roster, scoreboard, server
  browser counts and rows all need a capacity-aware path (dev-gated; no user-facing cap change without Tyler).
- Dedicated server is still a stub (no engine session), so large-session testing still needs 3-4 PCs x 4-5 instances.

## 5. How this is validated without 64/128 clients

1. `noble-dev.exe -test-slot-capacity` - headless, validates every layout (offsets, biases, overlap, stride, mask
   words, extension/message sizes, required wire bits, chunk counts/limits, shadow bytes) and **simulates the chunked
   dispatch**: for every record index it checks that the per-range constant maps both the block-A base and the block-A
   base + 0x2210 (block B) to the intended chunk offset, that chunk pairs end inside kSessionStride (0x8614 / 0xEAA4 /
   0x21A14 for 32/64/128) and that chunks of one kind cannot overlap. Exit 0 on 2026-11-16 for all three capacities:
   32 unchanged (ext 0x2200, msg 0xA300); 64 ext 0x6900 / msg 0xEA00 / peer 6 bits ok / player needs 6 / worst cave
   53 B; 128 ext 0xF700 / msg 0x17800 needs framing / peer+player 7 bits / 5 peer + 3 player chunks / worst cave 140 B.
2. Install-level sessions: declare C slots and join with 2 clients (proves shadow storage, message registration,
   encoder/decoder validity at capacity even without C clients).
3. Forced high slots (`NOBLE_SLOT_HIGH=1`, free-slot scans start at peer 17 / player 16) can place a handful of
   real clients at slots 17, 18, ... to exercise multiple shadow peers/players end-to-end; extend it per slot once
   the chunked-layout cave install lands.

## 6. Ordered work list

1. Install the chunked layout: grow the member-cave entry (48 -> >=160 B; measured worst case 140 B at 128), teach
   the generator to emit piecewise index-range constants (25 peer / 49 player / 726 table records per chunk), and
   make the 9 pointer-stepping step caves plus the whole-array copy/clear mirrors hop chunk boundaries. Keep the 32
   path byte-identical.
2. Wire the capacity into `SlotPatch` install behind dev env gates (e.g. `NOBLE_MP_SLOTS=64|128`), one capacity at
   a time, with all-or-nothing byte checks and the install-level validation from section 5.
3. Widen the membership encoder/decoder fields (peer/player index bits, multi-word masks) and the game-player event
   index; add round-trip tests for synthetic peer/player counts.
4. Solve the 128 message-size limit (channel/framing or batched membership updates); 64 fits the current 16-bit length.
5. Raise engine pools and the session slot count at 64/128; re-measure memory and the setter churn.
6. Generalize the UI surfaces (dev-gated) and the harness/test plan; measure with the multi-PC rig when available.
