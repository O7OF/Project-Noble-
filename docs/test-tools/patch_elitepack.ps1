$f = 'D:\Project-Noble-Closed-Source\src\engine\ElitePack.cpp'
$s = [IO.File]::ReadAllText($f)
$n = 0
function Rep([string]$a, [string]$b) {
    if (-not $script:s.Contains($a)) { "NOT FOUND: " + $a.Substring(0, [Math]::Min(70, $a.Length)); return }
    $script:s = $script:s.Replace($a, $b); $script:n++
}
# first-person: Noble indices -> names
Rep "    const char* id = nullptr;`r`n    if (slot == 1 && index >= 0 && index < int(std::size(kShoulder))) id = kShoulder[index];" "    const char* id = nullptr;`r`n    if (NobleActive() && (slot == 1 || slot == 4)) {`r`n        const int j = index - s_nobleBase[slot];`r`n        if (j >= 0 && j < int(kNoblePerms[slot].size())) id = kNoblePerms[slot][j];`r`n        if (id && std::strcmp(id, `"default`") == 0) id = `"minor`";`r`n        index = -1;`r`n    }`r`n    if (slot == 1 && index >= 0 && index < int(std::size(kShoulder))) id = kShoulder[index];"
Rep "    if (slot == 4 && index >= 0 && index < int(std::size(kForearms))) id = kForearms[index];" "    if (slot == 4 && index >= 0 && index < int(std::size(kForearms)) && !id) id = kForearms[index];"
# re-read the Armory picks when the Noble indices change
Rep "    if (ec || stamp == s_picksStamp) return;`r`n    s_picksStamp = stamp;" "    static int s_seenNobleGen = -1;`r`n    if (ec || (stamp == s_picksStamp && s_seenNobleGen == s_nobleGen.load())) return;`r`n    s_picksStamp = stamp;`r`n    s_seenNobleGen = s_nobleGen.load();"
Rep "            s_localPicks[k] = static_cast<uint8_t>(idx >= 0 ? idx : kSetPieces[set][k]);" "            s_localPicks[k] = static_cast<uint8_t>(idx >= 0 ? idx : SetPick(set, k));"
Rep "                picks[k] = s_localPicks[k] != 0xFF ? s_localPicks[k] : kSetPieces[set][k];" "                picks[k] = s_localPicks[k] != 0xFF ? s_localPicks[k] : SetPick(set, k);"
Rep "            picks[k] = s_localPicks[k] != 0xFF ? s_localPicks[k] : kSetPieces[effectiveSet][k];" "            picks[k] = s_localPicks[k] != 0xFF ? s_localPicks[k] : SetPick(effectiveSet, k);"
Rep "        for (int k = 0; k < 6; ++k) picks[k] = kSetPieces[effectiveSet][k];" "        for (int k = 0; k < 6; ++k) picks[k] = SetPick(effectiveSet, k);"
# after the pack Elite is found: add our pieces (no-op for RR's pack)
Rep "    s_activeDatums[1] = model;`r`n    s_activeDatums[2] = render;" "    s_activeDatums[1] = model;`r`n    s_activeDatums[2] = render;`r`n    AddNoblePieces(tt, render, s_pmcgAddress ? TagAt(s_pmcgAddress) : nullptr);"
[IO.File]::WriteAllText($f, $s)
"replacements: $n"
