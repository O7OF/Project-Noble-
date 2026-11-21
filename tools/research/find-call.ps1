# Finds E8 rel32 calls to a target RVA inside [Rva, Rva+Length) of a PE file (maps RVAs through the section table).
# Usage: find-call.ps1 -Dll <haloreach.dll> -Rva 0x6C2B10 -Length 0x200 -Target 0x58B5C
param([string]$Dll, [string]$Rva, [string]$Length = '0x200', [string]$Target)
$b = [IO.File]::ReadAllBytes($Dll)
$pe = [BitConverter]::ToInt32($b, 0x3C)
$nsec = [BitConverter]::ToUInt16($b, $pe + 6)
$optSize = [BitConverter]::ToUInt16($b, $pe + 20)
$sec = $pe + 24 + $optSize
function ToOff([long]$rva) {
    for ($i = 0; $i -lt $nsec; $i++) {
        $s = $sec + $i * 40
        $va = [BitConverter]::ToUInt32($b, $s + 12); $vs = [BitConverter]::ToUInt32($b, $s + 8); $raw = [BitConverter]::ToUInt32($b, $s + 20)
        if ($rva -ge $va -and $rva -lt $va + $vs) { return $rva - $va + $raw }
    }
    return -1
}
$r = [Convert]::ToInt64($Rva, 16); $len = [Convert]::ToInt64($Length, 16); $t = [Convert]::ToInt64($Target, 16)
$o = ToOff $r
"function rva 0x{0:X} -> file offset 0x{1:X}; first bytes: {2}" -f $r, $o, (($b[$o..($o + 15)] | % { '{0:X2}' -f $_ }) -join ' ')
for ($k = 0; $k -lt $len - 5; $k++) {
    if ($b[$o + $k] -ne 0xE8) { continue }
    $rel = [BitConverter]::ToInt32($b, $o + $k + 1)
    $dest = $r + $k + 5 + $rel
    if ($dest -eq $t) { "call at rva 0x{0:X} (+0x{1:X}): {2}  next: {3}" -f ($r + $k), $k, (($b[($o + $k)..($o + $k + 4)] | % { '{0:X2}' -f $_ }) -join ' '), (($b[($o + $k + 5)..($o + $k + 12)] | % { '{0:X2}' -f $_ }) -join ' ') }
}
