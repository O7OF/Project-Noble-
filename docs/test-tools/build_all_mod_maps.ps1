$ErrorActionPreference='Continue'
$H='D:\Steam\steamapps\common\HREK'
$MCC='D:\Steam\steamapps\common\Halo The Master Chief Collection\haloreach\maps'
$MOD='D:\Project-Noble-Closed-Source\build\bin\Release\data\mods\armor_fx\maps'
New-Item -ItemType Directory -Force $MOD | Out-Null
$skip=@('shared','campaign','mainmenu')
$done=@()
$failed=@()
$maps=Get-ChildItem $MCC -Filter *.map -File | Sort-Object Name
"starting: " + $maps.Count + " maps at " + (Get-Date)
foreach($m in $maps){
  $stem=$m.BaseName
  if($skip -contains $stem){ "SKIP $stem"; continue }
  $target=Join-Path $MOD ($stem + '.map')
  if(Test-Path $target){ "HAVE $stem"; $done+=$stem; continue }
  $sc=(Get-ChildItem (Join-Path $H 'tags\levels') -Recurse -Filter ($stem + '.scenario') -File -ErrorAction SilentlyContinue | Select-Object -First 1)
  if(-not $sc){ "NO-SCENARIO $stem"; $failed+=$stem; continue }
  $rel=$sc.FullName.Substring((Join-Path $H 'tags').Length+1)
  $rel=$rel.Substring(0,$rel.Length-9)
  $log=Join-Path $H ('build_modmap_'+$stem+'.log')
  $start=Get-Date
  Push-Location $H
  & .\tool.exe build-cache-file $rel pc english *> $log
  $code=$LASTEXITCODE
  Pop-Location
  $secs=[int]((Get-Date)-$start).TotalSeconds
  $built=Join-Path $H ('maps\' + $stem + '.map')
  if($code -eq 0 -and (Test-Path $built)){
    Copy-Item $built $target -Force
    $sz=[math]::Round((Get-Item $built).Length/1MB,1)
    "OK $stem " + $secs + "s " + $sz + "MB"
    $done+=$stem
  } else {
    "FAIL $stem exit=" + $code + " " + $secs + "s"
    $failed+=$stem
  }
}
"finished: " + $done.Count + " ok, " + $failed.Count + " failed at " + (Get-Date)
"failed: " + ($failed -join ',')
