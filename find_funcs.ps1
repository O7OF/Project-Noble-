$data = [System.IO.File]::ReadAllText('C:\Users\Chey1\Desktop\Halo Reach Community Edition\docs\decompiled\host-probe.exe\decompile-index.json')
$json = ConvertFrom-Json $data
$fns = $json.functions | Where-Object { $_.name -like 'host_probe*::host::*' -and $_.name -notlike '*VTABLE*' }
$fns | Select-Object address, name, relative_path | Format-Table -AutoSize
