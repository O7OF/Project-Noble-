# Make visor-colour Armory pictures (2026-11-13): stock Reach visor icon "silver" as the base; the visor is the area where
# the stock Silver and Blue icons differ; that area is recoloured with each kVisorColors colour (from ArmorCustomizer.cpp).
# -Out: output folder. -Only: comma list of ids. -Check: also writes check_<id>.png for blue/gold to compare with stock.
param([string]$Out, [string]$Only = '', [switch]$Overwrite)
$Vis = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\ui\reach\armory\visor_color'
$Src = Get-Content 'D:\Project-Noble-Closed-Source\src\ui\ArmorCustomizer.cpp' -Raw
Add-Type -ReferencedAssemblies System.Drawing @'
using System; using System.Drawing; using System.Drawing.Imaging;
public static class VisorGen {
    static float[] Read(Bitmap b) {
        var r = new Rectangle(0, 0, b.Width, b.Height);
        var d = b.LockBits(r, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
        var px = new byte[b.Width * b.Height * 4];
        System.Runtime.InteropServices.Marshal.Copy(d.Scan0, px, 0, px.Length); b.UnlockBits(d);
        var f = new float[px.Length]; for (int i = 0; i < px.Length; i++) f[i] = px[i] / 255f; return f;   // BGRA
    }
    public static float[] Mask(Bitmap s, Bitmap blue) {
        var a = Read(s); var b = Read(blue); int n = a.Length / 4; var m = new float[n];
        for (int i = 0; i < n; i++) {
            float d = Math.Abs(a[i*4] - b[i*4]) + Math.Abs(a[i*4+1] - b[i*4+1]) + Math.Abs(a[i*4+2] - b[i*4+2]);
            m[i] = Math.Min(1f, Math.Max(0f, (d - 0.06f) / 0.20f)) * a[i*4+3];
        }
        return m;
    }
    public static Bitmap Make(Bitmap s, float[] m, float tr, float tg, float tb) {
        var a = Read(s); int w = s.Width, h = s.Height;
        var o = new byte[a.Length];
        for (int i = 0; i < w * h; i++) {
            float B = a[i*4], G = a[i*4+1], R = a[i*4+2], A = a[i*4+3];
            float L = 0.299f * R + 0.587f * G + 0.114f * B;
            float hi = Math.Max(0f, L - 0.62f) * 1.6f;            // keep bright reflections bright
            float nr = Math.Min(1f, L * 1.35f * tr + hi), ng = Math.Min(1f, L * 1.35f * tg + hi), nb = Math.Min(1f, L * 1.35f * tb + hi);
            float k = m[i];
            o[i*4]   = (byte)(255 * (B + (nb - B) * k));
            o[i*4+1] = (byte)(255 * (G + (ng - G) * k));
            o[i*4+2] = (byte)(255 * (R + (nr - R) * k));
            o[i*4+3] = (byte)(255 * A);
        }
        var bmp = new Bitmap(w, h, PixelFormat.Format32bppArgb);
        var d = bmp.LockBits(new Rectangle(0, 0, w, h), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
        System.Runtime.InteropServices.Marshal.Copy(o, 0, d.Scan0, o.Length); bmp.UnlockBits(d);
        return bmp;
    }
}
'@
$silver = [Drawing.Bitmap]::FromFile("$Vis\silver.png"); $blue = [Drawing.Bitmap]::FromFile("$Vis\blue.png")
$mask = [VisorGen]::Mask($silver, $blue)
"visor pixels: " + ($mask | Where-Object { $_ -gt 0.5 }).Count
$re = [regex]'\{"[^"]+",\s*"([a-z_]+)",\s*([\d.]+)f,\s*([\d.]+)f,\s*([\d.]+)f'
$onlyList = @($Only -split ',' | Where-Object { $_ })
"colours found: " + $re.Matches($Src).Count + " (source chars " + $Src.Length + ")"
New-Item -ItemType Directory -Force $Out | Out-Null
foreach ($mt in $re.Matches($Src)) {
    $id = $mt.Groups[1].Value
    if ($onlyList.Count -and $onlyList -notcontains $id) { continue }
    $file = Join-Path $Out "$id.png"
    if ((Test-Path "$Vis\$id.png") -and -not $Overwrite -and $Out -eq $Vis) { "keep stock $id"; continue }
    $bmp = [VisorGen]::Make($silver, $mask, [float]$mt.Groups[2].Value, [float]$mt.Groups[3].Value, [float]$mt.Groups[4].Value)
    $bmp.Save($file, [Drawing.Imaging.ImageFormat]::Png); $bmp.Dispose()
    "made $id"
}

