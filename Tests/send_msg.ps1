# Target controller
$ip = "127.0.0.1"
$port = 20045   # or 6653 depending on your controller

# Create TCP client
$client = New-Object System.Net.Sockets.TcpClient
$client.Connect($ip, $port)
$stream = $client.GetStream()

# ---- OpenFlow 1.0 PACKET_IN ----
# Header fields:
# version = 1 (OF 1.0)
# type = 10 (PACKET_IN)
# length = 18 + payload
# xid = 1

# Payload (dummy Ethernet frame)
$payload = [byte[]](0x00,0x11,0x22,0x33,0x44,0x55,  # dst MAC
                   0x66,0x77,0x88,0x99,0xaa,0xbb,  # src MAC
                   0x08,0x00)                      # ethertype (IPv4)

$totalLength = 18 + $payload.Length

# Build header (big-endian)
$header = [byte[]](
    0x01,               # version
    0x0A,               # type = PACKET_IN
    0xFF,
    0xFF,
    0x00,0x00,0x00,0x01 # xid
)

# PACKET_IN body (simplified)
$bufferId = [byte[]](0xFF,0xFF,0xFF,0xFF)
$totalLenField = [byte[]](0x00, $payload.Length)
$inPort = [byte[]](0x00,0x01)
$reason = [byte[]](0x00)
$pad = [byte[]](0x00)

$body = $bufferId + $totalLenField + $inPort + $reason + $pad

# Final packet
$packet = $header + $body + $payload

# Send
$stream.Write($packet, 0, $packet.Length)

Write-Host "PACKET_IN sent."

$stream.Close()
$client.Close()