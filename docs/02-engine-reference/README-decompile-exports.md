# Decompile exports (haloreach.dll + Xbox 360 Reach) - index
Made 2026-11-15 by Claude with Tyler. This folder holds the SCRIPTS, FUNCTION INDEXES and notes only. The raw decompile
text (about 70 MB of .c files) and the game binaries stay on Tyler's PC (git-excluded, no game assets here).
Findings: `..\project-docs\NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md` (read first), `NOTE-MCC-HALOREACH-FULL-DECOMP-...`,
`NOTE-REACH-X360-RETAIL-AND-OMAHA-MAP-...`.

| Job | Where the full output lives (Tyler's PC) | Here |
|---|---|---|
| **MCC haloreach.dll** (same build as Noble, SHA-256 738dd2d2...0894, 13,229,016 bytes). 27,578 functions, names FUN_*, image base 0x180000000 | `D:\MCC HALO REACH DECOMP\decompiled\hr_00000..00013.c` | `mcc-haloreach\` (DecompileAll.java, run_decompile.bat, functions_index.csv, texthook_diag.cpp) |
| **Xbox 360 RETAIL Reach** (halo3_cache_release.exe, Jul 24 2010), named from the Omaha linker map. 33,787 functions, 99.8% named, base 0x82000000 | `D:\ReachX360\ghidra_retail\decompiled_v3\hr_00000..00016.c` | `x360-retail\` (scripts, run_decompile_retail.bat, functions_index.csv, symbols.csv = 57,153 map symbols) |
| **Xbox 360 dev-kit beta** (TU1_default_full.xex, Lord Zedd). 22,317 functions decompiled, names FUN_*, no map | `D:\ReachX360\ghidra\decompiled\hr_00000..00011.c` | `x360-devkit-beta\` |
| ISO / XEX tools | `D:\ReachX360\` (iso, default.xex, default_base.exe, xextool\xex1tool.exe) | `tools\xiso_extract.py` |

How to search: each function starts `// ==== <name>  rva 0x<rva> ====` (MCC rva = address - 0x180000000, retail rva = the
0x82xxxxxx virtual address). Example: `findstr /C:"rva 0x2d288c" "D:\MCC HALO REACH DECOMP\decompiled\hr_*.c"`;
retail by name: `findstr /C:"cui_property_editor_get" "D:\ReachX360\ghidra_retail\decompiled_v3\hr_*.c"`.
`functions_index.csv` columns: rva,name,size,file,status (look the function up here, then open the named hr_*.c).

Re-run a job (PowerShell, one command per block):
`& "D:\MCC HALO REACH DECOMP\run_decompile.bat"`  (MCC, ~10 min, needs GHIDRA_HEADLESS_MAXMEM=16G which the .bat sets)
`& "D:\ReachX360\ghidra_retail\run_decompile_retail_v3.bat"`  (retail v3, ~5 min; see update below)
Ghidra 12.1.4 headless lives at `C:\tools\ghidra_12.1.4_PUBLIC`. Retail language: `PowerPC:BE:64:A2ALT-32addr`.
Known gaps: no VMX128 (vector code shows "Unable to resolve constructor"), `.pdata` seeds the retail function list.

Do not confuse with the OLDER exports on the Desktop (`Reach.exe\functions`, `host-probe.exe\functions`: those are
Noble's own executables, not haloreach.dll). The Ghidra MCP on 127.0.0.1:8081 analyses `haloreach-copy.dll` (same DLL).

## Update 2026-11-15 (later): v3 fix, topics, and the other games
- **Retail 360 decompile: USE `D:\ReachX360\ghidra_retail\decompiled_v3\`**, not `decompiled\` or `decompiled_v2\`. The first two have ~10,100 empty stubs ("Subroutine does not return" = `__savegprlr_N` millicode wrongly flagged non-returning). v3 has 99. v2 (analyzer disabled) did NOT fix it; the fix is `FixMillicodePost.java`, run AFTER analysis, then re-decompile: `run_decompile_retail_v3.bat` (works on the saved project `project_v2`). Scripts live in `D:\ReachX360\ghidra_retail\`.
- **Other MCC engines decompiled** (same MCC build as Noble): output in `D:\MCC_DECOMP\<game>\decompiled\` for halo3 (20,444 fns), halo3odst (21,484), halo4 (36,876), halo2 (39,362), and **halo1 (Halo CE) finished 2026-11-15 10:19 ET: 79,637 functions, 2 failed** (40 `hr_*.c` files; `functions_index.csv` = 79,638 lines incl. header). Counts re-verified 2026-11-15 10:40 ET from each `functions_index.csv` (lines - 1) and `D:\MCC_DECOMP\halo1\ghidra.log` ("DONE: 79637 functions, 2 failed"). All x64, names are `FUN_*` unless noted. Batches: `D:\ReachX360\mcc_batches\run_<game>.bat`.
- **RTTI finding**: Ghidra's RTTI analyzer ran on halo1.dll ("Unprocessed TypeDescriptor" lines). x64 MSVC DLLs often keep C++ class names. NOT YET CHECKED for haloreach.dll: search the MCC Reach project for RTTI/vtable symbol names. If present, MCC functions can be named directly.
- **Topic chapters and the master upgrade list**: `topics\` (see `MASTER-UPGRADE-OPPORTUNITIES-2026-11-15.md`). Started `topics\mcc-address-table.md` (2026-11-15): one row per proven retail name -> MCC `hr+RVA`, open candidates marked UNPROVEN.
