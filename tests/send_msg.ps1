# Target controller
$ip = "127.0.0.1"
$port = 20045

# Create TCP client
$client = New-Object System.Net.Sockets.TcpClient
try {
    $client.Connect($ip, $port)
    $stream = $client.GetStream()

    # ---- Ethernet Header (LLDP) ----
    $dstMac = [byte[]](0x01,0x80,0xC2,0x00,0x00,0x0E) # LLDP Multicast
    $srcMac = [byte[]](0x00,0x11,0x22,0x33,0x44,0x55)
    $etherType = [byte[]](0x88, 0xCC) # LLDP

    # ---- LLDP TLVs ----
    # Chassis ID (Type 1, Length 7, Subtype 7: Locally Assigned)
    $chassisTlv = [byte[]](0x02, 0x07, 0x07) + [System.Text.Encoding]::ASCII.GetBytes("Switch1")
    
    # Port ID (Type 2, Length 6, Subtype 7: Locally Assigned)
    $portTlv = [byte[]](0x04, 0x06, 0x07) + [System.Text.Encoding]::ASCII.GetBytes("Eth1")
    
    # TTL (Type 3, Length 2, Value 120)
    $ttlTlv = [byte[]](0x06, 0x02, 0x00, 0x78)
    
    # End of LLDPDU (Type 0, Length 0)
    $endTlv = [byte[]](0x00, 0x00)

    $lldpPayload = $chassisTlv + $portTlv + $ttlTlv + $endTlv
    $fullEthernetFrame = $dstMac + $srcMac + $etherType + $lldpPayload

    # ---- OpenFlow 1.0 PACKET_IN Header ----
    $totalLength = 18 + $fullEthernetFrame.Length
    $lenHigh = [byte][math]::Floor($totalLength / 256)
    $lenLow = [byte]($totalLength % 256)

    $ofHeader = [byte[]](
        0x01,               # version 1.0
        0x0A,               # type = PACKET_IN
        $lenHigh,           # length high
        $lenLow,            # length low
        0x00,0x00,0x00,0x01 # xid
    )

    # ---- OpenFlow 1.0 PACKET_IN Body ----
    $bufferId = [byte[]](0xFF,0xFF,0xFF,0xFF) # No buffer
    $frameLenHigh = [byte][math]::Floor($fullEthernetFrame.Length / 256)
    $frameLenLow = [byte]($fullEthernetFrame.Length % 256)
    $inPort = [byte[]](0x00,0x01) # Port 1
    $reason = [byte[]](0x00)      # No match
    $pad = [byte[]](0x00)

    $ofBody = $bufferId + [byte[]]($frameLenHigh, $frameLenLow) + $inPort + $reason + $pad

    # Final packet
    $packet = $ofHeader + $ofBody + $fullEthernetFrame

    # Send
    $stream.Write($packet, 0, $packet.Length)
    Write-Host "Sent OpenFlow PACKET_IN containing LLDP frame ($($packet.Length) bytes total)."
    
} catch {
    Write-Error "Failed to send message: $_"
} finally {
    if ($null -ne $client) { $client.Close() }
}
