$H = 'D:\Steam\steamapps\common\HREK'
"--- multi scenarios"
Get-ChildItem "$H\tags\levels\multi" -Recurse -Filter *.scenario -ErrorAction SilentlyContinue | ForEach-Object { "{0} {1}" -f $_.FullName.Replace("$H\tags\",''), $_.Length }
"--- other small scenarios"
Get-ChildItem "$H\tags\levels" -Recurse -Filter *.scenario -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch '\\multi\\' } | Select-Object -First 25 | ForEach-Object { $_.FullName.Replace("$H\tags\",'') }
"--- maps folder"
Get-ChildItem "$H\maps" | ForEach-Object { "{0} {1} {2}" -f $_.Name, $_.Length, $_.LastWriteTime }
"--- globals refs to elite_noble"
Select-String -Path "$H\tags\globals\globals.globals" -Pattern 'elite_noble' -SimpleMatch -Encoding ascii | Measure-Object | Select-Object -ExpandProperty Count
