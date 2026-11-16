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

