# 2026-11-12: Council helmet colour mask has unpainted areas (R and G both low) that show the raw dark diffuse as a
# black patch on top of the helmet. Make those pixels Primary (R = 255 - G, so soft edges blend). -Restore undoes.
param([switch]$Restore)
$H = 'D:\Steam\steamapps\common\HREK'
$D = "$H\data\objects\characters\elite_noble\bitmaps"
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry\before_councillift"
$n = 'ch_helmet_councilor_01_diff_cc'
if ($Restore) { Copy-Item "$Bk\$n.tif" "$D\$n.tif" -Force; "restored" }
else {
  if (-not (Test-Path "$Bk\$n.tif")) { Copy-Item "$D\$n.tif" "$Bk\$n.tif" }
  Add-Type -AssemblyName System.Drawing
  Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class CCF { public static string Run(string src, string dst){
  int w,h; byte[] d;
  using(var b=new Bitmap(src)){ w=b.Width; h=b.Height; var c=b.Clone(new Rectangle(0,0,w,h),PixelFormat.Format32bppArgb);
    var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb); d=new byte[w*h*4]; Marshal.Copy(r.Scan0,d,0,d.Length); c.UnlockBits(r); c.Dispose(); }
  int ch=0;
  for(int i=0;i<w*h;i++){ int R=d[i*4+2], G=d[i*4+1]; int want=255-G; if(R<want){ d[i*4+2]=(byte)want; ch++; } }
  using(var c=new Bitmap(w,h,PixelFormat.Format32bppArgb)){ var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb); Marshal.Copy(d,0,r.Scan0,d.Length); c.UnlockBits(r);
    var codec=Array.Find(ImageCodecInfo.GetImageEncoders(), x=>x.MimeType=="image/tiff"); var ep=new EncoderParameters(1); ep.Param[0]=new EncoderParameter(System.Drawing.Imaging.Encoder.Compression,(long)EncoderValue.CompressionNone); c.Save(dst,codec,ep); }
  return string.Format("raised R on {0:P1} of pixels", (double)ch/(w*h)); } }
'@
  [CCF]::Run("$Bk\$n.tif", "$D\$n.tif")
}
Set-Location $H
& .\tool.exe reimport-bitmaps-single "objects\characters\elite_noble\bitmaps\$n" *> "$env:TEMP\ccf.txt"
"tool exit $LASTEXITCODE"
