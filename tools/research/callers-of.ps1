# Lists the functions (from "// ==== name  rva X ====" headers) whose bodies contain a pattern, in a Ghidra text export.
# Usage: callers-of.ps1 -Dir <decompiled dir> -Pattern '_game_is_campaign_or_survival__YA_NXZ'
param([string]$Dir, [string]$Pattern)
$rx = [regex]$Pattern
foreach ($file in Get-ChildItem $Dir -Filter 'hr_*.c' | Sort-Object Name) {
    $reader = [System.IO.StreamReader]::new($file.FullName)
    $cur = ''; $hit = $false
    while ($null -ne ($l = $reader.ReadLine())) {
        if ($l.StartsWith('// ==== ')) {
            if ($hit) { Write-Output $cur }
            $cur = $l.Substring(8).TrimEnd(' ', '='); $hit = $false
            continue
        }
        if (-not $hit -and $rx.IsMatch($l)) {
            if ($l -notmatch '^\S.*\(' -or $l.Contains(';')) { $hit = $true }
        }
    }
    if ($hit) { Write-Output $cur }
    $reader.Close()
}
