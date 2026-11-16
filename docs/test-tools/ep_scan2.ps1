Set-Location D:\Project-Noble-Closed-Source
$Pat = '0x1055EC|1055EC|representation|elite_pieces|rrpmcg|rrneeds|rremotes|Stage|TODO|0x8EAF0|0x30698|0xA38A3|MH_CreateHook|Retarget|LOG_(INFO|WARN)\("ElitePack'
Select-String -Path src\engine\ElitePack.cpp -Pattern $Pat | ForEach-Object { "{0}: {1}" -f $_.LineNumber, $_.Line.Trim() } | Select-Object -First 80
"--- ReachArmory elite pieces -> match"
Select-String -Path src\engine\ReachArmory.cpp,src\engine\ReachHost.cpp,src\main.cpp -Pattern 'elite_pieces|elitePieces|armory-appearance|1055EC' | ForEach-Object { "{0}:{1}: {2}" -f (Split-Path $_.Path -Leaf), $_.LineNumber, $_.Line.Trim() } | Select-Object -First 30
"--- log: ElitePack lines last run"
Select-String -Path build\bin\Release\noble-dev.log -Pattern 'ElitePack' | Select-Object -Last 15 | ForEach-Object { $_.Line }
"--- git"
git log --oneline -5 -- src/engine/ElitePack.cpp
