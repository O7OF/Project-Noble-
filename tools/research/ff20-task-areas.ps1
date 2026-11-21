# Read-only research: prints objective 0's zone index and the first task that has areas (all fields + its area
# elements), from a scenario tag via ManagedBlam. Log: HREK\temp\noble_task_areas_out.txt
param([string]$Hrek = 'D:\Steam\steamapps\common\HREK', [string]$Tag = 'levels\firefight\ff20_courtyard\ff20_courtyard')
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_task_areas_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
function Val($f) {
    try {
        if ($f -is [Bungie.Tags.TagFieldBlockIndex]) { return 'index ' + $f.Value }
        if ($f -is [Bungie.Tags.TagFieldBlock]) { return 'block count=' + $f.Elements.Count }
        if ($f -is [Bungie.Tags.TagFieldFlags]) { return 'flags raw=' + $f.RawValue }
        if ($f -is [Bungie.Tags.TagFieldEnum]) { return 'enum ' + $f.Value }
        if ($f.PSObject.Properties['Data']) { $d = $f.Data; if ($d -is [array]) { return ($d -join ',') } ; return "$d" }
    } catch { return '?' }
    return ''
}
function Dump($el, [string]$ind) { foreach ($f in $el.Fields) { Log ($ind + $f.GetTagFieldPath().FieldName + ' <' + $f.GetType().Name + '> ' + (Val $f)) } }
function F($el, [string]$name) { foreach ($f in $el.Fields) { if ($f.GetTagFieldPath().FieldName -eq $name) { return ,$f } } ; return $null }
try {
    $bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    [Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) Log "CRASH $i" })
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'scenario'))
    $objs = $null; foreach ($f in $tf.Fields) { if ($f.GetTagFieldPath().FieldName -eq 'ai objectives') { $objs = $f } }
    $o = $objs.Elements[0]
    Log ('objective 0 zone index: ' + (Val (F $o 'zone index')) + '  first task: ' + (Val (F $o 'first task index')))
    $tasks = F $o 'tasks'
    for ($i = 0; $i -lt $tasks.Elements.Count; $i++) {
        $t = $tasks.Elements[$i]
        $areas = F $t 'areas'
        if ($areas.Elements.Count -gt 0) {
            Log "task [$i]:"; Dump $t '    '
            Log "  area 0 of $($areas.Elements.Count):"; Dump $areas.Elements[0] '      '
            break
        }
    }
    $sq = $null; foreach ($f in $tf.Fields) { if ($f.GetTagFieldPath().FieldName -eq 'squads') { $sq = $f } }
    $s0 = $sq.Elements[0]
    Log ('squad 0 initial zone ' + (Val (F $s0 'initial zone')) + ' objective ' + (Val (F $s0 'initial objective')) + ' task ' + (Val (F $s0 'initial task')))
    Log 'DONE'
} catch { Log ('ERROR: ' + $_.Exception.ToString()) } finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
