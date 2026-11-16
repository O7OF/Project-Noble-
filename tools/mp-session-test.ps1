param(
    [int]$Players = 2,              # total players incl. host
    [switch]$Slot32,                # -slot-patch -slot-players 32 (+ unsafe override)
    [switch]$High,                  # NOBLE_SLOT_HIGH=1 (peer 17 / player 16 free-slot test)
    [int]$Seconds = 70,             # seconds to let the session run after all instances are up
    [int]$QuitAfter = 0,            # 0 = auto (host wait + Seconds + 40)
    [string]$Map = '20_sword_slayer',
    [string]$Mode = 'multiplayer',
    [int]$MaxPlayers = 0,           # 0 = $Players
    [string]$Tag = '',
    [int]$HostWait = 14,            # seconds between host launch and first guest
    [int]$GuestStagger = 4,         # seconds between guest launches
    [switch]$NoWaitPeers,           # do not pass -wait-peers to the host
    [int]$WaitPeersMs = 0,          # NOBLE_COOP_WAIT_MS override (0 = engine default 45 s)
    [int]$CaptureAt = 0,            # 0 = off
    [int]$ScoreboardAt = 0,         # 0 = off
    [int]$RamAbortGb = 26,          # abort if total working set exceeds this
    [string[]]$ExtraHostArgs = @(),
    [string[]]$ExtraGuestArgs = @()
)
# Multiplayer N-instance LAN harness for Project Noble (2026-11-16, multiplayer workstream).
# Host = build\bin\Release, guests = build\bin\Release2..ReleaseN (auto-created skeletons with junctions into
# Release\data when missing). Focus guard: whenever a noble-dev window grabs the foreground it is handed straight back,
# so Tyler's mouse/keyboard stay on his own window. Low-spec settings (1280x720, 30 fps, muted) on the AOC monitor.
# Writes <tag>-summary.txt plus a machine-readable <tag>-result.json next to the host log and prints the evidence.
$ErrorActionPreference = 'Continue'
$repo = 'D:\Project-Noble-Closed-Source'
$binRoot = Join-Path $repo 'build\bin'
$hostDir = Join-Path $binRoot 'Release'
$tplSettings = Join-Path $hostDir 'low-spec-settings.json'
$capDir = Join-Path $hostDir 'data\userdata\claude-tests-2026-11-11'
if ($Tag -eq '') { $Tag = ('mp' + $Players + $(if ($High) { 'h' } elseif ($Slot32) { 's32' } else { 'n' })) }
if ($QuitAfter -le 0) { $QuitAfter = $HostWait + $Seconds + 40 }
if ($MaxPlayers -le 0) { $MaxPlayers = $Players }
$suffix = if ($High) { 'high' } elseif ($Slot32) { 's32' } else { 'norm' }
$ts = Get-Date -Format 'yyyyMMdd-HHmmss'
$runTag = "$Tag-$ts"
$resultPath = Join-Path $hostDir "$runTag-result.json"
$summaryPath = Join-Path $hostDir "$runTag-summary.txt"
Write-Output "=== MP session test: players=$Players high=$High slot32=$Slot32 map=$Map mode=$Mode seconds=$Seconds run=$runTag ==="

# ---------------- focus guard ----------------
Add-Type -Namespace NobleTest -Name Win32Focus -MemberDefinition @'
[DllImport("user32.dll")] public static extern System.IntPtr GetForegroundWindow();
[DllImport("user32.dll")] public static extern bool SetForegroundWindow(System.IntPtr hWnd);
'@
$script:FocusTarget = [NobleTest.Win32Focus]::GetForegroundWindow()
$focusGuard = $null
if ($script:FocusTarget -ne [IntPtr]::Zero) {
    $focusGuard = Start-Job -ArgumentList ([Int64]$script:FocusTarget) -ScriptBlock {
        param([Int64]$TargetRaw)
        Add-Type -Namespace NobleTest -Name Win32Focus -MemberDefinition @'
[DllImport("user32.dll")] public static extern System.IntPtr GetForegroundWindow();
[DllImport("user32.dll")] public static extern bool SetForegroundWindow(System.IntPtr hWnd);
'@
        $target = [IntPtr]$TargetRaw
        while ($true) {
            try {
                $fg = [NobleTest.Win32Focus]::GetForegroundWindow()
                if ($fg -ne [IntPtr]::Zero -and $fg -ne $target) {
                    $noble = @(Get-Process -Name noble-dev -ErrorAction SilentlyContinue | ForEach-Object { $_.MainWindowHandle } | Where-Object { $_ -ne [IntPtr]::Zero })
                    if ($noble -contains $fg) { [NobleTest.Win32Focus]::SetForegroundWindow($target) | Out-Null }
                }
            } catch { }
            Start-Sleep -Milliseconds 150
        }
    }
}
function Start-Test([string]$file, [string]$cwd, [object[]]$argList) {
    $p = Start-Process -FilePath $file -WorkingDirectory $cwd -PassThru -ArgumentList $argList -ErrorAction Stop
    if ($script:FocusTarget -ne [IntPtr]::Zero) { [NobleTest.Win32Focus]::SetForegroundWindow($script:FocusTarget) | Out-Null }
    return $p
}

# ---------------- instance dirs ----------------
$dataJunctions = @('armor','blocks','game_modes','game_variants','host_variants','licenses','limits','loadscreen','maps','map_variants','menumaps','mods','ui')
function Ensure-InstanceDir([string]$dir, [string]$templateDir) {
    if (Test-Path (Join-Path $dir 'noble-dev.exe')) { return }
    Write-Output ("creating instance dir " + $dir + " from " + $templateDir)
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
    Copy-Item (Join-Path $hostDir 'noble-dev.exe') (Join-Path $dir 'noble-dev.exe') -Force
    $data = Join-Path $dir 'data'
    New-Item -ItemType Directory -Path $data -Force | Out-Null
    foreach ($j in $dataJunctions) {
        $target = Join-Path $hostDir ("data\" + $j)
        if (Test-Path $target) { New-Item -ItemType Junction -Path (Join-Path $data $j) -Target $target -Force | Out-Null }
    }
    foreach ($real in @('mvar_write','sandbox')) { New-Item -ItemType Directory -Path (Join-Path $data $real) -Force | Out-Null }
    $ad = Join-Path $hostDir 'data\appearance.dat'
    if (Test-Path $ad) { Copy-Item $ad (Join-Path $data 'appearance.dat') -Force }
    $seed = Join-Path $templateDir 'data\userdata'
    Copy-Item $seed (Join-Path $data 'userdata') -Recurse -Force
    New-Item -ItemType Directory -Path (Join-Path $dir 'running') -Force | Out-Null
}

$dirs = @($hostDir)
for ($i = 2; $i -le $Players; ++$i) { $dirs += (Join-Path $binRoot ("Release" + $i)) }
# Keep every guest on the exact host binary: a stale noble-dev.exe in Release2..N silently runs a different build.
$hostExeHash = (Get-FileHash (Join-Path $hostDir 'noble-dev.exe') -Algorithm SHA256).Hash
foreach ($d in $dirs) {
    if ($d -eq $hostDir) { continue }
    $exe = Join-Path $d 'noble-dev.exe'
    if (-not (Test-Path $exe) -or (Get-FileHash $exe -Algorithm SHA256).Hash -ne $hostExeHash) {
        Copy-Item (Join-Path $hostDir 'noble-dev.exe') $exe -Force
        Write-Output ('synced guest exe: ' + $d)
    }
}
$templateDir = Join-Path $binRoot 'Release5'
if (-not (Test-Path (Join-Path $templateDir 'noble-dev.exe'))) { $templateDir = Join-Path $binRoot 'Release2' }
foreach ($d in $dirs) { if ($d -ne $hostDir) { Ensure-InstanceDir $d $templateDir } }

# ---------------- settings + identities ----------------
function Apply-LowSettings([string]$dir) {
    $p = Join-Path $dir 'data\userdata\settings.json'
    Copy-Item $tplSettings $p -Force
}
$idBackups = @{}
foreach ($d in $dirs) {
    $p = Join-Path $d 'data\userdata\identity.json'
    if (Test-Path $p) { $idBackups[$p] = [IO.File]::ReadAllText($p) }
}
function Write-Identity([string]$dir, [string]$devId, [string]$name, [string]$tag) {
    $id = [ordered]@{ deviceId=$devId; playerName=$name; serviceTag=$tag; developer=$false } | ConvertTo-Json
    [IO.File]::WriteAllText((Join-Path $dir 'data\userdata\identity.json'), $id)
}
Get-Process noble-dev -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 3
Get-ChildItem $hostDir -Filter 'capture_*.bmp' -ErrorAction SilentlyContinue | Remove-Item -Force
foreach ($d in $dirs) { Get-ChildItem $d -Filter 'capture_*.bmp' -ErrorAction SilentlyContinue | Remove-Item -Force }

$env:NOBLE_WINDOW_SECONDARY = '1'
if ($Slot32) { $env:NOBLE_SLOT_PLAYERS_UNSAFE = '1' } else { Remove-Item Env:\NOBLE_SLOT_PLAYERS_UNSAFE -ErrorAction SilentlyContinue }
if ($High) { $env:NOBLE_SLOT_HIGH = '1' } else { Remove-Item Env:\NOBLE_SLOT_HIGH -ErrorAction SilentlyContinue }
if ($WaitPeersMs -gt 0) { $env:NOBLE_COOP_WAIT_MS = "$WaitPeersMs" } else { Remove-Item Env:\NOBLE_COOP_WAIT_MS -ErrorAction SilentlyContinue }

$slotArgs = @()
if ($Slot32) { $slotArgs = @('-slot-patch','-slot-players','32') }
$procs = @(); $logs = @(); $names = @(); $peaks = @{}
try {
    foreach ($d in $dirs) { Apply-LowSettings $d }
    for ($i = 1; $i -lt $Players; ++$i) {
        $n = $i + 1
        Write-Identity $dirs[$i] ("5a0000000000{0:x4}" -f (0x2200 + $i)) "NobleTest$n" ("T{0:d2}" -f $n)
    }
    $hostLog = "host-$runTag.log"
    Remove-Item (Join-Path $hostDir $hostLog) -ErrorAction SilentlyContinue
    $hostArgs = @('-host','-lan','-private','-max-players',"$MaxPlayers",'-map',$Map,'-mode',$Mode,'-log',$hostLog,'-quit-after',"$QuitAfter") + $slotArgs
    if (-not $NoWaitPeers) { $hostArgs += @('-wait-peers', "$($Players - 1)") }
    if ($CaptureAt -gt 0) {
        # capture on the host and on the last guest, so the screenshot shows two different clients
        $hostArgs += @('-capture-at', "$CaptureAt*,$($CaptureAt + 25)")
    }
    if ($ScoreboardAt -gt 0) { $hostArgs += @('-scoreboard-at', "$ScoreboardAt") }
    if ($ExtraHostArgs.Count -gt 0) { $hostArgs += $ExtraHostArgs }
    Write-Output ("host args: " + ($hostArgs -join ' '))
    $hp = Start-Test -file (Join-Path $hostDir 'noble-dev.exe') -cwd $hostDir -argList $hostArgs
    $procs += $hp; $logs += $hostLog; $names += 'HOST'
    Write-Output ("host pid " + $hp.Id)
    Start-Sleep -Seconds $HostWait
    for ($i = 1; $i -lt $Players; ++$i) {
        $d = $dirs[$i]
        $log = "client$($i+1)-$runTag.log"
        Remove-Item (Join-Path $d $log) -ErrorAction SilentlyContinue
        $guestArgs = @('-connect','127.0.0.1:11774','-lan','-map',$Map,'-mode',$Mode,'-log',$log,'-quit-after',"$QuitAfter") + $slotArgs
        if ($i -eq ($Players - 1)) {
            if ($CaptureAt -gt 0) { $guestArgs += @('-capture-at', "$CaptureAt*,$($CaptureAt + 25)") }
            if ($ScoreboardAt -gt 0) { $guestArgs += @('-scoreboard-at', "$ScoreboardAt") }
        }
        if ($ExtraGuestArgs.Count -gt 0) { $guestArgs += $ExtraGuestArgs }
        $g = Start-Test -file (Join-Path $d 'noble-dev.exe') -cwd $d -argList $guestArgs
        $procs += $g; $logs += $log; $names += ("GUEST$($i+1)")
        Write-Output ("guest " + ($i+1) + " pid " + $g.Id)
        if ($i -lt ($Players - 1)) { Start-Sleep -Seconds $GuestStagger }
    }

    # ---------------- run + resource sampling ----------------
    $start = Get-Date
    $peakTotal = 0.0
    $aborted = $false
    while (((Get-Date) - $start).TotalSeconds -lt $Seconds) {
        $ps = @(Get-Process noble-dev -ErrorAction SilentlyContinue)
        if ($ps.Count -eq 0) { break }
        $ws = 0.0
        foreach ($p in $ps) {
            $mb = $p.WorkingSet64 / 1MB
            $ws += $mb
            $key = [string]$p.Id
            if (-not $peaks.ContainsKey($key) -or $peaks[$key] -lt $mb) { $peaks[$key] = $mb }
        }
        $wsGb = $ws / 1024.0
        if ($wsGb -gt $peakTotal) { $peakTotal = $wsGb }
        if ($wsGb -gt $RamAbortGb) { Write-Output ("ABORT: total working set " + [math]::Round($wsGb,1) + " GB > " + $RamAbortGb); $aborted = $true; break }
        Start-Sleep -Seconds 3
    }
    $alive = @($procs | Where-Object { -not $_.HasExited }).Count
    Write-Output ("run done: peak total working set " + [math]::Round($peakTotal,1) + " GB, instances still alive " + $alive + "/" + $procs.Count)
    for ($i = 0; $i -lt $procs.Count; ++$i) {
        $p = $procs[$i]
        $state = 'exited'
        if (-not $p.HasExited) { $state = 'alive' } else { $state = 'exited(' + $p.ExitCode + ')' }
        Write-Output ("  " + $names[$i] + " pid " + $p.Id + " " + $state + " peakWS " + [math]::Round($peaks[[string]$p.Id],0) + " MB")
    }
} finally {
    foreach ($p in $procs) { if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
    Start-Sleep -Seconds 3
    foreach ($kv in $idBackups.GetEnumerator()) { [IO.File]::WriteAllText($kv.Key, $kv.Value) }
    Get-Process noble-dev -ErrorAction SilentlyContinue | Stop-Process -Force
}

# ---------------- screenshots ----------------
if ($CaptureAt -gt 0) {
    New-Item -ItemType Directory -Path $capDir -Force | Out-Null
    Add-Type -AssemblyName System.Drawing
    foreach ($pair in @(@('host',$hostDir),@('guestlast',$dirs[$dirs.Count-1]))) {
        $caps = @(Get-ChildItem $pair[1] -Filter 'capture_*.bmp' -ErrorAction SilentlyContinue | Sort-Object Name)
        $k = 0
        foreach ($c in $caps) {
            try {
                $b = [System.Drawing.Bitmap]::FromFile($c.FullName)
                $png = Join-Path $capDir ("$runTag-" + $pair[0] + "-cap" + $k + ".png")
                $b.Save($png, [System.Drawing.Imaging.ImageFormat]::Png); $b.Dispose()
                Write-Output ("capture " + $pair[0] + " -> " + $png)
            } catch { Write-Output ("capture convert failed: " + $_) }
            $k++
        }
    }
}

# ---------------- evidence ----------------
$keyPattern = 'SlotPatch:|SEND msg 26|RECV msg 26|msg 26 |CLOSE channel|BOOT peer|CONNECT REFUSED|join refused|connect refused|game players:|player table dump|MEMBER-NEW|membership producer|session state|peer [0-9]+ (state|promot)|engine peers|in session|GAME OVER|FATAL EXCEPTION|0xC0000005|crash dump|game net log|refusal'
$summary = New-Object System.Collections.Generic.List[string]
$summary.Add("=== MP session test ${runTag}: players=$Players high=$High slot32=$Slot32 map=$Map mode=$Mode ===")
$summary.Add("host args: " + (($hostArgs) -join ' '))
$evidence = [ordered]@{}
for ($i = 0; $i -lt $procs.Count; ++$i) {
    $name = $names[$i]; $lp = Join-Path $dirs[$i] $logs[$i]
    $summary.Add("")
    $summary.Add("##### $name ($lp)")
    if (-not (Test-Path $lp)) { $summary.Add('missing log'); continue }
    $crash = @(Select-String -Path $lp -Pattern 'FATAL EXCEPTION|0xC0000005|crash dump written|STACK OVERRUN' -ErrorAction SilentlyContinue)
    $lines = @(Select-String -Path $lp -Pattern $keyPattern -ErrorAction SilentlyContinue)
    $summary.Add("crash hits: " + $crash.Count + " | key lines: " + $lines.Count)
    $tail = @($lines | Select-Object -Last 12 | ForEach-Object { $_.Line.Trim() })
    $summary.AddRange([string[]]$tail)
    $evidence[$name] = [ordered]@{ log=$lp; crashHits=$crash.Count; keyLines=@($lines | ForEach-Object { $_.Line.Trim() }) }
}
$json = [ordered]@{
    runTag=$runTag; players=$Players; high=[bool]$High; slot32=[bool]$Slot32; map=$Map; mode=$Mode
    seconds=$Seconds; aborted=$aborted; peakTotalGb=[math]::Round($peakTotal,2)
    instances=@(); evidence=$evidence
}
for ($i = 0; $i -lt $procs.Count; ++$i) {
    $p = $procs[$i]
    $json.instances += [ordered]@{ name=$names[$i]; pid=$p.Id; dir=$dirs[$i]; log=$logs[$i]
        exited=$p.HasExited; exitCode=$(if ($p.HasExited) { $p.ExitCode } else { $null })
        peakWsMb=[math]::Round($peaks[[string]$p.Id],0) }
}
$summary | Out-File -FilePath $summaryPath -Encoding utf8
$json | ConvertTo-Json -Depth 6 | Out-File -FilePath $resultPath -Encoding utf8
Write-Output ""
Write-Output ("summary: " + $summaryPath)
Write-Output ("json:    " + $resultPath)
$summary | Select-Object -First 8 | ForEach-Object { Write-Output $_ }
if ($focusGuard) { Stop-Job $focusGuard -ErrorAction SilentlyContinue; Remove-Job $focusGuard -Force -ErrorAction SilentlyContinue }
Write-Output 'DONE'
