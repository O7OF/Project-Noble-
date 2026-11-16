# Diagnostic: set one channel of an elite_noble bitmap source tif to a value everywhere (or restore it), then reimport.
# Backup goes to HREK\noble_mods\elite_noble_backup_before_foundry\before_idtest\ (first run only).
# Usage: chanfill.ps1 -Name ch_elite_armor_02_cc -Chan R -Val 255   |   chanfill.ps1 -Name ch_elite_armor_02_cc -Restore
param([string]$Name, [string]$Chan = 'R', [int]$Val = 255, [switch]$Restore)
$H = 'D:\Steam\steamapps\common\HREK'
$D = "$H\data\objects\characters\elite_noble\bitmaps"
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry\before_idtest"
New-Item -ItemType Directory -Force $Bk | Out-Null
$cur = "$D\$Name.tif"
if ($Restore) { Copy-Item "$Bk\$Name.tif" $cur -Force; "restored $Name" }
else {
  if (-not (Test-Path "$Bk\$Name.tif")) { Copy-Item $cur "$Bk\$Name.tif" }
  Add-Type -AssemblyName System.Drawing
  Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class CF { public static void Run(string src, string dst, int off, byte v){
  int w,h; byte[] d;
  using(var b=new Bitmap(src)){ w=b.Width; h=b.Height; var c=b.Clone(new Rectangle(0,0,w,h),PixelFormat.Format32bppArgb);
    var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb); d=new byte[w*h*4]; Marshal.Copy(r.Scan0,d,0,d.Length); c.UnlockBits(r); c.Dispose(); }
  int[] pal={0xFF0000,0x00FF00,0x0000FF,0xFFFF00, 0x00FFFF,0xFF00FF,0xFFFFFF,0xFF8000, 0x8000FF,0x80FF00,0xFF80C0,0x008080, 0x804000,0x808080,0x000080,0x808000};
  if(off<0){ for(int y=0;y<h;y++) for(int x=0;x<w;x++){ int i=y*w+x; int k=(y*4/h)*4+(x*4/w); int c=pal[k]; d[i*4+2]=(byte)(c>>16); d[i*4+1]=(byte)((c>>8)&255); d[i*4]=(byte)(c&255); } }
  else for(int i=0;i<w*h;i++) d[i*4+off]=v;
  using(var c=new Bitmap(w,h,PixelFormat.Format32bppArgb)){ var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb); Marshal.Copy(d,0,r.Scan0,d.Length); c.UnlockBits(r);
    var codec=Array.Find(ImageCodecInfo.GetImageEncoders(), x=>x.MimeType=="image/tiff"); var ep=new EncoderParameters(1); ep.Param[0]=new EncoderParameter(System.Drawing.Imaging.Encoder.Compression,(long)EncoderValue.CompressionNone); c.Save(dst,codec,ep); } } }
'@
  $off = @{B=0;G=1;R=2;A=3;GRID=-1}[$Chan]
  [CF]::Run("$Bk\$Name.tif", $cur, $off, [byte]$Val); "filled $Name $Chan=$Val"
}
Set-Location $H
& .\tool.exe reimport-bitmaps-single "objects\characters\elite_noble\bitmaps\$Name" *> "$env:TEMP\cf_$Name.txt"
"tool exit $LASTEXITCODE"
