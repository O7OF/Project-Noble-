param([string]$Pattern = 'ElitePack: ', [string]$Since = '', [int]$First = 40, [int]$Tail = 6000, [string]$Exclude = 'MH_|Hook ''')
$Log = 'D:\Project-Noble-Closed-Source\build\bin\Release\noble-dev.log'
Get-Content $Log -Tail $Tail | Where-Object { $_ -match $Pattern -and $_ -notmatch $Exclude -and ($Since -eq '' -or $_ -ge "[$Since") } | Select-Object -First $First
