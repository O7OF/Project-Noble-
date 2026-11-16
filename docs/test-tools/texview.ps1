param([string[]]$Names, [int]$Size = 768)
# Writes RGB / alpha / R / G / B previews (PNG, resized) of elite_noble bitmap source tifs into T\view\
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class TV {
  public static void Make(string src, string outBase, int sz) {
    using (var b = new Bitmap(src)) {
      int w = b.Width, h = b.Height;
      var c = b.Clone(new Rectangle(0,0,w,h), PixelFormat.Format32bppArgb);
      var r = c.LockBits(new Rectangle(0,0,w,h), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
      var d = new byte[w*h*4]; Marshal.Copy(r.Scan0, d, 0, d.Length); c.UnlockBits(r); c.Dispose();
      string[] tags = {"rgb","a","r","g","b"};
      foreach (var t in tags) {
        var o = new byte[w*h*4];
        for (int i = 0; i < w*h; i++) {
          byte B = d[i*4], G = d[i*4+1], R = d[i*4+2], A = d[i*4+3];
          byte vr, vg, vb;
          if (t=="rgb") { vr=R; vg=G; vb=B; }
          else if (t=="a") { vr=vg=vb=A; }
          else if (t=="r") { vr=vg=vb=R; }
          else if (t=="g") { vr=vg=vb=G; }
          else { vr=vg=vb=B; }
          o[i*4]=vb; o[i*4+1]=vg; o[i*4+2]=vr; o[i*4+3]=255;
        }
        using (var bm = new Bitmap(w,h,PixelFormat.Format32bppArgb)) {
          var rr = bm.LockBits(new Rectangle(0,0,w,h), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
          Marshal.Copy(o, 0, rr.Scan0, o.Length); bm.UnlockBits(rr);
          using (var sm = new Bitmap(sz, sz)) using (var g = Graphics.FromImage(sm)) {
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
            g.DrawImage(bm, 0, 0, sz, sz);
            sm.Save(outBase + "_" + t + ".png", ImageFormat.Png);
          }
        }
      }
    }
  }
}
'@
$D = 'D:\Steam\steamapps\common\HREK\data\objects\characters\elite_noble\bitmaps'
$O = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\view'
New-Item -ItemType Directory -Force $O | Out-Null
foreach ($n in $Names) { $p = Join-Path $D "$n.tif"; if (Test-Path $p) { [TV]::Make($p, (Join-Path $O $n), $Size); "ok $n" } else { "missing $p" } }
