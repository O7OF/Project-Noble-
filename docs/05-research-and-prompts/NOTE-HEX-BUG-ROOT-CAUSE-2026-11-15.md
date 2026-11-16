# Hex-number UI bug ("0000001c", "Respawn in 00000001"): root cause (static analysis) - 2026-11-15

Status: static finding only. RUNTIME CHECK DID NOT CONFIRM IT (2026-11-15 06:40, see NOTE-HEX-BUG-RUNTIME-CHECK-2026-11-15.md):
in solo multiplayer and campaign the expander always saw a valid handle (not -1) and no output contained 8 hex digits;
the respawn string expanded normally to "Respawn in 0". Treat the theory below as UNPROVEN: it describes a real code path
in FUN_1802d288c, but nobody has seen it fire. The exact screen/mode/build that shows "0000001c" is still unknown.
TYLER'S CLUE (06:55): he saw the number on the ORIGINAL Reach (engine) scoreboard that draws BEHIND Noble's own scoreboard;
he does not remember the exact digits (a run of zeros then a number, maybe a letter). The runtime check above only covered
respawn/HUD text in solo MP + campaign and (as far as its note says) never held the scoreboard open, so the scoreboard
strings are still UNTESTED. Next run: same [hexbug] hook, but hold the scoreboard (Tab) during the match and ideally take a
screenshot. Also check whether Noble hides the engine scoreboard widget or only draws over it.
Supersedes the "token loop" paragraph of NOTE-MCC-HALOREACH-FULL-DECOMP (that note had the on/off paths described the
wrong way round; this one is the corrected reading).

## Root cause candidate: FUN_1802d288c (hr+0x2d288c) leaks the 8 hex digits
The function scans a localized string for `\a` (0x07). After `\a` it expects 8 hex digits (`wcstol(...,16)`) = an index
into a data table, and replaces the whole `\a<8 hex>` with the value's text (via FUN_1802d26c0).
- Success needs ALL of: exactly 8 digits parsed, `*(int*)(ctx+0x180) != -1`, parsed value != -1. Then
  `_Str = pwVar7 + 9` (digits consumed) and the value is looked up and inserted.
- On ANY failure `_Str` stays at `pwVar7 + 1`, i.e. pointing AT the digits. The loop then copies the 8 digits into the
  output as ordinary characters: the `\a` disappears and the digits remain, e.g. "0000001c" or "00000001".
That matches the symptom exactly (zero-padded 8-digit hex on scoreboard / respawn; the old TODO line "Respawn in
00000001" is the same thing: token index 1). Most likely trigger: `*(int*)(ctx+0x180) == -1` (handle not set) when the
string is expanded. Retail Xbox 360 has NO wcstol callers (only the CRT definition), so the `\a<8hex>` mechanism looks
MCC-specific (not original behaviour).

## Runtime test (replaces the earlier int32/int64 hook idea)
Detour hr+0x2D288C. Signature from the decompile:
`ulonglong F(longlong ctx, int* stack, longlong p3, int stringId, wchar_t* out, longlong cap)`
After calling the original, if `out` starts with 8 chars of [0-9a-f], log: stringId, `*(int*)(ctx+0x180)`, out[0..16],
and the return address as hr+0x.... Expect `*(int*)(ctx+0x180) == -1` on the bad lines. Then find who creates that ctx
(callers at decompile lines hr_00004.c:71443 and :71603 of the MCC export).

## Candidate fixes (after confirmation)
1. Real fix: make sure ctx+0x180 is a valid handle for the scoreboard / respawn text contexts.
2. Safety patch: in the failure path skip the 8 digits (`_Str = pwVar7 + 9` when 8 hex digits were parsed). Hides the
   digits; the value would be blank instead of shown.

## What the Xbox 360 decompile proved (retail, named from the Omaha map)
| MCC (hr+RVA) | Retail name | Retail VA |
|---|---|---|
| 0x2e9720 FUN_1802e9720 | cui_property_editor_get (14-entry binary search by type tag) | 0x82709a08 |
| 0xB4C800 handler table | g_*_property_editor array (g_bool/long/real/qword/const_wchar_ptr/string_id/...) | 0x82020018..0x82020410 |
| 0x381f10 | qword_property_to_string = _i64tow_s(val, buf, cap, 16) | 0x827c4df0 |
| 0x2f3ab8 | long_property_to_string = _itow_s(val, buf, cap, 10) | 0x82714de8 |
| 0x381eb0 | real_property_to_string (usnzprintf) | 0x827c4d58 |
Editor struct is identical in both: {type tag, can_convert_from, convert_from, to_string}. Tags: bool 0, long 1, real 2,
const_wchar_ptr 3, string_id 6, qword 8. So the editors are original behaviour and do NOT zero-pad.
Other finds: retail `c_number_formatter_component` (leading_zeroes, digit_count) formats floats as decimal via `%0*.*f`
(not the hex source); retail `parse_hud_scoreboard_string` (0x828619f8) is a stub shared with parse_build_number_string
and parse_vehicle_set_string; retail text tokens are private-use utf32 characters (0xE424 HUD quantity, 0xE425 game
engine) handled by `parse_*_string` functions.

## Where the data is
`decompile-exports\README.md` (this Compendium) lists every decompile, script and index. Raw decompile text lives on
Tyler's PC: `D:\MCC HALO REACH DECOMP\decompiled\` and `D:\ReachX360\ghidra_retail\decompiled\`.
