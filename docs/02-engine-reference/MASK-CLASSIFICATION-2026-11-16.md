# Mask-site classification pass 1 (2026-11-16)

Hand classification of the mask-immediate candidates from [`MASK-SITE-INVENTORY.md`](MASK-SITE-INVENTORY.md), done by
decompiling the containing functions in the Ghidra project (`haloreach-copy.dll`, base `0x180000000`). This is the
bridge between the raw scan and the multi-word mask widening described in `TOPIC-scaling-64-128.md` section 3.1.

## Method

1. `python tools/limits/scan_mask_sites.py` produced 25 candidates inside the known session/player code regions.
2. `get_function_by_address` mapped each candidate to its function; `decompile_function_by_address` was then read for
   the mask storage it touches (player-valid mask `block+0x170C`, peer-valid mask `session+0x5C`, membership fields).
3. Each candidate was sorted into **mask site** (must be widened), **sentinel/invalid constant** (do not patch), or
   **needs confirmation** (function read, site not yet pinpointed).

## Findings

| function | candidate sites | what it does | verdict |
|---|---|---|---|
| `FUN_180408ddc` | 0x408F48, 0x408F84, 0x409084, 0x409293, 0x4092F9 | scans the player-valid mask `*(uint *)(block + 0x170C) >> i & 1` | **mask function to widen**; the `0xFFFF` immediates inside it are sentinels (`uVar28 = 0xffff`), so the listed sites are most likely sentinel initialisers - confirm each before patching |
| `FUN_180414c28` | 0x414CF1, 0x414D08, 0x414DB2 | scans the peer-valid mask `*(uint *)(session + 0x5C) >> i & 1` | **mask function to widen**; the three `test r16,r16` candidates test only the low 16 peer bits - likely real widen sites |
| `FUN_18044ce18` | 0x44CE42 | builds a mask from a bit count: `0xffffffff >> (0x20 - count)` | not a literal mask site - no patch |
| `FUN_18044f650` | 0x44F666 | 740-byte helper, no mask reads found | `0xFFFF` is a sentinel/invalid value, not a mask |
| `FUN_180450ed4` | 0x450F1A | compares session fields against `0xffff` (`+0x75C`, `+0x740`) | sentinel/invalid comparison, not a mask |
| `FUN_1804086a4` | 0x405D16, 0x4085DF, 0x4086F0, 0x4087E2, 0x408885, 0x408B6C, 0x408CAB | large packet/table builder (9.9 KB decomp) with `0xFFFF` field initialisers and `0x40` flags | field values / sentinels, not peer/player valid masks |
| `FUN_18038def8`, `FUN_18038ee28`, `FUN_180390db0`, `FUN_1803919e4` | 0x38DF9C, 0x38DFA1, 0x38EF59, 0x391103, 0x3913E7, 0x391B4A | join/peer-state and player-name-table code | `test16` on 16-bit counters and `0xFFFF`/`0x1FFFF` constants; `0x391B4A` is a known peer-mask widening (already handled in the bounds tables) - the rest need per-site confirmation |
| `FUN_1803c6e7c` | 0x3C6F2F | game-player-event area | `0xFFFFFFFF` constant likely a sentinel; confirm when widening the event index |

## Confirmed mask storage to widen

| mask | engine location | width today | consumers |
|---|---|---|---|
| player-valid mask | player block `+0x170C` | DWORD (32 players) | `FUN_180408ddc` and other player loops |
| peer-valid mask | session `+0x5C` (and `+0x40` peer-state copies) | DWORD (32 peers) | `FUN_180414c28`, join/peer-state code |
| player mask in player-add path | passed in registers / block copies | DWORD | `0x39D620` family (already partly caved for 32) |

## Next steps

1. Confirm the candidate sites inside `FUN_180408ddc` (sentinel vs mask) and widen the real ones together with the
   `+0x170C` mask storage.
2. Widen the peer-mask scans in `FUN_180414c28` and the join/peer-state code (`+0x5C`), using the shape from
   `TOPIC-scaling-64-128.md` section 3.1 (low-word path + high-word branch).
3. Re-run the scanner after the patch set exists and keep the classified table current; the raw scan is not a patch
   list on its own.
