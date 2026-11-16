# Compare original Elite PNGs (Project-Noble-ORIGINAL) against installed sources (HREK data tiff + ph-export tif). Writes cmp_orig.csv
$o='D:\Project-Noble-ORIGINAL\data\menumaps\objects\characters\elite\bitmaps'
$T='D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
$roots=@{ hrek='D:\Steam\steamapps\common\HREK\data\objects\characters'; ph="$T\ph-export\objects\characters" }
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices;
public static class Cm {
  public static byte[] Load(string p, out int w, out int h){
    using(var b=new Bitmap(p)){ w=b.Width; h=b.Height; Bitmap c=b; bool own=false;
      if(b.PixelFormat!=PixelFormat.Format32bppArgb){ c=b.Clone(new Rectangle(0,0,w,h),PixelFormat.Format32bppArgb); own=true; }
      var r=c.LockBits(new Rectangle(0,0,w,h),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb);
      byte[] d=new byte[w*h*4]; Marshal.Copy(r.Scan0,d,0,d.Length); c.UnlockBits(r); if(own) c.Dispose(); return d; } }
  // returns: w,h, meanR,G,B,A of A, meanR,G,B,A of B, absdiff rgb, absdiff a ; null-size mismatch -> w2,h2 in [12],[13]
  public static double[] Cmp(string a, string b){
    int w1,h1,w2,h2; var A=Load(a,out w1,out h1); var B=Load(b,out w2,out h2);
    var r=new double[14]; r[0]=w1; r[1]=h1; r[12]=w2; r[13]=h2;
    int n1=w1*h1, n2=w2*h2;
    for(int i=0;i<n1;i++){ r[2]+=A[i*4+2]; r[3]+=A[i*4+1]; r[4]+=A[i*4]; r[5]+=A[i*4+3]; }
    for(int i=0;i<n2;i++){ r[6]+=B[i*4+2]; r[7]+=B[i*4+1]; r[8]+=B[i*4]; r[9]+=B[i*4+3]; }
    for(int k=2;k<6;k++) r[k]/=n1*255.0; for(int k=6;k<10;k++) r[k]/=n2*255.0;
    if(w1==w2&&h1==h2){ double d=0,da=0; for(int i=0;i<n1;i++){ d+=Math.Abs(A[i*4]-B[i*4])+Math.Abs(A[i*4+1]-B[i*4+1])+Math.Abs(A[i*4+2]-B[i*4+2]); da+=Math.Abs(A[i*4+3]-B[i*4+3]); } r[10]=d/(3.0*n1*255); r[11]=da/(n1*255.0); } else { r[10]=-1; r[11]=-1; }
    return r; }
}
'@ -ReferencedAssemblies System.Drawing
$idx=@{}
foreach($k in $roots.Keys){ Get-ChildItem $roots[$k] -Recurse -Include *.tif,*.tiff -File -ErrorAction SilentlyContinue | ForEach-Object { $key="$k|"+$_.BaseName; if(-not $idx.ContainsKey($key)){ $idx[$key]=$_.FullName } } }
$rows=@()
foreach($f in Get-ChildItem $o -Recurse -Filter *.png){
  foreach($k in 'hrek','ph'){
    $key="$k|"+$f.BaseName; if(-not $idx.ContainsKey($key)){ continue }
    try { $c=[Cm]::Cmp($f.FullName,$idx[$key]) } catch { continue }
    $rows += [pscustomobject]@{ name=$f.BaseName; src=$k; origWH="$($c[0])x$($c[1])"; instWH="$($c[12])x$($c[13])"; origRGB=('{0:N2}/{1:N2}/{2:N2}' -f $c[2],$c[3],$c[4]); instRGB=('{0:N2}/{1:N2}/{2:N2}' -f $c[6],$c[7],$c[8]); origA=[math]::Round($c[5],3); instA=[math]::Round($c[9],3); dRGB=[math]::Round($c[10],4); dA=[math]::Round($c[11],4) }
  }
}
$rows | Export-Csv "$T\cmp_orig.csv" -NoTypeInformation
"rows: $($rows.Count)"
