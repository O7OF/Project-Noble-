# NOTE: high-slot (peer >= 17 / player >= 16) player-16 removal, close reason 10, connect-refused 3

Date: 2026-11-16. Author: ghidra-joinpath (static Ghidra analysis of `haloreach-copy.dll`, image base
0x180000000, sha256 738dd2d2...8b0894; no game instance was run). Repo HEAD 4bfe827, 0.19.103.
Scope: explain the high-slot ("NOBLE_SLOT_HIGH=1") failure, list the exact engine call chain that clears the new
player, explain CLOSE reason 10 / CONNECT REFUSED reason 3, and give concrete patch candidates with RVAs and
stock bytes. All RVAs below are repo RVAs; Ghidra address = 0x180000000 + RVA.

## 0. Bottom line (read this first)

1. **Removal primitive.** Player removal is `FUN_18039D950(base, playerIndex)` (clears the player mask bit,
   decrements the player count and memsets the 0xB0 player record). It is reached from `FUN_18039D7F8`
   (remove wrapper, releases the engine player object first). Verified callers of the pair:
   * `FUN_180429FC0` in-band message dispatcher, **message type 0x20 = "player-remove"** handler
     (branch at 0x42AAAE, call site **0x42AB61**) - host side;
   * `FUN_180394D48` local-user drop helper (call site **0x394E03**) - host/session-owner side;
   * `FUN_18039E47C` membership consumer delta apply (calls 0x39E53E / 0x39E59B via `FUN_18039C6B4`);
   * `FUN_18039C8DC` peer-removal path (call site 0x39CA75);
   * `FUN_18039C6B4` membership reset (call site 0x39C6DF).
2. **The observed 16 ms remove is the guest-originated player-remove message.** The guest's per-frame
   local-user reconcile `FUN_18038F49C` decides "the player mapped to my local user is not mine"
   (mask bit set + mapping != -1 + player-record id != local user id) and calls `FUN_180394D48`
   (call site **0x38F77B**). Outside in-game states 6..9 (i.e. in the lobby / loading, which is exactly when the
   high-slot join runs) that helper does **not** remove locally: it sends session message type 0x20
   "player-remove" to the host peer (0x394DF6/0x394E0A, sender `FUN_1803971AC(session,0,hostPeer,0x20)`).
   The host's player-remove handler then clears the player mapped to (sending peer, user slot): the mapping
   lookup at 0x42AB43 (already caved for peer >= 17), the "is this the local peer" guard at 0x42AB24
   (cmp eax,[rsi+0x4464]) and the mapping==-1 skip at 0x42AB54, then 0x42AB61 `FUN_18039D7F8(rcx=session+0x30,
   edx=mapping[peer][slot])`. That is why the host's membership producer next frame prints 2 -> 1 players.
   The 16 ms is one network frame (guest reconcile -> msg 32 -> host apply).
3. **Close reason 10 is a consequence, not a cause.** `FUN_18043B98C(channel, reason)` is the channel close
   primitive (Noble hooks it as CLOSE channel). The reason-10 call is inside `FUN_180429AA4`, the **out-of-band**
   session message dispatcher, case **type 7 = "connect-closed"**: after sanity checks it does `uVar5 = 10;
   FUN_18043B98C(channel, 10)` at **0x429BEE**. So "CLOSE channel ... reason 10 (state was 5)" means the host's
   engine received the guest's OOB connect-closed message (the guest already gave up) and tore the channel down.
   The guest's own side logs reason 14 (channel no longer referenced by a session).
4. **CONNECT REFUSED reason 3 is also a consequence, and the log direction is easy to misread.** The Noble hook
   "CONNECT REFUSED" is installed on RVA **0x42459C**, which is the *encoder* of in-band message 0x05
   "connect-refuse" (registration `FUN_18042489C`, entry +0x140; encoder writes a 32-bit id then a 3-bit reason).
   So the line means "**this process sent** a connect-refuse", not "this process received one". The reason-3
   sender is `FUN_1803F8C60` (handler of OOB type 4 "connect-request"): it looks for a matching session and,
   when none is found / the request does not validate, falls to LAB_1803F8D3F (`uVar14 = 3`, send at
   **0x3F8D6F**). Confirmed with today's log (build\bin\Release2\client2-base2s32-20261116-105330.log):
   `CONNECT REFUSED: id 0x9 reason 3 ... stack: hr+0x423E2E <- hr+0x405E11 <- hr+0x3F8D6F <- hr+0x429BFB
   <- hr+0x405CAE ...` - i.e. the **guest** refused stale reconnect attempts *after* its session was gone.
   It is a symptom of the session teardown, not the cause of the join failure.
5. **It is not a spawn/unit failure.** The player-remove chain contains no unit/object-spawn validation: the
   host handler only checks session state, sender peer, user slot <= 3 and the peer->player table; the only
   object-ish call is `FUN_180400B70` inside `FUN_18039D7F8` (release of the engine player object when the
   owning peer's state > 4). The drop decision is pure membership/table validation.
6. **Fork still open (needs the trace in section E).** *(2026-11-16 follow-up: superseded - see section I. The 7-vs-8 peer-state evidence and the 0x44B24C root cause are there.)* In the *current* binary `FUN_18039CBF0` (the
   "sender address -> peer index" resolver used by the 0x20 handler at 0x42AAFC and by the player-add handler)
   scans only the original 17 peers: `cmp eax,0x11` at **0x39CC14** and `cmp r8d,0x11` at **0x39CC4E**.
   For a slot-17 sender it returns -1, and the 0x20 handler then **skips** the remove. Therefore either the
   current build's removal comes from one of the other D7F8/D950 callers (membership consumer / peer removal /
   drop helper), or the removal path is reached with a lower peer index. The instrumentation list in section E
   settles this in one run; section D ranks the patches that are needed either way.

## A. Reproduction summary (from the project docs)

* 32-player mode = `-slot-patch -slot-players 32` + `NOBLE_SLOT_PLAYERS_UNSAFE=1`; the slot patch allocates
  shadow session objects at a fixed delta from the four static sessions (0x2B07418, stride 0x61668) and
  redirects every peer >= 17 / player >= 16 slot address into them (110 sites).
* Normal-slot 2-instance LAN join works (2 peers/2 players, pmask 3, guest decodes the 0xA300 membership
  message, acks, msg 44 stream, no crash).
* High-slot test (`NOBLE_SLOT_HIGH=1`; `message::ApplyHighSlotTest`, caves 0x3953BC peer free-slot search and
  0x39D69A player free-slot search so the joiner lands in peer 17 / player 16) fails:
  host adds player 16 -> membership producer #2 prints "2 peers, 2 players" -> ~16 ms later #3 prints
  "2 peers, 1 player"; the guest sees its player gone, leaves and closes its channel; the host logs
  CLOSE channel reason 10; later reconnects are refused (reason 3).
* History that narrowed it: 2026-11-10 02:17 trace (docs/LIMITS-HANDOFF.md lines 26-48) shows the host
  receiving four msg-32 player-remove messages (one per user 0..3) and removing the guest's player "as asked";
  the guest's four sends came from the per-frame reconcile reading the LOCAL peer's player table through
  uncaved 0x158/0x56 math. Four caves were added (0x38F6E0/0x38F723/0x38F993/0x38F9A7, present today) and the
  later hand-added caves (0x42AB43, 0x394C38, 0x394D8A) covered the host handler and the drop helper.
  The remaining question is why the guest still decides its player is foreign.

## B. Removal call chain (verified, addresses + decompiled conditions)

### B.1 guest decision: per-frame local-user reconcile `FUN_18038F49C` (RVA 0x38F49C, 1429 bytes)

Runs for every frame while session state is 4..11 (first checks at 0x38F4xx). For each local user (u) and the
local peer index P = `[sessionBase+0x4464]` it evaluates (disassembly 0x38F6D4..0x38F70D):

```
0x38F6D8  mov  eax,[rbp+0x2c]          ; peer mask
0x38F6DB  bt   eax,ecx                ; peer mask bit P
0x38F6DE  jnc  skip                    ; no peer => bVar5 = true
0x38F6E0  imul rcx,rcx,0x56           ; [caved: 0x38F6E0]
0x38F6E4  add  rcx,r15                 ; r15 = 0xC, base = rbp+0x174
0x38F6E7  movsxd rax,[rbp+rcx*4+0x174] ; mapping[P][u]  (0x174 + 4*(P*0x56+0xC))
0x38F6EF  cmp  eax,-1
0x38F6F2  jz   ...
0x38F6F9  imul rax,rax,0xb0            ; [caved via SlotPatchTable.inc 0x38F6F9]
0x38F700  mov  rax,[rbp+rax+0x1714]    ; player record xuid (rbp = session+0x30, record base +0x1710, xuid +4)
0x38F708  cmp  rax,[r9]                ; local user xuid
0x38F70B  jz   ...
0x38F70D  mov  r8b,1                   ; bVar5 = id mismatch
...
0x38F77B  call FUN_180394D48           ; drop local user  (only caller)
```

Decompiled decision (FUN_18038F49C): the drop branch is taken when **mask bit P set AND mapping[P][u] != -1
AND the mapped player record's xuid != the local user's xuid**; the same block is gated by a re-try timeout
(`DAT_180d1fd40`), so the guest retries about once per ~16 ms window. The 2026-11-10 analysis identified this
as the source of the four msg-32s; the mapping reads and the player-record read above are now caved, so a
remaining mismatch has to come from the *contents* the guest's membership consumer wrote for shadow player 16
(mapping value, owning peer/slot, or the xuid field), or from a different peer index.

### B.2 guest send: `FUN_180394D48` (RVA 0x394D48)

```
0x394D48  reads mapping[localPeer][u] = iVar1  (0x394D8A imul rcx,rax,0x56 - caved)
0x394DF0  add eax,-6 ; cmp eax,3 ; ja  send_branch   ; in-game states 6..9 -> direct remove
0x394DF8  cmp esi,-1 ; jz return
0x394E03  call FUN_18039D7F8(sessionBase+0x30, iVar1)   ; host/owner path removes locally
...
0x394E0A  send_branch: builds a 12-byte payload { local id, user index (2 bits) }
0x394E4B  call FUN_1803971AC(sessionBase, 0, [sessionBase+0x40], 0x20)  ; msg 32 player-remove -> host peer
```

Stock bytes at the branch (verified): `83 C0 FA | 83 F8 03 | 77 12 | 83 FE FF | 74 56 | 8B D6 | 48 8D 4B 30 |
E8 F0 89 00 00 (call FUN_18039D7F8) | EB 46 | 8B 8B C8 14 06 00 (0x394E0A load)`.
Note the direction: in **lobby/loading** (states outside 6..9) even the *host* removes by sending msg 32 to the
**host peer index** (session+0x40); on the host this normally targets itself/host. For the high-slot case the
guest is the state<6 sender, so the host applies it.

### B.3 host apply: in-band dispatcher `FUN_180429FC0` (caller `FUN_18043BAE8`), type 0x20 handler

Message type table: DAT_184E38D28, 0x40-byte entries; registration `FUN_180427E8C` shows
type 26 (0x1A) membership-update, 30 (0x1E) player-add, 31 (0x1F) player-refuse, **32 (0x20) player-remove**,
33 (0x21) player-properties. Handler dispatch is `FUN_18043BAE8 -> FUN_180429FC0(sessionMgr, channel, ?, type)`.

```
0x42AAAE  cmp [rdx+0x110c],5        ; channel state == 5
0x42AABF  session = FUN_1803FA158(sessionList, &DAT_184e3bb80) ; session by sender address
0x42AAD7  state checks 6..9
0x42AB01  uVar5 = FUN_1803F93FC(state-4, session->0x20)        ; sender record id
0x42AB12  peer = FUN_18039CBF0(session+0x30, uVar5)           ; peer index  [17-bound issue! see B.5]
0x42AB1B  cmp eax,-1 ; jz   skip
0x42AB24  cmp eax,[rsi+0x4464] ; jz skip                     ; sender != local peer
0x42AB30  mov r9,[0x184e3bb88]    ; decoded user/slot index
0x42AB37  cmp r9d,3 ; ja skip                                 ; slot 0..3
0x42AB43  imul rdx,rax,0x56 ; add ...                         ; [caved: 0x42AB43, peer >= 17 -> shadow]
0x42AB4D  mov edx,[rsi+rdx*4+0x1A4]                           ; mapping[peer][slot] = player index
0x42AB54  cmp edx,-1 ; jz skip
0x42AB5D  lea rcx,[rsi+0x30]
0x42AB61  call FUN_18039D7F8                                  ; remove that player
```

### B.4 the primitive: `FUN_18039D7F8` -> `FUN_18039D950`

`FUN_18039D7F8(base, playerIndex)`:
* if player record's owning peer (`[base+player*0xB0+0x16EC]`-style / peer index at base+0x171C+player*0xB0)
  has state > 4: `FUN_180400B70(engineId, base+player*0x16+0x2EA, 1)` (release the engine player object);
* `FUN_18039D950(base, playerIndex)`;
* bumps the two session counters (base+8 / base+0x4438-ish) and calls `FUN_18000D6B8` if the local-peer
  condition holds.

`FUN_18039D950(base, idx)` (stock bytes verified):
```
4C 63 DA                         movsxd r11,edx
41 83 C8 FF                      or r8d,-1
4D 69 D3 B0 00 00 00             imul r10,r11,0xB0
4C 03 D1                         add r10,rcx            ; player record
45 39 82 1C 17 00 00             cmp [r10+0x171C],r8d  ; owning peer != -1
74 30 45 39 82 20 17 00 00       ...                    ; user slot != -1
...
44 89 84 82 74 01 00 00          ; mapping[peer][slot] = -1, peer count--
...
x & ~(1<<idx)                    ; player mask bit clear (base+0x170C)
count--                          ; base+0x1708
memset(base+0x1710+idx*0xB0, 0, 0xB0)
```
This is the exact write that makes the membership producer print one player less.

### B.5 the 17-bound in the sender->peer resolver (blocks the 0x20 attribution for slot 17)

`FUN_18039CBF0(sessionBase, recordId)` (RVA 0x39CBF0, 127 bytes), called by the 0x20 handler (0x42AB12) and
by the player-add handler (0x44B080 at 0x44B0E5):
* walks the peer mask (`[rcx+0x2C]`) starting at peer 0 while `cmp eax,0x11` (RVA **0x39CC14**) and searches the
  0x0C-stride table at `[sessionBase+0x4440]` (`cmp [r10+rax*4+0x4440],r9d`) while `cmp r8d,0x11`
  (RVA **0x39CC4E**).
* For a peer-17 sender it returns -1 -> the 0x20 handler skips `FUN_18039D7F8`, and the player-add handler
  `FUN_18044B080` skips its insert branch (`iVar2 != -1` guard).
This is a **required correctness fix** for any peer >= 17 message handling, but by itself it would let the host
obey the guest's wrong player-remove. Order it after the guest-side cause is fixed (section D).

## C. Close reason 10 / CONNECT REFUSED 3 (semantics + source addresses)

### C.1 close reason 10

* `FUN_18043B98C(channel, reason)` (RVA 0x43B98C) is the channel close primitive (only writer of channel state
  2; Noble logging hook). Reason values are caller-chosen codes, not an enum lookup.
* Caller for reason 10: `FUN_180429AA4` (RVA 0x429AA4, out-of-band session message dispatcher by type).
  Message type **7 = "connect-closed"** (registration `FUN_180424370`: type 7 entry name at +0x1c8
  "connect-closed"; the same registration function names type 2 broadcast-search, 3 broadcast-reply,
  4 connect-request, 5 connect-refuse, 6 connect-establish). At **0x429BEE** the case-7 code path sets
  `reason = 10` and calls `FUN_18043B98C`; case 5 (connect-refuse received) closes with reason 5.
* So CLOSE reason 10 on the host = "the peer sent out-of-band connect-closed" (the guest's teardown). The
  guest-side reason 14 is `FUN_1803F8A7F`-family (channel no longer referenced by any session). Both are
  aftermath lines; neither is the root cause.

### C.2 connect-refused reason 3

* Wire message: in-band type **5 "connect-refuse"**, payload {u32 id, u32 reason:3 bits}, encoder
  **0x42459C** (`FUN_18042459C`), decoder 0x42462C. Noble's CONNECT REFUSED log hook sits on the encoder.
* Producer: OOB type 4 "connect-request" handler **`FUN_1803F8C60`** (RVA 0x3F8C60). It builds a candidate
  session from the request; if the session does not match / local state rejects it, `uVar14 = 3` and it sends
  the refuse via `FUN_180405CC8(..., 5)` (send at **0x3F8D6F**, LAB_1803F8D3F). Other returns on the same
  function (e.g. `uVar14 = 1` accept) confirm 3 = "no matching/joinable session for this connect-request".
* Evidence (2026-11-16 10:55 normal-slot run, guest log): the guest itself sent reason 3 to ids 0x9/0xA with
  stack `hr+0x423E2E <- hr+0x405E11 <- hr+0x3F8D6F <- hr+0x429BFB <- hr+0x405CAE <- hr+0x42379D <- ...`,
  i.e. OOB case 4 -> 0x3F8C60 -> refuse. This happens after its session went away; it is a consequence.

## D. Ranked patch candidates (32-player mode only, behind the existing all-or-nothing checks)

Legend: all "stock bytes" below were read from the Ghidra program (stock haloreach-copy.dll).

### P0 (do first, log-only): put evidence on the three candidate call sites

1. MinHook on `FUN_18039D950` entry (`4C 63 DA 41 83 C8 FF ...`) logging `ReturnAddress` RVA, base, player
   index, `[base+player*0xB0+0x171C/0x1720]` (owning peer / slot), and `[base+0x170C]` player mask. This names
   the real remover (0x42AB61 vs 0x394E03 vs 0x39CA75 vs consumer 0x39E53E/0x39E59B) in one run.
2. MinHook on `FUN_180394D48` entry logging (caller RVA, session state, local peer, mapping value, mapped
   player id, local user id) when called from the reconcile; and on the msg-32 send path (0x394E0A) log the
   payload id/user.
3. MinHook on `FUN_1803973BC` entry/return logging (peer index arg, slot arg, return refusal code) and on
   `FUN_18039FB34` (queued player-add apply) logging the queued peer index. Log in `FUN_18044B080` when it
   takes the refuse branch (iVar3 != 0) and sends type 0x1F.

### P1 (most likely root cause of a wrongful drop): guest shadow record contents, not missing caves

All reconcile slot/record sites are caved today (0x38F6E0, 0x38F723, 0x38F993, 0x38F9A7 in
SlotMemberCaves.inc; 0x38F6F9, 0x38F959 in SlotPatchTable.inc). The remaining ways to get "mapping[P][0]
!= -1 but record xuid != user xuid" are the *data* the consumer wrote for shadow player 16. Verify in the
P0 log which one it is; the two concrete candidates are:
* consumer add call `FUN_18039E47C` at 0x39E763: args are player index = payload entry+0x1998 (count-based
  entry, not player-index-based), xuid = entry+9, owning peer = sign-extended short entry+0x12, slot = byte
  entry+0x14, flags = entry+0x18/0x1C. Producer side writes the same fields from the source player record
  (record+0x4 xuid, +0xC peer, +0x10 slot), so a mismatch means the source shadow record itself is stale.
  Evidence line: log payload entry (index, peer, slot, xuid) on the guest right after decode/consumer.
* `ResetShadowPeerPlayerTables` was extended on 2026-11-09/10; confirm the *player* shadow slots 16..31 are
  constructed (InitShadowRecords constructs arrays {0x1740,0xB0,16} and {0x3950,0xB0,16}) and that the
  membership consumer's d8a8 write to them is not undone by the whole-block mirror
  (`MirrorCopy`: a block copied into a static session from a non-session block clears shadow slots 16-31 and
  logs `SlotPatch: block copied into session ... shadow slots 16-31 cleared`). If that line appears between
  producer #2 and #3 in the host or guest log, the just-applied shadow player was wiped and the consumer must
  be taught to re-fill it (or the mirror must copy the decoded block's extension into the shadow, which is the
  real 32-player design work).
Risk: data/ordering only; no engine-code risk.

### P2 (uncaved rejection path on player add): `FUN_1803973BC` "can this session accept player-add"

RVA 0x3973BC..0x3975D7, called from the host join processing `FUN_180392DB0` (call site 0x392E8B) and from the
player-add receiver `FUN_18044B080` (0x44B0C2 -> 0x44B080 -> 0x44B...); it decides accept (0) vs refusal codes
(7, 8, 0x0a, 0x0c, 0x10, 0x12, 0x14, 0x16, 0x1b, 0x20). It reads the owning peer's record with stock address
math and has **no caves at all**:

```
0x39740C  48 69 CE 58 01 00 00   imul rcx,rsi,0x158     ; owning peer record  (param_3 = peer index)
0x39741A  48 03 C1               add  rax,rcx            ; rax = session + 0x60 + peer*0x158
0x397426  8B A8 40 01 00 00      mov  ebp,[rax+0x140]    ; peer player count -> EBP
0x397440  41 83 BC 82 44 01 00 00 FF  cmp [r10+rax*4+0x144],-1 ; slot table entry for param_4
...
0x3974EE  83 FD 04               cmp  ebp,4
0x3974F1  7E 0A                  jle  ...
0x3974F3  BB 08 00 00 00         mov  ebx,8              ; refuse 8 ("peer has too many players")
...
0x397503  49 69 CD B0 00 00 00   imul rcx,r13,0xB0       ; existing player record (param_4 slot owner)
0x39750A  39 B4 39 4C 17 00 00   cmp  [rcx+rdi+0x174C],esi
0x39752A  48 6B CE 56            imul rcx,rsi,0x56       ; peer->player mapping index (new player branch)
```

For peer 17 all of these land in stock inline memory (inside the player array / block B) instead of the shadow
peer record, so EBP and the slot entries are garbage; `EBP > 4` then returns refusal 8, or the later branch
returns 0x16. Fix = extend the existing member-cave shapes to this function:
* 0x39740C (`48 69 CE 58 01 00 00`, 7 bytes): imul 0x158, threshold 17, scaleShift 0 -> cave 41 bytes with
  constant `delta - 17*0x158` (same shape as SlotMemberCaves 0x39C910/0x39D917 but registers rcx/rsi);
* 0x397503 (`49 69 CD B0 00 00 00`): imul 0xB0, threshold 16, constant `delta - 16*0xB0`;
* 0x39752A (`48 6B CE 56`, 4 bytes): the 0x56 dword-index mapping form, threshold 17, scaleShift 2,
  constant `(delta>>2) - 17*0x56` (same as the 0x38F6E0 family);
* 0x397440 / 0x3974A8 slot compares then need the same peer-table redirection: the slot entry is at
  `peerRecord+0x144`, i.e. the same peer record, so redirecting the record base (0x39740C) is enough for both.
Invariant to preserve: non-session blocks (stack membership copies) must stay at the stock address; use the same
"is this base a static session" guard as the table caves if the redirected pointer is reused as a block base.
Risk: medium-high - this is join-acceptance logic; a wrong delta refuses/accepts the wrong player. Byte-check
all originals at load (the project already fails 32-player mode if any site mismatches).

### P3 (uncaved on queued player-add apply): `FUN_18039FB34` -> `FUN_18039D620`

RVA 0x39FB34 (called by the player-add/reserve queue path; the queue is at session+0x4508, 4 x 0x80 records,
index at +0x4708). It reads the owning peer's player table and then calls `FUN_18039D620` (the add/update
helper) with the queued peer/slot:

```
0x39FB5E  48 63 47 08            movsxd rax,[rdi+0x8]      ; queued peer index
0x39FB62  4C 69 C8 58 01 00 00   imul r9,rax,0x158         ; UNCACHED peer stride
0x39FB69  4C 03 C9               add  r9,rcx
0x39FB70  49 63 84 91 74 01 00 00  movsxd rax,[r9+rdx*4+0x174] ; mapping[peer][user]
0x39FB7D  48 69 C8 B0 00 00 00   imul rcx,rax,0xB0         ; UNCACHED player record
0x39FB84  80 BC 19 28 17 00 00 00  cmp byte [rcx+rbx+0x1728],0
```
Fix: same two caves (imul 0x158 threshold 17 constant delta-17*0x158; imul 0xB0 threshold 16
constant delta-16*0xB0). Risk medium: only used when a player-add is queued; if the queue is not on the
high-slot path this patch is dormant.

### P4 (needed for peer>=17 message attribution; enable only after P1/P2/P3)

`FUN_18039CBF0` bounds: replace
* 0x39CC14 `83 F8 11` (3) -> `83 F8 20`; and
* 0x39CC4E `41 83 F8 11` (4) -> `41 83 F8 20`.
The 0x0C-stride table it searches at `[sessionBase+0x4440]` is the sibling of the caved 17-entry table at
+0x446C; a peer-17 lookup also needs that table's 18th..32nd entries to exist (shadow storage). Do not ship
P4 alone: today the resolver returning -1 is what prevents the host from executing the guest's player-remove;
widening it without fixing the guest-side drop makes the observed symptom *faster*, not better.

### P5 (harden the drop decision, only if P1 evidence shows the guest data is actually correct)

If the trace shows mapping[17][0] points at player 16 with the right xuid and the drop still happens, the
remaining suspect is the reconcile's mask/state path (session+0x2C vs +0x5C mismatch) or the user's xuid table
`DAT_182a1e94c`. In that case the local fix is to gate the guest's lobby-state drop: at `FUN_180394D48` the
`state < 6` branch (0x394DF6 `77 12`) could be changed to return 0 for shadow player indices (>= 16) until
the membership consumer has acked once; keep in-game (state 6..9) behaviour stock. Risk: this hides bugs and can
leave a stale player; not recommended before the trace.

## E. Evidence to collect (exact log lines / expected values)

One high-slot run with P0 logging installed is enough to pick the branch:

1. `SlotPatch: 32-player ... 110 slot address sites redirected` and the shadow delta - confirms the build
   under test has the current caves (include `delta 0x...`).
2. Guest, per frame around the join: `reconcile: localPeer 17 state 4 mapping[17][0]=16 recXuid=0x... userXuid=0x...`
   * mapping=-1 -> the consumer never added the player (P1 data path, block-mirror wipe line
     `block copied into session ... shadow slots 16-31 cleared` would confirm the wipe);
   * mapping=16 and xuids equal -> the drop comes from another condition (P5);
   * mapping=16 and xuids differ -> consumer wrote wrong xuid/entry fields (P1).
3. `player_remove send peer 0 slot n player16` (guest) followed by host
   `player_remove recv from peer 17 slot 0 -> remove player 16` and `D950(base,16) caller hr+0x42AB61`.
   If instead the caller is 0x39CA75 / 0x39E53E / 0x39C6DF, the removal came from peer-removal / membership
   delta / membership reset, not the msg-32 path - then the guest had already been dropped as a peer and the
   root cause is earlier (host join processing / player-add refusal).
4. Host: `player-add accept/refuse peer 17 ... ret 8` from P0 hook 3 and the following
   `player-refuse send 0x1F` line, or the absence of both. If the host's join path refuses with 8/0x16, P2 is
   the root cause of the failed insert and the CLOSE/refusals are aftermath.
5. Producer lines around the event: `membership producer #n payload ...: 2 peers, 2 players` then
   `... 1 player`; the value of `[session+0x170C]` (player mask) and `[session+0x173C]` must show bit 0 and
   bit 16 set before the removal and only bit 0 after (P0 hook 1 logs this directly).
6. Close side: `CLOSE channel ... reason 10 ... stack ... hr+0x429BEE` (host) and reason 14 on the guest;
   `CONNECT REFUSED: id 0x... reason 3 ... hr+0x3F8D6F <- hr+0x429BFB` (guest sending, after teardown).

## F. What is already patched (do not re-patch)

* Reconcile/consumer/add-remove slot math: SlotPatchTable.inc 67 sites (incl. 0x38F6F9, 0x38F959, 0x39D6FE,
  0x39D9F4, 0x39E4C7, 0x39E6B5, 0x39E7AD, 0x39E897), SlotMemberCaves.inc 52+ hand-reviewed sites (incl.
  0x38F6E0/0x38F723/0x38F993/0x38F9A7, 0x39D63F/0x39D659/0x39D808/0x39D817/0x39D8E1/0x39D917/0x39D957/
  0x39D97A, 0x39C910/0x39C99C/0x39CA11, 0x42AB43, 0x394C38, 0x394D8A), plus the #32 message caves and the
  32-entry peer table caves.
* Loop bounds: SlotBoundsTable.inc 139 + kPeerMasks32 (incl. 0x39E8A7 consumer local-peer scan 0x11 -> 0x20).
* NOT patched anywhere: `FUN_1803973BC` (0x39740C / 0x397503 / 0x39752A), `FUN_18039FB34` (0x39FB62 /
  0x39FB7D), `FUN_18039CBF0` bounds (0x39CC14 / 0x39CC4E), and the OOB dispatcher's msg-7 close path is
  stock (no change wanted).

## G. Open questions for the next session

1. Is the host's player-add message (type 30) part of the high-slot flow at all? The 2026-11-09 normal-slot
   guest sent 22/35/27/33 and the high-slot guest sent 27 + four 32 (no 30/31). If type 30 never appears,
   P2/P3 are not on the observed path and the removal is P1 data or peer-removal.
2. Does the host's session ever apply the guest's player-remove with peer 17 in the current build (P4 says it
   should not, because FUN_18039CBF0 returns -1)? P0 hook 1's caller RVA answers definitively.
3. The whole-block mirror clearing shadow slots 16-31 for network-decoded blocks (SlotPatch.cpp MirrorCopy) is
   still the design gap for a real 32-player session: the #32 message extension is decoded but the block mirror
   does not fold the extension into the shadow membership. If P1 evidence shows the wipe, that mirror is the
   place to fix, not another engine cave.

## H. Reference addresses used in this note

| RVA | What |
|---|---|
| 0x38F49C | per-frame local-user reconcile (drop decision; 0x38F708 id compare; 0x38F77B drop call) |
| 0x394D48 | drop local user: in-game direct remove 0x394E03 / lobby send msg 32 at 0x394E0A/0x394E4B |
| 0x3971AC | send session message helper (type argument 0x20/0x1F/0x1E - see caller sites) |
| 0x3973BC | player-add acceptance/refusal evaluator (UNCACHED: 0x39740C imul 0x158, 0x397426 count, 0x397440 slot, 0x397503 imul 0xB0, 0x39752A imul 0x56) |
| 0x39FB34 | queued player-add apply -> 0x39D620 (UNCACHED: 0x39FB62 imul 0x158, 0x39FB7D imul 0xB0) |
| 0x39CBF0 | sender id -> peer index resolver (17 bounds at 0x39CC14 / 0x39CC4E) |
| 0x39D620 / 0x39D7F8 / 0x39D8A8 / 0x39D950 | player add-update / remove wrapper / add / remove |
| 0x39DD4C | membership producer (peer loop 17, player loop 16; loop-end caves in SlotMessageTable.inc) |
| 0x39E47C | membership consumer (applies #32 payload; calls 0x39D8A8 at 0x39E763) |
| 0x42AAAE..0x42AB61 | in-band dispatcher type 0x20 player-remove handler -> FUN_18039D7F8 |
| 0x44B080 | player-add receiver (validates via 0x3973BC; refuse send 0x3971AC(...,0x1F)) |
| 0x42459C | connect-refuse encoder (Noble CONNECT REFUSED hook) |
| 0x3F8C60 / 0x3F8D6F | connect-request handler / reason-3 refuse send |
| 0x429BEE / 0x43B98C | reason-10 channel close call / close primitive |
| 0x429AA4 | out-of-band session message dispatcher (type 7 connect-closed -> close 10) |


## I. FOLLOW-UP 2026-11-16 (runtime evidence: peer 17 stops at state 7) - supersedes section 0 item 6

New evidence from the lead's high-slot run on the fixed build: host peer 1 (normal) goes 5 -> 6 -> 8 and the consumer
accepts the delta; host peer 17 (high-slot) goes 5 -> 6 -> 7 and never reaches 8, the membership delta generated at
that moment carries peer state 7 and the guest's consumer returns 0 (reject + membership reset), then teardown
(guest channel close reason 14, host close reason 10) and connect-refused 3 on the reconnect attempts.

### I.1 What [block+0x10] is, and the real source of state 7

* In FUN_180392DB0 the base register is **W = session+0x30** (it is the same base that FUN_18039C668 uses:
  state write = W + peer*0x158 + 0x38). The compare at 0x392F79 is therefore
  CMP ESI,[W+0x10] = CMP ESI,[session+0x40].
* **session+0x40 is the host/leader peer index** (the project's own session-update hook labels it "host peer (+0x40)";
  FUN_180394D48 sends the out-of-band player-remove to it: FUN_1803971AC(session,0,[session+0x40],0x20)).
* 0x392F6E..0x392F84 is the **5 -> 6 (normal peer) / 5 -> 7 (host peer)** promotion:
  `
  0x392F4A  MOV R15,[peer*0x158 + W + 0x168]     ; peer id field (peer+0x138)
  0x392F56  IMUL RAX,R14,0x158                   ; [caved]
  0x392F5D  CMP [RAX+W+0x38],5                   ; peer state == 5
  0x392F64  CMP R15,[RAX+W+0x168]                ; same id
  0x392F73  MOV R8D,7
  0x392F79  CMP ESI,[W+0x10]                     ; peer == host peer ?
  0x392F7C  JZ  0x392F84
  0x392F7E  MOV R8D,6
  0x392F84  CALL FUN_18039C668                   ; set state 6 or 7
  `
  So the 7 written there is the *host's own peer record* path, not the joining peer. A joining peer gets **6** here.
* **The joining peer's 6 -> 7 is the peer-properties message (type 0x1B = 27) handler**
  FUN_18044ADF4 (in-band dispatcher case 0x1b): it resolves the sender peer with FUN_18039CBF0 and does
  if (peer record state == 6) FUN_18039C668(W, peer, 7) at **0x44AE7E** (bytes: CMP [R14+RSI+0x68],6 /
  MOV R8D,7 / CALL setter; the peer imul at 0x44AE64 is caved in SlotPatchTable.inc, and the FUN_18039CBF0
  0x11 bounds are now raised in SlotBoundsTable.inc 0x39CC14/0x39CC4D, so this handler works for peer 17).
* Therefore **state 7 is the normal intermediate state** ("peer has reported its properties, waiting for its
  player data / confirmation"). The normal guest passes through 7 too; the high-slot peer does not leave 7 because
  the 7 -> 8 promotion's readiness test fails (next section).

### I.2 The 7 -> 8 condition and why peer 17 fails it

7 -> 8 promotion lives in FUN_180392DB0, in the "pair up peer records by id" block (0x393119..0x39342B) followed by
the promote loop:

`
0x393185  CMP [RDX+RDI+0x38],R9D        ; candidate id/state compare (RDX = peer*0x158, caved 0x393165)
0x39318A  JZ  ...
0x393191  CMP [RDX+RDI+0x38],0x7        ; only state-7 peers go through the readiness test
0x393198  LEA R8,[RBP+0x1a4]            ; session + 0x1a4 + peer*0x158 = peer->player mapping table
0x3931A2  ADD R8,RDX
0x3931A5  CMP dword [R8],-1             ; each of the 4 mappings:
0x3931AB  MOVSXD RAX,[R8]
0x3931AE  IMUL RCX,RAX,0xB0             ; [caved 0x3931AE] player index -> player record
0x3931B9  CMP [RCX+RBP+0x175C],-1       ; player record +0x1C  == "not ready/not assigned"
0x3931C1  CMOVZ R13D,EBX                ; mapping != -1 AND player+0x1C == -1 -> pair INVALID
...
0x393480  IMUL RAX,RSI,0x158            ; [caved] same-id peers -> set state 8
0x393491  MOV R8D,8
0x39349D  CALL FUN_18039C668
`

The invariant: a state-7 peer is only promoted to 8 when every player mapped to it is "ready", i.e.
**mapping[p][slot] == -1 OR player record's +0x1C != -1**. Peer 17's player 16 fails this: on the host,
player 16's record+0x1C is still -1 because the **player-properties message (type 0x21 = 33) handler was never
applied to player 16**. The culprit is an **uncaved slot-address computation** in that handler:

`
FUN_18044B1DC (msg 33 player-properties):
0x44B248  MOVSXD RAX,[RBP+0x8]          ; slot (0..3) from payload
0x44B24C  IMUL RCX,RSI,0x56             ; RSI = sender peer index  -> 17
0x44B250  ADD RCX,RAX                  ; + slot
0x44B253  MOV EDX,[RBX+RCX*4+0x1A4]     ; mapping[peer][slot]  <-- UNCACHED
0x44B25A  CMP EDX,-1 ; JZ skip
0x44B292  CALL FUN_18039D9C8(W, EDX, ...) ; applies the properties, sets player record +0x1C
`

RVA 0x44B24C..0x44B258 has no entry in SlotPatchTable.inc / SlotMemberCaves.inc / SlotBoundsTable.inc /
SlotTableSites.inc (0x44B253 is only in MemberKnown.inc as a "known but unpatched" site). For peer 17
RCX*4+0x1A4 = 0x1A4 + 17*0x158 = **0x187C**, i.e. it reads inline player-array memory instead of the shadow
mapping: if it reads -1 the handler returns without applying (player 16 stays +0x1C == -1); if it reads a
player index it corrupts that player's record instead (this also explains the earlier "host's own player removed
/ overwritten" symptoms). Either way the 7 -> 8 readiness test fails, peer 17 stays at 7, the membership delta
produced that frame carries state 7, and the guest rejects it.

Why the guest rejects the state-7 delta (FUN_18039E47C): there is **no explicit "state 7 is invalid" check** in
the consumer. The sequence check passes (payload+0x0C == session msgSeq) as the lead observed, the player
removal probe fires for the entry the delta removes, and the reject comes from the consumer's
consistency/checksum test at the end of FUN_18039E47C (after applying every entry it re-computes the rolling
block checksum over param_1 and compares it with payload+0x2548; a mismatch takes the reset branch
LAB_18039EA51, which is the "ret 0 + membership reset" the lead sees). In the high-slot run the guest cannot
reproduce the host's block because the host's state-7 delta is itself the product of the un-ready player 16
(missing +0x1C and the peer's property fields), so the two blocks diverge -> reject. In the normal run the peer's
player record was completed (msg 33 applied to player 1) before the 7 -> 8 promotion, so the delta carries 8 and
the checksum matches -> ret 1. **Consumer-side reject is a consequence; do not patch the consumer.**

### I.3 Ranked patch candidates (all 32-player mode only; normal-slot must stay byte-identical)

**P1 (top candidate, data fix that unblocks the 7 -> 8 promotion): cave the msg-33 mapping read at 0x44B24C.**
* RVA: **0x44B24C**, stock bytes (verified in Ghidra):
  48 6B CE 56 48 03 C8 = imul rcx,rsi,0x56 ; add rcx,rax (7 bytes, jump back target 0x44B253).
* Proposed: the existing SlotMemberCaves "dword-index 0x56 form" cave (same shape as 0x38F6E0/0x38F723/
  0x38F993/0x393165): threshold 17, record 0x56, scaleShift 2, constant (delta>>2) - 17*0x56 (the generator
  gen_member_caves.py already emits this shape; this site needs registers rcx,rsi / rcx,rax):
  `
  48 83 FE 11              cmp rsi,0x11
  0F 83 0C 00 00 00        jae  big
  48 6B CE 56              imul rcx,rsi,0x56
  48 03 C8                 add  rcx,rax
  E9 <rel32 -> 0x44B253>   jmp back
  big:
  48 6B CE 56              imul rcx,rsi,0x56
  48 03 C8                 add  rcx,rax
  48 81 C1 <imm32>         add  rcx, (delta>>2) - 17*0x56
  E9 <rel32 -> 0x44B253>   jmp back
  `
  (40-byte cave; constantAt = 32, backAt1 = 18, backAt2 = 36, original length 7. Because the address is
  [rbx + rcx*4 + 0x1a4], adding (delta>>2 - 17*0x56) to rcx moves peer-17 slot reads to shadow+0x1a4.)
* Invariant: only peer index >= 17 takes the delta; slots/peers < 17 and non-session callers keep stock bytes.
  This makes FUN_18044B1DC apply the guest's player-properties to shadow player 16, setting record+0x1C and the
  other property fields, which is exactly the input of the 7 -> 8 readiness test. Expected result: peer 17
  6 -> 7 -> 8, delta carries 8, consumer accepts, the close-10/refuse-3 chain disappears.
* Risk: low/medium - pure address redirection with the same pattern already proven for 6 sibling sites; the
  load-time byte check will refuse 32-player mode if the original bytes are wrong.

**P0 (before/with P1): log-only confirmation of the failing readiness test.**
* Hook 0x3931A5..0x3931C1 (or add a dev log in FUN_180392DB0) to print, when a state-7 peer is tested:
  peer index, mapping[peer][0..3], each mapped player's record+0x1C. Expect: peer 17, mapping[17][0]=16,
  player16+0x1C == -1 in the failing run and != -1 in the normal run.
* Hook FUN_18044B1DC entry/0x44B253: log sender peer (RSI), slot (RAX), the raw mapping value read and (after
  P1) the shadow mapping value. Expect raw = garbage/-1 before P1, 16 after P1.

**P2 (fallback if the 7 -> 8 readiness still fails after P1): audit the consumer's full peer-record apply for
peer >= 17.** FUN_18039CF80 writes the whole peer record (state at 0x39CFxx, id, and the per-peer 0x0C-stride
table at [W + peer*0xC + 0x4440]); the peer-state/id writes are covered by the 0x158 imul cave at 0x39CFB5,
but the 0x0C-stride table write is not in SlotTableSites.inc - verify with a trace that a peer-17 entry applied
on the guest lands in the shadow table before chasing anything else.

**P3 (not recommended): force shadow peers to 8.** e.g. patch the 0x393191 state-7 test to skip the readiness
loop for peer >= 17, or add peer >= 17 to the blanket set-8 loop of FUN_180403BCC (0x403D46). This hides the
missing player data; the guest's checksum will still diverge and the consumer will keep rejecting. Do not ship.

### I.4 Updated bottom line

* State 7 is legitimate ("peer-properties received, player data not yet confirmed"); the failure is that peer 17
  never satisfies the 7 -> 8 readiness test because **the host's shadow player 16 record is never completed**.
* Root cause: the player-properties (msg 33) handler FUN_18044B1DC still computes the peer->player mapping with
  the stock peer*0x56*4 + 0x1A4 math (RVA 0x44B24C); for peer 17 it reads inline memory, so player 16's
  record+0x1C stays -1 and the host withholds state 8.
* The guest's consumer reject, the guest's channel close (reason 14), the host's close reason 10 and the
  connect-refused reason 3 are all downstream of that single missing cave.
* This supersedes section 0 item 6 and the earlier P4 ordering: the func_18039CBF0 bounds are already raised in
  the current tree (SlotBoundsTable 0x39CC14/0x39CC4D), so the correct next patch is P1 above.

## J. FOLLOW-UP 2026-11-16 #2: the high-slot membership delta mismatch - host "peer 0 / slot 4" decodes as
## guest "peer 1 / slot 0"

Author: ghidra-delta (static Ghidra analysis of haloreach-copy.dll, sha256 738dd2d2...8b0894; no game instance).
Evidence: host-ph5-20261116-112742.log, client2-ph5-20261116-112742.log (32-player + NOBLE_SLOT_HIGH=1, HEAD
c33ef10 with the 0x44B24C msg-33 cave and the 5b9d39f NOBLE_SLOT_PROBE=1 dumps). Repo HRVAs; Ghidra address =
0x180000000 + RVA. This section answers: (1) which player-record fields the membership producer copies into a
player entry, (2) why the host's "peer 0 slot 4" becomes the guest's "peer 1 slot 0", (3) which uncached peer
math is still missing on this path, and (4) what to patch / test next.

### J.1 What the two probe dumps show (same run, 11:28:07)

* HOST producer #1 (caller hr+0x396A2D, the full call with param_3 = 0):
  hdr 00000016 FFFFFFFF 00000000 cks B996924E, 2 peers/2 players,
  plr[0] idx 0 peer 0 slot 0; plr[1] idx 16 peer 17 slot 0.
* HOST producer #2 (caller hr+0x396A5C, the delta call): hdr 0000001A 00000016 B996924E cks C061DE4A,
  1 peer (idx 17 state 8) + 2 players,
  plr[0] idx 0 peer 0 **slot 4**; plr[1] idx 16 peer 17 slot 0.
* GUEST received delta: identical header/tail, but plr[0] idx 0 **peer 1 slot 0**; everything else identical.
* The only live-data difference is player 0's player-record slot dword (see J.2): the host "player sync in" probe
  (LobbyPatches.cpp Hook_PlayerSync) reads session+0x1750 = 0 at 11:28:07.592 and 4 at .763.
* Same delta (plr[0] peer 0 slot 4) already appears in the pre-c33ef10 high-slot runs host-highfix2-111933 /
  host-ph3-112116 (11:19/11:21, delta callers identical), so this is not caused by the msg-33 cave: it is a
  high-slot-path bug in the block built while the joining peer 17/player 16 is being established. In both PH3 and
  PH5 the write lands in the same place: producer #1 (slot 0) -> host receives msg 27 (peer-properties) + msg 33
  (player-properties) -> player-0 record+0x10 becomes 4 -> producer #2 (slot 4).

### J.2 Producer FUN_18039DD4C: player-entry source fields (Q1)

The 0x2550-byte membership payload is produced by FUN_18039DD4C(caller args: param_1 = session S, param_2 =
S+0x38 = the current/live membership block, param_3 = 0 for a full message or S+0x2248 = the previous sent block
for a delta, param_4 = payload buffer). It is called twice from FUN_180396828 at 0x396A2D (full) and 0x396A5C
(delta); both call sites are already routed through Noble's ProducerWithExtension stub (SlotMessage.cpp).

The "block" is live session memory viewed from S+0x38, so:
* peer records  = block+0x28+i*0x158 = session+0x60+i*0x158  (id at +0, state at +8, player mapping at +0x144)
* player records= block+0x1708+i*0xB0 = session+0x1740+i*0xB0
  (xuid +0x04, owning peer +0x0C (int16), slot +0x10 (dword), flags +0x14/+0x18, ready +0x1C,
  engine player id +0x40, property blob +0x20/+0x78/+0xA0)

Player wire entry E = payload+0x1998+count*0xB8; producer fill sites (verified disassembly, player loop):
| entry field | payload offset | source | producer site (stock bytes) |
|---|---|---|---|
| player index | E+0x00 | loop index | 0x39E077 `44 89 AC 2F 98 19 00 00` |
| present | E+0x04 | - | 0x39E07F |
| xuid (8B) | E+0x09 | player record+0x04 | 0x39E0D4 `48 8B 43 F8` + 0x39E0D8 |
| **owning peer** | **E+0x12 (int16)** | **player record+0x0C (int16)** | 0x39E0E0 `0F B7 03` + 0x39E0E3 |
| **slot** | **E+0x14 (byte)** | **player record+0x10 (dword low byte)** | 0x39E0EB `8A 43 04` + 0x39E0EE |
| record+0x14 | E+0x18 | player record+0x14 | 0x39E0F5 `8B 43 08` |
| record+0x18 | E+0x1C | player record+0x18 | 0x39E0FF `8A 43 0C` |
| record+0x00 / +0x1C / +0x20..0xAF | E+0x20 / E+0x24 / E+0x28 | record+0.. | 0x39E169 block (copies 0x90 bytes from record+0x20) |

Authoritative writer/reader of player record+0x10:
* writer: FUN_18039D8A8 (player add) **0x39D908** `45 89 4B 10` = MOV [R11+0x10],R9D, R9D = param_5 (slot);
  the same function writes the peer's mapping table entry [peerRec+0x144+slot*4] = player index at 0x39D921.
* readers: FUN_18039D950 (player remove) 0x39D96D `45 39 82 20 17 00 00` (CMP [R10+0x1720],r8d) and
  0x39D984 (MOVSXD RAX,[R10+0x1720]) -- i.e. slot is a dword 0..3 in the engine; the wire format stores it in
  2 bits (J.3), so a dword 4 is invalid data, not a valid encoding.
* No other instruction that writes player-record+0x10 was found in the add/remove/properties/consumer/session-update
  paths; the corrupt value therefore comes either from a FUN_18039D8A8 caller that derived a bad slot, or from an
  uncached address computation that aliases the record. Both are unresolved statically (see J.5).

### J.3 Why the guest decodes "peer 1 slot 0" (Q2): unmasked 2-bit slot write leaks into the peer field

Message 0x1A codec is registered by FUN_180427E8C at table entry +0x680: encode = FUN_18042675C, decode =
FUN_180427140. Wire player entry = player index (4 bits, Noble patches to 5), present 1 bit, changed 1 bit,
xuid 64 bits, **owning peer (5 bits, Noble patches to 6)**, **slot (2 bits)**, then 0x14-bit field, 1-bit fields,
property blob. So the wire field is NOT "slot 2 bits and nothing else": it is "peer 6 bits then slot 2 bits".

Encoder sites (FUN_18042675C, RBP = E+8; stock bytes, Noble edits from SlotMessageTable.inc apply to these):
* owning peer: 0x426C04 `0F BF 55 0A` (MOVSX EDX,word [RBP+0xA] = E+0x12), width r12 =
  0x4268BB `41 BC 05 00 00 00` (patched 6); fast path 0x426C1A `48 C1 E0 05` (SHL RAX,5; patched to
  SHL RAX,6) + 0x426C1E `48 0B C2` (OR RAX,RDX); fallback 0x426C30 CALL FUN_1800dcfc4.
* slot: 0x426C38 `0F B6 55 0C` (MOVZX EDX,byte [RBP+0xC] = E+0x14); fast path 0x426C50
  `48 C1 E0 02` (SHL RAX,2) + **0x426C54 `48 0B C2` (OR RAX,RDX)**; fallback 0x426C69 CALL
  FUN_1800dcfc4(...,2).
* **Both paths fail to mask the value to the field width.** The fast path is `acc = (acc << 2) | slot`; with
  slot = 4 the OR sets bit 2 of acc, which is the last already-written bit = the LSB of the 6-bit peer field.
  Peer 0 (000000) becomes 000001 = 1, slot decodes 0. This is exactly host "peer 0 slot 4" -> guest "peer 1
  slot 0". (The slow path FUN_1800dcfc4 likewise writes the value's out-of-range bits into the stream.)

Decoder sites (FUN_180427140, R14 = E+8; stock bytes):
* player index 0x42730F `BA 04 00 00 00` (patched to 5); present 0x427325; changed 0x42734F CALL
  FUN_180054214; xuid 0x427368 CALL FUN_1800dcf28(R14+1,0x40) -> E+9.
* owning peer 0x42736D `41 8B D5` (MOV EDX,R13D; R13D = 5, patched to 6 at 0x4271B9
  `41 BD 05 00 00 00`) + 0x427373 CALL FUN_1800dd10c; store word [R14+0xA] -> E+0x12 at 0x42737D.
* slot 0x427378 `BA 02 00 00 00` (MOV EDX,2) + 0x427385 CALL FUN_1800dd10c; store byte [R14+0xC] ->
  E+0x14 at 0x42738F.
* validity: 0x4273B0 `B8 10 00 00 00` (peer <= 0x10, patched to 0x1F) and 0x4273BC
  `41 80 7E 0C 04` (slot < 4). The decoder is symmetric with the encoder; it does not "pack differently",
  it simply reads the bit that the unmasked slot write corrupted.

Consequence: the guest's decoded block (plr[0] peer 1 / slot 0) cannot match the host's block (peer 0 / slot 4),
so FUN_18039E47C's final consistency/checksum check (checksum over the whole 0x2210-byte block incl. record+0x10,
payload+0x2548) fails -> ret 0 -> membership reset -> guest msg 7 -> host CLOSE reason 10. An encoder mask patch
alone cannot cure this: the payload checksum is computed over the host's live block, which still holds slot 4, so
the guest can never reproduce it while the corrupt dword remains.

### J.4 Uncached peer/player math still on this exact path (same class as 0x44B24C)

Checked by scanning every `imul reg,reg,0x158 / 0xB0 / 0x56` in the session region of the stock DLL against
every RVA present in src/host/SlotPatchTable.inc / SlotMemberCaves.inc / SlotMessageTable.inc /
SlotTableSites.inc / SlotBoundsTable.inc. The following sites that run in the high-slot join/props path are
still UNCACHED (peer >= 17 / player >= 16 lands on stock inline memory):

* **FUN_18039D1F8** (msg 27 peer-properties apply, called from FUN_18044ADF4 at 0x44AED6):
  **0x39D23E `49 69 D7 58 01 00 00` = IMUL RDX,R15,0x158**. _Buf1 = W+0x48+peer*0x158 (W = S+0x30);
  for peer 17 this is session+0x1768 = **player 0 record+0x28**, and the 0x120-byte property-block write/copy
  spans player 0 record+0x28..+0xAF and player 1 record+0x00..+0x97. Real, provable missed cave (write, not just
  read): the same "same class as 0x44B24C" bug, on the message that arrives immediately before the bad delta.
  Correct cave form = existing peer form: cmp r15,0x11; jae big; imul rdx,r15,0x158; jmp back; big: imul
  rdx,r15,0x158; add rdx,(delta-17*0x158); jmp back (back target 0x39D245).
* **FUN_18039F02C** (called by FUN_18039D1F8 after the props compare): **0x39F04E `49 69 D? 58 01 00 00`
  and 0x39F0C0** (0x158 forms) -- mask writes at W+0x15C+peer*0x158 and W+0x15C+(peer*0x56+lVar6)*4, i.e.
  session+0x187C / session+0x1864 for peer 17 (player 1 record tail). Same cave shapes.
* FUN_180396828 (the delta producer itself): 0x396AB4 peer-state read peer*0x158+0x68 -> for peer 17 reads
  player 0 record+0x18 (read-only; it gates the producer's delta-vs-full decision).
* FUN_180395F4C (props-failure path): 0x395E2F peer-state read peer*0x158+0x68 -> same alias, read-only.
* FUN_18039C8DC (peer removal): 0x39CB0C mapping read peer*0x158+0x174 -> for peer 17 reads session+0x1894.
* Already listed in section I/P2-P3 and still uncached: FUN_18039FB34 0x39FB62/0x39FB7D; FUN_1803973BC
  0x39740C/0x397503/0x39752A.

None of the above writes player-record+0x10, so they do not by themselves explain the "4"; they do explain
player-record corruption and must be fixed for a real 32-player session. The slot-4 writer is still open (J.5).

### J.5 Who writes player 0 record+0x10 = 4? (narrowed, not yet pinned)

Facts: (a) only FUN_18039D8A8 writes player-record+0x10, from its slot argument, and its in-engine callers
(FUN_18039D620 / membership consumer) pass a 0..3 slot; (b) an 8-byte write of 0x0000000000000004 at peer
record base is the only alias that produces record+0x10=4 / +0x14=0 (uncached peer-17 record base =
session+0x60+17*0x158 = session+0x1750 = player 0 record+0x10), but the only peer-record-id writer in the msg-27
handler (0x44AE92) IS caved (SlotPatchTable.inc entry 0x44AE64); (c) session update FUN_180392DB0 and the msg
27/33 handlers contain no other +0x10 store. Remaining explanations: a FUN_18039D8A8/D620 call whose (peer,slot)
came from an uncached computation, or a record writer not yet located. One run with instrumentation settles it:
* hardware watchpoint (the project already has the arm/log plumbing for Dr0/Dr1 in LobbyPatches.cpp
  ArmPlayer0Watch): set **Dr1 = session+0x1750, LEN=4, RW** (instead of +0x1744) for the high-slot run; the
  exception's RIP/RVA names the writing instruction. This is exactly how the earlier "peer 17 owns player 0"
  bug was found (SlotPatch.cpp:308-312).
* MinHook on FUN_18039D8A8 entry (0x39D8A8) logging caller RVA + (player index, peer, slot): if the corrupt
  write comes through the player-add path the call shows (index 0, peer 0, slot 4) directly.
* MinHook on FUN_18039D620 entry and on FUN_18039FB34 entry (queue record at W+0x4508+index*0x80, fields +8
  peer / +0xC slot) for the same run.

### J.6 Patch candidates (ranked)

**P1 (top, data fix, no engine bytes: sanitize the membership block inside the existing producer stub).**
Noble's ProducerWithExtension (src/host/SlotMessage.cpp, installed at producer call sites 0x396A28/0x396A57)
already runs before the engine producer. The producer's second argument (b) is the live membership block S+0x38;
sanitize it there before calling gProducer:
```
for i in 0..15, if block[0x1704 + maskbit]:            // live player mask in block terms +0x704
  rec = block + 0x1708 + i*0xB0;
  slot = *(uint32_t*)(rec + 0x10); peer = *(int16_t*)(rec + 0x0C);
  if (slot > 3) {                                       // invalid dword in a 2-bit wire field
    int fixed = 0;                                      // derive from the authoritative mapping table
    peerRec = PeerRecord(session, peer);                // SlotPatch helper (handles shadow peers >= 17)
    for j in 0..3: if (*(int32_t*)(peerRec + 0x144 + j*4) == i) fixed = j;
    *(uint32_t*)(rec + 0x10) = fixed;
  }
```
Do it for both producer calls (full and delta). Effect: the payload producer copies the repaired slot, the wire
encoder emits a legal 2-bit slot, and the checksum (computed over the same repaired block) matches the guest's
block after it applies the entry -> the delta is accepted. No engine code bytes are changed, normal 16-player
mode is untouched (stub only installed in the 32-player group), and it works without knowing the writer in J.5.
Note: this neutralizes the membership symptom but the underlying stray write still corrupts the record between
sends (msg-32 remove/readiness logic can still misread it), so J.5/J.7 stay on the list.

**P2 (diagnostic, do with P1 in the same run):** watchpoint + D8A8/D620/FB34 hooks of J.5; then cave the guilty
writer (same data-fix pattern as 0x44B24C). Also add the writer's RVA to SlotPatch's load-time byte checks.

**P3 (real missed cave, needed for a correct session regardless):** cave FUN_18039D1F8 0x39D23E with the existing
peer form (41-byte cave; threshold 17, record 0x158, constant delta-17*0x158; back target 0x39D245), and
FUN_18039F02C 0x39F04E/0x39F0C0 with the 0x158/0x56 forms. These stop peer-17 peer-properties from writing into
player 0/1 records. Risk: low (same proven shape; load-time byte check rejects a wrong build).

**P4 (defense in depth only, do NOT ship alone): mask the wire slot field in the encoder.** Patch
0x426C38..0x426C3F (8 stock bytes `0F B6 55 0C 8B C7 2B C1` = MOVZX EDX,[RBP+0xC]; MOV EAX,EDI;
SUB EAX,ECX) to a 5-byte jmp + 3 nops and a cave:
movzx edx,byte [rbp+0xc]; and edx,3; mov eax,edi; sub eax,ecx; jmp 0x426C40. This makes a slot>3 encode as 0
instead of corrupting the peer field (guest would decode peer 0 slot 0 instead of peer 1 slot 0), but the
payload checksum still reflects the host's invalid block, so the consumer still rejects while slot 4 remains.
Useful only as an assertion/canary; also mask the fallback FUN_1800dcfc4 call at 0x426C69.

### J.7 What to test next

1. One high-slot run with P1 (ProducerWithExtension sanitizer) only: expect the guest's membership consumer to
   return 1 for the delta (probe: ret 1, no reset), no msg-7/CLOSE-10, peer 17 -> state 8 delta accepted;
   host/guest blocks consistent. Log the sanitizer line (player index, old slot, fixed slot, peer) so the
   corrupt writer is also visible per run.
2. Same run with the J.5 watchpoint/D8A8 hook to name the writer; then P3 (and the guilty-writer cave) for the
   real fix.
3. Regression: normal-slot 2-instance run must stay byte-identical (stub inactive in 16-player mode); high-slot
   run with the msg-33 cave must still promote peer 17 7 -> 8.

### J.8 Bottom line

* Producer player entry: owning peer = player record+0x0C (int16), slot = player record+0x10 (dword); the wire
  stores them as 6-bit peer + 2-bit slot (Noble-patched), so a dword slot > 3 is unrepresentable.
* The observed round trip is explained: host slot 4 written with the encoder's unmasked `acc = acc<<2 | slot`
  fast path (0x426C50/0x426C54) sets the peer field's LSB -> guest reads peer 1, slot 0. The host's engine data
  is invalid (slot must be 0..3), not a decoder packing difference.
* The invalid dword is written while the high-slot join messages (msg 27/33) are processed; the writer is still
  open (only FUN_18039D8A8 writes record+0x10, so watchpoint it). Missed uncached peer math on this path
  (FUN_18039D1F8 0x39D23E, FUN_18039F02C 0x39F04E/0x39F0C0) is real and must be caved.
* Immediate cure that does not wait for the writer: sanitize the live block in ProducerWithExtension (P1) so the
  wire payload + checksum are consistent with the 2-bit wire field; then fix the writer and the missed caves.


## K. FOLLOW-UP 2026-11-16 #3: SEAT PATH FIXED - peer 17 seats and plays (commit 0fdcf97)

Runtime evidence: `build\bin\Release\host-seat1-20261116-135825.log` + `Release2\client2-seat1-*.log`
(2 instances, `-slot-patch -slot-players 32`, `NOBLE_SLOT_HIGH=1`, `NOBLE_SLOT_PROBE=1`).

### K.1 Root cause of the per-frame boot_peer(17, reason 9)

The stack was `hr+0x3950BC <- hr+0x44FFC2 <- hr+0x40A1BB <- hr+0x40739D <- hr+0x3AE716`. The boot loop lives in
**FUN_18044FC94** (per-frame duplicate-user/seat check): it scans the session peer mask (DWORD at session+0x5C), and
for each peer reads `peer*0x158 + session + 0x17C` (dword, bit 23 test) and `peer*0x158 + session + 0x118` (qword vs
the local user/join id at wrapper+0x68), then boots every matching peer except the local one (session+0x4464).
Its loop bounds had **already** been raised to 32 by gen_bounds (SlotBounds 0x44FF31 / 0x44FF8B / 0x44FFC4), but the
peer stride at **0x44FF46** (`imul rdx,r8,0x158`) was still stock, so peers 17..31 read player-record memory
(session+0x17xx) instead of the shadow peer record. On this data the check matched and tried to boot peer 17 every
frame; Noble's boot_peer hook suppressed the actual send, but the seat never completed (guest torn down ~19 s later).

### K.2 Fix (commit 0fdcf97, 6 new member caves, generator SITES updated)

- **0x44FF46** imul rdx,r8,0x158 - FUN_18044FC94 duplicate-user/seat check (the boot loop).
- **0x397424** imul rcx,rsi,0x158 - FUN_1803973BC player-add acceptance: owning peer record (+0x140 count, +0x144 slots).
- **0x397503** imul rcx,r13,0xB0  - player-add acceptance: existing player record.
- **0x39752A** imul rcx,rsi,0x56 / movsxd rax,r12d - player-add acceptance: peer dword player table.
- **0x39FB62** imul r9,rax,0x158 - FUN_18039FB34 queued player-add apply: peer record.
- **0x39FB7D** imul rcx,rax,0xB0  - queued player-add apply: player record.

SlotPatch now installs **53 member caves / 120 slot address sites**. All expected bytes byte-checked against the stock
dll by tools/limits/gen_member_caves.py.

### K.3 Evidence (2-instance high-slot run)

- Host 13:58:29 `120 slot address sites redirected`; host peer states `p0:8 p17:8`, peermask 00020001, pmask 00010001.
- **0 BOOT lines** in the whole run (was one per frame before).
- Host membership producer #2: peer 17 state 8 + `plr[0] idx 16`; guest consumer ret 1 on every delta (msgSeq 0x16..0x1D).
- Host `game session started` at 13:58:43; **guest `game session started` at 13:59:02** (frame 307), no CLOSE channel,
  0 crashes; both instances alive at the end.

### K.4 Multi-high-slot probe (4 instances, seat3 run) - partial

Two high peers seat at once: host `peermask 00060001 pmask 00030001 peer states p0:8 p17:8 p18:8`. The third guest got
a normal slot (local peer 1) and the host closed channels reason 10 (guest side reason 4 reconnect loop). Open question:
whether that is (a) the same-PC shared identity being detected as duplicate user once two high peers exist, (b) the
high-slot test caves only handling the first two joiners, or (c) a real seat-allocation limit. Test with distinct
profiles/accounts before treating it as an engine bug.

### K.5 Next (64/128 groundwork, from code reading this session)

- Shadow layout is the blocker past 32: `ShadowSlots(k,a) = gShadow + k*kSessionStride + a.offset` and every array shares
  one delta; at 32 the shadow storage holds 15 peer / 16 player / 15 table records from the *same* offset as the stock
  array (offset+record layout). For 128, peers 17..127 (111*0x158 = 0x95B8) would overlap the player array at 0x1740, so
  the shadow block needs a **new non-overlapping layout** and **per-array shadow base offsets** (extend Site with a
  shadowOffset/array id rather than one global delta).
- `kSlots32 = 32` is used by shadow sizes, step-loop bounds and mask patching; make it a capacity constant (32/64/128).
- Membership message: kMsgSize 0xA300 for 32 (stock 0x2550 + extension, SlotMessageTable.inc); 64/128 needs larger
  payloads and possibly message-size/fragmentation work; wire peer index is 6 bits (max 64) and player index 5 bits, so
  64 is the natural next ceiling and 128 needs a wider field on every encoder/decoder site.
- Engine pools: 32 raised players/fire team/sim view arrays; 64/128 needs the same raises at 64/128 plus pool cost
  (see docs/limits-players-*.md, docs/limits-ai.md).

## L. FOLLOW-UP 2026-11-16 #4: seat fixed (0fdcf97) and 64/128 capacity groundwork (1fbd952)

* **Seat fixed**: the per-frame boot_peer(17, reason 9) came from FUN_18044FC94 (duplicate-user/seat check). Bounds
  were already 32 but the peer stride at 0x44FF46 was stock, so peers >= 17 read player-record memory and matched.
  Six member caves added (0x44FF46, 0x397424, 0x397503, 0x39752A, 0x39FB62, 0x39FB7D); 53 member caves / 120 sites.
  Proof: host peer states p0:8 p17:8, 0 BOOT lines, guest consumer ret 1, host started 13:58:43 / guest 13:59:02, no CLOSE.
  Multi-high probe: p0:8 p17:8 p18:8 seat together; the third guest fell back to a normal slot and channels closed
  (suspect same-PC shared identity -> duplicate-user detection; retest with distinct accounts).
* **64/128 capacity model** (src/host/SlotPatch.cpp): capacity 32 keeps the proven one-shadow-block layout (biases 0,
  runtime byte-identical, extension 0x2200 asserted); 64/128 use two shadow blocks per static session (A and B) with
  back-to-back arrays and explicit biasA/biasB values, so the later per-site A/B cave classification can use them.
  static_assert: all layouts fit kSessionStride 0x61668 with no overlap.
* **Headless self-test** noble-dev.exe -test-slot-capacity (exit 0 observed 14:18): 32 ext 0x2200/msg 0xA300;
  64 ext 0x6900/msg 0xEA00/peer 6 bits ok/player needs 6; 128 ext 0xF700/msg 0x17800 needs framing/peer+player 7 bits.
* Tracked design doc: docs/02-engine-reference/TOPIC-scaling-64-128.md. Compendium 4.2/4.3/4.7/status matrix updated.
* Next for 64/128: (1) per-site A/B classification of all member/table/step caves; (2) dev-gated capacity install with
  install-level declared-N validation; (3) wire widening (index bits, multi-word masks, game-player event); (4) 128
  message framing; (5) pools/session-slot-count; (6) UI/harness generalization. Validate with the self-test + forced
  high slots until real 64/128 clients exist.

## M. FOLLOW-UP 2026-11-16 #5: chunked shadow layout chosen for 64/128 (round 2)

* Static check settled the layout question: the generic helpers (e.g. FUN_18039D620 player add/update) take a BLOCK
  pointer and their callers derive block-relative bases, so a caved site can run for block A and block B at different
  times. One constant cannot point two different shadow blocks -> per-site A/B classification is ambiguous. Therefore
  the chunked model is the plan: split every shadow array into chunks that fit below the 0x2210 A->B distance and
  place chunk A exactly 0x2210 before chunk B; each cave gets one constant per index range and works for either base.
* Chunk capacities: 25 peer records, 49 player records, 726 table entries (0x2210 / 0x158, 0xB0, 0xC).
* Chunk plan (self-test, exit 0): cap 64 = peers 2 chunks / players 1 / table 1, shadow 0x10F38, piecewise member
  cave ~77 B; cap 128 = peers 5 / players 3 / table 1, shadow 0x261C8, cave ~158 B. All fit kSessionStride 0x61668.
* Cost: member-cave entry must grow from 48 to >=160 bytes (53 sites -> ~8.5 KB cave code, fine); the 9 step caves and
  the whole-array copy/clear mirrors must hop chunk boundaries (contiguous memmove no longer valid across chunks).
* 32 keeps the proven single-block stock-relative layout (biases 0); chunked model applies only >32.
* Docs updated: docs/02-engine-reference/TOPIC-scaling-64-128.md sections 2.1/5/6 + compendium 4.3 note.

## N. Round 2 close - chunked model committed, upstream v0.20.10 integrated (2026-11-16 ~14:30)

* Pushed 0ebd1a2 (chunked model + self-test + docs) and 3044d65 (compendium v0.20.10). Remote main = wip-merge =
  3044d65 at close; tree clean; no instances left running.
* Upstream main moved again during the round: ed72784 = v0.20.10 ('stabilize post-match KBM UI focus and click
  handling', touches PauseMenu/FrontEndMenu/FrontEndPickers/NobleUi/FrontEndLobby). Merged clean as 8d16160, gates
  rebuilt clean, 2-player LAN smoke passed (peermask/pmask 0x3, 0 crashes), self-test still exit 0 (v0.20.10 binary).
* Chunked-model decision recorded in docs/02-engine-reference/TOPIC-scaling-64-128.md section 2.1; measured chunk plan:
  64 = 2 peer/1 player chunk, shadow 0x10F38, cave ~77 B; 128 = 5 peer/3 player chunks, shadow 0x261C8, cave ~158 B.
* Next round target: grow Site.cave to >=160 B, generator piecewise index-range emission, chunk-hopping step caves and
  copy/mirror loops, then a dev-gated capacity install with install-level validation (2-client declared-N session).

## O. Round 3 close - chunked dispatch math validated (50f2694)

* ValidateChunkPlan added to the -test-slot-capacity self-test: builds the chunk plan (limits 25 peer / 49 player /
  726 table records; chunk A at cursor, chunk B at cursor+0x2210), computes each range constant
  C = chunkOffsetA - stockOffset - first*record, then simulates EVERY record index on both bases (block A and
  block A + 0x2210 = block B): computed address must equal chunkOffset + (index-first)*record.
* Asserts: chunk pairs end inside kSessionStride (shadow end 0x8614 / 0xEAA4 / 0x21A14 for 32/64/128), chunks of one
  kind cannot overlap in either block, and the model's memberCaveBytes estimate equals the dispatch estimate.
* Per-kind piecewise cave sizes: 32 = 24 B (stock single-range cave unchanged at 41 B), 64 = peer 53 / player 24 /
  table 24, 128 = peer 140 / player 82 / table 24 -> Site.cave must grow 48 -> >=160 B for the 128 install.
* Self-test exit 0 (0 failed checks). Both gates clean. Remote main = wip-merge = 50f2694 at close.
* Next round: implement the chunked install - grow the Site cave array, extend the generator to emit piecewise
  index-range bodies, and teach the 9 step caves + copy/clear mirrors to hop chunk boundaries; then a dev-gated
  capacity install (NOBLE_MP_SLOTS=64/128) validated at install level with 2 clients on a declared-N session.
