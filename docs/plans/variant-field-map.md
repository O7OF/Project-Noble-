# Variant field map — proven native offsets (2026-10-31)

**Status:** General Settings and Player Traits are mapped for stock encoding 0x6A. Loadouts, scoring, Megalo options and TU1 remain open.

## Method

1. `noble-client -variant-dump <file>` decodes a stock `.bin` through the engine into a **64,516-byte (0xFC04) native `c_game_variant`**.
2. The same file is patched at a known bit offset; the engine decodes the patched file (no SHA-1 recompute needed for the probe).
3. Bytes that differ between the two native blobs are the field's native offset.

Probe file: `slayer_054.bin`, encoding 0x6A. Test files stayed outside git.

**TU1 check:** `hr_4v4_team_slayer_dmr_ar_50points_tu.bin` is encoding **0x6B**; the same offsets read time 12, respawn 5/5/5, traits defaults and score 50. The mapped General/Player Traits/score fields are therefore valid on base and TU1 variants. Unmapped fields still need a TU1 probe.

## General Settings

| Native | Field | Probe |
|---:|---|---|
| `0x2BC` | misc flags | 7 -> 0 |
| `0x2BD` | rounds to win | 0 -> 2 |
| `0x2BE` | time limit (minutes) | 10 -> 15 |
| `0x2BF` | round limit | 1 -> 3 |
| `0x2C0` | sudden death | 1 -> 2 |
| `0x2C2` | grace period | 0 -> 1 |
| `0x2C4` | respawn flags | 0 -> 1 (stores 8) |
| `0x2C5` | lives per round | 0 -> 3 |
| `0x2C6` | team lives per round | 0 -> 4 |
| `0x2C7` | respawn time | 5 -> 7 |
| `0x2C8` | suicide penalty | 5 -> 6 |
| `0x2C9` | betrayal penalty | 5 -> 6 |
| `0x2CA` | respawn growth | 0 -> 2 |
| `0x2CB` | loadout camera time | 10 -> 5 |
| `0x2CC` | traits duration | 5 -> 8 |
| `0x2FC` | social flags | 28 -> 3 (native 7 -> 24) |
| `0x300` | map flags | 31 -> 1 |
| `0xF840` | score to win (uint16) | Slayer 25, Team Slayer 50, Juggernaut 150, CTF 3 |

## Player Traits (the respawn/base trait block)

| Native | Field | Probe |
|---:|---|---|
| `0x2D0` | damage resistance | 0 -> 3 |
| `0x2D1` | health multiplier | 0 -> 2 |
| `0x2D2` | health recharge rate | 0 -> 3 |
| `0x2D3` | shield multiplier | 0 -> 2 |
| `0x2D4` | shield recharge rate | 0 -> 3 |
| `0x2D5` | shield recharge delay | 0 -> 2 |
| `0x2D6` | headshot immunity | 0 -> 1 |
| `0x2D7` | vampirism | 0 -> 2 |
| `0x2D8` | assassination immunity | 0 -> 1 |
| `0x2DC` | weapon damage multiplier | 0 -> 3 |
| `0x2DD` | melee damage multiplier | 0 -> 3 |
| `0x2DE` | grenade regeneration | 0 -> 1 |
| `0x2DF` | weapon pickup | 0 -> 1 |
| `0x2E0` | grenade count | 0 -> 1 |
| `0x2E1` | infinite ammo | 0 -> 1 |
| `0x2E2` | ability drop | 0 -> 1 |
| `0x2E3` | infinite ability | 0 -> 1 |
| `0x2E4` | ability usage | 0 -> 1 |
| `0x2E5` | primary weapon | 253 -> 5 |
| `0x2E6` | secondary weapon | 253 -> 5 |
| `0x2E7` | armor ability | 253 -> 0 |
| `0x2E8` | movement speed | 0 -> 2 |
| `0x2E9` | gravity | 0 -> 2 |
| `0x2EA` | vehicle usage | 0 -> 2 |
| `0x2EB` | unknown movement byte | 0 -> 1 |
| `0x2F0` | active camo | 0 -> 2 |
| `0x2F1` | waypoint | 0 -> 1 |
| `0x2F2` | visible name | 0 -> 1 |
| `0x2F3` | aura | 0 -> 2 |
| `0x2F4` | forced color | 0 -> 3 |
| `0x2F8` | radar state | 0 -> 2 |
| `0x2F9` | radar range | 0 -> 2 |
| `0x2FA` | directional damage indicator | 0 -> 1 |

The base-trait block for the map starts at `0x309`; `0x309` is its movement speed byte. Weapon set is `0x330`/`0x331`, vehicle set `0x332`/`0x333`.

## Team vs free-for-all

Patching a team's 4-bit flags in the file changed **nothing** in the decoded native blob. The eight team entries and their flags are not preserved as editable `c_game_variant` fields.

**Consequence:** there is no reliable team/FFA bit in the decoded variant. Stock Slayer and Team Slayer share the general layout; the difference is in the Megalo game-type script. The lobby split must be an explicit host choice or a name/mode heuristic, not a variant field.

## Loadouts are not in the native struct

Patching the first palette's loadout fields in the file (enabled, primary, secondary, ability, grenades at bits
3098+) produced **no change** in the decoded native blob. Loadouts are script/file data, not editable `c_game_variant`
fields. A loadout editor must write the variant file, not the decoded struct.

## Still open

- loadout file writer (fields are known at the file level, but not in the native struct);
- team entries, score-to-win and per-mode scoring;
- Megalo options/triggers, string tables, option toggles;
- TU1 (`encodingVersion 0x6B`) differences;
- a native writer, plus validation by feeding the re-encoded variant back through `decode`.

## Safe next step

Add a read-only native field reader behind `-variant-inspect` using the offsets above, then extend the patch/decode/diff probe to loadouts and scoring. Do not expose editing until every offset has a round-trip probe and a TU1 check.
