# Machine map — every path that matters

Claude (or any new agent) has no memory of this PC. This file is the map: what lives where, why you would go there, and
what is *not* in the repo. Verified 2026-10-31.

## Repositories and checkouts

| path | what it is |
|---|---|
| `D:\Halo-Reach-Community-Edition` | **main client checkout** (DeepSeek works here). Build output in `build\bin\Release` |
| `D:\Halo-Reach-Community-Edition-claude` | **Claude's second checkout** (separate working tree so both agents can work at once). Starts with **no build output** — see "Second checkout" below |
| `C:\Users\tyler\Documents\deepseek-harness\default-workspace\ServerTestDeploy` | **current backend clone** (`server.js`, `src/`, deployed to Render) |
| `D:\ServerTestDeploy` | older backend clone made by `tools/push-server.ps1`; still valid, but prefer the workspace one |

Remote: client `https://github.com/O7OF/test-build-01`, backend `https://github.com/O7OF/ServerTestDeploy`,
live backend `https://outbreak-z-test-server.onrender.com` (`/api/health`, `/api/relay/stats`).

## The game and its data

| path | what it is |
|---|---|
| `D:\Steam\steamapps\common\Halo The Master Chief Collection` | MCC install; `haloreach\haloreach.dll` (13,229,016 B, SHA-256 `738DD2D2…0894`) is the engine we host |
| `D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps` | shipped maps (`m10.map`, `m20.map`, `forge_halo.map`, …) |
| `D:\Steam\steamapps\workshop\content\976730` | Steam Workshop mods (per-mod `ModInfo.json`) |

## Repo runtime output (not in git)

| path | what it is |
|---|---|
| `D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.exe` | the client (2.8 MB) |
| `…\build\bin\Release\data` | runtime data: `launch_block_1.bin`, `lb_m_c_1.bin`, `blocks\campaign_*.bin`, `menumaps\`, `ui\` art (~2.2 GB) |
| `…\build\bin\Release\data\userdata\settings.json` | player settings — **`playerName` gets clobbered to `Bob`** by account-less runs |
| `…\build\bin\Release\data\userdata\career.json` | credits (lifetime) / `creditsSpent` / `bonusCredits` (spendable) |
| `…\build\bin\Release\data\appearance.json` | live Armory profile — back it up before touching ownership; `appearance.before-reset-20261030.json` is the pre-reset look |
| `…\build\bin\Release\*.log` | latest run logs (`noble-client.log`); test logs like `tp1.log`, `sb3.log` |
| `…\build\bin\Release\capture_*.bmp` | capture-harness screenshots (`capture_888888.bmp`, `capture_777777.bmp`) |
| `D:\Halo-Reach-Community-Edition\docs\screenshots\` | the captures that are committed as evidence |

## Tools installed on the PC

| path | what it is |
|---|---|
| `D:\Steam\steamapps\common\HREK` | Halo Reach Editing Kit: `Foundation.exe` (tag editor), `sapien.exe`, `tool.exe`, `MegaloEdit.exe`, extracted tags (~39 GB). Tag work needs Foundation → *Switch Current Project* → **Switch my project** first; `tool.exe` needs **absolute** tag paths |
| `D:\Reach` | **Reach Renaissance install**: `bin\host-probe.exe` (the 11.7 MB Rust host, COFF symbols, the camera/capacity/coopx reference), `bin\menu-overlay.exe`, `haloreach_copy.dll` (byte-identical to the Steam DLL), `data\lb_m_c_1.bin` + captured launch blocks, `README.txt`, `VERSION.txt` |
| `C:\tools\ghidra_12.1.4_PUBLIC` | Ghidra 12.1.4 (`ghidraRun.bat`); 12.2 also present |
| `C:\Users\tyler\Documents\deepseek-harness\default-workspace\ghidra-proj\NobleReach.gpr` | ready-made Ghidra project with the Forge join-queue gate labelled/commented (program `haloreach-copy.dll`); see `docs/GHIDRA-SETUP.md` |
| `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe` | the CMake used for every build command in the docs |

## Reverse-engineering material

| path | what it is |
|---|---|
| `D:\REACH REVERSE ENGINEER\docs\rr-launch-flow.md` | RR launcher flow + **§6 patch list** (capacity/coopx/camera work, env knobs) |
| `D:\REACH REVERSE ENGINEER\docs\rr-tags.md` | tag formats, launch block slot sizes |
| `D:\REACH REVERSE ENGINEER\docs\host-probe-functions-so-far.txt` | list of analysed RVAs in `host-probe.exe` |
| `D:\REACH REVERSE ENGINEER\docs.zip` | evidence bundle: pseudocode of `new_launch_block`, `host::hit`, `set_map`, `main`, … |
| `C:\Users\tyler\Documents\deepseek-harness\default-workspace\rrdocs\docs\evidence\pseudocode\` | that zip already extracted (the `.c` files are easier to read than the zip) |
| `C:\Users\tyler\Documents\deepseek-harness\default-workspace\haloreach-disasm.txt` | **117 MB text disassembly of `haloreach.dll`** — RVAs are `VA - 0x180000000`, e.g. the third-person check at `0x48A910` is `0x18048A910` in this file |
| `C:\Users\tyler\Documents\deepseek-harness\default-workspace\hostprobe-symbols.txt` | `dumpbin /symbols` of `host-probe.exe` (8.7 MB); values are **section-relative**: `.text` starts at RVA 0x1000, so `camera::install` value `0x195970` → VA `0x140196970` |
| `C:\Users\tyler\Documents\deepseek-harness\default-workspace\*.py / *.ps1` | throwaway analysis scripts from earlier sessions (disassemblers, launch-block dumps) |

## Handoffs, docs and packages

| path | what it is |
|---|---|
| `docs\START-HERE.md` · `docs\TODO.md` · `docs\TEAM-WORKFLOW.md` · `docs\AGENT-STATUS.md` | read first (see `CLAUDE-PROMPT.txt`) |
| `docs\HANDOFF.md` | the chronological log (newest entry first) |
| `docs\HANDOFF-FOR-CLAUDE.md` | long-form brief |
| `docs\menu-redesign-spec.md` | menu design contract |
| `docs\player-count-scaling.md` | RR >4-player reference + staged plan |
| `C:\Users\tyler\Downloads\HANDOFF (2).md` and friends | **mirrors** of the docs for reading outside the repo (HANDOFF, HANDOFF-FOR-CLAUDE, PROJECT-NOBLE-CONTINUATION, menu-redesign-spec, START-HERE, TODO, TEAM-WORKFLOW, AGENT-STATUS, CLAUDE-PROMPT) |
| `C:\Users\tyler\Downloads\Noble-Claude-Package-2026-10-31.zip` | self-contained package of the above + screenshots |

## Second checkout (Claude's tree) — first run

`D:\Halo-Reach-Community-Edition-claude` is a clean clone with no `build\`. To make it runnable without disturbing the main
tree:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S . -B build -G "Visual Studio 17 2022" -A x64
```

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build --config Release -- -m
```

Then copy the engine data over (it is not in git, ~2.2 GB):

```powershell
robocopy "D:\Halo-Reach-Community-Edition\build\bin\Release\data" "D:\Halo-Reach-Community-Edition-claude\build\bin\Release\data" /E /NFL /NDL /NJH /NJS
```

After that the second checkout runs the same as the main one. Announce it in `docs/AGENT-STATUS.md` so the other agent
knows not to use its build.

## Things that are NOT in this list but get searched for

- **No MCC menu screenshots** — the front-end is ours; use `-menu-capture`.
- **No engine PDB** — crash addresses are RVA-only (`hr+0x…`).
- **No original Reach 360 emblem asset sets** — see the emblem item in `docs/TODO.md`.
- **No compiled HREK project files committed anywhere** — Foundation's project state lives inside HREK itself.
