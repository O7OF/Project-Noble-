# Ghidra setup — Project Noble

The Forge join-queue gate in `haloreach.dll` is labelled and commented in a ready-made Ghidra project, so nobody has to
re-derive the addresses. This file records how the project was made and how to rebuild it.

## Open it

```
C:\tools\ghidra_12.1.4_PUBLIC\ghidraRun.bat
```

Then **File → Open Project** → `C:\Users\tyler\Documents\deepseek-harness\default-workspace\ghidra-proj\NobleReach.gpr`.

Inside: program **`haloreach-copy.dll`** (double-click to open the CodeBrowser). That file is a byte-for-byte copy of
`D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll`; the MCC install is never modified.
Image base `0x180000000`, image size `0x4EDA000`, imported and auto-analysed with Ghidra 12.1.4 (Java 21).

## What is labelled

Every name below is a real label in the program, with a plate comment explaining it:

| Address | Label | Meaning |
|---|---|---|
| `0x18039BEEC` | `Noble_JoinQueue_SetMode` | queue mode setter; 4-after-3 is ignored at `0x18039BF03` |
| `0x18039A8D2` | `Noble_JoinQueue_Dispatch` | mode must be 1..4, step machine, processor only at step 4 |
| `0x18039A92D` | `Noble_JoinQueue_ProcessCall` | only call site of the processor |
| `0x18039BDCC` | `Noble_JoinQueue_Process` | moves a queued join into the session (state `+0x60EF8` must be 6) |
| `0x182A49308` | `Noble_JoinQueueMode` | mode (0 idle, 1..4 active) |
| `0x182A4930C` | `Noble_JoinQueueCount` | queued join count |
| `0x182A49318` | `Noble_JoinQueueEntries` | entries, `0x588` bytes each |
| `0x182A49074` | `Noble_JoinQueueStep` | step index; processor runs at 4 |
| `0x182A1FD40` | `Noble_JoinQueueSlotValid` | per-mode session pointers are valid |
| `0x182A1FE08/FE10/FE18` | `Noble_JoinQueueSession1/2/34` | session pointer per mode |

Mapping these to a **live** client: take the runtime base the client logs (`haloreach.dll successfully loaded at Base
Address: 0x...`) and add the RVA (`address - 0x180000000`). Example from a run at base `0x7FFCA2F40000`: the mode global
`0x182A49308` is `0x7FFCA59E9308` in that process.

## Rebuild / re-apply

```powershell
$proj  = 'C:\Users\tyler\Documents\deepseek-harness\default-workspace\ghidra-proj'
$hl    = 'C:\tools\ghidra_12.1.4_PUBLIC\support\analyzeHeadless.bat'

# import + analyse (once, ~15 min)
& $hl $proj NobleReach -import haloreach-copy.dll -overwrite

# apply (or re-apply after an edit) the labels and comments
& $hl $proj NobleReach -process haloreach-copy.dll -noanalysis `
      -postScript NobleForgeGate.java -scriptPath 'D:\Halo-Reach-Community-Edition\tools\ghidra'
```

Close the Ghidra GUI before running headless (it takes the project lock).

The script lives in `tools/ghidra/NobleForgeGate.java` and is plain Ghidra Java, so it can be edited and re-run; each run
prints `labelled <address> as <name>` for every entry.

## If the project is missing

Any of these reconstruct it:

1. `analyzeHeadless` as above (needs the DLL copied somewhere writable — use `haloreach-copy.dll`, never the Steam path).
2. Or import the DLL in the GUI and run the script from the Script Manager.

The disassembly dump used for the original analysis is reproducible too:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe" `
  /disasm:nobytes /nopdata haloreach-copy.dll > haloreach-disasm.txt
```

`docs/FORGE-NETPLAY-OFFSETS.md` has the full state-machine write-up and the patch candidates per failure symptom.
