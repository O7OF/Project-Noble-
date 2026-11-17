# Modder guide - Noble hooks, launch flags and game options (v0.19.103 / current dev tree)

Written 2026-11-15 from the code, the live startup log and the stabilization QA. This documents what Noble actually
puts into the game and what a modder can drive from the outside.

**There is no public mod API yet.** Noble does not expose a scripting surface for third-party code in this build:
the items below are (a) internal engine hooks/patches Noble applies, (b) launch/test flags, and (c) player-facing game
options that mods/variants can influence through the normal lobby/variant data. Treat every hook as internal and
version-specific: it is matched against **haloreach.dll 1.3528.0.0** (SHA-256 `738dd2d2...0894`) and fails safe if the
bytes do not match. Networking and anti-cheat are out of scope (see `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`).

**Verification labels**
- **VERIFIED** - exercised in a real run and observed working (evidence: the 2026-11-15 QA or an earlier recorded test).
- **INSTALLS** - the hook/byte-patch installs and is signature-checked at every start, but its behaviour was not
  re-proven in the recent tests.
- **DEV-ONLY** - test/diagnostic code; not for players or public builds.
- **UNVERIFIED** - code exists but there is no test evidence (documented here so nobody assumes it works).

## 1. How Noble hooks the engine

- **Detours**: MinHook, installed by `HookManager` right after `haloreach.dll` loads. Targets are RVAs (`hr+0x...` =
  address - 0x180000000 in the Ghidra export). Each install checks prologue bytes/signatures first; on a mismatch Noble
  logs a warning and skips the hook rather than crashing.
- **Byte patches**: small signature-checked writes (crash fixes, cap gates, feature switches) applied before/after the
  hooks. Also fail safe.
- **Patched DLL profiles (EngineLimits)**: Noble builds its own patched copy of the player's `haloreach.dll` from
  `data\limits\{v16,v16p,full}.nlp` and loads that; the installed file is never modified. **Currently disabled**:
  upstream commit `4407101` resets the setting to Stock and forces Stock for non-Forge launches (stability).
- **Mod content**: the VFS serves `mods\<folder>` (Steam Workshop items and local mod folders) over the stock file
  paths; `-mod <folder>` selects a mod folder; `-map-override <file.map>` serves an arbitrary map file as a scenario;
  `-mvar <file.mvar>` injects a Forge map variant; `-variant <file>` loads a game variant.
- **Engine-state tooling**: `PoolReport` (live pool inspector), `PoolResize` (pool-size rules), `SpawnTest` (AI/object
  stress), `EngineLimits` (profiles), `SchedulerGuard`/`CollisionWalkGuard` (crash guards).

## 2. Engine hooks installed every run (68 detours)

RVAs below were measured from a live startup log (base `0x7FF9477F0000`) on v0.19.103. Hooks that target memory outside
the DLL are listed without an RVA.

### 2a. Crash / stability patches and guards

| Hook / patch | hr+RVA | What it does / when it fires | Limits / status |
|---|---|---|---|
| `haloreach_is_whitespace` detour | `0x362E8` | Replaces the engine whitespace check; fires on every text parse. Works around an underflow crash. | INSTALLS; prior session recorded the crash as fixed |
| armour-composite fallback jump patch | `0x105324` -> `0x1055C1` | Byte patch (je to jmp) on the armour composite path; applied at startup. Fixes crash at `0x105597`. | INSTALLS; crash fix from prior session |
| null-biped dereference patch | `0x99A0D` | Installs a trampoline for the null-biped dereference. | INSTALLS |
| Firefight worker-thread null TLS actors patch | `0x4E9F5C` | Trampoline; prevents the Firefight worker thread from dereferencing a null actor TLS (check `test rcx; jz skip` guards the per-actor table). | INSTALLS; behaviour **UNVERIFIED** (no test/log evidence, only the introduction commit) |
| UI interface null-slot guard | `0x41E3E3` | Trampoline guard for a null UI interface slot. | INSTALLS |
| `collision_walk_guard` detour | `0x150A30` | Guards the collision-walk path; fires during collision walks. | INSTALLS |
| `cinematic_skip_stop_internal` detour | `0x1ADE80` | Fires when a cinematic skip stops; used by the test cutscene-skip and to keep state consistent. | INSTALLS |

### 2b. Gameplay hooks (modder-relevant)

| Hook | hr+RVA | What it does / when it fires | Example use | Limits / status |
|---|---|---|---|---|
| `haloreach_player_control_update` | `0x5DF80` | Detour on player_control_update(playerIndex, inputUserIndex, dt, outA5, controlOut); fires every frame per player. Noble pulses control bit 10 for the bindable Lower weapon / Alert Carry action. | Bind Lower weapon in Settings > Controls. | Hook installs; key bind shipped (prior evidence) |
| Alert-Carry availability byte patch | `0x5D6BA` | Patch (74 02 -> 90 90) of the Machinima availability check so Alert Carry works in all sessions. | - | INSTALLS |
| `haloreach_ai_place` | `0x6C2B10` | Detour on ai_place(aiValue, count, flag); fires whenever a scenario/script places AI. In distributed multiplayer Noble suppresses duplicate local clones. | Use `-horde` to call it from the dev harness. | VERIFIED in game 2026-11-15 (horde placed actors) |
| `haloreach_observer_channel_send_message` | `0x3F9620` | Immediate observer-channel flush hook; fires on observer-channel sends. Used for lower input latency in co-op/MP. | - | INSTALLS |
| `object_cause_damage` / `object_kill` (DevCheats) | `0x1ED320` / `0x47CAA8` | Detours used by the dev cheat menu (godmode/kill helpers). | Dev-account hotkeys. | DEV-ONLY |
| `tag_get` (SharedTagCache) | `0x81BE8` | Tag lookup hook used by the shared tag-cache probing (visor/shared-heap diagnostics). | Only with the relevant probe env flags. | DEV-ONLY / UNVERIFIED in normal play |
| `spawn_test_tick` | `0x61A38` | Per-player control tick hook (unitHandle, playerIndex, c, dt1, dt2); fires every frame. Drives `-horde`, `-spawn-test`, dev cheats and the cinematic auto-spawn gate. | `-horde 100`, `-spawn-test 500`. | VERIFIED 2026-11-15 (fixed by `d636b31` to gate on playerIndex) |

### 2c. Forge hooks and patches (AI/objects in Forge)

| Hook / patch | hr+RVA | What it does / when it fires | Limits / status |
|---|---|---|---|
| `forge_object_new` detour | `0x46D6E0` | Detour on object_new (also used by the dev spawner); fires whenever an object is created, including Forge placement. Feeds ForgePlacement logging. | INSTALLS |
| Forge edit-mode check patch | `0x692CA5` | NOPs the check that aborts spawning unless the player is holding an object in Monitor edit mode. | INSTALLS; enables Forge spawning from menus/hotkeys (dev feature) |
| Forge AI biped/vehicle spawn patch | `0x692CB5` | NOPs the check that blocks AI bipeds/vehicles/non-standard palette objects in Forge. | INSTALLS; allows AI in Forge (AI Battleground-style mods); no full AI menu was added |

### 2d. Appearance / armour hooks and patches

| Hook / patch | hr+RVA | What it does / when it fires | Limits / status |
|---|---|---|---|
| Visor translation patch | `0x2EDC5E` | Replaces `call 0x3B2F4; mov edx,eax` with `lea edx,[rcx-0x9D]` so profile visor item ids 157+ map straight to indices 0-29 (30-finish path); fires while the player appearance is applied. Later work established that only **five** engine visor slots exist, so the shipped behaviour maps extended finishes to the nearest stock slot (`96fd2c4`, `8f467e1`); the 30-finish direct path is legacy. | Patch installs; 5-slot fallback mapping verified in game; 30-finish direct behaviour not re-proven |
| IsArmorItemUnlocked patch | `0x2C7DCC` | Forces mov eax,1 / ret so the local player's armour items are treated as unlocked (also used for network replication). | INSTALLS; part of the Armory/unlock work |
| Elite armour-effects tag list patch | `0x10580C` (0x8C to 0x80) + `0x105837` (mov r8b,1) | Enables the Elite armour-effect tag list in game. | INSTALLS |
| `elitepack_resource` (15 hooks) | `0x1BB6C0` ... `0x1BBDA0` | Resource/tag hooks for the ElitePack side-loaded map (elite_armor.map); fire while the pack's tags are resolved. | INSTALLS; ElitePack loading proven in prior sessions |
| `elitepack_cache_step` | `0x30698` | Tag-cache step hook for ElitePack. | INSTALLS |
| `elitepack_zone_rebuild` | `0x8EAF0` | Zone rebuild hook (also used by the old Trooper experiment). | INSTALLS |
| `elitepack_apply_profile` | `0x2EDA08` | Profile/appearance apply hook. | INSTALLS |
| `elitepack_settings_sync` | `0x2C837C` | Settings sync hook. | INSTALLS |

### 2e. Camera / UI / rendering hooks

| Hook | hr+RVA | What it does / when it fires | Limits / status |
|---|---|---|---|
| `camera_mode_check` | `0x48A910` | Camera-mode gate used by the third-person camera feature. | VERIFIED prior sessions (third person works) |
| `unit_third_person` | `0xC6948` | Third-person camera update hook. | VERIFIED prior |
| `first_person_camera_update` | `0xC6EE8` | First-person camera update hook (FOV offset mods). | VERIFIED prior |
| `camera_follow` / `camera_following_update` | `0x1DCADC` / `0x1E2028` | Camera follow hooks (third-person / front camera test). | `-third-person-front` test-only; verified for captures |
| `ui_cursor_screen` | `0x2D2018` | UI cursor screen-space hook; attaches Noble's ImGui cursor to the engine UI. | VERIFIED (menus clickable in QA) |
| `ui_widget_mouse_input` | `0x34E774` | UI widget mouse-input hook; forwards mouse input to Noble's menu layer. | VERIFIED (menu clicks work) |
| `subtitles_enabled` | `0xD5244` | Reads/forces subtitle enable state (Noble's subtitle setting). | INSTALLS |
| `dxgi_swapchain_present_settings` | external (DXGI) | FPS limit / VSync / present settings hook on the swapchain. | VERIFIED (settings exist; FPS counter/limit in QA captures) |

### 2f. Networking / session hooks (Claude's lane - do not build on these)

These are listed for completeness only. Networking/security is explicitly out of scope for the DeepSeek lane; any change
here goes through Claude and `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`.

| Hook / patch | hr+RVA | Purpose |
|---|---|---|
| can_accept_player_join_request | `0x3973BC` | Join admission (expanded-lobby work) |
| can_accept_join_request | `0x3975D8` | Join admission |
| acknowledge_join_request | `0x397914` | Join acceptance |
| join-any gates | `0x397635`, `0x397363` | Byte patches (74 -> EB) |
| co-op player-cap gates | `0x40954A`, `0x40955B`, `0x411B6E`, `0x411D71`, `0x397689` | Byte patches (74/76/7E -> EB) |
| evaluate_coop_insertion_point | `0x39CE10` | Co-op join insertion point |
| c_network_session::abort | `0x44EBA0` | Session abort handling |
| c_network_session::process_host_handoff | `0x4035DC` | Host handoff |
| c_game_engine::abort_game | `0x3ABB70` | Game-end handling |
| c_game_engine::set_game_ended | `0x3C08B4` | Game-end handling |
| c_saved_film_manager::session_monitor | `0x3ABE70` | Saved-film playback session monitor (VERIFIED by film playback 2026-11-15) |
| message/lobby traces | `0x423D88`, `0x423E40`, `0x4234E4`, `0x395DE8`, `0x3AFFB4`, `0x393830`, `0x397048`, `0x411F38`, `0x39E47C`, `0x41F510`, `0x133320`, `0x7BA58`, `0x59330`, `0x43B98C`, `0x43BAE8`, `0x429AA4`, `0x425368`, `0x42459C` | Encode/decode/session/refusal tracing |
| LobbyPatches summary | - | Target MP 128, co-op 16, 17 detours/patches (runtime log). The join gates normalise refusals/aborts to accept the join; several slot-count/peer-capacity patches are **pattern-scanned (no fixed RVA)** and fail safe on a mismatched DLL. The 128/16 targets are NOT proven with real players; the UI is clamped (MP 32, campaign 4). |

### 2g. Diagnostics / tracing (DEV-ONLY)

These install every run but only log: `trace_engine_main_loop` `0xC390`, `trace_map_parse` `0x31204`,
`trace_map_load` `0x141C0`, `trace_stage1` `0xD564`, `trace_stage2` `0xC8F8`, `trace_stage3` `0x3873C`,
`trace_exitprocess`, `trace_exitthread`, `trace_freelibexit`, `trace_postquit`, `trace_write_file` (external CRT),
`capture_engine_output`, `capture_engine_output_w`, `capture_mvar_serialize` `0x17AC38`, `capture_mvar_copy` `0x6C290`,
`capture_mvar_source` `0x6BCEC`.

### 2h. Limits / guard / scheduler hooks (Extended profile or dev builds only)

| Hook / patch | hr+RVA | What it does / when it fires | Limits / status |
|---|---|---|---|
| `ai_pool_resize` (`data_array_new` detour) | `0x12FCC` | Intercepts every game-state pool creation at map load and raises pools per profile / `-ai-actors`; also players / fire-team / simulation-view 16 -> 32 when SlotPatch is active; heap-backed fallback allocator. | Installs only when Extended, `-ai-actors > 96` or a 32-slot request is present; Extended is disabled in release builds today. |
| `ai_update_stack` | `0x70ABBC` | Commits stack pages before the enlarged AI update-scheduler frame runs (the patched frame has no `__chkstk`). | Extended only; INSTALLS, per-run behaviour not verified |
| `ai_spawn_search_stack` | `0x76E814` | Same stack-page commit before the AI spawn-candidate search frame. | Extended only |
| `gs_failure_report` | `0x788A78` | Logs function + call stack before the engine's stack-cookie fail-fast kills the process. | Extended only |
| `scheduler_guard_prepare` | `0x68ADD0` | Before the object-job prepare, if the free-job queue is short of `count+64`, calls the engine's own run-job helper (up to 3 s) to drain finished jobs. | Installs only when the V16 scheduler prologue matches. |
| `collision_walk_guard` | `0x150A30` | (also in 2a) Bounds the recursive "objects near object" walk: depth >= 512 or near the TEB stack limit returns 0 instead of overflowing the stack. | V16 builds; documented fix for the 0xC00000FD collision-walk crash. |
| Loader module-name IAT patch | n/a (`GetModuleFileNameW` IAT) | Redirects the engine's own module-name query to the patched DLL copy Noble loads, so the engine reports the right identity. | Startup, all profiles |

Not itemised above: the 15 `elitepack_resource` detours, `tag_get` / `object_cause_damage` / `object_kill` (dev), SlotPatch's 16 -> 32 layout writes, and the `shared tag cache` probe hooks - see their modules for details.

## 3. Engine-state tooling (how to inspect what you are about to hit)

### Pool inspector - `PoolReport`
- **What:** scans committed memory for the engine's data-array headers (signature `0x64407440`), logs one
  `POOL|name|used|capacity|recordSize|live` line per pool plus the 8 game-state banks (`BANK|n|used|capacity`).
- **When:** the reporter starts after the DLL loads; verbose once, then when a pool's capacity/usage changes (15 s).
- **Use:** run a build with `-pool-report-exit <seconds>` - it logs a full report after N seconds and exits.
  Example: `noble-dev.exe -map ff_unearthed -mode firefight -pool-report-exit 150 -log ff.log`.
- **Limits:** dev builds only; finds pools by signature scan, so a changed engine build can hide pools; a small number
  of capacities are map-dependent (page-reference/script-node caps differ per map - measured on Unearthed vs Forge World).
- **Proven:** live in-match, 124 pools + 8 banks, 2026-11-15.

### Pool resize - `PoolResize`
- **What:** by-name pool resizing hooked at data_array_new (hr+0x12FCC): actor/prop/squads/etc. plus optional
  render-state/script pools.
- **When:** when a game-state exists, if the active Engine Limits profile is Extended.
- **Use:** `-ai-actors 256|512|1024` (or the NOBLE_AI_ACTORS environment variable), `-ai-pools`, `-pool-trace`,
  `-render-states N`.
- **Limits:** **ignored on Stock**, and Extended profiles are currently disabled in release builds. Raising one pool
  alone can hit bank-size or network/determinism limits - use the pool report first.
- **Verified:** historically (2026-11-08) actor 1,024 and object 32,767 passed test-limits.ps1; not usable today.

### Engine Limits profiles - `EngineLimits`
- **What:** builds a patched haloreach.dll copy from `data\limits\{v16,v16p,full}.nlp` (V16 extended limits;
  V16+Particles; Full = V16+particles+light volumes) and loads it. The installed DLL is never touched; on any hash/row
  mismatch it falls back to Stock with a reason.
- **Use:** Settings > Game Options > ENGINE LIMITS (currently information-only STOCK) or `-ai-actors` for the AI count.
- **Limits:** disabled by upstream commit `4407101` (Settings.cpp resets to Stock; non-Forge launches forced Stock)
  because non-stock profiles caused black-screen loads and network kicks. Re-enable only after the stability and
  networking work; do not ship a mod that assumes it.

### Heavy-load dev harness - `SpawnTest`
- **What:** engine-thread test harness that can place every squad in a map, teleport/retarget actors, spawn object
  copies, and toggle a Spartan crowd.
- **Flags:** `-horde N` (queue N actors), `-horde-delay S`, `-horde-team T`, `-horde-infantry`, `-horde-full-lod`,
  `-horde-no-teleport`, `-spawn-test N`, `-spawn-max N`.
- **When:** after gameplay starts (gameplay detection: game-state cinematic flag, or `-horde-delay` override).
- **Use:** `noble-dev.exe -map ff_unearthed -mode firefight -horde 100 -horde-team 0 -horde-delay 20`.
- **Limits:** needs AI squads in the scenario (works on Firefight/campaign maps, **not** Forge World); needs a placed
  weapon/biped to copy for `-spawn-test`; respects actor/object pool capacity and logs caps/refusals; dev builds only.
  Hotkeys (Numpad 6/7/8/9) need Num Lock and the game window focused.
- **Verified:** 2026-11-15 (after `d636b31`): horde placed actors, 1,495 objects spawned.

### Other host modules
- `SchedulerGuard` - guards the engine scheduler path (stability).
- `CollisionWalkGuard` - the collision_walk_guard detour above.
- `MemberTrace` - `-trace-members` latency/member tracing (DEV-ONLY).
- `FrameCapture` - test screenshots: `-capture-at <seconds[,seconds]>`, `-capture-frames <frame,...>`,
  `-quit-after <seconds>`; writes `capture_<frame>.bmp` next to the exe.
- `MvarDump` - map-variant tools: `-mvar-parse`, `-mvar-save`, `-mvar-write`, `-mvar-roundtrip`, `-mvar-dump-at`,
  `-mvar-dump-every`, `-mvar-live-dump`, `-mvar-page-scan`, `-mvar-import`; see `src\mvar`.
- `ForgePlacement` - records Forge placements through the forge_object_new hook.
- **FOV** - not a hook: `CameraFov` writes the engine FOV global at `0xB42C10` (float degrees, stock 78; clamp 1-150) every 250 ms, match-only (no-op in the front-end/Armory). Harness: `-fov-test-at <s> <deg>` / `-fov-apply-test-at <s> <deg>` (these write settings.json).

## 4. Launch flags worth knowing (modder/tester)

### Content and session
| Flag | Value | What it does |
|---|---|---|
| `-map` | map name | Launch a map directly (e.g. m10, ff_unearthed, 20_sword_slayer, forge_halo) |
| `-mode` | campaign / firefight / multiplayer / forge | Launch mode |
| `-mod` | folder | Use a mod folder (VFS serves its files) |
| `-map-override` | path.map | Serve an arbitrary map file as the scenario |
| `-variant` / `-variant-override` | file / patch | Load a game variant / apply a variant patch |
| `-mvar` | path.mvar | Inject a Forge map variant at load |
| `-film` | path.film | Play a saved film; map/mode are resolved from the film |
| `-skulls`, `-difficulty` | int | Skulls and difficulty for the launch |
| `-max-players` | int | Override lobby size (dev; UI clamps apply in normal menus) |
| `-host` / `-lan` / `-online` / `-public` / `-private` | - | Host a session |
| `-connect`, `-session-id`, `-session-token`, `-relay`, `-local-port` | - | Join/relay (networking lane) |
| `-wait-peers`, `-loading-roster` | int / csv | Test the loading screen / waiting-for-players state |
| `-no-update`, `-no-mod-dll`, `-no-elite-pack` | - | Test switches (skip updater / mod DLL / ElitePack) |
| `-data-dir`, `-log` | path | Data folder / log file |

### Capture and menu automation
| Flag | What it does |
|---|---|
| `-capture-at <s[,s]>` / `-capture-frames <f,...>` / `-quit-after <s>` | Screenshots and timed exit |
| `-menu-screen <name>` | Open a menu screen (main, campaign, firefight, custom, forge, servers, settings, armory, theater, social, lobby, ...) |
| `-menu-capture <frames>` | Capture the menu once ready |
| `-menu-click-at <s> <x> <y>` (repeatable), `-menu-wheel-at <s> <notches>` | Inject clicks/wheel into the menu (works unfocused) |
| `-menu-tab <n>`, `-menu-ignore-input`, `-menu-debug-layout` | Menu tab / input / layout debugging |
| `-settings-search <text>` | Pre-fill Settings search (captures) |
| `-invite-code-selftest`, `-invite-code-make <session>`, `-join-code-test <code>` | Lobby-code codec/UI testing |
| `-server-browser-demo`, `-lobby-demo <host/guest> ...`, `-social-demo`, `-mod-prompt-demo` | Fake roster/UI captures without network |
| `-armory-*` (focus, cycle, species, look, scene, test colours, validate) | Armory screen automation for captures |
| `-drawer-filter-test <text>`, `-drawer-category-test <n>` | Map-picker drawer filter/category prefills |
| `-scoreboard-at <s>`, `-pause-at <s>`, `-postgame-*`, `-fake-pad[ -y|-amount]`, `-pad-press[-at|-ms]`, `-pad-script` | In-game state capture / synthetic input |
| `-engine-cmd <cmd>` / `-engine-cmd-at <s> <cmd>` | Run an engine command at launch / at N seconds (dev) |
| `-crash-test` | Throw an unhandled exception to test the crash handler |
| `-profile <path>`, `-capture-profile` | Performance capture (dev) |

### Heavy-load / diagnostics
`-horde*`, `-spawn-test`/`-spawn-max`, `-ai-actors`, `-ai-pools`, `-pool-trace`, `-render-states`,
`-pool-report-exit <s>`, `-trace-members`, `-team-probe`, `-mvar-*`, `-variant-dump`, `-voice-probe`,
`-live-window-test`, `-slot-patch`/`-slot-players`, `-end-round-after`, `-forge-save-at`, `-guest-post-start`.

### Account/session plumbing (do not use in mods)
`-account-user/-token/-dev/-tester`, `-user-id`, `-player-name`, `-service-tag`, `-remember-account`,
`-steam-subscribe/-dll/-progress`, `-parent-pid`, `-container-hwnd`, `-relaunch-test`, `-settings-export/-import`,
`-first-start`, `-launch-poke`, `-variant-poke`, `-lobby-*` automation flags (see Appendix A).

**More dev flags:** `-test-map-gametypes` (headless self-tests), `-variant-poke`/`-launch-poke` (byte patches into the decoded variant/launch block), `-slot-patch`/`-slot-players 32` (16 -> 32 slot layouts), `-elite-pack-*`/`-no-elite-pack`, `-emblem-*`, `-fov-test-at`/`-fov-apply-test-at` (write settings.json), `-no-fp-override`, `-pad-script`. `-pool-report-exit`, `-armory-validate`, `-armor-effect-test` and the armory cycling flags are compiled out of release builds (NOBLE_DEV_BUILD).

**Interactions to watch:** `-menu-*` / `-capture-*` / `-quit-after` are test-only and can leave the game in odd states;
`-horde`/`-spawn-test` stress pools and can hit the ceilings the pool report measures; `-map-override`/`-mvar` change
what the engine loads (never in a public session); `-max-players`/`-engine-cmd` are dev overrides that bypass the UI clamps.

## 5. Game options a modder can set (and what is not exposed)

### 5a. Game variants (Custom Games / Forge / Firefight) - 38-rule editor and on-disk format
- **Where:** Custom Game / Firefight / Forge setup screen -> GAME OPTIONS. The editor exposes the game's own named
  choices for 38 rules (teams, score/time/rounds, respawn, shields & health, weapons/damage, movement, appearance,
  sensors - see `src\engine\VariantRules.cpp`). Presets: CLASSIC, SWAT, SNIPERS, ROCKETS, SWORDS.
- **When/how it applies:** the selected base variant is decoded, the edited rules are merged into it, and the result is
  what host and guests load (host shares the variant with guests). Rules are clamped to the game's own choice lists and
  invalid values are dropped on load.
- **Saved modes:** `<user storage>\game_modes\<sanitised name>.json` with
  `{"name": "...", "base": "<shipped .bin>", "rules": {"<key>": <int>}}`. Only a bare file name in `base`.
- **Modder shortcut:** test a rule directly without the UI with `-variant-poke <offset>=<value>` (e.g.
  `-variant-poke 319=5,2BE=2`) or `-variant-patch` (VariantFields), applied before the launch block is built.
- **Limits / unverified:** each rule's in-game effect was not individually verified in the stabilization pass; the
  Firefight game type ignores some rules (e.g. grenade regeneration is documented as not applied in Firefight), and the
  variant is shared between peers, so treat this as networking-adjacent (Claude lane) for multiplayer sessions.

### 5b. Firefight
- Setup screen: 10 maps, 7 stock Firefight game types (+ player-made presets based on the `hr_ff_*` variants),
  MAX PLAYERS 4/8/12/16 (default 16), DIFFICULTY, and the same 38-rule GAME OPTIONS editor on the selected type.
- **Skulls are forced to 0** for Firefight launches (no skull row); teams are off; the lobby has no difficulty/skulls/
  max-players rows (they are fixed before hosting) and its GAME OPTIONS row is read-only ("DEFAULTS").
- **No** enemy-per-wave, friendly-AI-count or wave-pacing options exist (planned Phase-2 Stage 4, not built).
- **UNVERIFIED:** co-op above 4 players; each rule's Firefight behaviour.

### 5c. Appearance data (Armory)
- **Where it is saved:** the encrypted profile `data\appearance.dat` plus `data\userdata\armory-selection.json` (the
  copy match processes read). Both are Noble formats; mods that alter appearance should go through the Armory or the
  profile path rather than editing these files by hand.
- **What is selectable:** species (Spartan/Elite/Marine preview), Spartan armour pieces, Elite sets/pieces, primary/
  secondary colours (incl. custom wheel -> nearest palette index), 30 visor finishes (mapped to 5 stock slots in-match),
  7 stock armour effects (item ids 166-176), 12 Firefight voices, nameplates/emblems/emblem colours, service tag.
- **Rank/credit gates:** items can be rank-locked or cost cR; a dev build can bypass with `-armory-unlock-all`.
- **Not selectable / not implemented:** new recoloured effect variants (dev palette proof only), Elite lights colour
  row (hidden), Spartan lights/accent rows, undersuit colour (removed), exact per-player visor RGB, Elite lighting/shine.
- **Marine:** the Marine tab saves a Marine look; it is applied to the local character only when a mod spawns you as a
  Marine and not when other peers are in the session (lockstep safety).

### 5d. Settings and engine limits
- Settings are stored in `data\userdata\settings.json` (see `src\common\Settings.hpp`); `-settings-export/-import` moves
  them between machines. The stabilization pass did not verify each row's in-game effect one by one.
- **ENGINE LIMITS is Stock-only in this build**: upstream `4407101` resets the value on load and forces Stock for
  non-Forge launches; the UI row is information-only. Do not build a mod that assumes V16/V16Particles/Full.
- AI ACTORS (96/256/512/1024) is only active with Extended limits, i.e. disabled today.

### 5e. Mod content and maps
- **Mods:** `mods\<folder>` via the VFS (Steam Workshop items and local folders); select with `-mod <folder>`. Steam
  Workshop items are discovered through the Workshop cache/manifests; `ModCatalog` scans mod folders for their maps.
- **Maps:** `-map-override <file.map>` serves any map file as a scenario (used to test rebuilt maps); Forge variants can
  be injected with `-mvar <file.mvar>` (see `-mvar-*` tools).
- **Unverified:** the exact `modinfo.json` schema/mod packaging contract is not documented here, and the `.mvar`
  round-trip tooling has not been accepted-tested in the recent QA - check before shipping a mod that relies on it.

### 5f. Not exposed at all
No scripting/mod API for third-party code; no native team/spawn hook API yet; no 128-player session support; no
runtime navmesh baking; no Megalo compiler; no Firefight AI/wave options. Anything in the master list marked
NOT STARTED is not available.

## 6. What could not be verified by test (flagged)

| Item | Why unverified | Where it stands |
|---|---|---|
| `-horde-full-lod` path (ai_force_active `0x6C1A00`, ai_force_full_lod `0x70AB58`) | Signature-checked/hooked, never exercised in an accepted run | UNVERIFIED |
| `-horde-infantry` path (object_destroy `0x1EB650`, object_held `0x47A854`) | Not exercised in an accepted run | UNVERIFIED |
| Alert Carry / lower-weapon bind | Shipped and the hook installs; not re-tested in the stabilization QA | prior evidence only |
| `tag_get` shared-tag-cache diagnostics | Probe-only, removed after experiments | DEV-ONLY / UNVERIFIED |
| Firefight worker-thread null-TLS patch `0x4E9F5C` | Intro commit only; no log/test evidence | UNVERIFIED |
| LobbyPatches slot-count / peer-capacity patches | Pattern-scanned, no fixed RVA; no install line in the available logs | UNVERIFIED (fail-safe skip) |
| Extended-only limits/scheduler hooks (`ai_pool_resize` `0x12FCC`, `ai_update_stack`, `ai_spawn_search_stack`, `gs_failure_report`, `scheduler_guard_prepare`) | Extended profiles are disabled; no runtime evidence in release builds | UNVERIFIED today |
| FOV global write `0xB42C10` | No run evidence that the engine accepted a non-78 value | UNVERIFIED |
| Engine-side 128 MP / 16 co-op gates | UI clamps to 32/4; the patches are untested with real players | UNVERIFIED (networking lane) |
| Extended engine-limit profiles | Disabled by `4407101`; historic tests only | currently NOT SHIPPABLE |
| `-engine-cmd` / `-engine-cmd-at` | Dev hook; not exercised in QA | UNVERIFIED |
| Every individual Settings row's in-game effect | Screens render, effects not measured | mostly UNVERIFIED |
| 38 game-variant rules in Firefight | Editor opens; Firefight-specific rule effects not tested | UNVERIFIED |
| Armory item visuals in-match beyond visor/effects | Not measured | UNVERIFIED |
| `mods`/`modinfo.json` packaging contract & `.mvar` round-trip | No accepted test recorded | UNVERIFIED |
| Startup crash patches behaviour | Install/signature-check every run; crash fixes are from prior sessions | INSTALLS (prior evidence) |
| Forge editor monitor toggle | Forge plays as a player; flying editor not tested | UNVERIFIED |

## Appendix A - full launch-flag list (auto-generated from source)

This appendix is generated from the source tree: every `L"-..."` string literal with its first occurrence (`file:line`),
ordered by flag. It is the complete inventory; many entries are internal test/automation flags that have no player-facing
documentation and are grouped only by their source file. Treat everything here as dev/test-only unless section 4 above
describes it as a supported modder hook.

| Flag | First occurrence in source |
|---|---|
| -account-dev | src\common\Settings.cpp:322 |
| -account-tester | src\common\Settings.cpp:324 |
| -account-token | src\common\Settings.cpp:318 |
| -account-user | src\common\Relaunch.cpp:115 |
| -ai-actors | src\host\PoolResize.cpp:411 |
| -ai-pools | src\host\PoolResize.cpp:390 |
| -armor-effect-test | src\ui\ArmorCustomizer.cpp:981 |
| -armory-cycle | src\ui\ArmorCustomizer.cpp:2135 |
| -armory-cycle-chest | src\ui\ArmorCustomizer.cpp:2146 |
| -armory-cycle-delay | src\ui\ArmorCustomizer.cpp:2137 |
| -armory-cycle-list | src\ui\ArmorCustomizer.cpp:2149 |
| -armory-focus | src\ui\ArmorCustomizer.cpp:2917 |
| -armory-look | src\ui\ArmorCustomizer.cpp:2289 |
| -armory-scene | src\ui\ArmorCustomizer.cpp:2142 |
| -armory-scene-index | src\ui\ArmorCustomizer.cpp:1177 |
| -armory-scene-sweep | src\ui\ArmorCustomizer.cpp:1150 |
| -armory-settle-rebuild | src\engine\ReachArmory.cpp:1424 |
| -armory-species | src\ui\ArmorCustomizer.cpp:2110 |
| -armory-test-colors | src\ui\ArmorCustomizer.cpp:2291 |
| -armory-test-lit | src\ui\ArmorCustomizer.cpp:2295 |
| -armory-test-pri | src\ui\ArmorCustomizer.cpp:2293 |
| -armory-test-sec | src\ui\ArmorCustomizer.cpp:2294 |
| -armory-unlock-all | src\common\Relaunch.cpp:124 |
| -armory-validate | src\ui\ArmorCustomizer.cpp:889 |
| -armory-yaw | src\ui\ArmorCustomizer.cpp:2139 |
| -auto-play | src\ui\FrontEndMenu.cpp:4481 |
| -auto-play-map | src\ui\FrontEndMenu.cpp:4510 |
| -auto-play-variant | src\ui\FrontEndMenu.cpp:4530 |
| -campaign | src\ui\PauseMenu.cpp:373 |
| -capture-at | src\engine\FrameCapture.cpp:160 |
| -capture-frames | src\engine\FrameCapture.cpp:27 |
| -capture-profile | src\engine\ReachHost.cpp:1245 |
| --capture-profile | src\engine\ReachHost.cpp:1245 |
| -changelog | src\ui\FrontEndSocial.cpp:1135 |
| -connect | src\career\Career.cpp:812 |
| -container-hwnd | src\common\Relaunch.cpp:127 |
| -crash-test | src\main.cpp:815 |
| -data-dir | src\main.cpp:906 |
| -dev-commendation-tiers | src\career\Career.cpp:1549 |
| -difficulty | src\ui\PauseMenu.cpp:209 |
| -drawer-category-test | src\ui\FrontEndPickers.cpp:910 |
| -drawer-filter-test | src\ui\FrontEndPickers.cpp:907 |
| -elite-pack-death-fx | src\engine\ElitePack.cpp:1523 |
| -elite-pack-dump | src\engine\ElitePack.cpp:786 |
| -elite-pack-keep-globals | src\engine\ElitePack.cpp:1522 |
| -elite-pack-own-health | src\engine\ElitePack.cpp:1521 |
| -emblem-early | src\engine\ReachArmory.cpp:2506 |
| -emblem-patch | src\engine\ReachArmory.cpp:1033 |
| -emblem-patch2 | src\engine\ReachArmory.cpp:1058 |
| -emblem-patch3 | src\engine\ReachArmory.cpp:1048 |
| -emblem-trace | src\engine\ReachArmory.cpp:1099 |
| -end-round-after | src\engine\ReachHost.cpp:297 |
| -engine-cmd | src\main.cpp:1333 |
| -engine-cmd-at | src\main.cpp:1335 |
| -fake-pad | src\engine\ReachHost.cpp:2693 |
| -fake-pad-amount | src\engine\ReachHost.cpp:2718 |
| -fake-pad-y | src\engine\ReachHost.cpp:2711 |
| -film | src\ui\FrontEndMenu.cpp:6372 |
| -first-start | src\ui\FrontEndSocial.cpp:959 |
| -forge-save-at | src\main.cpp:1329 |
| -fov-apply-test-at | src\engine\CameraFov.cpp:138 |
| -fov-test-at | src\engine\CameraFov.cpp:107 |
| --guest-block | src\common\Relaunch.cpp:124 |
| -guest-post-start | src\main.cpp:1331 |
| -horde | src\host\SpawnTest.cpp:769 |
| -horde-delay | src\host\SpawnTest.cpp:789 |
| -horde-full-lod | src\host\SpawnTest.cpp:777 |
| -horde-infantry | src\host\SpawnTest.cpp:773 |
| -horde-no-teleport | src\host\SpawnTest.cpp:781 |
| -horde-team | src\host\SpawnTest.cpp:785 |
| -host | src\common\Relaunch.cpp:93 |
| -invite-code-make | src\ui\ServerBrowser.cpp:329 |
| -invite-code-selftest | src\ui\ServerBrowser.cpp:337 |
| -join-code-test | src\ui\ServerBrowser.cpp:324 |
| -lan | src\main.cpp:1178 |
| -launch-poke | src\engine\BlamEngine.cpp:1604 |
| -leave-lobby | src\ui\FrontEndMenu.cpp:4983 |
| -live-window-test | src\main.cpp:1211 |
| -loading-roster | src\main.cpp:1188 |
| -lobby- | src\ui\FrontEndMenu.cpp:6695 |
| -lobby-advance | src\ui\FrontEndMenu.cpp:6756 |
| -lobby-demo | src\ui\FrontEndMenu.cpp:6462 |
| -lobby-demo-ffa | src\ui\FrontEndMenu.cpp:6476 |
| -lobby-demo-map | src\ui\FrontEndMenu.cpp:6470 |
| -lobby-demo-mode | src\ui\FrontEndMenu.cpp:6468 |
| -lobby-demo-players | src\ui\FrontEndMenu.cpp:6466 |
| -lobby-demo-red | src\ui\FrontEndMenu.cpp:6472 |
| -lobby-demo-session | src\ui\FrontEndMenu.cpp:6464 |
| -lobby-demo-teams | src\ui\FrontEndMenu.cpp:6474 |
| -lobby-demo-unknown-mode | src\ui\FrontEndMenu.cpp:6501 |
| -lobby-difficulty | src\ui\FrontEndMenu.cpp:6734 |
| -lobby-endpoint | src\ui\FrontEndMenu.cpp:6778 |
| -lobby-map | src\ui\FrontEndMenu.cpp:6737 |
| -lobby-map-image | src\ui\FrontEndMenu.cpp:6742 |
| -lobby-map-override | src\ui\FrontEndMenu.cpp:6741 |
| -lobby-map-thumb | src\ui\FrontEndMenu.cpp:6742 |
| -lobby-map-title | src\ui\FrontEndMenu.cpp:6741 |
| -lobby-max-players | src\ui\FrontEndMenu.cpp:6733 |
| -lobby-mode | src\ui\FrontEndMenu.cpp:6737 |
| -lobby-mod-folder | src\ui\FrontEndMenu.cpp:6716 |
| -lobby-net | src\ui\FrontEndMenu.cpp:6731 |
| -lobby-playlist | src\ui\FrontEndMenu.cpp:6746 |
| -lobby-playlist-auto | src\ui\PauseMenu.cpp:3860 |
| -lobby-playlist-pos | src\ui\FrontEndMenu.cpp:6754 |
| -lobby-privacy | src\ui\FrontEndMenu.cpp:6732 |
| -lobby-return | src\ui\FrontEndMenu.cpp:4982 |
| -lobby-session | src\ui\FrontEndMenu.cpp:6718 |
| -lobby-session-token | src\ui\FrontEndMenu.cpp:6743 |
| -lobby-skulls | src\ui\FrontEndMenu.cpp:6735 |
| -lobby-teams | src\main.cpp:1993 |
| -lobby-variant | src\ui\FrontEndMenu.cpp:6720 |
| -lobby-variant-name | src\ui\FrontEndMenu.cpp:6721 |
| -local-port | src\main.cpp:1200 |
| -log | src\main.cpp:794 |
| -main-menu | src\common\Relaunch.cpp:289 |
| -map | src\ui\FrontEndMenu.cpp:6365 |
| -map-image | src\ui\PauseMenu.cpp:214 |
| -map-override | src\ui\PauseMenu.cpp:212 |
| -map-thumb | src\ui\PauseMenu.cpp:215 |
| -map-title | src\ui\PauseMenu.cpp:213 |
| -max-players | src\ui\PauseMenu.cpp:223 |
| -menu-capture | src\ui\FrontEndMenu.cpp:6425 |
| -menu-click-at | src\ui\FrontEndMenu.cpp:4593 |
| -menu-debug-layout | src\ui\FrontEndPickers.cpp:2575 |
| -menu-ignore-input | src\host\Window.cpp:543 |
| -menu-screen | src\ui\FrontEndMenu.cpp:6423 |
| -menu-tab | src\ui\FrontEndMenu.cpp:6427 |
| -menu-wheel-at | src\ui\FrontEndMenu.cpp:4639 |
| -mod | src\ui\PauseMenu.cpp:211 |
| -mode | src\host\SpawnTest.cpp:794 |
| -mod-prompt-demo | src\ui\FrontEndMenu.cpp:6595 |
| -mod-prompt-id | src\ui\FrontEndMenu.cpp:6601 |
| -mouse-scale | src\engine\ReachHost.cpp:1712 |
| --mouse-scale | src\engine\ReachHost.cpp:1712 |
| -mvar | src\main.cpp:1153 |
| -mvar-dump-at | src\main.cpp:1319 |
| -mvar-dump-every | src\main.cpp:1311 |
| -mvar-import | src\main.cpp:1250 |
| -mvar-live-dump | src\main.cpp:1313 |
| -mvar-page-scan | src\main.cpp:1316 |
| -mvar-parse | src\main.cpp:1304 |
| -mvar-roundtrip | src\main.cpp:1262 |
| -mvar-save | src\main.cpp:1240 |
| -mvar-write | src\main.cpp:1219 |
| -no-elite-pack | src\engine\ElitePack.cpp:1517 |
| -no-fp-override | src\engine\ElitePack.cpp:1524 |
| -no-mod-dll | src\main.cpp:1682 |
| -no-update | src\main.cpp:967 |
| -online | src\main.cpp:1176 |
| -open-mods | src\ui\FrontEndMenu.cpp:6619 |
| -pad-press | src\engine\ReachHost.cpp:1971 |
| -pad-press-at | src\engine\ReachHost.cpp:1973 |
| -pad-press-ms | src\engine\ReachHost.cpp:1975 |
| -pad-script | src\ui\FrontEndMenu.cpp:7239 |
| -parent-pid | src\main.cpp:767 |
| -pause-at | src\engine\ReachHost.cpp:2322 |
| -player-name | src\common\Relaunch.cpp:109 |
| -pool-report-exit | src\host\PoolReport.cpp:257 |
| -pool-trace | src\host\PoolResize.cpp:393 |
| -postgame-role | src\ui\FrontEndMenu.cpp:7239 |
| -postgame-test | src\ui\PauseMenu.cpp:4281 |
| -private | src\main.cpp:1182 |
| -profile | src\engine\ReachHost.cpp:1230 |
| --profile | src\engine\ReachHost.cpp:1230 |
| -profile-set | src\engine\ReachHost.cpp:1609 |
| -public | src\main.cpp:1180 |
| -quit-after | src\engine\FrameCapture.cpp:136 |
| -quit-app | src\ui\FrontEndMenu.cpp:4948 |
| -recommended-download | src\ui\FrontEndMenu.cpp:6429 |
| -relaunch-test | src\main.cpp:729 |
| -relaunch-test-child | src\main.cpp:730 |
| -relay | src\main.cpp:1343 |
| -remember-account | src\common\Settings.cpp:326 |
| -render-states | src\host\PoolResize.cpp:396 |
| -scoreboard-at | src\engine\ReachHost.cpp:2596 |
| -scoreboard-demo | src\ui\FrontEndMenu.cpp:7238 |
| -scoreboard-demo-players | src\ui\FrontEndMenu.cpp:7239 |
| -scoreboard-test | src\ui\PauseMenu.cpp:4299 |
| -server-browser-demo | src\ui\ServerBrowser.cpp:468 |
| -service-tag | src\common\Relaunch.cpp:112 |
| -session-id | src\common\Relaunch.cpp:93 |
| -session-token | src\ui\PauseMenu.cpp:220 |
| -settings-export | src\main.cpp:941 |
| -settings-import | src\main.cpp:942 |
| -settings-search | src\ui\SettingsPanel.cpp:407 |
| -skulls | src\ui\PauseMenu.cpp:210 |
| -slot-patch | src\host\SlotPatch.cpp:733 |
| -slot-players | src\host\SlotPatch.cpp:756 |
| -social-demo | src\ui\FrontEndMenu.cpp:7107 |
| -social-demo-open | src\ui\SocialOverlay.cpp:458 |
| -social-friends-demo | src\ui\FrontEndSocial.cpp:125 |
| -spawn-max | src\host\SpawnTest.cpp:765 |
| -spawn-test | src\host\SpawnTest.cpp:761 |
| -steam-dll | src\main.cpp:763 |
| -steam-progress | src\main.cpp:765 |
| -steam-subscribe | src\main.cpp:761 |
| -team-probe | src\engine\ReachHost.cpp:506 |
| -test-map-gametypes | src\ui\FrontEndSocial.cpp:1143 |
| -third-person | src\main.cpp:1203 |
| -third-person-distance | src\main.cpp:1213 |
| -third-person-front | src\engine\CameraThirdPerson.cpp:502 |
| -third-person-mode | src\main.cpp:1216 |
| -trace-members | src\host\MemberTrace.cpp:123 |
| -user-id | src\common\Settings.cpp:320 |
| -variant | src\ui\PauseMenu.cpp:208 |
| -variant-dump | src\main.cpp:1205 |
| -variant-override | src\main.cpp:1207 |
| -variant-poke | src\engine\BlamEngine.cpp:1545 |
| -voice-probe | src\main.cpp:1209 |
| -voice-test | src\audio\VoiceChat.cpp:1130 |
| -voice-test-relay | src\audio\VoiceChat.cpp:1133 |
| -voice-test-seconds | src\audio\VoiceChat.cpp:1134 |
| -voice-test-source | src\audio\VoiceChat.cpp:1132 |
| -voice-test-wav | src\audio\VoiceChat.cpp:1131 |
| -wait-peers | src\main.cpp:1186 |
| -wheel-after | src\engine\ReachHost.cpp:2525 |

Total: 216 flag literals in the source tree (some are sent/parsed internally, not all are user flags).


---

## Addendum 2026-11-17 - armor-effect colour variants and VFS map serving

**Armor FX variant state:** 24 primary-colour flames are selectable in the Armory (32 effect entries total: 7 stock +
PRIMARY MATCH + 24 variants); 6 colours are hidden pending a runtime approach. Custom effect item ids are `0xF0..0x107`
(Spartan profile +0x68, Elite +0x90); the translator detour on `haloreach.dll+0x3B2F4` maps them to pmcg indices 7..30.
Full details: `../04-project-state/HANDOFF-ARMOR-FX-COLOURS-2026-11-17.md`.

**Engine limits worth knowing as a modder:** the engine correctly permutation-gates at most **5 armor-effect attachments
total** (stock + 4 extras); more than 4 extra effect tags makes effects spawn regardless of selection ("two effects at
once"). Effect tag `locations` blocks cap at **6**; `Save()` silently drops locations added past 6. Render-model regions
cap at 16. Region permutation 0 is active by default, so any marker bound to perm 0 always plays. Detailed topic:
`../02-engine-reference/TOPIC-armor-appearance-colors-armory.md`.

**Serving rebuilt maps to every map without per-map overrides:** put rebuilt cache files in a mod folder as
`<mod>\\maps\\<scenario>.map` and launch with `-mod <folder>` (absolute path works). Noble's VFS resolves the active
mod's file before MCC's stock map, so one mod folder can carry many rebuilt maps and every match on those maps gets the
mod. Matchmaking does not use map mods; peers need the same mod for multiplayer. Measured rebuild ratio ~2.8x stock size
(70_boneyard 199 MB -> 566 MB); full 44-map pack ~25-30 GB. Batch script: `../test-tools/build_all_mod_maps.ps1`.

**Menu map gotcha:** after every mainmenu cache rebuild, copy it to `build\\bin\\Release\\data\\menumaps\\noble_menu.map`
**and** rerun `test-tools/menu_sidcsv.ps1`; stale string ids break Armory effect mapping.


## Armor FX runtime colour flags (dev only, 2026-11-17)

These flags are inert without a primary-flame variant selected (or the flag itself):

| Flag | Meaning |
|---|---|
| `-armor-fx-runtime-tint r,g,b` | Force a manual flame palette RGB (also accepts 6-digit hex). |
| `-armor-fx-runtime-variant N` | Force one of `kPrimaryFlames` 0..29 (steel..pale). |
| `-armor-fx-runtime-primary N` | Force the PRIMARY MATCH palette to primary colour N (0..29). |
| `-armor-effect-test N` | Set the profile Armory effect index (8..37 = variants steel..pale); use this for automatic tests. |

Effect index map: 32=RUST, 33=CORAL, 34=PEACH, 35=GOLD, 36=YELLOW, 37=PALE. On stock maps the profile writes the stock
LEGENDARY item and the runtime palette supplies the colour; no map rebuild or mod is needed.

- `-display-change-test-at <seconds>`: dev regression flag; moves the window to the next monitor from a non-window thread after N seconds. Used to reproduce/verify the display-change deadlock fix.
