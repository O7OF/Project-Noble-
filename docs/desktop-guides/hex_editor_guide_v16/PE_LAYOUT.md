# Final PE section map

File offsets are physical bytes. RVAs are loaded image addresses relative to the DLL base. A section with zero raw size has no physical bytes to paste.

| Section | Header file offset | RVA | Virtual size | Raw file offset | Raw size | Characteristics |
|---|---|---|---|---|---|
| .text | `0x00000270` | `0x00001000` | `0x0091CD0C` | `0x00000400` | `0x0091CE00` | `0x60000020` |
| .rdata | `0x00000298` | `0x0091E000` | `0x001DB214` | `0x0091D200` | `0x001DB400` | `0x40000040` |
| .data | `0x000002C0` | `0x00AFA000` | `0x04359AB0` | `0x00AF8600` | `0x0011D600` | `0xC0000040` |
| .pdata | `0x000002E8` | `0x04E54000` | `0x000550E0` | `0x00C15C00` | `0x00055200` | `0x40000040` |
| _RDATA | `0x00000310` | `0x04EAA000` | `0x00009930` | `0x00C6AE00` | `0x00009A00` | `0x40000040` |
| .rsrc | `0x00000338` | `0x04EB4000` | `0x0000050C` | `0x00C74800` | `0x00000600` | `0x40000040` |
| .reloc | `0x00000360` | `0x04EB5000` | `0x00024FBC` | `0x00C74E00` | `0x00025000` | `0x42000040` |
| .objpool | `0x00000388` | `0x04EDA000` | `0x05B76080` | `0x00C9DC00` | `0x00055A00` | `0xE0000060` |
| .deltrac | `0x000003B0` | `0x0AA51000` | `0x00097200` | `0x00CF3600` | `0x00097200` | `0xE0000060` |
| .hkwork | `0x000003D8` | `0x0AAE9000` | `0x105B9000` | `0x00000000` | `0x00000000` | `0xC0000080` |

- Section count: 7 → 10.
- SizeOfImage: `0x04EDA000` → `0x1B0A2000`.
- Preferred image base: `0x180000000`; runtime base can differ.
- Exception directory: RVA `0x0AA93000`, size `0x000551B8`.
- Final PE checksum at file offset `0x000001C0`: `25 72 D9 00`.
- The exact overwrite list includes section headers, data directories, image sizes, and checksum. The append block contains added initialized section bytes.
- SizeOfImage includes zero-filled virtual reservations and is much larger than the disk file. Do not pad the file to SizeOfImage.
