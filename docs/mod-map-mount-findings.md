# Mod map mounting: why the AI Battleground maps did not run

## The failure

Installation 04 AI Battleground (`cex_ff_halo`) and Glacier AI Battleground (`ff60_icecave`) loaded their map,
built the palette (26,936 tags, `ltom` + `glum` present) and were accepted (EngineEvent reason 0), then the engine's
launch routine unwound and the client sat on the loading screen. No exception, no dump.

## Root cause

The engine's map loader `FUN_1800141c0` (RVA `0x141C0`) does, in order:

1. `FUN_180031204` (RVA `0x31204`) opens and parses the cache file. This always succeeded (returned 1).
2. It compares the requested map id/type with the *loaded map header's* own id/type:
   `headerId == arg2 || arg2 == -1` and `headerType == arg3 || arg3 == -1`.
3. Only if those match does it load; otherwise it returns 0 and the launch unwinds.

Measured with a hook on the parse step (inside the loader, before the comparison):

```
stock cex_ff_halo:  header id 0x0  type 0x2760   requested type 0x2760   -> loads
mod   cex_ff_halo:  header id 0x0  type 0x7D2    requested type 0x2760   -> rejected
```

So the mod's map declares the game type of the canvas it was built on (`0x7D2` = Firefight), while a Forge or
Firefight launch asks for `0x2760`. MCC's manifest supplies that mapping; a raw workshop map does not.

## The fix

`src/main.cpp`, `MapParseHook` + `MapLoadFullHook`:

- Hook `0x141C0` to capture the requested type (`arg3`) of the load in progress.
- Hook `0x31204` (the parse step): after it returns, resolve the loaded header record
  (`index = *(u32*)(DAT_180c1a600 + 4 + (DAT_180afbe38 & 0xffff) * 8)`, record at
  `((&DAT_184e39f20)[index >> 28] + index * 4)`), and if `headerType != requested` write the requested type into the
  header's `+0x10` field before the loader compares them.

That happens between the parse and the comparison, so the engine's own check passes and the map mounts normally.
For maps whose declared type already matches (Forge World, Boneyard, all stock maps) nothing is written.

Also required for these maps: `<scenario>.mapinfo` is patched to multiplayer for Forge launches (it previously was
skipped for Firefight canvases, and the launch was refused with "map and game incompatible").

## Verification

```
Installation 04 (Forge): session STARTED   (map declares 0x7D2, mounted as 0x2760)
Glacier        (Forge): session STARTED
Forge World    (Forge): session STARTED
Boneyard       (Forge): session STARTED
```

## Tools in the repo

- `tools/ghidra/{DumpAt,Callers,Xrefs,Vtable,FindString,DataAround}.java` — headless scripts used for every
  decompilation here (`DUMP_RVA=0x... analyzeHeadless ... -postScript DumpAt.java`).
- `tools/hdr_diff.py`, `tools/hdr_dump.py`, `tools/find_type_field.py`, `tools/compare_mod_headers.py`,
  `tools/diff_two_mods.py`, `tools/find_scenario_paths.py` — map header/asset comparisons.
- Runtime switches: `NOBLE_MOD_FORCE_MP`, `NOBLE_SCENARIO_DUMP`, `NOBLE_TAG_MISS_TRACE`, `NOBLE_MAP_READ_TRACE`,
  `NOBLE_TRACE_MAP_STATE`, `NOBLE_STALL_DUMP`, `-no-mod-dll`, `-no-elite-pack`.
