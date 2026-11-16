# Player guide - Firefight, Marine and appearance options (v0.19.103 / current dev tree)

Written 2026-11-15 from the v0.19.103 code (verified against the source by a code inventory) and the stabilization QA.
This is a player-facing guide to what exists **now**. Planned options that do NOT exist are listed so nobody goes
looking. Anything not confirmed by a test is marked **UNVERIFIED**.

## Firefight

**Where:** Main Menu > FIREFIGHT.

### Setup screen (before hosting)

| Option | What it does | Status |
|---|---|---|
| MAP | 10 Firefight maps: Beachhead, Corvette, Courtyard, Glacier, Holdout, Outpost, Overlook, Waterfront, Unearthed, Installation 04 (Halo CE). | SHIPPED |
| GAME TYPE | 7 stock Firefight modes: Classic Firefight, Generator Defense, Gruntpocalypse, Rocketfight, Score Attack, Firefight, Firefight Versus; plus player-made modes saved as presets based on the Firefight variants. | SHIPPED |
| MAX PLAYERS | 4 / 8 / 12 / 16 (default 16). **UNVERIFIED above 4:** stock Reach Firefight is 4-player co-op; the 8/12/16 choices have never been tested with real players. If a big Firefight misbehaves, set it back to 4. | UNVERIFIED (option is real, the counts are not) |
| DIFFICULTY | Easy / Normal / Heroic / Legendary (affects enemy mix/behaviour as in stock Reach). | SHIPPED |
| GAME OPTIONS | Opens the same 38-rule variant editor as Custom Games for the selected Firefight game type (teams/time/score, respawn, shields and health, weapons, movement, appearance, sensors). Choose it **before** hosting - the lobby shows it read-only. | SHIPPED (editor opens; individual rules' Firefight behaviour NOT verified) |
| PLAY SOLO / HOST CO-OP | Start options. | SHIPPED |

**Skulls are NOT available in Firefight** - the Firefight launch always uses skulls 0 and there is no skull row in the
Firefight setup or lobby screen (skulls belong to the Campaign screen). **Teams are off** (co-op) and the Firefight
lobby has no team count.

### Firefight lobby
Rows: READY, GAME TYPE, MAP, GAME OPTIONS (display-only "DEFAULTS"), START FIREFIGHT. There is **no** difficulty row,
**no** skulls row and **no** MAX PLAYERS row in the lobby - those are fixed on the setup screen before hosting. Guests
see HOST MODE instead.

### Firefight share codes (SHIPPED, end-to-end UNVERIFIED)
Hosts get a lobby **share code** (`NB1-XXXX-XXXX-XXXX-XXXX`) with **COPY CODE**; friends join with **Main Menu >
JOIN WITH CODE**, the Server Browser's **JOIN WITH CODE**, or **Social > JOIN WITH CODE**. Private lobbies stay
unlisted but the code still joins them. The code/UI are proven; a real two-PC join has never been tested.

### NOT AVAILABLE in Firefight (planned, not built)
- Enemies per wave / dropship load multiplier.
- Friendly AI count / allied squads.
- Wave pacing controls.
- Firefight AI spawn multiplier (2x/3x) - it is in the roadmap, not in the build.
- Skulls, teams, or max-player changes inside a hosted Firefight lobby.

## Marine

### Armory Marine tab (SHIPPED)
The Armory's SPECIES row has SPARTAN / ELITE / **MARINE** tabs. The Marine tab has a separate appearance set:
- **Gender**: Male / Female (female has fewer pieces).
- **Marine / ODST slots** (9): helmet, helmet gear, chest, shoulders, backpack, etc. "MOD DEFAULT" keeps the mod's
  own look.
- **COLORS**: MOD DEFAULT or MY COLORS, with Primary / Secondary / Detail / Accent from the Reach palette.
This lets you preview and save a Marine look; it is applied to the local character when a mod spawns you as a Marine
(for example the old Trooper Firefight mod). **It is not applied when other players are in the session** (lockstep
safety). There is no stock way to play as a Marine in a match, and the old Trooper mode was removed - Marine
appearance is for modded Marine play / previews.

### NOT AVAILABLE
- Playing as a Marine in standard Campaign, Firefight or multiplayer.
- Marine animation fixes (parked).

## Appearance (Armory)

**Where:** Main Menu > ARMORY (APPEARANCE). Left rail = slots, middle = choices, right = 3D preview (drag or Q/E to rotate).

### What you can change (all SHIPPED)
- **Species**: Spartan / Elite / Marine previews.
- **Spartan**: Body (Male/Female); Helmet + Helmet Variant; Left/Right Shoulder; Chest; Wrist; Utility; Knee Guards;
  plus RANDOM.
- **Elite**: 11 Armor Sets (Minor, Spec Ops, Ranger, Ultra, Zealot, General, Field Marshal, Officer, Honor Guard,
  Counselor, Custom) and individual Helmet / Shoulders / Chest / Arms / Legs pieces.
- **Colours**: Primary and Secondary from the Reach palette; **CUSTOM COLOR** opens a colour wheel + RGB sliders and
  stores the exact colour (then matched to the nearest palette index for your profile). Emblem + Emblem Background
  plus their Primary/Secondary/Background colours and a Nameplate; **Service Tag** (1-4 characters).
- **Armour Effect** (Spartan and Elite): 7 selectable stock effects - NONE, BIRTHDAY PARTY, HEART ATTACK, PESTILENCE,
  INCLEMENT WEATHER, LEGENDARY (flames), ETERNAL - rank/credit gated.
- **Firefight Voice** (Spartan): 12 selectable voices, rank/credit gated.
- Most items are rank- and credit-gated: a locked item can be previewed but not equipped (the info panel offers BUY);
  a saved locked item falls back to the default/Minor.

### Visor colours (SHIPPED - known limitation)
The Armory offers 30 MCC visor finishes. In a match the engine has **five real visor slots** (Default/Silver, Blue,
Gold, Black); Noble maps every finish to the **nearest stock slot**, so nothing renders white (fixed v0.19.102+).
Extended finishes look exact in the Armory preview but close-not-exact in game. Exact per-player custom visor colour
is not implemented.

### NOT AVAILABLE in Appearance
- **Colour-variant armour effects** (extra blue/purple flames etc.): the 7 stock effects are selectable, but new
  recoloured variants are a developer proof only (modified particle palettes), not an Armory entry.
- **Elite Lights colour row**: hidden (the Honor Guard glow stays fixed gold).
- **Spartan lights/accent colour rows**: not exposed (primary/secondary only; undersuit colouring was removed).
- **Elite lighting/shine**: parked.
- **Exact in-match per-player visor RGB**: parked engine task.

## Settings that matter to the above

Settings > GAME OPTIONS (this is Noble's own settings page, not the lobby variant editor):
- **FIELD OF VIEW** 70-120 degrees; 1st-person FOV modifiers on foot and in vehicles; third-person zoom and camera
  offsets; **SUBTITLES**; **MENU BACKGROUND** (random/Noble art/Halo Reach art/custom image).
- **ENGINE LIMITS** shows **STOCK** only and **AI ACTORS** shows 96 (STOCK): the extended engine-limit profiles are
  disabled in this build for stability. This is intended, not a bug.

Settings > GRAPHICS: monitor, display mode (windowed/borderless/fullscreen), resolution/window size, VSync, frame-rate
limit, throttle-when-unfocused, FPS counter, UI scale, brightness, anti-aliasing (FXAA), menu supersampling.

Settings > CONTROLS: mouse sensitivity/invert/acceleration/smoothing, mouse-wheel actions, full key bindings,
**TOGGLE CROUCH**, **TOGGLE ZOOM**; controller presets/button bindings/paddles/toggle zoom/sensitivity/dead zones/
vibration/UI cursor.

## Lobby and party (context for hosting)

- Custom Games lobby: GAME TYPE, MAP, TEAM COUNT (2-8 when teams are on), CHANGE TEAM, VOTE (playlists) or GAME
  OPTIONS (read-only summary), START. Team colours/names come from the game variant.
- Campaign lobby: READY, MISSION, SKULLS (summary), DIFFICULTY, START CO-OP CAMPAIGN.
- Firefight/Forge lobbies: no team count; Forge teams are red/blue only.
- **MAX PLAYERS is chosen on the setup screen, not in the lobby**; campaign is 4 only, custom/Forge/MP picker is
  capped at 32 (16 default) by the stabilization pass.

## Known issues worth knowing (player-facing)

- Campaign co-op is **4 players only**; campaign >4 crashed/black-screened when tried.
- Multiplayer/Forge host MAX PLAYERS picker is capped at **32** (16 default); 128-player work is on hold.
- Firefight >4 players and Firefight lobby options are **unverified**; skulls are unavailable in Firefight.
- Two-PC lobby-code join is untested; treat first use as a test.
- Forge mode loads/plays on Forge World, but the Forge editor monitor (flying editor) is untested.
- Extended ENGINE LIMITS options are gone for now; the build resets to Stock on start.
- Most individual settings rows have not been verified one by one (they render, but their in-game effect was not
  measured in the stabilization QA) - flagged as UNVERIFIED above where relevant.

*Anything marked UNVERIFIED could not be confirmed by a test. Ask Tyler before relying on it for a community event.*
