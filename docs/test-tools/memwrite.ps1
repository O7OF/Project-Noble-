# Replace every occurrence of a hex pattern in a process's private (type 0x20000) writable memory with another hex string.
# memwrite.ps1 -Proc noble-dev -Find 'f2f0703f8d8c0c3fd2d0d03d' -Repl '000000000000000000000000803f'  (same length required)
param([string]$Proc = 'noble-dev', [string]$Find, [string]$Repl, [int]$Max = 500, [switch]$AnyType)
Add-Type @'
using System; using System.Runtime.InteropServices;
public static class MW {
  [DllImport("kernel32.dll")] static extern IntPtr OpenProcess(int a, bool b, int pid);
  [DllImport("kernel32.dll")] static extern bool ReadProcessMemory(IntPtr h, IntPtr a, byte[] buf, IntPtr n, out IntPtr r);
  [DllImport("kernel32.dll")] static extern bool WriteProcessMemory(IntPtr h, IntPtr a, byte[] buf, IntPtr n, out IntPtr r);
  [DllImport("kernel32.dll")] static extern int VirtualQueryEx(IntPtr h, IntPtr a, out MBI m, int len);
  [StructLayout(LayoutKind.Sequential)] public struct MBI { public IntPtr Base, AllocBase; public uint AllocProt; public uint pad0; public IntPtr Size; public uint State, Prot, Type, pad1; }
  public static string Run(int pid, byte[] f, byte[] r, int max, bool any) {
    var h = OpenProcess(0x0438, false, pid); int n = 0, w = 0; long a = 0; MBI m;
    while (n < max && VirtualQueryEx(h, (IntPtr)a, out m, Marshal.SizeOf(typeof(MBI))) != 0) {
      long sz = (long)m.Size; long b = (long)m.Base;
      if (m.State == 0x1000 && (any || m.Type == 0x20000) && (m.Prot & 0x44) != 0 && (m.Prot & 0x100) == 0 && sz < (1L << 31)) {
        var buf = new byte[sz]; IntPtr rd;
        if (ReadProcessMemory(h, (IntPtr)b, buf, (IntPtr)sz, out rd)) {
          for (long i = 0; i + f.Length <= sz && n < max; i++) {
            bool ok = true; for (int k = 0; k < f.Length; k++) if (buf[i + k] != f[k]) { ok = false; break; }
            if (!ok) continue; n++; IntPtr wr;
            if (WriteProcessMemory(h, (IntPtr)(b + i), r, (IntPtr)r.Length, out wr)) w++;
          }
        }
      }
      a = b + sz; if (sz == 0) break;
    }
    return "found " + n + " written " + w;
  }
}
'@
function HexBytes($s) { [byte[]]($s -replace '\s','' -split '(..)' | ? { $_ } | % { [Convert]::ToByte($_, 16) }) }
$p = Get-Process $Proc | Select -First 1
[MW]::Run($p.Id, (HexBytes $Find), (HexBytes $Repl), $Max, [bool]$AnyType)
