param(
    [string]$Port = "COM9",
    [int]$BaudRate = 115200,
    [string]$RtbenchIp = "192.168.31.110",
    [string]$LogRoot = "C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench\utils\remote-test\logs\oneos-nezha-d1h-acceptance-20260610_132750",
    [switch]$UseExistingBoardFiles,
    [switch]$SkipRecovery
)

$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Force -Path $LogRoot | Out-Null
$Timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$LogPath = Join-Path $LogRoot "oneos_nezha_serial_tftp_acceptance_$Timestamp.log"
$RcPath = Join-Path $LogRoot "oneos_nezha_serial_tftp_acceptance_$Timestamp.rc"
$SummaryPath = Join-Path $LogRoot "oneos_nezha_serial_tftp_acceptance_$Timestamp.md"

function Write-AcceptanceLog {
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
                Write-AcceptanceLog $chunk
            }
        } catch {
            # Idle serial reads time out normally.
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
    Write-AcceptanceLog ""
    Write-AcceptanceLog "===== CMD: $Command ====="
    $Serial.Write($Command + "`r")
    return Read-SerialFor -Serial $Serial -Milliseconds $WaitMilliseconds
}

function Run-Phase {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [string]$Name,
        [string]$Remote,
        [string]$BoardPath,
        [int]$WaitMilliseconds,
        [string]$PassPattern
    )

    Write-AcceptanceLog ""
    Write-AcceptanceLog "===== PHASE: $Name ====="
    $phaseOutput = ""
    if (-not $UseExistingBoardFiles) {
        $phaseOutput += Send-SerialCommand -Serial $Serial -Command "rm $BoardPath" -WaitMilliseconds 2000
        $phaseOutput += Send-SerialCommand -Serial $Serial -Command "tftp_client $RtbenchIp get $Remote $BoardPath" -WaitMilliseconds 30000
    }
    $phaseOutput += Send-SerialCommand -Serial $Serial -Command "ld $BoardPath" -WaitMilliseconds $WaitMilliseconds
    $passed = $phaseOutput -match $PassPattern
    Write-AcceptanceLog "PHASE_RESULT $Name=$passed"
    return [pscustomobject]@{
        Name = $Name
        Passed = $passed
        Pattern = $PassPattern
    }
}

$serial = New-Object System.IO.Ports.SerialPort $Port,$BaudRate,None,8,one
$serial.Handshake = [System.IO.Ports.Handshake]::None
$serial.DtrEnable = $true
$serial.RtsEnable = $true
$serial.ReadTimeout = 300
$serial.WriteTimeout = 1000

$exitCode = 0
$results = @()

try {
    Write-AcceptanceLog "===== OneOS Nezha D1H serial+tftp acceptance $(Get-Date -Format o) ====="
    Write-AcceptanceLog "Port=$Port Baud=$BaudRate RtbenchIp=$RtbenchIp UseExistingBoardFiles=$UseExistingBoardFiles"

    $serial.Open()
    $serial.Write([char]3)
    Read-SerialFor -Serial $serial -Milliseconds 1000 | Out-Null
    Send-SerialCommand -Serial $serial -Command "" -WaitMilliseconds 1000 | Out-Null

    if (-not $SkipRecovery) {
        Send-SerialCommand -Serial $serial -Command "default_netif e00" -WaitMilliseconds 3000 | Out-Null
        Send-SerialCommand -Serial $serial -Command "telnetd start" -WaitMilliseconds 3000 | Out-Null
        Send-SerialCommand -Serial $serial -Command "ifconfig" -WaitMilliseconds 3000 | Out-Null
        if (-not $UseExistingBoardFiles) {
            Send-SerialCommand -Serial $serial -Command "ping $RtbenchIp" -WaitMilliseconds 7000 | Out-Null
        }
    }

    if (-not $UseExistingBoardFiles) {
        Run-Phase -Serial $serial -Name "P-main-ctest" -Remote "ctest.out" -BoardPath "/user/ctest.out" -WaitMilliseconds 8000 -PassPattern "module\[/user/ctest\.out\] loaded|loaded \[cleanup" | Out-Null
    } else {
        Send-SerialCommand -Serial $serial -Command "list_lmodule" -WaitMilliseconds 5000 | Out-Null
    }

    $results += Run-Phase -Serial $serial -Name "P0-schedule" -Remote "schedrun.out" -BoardPath "/user/schedrun.out" -WaitMilliseconds 120000 -PassPattern "Final Score: 100\.00 / 100|schedule ret=0"
    $results += Run-Phase -Serial $serial -Name "P1-realtime" -Remote "rtrt.out" -BoardPath "/user/rtrt.out" -WaitMilliseconds 180000 -PassPattern "Benchmark completed with code: 0|realtime ret=0"
    $results += Run-Phase -Serial $serial -Name "P2-workloads-quick" -Remote "wlrun.out" -BoardPath "/user/wlrun.out" -WaitMilliseconds 300000 -PassPattern "workloads ret=0"
    $results += Run-Phase -Serial $serial -Name "P3-stress" -Remote "strun.out" -BoardPath "/user/strun.out" -WaitMilliseconds 120000 -PassPattern "test-stress ret=0"
    $results += Run-Phase -Serial $serial -Name "P4-test-all-quick" -Remote "allrun.out" -BoardPath "/user/allrun.out" -WaitMilliseconds 1200000 -PassPattern "test-all ret=0"
    $results += Run-Phase -Serial $serial -Name "P5-workloads-full" -Remote "wlfull.out" -BoardPath "/user/wlfull.out" -WaitMilliseconds 420000 -PassPattern "workloads ret=0"

    Send-SerialCommand -Serial $serial -Command "ls /user" -WaitMilliseconds 5000 | Out-Null
    Send-SerialCommand -Serial $serial -Command "list_lmodule" -WaitMilliseconds 5000 | Out-Null
    $serial.Close()

    if ($results | Where-Object { -not $_.Passed }) {
        $exitCode = 1
    }
} catch {
    $exitCode = 1
    Write-AcceptanceLog "ACCEPTANCE_EXCEPTION=$($_.Exception.Message)"
    if ($serial.IsOpen) {
        $serial.Close()
    }
}

Set-Content -Encoding ASCII -Path $RcPath -Value $exitCode

$summaryLines = @(
    "# OneOS Nezha D1H Serial/TFTP Acceptance",
    "",
    "- Status: $(if ($exitCode -eq 0) { 'PASS' } else { 'FAIL' })",
    "- Log: $LogPath",
    "- RC: $RcPath",
    "- UseExistingBoardFiles: $UseExistingBoardFiles",
    "",
    "| Phase | Result | Pattern |",
    "|---|---:|---|"
)
foreach ($result in $results) {
    $summaryLines += "| `$($result.Name)` | $(if ($result.Passed) { 'PASS' } else { 'FAIL' }) | `$($result.Pattern)` |"
}
$summaryLines | Set-Content -Encoding UTF8 -Path $SummaryPath

Write-AcceptanceLog "ACCEPTANCE_RC=$exitCode"
Write-Output "Log: $LogPath"
Write-Output "RC: $RcPath"
Write-Output "Summary: $SummaryPath"
exit $exitCode
