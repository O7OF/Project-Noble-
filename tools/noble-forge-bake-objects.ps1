# Phase 7 step 3: places Forge pieces in a scenario so `tool generate-pathfinding` stitches them into the nav mesh.
# DRY RUN unless -Save. Layout = JSON array of { "tag": "levels\\multi\\dlc\\objects\\x\\x", "pos": [x,y,z],
# "rot": [yaw,pitch,roll] (degrees) } where "tag" is the Forge piece's crate tag (no extension).
#
# -Kind scenery (default): `generate-pathfinding` deletes crates added to a multiplayer scenario (tested 2026-11-21:
#   crates, their palette entries and object names were all gone after the bake), so each piece is wrapped in a
#   scenery tag `objects\noble\forge_baked\<name>\<name>.scenery` (created if missing; same model, bounding radius /
#   offset copied from the crate, pathfinding policy STATIC) and placed as scenery.
# -Kind crate: places the crate tags themselves (kept for single-player scenarios / research).
# Pieces are named noble_forge_<n> (object names); -Replace removes those first. -ListFields prints the fields of
# the chosen placement blocks and stops.
#   powershell -ExecutionPolicy Bypass -Command "& tools\noble-forge-bake-objects.ps1 -Layout tools\forge-layouts\island_test.json -Save"
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag = 'levels\multi\forge_halo\forge_halo',
    [string]$Layout = '',
    [ValidateSet('scenery', 'crate')][string]$Kind = 'scenery',
    [switch]$ListFields,
    [switch]$Replace,
    [switch]$Save
)
$ErrorActionPreference = 'Stop'
if ($Layout) { $Layout = (Resolve-Path $Layout).Path }   # before Set-Location to HREK below
$out = Join-Path $Hrek 'temp\noble_forge_bake_objects_out.txt'
New-Item -ItemType Directory -Force (Split-Path $out) | Out-Null
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
function F($el, [string]$name) { foreach ($f in $el.Fields) { if ($f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; return $null }
function TopBlock($tf, [string]$name) { foreach ($f in $tf.Fields) { if ($f -is [Bungie.Tags.TagFieldBlock] -and $f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; throw "no block $name" }
function ListAll($el, [string]$ind, [int]$depth) {
    foreach ($f in $el.Fields) {
        $n = $f.GetTagFieldPath().FieldName
        $v = ''; try { if ($f.PSObject.Properties['Data']) { $v = ' = ' + (($f.Data | % { "$_" }) -join ',') } } catch { }
        if ($f -is [Bungie.Tags.TagFieldEnum]) { try { $v = ' = ' + $f.Value + ' [' + (($f.Items | % { $_.EnumName }) -join ' | ') + ']' } catch { } }
        Log ($ind + $n + ' <' + $f.GetType().Name + '>' + $v)
        if ($depth -gt 0 -and $f -is [Bungie.Tags.TagFieldStruct]) { foreach ($e in $f.Elements) { ListAll $e ($ind + '    ') ($depth - 1) } }
    }
}
function SetIndex($f, [int]$v) { if ($f.PSObject.Properties['Value']) { $f.Value = $v } else { $f.Data = $v } }
function IndexOf($f) { if ($f.PSObject.Properties['Value']) { return $f.Value } ; return $f.Data }
# Sets an enum field by the name of its option (option lists differ between tag types).
function SetEnumByName($f, [string]$pattern) {
    for ($i = 0; $i -lt $f.Items.Count; $i++) { if ($f.Items[$i].EnumName -match $pattern) { $f.Value = $i; return } }
    throw ("no enum option matching '$pattern' in " + $f.GetTagFieldPath().FieldName)
}

# Wraps a crate's model in a scenery tag (created once, reused after). Returns the scenery tag path (no extension).
function EnsureScenery([string]$crateTag) {
    $leaf = Split-Path $crateTag -Leaf
    $path = "objects\noble\forge_baked\$leaf\$leaf"
    if (Test-Path (Join-Path $Hrek "tags\$path.scenery")) { return $path }
    $crate = New-Object Bungie.Tags.TagFile
    $crate.Load([Bungie.Tags.TagPath]::FromPathAndExtension($crateTag, 'crate'))
    $co = (F $crate 'object').Elements[0]
    $sc = New-Object Bungie.Tags.TagFile
    $sc.New([Bungie.Tags.TagPath]::FromPathAndExtension($path, 'scenery'))
    $so = (F $sc 'object').Elements[0]
    (F $so 'model').Path = (F $co 'model').Path
    foreach ($fn in 'bounding radius', 'bounding offset') { $a = F $co $fn; $b = F $so $fn; if ($a -and $b) { $b.Data = $a.Data } }
    SetEnumByName (F $sc 'pathfinding policy') 'STATIC'
    $sc.Save()
    Log ("scenery tag + $path (model " + (F $co 'model').Path.RelativePath + ")")
    return $path
}

try {
    $bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    [Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) Log "CRASH $i" })
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'scenario'))
    Log ('loaded ' + $tf.Path.RelativePathWithExtension)
    $k = if ($Kind -eq 'scenery') { @{ pal = 'scenery palette'; blk = 'scenery'; data = 'scenery data'; ext = 'scenery'; type = 6 } }
         else { @{ pal = 'crate palette'; blk = 'crates'; data = 'crate data'; ext = 'crate'; type = 10 } }
    $pal = TopBlock $tf $k.pal
    $objs = TopBlock $tf $k.blk
    $names = TopBlock $tf 'object names'
    Log ("{0} {1}, {2} {3}, object names {4}" -f $k.pal, $pal.Elements.Count, $k.blk, $objs.Elements.Count, $names.Elements.Count)

    if ($ListFields) {
        if ($objs.Elements.Count -gt 0) { $el = $objs.Elements[0] } else { $el = $objs.AddElement() }   # no `$x = if`: it unrolls the element
        Log "$($k.blk)[0]:"; ListAll $el '  ' 2
        Log 'LIST ONLY (nothing saved)'
        return
    }

    if (-not $Layout) { throw 'no -Layout' }
    $pieces = Get-Content $Layout -Raw | ConvertFrom-Json

    # Instances added by this script carry unique ids -0x4E000000 - n (below -0x4E000000 + 1, above -0x4F000000).
    if ($Replace) {
        for ($i = $objs.Elements.Count - 1; $i -ge 0; $i--) {
            $oid = (F (F $objs.Elements[$i] 'object data').Elements[0] 'object id').Elements[0]
            $u = [int](F $oid 'unique id').Data
            if ($u -le -0x4E000000 -and $u -gt -0x4F000000) { $objs.RemoveElement($i); Log "$($k.blk) removed [$i]" }
        }
    }

    $palIndex = @{}
    for ($i = 0; $i -lt $pal.Elements.Count; $i++) { $r = F $pal.Elements[$i] 'name'; if ($r.Path) { $palIndex[$r.Path.RelativePath] = $i } }
    $n = 0
    foreach ($p in $pieces) {
        $crateTag = [string]$p.tag
        if (-not (Test-Path (Join-Path $Hrek ("tags\" + $crateTag + '.crate')))) { Log "missing tag $crateTag (skipped)"; continue }
        $t = if ($Kind -eq 'scenery') { EnsureScenery $crateTag } else { $crateTag }
        if (-not $palIndex.ContainsKey($t)) {
            $pe = $pal.AddElement()
            (F $pe 'name').Path = [Bungie.Tags.TagPath]::FromPathAndExtension($t, $k.ext)
            $palIndex[$t] = $pal.Elements.Count - 1
            Log ("palette + [" + ($pal.Elements.Count - 1) + "] $t")
        }
        $label = 'noble_forge_{0}' -f $n
        $c = $objs.AddElement()
        SetIndex (F $c 'type') $palIndex[$t]
        SetIndex (F $c 'name') -1   # no object name, like the stock scenery; pieces are found by their unique id
        $pd = (F $c 'object data').Elements[0]
        (F $pd 'position').Data = [single[]]@([single]$p.pos[0], [single]$p.pos[1], [single]$p.pos[2])
        $rot = @(0, 0, 0); if ($p.rot) { $rot = $p.rot }
        (F $pd 'rotation').Data = [single[]]@([single]$rot[0], [single]$rot[1], [single]$rot[2])
        $oid = (F $pd 'object id').Elements[0]
        (F $oid 'unique id').Data = [int](-0x4E000000 - $n)   # negative like the editor's ids; 'N' prefix stays clear of them
        if ($Kind -eq 'scenery') { (F (F $c 'multiplayer data').Elements[0] 'owner team').Value = 8 }   # neutral, as the stock scenery
        (F $oid 'type').Value = $k.type
        (F $oid 'source').Value = 1                            # editor
        # STATIC: the bake cuts the piece's collision into the nav mesh (walkable tops, blocked footprint).
        SetEnumByName (F (F $c $k.data).Elements[0] 'Pathfinding policy') 'STATIC'
        Log ("$($k.ext) + $label $t at " + ($p.pos -join ',') + " rot " + ($rot -join ','))
        $n++
    }
    Log ("{0} now {1}, {2} now {3}" -f $k.pal, $pal.Elements.Count, $k.blk, $objs.Elements.Count)
    if ($Save) { $tf.Save(); Log 'SAVED' } else { Log 'DRY RUN (use -Save)' }
} catch { Log ('ERROR: ' + $_.Exception.ToString()) } finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
