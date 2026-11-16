param([string]$Map, [string]$Triples)
# finds float32 triples (x,y,z; ';' separated, ',' inside) in a file
Add-Type -TypeDefinition @"
using System; using System.IO; using System.Collections.Generic;
public static class FF {
  public static List<string> Find(string path, float[][] pats) {
    var res = new List<string>(); byte[] all = File.ReadAllBytes(path);
    var bp = new List<byte[]>();
    foreach (var p in pats) { var b = new byte[12]; for (int i = 0; i < 3; i++) Array.Copy(BitConverter.GetBytes(p[i]), 0, b, i * 4, 4); bp.Add(b); }
    for (int k = 0; k < bp.Count; k++) {
      var b = bp[k]; int n = 0;
      for (long i = 0; i <= all.Length - 12 && n < 5; i++) {
        if (all[i] != b[0] || all[i+1] != b[1] || all[i+2] != b[2] || all[i+3] != b[3]) continue;
        bool ok = true; for (int j = 4; j < 12; j++) if (all[i+j] != b[j]) { ok = false; break; }
        if (ok) { res.Add("pat" + k + " @ " + i); n++; }
      }
    }
    return res;
  }
}
"@
$p = @(); foreach ($t in $Triples.Split(';')) { $p += ,([single[]]($t.Split(',') | ForEach-Object { [single]::Parse($_, [Globalization.CultureInfo]::InvariantCulture) })) }
[FF]::Find($Map, $p)
