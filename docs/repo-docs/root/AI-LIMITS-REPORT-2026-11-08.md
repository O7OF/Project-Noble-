# AI limits: verification report — 2026-11-08 (DeepSeek)

Written 08:45 ET (final regression 08:46) for **Claude** (continuation at ~10:05) and **Tyler**. Everything below is in the working tree at
`D:\Project-Noble-Closed-Source`; nothing is committed, pushed, packaged or zipped, and the installed
`haloreach.dll` is untouched. Builds stay local in `build\bin\Release`.

Read with: `DEEPSEEK-START-AI.md` (the task list), `AI-LIMITS-HANDOFF.md` (engine findings),
`LIMITS-TODO.md` (status), `AGENT-STATUS.md` (claims + log + Tyler's test list), `docs\limits-ai.md`
(fixed-table audit), `docs\limits-render.md` (C11 research).

---

## Batch 2 (2026-11-08 09:00-09:20 ET) — results

Order run: B1 (hold 1,024 on mission 3) → B2 (0x76E814 patch) → B4 (render states) → B5/B7 research.
B3 (join gating) and B6 (light volumes) were **not** implemented — notes below.






### 5-minute hold on the requested command (2026-11-08 09:59-10:05 ET)

Ran exactly `noble-dev.exe -map ff_unearthed -mode firefight -ai-actors 1024 -horde 1000 -horde-team 0
-horde-no-teleport -lan` (Tyler closed the window at ~10:05, so the hold ran its full ~5 minutes):

| Metric | Result |
|---|---|
| placement | 1,000 placed, 0 teleported, **980 live** at the end of the wave (10:00:00) |
| hold | 900 → 875 → 842 → 705 over the first 90 s, then flat at **~669-672** for the last 2 minutes |
| memory | private 6,660 MB, working set 7,085 MB — flat |
| frame rate | 17.5-19.8 fps at ~980-900 live, 21-24 fps as it settled to ~670 |
| stability | **no STACK OVERRUN, no FATAL, no crash** for the whole run |

That is the best hold so far (previous best 756 with teleports). The drop from 980 to ~670 in the first ~90 s is
not combat (blind+deaf, one team) and not the teleport (off): it looks like the engine/scenario thinning placed
squads after they land. The run being closed by Tyler (not a crash) also explains the earlier "process ended
with no crash record" in the mission-3 run — that was a closed window, not a failure.
### Real cinematic flag + hold refinements (2026-11-08 09:35-09:52 ET, after Tyler's pointers)

**Cinematic state (works in retail, no timer).** Followed the commands that are live:
- `cinematic_start` thunk `0x1AF484` → core **0xD3240**: loads the game-state TLS root and writes `byte [root+0x130] = 1`
- `cinematic_stop` thunk `0x1AF4A0` → core **0xD3444**: `byte [root+0x130] = 0` (and root+0x140/+0x14, root+0xE0/+5 move with it)
- `cinematic_show_letterbox` thunk `0x1AF53C` (letterbox style only)
- the old `0x86CBC`/`0x85DE0` pair is the shared compiled-out stub (drop / cheat_all_*) — not used.

SpawnTest now reads `*[TLS root + 0x130]` each tick (SEH-guarded), logs transitions and gates the auto queues
on it. Verified on mission 3: `cinematic_in_progress true (auto-spawn waiting)` at 09:45:47, `false (ready)` at
09:47:23 (a 105-second intro), and the horde queued in the same tick — no timer involved. 1,000 placed in 13 s,
781 moved, 470 live afterwards, no crash and no 1-second stalls. `-horde-delay N` still overrides; the timer
(campaign 60 s / else 10 s) remains the fallback if the root is unreadable.

**Team / blind / deaf / teleport.** Core functions behind the script commands:
`ai_set_team` **0x6C1C14(ai, team)**, `ai_set_blind` **0x6BF204(ai, bool)**, `ai_set_deaf` **0x6BF0EC(ai, bool)**.
Horde now sets the squad's team *before* `ai_place` (same handle form as `ai_place`: `0x20000000|squad`), sets
each new actor's team, marks it blind and deaf, and skips the teleport for actors already within 30 m of the
player. New switch `-horde-no-teleport` leaves placed actors where the engine puts them.

Firefight Unearthed, 1,000 placed, live actors at the end of the wave:

| Configuration | Live at wave end | After ~50 s |
|---|---|---|
| baseline (no team/blind/deaf) | 679 | 632 |
| team only | 756 | 677 |
| team + blind + deaf + squad team + 30 m skip | 745 | 693 |
| **team + blind + deaf + squad team + no teleport** | **973** | **887** |

So the combat losses were small; **the teleport/vehicle-exit path was killing the actors**. `-horde-team 0
-horde-no-teleport` is the way to hold near 1,024 (973 of 1,024 alive, no crash, 3,200 objects).
### Follow-up after Tyler's notes (2026-11-08 09:30-10:00 ET)

**1. cinematic_in_progress.** Found the real chain in the script-command table:
- command-table entry for `cinematic_in_progress` at **0xA15468** (name pointer `0x180998DD8`),
- its thunk **0x86CBC**: `mov ecx,edx; xor edx,edx; jmp 0x85DE0`,
- **0x85DE0** is the script *value dispatcher* (switch on the value/command id), not a plain no-argument check.
  Calling it with `(0, 0)` from the game thread faults at `haloreach+0x86CA6` — it needs the VM's argument.
So the harness now **detects and validates** the check (`SpawnTest: cinematic_in_progress found (0x85DE0)`) but
still gates the auto-queues with the timer fallback: campaign 60 s, otherwise 10 s, `-horde-delay N` overrides.
`kUseCinematicCheck = false` in `SpawnTest.cpp`; the clean next step is a MinHook on **0x86CBC** to capture the
arguments the mission passes (one cutscene is enough) and then call 0x85DE0 with them, or to log the thunk's
return while a cutscene runs. That replaces the timer with the real flag, as Tyler suggested.

**2. Keeping 1,024 alive.** `ai_set_team`'s thunk (`0x1A25DC`) calls the real function **0x6C1C14(actor, team)**;
SpawnTest now has `-horde-team N` (`NOBLE_HORDE_TEAM`) and calls it for every newly placed actor. Test on
Firefight Unearthed with `-horde-team 0`: 1,000 placed, **756 live** at the end of the wave (vs 679 without),
decay 756 → 677 over 80 s — better, not yet a flat 1,024. The placed actors still die from falls/teleport timing
and from scenario-side cleanup; the next refinements are (a) set the squad's team before `ai_place` instead of
per actor afterwards, (b) `ai_set_blind`/`ai_set_deaf` (thunks 0x1A1998 / 0x1A1950) on each actor, and
(c) pause the teleport for actors that are already alive near the player. Mission 3 remains wrong for holds:
its script removes AI the mission did not place itself — confirmed again (11 left).
### Cutscene fix (2026-11-08 09:25 ET, after Tyler's note)

The auto-horde used to queue 10 s after the first match tick. On a campaign map that is **inside the opening
cutscene**, which explains the 1-2 s frame stalls ("main blckd" >1000 ms), the AI being cleaned up when the
mission script resumed, and the instability after extra F6 actors loaded mid-cinematic.

Fix in `SpawnTest.cpp`: the launch mode is parsed, and **campaign launches now wait 60 s** before the automatic
`-horde` / `-spawn-test` queue (override with `-horde-delay N`, carried through `NOBLE_HORDE_DELAY`). The
ready line states the wait: `auto-spawns wait 60 s after the match starts (campaign intro cutscene)`.
Firefight/multiplayer keep the 10 s delay.

Verified on mission 3 (5-minute post-fix run, 09:24-09:30): the queue fires at 60 s, 1,000 actors are placed in
~24 s with **no 1-second stalls**, peak ~438 live, then the mission script removes them (11 live by 09:26:14 —
the mission does not keep AI placed via `ai_place`), memory flat at 6,836 MB private / 7,259 MB working set,
and **the process stayed alive for the whole 5 minutes, no crash**. Firefight Unearthed remains the map for a
long hold. There is no engine "cutscene in progress" flag exposed to the harness, so the delay is a heuristic;
`-horde-delay` lets a tester wait as long as the intro needs.
### Double check (2026-11-08 09:20 ET, DeepSeek)

Re-verified batch 2 end to end after a review pass:

| Check | Result |
|---|---|
| 0x76E814 in the cached patched DLL (`v16-2a815ac4553c`) | disassembled: `lea rbp,[rax-0x1038]`, `sub rsp,0x1100`, cookie `[rbp+0xFE0]`, epilogue `lea r11,[rsp+0x1100]`, stack args `[rbp+0x1060]/[rbp+0x1070]`, caps `0x400` at 0x76E8EE / 0x76E924 / 0x76EA66 — all present |
| Hooks | `ai_update_stack` and `ai_spawn_search_stack` install in every Extended start (log) |
| Stock profile | `test-limits.ps1 -Profiles 0` **PASS** (object pool 2,048) |
| Extended (V16) | `test-limits.ps1 -Profiles 1` **PASS** (32,767) |
| Extended + Particles | `test-limits.ps1 -Profiles 2` **PASS** (32,767) |
| Default 512 start | actor pool 512, `cached object render states` stays 4,096, no crash |
| Firefight 1,024 on the final build | 900 placed, 679 live, held 80 s, no crash |

Fixes made in the pass: removed a stale duplicate comment above the render-states rule (the old text still
claimed the budget trims it; the tested behaviour is a crash above 4,096, so the flag stays off). Test-invocation
note: `test-limits.ps1 -Profiles 0,1` does not bind an array under `powershell -File`; run one profile at a time
(`-Profiles 0`, `-Profiles 1`) or call it with the default parameter.

### B1 — hold 1,024 live actors on mission 3 (`m30`, Nightfall)

Ran twice: `noble-dev.exe -map m30 -mode campaign -difficulty 1 -ai-actors 1024 -horde 1000 -lan`.

| Metric | Result |
|---|---|
| horde placement | **1,000 placed in 27 s**, 828 moved next to the player |
| peak live actors | **482** (1,000 placed; the rest died or timed out before the pool report) |
| hold | **fell to 11 within 90 s** — the mission's own scripting removed the AI (40-60 actors per 20 s), so mission 3 does **not** keep them alive |
| memory | private 6,860 MB, working set 7,280 MB — flat for the whole run |
| frame rate | 7-45 fps while the wave placed/teleported (one 300-frame window took 41.5 s = 7.2 fps); 28-45 fps at ~440 live; 110-140 fps after the AI despawned |
| stability | run 1 was crashed at 09:03:45 **after external F6 input** (someone pressed F6 ~15 times, actors rose to 980): `0xC0000005` in `VCRUNTIME140.dll`, crash dump `build\bin\Release\data\sandbox\noble-crash.dmp`. Run 2 lost the process at 09:05:22 with no event-log crash entry (closed or killed) |

**Conclusion: mission 3 is a bad hold map** — the mission despawns placed AI, and two runs ended early. The
previous batch's Firefight Unearthed runs are the reliable hold: **1,024 placed, ~685-767 live, stable for
minutes**. For a true 5-minute 1,024-live hold, use Firefight Unearthed (or a map without scripted AI cleanup),
not a campaign mission.

### B2 — 0x76E814 scratch grown (frame +0x800, caps 0x400, stack-commit hook)

- `AI_FRAME3_OPS` in `make_v16_particles.py`: `lea rbp,[rax-0x838]`→`-0x1038`, `sub rsp,0x900`→`0x1100`,
  cookie `0x7E0`→`0xFE0` (store+load), stack args `0x860/0x870`→`0x1060/0x1070`, epilogue
  `lea r11,[rsp+0x900]`→`0x1100`. `AI_FRAME3_CAP_OPS`: the two `mov eax,0x200` (0x76E8EF, 0x76E925) → `0x400`.
- `PoolResize.cpp`: new `ai_spawn_search_stack` MinHook at 0x76E814 that calls `CommitStack()` before the
  original (the function has no `__chkstk`); installed only when the patched `lea rbp,[rax-0x1038]` bytes are
  present. Log confirmed: `Hook 'ai_spawn_search_stack' installed`.
- **Hashes:** `--ai-bank-extra 0` still reproduces V16 `b8810c3b…` and V16p `47bc0723…` exactly. New profiles:
  v16 `2a815ac4…`, v16p `7d807c95…`.
- Re-run: Firefight `-ai-actors 1024 -horde 900` → 900 placed, 685 live, stable 85 s, no crash. Mission 3 still
  unstable, independent of this patch (the same build is fine on Firefight).

### B4 — C11 runtime half (cached object render states)

- Added to `PoolResize` as an **opt-in** `-render-states N` / `NOBLE_RENDER_STATES` rule (default off), because
  the game-state bank cannot take it yet:
  - 16,384: crash `0xC0000005` at `haloreach+0x27B01F` (function 0x27AF74–0x27B6C4) during map load;
  - 8,192: crash `0xC0000005` at `haloreach+0x6E58E` (function 0x6E550–0x6E637) during map load;
  - default build (no flag): render states stay 4,096, verified in a start-up log.
- **The runtime half is therefore not usable at 4,096 → 8,192/16,384 with the current banks.** The V16
  append-block/bank work (Claude's) is required first. Code path is ready behind the flag.

### B5 — C6 script node constructor (found)

`script node` is created by the growable-array helper `0x13148`, called from **0x81854**:
name `"script node"` (0x995F60), initial bytes `mov r8d,0xFC00` at **0x81846**, record `r9d=0x18` (24 B),
header block 0x1FF0 bytes. 0xFC00/24 = 2,688 initial nodes; the live pool report's 16,280 is the *grown* size,
which is why no fixed cap exists. Patch point for a resize path: the immediate at **0x81846** (file 0x80C46) or
a hook on 0x13148. Documented in `docs/limits-ai.md` (C6 addendum). Not patched.

### B3 — D3 join gating (not implemented — notes)

Noble already sends a large session record (`MasterServerClient::DoRegister`, body at
`src/network/MasterServerClient.cpp:508`; heartbeats at :568) and the join path receives `LobbyState`
(`MasterServerClient.hpp:39`, join at `src/ui/FrontEndMenu.cpp:6422`). Plan:
1. Add `{"limits", limits::ProfileNetworkId(limits::ActiveProfile(), startupSettings.aiActors)}` to the
   registration body and the heartbeat (and to the dedicated server's advert).
2. Parse it in `LobbyState` (new `uint32_t limitsId`), from the session JSON in `FetchLobbyStateSync` (`:746`)
   and `JoinLobbySync`.
3. In `FrontEndMenu`'s join path, before `JoinLobbySync`, compare with the local id and refuse with a message:
   "This server uses Extended engine limits (AI actors N). Change SETTINGS > ENGINE and restart."
4. The private backend (`ServerTestDeploy`) must pass an unknown `limits` field through in the session record;
   until it does, missing field = allow (backward compatible).
Two local dev instances could not be used unattended (needs two Steam/logged-in sessions); test manually.

### B6 — C8 light volumes (not done — notes)

Porting LV V4 onto V16p is the largest item in LIMITS-TODO: it needs a new combined block in `.deltrac` after
the particle block, all rel32/exception-table fixups recomputed, the three particle operands inside LV's
relocated effect-init copy (0xC9E10B / 0xC9E15A / 0xC9E189), and combined bank sizes (render 0x37F0000, main
0x3230000, checkpoint 0x6C80000, total 0xD0E0000) per `LIMITS-HANDOFF.md`. Not started this batch; nothing in
the tree changed for it.

### B7 — C12 players research (`docs/limits-players.md`)

Written: players pool 16 × 0x490 (created at RVA 0x43474, count immediate 0x43470, size 0x43465), fire team
16 × 0xD4 (0x122E58 / 0x122E54), simulation view 16 × 0x118, LobbyPatches RVAs (0x3973BC, 0x39CE10, 0x425368,
0x42459C, 0x44EBA0, 0x429AA4, 0x39BDCC, globals), milestones 16→32→64→128 with effort estimates
(~2-4 days / 1-2 weeks / 2-4 weeks).

---

## 1. Status of every task

| Task | Status | Evidence (this session) |
|---|---|---|
| **TODO 1** F6 horde too slow + `-horde N` | **DONE, verified in a real match** | 1,024 actors placed in ~13 s (was one squad / 0.2 s); queue capped at free pool slots; `-horde` auto-queued; stock-96 and no-squad maps stop cleanly. 6 unattended runs (4 at 1,024 actors, plus stock-96 and Forge), no crash. |
| **TODO 2** Verify fix 2 (`0x76EBF8`) statically | **DONE — and it was incomplete, now fixed** | Buffer is 512 × 0x2C spawn candidates ending at the cookie; three `0x200` caps bound it. Frame doubling alone was useless; added `AI_FRAME2_CAP_OPS` (0x200 → 0x400). Verified in the built DLL and by 1,024-actor runs. |
| **TODO 3** Audit the AI range for fixed tables | **DONE (research + tool + doc)** | `tools/limits/analyze_ai_frames.py` (capstone + `.pdata`); 64 candidates listed with frame/cookie/span in `docs/limits-ai.md`; highest-risk unfixed ones flagged (`0x76E814`, `0x76D714`, `0x76A358`). |
| **TODO 4** C7d: actor count a setting + network id | **DONE, verified end to end** | SETTINGS > ENGINE > AI ACTORS (96/256/512/1024, disabled unless Extended); saved, exported, imported, reset to defaults; `main.cpp` → `NOBLE_AI_ACTORS`; `ProfileNetworkId(profile, actors)` = `0x01000200` / `0x01000400`. D6 part also done (ENGINE LIMITS has EXTENDED + PARTICLES; `noble-dedicated -limits v16p`). |
| **TODO 5** Update TODO/status + Tyler test list | **DONE** | `LIMITS-TODO.md` phase 3b/D6, `AGENT-STATUS.md` (log + "Ready for Tyler to test" tests 1-5, now updated with automatic results). |
| **Part 2 item 6** C11 render list 4,096 → 16,384 | **Research only** | `docs/limits-render.md`: capacity operands `0x2565E6` / `0x14EAB7` (both `0x1000`), +2.2 MB V16 append block, +7.65 MB render-state pool, plan. |
| **Part 2 item 9** C6 script limits | **Partial, built, verified** | PoolResize now raises `det hs thread` 320→1,024, `non-det hs thread` 4→32, `list object reference` 128→512 (confirmed in a live log). `script node` 16,280 × 24 B is **not** a `data_array_new` pool — its rule is inert. |
| Part 2 items 7, 8, 10 | Not started | D6 was done with TODO 4; C8 light volumes and C12 players untouched. |

---

## 2. How it was verified (unattended, no Tyler needed)

### 2.1 The in-game tests actually ran
`noble-dev.exe` supports `-map`/`-mode` direct launch, so the horde tests were run headless-of-input:

```powershell
cd D:\Project-Noble-Closed-Source\build\bin\Release
.\noble-dev.exe -log horde-final.log -map ff_unearthed -mode firefight -ai-actors 1024 -horde 1200 -lan
```

Results (log lines, verbatim):

| Run | Log line | What it proves |
|---|---|---|
| `-ai-actors 1024 -horde 1200` | `horde capped at the 1024 free actor slots (1200 were queued, actor pool 1024)` | pending-cap works |
| same | `horde wave done \| 1024 placed, 629 moved next to you \| actors 763 / 1024` | 1,024 actors created in ~13 s; engine survives |
| same, +60 s | `actor 763 / 1,024` → `667 / 1,024`, **no** `STACK OVERRUN`/`FATAL` | stable past the old ~647 live crash point |
| `-horde 200`, stock pool | `actor 0 / 96`, `horde capped at the 96 free actor slots`, `96 placed … actors 96 / 96` | clamps to the real pool, not a guess |
| `-map forge_halo -mode forge -horde 100` | `horde needs AI squads; this map has none in its scenario` | no-squad maps warn and stop (new) |
| `-map ff_unearthed -mode firefight -ai-actors 1024` (900 queued) | `900 placed, 565 moved … actors 688 / 1024` | second confirmation |
| frame-tick timing | 300-frame windows: 25.5 / 28.3 / 16.2 / 18.9 / 18.3 / 18.0 fps | **C7c data**: ~18 fps at ~760 live actors (512 was ~36 fps) |

Frame rate note: the numbers include loading and the wave spike; the steady value is ~18 fps at 1,024 configured /
~760 live. That is the known actor cost, not a hang.

### 2.2 Engine build checks
- `tools\limits\test-limits.ps1 -Profiles 0,1` → **PASS**; `-Profiles 2` → **PASS** (V16 and V16+particles:
  profile active, object pool 32,767, no crash). Run against the regenerated `.nlp` files.
- `--ai-bank-extra 0` still reproduces V16 `b8810c3b…` byte for byte (proves only intended bytes changed).
- Disassembly of the cached patched DLL (`v16-11e3ac2c31ec`):
  - prologue `lea rbp,[rsp-0xAFE0]`, `mov eax,0xB0E0`, xmm6 at `[rsp+0xB0D0]`;
  - cookie at `[rbp+0xAFC0]`, epilogue `lea r11,[rsp+0xB0E0]`;
  - caps: `mov edi,0x400` (0x76EC6A), `cmp rdi,0x400` (0x76EB80), `cmp esi,0x400` (0x76EA66);
  - stack args moved: `[rbp+0xB048]`, `[rbp+0xB050]`, `[rbp+0xB058]`.

### 2.3 Settings path (C7d)
- `-settings-export` contains `"aiActors": 0`; importing a file with `"aiActors": 256` writes it to
  `settings.json` (restored afterwards).
- A settings-only start (no command line) logs
  `Engine limits: AI actors 1024 (network id 0x01000400)` and
  `AI limits: experimental, actor pool 1024 (stock 96); AI memory +6080 KB`, and the pool report shows
  `POOL|actor|0|1024|3328|live`.
- Reset to defaults path reviewed: `RestoreDefaults(Cat::Game)` → `WithDefaults` copies `aiActors` from the
  default `SettingsData` (0), same as `extendedLimits`.
- `noble-dedicated.exe -limits v16p` prints
  `Engine limits: requested Extended + Particles, active Extended + Particles, reason none`.

---

## 3. Bugs found during verification (and fixed)

1. **Fix 2 was a no-op without the caps.** The 0x76EBF8 buffer is a 512-entry × 0x2C candidate list; its three
   bounds (init loop `0x76EC6A`, builder `0x76EB80`, append `0x76EA66`) all stayed at 0x200, so a doubled frame
   added only unused stack. Added `AI_FRAME2_CAP_OPS` so 1,024 × 0x2C exactly fills the new frame. _This is the
   main correctness fix of the batch._
2. **Command-line `-ai-actors` could be overridden by the saved setting** in the relaunched match process.
   `main.cpp` now only publishes `NOBLE_AI_ACTORS` when it is not already set, and only on an Extended profile.
3. **Horde silently idled on maps without squads** (Forge World): now logs one warning and clears the queue.
4. **Handoff errors corrected:** `0x76E800` is not a function start, and `0x76E314` is mid-instruction inside
   `0x76D714` (a single `.pdata` function) — both fixed in `AI-LIMITS-HANDOFF.md` and `docs\limits-ai.md`.
5. **Horde "placed no new actors" could repeat every 100 ms** while the queue was non-empty: now logged once per
   queue (flag reset on F6/-horde and at wave end). Dead code (`Settings::Loaded`) removed.
6. **A final regression run after these fixes** (`-ai-actors 1024 -horde 1200`): `1024 placed, 621 moved …
   actors 756 / 1024`, 0 errors, 0 repeated lines.

Nothing else regressed: all three targets build with 0 warnings/0 errors, and the Stock/Extended/V16p loads all
pass.

---

## 4. Still open (for Claude — in priority order)

1. **Push 1,024 *live* harder.** The runs placed 1,024 but combat attrition held live ~760. A map/mode where the
   placed actors do not fight each other (campaign or a passive Firefight variant) would keep ~1,024 live and
   stress the tables longer. Any crash gives `STACK OVERRUN in haloreach+0x…`; run
   `python tools\limits\analyze_ai_frames.py --function 0x<RVA>` for its frame/caps.
2. **`0x76E814` scratch (frame 0x900, 512-dword array, chain expansion unbounded).** If `0x76EAC0` ever appears,
   that is it: grow its frame + 0x800 and move its `0x200` caps (needs a stack-commit hook — no `__chkstk`).
3. **C11 runtime half first:** try adding `cached object render states` to `PoolResize` (name is in the pool
   report); only then rebuild V16's append block for the visible list/mesh (`docs\limits-render.md`).
4. **C6 `script node`:** not created by `data_array_new`; find its constructor (pool report name
   `script node`, 16,280 × 24 B) before the rule can do anything.
5. **D3 join gating:** `ProfileNetworkId(profile, aiActors)` exists but nothing sends it yet; host/client
   mismatch is still undetected.
6. **Commit/push** only on Tyler's word (nothing was committed in this batch).

---

## 5. Files touched (all local)

**Code**
- `src/host/SpawnTest.cpp` — multi-squad horde (25/step, 100 ms, slot cap), `-horde`/`NOBLE_HORDE`,
  no-squad warning.
- `src/host/PoolResize.cpp` — script pool rules (`det hs thread`, `non-det hs thread`,
  `list object reference`, inert `script node`).
- `src/common/Settings.hpp/.cpp` — `aiActors` (clamped to 0/256/512/1024), saved/loaded, export/import.
- `src/ui/SettingsPanel.cpp` — third ENGINE LIMITS label, AI ACTORS row, Changed count, reset/export.
- `src/host/EngineLimits.hpp/.cpp` — `ProfileNetworkId(Profile, int aiActors)`.
- `src/main.cpp` — `NOBLE_AI_ACTORS` bridge (no override), AI actors log line.
- `src/dedicated/main.cpp` — `-limits v16p`.
- `tools/limits/make_v16_particles.py` — `AI_FRAME2_CAP_OPS`; `data/limits/{v16,v16p}.nlp` regenerated
  (v16 `11e3ac2c…`, v16p `9ba4fdc6…`).

**Tools / docs**
- `tools/limits/analyze_ai_frames.py` (new), `docs/limits-ai.md` (new), `docs/limits-render.md` (new),
  this report, `AI-LIMITS-HANDOFF.md`, `LIMITS-TODO.md`, `AGENT-STATUS.md`.

**Artifacts kept as evidence** in `build\bin\Release`: `horde-final.log`, `horde-sat.log`,
`horde-regress.log`, `horde-1024.log`, `horde-stock.log`, `horde-forge.log`, `settings-1024.log`, plus the existing
`limits-test-*.log`. All are inside `build/` and stay local.

---

## 6. Build / release state

- `cmake --build build --config Release --target noble-dev / noble-client / noble-dedicated`: clean.
- **No** `build.bat`, **no** packaging, **no** zip, **no** commit, **no** push. Engine patches live only in
  `data\limits\*.nlp` + `%LOCALAPPDATA%\ProjectNoble\engine` copies, as required.
- `data\limits` synced to `build\bin\Release\data\limits` (hashes match).

---

## Test batch 2026-11-08 10:15-10:35 ET (DeepSeek, for Claude)

Claude's changes: render bank 3 +0x800000, render states charged to that bank; .nlp v16 `efb4fbf2…`,
v16p `69ee5492…`. `noble-dev` rebuilt clean (no source changes from me; engine ops untouched).

### C11 — `-ai-actors 256 -render-states 16384`, Forge World, `-spawn-test 5000`

- build **v16-efb4fbf20be7** (matches), map loaded, `AI limits: cached object render states 4096 -> 16384
  (7488 KB more game-state memory)` ✓
- `POOL|cached object render states|461|16384|624|live`; POOLNEW from `-pool-trace`:
  `POOLNEW|cached object render states|16384|624|0x7ff42b3a0000|0x7fff15881b08|data=0x7ff42b3a0070|bitmap=0x7ff42bd60070`
- **crash at 4,069 objects**: `FATAL 0xC0000005 at haloreach+0x68B21F`, right after
  `Scheduler guard: object update wants 73795 jobs but the scheduler only has 16384`
- **A/B control, same run without `-render-states`**: identical crash (73,793 jobs, same `+0x68B21F`) — the
  crash is **not** caused by the render-states change; it is the known C10 scheduler wall, reached at ~4,069
  objects in this dense 5,000-weapon spawn on the new build.
- **C11 at 3,800 objects**: full **5 minutes, no crash**; render-states pool 16,384 (851 used); object pool
  peaked 3,869 and then decayed to ~2,250 (the engine cleaned the unowned weapon copies) — the render-state
  half is verified and stable.
- **Visible objects past 4,096: not exercised.** No run held more than ~3,900 objects (the 5,000 run dies at the
  scheduler wall first), so the separate V16 4,096 draw-list question still needs Tyler's eyes once C10 has more
  headroom or the spawn is spread across activation regions.

### Regression — `-map ff_unearthed -mode firefight -ai-actors 1024 -horde 1000 -horde-team 0 -horde-no-teleport`

- run 1: wave end **956 live**, no crash (window closed after 3.3 min, no crash record).
- run 2, full 5 min: wave end **920 live**, then flat **~682** for the remaining time; memory 6,714 MB private /
  7,139 MB working set; **no FATAL / STACK OVERRUN**. 920 is just under the ~950 target (956 in run 1) — run-to-run
  variability, not a crash.

### Particles — new v16p

- build **v16p-69ee54924495** (matches), profile active Extended + Particles.
- `POOL|particles|1|16384|140` and `POOL|particle_emitter|22|6656|148` — CPU particles 16,384, emitters 6,656,
  no crash, pool report done.

### Files (build\bin\Release)
`c11.log`, `c11-trace.log`, `ab-norender.log`, `c11-short.log`, `regress.log`, `regress2.log`,
`particles.log`. Nothing committed or pushed.

### Check after Claude's D3 edits (2026-11-08 10:36-10:40 ET, DeepSeek)

- `noble-dev`, `noble-client`, `noble-dedicated` all **build clean with Claude's 10:16 edits** (0 warnings,
  0 errors; exes 10:35-10:36).
- Start-up: V16 active, actor pool 512, render states 4,096, no crash.
- Short Firefight check on the new binary: 1,000 placed, **982 live** at wave end, 920 after 10 s, no
  FATAL/STACK OVERRUN.
- D3 (join gating) present and compiling: registration/heartbeat send `limits_id` from
  `limits::LocalNetworkId()`, `LobbyState.limitsId` is parsed, and the join path refuses a mismatch with a
  descriptive message (FrontEndMenu ~line 6554). Untested end-to-end (needs two instances); the field is
  optional, so hosts without it are still allowed.

### Check on the D3 build (2026-11-08 10:35-10:46 ET, DeepSeek)

- No source change after Claude's 10:16 edits; the 10:35 build already contains them. Rebuild is a no-op and
  clean (0 warnings, 0 errors).
- **C11 on this build**: `-ai-actors 256 -render-states 16384 -spawn-test 3800`, Forge World — full **5+ minutes,
  no crash** (alive at end), `cached object render states 4096 -> 16384`, `POOL|cached object render states|841|16384|624`,
  3,800 objects spawned with 0 refusals, object pool peaked 3,869 then decayed (engine cleans unowned weapons).
- **Horde regression on this build**: 1,000 placed, **982 live** at wave end, no crash (10:38 run).
- **D3 join check**: compiles and is present (registration/heartbeat `limits_id`, `LobbyState.limitsId`, refusal
  message); **not testable yet** because the master-server backend does not store/return `limits_id`, so every
  host looks like an old host and joins are allowed by design.
- Claude's new docs reviewed: `docs/limits-light-volumes-port.md` (C8 plan) and `docs/limits-players-audit.md`
  (33 functions with 16 bounds, 32-bit player mask at +0x173C) — consistent with `docs/limits-players.md`.

### Follow-up 1: why the guard did not prevent the 0x68B21F crash (10:50 ET)

Evidence from the three crash runs (`c11.log`, `c11-trace.log`, `ab-norender.log`):

- **`Scheduler guard: active` IS in the match process** in all three (`Hook 'scheduler_guard_prepare' installed at
  0x7FFF1535ADD0` = engine base + 0x68ADD0, detour at noble-dev+0x630F0).
- In every run the guard **never waited**: there are no `waited N ms` lines. It printed exactly one line,
  1 ms before the crash: `Scheduler guard: object update wants 73759/73795/73793 jobs but the scheduler only has
  16384 ...` — the deliberate "request exceeds the scheduler, skip waiting" path.
- **Crash stack from the minidump** (`data\sandbox\noble-crash.dmp`, fault address 0x7FFF1535B21F):
  `haloreach+0x68B21F` (fault, the free-job pop) ← `haloreach+0x68AE23` (return address inside the *same*
  function whose entry 0x68ADD0 is hooked; call at 0x68AE1E) ← `noble-dev.exe+0x635EA` (the guard's detour —
  proof the hook was on the path) ← `haloreach+0x46A604` (object update dispatcher, call at 0x46A5FF) ←
  `0x13B43` / `0x13A8F` / `0x12EAE` (engine main/update loop).

So it is **not** a path that skips the prepare hook: the guard saw the request and skipped waiting because waiting
for 73,759 jobs is impossible, then the same function popped from an empty queue. The fix is to guard the pop
itself (0x68B20C / crash 0x68B21F: return "no job" instead of dereferencing an empty slot) or to raise the
scheduler; the current guard policy can only cover requests that fit.

Also note the request size: 73,759 jobs for ~4,069 objects (~18 jobs/object) — the dense 5,000-weapon grid makes
one object update ask for far more than the 16,384-job scheduler.

### Follow-up 2: what 16,384 / 6,656 are, and where 65,535 lives (10:52 ET)

Pool report, same map (forge_halo), V16 vs V16p:

| Pool name | V16 (no particle profile = stock values) | V16p | Recipe meaning |
|---|---|---|---|
| `particles` | **2,048** | **16,384** | CPU particle pool (2,048 -> 16,384) |
| `particle_emitter` | 832 | **6,656** | CPU emitters (832 -> 6,656) |
| `c_particle_emitter_gpu` | 832 | 6,656 | GPU emitters |
| `c_particle_emitter_gpu::s_row` | 448 | **4,096** | GPU particle rows (448 -> 4,096) |
| `particle_location` / `particle_system` | 576 | 4,608 | per-emitter data |
| `cheap_particle_emitters` | 200 | 200 | unchanged |

So **16,384 is the CPU particle pool** (`particles`), and **6,656 is the emitter pool** — not the 65,535 cap.
The **65,535 figures are per-frame GPU budgets/counters, patched in code, not data-array pools**, which is why
they never appear in the pool report. Verified in the built DLLs (file offset = RVA − 0xC00):

- file 0x27B5F2: V16 `40 02 00 00` (576) → V16p `FF FF 00 00` (**65,535**) — GPU span/attribute buffer capacity
- file 0x279171: spawn-queue admission changed to accept up to **65,534** (`41 81 FE FE FF 00 00 90`)
- file 0x1D48FD: GPU row redirect `E9 ...` (rows 448 -> 4,096)

That matches the particle guide: "GPU particle rows 448 → 4,096", "usable GPU particles across all GPU emitters
7,168 → 65,535", "new GPU spawns per frame 576 → 65,535". The CPU particle pool and emitter pool are the two
numbers the pool report can show; the GPU budget is enforced in code.

---

## C8 light volumes — port progress (2026-11-08 11:30-11:55 ET, DeepSeek)

Claude handed the C8 port over; the plan is `docs/limits-light-volumes-port.md`. **The port is written and
builds, but the Full profile still crashes at map load.** Exact state below so this can be picked up without
redoing the work.

### Done

- **`tools/limits/lv_port.py`** (new): extracts V4's seven relocated routines and five small blocks from
  `V4_append_at_00C9DBD8.bin`, places them 16-byte aligned in the free .deltrac room after the particle block
  (base **0xAAE8400**, 2,654 bytes, ends 0xAAE8E5E before .hkwork at 0xAAE9000), and recomputes **every** branch
  rel32 and RIP-relative disp for the new address with capstone (`imm_offset`/`disp_offset`; plain
  end-4 arithmetic is wrong for instructions with an immediate after a RIP operand). All targets verified to
  land in stock code/data or the new block; blocks match V4's intended jumps (0x12FCC, 0x270BB8, 0x284C8D,
  0x2848DD, 0x11BA33, 0x11B98F).
- **`make_v16_particles.py`**: `--profile full` (profile id 3), `Layout` gains LV render/main extras and the
  transparent storage (records 0x2C0000 + 64 KB queue block + indices 0x8000 at the end of .hkwork), and
  `apply_rows` applies the 43 functional LV overwrite rows (PE-header / bank / snapshot rows skipped; the 3
  particle operands inside the relocated effect init set to 4,608 / 16,384 / 6,656). The 95-row guide is
  reproduced by the parser; every applied row is checked against the guide's original bytes.
- **Zero extras still reproduce the baselines**: V16 `b8810c3b…`, V16p `47bc0723…` (checked after the port).
  Full profile: 14,201,950 bytes, sha256 `965453e2…`, render 0x3FF0000, main 0x3230000, checkpoint 0x7A80000,
  total 0xDEE0000, image 0x2495A000.
- **Profile::Full plumbing**: enum, name "Full (V16 + particles + light volumes)", `_ext_full` save suffix,
  `full.nlp`, settings clamp, ENGINE LIMITS label "FULL (V16 + PARTICLES + LIGHT VOLUMES)", and
  `noble-dedicated -limits full`.
- **Unwind**: no .pdata slots, so `lims::RegisterUnwindTables` (EngineLimits/Loader, Full only) registers six
  RUNTIME_FUNCTIONs with `RtlAddFunctionTable`, reusing the stock unwind RVAs (0xAA49D8, 0xA9A878, 0xA9A83C,
  0xA9ABF4, 0xA9A880, 0xA9ABD8); msg_append has no unwind info in stock or V4, so none is registered for it.
  Log confirms "registered 6 unwind entries" at load, and the code bytes are sanity-checked first.

### What still fails: boot crash at haloreach+0x2C2E0B

The Full profile loads, registers unwind info, then faults ~10 s in at **0x2C2E0B** (function 0x2C2CE4,
`movsx rax, byte ptr [rbx+2]`, rbp=0; called from 0x26C97B in 0x26C6DC). This is a light-volume/effect
initialization path with a bad datum pointer.

**Bisect (each variant booted 10 s):**

| Variant (rows applied on top of V16+particles+LV layout) | Boot |
|---|---|
| allocator rows only (7: five pool 160→65,535 calls + two queue initialisers) | **OK — pool report shows `light_volume 65,535`, `light_volume_location 65,535`, `light_volume_system 65,535`** |
| + the four admission rows | OK |
| + GPU profile-storage row 0xBB7650 | OK |
| + transparent draw capacities | OK |
| + the 17 transparent draw address rows + record pointer | OK |
| **+ parent rows 0x114D31 / 0x11ADCB / 0x11AD85 (parent effect-location 65,534 + admission redirect)** | **crash 0x2C2E0B** |
| **+ message queue redirects (effect init 0x114C88 + six message queue redirects)** | **crash 0x2C2E0B** |

So the code relocation, the bank layout, the light-volume pools at 65,535 and the transparent storage are all
good; the remaining failure is in the **parent effect-location** and **effect-message queue** paths. Hypothesis:
both need the storage/control structures V4 keeps in its `.lvlimit` virtual section; the queue control block
(0xCD90040) and parent backing pool are not provided by our .hkwork placement. The 20 "snapshot reference" rows
were skipped because V16 already repointed those references (STATE_BANKS) — patching them fails the stock-byte
check (V16 bytes differ), so they are not simply the queue block.

Next steps for Claude:
1. Diff the pointers the parent/message code builds at init against V4 (0xCD90040 block, parent pool 65,534 ×
   record): find what the stock/V16 build supplies and what our build leaves zero.
2. Or apply parent+message rows one at a time (0x114D31, 0x11ADCB, 0x11AD85, 0x114C88, then each message
   redirect) to isolate the first failing row.
3. The crash is reproducible in seconds with settings `extendedLimits=3` and `data/limits/full.nlp` present.

**Do not ship full.nlp to `build\bin\Release\data\limits` yet** — selecting FULL currently crashes. The
repo copy stays for iteration; without it, EngineLimits falls back to Stock with a reason.

### Scheduler ceiling (known limit, logged, not fixed)

The V16 scheduler holds **16,384 jobs** (inline arrays at body +0x230010 / +0x260038 / +0x290038). A dense
5,000-object spawn asks for **~74,000 jobs** in one object update; the guard logs "wants 73,795 jobs but the
scheduler only has 16,384", skips waiting (it cannot wait for more jobs than exist) and the engine's free-job pop
at 0x68B21F then hits an empty queue. Confirmed with and without the C11 render-states change; left unfixed.

---

### C8 follow-up (12:00-12:14 ET): root cause found and fixed

Claude's main-bank read was right, with one detail: the Full builder grew the checkpoint/total operands but
never wrote the main-bank operand, so the engine's bank 4 still had V16's 0x27D0000 while the LV pools were
allocated on top of it.

BANK diagnostic (permanent, in PoolReport.cpp). The game-state bank table is the static array at RVA 0x27BF270:
8 records, stride 0x18F0, +0xA0 = capacity, +0xA8 = bytes bumped. The reporter now logs BANK|n|used|capacity
on its first pass and after every map/limit change, warns when used > capacity, and the -pool-report-exit path
logs the banks before the slow full-memory scan.

Pre-fix numbers on the crashing Full profile (Forge World):

    BANK|0|125992|131072
    BANK|1|2222752|8519680
    BANK|2|50740|65536
    BANK|3|58613580|67043328
    BANK|4|51649416|41746432      <-- 9,902,984 bytes over; crash at 0x2C2E0B right after
    BANK|5|5124|65536
    BANK|6|338488|1572864
    BANK|7|0|589824

Fix. MAIN_OPS = [0x155A9] (the main-bank and rollback-copies operand) is now written from Layout.main, which is
V16's 0x27D0000 + V4's 0xA60000 delta + a measured margin. The earlier crash bisect (parent rows / message
redirects) was a symptom: those rows are the allocations that pushed bank 4 over.

Margin from measurement. With an 8 MB margin the profile boots and bank 4 sits at 51,649,416 / 61,014,016; the
figure stays flat through Firefight (the pools are all allocated at map load). Final margin is 0x20000:
main = 0x3250000 (52,756,480 bytes), i.e. 1,107,064 bytes (~1.06 MB) above the measured peak.

Verification:
- Boot: BANK|4|51649416|52756480, POOL-REPORT-DONE, light_volume / light_volume_location / light_volume_system /
  c_light_volume_gpu / ::s_row all 65,535.
- 5-minute plasma-heavy Firefight (ff_unearthed, heroic): alive after 315 s, no FATAL, frame 38,700, ~6.9 GB
  private. Bank 4 never moved off 51,649,416.
- Zero extras still reproduce V16 b8810c3b and V16p 47bc0723.
- Final Full: 14,201,950 bytes, sha256 3a80f35d, render 0x3FF0000, main 0x3250000, checkpoint 0x7AA0000,
  total 0xDF40000, image 0x2499A000.

full.nlp stays out of build\bin\Release\data\limits (repo copy only) until Tyler/Claude accept it.

---

### Static-list guard for the F6 horde crash (12:30-12:41 ET)

Claude reported a FULL crash at haloreach+0x1D4F96 during an F6 horde: the append at 0x1D4F85 to one of
four 512-entry lists (base RVA 0x477E700, stride 0x1204, count at +0x1200) has no bounds check.

Fix (all extended profiles): the 7-byte count load at 0x1D4F85 is replaced with a jump to a cave in the
.deltrac gap (v16 0xAAE8200, v16p 0xAAE8400, full 0xAAE8E60). The cave reloads the count, compares with 0x1FF,
skips the append (to 0x1D4FA1) when full, and otherwise resumes at 0x1D4F8C. Verified by disassembly:

    movsxd rcx, [rdx+0x1200] / cmp ecx,0x1FF / jge 0x1D4FA1 / jmp 0x1D4F8C

A scan of every .pdata function found only this writer of the four counts (the reset routine at 0x1D4DD5 just
zeroes them with a 0x1204 stride loop), so one site covers all four lists. Entries past the limit are dropped
instead of overrunning the array.

Verification:
- All three profiles boot with the guard (V16, V16p, Full; POOL-REPORT-DONE, no FATAL).
- FULL, -ai-actors 1024, Firefight on ff_unearthed, F6 x10 (1,000 actors queued): ran 12:35:08-12:40:26
  (5 min 18 s, frame 9,000) with **no crash**. The old build faulted at 0x1D4F96 at ~250 actors.
- The guard is emitted only when the AI/render extras are on, so the zero-extra proof still holds:
  --ai-bank-extra 0 --render-extra 0 reproduces GreenKnight's exact V16 b8810c3b / V16p 47bc0723.
  Default (extras on) builds: v16 007a4dac, v16p 6403f044, full d5b0122e. Only the .deltrac tail and the
  7-byte site differ from the unguarded images.

Capacity note: the lists still hold only 512 entries each; the guard makes the overflow safe, it does not
raise the limit. If the renderers need more than 511 entries per list, the next step is moving the four lists
to .hkwork at 4 x 4,096 entries and repointing the two references (0x1D4DD5 reset, 0x1D4F70 append) - the
consumer side has no other direct references in .pdata functions.
