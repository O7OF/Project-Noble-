$path = "noble-crash.dmp"
$data = [System.IO.File]::ReadAllBytes($path)

$numStreams = [System.BitConverter]::ToUInt32($data, 8)
$dirRva = [System.BitConverter]::ToUInt32($data, 12)

$modules = @()
for ($i = 0; $i -lt $numStreams; $i++) {
    $entryOffset = $dirRva + ($i * 12)
    $type = [System.BitConverter]::ToUInt32($data, $entryOffset)
    $rva = [System.BitConverter]::ToUInt32($data, $entryOffset + 8)
    
    if ($type -eq 4) { # ModuleListStream
        $numModules = [System.BitConverter]::ToUInt32($data, $rva)
        for ($m = 0; $m -lt $numModules; $m++) {
            $modOffset = $rva + 4 + ($m * 108)
            $baseOfMod = [System.BitConverter]::ToUInt64($data, $modOffset)
            $sizeOfMod = [System.BitConverter]::ToUInt32($data, $modOffset + 8)
            $nameRva = [System.BitConverter]::ToUInt32($data, $modOffset + 20)
            $nameLen = [System.BitConverter]::ToUInt32($data, $nameRva)
            $name = [System.Text.Encoding]::Unicode.GetString($data, $nameRva + 4, $nameLen)
            $modules += [PSCustomObject]@{
                Base = $baseOfMod
                Size = $sizeOfMod
                Name = [System.IO.Path]::GetFileName($name)
            }
        }
    }
}

function Resolve-Address($addr) {
    foreach ($mod in $modules) {
        if ($addr -ge $mod.Base -and $addr -lt ($mod.Base + $mod.Size)) {
            $offset = $addr - $mod.Base
            return ("{0}+0x{1:X}" -f $mod.Name, $offset)
        }
    }
    return ("0x{0:X16}" -f $addr)
}

# Inspect all threads
for ($i = 0; $i -lt $numStreams; $i++) {
    $entryOffset = $dirRva + ($i * 12)
    $type = [System.BitConverter]::ToUInt32($data, $entryOffset)
    $rva = [System.BitConverter]::ToUInt32($data, $entryOffset + 8)
    
    if ($type -eq 3) { # ThreadListStream
        $numThreads = [System.BitConverter]::ToUInt32($data, $rva)
        for ($t = 0; $t -lt [Math]::Min(5, $numThreads); $t++) {
            $tOffset = $rva + 4 + ($t * 48)
            $tId        = [System.BitConverter]::ToUInt32($data, $tOffset)
            $stackStart = [System.BitConverter]::ToUInt64($data, $tOffset + 24)
            $stackSize  = [System.BitConverter]::ToUInt32($data, $tOffset + 32)
            $stackRva   = [System.BitConverter]::ToUInt32($data, $tOffset + 36)
            $cRva       = [System.BitConverter]::ToUInt32($data, $tOffset + 44)
            
            $rip = [System.BitConverter]::ToUInt64($data, $cRva + 0xf8)
            $rsp = [System.BitConverter]::ToUInt64($data, $cRva + 0x98)
            
            Write-Host ("Thread {0} (TID {1}): RIP = {2}" -f $t, $tId, (Resolve-Address $rip))
            
            # Print top 5 frames from stack
            $offsetInStack = [Math]::Max(0, [int]($rsp - $stackStart))
            $found = 0
            for ($p = $offsetInStack; $p -lt ($stackSize - 8) -and $found -lt 20; $p += 8) {
                $val = [System.BitConverter]::ToUInt64($data, $stackRva + $p)
                $res = Resolve-Address $val
                if ($res -notmatch "^0x" -and $res -notmatch "ntdll" -and $res -notmatch "KERNELBASE") {
                    Write-Host ("    Frame: {0}" -f $res)
                    $found++
                }
            }
        }
    }
}
