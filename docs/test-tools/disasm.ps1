param([string]$Start = '30698', [string]$Stop = '30880')
$Dll = 'D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\haloreach.dll'
$Db = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe'
$base = (& $Db /headers $Dll | Select-String 'image base' | Select-Object -First 1).Line
"base: $base"
$ib = [Convert]::ToUInt64((($base.Trim() -split '\s+')[0]), 16)
$s = '{0:X}' -f ($ib + [Convert]::ToUInt64($Start, 16))
$e = '{0:X}' -f ($ib + [Convert]::ToUInt64($Stop, 16))
& $Db /nologo /disasm:nobytes "/range:0x$s,0x$e" $Dll | Select-Object -Skip 5
