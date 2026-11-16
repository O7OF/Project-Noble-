# 2026-11-12 v2: match the Council helmet diffuse to ch_elite_armor_02 (the body): affine remap of every pixel so the
# colour-change area has the body's mean and contrast (v1 multiplied x3 and kept the helmet's extreme contrast, so the
# colour looked patchy and unpainted areas stayed black). Spec alpha matched the same way.
# Usage: lift_council.ps1 [-Mean 149] [-Sd 64] [-AlphaMean 60] | -Restore
param([string]$Name = 'ch_helmet_councilor_01_diff', [double]$Mean = 149, [double]$Sd = 64, [double]$AlphaMean = 60, [switch]$Restore)
$H = 'D:\Steam\steamapps\common\HREK'
$D = "$H\data\objects\characters\elite_noble\bitmaps"
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry\before_councillift"
$n = $Name
New-Item -ItemType Directory -Force $Bk | Out-Null
if ($Restore) { Copy-Item "$Bk\$n.tif" "$D\$n.tif" -Force; "restored" }
else {
  if (-not (Test-Path "$Bk\$n.tif")) { Copy-Item "$D\$n.tif" "$Bk\$n.tif" }
  Add-Type -AssemblyName System.Drawing
  Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class LC3 {
  static byte[] Load(string p, out int w, out int h){ using(var b=new Bitmap(p)){ w=b.Width; h=b.Height; var c=b.Clone(new Rectangle(0,0,w,h),PixelFormat.Format32bppArgb);
    var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb); var d=new byte[w*h*4]; Marshal.Copy(r.Scan0,d,0,d.Length); c.UnlockBits(r); c.Dispose(); return d; } }
  public static string Run(string src, string cc, string dst, double tm, double tsd, double tam){
    int w,h,cw,ch; var d=Load(src,out w,out h); var m=Load(cc,out cw,out ch);
    double s=0,s2=0,sa=0; int n=0;
    for(int y=0;y<h;y++) for(int x=0;x<w;x++){ int i=y*w+x; int j=(y*ch/h)*cw+(x*cw/w);
      if(Math.Max(m[j*4+2],m[j*4+1])<128) continue; double v=(d[i*4]+d[i*4+1]+d[i*4+2])/3.0; s+=v; s2+=v*v; sa+=d[i*4+3]; n++; }
    double mean=s/n, sd=Math.Sqrt(s2/n-mean*mean), am=sa/n, b=tsd/sd, a=tm-b*mean, fa=tam/am;
    for(int i=0;i<w*h;i++){ for(int c=0;c<3;c++) d[i*4+c]=(byte)Math.Max(0,Math.Min(255,Math.Round(a+b*d[i*4+c]))); d[i*4+3]=(byte)Math.Min(255,Math.Round(d[i*4+3]*fa)); }
    using(var c=new Bitmap(w,h,PixelFormat.Format32bppArgb)){ var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb); Marshal.Copy(d,0,r.Scan0,d.Length); c.UnlockBits(r);
      var codec=Array.Find(ImageCodecInfo.GetImageEncoders(), x=>x.MimeType=="image/tiff"); var ep=new EncoderParameters(1); ep.Param[0]=new EncoderParameter(System.Drawing.Imaging.Encoder.Compression,(long)EncoderValue.CompressionNone); c.Save(dst,codec,ep); }
    return string.Format("source mean {0:N0} sd {1:N0} -> v' = {2:N1} + {3:N2}*v ; alpha {4:N0} x{5:N2}", mean, sd, a, b, am, fa);
  } }
'@
  [LC3]::Run("$Bk\$n.tif", "$D\${n}_cc.tif", "$D\$n.tif", $Mean, $Sd, $AlphaMean)
}
Set-Location $H
& .\tool.exe reimport-bitmaps-single "objects\characters\elite_noble\bitmaps\$n" *> "$env:TEMP\lc.txt"
"tool exit $LASTEXITCODE"

