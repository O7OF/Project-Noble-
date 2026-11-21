# Read-only: lists the fields of a tag via ManagedBlam (depth-limited). Log: HREK\temp\noble_tag_fields_out.txt
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag  = 'levels\ui\mainmenu\resources\mainmenu',
    [string]$Ext  = 'scenario_cinematics_resource',
    [int]$Depth = 2
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_tag_fields_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8 }
function Walk($fields, [string]$ind, [int]$d) {
    foreach ($f in $fields) {
        $n = ''; try { $n = $f.GetTagFieldPath().FieldName } catch { $n = '?' }
        $extra = ''
        if ($f -is [Bungie.Tags.TagFieldBlock]) { $extra = ' count=' + $f.get_Elements().get_Count() }
        Log ($ind + $n + '  <' + $f.GetType().Name + '>' + $extra)
        if ($d -gt 0) {
            if ($f -is [Bungie.Tags.TagFieldStruct]) { foreach ($e in $f) { Walk $e.get_Fields() ($ind + '    ') ($d - 1) } }
            elseif ($f -is [Bungie.Tags.TagFieldBlock]) { if ($f.get_Elements().get_Count() -gt 0) { Walk $f.get_Elements().get_Item(0).get_Fields() ($ind + '    ') ($d - 1) } }
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
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, $Ext))
    Log ('loaded ' + $tf.Path.RelativePathWithExtension)
    Walk $tf.get_Fields() '' $Depth
    Log 'DONE'
}
catch { Log ('ERROR: ' + $_.Exception.ToString()) }
finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
