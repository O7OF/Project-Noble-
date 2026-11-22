# Adds Forge menu names to ui\cui\strings\forge_strings (the string list the native Forge palette reads its entry and
# variant names from). A palette entry or variant whose string id has no text here shows a blank name.
# Dry run unless -Save. Skips ids that already exist. Backs the tag up into HREK\noble_mods\forge_strings_backup\ first.
#   powershell -ExecutionPolicy Bypass -File tools\noble-forge-add-strings.ps1 [-Save]
# Then rebuild the map: tool build-cache-file levels\multi\forge_halo\forge_halo (no pathfinding bake needed).
param(
    [string]$Hrek = 'D:\Steam\steamapps\common\HREK',
    [string]$Tag = 'ui\cui\strings\forge_strings',
    [switch]$Save
)
$ErrorActionPreference = 'Stop'

# string id -> English text. Ids are the `name` / variant `display name` values in forge_halo.scenario's map variant
# palettes (tools\research\forge-palette-names.ps1 lists them).
$Strings = [ordered]@{
    'pelican'                       = 'Pelican'
    'pelican_door'                  = 'Pelican (Doors)'
    'spectre'                       = 'Spectre'
    'spectre_classic'               = 'Spectre (Classic)'
    'spectre_needler'               = 'Needler Spectre'
    'spectre_antiair'               = 'Anti-Air Spectre'
    'spectre_armored'               = 'Armored Spectre'
    'boathog'                       = 'Boathog'
    'boathog_gauss'                 = 'Gauss Boathog'
    'boathog_rocket'                = 'Rocket Boathog'
    'boathog_troop'                 = 'Troop Boathog'
    'target_laser'                  = 'UNSC Orbital Beacon'
    'target_laser_longsword'        = 'Longsword Strike'
    'target_laser_carpet_bomb'      = 'Longsword Carpet Bomb'
    'target_laser_mac'              = 'MAC Round'
    'target_laser_pelican_warthog'  = 'Pelican Drop: Warthog'
    'target_laser_pelican_scorpion' = 'Pelican Drop: Scorpion'
    'target_laser_pelican_weapons'  = 'Pelican Drop: Weapons'
    'target_laser_covenant'         = 'Covenant Orbital Beacon'
    'target_laser_corvette'         = 'Corvette Bombing'
    'target_laser_seraph'           = 'Seraph Strike'
    'target_laser_spirit_ghost'     = 'Spirit Drop: Ghost'
    'target_laser_spirit_wraith'    = 'Spirit Drop: Wraith'
    'target_laser_glassing'         = 'Glassing Beam'
    'target_laser_halo_array'       = 'Halo Array'
    'target_laser_game_ender'       = 'Game Ender'
}

$out = Join-Path $Hrek 'temp\noble_forge_add_strings.txt'
'' | Out-File $out -Encoding utf8
function Log([string]$m) { $m | Out-File $out -Append -Encoding utf8; Write-Host $m }
function Name($f) { try { return $f.GetTagFieldPath().FieldName } catch { return '?' } }
try {
    $tagFile = Join-Path $Hrek ('tags\' + $Tag + '.multilingual_unicode_string_list')
    if ($Save) {
        $bk = Join-Path $Hrek 'noble_mods\forge_strings_backup'
        New-Item -ItemType Directory -Force $bk | Out-Null
        $dest = Join-Path $bk ('forge_strings.multilingual_unicode_string_list.' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
        Copy-Item $tagFile $dest
        Log "backup $dest"
    }
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
        if ((Name $f) -eq 'string data utf8') { $data = $f }
    }
    if (-not $refs -or -not $data) { throw 'string references / string data utf8 not found' }
    $raw = [byte[]]$data.GetData()
    $ms = New-Object System.IO.MemoryStream
    $ms.Write($raw, 0, $raw.Length)
    Log ("before: $($refs.Elements.Count) strings, $($raw.Length) bytes")

    $existing = @{}
    foreach ($e in $refs.Elements) { foreach ($f in $e.Fields) { if ((Name $f) -eq 'string id') { $existing["$($f.Data)"] = $true } } }

    $added = 0
    foreach ($id in $Strings.Keys) {
        if ($existing.ContainsKey($id)) { Log "skip $id (exists)"; continue }
        $off = [int]$ms.Length
        $tb = [Text.Encoding]::UTF8.GetBytes($Strings[$id] + [char]0)
        $ms.Write($tb, 0, $tb.Length)
        $e = $refs.AddElement()
        foreach ($f in $e.Fields) {
            $n = Name $f
            if ($n -eq 'string id') { $f.Data = $id }
            elseif ($n -match ' offset$') { $f.Data = [int]$off }   # English text for every language
        }
        Log ("add $id = '" + $Strings[$id] + "' @ $off")
        $added++
    }
    $data.SetData($ms.ToArray())
    Log ("after: $($refs.Elements.Count) strings, $($ms.Length) bytes; added $added")
    if ($Save -and $added -gt 0) { $tf.Save(); Log 'SAVED' } elseif (-not $Save) { Log 'dry run (use -Save)' }
    $tf.Dispose()
    Log 'DONE'
}
catch { Log ('ERROR: ' + $_.Exception.ToString()) }
finally { try { [Bungie.ManagedBlamSystem]::Stop() } catch { } }
