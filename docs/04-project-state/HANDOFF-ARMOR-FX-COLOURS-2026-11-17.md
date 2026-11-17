# Handoff - Armor FX primary-colour flame variants (2026-11-17)

Status: **24 of 30 colours are working and selectable.** Map rebuild/mod route proven and paused; runtime particle-colour
override is the chosen next approach. Repo is on branch `wip-merge` (== `main`), last feature commit **789c950**
("Armor FX: ship working 24-colour variant set"). Build clean for `noble-dev`/`noble-client`.

Read this first, then:
- Deep history/RE detail: `docs/project-docs/RE-ARMOR-FX-VARIANTS-2026-11-16.md`
- Colour research history: `docs/05-research-and-prompts/RE-ARMOR-FX-COLORS-2026-11-15.md`
- Engine limits: `docs/02-engine-reference/TOPIC-armor-appearance-colors-armory.md`
- Runtime-colour plan: `docs/05-research-and-prompts/RE-ARMOR-FX-RUNTIME-COLOR-PLAN-2026-11-17.md`
- Script index: `docs/test-tools/ARMOR-FX-PIPELINE-INDEX.md`

---

## 1. What exists and works right now

### Player-facing
- Armory **ARMOR EFFECT** list has **32 entries**: 7 stock effects + `PRIMARY MATCH` + **24 primary-colour flames**
  (STEEL, SILVER, WHITE, BROWN, TAN, KHAKI, SAGE, OLIVE, DRAB, FOREST, GREEN, SEA FOAM, TEAL, AQUA, CYAN, BLUE,
  COBALT, ICE, VIOLET, ORCHID, LAVENDER, MAROON, BRICK, ROSE).
- Six colours are **hidden/not working yet**: RUST, CORAL, PEACH, GOLD, YELLOW, PALE (no attachment slots left; see §3.1).
- `PRIMARY MATCH` resolves the player's `primary_color` and clamps index 24..29 (the six missing colours) to ROSE.
- Verified in the Armory (clean process each time): NONE and BIRTHDAY PARTY show **no** regular flame; STEEL shows only a
  grey flame; BRICK only a red/pink flame; PRIMARY MATCH + green primary -> green flames and green armour.
- Verified in-match on rebuilt maps: BRICK red/pink only, auto-green green only (Forge World and Boneyard).
- Custom item ids 0xF0..0x107 are written to profile fields +0x68 (Spartan) and +0x90 (Elite); the MCC item translator
  detour maps them to pmcg list indices 7..30. Hook installed unconditionally; `-armor-fx-probe` only enables logging.
- Settings: game window opens on the non-primary/second monitor (settings monitor = 2) for Tyler's AOC.

### Assets / tags (all local HREK, NOT committed - large binaries)
- 30 palette tags `tags\fx\particles\fire\gradients\fire_pri_<id>.bitmap` (solid-tint formula, no white-hot core).
- 30 particle tags `tags\objects\characters\spartans\fx\armor_fx\particles\head_fire_pri_<id>.particle`.
- 5 effect tags `...\fx\armor_fx\armor_fx_ext0..4.effect` (6 events/locations each). **Only ext0..3 are attached**
  (ext4 exists but is unused; needed again only if more slots become possible).
- `spartans.render_model`: 30 Foundry markers `flame_pri_<id>` merged in; armour regions/permutations rearranged:
  - `armor_fx` perm index 0 = **armor_fx_default** (markerless; stops the stock flame being active by default)
  - perms 1..6 = stock `armor_fx_00..05` (stock effects; markers shifted to match)
  - perms 7..30 = variant names `armor_fx_06..29` for variants 0..23 (steel..rose)
  - `armor_fx_pri` perms 0..4 = `armor_fx_pri_00..04` (variants 25..29: coral..pale; currently unused by the 24 set)
  - markers use stock `armor_fx_00` transform (node 42, stock translation/rotation/scale) so flames stay centred.
- `spartans.biped`: **9 attachments** = 5 stock + ext0..3:
  stock: shield_down.effect, body_damage_screen_distortion.effect, out_of_bounds.effect, armor_fx.effect,
  flashlight_3p.light. Dropped to make room: night_vision.light, bungie_helmet.cheap_light, flashlight_1p.light,
  shield_down.light (minor missing visual features; revisit).
- pmcg: both effect lists (Spartan `spartan armor effect selections`, Elite `elite armor effect selections`) have 37
  entries (7 stock + 30 variants); custom ids map 0xF0..0x10D to list indices 7..36, but only 7..30 are used by the
  24-colour set.
- `spartans.biped.bak` is the stock backup (68,974 bytes). Maps rebuilt: `HREK\maps\forge_halo.map` (797,151,232 B)
  and `HREK\maps\mainmenu.map` (312,242,176 B), installed as `build\bin\Release\data\menumaps\noble_menu.map` plus
  `noble_menu.string_ids.csv` (regenerated after the last mainmenu build).
- Mod folder PoC: `build\bin\Release\data\mods\armor_fx\maps\` contains `forge_halo.map` (760 MB) and
  `70_boneyard.map` (566 MB); `-mod` serving was proven. The all-maps batch was paused (see §5).
- Desktop launchers created: `Armor-FX-Test-ForgeWorld.bat` (map override, Forge only) and `Armor-FX-Mod.bat`
  (starts noble-dev with `-mod <abs data\mods\armor_fx>`; any map in the mod folder gets variants).

### Code (committed 789c950)
- `src/common/ArmorEffects.hpp`: 30 palette entries, effect-list constants (stock 7, auto index 7, variant base 8,
  `kArmorEffectVariantCount = 24`, `kArmorEffectCount = 32`), custom-id mapping and `ArmorFlamePmcgTarget` (pmcg item
  name + region/permutation; variants 0..24 on `armor_fx`, 25..29 on `armor_fx_pri`).
- `src/ui/ArmorCustomizer.cpp`: kArmorEffects 32 entries; dev flags `-armor-effect-test <0..31>`,
  `-armor-primary-test <0..29>`; profile save/load remaps.
- `src/engine/ReachHost.cpp`: computes primary colour; writes custom effect item id to +0x68/+0x90; auto clamped to 24.
- `src/engine/ReachArmory.cpp`: effect id -> pmcg names/permutations (auto clamped); Armory preview path.
- `src/hooks/EnginePatches.cpp`: permanent translator detour 0x3B2F4; maps custom ids; optional logging.
- `src/res/spartan_armor_parts.txt`: embedded pmcg data; 30 variant entries inserted in lists 80/8c (Elite list coral
  fixed to `armor_fx_pri_00`).
- `src/ui/SpartanArmoryCatalog.inc`: variant rows for the 24 supported colours (rows for the 6 missing removed).
- Armory thumbnails: `data\ui\reach\armory\armor_effects\flame_pri_<id>.png` (30 old screenshots; user handles photos;
  not part of the working-state requirement).

---

## 2. How to reproduce the current 24-colour build

1. HREK tags must be in the state in §1 (backup files: `spartans.biped.bak`, `armor_fx.effect` original, etc.).
2. Rebuild gameplay map used for testing:
   `cd HREK; .\tool.exe build-cache-file levels\multi\forge_halo\forge_halo pc english`
   (mainmenu: `levels\ui\mainmenu\mainmenu pc english`).
3. Copy rebuilt menu map to `build\bin\Release\data\menumaps\noble_menu.map` and **rerun**
   `docs/test-tools/menu_sidcsv.ps1` (writes `noble_menu.string_ids.csv`). Skipping this makes effects disappear or
   resolve to the wrong permutation.
4. Build exe: `cmake --build build --config Release --target noble-dev noble-client`.
5. For in-match: either `-map-override HREK\maps\forge_halo.map` or the VFS mod folder + `-mod <abs data\mods\armor_fx>`.
6. Test flags: `-menu-screen armory -menu-ignore-input -armory-focus "Armor Effect" -armor-effect-test <idx>
   -capture-at 24 -quit-after 30 -NoFocus -log <log>`; match: `-map forge_halo -mode multiplayer -third-person
   -capture-at 40 -quit-after 50`.
   Effect indices: 0..6 stock, 7 auto; variant i = 8+i (8 steel ... 30 brick ... 31 rose).

Helper scripts used tonight are indexed in `docs/test-tools/ARMOR-FX-PIPELINE-INDEX.md`.

---

## 3. Critical engine findings (do not re-learn the hard way)

### 3.1 Attachment gating limit - the cause of "two effects at once"
- The engine reliably permutation-gates **at most 5 armor-effect attachments total** = stock `armor_fx.effect` + **4**
  extra `armor_fx_ext*.effect` attachments. This gives 4 x 6 = 24 variant slots.
- With a **5th extra** effect tag attached (even in a 9-total-attachment biped), effects spawned regardless of the
  selected permutation: NONE/birthday showed flames and variants overlapped the regular/gold flame. Rebooting the game
  and cleaning stale processes did not change it; removing the 5th tag fixed it.
- Bisection summary:
  - 9 attachments, 4 extras -> clean (NONE no flame; steel/brick correct; auto-green correct).
  - 9 attachments, 5 extras -> leaks.
  - 10 attachments, 5 extras -> leaks.
  - 14 attachments, 5 extras -> leaks.
  - Remove stock effect attachment (no extras) -> no regular flame, but birthday/pestilence stock effects disappear.
- Consequence: cannot host 30 colours with the extra-attachment design while keeping stock effects.

### 3.2 Effect-tag limits / ManagedBlam quirks
- An effect tag has a **max of 6 `locations`**. `TagBlock.AddElement` can grow the block in memory, but
  `TagFile.Save()` **silently drops** locations beyond 6. Events beyond 6 are kept (events count can grow).
- Therefore 6 variant locations/events per tag is the hard cap; use more tags, not more locations.
- Regenerating ext effect tags writes placeholder location names (`flame_pri_t<N>_<i>`) - must run
  `rename_ext_locations.py` to set real marker names, otherwise all variant effects silently vanish.
- Cross-tag copy via clipboard fails after one paste; use `SerializeElement`/`DeserializeElement` on the block.

### 3.3 Default permutation is the stock flame
- Marker `armor_fx_00` was region permutation 0 and therefore active by default, stacking the stock legendary flame
  under every selection. Fixed by making perm 0 markerless (`armor_fx_default`) and shifting stock perms/markers to
  1..6. Keep that layout if touching `armor_fx` perms.
- `flame_pri_*` markers were created by Foundry **markers-only** export (full geometry round-trip crashes Tool) and
  merged with ManagedBlam; their transform fields were copied from the stock `armor_fx_00` marker to stay centred.

### 3.4 Menu map / string ids
- Every mainmenu cache rebuild must be followed by `tools/menu_sidcsv.ps1` + copying `noble_menu.map` to
  `build\bin\Release\data\menumaps`. Stale CSV => Armory effects blank or wrong permutation.
- Armory reads embedded pmcg data from `src/res/spartan_armor_parts.txt`; engine uses the pmcg tag. Keep both in sync,
  including Elite list 8c (coral was wrong there until fixed).

### 3.5 VFS mod maps (map-rebuild route)
- Noble VFS resolves an active mod's `<mod>\maps\<name>.map` before MCC stock maps; `-mod <folder>` activates it.
  No per-map override needed: one mod folder can hold many rebuilt maps and every match on those maps gets variants.
- Verified: Forge World and 70_boneyard via `-mod` (no `-map-override`), brick flame rendered.
- Rebuild cost: Boneyard stock 199 MB -> rebuilt **566 MB** (~2.8x); Forge 760 MB. Full 44-map set estimate:
  **~25-30 GB built** (zipped ~18-24 GB); Multiplayer-only ~11-12 GB (zipped ~8-10 GB); MP+Firefight ~16-17 GB;
  Campaign-only ~11-13 GB. Build time ~3 min/map (~2 h for all).
- Matchmaking does not use map mods: this route is custom games/Forge only, and all peers need the same mod.
- The all-maps batch was **paused after 2 maps**; script `docs/test-tools/build_all_mod_maps.ps1` can resume.
  Mod folder currently 1.3 GB (forge + boneyard).

### 3.6 Tooling gotchas learned tonight
- PowerShell redirection (`>`) writes UTF-16, which corrupts C++ sources if read/written as UTF-8. Use `git checkout`
  to restore, and use the editor/Node for source files. One such corruption broke `ArmorCustomizer.cpp` tonight.
- Stale/stuck game processes can make captures show the previous build. Always kill `noble-dev` and
  `MCC-Win64-Shipping` before a verification run and wait for exit.
- Ghidra MCP tools are declared for the session but **not currently callable through the run_code `tools` object**
  (no `mcp__ghidra__*` keys; direct names are undefined). Runtime RE needs this MCP wiring fixed/verified first.
- Regression evidence: previous try at native effect tinting (`change color`/particle color fields) left flames orange;
  palette-reference patching is the only proven colour path (requires rebuilt tags/maps).

---

## 4. Repo/code state, commits, assets
- Branch `wip-merge` == `main`; pushed commits for this work:
  `0ee0a72` (initial 30 variants), `2ac918e` (default perm/attachment fixes), `05f681e` (docs + Elite coral),
  `7f10800` (stable stock restore), `789c950` (working 24-colour set).
- Working tree is clean for tracked files; many unrelated untracked files exist in the repo root/docs - do not
  `git add .`.
- HREK assets and maps are **local only** and must not be committed (large binaries). Evidence captures under
  `docs/evidence/armor-fx-primary-2026-11-17/` are committed as PNGs.
- No release/version/cap change was made tonight.

---

## 5. Next steps (priority order)

1. **Implement runtime particle colour override (chosen path).**
   - Goal: on stock maps, recolour/tint the stock legendary/flame particle per player based on profile effect or
     primary colour; cover all 30 colours and remove the need for extra attachments/map packs.
   - Suggested attack: find the particle-system palette texture binding / particle render path (GPU particle pools
     `c_particle_emitter_gpu`, `c_particle_gpu`; palette bitmap SRV), create/select palette textures at runtime and
     choose per owning player/effect; or hook particle-system spawn and set the palette pointer/colour per instance.
   - Ghidra MCP must be usable (see §3.6). Starting docs: `RE-ARMOR-FX-COLORS-2026-11-15.md`,
     `RE-VISOR-COLOR-2026-11-15.md` (shader-parameter/tint attempts), this repo's hook framework in
     `src/hooks/EnginePatches.cpp`.
   - Runtime approach must also answer: how does another Noble client know which variant a remote player picked?
     Custom effect ids replicate in the appearance record for Noble clients; test with peers. Guests without Noble
     should fall back to a stock effect.
2. **Restore / keep the 24-colour build as fallback** until runtime works; optionally resume map batch for only the
   maps Tyler actually plays (MP first) if testing is needed sooner.
3. **Fix dropped cosmetic attachments** or document the loss: night_vision.light, bungie_helmet.cheap_light,
   flashlight_1p.light, shield_down.light were removed to make room.
4. **Photos/thumbnails**: Tyler handles the effect photo set; update `armor_effects/flame_pri_*.png` if needed.
5. **Cleanup**: remove/keep mod map folder, ext4 tag, stale scripts/logs; ensure no debug processes or extra game
   instances remain; update README/TODO when runtime path is complete.
6. **Six remaining colours** (RUST/CORAL/PEACH/GOLD/YELLOW/PALE) are hidden in the UI; runtime work should restore them
   and then re-add catalog rows/thumbnails.

---

## 6. Useful commands / paths

- Game: `build\bin\Release\noble-dev.exe` (dev), `noble-client.exe`; logs in same dir.
- Armory test:
  `noble-dev.exe -menu-screen armory -menu-ignore-input -armory-focus "Armor Effect" -armor-effect-test 30
   -capture-at 24 -quit-after 30 -NoFocus -log armory.log`
- Match with map override:
  `noble-dev.exe -map forge_halo -mode multiplayer -map-override HREK\maps\forge_halo.map
   -armor-effect-test 30 -third-person -capture-at 40 -quit-after 50 -NoFocus`
- Match via VFS mod:
  `noble-dev.exe -mod <abs path>\data\mods\armor_fx -map forge_halo -mode multiplayer ...
   -armor-effect-test 30 -third-person`
- HREK build tools: `tool.exe build-cache-file <scenario-path> pc english`; menu `... mainmenu pc english`;
  scripts: `build_map.ps1`, `build_all_mod_maps.ps1`.
- ManagedBlam wrapper: `docs/test-tools/runmb.ps1`; scripts run from
  `build\bin\Release\data\userdata\claude-tests-2026-11-11\tools`.
- Evidence: `docs/evidence/armor-fx-primary-2026-11-17/` (24-colour armory/matches, earlier 30-variant contact sheet).

---

## 7. Open questions / risks
- Exact engine reason the 5th effect attachment loses permutation gating (not fully understood; empirical limit).
- Runtime palette/texture swapping may conflict with GPU particle batching; needs proof with one colour first.
- Network replication of custom effect ids to peers; anti-cheat/matchmaking policy for cosmetic runtime patches.
- Map-mod download sizes (see §3.5) and peer mod version handshake.
- Ghidra MCP availability is a prerequisite for RE; check session tool exposure before starting.

---

## 8. Session experiment log (chronological, for future agents)

1. Extended MCC item translator hook to map custom ids; added 30 variant effect tags/markers/pmcg entries and the Armory list; initial 30-variant build rendered but colours looked mostly white due a white-hot palette core.
2. Rebuilt palettes with a stronger saturated tint; then removed the white-hot term entirely (solid tint) after Tyler reported regular/gold mixing. Solid palette formula is in pri_palettes_solid.py.
3. Tyler reported the regular flame always present and variants overlapping. Investigated default permutation, menu map/CSV staleness, extra attachment count and marker names.
   - Found stock armor_fx_00 at perm 0 -> always active; introduced markerless armor_fx_default perm 0, shifted stock perms/markers to 1..6, variants after (fix_default_perm.py).
   - Found menu map rebuild without CSV regeneration breaks effect resolution (blank/wrong previews).
   - Found regenerated ext tags had placeholder location names and silently disabled variants; renamed with rename_ext_locations.py.
4. Bisected attachment layouts (stock biped, 1/2/3/4/5 extras, 9-14 total attachments): 4 extras gate correctly, 5 extras leak regardless of total count. 24-colour working set committed as 789c950; 6 colours hidden.
5. Added a Noble VFS mod map route (mod\maps\<name>.map + -mod), verified Forge World and 70_boneyard, then started a full 44-map rebuild batch (paused after 2 maps at Tyler's request once size estimates became clear).
6. Size estimates: rebuilt map ~2.8x stock (Boneyard 199->566 MB); all maps ~25-30 GB, MP-only ~11-12 GB. Decided the map-pack route is too heavy for matchmaking distribution; chose the runtime particle-colour override instead.
7. Runtime-colour RE started only to the point of document/plan; Ghidra MCP tools were not callable from the run_code tools object, so static-analysis work is blocked until that wiring is fixed/verified. No runtime code was written.

Other failures worth remembering: Foundry full-geometry round-trip crashes Tool (markers-only export is the workaround); ManagedBlam Save drops effect locations >6; cross-tag clipboard copy fails after one paste; PowerShell > emits UTF-16 and corrupted a source file (restore with git); stale game processes can capture the previous build.


## 9. Runtime-colour milestone M1 (2026-11-17 04:03)

**PROVEN:** on a stock map with no rebuild/mod/override, the runtime palette hook recolours the stock legendary flames
(magenta proof: `docs/evidence/armor-fx-primary-2026-11-17/runtime-tint-proof.png`). Implementation is in
`src/engine/ParticleTint.{hpp,cpp}`, wired through `ReachHost::HookSwapChainPresent`; dev flag
`-armor-fx-runtime-tint r,g,b`. Key details: engine context is obtained from the presenting swapchain
(GetDevice/GetImmediateContext); PSSetShaderResources vtable index is 8; matched palette is 256x16
B8G8R8A8_UNORM_SRGB; per-device runtime tint SRV. This is global (all flame particles in the process) until M2 adds
per-player selection. Ghidra MCP headless bridge + RenderDoc capture/replay instructions are in the runtime plan doc.

**M2 status 2026-11-17 04:12:** per-player colour is not available in `ParticleRenderVS` constant buffers (only
matrices/state/sprite data); VS uses `vs_particle_state_buffer` (stride 80) and PS does `palette*v1+v2`. Next options:
patch the particle state buffer/shader for a palette index, expose owner per draw, or ship local-player-only tint first.
Details in the runtime plan doc. The current M1 hook still matches any 256x16 BGRA palette, so other particle effects may
tint as well; restrict matching to the particle shader hash before release.

**Steps 1-3 (2026-11-17 04:47):** palette matching restricted to particle slot 2; runtime selection wired to the profile
(Stock LEGENDARY item + `ParticleTint::SetVariantIndex`); Armory restored to 38 entries / 30 primary variants, six missing
colours selectable again via runtime palette; verified automatic gold/rust/pale/brick on stock Forge World. Armory preview
for the six currently shows stock flame colour, thumbnails pending Tyler. Per-player remains pending.
