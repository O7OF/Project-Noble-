# Copy a tif with its alpha set to a constant (0..255), or scaled (-Scale). setalpha.ps1 -In a.tif -Out b.tif -A 0 | -Scale 0.5
param([string]$In, [string]$Out, [int]$A = -1, [double]$Scale = -1)
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class SA {
  public static string Run(string src, string dst, int a, double sc) {
    using (var b0 = new Bitmap(src)) using (var b = b0.Clone(new Rectangle(0,0,b0.Width,b0.Height), PixelFormat.Format32bppArgb)) {
      int w=b.Width,h=b.Height; var d=b.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadWrite,PixelFormat.Format32bppArgb);
      var px=new byte[w*h*4]; Marshal.Copy(d.Scan0,px,0,px.Length); long s0=0,s1=0;
      for(int i=0;i<w*h;i++){ s0+=px[i*4+3]; int v = a>=0 ? a : (int)Math.Round(px[i*4+3]*sc); px[i*4+3]=(byte)Math.Max(0,Math.Min(255,v)); s1+=px[i*4+3]; }
      Marshal.Copy(px,0,d.Scan0,px.Length); b.UnlockBits(d); b.Save(dst, ImageFormat.Tiff);
      return string.Format("alpha mean {0:F1} -> {1:F1}", s0/(double)(w*h), s1/(double)(w*h)); } } }
'@
[SA]::Run($In, $Out, $A, $Scale)
