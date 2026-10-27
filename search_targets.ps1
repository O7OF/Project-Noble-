
$data = [System.IO.File]::ReadAllText('C:\Users\Chey1\Desktop\Halo Reach Community Edition\docs\decompiled\host-probe.exe\decompile-index.json')
$json = ConvertFrom-Json $data
$targets = @('host::hit', 'new_launch_block', 'set_map', 'main', 'run', 'detour', 'patch', 'hook', 'load', 'engine')
foreach ($t in $targets) {
    $found = $json.functions | Where-Object { $_.name -like "*$t*" }
    Write-Output "=== Search: $t (Found $($found.Count)) ==="
    $found | Select-Object -First 10 address, name, relative_path | Format-Table -AutoSize
}
