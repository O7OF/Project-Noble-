param([ValidateSet('rr','noble')][string]$Use)
$Armor = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\armor'
$RrBak = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\armor_rr_parked'
$NbBak = 'D:\Project-Noble-Closed-Source\build\bin\Release\data\armor_noble_parked'
New-Item -ItemType Directory -Force $RrBak, $NbBak | Out-Null
if ($Use -eq 'rr') {
  Get-ChildItem $Armor -Filter 'noble_elite_pack.map*' | Move-Item -Destination $NbBak -Force
  Get-ChildItem $RrBak -Filter 'rr_elite_pack.map*' | Move-Item -Destination $Armor -Force
} else {
  Get-ChildItem $Armor -Filter 'rr_elite_pack.map*' | Move-Item -Destination $RrBak -Force
  Get-ChildItem $NbBak -Filter 'noble_elite_pack.map*' | Move-Item -Destination $Armor -Force
}
"armor now: " + ((Get-ChildItem $Armor | ForEach-Object { $_.Name }) -join ', ')
