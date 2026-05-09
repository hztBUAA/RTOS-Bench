$client = New-Object System.Net.Sockets.TcpClient('10.134.151.3', 3020)
Write-Host 'Connected successfully'
$client.Close()
