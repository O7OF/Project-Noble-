# 2026-11-12 (Tyler): strip ElitePack's RR-pack-only paths (RR index tables, pmcg merges, -elite-pack-list).
$f = 'D:\Project-Noble-Closed-Source\src\engine\ElitePack.cpp'
$L = [System.Collections.Generic.List[string]](Get-Content $f)
$s = $L.FindIndex([Predicate[string]]{ param($x) $x -like 'void MergeEliteSets(uint8_t* mapList, uint8_t* packList) {*' })
$e = $L.FindIndex([Predicate[string]]{ param($x) $x -like 'void MatchStockDamage(const TagTable& tt, uint32_t packElite, uint32_t stockElite);*' })
if ($s -lt 0 -or $e -lt $s) { "merge functions already removed" } else {
$new = @'
// The appearance builder exits early for Elites when the map's Elite armour-effect list (+0x8C) is empty: fill it
// from the pack's list, else from the map's Spartan effects (+0x80).
void EnsureEliteEffects(uint8_t* mapList, uint8_t* packList) {
    auto count = [](const uint8_t* at) { int32_t c = 0; uint32_t p = 0; std::memcpy(&c, at, 4); std::memcpy(&p, at + 4, 4); return (c > 0 && c < 0x400 && p) ? c : 0; };
    if (count(mapList + 0x8C)) return;
    if (packList && count(packList + 0x8C)) {
        std::memcpy(mapList + 0x8C, packList + 0x8C, 12);
        NOBLE_INFO("ElitePack: Elite armour effects (+0x8C) taken from the pack ({} effects)", count(packList + 0x8C));
    } else if (count(mapList + 0x80)) {
        std::memcpy(mapList + 0x8C, mapList + 0x80, 12);
        NOBLE_INFO("ElitePack: Elite armour effects (+0x8C) taken from the map's Spartan effects ({} effects)", count(mapList + 0x80));
    }
}

'@ -split "`r?`n"
$L.RemoveRange($s, $e - $s); $L.InsertRange($s, [string[]]$new) }
$t = ($L -join "`r`n") + "`r`n"
$n = 0
function Rp([string]$a, [string]$b) { $a = $a.Replace("`r`n", "`n").Replace("`n", "`r`n"); $b = $b.Replace("`r`n", "`n").Replace("`n", "`r`n"); if ($script:t.Contains($a)) { $script:t = $script:t.Replace($a, $b); $script:n++ } else { "NOT FOUND: " + $a.Substring(0, [Math]::Min(70, $a.Length)) } }
Rp @'
    // The map keeps its own armour list for now (merging the pack's Elite pieces in is the next step).
    // The pack replaces the armour list (pmcg) with its own. Like the engine, the map's list stays the one in use
    // (its Elite sets also dress the first-person Elite arms) and the pack's Elite piece changes are copied
    // into the map's Spartan piece lists (MergeElitePieces). "-elite-pack-list" keeps the pack's list (test).
    if (s_pmcgIndex >= 0 && tt.Address(s_pmcgIndex) != s_pmcgAddress) {
        uint8_t* packList = TagAt(tt.Address(s_pmcgIndex));
        uint8_t* mapList = TagAt(s_pmcgAddress);
        if (s_keepPackList) {
            MergeEliteSets(mapList, packList);
            NOBLE_INFO("ElitePack: keeping the pack's armour list (-elite-pack-list)");
        } else {
            tt.SetAddress(s_pmcgIndex, s_pmcgAddress);
            MergeElitePieces(mapList, packList);
        }
    }
'@ @'
    // The map's armour list (pmcg) stays the one in use (its Elite sets also dress the first-person arms); if the pack
    // replaced it, put the map's back. Noble's Elite pieces are added to it later (AddNoblePieces).
    if (s_pmcgIndex >= 0) {
        uint8_t* packList = tt.Address(s_pmcgIndex) != s_pmcgAddress ? TagAt(tt.Address(s_pmcgIndex)) : nullptr;
        if (packList) tt.SetAddress(s_pmcgIndex, s_pmcgAddress);
        if (s_pmcgAddress) EnsureEliteEffects(TagAt(s_pmcgAddress), packList);
    }
'@
Rp "bool s_keepPackList = false;`n" ""
Rp "    s_keepPackList = HasArg(L`"-elite-pack-list`");`n" ""
Rp @'
// Set -> pieces table (helmet, left, right, chest, forearms, legs), sets in Armory order (the same
// kEliteSets order as ArmorCustomizer: 0..8 Reach's own + Honor Guard, 9 Counselor; H2A removed 2026-11-12).
constexpr uint8_t kSetPieces[10][6] = {
    {3, 3, 3, 0, 0, 2},   {7, 7, 7, 4, 5, 5},   {6, 6, 6, 3, 4, 4},   {9, 9, 9, 5, 7, 7},
    {10, 10, 10, 7, 8, 8}, {1, 1, 1, 1, 1, 0},  {8, 8, 8, 6, 6, 6},   {4, 4, 4, 2, 3, 3},   {2, 2, 2, 8, 2, 10},
    {0, 5, 5, 8, 0, 1},     // Counselor (RR pack indices): Council helmet, Schism shoulders, Honor Guard chest + legs, Minor arms
};
'@ @'
// Armory presets, in ArmorCustomizer's kEliteSets order: 0..8 Reach's own + Honor Guard, 9 Counselor (10 = Custom).
constexpr int kArmorySets = 10;
'@
Rp "    return kSetPieces[(set >= 0 && set < static_cast<int>(std::size(kSetPieces))) ? set : 0][slot];" "    return 0;   // not Noble's pack: no Elite pieces to pick"
Rp "// Armory set (kSetPieces order) -> piece name per slot, for the Noble pack. H2A pieces aren't in elite_noble yet." "// Armory set (kEliteSets order) -> piece name per slot, for the Noble pack."
$a = $t.IndexOf('// Armory piece ids -> the pack''s index'); $b = $t.IndexOf('std::filesystem::path s_appearancePath;')
if ($a -lt 0 -or $b -lt $a) { "PieceIndex range not found" } else {
$t = $t.Substring(0, $a) + @'
// Armory piece ids -> index in the map's armour list (Noble's pieces, see AddNoblePieces), or -1.
int PieceIndex(int slot, const std::string& id) {
    return NobleActive() ? NoblePick(slot, id) : -1;
}

// Elite set (0..7, pmcg list +0x74 order) whose first-person version matches a picked piece, or -1 when the
// piece has none (Honor Guard, Council, Ossoona, Schism, Combat). slot: 1 left shoulder, 4 forearms.
int SetOfPiece(int slot, int index) {
    static const char* kSetIds[8] = {"minor", "spec_ops", "space", "ultra", "zealot", "general", "story_elite", "officer"};
    if (!NobleActive() || (slot != 1 && slot != 4)) return -1;
    const int j = index - s_nobleBase[slot];
    if (j < 0 || j >= int(kNoblePerms[slot].size())) return -1;
    const char* id = kNoblePerms[slot][j];
    if (std::strcmp(id, "default") == 0) id = "minor";
    for (int k = 0; k < 8; ++k) if (std::strcmp(kSetIds[k], id) == 0) return k;
    return -1;
}

'@.Replace("`r`n", "`n").Replace("`n", "`r`n") + $t.Substring($b); $n++ }
$t = $t.Replace('static_cast<int>(std::size(kSetPieces))', 'kArmorySets')
[IO.File]::WriteAllText($f, $t)
"replaced $n (expected 7); merge functions removed (lines $s..$e)"
Select-String -Path $f -Pattern 'kSetPieces|MergeElite|s_keepPackList|h2a|honor_guard_legs' | ForEach-Object { "left: {0}: {1}" -f $_.LineNumber, $_.Line.Trim() }
