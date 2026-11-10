# Mod map mounting: what blocks Forge on a Firefight canvas

Findings from the 2026-11-10 session (Halo: Reach, `haloreach.dll`, image base 0x180000000).

## What already works

- VFS serves a mod's own map: `maps/<scenario>_normal.map` for the engine's `maps/<scenario>.map` request
  (`src/vfs/VFS.cpp`). Without this every mod ran on MCC's stock map.
- A mod's own `haloreach.dll` ("modded haloreach.dll (REQUIRED)") is loaded when it is the installed build
  (`src/main.cpp`), so the author's patches are present.
- `ModCatalog` parses MCC's manifest fields (`ValidMultiplayerObjectTypes`, `MapDefaultPrimaryWeaponForge`,
  the JSON folder as game category).
- `<scenario>.mapinfo` byte `+0x42` is the map's game-engine type: `0x00` multiplayer, `0x04` Firefight,
  `0x08` campaign; `+0x43` carries mode flags (`|= 0x40` = multiplayer). `NOBLE_MOD_FORCE_MP=1` patches a mod
  map's entry.
- Firefight canvases keep `kMainMenuMapId` (1035) in the launch block instead of a game-mode tag.

With those, a Forge launch of Installation 04 (cex_ff_halo) finally takes the multiplayer load path:

```
ElitePack: cache step call 1 (tags 0x7FFFC5DCDA40, cache 0x7FF3EE4BA0D0)
SharedTagCache: palette step on tag table 0x7FFFC5DCDA40: 26936 tag(s)
SharedTagCache: 153 tag group(s): ... ltom ... glum ...
EngineEvent: reason=0
```

(`ltom` = multiplayer_object_type_list, `glum` = multiplayer_globals: the map does carry the Forge data.)

## The remaining fault

```
exception 0xC0000005 (read) at haloreach+0x10DB1B
AV parameter: read address 0x748
```

Ghidra decompiles the containing function (`FUN_18010da70`) as:

```c
lVar12 = *(longlong *)((longlong)ThreadLocalStoragePointer + _tls_index * 8);
iVar7  = *(int *)(*(longlong *)(lVar12 + 0x20) + 400);
sVar1  = (short)(iVar7 / 5);
sVar6  = (short)iVar7 + sVar1 * -5;
if ((-1 < sVar6) && (sVar6 < 0x10)) {
    if (iVar7 == 0xf) uVar10 = *(uint *)(DAT_184e30388 + 0x7e8);
    else              uVar10 = *(uint *)((longlong)sVar1 * 0x34 + 0x748 + DAT_184e30388);
}
```

`DAT_184e30388` (RVA 0x4E30388) is **null**, so the read lands at 0x748. Its layout — a 0x34-byte stride,
0x748 base, index < 0x10 — is a team/score array inside the **game engine globals**. So the crash means no
multiplayer game engine instance was created for this launch: the map is still Firefight-typed where the engine
picks its game engine, even though the launch block and the map info say multiplayer.

## Next step

Find where the engine selects the game engine (scenario/map type), and either force the multiplayer selection
for a mod map whose manifest category is multiplayer, or patch the scenario's type inside the mod's `.map`
(its own `map_type` field — locating it needs the tag index, not a byte poke).
