# Stack overflow in the collision-hierarchy walk (0xC00000FD @ haloreach+0x150B74)

Overnight crash that appeared after the extended-limit runtime pools went live (v0.19.69/0.19.70).
Captured 2026-11-13. Fixed by `src/host/CollisionWalkGuard.cpp` (v0.19.71).

## What the log said
Two runs in a row ended like this (the second is 2 s after the Armory scene was applied):

```
[2026-11-13 07:00:16.317] [error] FATAL EXCEPTION CAUGHT: Code 0xC00000FD at Address 0x7FFCA7510B74 (+0x150B74) (Module: haloreach.dll)
 Frame 07: 0x7FFCA7510B74 (+0x150B74) [haloreach.dll]
 Frame 08..31: 0x7FFCA7510B91 (+0x150B91) [haloreach.dll]   <- same return address, 32 deep in the printed window
```

`0xC00000FD` is STATUS_STACK_OVERFLOW; the repeated `0x150B91` return address is one function calling
itself. The in-process crash handler cannot write a dump from an exhausted stack, which is why
`noble-dev-crash.dmp` was 0 bytes and the log ends mid-frames.

## Reproducing with a stack
`scratch/dbg_repro.py` launches `noble-dev.exe` as a debuggee (`DEBUG_ONLY_THIS_PROCESS`), waits for the
first-chance stack overflow, dumps the faulting thread's TEB/registers, scans its stack for return
addresses, writes a full minidump (`MiniDumpWithFullMemory`, ~4.5 GB) and kills the process.

What it showed (menu, `extendedLimits 3`, `aiActors 1024`, no stock-profile run):

* faulting thread = the main thread, `rsp` at the guard page, **9,130** frames of `haloreach+0x150B91`;
* the caller chain below the recursion is
  `0x1D6380 -> 0x150BC0 -> 0x150A30` (`0x150D67` = the `call 0x150A30` at `0x150D62` in `0x150BC0`);
* below that the engine's per-frame object/light update, i.e. a normal frame on the main thread.

## The walk
`haloreach+0x150A30` is the recursive half of the *objects near this object* query. Per node
(0x1C-byte record in the `collision hierarchy node header` pool, 6,144 slots):

| node field | use |
|---|---|
| `+0x08` | first element handle (0x30-byte record in `collision hierarchy element headers`, 7,168 slots) |
| `+0x10`, `+0x14` | child node handles, `-1` ends the walk |
| element `+0x0C..+0x18` | sphere centre / radius, compared with the query sphere |
| element `+0x28` | next element handle |
| element `+0x20` | hierarchy root node handle (used by the outer function, `0x150BC0`) |

Each level consumes `0x70` bytes of stack (`20 + 50h` prologue + return address), so a cycle is fatal in
`~1 MB / 0x70 ≈ 9,000` levels — exactly what was observed.

## The corrupted data
At the fault the walk was at node 6; the minidump's node pool showed a **live** copy and a second copy
(+0x4A0000, the engine keeps two game-state pool sets) with scattered 4-byte writes:

```
node 3: c0/c1 = 00000000 (was -1/-1)      -> child 0
node 4: record all zeroes                 -> child 0
node 5: elem   = 00000000 (was -1)        -> element 0
node 0: c0 = handle(0xEF69, 6)
node 6: c0 = handle(0xEF68, 5)
  => 0 -> 6 -> 5 -> 4 -> 0 -> ...
```

The terminator is `0xFFFFFFFF`, so a **zeroed (freed / re-initialised) node reads as "child 0"**
instead of "leaf". The pool is shared and recycled; a parent whose child handle outlives the child is
enough to make the walk loop forever. The extended object pool (2,048 -> 32,767) changes which slots the
Armory model rebuild recycles, which is why only non-stock limit profiles crashed.

## Fix
`src/host/CollisionWalkGuard.cpp` detours `haloreach+0x150A30` once at load. Per thread it tracks depth
and refuses to descend past 512 levels (or within 64 KB of `TEB.StackLimit`), returning 0 for the
subtree. A healthy hierarchy is tens of nodes deep, so results are unchanged unless the data is already
damaged. The first eight bails are logged:

```
Collision walk guard: collision hierarchy is cyclic or stale (node XXX, 512 levels deep); stopping this walk instead of overflowing the stack
```

The under-lying data problem (a stale child handle in a recycled collision hierarchy) is an engine bug;
the guard keeps it from taking the process down. If the walk ever bails in normal play, the collision
pools are being recycled in a way the engine does not expect — capture `noble-dev.log` (the guard line)
and a dump.

## Reproduce / verify
```
py -3.15 scratch\dbg_repro.py                 # runs noble-dev.exe under the debugger, ~2 min
```
Set `NOBLE_REPRO_TIMEOUT` (seconds) to change the watch window. Compare against
`extendedLimits 0` in `data/userdata/settings.json` — stock limits never hit it.

| build | configuration | result |
|---|---|---|
| v0.19.70 (before) | Full + 1024 actors | `0xC00000FD` 8-35 s after the menu, 3/3 runs (one under the debugger, 9,130 recursion frames) |
| v0.19.71 without the guard (A/B) | Full + 1024 actors | `0xC00000FD` again, same `0x1D6415 -> 0x150D67 -> 0x150B91...` chain |
| v0.19.71 with the guard | Full + 1024 actors | 3 runs (120 s, 150 s, 150 s), no crash; the walk bails at 512 levels on handles `EF670004`, `EFB30014`, `EFBF0016`, `EFD9001C` |
| any build | `extendedLimits 0` | no crash |

`EF670004` is the handle that points at the zeroed node 4 in the dump above, so the guard is stopping
exactly the loop that used to end in the stack overflow.

