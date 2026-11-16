# Halo Reach MCC — V16 hex-editor change guide

This documents the installed **V16** `haloreach.dll`: exact stock-to-V16 limit changes, operand locations, relocated storage, and bytes required to reproduce it in a hex editor. The proposed 6,144-entry input queue is **not included**; V16 still has a 4,096-entry input queue.

## 1. Identify the exact DLL

These addresses apply to this specific stock binary and the V16 result below. Another MCC build can have different offsets.

| Item | Stock input | V16 output |
|---|---|---|
| File size, decimal | 13,229,016 bytes | 14,198,784 bytes |
| File size, hexadecimal | `0x00C9DBD8` | `0x00D8A800` |
| SHA-256 | `738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894` | `b8810c3b9822f5d6563439fc6b8b16c2def6cd7a6f660dd3775c42edcd0a5f24` |

Work on a copy with MCC closed. Use a hex editor's **hexadecimal byte view** and **overwrite mode** for replacements.

**File offset** means a physical position in the DLL on disk. **RVA** means an address relative to the DLL's loaded base. Use the **file offset column** for a hex editor. They are not interchangeable. For this file's `.text` section, file offset = RVA − `0xC00`; that shortcut does not apply to the added sections.

Integers are little-endian. For example:

| Decimal | Hex integer | Four bytes in the file |
|---:|---|---|
| 2,048 | `0x00000800` | `00 08 00 00` |
| 4,096 | `0x00001000` | `00 10 00 00` |
| 12,000 | `0x00002EE0` | `E0 2E 00 00` |
| 16,384 | `0x00004000` | `00 40 00 00` |
| 32,767 | `0x00007FFF` | `FF 7F 00 00` |
| 65,536 | `0x00010000` | `00 00 01 00` |

## 2. Reproduce the complete installed patch

Several changes required new machine code, relocated arrays, PE section headers, and matching unwind metadata. The key limit operands alone are not a complete stock-to-V16 patch.

1. Open a copy of the matching stock DLL. Its length must be **13,229,016 bytes**.
2. Apply **every row** in [ALL_OVERWRITES.md](ALL_OVERWRITES.md), or use the identical [CSV list](ALL_OVERWRITES.csv). Go to the listed **file offset**, confirm the stock bytes, and overwrite them with the V16 bytes. Do not insert or remove bytes in this portion of the file. Rows are ordered by offset and do not overlap.
3. Open [APPEND_AT_ORIGINAL_EOF.bin](APPEND_AT_ORIGINAL_EOF.bin) in a second hex-editor tab. Copy **all 969,768 raw bytes**.
4. In the DLL, go to its original end at **file offset `0x00C9DBD8`**. Append/insert those raw bytes once. This is the only step that increases the file length. Paste raw bytes, not the ASCII characters of a hex string.
5. Save the DLL. The result is **14,198,784 bytes**, ending at `0x00D8A800`, with the V16 SHA-256 above.

The overwrite list contains **642 replacement rows**. It includes PE headers, redirects, existing instruction operands, and metadata. The append block supplies the added code, exception/unwind tables, recording code, and alignment padding. Its SHA-256 is `e2c1366d610f85b26f78cb139ef2f34e749dc18b3a8b6e0892cd4a2f0f4ebd17`.

Do **not** pad the file to the loaded image size. Large buffers are zero-filled virtual sections declared by the PE headers; their reserved memory is not stored as hundreds of megabytes of file data.

## 3. Limits before and after

| Resource | Stock | Installed V16 |
|---|---:|---:|
| Shared object header slots | 2,048 | **32,767** |
| Object body heap | 2,252,800 bytes | **36,044,800 bytes** |
| Object render-data array | 8,192 bytes | **131,072 bytes** |
| Render-message queue physical records | 2,048 | **32,768** |
| Render-message visited bitmap | 256 bytes | **4,096 bytes** |
| Cached object render states | 1,024 | **4,096** |
| Render-state auxiliary heap | 393,216 bytes | **1,572,864 bytes** |
| Havok component slots | 1,500 | **12,000** |
| Havok contact slots | 1,024 | **32,767** |
| Permanent allocation arena | 4,915,200 bytes | **16,777,216 bytes** |
| Physics working arenas | 3,670,016-byte primary; 2,560,000-byte secondary backing | **67,108,864 bytes each** |
| Physics working-memory soft limit | 3,119,513 bytes | **57,042,534 bytes** |
| World entity reclamation threshold | 1,700 | **12,000** |
| World entity low-water threshold | 1,696 | **11,996** |
| Separate solver primary arena | 409,600 bytes | **16,777,216 bytes** |
| Separate solver secondary arena | 204,800 bytes | **16,777,216 bytes** |
| Object-indexed spatial visited bitmaps | 256 bytes each | **4,096 bytes each** |
| Primary visibility scratch slots | 2,048 | **32,768** |
| Per-frame render packet arena | 220,160 bytes | **1,048,576 bytes** |
| Render packet offset bits / generation bits | 18 / 14 | **20 / 12** |
| Visible render list entries | 1,024 | **4,096** |
| Render mesh records | 1,024 | **4,096** |
| Physics collector object entries | 2,048 | **32,768** |
| Physics collector tag entries | 2,048 | **32,768** |
| Shared scheduler job slots | 4,096 | **16,384** |
| Shared scheduler dependency entries | 4,096 | **65,536** |
| Scheduler ready/free queue slots, each | 4,096 | **16,384** |
| Per-call object-job scratch slots | 2,048 | **16,384** |
| Input object-update queue entries | 2,048 | **4,096** |
| Companion input array entries | 2,048 | **4,096** |
| Collision hierarchy element slots | 7,168 | **32,767** |
| Collision hierarchy node/hash capacity | 6,144 | **6,144, unchanged** |
| Player pool | 16 | **16, unchanged** |

The shared object allocation limit, input queue capacity, scheduler slots, and collision elements are separate quantities. V16 does not turn all of them into a single 32,767-object gameplay limit.

## 4. Main editable limit operands

[KEY_FIELDS.md](KEY_FIELDS.md) gives **48 important operands**, with stock and V16 file offsets, exact byte replacements, and decimal values. [KEY_FIELDS.csv](KEY_FIELDS.csv) also includes RVAs.

Examples of fields that remain at the same file position:

| Field | File offset | Stock bytes | V16 bytes |
|---|---|---|---|
| Havok components | `0x0009862A` | `DC 05 00 00` | `E0 2E 00 00` |
| Havok contacts | `0x00098675` | `00 04 00 00` | `FF 7F 00 00` |
| Permanent arena reservation | `0x0000B8CD` | `00 00 4B 00` | `00 00 00 01` |
| Permanent arena boundary | `0x000169BA` | `00 00 4B 00` | `00 00 00 01` |
| Cached render states | `0x002559E6` | `00 04 00 00` | `00 10 00 00` |
| Collision elements | `0x0014F433` | `00 1C 00 00` | `FF 7F 00 00` |
| Collision registration guard displacement | `0x00150933` | `44` | `48` |

The collision guard's `44 → 48` change selects **header+0x48, current occupancy**, instead of **header+0x44, allocation high-water mark**. It is a one-byte structure displacement change, not a count increase. Full occupancy still refuses allocation; freed holes can be reused after the high-water mark reaches capacity.

The object constructor was relocated. The active V16 object-count operand is at **file offset `0x00C9DE62`**, containing `FF 7F 00 00`. Its original stock operand was at `0x00469F1A`, containing `00 08 00 00`. Editing only the old constructor copy in V16 does not change the active constructor.

## 5. Changes that must stay coordinated

### Object pool, heap, and render-message queues

- Object headers retain **24-byte records**. The object-body heap and render-data array grow with the header capacity.
- Render-message records remain **8 bytes each**. The bitmap grows from 64 to **1,024 dwords**. The message data offset changes **`0x10C → 0x100C`**, and the allocation changes **`0x410C → 0x4100C` bytes**.
- Producers, consumers, merge/reset operations, delete-time invalidation and bitmap accesses use the new layout. Bounded queue reservations and the merge path's reserved-slot behavior remain included.
- The object constructor, invalidation writer and two render-cache writers are redirected to relocated code. Their exact redirects and bodies are part of the overwrite/append recipe.

### Render states and packet handles

- Cached states retain **624-byte records**. Count, auxiliary heap allocation/registration and render-bank space grow together.
- The packet arena moves to RVA **`0x12AE9000`**. Allocation/reset sizes become 1 MiB.
- The packet offset mask changes **`0x0003FFFF → 0x000FFFFF`** at all ten decoder operands in `KEY_FIELDS.md`. Packet creation shifts the generation by **18 → 20 bits**. Both allocation sizes and all handle encoders/decoders must use the same format.
- The 20,480-byte priority reserve remains unchanged.

### Visible list and mesh pool

- The visible-list header moves **`0x0291C7B0 → 0x12BE9000`**; mesh header moves **`0x029207C0 → 0x12BF9020`**.
- Visible records remain 16 bytes at list+`0x1C`; mesh records remain 168 bytes at mesh+`0x20`.
- Constructor capacity, direct references, module-indexed operands and mesh offsets relative to the list base move together. A trampoline restores an unrelated constructor count to 1,024 after the shared setup.
- Mesh exhaustion now takes a refusal path before an invalid index can write into the preceding header. That code change is included alongside the increased capacity.

### Physics pools and arenas

- Components retain **192-byte records** and 16-byte data alignment. Contacts retain **80-byte records**. Their data and bitmap layout is derived from pool capacity.
- The permanent arena's reservation **and** boundary check both change to 16 MiB.
- Primary/secondary working backings are at RVAs **`0x0AAE9000` / `0x0EAE9000`**, 64 MiB each. All block-size overrides and fallback registrations point to the matching backing.
- The separate solver buffers are at RVAs **`0x12EE5000` / `0x13EE5000`**, 16 MiB each, with end RVA **`0x14EE5000`**.
- Reclamation thresholds grow to 12,000/11,996; cleanup and contact recycling remain enabled.

### Physics collector and spatial scratch

- Collector backing moves to RVA **`0x12CA2000`**, total **`0x41010` bytes**. Relative layout: bitmap `+0x0008`, objects `+0x1008`, object count `+0x21008`, tags `+0x2100C`, tag count `+0x4100C`.
- Collector object/tag arrays both hold 32,768 entries; its bitmap is 4,096 bytes.
- Visibility scratch's table offset changes **`0x104 → 0x1004`**, separately from the render-message queue's `0x100C` header.
- Stack reservations, saved registers, unwind offsets, page probes and GS cookies are coordinated across the affected spatial/visibility frames. Their function and unwind locations are listed in [SPATIAL_FRAMES.md](SPATIAL_FRAMES.md).

### Scheduler and input queue

- [SCHEDULER.md](SCHEDULER.md) lists the old/new scheduler owner, records, dependency, queue and scratch layouts, including the pointer/counter corrections.
- [INPUT_QUEUE.md](INPUT_QUEUE.md) lists the primary/companion layout and **every queue-specific operand replacement**. The primary queue's active flag remains at its original address.
- These require updating array backing, counts, flags, all field accesses, constructor/cleanup paths and metadata together. V16 keeps the input queue at 4,096 entries and the scheduler at 16,384 job slots.

### State banks and checkpoint buffers

- The main bank and both rollback copies reserve 41,746,432 bytes each. The render bank reserves 5,636,096 bytes.
- The whole partition is 133,365,760 bytes; checkpoint prefix is 49,872,896 bytes.
- Both static checkpoint buffers and their queue move with matching sizes and references. All **22 size operands** and **20 buffer references** are listed in [STATE_BANKS.md](STATE_BANKS.md).
- [PE_LAYOUT.md](PE_LAYOUT.md) lists the final section headers, raw offsets, virtual reservations, exception directory and checksum.

## 6. Complete operand index

[COORDINATED_OPERANDS.csv](COORDINATED_OPERANDS.csv) lists **839 small operand/code edits**, grouped by subsystem, with their final RVAs, file offsets, stock-location bytes and V16 bytes. It includes secondary displacements and unwind/cookie operands beyond the main limit table.

Entries in a **new file area** have no stock bytes at that location; their bytes come from the append block. Some entries describe overlapping parts of an instruction or structure. Use this index for locating fields; use the non-overlapping `ALL_OVERWRITES` list plus the append block for complete replication.

Four-byte RIP-relative operands are **signed displacements**, not absolute pointers or capacities:

`target RVA = instruction RVA + full instruction length + signed displacement`

When choosing different backing RVAs, those displacements must be recalculated. New raw file locations and new virtual addresses must also agree with the PE section map.

## 7. Other included changes and unchanged limits

V16 includes two deletion-event recording wrappers at call sites RVA **`0x0046F0FC` / `0x0046F2C3`**, plus a physics-reclamation recording wrapper. They record events without changing deletion decisions. Their code, calls and recording-buffer reservations are included in the exact byte recipe.

Player capacity remains **16**. Collision node/hash capacity remains **6,144**. The guarded optional object-cache path at RVA `0x0046D8B4` remains limited to original indices through `0x7FF`. Garbage cleanup, render-cache eviction and contact recycling remain enabled. No map or tag changes are part of this guide.

All values in this document are the **final installed V16 values**, rather than intermediate limits from earlier revisions.
