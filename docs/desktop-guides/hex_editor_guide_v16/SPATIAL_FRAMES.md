# Spatial and visibility stack frames

Object-indexed visited bitmaps grow from 256 to 4,096 bytes. The primary visibility table reserves 32,768 slots; its table offset changes 0x104 → 0x1004. These are stack-frame changes, so the matching stack reservation, saved-register offsets, unwind information, page probes and GS cookie operands are part of the same change.

| Function RVA | Frame / stack reservation before | After | Unwind metadata RVA |
|---|---|---|---|
| `0x00227250` | `0x000022F0` | `0x000220F0` | `0x00AADA2C` |
| `0x00227538` | `0x00002170` | `0x00021070` | `0x00AADA54` |
| `0x0027E624` | `0x00002180` | `0x00021080` | `0x00AB08E4` |
| `0x000CFD1C` | `0x00002160` | `0x00003060` | `0x00AA2FCC` |
| `0x0047427C` | `0x00002160` | `0x00003060` | `0x00AC2380` |
| `0x000D3E6C` | `0x000001C8` | `0x000010C8` | `0x00AA335C` |
| `0x001284F8` | `0x00000230` | `0x00001130` | `0x00AA53D4` |
| `0x0012969C` | `0x00016910` | `0x00017810` | `0x00AA5470` |
| `0x0012A1CC` | `0x00001AE0` | `0x000029E0` | `0x00AA54B4` |
| `0x0012B3CC` | `0x000021F8` | `0x000030F8` | `0x00AA5568` |
| `0x001683EC` | `0x000007B0` | `0x000016B0` | `0x00AA7B20` |
| `0x001E9B90` | `0x000046B0` | `0x000055B0` | `0x00AABCF4` |
| `0x002EC404` | `0x00000160` | `0x00001060` | `0x00AB49D8` |
| `0x00473ED0` | `0x00000170` | `0x00001070` | `0x00AC2340` |
| `0x00474118` | `0x00000180` | `0x00001080` | `0x00AC2360` |
| `0x006BDDC0` | `0x000001A0` | `0x000010A0` | `0x00AD71BC` |
| `0x00736D00` | `0x00000160` | `0x00001060` | `0x00AB4748` |
| `0x007370B4` | `0x00000230` | `0x00001130` | `0x00ADADD4` |
| `0x007373F4` | `0x00000C00` | `0x00001B00` | `0x00ADADFC` |
| `0x0073926C` | `0x00000160` | `0x00001060` | `0x00ADAEB0` |

Some functions contain more than one bitmap. The complete operand list contains every changed clear length, field displacement, stack size, cookie location and unwind entry. The additional probe instructions and their runtime-function entries are in the append block. Do not change only a frame’s `SUB RSP` value.
