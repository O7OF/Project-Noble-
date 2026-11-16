# Reach + Halo 3 focused, throttled export. Resumable (skips channels whose folder already has json).
$ErrorActionPreference = 'Continue'
Set-Location 'D:\discord chat'
$exe = '.\DiscordChatExporter.Cli.exe'
$log = '.\full\export.log'
function W($m) { ('{0:HH:mm:ss} {1}' -f (Get-Date), $m) | Out-File -Append -Encoding utf8 $log }
$F = 'reach | hrek | gr2 | shader | cubemap | elite | sangheili | colour | color | halo3 | h3 | h3ek | odst | network | netcode | multiplayer | latency | desync | packet | dedicated'
$list = @(
 @('archive','help_texturing-and-shading','829278127083225118',$F),
 @('archive','help_general-help','830517591184506972',$F),
 @('archive','help_animation','841921592350212116',$F),
 @('archive','help_rigging','837250048864550963',$F),
 @('archive','help_ripping','819709677243858975',$F),
 @('archive','creative_modding-halo','832145913177702470',$F),
 @('archive','creative_modding-general','821250835110428702',$F),
 @('archive','cls_all-halo-asset-resources','821145601416364063',$F),
 @('archive','cls_misc-links-and-resources','1152763630400573481',$F),
 @('archive','cls_archive-research','869749654856814592',$F),
 @('halomods','dev_materials','969261291325759619',$F),
 @('halomods','dev_modding','651821465699876874',$F),
 @('halomods','dev_support','1020023213700419645',$F),
 @('halomods','dev_scripting','968752034901815297',$F),
 @('halomods','dev_animation','968752003733938226',$F),
 @('halomods','dev_level-design','1387931395615363122',$F),
 @('halomods','dev_campaign-evolved-research','1529869709636538408',$F),
 @('halomods','content_resources','1020694975824011308',$F),
 @('halomods','info_documentation','1278850445233754266',$F),
 @('halomods','info_tools','1278850494474878997',$F),
 @('halomods','info_tools-updates','970099317660606504',$F),
 @('halomods','nettest_network-test-text','1558064002855079936','')
)
W ('reach+h3 focused list: ' + $list.Count + ' channels')
foreach ($c in $list) {
  $dir = ".\full\$($c[0])\$($c[1])"
  if ((Test-Path $dir) -and (Get-ChildItem $dir -Filter *.json -ErrorAction SilentlyContinue | Where-Object { $_.Length -gt 0 })) { W "skip $($c[1]) (done)"; continue }
  W "export $($c[0])/$($c[1])"
  New-Item -ItemType Directory -Force -Path $dir | Out-Null
  $tmp = ".\full\tmp_$($c[1]).log"
  $fa = @(); if ($c[3]) { $fa = @('--filter', $c[3]) }
  & $exe export -c $c[2] -f Json -o ($dir + '\') --include-threads All @fa 2>&1 | Out-File -Encoding utf8 $tmp
  if (Select-String -Path $tmp -Pattern 'Successfully exported' -Quiet) {
    $fs = @(Get-ChildItem $dir -Filter *.json)
    W ("ok   $($c[1]) files=" + $fs.Count + ' bytes=' + (($fs | Measure-Object Length -Sum).Sum))
    Remove-Item $tmp -ErrorAction SilentlyContinue
  } else {
    $why = (Select-String -Path $tmp -Pattern 'failed|forbidden|multiple|Exception|filter' | Select-Object -First 1).Line
    W "FAIL $($c[1]) : $why"
    Remove-Item $dir -Recurse -Force -ErrorAction SilentlyContinue
  }
  Start-Sleep -Seconds 10
}
W 'ALL DONE'
