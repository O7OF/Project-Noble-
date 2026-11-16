# Full Ghidra decompile of MCC haloreach.dll (2026-11-15)

## What / where (Tyler's PC)
- Folder: `D:\MCC HALO REACH DECOMP`. Run `run_decompile.bat` (headless Ghidra 12.1.4; sets GHIDRA_HEADLESS_MAXMEM=16G;
  copies the DLL to `dll\`, never touches the installed one). Script: `DecompileAll.java`.
- DLL analysed: installed MCC `haloreach\haloreach.dll`, SHA-256
  `738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894`, 13,229,016 bytes = the SAME build Noble uses (all
  `hr+0x...` RVAs in the other docs apply directly). It is NOT a different build.
- Output: `decompiled\hr_00000.c`..`hr_00013.c` (2000 functions per file, ~51 MB), `functions_index.csv`
  (rva,name,size,file,status). Each function starts with `// ==== FUN_1802d288c  rva 0x2d288c ====`.
- Result: 27,578 functions, 1 failure (rva 0x77f9e4, bad pcode at 0x18000000c). Auto-analysis 373 s; whole run ~10 min.
  Ghidra image base 0x180000000 (RVA = address - 0x180000000). No PDB, so names are FUN_*.
- Search example: `findstr /C:"rva 0x381f10" "D:\MCC HALO REACH DECOMP\decompiled\hr_*.c"`.

## Handler table decoded (14 entries at RVA 0xB4C800)
Handler object = {+0 tag, +8 can_convert_from, +0x10 convert, +0x18 to_text}. `0x36930` is a stub returning 0.
| tag | handler rva | can_convert/convert | to_text |
|---|---|---|---|
| 0 bool | 0xA45B10 | 0x381DC0/0x381DDC | 0x381E2C |
| 1 int32 | 0xA36058 | 0x2F3A6C/0x2F3A88 | 0x2F3AB8 (`_itow_s` radix 10) |
| 2 float | 0xA45B30 | 0x381E58/0x381E68 | 0x381EB0 |
| 8 int64 | 0xA45B50 | 0x381EE4/0x381EF0 | 0x381F10 (`_i64tow_s` radix 16) |
| 3 / 12 / 13 string | 0xA45B70/90/B0 | stub | 0x381F30 / 0x381F58 / 0x381F90 |
| 6 | 0xA36738 | 0x2F8FC8/0x2F8FD4 | stub |
| 4,5,7,9,10,11 | - | stubs | stubs |
`FUN_1802e9720` (0x2e9720) = binary-search tag -> handler; callers 0x2d26c0 (value->text), 0x2f4cf4 (data binding),
0x382490 (x2). The zero-padded hex bug is NOT in these handlers: see NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md
(the `\a<8hex>` expander FUN_1802d288c leaks its digits when the binding handle is -1).
