param(
    [string]$Port = "COM9",
    [int]$BaudRate = 115200,
    [string]$BoardIp = "192.168.31.211",
    [string]$Gateway = "192.168.31.1",
    [string]$Netmask = "255.255.255.0",
    [string]$Netif = "e00",
    [string]$RtbenchIp = "192.168.31.110",
    [string]$WinIp = "192.168.31.107",
    [string]$LogRoot = "C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench\utils\remote-test\logs\oneos-nezha-d1h-acceptance-20260610_132750",
    [switch]$Reboot,
    [switch]$RequireNetwork
)

$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path $LogRoot | Out-Null
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$LogPath = Join-Path $LogRoot "oneos_nezha_serial_recover_$Timestamp.log"
$RcPath = Join-Path $LogRoot "oneos_nezha_serial_recover_$Timestamp.rc"

function Write-RecoverLog {
    param([string]$Text)
    $Text | Tee-Object -FilePath $LogPath -Append
}

function Read-SerialFor {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [int]$Milliseconds
    )

    $deadline = (Get-Date).AddMilliseconds($Milliseconds)
    $buffer = ""
    while ((Get-Date) -lt $deadline) {
        try {
            $chunk = $Serial.ReadExisting()
            if ($chunk) {
                $buffer += $chunk
                Write-RecoverLog $chunk
            }
        } catch {
            # Serial reads commonly time out when the shell is idle.
        }
        Start-Sleep -Milliseconds 100
    }
    return $buffer
}

function Send-SerialCommand {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [string]$Command,
        [int]$WaitMilliseconds = 2000
    )

    Write-RecoverLog ""
    Write-RecoverLog "===== CMD: $Command ====="
    $Serial.Write($Command + "`r")
    return Read-SerialFor -Serial $Serial -Milliseconds $WaitMilliseconds
}

$serial = New-Object System.IO.Ports.SerialPort $Port,$BaudRate,None,8,one
$serial.Handshake = [System.IO.Ports.Handshake]::None
$serial.DtrEnable = $true
$serial.RtsEnable = $true
$serial.ReadTimeout = 300
$serial.WriteTimeout = 1000

$exitCode = 0
$allOutput = ""

try {
    Write-RecoverLog "===== OneOS Nezha D1H serial recovery $(Get-Date -Format o) ====="
    Write-RecoverLog "Port=$Port Baud=$BaudRate BoardIp=$BoardIp Netif=$Netif RtbenchIp=$RtbenchIp"

    $serial.Open()

    $serial.Write([char]3)
    $allOutput += Read-SerialFor -Serial $serial -Milliseconds 1000
    $allOutput += Send-SerialCommand -Serial $serial -Command "" -WaitMilliseconds 1200

    if ($Reboot) {
        Write-RecoverLog "===== REBOOT REQUESTED ====="
        $allOutput += Send-SerialCommand -Serial $serial -Command "reboot" -WaitMilliseconds 3000
        Start-Sleep -Seconds 8
        $serial.Write("`r")
        $allOutput += Read-SerialFor -Serial $serial -Milliseconds 8000
    }

    $allOutput += Send-SerialCommand -Serial $serial -Command "ifconfig" -WaitMilliseconds 3000

    if ($allOutput -notmatch [regex]::Escape($BoardIp)) {
        $allOutput += Send-SerialCommand -Serial $serial -Command "set_if $Netif $BoardIp $Gateway $Netmask" -WaitMilliseconds 5000
    } else {
        Write-RecoverLog "Board IP already present: $BoardIp"
    }

    $allOutput += Send-SerialCommand -Serial $serial -Command "default_netif $Netif" -WaitMilliseconds 3000
    $allOutput += Send-SerialCommand -Serial $serial -Command "telnetd start" -WaitMilliseconds 3000
    $allOutput += Send-SerialCommand -Serial $serial -Command "ifconfig" -WaitMilliseconds 3000
    $allOutput += Send-SerialCommand -Serial $serial -Command "ping $Gateway" -WaitMilliseconds 7000
    $allOutput += Send-SerialCommand -Serial $serial -Command "ping $RtbenchIp" -WaitMilliseconds 7000
    $allOutput += Send-SerialCommand -Serial $serial -Command "ping $WinIp" -WaitMilliseconds 7000
    $allOutput += Send-SerialCommand -Serial $serial -Command "list_lmodule" -WaitMilliseconds 4000

    $serial.Close()

    $hasShell = $allOutput -match "sh /user>|sh />|msh />"
    $hasIp = $allOutput -match [regex]::Escape($BoardIp)
    $gatewayOk = $allOutput -match "bytes from $([regex]::Escape($Gateway))"
    $rtbenchOk = $allOutput -match "bytes from $([regex]::Escape($RtbenchIp))"

    if (-not $hasShell -or -not $hasIp) {
        $exitCode = 1
        Write-RecoverLog "RECOVERY_RESULT=SERIAL_OR_IP_FAILED"
    } elseif ($RequireNetwork -and (-not $gatewayOk -or -not $rtbenchOk)) {
        $exitCode = 2
        Write-RecoverLog "RECOVERY_RESULT=NETWORK_FAILED"
    } else {
        Write-RecoverLog "RECOVERY_RESULT=OK"
    }
} catch {
    $exitCode = 1
    Write-RecoverLog "RECOVERY_EXCEPTION=$($_.Exception.Message)"
    if ($serial.IsOpen) {
        $serial.Close()
    }
}

Set-Content -Encoding ASCII -Path $RcPath -Value $exitCode
Write-RecoverLog "RC=$exitCode"
Write-Output "Log: $LogPath"
Write-Output "RC: $RcPath"
exit $exitCode
