# Test Nezha board test-schedule
param(
    [int]$Cycles = 3,
    [int]$TimeoutMinutes = 15
)

$client = New-Object System.Net.Sockets.TcpClient
$client.ReceiveTimeout = $TimeoutMinutes * 60 * 1000
$client.SendTimeout = 30000
$client.Connect('10.134.151.3', 3014)
$stream = $client.GetStream()
$writer = New-Object System.IO.StreamWriter($stream)
$writer.AutoFlush = $true

Write-Host "Connected to Nezha (RISC-V SylixOS)..."
Start-Sleep -Milliseconds 500

# Login
$writer.WriteLine('root')
Start-Sleep -Milliseconds 500
$writer.WriteLine('root')
Start-Sleep -Milliseconds 2000

# Drain login banner
while ($stream.DataAvailable) { [void]$stream.ReadByte() }

# Run test-schedule
Write-Host "Running: /apps/nezha-rtos-bench test-schedule --cycles $Cycles"
$writer.WriteLine("/apps/nezha-rtos-bench test-schedule --cycles $Cycles")

# Read output
$timeout = [DateTime]::Now.AddMinutes($TimeoutMinutes)
$output = ''
$lastActivity = [DateTime]::Now
while ([DateTime]::Now -lt $timeout) {
    if ($stream.DataAvailable) {
        $char = [char]$stream.ReadByte()
        $output += $char
        Write-Host -NoNewline $char
        $lastActivity = [DateTime]::Now
        if ($output -match 'Final Score:.*\r?\n') {
            Write-Host ''
            Write-Host '=== Test Complete ==='
            break
        }
    } else {
        Start-Sleep -Milliseconds 100
        # Timeout if no activity for 2 minutes
        if (([DateTime]::Now - $lastActivity).TotalSeconds -gt 120) {
            Write-Host ''
            Write-Host '=== Timeout: No activity for 2 minutes ==='
            break
        }
    }
}

$client.Close()
Write-Host ''
Write-Host 'Connection closed.'
