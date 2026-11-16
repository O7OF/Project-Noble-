# State banks and static checkpoint buffers

| Allocation | Stock bytes | V16 bytes |
|---|---:|---:|
| Render bank | 3,604,480 | 5,636,096 |
| Main bank, each of three copies | 4,784,128 | 41,746,432 |
| Checkpoint prefix | 10,878,976 | 49,872,896 |
| Checkpoint buffer bytes (prefix + 32) | 10,879,008 | 49,872,928 |
| Whole game-state partition | 20,447,232 | 133,365,760 |

| Static allocation | Stock RVA | V16 RVA |
|---|---|---|
| First checkpoint buffer | `0x00FD7120` | `0x15181000` |
| Second checkpoint buffer | `0x01A37140` | `0x18111020` |
| Queue header | `0x02497160` | `0x1B0A1040` |
| End | `0x024971A0` | `0x1B0A1080` |

The two static buffers each reserve checkpoint-prefix bytes plus 32. The queue reserves 64 bytes. All final addresses are in zero-filled virtual storage and have no file offsets. Change the PE section virtual size and SizeOfImage along with moved backing.

## Reservations and size operands

| Operand RVA | File offset | Stock bytes | V16 bytes | Meaning |
|---|---|---|---|---|
| `0x000161A9` | `0x000155A9` | `00 00 49 00` | `00 00 7D 02` | main and two rollback bank reservations |
| `0x0000C3FE` | `0x0000B7FE` | `20 00 A6 00` | `20 00 F9 02` | game-state and checkpoint size |
| `0x0000C473` | `0x0000B873` | `00 00 38 01` | `00 00 F3 07` | game-state and checkpoint size |
| `0x0000C49C` | `0x0000B89C` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x00015FB7` | `0x000153B7` | `00 00 38 01` | `00 00 F3 07` | game-state and checkpoint size |
| `0x000160BB` | `0x000154BB` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x0001612F` | `0x0001552F` | `00 00 38 01` | `00 00 F3 07` | game-state and checkpoint size |
| `0x00016B1E` | `0x00015F1E` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x00016E7D` | `0x0001627D` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x00017168` | `0x00016568` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x000171C7` | `0x000165C7` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x000171CF` | `0x000165CF` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x000171EB` | `0x000165EB` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x0003559F` | `0x0003499F` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x00088249` | `0x00087649` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x00088766` | `0x00087B66` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x00088A5A` | `0x00087E5A` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x000A53E7` | `0x000A47E7` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x001713DB` | `0x001707DB` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x001716C9` | `0x00170AC9` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x00171939` | `0x00170D39` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x00171C48` | `0x00171048` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x001721B4` | `0x001715B4` | `00 00 A6 00` | `00 00 F9 02` | game-state and checkpoint size |
| `0x0001619A` | `0x0001559A` | `00 00 37 00` | `00 00 56 00` | render bank reservation |

## References to the new static buffers

These four-byte values are signed RIP-relative displacements, not capacities or direct addresses.

| Instruction RVA | Operand file offset | Stock target RVA | V16 target RVA | V16 displacement bytes |
|---|---|---|---|---|
| `0x000037F0` | `0x00002BF2` | `0x02497184` | `0x1B0A1064` | `6D D8 09 1B` |
| `0x000037F7` | `0x00002BF9` | `0x02497180` | `0x1B0A1060` | `62 D8 09 1B` |
| `0x0000C3D8` | `0x0000B7DB` | `0x02497160` | `0x1B0A1040` | `61 4C 09 1B` |
| `0x0000C3E8` | `0x0000B7EB` | `0x02497170` | `0x1B0A1050` | `61 4C 09 1B` |
| `0x0000C3F5` | `0x0000B7F8` | `0x00FD7120` | `0x15181000` | `04 4C 17 15` |
| `0x0000C40D` | `0x0000B80F` | `0x02497180` | `0x1B0A1060` | `4D 4C 09 1B` |
| `0x0000C419` | `0x0000B81C` | `0x02497184` | `0x1B0A1064` | `44 4C 09 1B` |
| `0x0000C427` | `0x0000B82A` | `0x01A37140` | `0x18111020` | `F2 4B 10 18` |
| `0x0000C444` | `0x0000B847` | `0x02497170` | `0x1B0A1050` | `05 4C 09 1B` |
| `0x0000CD21` | `0x0000C124` | `0x02497160` | `0x1B0A1040` | `18 43 09 1B` |
| `0x0000CD41` | `0x0000C144` | `0x02497170` | `0x1B0A1050` | `08 43 09 1B` |
| `0x0000CD56` | `0x0000C159` | `0x02497180` | `0x1B0A1060` | `03 43 09 1B` |
| `0x0000CEA2` | `0x0000C2A5` | `0x02497160` | `0x1B0A1040` | `97 41 09 1B` |
| `0x0000CEDA` | `0x0000C2DD` | `0x02497170` | `0x1B0A1050` | `6F 41 09 1B` |
| `0x0000CEE7` | `0x0000C2EA` | `0x02497160` | `0x1B0A1040` | `52 41 09 1B` |
| `0x0000CF09` | `0x0000C30C` | `0x02497170` | `0x1B0A1050` | `40 41 09 1B` |
| `0x0000CF54` | `0x0000C357` | `0x02497170` | `0x1B0A1050` | `F5 40 09 1B` |
| `0x0000CF6E` | `0x0000C371` | `0x02497160` | `0x1B0A1040` | `CB 40 09 1B` |
| `0x0000CF8F` | `0x0000C392` | `0x02497170` | `0x1B0A1050` | `BA 40 09 1B` |
| `0x0000CFCA` | `0x0000C3CD` | `0x02497160` | `0x1B0A1040` | `6F 40 09 1B` |
