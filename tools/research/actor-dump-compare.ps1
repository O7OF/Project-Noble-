# Phase 7 research: rebuilds the ADUMP actor record from a Noble log and lists every offset holding a float triple
# within 1.5 m of the body position (copies of the actor's location), with the 16 bytes that follow each one.
# Usage: actor-dump-compare.ps1 -Log host-xxx.log
param([string]$Log)
$lines = Select-String -Path $Log -Pattern 'ADUMP ([0-9A-F]{4}):((?: [0-9A-F]{2})+)'
$hdr = Select-String -Path $Log -Pattern 'actor dump slot (\d+) body ([-0-9.]+),([-0-9.]+),([-0-9.]+)' | Select-Object -First 1
$bx = [double]$hdr.Matches[0].Groups[2].Value; $by = [double]$hdr.Matches[0].Groups[3].Value; $bz = [double]$hdr.Matches[0].Groups[4].Value
$bytes = New-Object byte[] 0xD00
foreach ($l in $lines) {
    $m = $l.Matches[0]; $off = [Convert]::ToInt32($m.Groups[1].Value, 16)
    $hex = $m.Groups[2].Value.Trim().Split(' ')
    for ($k = 0; $k -lt $hex.Count; $k++) { $bytes[$off + $k] = [Convert]::ToByte($hex[$k], 16) }
}
"body {0},{1},{2}" -f $bx, $by, $bz
for ($o = 0; $o -le 0xD00 - 12; $o += 4) {
    $x = [BitConverter]::ToSingle($bytes, $o); $y = [BitConverter]::ToSingle($bytes, $o + 4); $z = [BitConverter]::ToSingle($bytes, $o + 8)
    if ([math]::Abs($x - $bx) -lt 1.5 -and [math]::Abs($y - $by) -lt 1.5 -and [math]::Abs($z - $bz) -lt 3) {
        $after = ($bytes[($o + 12)..([math]::Min(0xCFF, $o + 27))] | ForEach-Object { '{0:X2}' -f $_ }) -join ' '
        "{0:X4}: {1:N2},{2:N2},{3:N2} | next: {4}" -f $o, $x, $y, $z, $after
    }
}
