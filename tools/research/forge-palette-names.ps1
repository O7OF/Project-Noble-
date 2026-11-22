# Read-only: Forge palette names. Lists the scenario's top-level blocks (with counts), dumps the entries of the
# palette blocks named by -Palettes (name string_id and the variant names), and dumps a string list's ids and text.
#   powershell -ExecutionPolicy Bypass -File tools\research\forge-palette-names.ps1
# Log: HREK\temp\noble_forge_palette_names.txt
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Scenario = 'levels\multi\forge_halo\forge_halo',
    [string]$StringList = 'ui\cui\strings\forge_strings',
    [string]$Palettes = 'palette',
    [string]$Find = 'pelican|spectre|boathog|beacon|laser|airstrike|orbital|banshee'
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_forge_palette_names.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
function Name($f) { try { return $f.GetTagFieldPath().FieldName } catch { return '?' } }
function Val($f) {
    try {
        if ($f -is [Bungie.Tags.TagFieldReference]) { return 'ref ' + $(if ($f.Path) { $f.Path.RelativePathWithExtension } else { 'none' }) }
        if ($f -is [Bungie.Tags.TagFieldBlock]) { return 'block count=' + $f.Elements.Count }
        if ($f -is [Bungie.Tags.TagFieldBlockIndex]) { return 'index ' + $f.Value }
        if ($f -is [Bungie.Tags.TagFieldEnum]) { return 'enum ' + $f.Value }
        if ($f.PSObject.Properties['Data']) { $d = $f.Data; if ($d -is [array]) { if ($d.Count -gt 64) { return "data[$($d.Count)]" } ; return ($d -join ',') } ; return "$d" }
    } catch { return '?' }
    return ''
}
function Walk($fields, [string]$ind, [int]$d) {
    foreach ($f in $fields) {
        Log ($ind + (Name $f) + ' = ' + (Val $f))
        if ($d -gt 0 -and $f -is [Bungie.Tags.TagFieldStruct]) { foreach ($e in $f) { Walk $e.Fields ($ind + '    ') ($d - 1) } }
        if ($d -gt 0 -and $f -is [Bungie.Tags.TagFieldBlock]) {
            for ($i = 0; $i -lt $f.Elements.Count; $i++) { Log ($ind + "  [$i]"); Walk $f.Elements[$i].Fields ($ind + '    ') ($d - 1) }
        }
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
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Scenario, 'scenario'))
    Log ('== scenario blocks: ' + $tf.Path.RelativePathWithExtension)
    foreach ($f in $tf.Fields) { if ($f -is [Bungie.Tags.TagFieldBlock]) { Log ('  ' + (Name $f) + ' count=' + $f.Elements.Count) } }
    foreach ($f in $tf.Fields) {
        if ($f -is [Bungie.Tags.TagFieldBlock] -and (Name $f) -match $Palettes -and $f.Elements.Count -gt 0) {
            Log ('== ' + (Name $f) + ' count=' + $f.Elements.Count)
            for ($i = 0; $i -lt $f.Elements.Count; $i++) { Log "  [$i]"; Walk $f.Elements[$i].Fields '    ' 2 }
        }
    }
    $tf.Dispose()

    if ($StringList) {
        $sl = New-Object Bungie.Tags.TagFile
        $sl.Load([Bungie.Tags.TagPath]::FromPathAndExtension($StringList, 'multilingual_unicode_string_list'))
        Log ('== string list: ' + $sl.Path.RelativePathWithExtension)
        Walk $sl.Fields '  ' 0
        $sl.Dispose()
    }
    Log 'DONE'
}
catch { Log ('ERROR: ' + $_.Exception.ToString()) }
finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
