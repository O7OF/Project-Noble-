# Read-only: which placed objects a scenario's baked nav mesh stitched in ("ai pathfinding data" -> object refs), with
# each object's block, tag and pathfinding policy (ManagedBlam). Used in Phase 7 step 3 to learn what the bake accepts.
param([string]$Hrek = 'D:\Steam\steamapps\common\HREK', [string]$Tag = 'levels\firefight\ff10_prototype\ff10_prototype')
$ErrorActionPreference = 'Stop'
$bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
[void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
[Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) })
function F($el, [string]$name) { foreach ($f in $el.Fields) { if ($f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; return $null }
function V($f) { if ($f -is [Bungie.Tags.TagFieldEnum] -or $f -is [Bungie.Tags.TagFieldBlockIndex]) { return $f.Value } ; return $f.Data }
try {
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'scenario'))
    $want = @{}
    foreach ($bspEl in (F $tf 'ai pathfinding data').Elements) {
        foreach ($r in (F $bspEl 'object refs').Elements) {
            $oid = (F $r 'object id').Elements[0]
            $want[[string](V (F $oid 'unique id'))] = 'type ' + (V (F $oid 'type')) + ' source ' + (V (F $oid 'source'))
        }
    }
    "object refs: " + $want.Count
    foreach ($pair in @(@('scenery', 'scenery palette', 'scenery data'), @('crates', 'crate palette', 'crate data'), @('machines', 'machine palette', 'machine data'))) {
        $blk = F $tf $pair[0]; $pal = F $tf $pair[1]
        if (-not $blk) { continue }
        $n = 0; $pol = @{}
        foreach ($e in $blk.Elements) {
            $pd = (F $e 'object data').Elements[0]; $oid = (F $pd 'object id').Elements[0]
            $uid = [string](V (F $oid 'unique id'))
            $sd = F $e $pair[2]; $policy = '?'
            if ($sd) { $pf = F $sd.Elements[0] 'Pathfinding policy'; if ($pf) { $policy = $pf.Items[$pf.Value].EnumName } }
            $pol[$policy] = 1 + [int]$pol[$policy]
            if ($want.ContainsKey($uid)) {
                $ti = V (F $e 'type'); $tp = ''
                if ($ti -ge 0 -and $ti -lt $pal.Elements.Count) { $tp = (F $pal.Elements[$ti] 'name').Path.RelativePathWithExtension }
                "  REF {0} [{1}] uid {2} policy '{3}' {4}" -f $pair[0], $n, $uid, $policy, $tp
            }
            $n++
        }
        "  {0}: {1} placed; policies: {2}" -f $pair[0], $blk.Elements.Count, (($pol.GetEnumerator() | % { "$($_.Key)=$($_.Value)" }) -join ', ')
    }
} finally { [Bungie.ManagedBlamSystem]::Stop() }
