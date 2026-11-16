# 2026-11-13: find the hs function table entry for a script function name in haloreach.dll (read only).
# Prints the RVA of the name string, every 8-byte pointer to it, and the 8 qwords around each hit (as RVAs).
param([string]$Name = 'object_set_permutation')
$dll = 'D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll'
$b = [IO.File]::ReadAllBytes($dll)
$pe = [BitConverter]::ToInt32($b, 0x3C)
$nsec = [BitConverter]::ToUInt16($b, $pe + 6)
$optSize = [BitConverter]::ToUInt16($b, $pe + 20)
$imageBase = [BitConverter]::ToUInt64($b, $pe + 24 + 24)
$secs = @()
for ($i = 0; $i -lt $nsec; $i++) {
    $o = $pe + 24 + $optSize + $i * 40
    $secs += [pscustomobject]@{ Name = [Text.Encoding]::ASCII.GetString($b, $o, 8).Trim([char]0); VA = [BitConverter]::ToUInt32($b, $o + 12); Size = [BitConverter]::ToUInt32($b, $o + 16); Raw = [BitConverter]::ToUInt32($b, $o + 20) }
}
function OffToRva([long]$off) { foreach ($s in $secs) { if ($off -ge $s.Raw -and $off -lt $s.Raw + $s.Size) { return $off - $s.Raw + $s.VA } }; return -1 }
function RvaToOff([long]$rva) { foreach ($s in $secs) { if ($rva -ge $s.VA -and $rva -lt $s.VA + $s.Size) { return $rva - $s.VA + $s.Raw } }; return -1 }
$t = [Text.Encoding]::ASCII.GetString($b)
$off = $t.IndexOf("$Name`0")
$rva = OffToRva $off
"imagebase 0x{0:X}  {1} string rva 0x{2:X}" -f $imageBase, $Name, $rva
$va = [uint64]$imageBase + [uint64]$rva
$pat = [BitConverter]::GetBytes($va)
for ($i = 0; $i -lt $b.Length - 8; $i += 8) {
    if ($b[$i] -eq $pat[0] -and $b[$i+1] -eq $pat[1] -and $b[$i+2] -eq $pat[2] -and $b[$i+3] -eq $pat[3] -and $b[$i+4] -eq $pat[4] -and $b[$i+5] -eq $pat[5]) {
        "pointer at rva 0x{0:X}" -f (OffToRva $i)
        for ($k = -4; $k -le 6; $k++) {
            $q = [BitConverter]::ToUInt64($b, $i + $k * 8)
            $r = if ($q -ge $imageBase -and $q -lt $imageBase + 0x10000000) { "rva 0x{0:X}" -f ($q - $imageBase) } else { "0x{0:X}" -f $q }
            "   [{0,3}] {1}" -f ($k * 8), $r
        }
    }
}
