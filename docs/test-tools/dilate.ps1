# Edge-pad (dilate) UV islands of a tif into its gutter. Gutter = texels whose MASK tif has RGB sum < 6 (default: the file itself).
# dilate.ps1 -In a.tif -Out b.tif [-Mask m.tif] [-Px 16]   (copies all 4 channels from the nearest island texel ring by ring)
param([string]$In, [string]$Out, [string]$Mask = '', [int]$Px = 16)
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class DL {
  static byte[] Load(string p, out int w, out int h) {
    using (var b0 = new Bitmap(p)) using (var b = b0.Clone(new Rectangle(0,0,b0.Width,b0.Height), PixelFormat.Format32bppArgb)) {
      w=b.Width; h=b.Height; var d=b.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb);
      var px=new byte[w*h*4]; Marshal.Copy(d.Scan0,px,0,px.Length); b.UnlockBits(d); return px; } }
  public static string Run(string src, string dst, string mask, int iters) {
    int w,h,mw,mh; var px=Load(src,out w,out h); var mp = string.IsNullOrEmpty(mask) ? px : Load(mask,out mw,out mh);
    var filled=new bool[w*h]; int gut=0;
    for(int i=0;i<w*h;i++){ filled[i] = (mp[i*4]+mp[i*4+1]+mp[i*4+2])>=6; if(!filled[i]) gut++; }
    int done=0;
    for(int it=0;it<iters;it++){
      var nf=(bool[])filled.Clone(); var np=(byte[])px.Clone();
      for(int y=0;y<h;y++) for(int x=0;x<w;x++){ int i=y*w+x; if(filled[i]) continue;
        int s0=0,s1=0,s2=0,s3=0,n=0;
        for(int k=-1;k<=1;k++) for(int m=-1;m<=1;m++){ int xx=x+m,yy=y+k; if(xx<0||yy<0||xx>=w||yy>=h) continue; int j=yy*w+xx; if(!filled[j]) continue;
          s0+=px[j*4];s1+=px[j*4+1];s2+=px[j*4+2];s3+=px[j*4+3];n++; }
        if(n>0){ np[i*4]=(byte)(s0/n);np[i*4+1]=(byte)(s1/n);np[i*4+2]=(byte)(s2/n);np[i*4+3]=(byte)(s3/n); nf[i]=true; done++; } }
      filled=nf; px=np; }
    using (var ob=new Bitmap(w,h,PixelFormat.Format32bppArgb)) { var od=ob.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb); Marshal.Copy(px,0,od.Scan0,px.Length); ob.UnlockBits(od); ob.Save(dst, ImageFormat.Tiff); }
    return string.Format("{0}x{1} gutter {2} ({3:P1}) padded {4}", w,h,gut,gut/(double)(w*h),done);
  } }
'@
[DL]::Run($In, $Out, $Mask, $Px)
