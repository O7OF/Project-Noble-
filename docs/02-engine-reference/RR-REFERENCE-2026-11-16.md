# Reach Renaissance (RR) reference notes for Project Noble

> **What this is.** A clean-room reference study of **Reach Renaissance 0.1.50** (released 2026-11-15, commit
> c9c8e3e7b) for the Noble multiplayer/scaling workstream: what RR has that Noble does not, how RR got past the
> 16-player ceiling (and what its shipped player limits actually are), and which of its architectural choices are
> useful reference points. **No RR code, assets, strings, profile names or ports are copied into Noble** — this is
> behaviour/structure documentation for engineering comparison only. Raw evidence (string dumps, recon reports,
> decompiled C of RR binaries) stays outside the repo in
> `C:\Users\tyler\Desktop\ReachRenaissance-Decompiled\` (see section 8).

Confidence tags: **proven** = read directly from the shipped RR files or our logs; **observed** = seen in shipped
files but not executed by us; **inferred** = deduced from binary strings/structure; **open** = unresolved.

## 1. What the shipped package is

| Piece | Evidence | Notes |
|---|---|---|
| `Reach Renaissance.exe` (2.6 MB) | launcher | starts, updates and re-launches the other processes |
| `bin/host-probe.exe` (22 MB) | main client/injection host | Rust; 48,045 COFF symbols kept (mangled Rust v0 names), no PDB; carries the engine patches, capacity profiles, lookup/armor/forge systems |
| `bin/menu-overlay.exe` (15 MB) | WebView2 menu host | Rust; serves the `themes/` HTML/JS menus, talks to the client via a local bridge |
| `bin/rr-master.exe` (12 MB), `bin/rr-browser.exe` (12 MB), `bin/rr-voice.exe` (11 MB) | service processes | master/directory, server browser + server admin, voice |
| `data/*.map` | 3 packs | Halo tag packs: armor (`rr_elite_pack`, 99 MB), forge (`rr_forge_pack`, 356 MB), menus (`mainmenu_all`, 285 MB) |
| `data/armor/*.rrreloc`, `.rrneeds`, `.rrpmcg`, `.rremotes` | per-pack metadata | relocation/patch records, tag dependencies, player-model customization, animation remotes |
| `themes/classic/*` | 60+ JS files | the whole front end: lobby, server browser, armory, forge UI, chat, scoreboard, career, emotes, theater |
| `bin/WebView2Loader.dll`, `bin/vgstream/*` | third-party | WebView2 runtime, vgmstream audio |

**Stack in one line (inferred):** a Rust client that injects/patches the MCC Reach engine (`haloreach.dll`) and adds
its own session/networking, menu and content layers; WebView2 + local JS is the entire UI; custom Halo tag packs carry
the new armour/forge/menu content. This is close to Noble's architecture (engine patch + own UI/content) but RR owns
the whole pipeline: menus, master/browser/voice services, updates and packs.

## 2. Player capacity: what RR actually ships (and the 88 question)

### 2.1 Shipped claims (documented in RR itself)

| Version | Date | Claim |
|---|---|---|
| 0.1.20 | 2026-10-22 | up to **64 players** in Multiplayer; 16 in Firefight (32 as a host option); scoreboard fits everyone |
| 0.1.34 | 2026-11-06 | **128 players** experimental in Custom Games: `Max Players` up to 128; 128-player lobbies; *two players can share each of up to 64 machines*; team/party menus handle 128; *hosting above 64 uses the 128-player engine profile; the stock 64-player path is unchanged* |
| 0.1.39-0.1.42 | 2026-11-08..11 | big-lobby fixes (35th player crash, guests dropping at game start, results loss); dedicated-server option `big_lobby=81` allows up to 80 players on a server; if the host leaves the game ends (no host migration) |
| 0.1.43+ | 2026-11-13..15 | parties hold up to **81 players**; lobby `Max Players` up to 81; M81 is described as *81 machines, one player per copy of the game* |

Shipped UI limits (proven, from `themes/classic/index.html` comments and code): the Custom Games lobby's Max Players
row goes **1..81**; the lobby chooses a host profile by count: default, **M64** over 32, **M81** over 64 (not in
Forge); the party size cap is **81**; Forge sessions cap at 32; Firefight caps at 16 (32 as a host option). The
server browser shows a host fact tag for the 64-player profile. The shipped changelog's most recent capacity statement
is 128 experimental (M128), with the stock 64 path untouched.

### 2.2 The "88" claim

**No evidence of 88 exists in the shipped RR 0.1.50 files, changelog, README, menu code or the client binary strings
we scanned.** The documented maxima are **81** (M81: 81 machines, one player per machine) and **128** (M128: 128
players, two per machine, up to 64 machines). External coverage from RR's launch week only mentions 32 (it was the
0.1.x launch ceiling at the time). Most likely the "88" figure is a misremembered 81/128 (or 81 players plus a
different spectator/admin count) from a video or a later build; **open** — if a specific RR page/video claims 88,
send it and it can be checked against the build.

### 2.3 How RR does capacity >32 (inferred from host-probe strings, high confidence)

RR does **not** keep the stock 16-player session layout and grow arrays in place. It runs a **capacity-profile
system** with a dual machine/player model:

- **Profiles:** `default`, **M64** (multiplayer, machines 1..64), **M81** (multiplayer, 81 machines / one player each),
  **M128** (multiplayer, 128 players on up to 64 machines, two players per machine), **FF32** (Firefight: 32 players)
  and an authoring profile. The lobby picks the profile from Max Players and passes it to the client; the WebView UI
  then holds the game to that count.
- **Machine vs player tables:** RR tracks *machines* (connections, authority host included) separately from *players*
  (up to two per machine). Strings describe machine counts (`--max-machines`, 1..64 normally, 81 with M81) and player
  storage separately (`--players-table 64|128`, `--session-players`, `--game-players`); the M81 note says *81 machines
  need the moved player block and 128 player storage slots*, and *64 machines need the moved player block and 64
  player slots*.
- **Widened bitmask sets:** its profile config carries `machine_index_bits` and `machine_set_bytes`; the M64 membership
  builder is described as *host membership send over 64 machines (u64 full/delta masks)*. In other words RR replaced
  the stock 32-bit peer/machine mask with a **64-bit (and larger) mask set** and sends full/delta membership updates.
  This is exactly the blocker that stops Noble above 32 today (the engine's peer/player valid masks are 32-bit DWORDs).
- **Own transport / membership layer:** a module named `m64_transport` installs the big-lobby transport; RR has
  `memberx`/membership resync paths, a *full membership* message (type 0x16 mentioned), deferred membership for M81
  machines, checksum validation, and explicit *resync not installed / out of step* handling. Big-lobby networking is
  therefore RR's own layer on top of the engine, not stock MCC networking.
- **Engine patch blocks:** flags such as `RR_PB`, `RR_PB64` and per-machine group patches (`P64/M64 groups`) gate
  groups of engine patches; several strings name the patched engine work (*census fixes 32*, a capacity detour, a
  5-bit writer, a round-robin set). This is the same technique as Noble's slot caves, but organised as selectable
  patch profiles instead of one global slot patch.
- **Known limitations RR documents:** M81 host migration is unsupported (the game ends and guests return to menus);
  M128 is experimental; `--m128-capacity` is multiplayer-only; `--m64-profile` is multiplayer-only (Firefight uses
  FF32); hosting above 64 uses the M128 profile while the stock 64 path stays unchanged.

**Confirmed by the full demangled function index (2026-11-16).** The completed Ghidra analysis of `host-probe.exe`
(24,608 functions; Ghidra's Rust demangler applied) exposes the capacity layer as real modules, not just strings:

| Module / function | Evidence from the demangled index | Role (inferred) |
|---|---|---|
| `m64_transport::m81::masks::replacement_spans` | function 0x14000b9b0 and iterator closures | computes the byte spans to replace when widening/replacing M81 membership masks |
| `m64_transport::m81::scheduler::{MASKS, SchedulerMasks, phase_action, initial_query, prioritize_native, clear_loss}` | statics at 0x14014a860 / 0x140150090 plus update closures | schedules the M81 transport; prioritises native traffic and tracks mask loss |
| `m64_member::send81::{RECIPIENTS, Recipients}` | statics 0x14014a770 / 0x140150000 | membership sender for up to 81 machines, with a recipient set |
| `memberx::{player_fields81, WireField, machine_props_valid, appearance_valid, drop_invalid_entries}` | many functions, e.g. 0x140081cd0 | membership wire-field model for 81 players (validation/rejection of machine/player fields) |
| `m64_update::{ready_update, High}` | 0x140226020 and closures | applies the big-lobby membership update (full/delta) |
| `m64_readers::forget` | 0x140005de0 | drops stale per-machine reader state |
| `install_stream128` | 0x1400eeb50, 4,841 bytes | installs the 128-player stream/profile (M128) |
| `reset_ppm64` | 0x1401eb740 | resets the P64/M64 player-state block |
| `capacity::check_ff32_profile` | functions around 0x140388a30 | validates the FF32 (Firefight 32) profile |

Together this is a **custom transport + membership stack for big lobbies**: a scheduler and mask-replacement
layer (`m64_transport`), an 81-machine membership sender (`m64_member::send81`), an 81-player wire-field validator
(`memberx`), a stream installer for 128 players (`install_stream128`), and profile checkers. That is the concrete
architecture behind the shipped 64/81/128 claims; it is considerably more than widening arrays in place.

**What the M128 installer concretely does.** The decompiled `host_probe::p64_actions::install_stream128`
(0x1400eeb50, 4,841 bytes) builds a **patch table**: for ~21 engine sites at 28 RVAs in the `0x421340..0x421bd3`
range it records the target RVA together with the expected and replacement byte strings (through `p64::Batch`), i.e.
exactly the all-or-nothing site-patch shape Noble uses, expanded into a named stream/profile. The RVAs seen in the
decompiled function: `0x421340 0x421350 0x4213db 0x42143c 0x4215b9 0x4215bc 0x4216c0 0x4216d7 0x4216e1 0x421713
0x42171e 0x421748 0x42174e 0x421765 0x42176f 0x42177f 0x4217a8 0x4217ae 0x4217c8 0x421836 0x421842 0x42184a
0x421851 0x421917 0x421927 0x421b45 0x421bc7 0x421bd3` (patch bytes themselves are RR data and are not copied).

Also confirmed as named modules by the analysis: `m64_join::{BitBranch, bit_branch_code, code_sites, may_leave_out,
platform_diag_on}` (a bit-branch/code-site patch generator), `capacity::{apply, machine_storage_plan, scoreboard_rows,
check_ff32_profile, unique_value}`, `lobby_profile::{check, default_switches, plan, prepare_plan}`, `m128::PlayerSet`,
`m64_readers::install`, `m64_transport::m81::NativeBudget`, `p64::{Batch}`, and `scoreboard::{left_rows,
membership_users, peer_records, record_peer, session_player, session_users, tick, validate_target}`.

Function names/addresses are in `decompiled/host-probe-analysis/function-index.csv` (24,608 rows, demangled); the
matching decompiled C for these modules is being exported into the same folder.

**Reference value for Noble:** RR proves the >32 path needs (1) separate machine and player capacity, (2) 64/128-bit
membership mask sets and full/delta membership updates, (3) moved player blocks / enlarged player storage, (4) an
explicit profile-selection surface and a UI that clamps to it, and (5) a transport/membership layer that accepts
large lobby counts. Noble's mask-widening model (high-word mask storage, 1/2/4 words, mask shadow lifecycle) is a
subset of the same design; RR's `machine_index_bits`/`machine_set_bytes` config is a useful naming/shape reference,
and its M128 two-players-per-machine approach (players > machines) is worth considering to keep peer/machine masks
smaller than the player count.

## 2.4 How RR's multiplayer works end to end (inferred)

This is the best reconstruction from the shipped process layout, strings, changelog and the decompiled module names;
treat the arrows as inferred and the named modules as observed. RR does not use MCC matchmaking: it starts the
owned MCC Reach engine, patches it, and runs the lobby/session/directory/voice stack itself.

```
launcher (Reach Renaissance.exe)
  -> finds MCC+Halo: Reach, checks the supported engine build (log: 'build check passed')
  -> starts menu-overlay (WebView2 UI) + host-probe (game client / engine patcher)
host-probe
  -> launches Reach with RR's own launch block (data/launch_block_1.bin, lb_m_c_1.bin)
  -> installs engine patches/hooks, including the capacity profile patches
     (p64_actions::install_stream128 builds a patch table over 28 engine RVAs;
      p64::Batch, m64_join::{code_sites,bit_branch_code}, capacity::machine_storage_plan)
  -> runs its own session membership/transport layer
     (m64_transport (incl. m81 scheduler/masks/NativeBudget), m64_member::send81,
      memberx::player_fields81/WireField/validators, m64_update, m64_readers, resync paths)
  -> engine runs the actual game (simulation, rendering, game rules)
WebView2 menus <-> host-probe
  -> lobby, armory, server browser, scoreboard, chat, career, forge UIs talk to the client
services
  -> rr-master: public directory/master
  -> rr-browser: server browser / server admin tools
  -> rr-voice: voice service
```

**Lobby and hosting.** A host picks a mode in the menus: `Offline`, `Host Online` (home network) or
`Host on Server Browser` (public). Max Players chooses the engine profile (default, M64 over 32, M81 over 64 in
multiplayer; FF32 in Firefight; Forge stays 32). The host-probe validates the choice (`lobby_profile::plan/check`,
`capacity::check_ff32_profile`) and holds the lobby to that count; parties can be up to 81 (M81 machines).

**Discovery and joining.** Public hosts are listed by the master/directory service and shown in the browser with
game type/map/players/ping/region/version facts (version tags advertise the host profile). A join connects to the
host either on the home network/directly or via RR's relay path; the client then performs RR's own admission
(`m64_join`/membership modules) instead of stock MCC join logic. The host's membership sender builds full/delta
updates for the machine set (63/81-machine recipient sets, checksum + resync handling) and the engine session is
expanded to the selected profile.

**In game.** The engine simulates and renders Reach; RR supplies the roster/scoreboard data (own scoreboard modules
for the larger player counts), in-game chat over its own transport (`ingame_chat::transport`), and voice through
`rr-voice`. Big-lobby membership is kept in sync by the transport/membership layer; if the host leaves an M81 lobby
the game ends and guests return to menus (host migration is not supported, as RR documents).

**What is proven vs inferred.** Observed directly: the process layout, launch-block files, engine build check, the
capacity/profile flags and strings, the M128 patch-table installer, and the named transport/membership modules.
Inferred: the exact split between RR's transport and the engine's stock network layer (RR clearly owns session
membership/roster/chat and has a relay, but we have not traced the game-traffic path end to end). The decompile
workspace can settle that: read `m64_transport`, `m64_member::send81`, `memberx`, `p64_actions`, `m64_join` and the
network modules (`net::*`, `ingame_chat::transport`) next.

## 3. Feature comparison: where RR differs from Noble (corrected 2026-11-16)

Confidence: RR side observed/proven from the shipped files, changelog and binary strings; the **Noble side is
owner-confirmed as of 2026-11-16** where noted. This is not a "Noble is missing X" list for everything - Noble has
accounts, credits, challenges, commendations, ranks, voice chat, regular chat, moderation/reports and (likely) host
protection. Those should not be treated as gaps. Rows below only call out real differences or things still to verify.

### 3.1 Networking, servers and matchmaking

| Area | RR | Noble (confirmed/known) | Difference |
|---|---|---|---|
| Server browser | own master/directory service (`rr-master`) and browser process; listings carry game type/map/players/ping/region/version facts and host-profile facts | Noble has a server browser path (MCC-network based) | RR runs its own public directory and advertises host profiles; Noble browser is the MCC path |
| Dedicated servers | hosts dedicated servers from the menus; `server.ini` options incl. `big_lobby=81` (80 players + host), playlists, admin dashboard, lasting bans, restart/next-game, UPnP | not part of the current Noble client (verify if scheduled) | dedicated-server tooling/capacity is RR-only so far |
| Relay / NAT traversal | own relay path (players joined through the relay get host scores/colours/kicks), master/browser/voice services | Noble relies on MCC networking | RR owns the relay layer |
| Parties and social | parties up to 81, leader transfer, friends board, close-friends notices, invites, whisper, View Card, clans | Noble has social/lobby UI and parties within MCC limits (exact feature set to verify) | RR's party scale (81) and extra social boards are larger |
| Moderation and reports | kick/ban by number, moderator popups, reports from scoreboard/chat, host protection / Lock Game | **Noble has moderation, reports and likely host protection (owner-confirmed)** | both have it; RR adds big-lobby/UI variants - not a gap |
| Match rotation / map vote | rotations (playlists) of maps + game types, server playlists with shuffle/player counts, map vote keeps everyone | Noble has lobby/map picker (rotations/playlists to verify) | playlist/rotation tooling is RR-only as far as we know |
| In-match chat and voice | chat in matches/rotations/servers; own voice service (`rr-voice`) with mute in scoreboard, speaker icons | **Noble has regular chat and voice chat (owner-confirmed)** | both; RR integrates voice into its own scoreboard/UI - verify Noble's current UX |
| Host migration | not supported in big lobbies: game ends, guests return to menus | Noble sessions end/close similarly today | both effectively lack it; RR documents the limitation |

### 3.2 Armour, appearance and the colour-changing lights

#### 3.2.1 What the decompiled client shows (glow module, 2026-11-16)

The targeted Ghidra decompile of `host-probe.exe` (clean-room: kept outside the repo, in the Desktop decompile
workspace) confirms the mechanism. Addresses are host-probe RVAs with image base `0x140000000`:

| Function | Address | What it does (from the decompiled C) |
|---|---|---|
| `glow::parse_hex` | 0x140432030 | trims, accepts an optional `+`, validates hex digits, and returns true only for a **6-hex-digit RGB** value |
| `glow::parse_level` | 0x14042ebf0 | parses the brightness level as an integer, bounded to the level table's range |
| `glow::intensity_of` | 0x14042eda0 | brightness multiplier: indexes an 11-entry float table which is exactly **1, 2, 3, 5, 8, 12, 16, 24, 32, 50, 64** (the x1..x64 levels); while in the Armory it uses a menu-specific level/glow instead |
| `glow::glow_of` | 0x140431d00 | per-draw colour: reads the player's look colours, honours forced colours/match rules, converts the float RGB to bytes and packs **0xRRGGBB**; tracks the last painted objects (a small ring, so one object isn't repainted every draw) and counts calls |
| `glow::apply` | 0x14042ef60 | installs/uses the draw hook; calls `glow_of`/`intensity_of` on the draw path (55 KB function) |
| `glow::frame` | 0x140431580 | per-frame upkeep: resets the call/paint counters, reports painted draws, handles the Armory/menu state |
| `srgb::of` | 0x14052ee40 | standard sRGB conversion of the chosen colour (0.04045 branch, /12.92, ((c+0.055)/1.055)^2.4) before it is blended |
| `glow::enabled` / `in_menu` | 0x140431cc0 / once-lock inits | feature gate and menu/Armory mode |

So the shipped "colour-changing lights / armour lights" is a **custom exact-RGB armour-light glow with a
brightness multiplier**, applied by hooking the armour/light draw path and replacing its colour - not an
animated hue cycle. The colour is whatever RGB the player picks (per look; per piece for the Elite/Spartan pieces),
and the brightness is the x1..x64 level. While the player is in the Armory the same painter runs with a menu glow
so the model preview shows the selected light colour immediately.

The decompiled files for exactly these functions are in
`C:\Users\tyler\Desktop\ReachRenaissance-Decompiled\decompiled\host-probe-fast\`
(file names start with the address above, e.g. `140431d00__...4glow7glow_of.c`).




| Area | RR | Noble (confirmed/known) | Difference |
|---|---|---|---|
| **Armor Light Glow colour** | exact RGB sent per look, applied by a draw hook to armour-light draws (see 3.2.1) | Noble has armour/visor colour customisation; **armour-light glow painter not confirmed in Noble** | likely RR-only feature - verify current Noble armour-light support |
| **Armor Light Brightness x1..x64** | level index 0..10, table 1,2,3,5,8,12,16,24,32,50,64 | not known in Noble | RR-only as far as we know |
| Per-piece armour colours | exact RGB per piece (helmet, shoulders, chest, forearms, wrist, utility, knees), Elite primary/secondary per piece | Noble ships armour/visor customisation and mod packs (exact per-piece/exact-RGB support to verify) | RR exposes exact RGB per piece; verify Noble's current level |
| Colour wheel / exact colours | WebView colour wheel, exact RGB swatches beyond the stock palette | Noble uses palette-based customisation (known) | custom exact colour picking is an RR UX difference |
| Elite armour pack | `rr_elite_pack.map` + `.rrpmcg` metadata: extra pieces, presets, one glow colour for Elite lights/visor | Noble has mod packs/VFS; Elite-pack equivalent not confirmed | content/UX difference to verify |
| Armour effects, poses, nameplates, emblems | armour effects, 360-style emblem editor + special animated emblems, poses, player cards, nameplates | Noble ships armour/visors and emblems/customisation (owner-confirmed areas); verify effects/poses/nameplates | overlapping; compare item by item rather than assuming gaps |
| Credits / challenges / commendations / ranks / mailbox | accounts, credits, challenges/commendations, ranks, mailbox with gifts and Armor Voucher events | **Noble has accounts, credits, challenges, commendations, ranks (owner-confirmed)**; mailbox/voucher events to verify | progression is shared; mailbox/voucher-event shape is the only candidate gap |

### 3.3 Forge, Firefight, Campaign, Theater

| Area | RR | Noble (confirmed/known) | Difference |
|---|---|---|---|
| Expanded Forge | piece limits past 1,024/1,302, $600k budget, File Share of big maps, vehicles past the limit, Forge on the 10 Firefight maps, Sandbox Evolved Forge World (auto-built ~1.1 GB), map converter | Noble has Forge and Forge AI work (limits/authoring work in progress) | RR raises the content/authoring limits further; Noble has its own Forge path |
| Forge Firefight (AI authoring) | Covenant spawns/firing positions/dropship waypoints; AI walks/cover/crosses floors; AI limit 96/128/192 experimental; Save and Test; 4 Phantoms + 4 Pelicans; wave traits | Noble has Forge AI work (toggle/limits investigations), no shipping equivalent confirmed | authoring tooling is RR-only as far as we know |
| Firefight online | rotations, 16 players (32 host option), skulls/scoring, voice packs, Elite players, lives/respawns, join-in-progress | Noble runs Reach modes via MCC; large Firefight/rotation support to verify | RR's big-Firefight + rotations are ahead |
| Campaign online | 4-player co-op online, checkpoints, scoring, skulls screen, campaign mods' missions | **Noble has 4-player campaign co-op; more than 4 parked (known)** | both 4-player; RR adds mod-mission picker/checkpoints UI |
| Theater / films | films saved, browser/playback up to 64 players, run reports | Noble has theater paths (verify scope) | RR's big-lobby theater is a reference point |

### 3.4 UI, platform and content operations

| Area | RR | Noble (confirmed/known) | Difference |
|---|---|---|---|
| UI stack | WebView2 HTML/JS (`themes/classic`) with animations, intro video, controller/OSK, themes | Noble UI is in-engine | architecture difference, not a feature gap per se; RR's WebView approach is a reference for menu/content velocity |
| In-menu updates/changelog/guides/MOTD/bug reports | launcher updater, WebView changelog/guides, bug-report export bundles | **Noble has updater/changelog paths (known)**; guides/MOTD/bug-report bundles to verify | partially overlapping; RR's bug-report bundle is a reference |
| Linux/Steam Deck support | Proton wrapper path, Steam shortcut docs, WebView2-under-Proton notes, Vulkan requirement | Noble is Windows/MCC-focused (known) | RR-only platform support |
| Map/game-type sharing | File Share packs rotations + Forge maps; import playlists; mods download; Maps folder management | Noble has mod VFS/packs (known); File-Share-style exchange to verify | RR's sharing flow is a reference |
| Content as tag packs | `.map` packs + RR metadata (`.rrreloc`, `.rrneeds`, `.rrpmcg`, `.rremotes`), cached/prepared packs | Noble has mod packs/VFS (known) | comparable; RR's per-pack metadata is a reference design |

### 3.5 Noble capabilities explicitly not to treat as gaps

Owner-confirmed 2026-11-16: **accounts, credits, challenges, commendations, ranks, voice chat, regular chat,
moderation and reports**, and (likely) **host protection**. Also known from the Noble workstream: armour/visor
customisation, mod packs/VFS, server-browser path, lobby/picker, campaign co-op (4), updater/changelog. Do not copy RR
features into Noble to "catch up" on these; they exist here already.


## 4. Other useful reference points for Noble

1. **Capacity is a first-class "profile", not just a bigger array.** RR names profiles (default/M64/M81/M128/FF32),
   gates engine patch groups behind flags (`RR_PB`, `RR_PB64`, P64/M64 groups), and makes the lobby/host choose the
   profile from Max Players. Noble currently has `-slot-players 32` + shadow-session layout; a profile abstraction would
   let 32 stay default and 64/128 be selected/tested per host without changing the stock path.
2. **Machine and player counts are separate.** Keeping players > machines (M128: two per machine) reduces the width
   needed for machine masks. Noble's mask-widening model assumes peers == players for capacity; RR shows a path where
   the machine set stays at 64 bits while player storage goes to 128. This could make a 64-player dev target cheaper
   (32 machines x 2 players) than 64 machines.
3. **Mask/set widths are explicit config.** `machine_index_bits` / `machine_set_bytes` in RR's profile map directly
   onto our planned high-word mask plan (word count, bits per machine/player, full/delta mask messages). RR also
   rebroadcasts full/delta membership for big lobbies and has resync handling.
4. **UI honesty and party capacity.** RR caps Max Players to the profile's machine limit and the party to 81, and
   shows profile facts in the browser. Noble already clamps advertised seats; the profile/party-size wiring is the
   next analogous step.
5. **Engine patch organization.** RR groups patches per profile and keeps the stock 64 path unchanged when M128 is
   selected. Noble's all-or-nothing byte-checked install is safer for a single mode; a profile flag set could be
   layered over it for >32.
6. **Tag pack metadata.** RR ships per-pack `.rrneeds` (tag dependencies), `.rrreloc` (relocations/patch records),
   `.rrpmcg` (model customisation) and `.rremotes` (animation remotes). Useful reference for Noble's pack format if we
   want portable, dependency-checked packs.
7. **Rust symbols retained.** host-probe keeps its COFF symbol table (48k symbols), so even without PDB it is
   practical to locate functions by name in Ghidra (module paths and function names are readable). This is why the
   RR reference is easier to study than a fully stripped engine.

**Clean-room reminder:** no RR code, assets, strings, profile names or ports go into Noble. This document records
behaviour and structure only; the same limits that applied to the earlier RR study apply here.

## 5. Decompilation workspace (set up 2026-11-16)

Location (outside the repo): `C:\Users\tyler\Desktop\ReachRenaissance-Decompiled\`

| Path | Contents |
|---|---|
| `README.md` | how to open/rerun the Ghidra project and the decompile commands |
| `scripts/` | Ghidra export scripts (`ExportDecompiled.java`, `DecompileNamed.java`), `recon_rr.py`, `decode_symbols.py` |
| `ghidra-project/` | Ghidra project(s); `ReachRenaissance` holds the launcher analysis, per-binary projects are created as runs continue |
| `decompiled/host-probe-fast/` | targeted decompile of glow/look/armour/colour functions from host-probe (340+ `.c` files) |
| `decompiled/reach-renaissance/` | launcher analysis log (4378 functions analysed; filtered export found no glow names) |
| `reports/` | PE/symbol recon, Rust source module list, capacity strings, changelog feature extract |
| `GLOW-NOTES.md` | the armour-light glow feature notes (section 3.2 detail + where the code is) |

Status 2026-11-16 (final for this workspace): launcher analysed (4,378 functions); **host-probe full analysis
complete** - 24,608 functions, 1,530 s auto-analysis, Ghidra's Rust demangler applied (a non-fatal analyzer/decompiler
ArrayIndexOutOfBoundsException was logged; the program saved and is usable). Artifacts: `decompiled/host-probe-fast/`
(364 C files: whole glow/srgb module and armour/look systems, mangled filenames), `decompiled/host-probe-analysis/`
(**198 exported functions / 203 C files, ~948 KB, demangled names** - capacity/m63/m64/m81/m128/memberx/transport/
m64_join/scoreboard/lobby-profile modules and `install_stream128`) and `function-index.csv` (24,608 demangled
names/addresses/sizes). The other binaries can be run the same way with `scripts/decompile-all.ps1`; expect ~25 min of
analysis for host-probe and a few minutes each for the rest.

## 6. Sources

- Shipped RR 0.1.50 package on this PC: `README.txt`, `VERSION.txt`, `themes/classic/*` (changelog, lobby/browser/
  start-menu/armory UI, comments with design notes), `data/*`, `bin/*.exe` (strings + PE/COFF metadata).
- Our logs/evidence for Noble's side: `docs/02-engine-reference/TOPIC-scaling-64-128.md`, `MASK-SITE-INVENTORY.md`,
  `MASK-CLASSIFICATION-2026-11-16.md`, and the Noble test logs referenced there.
- External coverage of RR at launch (32-player era): noobfeed.com coverage of Halo: Reach Renaissance, 2026-10-23
  (https://www.noobfeed.com/news/halo-reach-renaissance-pc). No source found for an 88-player claim.

## 7. Open questions

- Where does the **88** figure come from? Not in RR 0.1.50; likely 81/128 confusion. Need a source link.
- M128 details (machine set width, player block layout, message framing) are only visible as strings today; the
  decompiled host-probe functions for capacity/m64_transport should settle them.
- RR's exact engine patch list per profile: could be recovered from the binary's patch tables/patterns; useful as a
  cross-check for Noble's mask/session work, but only conceptually (clean-room).
