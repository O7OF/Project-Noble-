# Cut an Armory character out of the blue-grid backdrop (2026-11-13). Background = pixels reached from the image border
# through "bluish or near-black" pixels (flood fill). Output: tight crop, padded, resized into WxH (default 356x456 = stock
# elite_sets pictures), transparent background.
param([string]$In, [string]$Out, [int]$W = 356, [int]$H = 456, [double]$Blue = 1.12, [int]$Dark = 7, [double]$Top = 1.0)
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Collections.Generic;
public static class Cut {
    public static Bitmap Run(Bitmap src, double blueRatio, int dark) {
        int w = src.Width, h = src.Height;
        var d = src.LockBits(new Rectangle(0,0,w,h), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
        var px = new byte[w*h*4]; System.Runtime.InteropServices.Marshal.Copy(d.Scan0, px, 0, px.Length); src.UnlockBits(d);
        Func<int,bool> bg = i => { int b = px[i*4], g = px[i*4+1], r = px[i*4+2]; int mx = Math.Max(r, g);
            return (b > 12 && b > blueRatio * mx + 4) || (r + g + b) / 3 <= dark; };
        var isBg = new bool[w*h]; var q = new Queue<int>();
        for (int x = 0; x < w; x++) { q.Enqueue(x); q.Enqueue((h-1)*w + x); }
        for (int y = 0; y < h; y++) { q.Enqueue(y*w); q.Enqueue(y*w + w-1); }
        while (q.Count > 0) { int i = q.Dequeue(); if (isBg[i] || !bg(i)) continue; isBg[i] = true;
            int x = i % w, y = i / w;
            if (x > 0) q.Enqueue(i-1); if (x < w-1) q.Enqueue(i+1); if (y > 0) q.Enqueue(i-w); if (y < h-1) q.Enqueue(i+w); }
        // keep only the largest solid area (drops UI text bits); bbox of foreground
        int x0 = w, y0 = h, x1 = 0, y1 = 0;
        var o = new byte[w*h*4];
        // Soft key (black backdrop): pixels reached from the border keep alpha = max channel / dark, colour divided by
        // alpha (un-blend from black). Everything else is opaque.
        for (int i = 0; i < w*h; i++) {
            int x = i % w, y = i / w;
            int b = px[i*4], g = px[i*4+1], r = px[i*4+2];
            int a = 255;
            if (isBg[i]) {
                int m = Math.Max(r, Math.Max(g, b));
                a = Math.Min(255, Math.Max(0, (m - 7) * 255 / Math.Max(1, dark - 7)));
            }
            if (Math.Max(r, Math.Max(g, b)) > 30) { if (x < x0) x0 = x; if (x > x1) x1 = x; if (y < y0) y0 = y; if (y > y1) y1 = y; }
            if (a == 0) continue;
            float k = 255f / a;
            o[i*4] = (byte)Math.Min(255, b * k); o[i*4+1] = (byte)Math.Min(255, g * k); o[i*4+2] = (byte)Math.Min(255, r * k);
            o[i*4+3] = (byte)a;
        }
        var full = new Bitmap(w, h, PixelFormat.Format32bppArgb);
        var fd = full.LockBits(new Rectangle(0,0,w,h), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
        System.Runtime.InteropServices.Marshal.Copy(o, 0, fd.Scan0, o.Length); full.UnlockBits(fd);
        return full.Clone(new Rectangle(x0, y0, x1-x0+1, y1-y0+1), PixelFormat.Format32bppArgb);
    }
}
'@
$src = [Drawing.Bitmap]::FromFile($In)
$c = [Cut]::Run($src, $Blue, $Dark)
if ($Top -lt 1) { $c = $c.Clone((New-Object Drawing.Rectangle 0, 0, $c.Width, ([int]($c.Height * $Top))), $c.PixelFormat) }
$o = New-Object Drawing.Bitmap $W, $H
$g = [Drawing.Graphics]::FromImage($o); $g.InterpolationMode = 'HighQualityBicubic'; $g.Clear([Drawing.Color]::Transparent)
$pad = 0.04; $sc = [Math]::Min($W * (1 - 2*$pad) / $c.Width, $H * (1 - 2*$pad) / $c.Height)
$dw = [int]($c.Width * $sc); $dh = [int]($c.Height * $sc)
$g.DrawImage($c, [int](($W - $dw) / 2), [int]($H - $dh - $H * $pad), $dw, $dh)
$o.Save($Out, [Drawing.Imaging.ImageFormat]::Png)
"cut {0}x{1} -> {2}" -f $c.Width, $c.Height, $Out
