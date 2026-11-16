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

**Reference value for Noble:** RR proves the >32 path needs (1) separate machine and player capacity, (2) 64/128-bit
membership mask sets and full/delta membership updates, (3) moved player blocks / enlarged player storage, (4) an
explicit profile-selection surface and a UI that clamps to it, and (5) a transport/membership layer that accepts
large lobby counts. Noble's mask-widening model (high-word mask storage, 1/2/4 words, mask shadow lifecycle) is a
subset of the same design; RR's `machine_index_bits`/`machine_set_bytes` config is a useful naming/shape reference,
and its M128 two-players-per-machine approach (players > machines) is worth considering to keep peer/machine masks
smaller than the player count.

## 3. Features RR has that Noble does not (and how RR does them)

Confidence: observed/proven from the shipped UI + changelog + binary strings unless marked inferred. "Noble today"
describes the state of our workstream as of 2026-11-16; confirm before acting on any single row.

### 3.1 Networking, servers and matchmaking

| Feature | How RR does it | Noble today |
|---|---|---|
| Public server browser with host options | master/directory server (`rr-master`), browser process, lobby rows carry game type/map/players/ping/region/version facts; version tags advertise host profiles (e.g. a 64-player host fact) | Noble has a server browser path for MCC networking; no public master/directory of its own |
| Dedicated servers | host runs a dedicated server from the menus; server.ini options incl. `big_lobby=81` (80 players + host), playlists, admin dashboard, lasting bans, restart/next-game, UPnP port request | none |
| Relay/NAT traversal | RR's own relay path (players "joined through the relay" get host scores/colours/kicks); master/browser/voice services | Noble relies on MCC networking; no relay service |
| Parties & social | parties up to 81, party leader transfer, friends board, close-friends online notices, invites board, whisper, Leave Party, View Card | Noble has lobby/social UI but no 81-party or friends/clan service |
| Moderation | kick/ban by player number, host protection / Lock Game, moderator popups to all menus, player reports with the chat's reason list | partial/no |
| Match rotation / map vote | rotations (playlists) of maps + game types, server playlists with shuffle and player counts, map vote keeps everyone | Noble has lobby/map picker only |
| In-match chat + voice | chat in matches/rotations/dedicated servers; voice service (`rr-voice`) with mute in scoreboard, speaker icons | Noble has chat/voice chat work but not an RR-style voice service |
| Host migration | not supported in big lobbies: game ends, guests return to menus (documented) | Noble sessions end/close similarly today |

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



| Feature | How RR does it | Noble today |
|---|---|---|
| **Armor Light Glow colour** (the light/armour-light colour feature) | UI sends an exact RGB look value plus a brightness index; the client's `glow` module parses the colour/brightness level, paints the armour light draws (draw hook, per-frame `frame`/`frames`, paint list) with sRGB conversion; armory uses a menu-specific glow | Noble has visor/armour colour customisation but not an armour-light glow painter |
| **Armor Light Brightness x1..x64** | brightness levels 1,2,3,5,8,12,16,24,32,50,64 (indices) sent as a look level; default applies when the player hasn't picked | not present |
| Per-piece armour colours (Spartan and Elite) | exact RGB per piece (helmet, shoulders, chest, forearms, wrist, utility, knees) sent as look entries; Elite primary/secondary per piece; undermap in the client | Noble has armour/visor colour sets but not per-piece exact RGB |
| Colour wheel + exact colours | WebView colour wheel, exact RGB swatches, custom colours beyond the stock palette | Noble has palette-based customisation |
| Elite armour pack | custom tag pack `rr_elite_pack.map` + `rrpmcg` player-model customisation metadata; extra pieces, presets, colours; one glow colour for all Elite lights/visor | Noble has mod packs; no equivalent Elite pack integration |
| Armor effects, poses, nameplates, emblems | armour effects (e.g. armour_fx), 360-style emblem editor + special animated emblems, poses, player cards, nameplates | partial: Noble has emblems/armour customizer pieces |
| Armor Voucher / mailbox / credits | accounts, credits, challenges/commendations, mailbox with gifts and voucher events, career ranks | Noble has no progression service |

### 3.3 Forge, Firefight, Campaign, Theater

| Feature | How RR does it | Noble today |
|---|---|---|
| Expanded Forge | Forge piece limit past 1,024/1,302, $600k budget, big-map File Share, vehicles past the limit, Forge on the 10 Firefight maps, Sandbox Evolved Forge World (auto-built ~1.1 GB map), map converter | Noble has Forge work (AI/limits investigations) but not this content pipeline |
| Forge Firefight (AI authoring) | place Covenant spawns/firing positions/dropship waypoints; AI walk/cover/cross floors; AI limit 96/128/192 experimental; Save and Test; dropships (4 Phantoms + 4 Pelicans); wave traits | Noble has Forge AI work (asked about AI toggle); no shipping equivalent |
| Firefight online | rotations, 16 players (32 host option), skulls/scoring, voice packs (Firefight voices), Elite players, lives/respawns, Join in progress | Noble has Firefight? (campaign co-op 4) — no shipping big-Firefight online |
| Campaign online | 4-player co-op online, mission checkpoints, scoring, skulls screen, campaign mods' missions | Noble has campaign co-op (4, more parked) |
| Theater/films | games saved as films, film browser/playback up to 64 players, run reports | Noble has theater? not in our workstream |

### 3.4 UI, platform and content operations

| Feature | How RR does it | Noble today |
|---|---|---|
| Entire UI as WebView2 HTML/JS | `themes/classic` with themed menu screens, animations, intro video, loading screens, controller/OSK/accessibility, keyboard/mouse + controller navigation | Noble UI is in-engine; RR's approach is a WebView layer |
| Theme system | multiple themes (`themes/classic`, other folders), per-theme assets/music/movies, style tokens | Noble has UI theming? (not in this workstream) |
| In-menu updates, changelog, guides, MOTD, bug reports | launcher checks updates; WebView changelog/guides; bug-report export bundles latest run logs | Noble has updater/changelog paths |
| Linux/Steam Deck support | Proton wrapper path, Steam shortcut docs, WebView2 under Proton notes, Vulkan requirement | Noble is Windows/MCC-focused |
| Map/game-type sharing | File Share packs rotations and Forge maps; import playlists; mods download; Maps folder management | Noble has mod VFS and packs but not this sharing flow |
| Content as tag packs | `.map` packs plus RR metadata (`.rrreloc` relocation/patch records, `.rrneeds` tag dependencies, `.rrpmcg` model customisation, `.rremotes` animation remotes), cached/prepared packs | Noble has mod packs/VFS; RR's per-pack metadata is a reference design |

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

Status 2026-11-16: launcher analysis complete (4378 functions); **host-probe targeted decompile complete** -
400 functions exported (46,177 symbols available, 401 name matches), including the whole `host_probe::glow` module
(`enabled`/`in_menu`/`arg`/`apply`/`frame`/`frames`/`glow_of`/`intensity_of`/`parse_hex`/`parse_level`), `host_probe::srgb`,
the armour look/armour-pack systems, and the lobby/armour UI helpers (`index.csv` in the folder). The host-probe
**full** analysis + full export (every function of the 22 MB binary) is still running in the background and can be
resumed/awaited; the launcher, menu-overlay, rr-master, rr-browser and rr-voice binaries can be run the same way with
`scripts/decompile-all.ps1`.

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
