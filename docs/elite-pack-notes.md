# Elite armour pack in matches: how Reach Renaissance does it (Stage A research, 2026-10-28)

Source: `D:\Reach\bin\host-probe.exe` (Rust symbols, `llvm-objdump -d -C`), RR's sidecar files in
`D:\Reach\data\armor\`, and Tyler's RR match log (`runs\20261027-212459\match\host_probe_1793150738.log`).
All `hr+` addresses are RVAs in the supported `haloreach.dll` build. Nothing here is committed; RR files stay local.

## What RR passes to its match process

`--extra-maps D:\Reach\data\armor --armor-pack rr_elite_pack --elite-armor-slots` and
`--armory-appearance armory-appearance.txt`, which holds `look.elite_pieces=3,3,3,0,0,2`
(helmet, left shoulder, right shoulder, chest, forearms, legs; permutation indices in the pack's Elite).

## Files in `D:\Reach\data\armor\`

| File | Format |
|---|---|
| `rr_elite_pack.map` (95.6 MB) | a trimmed Reach MCC cache file holding the player Elite (biped + model + shaders + bitmaps) |
| `.rrneeds` | text, `RRNEEDS 1`, then `root pmcg globals\player_model_customization` and 149 `stock <group> <path>` lines: tags the pack expects the match map to already have (RR checks: "50_panopticon has all 149 tags the trimmed pack needs") |
| `.rrreloc` | binary `RRRL`, u32 version 1, u32 count 121513, then base/section fields, then the offsets of the packed pointers to rebase when the pack is loaded at a new address |
| `.rrpmcg` | text dump of the pack's player_model_customization: `L <offset> <count>` lists, `S <name>` selections, `C <region> <permutation> <n>` changes (Spartan lists, Elite 0x74 armour sets, 0x80/0x8C armour effects) |

## Hooks `armorpack::apply` installs (`haloreach.dll`)

| Kind | Site | Original | RR function |
|---|---|---|---|
| retarget call | `hr+0xA38A3` | `hr+0x141C0` | `armorpack::load` (map load: loads the pack with the match map) |
| retarget call | `hr+0x31395` | `hr+0x31078` | `armorpack::extra` (extra cache file) |
| retarget call | `hr+0x3212F` | `hr+0x11978` | `armorpack::format` |
| inline hook | `hr+0x30698` | - | `armorpack::callback` ("extra-cache callback returned 1") |
| inline hook | `hr+0x8EAF0` | - | `armorpack::rebuild` (keeps the player Elite's tags active through zone-set rebuilds) |
| `resadapt::install` | 14 functions `hr+0x1BB6C0..0x1BBD34` | - | tag-block reads by u16 index |
| code patch | `hr+0x30859` | checks words 0xD28B / 0xD289 | `win::patch_code` |
| optional (`--armor-pack-probe`) | call `hr+0x23D742` -> `hr+0x13510` | | `armorpack::claim` |
| optional (`--armor-pack-pages`) | hook `hr+0x1BC244` | | `armorpack::page` |

Log of a working load (Panopticon): slot 48 resolves `haloreach\maps\rr_elite_pack.map` to the extra-maps folder,
"header slot ... ready", "relocated 121513 packed pointers ... from 0x1cc700000 to 0x7ff472170000", "extra-cache
callback returned 1", "player representation 1 -> the pack's player Elite biped 0x855649a9 (AI Elites keep the map's
stock Elite 0x8b1f2328)", "map load returned 1". The pmcg merge puts the pack's Elite pieces into the map's armour list.

## Per-piece Elite (`elitepicks`)

Four calls to `hr+0x1055EC` (builds a unit's model selection from the player's appearance) are retargeted to
`elitepicks::build` (skipped in the menu process, `--menu-scene`):

| Call site | When |
|---|---|
| `hr+0x4AC36` | spawn |
| `hr+0x447B0` | live change |
| `hr+0x3B184F` | another machine's player |
| `hr+0x524CD5` | copy of a unit |

Only when the pack is active: it reads the player's six piece indices (`looks::elite_pieces`, from the appearance
file / per user) or, without them, derives them from the Elite armour set (appearance byte +0x17, 0..7), then calls
the original. Set -> pieces table (helmet, L, R, chest, forearms, legs):

| Set | Pieces |
|---|---|
| 0 Minor | 3 3 3 0 0 2 |
| 1 Spec Ops | 7 7 7 4 5 5 |
| 2 Space | 6 6 6 3 4 4 |
| 3 Ultra | 9 9 9 5 7 7 |
| 4 Zealot | 10 10 10 7 8 8 |
| 5 General | 1 1 1 1 1 0 |
| 6 Field Marshal | 8 8 8 6 6 6 |
| 7 Officer | 4 4 4 2 3 3 |

**Inference (not verified):** these indices match the menu map's Elite permutation lists with the H2A entries
removed (helmet: council, general, honor_guard, minor, officer, ossoona, space, spec_ops, story_elite, ultra, zealot;
shoulders: combat, general, honor_guard, minor, officer, schism, ...; forearms exactly the menu list). So RR's pack
Elite likely also carries Council / Honor Guard / Ossoona / Schism / Combat, just not the H2A pieces.

## Plan for Noble (Stage A)

1. Add an `ElitePack` module (new files, so ReachHost changes stay small): read the three sidecars, answer slot 48
   for the pack path, install the hooks above, relocate, merge pmcg, switch player representation 1.
2. Retarget the four `hr+0x1055EC` calls and feed the Armory's six Elite pieces (mapped to the pack's indices).
3. Test with RR's pack on Sword Base: mixed set pieces first, then Honor Guard etc. if the inference holds.
Stage B (Noble's own pack with the H2A pieces) needs a tool that writes a trimmed cache file + reloc table.
