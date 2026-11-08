# C12 bounds review: the 60 loop entries in limits-players-bounds.json

Scope: entries with kind=loop in docs/limits-players-bounds.json (174 entries total: 60 loop,
111 bit-scan, 3 other). Each loop body was disassembled from its backward branch to the bounds
check and classified by how it reaches slot records.

Classes: C1 = slot pointers built by the patched 0x158/0xB0 imul (covered, widen the compare);
C2 = walks a slot pointer by 0x158/0xB0 - these need a step cave and are added to Claude's loop
list (same detour as the six functions: stepping past record 16/17 runs into the scalar fields at
+0x1718 peers / +0x2240 players); C3 = indexes a table that is not the session slot array;
C4 = mask scan with the per-slot work in a callee.

C3 location column: where the table lives, entry size, entry count and who allocates it (from the
instructions that form the base and the object constructors).

| # | function | bounds check | class | evidence | C3 table location / entry size / count / allocator |
|---:|---|---|---|---|---|
| 1 | 0xd6b8 | cmp    r9d,0x10 jl | **C1** | imul 0x158 at 0xD8AA (peer record) plus a 0xB0 step at 0xD8C9 | - |
| 2 | 0x14f430 | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x14F49D, mask 0x173C | - |
| 3 | 0x16fe2c | cmp    eax,0x10 jge | **C1** | imul 0xB0/0x158 at 0x16FEFD/0x16FF10/0x16FF17 | - |
| 4 | 0x2a5b84 | cmp    edx,0x10 jge | **C1** | imul 0xB0 at 0x2A5C66 | - |
| 5 | 0x38da74 | cmp    rcx,0x10 jge | **C3** | pointer array [rsi+idx*8] | stack frame of 0x390DB0 (rbp = rsp-0x228); caller 0x391203 passes r9 = rbp+0x150; 8 B; 16; stack (no allocator) |
| 6 | 0x390db0 | cmp    edi,0x10 jge | **C1** | imul 0xB0 at 0x390EFC/0x390F13/0x390F44 | - |
| 7 | 0x390db0 | cmp    eax,0x10 jge | **C3** | per-player pointer array [rbp+idx*8+0x150] | stack frame of 0x390DB0 at rbp+0x150; 8 B; 16; stack (no allocator) |
| 8 | 0x390db0 | cmp    rcx,0x10 jbe | **C3** | 16-word buffer copy (cmp rcx,0x10; add rdi,2) | session membership object (static array 0x2B07418), player record field at +0x17B8 (+0x78 in the 0xB0 record); 2 B; 16 words (name length); session constructor 0x3CF864 |
| 9 | 0x390db0 | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x390FD0/0x390FF5 | - |
| 10 | 0x390db0 | cmp    rcx,0x10 jbe | **C3** | 16-word buffer copy (add rdx,2; cmp rcx,0x10) | same session-object field, second copy path; 2 B; 16 words (name length); 0x3CF864 |
| 11 | 0x390db0 | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x3912BC/0x391342/0x3913B2 | - |
| 12 | 0x3915a8 | cmp    ecx,0x10 jge | **C1** | imul 0xB0 at 0x39161B | - |
| 13 | 0x3915a8 | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x3916C4 | - |
| 14 | 0x3915a8 | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x39186E | - |
| 15 | 0x3915a8 | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x391930 | - |
| 16 | 0x3915a8 | cmp    ecx,0x11 jge | **C1** | imul 0x158 at 0x391A41 | - |
| 17 | 0x391f0c | cmp    ebx,0x11 jge | **C1** | imul 0x158 at 0x3920F4 | - |
| 18 | 0x391f0c | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x39215D | - |
| 19 | 0x391f0c | cmp    eax,0x11 jge | **C1** | imul 0x158 at 0x392631 | - |
| 20 | 0x392c90 | cmp    eax,0x11 jge | **C1** | imul 0x158/0xB0 at 0x393165/0x3931AE | - |
| 21 | 0x392c90 | cmp    eax,0x11 jge | **C1** | imul 0x158/0xB0 at 0x393361/0x39338D | - |
| 22 | 0x392c90 | cmp    eax,0x11 jge | **C1** | imul 0x158 at 0x393480 | - |
| 23 | 0x392c90 | cmp    eax,0x11 jge | **C1** | imul 0x158 at 0x39356F | - |
| 24 | 0x3951ac | cmp    edi,0x11 jge | **C4** | mask scan (bt [rbx+0x5C]) + call 0x39C8DC / 0x39C668 per slot | - |
| 25 | 0x396828 | cmp    r10d,0x11 jge | **C1** | imul 0x158 at 0x396946 | - |
| 26 | 0x396828 | cmp    ebx,0x11 jge | **C4** | mask scan + calls 0x3F950C/0x3F97DC/0x3971AC | - |
| 27 | 0x3971ac | cmp    eax,0x11 jge | **C4** | mask scan + recursive call 0x3971AC | - |
| 28 | 0x39c6b4 | cmp    ebx,0x10 jl | **C4** | mask scan (bt [rsi+0x170C]) + call 0x39D950 | - |
| 29 | 0x39c6b4 | cmp    ebx,0x11 jl | **C4** | mask scan (bt [rsi+0x2C]) + call 0x39D0AC | - |
| 30 | 0x39c6b4 | cmp    edx,0x11 jb | **C1** | imul 0x158 at 0x39C742 | - |
| 31 | 0x39c6b4 | cmp    ebp,0x11 jb | **C3** | 0xC-stride array of 0x11 entries | session object +0x4440 (arg rcx); 0xC B; 16; 0x3CF864 (inside the membership object) |
| 32 | 0x39dd4c | cmp    r12d,0x11 jl | **C3** | 0x180-stride table at object+0x18 | stack buffer in caller 0x396828: r9 = rbp+0x68A0 / rbp+0x4350; 0x180 B; 16; caller stack frame (0x1800 bytes) |
| 33 | 0x39dd4c | cmp    r13d,0x10 jl | **C2** | steps 0xB0 over records at 0x39E212/0x39E219 | - |
| 34 | 0x39e47c | cmp    edx,0x11 jl | **C1** | imul 0x158 at 0x39E897 | - |
| 35 | 0x39ebf0 | cmp    ebx,0x11 jl | **C4** | mask scan (bt [rdi+0x2C]) + call 0x39D510 | - |
| 36 | 0x39f02c | cmp    eax,0x11 jge | **C1** | imul 0x158 at 0x39F0C0 | - |
| 37 | 0x3a3224 | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x3A3462 | - |
| 38 | 0x3c0248 | cmp    ebx,0x11 jl | **C3** | 6-byte stride table at [r15]+0x34 | object at [arg]+0x34 (r15=rcx, rdi=[r15]); 6 B; 16; owning object, caller 0x3C057C (type TBD) |
| 39 | 0x3dc138 | cmp    eax,0x10 jge | **C4** | mask scan only (bt [rsi+0x173C]) | - |
| 40 | 0x3e1ecc | cmp    edi,0x11 jge | **C1** | imul 0x158 at 0x3E1F1B | - |
| 41 | 0x400bc4 | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x400C4C | - |
| 42 | 0x4029e8 | cmp    eax,0x11 jge | **C1** | imul 0x158 at 0x402A76 | - |
| 43 | 0x40429c | cmp    eax,0x11 jge | **C1** | imul 0x158 at 0x40433A | - |
| 44 | 0x408ddc | cmp    eax,0x10 jge | **C1** | imul 0xB0 at 0x4093BB | - |
| 45 | 0x40a760 | cmp    ecx,0x10 jl | **C3** | 0xA0-stride table at object base | virtual method, vtable RVA 0xA47750; object rcx; 0xA0 B; 16; vtable owner object (allocation TBD) |
| 46 | 0x41e7c8 | cmp    edi,0x11 jl | **C2** | steps 0x158 at 0x41E9B5 | - |
| 47 | 0x41e7c8 | cmp    edi,0x11 jl | **C3** | 6-byte stride table at [rsi+8]+0x34 | object from global pointer 0x4E0E910 (read at 0x3A9B2E); 6 B; 16; static-static .data object 0x2C92050 (alloc TBD) |
| 48 | 0x41e7c8 | cmp    eax,0x10 jne | **C3** | object field checks ([rdi+0x18], [rax+0x60EF8]) | no table: two object fields ([rdi+0x18], [rax+0x60EF8]); -; -; object (not an array) |
| 49 | 0x41f510 | cmp    ebx,0x11 jl | **C2** | steps 0x158 at 0x41F576 | - |
| 50 | 0x41f510 | cmp    esi,0x10 jl | **C2** | steps 0xB0 at 0x41F61E; mask 0x173C | - |
| 51 | 0x41f510 | cmp    edx,0x10 jl | **C3** | 0xA0-stride table at object+0xB8 | outer object rcx = global 0x4E0E910 (loaded 0x3AB007); one 16 x 0xA0 array at +0xA0; 0xA0 B; 16; static .data object 0x2C92050 (membership at +0x18) |
| 52 | 0x41f510 | cmp    edx,0x10 jl | **C3** | 0xA0-stride table at object+0xA8 | same global object, +0xA8 (rdi = r14+0xA0); 0xA0 B; 16; global singleton |
| 53 | 0x41f510 | cmp    eax,0x10 jl | **C3** | 0xA0-stride table at object+0xB4 | same global object, +0xB4; 0xA0 B; 16; global singleton |
| 54 | 0x41f510 | cmp    esi,0x10 jl | **C2** | steps 0xB0 at 0x41F95B; mask 0x173C | - |
| 55 | 0x41f510 | cmp    ecx,0x10 jl | **C3** | 0xA0-stride table at object+0x150 | same global object, +0x150; 0xA0 B; 16; global singleton |
| 56 | 0x41f510 | cmp    ecx,0x10 jl | **C3** | 0xA0-stride table at object+0x148 | same global object, +0x148; 0xA0 B; 16; global singleton |
| 57 | 0x41f510 | cmp    r8d,0x11 jl | **C3** | 6-byte stride table at object+0x34 | same global object (rcx from [rsp+0x38]) +0x34; 6 B; 16; global singleton |
| 58 | 0x42008c | cmp    esi,0x11 jge | **C1** | imul 0x158 at 0x420133/0x420170 | - |
| 59 | 0x44e7e8 | cmp    eax,0x11 jge | **C1** | imul 0x158 at 0x44E909 | - |
| 60 | 0x44fadc | cmp    ebx,0x11 jl | **C4** | mask scan + call 0x395090 per slot | - |

**Totals: 31 C1 (covered), 5 C2 (step cave, loop list), 16 C3 (table), 8 C4 (indirect/mask).**

## What the C3 locations mean

- **Stack-only (5, 7)**: 16 qword pointer array at rbp+0x150 in the 0x390DB0 frame; a bigger frame
  is enough, no shadow storage.
- **Session membership object (8, 10, 31)**: the arrays live inside the static session object at
  0x2B07418 (constructor 0x3CF864). 8/10 are 16-word name copies (a length limit, not a player
  count), 31 is a 16 x 0xC table at +0x4440. Growing these means growing or shadowing the session
  object like the slot arrays.
- **Heap / outer objects (32, 38, 45, 47, 51-57)**: tables inside heap objects - the 16 x 0xA0
  player/lobby array at object+0xA0 in 0x41F510 (fields +0xA8/+0xB4/+0xB8/+0x148/+0x150), the
  0x180-stride table at +0x18, and the 6-byte tables at +0x34. The owning object's constructor has to
  grow those arrays (or they need shadow storage); check each object's allocator before 32 players.
- **No table (48)**: two object fields, not an array.

## Loops that already need only the compare

The 31 C1 loops touch slot records only through the patched 0x158/0xB0 imul; widening 0x10/0x11
is sufficient. The 8 C4 loops scan the player mask and call a per-slot helper; the helper's slot
access is covered, so they need the mask width and call-site index widened.

## Object allocation: static .data objects, shadow storage (corrected)

0x4E0E910 is writable .data (section ends at 0x4E53AB0). The initializer at 0x3A9544-0x3A9567 stores the
static addresses: 0x2C8DFD8 -> 0x4E0E908, 0x2C92050 -> 0x4E0E910 (next object 0x2C92B68, size 0xB18), and
0x2C92B68 -> 0x4E0E918. A second writer 0x3A994E stores rdi into 0x4E0E908 (runtime override).

The lobby/player-data entries start at **object+0xA8**, not +0xA0. Initializer 0x3CFCC8 builds them:

    3CFE8C lea rbx,[0x2C920F8]      ; = 0x2C92050 + 0xA8, first entry
    3CFE93 mov rcx,rbx
    3CFE96 call 0xD358             ; entry constructor
    3CFE9B add rbx,0xA0
    3CFEA2 sub rdi,1 ; jne         ; rdi = 0x10 (set at 0x3CFE08), 16 entries

so the array is 16 x 0xA0 at 0x2C920F8..0x2C92AF8; +0xAA8 is the next field. object+0xA0 is a **DWORD entry
mask** (`test [rdi],eax` in 0x41F510 with rdi = r14+0xA0) and is already 32-bit.

Plan (same scheme as the session slots): entries 0..15 stay in the static object; entries 16..31 go to a
Noble-owned shadow at a constant delta, built with the same entry constructor 0xD358. Do **not** relocate the
object through the globals - it has self-pointers and the static teardown at 0x91D978 would break.

## Cave inventory: entry base +0xA8

| Loop | Entry base | Field offset in the 0xA0 entry | Counter | Index arithmetic |
|---|---|---|---|---|
| 51 | object+0xA8 | +0x10 (0x41F712 lea rcx,[r14+0xB8]) | edx | 0x41F746 add rcx,0xA0 |
| 52 | object+0xA8 | +0 (rdi = r14+0xA0 at 0x41F584; lea r9,[rdi+8]) | edx | 0x41F787 add r9,0xA0 |
| 53 | object+0xA8 | +0xC (0x41F7BE lea rcx,[rdi+0x14]) | eax | 0x41F7DB add rcx,0xA0 |
| 55 | object+0xA8 | +0xA8 (0x41F9F0 lea r8,[rdi+0xB0]) | ecx | 0x41FA72 add r8,0xA0 |
| 56 | object+0xA8 | +0xA0 (0x41FA9C lea rdx,[rdi+0xA8]) | ecx | 0x41FAB6 add rdx,0xA0 |
| 50 | object+0xA8 | mask-driven (0x41F5AC test [r13+0x173c]) | esi | 0x41F61E add rbp,0xB0? (slot stride, not entry) |
| 45 | owner object (vtable 0x40A760) | +0 (rdx=rdi=object) | ecx | 0x40AA03 add rdx,0xA0 |

The 0xA0 stride also appears at 0x41F617 (`add rbx,0xA0`) and 0xA4230 (`add rdi,0xA0` in the G908/G910 reader
function 0xA411C); both are additional entry walks, not imul-indexed.

## 6-byte tables at object+0x34

No explicit builder was found: the initializer 0x3CFCC8 has no store into 0x2C92084..0x2C920E4, no RIP refs
to that range exist (the table is reached through the object pointer), and none of the 22 reader functions
contains an imul by 6. The tables are per-PEER: the loops bound at 0x11, so 17 x 6 = 0x66 at +0x34..+0x9A (IPv4+port per machine); the shadow split is 17, not 16. The only 6-byte arithmetic is the loop step: 0x41EA78 lea rcx,[rsi+0x34] +
0x41EB1A add rcx,6 (loop 47), 0x41FAE4 lea r10,[rcx+0x34] + 0x41FB10 add r10,6 (loop 57), and the 6-byte
stride in loop 38 ([rdi+0x34] with 0x3C0318 add rsi,6). Count is 17 (the loop bounds). It is most likely
zero-initialised BSS filled lazily per player record; if it has a builder it is outside the init function.

## Index/imul-style accesses into the tables

Scan of the 22 functions that read the three globals:

- **No `imul` by 0x A0 or 6** anywhere in those functions (the only stride multiples in the code are the
  slot-record 0x158/0xB0 and the loop bound tables);
- **add-step walks only**: 0x41F617, 0x41F746, 0x41F787, 0x41F7DB, 0x41F962 (all in 0x41F510) and 0xA4230
  (function 0xA411C) for 0xA0; 0x41EB1A / 0x41FB10 for 6;
- field accesses inside the entries are plain `[reg+field]` loads/stores (the offsets listed above), not
  index multiplications, so the shadow redirect only needs the entry-base walks and the mask test.

## Step sites for the SlotPatch caves (gen_steps.py format)

Every counter is incremented BEFORE the step in the same iteration (the inc is the instruction
immediately before the add in all 12 loops). Exact bytes, bound instruction and post-loop use:

| Site | Bytes | Ptr / ctr | Record | Bound | Stepped reg read after exit |
|---|---|---|---|---|---|
| 0x41F617 | 48 81 C3 A0 00 00 00 | rbx / esi | 0xA0 | 0x41F625 cmp esi,0x10 | no - rbx reloaded at 0x41F630 (mov rbx,[rsp+0x30]); the push is a misdecode |
| 0x41F746 | 48 81 C1 A0 00 00 00 | rcx / edx | 0xA0 | 0x41F74D cmp edx,0x10 | no - rcx overwritten at 0x41F75E |
| 0x41F787 | 49 81 C1 A0 00 00 00 | r9 / edx | 0xA0 | 0x41F78E cmp edx,0x10 | no - r9 rewritten before use |
| 0x41F7DB | 48 81 C1 A0 00 00 00 | rcx / eax | 0xA0 | 0x41F7E2 cmp eax,0x10 | no - rcx overwritten at 0x41F801 |
| 0x41F962 | 48 81 C5 A0 00 00 00 | rbp / esi | 0xA0 | 0x41F969 cmp esi,0x10 | verify - exit lands at 0x41F96E (misaligned decode) |
| 0x41FA72 | 49 81 C0 A0 00 00 00 | r8 / ecx | 0xA0 | 0x41FA79 cmp ecx,0x10 | no - r8 overwritten at 0x41FA83 |
| 0x41FAB6 | 48 81 C2 A0 00 00 00 | rdx / ecx | 0xA0 | 0x41FAC1 cmp ecx,0x10 | no - returns without reading rdx |
| 0x40AA03 | 48 81 C2 A0 00 00 00 | rdx / ecx | 0xA0 | 0x40AA0A cmp ecx,0x10 | no - rdx not read before the call |
| 0xA4230 | 48 81 C7 A0 00 00 00 | rdi / ebp | 0xA0 | 0xA4237 cmp ebp,0x10 | no - rdi not read after exit |
| 0x41EB1A | 48 83 C1 06 | rcx / edi | 6 | 0x41EB1E cmp edi,0x11 | no - rcx overwritten at 0x41EB31 |
| 0x41FB10 | 49 83 C2 06 | r10 / r8d | 6 | 0x41FB14 cmp r8d,0x11 | no - returns without reading r10 |
| 0x3C0318 | 48 83 C6 06 | rsi / ebx | 6 | 0x3C0320 cmp ebx,0x11 | verify - exit is 0x3C0325 (cold part) |

Copy-paste list for gen_steps.py (rva, ptr, ctr, record, first-shadow-slot, note):

    # C3 table walks (entry base object+0xA8 for 0xA0; per-peer +0x34 for 6-byte)
    STEPS_C3 = [
        (0x41F617, 'rbx', 'esi', 0xA0, 16, '41F510: 0xA0 entry walk (mask +0xA0), bound 0x41F625, rbx reloaded after exit'),
        (0x41F746, 'rcx', 'edx', 0xA0, 16, '41F510: entry field +0x10, bound 0x41F74D'),
        (0x41F787, 'r9',  'edx', 0xA0, 16, '41F510: entry field +0, bound 0x41F78E'),
        (0x41F7DB, 'rcx', 'eax', 0xA0, 16, '41F510: entry field +0xC, bound 0x41F7E2'),
        (0x41F962, 'rbp', 'esi', 0xA0, 16, '41F510: entry walk, bound 0x41F969, post-exit decode needs review'),
        (0x41FA72, 'r8',  'ecx', 0xA0, 16, '41F510: entry field +0xA8, bound 0x41FA79'),
        (0x41FAB6, 'rdx', 'ecx', 0xA0, 16, '41F510: entry field +0xA0, bound 0x41FAC1'),
        (0x40AA03, 'rdx', 'ecx', 0xA0, 16, '40A760: vtable object entry walk, bound 0x40AA0A'),
        (0xA4230,  'rdi', 'ebp', 0xA0, 16, 'A411C: extra entry walk, bound 0xA4237'),
        (0x41EB1A, 'rcx', 'edi', 6,   17, '41E7C8: per-peer table at [obj+8]+0x34, bound 0x41EB1E'),
        (0x41FB10, 'r10', 'r8d', 6,   17, '41F510: per-peer table, bound 0x41FB14'),
        (0x3C0318, 'rsi', 'ebx', 6,   17, '3C0248: per-peer table at [arg]+0x34, bound 0x3C0320'),
    ]

## Loop 57 and loop 47 object check

- Loop 47: `rsi = rcx` (0x41E8B1) is the object passed by caller 0x3A9B35, which loads it from
  [0x4E0E910] = static 0x2C92050. The 6-byte table is at **[rsi+8]+0x34**, i.e. the pointer field at
  object+8 (not object+0x34). No initializer store to object+8 exists in 0x3CFCC8, so that sub-object
  pointer is set elsewhere; the table itself is at that sub-object's +0x34.
- Loop 57: rcx at 0x41FAE4 comes from [rsp+0x38] (loaded at 0x41FACE). [rsp+0x38] is written only at
  0x41F701, from `r11 = 3 * [rbx-0x14]` - an index-derived value, NOT confirmed as 0x2C92050. The loop
  dereferences [r10], so [rsp+0x38] must be a real pointer; tracing which path sets [rbx-0x14] is needed
  before this one can be claimed as the static object.

## Follow-up checks (Claude's verification round)

**0x41F617 rbx**: not read after the loop - 0x41F630 `mov rbx,[rsp+0x30]` reloads it; the earlier
`push [rbx]` at 0x41F62D is a capstone misdecode. Corrected above.

**0x41FA72 / 0x41FAB6 are in 0x41F9C4, not 0x41F510.** That function (entry 0x41F9C4) takes rcx = object,
r8 = out mask pointer, r9 = out 0x80-byte buffer, plus stack arguments. Its caller is 0x3C2D7A
(the caller loads rcx with `mov rcx,[rdi]` at 0x3C2D44 - a pointer field of its own object argument, so a
runtime pointer; the chain above 0x3C2D7A is still needed to name it). The 0x80-byte buffer copied by 0x41FAB6 is **caller stack**: the caller
passes `r9 = [rsp+0x50]` and the memset target `[rsp+0x60]` is one of the caller's stack arguments
([rsp+0xC8]); it is not static.

**Leaf 0x41FAE0 (0x41FB10's function) is called by six sites:**

| Caller | rcx passed | Struct |
|---|---|---|
| 0xF8B96 | r12 (runtime object) | runtime lobby struct |
| 0x3AB902 | [0x4E0E910] = static 0x2C92050 | static lobby object |
| 0x3AC0AB | [0x4E0E910] (load at 0x3AC0A4) | static lobby object |
| 0x3CCD4F | [rcx] (runtime) | runtime object, mask +0x2C / 17x6 at +0x34 |
| 0x3CCECB | [r14] (runtime) | runtime object |
| 0x3CD0CA | [rdi] (runtime) | runtime object |

So the leaf's 17x6 contract is shared by the static lobby object and four runtime objects of the same layout.

**0x41F510 stack table at rsp+0x40 (17 x 6 = 0x66 at 0x40..0xA6).** All accesses:

| RVA | Instruction |
|---|---|
| 0x41F53D | `lea rdi,[rsp+0x40]` (table base) |
| 0x41F5C9 | `cmp eax,[rsp+rcx*2+0x40]` (rcx = 3*idx) |
| 0x41F5D3 | `cmp ax,[rsp+rcx*2+0x44]` |
| 0x41F727 | `cmp eax,[rsp+r11*2+0x40]` |
| 0x41F732 | `cmp ax,[rsp+r11*2+0x44]` |
| 0x41F8F2 | `mov eax,[rsp+rcx*2+0x40]` |
| 0x41F8FA | `movzx eax,word ptr [rsp+rcx*2+0x44]` |

Because it is a stack table inside 0x41F510, moving it to a Noble buffer means replacing these six
accesses (and the base lea) with shadow-buffer reads, not resizing an object.

**Still unnamed:**

- 0x40AA03: vtable 0xA47750 owner; no lea/mov reference to the vtable address was found in .text, so the
  class instance is reached indirectly.
- 0xA4230: function 0xA411C uses rcx as the object (rdi = rcx at 0xA413F) and steps 0xA0 over it; object
  type not resolved (it reads the G908/G910 globals in the same function).
- 0x41EB1A: table at [0x2C92050+8]+0x34; the pointer field at object+8 is set outside 0x3CFCC8.
- 0x3C0318: table at [arg]+0x34; caller 0x3C057C passes arguments of 0x3C034C (rsi = rcx, rdi = r8), so it
  is a caller-owned object, not one of the statics.

## 0xB18 struct: where instances are created or copied

Type: size 0xB18, peer mask +0x2C, 17 x 6 at +0x34, player mask +0xA0, 16 x 0xA0 at +0xA8 (entry ctor 0xD358).

**The only confirmed instance is the static one** at 0x2C92050: built by 0x3CFCC8, which has a single caller,
0x87D4 (a static-init thunk: `call 0x3CFCC8; lea rcx,[...]; jmp 0x7881F8`), so it is constructed once at
image init. Its 16 entry objects are built at 0x3CFE8C-0x3CFEA6 (`lea rbx,[0x2C920F8]`, ctor 0xD358 x16).

**No 0xB18 allocation or 0xB18 memcpy exists in .text.** The only 0xB18 immediate is `add rcx,0xB18` at
0x3E03B1, inside constructor 0x3E037C: it writes +0x708/+0x710/+0xB10 and then calls 0x3E031C on rcx+0xB18 -
so 0xB18 is a member/array stride there, not an allocation size. No `mov imm,0xB18` -> allocator and no
memcpy/memset with 0xB18 were found.

**Entry-constructor 0xD358 call sites (7):**

| Site | Base | Owner |
|---|---|---|
| 0x3CFE96 | 0x2C920F8 (= static+0xA8) | static 0x2C92050 |
| 0xD2EB | rcx+0x2C8 | 16-entry array inside a larger object (entries at +0x2C8, not +0xA8) |
| 0x3E0E62 | rbx (object base) | 16-entry array at object base; same function also builds +0xF78 and memsets +0x488/0x14F0 |
| 0x3E0E7E | r14+0xF78 | second 16-entry array in that larger object |
| 0x438D9 | stack rsp+0x30 | stack-local 16-entry array (frame 0xA40) |
| 0x409F46 | stack rsp+0x40 | stack-local 16-entry array (frame 0xA50) |
| 0x44E86E | stack rsp+0xA0 | stack-local 16-entry array (frame 0xAB0) |

**Stack instances:** a full 0xB18 struct would need a frame of at least 0xB18. The three functions that build
the 16 x 0xA0 array on the stack have frames 0xA40, 0xA50 and 0xAB0 - enough for the 0xA00 entry array only, not
the whole 0xB18 type (0xA40/0xA50/0xAB0 frames). None of the 0xD358 builders has a frame sized for the whole
struct, so there is no evidence of a complete 0xB18 instance on a stack frame.

**Larger objects that embed the layout** (candidates for the runtime instances):

- 0x3E037C constructor: fields +0x708/+0x710/+0xB10, then sub-object at +0xB18 init by 0x3E031C (which sets
  string fields at +0/+4/+0x24 and clears +0x128/+0x130/+0x138/+0x140).
- 0x3E0E3E constructor: two 16 x 0xA0 arrays (object base and +0xF78) plus a 0x14F0 memset at +0x488; frame
  large - a session/lobby object, not a plain 0xB18.
- 0xD2B0 constructor: 16 x 0xA0 entries at +0x2C8, fields at +0x1C/+0x24/+0x2C/+0x34/+0x1D8.

0x3E031C (the sub-object initializer) has 19 callers (0x84D2, 0x86D7, 0x242BFB, 0x345C29, 0x3ACA9C, 0x3BC65B,
0x3CF628, 0x3DC253, 0x3DE10F, 0x3E03B8, 0x3F4151, 0x3F4B82, 0x3F4BDE, 0x3F60F8, 0x3F62FA, 0x42F2C7, 0x431174,
0x432884, 0x432932), i.e. many objects embed that sub-structure.

**Conclusion for SlotPatch:** the 0xB18 layout type is only ever materialised as (a) the static 0x2C92050 and
(b) 16 x 0xA0 entry arrays inside larger objects or on the stack. There are no standalone 0xB18 heap/static
instances and no 0xB18 copy routine, so the shadow-copy plan targets the static object's entry array (entries
16..31) and the larger objects' own arrays, not a per-instance allocation site.
