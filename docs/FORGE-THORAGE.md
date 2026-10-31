# Forge Thorage flyable vehicles

The Forge World mod map keeps the native Thorage palette slot (`ff_thorage_more`) and points its
two entries at the real flyable tags:

| Entry | Tag |
|---|---|
| Pelican | `o7of_tags\vehicles\pelican_o7\pelican_mp` |
| Phantom | `objects\vehicles\covenant\phantom\phantom_mp` |

Seat labels applied to both tags:

- driver seat `unit/seats[0]/label` = `wraith_d`
- Pelican passenger seats `1-10` = `warthog_p`
- Phantom passenger seats `1-3` and `8-41` = `warthog_p`
- Phantom turret seats `4-7` and the door/exit seat `42` are unchanged

The map is built from the HREK tag tree with:

    tool.exe build-cache-file levels\multi\forge_halo\forge_halo pc

then copied to `data\mods\noble-forge\maps\forge_halo.map`.

## Poking more tag fields

`tools\PokeTag.ps1` edits a loose HREK tag by Assembly-style field path and can rebuild/install the
map in one step:

    .\tools\PokeTag.ps1 -Tag 'o7of_tags\vehicles\pelican_o7\pelican_mp.vehicle' -Field 'unit/seats[1]/label' -Value 'warthog_p' -Build

The MCC retail install is not modified by this workflow; its `forge_halo.map` remains the original
332,328,960-byte file. Rebuilt maps live in HREK and in `data\mods`.
