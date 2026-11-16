# Xbox 360 Reach: retail executable matches the Omaha release map (2026-11-15)

## Key finding
- Retail disc ISO `D:\ReachX360\Halo - Reach (World) (En,Ja,Fr,De,Es,It,Pt,Zh,Ko).iso` -> `default.xex` (12,468,224 bytes;
  retail-key encrypted, no real compression; original PE name `halo3_cache_release.exe`, base 0x82000000, entry
  0x827E0BF0, image 0x1AC0000, filetime 4C4AAE66 = Sat Jul 24 2010).
- Leaked Omaha build `C:\Users\tyler\Desktop\Halo Reach (Jul 24 2010) (Omaha) (Cache Build)` has
  `halo3_cache_release.xex` (same size, different signing/encryption bytes) and `halo3_cache_release.map` (MSVC linker
  map, "Timestamp is 4c4aae66" = same build time as retail).
- Verified: of the retail image's 23,091 .pdata function starts, 22,935 (99.3%) have a name in the map's section-0004
  symbols; the map says 0x827E0BF0 = `mainCRTStartup`. So the map's names (mangled C++ names + object file) apply
  directly to the retail executable.
- The Omaha folder also holds debug/play/playtest/profile/test builds with maps (`halo3_cache_debug.map` 34 MB etc.),
  `crash_info.txt`, `debug_menu_init.txt`, `reports\*`.

## Files produced (Tyler's PC)
- `D:\ReachX360\xiso_extract.py` (run with `py`): lists / extracts files from the ISO.
- `D:\ReachX360\xextool\xex1tool.exe` (emoose): `-l` info, `-b out in` dumps the basefile -> `D:\ReachX360\default_base.exe`
  (28,049,408 bytes, PowerPC PE, flat image at 0x82000000; load as binary).
- Dev-kit pre-beta (`D:\ReachX360\dev kit beta build\Unlock\TU1_default_full.xex`, Lord Zedd mod): unencrypted; unpacked
  to `reach_dev_beta_default.exe`; Ghidra files in `D:\ReachX360\ghidra\`. Names are FUN_* (earlier build, no map applied).
- Retail Ghidra job: `D:\ReachX360\ghidra_retail\run_decompile_retail.bat` (BinaryLoader at 0x82000000, language
  `PowerPC:BE:64:A2ALT-32addr`, pre-scripts `CreateFunctionsFromPdata.java` and `ApplyMapSymbols.java` reading
  `symbols.csv`, post-script `DecompileAll.java`). Known gap: Ghidra has no VMX128 support.

## RESULT (run finished 2026-11-15 ~06:00 ET)
- pdata functions created 22,998 (93 skipped); map symbols applied 57,153 (15,046 extra functions created, 0 failed).
- Decompile output `D:\ReachX360\ghidra_retail\decompiled\hr_00000..hr_00016.c` + `functions_index.csv`:
  33,787 functions, 33,712 named (99.8%), 75 still FUN_*, 1 decompile failure. Entry = mainCRTStartup (names land on the
  right code).

## Why it matters for MCC
Named Xbox 360 functions can be matched to the MCC x64 `haloreach.dll` through shared structure, strings and asserts, to
name `FUN_*` functions in `D:\MCC HALO REACH DECOMP`. First matches (property editors, CUI text) are in
NOTE-HEX-BUG-ROOT-CAUSE-2026-11-15.md. Legal note: the retail game is Microsoft/Bungie code; these decompiles stay
local and git-excluded (Compendium contains scripts/indexes/notes only, no game binaries or decompile text).
