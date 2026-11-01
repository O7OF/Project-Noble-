# Variant field map — first proven offsets (2026-10-31)

**Status:** started, not complete. Core general/respawn/social fields are mapped; player traits, loadouts, scoring and team-vs-FFA are still open.

## Method

1. `noble-client -variant-dump <file>` decodes a stock `.bin` through the engine (`decode` at `haloreach+0x373A0`) into a **64,516-byte (0xFC04) native `c_game_variant`**.
2. The same stock file is parsed at the bit level; one known field is changed (for example, time limit 10 -> 15).
3. The patched file is decoded again. The engine accepts it without a recomputed SHA-1.
4. Bytes that differ between the two native blobs are the field's native offset.

Patch probes used `slayer_054.bin` (encoding 0x6A). Test files were kept outside git.

## Proven native offsets

| Native offset | Width | Field | Evidence |
|---:|---:|---|---|
| `0x2BC` | byte | misc flags | file flags 7 -> 0; native 14 -> 0 |
| `0x2BD` | byte | rounds to win | 0 -> 2 |
| `0x2BE` | byte | time limit (minutes) | 10 -> 15 |
| `0x2BF` | byte | round limit | 1 -> 3 |
| `0x2C0` | byte | sudden death time | 1 -> 2 |
| `0x2C2` | byte | grace period | 0 -> 1 |
| `0x2C4` | byte | respawn flags | 0 -> 1 (stores 8, not 1) |
| `0x2C5` | byte | lives per round | 0 -> 3 |
| `0x2C6` | byte | team lives per round | 0 -> 4 |
| `0x2C7` | byte | respawn time | 5 -> 7 |
| `0x2C8` | byte | suicide penalty | 5 -> 6 |
| `0x2C9` | byte | betrayal penalty | 5 -> 6 |
| `0x2CA` | byte | respawn growth | 0 -> 2 |
| `0x2CB` | byte | loadout camera time | 10 -> 5 |
| `0x2CC` | byte | traits duration | 5 -> 8 |
| `0x300` | byte | social bitfield | observers bit 0, team-change bits 1-2, social flags bits 3-7 |
| `0x306` | byte | map flags | `0x00` -> `0x08` for one flag; encoding is not a plain mask |

The `0x300` byte combines fields: observers `0x01`, team changes `0x06`, social flags `0xF8` (stock `0x1F` = observers off, team changes 3, flags 28). Treat it as one byte for now.

## Team vs free-for-all

Patching the first team's `flags` in the file (4 bits, `1 -> 0` or `1 -> 2`) produced **no change** in the decoded native blob. Team flags and the eight team entries are not preserved in `c_game_variant` as editable fields.

**Consequence:** there is no reliable team/FFA bit in the decoded variant to drive the lobby split. Stock Slayer (FFA) and Team Slayer both decode with the same general layout; their difference is in the Megalo game-type script. The lobby should use an explicit host choice, or a name/mode heuristic, not pretend the variant carries a team flag.

## What still needs mapping

- player traits (health/weapons/movement/appearance/sensors) — fixed 125-bit blocks in the file, stored in several native arrays;
- loadouts (6 palettes x 5 loadouts) and their native arrays;
- team entries, scores and score-to-win;
- Megalo options/triggers, string tables, option toggles;
- TU1 differences (`encodingVersion 0x6B`);
- writing back: a native writer must preserve the engine's own layout and any hash/chunk metadata.

## Safe next step

Extend the patch/decode/diff probe to the remaining fixed-size fields, then add a **read-only** native field reader behind `-variant-inspect`. Do not expose editing until each offset has a round-trip probe and a TU1 check. The lobby team split should not depend on this map.

The file-level layout (bit order, block headers, option structure) is documented in the reach-variant-editor project; use it as a reference, not as copied code.
