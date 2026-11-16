# 2026-11-12 (Tyler): remove the H2A pieces + H2A Ranger preset and the Honor Guard robe legs (not in elite_noble).
# Counselor becomes preset 9 (legs: Honor Guard instead of H2A), Custom 10.
$n = 0
function Patch([string]$file, [string]$a, [string]$b) {
    $s = [IO.File]::ReadAllText($file)
    $a = $a.Replace("`n", "`r`n"); $b = $b.Replace("`n", "`r`n")
    if (-not $s.Contains($a)) { $a2 = $a.Replace("`r`n", "`n"); $b2 = $b.Replace("`r`n", "`n"); if ($s.Contains($a2)) { $a = $a2; $b = $b2 } else { "NOT FOUND in $(Split-Path $file -Leaf): " + $a.Substring(0, [Math]::Min(80, $a.Length)); return } }
    [IO.File]::WriteAllText($file, $s.Replace($a, $b)); $script:n++
}
$AC = 'D:\Project-Noble-Closed-Source\src\ui\ArmorCustomizer.cpp'
$EP = 'D:\Project-Noble-Closed-Source\src\engine\ElitePack.cpp'
$RH = 'D:\Project-Noble-Closed-Source\src\engine\ReachHost.cpp'
Patch $AC "harnesses (plus the Honor Guard bonus); 9 and 10 are the two mixed presets (H2A Ranger and Counselor).
constexpr std::array<EliteSetSpec, 12> kEliteSets = {{" "harnesses (plus the Honor Guard bonus); 9 is the mixed Counselor preset. (H2A Ranger + the H2A / Honor Guard robe
// pieces were removed 2026-11-12: Noble's Elite model has no such meshes.)
constexpr std::array<EliteSetSpec, 11> kEliteSets = {{"
Patch $AC '    {"H2A RANGER", "h2a_ranger",
     "Halo 2 Anniversary Ranger harness: H2A Ranger helmet, H2A pauldrons and H2A legs with the Ranger chest and arms.",
     {"h2a_space_helmet", "h2a_l_shoulder", "h2a_r_shoulder", "space", "space", "h2a_legs"}},
' ''
Patch $AC '"Counselor harness: Council helmet, Schism pauldrons, Honor Guard chest and H2A legs with Minor arms.",
     {"council", "schism", "schism", "honor_guard", "default", "h2a_legs"}},' '"Counselor harness: Council helmet, Schism pauldrons, Honor Guard chest and legs with Minor arms.",
     {"council", "schism", "schism", "honor_guard", "default", "honor_guard"}},'
Patch $AC 'constexpr int kEliteCustomSet = 11;' 'constexpr int kEliteCustomSet = 10;'
Patch $AC '        l[kEliteHelmet].push_back({ "RANGER (H2A)", "h2a_space_helmet", kExtraNote });
' ''
Patch $AC '        l[kEliteLeftShoulder].push_back({ "H2A", "h2a_l_shoulder", kExtraNote });
' ''
Patch $AC '        l[kEliteRightShoulder].push_back({ "H2A", "h2a_r_shoulder", kExtraNote });
' ''
Patch $AC '        l[kEliteLegs].push_back({ "HONOR GUARD (ROBE)", "honor_guard_legs", kExtraNote });
        l[kEliteLegs].push_back({ "H2A", "h2a_legs", kExtraNote });
' ''
Patch $AC '    // 2026-10-30 (Tyler): the H2A Ranger set and its pieces take the General slot Honor Guard used to
    // hold; Honor Guard moves later and the Counselor preset is the last Elite set to unlock.
    { "h2a_space_helmet", "General" },   { "h2a_l_shoulder", "General" },
    { "h2a_r_shoulder", "General" },     { "h2a_legs", "General" },
    { "h2a_ranger", "General" },         { "honor_guard", "Field Marshall" },  { "honor_guard_legs", "Field Marshall" },' '    // Honor Guard unlocks late and the Counselor preset is the last Elite set to unlock (H2A removed 2026-11-12).
    { "honor_guard", "Field Marshall" },'
# ElitePack: set table + Noble set mapping follow the new numbering (9 Counselor)
Patch $EP '    {11, 11, 11, 3, 4, 9},  // H2A Ranger: h2a helmet/shoulders/legs, Ranger chest (3) and arms (4)
    {0, 5, 5, 8, 0, 9},     // Counselor: Council helmet, Schism shoulders, Honor Guard chest (8), Minor arms, H2A legs' '    {0, 5, 5, 8, 0, 1},     // Counselor (RR pack indices): Council helmet, Schism shoulders, Honor Guard chest + legs, Minor arms'
Patch $EP 'constexpr uint8_t kSetPieces[11][6] = {' 'constexpr uint8_t kSetPieces[10][6] = {'
Patch $EP '// kEliteSets order as ArmorCustomizer: 0..8 Reach''s own + Honor Guard, 9 H2A Ranger, 10 Counselor).' '// kEliteSets order as ArmorCustomizer: 0..8 Reach''s own + Honor Guard, 9 Counselor; H2A removed 2026-11-12).'
Patch $EP '    static const char* kSet[11] = {"minor", "spec_ops", "space", "ultra", "zealot", "general", "story_elite", "officer",
                                   "honor_guard", "space", "council"};
    if (set == 10) {   // Counselor: Council helmet, Schism shoulders, Honor Guard chest, Minor arms and legs
        static const char* kC[6] = {"council", "schism", "schism", "honor_guard", "default", "minor"};
        return kC[slot];
    }
    return kSet[(set >= 0 && set < 11) ? set : 0];' '    static const char* kSet[9] = {"minor", "spec_ops", "space", "ultra", "zealot", "general", "story_elite", "officer",
                                  "honor_guard"};
    if (set == 9) {   // Counselor: Council helmet, Schism shoulders, Honor Guard chest and legs, Minor arms
        static const char* kC[6] = {"council", "schism", "schism", "honor_guard", "default", "honor_guard"};
        return kC[slot];
    }
    return kSet[(set >= 0 && set < 9) ? set : 0];'
Patch $EP '    return kSetPieces[(set >= 0 && set < 11) ? set : 0][slot];' '    return kSetPieces[(set >= 0 && set < static_cast<int>(std::size(kSetPieces))) ? set : 0][slot];'
Patch $RH '        // Armory set indices: 0..7 = MCC''s eight Elite sets (194..201), 8 = Honor Guard, 9 = H2A Ranger,
        // 10 = Counselor. MCC has no set item for 8..10 - those presets are carried by their per-piece
        // entries in the appearance record (ElitePack) - so the profile set item stays at Minor (194).
        const int eliteSet = num(el, "armor_set", 0, 10, 0);' '        // Armory set indices: 0..7 = MCC''s eight Elite sets (194..201), 8 = Honor Guard, 9 = Counselor,
        // 10 = Custom. MCC has no set item for 8..10 - those presets are carried by their per-piece
        // entries in the appearance record (ElitePack) - so the profile set item stays at Minor (194).
        const int eliteSet = num(el, "armor_set", 0, 10, 0);'
"patched: $n (expected 15)"
Select-String -Path $AC, $EP -Pattern 'h2a|honor_guard_legs' | ForEach-Object { "left: {0}:{1}: {2}" -f $_.Filename, $_.LineNumber, $_.Line.Trim() }
