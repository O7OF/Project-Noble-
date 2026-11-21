# Sets a top-level enum field of a tag (ManagedBlam). DRY RUN unless -Save.
#   powershell -ExecutionPolicy Bypass -File tools\noble-tag-set-enum.ps1 -Tag levels\multi\forge_halo\forge_halo -Ext scenario -Field type -Value 0 -Save
param([string]$Hrek = 'D:\Steam\steamapps\common\HREK', [string]$Tag, [string]$Ext = 'scenario', [string]$Field, [int]$Value, [switch]$Save)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_tag_set_enum_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
try {
    $bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    [Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) Log "CRASH $i" })
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, $Ext))
    $f = $null; foreach ($x in $tf.Fields) { if ($x -is [Bungie.Tags.TagFieldEnum] -and $x.GetTagFieldPath().FieldName -eq $Field) { $f = $x } }
    if (-not $f) { throw "enum '$Field' not found" }
    $before = $f.Value; $f.Value = $Value
    Log ("$Field $before -> " + $f.Value)
    if ($Save) { $tf.Save(); Log 'SAVED' } else { Log 'DRY RUN (use -Save)' }
} catch { Log ('ERROR: ' + $_.Exception.ToString()) } finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
