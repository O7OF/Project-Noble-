# Read-only: maximum element counts of the zone / objective blocks in a scenario (ManagedBlam).
param([string]$Hrek = 'D:\Steam\steamapps\common\HREK', [string]$Tag = 'levels\multi\forge_halo\forge_halo')
$ErrorActionPreference = 'Stop'
function F($el, [string]$name) { foreach ($f in $el.Fields) { if ($f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; return $null }
$bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
[void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
[Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) })
try {
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'scenario'))
    $zones = $null; $objs = $null; $sq = $null
    foreach ($f in $tf.Fields) { $n = $f.GetTagFieldPath().FieldName; if ($n -eq 'zones') { $zones = $f }; if ($n -eq 'ai objectives') { $objs = $f }; if ($n -eq 'squads') { $sq = $f } }
    "zones max " + $zones.MaximumElementCount
    $z = $zones.Elements[0]
    "firing positions max " + (F $z 'firing positions').MaximumElementCount
    "areas max " + (F $z 'areas').MaximumElementCount
    "objectives max " + $objs.MaximumElementCount
    $t = (F $objs.Elements[0] 'tasks')
    "tasks max " + $t.MaximumElementCount
    "task areas max " + (F $t.Elements[0] 'areas').MaximumElementCount
    "squads max " + $sq.MaximumElementCount + "  spawn points max " + (F $sq.Elements[0] 'spawn points').MaximumElementCount
} finally { [Bungie.ManagedBlamSystem]::Stop() }
