# 2026-11-13: H2A Marine Armory preview in mainmenu.hsc (after hsc_marine.ps1). rr_elite = 3 means "H2A Marine":
# h2a_armory (same pedestal) takes the rr_r/rr_p pairs and is shown instead of chief_armory / marine_armory.
$f = 'D:\Steam\steamapps\common\HREK\data\levels\ui\mainmenu\scripts\mainmenu.hsc'
$s = [IO.File]::ReadAllText($f)
if (-not $s.Contains('marine_armory')) { throw 'run hsc_marine.ps1 first' }
if ($s.Contains('h2a_armory')) { 'already patched'; exit }
function Rep([string]$old, [string]$new) {
    $n = ([regex]::Matches($script:s, [regex]::Escape($old))).Count
    if ($n -ne 1) { throw "expected 1 match, found $n : $old" }
    $script:s = $script:s.Replace($old, $new)
}
Rep "`t(object_hide chief_armory (or (not ca) (= rr_elite 2)))" "`t(object_hide chief_armory (or (not ca) (= rr_elite 2) (= rr_elite 3)))`r`n`t(object_hide h2a_armory (or (not ca) (!= rr_elite 3)))"
$pairs = (0..15 | ForEach-Object { "`t`t`t`t`t`t`t(object_set_permutation h2a_armory rr_r$_ rr_p$_)" }) -join "`r`n"
$h2a = "`t`t`t`t`t(if (= rr_elite 3)`r`n`t`t`t`t`t`t(begin`r`n$pairs`r`n`t`t`t`t`t`t`t(object_hide chief_armory TRUE)`r`n`t`t`t`t`t`t`t(object_hide marine_armory TRUE)`r`n`t`t`t`t`t`t`t(object_hide h2a_armory FALSE)`r`n`t`t`t`t`t`t`t(print `"noble h2a pieces applied`"))`r`n"
Rep "`t`t`t`t`t`t`t(object_hide marine_armory FALSE)`r`n" "`t`t`t`t`t`t`t(object_hide marine_armory FALSE)`r`n`t`t`t`t`t`t`t(object_hide h2a_armory TRUE)`r`n"
Rep "`t`t`t`t`t`t`t(object_hide marine_armory TRUE)`r`n" "`t`t`t`t`t`t`t(object_hide marine_armory TRUE)`r`n`t`t`t`t`t`t`t(object_hide h2a_armory TRUE)`r`n"
Rep "`t`t`t`t`t(if (= rr_elite 2)`r`n" ($h2a + "`t`t`t`t`t(if (= rr_elite 2)`r`n")
Rep '(print "noble elite pieces applied"))))))' '(print "noble elite pieces applied")))))))'
[IO.File]::WriteAllText($f, $s)
'patched'
