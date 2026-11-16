# Complete original-file overwrite list

Starting file: the exact stock DLL identified in `GUIDE.md`. Offsets are absolute file offsets in hexadecimal. Overwrite each row with its V16 bytes; do not insert bytes inside the existing file. Then append `APPEND_AT_ORIGINAL_EOF.bin` as described in the guide.

| # | File offset | Bytes | Stock bytes | V16 bytes | Change |
|---:|---|---:|---|---|---|
| 1 | `0x0000016E` | 1 | `07` | `0A` | Final binary layout |
| 2 | `0x00000185` | 11 | `CE 91 00 00 CC 37 00 00 00 00 00` | `9A A0 00 00 CC 37 00 80 E0 0C 16` | PE zero-filled byte accounting |
| 3 | `0x000001B9` | 10 | `A0 ED 04 00 04 00 00 A8 22 CA` | `20 0A 1B 00 04 00 00 25 72 D9` | PE virtual image size; PE checksum |
| 4 | `0x00000209` | 13 | `40 E5 04 E0 50 05 00 00 9E C9 00 D8 3D` | `30 A9 0A B8 51 05 00 00 00 00 00 00 00` | exception directory size |
| 5 | `0x00000388` | 23 | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` | `2E 6F 62 6A 70 6F 6F 6C 80 60 B7 05 00 A0 ED 04 00 5A 05 00 00 DC C9` | Final binary layout |
| 6 | `0x000003AC` | 27 | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` | `60 00 00 E0 2E 64 65 6C 74 72 61 63 00 72 09 00 00 10 A5 0A 00 72 09 00 00 36 CF` | Final binary layout |
| 7 | `0x000003D4` | 20 | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` | `60 00 00 E0 2E 68 6B 77 6F 72 6B 00 00 90 5B 10 00 90 AE 0A` | snapshot zero-filled virtual backing |
| 8 | `0x000003FC` | 4 | `00 00 00 00` | `80 00 00 C0` | Final binary layout |
| 9 | `0x00002BF2` | 11 | `8D 39 49 02 00 C6 05 82 39 49 02` | `6D D8 09 1B 00 C6 05 62 D8 09 1B` | snapshot backing reference |
| 10 | `0x000038AD` | 8 | `17 4E 87 02 41 B8 00 01` | `57 DB C9 12 41 B8 00 10` | relocated physics collection reference; physics collection bitmap reset length |
| 11 | `0x000038BE` | 11 | `05 6F 87 02 00 83 25 02 8F 87 02` | `45 EB CB 12 00 83 25 42 EB CD 12` | relocated physics collection reference |
| 12 | `0x0000A909` | 2 | `C0 08` | `00 29` | scheduler owner array stride / constructor size |
| 13 | `0x0000A921` | 4 | `AB 3E D6 02` | `DB 9A ED 14` | relocated global scheduler owner |
| 14 | `0x0000B7DB` | 4 | `81 AD 48 02` | `61 4C 09 1B` | snapshot backing reference |
| 15 | `0x0000B7EB` | 4 | `81 AD 48 02` | `61 4C 09 1B` | snapshot backing reference |
| 16 | `0x0000B7F8` | 10 | `24 AD FC 00 41 B9 20 00 A6 00` | `04 4C 17 15 41 B9 20 00 F9 02` | snapshot backing reference; game-state and checkpoint size |
| 17 | `0x0000B80F` | 4 | `6D AD 48 02` | `4D 4C 09 1B` | snapshot backing reference |
| 18 | `0x0000B81C` | 4 | `64 AD 48 02` | `44 4C 09 1B` | snapshot backing reference |
| 19 | `0x0000B82A` | 4 | `12 AD A2 01` | `F2 4B 10 18` | snapshot backing reference |
| 20 | `0x0000B847` | 4 | `25 AD 48 02` | `05 4C 09 1B` | snapshot backing reference |
| 21 | `0x0000B875` | 2 | `38 01` | `F3 07` | game-state and checkpoint size |
| 22 | `0x0000B89E` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 23 | `0x0000B8CF` | 2 | `4B 00` | `00 01` | permanent arena reservation |
| 24 | `0x0000B998` | 4 | `34 2E D6 02` | `64 8A ED 14` | relocated global scheduler owner |
| 25 | `0x0000B9BA` | 2 | `C0 08` | `00 29` | scheduler owner array stride / constructor size |
| 26 | `0x0000B9D3` | 2 | `C0 08` | `00 29` | scheduler owner array stride / constructor size |
| 27 | `0x0000BA64` | 2 | `C0 08` | `00 29` | scheduler owner array stride / constructor size |
| 28 | `0x0000BC07` | 10 | `5C 03 00 48 8D 0D 2F 54 C4 00` | `00 10 00 48 8D 0D EF C7 AD 12` | CPU packet reset/allocation limit; per-frame packet backing pointer |
| 29 | `0x0000BEB8` | 10 | `C0 08 00 4C 8D 3D 0E 29 D6 02` | `00 29 00 4C 8D 3D 3E 85 ED 14` | scheduler owner array stride / constructor size; relocated global scheduler owner |
| 30 | `0x0000BEEB` | 2 | `C0 08` | `00 29` | external scheduler layout access |
| 31 | `0x0000BF0A` | 1 | `08` | `26` | shutdown free-job queue pointer |
| 32 | `0x0000BF1B` | 2 | `C0 08` | `00 29` | scheduler owner array stride / constructor size |
| 33 | `0x0000C124` | 4 | `38 A4 48 02` | `18 43 09 1B` | snapshot backing reference |
| 34 | `0x0000C144` | 4 | `28 A4 48 02` | `08 43 09 1B` | snapshot backing reference |
| 35 | `0x0000C159` | 4 | `23 A4 48 02` | `03 43 09 1B` | snapshot backing reference |
| 36 | `0x0000C2A5` | 4 | `B7 A2 48 02` | `97 41 09 1B` | snapshot backing reference |
| 37 | `0x0000C2DD` | 4 | `8F A2 48 02` | `6F 41 09 1B` | snapshot backing reference |
| 38 | `0x0000C2EA` | 4 | `72 A2 48 02` | `52 41 09 1B` | snapshot backing reference |
| 39 | `0x0000C30C` | 4 | `60 A2 48 02` | `40 41 09 1B` | snapshot backing reference |
| 40 | `0x0000C357` | 4 | `15 A2 48 02` | `F5 40 09 1B` | snapshot backing reference |
| 41 | `0x0000C371` | 4 | `EB A1 48 02` | `CB 40 09 1B` | snapshot backing reference |
| 42 | `0x0000C392` | 4 | `DA A1 48 02` | `BA 40 09 1B` | snapshot backing reference |
| 43 | `0x0000C3CD` | 4 | `8F A1 48 02` | `6F 40 09 1B` | snapshot backing reference |
| 44 | `0x000153B9` | 2 | `38 01` | `F3 07` | game-state and checkpoint size |
| 45 | `0x000154BD` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 46 | `0x00015531` | 2 | `38 01` | `F3 07` | game-state and checkpoint size |
| 47 | `0x0001559C` | 1 | `37` | `56` | render bank reservation |
| 48 | `0x000155AB` | 2 | `49 00` | `7D 02` | main and two rollback bank reservations |
| 49 | `0x00015F20` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 50 | `0x0001627F` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 51 | `0x0001656A` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 52 | `0x000165C9` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 53 | `0x000165D1` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 54 | `0x000165ED` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 55 | `0x000169BC` | 2 | `4B 00` | `00 01` | permanent arena allocation boundary |
| 56 | `0x00016F82` | 2 | `40 07` | `00 23` | external scheduler layout access |
| 57 | `0x000349A1` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 58 | `0x00056C65` | 4 | `FB 5F BB 00` | `B7 E7 11 15` | relocated update queue global |
| 59 | `0x00056CD0` | 4 | `90 5F BB 00` | `4C E7 11 15` | relocated update queue global |
| 60 | `0x00056D07` | 2 | `C0 00` | `00 03` | external scheduler layout access |
| 61 | `0x0008764B` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 62 | `0x00087B68` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 63 | `0x00087E5C` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 64 | `0x0009862A` | 2 | `DC 05` | `E0 2E` | Havok component capacity; original alignment retained |
| 65 | `0x00098675` | 2 | `00 04` | `FF 7F` | Havok contact capacity |
| 66 | `0x0009A204` | 2 | `A4 06` | `E0 2E` | world entity insertion reclamation threshold |
| 67 | `0x0009B7C0` | 2 | `A0 06` | `DC 2E` | world entity reclamation low-water threshold |
| 68 | `0x0009B7D4` | 2 | `A0 06` | `DC 2E` | world entity reclamation repeated threshold |
| 69 | `0x000A3330` | 18 | `8B 53 7D 02 00 48 8D 0D 8C 53 7D 02 83 25 81 53 7D 02` | `CB E0 BF 12 00 48 8D 0D CC E0 BF 12 83 25 C1 E0 BF 12` | relocated physics collection reference |
| 70 | `0x000A3348` | 1 | `01` | `10` | physics collection bitmap reset length |
| 71 | `0x000A3352` | 13 | `71 74 7D 02 00 B0 01 83 25 6C 94 7D 02` | `B1 F0 C1 12 00 B0 01 83 25 AC F0 C3 12` | relocated physics collection reference |
| 72 | `0x000A47E9` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 73 | `0x000CEB6C` | 3 | `10 27 00` | `00 00 04` | physics phase allocator block-size override |
| 74 | `0x000CF120` | 1 | `21` | `30` | physics iterator caller stack reservation |
| 75 | `0x000CF13A` | 1 | `21` | `30` | physics iterator caller cookie |
| 76 | `0x000CF141` | 1 | `01` | `10` | physics iterator traversal frame address |
| 77 | `0x000CF150` | 1 | `01` | `10` | physics iterator visited bitmap clear |
| 78 | `0x000CF174` | 1 | `01` | `10` | physics iterator traversal frame address |
| 79 | `0x000CF19F` | 1 | `01` | `10` | physics iterator traversal frame address |
| 80 | `0x000CF1B1` | 1 | `21` | `30` | physics iterator caller cookie |
| 81 | `0x000CF1C0` | 1 | `21` | `30` | physics iterator stack restoration |
| 82 | `0x000CF1E3` | 13 | `DD 94 7A 02 48 8B F2 8B 3D D8 B5 7A 02` | `1D 22 BD 12 48 8B F2 8B 3D 18 32 BF 12` | relocated physics collection reference |
| 83 | `0x000CF20E` | 4 | `AE 94 7A 02` | `EE 21 BD 12` | relocated physics collection reference |
| 84 | `0x000CF232` | 1 | `01` | `10` | physics collected object-array relative access |
| 85 | `0x000CF252` | 1 | `01` | `10` | physics collected object-array relative access |
| 86 | `0x000CF25D` | 4 | `63 94 7A 02` | `A3 21 BD 12` | relocated physics collection reference |
| 87 | `0x000CF267` | 4 | `59 94 7A 02` | `99 21 BD 12` | relocated physics collection reference |
| 88 | `0x000CF2B3` | 13 | `0D 94 7A 02 48 8B F2 8B 3D 08 B5 7A 02` | `4D 21 BD 12 48 8B F2 8B 3D 48 31 BF 12` | relocated physics collection reference |
| 89 | `0x000CF2DE` | 4 | `DE 93 7A 02` | `1E 21 BD 12` | relocated physics collection reference |
| 90 | `0x000CF302` | 1 | `01` | `10` | physics collected object-array relative access |
| 91 | `0x000CF322` | 1 | `01` | `10` | physics collected object-array relative access |
| 92 | `0x000CF32D` | 4 | `93 93 7A 02` | `D3 20 BD 12` | relocated physics collection reference |
| 93 | `0x000CF337` | 4 | `89 93 7A 02` | `C9 20 BD 12` | relocated physics collection reference |
| 94 | `0x000CF384` | 4 | `38 93 7A 02` | `78 20 BD 12` | relocated physics collection reference |
| 95 | `0x000CF3E4` | 19 | `E0 B3 7A 02 8D 41 01 89 05 D7 B3 7A 02 44 89 94 8E 08 01` | `20 30 BF 12 8D 41 01 89 05 17 30 BF 12 44 89 94 8E 08 10` | relocated physics collection reference; physics collected object-array relative access |
| 96 | `0x000CF3FE` | 19 | `CA D3 7A 02 8D 41 01 89 05 C1 D3 7A 02 89 94 8E 0C 21 00` | `0A 30 C1 12 8D 41 01 89 05 01 30 C1 12 89 94 8E 0C 10 02` | relocated physics collection reference; physics collected tag-array relative access |
| 97 | `0x000CF419` | 4 | `A3 92 7A 02` | `E3 1F BD 12` | relocated physics collection reference |
| 98 | `0x000CF6FF` | 17 | `BD 8F 7A 02 41 B8 00 01 00 00 48 8D 0D B8 8F 7A 02` | `FD 1C BD 12 41 B8 00 10 00 00 48 8D 0D F8 1C BD 12` | relocated physics collection reference; physics collection bitmap reset length |
| 99 | `0x000CF71B` | 11 | `A9 B0 7A 02 44 89 15 A6 D0 7A 02` | `E9 2C BF 12 44 89 15 E6 2C C1 12` | relocated physics collection reference |
| 100 | `0x000CF8AD` | 4 | `13 8E 7A 02` | `53 1B BD 12` | relocated physics collection reference |
| 101 | `0x000CFE2F` | 4 | `95 A9 7A 02` | `D5 25 BF 12` | relocated physics collection reference |
| 102 | `0x000CFE3A` | 4 | `82 88 7A 02` | `C2 15 BD 12` | relocated physics collection reference |
| 103 | `0x000D00A9` | 4 | `1F C7 7A 02` | `5F 23 C1 12` | relocated physics collection reference |
| 104 | `0x000D00BD` | 4 | `CC B3 87 02` | `0C 30 CC 12` | relocated physics tag-array indexed reader |
| 105 | `0x000D0227` | 4 | `9D A5 7A 02` | `DD 21 BF 12` | relocated physics collection reference |
| 106 | `0x000D0232` | 4 | `8A 84 7A 02` | `CA 11 BD 12` | relocated physics collection reference |
| 107 | `0x000D3285` | 10 | `01 00 00 48 8B 05 81 61 A2 00` | `10 00 00 E9 F3 D4 97 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 108 | `0x000D3297` | 1 | `01` | `10` | spatial frame high field / restoration |
| 109 | `0x000D32D4` | 1 | `01` | `10` | spatial bitmap clear length |
| 110 | `0x000D3390` | 1 | `01` | `10` | spatial frame high field / restoration |
| 111 | `0x000D339F` | 1 | `01` | `10` | spatial stack restoration |
| 112 | `0x00118379` | 4 | `48 08 92 02` | `98 90 BF 12` | visible list/mesh module-indexed reference |
| 113 | `0x0011838F` | 4 | `7C 08 92 02` | `CC 90 BF 12` | visible list/mesh module-indexed reference |
| 114 | `0x0011839A` | 4 | `48 08 92 02` | `98 90 BF 12` | visible list/mesh module-indexed reference |
| 115 | `0x001183B1` | 4 | `7C 08 92 02` | `CC 90 BF 12` | visible list/mesh module-indexed reference |
| 116 | `0x0012790D` | 1 | `FE` | `EF` | retain low spatial frame RBP anchor |
| 117 | `0x00127914` | 10 | `02 00 00 48 8B 05 F2 1A 9D 00` | `11 00 00 E9 84 8E 92 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 118 | `0x00127925` | 1 | `01` | `10` | spatial RBP high field |
| 119 | `0x0012792C` | 1 | `01` | `10` | spatial RBP high field |
| 120 | `0x00127955` | 1 | `01` | `10` | spatial bitmap clear length |
| 121 | `0x00127C0E` | 1 | `01` | `10` | spatial RBP high field |
| 122 | `0x00127C1E` | 1 | `02` | `11` | spatial frame high field / restoration |
| 123 | `0x00127C25` | 1 | `02` | `11` | spatial stack restoration |
| 124 | `0x00128AB1` | 6 | `97 FE FF B8 10 69` | `88 FE FF B8 10 78` | retain low spatial frame RBP anchor; spatial bitmap stack reservation |
| 125 | `0x00128AC6` | 1 | `69` | `78` | spatial frame high field / restoration |
| 126 | `0x00128ACE` | 1 | `68` | `77` | spatial frame high field / restoration |
| 127 | `0x00128AD7` | 1 | `68` | `77` | spatial frame high field / restoration |
| 128 | `0x00128AE0` | 1 | `68` | `77` | spatial frame high field / restoration |
| 129 | `0x00128AE9` | 1 | `68` | `77` | spatial frame high field / restoration |
| 130 | `0x00128AF2` | 1 | `68` | `77` | spatial frame high field / restoration |
| 131 | `0x00128AFB` | 1 | `68` | `77` | spatial frame high field / restoration |
| 132 | `0x00128B04` | 1 | `68` | `77` | spatial frame high field / restoration |
| 133 | `0x00128B15` | 1 | `67` | `76` | spatial RBP high field |
| 134 | `0x00128B1C` | 1 | `68` | `77` | spatial RBP high field |
| 135 | `0x00128E33` | 1 | `68` | `77` | spatial RBP high field |
| 136 | `0x00128E56` | 1 | `68` | `77` | spatial RBP high field |
| 137 | `0x00128E66` | 1 | `68` | `77` | spatial RBP high field |
| 138 | `0x00128F2B` | 1 | `04` | `13` | spatial RBP high field |
| 139 | `0x00128FD9` | 1 | `01` | `10` | spatial RBP high field |
| 140 | `0x0012900F` | 1 | `01` | `10` | spatial RBP high field |
| 141 | `0x0012908C` | 1 | `01` | `10` | spatial RBP high field |
| 142 | `0x001290A2` | 1 | `02` | `11` | spatial RBP high field |
| 143 | `0x001290E7` | 1 | `02` | `11` | spatial RBP high field |
| 144 | `0x00129163` | 1 | `02` | `11` | spatial RBP high field |
| 145 | `0x001291AB` | 1 | `04` | `13` | spatial RBP high field |
| 146 | `0x001291C3` | 1 | `04` | `13` | spatial RBP high field |
| 147 | `0x00129238` | 1 | `06` | `15` | spatial RBP high field |
| 148 | `0x00129248` | 1 | `06` | `15` | spatial RBP high field |
| 149 | `0x0012925E` | 1 | `06` | `15` | spatial RBP high field |
| 150 | `0x0012928A` | 1 | `06` | `15` | spatial RBP high field |
| 151 | `0x001292B3` | 1 | `06` | `15` | spatial RBP high field |
| 152 | `0x001292E3` | 1 | `06` | `15` | spatial RBP high field |
| 153 | `0x0012933E` | 1 | `01` | `10` | spatial bitmap clear length |
| 154 | `0x001293AC` | 1 | `04` | `13` | spatial RBP high field |
| 155 | `0x001294C2` | 1 | `67` | `76` | spatial RBP high field |
| 156 | `0x001294D2` | 1 | `69` | `78` | spatial frame high field / restoration |
| 157 | `0x001295E1` | 6 | `E6 FF FF B8 E0 1A` | `D7 FF FF B8 E0 29` | retain low spatial frame RBP anchor; spatial bitmap stack reservation |
| 158 | `0x001295F6` | 1 | `1A` | `29` | spatial frame high field / restoration |
| 159 | `0x001295FE` | 1 | `1A` | `29` | spatial frame high field / restoration |
| 160 | `0x0012960F` | 1 | `19` | `28` | spatial RBP high field |
| 161 | `0x00129616` | 1 | `1A` | `29` | spatial RBP high field |
| 162 | `0x00129620` | 1 | `1A` | `29` | spatial RBP high field |
| 163 | `0x001296B8` | 1 | `01` | `10` | spatial bitmap clear length |
| 164 | `0x001297A0` | 1 | `1A` | `29` | spatial RBP high field |
| 165 | `0x001297AC` | 1 | `1A` | `29` | spatial RBP high field |
| 166 | `0x001297B5` | 1 | `1A` | `29` | spatial RBP high field |
| 167 | `0x0012994F` | 1 | `17` | `26` | spatial RBP high field |
| 168 | `0x0012998A` | 1 | `17` | `26` | spatial RBP high field |
| 169 | `0x001299A0` | 1 | `18` | `27` | spatial RBP high field |
| 170 | `0x001299DB` | 1 | `18` | `27` | spatial RBP high field |
| 171 | `0x00129CC3` | 1 | `1A` | `29` | spatial RBP high field |
| 172 | `0x00129CDB` | 1 | `1A` | `29` | spatial RBP high field |
| 173 | `0x00129CE8` | 1 | `1A` | `29` | spatial RBP high field |
| 174 | `0x00129CFE` | 1 | `19` | `28` | spatial RBP high field |
| 175 | `0x00129D0E` | 1 | `1A` | `29` | spatial frame high field / restoration |
| 176 | `0x0012A7DE` | 6 | `DF FF FF B8 F8 21` | `D0 FF FF B8 F8 30` | retain low spatial frame RBP anchor; spatial bitmap stack reservation |
| 177 | `0x0012A7F3` | 1 | `21` | `30` | spatial frame high field / restoration |
| 178 | `0x0012A7FB` | 1 | `21` | `30` | spatial frame high field / restoration |
| 179 | `0x0012A804` | 1 | `21` | `30` | spatial frame high field / restoration |
| 180 | `0x0012A815` | 1 | `20` | `2F` | spatial RBP high field |
| 181 | `0x0012A81C` | 1 | `21` | `30` | spatial RBP high field |
| 182 | `0x0012A831` | 1 | `01` | `10` | spatial bitmap clear length |
| 183 | `0x0012A85E` | 1 | `21` | `30` | spatial RBP high field |
| 184 | `0x0012A9B5` | 1 | `00` | `0F` | spatial RBP high field |
| 185 | `0x0012A9F5` | 1 | `00` | `0F` | spatial RBP high field |
| 186 | `0x0012AA31` | 1 | `18` | `27` | spatial RBP high field |
| 187 | `0x0012AA50` | 1 | `18` | `27` | spatial RBP high field |
| 188 | `0x0012AA66` | 1 | `18` | `27` | spatial RBP high field |
| 189 | `0x0012AAE4` | 1 | `21` | `30` | spatial RBP high field |
| 190 | `0x0012AB0E` | 1 | `18` | `27` | spatial RBP high field |
| 191 | `0x0012ABBB` | 1 | `20` | `2F` | spatial RBP high field |
| 192 | `0x0012ABCB` | 1 | `21` | `30` | spatial frame high field / restoration |
| 193 | `0x0014DEB8` | 10 | `04 00 00 48 8D 05 08 DD 7C 02` | `10 00 00 48 8D 05 58 A5 A9 12` | visible list and mesh constructor capacity; visible list/mesh RIP reference |
| 194 | `0x0014DEC8` | 25 | `F4 DC 7C 02 66 89 2D E9 DC 7C 02 48 8D 1D F2 DC 7C 02 44 89 35 E7 DC 7C 02` | `44 A5 A9 12 66 89 2D 39 A5 A9 12 48 8D 1D 42 A5 A9 12 44 89 35 37 A5 A9 12` | visible list/mesh RIP reference |
| 195 | `0x0014DEF9` | 32 | `ED 1C 7D 02 66 89 2D D8 1C 7D 02 48 89 05 D5 1C 7D 02 48 8D 1D DE 1C 7D 02 44 89 35 CF 1C 7D 02` | `3D A5 AA 12 66 89 2D 28 A5 AA 12 48 89 05 25 A5 AA 12 48 8D 1D 2E A5 AA 12 44 89 35 1F A5 AA 12` | visible list/mesh RIP reference |
| 196 | `0x0014DF31` | 7 | `48 8D 05 D2 BC 7F 02` | `E9 CA 2C 90 0A 90 90` | constructor capacity restore hook |
| 197 | `0x0014E098` | 11 | `2E DB 7C 02 66 89 05 47 1B 7D 02` | `7E A3 A9 12 66 89 05 97 A3 AA 12` | visible list/mesh RIP reference |
| 198 | `0x0014E1C4` | 4 | `14 1A 7D 02` | `64 A2 AA 12` | visible list/mesh RIP reference |
| 199 | `0x0014E1CF` | 4 | `09 1A 7D 02` | `59 A2 AA 12` | visible list/mesh RIP reference |
| 200 | `0x0014E1DA` | 4 | `02 1A 7D 02` | `52 A2 AA 12` | visible list/mesh RIP reference |
| 201 | `0x0014E1E6` | 11 | `D0 07 92 02 0F B7 05 CB D9 7C 02` | `20 90 BF 12 0F B7 05 1B A2 A9 12` | visible list/mesh module-indexed reference; visible list/mesh RIP reference |
| 202 | `0x0014E1F8` | 4 | `C0 D9 7C 02` | `10 A2 A9 12` | visible list/mesh RIP reference |
| 203 | `0x0014E203` | 4 | `B9 D9 7C 02` | `09 A2 A9 12` | visible list/mesh RIP reference |
| 204 | `0x0014E20F` | 4 | `B0 C7 91 02` | `00 90 BE 12` | visible list/mesh module-indexed reference |
| 205 | `0x0014E3D6` | 20 | `02 18 7D 02 41 0F B7 8C 41 D0 07 92 02 48 8B 05 F6 17 7D 02` | `52 A0 AA 12 41 0F B7 8C 41 20 90 BF 12 48 8B 05 46 A0 AA 12` | visible list/mesh RIP reference; visible list/mesh module-indexed reference |
| 206 | `0x0014E3F0` | 28 | `E8 17 7D 02 48 0F BF 05 C0 D7 7C 02 41 0F B7 8C 41 B0 C7 91 02 48 8B 05 B4 D7 7C 02` | `38 A0 AA 12 48 0F BF 05 10 A0 A9 12 41 0F B7 8C 41 00 90 BE 12 48 8B 05 04 A0 A9 12` | visible list/mesh RIP reference; visible list/mesh module-indexed reference |
| 207 | `0x0014E412` | 4 | `A6 D7 7C 02` | `F6 9F A9 12` | visible list/mesh RIP reference |
| 208 | `0x0014F433` | 2 | `00 1C` | `FF 7F` | collision element capacity; 48-byte format unchanged |
| 209 | `0x00150933` | 1 | `44` | `48` | registration guard uses occupancy so freed slots are reusable |
| 210 | `0x00167802` | 1 | `F9` | `EA` | retain low spatial frame RBP anchor |
| 211 | `0x00167809` | 1 | `07` | `16` | spatial bitmap stack reservation |
| 212 | `0x00167846` | 7 | `48 8B 05 C3 1B 99 00` | `E9 75 8F 8E 0A 90 90` | spatial stack page-probe hook |
| 213 | `0x00167854` | 1 | `06` | `15` | spatial RBP high field |
| 214 | `0x0016793E` | 1 | `03` | `12` | spatial RBP high field |
| 215 | `0x00167A24` | 1 | `01` | `10` | spatial bitmap clear length |
| 216 | `0x00167A75` | 1 | `03` | `12` | spatial RBP high field |
| 217 | `0x00168296` | 1 | `03` | `12` | spatial RBP high field |
| 218 | `0x001682D2` | 1 | `03` | `12` | spatial RBP high field |
| 219 | `0x001682DE` | 1 | `03` | `12` | spatial RBP high field |
| 220 | `0x00168307` | 1 | `03` | `12` | spatial RBP high field |
| 221 | `0x00168514` | 1 | `06` | `15` | spatial RBP high field |
| 222 | `0x00168524` | 1 | `07` | `16` | spatial frame high field / restoration |
| 223 | `0x001707DD` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 224 | `0x00170ACB` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 225 | `0x00170D3B` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 226 | `0x0017104A` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 227 | `0x001715B6` | 2 | `A6 00` | `F9 02` | game-state and checkpoint size |
| 228 | `0x001CAC6B` | 4 | `41 38 2A 00` | `91 5A 88 0A` | observe physics reclamation without changing deletion |
| 229 | `0x001E5ECD` | 2 | `38 00` | `00 04` | initial Havok allocator block size |
| 230 | `0x001E5F17` | 4 | `99 99 2F 00` | `66 66 66 03` | Havok free-list soft memory limit; retain 85 percent margin |
| 231 | `0x001E607D` | 10 | `40 06 00 48 8D 15 89 D2 B8 04` | `00 00 01 48 8D 15 79 E3 CF 12` | primary Havok solver buffer capacity; Havok solver arena backing / end |
| 232 | `0x001E612E` | 4 | `DE 11 BF 04` | `CE E2 CF 13` | Havok solver arena backing / end |
| 233 | `0x001E6142` | 4 | `CA 31 C2 04` | `BA E2 CF 14` | Havok solver arena backing / end |
| 234 | `0x001E63A2` | 4 | `6A BF 91 04` | `5A 20 90 0E` | secondary Havok working arena |
| 235 | `0x001E63AF` | 12 | `5D BF 59 04 48 C7 40 28 00 10 27 00` | `4D 20 90 0A 48 C7 40 28 00 00 00 04` | primary Havok working arena; first backing callback subsequent block size |
| 236 | `0x001E63E0` | 2 | `38 00` | `00 04` | backing free callback reset block size |
| 237 | `0x001E8FA5` | 6 | `BA FF FF B8 B0 46` | `AB FF FF B8 B0 55` | retain low spatial frame RBP anchor; spatial bitmap stack reservation |
| 238 | `0x001E8FBA` | 1 | `46` | `55` | spatial frame high field / restoration |
| 239 | `0x001E8FC2` | 1 | `46` | `55` | spatial frame high field / restoration |
| 240 | `0x001E8FCB` | 1 | `46` | `55` | spatial frame high field / restoration |
| 241 | `0x001E8FDC` | 1 | `45` | `54` | spatial RBP high field |
| 242 | `0x001E8FE3` | 1 | `46` | `55` | spatial RBP high field |
| 243 | `0x001E8FED` | 1 | `46` | `55` | spatial RBP high field |
| 244 | `0x001E9023` | 1 | `46` | `55` | spatial RBP high field |
| 245 | `0x001E9109` | 1 | `46` | `55` | spatial RBP high field |
| 246 | `0x001E91B8` | 1 | `46` | `55` | spatial RBP high field |
| 247 | `0x001E91DF` | 1 | `46` | `55` | spatial RBP high field |
| 248 | `0x001E923A` | 1 | `01` | `10` | spatial bitmap clear length |
| 249 | `0x001E92B7` | 1 | `05` | `14` | spatial RBP high field |
| 250 | `0x001E92E8` | 1 | `05` | `14` | spatial RBP high field |
| 251 | `0x001E931A` | 1 | `45` | `54` | spatial RBP high field |
| 252 | `0x001E932A` | 1 | `46` | `55` | spatial frame high field / restoration |
| 253 | `0x0022666C` | 7 | `DD FF FF B8 F0 22 00` | `DF FD FF B8 F0 20 02` | primary visibility frame base; primary visibility stack reservation |
| 254 | `0x0022668A` | 2 | `21 00` | `1F 02` | primary cookie storage/load |
| 255 | `0x002266D1` | 7 | `00 00 00 41 B8 00 01` | `0F 00 00 41 B8 00 10` | primary second bitmap initialization address; primary object bitmap clearing size |
| 256 | `0x002266F2` | 1 | `00` | `0F` | primary second scratch structure address |
| 257 | `0x00226741` | 1 | `01` | `10` | primary object bitmap clearing size |
| 258 | `0x002267CB` | 1 | `00` | `0F` | primary second scratch structure address |
| 259 | `0x0022690A` | 2 | `21 00` | `1F 02` | primary cookie storage/load |
| 260 | `0x0022691A` | 2 | `22 00` | `20 02` | primary visibility epilogue stack restoration |
| 261 | `0x0022693C` | 2 | `21 00` | `10 02` | secondary visibility stack reservation |
| 262 | `0x00226956` | 2 | `21 00` | `10 02` | secondary cookie storage/load |
| 263 | `0x0022696E` | 1 | `01` | `10` | secondary object bitmap clearing size |
| 264 | `0x002269B8` | 2 | `21 00` | `10 02` | secondary cookie storage/load |
| 265 | `0x002269C7` | 2 | `21 00` | `10 02` | secondary stack restoration |
| 266 | `0x00226BB2` | 4 | `16 50 6F 02` | `66 18 9C 12` | visible list/mesh RIP reference |
| 267 | `0x00226C6D` | 10 | `01 00 00 48 8D 05 55 4F 6F 02` | `10 00 00 48 8D 05 A5 17 9C 12` | shared scratch index-table reader; visible list/mesh RIP reference |
| 268 | `0x00227023` | 4 | `A3 4B 6F 02` | `F3 13 9C 12` | visible list/mesh RIP reference |
| 269 | `0x0022702E` | 4 | `96 4B 6F 02` | `E6 13 9C 12` | visible list/mesh RIP reference |
| 270 | `0x00227044` | 4 | `82 4B 6F 02` | `D2 13 9C 12` | visible list/mesh RIP reference |
| 271 | `0x00227061` | 4 | `67 4B 6F 02` | `B7 13 9C 12` | visible list/mesh RIP reference |
| 272 | `0x00227104` | 1 | `01` | `10` | shared scratch index-table writer |
| 273 | `0x0022710C` | 4 | `BA 4A 6F 02` | `0A 13 9C 12` | visible list/mesh RIP reference |
| 274 | `0x0022711B` | 4 | `AB 4A 6F 02` | `FB 12 9C 12` | visible list/mesh RIP reference |
| 275 | `0x002272B8` | 11 | `2E 89 6F 02 66 3B 05 25 89 6F 02` | `7E 11 9D 12 66 3B 05 75 11 9D 12` | visible list/mesh RIP reference |
| 276 | `0x002272CE` | 4 | `18 89 6F 02` | `68 11 9D 12` | visible list/mesh RIP reference |
| 277 | `0x00227402` | 4 | `EA 87 6F 02` | `3A 10 9D 12` | visible list/mesh RIP reference |
| 278 | `0x0024A43A` | 8 | `02 6C A0 00 25 FF FF 03` | `C2 DF 89 12 25 FF FF 0F` | per-frame packet backing pointer; CPU packet offset decoder |
| 279 | `0x0024A53F` | 9 | `FD 6A A0 00 81 E3 FF FF 03` | `BD DE 89 12 81 E3 FF FF 0F` | per-frame packet backing pointer; CPU packet offset decoder |
| 280 | `0x0024FD74` | 2 | `5C 03` | `00 10` | CPU packet reset/allocation limit |
| 281 | `0x0024FD9A` | 1 | `12` | `14` | CPU packet generation shift |
| 282 | `0x0025596F` | 1 | `06` | `18` | render-state auxiliary heap allocation |
| 283 | `0x00255993` | 1 | `06` | `18` | render-state auxiliary heap registration |
| 284 | `0x002559E7` | 1 | `04` | `10` | cached object render-state capacity |
| 285 | `0x00263E50` | 7 | `4C 8B DC 49 89 5B 08` | `E9 BB 5B C7 04 90 90` | redirect render-cache color update append |
| 286 | `0x0026565C` | 7 | `4C 8B DC 49 89 5B 08` | `E9 9F 45 C7 04 90 90` | redirect render-cache invalidate append |
| 287 | `0x0027DA2C` | 2 | `21 00` | `10 02` | third visibility stack reservation |
| 288 | `0x0027DA3C` | 2 | `21 00` | `10 02` | third frame saved SIMD location |
| 289 | `0x0027DA4E` | 2 | `21 00` | `10 02` | third cookie storage/load |
| 290 | `0x0027DA8F` | 1 | `01` | `10` | third object bitmap clearing size |
| 291 | `0x0027DAE6` | 2 | `21 00` | `10 02` | third cookie storage/load |
| 292 | `0x0027DAF6` | 2 | `21 00` | `10 02` | third stack restoration |
| 293 | `0x0027E035` | 4 | `91 DB 69 02` | `E1 A3 96 12` | visible list/mesh RIP reference |
| 294 | `0x0027E041` | 4 | `A5 1B 6A 02` | `F5 A3 97 12` | visible list/mesh RIP reference |
| 295 | `0x0027E04C` | 4 | `78 DB 69 02` | `C8 A3 96 12` | visible list/mesh RIP reference |
| 296 | `0x0027E074` | 4 | `52 DB 69 02` | `A2 A3 96 12` | visible list/mesh RIP reference |
| 297 | `0x0027E0A5` | 4 | `D4 C7 91 02` | `24 90 BE 12` | visible list/mesh module-indexed reference |
| 298 | `0x0027E0B1` | 12 | `D2 C7 91 02 43 C6 84 16 D5 C7 91 02` | `22 90 BE 12 43 C6 84 16 25 90 BE 12` | visible list/mesh module-indexed reference |
| 299 | `0x0027E0C3` | 4 | `CC C7 91 02` | `1C 90 BE 12` | visible list/mesh module-indexed reference |
| 300 | `0x0027E255` | 26 | `91 19 6A 02 66 3B 05 88 19 6A 02 7D 0F 0F BF C8 66 FF C0 66 89 05 7B 19 6A 02` | `E1 A1 97 12 66 3B 05 D8 A1 97 12 E9 9B 2A 7D 0A 90 90 90 66 89 05 CB A1 97 12` | visible list/mesh RIP reference; guard invalid mesh index before record writes |
| 301 | `0x0027E284` | 4 | `68 19 6A 02` | `B8 A1 97 12` | visible list/mesh RIP reference |
| 302 | `0x0027E5AE` | 8 | `8E 2A 9D 00 25 FF FF 03` | `4E 9E 86 12 25 FF FF 0F` | per-frame packet backing pointer; CPU packet offset decoder |
| 303 | `0x0027E748` | 13 | `CE C7 91 02 66 43 89 84 16 D0 C7 91 02` | `1E 90 BE 12 66 43 89 84 16 20 90 BE 12` | visible list/mesh module-indexed reference |
| 304 | `0x0027E76B` | 4 | `7B 14 6A 02` | `CB 9C 97 12` | visible list/mesh RIP reference |
| 305 | `0x0027E777` | 29 | `4F D4 69 02 EB 23 0F B7 0D 66 14 6A 02 0F B7 3D 3F D4 69 02 EB 13 0F B7 0D 56 14 6A 02` | `9F 9C 96 12 EB 23 0F B7 0D B6 9C 97 12 0F B7 3D 8F 9C 96 12 EB 13 0F B7 0D A6 9C 97 12` | visible list/mesh RIP reference |
| 306 | `0x0027E797` | 4 | `2F D4 69 02` | `7F 9C 96 12` | visible list/mesh RIP reference |
| 307 | `0x0027E7BC` | 4 | `0A D4 69 02` | `5A 9C 96 12` | visible list/mesh RIP reference |
| 308 | `0x0027E7E7` | 4 | `FF 13 6A 02` | `4F 9C 97 12` | visible list/mesh RIP reference |
| 309 | `0x0027F07B` | 4 | `5D 0B 6A 02` | `AD 93 97 12` | visible list/mesh RIP reference |
| 310 | `0x0027F088` | 4 | `5E 0B 6A 02` | `AE 93 97 12` | visible list/mesh RIP reference |
| 311 | `0x0027F09C` | 4 | `D0 07 92 02` | `20 90 BF 12` | visible list/mesh module-indexed reference |
| 312 | `0x0027F0B1` | 4 | `F0 07 92 02` | `40 90 BF 12` | visible list/mesh module-indexed reference |
| 313 | `0x0027F184` | 4 | `62 0A 6A 02` | `B2 92 97 12` | visible list/mesh RIP reference |
| 314 | `0x0027F1A3` | 4 | `49 0A 6A 02` | `99 92 97 12` | visible list/mesh RIP reference |
| 315 | `0x0027F249` | 4 | `A3 09 6A 02` | `F3 91 97 12` | visible list/mesh RIP reference |
| 316 | `0x0028AED6` | 4 | `F0 0C 69 02` | `40 D5 95 12` | visible list/mesh RIP reference |
| 317 | `0x0028AEF3` | 4 | `C5 0C 69 02` | `15 D5 95 12` | visible list/mesh RIP reference |
| 318 | `0x0028AF06` | 4 | `B0 C7 91 02` | `00 90 BE 12` | visible list/mesh module-indexed reference |
| 319 | `0x0028AF18` | 4 | `CC C7 91 02` | `1C 90 BE 12` | visible list/mesh module-indexed reference |
| 320 | `0x0028AF5D` | 4 | `7C 08 92 02` | `CC 90 BF 12` | visible list/mesh module-indexed reference |
| 321 | `0x0028AF6C` | 4 | `90 08 92 02` | `E0 90 BF 12` | visible list/mesh module-indexed reference |
| 322 | `0x0028B09F` | 4 | `7C 08 92 02` | `CC 90 BF 12` | visible list/mesh module-indexed reference |
| 323 | `0x0028B0CB` | 4 | `7C 08 92 02` | `CC 90 BF 12` | visible list/mesh module-indexed reference |
| 324 | `0x0028B134` | 4 | `92 0A 69 02` | `E2 D2 95 12` | visible list/mesh RIP reference |
| 325 | `0x0028B164` | 4 | `64 0A 69 02` | `B4 D2 95 12` | visible list/mesh RIP reference |
| 326 | `0x00297CFE` | 18 | `BA 3E 68 02 48 8D 2D A7 3E 68 02 48 8B 15 BA 3E 68 02` | `0A 07 95 12 48 8D 2D F7 06 95 12 48 8B 15 0A 07 95 12` | visible list/mesh RIP reference |
| 327 | `0x00297D4C` | 2 | `40 00` | `00 01` | mesh field relative to relocated list base |
| 328 | `0x00297D77` | 4 | `4F 3E 68 02` | `9F 06 95 12` | visible list/mesh RIP reference |
| 329 | `0x00297D89` | 4 | `3F 3E 68 02` | `8F 06 95 12` | visible list/mesh RIP reference |
| 330 | `0x00297F95` | 4 | `23 3C 68 02` | `73 04 95 12` | visible list/mesh RIP reference |
| 331 | `0x00297FA3` | 4 | `23 3C 68 02` | `73 04 95 12` | visible list/mesh RIP reference |
| 332 | `0x00297FB0` | 4 | `B0 C7 91 02` | `00 90 BE 12` | visible list/mesh module-indexed reference |
| 333 | `0x00297FC1` | 4 | `07 3C 68 02` | `57 04 95 12` | visible list/mesh RIP reference |
| 334 | `0x0029800D` | 4 | `44 08 92 02` | `94 90 BF 12` | visible list/mesh module-indexed reference |
| 335 | `0x00298020` | 11 | `80 08 92 02 4C 8D 35 15 90 9B 00` | `D0 90 BF 12 4C 8D 35 D5 03 85 12` | visible list/mesh module-indexed reference; per-frame packet backing pointer |
| 336 | `0x00298039` | 1 | `03` | `0F` | CPU packet offset decoder |
| 337 | `0x00298043` | 4 | `4B 08 92 02` | `9B 90 BF 12` | visible list/mesh module-indexed reference |
| 338 | `0x002980AB` | 4 | `90 08 92 02` | `E0 90 BF 12` | visible list/mesh module-indexed reference |
| 339 | `0x002980D8` | 1 | `03` | `0F` | CPU packet offset decoder |
| 340 | `0x002989C2` | 9 | `7A 86 9B 00 81 E2 FF FF 03` | `3A FA 84 12 81 E2 FF FF 0F` | per-frame packet backing pointer; CPU packet offset decoder |
| 341 | `0x00298BDD` | 18 | `DB 2F 68 02 48 8D 15 C8 2F 68 02 0F BF 0D DB 2F 68 02` | `2B F8 94 12 48 8D 15 18 F8 94 12 0F BF 0D 2B F8 94 12` | visible list/mesh RIP reference |
| 342 | `0x00298C8F` | 4 | `1D 2F 68 02` | `6D F7 94 12` | visible list/mesh RIP reference |
| 343 | `0x00298CBD` | 2 | `40 00` | `00 01` | mesh field relative to relocated list base |
| 344 | `0x00298CC8` | 2 | `40 00` | `00 01` | mesh field relative to relocated list base |
| 345 | `0x00298CDE` | 2 | `40 00` | `00 01` | mesh field relative to relocated list base |
| 346 | `0x00298DF9` | 4 | `CD 2D 68 02` | `1D F6 94 12` | visible list/mesh RIP reference |
| 347 | `0x00298E08` | 4 | `C0 2D 68 02` | `10 F6 94 12` | visible list/mesh RIP reference |
| 348 | `0x00299525` | 9 | `17 7B 9B 00 81 E2 FF FF 03` | `D7 EE 84 12 81 E2 FF FF 0F` | per-frame packet backing pointer; CPU packet offset decoder |
| 349 | `0x0029954E` | 9 | `EE 7A 9B 00 81 E2 FF FF 03` | `AE EE 84 12 81 E2 FF FF 0F` | per-frame packet backing pointer; CPU packet offset decoder |
| 350 | `0x002997AF` | 10 | `8D 78 9B 00 41 81 E1 FF FF 03` | `4D EC 84 12 41 81 E1 FF FF 0F` | per-frame packet backing pointer; CPU packet offset decoder |
| 351 | `0x00299A33` | 18 | `85 21 68 02 4C 8D 2D 72 21 68 02 4C 8B 0D 85 21 68 02` | `D5 E9 94 12 4C 8D 2D C2 E9 94 12 4C 8B 0D D5 E9 94 12` | visible list/mesh RIP reference |
| 352 | `0x00299A98` | 2 | `40 00` | `00 01` | mesh field relative to relocated list base |
| 353 | `0x00299AC6` | 4 | `00 21 68 02` | `50 E9 94 12` | visible list/mesh RIP reference |
| 354 | `0x00299AD9` | 4 | `EF 20 68 02` | `3F E9 94 12` | visible list/mesh RIP reference |
| 355 | `0x00299B6A` | 18 | `4E 20 68 02 4C 8D 15 3B 20 68 02 4C 8B 05 4E 20 68 02` | `9E E8 94 12 4C 8D 15 8B E8 94 12 4C 8B 05 9E E8 94 12` | visible list/mesh RIP reference |
| 356 | `0x00299BD3` | 2 | `40 00` | `00 01` | mesh field relative to relocated list base |
| 357 | `0x00299C09` | 4 | `E3 5F 68 02` | `33 E8 95 12` | visible list/mesh RIP reference |
| 358 | `0x00299CE7` | 11 | `DF 1E 68 02 4C 8D 15 BE 1E 68 02` | `2F E7 94 12 4C 8D 15 0E E7 94 12` | visible list/mesh RIP reference |
| 359 | `0x00299D06` | 4 | `C2 1E 68 02` | `12 E7 94 12` | visible list/mesh RIP reference |
| 360 | `0x00299DCB` | 4 | `21 5E 68 02` | `71 E6 95 12` | visible list/mesh RIP reference |
| 361 | `0x00299FB3` | 4 | `39 5C 68 02` | `89 E4 95 12` | visible list/mesh RIP reference |
| 362 | `0x0029D6EC` | 4 | `EC 24 68 02` | `3C AD 95 12` | visible list/mesh RIP reference |
| 363 | `0x0029D6FA` | 4 | `EC 24 68 02` | `3C AD 95 12` | visible list/mesh RIP reference |
| 364 | `0x0029D70C` | 4 | `D0 07 92 02` | `20 90 BF 12` | visible list/mesh module-indexed reference |
| 365 | `0x0029D721` | 4 | `F0 07 92 02` | `40 90 BF 12` | visible list/mesh module-indexed reference |
| 366 | `0x0029D7F2` | 4 | `F4 23 68 02` | `44 AC 95 12` | visible list/mesh RIP reference |
| 367 | `0x0029D80C` | 4 | `E0 23 68 02` | `30 AC 95 12` | visible list/mesh RIP reference |
| 368 | `0x002A185D` | 10 | `DF F7 9A 00 41 81 E1 FF FF 03` | `9F 6B 84 12 41 81 E1 FF FF 0F` | per-frame packet backing pointer; CPU packet offset decoder |
| 369 | `0x002EB825` | 10 | `01 00 00 48 8B 05 E1 DB 80 00` | `10 00 00 E9 B3 4F 76 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 370 | `0x002EB843` | 1 | `01` | `10` | spatial bitmap clear length |
| 371 | `0x002EBAB3` | 1 | `01` | `10` | spatial frame high field / restoration |
| 372 | `0x003BBB42` | 4 | `CA 67 74 04` | `BA C8 72 0E` | fallback arena registrations retain shared secondary backing |
| 373 | `0x004695E4` | 9 | `65 48 8B 04 25 58 00 00 00` | `E9 87 03 A7 04 90 90 90 90` | redirect object render invalidation append |
| 374 | `0x0046996D` | 7 | `78 FF FF B8 A0 88 00` | `C0 FB FF B8 A0 40 04` | object-job scratch frame |
| 375 | `0x0046998B` | 2 | `88 00` | `40 04` | object-job scratch frame |
| 376 | `0x0046999C` | 2 | `87 00` | `3F 04` | object-job scratch frame |
| 377 | `0x004699AB` | 1 | `28` | `50` | consumer queue flag / count offset |
| 378 | `0x004699BB` | 1 | `08` | `40` | object-job scratch frame |
| 379 | `0x004699C2` | 2 | `7F 00` | `FF 03` | object-job scratch frame |
| 380 | `0x004699D5` | 27 | `87 00 00 83 A5 70 87 00 00 00 C6 85 80 87 00 00 00 2B BB 08 28 00 00 2B BB 0C 28` | `3F 04 00 83 A5 70 3F 04 00 00 C6 85 80 3F 04 00 00 2B BB 08 50 00 00 2B BB 0C 50` | object-job scratch frame; consumer queue flag / count offset |
| 381 | `0x004699F5` | 1 | `28` | `50` | consumer queue flag / count offset |
| 382 | `0x00469A0A` | 7 | `28 00 00 8B B3 00 28` | `50 00 00 8B B3 00 50` | consumer queue flag / count offset |
| 383 | `0x00469A6B` | 1 | `20` | `40` | consumer queue flag / count offset |
| 384 | `0x00469AE5` | 1 | `28` | `50` | consumer queue flag / count offset |
| 385 | `0x00469AEE` | 1 | `28` | `50` | consumer queue flag / count offset |
| 386 | `0x00469B17` | 1 | `28` | `50` | consumer queue flag / count offset |
| 387 | `0x00469B3E` | 2 | `7F 00` | `FF 03` | object-job scratch frame |
| 388 | `0x00469B4A` | 2 | `87 00` | `3F 04` | object-job scratch frame |
| 389 | `0x00469B57` | 2 | `40 07` | `00 23` | external scheduler layout access |
| 390 | `0x00469B7A` | 2 | `40 07` | `00 23` | external scheduler layout access |
| 391 | `0x00469B92` | 2 | `40 07` | `00 23` | external scheduler layout access |
| 392 | `0x00469BCE` | 2 | `C0 00` | `00 03` | external scheduler layout access |
| 393 | `0x00469BE3` | 19 | `28 00 00 89 83 08 28 00 00 8B 83 04 28 00 00 89 83 0C 28` | `50 00 00 89 83 08 50 00 00 8B 83 04 50 00 00 89 83 0C 50` | consumer queue flag / count offset |
| 394 | `0x00469BFD` | 2 | `87 00` | `3F 04` | object-job scratch frame |
| 395 | `0x00469C08` | 2 | `7F 00` | `FF 03` | object-job scratch frame |
| 396 | `0x00469C13` | 2 | `3F 00` | `FF 01` | object-job scratch frame |
| 397 | `0x00469C1F` | 2 | `87 00` | `3F 04` | object-job scratch frame |
| 398 | `0x00469C3E` | 9 | `3F 00 00 00 8B 85 70 87 00` | `FF 01 00 00 8B 85 70 3F 04` | object-job scratch frame |
| 399 | `0x00469C52` | 2 | `87 00` | `3F 04` | object-job scratch frame |
| 400 | `0x00469C62` | 9 | `88 00 00 48 81 C4 A0 88 00` | `40 04 00 48 81 C4 A0 40 04` | object-job scratch frame |
| 401 | `0x00469CC0` | 7 | `4C 8B DC 49 89 5B 08` | `E9 3B F7 A6 04 90 90` | redirect object initialization |
| 402 | `0x0046B204` | 1 | `20` | `40` | primary and companion clear length |
| 403 | `0x0046B219` | 4 | `47 1A 7A 00` | `03 A2 D0 14` | relocated update queue global |
| 404 | `0x0046B227` | 17 | `39 42 7A 00 48 89 3D 3A 42 7A 00 89 3D 3C 62 7A 00` | `F5 F1 D0 14 48 89 3D F6 F1 D0 14 89 3D C8 41 D1 14` | relocated update queue global |
| 405 | `0x0046B243` | 4 | `2D 42 7A 00` | `B9 01 D1 14` | relocated update queue global |
| 406 | `0x0046B552` | 2 | `C0 00` | `00 03` | owner-relative frame-fence pending counter |
| 407 | `0x0046B588` | 4 | `D8 3E 7A 00` | `94 EE D0 14` | relocated update queue global |
| 408 | `0x0046B59B` | 4 | `C5 16 7A 00` | `81 9E D0 14` | relocated update queue global |
| 409 | `0x0046B5F4` | 23 | `6B 3E 7A 00 00 48 8D 0D 64 16 7A 00 48 83 25 64 3E 7A 00 00 BB 00 20` | `27 EE D0 14 00 48 8D 0D 20 9E D0 14 48 83 25 20 EE D0 14 00 BB 00 40` | relocated update queue global; primary and companion clear length |
| 410 | `0x0046B61C` | 4 | `54 5E 7A 00` | `E0 3D D1 14` | relocated update queue global |
| 411 | `0x0046B62B` | 4 | `45 3E 7A 00` | `D1 FD D0 14` | relocated update queue global |
| 412 | `0x0046E4FD` | 4 | `DB 27 00 00` | `FF 1E 5E 0A` | Deletion event recording |
| 413 | `0x0046E629` | 1 | `01` | `10` | delete-time queued invalidation record offset |
| 414 | `0x0046E6C4` | 4 | `A4 FA FF FF` | `A8 1E 5E 0A` | Deletion event recording |
| 415 | `0x004732E4` | 10 | `01 00 00 48 8B 05 22 61 68 00` | `10 00 00 E9 14 D5 5D 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 416 | `0x004732F6` | 1 | `01` | `10` | spatial frame high field / restoration |
| 417 | `0x004732FE` | 1 | `01` | `10` | spatial frame high field / restoration |
| 418 | `0x00473367` | 1 | `01` | `10` | spatial bitmap clear length |
| 419 | `0x00473378` | 1 | `01` | `10` | spatial frame high field / restoration |
| 420 | `0x00473394` | 1 | `01` | `10` | spatial frame high field / restoration |
| 421 | `0x004733BA` | 1 | `01` | `10` | spatial frame high field / restoration |
| 422 | `0x004733CA` | 1 | `01` | `10` | spatial frame high field / restoration |
| 423 | `0x0047352B` | 10 | `01 00 00 48 8B 05 DB 5E 68 00` | `10 00 00 E9 ED D2 5D 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 424 | `0x0047353D` | 1 | `01` | `10` | spatial frame high field / restoration |
| 425 | `0x00473545` | 1 | `01` | `10` | spatial frame high field / restoration |
| 426 | `0x0047354F` | 1 | `01` | `10` | spatial frame high field / restoration |
| 427 | `0x004735B3` | 1 | `01` | `10` | spatial bitmap clear length |
| 428 | `0x004735C4` | 1 | `01` | `10` | spatial frame high field / restoration |
| 429 | `0x004735F7` | 1 | `01` | `10` | spatial frame high field / restoration |
| 430 | `0x0047362A` | 1 | `01` | `10` | spatial frame high field / restoration |
| 431 | `0x00473636` | 1 | `01` | `10` | spatial frame high field / restoration |
| 432 | `0x00473655` | 1 | `01` | `10` | spatial frame high field / restoration |
| 433 | `0x00473665` | 1 | `01` | `10` | spatial frame high field / restoration |
| 434 | `0x0047369D` | 1 | `21` | `30` | second iterator caller stack reservation |
| 435 | `0x004736D1` | 1 | `01` | `10` | second iterator visited bitmap clear |
| 436 | `0x0047375E` | 1 | `21` | `30` | second iterator caller stack restoration |
| 437 | `0x00473DD7` | 4 | `75 8E 79 00` | `31 16 D0 14` | writer indexed alias, phase flag unchanged |
| 438 | `0x00473DE4` | 4 | `7C B6 79 00` | `38 66 D0 14` | relocated update queue global |
| 439 | `0x00473DF2` | 27 | `6E B6 79 00 42 88 8C 00 14 20 00 00 FF 05 60 B6 79 00 0F B6 C1 01 05 5B B6 79 00` | `2A 66 D0 14 42 88 8C 00 14 40 00 00 FF 05 1C 66 D0 14 0F B6 C1 01 05 17 66 D0 14` | relocated update queue global; writer flag-array offset |
| 440 | `0x0047B82C` | 2 | `40 00` | `00 04` | queue merge reset bitmap words |
| 441 | `0x0047B923` | 1 | `01` | `10` | message consumer record offset |
| 442 | `0x0047C5E6` | 1 | `01` | `10` | queue merge source records |
| 443 | `0x0047C612` | 1 | `08` | `80` | queue merge capacity; one reserved slot retained |
| 444 | `0x0047C631` | 1 | `01` | `10` | queue merge destination records |
| 445 | `0x00689D6C` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 446 | `0x00689D88` | 1 | `08` | `26` | coordinated scheduler layout |
| 447 | `0x00689D92` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 448 | `0x00689DC0` | 1 | `02` | `08` | scheduler biased record pointer |
| 449 | `0x00689E30` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 450 | `0x00689E64` | 9 | `C0 00 00 04 8B 8E 0C 40 07` | `00 03 00 04 8B 8E 0C 00 23` | coordinated scheduler layout |
| 451 | `0x00689E79` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 452 | `0x00689E89` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 453 | `0x00689F00` | 2 | `C0 08` | `00 29` | main-thread job wait worker-context pointer |
| 454 | `0x00689F6A` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 455 | `0x00689F9A` | 1 | `02` | `08` | scheduler biased record pointer |
| 456 | `0x00689FA4` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 457 | `0x00689FB3` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 458 | `0x00689FBB` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 459 | `0x00689FC3` | 8 | `C0 06 00 89 8F 04 40 07` | `00 1B 00 89 8F 04 00 23` | coordinated scheduler layout |
| 460 | `0x00689FD6` | 2 | `C0 06` | `00 1B` | coordinated scheduler layout |
| 461 | `0x00689FE8` | 2 | `C0 06` | `00 1B` | coordinated scheduler layout |
| 462 | `0x00689FFD` | 8 | `C0 00 00 8B 8F 14 40 07` | `00 03 00 8B 8F 14 00 23` | coordinated scheduler layout |
| 463 | `0x0068A09B` | 1 | `10` | `40` | scheduler job / queue capacity |
| 464 | `0x0068A0B3` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 465 | `0x0068A0DB` | 2 | `C0 08` | `00 29` | coordinated scheduler layout |
| 466 | `0x0068A10E` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 467 | `0x0068A120` | 1 | `10` | `40` | scheduler job / queue capacity |
| 468 | `0x0068A138` | 1 | `10` | `40` | scheduler job / queue capacity |
| 469 | `0x0068A16D` | 2 | `88 00` | `40 04` | object-job context helper field |
| 470 | `0x0068A176` | 2 | `80 00` | `00 04` | object-job context helper field |
| 471 | `0x0068A181` | 2 | `40 00` | `00 02` | object-job context helper field |
| 472 | `0x0068A18D` | 2 | `88 00` | `40 04` | object-job context helper field |
| 473 | `0x0068A1B1` | 2 | `40 00` | `00 02` | object-job context helper field |
| 474 | `0x0068A1BA` | 2 | `88 00` | `40 04` | object-job context helper field |
| 475 | `0x0068A1E9` | 9 | `88 00 00 4C 8D B1 00 40 00` | `40 04 00 4C 8D B1 00 00 02` | object-job context helper field |
| 476 | `0x0068A208` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 477 | `0x0068A21C` | 1 | `08` | `26` | coordinated scheduler layout |
| 478 | `0x0068A24C` | 1 | `02` | `08` | scheduler biased record pointer |
| 479 | `0x0068A260` | 2 | `C0 FF` | `00 FE` | object-job context helper field |
| 480 | `0x0068A292` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 481 | `0x0068A2A4` | 2 | `88 00` | `40 04` | object-job context helper field |
| 482 | `0x0068A2C3` | 2 | `40 00` | `00 02` | object-job context helper field |
| 483 | `0x0068A300` | 2 | `80 00` | `00 04` | object-job context helper field |
| 484 | `0x0068A322` | 2 | `C0 08` | `00 29` | coordinated scheduler layout |
| 485 | `0x0068A32A` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 486 | `0x0068A337` | 2 | `C0 08` | `00 29` | coordinated scheduler layout |
| 487 | `0x0068A395` | 2 | `C0 00` | `00 03` | scheduler record pointer inversion |
| 488 | `0x0068A3B9` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 489 | `0x0068A3CD` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 490 | `0x0068A3E5` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 491 | `0x0068A3F5` | 2 | `C0 06` | `00 1B` | coordinated scheduler layout |
| 492 | `0x0068A405` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 493 | `0x0068A40E` | 27 | `C0 06 00 8B 87 04 40 07 00 89 84 D7 00 C0 06 00 FF 87 00 40 07 00 89 8F 04 40 07` | `00 1B 00 8B 87 04 00 23 00 89 84 D7 00 00 1B 00 FF 87 00 00 23 00 89 8F 04 00 23` | coordinated scheduler layout |
| 494 | `0x0068A432` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 495 | `0x0068A464` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 496 | `0x0068A482` | 2 | `C0 00` | `00 03` | scheduler record pointer inversion |
| 497 | `0x0068A48D` | 1 | `08` | `26` | coordinated scheduler layout |
| 498 | `0x0068A4B8` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 499 | `0x0068A4D7` | 14 | `C0 00 00 00 BA 00 10 00 00 83 A1 04 C0 00` | `00 03 00 00 BA 00 40 00 00 83 A1 04 00 03` | coordinated scheduler layout; scheduler job / queue capacity |
| 500 | `0x0068A4EE` | 23 | `C0 00 00 44 89 81 14 C0 00 00 44 89 81 0C C0 00 00 44 89 81 10 C0 00` | `00 03 00 44 89 81 14 00 03 00 44 89 81 0C 00 03 00 44 89 81 10 00 03` | coordinated scheduler layout |
| 501 | `0x0068A515` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 502 | `0x0068A51E` | 8 | `C0 00 00 89 91 14 C0 00` | `00 03 00 89 91 14 00 03` | coordinated scheduler layout |
| 503 | `0x0068A53E` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 504 | `0x0068A54E` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 505 | `0x0068A56E` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 506 | `0x0068A57F` | 16 | `C0 00 00 45 89 88 0C C0 00 00 45 89 88 10 C0 00` | `00 03 00 45 89 88 0C 00 03 00 45 89 88 10 00 03` | coordinated scheduler layout |
| 507 | `0x0068A596` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 508 | `0x0068A5B4` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 509 | `0x0068A5C5` | 9 | `C0 00 00 45 89 88 10 C0 00` | `00 03 00 45 89 88 10 00 03` | coordinated scheduler layout |
| 510 | `0x0068A5D8` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 511 | `0x0068A5E3` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 512 | `0x0068A5F3` | 9 | `80 00 00 41 FF 80 00 C0 00` | `00 02 00 41 FF 80 00 00 03` | coordinated scheduler layout |
| 513 | `0x0068A610` | 14 | `C0 00 00 FF 89 00 C0 00 00 8B 91 14 C0 00` | `00 03 00 FF 89 00 00 03 00 8B 91 14 00 03` | coordinated scheduler layout |
| 514 | `0x0068A626` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 515 | `0x0068A633` | 15 | `C0 00 00 8B 81 0C C0 00 00 44 89 89 14 C0 00` | `00 03 00 8B 81 0C 00 03 00 44 89 89 14 00 03` | coordinated scheduler layout |
| 516 | `0x0068A650` | 2 | `80 00` | `00 02` | coordinated scheduler layout |
| 517 | `0x0068A6B4` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 518 | `0x0068A6C1` | 1 | `08` | `26` | coordinated scheduler layout |
| 519 | `0x0068A6D0` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 520 | `0x0068A6F9` | 2 | `C0 07` | `00 25` | coordinated scheduler layout |
| 521 | `0x0068A70B` | 2 | `C0 00` | `00 03` | coordinated scheduler layout |
| 522 | `0x0068A71F` | 1 | `02` | `08` | scheduler biased record pointer |
| 523 | `0x0068A73F` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 524 | `0x0068A74A` | 1 | `08` | `26` | coordinated scheduler layout |
| 525 | `0x0068A751` | 1 | `08` | `26` | coordinated scheduler layout |
| 526 | `0x0068A75A` | 8 | `40 07 00 89 83 28 00 08` | `00 23 00 89 83 28 00 26` | coordinated scheduler layout |
| 527 | `0x0068A768` | 1 | `08` | `26` | coordinated scheduler layout |
| 528 | `0x0068A771` | 8 | `40 07 00 89 83 2C 00 08` | `00 23 00 89 83 2C 00 26` | coordinated scheduler layout |
| 529 | `0x0068A77F` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 530 | `0x0068A78D` | 9 | `40 07 00 89 84 CB 1C 40 07` | `00 23 00 89 84 CB 1C 00 23` | coordinated scheduler layout |
| 531 | `0x0068A79C` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 532 | `0x0068A7AA` | 15 | `40 07 00 89 84 CB 20 40 07 00 8B 83 30 00 08` | `00 23 00 89 84 CB 20 00 23 00 8B 83 30 00 26` | coordinated scheduler layout |
| 533 | `0x0068A7BF` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 534 | `0x0068A7CA` | 1 | `08` | `26` | coordinated scheduler layout |
| 535 | `0x0068A7D1` | 1 | `08` | `26` | coordinated scheduler layout |
| 536 | `0x0068A7F3` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 537 | `0x0068A805` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 538 | `0x0068A9B1` | 2 | `40 07` | `00 23` | coordinated scheduler layout |
| 539 | `0x0068A9C1` | 29 | `40 07 00 89 86 10 40 07 00 44 89 86 14 40 07 00 44 89 8E 18 40 07 00 44 89 96 54 C0 08` | `00 23 00 89 86 10 00 23 00 44 89 86 14 00 23 00 44 89 8E 18 00 23 00 44 89 96 54 00 29` | coordinated scheduler layout |
| 540 | `0x0068A9E9` | 1 | `08` | `26` | coordinated scheduler layout |
| 541 | `0x0068AA17` | 1 | `10` | `40` | scheduler job / queue capacity |
| 542 | `0x0068AA20` | 8 | `C0 06 00 83 A0 04 80 00` | `00 1B 00 83 A0 04 00 08` | coordinated scheduler layout |
| 543 | `0x0068AA3A` | 1 | `0F` | `FF` | scheduler dependency free-list capacity |
| 544 | `0x0068AA42` | 2 | `7F 00` | `FF 07` | coordinated scheduler layout |
| 545 | `0x0068AA4A` | 12 | `C0 08 00 C7 80 00 80 00 00 00 10 00` | `00 29 00 C7 80 00 00 08 00 00 00 01` | coordinated scheduler layout; scheduler dependency free-list capacity |
| 546 | `0x006BD1CF` | 10 | `01 00 00 48 8B 05 37 C2 43 00` | `10 00 00 E9 69 36 39 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 547 | `0x006BD1E1` | 1 | `01` | `10` | spatial frame high field / restoration |
| 548 | `0x006BD209` | 1 | `01` | `10` | spatial bitmap clear length |
| 549 | `0x006BD27D` | 1 | `01` | `10` | spatial frame high field / restoration |
| 550 | `0x006BD28D` | 1 | `01` | `10` | spatial frame high field / restoration |
| 551 | `0x00736119` | 10 | `01 00 00 48 8B 05 ED 32 3C 00` | `10 00 00 E9 3F A7 31 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 552 | `0x0073623F` | 1 | `01` | `10` | spatial bitmap clear length |
| 553 | `0x007363BA` | 1 | `01` | `10` | spatial frame high field / restoration |
| 554 | `0x007363C1` | 1 | `01` | `10` | spatial stack restoration |
| 555 | `0x007364D0` | 1 | `FE` | `EF` | retain low spatial frame RBP anchor |
| 556 | `0x007364D7` | 10 | `02 00 00 48 8B 05 2F 2F 3C 00` | `11 00 00 E9 A1 A3 31 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 557 | `0x007364E8` | 1 | `01` | `10` | spatial RBP high field |
| 558 | `0x00736515` | 1 | `01` | `10` | spatial bitmap clear length |
| 559 | `0x00736676` | 1 | `00` | `0F` | spatial RBP high field |
| 560 | `0x0073669A` | 1 | `00` | `0F` | spatial RBP high field |
| 561 | `0x0073679F` | 1 | `01` | `10` | spatial RBP high field |
| 562 | `0x007367AF` | 1 | `02` | `11` | spatial frame high field / restoration |
| 563 | `0x00736810` | 1 | `F4` | `E5` | retain low spatial frame RBP anchor |
| 564 | `0x00736817` | 1 | `0C` | `1B` | spatial bitmap stack reservation |
| 565 | `0x0073681E` | 7 | `48 8B 05 EB 2B 3C 00` | `E9 7D A0 31 0A 90 90` | spatial stack page-probe hook |
| 566 | `0x0073682C` | 1 | `0A` | `19` | spatial RBP high field |
| 567 | `0x007368C0` | 1 | `00` | `0F` | spatial RBP high field |
| 568 | `0x007368C7` | 1 | `01` | `10` | spatial RBP high field |
| 569 | `0x007368D3` | 1 | `01` | `10` | spatial RBP high field |
| 570 | `0x007368DD` | 1 | `00` | `0F` | spatial RBP high field |
| 571 | `0x00736A13` | 1 | `00` | `0F` | spatial RBP high field |
| 572 | `0x00736A6A` | 1 | `02` | `11` | spatial RBP high field |
| 573 | `0x00736A76` | 1 | `00` | `0F` | spatial RBP high field |
| 574 | `0x00736A7D` | 1 | `00` | `0F` | spatial RBP high field |
| 575 | `0x00736A84` | 1 | `02` | `11` | spatial RBP high field |
| 576 | `0x00736AB7` | 1 | `01` | `10` | spatial bitmap clear length |
| 577 | `0x00736BA4` | 1 | `04` | `13` | spatial RBP high field |
| 578 | `0x00736C5F` | 1 | `00` | `0F` | spatial RBP high field |
| 579 | `0x00736C7D` | 1 | `04` | `13` | spatial RBP high field |
| 580 | `0x00736D1C` | 1 | `00` | `0F` | spatial RBP high field |
| 581 | `0x00736D2C` | 1 | `04` | `13` | spatial RBP high field |
| 582 | `0x00736D89` | 1 | `04` | `13` | spatial RBP high field |
| 583 | `0x00736DDC` | 1 | `04` | `13` | spatial RBP high field |
| 584 | `0x00736DEF` | 1 | `04` | `13` | spatial RBP high field |
| 585 | `0x00736DFC` | 1 | `04` | `13` | spatial RBP high field |
| 586 | `0x00736E05` | 1 | `04` | `13` | spatial RBP high field |
| 587 | `0x00736E41` | 1 | `04` | `13` | spatial RBP high field |
| 588 | `0x00736E4D` | 1 | `04` | `13` | spatial RBP high field |
| 589 | `0x00736ED9` | 1 | `0A` | `19` | spatial RBP high field |
| 590 | `0x00736EE9` | 1 | `0C` | `1B` | spatial frame high field / restoration |
| 591 | `0x00738678` | 10 | `01 00 00 48 8B 05 8E 0D 3C 00` | `10 00 00 E9 40 82 31 0A 90 90` | spatial bitmap stack reservation; spatial stack page-probe hook |
| 592 | `0x0073868A` | 1 | `01` | `10` | spatial frame high field / restoration |
| 593 | `0x007386A9` | 1 | `01` | `10` | spatial bitmap clear length |
| 594 | `0x0073874A` | 1 | `01` | `10` | spatial frame high field / restoration |
| 595 | `0x0073875A` | 1 | `01` | `10` | spatial frame high field / restoration |
| 596 | `0x00738761` | 1 | `01` | `10` | spatial stack restoration |
| 597 | `0x0091D076` | 2 | `C0 08` | `00 29` | scheduler owner array stride / constructor size |
| 598 | `0x0091D082` | 4 | `4A 17 45 02` | `7A 73 5C 14` | relocated global scheduler owner |
| 599 | `0x00AA21D2` | 2 | `2C 04` | `0C 06` | iterator caller unwind offset |
| 600 | `0x00AA21DD` | 1 | `21` | `30` | iterator caller GS cookie offset |
| 601 | `0x00AA2562` | 2 | `39 00` | `19 02` | spatial unwind allocation / saved register |
| 602 | `0x00AA2579` | 1 | `01` | `10` | spatial GS cookie frame offset |
| 603 | `0x00AA45DA` | 6 | `50 00 1F 01 46 00` | `30 02 1F 01 26 02` | spatial unwind allocation / saved register |
| 604 | `0x00AA45F5` | 1 | `02` | `11` | spatial GS cookie frame offset |
| 605 | `0x00AA4676` | 30 | `89 16 62 C8 8A 16 59 B8 8B 16 50 A8 8C 16 47 98 8D 16 3E 88 8E 16 35 78 8F 16 2D 68 90 16` | `79 17 62 C8 7A 17 59 B8 7B 17 50 A8 7C 17 47 98 7D 17 3E 88 7E 17 35 78 7F 17 2D 68 80 17` | spatial unwind allocation / saved register |
| 606 | `0x00AA4696` | 6 | `2B 2D 25 01 22 2D` | `0B 2F 25 01 02 2F` | spatial unwind allocation / saved register |
| 607 | `0x00AA46B1` | 1 | `68` | `77` | spatial GS cookie frame offset |
| 608 | `0x00AA46BA` | 14 | `AC 01 2D 68 AD 01 25 34 66 03 25 01 5C 03` | `9C 02 2D 68 9D 02 25 34 46 05 25 01 3C 05` | spatial unwind allocation / saved register |
| 609 | `0x00AA46DD` | 1 | `1A` | `29` | spatial GS cookie frame offset |
| 610 | `0x00AA476E` | 14 | `1C 02 32 78 1D 02 2A 68 1E 02 22 01 3F 04` | `0C 03 32 78 0D 03 2A 68 0E 03 22 01 1F 06` | spatial unwind allocation / saved register |
| 611 | `0x00AA4791` | 1 | `21` | `30` | spatial GS cookie frame offset |
| 612 | `0x00AA6D26` | 30 | `72 00 52 D8 73 00 4A C8 74 00 42 B8 75 00 3A A8 76 00 32 98 77 00 2D 88 78 00 28 78 79 00` | `62 01 52 D8 63 01 4A C8 64 01 42 B8 65 01 3A A8 66 01 32 98 67 01 2D 88 68 01 28 78 69 01` | spatial unwind allocation / saved register |
| 613 | `0x00AA6D46` | 10 | `7A 00 20 34 00 01 20 01 F6 00` | `6A 01 20 34 E0 02 20 01 D6 02` | spatial unwind allocation / saved register |
| 614 | `0x00AA6D65` | 1 | `07` | `16` | spatial GS cookie frame offset |
| 615 | `0x00AAAEFA` | 18 | `68 04 35 78 69 04 2D 68 6A 04 25 34 DF 08 25 01 D6 08` | `58 05 35 78 59 05 2D 68 5A 05 25 34 BF 0A 25 01 B6 0A` | spatial unwind allocation / saved register |
| 616 | `0x00AAAF21` | 1 | `46` | `55` | spatial GS cookie frame offset |
| 617 | `0x00AACC32` | 14 | `67 04 2C 64 66 04 2C 34 65 04 2C 01 5E 04` | `27 44 2C 64 26 44 2C 34 25 44 2C 01 1E 44` | unwind 0x227250 operation 4; unwind 0x227250 operation 1 |
| 618 | `0x00AACC51` | 2 | `22 00` | `20 02` | GS unwind cookie offset 0x227250 |
| 619 | `0x00AACC5A` | 2 | `2E 04` | `0E 42` | unwind 0x227538 operation 1 |
| 620 | `0x00AACC65` | 2 | `21 00` | `10 02` | GS unwind cookie offset 0x227538 |
| 621 | `0x00AAFAEA` | 10 | `17 02 13 34 33 04 13 01 30 04` | `07 21 13 34 13 42 13 01 10 42` | unwind 0x27e624 operation 8; unwind 0x27e624 operation 4; unwind 0x27e624 operation 1 |
| 622 | `0x00AAFAFD` | 2 | `21 00` | `10 02` | GS unwind cookie offset 0x27e624 |
| 623 | `0x00AB394E` | 6 | `36 00 1C 01 2C 00` | `16 02 1C 01 0C 02` | spatial unwind allocation / saved register |
| 624 | `0x00AB3969` | 1 | `01` | `10` | spatial GS cookie frame offset |
| 625 | `0x00AB3BDE` | 14 | `35 00 24 64 34 00 24 34 33 00 24 01 2C 00` | `15 02 24 64 14 02 24 34 13 02 24 01 0C 02` | spatial unwind allocation / saved register |
| 626 | `0x00AB3BFD` | 1 | `01` | `10` | spatial GS cookie frame offset |
| 627 | `0x00AC126F` | 5 | `11 21 01 14 11` | `88 21 01 14 88` | object-job unwind allocation / save |
| 628 | `0x00AC128D` | 2 | `88 00` | `40 04` | object-job GS+EH cookie offset |
| 629 | `0x00AC1546` | 14 | `32 00 17 54 31 00 17 34 30 00 17 01 2E 00` | `12 02 17 54 11 02 17 34 10 02 17 01 0E 02` | spatial unwind allocation / saved register |
| 630 | `0x00AC155D` | 1 | `01` | `10` | spatial GS cookie frame offset |
| 631 | `0x00AC1566` | 10 | `37 00 16 54 36 00 16 01 30 00` | `17 02 16 54 16 02 16 01 10 02` | spatial unwind allocation / saved register |
| 632 | `0x00AC157D` | 1 | `01` | `10` | spatial GS cookie frame offset |
| 633 | `0x00AC1586` | 14 | `34 04 2C 64 33 04 2C 34 32 04 2C 01 2C 04` | `14 06 2C 64 13 06 2C 34 12 06 2C 01 0C 06` | iterator caller unwind offset |
| 634 | `0x00AC15A5` | 1 | `21` | `30` | iterator caller GS cookie offset |
| 635 | `0x00AD63C2` | 10 | `38 00 12 34 37 00 12 01 34 00` | `18 02 12 34 17 02 12 01 14 02` | spatial unwind allocation / saved register |
| 636 | `0x00AD63D5` | 1 | `01` | `10` | spatial GS cookie frame offset |
| 637 | `0x00AD9FDA` | 14 | `4F 00 26 64 4E 00 26 34 4D 00 26 01 46 00` | `2F 02 26 64 2E 02 26 34 2D 02 26 01 26 02` | spatial unwind allocation / saved register |
| 638 | `0x00AD9FF9` | 1 | `02` | `11` | spatial GS cookie frame offset |
| 639 | `0x00ADA002` | 18 | `BF 00 26 74 89 01 26 64 88 01 26 34 87 01 26 01 80 01` | `AF 01 26 74 69 03 26 64 68 03 26 34 67 03 26 01 60 03` | spatial unwind allocation / saved register |
| 640 | `0x00ADA025` | 1 | `0B` | `1A` | spatial GS cookie frame offset |
| 641 | `0x00ADA0B6` | 6 | `32 00 0F 01 2C 00` | `12 02 0F 01 0C 02` | spatial unwind allocation / saved register |
| 642 | `0x00ADA0C9` | 1 | `01` | `10` | spatial GS cookie frame offset |
