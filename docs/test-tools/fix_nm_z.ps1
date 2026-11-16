# Rebuild the Z (blue) channel of PH normal maps that were exported with B = 0 (XY only): z = sqrt(1 - x^2 - y^2), B = z*0.5+0.5.
# Originals are copied once to HREK\noble_mods\elite_noble_backup_before_foundry\ph_orig\<name>_nm_orig.tif and always read from there.
# Usage: fix_nm_z.ps1 -Report | -Apply [-NoReimport] [-Only name,name]
param([switch]$Report, [switch]$Apply, [switch]$NoReimport, [string[]]$Only)
$H = 'D:\Steam\steamapps\common\HREK'
$D = "$H\data\objects\characters\elite_noble\bitmaps"
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry\ph_orig"
New-Item -ItemType Directory -Force $Bk | Out-Null
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class NmZ {
  public static double[] Means(string path) {
    using (var b = new Bitmap(path)) {
      int w = b.Width, h = b.Height;
      using (var c = b.Clone(new Rectangle(0,0,w,h), PixelFormat.Format32bppArgb)) {
        var r = c.LockBits(new Rectangle(0,0,w,h), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
        byte[] d = new byte[w*h*4]; Marshal.Copy(r.Scan0, d, 0, d.Length); c.UnlockBits(r);
        double R=0,G=0,B=0; int n=w*h;
        for (int i=0;i<n;i++){ R+=d[i*4+2]; G+=d[i*4+1]; B+=d[i*4]; }
        return new double[]{ R/n/255, G/n/255, B/n/255 };
      }
    }
  }
  public static double Fix(string src, string dst) {
    using (var b = new Bitmap(src)) {
      int w = b.Width, h = b.Height;
      using (var c = b.Clone(new Rectangle(0,0,w,h), PixelFormat.Format32bppArgb)) {
        var r = c.LockBits(new Rectangle(0,0,w,h), ImageLockMode.ReadWrite, PixelFormat.Format32bppArgb);
        byte[] d = new byte[w*h*4]; Marshal.Copy(r.Scan0, d, 0, d.Length);
        double sum = 0; int n = w*h;
        for (int i=0;i<n;i++) {
          double x = d[i*4+2]/255.0*2.0-1.0, y = d[i*4+1]/255.0*2.0-1.0;
          double z = Math.Sqrt(Math.Max(0.0, 1.0 - x*x - y*y));
          byte bb = (byte)Math.Min(255, Math.Round((z*0.5+0.5)*255.0));
          d[i*4] = bb; sum += bb;
        }
        Marshal.Copy(d, 0, r.Scan0, d.Length); c.UnlockBits(r);
        var codec = Array.Find(ImageCodecInfo.GetImageEncoders(), e => e.MimeType == "image/tiff");
        var ep = new EncoderParameters(1); ep.Param[0] = new EncoderParameter(System.Drawing.Imaging.Encoder.Compression, (long)EncoderValue.CompressionNone);
        c.Save(dst, codec, ep);
        return sum/n/255.0;
      }
    }
  }
}
'@
$names = 'ch_elite_armor_02_nm','ch_elite_honor_armor_01_nm','ch_helmet_councilor_01_nm','elite_ossoona_helmet_armor_normal_2k','mp_newelite_zbump'
if ($Only) { $names = $names | ? { $Only -contains $_ } }
foreach ($n in $names) {
    $cur = "$D\$n.tif"
    if (-not (Test-Path $cur)) { "{0,-40} (no tif)" -f $n; continue }
    $orig = "$Bk\${n}_nm_orig.tif"
    if (-not (Test-Path $orig)) { Copy-Item $cur $orig }
    $m = [NmZ]::Means($orig)
    "{0,-40} orig mean R {1:N2} G {2:N2} B {3:N2}" -f $n, $m[0], $m[1], $m[2]
    if ($Apply -and $m[2] -lt 0.5) {
        $nb = [NmZ]::Fix($orig, $cur)
        "   -> rebuilt Z, new B mean {0:N2}" -f $nb
        if (-not $NoReimport) {
            Push-Location $H
            $p = Start-Process -FilePath .\tool.exe -ArgumentList @('reimport-bitmaps-single', "objects\characters\elite_noble\bitmaps\$n") -NoNewWindow -PassThru -RedirectStandardOutput "$env:TEMP\ri_$n.txt" -RedirectStandardError "$env:TEMP\ri_$n.err"
            $p.WaitForExit(300000) | Out-Null
            Pop-Location
            "   tool exit {0} | {1}" -f $p.ExitCode, ((Get-Content "$env:TEMP\ri_$n.txt","$env:TEMP\ri_$n.err" | Select-String -Pattern 'error|fail|warn' | Select-Object -First 2 | ForEach-Object { $_.Line.Trim() }) -join ' / ')
        }
    }
}
