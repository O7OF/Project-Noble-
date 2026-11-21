# Lists functions (from "// ==== name  rva X ====" headers) whose bodies match EVERY regex given, with body length.
# Usage: funcs-matching-all.ps1 -Dir <decompiled dir> -Patterns '\* 0xd00','\* 0xec' [-MaxLines 400]
param([string]$Dir, [string[]]$Patterns, [int]$MaxLines = 100000)
$rx = $Patterns | ForEach-Object { [regex]$_ }
foreach ($file in Get-ChildItem $Dir -Filter 'hr_*.c' | Sort-Object Name) {
    $reader = [System.IO.StreamReader]::new($file.FullName)
    $cur = $null; $hits = New-Object bool[] $rx.Count; $n = 0
    $flush = { if ($cur -and ($hits -notcontains $false) -and $n -le $MaxLines) { Write-Output ("{0}  ({1} lines)" -f $cur, $n) } }
    while ($null -ne ($l = $reader.ReadLine())) {
        if ($l.StartsWith('// ==== ')) { & $flush; $cur = $l.Substring(8).TrimEnd(' ', '='); $hits = New-Object bool[] $rx.Count; $n = 0; continue }
        $n++
        for ($k = 0; $k -lt $rx.Count; $k++) { if (-not $hits[$k] -and $rx[$k].IsMatch($l)) { $hits[$k] = $true } }
    }
    & $flush
    $reader.Close()
}
