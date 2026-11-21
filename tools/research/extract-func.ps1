# Print decompiled function bodies from a Ghidra text export (hr_000NN.c + functions_index.csv).
# Usage: extract-func.ps1 -Dir <decompiled dir> -Names 0x826ba870,0x8269aef8 [-MaxLines 250]
# A name can be an RVA (as in the index) or a substring of the function name.
param([string]$Dir, [string[]]$Names, [int]$MaxLines = 250)
$idx = Import-Csv (Join-Path $Dir 'functions_index.csv')
foreach ($n in $Names) {
    $hit = $idx | Where-Object { $_.rva -eq $n } | Select-Object -First 1
    if (-not $hit) { $hit = $idx | Where-Object { $_.name -like "*$n*" } | Select-Object -First 1 }
    if (-not $hit) { Write-Output "not in index: $n"; continue }
    $file = Join-Path $Dir $hit.file
    $rx = "^// ==== .* rva $($hit.rva) ===="
    $reader = [System.IO.StreamReader]::new($file)
    $on = $false; $count = 0
    while ($null -ne ($l = $reader.ReadLine())) {
        if (-not $on) { if ($l -match $rx) { $on = $true; Write-Output $l } ; continue }
        if ($l.StartsWith('// ==== ')) { break }
        Write-Output $l; $count++
        if ($count -ge $MaxLines) { Write-Output '... (cut)'; break }
    }
    $reader.Close()
    if (-not $on) { Write-Output "header not found: $($hit.rva) in $($hit.file)" }
}
