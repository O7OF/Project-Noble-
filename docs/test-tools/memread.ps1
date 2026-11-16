# Read bytes at module+offset in a running process. memread.ps1 -Module haloreach.dll -Off 0x4E38F60 -Len 16
param([string]$Proc = 'noble-dev', [string]$Module = 'haloreach.dll', [string]$Off, [int]$Len = 16, [string]$Deref = '')
Add-Type @'
using System; using System.Runtime.InteropServices;
public static class MR {
  [DllImport("kernel32.dll")] static extern IntPtr OpenProcess(int a, bool b, int pid);
  [DllImport("kernel32.dll")] static extern bool ReadProcessMemory(IntPtr h, IntPtr a, byte[] buf, IntPtr n, out IntPtr r);
  public static byte[] Read(int pid, long addr, int len) { var h = OpenProcess(0x10, false, pid); var b = new byte[len]; IntPtr r; ReadProcessMemory(h, (IntPtr)addr, b, (IntPtr)len, out r); return b; }
}
'@
$p = Get-Process $Proc | Select -First 1
$m = $p.Modules | ? ModuleName -eq $Module | Select -First 1
$a = $m.BaseAddress.ToInt64() + [Convert]::ToInt64($Off, 16)
if ($Deref) { $a = [BitConverter]::ToInt64([MR]::Read($p.Id, $a, 8), 0) + [Convert]::ToInt64($Deref, 16); "ptr+$Deref = {0:X}" -f $a }
(([MR]::Read($p.Id, $a, $Len)) | % { '{0:x2}' -f $_ }) -join ' '
