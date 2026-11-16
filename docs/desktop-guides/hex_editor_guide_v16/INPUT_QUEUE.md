# Input object-update queue: stock to V16

This input queue is distinct from the 16,384-slot job scratch and scheduler. Its capacity is encoded by its layout and clear sizes; there is no single constructor count to change. All offsets below are relative to each queue base.

| Item | Stock | V16 |
|---|---|---|
| Entries | 2,048 | 4,096 |
| Primary base RVA | `0x00C0D864` | `0x15176020` |
| Object datum array | `+0x0000`, 2,048 × 4 | `+0x0000`, 4,096 × 4 |
| Flag array | `+0x2000`, 2,048 × 1 | `+0x4000`, 4,096 × 1 |
| Total count | `+0x2800` | `+0x5000` |
| Flagged count | `+0x2804` | `+0x5004` |
| Processed count | `+0x2808` | `+0x5008` |
| Processed flagged count | `+0x280C` | `+0x500C` |
| Primary bytes | `0x2810` | `0x5010` |
| Companion base RVA | `0x00C10074` | `0x1517C000` |
| Companion count offset | `+0x2000` | `+0x4000` |
| Companion bytes | `0x2004` | `0x4004` |

The separate phase-active byte remains at RVA `0x00C0D850`. Only the writer alias at instruction RVA `0x004749D4` moves from old base−0x14 to new base−0x14. Its indexed handle displacement stays +0x14; its indexed flag displacement changes +0x2014 → +0x4014. Both phase clears change 0x2000 → 0x4000 bytes.

Changing this queue requires matching backing, all counters, all flag accesses, both clears, and the companion array. Worst-case job demand can be two jobs per input entry, in addition to other scheduler work.

## Exact operand bytes

| Operand RVA | File offset | Stock bytes | V16 bytes | Meaning |
|---|---|---|---|---|
| `0x00057865` | `0x00056C65` | `FB 5F BB 00` | `B7 E7 11 15` | relocated update queue global |
| `0x000578D0` | `0x00056CD0` | `90 5F BB 00` | `4C E7 11 15` | relocated update queue global |
| `0x0046BE19` | `0x0046B219` | `47 1A 7A 00` | `03 A2 D0 14` | relocated update queue global |
| `0x0046C19B` | `0x0046B59B` | `C5 16 7A 00` | `81 9E D0 14` | relocated update queue global |
| `0x0046C1FC` | `0x0046B5FC` | `64 16 7A 00` | `20 9E D0 14` | relocated update queue global |
| `0x0046BE27` | `0x0046B227` | `39 42 7A 00` | `F5 F1 D0 14` | relocated update queue global |
| `0x0046C188` | `0x0046B588` | `D8 3E 7A 00` | `94 EE D0 14` | relocated update queue global |
| `0x0046C1F4` | `0x0046B5F4` | `6B 3E 7A 00` | `27 EE D0 14` | relocated update queue global |
| `0x004749E4` | `0x00473DE4` | `7C B6 79 00` | `38 66 D0 14` | relocated update queue global |
| `0x004749F2` | `0x00473DF2` | `6E B6 79 00` | `2A 66 D0 14` | relocated update queue global |
| `0x00474A00` | `0x00473E00` | `60 B6 79 00` | `1C 66 D0 14` | relocated update queue global |
| `0x0046BE2E` | `0x0046B22E` | `3A 42 7A 00` | `F6 F1 D0 14` | relocated update queue global |
| `0x0046C203` | `0x0046B603` | `64 3E 7A 00` | `20 EE D0 14` | relocated update queue global |
| `0x0046BE34` | `0x0046B234` | `3C 62 7A 00` | `C8 41 D1 14` | relocated update queue global |
| `0x0046C21C` | `0x0046B61C` | `54 5E 7A 00` | `E0 3D D1 14` | relocated update queue global |
| `0x0046BE43` | `0x0046B243` | `2D 42 7A 00` | `B9 01 D1 14` | relocated update queue global |
| `0x0046C22B` | `0x0046B62B` | `45 3E 7A 00` | `D1 FD D0 14` | relocated update queue global |
| `0x00474A09` | `0x00473E09` | `5B B6 79 00` | `17 66 D0 14` | relocated update queue global |
| `0x004749D7` | `0x00473DD7` | `75 8E 79 00` | `31 16 D0 14` | writer indexed alias, phase flag unchanged |
| `0x004749FA` | `0x00473DFA` | `14 20 00 00` | `14 40 00 00` | writer flag-array offset |
| `0x0046A5AA` | `0x004699AA` | `00 28 00 00` | `00 50 00 00` | consumer queue flag / count offset |
| `0x0046A5E8` | `0x004699E8` | `08 28 00 00` | `08 50 00 00` | consumer queue flag / count offset |
| `0x0046A5EE` | `0x004699EE` | `0C 28 00 00` | `0C 50 00 00` | consumer queue flag / count offset |
| `0x0046A5F4` | `0x004699F4` | `04 28 00 00` | `04 50 00 00` | consumer queue flag / count offset |
| `0x0046A609` | `0x00469A09` | `08 28 00 00` | `08 50 00 00` | consumer queue flag / count offset |
| `0x0046A60F` | `0x00469A0F` | `00 28 00 00` | `00 50 00 00` | consumer queue flag / count offset |
| `0x0046A66A` | `0x00469A6A` | `00 20 00 00` | `00 40 00 00` | consumer queue flag / count offset |
| `0x0046A6E4` | `0x00469AE4` | `00 28 00 00` | `00 50 00 00` | consumer queue flag / count offset |
| `0x0046A6ED` | `0x00469AED` | `08 28 00 00` | `08 50 00 00` | consumer queue flag / count offset |
| `0x0046A716` | `0x00469B16` | `00 28 00 00` | `00 50 00 00` | consumer queue flag / count offset |
| `0x0046A7E2` | `0x00469BE2` | `00 28 00 00` | `00 50 00 00` | consumer queue flag / count offset |
| `0x0046A7E8` | `0x00469BE8` | `08 28 00 00` | `08 50 00 00` | consumer queue flag / count offset |
| `0x0046A7EE` | `0x00469BEE` | `04 28 00 00` | `04 50 00 00` | consumer queue flag / count offset |
| `0x0046A7F4` | `0x00469BF4` | `0C 28 00 00` | `0C 50 00 00` | consumer queue flag / count offset |
| `0x0046BE03` | `0x0046B203` | `00 20 00 00` | `00 40 00 00` | primary and companion clear length |
| `0x0046C209` | `0x0046B609` | `00 20 00 00` | `00 40 00 00` | primary and companion clear length |
