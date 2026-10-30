# Co-op test harness - one side of a two-machine campaign co-op run.
#
# Designed so two agents on two different PCs (one hosting, one joining) can run the same script, exchange the host's
# endpoint through the `coop/` folder in the repository, and compare filtered logs afterwards.
#
#   Host side:   .\tools\coop-test.ps1 -Role host  -Peers 1 -PublicIp 203.0.113.7
#   Guest side:  .\tools\coop-test.ps1 -Role guest -Connect 203.0.113.7:11774
#   No launch:   .\tools\coop-test.ps1 -Role report -InputLog path\to\noble-client.log
#
# Produces, under <repo>\coop\:
#   endpoint.txt            host only - "ip:port" the guest should dial
#   host-<stamp>.log        filtered host log
#   guest-<stamp>.log       filtered guest log
#   result-<stamp>.json     outcome summary for whoever reads it next

param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('host', 'guest', 'report')]
    [string]$Role,

    [string]$Map = 'm10',
    [ValidateSet('campaign', 'firefight', 'multiplayer')]
    [string]$Mode = 'campaign',
    [int]$Difficulty = 1,
    [int]$Peers = 1,
    [string]$Connect = '',
    [string]$Relay = '',
    [string]$PublicIp = '',
    [int]$Seconds = 150,
    [string]$InputLog = '',
    [switch]$KeepClient
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$release = Join-Path $root 'build\bin\Release'
$exe = Join-Path $release 'noble-client.exe'
$log = Join-Path $release 'noble-client.log'
$coop = Join-Path $root 'coop'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'

New-Item -ItemType Directory -Force -Path $coop | Out-Null

# Lines that actually decide a co-op failure: the peer barrier, the two start commands, the co-op insertion point, the
# session abort (with its 0x1B/0x1C/0x1D reason) and the join/connect refusals.
$coopPatterns = @(
    'peer wait', 'engine peer', 'Command 10', 'Command 11', 'launching match',
    'game session started', 'abort called', 'SUPPRESSED session abort',
    'evaluate_coop_insertion_point', 'JOIN REFUSED', 'CONNECT REFUSED', 'boot_peer',
    'Registered peer', 'Configured guest', 'game over', 'Career:', 'GUEST\]', 'peer\(s\)'
)

function Filter-CoOpLog {
    param([string]$Source, [string]$Destination)
    if (-not (Test-Path $Source)) { throw "log not found: $Source" }
    $hits = Get-Content -LiteralPath $Source -Encoding UTF8 | Select-String -Pattern $coopPatterns
    $hits | ForEach-Object { $_.Line } | Set-Content -LiteralPath $Destination -Encoding UTF8
    return $hits.Count
}

function Write-Result {
    param([string]$Status, [string]$Note, [string]$LogPath)
    $ipPort = ''
    if (Test-Path (Join-Path $coop 'endpoint.txt')) {
        $ipPort = (Get-Content (Join-Path $coop 'endpoint.txt') -Raw).Trim()
    }
    $result = [ordered]@{
        stamp     = $stamp
        role      = $Role
        map       = $Map
        mode      = $Mode
        peers     = $Peers
        endpoint  = $ipPort
        status    = $Status
        note      = $Note
        log       = $LogPath
        commit    = (& git -C $root rev-parse --short HEAD)
    }
    $out = Join-Path $coop ("result-{0}-{1}.json" -f $Role, $stamp)
    $result | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $out -Encoding UTF8
    Write-Host "result written: $out"
}

switch ($Role) {
    'report' {
        if (-not $InputLog) { throw '-Role report needs -InputLog <path>' }
        $dest = Join-Path $coop ("report-{0}.log" -f $stamp)
        $count = Filter-CoOpLog -Source $InputLog -Destination $dest
        Write-Host "filtered $count co-op lines into $dest"
        Write-Result -Status 'report' -Note ("$count relevant lines") -LogPath $dest
        break
    }

    'host' {
        Get-Process noble-client -ErrorAction SilentlyContinue | Stop-Process -Force
        Start-Sleep -Seconds 1
        if (-not (Test-Path $exe)) { throw "client not built: $exe" }

        $clientArgs = @('-map', $Map, '-mode', $Mode, '-difficulty', "$Difficulty", '-host', '-online',
                  '-wait-peers', "$Peers")
        if ($Relay) { $clientArgs += @('-relay', $Relay) }
        Write-Host ("hosting: {0} {1}" -f $exe, ($clientArgs -join ' '))
        $proc = Start-Process -FilePath $exe -ArgumentList $clientArgs -WorkingDirectory $release -PassThru

        # Wait for the transport to report its local UDP port, then publish the endpoint for the guest.
        $port = $null
        $deadline = (Get-Date).AddSeconds(60)
        while ((Get-Date) -lt $deadline) {
            Start-Sleep -Milliseconds 500
            if (Test-Path $log) {
                $m = Select-String -LiteralPath $log -Pattern 'on UDP port (\d+)' | Select-Object -Last 1
                if ($m) { $port = $m.Matches[0].Groups[1].Value; break }
            }
        }
        if ($port) {
            $endpoint = "127.0.0.1:$port"
            if ($PublicIp) { $endpoint = "${PublicIp}:$port" }
            $endpoint | Set-Content -LiteralPath (Join-Path $coop 'endpoint.txt') -Encoding ASCII
            Write-Host "endpoint published: $endpoint (UDP $port)"
        } else {
            Write-Warning 'no UDP port found in the log; the guest has no endpoint to dial'
        }

        Write-Host "hosting for $Seconds s - guest should join now"
        Start-Sleep -Seconds $Seconds
        if (-not $KeepClient) {
            Get-Process noble-client -ErrorAction SilentlyContinue | Stop-Process -Force
        }
        $dest = Join-Path $coop ("host-{0}.log" -f $stamp)
        $count = Filter-CoOpLog -Source $log -Destination $dest
        Write-Host "filtered $count host co-op lines into $dest"
        Write-Result -Status 'host-finished' -Note 'check for abort lines and whether the mission started with peers' -LogPath $dest
        break
    }

    'guest' {
        if (-not $Connect) { throw '-Role guest needs -Connect ip:port (from coop\endpoint.txt)' }
        Get-Process noble-client -ErrorAction SilentlyContinue | Stop-Process -Force
        Start-Sleep -Seconds 1
        if (-not (Test-Path $exe)) { throw "client not built: $exe" }

        $clientArgs = @('-map', $Map, '-mode', $Mode, '-difficulty', "$Difficulty", '-connect', $Connect)
        if ($Relay) { $clientArgs += @('-relay', $Relay) }
        Write-Host ("joining: {0} {1}" -f $exe, ($clientArgs -join ' '))
        $proc = Start-Process -FilePath $exe -ArgumentList $clientArgs -WorkingDirectory $release -PassThru

        Start-Sleep -Seconds $Seconds
        if (-not $KeepClient) {
            Get-Process noble-client -ErrorAction SilentlyContinue | Stop-Process -Force
        }
        $dest = Join-Path $coop ("guest-{0}.log" -f $stamp)
        $count = Filter-CoOpLog -Source $log -Destination $dest
        Write-Host "filtered $count guest co-op lines into $dest"
        Write-Result -Status 'guest-finished' -Note 'check for abort lines, and whether the cutscene had a camera' -LogPath $dest
        break
    }
}
