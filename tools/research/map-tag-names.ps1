# List tag names in a Reach .map whose path matches a regex (reads the null-separated name buffer that starts at
# "\0globals\globals\0", the same buffer ReachHost::LoadSquadTemplateNames uses). Prints index (hex) and name.
# Usage: map-tag-names.ps1 -Map <path.map> -Pattern 'characters|squad_templates' [-CountOnly]
param([string]$Map, [string]$Pattern = '.', [switch]$CountOnly)
$bytes = [System.IO.File]::ReadAllBytes($Map)
$needle = [System.Text.Encoding]::ASCII.GetBytes("`0globals\globals`0")
$start = -1
for ($i = 0; $i -lt $bytes.Length - $needle.Length; $i++) {
    if ($bytes[$i] -ne 0 -or $bytes[$i + 1] -ne 0x67) { continue }
    $ok = $true
    for ($j = 2; $j -lt $needle.Length; $j++) { if ($bytes[$i + $j] -ne $needle[$j]) { $ok = $false; break } }
    if ($ok) { $start = $i + 1; break }
}
if ($start -lt 0) { Write-Output "name buffer not found"; exit 1 }
$idx = 0; $p = $start; $hits = 0; $total = 0
while ($p -lt $bytes.Length) {
    $e = [Array]::IndexOf($bytes, [byte]0, $p)
    if ($e -lt 0 -or $e -eq $p) { break }
    $name = [System.Text.Encoding]::ASCII.GetString($bytes, $p, $e - $p)
    if ($name -notmatch '^[\x20-\x7e]+$') { break }
    $total++
    if ($name -match $Pattern) { $hits++; if (-not $CountOnly) { Write-Output ("{0:X4} {1}" -f $idx, $name) } }
    $idx++; $p = $e + 1
}
Write-Output "$hits of $total names match '$Pattern'"
