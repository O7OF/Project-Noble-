# 2026-11-13: list every direct call (E8 rel32) in haloreach.dll's .text to the given RVAs. Read only.
param([string]$Targets = '0x473028,0x4730CC,0x1CBF98')
$dll = 'D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll'
$b = [IO.File]::ReadAllBytes($dll)
$pe = [BitConverter]::ToInt32($b, 0x3C)
$nsec = [BitConverter]::ToUInt16($b, $pe + 6)
$optSize = [BitConverter]::ToUInt16($b, $pe + 20)
$text = $null
for ($i = 0; $i -lt $nsec; $i++) {
    $o = $pe + 24 + $optSize + $i * 40
    $name = [Text.Encoding]::ASCII.GetString($b, $o, 8).Trim([char]0)
    if ($name -eq '.text') { $text = @{ VA = [BitConverter]::ToUInt32($b, $o + 12); Size = [BitConverter]::ToUInt32($b, $o + 16); Raw = [BitConverter]::ToUInt32($b, $o + 20) } }
}
$want = @{}
foreach ($t in $Targets.Split(',')) { $want[[Convert]::ToInt64($t.Trim(), 16)] = @() }
$end = $text.Raw + $text.Size - 5
for ($off = $text.Raw; $off -lt $end; $off++) {
    if ($b[$off] -ne 0xE8) { continue }
    $rel = [BitConverter]::ToInt32($b, $off + 1)
    $rva = $off - $text.Raw + $text.VA
    $dst = $rva + 5 + $rel
    if ($want.ContainsKey([int64]$dst)) { $want[[int64]$dst] += ('0x{0:X}' -f $rva) }
}
foreach ($k in $want.Keys) { 'target 0x{0:X}: {1} caller(s): {2}' -f $k, $want[$k].Count, ($want[$k] -join ' ') }
