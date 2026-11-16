Add-Type -AssemblyName System.Drawing
$D = 'D:\Steam\steamapps\common\HREK\data\objects\characters\elite_noble\bitmaps'
foreach ($n in 'elite_cc','ch_elite_armor_02_cc','ch_elite_honor_armor_01_cc','ch_helmet_councilor_01_diff_cc','elite_ossoona_helmet_armor_diffuse_4k_cc','cc_full_primary','mp_newelite_diffuse') {
    $b = [Drawing.Bitmap]::FromFile("$D\$n.tif"); $w=$b.Width; $h=$b.Height
    $r=$b.LockBits((New-Object Drawing.Rectangle 0,0,$w,$h),'ReadOnly','Format32bppArgb'); $by=New-Object byte[] ($w*$h*4); [Runtime.InteropServices.Marshal]::Copy($r.Scan0,$by,0,$by.Length); $b.UnlockBits($r)
    $n4=$w*$h; $sr=0;$sg=0;$sb=0;$sa=0; for($i=0;$i -lt $n4;$i+=7){ $sb+=$by[$i*4];$sg+=$by[$i*4+1];$sr+=$by[$i*4+2];$sa+=$by[$i*4+3] } ; $c=[math]::Ceiling($n4/7)
    "{0,-42} {1}x{2} {3}  meanR {4:N2} G {5:N2} B {6:N2} A {7:N2}" -f $n,$w,$h,$b.PixelFormat,($sr/$c/255),($sg/$c/255),($sb/$c/255),($sa/$c/255)
    $b.Dispose()
}
