# Anti-reversing notes

Project Noble ships as a closed-source binary. This document lists what the build does to make Ghidra/IDA
work harder and, more importantly, what it no longer gives away for free.

## What the release build does

| Measure | Where |
|---|---|
| No `.map` and no `.pdb` next to the shipped executable | `CMakeLists.txt` (post-build deletes them; `/MAP` only on `noble-dev`) |
| Whole-program optimisation and COMDAT folding (`/GL /LTCG /OPT:REF /OPT:ICF /Gy /Gw`) | `CMakeLists.txt`, `noble-client` |
| RTTI removed (`/GR-`) - the client uses no `dynamic_cast`/`typeid` | `CMakeLists.txt`, `noble-client` |
| No log file in release; log macros compile to `((void)0)` | `src/common/Logger.hpp` |
| Release diagnostics reduced to a crash minidump beside the exe | `src/main.cpp` (crash/terminate/abort handlers) |
| Sensitive literals encrypted at compile time (`NOBLE_OBF` / `NOBLE_OBFW`) | `src/common/ObfuscatedString.hpp` |
| Engine module name / window class resolved from obfuscated literals | `src/common/EngineModule.hpp` |

The development build keeps the full trace and the `.map`, because that is what makes a bug report
actionable. The dedicated server keeps its log in release (it is a server binary).

## String obfuscation

Plain literals live in `.rdata` and are the first thing `strings`/Ghidra shows. `NOBLE_OBF` stores the
literal XOR-encrypted with a seed derived from the source line and a per-file counter, and decrypts it on
the stack at the point of use:

```cpp
GetModuleHandleW(NOBLE_OBFW(L"haloreach.dll").c_str());
http.Post(NOBLE_OBF("https://backend.example"), body);
```

Rules:

* The result is a temporary `std::string`/`std::wstring`. Copy it if it must outlive the full expression.
* Do not store the pointer returned by `.c_str()`.
* Never put real secrets in a literal; this raises the cost of static analysis but is not encryption.
* Prefer the helpers in `src/common/EngineModule.hpp` (`ReachModule()`, `ReachModuleFileName()`,
  `ReachWindowClass()`) over spelling the engine names out again.

Converted so far: backend and relay URLs, the master-server default, the engine module name, the engine
window class, and the module lookups that used to duplicate `L"haloreach.dll"` across the codebase.

## What is deliberately not done

* No packer/protector is applied. A commercial protector (VMProtect, Themida) or a virtualising
  obfuscator raises the bar much further, but it adds cost, build complexity and antivirus false
  positives - evaluate before adopting.
* No anti-debug or anti-dump checks: they are trivially bypassed, often trip anticheat/AV heuristics, and
  do not stop a determined analyst.
* Log messages are not obfuscated in dev builds; they are compiled out of release instead.

## Verifying a build

```powershell
# No symbol files, no plaintext endpoints or engine literals:
Get-ChildItem build\bin\Release\noble-client.exe, build\bin\Release\*.map, build\bin\Release\*.pdb
# Release: only a crash dump is produced (no log):
Remove-Item build\bin\Release\noble-*.log -ErrorAction SilentlyContinue
build\bin\Release\noble-client.exe -crash-test
Get-ChildItem build\bin\Release\noble-client.log, build\bin\Release\noble-crash.dmp
```
