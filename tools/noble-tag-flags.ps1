# Read-only: prints the flag names (set / not set) of every flags field in elements of one top-level block.
#   powershell -ExecutionPolicy Bypass -File tools\noble-tag-flags.ps1 -Tag levels\multi\forge_halo\forge_halo -Ext scenario -Block "structure bsps"
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag, [string]$Ext = 'scenario', [string]$Block = 'structure bsps'
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_tag_flags_out.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
try {
    $bin = Join-Path $Hrek 'bin'; $env:PATH = "$bin;$bin\x64;" + $env:PATH; Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    [Bungie.ManagedBlamSystem]::Start($Hrek, [Bungie.ManagedBlamCrashCallback]{ param($i) Log "CRASH $i" })
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, $Ext))
    $sets = @()
    if ($Block) { foreach ($f in $tf.Fields) { if ($f -is [Bungie.Tags.TagFieldBlock] -and $f.GetTagFieldPath().FieldName -eq $Block) { $k = 0; foreach ($e in $f.Elements) { $sets += ,@("[$k]", $e.Fields); $k++ } } } }
    else { $sets += ,@('top', $tf.Fields) }
    $first = $true
    foreach ($s in $sets) {
        foreach ($f in $s[1]) {
            if (-not ($f -is [Bungie.Tags.TagFieldFlags])) { continue }
            if ($first) { Log ('members: ' + (($f | Get-Member -MemberType Property | % Name) -join ',')); $first = $false }
            $items = @()
            foreach ($it in $f.Items) {
                if ($items.Count -eq 0) { Log ('item members: ' + (($it | Get-Member -MemberType Property | % Name) -join ',')) }
                $items += $it
            }
            $txt = ($items | % { $nm = $_.FlagName; if (-not $nm) { $nm = $_.Name } ; $on = $_.IsSet; "$nm=$on" }) -join '; '
            Log ($s[0] + ' ' + $f.GetTagFieldPath().FieldName + ' raw=' + $f.RawValue + ' :: ' + $txt)
        }
    }
    Log 'DONE'
} catch { Log ('ERROR: ' + $_.Exception.ToString()) } finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
