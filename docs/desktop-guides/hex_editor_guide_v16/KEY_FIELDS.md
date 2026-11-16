# Key limit and size operands

Each row identifies a value inside an instruction. File offsets point at the operand bytes, not the instruction start. Multi-byte integers are little-endian. Stock and V16 locations differ where code was relocated. These rows identify values; use the complete overwrite list plus append block to reproduce the complete patch.

| Field | Stock file offset | V16 file offset | Stock bytes → V16 bytes | Value before → after |
|---|---|---|---|---|
| Shared object headers | `0x00469F1A` | `0x00C9DE62` | `00 08 00 00` → `FF 7F 00 00` | 2,048 → 32,767 |
| Object body heap bytes | `0x00469F54` | `0x00C9DE9C` | `00 60 22 00` → `00 00 26 02` | 2,252,800 → 36,044,800 |
| Object render-data array bytes | `0x00469F87` | `0x00C9DECF` | `00 20 00 00` → `00 00 02 00` | 8,192 → 131,072 |
| Render-message queue allocation bytes | `0x0046A0EB` | `0x00C9E033` | `0C 41 00 00` → `0C 10 04 00` | 16,652 → 266,252 |
| Cached render states | `0x002559E6` | `0x002559E6` | `00 04 00 00` → `00 10 00 00` | 1,024 → 4,096 |
| Cached render auxiliary heap allocation bytes | `0x0025596D` | `0x0025596D` | `00 00 06 00` → `00 00 18 00` | 393,216 → 1,572,864 |
| Cached render auxiliary heap registration bytes | `0x00255991` | `0x00255991` | `00 00 06 00` → `00 00 18 00` | 393,216 → 1,572,864 |
| Havok components | `0x0009862A` | `0x0009862A` | `DC 05 00 00` → `E0 2E 00 00` | 1,500 → 12,000 |
| Havok contacts | `0x00098675` | `0x00098675` | `00 04 00 00` → `FF 7F 00 00` | 1,024 → 32,767 |
| Permanent arena physical reservation bytes | `0x0000B8CD` | `0x0000B8CD` | `00 00 4B 00` → `00 00 00 01` | 4,915,200 → 16,777,216 |
| Permanent arena boundary bytes | `0x000169BA` | `0x000169BA` | `00 00 4B 00` → `00 00 00 01` | 4,915,200 → 16,777,216 |
| Havok working block initial bytes | `0x001E5ECB` | `0x001E5ECB` | `00 00 38 00` → `00 00 00 04` | 3,670,016 → 67,108,864 |
| Havok working memory soft limit bytes | `0x001E5F17` | `0x001E5F17` | `99 99 2F 00` → `66 66 66 03` | 3,119,513 → 57,042,534 |
| Havok backing callback block bytes | `0x001E63B7` | `0x001E63B7` | `00 10 27 00` → `00 00 00 04` | 2,560,000 → 67,108,864 |
| Havok backing reset block bytes | `0x001E63DE` | `0x001E63DE` | `00 00 38 00` → `00 00 00 04` | 3,670,016 → 67,108,864 |
| Havok physics phase block bytes | `0x000CEB6B` | `0x000CEB6B` | `00 10 27 00` → `00 00 00 04` | 2,560,000 → 67,108,864 |
| Physics world entity reclaim threshold | `0x0009A204` | `0x0009A204` | `A4 06 00 00` → `E0 2E 00 00` | 1,700 → 12,000 |
| Physics world entity low-water threshold | `0x0009B7C0` | `0x0009B7C0` | `A0 06 00 00` → `DC 2E 00 00` | 1,696 → 11,996 |
| Physics world entity repeated low-water threshold | `0x0009B7D4` | `0x0009B7D4` | `A0 06 00 00` → `DC 2E 00 00` | 1,696 → 11,996 |
| Render packet arena reset bytes | `0x0000BC06` | `0x0000BC06` | `00 5C 03 00` → `00 00 10 00` | 220,160 → 1,048,576 |
| Render packet arena allocation bytes | `0x0024FD73` | `0x0024FD73` | `00 5C 03 00` → `00 00 10 00` | 220,160 → 1,048,576 |
| Render packet generation shift bits | `0x0024FD9A` | `0x0024FD9A` | `12` → `14` | 18 → 20 |
| Visible list and mesh constructor capacity | `0x0014DEB7` | `0x0014DEB7` | `00 04 00 00` → `00 10 00 00` | 1,024 → 4,096 |
| Main and rollback bank reservation bytes | `0x000155A9` | `0x000155A9` | `00 00 49 00` → `00 00 7D 02` | 4,784,128 → 41,746,432 |
| Render bank reservation bytes | `0x0001559A` | `0x0001559A` | `00 00 37 00` → `00 00 56 00` | 3,604,480 → 5,636,096 |
| Collision element capacity | `0x0014F433` | `0x0014F433` | `00 1C 00 00` → `FF 7F 00 00` | 7,168 → 32,767 |
| Collision guard header-field displacement | `0x00150933` | `0x00150933` | `44` → `48` | 68 → 72 |
| Primary solver arena bytes | `0x001E607C` | `0x001E607C` | `00 40 06 00` → `00 00 00 01` | 409,600 → 16,777,216 |
| Scheduler job / queue capacity | `0x0068A09A` | `0x0068A09A` | `00 10 00 00` → `00 40 00 00` | 4,096 → 16,384 |
| Scheduler job / queue capacity | `0x0068A11F` | `0x0068A11F` | `00 10 00 00` → `00 40 00 00` | 4,096 → 16,384 |
| Scheduler job / queue capacity | `0x0068A137` | `0x0068A137` | `00 10 00 00` → `00 40 00 00` | 4,096 → 16,384 |
| Scheduler job / queue capacity | `0x0068A4DC` | `0x0068A4DC` | `00 10 00 00` → `00 40 00 00` | 4,096 → 16,384 |
| Scheduler job / queue capacity | `0x0068AA16` | `0x0068AA16` | `00 10 00 00` → `00 40 00 00` | 4,096 → 16,384 |
| Scheduler dependency count | `0x0068AA53` | `0x0068AA53` | `00 10 00 00` → `00 00 01 00` | 4,096 → 65,536 |
| Scheduler dependency final index | `0x0068AA39` | `0x0068AA39` | `FF 0F 00 00` → `FF FF 00 00` | 4,095 → 65,535 |
| Object-job scratch flag-clear count | `0x004699BA` | `0x004699BA` | `00 08 00 00` → `00 40 00 00` | 2,048 → 16,384 |
| Input queue and companion handle clear bytes | `0x0046B203` | `0x0046B203` | `00 20 00 00` → `00 40 00 00` | 8,192 → 16,384 |
| Input queue and companion handle clear bytes | `0x0046B609` | `0x0046B609` | `00 20 00 00` → `00 40 00 00` | 8,192 → 16,384 |
| Render packet offset mask | `0x0024A43F` | `0x0024A43F` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x0024A545` | `0x0024A545` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x0027E5B3` | `0x0027E5B3` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x00298037` | `0x00298037` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x002980D6` | `0x002980D6` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x002989C8` | `0x002989C8` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x0029952B` | `0x0029952B` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x00299554` | `0x00299554` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x002997B6` | `0x002997B6` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |
| Render packet offset mask | `0x002A1864` | `0x002A1864` | `FF FF 03 00` → `FF FF 0F 00` | 262,143 → 1,048,575 |

The accompanying `KEY_FIELDS.csv` also includes the stock and V16 RVAs and location notes.
