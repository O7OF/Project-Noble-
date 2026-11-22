# Read-only: every string id and its English text in a multilingual_unicode_string_list (ManagedBlam).
#   powershell -ExecutionPolicy Bypass -File tools\research\string-list-dump.ps1 -Tag ui\cui\strings\forge_strings [-Match regex]
# Log: HREK\temp\noble_string_list_dump.txt (id<TAB>text per line).
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag = 'ui\cui\strings\forge_strings',
    [string]$Match = ''
)
$ErrorActionPreference = 'Stop'
$out = Join-Path $Hrek 'temp\noble_string_list_dump.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
function Name($f) { try { return $f.GetTagFieldPath().FieldName } catch { return '?' } }
try {
    $bin = Join-Path $Hrek 'bin'
    $env:PATH = "$bin;$bin\x64;" + $env:PATH
    Set-Location $Hrek
    [void][Reflection.Assembly]::LoadFrom((Join-Path $bin 'ManagedBlam.dll'))
    $cb = [Bungie.ManagedBlamCrashCallback]{ param($info) Log ('CRASH: ' + $info) }
    [Bungie.ManagedBlamSystem]::Start($Hrek, $cb)
    $tf = New-Object Bungie.Tags.TagFile
    $tf.Load([Bungie.Tags.TagPath]::FromPathAndExtension($Tag, 'multilingual_unicode_string_list'))
    $refs = $null; $data = $null
    foreach ($f in $tf.Fields) {
        if ((Name $f) -eq 'string references') { $refs = $f }
        if ((Name $f) -match 'string data') { $data = $f }
    }
    if (-not $refs) { throw 'no string references block' }
    Log ('fields of element 0: ' + (($refs.Elements[0].Fields | % { (Name $_) + '<' + $_.GetType().Name + '>' }) -join ', '))
    if ($data) { Log ('string data field: ' + (Name $data) + ' <' + $data.GetType().Name + '>') }
    # Text through the unicode string list API when ManagedBlam offers it, else the raw English offset.
    $bytes = $null
    if ($data) {
        foreach ($p in 'Data','DataAsByteArray','GetData') { if (-not $bytes -and $data.PSObject.Properties[$p]) { $bytes = [byte[]]$data.$p } }
        if (-not $bytes) { try { $bytes = [byte[]]$data.GetData() } catch { } }
        Log ('string data bytes via ' + $(foreach ($q in 'Data','DataAsByteArray') { if ($data.PSObject.Properties[$q] -and $data.$q) { $q } }) + ': ' + $(if ($bytes) { $bytes.Length } else { 'none; members ' + (($data | Get-Member -MemberType Property,Method | % Name) -join ',') }))
    }
    foreach ($e in $refs.Elements) {
        $id = ''; $off = -1
        foreach ($f in $e.Fields) {
            $n = Name $f
            if ($n -eq 'string id') { $id = "$($f.Data)" }
            elseif ($n -match '^english') { $off = [int]$f.Data }
            elseif ($f -is [Bungie.Tags.TagFieldStruct] -and $off -lt 0) {
                foreach ($s in $f) { foreach ($g in $s.Fields) { if ((Name $g) -match '^english' ) { $off = [int]$g.Data } } }
            }
        }
        $text = ''
        if ($bytes -and $off -ge 0 -and $off -lt $bytes.Length) {
            $end = [Array]::IndexOf($bytes, [byte]0, $off); if ($end -lt 0) { $end = $bytes.Length }
            $text = [Text.Encoding]::UTF8.GetString($bytes, $off, $end - $off)
        }
        if (-not $Match -or $id -match $Match -or $text -match $Match) { Log ("$id`t$text") }
    }
    Log 'DONE'
}
catch { Log ('ERROR: ' + $_.Exception.ToString()) }
finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
