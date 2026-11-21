# Phase 7: gives the noble_ai_* test squads somewhere to fight: one zone with a grid of areas (corner heights from the
# baked pathfinding vertices), one objective with one task over all the areas, and the squads' initial zone /
# objective set to them. Run tool.exe generate-pathfinding afterwards so the areas get firing positions.
# DRY RUN unless -Save. Re-runs replace the noble_zone / noble_obj content.
#   powershell -ExecutionPolicy Bypass -File tools\noble-forge-ai-zone.ps1 [-Save]
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag = 'levels\multi\forge_halo\forge_halo',
    [double]$CentreX = -157.351, [double]$CentreY = -224.8386, [double]$CentreZ = 0.5,
    [int]$Grid = 4, [double]$Cell = 12.0, [double]$ZWindow = 12.0,
    [switch]$Save
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_forge_ai_zone_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
function F($el, [string]$name) { foreach ($f in $el.Fields) { if ($f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; return $null }
function TopBlock($tf, [string]$name) { foreach ($f in $tf.Fields) { if ($f -is [Bungie.Tags.TagFieldBlock] -and $f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; throw "no block $name" }
function SetIdx($f, [int]$v) { $f.Value = $v }
function SetIf($el, [string]$name, $value) { $f = F $el $name; if ($f) { $f.Data = $value } else { Log "  (no field '$name')" } }

try {
    $bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    [Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) Log "CRASH $i" })
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'scenario'))
    Log ('loaded ' + $tf.Path.RelativePathWithExtension)

    # Ground heights: baked pathfinding vertices of BSP 0 near the centre.
    $pf = (TopBlock $tf 'ai pathfinding data').Elements[0]
    $verts = F $pf 'vertices'
    $v0 = $verts.Elements[0]
    Log ('vertex fields: ' + (($v0.Fields | % { $_.GetTagFieldPath().FieldName + '<' + $_.GetType().Name + '>' }) -join ', '))
    $pts = New-Object System.Collections.Generic.List[double[]]
    $half = $Grid * $Cell / 2 + 40
    foreach ($v in $verts.Elements) {
        $p = $v.Fields[0].Data
        if ([math]::Abs($p[0] - $CentreX) -le $half -and [math]::Abs($p[1] - $CentreY) -le $half -and [math]::Abs($p[2] - $CentreZ) -le $ZWindow) {
            $pts.Add([double[]]@($p[0], $p[1], $p[2]))
        }
    }
    Log ("vertices near the centre: " + $pts.Count); foreach ($q in $pts) { Log ("  v {0:N1},{1:N1},{2:N2}" -f $q[0],$q[1],$q[2]) }
    function GroundZ([double]$x, [double]$y) {
        # Nearest baked vertex in x/y (sectors on flat ground are large, so vertices are sparse).
        $best = $null; $bd = 1e9
        foreach ($q in $pts) { $d = ($q[0] - $x) * ($q[0] - $x) + ($q[1] - $y) * ($q[1] - $y); if ($d -lt $bd) { $bd = $d; $best = $q } }
        if ($best -eq $null -or $bd -gt 40 * 40) { return $null }
        return $best[2]
    }

    # Zone.
    $zones = TopBlock $tf 'zones'
    $zi = -1
    for ($i = 0; $i -lt $zones.Elements.Count; $i++) { if ((F $zones.Elements[$i] 'name').Data -eq 'noble_zone') { $zi = $i } }
    if ($zi -lt 0) { $z = $zones.AddElement(); (F $z 'name').Data = 'noble_zone'; $zi = $zones.Elements.Count - 1; Log "zone + [$zi] noble_zone" }
    $zone = $zones.Elements[$zi]
    $areas = F $zone 'areas'
    while ($areas.Elements.Count -gt 0) { $areas.RemoveElement(0) }
    $x0 = $CentreX - $Grid * $Cell / 2; $y0 = $CentreY - $Grid * $Cell / 2
    $made = 0; $skipped = 0
    for ($r = 0; $r -lt $Grid; $r++) {
        for ($c = 0; $c -lt $Grid; $c++) {
            $ax = $x0 + $c * $Cell; $ay = $y0 + $r * $Cell
            $corners = @(@($ax, $ay), @(($ax + $Cell), $ay), @(($ax + $Cell), ($ay + $Cell)), @($ax, ($ay + $Cell)))
            $zs = @(); foreach ($k in $corners) { $zs += ,(GroundZ $k[0] $k[1]) }
            if (@($zs | Where-Object { $_ -eq $null }).Count -gt 0) { $skipped++; Log ("  area r$r c$c skipped (no ground under a corner)"); continue }
            $a = $areas.AddElement()
            (F $a 'name').Data = ('noble_area_{0}_{1}' -f $r, $c)
            $pb = F $a 'points'
            for ($k = 0; $k -lt 4; $k++) {
                $p = $pb.AddElement()
                (F $p 'point').Data = [single[]]@($corners[$k][0], $corners[$k][1], ($zs[$k] + 0.3))
                (F $p 'normal').Data = [single[]]@(0, 90)
            }
            (F $a 'preset').Value = 1
            (F $a 'flags').RawValue = 2
            SetIf $a 'extrusion' ([single]0.5); SetIf $a 'sink' ([single]0.5); SetIf $a 'spacing' ([single]1.5)
            SetIf $a 'airborne spacing' ([single]1); SetIf $a 'min cover length' ([single]0.2); SetIf $a 'cover spacing' ([single]1.2)
            SetIf $a 'cover offset distance' ([single]0.25); SetIf $a 'too close distance' ([single]0.5)
            $made++
            if ($made -eq 1) { Log ("  first area corners z: " + (($zs | % { '{0:N1}' -f $_ }) -join ', ')) }
        }
    }
    Log "areas: $made made, $skipped skipped"

    # Firing positions (Sapien makes these; tool.exe does not): a grid inside each area, heights interpolated from
    # the area's corners, sorted by area, with each area's runtime start / count. Flags 321 = what ff20's
    # generated points carry. Sector / cluster are left for the bake or build to fill (-1 = unknown).
    $fps = F $zone 'firing positions'
    while ($fps.Elements.Count -gt 0) { $fps.RemoveElement(0) }
    # Vertices bucketed by 2 m cell, so each firing position can take the height of a nearby baked vertex.
    $buckets = @{}
    foreach ($q in $pts) { $k = '{0},{1}' -f [math]::Floor($q[0] / 2), [math]::Floor($q[1] / 2); if (-not $buckets.ContainsKey($k)) { $buckets[$k] = New-Object System.Collections.Generic.List[double[]] }; $buckets[$k].Add($q) }
    function NearZ([double]$x, [double]$y, [double]$guess) {
        $bx = [math]::Floor($x / 2); $by = [math]::Floor($y / 2); $best = $null; $bd = 1e9
        for ($i = -1; $i -le 1; $i++) { for ($j = -1; $j -le 1; $j++) {
            $k = '{0},{1}' -f ($bx + $i), ($by + $j)
            if (-not $buckets.ContainsKey($k)) { continue }
            foreach ($q in $buckets[$k]) { $d = [math]::Abs($q[2] - $guess); if ($d -lt $bd) { $bd = $d; $best = $q[2] } }
        } }
        if ($best -eq $null) { return $guess }
        return $best
    }
    $step = 2.0
    $n = [int][math]::Floor($Cell / $step)
    for ($ai = 0; $ai -lt $areas.Elements.Count; $ai++) {
        $a = $areas.Elements[$ai]
        $pb = F $a 'points'
        $cz = @(); $cxy = @()
        foreach ($p in $pb.Elements) { $d = (F $p 'point').Data; $cxy += ,@($d[0], $d[1]); $cz += $d[2] }
        $start = $fps.Elements.Count
        for ($i = 0; $i -lt $n; $i++) {
            for ($j = 0; $j -lt $n; $j++) {
                $u = ($i + 0.5) / $n; $w = ($j + 0.5) / $n
                $x = $cxy[0][0] + $u * ($cxy[1][0] - $cxy[0][0]); $y = $cxy[0][1] + $w * ($cxy[3][1] - $cxy[0][1])
                $z = (1 - $u) * (1 - $w) * $cz[0] + $u * (1 - $w) * $cz[1] + $u * $w * $cz[2] + (1 - $u) * $w * $cz[3]
                $z = NearZ $x $y $z
                $fp = $fps.AddElement()
                (F $fp 'position (local)').Data = [single[]]@($x, $y, ($z + 0.1))
                (F $fp 'flags').RawValue = 321
                SetIdx (F $fp 'area') $ai
                SetIf $fp 'cluster index' ([int16]-1); SetIf $fp 'bsp index' ([int16]0); SetIf $fp 'sector index' ([int32]-1)
                SetIf $fp 'normal' ([single[]]@(0, 90))
            }
        }
        SetIf $a 'runtime starting index' ([int16]$start)
        SetIf $a 'runtime count' ([int16]($fps.Elements.Count - $start))
    }
    Log ("firing positions: " + $fps.Elements.Count + " (" + $n * $n + " per area, " + $step + " m apart)")

    # Objective with one task over every area.
    $objs = TopBlock $tf 'ai objectives'
    $oi = -1
    for ($i = 0; $i -lt $objs.Elements.Count; $i++) { if ((F $objs.Elements[$i] 'name').Data -eq 'noble_obj') { $oi = $i } }
    if ($oi -lt 0) { $o = $objs.AddElement(); (F $o 'name').Data = 'noble_obj'; $oi = $objs.Elements.Count - 1; Log "objective + [$oi] noble_obj" }
    $obj = $objs.Elements[$oi]
    SetIdx (F $obj 'zone index') $zi
    $tasks = F $obj 'tasks'
    while ($tasks.Elements.Count -gt 0) { $tasks.RemoveElement(0) }
    $t = $tasks.AddElement()
    (F $t 'name').Data = 'noble_fight'
    (F $t 'priority').Data = [int16]100
    (F $t 'capacity').Data = [int16[]]@(0, 32)
    SetIdx (F $t 'first_child') -1; SetIdx (F $t 'next_sibling') -1; SetIdx (F $t 'parent') -1
    $ta = F $t 'areas'
    for ($i = 0; $i -lt $areas.Elements.Count; $i++) {
        $e = $ta.AddElement()
        (F $e 'flags').RawValue = 2
        SetIdx (F $e 'zone') $zi
        SetIdx (F $e 'area') $i
    }
    SetIdx (F $obj 'first task index') 0
    Log ("objective [$oi] task noble_fight over " + $ta.Elements.Count + " areas")

    # Squads.
    $sq = TopBlock $tf 'squads'
    foreach ($s in $sq.Elements) {
        $n = (F $s 'name').Data
        if ($n -notlike 'noble_ai_*') { continue }
        SetIdx (F $s 'initial zone') $zi
        SetIdx (F $s 'initial objective') $oi
        SetIdx (F $s 'initial task') -1
        Log "squad $n -> zone $zi, objective $oi"
    }
    if ($Save) { $tf.Save(); Log 'SAVED' } else { Log 'DRY RUN (use -Save)' }
} catch { Log ('ERROR: ' + $_.Exception.ToString()) } finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
