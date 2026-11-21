# Read-only: nav-mesh vertices of one BSP nearest to a point (x, y), with heights (ManagedBlam).
param([string]$Hrek = 'D:\Steam\steamapps\common\HREK', [string]$Tag = 'levels\multi\forge_halo\forge_halo',
      [double]$X, [double]$Y, [int]$Count = 8)
$ErrorActionPreference = 'Stop'
$bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
[void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
[Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) })
try {
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'scenario'))
    $pf = $null; foreach ($f in $tf.Fields) { if ($f.GetTagFieldPath().FieldName -eq 'ai pathfinding data') { $pf = $f.Elements[0] } }
    $vb = $null; foreach ($f in $pf.Fields) { if ($f.GetTagFieldPath().FieldName -eq 'vertices') { $vb = $f } }
    $list = foreach ($v in $vb.Elements) { $p = $v.Fields[0].Data; [pscustomobject]@{ x = $p[0]; y = $p[1]; z = $p[2]; d = [math]::Sqrt(($p[0] - $X) * ($p[0] - $X) + ($p[1] - $Y) * ($p[1] - $Y)) } }
    $list | Sort-Object d | Select-Object -First $Count | ForEach-Object { '{0:N1},{1:N1},{2:N2}  ({3:N1} m)' -f $_.x, $_.y, $_.z, $_.d }
} finally { [Bungie.ManagedBlamSystem]::Stop() }
