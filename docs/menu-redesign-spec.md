# Project Noble: menu redesign spec ("Reclaimer layout, Reach soul")

Written 2026-10-28 for the agent / developer (O7OF) who builds it. Source material: Tyler's brief (2026-10-28 09:56),
43 reference screenshots of Project Reclaimer (a Halo 3 PC project) that Tyler sent, and `docs/HANDOFF.md`
(09:55 ET). Read the handoff first for build commands, engine facts and repo rules; this document only covers the
menus, the customization system and the career screens.

**Files that come with this document:** Tyler is uploading **Halo: Reach rank images** together with this spec. Use
them as **placeholders** for now (rank icons in the menus, lobby, scoreboard, post-game and Service Record) so there is
something real to look at; they are templates until the final art and how it is distributed are decided. **Do not
replace them with AI-generated art.** Tyler also has the 43 Reclaimer reference screenshots this spec was written
from (described in Appendix A): **if any screen here is unclear, ask Tyler for the matching screenshots.**

**Goal in one line:** Noble's menus should work like Reclaimer's (clear three-column layouts, live previews, every
option explained, fast to use with mouse, keyboard or controller) but look and sound like Halo: Reach (Reach's
colours, type, icons, art, sounds and music). Reach is a 2010 game; keep its identity, modernise the usability.

---

## 0. Ground rules for this work

- **Git:** the agent may add and commit files and push them. Repository settings are O7OF's alone: do not change
  them.
- **Reach artwork:** Tyler wants Reach's own art (rank icons, commendation icons and medals, difficulty icons,
  game-type icons, skulls, map pictures, emblems). Tyler owns Halo: Reach. The rank images Tyler supplies with this
  document are placeholders (see above). Section 7 describes where art comes from and how it is laid out; where it
  is stored in the repo is O7OF's decision (the handoff's art rule is his to change).
- Everything else in handoff §8 stays: MCC files read-only, no RR/Omaha/`haloreach.dll` material, no logs or dumps,
  Steam ownership check stays, small PRs, tell O7OF before large edits to `ReachHost.cpp` / `BlamEngine.cpp`, no
  AI/co-author lines in commits or docs, re-check files after writing (stale copies have reappeared on Tyler's PC).
- Tyler is not a C++ developer: when he has to test, give him exact commands and say what to look for.

---

## 1. What exists today (read before building)

| Area | Files | State |
|---|---|---|
| Front-end menu | `src/ui/FrontEndMenu.cpp/.hpp` | O7OF's in-engine menu (`mainmenu` mode) drawn by `RenderOverlay` from host slot 1; `HoldFrame()` keeps drawing after the engine stops (~60 s). Screens: Main, Campaign, Skulls, Firefight, Custom Game, Forge, Servers, Settings, Armory, Mods, Service Record. Vanilla / Modded tabs with collapsible mods, click-to-pick lists, scrolling, quit confirmation, clickable hints. `-menu-screen` is ignored by the in-engine menu. |
| Shared look | `src/ui/NobleStyle.cpp/.hpp`, `UiArt.cpp/.hpp` | Fonts, accent colour, MCC texture-pack reader (map previews, skulls, controller), backgrounds. |
| Input prompts | `src/ui/InputPrompts.cpp/.hpp` | Last-used device tracking, drawn A/B/LB/RB/D-pad glyphs or key boxes. |
| Sounds / music | `src/audio/ReachAudio.cpp/.hpp`, `src/ui/MenuSounds.*` | Reach UI sounds and menu music from MCC's FMOD banks via vgmstream; ~360 tracks; two-pane music picker in Settings > Audio. Post-game sounds (`GameCompletion`, `Totals`, `RankUp`, `SlotMachine`) are decoded but not used yet. |
| Settings | `src/ui/SettingsPanel.cpp/.hpp`, `src/common/Settings.*` | Tabs: Controls (mouse, gamepad incl. stick speed +0x1B5/+0x1B6, dead zones, vibration), Key bindings (incl. wheel up/down), Video (resolution, borderless, FOV 70-120, FPS limit, V-Sync), Audio, Gameplay, Network. |
| Armory | `ArmorCustomizer.*`, `ReachArmory`, `ObjModel*.cpp` | 3D OBJ preview (local exports only), colours/visor tint; `ApplyArmoryAppearance` writes helmet/shoulders/chest/wrist/utility/knees/visor/effects/voice/colours/Elite set into the profile (+0x40..+0xA0, MCC global item ids). Third-person check still pending. |
| Career | `src/career/Career.*`, `src/ui/CareerPanel.*` | 50 ranks, cR pay, Service Record (rank, totals, campaign, last 20 games), 7 daily / 5 weekly challenges from a small pool, 6 commendation tracks, post-game scoreboard + cR lines. Reference data in `docs/career/` (all 45 Reach commendations, 2,103 real Reach challenges, ranks and pay, Armory list). |
| Servers / mods | `ServerBrowser.*`, `MasterServerClient.*`, `WorkshopManager.*`, `ModCatalog.*`, `vfs/VFS.*` | Server list + favourites + direct connect (joining needs engine networking, handoff §6 item 4); Workshop/local Reach mods; `VFS::SetActiveMod`. |

---

## 2. Design direction

### 2.1 What to take from Reclaimer (structure)
- **Screen shell:** big thin title top-left, `BACK` (and other top-level actions) as text buttons top-right, a dark
  scene behind everything, content in up to three columns: *navigation* | *content* | *details*.
- **Focus bar:** the focused row gets a solid 3 px left edge and a gradient fill; hover is a faint fill.
- **Section headers:** small letter-spaced caps followed by a thin rule.
- **Every option explains itself:** the details panel shows the option's name, when it takes effect ("Takes effect
  when you apply changes" / "Saved as soon as you change it" / "Next time you start the game"), a live preview when
  one makes sense, a plain-language description, and a fact row (default value, size on disk, ...).
- **Summaries everywhere:** category rows show their current values ("Borderless · No limit"), tiles show variant
  counts, the lobby shows "Map · Game type".
- **Unsaved changes box** (count + which category, APPLY CHANGES / DISCARD) instead of an always-visible Apply.
- **Grids of pictures** for anything visual (maps, game types, armour pieces, emblems), lists for everything else.
- **Search** on every long list (maps, game types, settings, servers, music).

### 2.2 What makes it Reach (identity)
- **Colour:** Reach's menus are steel-blue/grey with white type and a cool cyan-blue highlight, with amber/gold for
  emphasis. Use Noble's existing accent `kAccent` (143, 216, 255) for focus and selection, white/grey text, and keep
  amber only for warnings, "unsaved" dots and post-game cR. Do **not** copy Reclaimer's orange as the main accent.
- **Type:** Reach's upper-case condensed headings (Noble's `TitleFont`/`HeroFont`) for titles and labels, a plain
  sans for descriptions.
- **Shapes:** Reach's angular cut corners and the diamond motif already used in `DrawTitle`; thin 1 px frames.
- **Backgrounds:** layered concept-art / map scenes with a slow parallax drift (Bungie's original menus did this;
  handoff §6 item 7). Darken behind text.
- **Sound:** Reach's own UI sounds for every move/accept/back/tab (already wired through `MenuSounds`), the menu
  music, and the exit-experience sounds (`Totals`, `RankUp`, `SlotMachine`, `GameCompletion`) on the post-game and
  career screens.
- **Motion:** short and quiet: 120-180 ms slides/fades between screens, list rows that settle in, numbers that count
  up (cR, XP), a short flash on rank-up/commendation tier-up. Respect a "reduce motion" setting.
- **Words:** Reach's terms: Armory, Service Record, Commendations, Challenges, cR, Firefight, Forge, Theater,
  Custom Games, Matchmaking (greyed until online play exists).

### 2.3 Input
Mouse, keyboard and controller must all reach every control. Prompts follow the last-used device
(`InputPrompts`). LB/RB (Q/E) switch tabs/categories, A/Enter accepts, B/Esc goes back, X/Y for secondary actions
(e.g. Y = randomise, X = reset), D-pad/arrows move focus, triggers page long lists. Mouse: hover highlights, click
selects (lists don't change selection on hover, already the rule in `DrawList`), wheel scrolls.

---

## 3. Shared UI kit (build first, then every screen uses it)

Put these in a new `src/ui/NobleUi.cpp/.hpp` (or grow `NobleStyle`), drawn with ImGui draw lists like `FrontEndMenu`:

| Component | Notes |
|---|---|
| `ScreenShell(title, topActions)` | Title, top-right text buttons, background, safe margins, scale by window height (as `Ui::s`). |
| `SectionHeader(text)` | Caps + rule. |
| `CategoryList` | Icon + name + live summary + unsaved dot; groups with headers. |
| `Row` types | Slider (fill bar, value text, optional default tick), Cycle `‹ value ›` with position dots under the value, Toggle pill, Binding pair (primary + alternate key/button, "—" when unbound), Text field, Path row (path + CHANGE… / OPEN), Button row. Disabled/greyed state when a parent option is off. Changed-not-applied dot. |
| `DetailsPanel` | Title, effect line, preview slot (callback), description, facts. |
| `UnsavedBox` | "● N UNSAVED CHANGES / In <category>" + APPLY / DISCARD. |
| `TileGrid` | Picture tiles with name strip, corner badge ("+8 VARIANTS"), equipped check, hover/selected frames; keyboard/pad navigation in 2D. |
| `HeroBanner` | Big picture with gradient, small accent kicker ("HALO: REACH · FIREFIGHT"), title, one line. |
| `BigAction` | Large primary button with subtitle ("START GAME / Sword Base · Slayer") and a secondary next to it. |
| `Modal` | Kicker + title + body + prompts row + CLOSE / primary (required mods, players list, quit). |
| `SearchBox`, `FilterChips`, `SortableTable` | For servers, maps, game types, settings search, music. |
| `CountUp`, `ProgressBar`, `ProgressRing` | Career screens and post-game. |

---

## 4. Screens

Each screen lists: reference (Reclaimer), what Noble's version should do, data/engine status, and how to test.

### 4.1 Main menu
- Keep Reach's vertical list (Campaign, Matchmaking (disabled), Firefight, Custom Games, Forge, Theater (disabled
  until films exist), Armory, Service Record, Settings, Quit) over a layered background, party box top-right with
  emblem, name, service tag and **rank icon**.
- Add a "fast path" tile: "Continue: <last mode> on <last map>" (Bungie's idea from the original menus).
- Test: every entry reachable by mouse, keys and pad; prompts switch with the device.

### 4.2 Settings (three columns, Reclaimer layout)
Reference: Settings screenshots (Graphics, Advanced graphics, Audio, Game options, Language, Keyboard & mouse,
Controller, Communication, Online services, Mods, Folders).

Categories for Noble (every Reclaimer row is wanted, see §4.10; rows that can't work yet are shown disabled with a
one-line reason):

| Group | Category | Rows (status) |
|---|---|---|
| DISPLAY & SOUND | Graphics | Display mode, window size, resolution, FPS limit with frame-time preview, V-Sync (exist); graphics card info line (new, read from DXGI). |
| | Advanced graphics | Every row from §4.10; each enabled as its engine field is found. |
| | Audio | Master/game/music/voice volumes (exist), mute in background (new, easy), menu music on/off + volume + song order + per-song on/off with ▶ preview and length (extend the current two-pane picker), volume meters preview. |
| GAME | Game options | FOV 70-120 (exists), subtitles (exists), crosshair/HUD options only if the engine supports them; preview images like Reclaimer's crosshair mock. |
| | Language | Text/voice/subtitle language once MCC's Reach language files are understood (§4.10). |
| CONTROLS | Keyboard & mouse | Mouse section (sensitivity, vertical ratio, invert exist; acceleration/smoothing new, Noble-side), bindings grouped MOVEMENT / COMBAT / VEHICLES / FORGE / COMMUNICATION & INTERFACE / ARMOR ABILITY, two keys per action (engine has a second key slot at +8, see handoff), wheel up/down (exists). |
| | Controller | Status line ("No controller found" / name), button layout (Reach presets: Default, Southpaw, Boxer, Green Thumb, Bumper Jumper, Recon...), stick speed H/V, invert, dead zones, vibration (exist), per-action pad bindings. |
| ONLINE | Communication | Chat, voice, content filters, Discord (§4.10). Filters work now on names; chat/voice with networking. |
| | Online services | Master server URL(s), player ID + COPY. |
| SYSTEM | Mods | Workshop/local folders, installed Reach mods with ON/OFF, OPEN FOLDER, size; character-mod toggles (see 4.6). |
| | Folders | MCC folder, Noble data folder, audio cache folder + size + CLEAR, disk space bars. |

Rules: search across all settings; per-category footer says when changes apply; unsaved box; RESTORE DEFAULTS per
category. Keep the existing `Settings` JSON keys (old files must load).
Test: change a value, see the unsaved dot and box, DISCARD restores, APPLY saves to `settings.json`.

### 4.3 Campaign and Firefight setup
Reference: Firefight screenshots (map list with thumbnails + numbers, hero banner, game type dropdown, options
button, difficulty tiles, PLAY SOLO / HOST CO-OP, "Firefight options" override page with skulls).
- Left: VANILLA / MODDED tabs (exist) + list rows with **map thumbnail** (UiArt::MapPreview small) and number.
  Modded: collapsible mod groups (exist).
- Right: HeroBanner (kicker "HALO: REACH · CAMPAIGN" or "MOD: <name>"), mission/map blurb.
- Campaign: DIFFICULTY as four big tiles with **Reach's difficulty icons** (Easy/Normal/Heroic/Legendary; art per
  §7), SKULLS as a row of small skull icons (opens the Skulls screen, exists), score multiplier, (later) insertion
  point / chapter select if the engine exposes it.
- Firefight: GAME TYPE picker (variants from MCC, exist) + "FIREFIGHT OPTIONS · DEFAULT" button that opens an
  override page (time limit, lives, skulls on all game) **only once Noble can write those variant fields**;
  difficulty tiles.
- Bottom: PLAY SOLO (primary) and HOST CO-OP (disabled until networking).
- Test: pick a mission/map, change difficulty and skulls, start; the right mission/map/difficulty load.

### 4.4 Custom games: lobby, map picker, game type picker, game options
Reference: Host Game, Select Map, Select Game Type, Game Options screenshots.
- **Lobby:** left column rows MAP / GAME TYPE (with base type chip) / GAME OPTIONS / (PLAYLIST later) / NETWORK /
  HOST SETTINGS, each opening its picker; middle: lobby name, address + COPY, hero card (map, variant, description);
  right: PLAYERS list (emblem, name, tag, rank icon); big START GAME with "Map · Game type" subtitle.
- **Select map:** search, OPEN FOLDER, REFRESH, tabs (Multiplayer / Forge / modded), TileGrid of map pictures with
  "+N VARIANTS" (map variants from MCC `map_variants` and the player's own), right panel with preview, description,
  variant list (BASE MAP / MCC / MY FILES / MOD tags) and USE <MAP>.
- **Select game type:** same pattern with Reach's game-type icons (Slayer, CTF, Oddball, KotH, Assault, Infection,
  Territories, Juggernaut, Headhunter, Stockpile, Race, Rocket Race, Invasion, Grifball, Insane, Forge; art per §7)
  and the variant list per type (MCC `game_variants`, `hopper_game_variants`, player files).
- **Game options editor** (Reclaimer's: Changes, rules, Match, Map & equipment, Traits with Everyone / After respawn
  / Leader / Powerups, search, "Unchanged" values, SAVE AS VARIANT): **needs Reach game-variant (`.bin`) read/write
  support first** (Megalo variant format). Treat as a later milestone; start with read-only display of a variant's
  settings.
- Test: choose map + variant in the pickers, the lobby summary updates, START GAME launches that combination.

### 4.5 Server browser
Reference: Server browser screenshots, filters panel, required mods dialog, players modal.
- Extend `ServerBrowser`: ALL / FAVORITES tabs with counts, search (servers, hosts, maps, modes, mods), mode chips
  with counts, HIDE FULL / HIDE EMPTY, FILTERS (private, other versions, mods any/none/with, ping), sortable table
  (star, name, mod badge, map, game type icon, players, ping bars, version), footer totals and master status.
- Right panel: nothing selected = big counters + how-to; selected = map banner, warnings (ping limit, version),
  host, players, ping, mods, JOIN GAME; VIEW PLAYERS modal.
- Required mods modal (thumbnail, Installed / Download size / conflict, total, trust note) once servers advertise
  mods.
- Joining still needs engine networking (handoff §6 item 4); until then JOIN is disabled with a reason.

### 4.6 Armory / Customize (biggest piece)
Reference: Customize screenshots (identity card, character, armour slots, colours, emblem, 3D model with rotate /
zoom / stance / reset, picture grids with equipped check and randomise, Looks priority list).

**Layout:** left = identity card (emblem, name, service tag, species/character) + slot list grouped CHARACTER /
ARMOR / COLORS / EMBLEM / VOICE; middle = 3D model (drag to rotate, wheel to zoom, STANCE ‹ Rifle / Pistol / Crouch
/ Reach Armory poses ›, RESET VIEW; camera moves to the slot being edited: helmet close-up, shoulder, chest);
right = picker for the focused slot: title, "Equipped: X", RANDOM button, TileGrid of rendered thumbnails with name,
rank/cR lock state (career unlocks, `docs/armor-list.md`), equipped check; IDENTITY panel (player name, service
tag, voice).

**Reach slots:** helmet + helmet attachment (variant), left/right shoulder, chest, wrist, utility, knee guards,
visor colour, armour effect, Firefight voice, body (male/female), primary/secondary colour (Reach has no third
user colour; the third is fixed), emblem foreground/background/colours. Elite: armour set + armour effect.
These map to the profile fields already written by `ApplyArmoryAppearance` (+0x40..+0xA0). Colours are Reach's
**30 palette colours only** (10 × 3 shades); a custom hex picker can't reach the engine, so leave it out (or use
it only for the menu's own accent theme).
**Emblems:** Reach emblem foreground/background indices and colours: the profile offsets are not mapped yet
(research item); the picker grid can use MCC's emblem art if it is in a texture pack, otherwise Tyler's art (§7).

**Characters per mod (Tyler's requirement):** a player keeps a *preferred look per character type*, and a mod that
puts the player in that character type uses it:
- Character types: **Spartan**, **Elite** (both native Reach), **ODST**, **Marine / UNSC infantry**, and any type a
  mod declares (e.g. "RAMP Spartan", "Covenant Grunt").
- Storage: `data/appearance.json` becomes `{"looks": {"spartan": {...}, "elite": {...}, "odst": {...},
  "marine": {...}, "<mod-id>:<type>": {...}}, "priority": [...]}` (keep reading the old single-look file).
- UI: CHARACTER row switches which look is being edited; the slot list shows that character's slots (Marine:
  helmet/face/chest/sleeves/pads/pouches/back; ODST: helmet/gear/attachment/knife/emblem; mod types: whatever the mod
  declares). A LOOKS panel (Reclaimer's priority list) orders the character sets and turns mod sets on/off.
- **Unknown, research first:** how a Reach mod makes the player an ODST/Marine and which permutation/region names
  select its armour pieces. Reach's profile only carries Spartan/Elite appearance. Expected approach: a small Noble
  manifest in the mod folder (`noble-characters.json`: character type, biped/tag name, slot -> permutation names,
  thumbnail paths) plus a runtime step that applies the chosen permutations to the player's biped when the mod is
  active. Prove it on one Workshop mod that uses Marines or ODSTs before building the full UI. Until then the ODST /
  Marine looks are saved and previewed but marked "used by mods that support Noble looks".
- Preview models come from local OBJ exports today (git-ignored `data\armor\`); the shipped version must read models
  from the player's MCC maps (handoff Armory notes). Thumbnails for the picker grids can be rendered once with the
  preview renderer and cached under `data\ui\cache\armory\`.
- Test: pick each Reach slot, APPLY, start a game, check in third person (or the post-game/Armory view) that the
  piece and colours match; switch character types and confirm each keeps its own look.

### 4.7 Service Record: ranks, commendations, challenges
Data already exists in `docs/career/` (all 45 commendations with tiers and cR, 2,103 real challenges, 50 ranks).

**Ranks**
- Rank icon everywhere a player appears: party box, lobby, scoreboard, post-game, Service Record header.
- Service Record header: big rank icon + name ("Lt. Colonel, Grade 2"), cR total, progress bar to next rank with
  numbers, "next: <rank icon> <name> at N cR".
- A RANKS tab: all 50 ranks as a grid of icons (earned bright, next highlighted, later dim) with the cR needed.
- Post-game rank-up: icon flips to the new rank with `ReachAudio::Sfx::RankUp`.

**Commendations** (replace the 6 generic tracks with Reach's 45)
- Tabs CAMPAIGN (16) / FIREFIGHT (15) / MULTIPLAYER (14); a grid of commendation cards: icon in the tier's frame
  (Iron, Bronze, Silver, Gold, Onyx, Max), name, progress ring or bar toward the next tier ("312 / 500"), cR for the
  next tier. Selecting one shows the details panel: what counts, all tiers with their counts and cR, date reached.
- Tier-up during a game shows on the post-game screen with the tier icon and `Totals`/`RankUp` sounds.
- Counting needs the stats from the slot-6 results record (kills by weapon class, headshots, sprees, ...;
  `career-reference.md` §4). Wire the ones that are already decoded first; show the others as "tracking soon".
- **Art:** Tyler said he would attach the commendation rank pictures and commendation photos; **they were not in
  the reference set received on 2026-10-28** (the 43 images are all Reclaimer screens). Ask Tyler for them.

**Challenges** (make them feel like Reach, not a plain list)
- DAILY and WEEKLY sections with a **countdown to reset** ("Resets in 7 h 12 m", reset 11:00 GMT like Reach or local
  time, decide with Tyler).
- Each challenge is a card: mode icon (Campaign / Firefight / Multiplayer), name, requirement, progress bar with
  numbers, cR reward in amber; completed cards flip to a "COMPLETE" state with a check and the cR counted in.
- Use Reach's real challenges: pick today's set from `challenges.json` (e.g. the same calendar day of 2010-2012 or a
  seeded pick per mode; Tyler earlier chose 7 daily / 5 weekly). Skip or reword ones that need Xbox Live.
- Post-game: a "Challenges" strip showing progress made this game; completion plays `SlotMachine`/`Totals`.
- Test: finish a short game, see progress move on the post-game screen and in the Service Record; completed
  challenge pays its cR once.

### 4.8 Post-game
Already has scoreboard + cR lines + rank bar. Make it Reach's "exit experience": staged reveal (score, then cR lines
counting up with `Totals`, slot-machine bonus with `SlotMachine`, challenge/commendation progress, rank bar fill,
rank-up), skippable with A/Enter, then CONTINUE / PLAY AGAIN / LEAVE.

### 4.9 Theater
Reference: Theater screenshot (recent games list, hero card, players, WATCH / SAVE FILM / RENAME / DELETE / EXPORT).
Needs film recording/playback support in the host (engine work, not started). Keep the menu entry disabled with
"Coming later" until then.

### 4.10 Features Reach never had (all wanted)
Tyler (2026-10-28 10:02): every idea in the Reclaimer screens should come to Noble, including things Halo: Reach
never had, so Noble becomes one complete package. Build all of these. "Noble-side" means it needs no engine
research; "engine" means a field or hook has to be found first. Where a row can't work yet, show it disabled with a
one-line reason instead of hiding it.

| Feature | Where | How / status |
|---|---|---|
| Mouse acceleration (Off..N) | Keyboard & mouse | Noble-side: scale the slot-36 look delta by speed (`ReachHost` input). |
| Mouse smoothing (0-100 %) | Keyboard & mouse | Noble-side: average the last few look deltas. |
| Invert flight (separate from invert look) | Mouse + controller | Engine: needs "in a flying vehicle" state, else Noble flips Y only while in Banshee/Falcon (research). |
| Halve / double sensitivity hotkeys | Key bindings | Noble-side (temporary multiplier while held/toggled). |
| Release cursor key (F1) | Key bindings | Noble-side (frees the clip, already have the code path). |
| Second key per action | Key bindings | Engine has a second key slot (+8, used for the wheel); extend to every action. |
| Aim control Modern / Classic, aim acceleration, stick layout, look radial dead zone | Controller | Noble-side on the XInput values before they go into slot 36. |
| Button layout presets (Reach's: Default, Southpaw, Boxer, Green Thumb, Bumper Jumper, Recon) | Controller | Noble-side remap of the pad buttons. |
| In-vehicle FOV, third-person FOV | Game options | Engine: find the vehicle/third-person camera FOV (FOV research already found the first-person one). |
| Centered crosshair, crosshair size, scale with FOV | Game options | Engine/HUD research; preview image in the details panel. |
| Hide HUD, hide helmet visor | Game options | Engine/HUD research (HUD already hooked for colour). |
| Fade distant player labels (distance, opacity) | Game options | Engine/HUD research. |
| Weapon viewmodel offsets (sprint sway, H/V/depth, per weapon) | Game options | Engine research. |
| Graphics card choice + "in use now" line | Graphics | Noble-side (DXGI adapter enumeration; choose adapter when creating the D3D11 device). |
| Advanced graphics (texture filtering, AA, motion blur, shadows, lights, effects, draw distance, LOD, decals, water, presets) | Advanced graphics | Engine research per item; texture filtering / AA may be Noble-side in the D3D11 layer. |
| Mute in background | Audio | Noble-side (lower XAudio2/engine volume when the window loses focus). |
| Menu music playlist: per-song on/off, ▶ preview, length, song order | Audio | Noble-side (ReachAudio already knows every track and length). |
| Volume meters preview | Audio | Noble-side. |
| Text / voice language, subtitles language, extra alphabets | Language | Research: which MCC Reach language files exist (`english.fsb` etc. for voice). |
| Content filter: filter chat / player / server names, slurs filter, profanity filter, custom filter + custom words (one per line) | Communication | Noble-side word lists applied to chat, player names and server names in the browser. |
| Game chat on/off, coloured chat names | Communication | Noble-side once chat exists (networking). |
| Voice chat (on/off, volume, global / team / proximity volume, microphone on/off + device, talk mode hold/toggle, push-to-talk keys) | Communication | New Noble voice system on top of networking; device list via Windows audio APIs. Later milestone. |
| Show my game on Discord + join button | Communication | Noble-side Discord Rich Presence (game/map/mode, party size; join secret once joining works). |
| Master servers list, server query batch size, stats service, player ID + COPY | Online services | Noble-side (`MasterServerClient`; player ID = stable hash, stays the same when renamed). |
| MCC folder / cache folder / download folder with CHANGE… / OPEN, sizes, disk-space bars per drive | Folders | Noble-side (settings + `SHBrowseForFolder`/IFileDialog; "restarts to use a new folder"). |
| Mods: downloaded mods folder, prefer Steam Workshop, download without asking, installed mods ON/OFF, FOLDER, DELETE (to Recycle Bin), totals, character mods on/off, asset library rebuild (Forge objects of all mods) | Mods | Noble-side except the Forge asset library (engine research). |
| Server browser: favourites (star a server), search servers/hosts/maps/modes/mods, mode chips with counts, hide full / empty, filters (hide private, hide other versions, mods any/none/with, VPN any/blocked/allowed, ping 60/110/180), sortable columns, version column, master status footer | Servers | Noble-side UI over `MasterServerClient`; servers must advertise mods, version, VPN policy. |
| **Mod badge in the server list** (the little cube with a number, e.g. on the Permafrost server): click it to open that server's **mod list** (the "Required mods" window: thumbnail, Installed / Download size / conflict, total to download) | Servers | Noble-side UI; needs servers to list mod ids + sizes; download from Steam Workshop or the server with manifest checks. |
| VIEW PLAYERS (list of names on a server), ping-limit warning, "Hosted by", cosmetic mods | Servers | Noble-side once servers report them. |
| Favourite maps and game types (star in the pickers, a FAVORITES tab) | Map / game type pickers | Noble-side (saved in settings). |
| OPEN FOLDER / REFRESH in map and game-type pickers, "+N variants", search | Pickers | Noble-side. |
| Game options editor with SAVE AS VARIANT, search every setting, "Unchanged" values | Custom games | Engine/format: Reach game-variant read/write. |
| Playlist (rotate maps/game types in a hosted lobby) | Lobby | Noble-side once hosting works. |
| Host settings (max players, voice, chat), network Online/LAN, address + COPY | Lobby | Noble-side with networking. |
| Randomise buttons (name, emblem, armour slot) | Armory | Noble-side. |
| Stances, drag-to-rotate, scroll-to-zoom, reset view, camera close-ups per slot | Armory | Noble-side in the preview renderer. |
| Per-character looks (Spartan, Elite, ODST, Marine, mod characters) + LOOKS priority list | Armory | §4.6 (research spike first). |
| Theater: recent games, save film, rename, delete, export MP4, open folder | Theater | Engine research (film recording/playback); MP4 export via the frame capture path. |
| "Continue last game" fast path | Main menu | Noble-side. |
| Search across every setting | Settings | Noble-side. |

### 4.11 Details found on the second and third pass over the screenshots
Small things that make the Reclaimer screens feel finished; add them while building the matching screen.

- **Split screen** category (GAME > SPLIT SCREEN, "Just you"): local split-screen co-op for 2-4 players, with its
  own graphics preset ("Auto keeps less detail with more players"). Reach supported 4-player split screen on
  Xbox 360; engine research for the PC host.
- **Mute** (separate from mute in background), and a master volume above the per-group volumes.
- **Default marks on sliders**: a small tick shows the default (FOV, mouse sensitivity) and on the FPS-limit track
  the display's refresh rate. "Custom" values (FPS 30-1000) typed in.
- **Value position dots** under every ‹ value › selector, and the arrow greys out at the first/last value.
- **When it applies**, stated per option: "Takes effect when you apply changes", "Saved and used as soon as you
  change it", "Takes effect the next time you start the game", "The game restarts to use a folder you choose here".
- **Unlock viewmodel FOV** (weapon FOV separate from world FOV) and **per-weapon viewmodel** ("Weapon: All weapons"
  ‹›, "Own placement" per weapon).
- **Record my games** (Theater) on/off.
- **Language rows** say "Automatic" by default; optional **font packages** for Chinese / Japanese / Korean player
  names with their size ("about 4 MB").
- **Graphics card line** names the adapter, its video memory and who chose it ("chosen by Windows").
- **Window size greys out** in borderless ("Fills your display").
- **Identity**: player name with a random-name button, service tag "blank keeps a random one", voice male/female,
  note "On lobby cards, scoreboards and chat".
- **Equipped line** on every picker ("Equipped: CQB", "Equipped: #54 of 80", "Equipped: White, Cobalt on White").
- **Armour lights colour** and **visor colour** rows; **Uniform** colour for ODST/Marine.
- **Emblem options**: INVERT and HIDE SECONDARY toggles, big emblem preview with "FOREGROUND 54 · BACKGROUND 38".
- **Custom colour with nearest-palette fallback**: Reclaimer allows any hex colour and shows "Older versions show
  White, the nearest Halo 3 colour". Noble can do the same: custom colours for Noble's own UI (lobby cards, emblem in
  menus, other Noble players), while the engine gets the nearest of Reach's 30 palette colours.
- **Character mods shown to others**: "Servers that add it show you as this character"; a mod turned off is neither
  played nor seen (others using it look like the default character, but hit boxes stay real).
- **Mod list rows**: source ("Steam Workshop folder", "Mods folder", "From modded missions"), maps inside, size;
  DELETE is greyed for Workshop items (unsubscribe in Steam instead) and has a tooltip ("Move this mod to the
  Recycle Bin"); totals "20 mods · 105 GB".
- **MY MODS** shortcut from the server browser's top bar.
- **Server rows**: status dot (colour = state, e.g. joinable / private / modded), "Hosted by Dedicated server",
  "Blocks VPNs", "It checks again as you join" (ping re-check), master status "2/3 masters responded. Listings
  combined from multiple sources. 358 of 378 servers answered", "Refreshing…" state on the button, double-click to
  join, star to favourite.
- **Lobby**: "CAMPAIGN ›" switch at the top of the GAME section, ONLINE chip, host IP + COPY, "Hosted by",
  chat prompt at the bottom ("T CHAT"), rank icon on each player row.
- **Theater**: "+220 MORE" players, film length and player count on the card, WATCH shows a progress sub-line.
- **Modal windows** always show the keyboard/controller prompts (ENTER Download, ESC Close) next to the buttons.
- **Mod checks when joining**: the Required mods window flags a mod "named like a mod on this PC" (same title, maybe
  a different version) and **conflicts** ("Map deadlock clashes with <other mod> on this PC" = two active mods
  replace the same map file); every downloaded file is checked against the server's manifest.
- **Map picker**: a **Campaign** tab next to Multiplayer (campaign maps for custom games), map variants listed per
  base map with their source (BASE MAP / MATCHMAKING / MCC / MY FILES / MOD), checkmark on the chosen one.
- **Pickers** start with a **Reset** tile (back to default) where it makes sense (Body: Reset / Male / Female).
- **Bindings**: separate ZOOM IN / ZOOM OUT next to ZOOM, TOGGLE CROUCH (hold vs toggle), GAME MODE ABILITIES 1-4,
  vehicle tricks (Reach: Banshee flip / trick, Banshee bomb), unbound shown as "—"; the details panel tells how to
  bind ("Select the key… Backspace clears").
- **Server details** chips such as READY / PING LIMIT on the map banner; players modal ("59 / 60 players · CTF
  30VS30 on Valhalla") with JOIN GAME inside it.
- **Server browser empty state**: "SELECT A SERVER" with three tips (click to see map/players/ping/mods,
  double-click to join, star to keep under Favorites); a live line while refreshing ("Refreshing: 358 of 378 servers
  answered so far") and the REFRESH button showing "REFRESHING…"; ping numbers coloured green/amber/red with bars;
  player counts dim when a server is empty; mod badge in orange when mods are needed, outlined when only optional;
  the PLAYERS column header is the sort key (arrow).
- **Server panel**: a short player-name preview line ("GoldenS4ucy, Door, Bifus…") above VIEW PLAYERS; star button on
  the banner to favourite.
- **Mods settings**: DOWNLOADED MODS DIRECTORY choice (Noble default / a Noble subfolder in MCC's Workshop library /
  custom folder; existing downloads stay where they are), DOWNLOAD FOLDER with CHANGE… / OPEN and the full path shown,
  ASSET LIBRARY with REBUILD and a status ("Looking…"), INSTALLED MODS status line ("Looking at your mods…"),
  CHARACTER MODS section ("Turn a character mod off to keep it without using it: Customize, the main menu and the
  games you host leave it out").
- **Folders settings**: MCC FOLDER, CACHE FOLDER (+ size), DOWNLOADED MODS; details panel shows free space bars per
  drive ("205 GB free of 465 GB") and the chosen folder's full path.
- **Online services**: several master servers (one per line), SERVER LIST REFRESH NOW, player ID shown in full in
  the details panel with "Copy it with the button beside it".
- **Communication**: CHAT NAMES ‹Colored›; the push-to-talk preview shows the bound key.
- **Advanced graphics**: texture filtering values (Original … 16× anisotropic); preset that sets every row; rows
  whose highest step is reached grey their right arrow; details panel bars per item with the focused item
  highlighted and the group name ("ONE PLAYER"); SPLIT SCREEN preset ("Auto").
- **Graphics**: rendering resolution separate from window size (render scale), display mode Borderless fullscreen /
  Fullscreen / Windowed.
- **Game options**: FADE START DISTANCE (units) and DISTANT LABEL OPACITY greyed until FADE DISTANT PLAYER LABELS is
  on; crosshair preview captioned with the current setting.
- **Game type picker**: tile icon per type, variant list tags (DEFAULT RULES / MATCHMAKING / source game), USE <TYPE>.
- **Game options editor**: "Changes apply to the lobby as you make them. Each setting shows the game type's own
  value until you change it; Reset puts it back"; per-row subtitle; value with ◀ ▼ ▶ (step or open the list);
  categories show "No changes" / count; NO CHANGES label next to SAVE AS VARIANT.
- **Firefight options** override page: every row starts at "Game type's"; skulls listed one per row for the whole
  game; DONE returns to the map page.
- **Armory, more slots seen** (map Reach's equivalents): left / right / top / bottom helmet attachments, backpack,
  left/right wrist, waist, left/right thigh (Reach's utility pieces sit on the thigh/waist), body; Spartan colour rows
  PRIMARY / SECONDARY / DETAIL / VISOR / ARMOR LIGHTS ("Default"); emblem colour tabs PRIMARY / SECONDARY / BACKGROUND
  each with PALETTE / CUSTOM; random-dice button on every picker; stance list Rifle / Crouch (add Reach's Armory
  poses); helmet effect row ("HELMET FIRE · Off" = Reach's armour effects).
- **Customize identity card** at the top of the slot list: emblem, name, "KEK · SPARTAN" (tag · character).
- **Per-character slot lists seen** (use as the model for Noble's ODST / Marine looks): ODST = HELMET (ODST
  (standard)), GEAR (Rookie), HELMET ATTACHMENT (None), KNIFE On/Off, CHEST EMBLEM On/Off, colours ARMOR / SECONDARY
  / UNIFORM / VISOR; Marine = HELMET (Standard, No Goggles), FACE (Balaclava (light skin), i.e. face + skin tone),
  CHEST (Pilot Plate), SLEEVES (Full), ARMOR PADS / CHEST POUCHES / BELT POUCHES / THIGH POUCH On/Off, BACK (None),
  colours ARMOR / UNIFORM / VISOR. On/Off rows are simple toggles; others open picture grids.
- **Character picker panel**: "UNSC MARINES · Equipped: Marine" with the choices of that set (MARINE / ODST) and
  "From UNSC Marines. Servers that add it show you as this character."
- **Lobby details**: lobby name editable ("Project Reclaimer game"), NETWORK row ‹ Online / LAN / Offline ›,
  host settings summary "16 MAX · VOICE ON · CHAT ON", players header "1 / 16", local player row highlighted with
  emblem, name, tag and rank icon; START GAME subtitle "Guardian · Custom Slayer"; map card blurb from the variant.
- **Map picker details**: a scroll bar on the tile grid, hovered tile white frame vs chosen tile accent frame + check,
  variant list scrolls with the USE button pinned under it, the chosen variant's description replaces the map blurb.
- **Theater details**: films without a thumbnail show a dark placeholder; list rows "Team Slayer BR on <map>" with
  "2 days ago · 13:07" (age · length); the chosen film shows players in four columns; WATCH is the primary button.
- **Installed mods row**: title, then "<maps inside> · <source> · <size>" truncated with "…"; ON/OFF toggle, FOLDER,
  DELETE (enabled only for mods Noble downloaded or that live in Noble's mods folder).

- **More small print** (later pass):
  - Category summaries reuse the main values ("90° · Crosshair lowered", "Volume 100 % · Music on", "High · Split
    screen Auto", "Just you"); the left category list scrolls when it doesn't fit.
  - FOV sliders show the default as a tick; third-person FOV has its own default (70°).
  - Audio meters in the details panel show each group's level (Game / Voice chat / Menu music); the menu-music
    description names the pieces it plays and says the order and songs are chosen below; songs turned off hide
    their ▶ button.
  - Emblem colours: "Equipped: White, Cobalt on White" (primary, secondary on background); colour swatches in the
    slot list show all three; HEX CODE field with a big swatch; HUE (degrees), SATURATION and BRIGHTNESS (%) sliders
    with value labels; the palette marks the chosen colour with a check.
  - Firefight hero banner: kicker in the accent colour ("HALO 3: ODST · FIREFIGHT"), one-line blurb; difficulty tiles
    show the icon left of the name; PLAY SOLO subtitle "Hold out alone", HOST CO-OP "A lobby for up to 16 players";
    "Night" variants of maps listed as their own entries.
  - Server list: favourite star on every row (filled when starred), the selected row keeps the accent bar, rows of
    modded servers show the mod count next to the cube, campaign/firefight servers use a different game-type icon,
    long names cut with "…", PLAYERS sorted descending by default.
  - Server details: map banner with the map name in large type and game type under it, "Hosted by …", PLAYERS /
    PING / MODS / COSMETIC MODS as four labelled stats, warning text explaining the chip ("takes players with a ping
    up to 250 ms, and yours measured 271 ms. It checks again as you join").

- **Small print, next pass:**
  - Content filter: CUSTOM WORDS box ("One word per line") stays greyed until CUSTOM FILTER is on; the first row
    reads "Filter chat, player and server names".
  - Language: three separate rows TEXT / VOICE / SUBTITLES language, each "Automatic"; alphabet rows are per
    language (Chinese, Japanese, Korean) with the details panel naming what it fixes ("Shows Hangul in HUD text such
    as player names instead of boxes") and the download size; effect line "Saved when you apply changes; takes effect
    the next time you start the game"; RESTORE DEFAULTS greys out when nothing differs from the defaults.
  - Weapon viewmodel: SPRINT SWAY in %, WEAPON ‹All weapons / each weapon›, OWN PLACEMENT greyed for "All weapons",
    offsets shown signed (+0).
  - Advanced graphics: a changed row gets the dot before its name *and* its category gets the dot in the left list;
    SPLIT SCREEN section explains that split screen draws less detail per extra player and that Auto keeps that.
  - Graphics: FRAME-RATE LIMIT details show a big value ("NO LIMIT"), a frame-time bar 0-100 ms, explanation
    (menus and loading screens keep the display's refresh rate; a limit at the refresh rate keeps the PC cooler and
    quieter), DEFAULT 60 FPS; WINDOW SIZE reads "Fills your display" in borderless; RENDERING RESOLUTION 3840 × 2160.
  - Firefight options: row titles explain units ("Time limit (minutes; 0 = unlimited)", "Most lives the team may
    hold"), subtitles "Over the game type's own" / "On for the whole game"; left arrow greyed at "Game type's".

- **Small print, full pass 2026-10-28 (all 43 again):**
  - Category summaries that reveal options: ONLINE SERVICES "Ranks on" (a setting to show or hide ranks),
    COMMUNICATION "Hold to talk · Filter on", MODS "Downloads without asking", FOLDERS "Cache on C: · MCC on D:".
  - Identity panel subtitle "How other players see you"; random buttons on the panel and on the name field; voice
    as two buttons MALE / FEMALE. (Reclaimer's tag is 3 characters; **Reach's service tag is 4**, keep 4.)
  - Installed mods: OPEN MODS FOLDER + REFRESH + totals on one line; DELETE lights up in the accent on hover and
    shows its tooltip; greyed DELETE for Workshop / modded-mission mods.
  - Download without asking: details text explains servers share their mods and that this skips the list ("only
    turn it on if you trust the servers you join"); PREFER STEAM WORKSHOP on/off.
  - Controller page starts with a status line ("No controller found. Connect an Xbox or other XInput controller;
    it works as soon as Windows sees it."); AIM ACCELERATION as a multiplier (1.00×); STICK LAYOUT has four
    choices (dots).
  - Keyboard bindings seen (defaults worth copying where Reach has the action): scoreboard Tab, chat T, team chat Y,
    push to talk V, team push to talk B, proximity chat U, proximity push to talk N, vote yes F5 / no F6, release
    cursor F1, halve sensitivity F2, double sensitivity F3; Forge: monitor mode `~, object properties 2, cycle
    monitor speed F, grab/drop Space, rotate held object right mouse, switch rotation axes R, rotation snap larger /
    smaller Right / Left arrow, reset rotation middle mouse + Up arrow (second key), push / pull wheel up / down,
    clone C, delete Delete, select left mouse, clear selection Right arrow, magnets M, fly up / down E / Q, move
    faster Shift, monitor zoom and Forge tools unbound. Controller: abilities 1-3 on D-pad right/left/down, vehicle
    trick A, Banshee bomb B, monitor mode D-pad up, create objects X, grab A, cycle monitor speed left stick click,
    jump A, crouch / sprint left stick click, fire RT, zoom right stick click, melee B, grenade LT.
  - Host Game: GAME TYPE row shows the variant name plus a base-type chip; NETWORK row has inline ‹ › arrows;
    "Hosted by <name>"; the player row ends with the rank icon.
  - Game type picker: right panel big type icon, variant count heading ("137 VARIANTS"), the chosen variant's
    description replaces the type description, DEFAULT RULES tagged with a check.
  - Players modal: info icon, kicker PLAYERS, server name as title, "59 / 60 players · CTF 30VS30 on <map>", names in
    two columns, ENTER Join game / ESC Close; behind it the panel shows READY and "Blocks VPNs".
  - Game options editor: TRAITS intro ("What players can do: weapons, shields, damage, movement, sensors. Choose who
    gets them."), group tabs with a line under them ("Base traits every player has for the whole game"), rows
    grouped (SHIELDS & HEALTH ...), each value "Unchanged" with ◀ ▼ ▶.
  - Theater: WATCH is a big accent button with a subtitle; hero kicker "RECENT GAME · MULTIPLAYER · 2 DAYS AGO";
    line "<variant> · <map> · <length> · <players>".
  - Server table: VERSION column (e.g. 0.9.23); mod cube outlined when the server's mods are cosmetic/optional,
    filled with a count when required.

- **Small print, second full pass:**
  - LOOKS panel rows: drag handle (≡), number, optional thumbnail of the set, ON/OFF toggle per mod set; the base
    set (row 1) has no toggle; a fixed row under the list for the game-specific set ("Halo 3: ODST · ODST'S GAMES")
    that isn't reordered. Noble equivalent: "Halo: Reach (Spartan / Elite)" fixed at the top, mod sets below.
  - Picker grids: thumbnails are rendered with the player's current colours; the equipped tile has a small corner
    check; long lists scroll inside the right panel; the LOOKS button at the top centre shows the current set
    ("LOOKS · Halo 3 ›").
  - Customize identity card turns into the selected row (accent frame) when focused; the 3D preview background is
    a map scene, not a flat colour.
  - Server rows: the selected row keeps its accent fill while the details panel is open; the status dot is amber for
    servers that need mods or are full / a different colour for normal ones.
  - Settings left list keeps its scroll position per category; categories above the fold scroll off under the
    title (seen with GAME OPTIONS cut off at the top).
  - Folders: each path is shown in full in accent text under its row, and the size of the cache folder at the right.

- **Small print, final pass:**
  - **Admins by player ID**: the player-ID panel explains "Server hosts put this ID in their lists to make you an
    admin on their dedicated servers. It stays the same when you rename yourself." Use exactly this for §4.15: a
    dedicated server's admin list holds player IDs, not names.
  - Menu music details: the music also plays in the pre-game lobby, not only the main menu.
  - Installed mods help text: turning a mod off removes its maps from Host Game and (after an asset-library rebuild)
    its objects from Forge; mods downloaded from servers are stored once however many servers use them; deleting
    moves to the Recycle Bin and a server that needs the mod offers it again.
  - Required mods window: kicker "SERVER MODS" with a warning icon, subtitle "<server>: 5 mods · 1 named like a mod on
    this PC", mods without a picture show a "MOD" placeholder tile, status colours amber (download + size), green
    (Installed), red (conflict text).
  - Advanced graphics value steps include Off / Low / High / Ultra (Dynamic lights: "Ultra lights twice as many as
    High"); each step is explained in the details panel.
  - Controller per-action rows show the bound button as text ("Left stick click", "D-pad right"); the details panel
    for a binding says how to rebind and that Backspace clears it.
  - Theater WATCH button shows a play icon and a progress sub-line under it.

**Coverage check (2026-10-28):** every one of the 43 Reclaimer screenshots was reviewed at least five times while
writing this spec; Appendix A plus §4.10-§4.11 list everything seen on them.

### 4.12 A game for every player
Noble is meant for everyone (new, regular, veteran and returning players alike). That does **not** mean separate
menus or a different menu look per player type: there is one menu design (this spec), and no "classic look" mode
for now.

**First start (shown once, the very first time Noble starts):** one question, "New to Halo: Reach?" with two answers:
- **New player** -> a short setup: keyboard/mouse or controller, player name + service tag, sensitivity (optionally
  a quick turn test), recommended graphics preset; then the main menu.
- **Regular / returning player** -> straight to the main menu.
Store that it was answered (`firstStartDone` in settings) so it never shows again. Settings can re-open the setup.

**Armory unlocks are the same for everyone (Tyler, 2026-10-28):** every Armory item keeps **Reach's original cR
price and rank requirement**, from Halopedia ("Armor customization (Halo: Reach)"; already collected in
`docs/career/armor.json` and `docs/armor-list.md`). Players must **reach the rank and spend the cR they earn** to
unlock helmets, attachments, shoulders, chests, wrists, utility, knees, visor colours, **armour effects** (the
helmet effects) and **Firefight voices** (and any other Firefight extras Reach sold). No free unlocks, no difficulty-based or player-type discounts. Items Reach gave away outside
the Armory (pre-order, Legendary Edition, Waypoint) still need an in-game unlock decided with Tyler (e.g. a
commendation). The Armory shows locked items with their rank and price, and a BUY button with the player's cR
balance.

**Also good for everyone:**
- "Continue" fast path, recent and favourite maps / game types, quick settings in the pause menu.
- Recent players list, quick re-join of the last server, screenshot key, FPS / network overlay (toggle).
- Detailed post-game carnage report (medals, weapon kills, per player) and Service Record history.
- Import/export of key bindings and settings, Reach's controller presets, raw mouse input.
- Accessibility: UI and text size, subtitle size/background, high-contrast focus, colour-blind-safe status colours,
  reduce motion, hold-vs-toggle for crouch/zoom/sprint, full remapping, vibration strength.
- **Crash / issue report button** (approved): bundles the log with player names removed; plus "Open logs folder",
  version on every screen, settings backup/restore, reset buttons, update check with "What's new".

### 4.13 Cutscenes
- **Space skips a cutscene immediately** (and A/Start on a controller): no hold timer, no "press again" step.
- In co-op **campaign and Firefight only the host can skip**; other players' skip presses are ignored (no spam).
  Show "Host can skip" to the others.
- Needs the engine's cinematic-skip call (research) and, for co-op, networking.

### 4.14 Map rotation and voting
- **Dedicated servers:** the server owner/admin sets up a **rotation list** (map + game type + options per entry, any
  number of entries), including rotations made for a **mod set** (maps uploaded with that mod pack). Stored in the
  server's config; the browser shows the current and next map.
- **Player-hosted lobbies:** the host builds a rotation in the lobby (PLAYLIST row) and can turn on **voting**:
  after each game players vote between 2-3 options from the rotation (or "keep playing"); ties go to the host.
- **Joining a modded server:** before joining, Noble checks every map/mod the server's rotation needs: already in
  Noble's mods folder, in the Steam Workshop folder, or missing. Missing ones are listed in the Required mods window
  (size, source) and must be downloaded (Workshop or server, with file checks) before the join continues.
- Needs networking for the multiplayer parts; the rotation editor UI and file format can be built first.

### 4.15 Host and admin controls
For the lobby host and dedicated-server admins (from the lobby and the in-game pause/scoreboard):
- **Kick** a player (and ban for the session / permanently for dedicated servers).
- **End round**, **restart round**, **restart match** (whole match, not just the round).
- **Shuffle teams** / balance teams.
- **End game -> back to the lobby with everyone**, where the host can change map, game type or options and start
  again (a matchmaking-style lobby that keeps the party together).
- Change map / skip to the next rotation entry.
- Only the host/admin sees these; actions are confirmed and announced to players ("The host ended the round").

### 4.16 In-game scoreboard (Tab)
- Each player row has a **microphone icon: one click mutes / unmutes that player's voice** (and a text-chat mute).
- **Mute all** button (and unmute all).
- Host/admin actions from §4.15 on each row (kick, etc.) for the host only.
- Rank icon, emblem, service tag, ping per row.

*Tyler will add more requirements later; keep this section open.*

---

## 5. Settings data to add (when the matching feature lands)

`menuMotion` (full / reduced), `muteInBackground`, `menuMusicPlaylist` (per-track on/off + order), `mouseAcceleration`,
`mouseSmoothing`, `secondaryKeyOverrides`, `padLayout`, `lastPlayed` (mode, map, variant, mod) for the fast path,
`appearance.json` v2 (per character type + priority, §4.6), `challengeResetMode`.

---

## 6. Order of work (small PRs)

1. UI kit (§3) + screen shell, applied to one screen (Settings) as the pilot.
2. Settings in three columns with details panel, search and unsaved box (§4.2).
3. Campaign / Firefight setup with thumbnails, hero banner, difficulty tiles (§4.3).
4. Ranks everywhere + Service Record header + ranks grid (§4.7), using art from §7.
5. Challenges cards with reset timers using `challenges.json` (§4.7).
6. Commendations with Reach's 45 and tier art (§4.7), stats wiring as available.
7. Post-game exit experience (§4.8).
8. Armory layout v2: slot list + picture grids + camera focus + identity panel (§4.6, Spartan/Elite).
9. Research spike: one mod with ODST/Marine player; then per-character looks + LOOKS priority (§4.6).
10. Custom game lobby + map / game type pickers (§4.4); game options editor after variant read/write exists.
11. Server browser upgrade (§4.5) incl. favourites, filters and the clickable mod badge; joining after networking.
12. The Noble-side items of §4.10-§4.12, then §4.13-§4.16 (several need networking) in small batches (mouse acceleration/smoothing, content filters, Discord, folders,
    favourite maps, mute in background, playlist editor, graphics card), then the engine-research items one by one.

---

## 7. Art and assets

**Sources, in order of preference:**
1. **The player's MCC install at run time** (already done for map previews, skulls, controller): look in
   `<MCC>\Data\UI\texturepacks\*` for Reach rank, commendation, medal, difficulty, game-type and emblem images
   (`UiArt::PackImage`). List what each pack contains first (a small dump tool that prints entry names).
2. **Tyler's own Reach copy** (not installed right now; Xbox 360 or MCC files): extracted images placed in
   `data\ui\reach\<category>\<name>.png` on each PC (git-ignored), loaded by `UiArt` the same way as
   `data\ui\custom`.
3. **Community HD remakes** of Reach icons: only if their licence allows use in this project; record the source,
   author and licence in `assets/ui/reach/CREDITS.md`.
4. Noble's own drawings as the fallback (like the controller and button glyphs).

**Layout and names** (whatever the source): `ranks/<00-49>_<name>.png`, `commendations/<id>_<tier>.png` and
`commendations/frames/<tier>.png`, `difficulty/<easy|normal|heroic|legendary>.png`, `gametypes/<type>.png`,
`medals/<id>.png`, `emblems/fg_<n>.png` / `bg_<n>.png`, plus `manifest.json` mapping ids used in code
(`Career.cpp` rank index, commendation ids from `commendations.json`) to files. Code must work with any subset
present (fall back to text/drawn shapes).

**Rank placeholders:** start with the rank images Tyler uploads with this document: name them by the layout above
(`ranks/<00-49>_<name>.png`, rank order as in `Career.cpp` `kRanks`) and load them through `UiArt`. Where they live
(git-ignored `data\ui\reach\` or a tracked `assets/ui/reach/` that CMake copies into `<exe>\data\` like
`assets/ui/noble/`) is O7OF's call.

---

## 8. Open questions for Tyler / O7OF

1. Commendation tier pictures and commendation icons: please send them (not in the 2026-10-28 set).
0. Anything unclear in this spec: ask Tyler for the matching Reclaimer screenshot(s).
2. Rank icons: Tyler's uploaded rank images are the placeholders; final source (MCC packs, Tyler's Reach copy or a
   community HD set) later.
3. Accent colour: keep Noble's Reach blue for focus, amber only for emphasis (proposed), or a different scheme?
4. Challenge reset: Reach's 11:00 GMT or local midnight? Real 2010-2012 challenge sets by date, or random from the
   pool?
5. Which mods (Workshop ids) use ODST/Marine/other player characters, to test per-character looks?
6. Is Theater wanted soon (needs film support in the engine) or later?
7. Version scheme (handoff 09:55): patch bumps for normal changes, minor for agreed milestones, 1.0 for a release?

---

## Appendix A: Reclaimer reference, screen by screen

### Settings (3 columns)
- Left category list grouped DISPLAY & SOUND / GAME / CONTROLS / ONLINE / SYSTEM; each row = line icon, name and a
  live summary ("Borderless · No limit", "Sensitivity 2.40 · 5 rebound", "No controller connected"); orange dot on a
  category with unsaved changes. Bottom-left RESTORE DEFAULTS, or "● 1 UNSAVED CHANGE / In Advanced graphics" with
  APPLY CHANGES / DISCARD. Footer note per category ("Changes in this category are saved as you make them").
- Middle rows by section; row types: slider (fill, value "90°" / "100 %" / "+0", default tick), ‹ value › cycle with
  position dots, ON/OFF pill, two binding buttons, text field, path rows (CHANGE… / OPEN). Dependent rows dim.
  Changed rows get a dot before the label.
- Right details: title, effect line, preview (crosshair mock "LOWERED, AS IN HALO 3"; FPS limit bar 0-100 ms with
  "NO LIMIT as many frames as your PC can draw"; volume meters GAME / VOICE CHAT / MENU MUSIC; quality bars for every
  advanced-graphics item; mic icon + push-to-talk key; disk usage bars per drive; player ID box), description,
  facts (DEFAULT 60 FPS, FONT PACKAGE about 4 MB).
- Graphics: graphics card (Automatic + "In use now: NVIDIA GeForce RTX 4070 (11.7 GB), chosen by Windows"), display
  mode, window size (dim in borderless), rendering resolution, frame-rate limit (No limit / custom 30-1000), VSync.
- Advanced graphics: texture filtering, antialiasing, motion blur, preset, shadows, dynamic lights, effects, draw
  distance, level of detail, decals, water, split-screen preset.
- Audio: master, game audio, mute, mute in background, menu music on/off, volume, song order, each song with ▶,
  length and ON/OFF.
- Game options: FOV, in-vehicle FOV, third-person FOV, unlock viewmodel FOV, centered crosshair, scale crosshair with
  FOV, crosshair size, hide visor, hide HUD, fade distant labels (+start distance, opacity), weapon viewmodel (sprint
  sway, per-weapon own placement, H/V/depth offsets), theater record my games.
- Language: text language, voice language, subtitles; alphabets (Chinese/Japanese/Korean font packs).
- Keyboard & mouse: mouse sensitivity (default tick), aim acceleration, mouse smoothing, invert look, invert flight;
  then MOVEMENT, COMBAT, FORGE (monitor mode, create objects, properties, monitor speed, grab/drop, rotate held,
  rotation axes, snap larger/smaller, reset rotation, push/pull, clone, delete, select, clear, magnets, fly up/down,
  move faster, monitor zoom), VEHICLES, COMMUNICATION & INTERFACE (scoreboard, chat, team chat, push to talk, team /
  proximity talk, vote yes/no, release cursor, halve/double sensitivity), GAME MODE ABILITIES 1-4; two keys each.
- Controller: status line, button layout, sticks (aim control Modern/Classic, H/V sensitivity, aim acceleration,
  invert look/flight, stick layout, vibration), dead zones (look, look radial, move), per-action pad bindings.
- Communication: game chat, chat names coloured, voice chat on/volume, global/team/proximity volumes, microphone +
  device, talk mode; content filter (chat/player/server names, slurs, profanity, custom words); Discord presence +
  join button.
- Online services: server query batch size, master servers list, refresh, stats service URL, player ID + COPY.
- Mods: downloaded mods directory, download folder, prefer Steam Workshop, download without asking; asset library
  (Forge objects of all mods) rebuild; installed mods with ON/OFF, FOLDER, DELETE (to Recycle Bin), "20 mods ·
  105 GB"; character mods on/off.
- Folders: MCC folder, cache folder (+size), downloaded mods; "The game restarts to use a folder you choose here".

### Firefight
Map list with thumbnails and numbers; hero banner ("HALO 3: ODST · FIREFIGHT", CRATER, "Fight in Dare's sunken
crash-site."); GAME TYPE dropdown + "FIREFIGHT OPTIONS · DEFAULT"; four difficulty tiles with icons; PLAY SOLO
("Hold out alone") + HOST CO-OP ("A lobby for up to 16 players"). Options page: "Settings over the game type's own,
for every player of the game", GAME (time limit, team lives, most lives), SKULLS ALL GAME (Iron, Black Eye, Tough
Luck, Catch, ...), each ‹ Game type's ›, DONE.

### Customize
Identity card; CHARACTER (Spartan / ODST / Marine / RAMP Spartans); ARMOR slots differ per character (Spartan H3:
helmet, shoulders, chest, helmet fire; RAMP: helmet + 4 attachments, chest, backpack, shoulders, wrists, waist,
thighs, knees, body; ODST: helmet, gear, attachment, knife, chest emblem; Marine: helmet, face, chest, sleeves, pads,
pouches, back); COLORS (primary, secondary, detail/uniform, visor hex, armour lights); EMBLEM (foreground #/80,
background #, colours with palette of 30 named colours or custom HSV + hex, invert, hide secondary). 3D model with
drag/zoom, stance, reset view, camera close-ups per slot. Right picker grids with thumbnails, equipped check,
randomise. Identity: player name (+random), service tag (3 chars), voice male/female. LOOKS: priority list of
character sets (drag to reorder, toggle mods), "Halo 3: ODST" set used for ODST's own games.

### Custom games
Host Game lobby (top nav SETTINGS · CUSTOMIZE · LEAVE LOBBY; MAP, GAME TYPE + base chip, GAME OPTIONS, PLAYLIST,
NETWORK, HOST SETTINGS; lobby card with IP + COPY; players list with rank; START GAME with subtitle; "T CHAT").
Select Map / Select Game Type (search, OPEN FOLDER, REFRESH, tabs, tile grid with "+N VARIANTS", right panel with
variant list and USE button). Game Options editor (name, "based on SLAYER", NO CHANGES / SAVE AS VARIANT, categories
CHANGES / SLAYER RULES / MATCH / MAP & EQUIPMENT / TRAITS / WORLD & FORGE, search every setting, traits per group
EVERYONE / AFTER RESPAWN / LEADER / RED / BLUE / YELLOW POWERUP, values "Unchanged").

### Server browser
ALL SERVERS / FAVORITES, search, DIRECT CONNECT, REFRESH, mode chips with counts, hide full/empty, filters (private,
other versions, mods, VPN, ping), table (star, status dot, name, mod badge, map, game type, players, ping, version),
footer "358 of 358 servers · 150 players online · 2/3 masters responded"; right panel counters or server details
(banner, warning chip, host, players, ping, mods, cosmetic mods, explanation, VIEW PLAYERS, JOIN GAME); players
modal; required mods modal (Installed / Download size / conflict, total, trust note, ENTER/ESC prompts).

### Theater
Recent games list (count shown, e.g. 25) (thumbnail, "<game> on <map>", "2 days ago · 28:33"), hero card ("RECENT GAME · MULTIPLAYER · 2
DAYS AGO", title, variant, map, length, players), players grid "+220 MORE", WATCH / SAVE FILM / RENAME / DELETE /
EXPORT MP4 / OPEN FOLDER.
