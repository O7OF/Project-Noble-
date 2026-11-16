# Install the original Reach ("rr") Elite textures from C:\Users\tyler\Pictures\halo into the elite_noble bitmap sources and reimport.
# Writes via the same GDI+ byte-space code as darken_ph.ps1 (alpha and RGB-under-alpha kept). Alpha (specular mask) mean can be overridden per job.
# Usage: apply_rr.ps1 [-NoReimport] [-Only name,name]
param([switch]$NoReimport, [string[]]$Only)
$H = 'D:\Steam\steamapps\common\HREK'
$D = "$H\data\objects\characters\elite_noble\bitmaps"
$Bk = "$H\noble_mods\elite_noble_backup_before_foundry\before_rr"
$S = 'C:\Users\tyler\Pictures\halo'
New-Item -ItemType Directory -Force $Bk | Out-Null
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class Rr {
  public static double[] Convert(string src, string dst, double alphaMean){
    int w,h; byte[] d;
    using(var b=new Bitmap(src)){
      w=b.Width; h=b.Height; Bitmap c=b; bool own=false;
      if(b.PixelFormat!=PixelFormat.Format32bppArgb){ c=b.Clone(new Rectangle(0,0,w,h),PixelFormat.Format32bppArgb); own=true; }
      var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb);
      d=new byte[w*h*4]; Marshal.Copy(r.Scan0,d,0,d.Length); c.UnlockBits(r); if(own) c.Dispose();
    }
    int n=w*h; double sa=0; for(int i=0;i<n;i++) sa+=d[i*4+3]; sa/=n*255.0;
    if(alphaMean>0 && sa>0){ double f=alphaMean/sa; for(int i=0;i<n;i++) d[i*4+3]=(byte)Math.Min(255,Math.Round(d[i*4+3]*f)); }
    double rgb=0, al=0; for(int i=0;i<n;i++){ rgb+=(d[i*4]+d[i*4+1]+d[i*4+2])/3.0; al+=d[i*4+3]; }
    using(var c=new Bitmap(w,h,PixelFormat.Format32bppArgb)){
      var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.WriteOnly,PixelFormat.Format32bppArgb);
      Marshal.Copy(d,0,r.Scan0,d.Length); c.UnlockBits(r);
      var codec=Array.Find(ImageCodecInfo.GetImageEncoders(), x=>x.MimeType=="image/tiff");
      var ep=new EncoderParameters(1); ep.Param[0]=new EncoderParameter(System.Drawing.Imaging.Encoder.Compression,(long)EncoderValue.CompressionNone);
      c.Save(dst,codec,ep);
    }
    return new double[]{rgb/n/255, al/n/255, sa};
  }
}
'@ -ReferencedAssemblies System.Drawing
# dst tif name (in $D), source PNG, alpha mean override (0 = keep original alpha)
$jobs = @(
  # shoulder fix 09:35: HG shoulder pad islands (UV-masked) lifted + added to the primary colour mask (originals: ch_elite_honor_armor_01_rr3_diff/cc.png)
  @{ dst='ch_elite_honor_armor_01_grey'; src='_work\hg_diff_sh.png'; at=0 },
  @{ dst='ch_elite_honor_armor_01_cc';   src='_work\hg_cc_sh.png';   at=0 },
  @{ dst='ch_elite_honor_armor_01_em';   src='ch_elite_honor_armor_01_em_rr_gray.png'; at=0 },
  @{ dst='ch_elite_armor_02';            src='_work\a02_lift.png'; at=0.10 },
  @{ dst='ch_elite_armor_02_cc';         src='ch_elite_armor_02_rr_cc.png';          at=0 },
  @{ dst='ch_helmet_councilor_01_diff';  src='ch_helmet_councilor_01_diff_rr_diff.png'; at=0.28 },
  @{ dst='ch_helmet_councilor_01_diff_cc'; src='ch_helmet_councilor_01_diff_rr_cc.png'; at=0 }
)
foreach ($j in $jobs) {
  if ($Only -and $Only -notcontains $j.dst) { continue }
  $cur = "$D\$($j.dst).tif"
  if (-not (Test-Path "$Bk\$($j.dst).tif")) { Copy-Item $cur "$Bk\$($j.dst).tif" }
  $m = [Rr]::Convert("$S\$($j.src)", $cur, $j.at)
  "{0}: RGB mean {1:N3} alpha {2:N3} (src alpha {3:N3})" -f $j.dst,$m[0],$m[1],$m[2]
}
if (-not $NoReimport) {
  Set-Location $H
  foreach ($j in $jobs) { $n = $j.dst; if ($Only -and $Only -notcontains $n) { continue }
    $p = Start-Process -FilePath .\tool.exe -ArgumentList @('reimport-bitmaps-single', "objects\characters\elite_noble\bitmaps\$n") -NoNewWindow -PassThru -RedirectStandardOutput "$env:TEMP\rr_$n.txt" -RedirectStandardError "$env:TEMP\rr_$n.err"
    $p.WaitForExit(300000) | Out-Null
    "{0}: tool exit {1} | {2}" -f $n, $p.ExitCode, ((Get-Content "$env:TEMP\rr_$n.txt","$env:TEMP\rr_$n.err" | Select-String -Pattern 'error|fail|warn' | Select-Object -First 2 | ForEach-Object { $_.Line.Trim() }) -join ' / ')
  }
}

