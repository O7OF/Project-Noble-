param([string[]]$Files)
# Mean alpha / RGB of images via GDI+ (handles Reach's .tiff that Blender hangs on).
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class AS {
  public static string Stats(string path) {
    using (var src = new Bitmap(path)) {
      var r = new Rectangle(0, 0, src.Width, src.Height);
      var d = src.LockBits(r, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
      int n = d.Stride * d.Height; var buf = new byte[n]; Marshal.Copy(d.Scan0, buf, 0, n); src.UnlockBits(d);
      double a = 0, rgb = 0, R = 0, G = 0, B = 0; long cnt = 0; int lowA = 0;
      for (int i = 0; i + 3 < n; i += 4) { a += buf[i+3]; R += buf[i+2]; G += buf[i+1]; B += buf[i]; rgb += (buf[i] + buf[i+1] + buf[i+2]) / 3.0; cnt++; if (buf[i+3] < 64) lowA++; }
      return String.Format("{0}x{1} A {3:F2} (<0.25: {4:P0}) R {6:F2} G {7:F2} B {8:F2}", src.Width, src.Height, src.PixelFormat, a / cnt / 255, (double)lowA / cnt, rgb / cnt / 255, R / cnt / 255, G / cnt / 255, B / cnt / 255);
    }
  }
}
'@
foreach ($f in ($Files -join ',' -split ',')) { try { "{0,-52} {1}" -f (Split-Path $f -Leaf), [AS]::Stats($f) } catch { "{0}: {1}" -f (Split-Path $f -Leaf), $_.Exception.Message } }
