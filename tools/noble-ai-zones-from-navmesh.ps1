# Phase 7: whole-map AI zones from the baked nav mesh (ManagedBlam). DRY RUN unless -Save.
# For every Tile x Tile metre square that has walkable sectors: one zone "nav_<col>_<row>" with Cell x Cell metre areas
# (at most 64, so one task covers them), firing positions on a Step-metre grid wherever a grid point lies inside a
# walkable sector (several levels allowed), heights from that sector's vertices, and one objective "nav_obj_<col>_<row>"
# with one task over all the zone's areas. Previous nav_* zones / objectives are replaced; other zones stay.
# Squads whose name matches -SquadPattern get the zone / objective of the tile their first spawn point is in.
# Afterwards run: tool generate-pathfinding (fills the firing positions' sector / cluster), then build-cache-file.
#   powershell -ExecutionPolicy Bypass -File tools\noble-ai-zones-from-navmesh.ps1 [-Save]
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag = 'levels\multi\forge_halo\forge_halo',
    [int]$Bsp = 0,
    [double]$Tile = 160.0, [double]$Cell = 20.0, [double]$Step = 3.5,
    [int]$MaxPerZone = 2000, [double]$MinSectorArea = 0.5,
    [string]$SquadPattern = 'noble_ai_*',
    [switch]$Save
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_ai_zones_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
function F($el, [string]$name) { foreach ($f in $el.Fields) { if ($f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; return $null }
function TopBlock($tf, [string]$name) { foreach ($f in $tf.Fields) { if ($f -is [Bungie.Tags.TagFieldBlock] -and $f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; throw "no block $name" }

Add-Type -Language CSharp -TypeDefinition @'
using System;
using System.Collections.Generic;
public class NavFp { public double X, Y, Z; public int Sector; }
public static class NavGen {
    // links: v1, v2, fwd, rev, left, right (6 ints each); verts: x,y,z
    public static List<int[]> SectorEdges(int sector, int firstLink, int[] links) {
        var edges = new List<int[]>();
        int l = firstLink, guard = 0;
        while (l >= 0 && guard++ < 256) {
            int b = l * 6;
            edges.Add(new int[] { links[b], links[b + 1] });
            int next;
            if (links[b + 4] == sector) next = links[b + 2];
            else if (links[b + 5] == sector) next = links[b + 3];
            else break;
            if (next == firstLink) break;
            l = next;
        }
        return edges;
    }
    public static bool Inside(double x, double y, List<int[]> edges, double[] v) {
        bool inside = false;
        foreach (var e in edges) {
            double x1 = v[e[0] * 3], y1 = v[e[0] * 3 + 1], x2 = v[e[1] * 3], y2 = v[e[1] * 3 + 1];
            if ((y1 > y) != (y2 > y)) {
                double xc = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
                if (x < xc) inside = !inside;
            }
        }
        return inside;
    }
    public static double HeightAt(double x, double y, List<int[]> edges, double[] v) {
        double ws = 0, zs = 0;
        foreach (var e in edges) for (int k = 0; k < 2; k++) {
            int i = e[k]; double dx = v[i * 3] - x, dy = v[i * 3 + 1] - y; double w = 1.0 / (dx * dx + dy * dy + 0.01);
            ws += w; zs += w * v[i * 3 + 2];
        }
        return zs / ws;
    }
    public static double Area(List<int[]> edges, double[] v) {   // |sum of cross products| / 2 over the loop's edges
        double a = 0;
        foreach (var e in edges) a += v[e[0] * 3] * v[e[1] * 3 + 1] - v[e[1] * 3] * v[e[0] * 3 + 1];
        return Math.Abs(a) / 2;
    }
    // Returns firing positions per tile key "c,r" -> list, sampling a Step grid inside each walkable sector.
    public static Dictionary<string, List<NavFp>> Generate(int[] sectorFlags, int[] sectorFirst, int[] links, double[] v,
        double tile, double step, double minArea, out int usedSectors) {
        var res = new Dictionary<string, List<NavFp>>();
        usedSectors = 0;
        for (int s = 0; s < sectorFirst.Length; s++) {
            if ((sectorFlags[s] & 1) == 0 || sectorFirst[s] < 0) continue;   // bit 0 = walkable
            var edges = SectorEdges(s, sectorFirst[s], links);
            if (edges.Count < 3 || Area(edges, v) < minArea) continue;
            double minx = 1e9, miny = 1e9, maxx = -1e9, maxy = -1e9;
            foreach (var e in edges) for (int k = 0; k < 2; k++) {
                int i = e[k]; minx = Math.Min(minx, v[i * 3]); maxx = Math.Max(maxx, v[i * 3]); miny = Math.Min(miny, v[i * 3 + 1]); maxy = Math.Max(maxy, v[i * 3 + 1]);
            }
            usedSectors++;
            int added = 0;
            for (double gx = Math.Floor(minx / step) * step + step / 2; gx <= maxx; gx += step)
                for (double gy = Math.Floor(miny / step) * step + step / 2; gy <= maxy; gy += step) {
                    if (!Inside(gx, gy, edges, v)) continue;
                    Add(res, tile, gx, gy, HeightAt(gx, gy, edges, v), s); added++;
                }
            if (added == 0) {   // small sector: one point at the centre of its outline
                double cx = 0, cy = 0; int n = 0;
                foreach (var e in edges) { cx += v[e[0] * 3]; cy += v[e[0] * 3 + 1]; n++; }
                cx /= n; cy /= n;
                if (Inside(cx, cy, edges, v)) Add(res, tile, cx, cy, HeightAt(cx, cy, edges, v), s);
            }
        }
        return res;
    }
    static void Add(Dictionary<string, List<NavFp>> res, double tile, double x, double y, double z, int s) {
        string key = Math.Floor(x / tile) + "," + Math.Floor(y / tile);
        List<NavFp> l; if (!res.TryGetValue(key, out l)) { l = new List<NavFp>(); res[key] = l; }
        l.Add(new NavFp { X = x, Y = y, Z = z, Sector = s });
    }
}
'@

try {
    $bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    [Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) Log "CRASH $i" })
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'scenario'))
    Log ('loaded ' + $tf.Path.RelativePathWithExtension)

    # Read the nav mesh of one BSP into flat arrays.
    $pf = (TopBlock $tf 'ai pathfinding data').Elements[$Bsp]
    $sb = F $pf 'sectors'; $lb = F $pf 'links'; $vb = F $pf 'vertices'
    $t0 = Get-Date
    $ns = $sb.Elements.Count; $sf = New-Object int[] $ns; $sl = New-Object int[] $ns
    for ($i = 0; $i -lt $ns; $i++) { $e = $sb.Elements[$i]; $sf[$i] = [int](F $e 'Path-finding sector flags').RawValue; $sl[$i] = [int](F $e 'first link (do not set manually)').Data }
    $nl = $lb.Elements.Count; $links = New-Object int[] ($nl * 6)
    for ($i = 0; $i -lt $nl; $i++) {
        $f = $lb.Elements[$i].Fields
        $b = $i * 6
        foreach ($x in $f) {
            switch ($x.GetTagFieldPath().FieldName) {
                'vertex 1' { $links[$b] = [int]$x.Data } 'vertex 2' { $links[$b + 1] = [int]$x.Data }
                'forward link' { $links[$b + 2] = [int]$x.Data } 'reverse link' { $links[$b + 3] = [int]$x.Data }
                'left sector' { $links[$b + 4] = [int]$x.Data } 'right sector' { $links[$b + 5] = [int]$x.Data }
            }
        }
    }
    $nv = $vb.Elements.Count; $verts = New-Object double[] ($nv * 3)
    for ($i = 0; $i -lt $nv; $i++) { $p = $vb.Elements[$i].Fields[0].Data; $verts[$i * 3] = $p[0]; $verts[$i * 3 + 1] = $p[1]; $verts[$i * 3 + 2] = $p[2] }
    Log ("nav mesh BSP $Bsp : $ns sectors, $nl links, $nv vertices (read in {0:N0} s)" -f ((Get-Date) - $t0).TotalSeconds)

    $used = 0
    $tiles = [NavGen]::Generate($sf, $sl, $links, $verts, $Tile, $Step, $MinSectorArea, [ref]$used)
    $total = 0; foreach ($k in $tiles.Keys) { $total += $tiles[$k].Count }
    Log ("walkable sectors used: $used; candidate firing positions: $total in " + $tiles.Count + " tiles")

    # Replace earlier generated zones / objectives (names nav_*), keep everything else.
    $zones = TopBlock $tf 'zones'; $objs = TopBlock $tf 'ai objectives'
    for ($i = $zones.Elements.Count - 1; $i -ge 0; $i--) { $n = (F $zones.Elements[$i] 'name').Data; if ($n -like 'nav_*' -or $n -eq 'noble_zone') { $zones.RemoveElement($i) } }
    for ($i = $objs.Elements.Count - 1; $i -ge 0; $i--) { $n = (F $objs.Elements[$i] 'name').Data; if ($n -like 'nav_*' -or $n -eq 'noble_obj') { $objs.RemoveElement($i) } }

    $tileZone = @{}; $made = 0; $fpTotal = 0
    foreach ($key in ($tiles.Keys | Sort-Object)) {
        $list = $tiles[$key]
        if ($list.Count -lt 8) { continue }   # stray bits of walkable surface
        if ($zones.Elements.Count -ge 128 -or $objs.Elements.Count -ge 64) { Log "zone / objective limit reached; tile $key skipped"; continue }
        $c, $r = $key.Split(',') | ForEach-Object { [int]$_ }
        $x0 = $c * $Tile; $y0 = $r * $Tile
        $nCell = [int][math]::Floor($Tile / $Cell)
        # Group points into cells; drop points evenly when the zone would exceed its firing position budget.
        $keep = [math]::Min(1.0, $MaxPerZone / [double]$list.Count)
        $byCell = @{}; $acc = 0.0
        foreach ($p in $list) {
            $acc += $keep; if ($acc -lt 1.0) { continue }; $acc -= 1.0
            $cc = [math]::Min($nCell - 1, [int][math]::Floor(($p.X - $x0) / $Cell)); $cr = [math]::Min($nCell - 1, [int][math]::Floor(($p.Y - $y0) / $Cell))
            $ck = $cr * $nCell + $cc
            if (-not $byCell.ContainsKey($ck)) { $byCell[$ck] = New-Object System.Collections.Generic.List[NavFp] }
            $byCell[$ck].Add($p)
        }
        $z = $zones.AddElement(); $zi = $zones.Elements.Count - 1
        (F $z 'name').Data = ('nav_{0}_{1}' -f $c, $r)
        $areas = F $z 'areas'; $fps = F $z 'firing positions'
        foreach ($ck in ($byCell.Keys | Sort-Object)) {
            $pts = $byCell[$ck]
            $cc = $ck % $nCell; $cr = [int][math]::Floor($ck / $nCell)
            $ax = $x0 + $cc * $Cell; $ay = $y0 + $cr * $Cell
            $zs = @($pts | ForEach-Object { $_.Z } | Sort-Object); $zm = $zs[[int][math]::Floor($zs.Count / 2)]
            $a = $areas.AddElement(); $ai = $areas.Elements.Count - 1
            (F $a 'name').Data = ('nav_{0}_{1}_{2}' -f $c, $r, $ck)
            foreach ($corner in @(@($ax, $ay), @(($ax + $Cell), $ay), @(($ax + $Cell), ($ay + $Cell)), @($ax, ($ay + $Cell)))) {
                $p = (F $a 'points').AddElement()
                (F $p 'point').Data = [single[]]@($corner[0], $corner[1], ($zm + 0.3)); (F $p 'normal').Data = [single[]]@(0, 90)
            }
            (F $a 'preset').Value = 1; (F $a 'flags').RawValue = 2
            foreach ($nm in 'extrusion', 'sink') { (F $a $nm).Data = [single]0.5 }
            (F $a 'spacing').Data = [single]$Step
            $start = $fps.Elements.Count
            foreach ($p in $pts) {
                $fp = $fps.AddElement()
                (F $fp 'position (local)').Data = [single[]]@($p.X, $p.Y, ($p.Z + 0.1))
                (F $fp 'flags').RawValue = 321
                (F $fp 'area').Value = $ai
                (F $fp 'cluster index').Data = [int16]-1; (F $fp 'bsp index').Data = [int16]$Bsp; (F $fp 'sector index').Data = [int32]$p.Sector
                (F $fp 'normal').Data = [single[]]@(0, 90)
            }
            (F $a 'runtime starting index').Data = [int16]$start
            (F $a 'runtime count').Data = [int16]($fps.Elements.Count - $start)
        }
        $o = $objs.AddElement(); $oi = $objs.Elements.Count - 1
        (F $o 'name').Data = ('nav_obj_{0}_{1}' -f $c, $r)
        (F $o 'zone index').Value = $zi
        $t = (F $o 'tasks').AddElement()
        (F $t 'name').Data = 'fight'; (F $t 'priority').Data = [int16]100; (F $t 'capacity').Data = [int16[]]@(0, 64)
        (F $t 'first_child').Value = -1; (F $t 'next_sibling').Value = -1; (F $t 'parent').Value = -1
        for ($i = 0; $i -lt $areas.Elements.Count; $i++) { $e = (F $t 'areas').AddElement(); (F $e 'flags').RawValue = 2; (F $e 'zone').Value = $zi; (F $e 'area').Value = $i }
        (F $o 'first task index').Value = 0
        $tileZone[$key] = @($zi, $oi)
        $made++; $fpTotal += $fps.Elements.Count
        Log ("zone [$zi] nav_{0}_{1}: {2} areas, {3} firing positions" -f $c, $r, $areas.Elements.Count, $fps.Elements.Count)
    }
    Log "zones made: $made, firing positions: $fpTotal"

    $sq = TopBlock $tf 'squads'
    foreach ($s in $sq.Elements) {
        $n = (F $s 'name').Data
        if ($n -notlike $SquadPattern) { continue }
        $sp = F $s 'spawn points'
        if ($sp.Elements.Count -eq 0) { continue }
        $pos = (F $sp.Elements[0] 'position').Data
        $key = '{0},{1}' -f [math]::Floor($pos[0] / $Tile), [math]::Floor($pos[1] / $Tile)
        if ($tileZone.ContainsKey($key)) {
            (F $s 'initial zone').Value = $tileZone[$key][0]; (F $s 'initial objective').Value = $tileZone[$key][1]; (F $s 'initial task').Value = -1
            Log "squad $n -> tile $key zone $($tileZone[$key][0]) objective $($tileZone[$key][1])"
        } else { Log "squad $n : no zone for tile $key" }
    }
    if ($Save) { $tf.Save(); Log 'SAVED' } else { Log 'DRY RUN (use -Save)' }
} catch { Log ('ERROR: ' + $_.Exception.ToString()) } finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
