# Halo: Reach Omaha Cache Build (Jul 24 2010) — Decompilation & Reverse Engineering Reference

This reference document compiles and organizes all reverse engineering findings, memory structures, engine internals, subsystem logs, and network protocol specifications extracted from the Halo: Reach Xbox 360 developer cache build (`reach halo3 cache play/test xenon 11860.10.07.24.0147.omaha_relea Jul 24 2010`).

---

## 1. Build Metadata & Environment

- **Build String:** `reach halo3 cache test xenon 11860.10.07.24.0147.omaha_relea Jul 24 2010 02:29:04`
- **Internal Branch:** `omaha_relea` (Omaha release candidate build 11860)
- **Engine Ancestry:** Halo 3 / ODST Blam! Engine evolved into Halo: Reach Blam!
- **Target Platform (Original):** Xbox 360 (Xenon 3-core 6-thread PowerPC)
- **Target Platform (MCC PC):** `haloreach.dll` 64-bit x86-64 port
- **Title ID:** `4D53085B` (Microsoft Title ID for Halo: Reach)
- **Default Game Directory Structure:**
  - `d:\maps\` (Map cache files: `mainmenu.map`, campaign, multiplayer)
  - `e:\reachrtx\reports\` (Diagnostic dumps and runtime event logs)
  - `data\` (Host user data, player profiles, screenshots, variants)
  - `game:\` (Mounted title drive)

---

## 2. Engine Memory Regions & Game State Partitioning

The Blam! engine partitions dynamic memory into dedicated game state regions (source: `gamestate.txt` and `system_log.txt`). These structs are laid out contiguously or allocated via dedicated heaps.

### A. Game State Regions (`gamestate.txt`)
| Region Name | Purpose | Key Sub-Structures & Sizes |
| :--- | :--- | :--- |
| `game_state_header_region` | Header & magic identifier | Validates save/load state compatibility |
| `game_state_update_region` | Core deterministic tick state | • `game globals`: 125,912 bytes<br>• `players`: 22,488 bytes<br>• `game engine globals`: 33,320 bytes<br>• `local game engine globals`: 56,500 bytes<br>• `havok world`: 4,736 bytes<br>• `havok proxies`: 134,784 bytes<br>• `megalogamengine`: Megalo script runtime state |
| `game_state_audio_region` | Sound emitter & playback state | Dynamic sound channel cache |
| `game_state_render_region` | Viewports, cameras, decals | Decal cache (6,396 B), render models |
| `game_state_shared_region` | Inter-thread shared state | Network synchronizer, physics contacts |

### B. Dynamic Physical Heaps (`system_log.txt`)
| Subsystem | Allocation Size | Base / Alignment | Description |
| :--- | :--- | :--- | :--- |
| `simulation priority heap` | 1,048,576 B (1 MB) | `0xAC280000` | Priority queue for packet transmission |
| `simulation profiler rockall heap` | 12,582,912 B (12 MB) | `0xAC380000` | High-frequency Rockall memory allocator |
| `simulation distributed world` | 78,168 B | `0xAD150000` | Replicated world entity database |
| `simulation distributed view data` | 1,086,208 B | `0xAD170000` | Per-client replication view buffers |
| `network-active-roster` | 1,091,624 B (~1.04 MB) | `0xAE970000` | Full 16-player active roster & profile cache |
| `network-presence` | 69,688 B | `0xAE940000` | Online presence tracking state |
| `network-party` | 11,552 B | `0xAE960000` | Matchmaking party structures |
| `network-friends` | 78,552 B | `0xAE920000` | Live friends list tracking |
| `online_files` | 307,200 B (300 KB) | `0xADF90000` | Temporary downloaded BLF/variant cache |
| `ui:memory_peak` | 1,532,000 B (~1.46 MB) | Dynamic | Peak CUI UI component allocation |

---

## 3. Crash Diagnosis: RVA `0x2744F2` & Resource Pool Internals

### Root Cause Analysis
- **Assembly in `haloreach.dll`:**
  ```x86asm
  RVA 0x274488:
  mov     [rsp+8], rbx
  movsxd  rax, edx                          ; edx = resourcePoolIndex (0xC = 12)
  lea     rbx, [rip - 0x274497]             ; rbx = haloreach.dll base address
  imul    r10, rax, 0x70                    ; r10 = edx * 0x70 (112-byte pool descriptor stride)
  movsxd  r11, ecx                          ; r11 = category (9)
  xor     r8d, r8d
  mov     eax, [r10 + rbx + 0xC8E4C0]       ; pool flags
  mov     [r11*4 + rbx + 0x04E38A10], edx
  ...
  movsxd  rax, r8d                          ; atom element index
  imul    rcx, rax, 0x88                    ; atomIndex * 136 (0x88 bytes per atom)
  mov     rax, [r10 + rbx + 0xC8E510]       ; rax = descriptor[edx].atom_array_ptr
  mov     rdx, [rax + rcx + 8]              ; DEREFERENCE: rax is NULL -> crashes at 0x8
  ```
- **Structure Details:**
  - Resource pool descriptors are stored in an array at `base + 0xC8E4C0`.
  - Descriptor stride is `0x70` (112 bytes).
  - Offset `+0x50` (`0xC8E510`) holds `atom_array_ptr`.
  - Prediction atoms are `0x88` (136 bytes) each.
  - At startup, the engine invokes **Host Slot 32** with `a2 = 0x88` (atom element size) and `a3 = 0xA0` (initial count or pool capacity).
  - If the host returns `0` or does not allocate a buffer, `atom_array_ptr` remains `NULL`, causing `mov rdx, [rax + rcx + 8]` to fault with `0xC0000005` reading `0x0000000000000008`.

---

## 4. Bungie Datum Index Handle Convention

Throughout Reach engine logs (`incident_debug.txt`, `reports/system_log.txt`), object identifiers appear as 32-bit hex values (e.g. `0xE36100AF`, `0xE36800B6`):
- High 16 bits (`0xE361`): Salt (uniqueness counter to detect stale references across entity deletion/reuse)
- Low 16 bits (`0x00AF`): Datum index into the global object table (index 175)
- Special value `0xFFFFFFFF`: NULL datum handle / none

---

## 5. Subsystem Synchronization: Critical Section Lock Hierarchy (`lock_hierarchy.txt`)

Reach enforces strict lock acquisition ordering to avoid deadlocks across worker threads. Key critical sections include:
1. `CS:Prof Session` (Profiling / instrumentation lock)
2. `CS:Resource RW Lock` / `S4:ResourceRW` (Tag and cache file resource lock)
3. `CS:Simple Resource Access RW Lock` (Fast-path tag reference reads)
4. `CS:Resource Publish` (Tag loading completion & publishing lock)
5. `CS:Network Debug` (Networking status & telemetry logging)
6. `CS:D3D_Device` (Direct3D render context lock)

---

## 6. Networking, LSP, and Title Storage Protocols

### A. Title Endpoints & Ports
- **Presence Heartbeat:**
  - URL: `/ReachPresenceApi/heartbeat.ashx`
  - Period: Every ~30 seconds (`g000629 f7723462`)
- **Title Storage Machine Files:**
  - URL: `/storage/machine/4d53085b/4/0/D6/<MachineID>/machine.bin`
  - Contents: Machine-level network statistics, ban flags (`ban flags 0x00000000`), locality chunk.
- **Hoppers & Playlist Manifests:**
  - URL: `/storage/title/4d53085b/tracked/11860/default_hoppers/dynamic_pres_hopper_statistics.bin`
  - URL: `/storage/title/4d53085b/tracked/11860/default_hoppers/manifest_001.bin`
- **LSP Service Ports:**
  - `1016`: Title Storage & Machine files HTTP
  - `1014`: Hopper statistics & live player counts HTTP
  - `1012`: Player persistence, challenges, and reward tracking HTTP

### B. Player Rewards & Progression ("Cookies")
- Halo: Reach internally refers to player Credits (cR) as **cookies**:
  - `player_rewards:persistence: Cannot get cookie total because reward state is not available`
- Daily rewards reset when the day index increments (`timezone 0 seconds offset from UTC`).
- Challenge progress chunks are bundled in the user profile and synchronized via LSP service type 5.

### C. Banhammer System
- `networking:banhammer: caught local box cheating [0]`
- Bans apply two bitfields: `ban flags` (e.g., matchmaking, forge, credit progression) and `messages` (localized notice dialog string ID).

---

## 7. Tag & Map Cache Architecture

### A. Mainmenu Initialization Sequence
1. Allocate resource cache for `mainmenu, no active zone_set`:
   - Required size: `104,288,256` bytes (~99.4 MB)
   - Optional size: `4,370,432` bytes (~4.1 MB)
   - Total available: `108,658,688` bytes
2. Activate zone set `set_menu`:
   - Structure BSP mask: `0x3` (`mainmenu_000`, `mainmenu_lobbies`)
   - Invoke `structure_bsp_resources_fixup`
3. Expand cache for UI assets:
   - Optional size grows from 4 MB -> 48 MB -> 152 MB as UI packages load.

### B. Multiplayer / Campaign Map Transition
- For multiplayer maps (e.g., `45_aftship` / Zealot):
  - Required tag cache size: `141,922,304` bytes (~135.3 MB)
  - Optional tag cache size: `103,600,128` bytes (~98.8 MB)
  - Total buffer: `245,522,432` bytes (~234 MB)
- `stream_manager` bandwidth tracking reset occurs on every map boundary.

### C. BLF (Bungie Link Format) Image Data
- Screenshots and variant icons use BLF container files.
- Embedded JPEGs begin at offset `+20` bytes from chunk start (`ui:load_jpeg_from_blf_chunk_async: skipped 20 bytes after seek`).

---

## 8. Incident System & Game Simulation

- **Pipeline:**
  1. `INCIDENT BUCKET DISPATCH`: Collects event occurrences during deterministic tick (e.g., `respawn_tick`, `player_spawn`, `kill`, `assist`).
  2. `INCIDENT BUCKET DISPLAY`: Translates events to UI announcements, medals, HUD feed messages, and audio cues.
- **Megalo Engine Object Management:**
  - Game variants run Megalo bytecode scripts.
  - When rounds end or objective resets occur, Megalo triggers clean up spawned entities:
    `megalogamengine: trigger index 1 just deleted 16 objects`

---

## 9. Controller Cheats & Console Commands (`cheats.txt` & `init.txt`)

### Controller Bindings (Developer Builds)
- `dpad-up`: Toggle debug structures, collision models, physics pills, cookie cutters, and AI paths.
- `a-button`: `game_speed 1` (normal speed)
- `b-button`: `game_speed 4` (fast forward)
- `x-button`: `(set debug_structure_automatic (not debug_structure_automatic))`
- `y-button`: `controller_invert_look`
- `left bumper`: `flying_camera_use_old_controls`
- `right bumper`: `director_cycle_debug_camera 0`

### Useful Startup Commands
- `run_like_dvd`: Forces the engine to behave as a retail optical disc / retail package rather than a development tree.
- `cheat_controller 0`: Assigns cheat input listen port to local player 0.
- `game_tick_rate 30`: Standard tick rate (30 Hz deterministic simulation rate for Reach).
- `objects_move_multithreading_enabled 1`: Enables parallel object position updates.

---

## 10. Engine Host Interface (`ReachHost`) Slot Mapping

Based on the Omaha build initialization trace and `haloreach.dll` reverse engineering:

| Slot Index | Method / Purpose | Parameters Observed | Expected Behavior |
| :---: | :--- | :--- | :--- |
| **0** | `QueryInterface` | `(GUID* riid, void** outPtr)` | Returns `S_OK` (0) and assigns interface pointer. |
| **18** (vtbl+0x18) | `PostCommand` / `EngineMessage` | `(int commandId, void* param)` | Command 10 triggers engine launch phase after ~300 ms. |
| **23** | `LoadProgress` | `(float progress, wchar_t* stage)` | Tracks map loading and compilation percentage. |
| **32** | `AllocateAtomPool` / Memory Provider | `(void** outPtr, size_t elemSize, size_t count)` | Allocates prediction atom buffer (elemSize = `0x88` / 136 B). Must assign valid pointer to prevent RVA `0x2744F2` crash! |
| **47** | `GetUserDataFolder` | `(wchar_t* outBuf, size_t maxLen)` | Returns writable user directory path (e.g. `L"data"`). |
| **48** | `ResolveFilePath` | `(const wchar_t* inPath, wchar_t* outPath)` | VFS file redirection for maps and assets. |
| **51** | `GetEngineCapability` | `(uint32_t capId, void* inOutStruct)` | Emulates MCC feature flags and platform limits. |
| **57** | `StringLookup` | `(const char* key, wchar_t* outVal)` | Localized string resolution for UI and error text. |
