# Armour-effect colour variants (#15-B) - RE findings and plan (2026-11-16)

Owner work resumed after the multiplayer handover (O7) and visor/lighting scope removal. #15 armour-effect colour
variants stay in scope; this note records what is proven and the implementation plan.

## Proven in-engine chain (Ghidra + live probe)

1. Profile effect item ids: `+0x68` Spartan armour effect, `+0x90` Elite armour effect (`ReachHost` writes them).
   MCC item ids: Spartan effects `kEffect + 0..6` = **166..172 (0xA6..0xAC)**, Elite effects `kEliteEffect + 0..6` =
   **173..180 (0xAD..0xB4)** in Armory order (0 default, 1 birthday, 2 heart, 3 pestilence, 4 inclement, 5 flames,
   6 blue/eternal flames).
2. The engine copies the profile to an appearance record, then converts the item id through the MCC item translator
   **`FUN_18003B2F4`** (`haloreach.dll+0x3B2F4`) to an internal pmcg index. Decompiled mapping:

   | item id | Armory effect | internal pmcg index |
   |---|---|---|
   | 0xA6 (166) | default | 0 (default_armor) |
   | 0xA7 (167) | birthday_party | 6 (halo_bday) |
   | 0xA8 (168) | heart_attack | 3 (heart_attack) |
   | 0xA9 (169) | pestilence | 4 (pestilence) |
   | 0xAA (170) | inclement_weather | 5 (storm) |
   | 0xAB (171) | flames | 1 (legendary_flame) |
   | 0xAC (172) | blue_flames | 2 (eternal_flame) |
   | 0xAD (173) | Elite default | 0 |
   | 0xAE..0xB3 | Elite effects 1..6 | 6,3,4,5,1,2 (same order as Spartan) |
   | 0xB4 (180) | (unused) | 0 |

3. The appearance builder **`FUN_1801055EC`** selects the pmcg effect list by species (Spartan list `globals+0x80`,
   record byte `+0x18`; Elite list `globals+0x8C`, record byte `+0x19`) and bounds-checks the index against the**
   list count**: `index >= 0 && index < *(int*)(listBase)`, then applies that entry's changes through
   `FUN_180105BDC`. **There is no hard 0..6 clamp in the builder** - index 7+ is accepted if the list count is
   extended.
4. pmcg layout (from `ElitePack`/`ReachArmory`): lists start at `+0x20`, 12 bytes apart, header `{i32 count, u32 packed
   pointer, u32}`; effect lists at `+0x80` (Spartan) and `+0x8C` (Elite). Each list entry is 16 bytes
   `{name StringId, changes {i32 count, u32 pointer, u32}}`; a change is 12 bytes `{region StringId, permutation
   StringId, flags}`. The existing Elite pack code already copies the 12-byte list header from the pack into the map
   (`ElitePack::EnsureEliteEffects`) - precedent for extending a list at runtime.

## Probe implemented (dev-only this session)

* `EnginePatches`: `-armor-fx-probe` installs a MinHook detour on the item translator (`RVA 0x3B2F4`). It logs effect
  item ids/results and maps the custom id range **0xF0..0xF7 -> internal pmcg indices 7..14**; the original translator
  is used for everything else. Installed only when the flag is present (noble-client unaffected).
* `ReachHost`: `-armor-effect-item <id>` dev override writes that item id into profile `+0x68`/`+0x90` instead of
  `kEffect`/`kEliteEffect` + index.
* Evidence run 1 (`-armor-effect-test 5 -armor-fx-probe`, forge_halo match):
  `fx-translator: effect item id 0xAB -> internal index 1` (flames -> legendary_flame), `0xAD -> 0` (Elite default).
* Evidence run 2 (`-armor-effect-test 5 -armor-effect-item 240 -armor-fx-probe`):
  `fx-translator: CUSTOM armor-fx item 0xF0 -> internal index 7`; **game session started, no crash** - the builder
  receives index 7 and (correctly) skips it because the map's pmcg effect list count is still 7.

## Why variants still do not render

The engine side is ready for indices 7+, but three asset/data pieces are missing:
1. the map's pmcg effect list (+0x80/+0x8C) has only the 7 stock entries; index 7 fails the `index < count` check;
2. the model (`spartans.biped`) `armor_fx` region has only the stock permutations - a new variant needs its own region
   permutation (e.g. `armor_fx_06`) and marker/event wiring;
3. the `armor_fx.effect` events/particles only cover the stock effects - a recoloured variant needs a new particle/effect
   asset (the palette-reference recolour proof from 2026-11-15 already shows how to make one: e.g.
   `head_fire_blue.particle`).

## Implementation plan

**Phase 1 - assets (HREK, not committed):**
1. Build the recoloured particle/effect assets (blue/green/purple flames, purple lightning; reuse the palette-reference
   recipe; keep effect event names stable).
2. Extend `spartans.biped` region `armor_fx` with the new permutations and markers that spawn the new effect events.
3. Extend `armor_fx.effect` (or a sibling effect tag) with the new events/particles.
4. Extend the pmcg `player_model_customization` effect lists +0x80/+0x8C with entries `{name, change(region armor_fx,
   permutation <variant>)}` and bump the list count; rebuild the affected maps (Forge World first, then pack maps) and
   ship them as a mod/map update.

**Phase 2 - engine:**
5. Keep the translator detour (already scaffolded) but map real custom item ids to indices 7+; verify with
   `-armor-fx-probe` that the builder applies the new entry (log item id -> index and check rendering).
6. Audit for other clamps/validation of the record effect byte (network appearance writer/reader, Elite paths) once an
   index 7+ entry actually applies.

**Phase 3 - UI/profile:**
7. Add Armory variant rows only when the asset/data capability is present (extra pmcg entries detected), save the
   variant choice in `armor_effect_id`, and map new ids to the custom item ids in `ReachHost`.
8. Decide replication expectation: profile item id is a u32 and is replicated; peers running Noble with the same assets
   resolve the same index. Vanilla clients would fall back to the default effect for the custom id.

**Phase 4 - validation:**
9. Capture Armory + match with `-armor-fx-probe`; verify `CUSTOM ... -> index 7`, effect renders in third person, no
   crash on respawn/match end; test a peer receiving the custom id.

## Risks / open questions

* Extending the pmcg list at **runtime** needs the packed pointer encoding rules (`EncodePointer` in ElitePack); the safer
  first route is rebuilding the maps/pack with extra entries in HREK.
* Asset distribution: new particles/effect events live in HREK/map tags, not in the repo; shipping this feature needs a
  map/pack update and a size decision (Forge World rebuild is ~270 s and ~796 MB).
* Replication/vanilla compatibility: custom ids are Noble-only; default-effect fallback for vanilla peers.
* The three stock lists (Spartan/Elite/armor pack) must be extended consistently or a variant exists for one species
  only.

## Files touched this session

* `src/hooks/EnginePatches.cpp` - `-armor-fx-probe` translator probe (dev-only flag).
* `src/engine/ReachHost.cpp` - `-armor-effect-item <id>` dev override.
* Evidence logs: `build/bin/Release/armor-fx-probe.log` / `armor-fx-probe2.log`.


## RESULT 2026-11-16: blue-flame variant rendering end-to-end (dev map)

**Done:** the first recoloured variant, **legendary_flame_blue** (blue flames), now renders in-game from a rebuilt
Forge World loaded with `-map-override`, using the dev probes:

1. **Effect tag** (`objects\characters\spartans\fx\armor_fx\armor_fx.effect`): added the event
   `legendary_flame_blue` (copy of `legendary_flame`) pointing at the existing recoloured particle
   `particles\head_fire_blue.particle`. ManagedBlam's `Save` drops newly added **location** elements (verified), so the
   unused location 4 (`armor_fx_03_shoulders`, no stock event uses it) was renamed **`armor_fx_06`** and the new event
   uses location 4.
2. **Render model** (`spartans.render_model`): added region `armor_fx` permutation **`armor_fx_06`** and marker group
   **`armor_fx_06`** (marker bound to permutation index 6, copied from `armor_fx_00`).
3. **pmcg** (`globals\player_model_customization...`): added selection **`legendary_flame_blue`** ->
   `armor_fx/armor_fx_06` in **both** the Spartan and Elite armour-effect lists (7 -> 8 entries).
4. **Rebuild**: `tool.exe build-cache-file levels\multi\forge_halo\forge_halo` -> 796,790,784 B in 226 s.
5. **Engine**: dev flags `-armor-fx-probe -armor-effect-item 240` put custom item id 0xF0 in the profile; the probe maps
   it to internal pmcg index 7, the builder now accepts it (list count 8) and applies `armor_fx_06`.

**Captures:**
* blue variant: `build\bin\Release\data\userdata\claude-tests-2026-11-11\armor-fx-blue-variant-2026-11-16.png`
  (blue flames around the helmet in third person; log: `CUSTOM armor-fx item 0xF0 -> internal index 7` plus
  `game session started`),
* stock regression: `...\armor-fx-stock-flames-regression-2026-11-16.png` (stock orange flames unchanged;
  translator log `0xAB -> 1`).

**Tag state:** the three tags above were edited in place in HREK with `.bak` backups; the rebuilt map is
`D:\Steam\steamapps\common\HREK\maps\forge_halo.map` (not copied into MCC or the repo). Reproducible editor:
`docs/test-tools/armor_fx_add_variant.py` (run through `runmb.ps1`), inspection scripts alongside it.

**Next:** more flame colours (green/purple/white) with the same recipe; then lightning (storm) recolours using the
plasma palette; then Armory UI rows + profile persistence/replication (custom item ids in the u32 profile fields,
vanilla fallback to default), capability gating and the map/mod distribution decision.

## 2026-11-16 late: 30-colour (visor palette) scaling attempt - status and blockers

Goal from Tyler: flame variants matching **all 30 Armory visor colours** (see `src/common/ArmoryVisor.hpp`), plus
screenshots for the Armory UI, then Armory UI wiring.

**Working:**
* The blue-flame variant (`legendary_flame_blue`, pmcg index 7) still renders from the rebuilt Forge World
  (`armor-fx-blue-restored-2026-11-16.png`).
* Recoloured palette bitmaps work: copying `fx\particles\fire\gradients\fire.bitmap` to a new bitmap tag and
  hue-shifting its `processed pixel data` (ManagedBlam `GetData`/`SetData`, 256x16 BGRA with 3 mips) produces valid
  palettes (gold gradient preview verified; pointing the known-good legacy blue particle at the gold palette renders
  gold/white flames - `armor-fx-gold-palette-legacy-particle-2026-11-16.png`).
* A generator created all 29 remaining visor colours: recoloured palettes + `head_fire_<id>.particle` copies, five
  extra `armor_fx_extN.effect` tags (6 events/locations each), a new render-model region `armor_fx_ext` with 30
  permutations + 29 marker groups, 29 pmcg entries in both effect lists (list count 7 blue + 29 = 37), and five extra
  biped effect attachments. The engine translator maps custom item ids 0xF0..0x10D (240..269) to pmcg indices 7..36
  (dev flag path).

**Blockers / findings:**
* `effect` tag **locations** block: ManagedBlam `Save` silently drops newly added location elements (tested with
  `AddElement`, `InsertElement`, `DuplicateElement`, with/without new string ids, even though `MaximumElementCount` is
  8 and the count increases in memory). Existing locations can be renamed, and events can be added. Consequence: max 6
  usable locations per effect tag; 30 colours need multiple effect tags/attachments (or a different tag-edit route).
* Extra biped effect attachments: a duplicate of the **stock** `armor_fx.effect` attachment visibly increased flame
  pixels (attachment slots beyond the original 9 are not obviously capped), so multiple effect tags may be viable.
* New render-model region `armor_fx_ext` + marker groups + pmcg mapping: selecting the midnight variant (custom id 266)
  with a **stock** particle in the matching ext event produced flames at the helmet
  (`armor-fx-ext-region-stock-particle-2026-11-16.png`), i.e. the new region/marker/pmcg path works at least for that
  event.
* Custom particle tags (`head_fire_<id>.particle`) are valid: the legacy blue event pointed at `head_fire_gold.particle`
  rendered gold flames. However, custom particles in the **ext effect tags** were not consistently visible in captures
  (gold ext attempt showed only sparse embers, magenta showed no clear magenta plume), so the remaining failure is in
  the ext effect-tag/particle combination, not the palette or pmcg/region chain.
* Diagnostic leftovers were restored for the blue variant (blue palette + legacy event particle, duplicate stock
  attachment removed) and the map rebuilt. The five ext effect tags/attachments and the 29 pmcg entries remain in HREK
  for the next debugging round (inactive unless a custom id is chosen).

**Next steps:**
1. Isolate the ext effect-tag + custom-particle failure with one controlled build (single ext attachment, one event,
   custom particle vs stock particle; no duplicate attachment) and inspect the build's particle/palette postprocessing.
   Suspects: custom particle palette parameter flags or tag inclusion/fallback in the ext tag path.
2. Once rendering, batch-capture all 30 variants (one launch per id 240..269) for Armory UI screenshots.
3. Armory UI/profile wiring: expand `kArmorEffects`/profile indices to include the 30 variants, map ids to custom item
   ids in `ReachHost`, capability-gate rows until the asset pack is present, and replicate custom ids (vanilla peers
   fall back to default).

All HREK tag edits are local to `D:\Steam\steamapps\common\HREK\tags` (map rebuilt at ~797 MB); scripts are in
`docs/test-tools/` (`armor_fx_generate_all.py`, `armor_fx_fix_tags.py`, `armor_fx_repair.py`, `restore_blue.py`, the
inspection/diagnostic scripts).

## 2026-11-16: Primary armour colours as the variant base - test result

Tyler suggested basing the flame colours on Reach's **primary armour colour** palette (`kReachColors`, 30 authentic
engine palette entries, `src/ui/ArmorCustomizer.cpp`) instead of the visor finishes. Two things were tested:

1. **Native tinting via the attachment colour channel does not work for these particles.** The biped's stock
   `armor_fx.effect` attachment has `change color` options none/primary/secondary/tertiary/quaternary. Setting it to
   `primary` and testing the same flames with primary palette index 2 (White) and 21 (Maroon) produced correctly
   coloured armour but **orange flames in both cases** - the attachment colour channel does not tint the particle
   palette. Evidence: `armor-fx-primary-white-2026-11-16.png`, `armor-fx-primary-maroon-2026-11-16.png`. Reverted the
   attachment to `change color = none` afterwards.
2. A dev flag `-armor-primary-test <0..29>` was added (`ArmorCustomizer.cpp`) so match captures can set the Reach
   primary palette index.

Conclusion / proposed design: using primary colours is still a good **colour source** for the variant palettes (30
authentic Reach colours, already exposed in the Armory), and it enables a nicer UX - the flame colour can be derived
automatically from the player's primary colour index instead of adding 30 separate effect rows. This is a data-only
swap in the generator (primary RGB list + `legendary_flame_pri_<name>` naming, pmcg entry index = 7 + primary index)
and a mapping in `ReachHost`/`ReachArmory`; it does not fix the still-open ext-effect-tag/custom-particle render
blocker, which is the next debugging step.
