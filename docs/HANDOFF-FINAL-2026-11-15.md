# Project Noble — FINAL HANDOFF (2026-11-15, 10:30 ET)

Written at the close of this session. Paths use Windows form. Repo: `D:\Project-Noble-Closed-Source`.
Companion copy: `C:\Users\tyler\Desktop\Compendium\HANDOFF-FINAL-2026-11-15.md`.

---

## 1. TL;DR / current state (verified just before writing this)

| Item | State |
|---|---|
| Repo | `D:\Project-Noble-Closed-Source` |
| Branch | `wip-merge` checked out; `main` = `wip-merge` = `origin/main` = `origin/wip-merge` |
| Commit | **`eea9868`** — `Release v0.19.103: lobby share codes, drawer filters, click fixes` |
| Version | **0.19.103** (`CMakeLists.txt`, `Version.hpp`, `BuildIdentity.hpp`, `noble.rc`) |
| Tracked working tree | **clean** — no staged/unstaged/untracked *tracked* changes |
| Untracked files | pre-existing assets/docs/tools only (see §7); not created by this final work |
| Gates | **both pass** — `noble-dev.exe` and `noble-client.exe` rebuilt now, exit 0, both FileVersion/ProductVersion **0.19.103.0** |
| Public release | **v0.19.103 published and latest**, draft=false, assets verified (see §5) |
| Game processes | none left running |
| Goal / task board | goal was marked blocked before the release decision (multiplayer hold); release was completed afterwards. Task board: task-2/4/7 completed; task-1/6 in progress; task-3 external; task-8 parked; task-9 on hold (§6) |

The repo is **not literally `git status`-empty** because the checkout contains many pre-existing untracked asset/tool/doc files (listed in §7). There are **no tracked modifications**; the release commit is on both branches and pushed.

Build command used:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' `
  --build build --config Release --target noble-dev noble-client
```

---

## 2. What is done and released

### v0.19.103 (this release, commit `eea9868`)

Release notes: `release-notes\v0.19.103.md`. Changes since v0.19.102:

- **Lobby share codes** — 23-char `NB1-XXXX-XXXX-XXXX-XXXX` codec (`src/network/InviteCode.*`), COPY CODE in the lobby
  HOSTING panel, JOIN WITH CODE on the Main Menu, Server Browser and Social screen. Public and private lobbies; private
  stays unlisted but the code joins. (Core `88d4937`, Social button `ca770e2`.)
- **Variant drawer search + category filters** with `N OF M VARIANTS` (`ca42a24`, `src/ui/FrontEndPickers.cpp`).
- **Server-browser Refresh status + pinned FAVOURITE/COPY INVITE footer** (`0f47a63`, `src/ui/ServerBrowser.*`).
- **Menu click fixes + toggle crouch / toggle zoom bindings** (`0efb619`): Game Options full-row clicks, clickable
  What's New hint, controller toggle crouch, M&K toggle zoom.
- Version bump to 0.19.103 + in-game What's New entry (`src/ui/ChangelogData.hpp`) + release notes.
- Two developer test hooks are also in the tree from this session (dev/test only, not user-facing):
  `-armor-effect-test <0..6>` (`64e6d22`) and `-third-person-front` (`b1680d1`).

### Shipped earlier in the 0.19.10x line (from this session's queue work)

These were already on the public line before the v0.19.103 bump:

- Armory scroll/FOV fixes (including FOV save no longer zooming the Armory).
- Campaign/Custom Games collapsed MODDED mod sections (click to expand, FOLD to collapse).
- Campaign **MAX PLAYERS** row (4/8/12/16) wired into HOST CO-OP.
- Custom-games cleanup.
- Campaign search clamp + ASCII hyphen replacing the missing em-dash glyph; campaign **skulls row** (awaiting Tyler retest).
- Server browser style-2 blue-glass layout as default, Refresh feedback, pinned actions, mod/version warnings.
- Settings Game Options camera slider alignment; Game Options row clicks; What's New hint click.
- Controller toggle crouch / M&K toggle zoom.
- Visor **white bug fixed** (`96fd2c4`): every Armory finish now writes a valid stock engine visor slot (nearest stock
  colour). Extended finishes stay exact only in the Armory preview; matches show the nearest stock colour. No white.
- Housekeeping: 3,313.7 MB of obsolete `noble_menu.map.*` backups deleted; active maps/packs kept.

### Release packaging / publishing

- Release version/references bumped and committed in `eea9868`; both gates built 0.19.103.
- Published with `tools\publish-release.ps1` (base + optional Forge World pack + checksums).
- First publish attempt hit a transient GitHub connection error; the retry completed. Package was restaged on the retry,
  so the final uploaded hashes are the ones in §5.

---

## 3. Proven vs unproven

### Proven (evidence in this session / this checkout)

| Claim | Evidence |
|---|---|
| v0.19.103 release is public/latest and assets uploaded | GitHub API: release `v0.19.103`, draft=false, published 2026-11-15T14:57:37Z; latest release check; four assets `uploaded` |
| Release zips match local build | GitHub asset digests equal local SHA-256 (§5); zip contains `Noble.exe`, README, `version.txt` = “Noble 0.19.103 release build”, `noble_menu.map`, `elite_armor.map` |
| Both gates build and report 0.19.103 | Rebuilt at handoff time; exit 0; exe FileVersion/ProductVersion 0.19.103.0 |
| Lobby-code codec logic | `InviteCodeSelfTest()` reported **0 failures** earlier in the session; UI captures `join_code_main.png`, `join_code_modal2.png`, `join_code_lobby.png`, `social_join_code.png` |
| Share-code UI (Main Menu / Server Browser / Social / lobby HOSTING) | Screenshot captures in `build\bin\Release\data\userdata\claude-tests-2026-11-11\` |
| Variant drawer search/category, server browser pinned footer, Game Options rows, toggle rows | Screenshot captures in the same test folder (`settings_toggle_rows.png`, drawer/browser/server captures) |
| Visor fallback: no white, stock colours correct, extended finishes armory-only | Fix committed `96fd2c4`; release notes and prior Armory/in-match captures |
| Blue/cyan Legendary armour-flame recolour works in game | Palette-reference patch on `head_fire.particle`, rebuilt Forge World, front-third-person captures `blueflame_0.png`, `blueflame_1.png` |
| `-third-person-front` capture camera works | `front_cam_final.png` (local Spartan visible from front, unfocused) |
| Release packaging/reproducibility | Dry run + publish both succeeded; hashes/digests verified |

### Unproven / not done

| Item | Status |
|---|---|
| **Real two-PC lobby-code join** (host → code → second PC → join, public and private) | **Not tested** — needs Tyler / a second machine (task-3) |
| Tyler's live retest of the **campaign skulls row** and other late UI items | Awaiting Tyler's hand check |
| **Exact custom per-player visor colour in matches** | Not implemented. Only the stock-slot fallback shipped. Map/HREK shader routes are closed; shared-heap/engine visor-table work is a separate RE project (task-1/task-5) |
| **Selectable colour-variant armour effects** (blue flames etc. in the Armory list) | Not implemented. Only the dev recolour *proof* exists; exposing entries needs engine effect-table/lookup work (task-6) |
| **Multiplayer 128 players** | Explicitly on Tyler's hold (task-9); launch blocks still cap co-op at 16 |
| **Parked items** (flashlight open-value reset, two-PC multiplayer, Armory knife chests/emblem textures slots 73/74, undersuit colour, Elite shine/lights, Forge-palette Option 2, Marine animation) | Untouched (task-8; only after main queue + QA + housekeeping) |
| Real network gameplay/multiplayer smoke test after release | Not performed in this session |

Known limitation to repeat to users: extended visor finishes look exact in the **Armory preview** but use the **nearest
stock colour in a match**; that is the accepted fallback as of 2026-11-15.

---

## 4. Half-finished / work in progress

### Branch / commits

- `main` = `wip-merge` = `eea9868`; both branches and origin are in sync (`origin/main` = `origin/wip-merge` = `eea9868`).
- No feature branches; no source-repo tags were created for v0.19.103.
- The GitHub release tag for v0.19.103 lives in the **public release repo** `O7OF/Project-Noble-` and its tag/source archive
  points at that repo's `main` (which is older than this private source tree). The uploaded **binaries** were built from
  private commit `eea9868` and their hashes were verified. If correct source archives matter, update the public release
  repo/tag separately; do not assume its `main` matches the binaries.

### Task board (shared)

- **task-1 (#14-A visor RE)** — `in_progress`, owner lead. Shader-tag route closed; remaining route is shared-tag-cache
  rmsh tint or engine visor-table extension. Read `project-docs\RE-VISOR-COLOR-2026-11-15.md`.
- **task-5 (#14-B per-player visor)** — pending, blocked by task-1.
- **task-6 (#15-B selectable armour effect variants)** — `in_progress`, owner lead. Architecture mapped; next is the
  profile effect-index → event-StringId path. Read `project-docs\RE-ARMOR-FX-COLORS-2026-11-15.md`.
- **task-3 (#13-B real two-PC lobby-code test)** — pending, external dependency.
- **task-8 (parked items)** — pending; only after everything above.
- **task-9 (#16 multiplayer 128)** — on Tyler's hold.
- **task-2, task-4, task-7** — completed.
- QA/release prep and publishing (task-4) completed.

### Temporary probes / test hooks

- No temporary `VisorHeapProbe.*` or other scratch probe files remain in `src/engine`. All experiments from this session's
  end were removed and both gates rebuilt clean before the handoff.
- Committed dev-only test hooks remain and are useful:
  - `-armor-effect-test <0..6>` — equips/saves a profile armour effect for match captures (commits `64e6d22`).
  - `-third-person-front` — mirrors the third-person camera for front captures (commit `b1680d1`).
  - `-scoreboard-at`, `-menu-*`, `-server-browser-demo`, `-armor-focus`, etc. documented in `TODO-MASTER` and
    `QA-VERIFY-2026-11-15.md`.
- `appearance.dat` in `build\bin\Release\data` was backed up/restored around test runs; no `.bak` backups remain. If a
  test hook is used again, back it up first.

### External (HREK) test-asset state — read before future asset tests

`D:\Steam\steamapps\common\HREK` is **not** part of the repo and is currently in a test state:

- `tags\...\armor_fx\particles\head_fire.particle` — original 20,629-byte stock tag (timestamp 2010-09-14) restored.
- `tags\...\armor_fx\particles\head_fire_blue.particle` — **test variant** left behind (20,053 bytes, created 2026-11-15
  08:26) from the blue-flame proof. Unused by stock tags; delete or keep for future variant work.
- `maps\forge_halo.map` — 796,790,784 bytes, rebuilt 2026-11-15 09:10 during visor-shader tests. The visor shader source
  tags were restored afterwards but the **compiled map still has the test shader values baked**. Rebuild
  `tool.exe build-cache-file levels\multi\forge_halo\forge_halo` before using that map for other asset tests, or load
  stock maps.
- All 35 `*visor*.shader` source tags were restored from backups; no `.bakvisor`/`.bak` files remain.
- The HREK `forge_halo.map` is only used with Noble `-map-override` and was never shipped; release Forge pack is built
  from the repo/`data` path, not this test map.

### Local build/dist artifacts

- `dist\` contains the published v0.19.103 zips + checksums (357.1 MB + 652.6 MB); they are uploaded and can be deleted to
  reclaim space. `dist` is not part of the release commit.
- `build\bin\Release\` contains both 0.19.103 gates and test capture/userdata folders; not committed.

---

## 5. Release/version/commit evidence

- Source commit: **`eea9868`** `Release v0.19.103: lobby share codes, drawer filters, click fixes`
- Branches: `main` = `wip-merge` = `origin/main` = `origin/wip-merge` = `eea9868`.
- Version: **0.19.103**; both `noble-dev.exe` and `noble-client.exe` report 0.19.103.0.
- Release page: https://github.com/O7OF/Project-Noble-/releases/tag/v0.19.103
- Base asset: `Project-Noble-v0.19.103-win64.zip` — 374,494,102 bytes (357.1 MB)
  - SHA-256 `812606825f61d400e4ea7c7ff7808ffb2087c84c50399d63a6ded739c332c834`
- Forge pack: `Noble-ForgeWorld-v0.19.103-win64.zip` — 684,304,362 bytes (652.6 MB)
  - SHA-256 `be6931434743a6edf59bc09b2c745358588114b422db3a80aecb6a756cde932b`
- Both `.sha256` files uploaded; GitHub asset digests match the local hashes; release is public, draft=false, latest.
- Package checked: `Noble.exe`, `README.txt`, `version.txt` = “Noble 0.19.103 release build”, `data/menumaps/noble_menu.map`,
  `data/armor/elite_armor.map`; no dev binaries, `.pdb`, saves or userdata.

---

## 6. Next agent — do this first

1. **Read** this handoff, then `knowledge\project-docs\WORKFLOW-CONTINUOUS-2026-11-15.md`,
   `WORKFLOW-LEDGER-2026-11-15.md`, `QA-VERIFY-2026-11-15.md`, `RE-VISOR-COLOR-2026-11-15.md`,
   `RE-ARMOR-FX-COLORS-2026-11-15.md`, `PLAN-LOBBY-CODES-2026-11-15.md`, and
   `knowledge\desktop-guides\TODO-MASTER-2026-11-14.md`.
2. **Verify state before touching anything:** `git status --porcelain`, `git rev-parse --short HEAD` (expect `eea9868`),
   both gates build (§1 command). Do **not** republish v0.19.103 or reuse its version; the next release must be a new
   version (e.g. 0.19.104) if anything changes.
3. **Ask Tyler for the pending decisions before large work:**
   - lift the **multiplayer 128** hold?
   - authorize the **visor shared-tag-cache/engine visor-table RE project**, or keep the stock-slot fallback indefinitely?
   - does he want the **two-PC lobby-code test** scheduled?
4. If continuing engineering without a decision, the first ready main-queue work is **task-1** (visor shared-heap RE). The
   next concrete step documented in `RE-VISOR-COLOR` is to locate the shared tag-cache table/global (not a blind memory
   sweep) and patch validated rmsh visor parameter blocks; the background memory sweep was too slow and was removed.
   Task-6 (armour effect variants) needs the profile-index → event-StringId trace. Do not touch task-8 parked items or
   multiplayer until the holds/decisions are resolved.
5. **Two-PC lobby test (task-3)** requires Tyler: private lobby → copy NB1 code → second PC join; repeat public. Record in
   `QA-VERIFY` + ledger.
6. **Testing rules:** AOC monitor (`DISPLAY1`, X=-1920) only; never Samsung primary; always `-NoFocus`; do not steal
   Tyler's mouse/focus; never test while he is playing. Harness/captures:
   `build\bin\Release\data\userdata\claude-tests-2026-11-11\tools\` and `...\captures`/`*.png` in that folder.
7. **Asset hygiene:** before asset experiments, note the HREK test state in §4; rebuild HREK Forge World from restored tags
   if using `-map-override`, and back up `appearance.dat`/any tags you patch. Restore originals afterwards.
8. **Release process for a future version:** bump all version references, add `release-notes\vX.md`, add the
   `ChangelogData` entry, build both gates, commit/push `main` + `wip-merge`, then
   `tools\publish-release.ps1` (it targets `O7OF/Project-Noble-`). Never publish without Tyler's explicit go.
9. **Mirror docs:** after editing `knowledge\`, copy changed files into the matching `C:\Users\tyler\Desktop\Compendium\`
   path and verify the hashes.

---

## 7. Untracked files present at handoff (pre-existing; do not commit blindly)

`git status --porcelain` currently reports only untracked items — these are pre-existing asset/doc/tool files from earlier
sessions, not produced by the final handoff work. Categories:

- `assets/ui/noble/{emblems,loading,screens}/`
- `data/credits/`, `data/game_variants/`, `data/map_variants/`, `data/menumaps/menumaps.zip`
- `docs/` older notes/handoffs/`engine.rar`
- `knowledge.rar`, `knowledge.zip`
- overnight/`run_*` bat/ps1 scripts
- `tools/hrek/`, `tools/limits/`, `tools/megapack/`, `tools/noble-*.ps1`, `tools/swarm/`
- `release-notes/v0.19.103.md` **is tracked** (committed in `eea9868`), not in this list.

If the next agent needs a clean tree for a build, leave these untouched; none are required by the build except the ones
already referenced from tracked code. Do not `git clean` this checkout.

---

## 8. Useful exact commands

```powershell
# Build both gates
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' `
  --build D:\Project-Noble-Closed-Source\build --config Release --target noble-dev noble-client

# Test harness example (unfocused AOC)
$tools = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\tools'
& "$tools\launch_noble.ps1" -Exe noble-dev.exe -Arguments '-map 20_sword_slayer -mode multiplayer -quit-after 20 -NoFocus'

# Publish a future release (after commit/push and Tyler approval)
cd D:\Project-Noble-Closed-Source
.\tools\publish-release.ps1 -SkipBuild

# Mirror knowledge to Compendium for one file
Copy-Item 'D:\Project-Noble-Closed-Source\knowledge\<file>.md' 'C:\Users\tyler\Desktop\Compendium\<file>.md' -Force
```

---

## 9. One-line summary for the next session

**v0.19.103 is shipped from `eea9868` (main=wip-merge), repo has no tracked changes and both gates build 0.19.103.0.**
Main queue is done except exact per-player visor colour (fallback accepted) and multiplayer 128 (on Tyler's hold);
todo next is Tyler decisions, then visor shared-heap/engine RE (task-1) or armour-effect variants (task-6), and the
two-PC lobby-code test (task-3).
