# Make dark texels DXT-safe: texels with max(R,G,B) <= MaxV become a neutral grey that RGB565 stores exactly
# (k*255/31 for R/B, 2k*255/63 for G), so DXT can't tint them green. Alpha untouched.
# snap565.ps1 -In a.tif -Out b.tif [-MaxV 48]
param([string]$In, [string]$Out, [int]$MaxV = 48)
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class S5 {
  public static string Run(string src, string dst, int maxV) {
    using (var b0 = new Bitmap(src)) using (var b = b0.Clone(new Rectangle(0,0,b0.Width,b0.Height), PixelFormat.Format32bppArgb)) {
      int w=b.Width,h=b.Height; var d=b.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadWrite,PixelFormat.Format32bppArgb);
      var px=new byte[w*h*4]; Marshal.Copy(d.Scan0,px,0,px.Length); long n=0;
      for(int i=0;i<w*h;i++){ int B=px[i*4],G=px[i*4+1],R=px[i*4+2]; int mx=Math.Max(R,Math.Max(G,B)); if(mx>maxV) continue;
        double l=(R+G+B)/3.0; int k=(int)Math.Round(l*31/255.0);
        byte rb=(byte)Math.Round(k*255/31.0); byte g=(byte)Math.Round(2*k*255/63.0);
        px[i*4]=rb; px[i*4+1]=g; px[i*4+2]=rb; n++; }
      Marshal.Copy(px,0,d.Scan0,px.Length); b.UnlockBits(d); b.Save(dst, ImageFormat.Tiff);
      return string.Format("snapped {0} texels ({1:P1})", n, n/(double)(w*h)); } } }
'@
[S5]::Run($In, $Out, $MaxV)
