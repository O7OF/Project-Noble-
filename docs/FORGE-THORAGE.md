# Forge Thorage flyable vehicles

The map keeps the stock `ff_thorage_more` palette and its stock flyable entries:

| Entry | Stock tag reference |
|---|---|
| `ff_thorage_pelican_flyable` | `objects\vehicles\human\pelican\pelican` |
| `ff_thorage_phantom_flyable` | `objects\vehicles\covenant\phantom\phantom_mp` |

Instead of changing those palette references, the base tag **data** is swapped:

- `objects\vehicles\human\pelican\pelican.vehicle` now contains the O7
  `pelican_mp.vehicle` data (14 seats, `wraith_d` driver, `warthog_p` passengers).
- `objects\vehicles\covenant\phantom\phantom_mp.vehicle` is the real Phantom MP tag
  (43 seats, `wraith_d` driver, `warthog_p` passengers, turret/exit seats unchanged).

The original base Pelican tag is backed up at
`scratch\hrek_backup\pelican_base_before_swap.vehicle`.

Rebuild:

    tool.exe build-cache-file levels\multi\forge_halo\forge_halo pc

then copy `HREK\maps\forge_halo.map` to
`data\mods\noble-forge\maps\forge_halo.map`.

## Poking more tag fields

`tools\PokeTag.ps1` edits a loose HREK tag by Assembly-style field path and can rebuild/install
the map in one step:

    .\tools\PokeTag.ps1 -Tag 'objects\vehicles\human\pelican\pelican.vehicle' -Field 'unit/seats[1]/label' -Value 'warthog_p' -Build

The MCC retail install is not modified by this workflow; its `forge_halo.map` remains the original
332,328,960-byte file. Rebuilt maps live in HREK and in `data\mods`.
