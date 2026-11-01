# Prophets' Hand — custom model inventory for Noble

Source: raw string scan of `maps\m10.map` (1.22 GB), 1,047 unique `objects\characters\elite` paths. This lists what the
mod actually carries; it does **not** claim every item is a standalone tag. The armour-only pack blocker is in
`prophets-hand-armory-loader-spike.md`.

## New Elite bodies (the main source of arms/attachments)

| Path | Likely role |
|---|---|
| `objects\characters\elite_ai_ultimate\elite_ai_ultimate` | new high-quality Elite body used by AI |
| `objects\characters\elite_ai_ultimate\elite_ai_ultimate_player` | playable variant |
| `objects\characters\elite_ai_ultimate\elite_ai_ultimate_silent_shadow` | Silent Shadow variant |
| `objects\characters\elite_ai\elite_ai_new` | older/new AI body |
| `objects\characters\elite\elite_ai_new` | custom body variant |
| `objects\characters\elite\elite_ai_custom - copy` | custom body variant |
| `objects\characters\elite\silent_shadow\silent_shadow` | Silent Shadow body |
| `objects\characters\elite\silent_shadow\silent_shadow_new` | Silent Shadow revision |
| `objects\characters\elite_fc\elite_fc_ai` | separate Elite body family |
| `objects\characters\elite_ai\elite_unarmed` | unarmed body/arms variant |

These are the candidates for **arms, gauntlets and full-body attachments**. If the ultimate model has separate regions
(`gauntlet`, `shoulder`, `helmet`, `chest`, `thigh`), those regions can become Armory pieces; if it is one merged mesh,
only whole-body variants can be pulled without splitting geometry.

## First-person arms

- `objects\characters\elite\fp_body\fp_body`
- `objects\characters\elite\fp\weapons\...` — per-weapon first-person arms/hands, e.g. `fp_plasma_repeater`,
  `fp_needle_rifle`, `fp_concussion_rifle`, `fp_focus_rifle`, `fp_energy_sword`, `fp_elite_zealot_blade`.

These are the cleanest "arm" pulls for first-person views, but they are rigged/animated per weapon.

## Armour piece material families

The mod has diffuse/normal/CC/emissive material sets for each piece and rank:

- pieces: `helmet`, `chest`, `gauntlet`, `shoulder`, `thigh`, `illum_cards`, `visor`
- ranks: `general`, `minor`, `officer`, `space`, `spec_ops`, `ultra`, `zealot`
- additional/new looks: `council`, `honor`, `ossoona`, `blademaster`, `silent_shadow`
- `ultimate_bitmaps`: `mp_newelite_diffuse`, `elite_cc`, `newelite_zbump`, `ch_elite_honor_armor_01`,
  `ch_helmet_councilor_01_diff/nm`, `elite ossoona helmet armor diffuse 4k`

These materials are what make the new pieces look different; geometry alone would lose the rank identity.

## What is worth pulling first

1. **Ultimate Elite gauntlets/forearms** — the "arms" attachment with the highest visual payoff.
2. **Shoulder plates** — left/right modular pieces; the current Elite pack is thin here.
3. **Council / Honor / Ossoona / Silent Shadow helmets** — strong silhouettes and clearly new.
4. **Chest and thigh plates** — completes the set.
5. **FP arms** — for first-person weapon poses, after the third-person pieces.

Weapons (60+ new/classic) and vehicles (20+) are also in the mod, but they are separate sandbox work; they do not help
the Armory attachment problem.

## What blocks pulling them today

- The assets live inside 1.0–1.4 GB scenario maps, not a menumap or tag folder.
- Noble's `ElitePack` only loads sidecar-backed armour caches (`.rrreloc`, `.rrneeds`, `.rrpmcg`); Prophets' Hand ships
  none.
- The models are credited to **Lachiebuzzx**; permission/attribution must be agreed before redistributing extracted
  geometry or textures.
- Shaders may reference Prophets' Hand globals/functions that do not exist in `rr_elite_pack.map`; a straight tag copy
  can render black or fall back to stock.

## Recommended next step

Ask O7OF/the mod author for an **armour-only pack** containing the ultimate Elite bodies, the rank material sets and the
permutation table, in the existing sidecar format. Then Noble can expose the gauntlets/shoulders/helmets without a
1.2 GB scenario cache. If that is refused, the fallback is a tag dump + geometry split, which is a separate approved
project.
