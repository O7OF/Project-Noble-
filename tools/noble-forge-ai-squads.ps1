# Phase 7: adds AI characters and test squads to a scenario (Forge World) with ManagedBlam. DRY RUN unless -Save.
# Each squad: one designer cell (character palette index, count) and N spawn points in a ring around a centre point.
# Re-runs skip squads / palette entries that already exist (matched by name / reference).
#   powershell -ExecutionPolicy Bypass -File tools\noble-forge-ai-squads.ps1            (dry run, prints the fields)
#   powershell -ExecutionPolicy Bypass -File tools\noble-forge-ai-squads.ps1 -Save
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag = 'levels\multi\forge_halo\forge_halo',
    [double[]]$Centre = @(-157.351, -224.8386, 4.8),
    [switch]$Save
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_forge_ai_squads_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }

$characters = @('objects\characters\elite\ai\elite', 'objects\characters\grunt\ai\grunt', 'objects\characters\jackal\ai\jackal',
                'objects\characters\brute\ai\brute', 'objects\characters\marine\ai\trooper')
# name, character (index into $characters), count, ring radius (m), angle offset (deg), distance from centre (m)
$squads = @(
    @{ name = 'noble_ai_elite';   ch = 0; n = 2; dist = 6; ang = 0   },
    @{ name = 'noble_ai_grunt';   ch = 1; n = 4; dist = 6; ang = 72  },
    @{ name = 'noble_ai_jackal';  ch = 2; n = 3; dist = 6; ang = 144 },
    @{ name = 'noble_ai_brute';   ch = 3; n = 2; dist = 6; ang = 216 },
    @{ name = 'noble_ai_trooper'; ch = 4; n = 3; dist = 6;  ang = 288 }
)

function F($el, [string]$name) { foreach ($f in $el.Fields) { if ($f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; return $null }
function TopBlock($tf, [string]$name) { foreach ($f in $tf.Fields) { if ($f -is [Bungie.Tags.TagFieldBlock] -and $f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; throw "no block $name" }
function ListFields($el, [string]$ind) { foreach ($f in $el.Fields) { Log ($ind + $f.GetTagFieldPath().FieldName + ' <' + $f.GetType().Name + '>') } }
function SetIndex($f, [int]$v) {
    if ($f.PSObject.Properties['Value']) { $f.Value = $v } else { $f.Data = $v }
}

try {
    $bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    [Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) Log "CRASH $i" })
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'scenario'))
    Log ('loaded ' + $tf.Path.RelativePathWithExtension)

    # Character palette.
    $pal = TopBlock $tf 'character palette'
    $palIndex = @{}
    for ($i = 0; $i -lt $pal.Elements.Count; $i++) { $r = F $pal.Elements[$i] 'reference'; if ($r.Path) { $palIndex[$r.Path.RelativePath] = $i } }
    foreach ($c in $characters) {
        if ($palIndex.ContainsKey($c)) { continue }
        $el = $pal.AddElement()
        (F $el 'reference').Path = [Bungie.Tags.TagPath]::FromPathAndExtension($c, 'character')
        $palIndex[$c] = $pal.Elements.Count - 1
        Log ("palette + [" + ($pal.Elements.Count - 1) + "] $c")
    }

    $sq = TopBlock $tf 'squads'
    $have = @{}
    foreach ($e in $sq.Elements) { $have[(F $e 'name').Data] = $true }
    $first = $true
    foreach ($s in $squads) {
        if ($have.ContainsKey($s.name)) { Log ("squad exists: " + $s.name); continue }
        $el = $sq.AddElement()
        (F $el 'name').Data = $s.name
        $a0 = $s.ang * [math]::PI / 180.0
        $cx = $Centre[0] + $s.dist * [math]::Cos($a0); $cy = $Centre[1] + $s.dist * [math]::Sin($a0)
        $yaw = [single](($s.ang + 180.0) % 360.0)   # face the centre
        $pts = F $el 'spawn points'
        for ($k = 0; $k -lt $s.n; $k++) {
            $p = $pts.AddElement()
            if ($first) { Log 'spawn point fields:'; ListFields $p '    ' }
            $th = 2.0 * [math]::PI * $k / [math]::Max(1, $s.n)
            $px = $cx + 1.5 * [math]::Cos($th); $py = $cy + 1.5 * [math]::Sin($th)
            $pos = [single[]]@($px, $py, $Centre[2])
            (F $p 'name').Data = ('{0}_{1}' -f $s.name, $k)
            (F $p 'position').Data = $pos
            SetIndex (F $p 'cell') 0
            $fc = F $p 'facing (yaw, pitch)'; if ($fc) { $fc.Data = [single[]]($yaw, 0) }
            $first = $false
        }
        $designer = F $el 'designer'
        $cells = $null; foreach ($e in $designer) { $cells = F $e 'cells' }
        $cell = $cells.AddElement()
        if ($s.name -eq 'noble_ai_elite') { Log 'cell fields:'; ListFields $cell '    ' }
        (F $cell 'name').Data = ($s.name + '_cell')
        (F $cell 'normal diff count').Data = [int16]$s.n
        $ct = (F $cell 'character type').AddElement()
        if ($s.name -eq 'noble_ai_elite') { Log 'character type fields:'; ListFields $ct '    '; $ix = F $ct 'character type'; Log ('  index members: ' + (($ix | Get-Member -MemberType Property | % Name) -join ',')) }
        SetIndex (F $ct 'character type') $palIndex[$characters[$s.ch]]
        (F $ct 'chance').Data = [int16]1
        Log ("squad + " + $s.name + ": " + $s.n + " x " + $characters[$s.ch] + " (palette " + $palIndex[$characters[$s.ch]] + ") at " + [int]$cx + "," + [int]$cy)
    }
    Log ("character palette now " + $pal.Elements.Count + ", squads now " + $sq.Elements.Count)
    if ($Save) { $tf.Save(); Log 'SAVED' } else { Log 'DRY RUN (use -Save)' }
} catch { Log ('ERROR: ' + $_.Exception.ToString()) } finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
