# Darken PH diffuse textures with GDI+ in BYTE space (the "0.20 stock" metric = mean of raw 8-bit RGB / 255). Alpha and RGB-under-alpha-0 are kept
# (LockBits directly on the loaded 32bppArgb bitmap; never DrawImage, which zeroes RGB where alpha = 0).
# Originals are copied once to HREK\noble_mods\elite_noble_backup_before_foundry\ph_orig and always read from there.
# Usage: darken_ph.ps1 -Report | -Apply [-NoReimport] [-Target 0.24]
param([switch]$Report, [switch]$Apply, [switch]$NoReimport, [switch]$NoLut, [switch]$Raw, [switch]$AlphaOnly, [string[]]$Only, [double]$Target = 0.24)
$H = 'D:\Steam\steamapps\common\HREK'
$D = "$H\data\objects\characters\elite_noble\bitmaps"
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry\ph_orig"
New-Item -ItemType Directory -Force $Bk | Out-Null
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class Dk {
  static byte[] Load(string path, out int w, out int h){
    using(var b=new Bitmap(path)){
      w=b.Width; h=b.Height;
      Bitmap c=b; bool own=false;
      if(b.PixelFormat!=PixelFormat.Format32bppArgb){ c=b.Clone(new Rectangle(0,0,w,h),PixelFormat.Format32bppArgb); own=true; }
      var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb);
      byte[] d=new byte[w*h*4]; Marshal.Copy(r.Scan0,d,0,d.Length); c.UnlockBits(r); if(own) c.Dispose();
      return d;
    }
  }
  public static double[] Stats(string path){
    int w,h; var d=Load(path,out w,out h); int n=w*h; double r=0,g=0,b=0,a=0;
    for(int i=0;i<n;i++){ b+=d[i*4]; g+=d[i*4+1]; r+=d[i*4+2]; a+=d[i*4+3]; }
    return new double[]{r/n/255,g/n/255,b/n/255,a/n/255};
  }
  // mode 0: multiply RGB by f; mode 1: luminance grey * f. Returns resulting RGB mean (0..1).
  public static double Scale(string src,string dst,double f,int mode,double af,int[] lut){
    int w,h; var d=Load(src,out w,out h); int n=w*h; double sum=0;
    for(int i=0;i<n;i++){
      double B=d[i*4],G=d[i*4+1],R=d[i*4+2];
      if(mode==1){ double l=0.299*R+0.587*G+0.114*B; R=G=B=l; }
      if(lut!=null){ double l=0.299*R+0.587*G+0.114*B; double nl=lut[(int)Math.Min(255,Math.Round(l))];
        if(l<1.0){ R=G=B=nl; } else { double g=nl/l; R*=g; G*=g; B*=g; } }
      d[i*4]=(byte)Math.Min(255,Math.Round(B*f)); d[i*4+1]=(byte)Math.Min(255,Math.Round(G*f)); d[i*4+2]=(byte)Math.Min(255,Math.Round(R*f));
      if(af!=1.0) d[i*4+3]=(byte)Math.Round(d[i*4+3]*af);
      sum+=(d[i*4]+d[i*4+1]+d[i*4+2])/3.0;
    }
    using(var c=new Bitmap(w,h,PixelFormat.Format32bppArgb)){
      var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb);
      Marshal.Copy(d,0,r.Scan0,d.Length); c.UnlockBits(r);
      var codec=Array.Find(ImageCodecInfo.GetImageEncoders(), x=>x.MimeType=="image/tiff");
      var ep=new EncoderParameters(1); ep.Param[0]=new EncoderParameter(System.Drawing.Imaging.Encoder.Compression,(long)EncoderValue.CompressionNone);
      c.Save(dst,codec,ep);
    }
    return sum/n/255;
  }
  public static double LumMean(string src){
    int w,h; var d=Load(src,out w,out h); int n=w*h; double s=0;
    for(int i=0;i<n;i++) s+=0.299*d[i*4+2]+0.587*d[i*4+1]+0.114*d[i*4];
    return s/n/255;
  }
}
'@ -ReferencedAssemblies System.Drawing
# jobs: dest name, source name (read from $Bk), mode (0 scale, 1 grey), target mean (-1 = factor 1)
$jobs = @(
  @{ dst='ch_helmet_councilor_01_diff'; src='ch_helmet_councilor_01_diff'; mode=0; tgt=$Target; at=0.28; lut='council' },
  @{ dst='elite_ossoona_helmet_armor_diffuse_4k'; src='elite_ossoona_helmet_armor_diffuse_4k'; mode=0; tgt=$Target; at=0.28; lut='ossoona' },
  @{ dst='mp_newelite_diffuse'; src='mp_newelite_diffuse'; mode=0; tgt=0.27; at=0.28; lut='combat' },
  @{ dst='ch_elite_armor_02'; src='ch_elite_armor_02'; mode=0; tgt=-1; at=0.25; lut='armor02' },
  @{ dst='ch_elite_honor_armor_01_grey'; src='ch_elite_honor_armor_01'; mode=1; tgt=$Target; at=0.22; lut='hg' },
  @{ dst='ch_elite_honor_armor_01_em'; src='ch_elite_honor_armor_01_em'; mode=1; tgt=0.5 }
)
foreach ($j in $jobs) { if (-not (Test-Path "$Bk\$($j.src).tif")) { Copy-Item "$D\$($j.src).tif" "$Bk\$($j.src).tif" } }
if ($Report) {
  foreach ($j in $jobs) { $s = [Dk]::Stats("$Bk\$($j.src).tif"); "{0,-44} src RGBA mean {1:N3} {2:N3} {3:N3} a {4:N3} | lum {5:N3}" -f $j.src,$s[0],$s[1],$s[2],$s[3],[Dk]::LumMean("$Bk\$($j.src).tif") }
  foreach ($n in 'ch_elite_armor_02','mp_newelite_diffuse','ch_elite_honor_armor_01_classic') { $s = [Dk]::Stats("$D\$n.tif"); "{0,-44} ref  RGB mean {1:N3}" -f $n,(($s[0]+$s[1]+$s[2])/3) }
}
if ($Apply) {
  foreach ($j in $jobs) {
    if ($Only -and $Only -notcontains $j.dst) { continue }
    $src = "$Bk\$($j.src).tif"; $dst = "$D\$($j.dst).tif"
    if ($j.tgt -lt 0) { $f = 1.0 } else {
      if ($j.mode -eq 1) { $cur = [Dk]::LumMean($src) } else { $s = [Dk]::Stats($src); $cur = ($s[0]+$s[1]+$s[2])/3 }
      $f = $j.tgt / $cur }
    $af = 1.0; if ($j.af) { $af = [double]$j.af }
    if ($j.at) { $sa = [Dk]::Stats($src); $atv = [double]$j.at; if ($env:ALPHA_AT) { $atv = [double]$env:ALPHA_AT }; $af = $atv / $sa[3] }   # target alpha (specular mask) mean; stock Reach Elite = 0.25-0.32
    $lut = $null
    if ($j.lut -and -not $NoLut) { $lf = "$env:T_LUTDIR\lut_$($j.lut).txt"; if (Test-Path $lf) { $lut = [int[]]((Get-Content $lf -Raw).Trim() -split ','); $f = 1.0 } }
    if ($AlphaOnly) { $f = 1.0; $lut = $null }
    if ($Raw) { $f = 1.0; $af = 1.0; $lut = $null }   # -Raw: original brightness/alpha (grey job still converts to luminance)
    $m = [Dk]::Scale($src, $dst, $f, $j.mode, $af, $lut)
    "{0}: factor {1:N3} -> RGB mean {2:N3}" -f $j.dst,$f,$m
  }
  if (-not $NoReimport) {
    Set-Location $H
    foreach ($j in $jobs) { $n = $j.dst; if ($Only -and $Only -notcontains $n) { continue }
      $p = Start-Process -FilePath .\tool.exe -ArgumentList @('reimport-bitmaps-single', "objects\characters\elite_noble\bitmaps\$n") -NoNewWindow -PassThru -RedirectStandardOutput "$env:TEMP\dk_$n.txt" -RedirectStandardError "$env:TEMP\dk_$n.err"
      $p.WaitForExit(300000) | Out-Null
      "{0}: tool exit {1} | {2}" -f $n, $p.ExitCode, ((Get-Content "$env:TEMP\dk_$n.txt","$env:TEMP\dk_$n.err" | Select-String -Pattern 'error|fail|warn' | Select-Object -First 2 | ForEach-Object { $_.Line.Trim() }) -join ' / ')
    }
  }
}
