# Read-only: prints every field (with values) of the elements of one top-level block of a tag via ManagedBlam.
# Log: HREK\temp\noble_tag_dump_block_out.txt (also echoed).
#   powershell -ExecutionPolicy Bypass -File tools\noble-tag-dump-block.ps1 -Tag levels\multi\forge_halo\forge_halo -Ext scenario -Block "structure bsps"
#   -Block "" dumps the tag's top-level non-block fields (flags, enums, values).
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag,
    [string]$Ext = 'scenario',
    [string]$Block = '',
    [int]$MaxElements = 4,
    [int]$Depth = 1
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_tag_dump_block_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
function Val($f) {
    try {
        if ($f -is [Bungie.Tags.TagFieldFlags]) {
            $names = @(); $i = 0
            foreach ($n in $f.Items) { if ($n.IsSet) { $names += $n.FlagName } ; $i++ }
            return 'flags[' + ($names -join '|') + '] raw=' + $f.RawValue
        }
        if ($f -is [Bungie.Tags.TagFieldEnum]) { return 'enum ' + $f.Value + ' (' + $f.Items[$f.Value].EnumName + ')' }
        if ($f -is [Bungie.Tags.TagFieldReference]) { return 'ref ' + $(if ($f.Path) { $f.Path.RelativePathWithExtension } else { 'none' }) }
        if ($f -is [Bungie.Tags.TagFieldBlock]) { return 'block count=' + $f.Elements.Count }
        if ($f.PSObject.Properties['Data']) { $d = $f.Data; if ($d -is [array]) { return ($d -join ',') } ; return "$d" }
    } catch { return '?' }
    return ''
}
function Walk($fields, [string]$ind, [int]$d) {
    foreach ($f in $fields) {
        $n = '?'; try { $n = $f.GetTagFieldPath().FieldName } catch { }
        Log ($ind + $n + ' <' + $f.GetType().Name + '> ' + (Val $f))
        if ($d -gt 0 -and $f -is [Bungie.Tags.TagFieldStruct]) { foreach ($e in $f) { Walk $e.Fields ($ind + '    ') ($d - 1) } }
        if ($d -gt 0 -and $f -is [Bungie.Tags.TagFieldBlock] -and $f.Elements.Count -gt 0) { Log ($ind + '  (element 0 of ' + $f.Elements.Count + ')'); Walk $f.Elements[0].Fields ($ind + '    ') ($d - 1) }
    }
}
try {
    $bin = Join-Path $Hrek 'bin'
    $env:PATH = "$bin;$bin\x64;" + $env:PATH
    Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    $cb = [Bungie.ManagedBlamCrashCallback]{ param($info) Log ('CRASH: ' + $info) }
    [Bungie.ManagedBlamSystem]::Start($Hrek, $cb)
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, $Ext))
    Log ('loaded ' + $tf.Path.RelativePathWithExtension)
    if (-not $Block) {
        foreach ($f in $tf.Fields) { if (-not ($f -is [Bungie.Tags.TagFieldBlock])) { $n = $f.GetTagFieldPath().FieldName; Log ($n + ' <' + $f.GetType().Name + '> ' + (Val $f)) } }
    } else {
        $blk = $null
        foreach ($f in $tf.Fields) { if ($f -is [Bungie.Tags.TagFieldBlock] -and $f.GetTagFieldPath().FieldName -eq $Block) { $blk = $f } }
        if (-not $blk) { throw "block '$Block' not found" }
        Log ("$Block count=" + $blk.Elements.Count)
        for ($i = 0; $i -lt [Math]::Min($MaxElements, $blk.Elements.Count); $i++) {
            Log ("[$i]"); Walk $blk.Elements[$i].Fields '    ' $Depth
        }
    }
    Log 'DONE'
}
catch { Log ('ERROR: ' + $_.Exception.ToString()) }
finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
