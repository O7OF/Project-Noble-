$bytes = [System.IO.File]::ReadAllBytes('c:\program files (x86)\steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll')
# In PE header, find .text section offset and virtual address
$peOffset = [BitConverter]::ToUInt32($bytes, 0x3C)
$numSections = [BitConverter]::ToUInt16($bytes, $peOffset + 6)
$optHeaderSize = [BitConverter]::ToUInt16($bytes, $peOffset + 20)
$sectionHeadersOffset = $peOffset + 24 + $optHeaderSize
$textOffset = [BitConverter]::ToUInt32($bytes, $sectionHeadersOffset + 20)
$textRva = [BitConverter]::ToUInt32($bytes, $sectionHeadersOffset + 12)
$textSize = [BitConverter]::ToUInt32($bytes, $sectionHeadersOffset + 16)

Write-Output (".text offset: {0:X}, RVA: {1:X}, size: {2:X}" -f $textOffset, $textRva, $textSize)

for ($i = $textOffset; $i -lt ($textOffset + $textSize - 7); $i++) {
    if ($bytes[$i] -eq 0x8A -and $bytes[$i+2] -eq 0xA6 -and $bytes[$i+3] -eq 0x00 -and $bytes[$i+4] -eq 0x00 -and $bytes[$i+5] -eq 0x00) {
        $rva = $textRva + ($i - $textOffset)
        Write-Output ("Match 8A .. A6 at RVA: 0x{0:X}" -f $rva)
    }
}
