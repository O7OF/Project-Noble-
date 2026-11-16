# Search a running process's committed memory for a hex byte pattern; prints address, module-relative hint and context.
# memscan.ps1 -Proc noble-dev -Hex '1a8cf0ff' [-Max 20] [-Ctx 24]
param([string]$Proc = 'noble-dev', [string]$Hex, [int]$Max = 20, [int]$Ctx = 24)
Add-Type @'
using System; using System.Collections.Generic; using System.Runtime.InteropServices; using System.Text;
public static class MS {
  [DllImport("kernel32.dll")] static extern IntPtr OpenProcess(int a, bool b, int pid);
  [DllImport("kernel32.dll")] static extern bool ReadProcessMemory(IntPtr h, IntPtr a, byte[] buf, IntPtr n, out IntPtr r);
  [DllImport("kernel32.dll")] static extern int VirtualQueryEx(IntPtr h, IntPtr a, out MBI m, int len);
  [StructLayout(LayoutKind.Sequential)] public struct MBI { public IntPtr Base, AllocBase; public uint AllocProt; public uint pad0; public IntPtr Size; public uint State, Prot, Type, pad1; }
  public static string Run(int pid, byte[] pat, int max, int ctx) {
    var h = OpenProcess(0x0410, false, pid); var sb = new StringBuilder(); int n = 0; long a = 0;
    MBI m;
    while (n < max && VirtualQueryEx(h, (IntPtr)a, out m, Marshal.SizeOf(typeof(MBI))) != 0) {
      long sz = (long)m.Size; long b = (long)m.Base;
      if (m.State == 0x1000 && (m.Prot & 0x101) == 0 && (m.Prot & 0xEE) != 0 && sz < (1L << 31)) {
        var buf = new byte[sz]; IntPtr r;
        if (ReadProcessMemory(h, (IntPtr)b, buf, (IntPtr)sz, out r)) {
          for (long i = 0; i + pat.Length <= sz && n < max; i++) {
            bool ok = true; for (int k = 0; k < pat.Length; k++) if (buf[i + k] != pat[k]) { ok = false; break; }
            if (!ok) continue; n++;
            sb.AppendFormat("{0:X} (region {1:X} type {2:X} prot {3:X}): ", b + i, b, m.Type, m.Prot);
            for (long k = Math.Max(0, i - ctx); k < Math.Min(sz, i + pat.Length + ctx); k++) sb.AppendFormat("{0:x2}{1}", buf[k], k == i - 1 ? "|" : " ");
            sb.AppendLine();
          }
        }
      }
      a = b + sz; if (sz == 0) break;
    }
    sb.AppendLine("hits " + n); return sb.ToString();
  }
}
'@
$p = Get-Process $Proc | Select -First 1
$bytes = [byte[]]($Hex -replace '\s','' -split '(..)' | ? { $_ } | % { [Convert]::ToByte($_, 16) })
[MS]::Run($p.Id, $bytes, $Max, $Ctx)
