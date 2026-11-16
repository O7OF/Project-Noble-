# Map greenish texels (G > R+Th and G > B+Th) of a tif: writes <Out> (green = hit over dimmed texture) and prints stats.
param([string]$In, [string]$Out, [int]$Th = 12)
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class GM {
  public static string Run(string src, string dst, int th) {
    using (var b0 = new Bitmap(src)) using (var b = b0.Clone(new Rectangle(0,0,b0.Width,b0.Height), PixelFormat.Format32bppArgb)) {
      int w=b.Width,h=b.Height; var d=b.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadWrite,PixelFormat.Format32bppArgb);
      var px=new byte[w*h*4]; Marshal.Copy(d.Scan0,px,0,px.Length);
      long hits=0, edgeHits=0; long sr=0,sg=0,sb=0,sa=0;
      var o=new byte[px.Length];
      for(int y=0;y<h;y++) for(int x=0;x<w;x++){ int i=(y*w+x)*4; int B=px[i],G=px[i+1],R=px[i+2],A=px[i+3];
        bool hit = G>R+th && G>B+th;
        o[i]=(byte)(B/3); o[i+1]=(byte)(G/3); o[i+2]=(byte)(R/3); o[i+3]=255;
        if(hit){ hits++; sr+=R; sg+=G; sb+=B; sa+=A; o[i]=0;o[i+1]=255;o[i+2]=0;
          bool edge=false; for(int k=-2;k<=2&&!edge;k++) for(int m=-2;m<=2&&!edge;m++){int xx=x+k,yy=y+m; if(xx<0||yy<0||xx>=w||yy>=h) continue; int j=(yy*w+xx)*4; if(px[j]+px[j+1]+px[j+2]<6) edge=true;}
          if(edge) edgeHits++; } }
      b.UnlockBits(d);
      using (var ob=new Bitmap(w,h,PixelFormat.Format32bppArgb)) { var od=ob.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb); Marshal.Copy(o,0,od.Scan0,o.Length); ob.UnlockBits(od);
        using (var sm=new Bitmap(ob, w/2, h/2)) sm.Save(dst, ImageFormat.Png); }
      if(hits==0) return "hits 0";
      return string.Format("hits {0} ({1:P2}) nearGutter {2} meanRGBA {3},{4},{5},{6}", hits, hits/(double)(w*h), edgeHits, sr/hits, sg/hits, sb/hits, sa/hits);
    } } }
'@
[GM]::Run($In, $Out, $Th)
