Add-Type -AssemblyName System.Drawing
$D = 'D:\Steam\steamapps\common\HREK\data\objects\characters\elite_noble\bitmaps'
foreach ($n in 'ch_elite_honor_armor_01','ch_elite_honor_armor_01_grey','ch_helmet_councilor_01_diff') {
    $b = [Drawing.Bitmap]::FromFile("$D\$n.tif")
    "{0} {1}x{2} fmt={3}" -f $n,$b.Width,$b.Height,$b.PixelFormat
    $w = $b.Width; $h = $b.Height
    foreach ($pt in @(@([int]($w/2),[int]($h/2)),@([int]($w/4),[int]($h/4)),@([int](3*$w/4),[int]($h/3)),@([int]($w/3),[int](2*$h/3)))) {
        $c = $b.GetPixel($pt[0],$pt[1]); "  ({0},{1}) rgba {2} {3} {4} {5}" -f $pt[0],$pt[1],$c.R,$c.G,$c.B,$c.A
    }
    $b.Dispose()
}
