# Forge netplay — the engine gate map (haloreach.dll)

Written 2026-10-30 after reproducing Forge netplay with two local clients. This records the join-queue state machine in
`haloreach.dll` so the next person does not have to re-derive it, and so a byte patch can be aimed precisely if a real
two-machine run still fails.

`haloreach.dll` (Steam MCC build used here): image base `0x180000000`, on-disk size 13,229,016 bytes, image size
`0x4EDA000` (82 MB, most of it `.data`). All addresses below are **RVAs** — add the runtime base (`s_reachBase`) to get the
address in a running client.

## What "forge netplay does not work" actually was

| | join mode (`+0x2A49308`) | queue count | guest join-requests | guest join-aborts (msg 10) | guest result |
|---|---|---|---|---|---|
| before the fix | **0** | 0 | 30 | 10 | never in game |
| after the fix | **1** | 1 | 1 | 0 | `game session started ... loading screen off` |
| slayer control | 1 | 1 | 1 | 0 | in game |

The host's session must be past its initial state before a join can be queued, and the campaign fix from earlier the same
day held `Command 10 (Event: Start)` back, keeping the Forge session at state 0. The fix for that is in
`BlamEngine::LaunchMap` (`holdMissionForPeers` is campaign-only now) — no byte patch needed.

## The state machine (exact instructions)

Global variables:

| RVA | meaning |
|---|---|
| `0x2A49308` | join **queue mode**: 0 = idle, 1..4 = active (which session slot the queue belongs to) |
| `0x2A4930C` | queued join count |
| `0x2A49318` | queue entries, `0x588` bytes each |
| `0x2A49074` | queue **step** index (the processor only runs at step 4) |
| `0x2A1FD40` | byte: "session pointers are valid" |
| `0x2A1FE08` / `FE10` / `FE18` | session pointers for mode 1 / 2 / 3(+4) |
| `+0x60EF8` (inside a session) | session **state**; 6 = loaded, which is what the queue requires |

### 1. Mode setter — RVA `0x39BEEC`

```
0x39BEF2  mov  eax, dword ptr [0x182A49308]     ; current mode
0x39BEF8  mov  ebx, ecx                         ; requested mode
0x39BEFA  cmp  eax, ecx
0x39BEFC  je   0x39BF0D                         ; unchanged -> nothing
0x39BEFE  cmp  ecx, 4                           ; requesting mode 4?
0x39BF01  jne  0x39BF08
0x39BF03  cmp  eax, 3
0x39BF06  je   0x39BF0D                         ; 4 after 3 is ignored
0x39BF08  call 0x39C2B8                         ; reset/initialise the queue
0x39BF0D  mov  dword ptr [0x182A49308], ebx     ; commit the mode
```

The `4 after 3` rule is the only special case: raising the queue to mode 4 while it is mode 3 does nothing.

### 2. Dispatcher — RVA `0x39A8D2`

```
0x39A8D2  cmp  dword ptr [0x182A4930C], edi     ; queue count
0x39A8D8  je   0x39A8FD
0x39A8DA  mov  ecx, dword ptr [0x182A49308]     ; mode
0x39A8E0  lea  eax, [rcx-1]
0x39A8E3  test eax, 0xFFFFFFFC
0x39A8E8  jne  0x39A8FD                         ; mode not in 1..4 -> skip the whole queue
...
0x39A903  mov  ecx, dword ptr [0x182A49074]     ; step
0x39A90B  je   ...  (step 0)
0x39A914  je   ...  (step 1)
0x39A91D  je   ...  (step 2)
0x39A922  je   ...  (step 3)
0x39A924  cmp  ecx, 1
0x39A927  jne  0x39B434
0x39A92D  call 0x39BDCC                         ; step 4: process the queue (only caller)
```

So a queued join is only moved into the session when the mode is 1..4 **and** the step reaches 4.

### 3. Processor — RVA `0x39BDCC`

```
0x39BDE1  mov  eax, dword ptr [0x182A49308]     ; mode
0x39BDEC  cmp  eax, 1
0x39BDEF  jne  0x39BE02                         ; other modes take the FE10/FE18 paths
0x39BDF1  cmp  byte ptr [0x182A1FD40], dil
0x39BDF8  cmovne rdi, qword ptr [0x182A1FE08]   ; mode 1 -> session pointer for slot 1
```

The processor then requires that session's state (`+0x60EF8`) to be exactly 6 before moving the queued player in.

## If a real two-machine Forge join still fails

Capture the runtime numbers first (the client already logs them): `LobbyPatches: join queue after request: mode N count N
... state N`. Then choose by symptom:

| Symptom | Patch candidate | Bytes |
|---|---|---|
| mode stays 0 while a guest is waiting | force the mode: at `0x39BF0D` the store, or at `0x39BEEC` make it store 1 unconditionally | `C7 05 <rel32> 01 00 00 00` |
| mode reaches 3 and then refuses to become 4 | bypass the `4 after 3` rule: `0x39BF03`..`0x39BF06` (`cmp eax,3; je`) -> NOP both | `90 90 90 90 90 90` |
| mode is not 1..4 and the queue is skipped | bypass the mode mask: `0x39A8E3`..`0x39A8E8` (`test; jne`) -> force fall-through | `90 90 90 90 90 90` |
| step never reaches 4 | the step is advanced by the session update; patch the `cmp ecx,1 / jne` at `0x39A924` to always call | `EB 07` style short jump |
| queued but never inserted | the state check (`+0x60EF8` must be 6) — patch that comparison where it is read in the processor | compare against 6 -> NOP the branch |

Do **not** patch `0x39A8D2`'s count test without care: it also guards legitimate queue draining.

## How to re-derive any of this

```powershell
# disassemble once (115 MB of text, ~2.5 M lines)
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe" /disasm:nobytes /nopdata haloreach-copy.dll > haloreach-disasm.txt
```

Addresses in that file are `0x180000000 + RVA`. A Ghidra project is also set up for symbol-level work:
`ghidra-proj\NobleReach` (imported from `haloreach-copy.dll`, Ghidra 12.1.4, `C:\tools\ghidra_12.1.4_PUBLIC`).

Quick cross-reference scan used to find every RIP-relative reference to a global (works without a disassembler):

```python
# for each byte offset: optional REX prefix, opcode, modrm with mod=00/rm=101, disp32
# target = image_base + (rva_of_instruction + instruction_length) + disp
```

That is how `0x2A49308` was located: 27 references, all inside `0x39A8xx..0x39C2xx`, i.e. one join-queue subsystem.
