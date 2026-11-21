# Sets the raw value of a flags field in one element of a top-level block (ManagedBlam). DRY RUN unless -Save.
#   powershell -ExecutionPolicy Bypass -File tools\noble-tag-set-flags.ps1 -Tag levels\multi\forge_halo\forge_halo -Ext scenario `
#       -Block "structure bsps" -Index 0 -Field flags -Clear 512 -Save
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag, [string]$Ext = 'scenario', [string]$Block, [int]$Index = 0, [string]$Field = 'flags',
    [long]$Set = 0, [long]$Clear = 0, [switch]$Save
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_tag_set_flags_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
try {
    $bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    [Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) Log "CRASH $i" })
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, $Ext))
    $fields = $tf.Fields   # -Block '' = a top-level flags field
    if ($Block) {
        $blk = $null
        foreach ($f in $tf.Fields) { if ($f -is [Bungie.Tags.TagFieldBlock] -and $f.GetTagFieldPath().FieldName -eq $Block) { $blk = $f } }
        if (-not $blk) { throw "block '$Block' not found" }
        $fields = $blk.Elements[$Index].Fields
    }
    $fl = $null
    foreach ($f in $fields) { if ($f -is [Bungie.Tags.TagFieldFlags] -and $f.GetTagFieldPath().FieldName -eq $Field) { $fl = $f } }
    if (-not $fl) { throw "flags field '$Field' not found" }
    $before = [long]$fl.RawValue
    $after = ($before -bor $Set) -band (-bnot $Clear)
    $fl.RawValue = [uint32]$after
    Log ("$Block[$Index].$Field raw $before -> " + $fl.RawValue)
    if ($Save) { $tf.Save(); Log 'SAVED' } else { Log 'DRY RUN (use -Save)' }
} catch { Log ('ERROR: ' + $_.Exception.ToString()) } finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
