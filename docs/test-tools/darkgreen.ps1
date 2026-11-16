# Map DARK greenish texels (luma < MaxL, G > R+Th, G > B+Th). -Fix writes a copy with those texels made neutral grey
# (G set to avg(R,B)), keeping alpha. Usage: darkgreen.ps1 -In a.tif -Out map.png [-Fix fixed.tif] [-Th 3] [-MaxL 60]
param([string]$In, [string]$Out, [string]$Fix = '', [int]$Th = 3, [int]$MaxL = 60)
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class DG {
  public static string Run(string src, string dst, string fix, int th, int maxL) {
    using (var b0 = new Bitmap(src)) using (var b = b0.Clone(new Rectangle(0,0,b0.Width,b0.Height), PixelFormat.Format32bppArgb)) {
      int w=b.Width,h=b.Height; var d=b.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb);
      var px=new byte[w*h*4]; Marshal.Copy(d.Scan0,px,0,px.Length); b.UnlockBits(d);
      var o=new byte[px.Length]; var f=(byte[])px.Clone(); long hits=0; long sg=0, sr=0, sb=0;
      for(int i=0;i<w*h;i++){ int B=px[i*4],G=px[i*4+1],R=px[i*4+2]; int l=(R*3+G*6+B)/10;
        o[i*4]=(byte)Math.Min(255,B*3); o[i*4+1]=(byte)Math.Min(255,G*3); o[i*4+2]=(byte)Math.Min(255,R*3); o[i*4+3]=255;
        if(l<maxL && G>R+th && G>B+th){ hits++; sg+=G; sr+=R; sb+=B; o[i*4]=0;o[i*4+1]=255;o[i*4+2]=255; f[i*4+1]=(byte)((R+B)/2); } }
      using (var ob=new Bitmap(w,h,PixelFormat.Format32bppArgb)) { var od=ob.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb); Marshal.Copy(o,0,od.Scan0,o.Length); ob.UnlockBits(od);
        using (var sm=new Bitmap(ob, w/2, h/2)) sm.Save(dst, ImageFormat.Png); }
      if(!string.IsNullOrEmpty(fix)) using (var fb=new Bitmap(w,h,PixelFormat.Format32bppArgb)) { var fd=fb.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb); Marshal.Copy(f,0,fd.Scan0,f.Length); fb.UnlockBits(fd); fb.Save(fix, ImageFormat.Tiff); }
      return hits==0 ? "hits 0" : string.Format("dark-green hits {0} ({1:P2}) mean RGB {2},{3},{4}", hits, hits/(double)(w*h), sr/hits, sg/hits, sb/hits);
    } } }
'@
[DG]::Run($In, $Out, $Fix, $Th, $MaxL)
