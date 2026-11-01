# Prophets' Hand — Armory loader feasibility spike (2026-10-31)

**Question:** can the Prophets' Hand extra Elite armour pieces and lighting be made selectable in Noble's Armory without copying assets into the repo?

**Answer: no-go for a quick runtime loader.** The gameplay map path works; the Armory needs an armour-only cache with relocation metadata that the mod does not ship. This is an asset-source/porting decision, not a missing hook.

## What already works

- `-map-override <Prophets' Hand map>` serves the mod's campaign/Firefight map through the VFS. The map's lighting, shaders, Elite render models and rank permutations load with it.
- `ff45_corvette` + `TPH_ff.bin` reaches `game_session started`; the player is an Elite with the mod's HUD/arms.
- No assets are copied into the repo.

## Why the Armory does not see them

Noble's Armory renders `data/menumaps/mainmenu_all.map` (stock HREK) plus one optional armour cache:

| File | Size | Purpose |
|---|---:|---|
| `data/armor/rr_elite_pack.map` | 95.6 MB | extra armour cache |
| `rr_elite_pack.map.rrreloc` | 486 KB | 64-bit relocation list for packed tag pointers |
| `rr_elite_pack.map.rrneeds` | 10 KB | tag references the pack replaces |
| `rr_elite_pack.map.rrpmcg` | 11 KB | permutation/region table used by the Armory UI |
| `.../maps/m10.map` (Prophets' Hand) | 1.22 GB | full scenario cache, no sidecars |

`ElitePack::Install` only loads a cache next to a matching `.rrreloc` sidecar; the loader uses that list to rebase packed pointers and then swaps the tags named in `.rrneeds`. A full campaign/Firefight map is not an armour pack:

- it has no `.rrreloc` / `.rrneeds` / `.rrpmcg` sidecars;
- it is a complete scenario, not a tag subset, and loading it as a second cache next to `mainmenu_all.map` would collide with the menu's globals and one-of-a-kind tags;
- generating the relocation list for 1.22 GB and then filtering the armour tags is effectively a cache-extraction/port project, not a loader flag.

The mod also ships no menumap or standalone tag tree (`ModInfo.json` declares campaign/Firefight maps only; the `haloreach` folder contains audio and a game variant, not armour tags).

## Options

1. **Armour-only pack from the mod author (recommended).** Ask O7OF / the Prophets' Hand author for a small extra cache containing their Elite `render_model`/shader tags with `.rrreloc`, `.rrneeds` and `.rrpmcg`. Noble already knows how to consume that format. No assets are committed to the repo, and the mod's licence/attribution stays with its pack.
2. **Port/extract the tags ourselves.** Locate the Elite render model, permutation table, shaders and bitmaps inside `m10.map`, build a small cache plus the three sidecars, and map the pieces into the Armory UI. This is a multi-day reverse-engineering/asset-pipeline task; the licence and redistribution question needs O7OF/Tyler's approval.
3. **Gameplay only.** Keep using `-map-override` and expose the mod maps through the normal picker. This gives the pieces and lighting in campaign/Firefight but never in the Armory.
4. **Armory lighting separately.** The Armory chamber uses stock `mainmenu_all.map` lighting. Even with option 1, the mod's gameplay lighting does not transfer; armory key/rim light presets are a separate feature.

## Recommendation

- Ship option 3 now: the map override and normal picker path are the working route for the mod's lighting and Elite pieces in gameplay.
- Ask for option 1 before attempting option 2. A ~100 MB armour-only pack is a small ask compared with extracting from a 1.22 GB map.
- If the answer is "no pack", treat option 2 as a separate approved project, not a quick fix.

**Decision needed:** O7OF/Tyler — request an armour-only pack from the mod author, or approve the extraction/port work and its redistribution terms.
