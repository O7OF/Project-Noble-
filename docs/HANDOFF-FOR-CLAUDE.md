# Handoff for Claude — Project Noble

Written 2026-10-30 16:25 ET by the previous agent. This is the concise, self-contained brief: read it top to bottom and
you can pick up without reading the (very long) `docs/HANDOFF.md`, which remains the detailed log.

---

## 0. Current state — read these three files first (2026-10-31)

| file | why |
|---|---|
| `docs/START-HERE.md` | one-page orientation: repos, build command, flags, current focus, hard rules |
| `docs/TODO.md` | the prioritized list **and the claim board** — `- [~] @name ...` means somebody is on it |
| `docs/TEAM-WORKFLOW.md` | how Claude and DeepSeek share this repo without collisions |

Heads: client `97989d1`, backend `b8fb2ad` (Render). Both green, tree clean.

Landed since the previous version of this brief: campaign co-op fix (`0b83b1c`, launch block `0xE8` player count),
forge online, backend 1.3.0 (relay heartbeat, `/api/relay/stats`), lobby identity pass-through (`b8fb2ad` + `663cae2`),
Noble scoreboard overlay (`7cef78c`), third-person camera on F5 (`97989d1`). Evidence screenshots live in
`docs/screenshots/`.

In progress: third-person distance/shoulder tuning and the settings/host gate. Next unclaimed items are in `TODO.md`.

## 1. What this is

**Project Noble** is a C++20 Windows client that runs *Halo: Reach* from the MCC install as a **standalone game with its own
front-end menus** — its own main menu, campaign/multiplayer/Firefight setup, server browser, settings, career, Armory — drawn
as an ImGui overlay over `haloreach.dll`, which is loaded without MCC's launcher. MCC itself is read-only; Noble only reads
its data. Steam ownership is checked.

- Repo: `D:\Halo-Reach-Community-Edition` — remote `https://github.com/O7OF/test-build-01`, branch `main`.
- Version: `src/common/Version.hpp` → `0.8.3 (Menu Cleanup & Controls)`.
- The user, **Tyler (`WeabooTyler117`)**, is not a C++ developer. He tests by playing and reports what looks wrong. Keep
  messages plain; never paste diffs at him; give him one command per code block.
- Repo owner/server host is **O7OF**. A push never ships the exe (`build/` is ignored) — players must rebuild.

## 2. Build and verify

```powershell
taskkill /IM noble-client.exe /F 2>$null
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build --config Release -- -m
```

Expect **0 errors, 0 warnings**. Never use `build.bat`. `LNK1104` means the client is still running.

Headless / capture harness (run from `build\bin\Release`):

```powershell
.\noble-client.exe -menu-screen armory -menu-capture 90     # writes capture_<frame>.bmp and exits
```

- Screens: `main, campaign, skulls, firefight, custom, forge, servers, settings, armory, mods, career, lobby, social`.
- `-menu-tab N` = Settings/Career tab **and** VANILLA(0)/MODDED(1) elsewhere.
- Other hooks: `-test-map-gametypes` (headless suites, writes `gametype-selftest.log`, exit 0/1), `-auto-play`,
  `-auto-play-variant <stem>`, `-auto-play-map <scenario>`, `-pause-at <s>`, `-scoreboard-at <s>`, `-quit-after <s>`,
  `-armory-scene-index <n>`, `-armory-focus "<slot>"`. The timed ones forward to the relaunched child.
- A bare CLI launch **cannot start a match**: it needs `-player-name/-service-tag/-account-user/-account-token/-user-id/
  -account-dev` (the menu hands these over). Without them the engine logs `'no game selected'` and pumps no frames.
- `tools/test-modes.ps1` drives real launches (15 game types; `-Maps` adds Slayer on all 20 maps; `-Only`, `-PlaySeconds`).
- `tools/check-settings-wired.ps1` audits every `SettingsData` field for a real consumer.

Verify UI work with **actual captures**, then look at the image — Tyler's complaints are visual, and a plausible-looking
diff has been wrong before.

## 2b. Two repositories - netcode and server scripts go to ServerTestDeploy

| what | repo |
|---|---|
| Client, engine host, menus, tools | `O7OF/test-build-01` (this checkout) |
| **Backend, netcode, server scripts** | **`O7OF/ServerTestDeploy`** - deployed to `https://outbreak-z-test-server.onrender.com` |

Commit `19207fb` removed `server/` from this repo; the backend is centralised in ServerTestDeploy and nothing
server-side should ever be committed here again. Workflow:

```powershell
.\tools\server-repo.ps1            # clone next to this checkout (or fast-forward an existing clone)
# edit server.js / src/* inside that clone
node --check server.js
git -C <clone> add -A; git -C <clone> commit -m "..."; git -C <clone> push origin main
```

Diagnostics that exist there (use them before guessing): `GET /api/health` reports version, `commit`, and relay counters;
`GET /api/relay/stats` lists every live relay room as `{ session, hasHost, guests }` plus connection/packet counters - that
one look separates a server/relay failure from an engine failure.

Current backend state: `4a520d1` - 25 s WebSocket heartbeat (kills idle-socket churn), timestamped relay logs,
session timeout 90 s, lobby rate limit 240/min.

## 3. What the previous session changed (all pushed, `main` clean)

| commit | what |
|---|---|
| `6048d9a` | career: `bonusCredits` — granted cR that is spendable but does **not** count toward rank |
| `da95dcb` | armory: **deleted** the OBJ preview (`ObjModelPreview`, `ArmoryPreviewLayers`, `armoryPreview` setting + row) — only Reach's in-game model is allowed |
| `c3a93ec` | armory: back on the in-game model (the OBJ re-export is a different mesh; Tyler rejected it) |
| `cafb0f4` | docs: what MCC actually ships for emblems (see §5.1) |
| `d89944e` | (superseded) OBJ renderer: eye-strafe pan + made it default |
| `d0026f1` | (superseded) designed stage behind the OBJ renderer |
| `a159559` | armory cameras mapped by capture; `-armory-scene-index` / `-armory-focus`; left/right shoulder proven impossible on the engine path |
| `80eba4a` | (superseded) `armoryPreview` switch |
| `d136a00` | Relaunch keeps the old window on screen until the new process has one — no more desktop flash |
| `9308d71` | armory: appearance writes coalesced (250 ms) so holding an arrow key no longer re-instantiates the model |

## 4. Tyler's save state — handle with care

He asked for a **career reset with money**: level back to Recruit, but 1,000,000 cR to spend on unlocks.

- `data\userdata\career.json`: `credits` (lifetime earned) **0**, `creditsSpent` 0, `bonusCredits` **1,000,000**,
  `armoryOwned` empty, `armoryUnlockAll` **false**, all stats/history/challenges/commendations zeroed. Career screen shows
  Recruit, 0/7,500 cR.
- Backups that must not be lost:
  - `data\userdata\career.backup-20261030-154234.json` — the whole pre-reset career (rank 49, 1e9 cR).
  - `data\appearance.before-reset-20261030.json` — his original Spartan look (Hilands helmet, Emile chest, Kat tricorder,
    Linda shoulder, Adrianna knee, blue flames, Jorge voice, colours 15/14).
- **The trap that bit:** clearing `armoryOwned` makes the Armory treat equipped pieces as "isn't unlocked yet; back to
  'DEFAULT'" and it then **saves the default look over `data\appearance.json`**. Do not clear ownership without backing up
  `data\appearance.json` first.

## 5. Open work, in priority order

### 5.1 Emblem background / foreground picker (in progress, ~30 % done)

Tyler wants Reach's emblem editor: pick a **background** shape and a **foreground** symbol.

- Data reality (`D:\Steam\steamapps\common\Halo The Master Chief Collection\Data\UI\playercustomization.xml`): 330
  `<Emblem id image1="e_002_p" image2="e_002_s" image1_small image2_small name>` entries. `image1`/`image2` are the
  **primary/secondary colour layers of one finished emblem**, not a reusable backdrop + symbol pair, and the backgrounds
  are effectively unique per emblem. Reach 360's own emblem sets are **not** in MCC or in Noble's data.
- Already landed: `UiArt::GetEmblemBackdropLayers()` / `GetEmblemSymbolLayers()` / `EmblemLayerTexture(image)` in
  `src/ui/UiArt.cpp` — ~225 layers each, loaded by name through `EmblemByName`.
- Remaining: two Armory identity rows (block starts near `ArmorCustomizer.cpp` "IDENTITY" rows, ~line 1638), storage for the
  chosen pair (profile or `SettingsData`), and composition where emblems are drawn — `FrontEndMenu.cpp:861` (`PlayerRow`)
  and `:3660` (Armory nameplate): draw backdrop layer then symbol layer.
- Tyler approved **prototyping the matrix from MCC's layers** (any backdrop × any symbol) rather than waiting for the
  original Reach assets.

### 5.2 Armory: the black chamber, and left vs right shoulder

- The Armory shows Reach's in-game scene: correct model, **unlit black chamber** behind it. A renderer swap was tried and
  rejected (wrong mesh). Code cannot composite behind the engine's 3D; the void is engine-drawn geometry (five approaches
  ruled out — depth copy crash, no `ClearRenderTargetView`, painting before draws is occluded, etc.).
- Remaining route: **data** — light or replace the chamber in `data\menumaps\mainmenu_all.map` (material/lightmap change, or
  swap the geometry for a backdrop). `ReachArmory` already walks the GLOBALS table, which is the entry point for that work.
- **Per-slot camera panning works** through the engine's own cameras (verified by capture): `0x0D` helmet, `0x0F` chest,
  `0x10` wrist, `0x11` utility, `0x12` knees, `0x13` shoulders, `0x14` colours; `0x0A`/`0x0C` full body; `0x15+` leave the
  armory (other menu scenes). `0x0E` is the roster scene, excluded.
- **Left vs right shoulder cannot be done with scene indices**: the map has one shoulder camera, and rotating the model
  (`SetYawRotation`, tested ±70°) changes nothing because the armory camera rides with the character (measured 5 % pixel
  difference = animated particles). Needs a camera with a lateral offset — engine camera globals, not found; the GLOBALS
  table only carries the scene index at `base + 0x24 * stride`.

### 5.3 Scoreboard modernisation

Reach's scoreboard is engine-drawn; Tyler says it "still looks like Halo Reach's scoreboard". Either restyle the engine's
screen (no handle found) or draw a Noble overlay over it, which needs per-player rows (name, team, score, K/D). The same
data is already parsed for the post-game results screen, so this is data plumbing, not new RE. `-scoreboard-at <s>` exists
for captures.

### 5.4 Campaign CONTINUE should resume the **last checkpoint**

CONTINUE currently relaunches the last campaign mission from its start (`FrontEndMenu.cpp`, the `kContinueEntry` row and
the launch choke point; description says "starts the mission again"). Real checkpoint resume needs Reach's save/checkpoint
state — nothing in Noble reads it yet. First step: locate the checkpoint block in the save data (compare two saves at
different points in one mission), then pass it to the new process.

### 5.5 Truly seamless transitions (no process boundary)

Transitions still cross processes: a new `noble-client.exe` is spawned for every match/mission and the old one exits.
`d136a00` made it a window-for-window swap, but it is not seamless. A single-process transition needs the engine's own
session control: `BlamEngine::ExecuteCommand()` (engine vfunc[9]) exists but **nothing calls it** and `BlamEngine` is not in
the running client's path. Proposed experiment: a `-exec-at <seconds> <command>` hook (same shape as `-pause-at`) and
dispatch candidates (`game_end`, `game_start`, `map_reload`, `quit`) mid-match, watching whether the engine returns to the
menu instead of tearing down. The previous notes say the engine cannot switch maps once a match started (RR also runs each
match as its own process), so treat this as research, not a port.

### 5.6 Older backlog (still open)

Vote-to-kick (needs a `NetworkTransport` kick message plus a roster in the pause menu/TAB), CHANGE TEAM (needs an engine
action; the row should only appear when a session reports teams), real solo pause (no engine pause command found), and
engine-side graphics (texture filtering, shadows, motion blur, draw distance) which only MCC can change.

## 6. Traps and house rules

1. **Standing instruction from Tyler: always update the handoff.** Every change gets a new
   `## 0. Pick up here (YYYY-MM-DD, HH:MM ET) - <what> (<name>)` entry at the **top** of `docs/HANDOFF.md`, then mirror to
   `C:\Users\tyler\Downloads\HANDOFF (2).md`, `PROJECT-NOBLE-CONTINUATION.md`, `MENU-REDESIGN-SPEC.md`. Commit and push,
   and close the client when done testing.
2. `career.json` `credits` = **lifetime earned**; the spendable balance is `credits - creditsSpent + bonusCredits`. Rank is
   `RankIndex(credits)`. Never hand out rank by raising `credits` — use `bonusCredits`.
3. `data\appearance.json` is the Armory profile and the client **rewrites it**; see §4 before touching ownership.
4. `settings.json` `playerName` has been clobbered to `Bob` before during account-less captures — check identity after any
   run that saves settings.
5. Layout contract: `RightColumnLayout(ui)` → `actions/strip/content`, `HeroBannerHeight()`, `BlurbBlockHeight()`
   (2-line cap), `PlayerRow(...)`; never pin rows to `actionsY`.
6. `SettingsPanel::Row()` **always returns true** — it is a layout helper. Action rows must test
   `m_rowHovered && ImGui::IsMouseClicked(0)` (or focused + Accept). This once opened a file dialog every frame.
7. Never create textures on the main-thread fallback draw path; warm art in `FrontEndMenu::Initialize`
   (`UiArt::WarmMenuBackground()`).
8. Post-processing: `GammaPass` in `ReachHost.cpp` is the only place Noble can change the picture. The C++ `GammaCB` must
   match the HLSL cbuffer exactly (16 vs 32 bytes crashed it) and never pass a null array to `PSSetSamplers`.
9. Relaunching needs the account flags listed in §2; test flags must be forwarded to the child (already handled).
10. Windows PowerShell 5.1: no ternary operator; `$j.someKey =` fails on a missing JSON key (`Add-Member -Force`); never
    assign to `$args`.
11. Minimised windows stop presenting, so captures and `-pause-at` never fire — keep the window on screen for tests.
12. Two Desktop folders exist; the active one is `C:\Users\tyler\Desktop` (the `Project Noble.lnk` shortcut lives there).

## 7. Tyler's working style

He says "do whatever you recommend", "crank it out", and leaves the session running; act autonomously, verify with real
captures/tests, keep him posted in plain language, and ask only when something genuinely needs a decision (for example:
which of two data routes to take, or permission to destroy save data). He will reject anything that looks like the wrong
model or a placeholder — check visuals against his screenshots before declaring something done.
