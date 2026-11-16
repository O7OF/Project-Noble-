# 2026-11-13: Marine Armory preview in mainmenu.hsc. rr_elite = 2 means "Marine": marine_armory (same pedestal as
# chief_armory) takes the rr_r/rr_p pairs and is shown instead of chief_armory.
$f = 'D:\Steam\steamapps\common\HREK\data\levels\ui\mainmenu\scripts\mainmenu.hsc'
$s = [IO.File]::ReadAllText($f)
if ($s.Contains('marine_armory')) { 'already patched'; exit }
function Rep([string]$old, [string]$new) {
    $n = ([regex]::Matches($script:s, [regex]::Escape($old))).Count
    if ($n -ne 1) { throw "expected 1 match, found $n : $old" }
    $script:s = $script:s.Replace($old, $new)
}
Rep "`t(object_hide chief_armory (not ca))" ("`t(object_hide chief_armory (or (not ca) (= rr_elite 2)))`r`n`t(object_hide marine_armory (or (not ca) (!= rr_elite 2)))")
$pairs = (0..15 | ForEach-Object { "`t`t`t`t`t`t`t(object_set_permutation marine_armory rr_r$_ rr_p$_)" }) -join "`r`n"
$marine = "`t`t`t`t`t(if (= rr_elite 2)`r`n`t`t`t`t`t`t(begin`r`n$pairs`r`n`t`t`t`t`t`t`t(object_hide chief_armory TRUE)`r`n`t`t`t`t`t`t`t(object_hide marine_armory FALSE)`r`n`t`t`t`t`t`t`t(print `"noble marine pieces applied`"))`r`n"
Rep "`t`t`t`t`t(set rr_done rr_seq)`r`n`t`t`t`t`t(if (= rr_elite 0)`r`n`t`t`t`t`t`t(begin" ("`t`t`t`t`t(set rr_done rr_seq)`r`n" + $marine + "`t`t`t`t`t(if (= rr_elite 0)`r`n`t`t`t`t`t`t(begin`r`n`t`t`t`t`t`t`t(object_hide marine_armory TRUE)")
Rep '(print "noble elite pieces applied")))))' '(print "noble elite pieces applied"))))))'
[IO.File]::WriteAllText($f, $s)
'patched'
