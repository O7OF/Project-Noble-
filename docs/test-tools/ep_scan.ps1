Set-Location D:\Project-Noble-Closed-Source
Get-ChildItem src\engine\ElitePack.* | ForEach-Object { "{0} {1} lines" -f $_.Name, (Get-Content $_.FullName).Count }
"--- refs to ElitePack elsewhere"
Get-ChildItem src -Recurse -Include *.cpp,*.hpp,*.h | Select-String -Pattern 'ElitePack::|ElitePack\.hpp|rr_elite_pack' | ForEach-Object { "{0}:{1}: {2}" -f $_.Path.Replace('D:\Project-Noble-Closed-Source\',''), $_.LineNumber, $_.Line.Trim() } | Select-Object -First 40
"--- data\armor"
Get-ChildItem build\bin\Release\data\armor -ErrorAction SilentlyContinue | ForEach-Object { "{0} {1}" -f $_.Name, $_.Length }
