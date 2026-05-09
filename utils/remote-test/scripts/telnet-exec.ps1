# Telnet auto-login and execute command script for SylixOS
# Usage: .\telnet-exec.ps1 -HostName <host> -Port <port> -Command <command>

param(
    [Parameter(Mandatory=$true)]
    [string]$HostName,

    [Parameter(Mandatory=$true)]
    [int]$Port,

    [Parameter(Mandatory=$true)]
    [string]$Command,

    [int]$Timeout = 600
)

# Preserve the command exactly as provided (don't let PowerShell modify paths)
$CommandToSend = $Command

function Read-TelnetResponse {
    param($stream, $delay = 500)

    Start-Sleep -Milliseconds $delay
    $response = ""
    while ($stream.DataAvailable) {
        $buffer = New-Object byte[] 4096
        $read = $stream.Read($buffer, 0, $buffer.Length)
        $response += [System.Text.Encoding]::ASCII.GetString($buffer, 0, $read)
        Start-Sleep -Milliseconds 50
    }
    return $response
}

function Write-TelnetCommand {
    param($stream, $command)

    $bytes = [System.Text.Encoding]::ASCII.GetBytes($command + "`r`n")
    $stream.Write($bytes, 0, $bytes.Length)
    $stream.Flush()
}

try {
    Write-Host "=== Connecting to ${HostName}:${Port} ===" -ForegroundColor Cyan

    $client = New-Object System.Net.Sockets.TcpClient
    $client.ReceiveTimeout = 30000
    $client.SendTimeout = 10000
    $client.Connect($HostName, $Port)

    if (-not $client.Connected) {
        throw "Failed to connect"
    }

    $stream = $client.GetStream()
    Write-Host "Connected!" -ForegroundColor Green

    # Wait for login prompt
    Start-Sleep -Milliseconds 2000
    $response = Read-TelnetResponse $stream 1000
    Write-Host "[RECV] $response"

    # Handle login: username
    if ($response -match "login:") {
        Write-Host "[SEND] root" -ForegroundColor Yellow
        Write-TelnetCommand $stream "root"
        Start-Sleep -Milliseconds 1000
        $response = Read-TelnetResponse $stream 500
        Write-Host "[RECV] $response"
    }

    # Handle login: password
    if ($response -match "password:") {
        Write-Host "[SEND] root (password)" -ForegroundColor Yellow
        Write-TelnetCommand $stream "root"
        Start-Sleep -Milliseconds 2000
        $response = Read-TelnetResponse $stream 1000
        Write-Host "[RECV] $response"
    }

    # Check login result
    if ($response -match "login fail|Login incorrect") {
        throw "Login failed - check username/password"
    }

    # Wait for shell prompt
    for ($i = 0; $i -lt 5; $i++) {
        if ($response -match "\[root@") {
            Write-Host "Shell prompt detected!" -ForegroundColor Green
            break
        }
        Start-Sleep -Milliseconds 1000
        $response += Read-TelnetResponse $stream 500
    }

    Write-Host "`n=== Executing: $CommandToSend ===" -ForegroundColor Yellow

    Write-TelnetCommand $stream $CommandToSend

    # Read command output
    $startTime = Get-Date
    $output = ""
    $lastOutputTime = Get-Date

    while ($true) {
        $chunk = Read-TelnetResponse $stream 500
        if ($chunk) {
            Write-Host $chunk -NoNewline
            $output += $chunk
            $lastOutputTime = Get-Date

            # Success: Found Final Score
            if ($output -match "Final Score:\s*[\d.]+\s*/\s*100") {
                Start-Sleep -Milliseconds 2000
                $chunk = Read-TelnetResponse $stream 500
                if ($chunk) { Write-Host $chunk -NoNewline; $output += $chunk }
                break
            }

            # Failure: Crash detected
            if ($output -match "PSTATE|RFLAGS|Backtrace") {
                Start-Sleep -Milliseconds 2000
                $chunk = Read-TelnetResponse $stream 500
                if ($chunk) { Write-Host $chunk -NoNewline; $output += $chunk }
                break
            }
        }

        $elapsed = ((Get-Date) - $startTime).TotalSeconds
        $idleTime = ((Get-Date) - $lastOutputTime).TotalSeconds

        if ($elapsed -gt $Timeout) {
            Write-Host "`n[TIMEOUT after $Timeout seconds]" -ForegroundColor Red
            break
        }

        if ($idleTime -gt 60 -and $output.Length -gt 50) {
            Write-TelnetCommand $stream ""
            Start-Sleep -Milliseconds 2000
            $chunk = Read-TelnetResponse $stream 500
            if ($chunk) { Write-Host $chunk -NoNewline; $output += $chunk }
            if ($output -match "\[root@[^\]]+\]#\s*$") { break }
        }
    }

    Write-Host "`n`n=== Result ===" -ForegroundColor Cyan

    if ($output -match "Final Score:\s*([\d.]+)\s*/\s*100") {
        Write-Host "SUCCESS: Final Score = $($Matches[1]) / 100" -ForegroundColor Green
        exit 0
    } elseif ($output -match "PSTATE|RFLAGS|Backtrace|fault|panic") {
        Write-Host "FAILED: Crash detected" -ForegroundColor Red
        exit 1
    } else {
        Write-Host "UNKNOWN: Test may still be running or incomplete" -ForegroundColor Yellow
        exit 2
    }

} catch {
    Write-Host "Error: $_" -ForegroundColor Red
    exit 1
} finally {
    if ($client) { $client.Close() }
}
