$path = "build/bin/Release/noble-crash.dmp"
$data = [System.IO.File]::ReadAllBytes($path)

$numStreams = [System.BitConverter]::ToUInt32($data, 8)
$dirRva = [System.BitConverter]::ToUInt32($data, 12)

for ($i = 0; $i -lt $numStreams; $i++) {
    $entryOffset = $dirRva + ($i * 12)
    $type = [System.BitConverter]::ToUInt32($data, $entryOffset)
    $size = [System.BitConverter]::ToUInt32($data, $entryOffset + 4)
    $rva = [System.BitConverter]::ToUInt32($data, $entryOffset + 8)
    
    if ($type -eq 6) { # ExceptionStream
        $threadId = [System.BitConverter]::ToUInt32($data, $rva)
        $code = [System.BitConverter]::ToUInt32($data, $rva + 8)
        $addr = [System.BitConverter]::ToUInt64($data, $rva + 24)
        Write-Host ("Exception: 0x{0:X8} at 0x{1:X16}" -f $code, $addr)
        
        $ctxSize = [System.BitConverter]::ToUInt32($data, $rva + 160)
        $ctxRva  = [System.BitConverter]::ToUInt32($data, $rva + 164)
        Write-Host "Thread Context RVA: $ctxRva (size $ctxSize)"
        
        $rax = [System.BitConverter]::ToUInt64($data, $ctxRva + 0x78)
        $rcx = [System.BitConverter]::ToUInt64($data, $ctxRva + 0x80)
        $rdx = [System.BitConverter]::ToUInt64($data, $ctxRva + 0x88)
        $rbx = [System.BitConverter]::ToUInt64($data, $ctxRva + 0x90)
        $rsp = [System.BitConverter]::ToUInt64($data, $ctxRva + 0x98)
        $rbp = [System.BitConverter]::ToUInt64($data, $ctxRva + 0xA0)
        $rsi = [System.BitConverter]::ToUInt64($data, $ctxRva + 0xA8)
        $rdi = [System.BitConverter]::ToUInt64($data, $ctxRva + 0xB0)
        $r8  = [System.BitConverter]::ToUInt64($data, $ctxRva + 0xB8)
        $r9  = [System.BitConverter]::ToUInt64($data, $ctxRva + 0xC0)
        $r10 = [System.BitConverter]::ToUInt64($data, $ctxRva + 0xC8)
        $r11 = [System.BitConverter]::ToUInt64($data, $ctxRva + 0xD0)
        $rip = [System.BitConverter]::ToUInt64($data, $ctxRva + 0xF8)
        
        Write-Host ("RAX: 0x{0:X16}" -f $rax)
        Write-Host ("RCX: 0x{0:X16}" -f $rcx)
        Write-Host ("RDX: 0x{0:X16}" -f $rdx)
        Write-Host ("RBX: 0x{0:X16}" -f $rbx)
        Write-Host ("RSP: 0x{0:X16}" -f $rsp)
        Write-Host ("RBP: 0x{0:X16}" -f $rbp)
        Write-Host ("R8 : 0x{0:X16}" -f $r8)
        Write-Host ("R9 : 0x{0:X16}" -f $r9)
        Write-Host ("R10: 0x{0:X16}" -f $r10)
        Write-Host ("R11: 0x{0:X16}" -f $r11)
        Write-Host ("RIP: 0x{0:X16}" -f $rip)
    }
}
