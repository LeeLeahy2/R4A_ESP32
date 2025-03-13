/**********************************************************************
  ESP-NOW.ino

  Handle the ESP-NOW events
  Use ESP NOW protocol to transmit RTCM between RTK Products via 2.4GHz
  ESP-NOW start and stop based upon example 4_9_ESP_NOW in hw EVK repo

  How pairing works:
    1. Device enters pairing mode
    2. Device adds the broadcast MAC (all 0xFFs) as peer
    3. Device waits for incoming pairing packet from remote
    4. If valid pairing packet received, add peer, immediately transmit
       a pairing packet to that peer and exit.

  ESP NOW is bare metal, there is no guaranteed packet delivery. For RTCM
  byte transmissions using ESP NOW:
  * We don't care about dropped packets or packets out of order.  The GNSS
    checks the integrity of the RTCM packet.
  * We don't care if the ESP NOW packet is corrupt or not. RTCM has its own
    CRC. RTK needs valid RTCM once every few seconds so a single dropped
    frame is not critical.
**********************************************************************/

#ifdef  COMPILE_ESPNOW

//****************************************
// Constants
//****************************************

const uint8_t r4aEspNowBroadcastAddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

//****************************************
// Types
//****************************************

// Create a struct for ESP NOW pairing
typedef struct _R4A_ESP_NOW_PAIR_MESSAGE
{
    uint8_t macAddress[6];
    bool encrypt;
    uint8_t channel;
    uint8_t crc; // Simple check - add MAC together and limit to 8 bit
} R4A_ESP_NOW_PAIR_MESSAGE;

//****************************************
// Locals
//****************************************

uint16_t r4aEspNowBytesSent;    // May be more than 255
unsigned long r4aEspNowLastAdd; // Tracks how long since the last byte was added to the outgoing buffer
unsigned long r4aEspNowLastRssiUpdate;
uint8_t r4aEspNowOutgoing[250]; // ESP NOW has max of 250 characters
uint8_t r4aEspNowOutgoingSpot;  // ESP Now has a max of 250 characters
uint8_t r4aEspNowReceivedMAC[6]; // Holds the broadcast MAC during pairing
ESPNOWState r4aEspNowState;

//****************************************
// Forward routine declarations
//****************************************

esp_err_t r4aEspNowSendPairMessage(const uint8_t *sendToMac = espNowBroadcastAddr);

//*********************************************************************
// Add a peer to the ESP-NOW network
esp_err_t r4aEspNowAddPeer(const uint8_t * peerMac)
{
    esp_now_peer_info_t peerInfo;

    // Describe the peer
    memcpy(peerInfo.peer_addr, peerMac, 6);
    peerInfo.channel = 0;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.encrypt = false;

    // Add the peer
    if (settings.debugEspNow)
        systemPrintf("Calling esp_now_add_peer\r\n");
    esp_err_t result = esp_now_add_peer(&peerInfo);
    if (settings.debugEspNow == true)
    {
        if (result != ESP_OK)
            systemPrintf("ERROR: Failed to add ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x, result: %s\r\n",
                         peerMac[0], peerMac[1],
                         peerMac[2], peerMac[3],
                         peerMac[4], peerMac[5],
                         esp_err_to_name(result));
        else
            systemPrintf("Added ESP-NOW %s peer %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                         peerInfo.encrypt ? "encrypted" : "unencrypted",
                         peerMac[0], peerMac[1],
                         peerMac[2], peerMac[3],
                         peerMac[4], peerMac[5]);
    }
    return result;
}

//*********************************************************************
// Start ESP-Now if needed, put ESP-Now into broadcast state
void r4aEspNowBeginPairing()
{
    // Start ESP-NOW if necessary
    r4aWifiEspNowOn(__FILE__, __LINE__);

    // To begin pairing, we must add the broadcast MAC to the peer list
    r4aEspNowAddPeer(espNowBroadcastAddr);

    r4aEspNowSetState(ESPNOW_PAIRING);
}

//*********************************************************************
// Determine if ESP-NOW is paired
bool r4aEspNowIsPaired()
{
    return (r4aEspNowState == ESPNOW_PAIRED);
}

//*********************************************************************
// Callback when data is received
void r4aEspNowOnDataReceived(const esp_now_recv_info *mac,
                             const uint8_t *incomingData,
                             int len)
{
//    typedef struct esp_now_recv_info {
//        uint8_t * src_addr;             // Source address of ESPNOW packet
//        uint8_t * des_addr;             // Destination address of ESPNOW packet
//        wifi_pkt_rx_ctrl_t * rx_ctrl;   // Rx control info of ESPNOW packet
//    } esp_now_recv_info_t;

    // Display the packet
    if (settings.debugEspNow == true)
    {
        systemPrintf("*** ESP-NOW: RX %02x:%02x:%02x:%02x:%02x:%02x --> %02x:%02x:%02x:%02x:%02x:%02x, %d bytes, rssi: %d\r\n",
                     mac->src_addr[0], mac->src_addr[1], mac->src_addr[2],
                     mac->src_addr[3], mac->src_addr[4], mac->src_addr[5],
                     mac->des_addr[0], mac->des_addr[1], mac->des_addr[2],
                     mac->des_addr[3], mac->des_addr[4], mac->des_addr[5],
                     len, packetRSSI);
    }

    if (r4aEspNowState == ESPNOW_PAIRING)
    {
        ESP_NOW_PAIR_MESSAGE pairMessage;
        if (len == sizeof(pairMessage)) // First error check
        {
            memcpy(&pairMessage, incomingData, len);

            // Compute the CRC
            uint8_t tempCRC = 0;
            for (int x = 0; x < 6; x++)
                tempCRC += pairMessage.macAddress[x];

            // Check the CRC
            if (tempCRC == pairMessage.crc)
            {
                // Valid CRC, save the MAC address
                memcpy(&espNowReceivedMAC, pairMessage.macAddress, 6);
                espNowSetState(ESPNOW_MAC_RECEIVED);
            }
            // else Pair CRC failed
        }
    }
    else
    {
        r4aEspNowRSSI = packetRSSI; // Record this packet's RSSI as an ESP NOW packet

        // We've just received ESP-Now data. We assume this is RTCM and push it directly to the GNSS.
        // Determine if ESPNOW is the correction source
        if (correctionLastSeen(CORR_ESPNOW))
        {
            // Pass RTCM bytes (presumably) from ESP NOW out ESP32-UART to GNSS
            gnss->pushRawData((uint8_t *)incomingData, len);

            if ((settings.debugEspNow == true || settings.debugCorrections == true) && !inMainMenu)
                systemPrintf("ESPNOW received %d RTCM bytes, pushed to GNSS, RSSI: %d\r\n", len, espNowRSSI);
        }
        else
        {
            if ((settings.debugEspNow == true || settings.debugCorrections == true) && !inMainMenu)
                systemPrintf("ESPNOW received %d RTCM bytes, NOT pushed due to priority, RSSI: %d\r\n", len,
                             espNowRSSI);
        }

        r4aEspNowIncomingRTCM = true; // Display a download icon
        r4aEspNowLastRssiUpdate = millis();
    }
}

//*********************************************************************
// Buffer RTCM data and send to ESP-NOW peer
void r4aEspNowProcessRTCM(byte incoming)
{
    // If we are paired,
    // Or if the radio is broadcasting
    // Then add bytes to the outgoing buffer
    if (r4aEspNowState == ESPNOW_PAIRED || r4aEspNowState == ESPNOW_BROADCASTING)
    {
        // Move this byte into ESP NOW to send buffer
        r4aEspNowOutgoing[r4aEspNowOutgoingSpot++] = incoming;
        r4aEspNowLastAdd = millis();
    }

    // Send buffer when full
    if (r4aEspNowOutgoingSpot == sizeof(r4aEspNowOutgoing))
    {
        r4aEspNowOutgoingSpot = 0; // Wrap

        if (r4aEspNowState == ESPNOW_PAIRED)
            esp_now_send(0, (uint8_t *)&r4aEspNowOutgoing, sizeof(r4aEspNowOutgoing)); // Send packet to all peers
        else // if (espNowState == ESPNOW_BROADCASTING)
        {
            esp_now_send(espNowBroadcastAddr, (uint8_t *)&r4aEspNowOutgoing,
                         sizeof(r4aEspNowOutgoing)); // Send packet via broadcast
        }

        delay(10); // We need a small delay between sending multiple packets

        r4aEspNowBytesSent += sizeof(r4aEspNowOutgoing);

        r4aEspNowOutgoingRTCM = true;
    }
}

//*********************************************************************
// Remove a given MAC address from the peer list
esp_err_t r4aEspNowRemovePeer(const uint8_t *peerMac)
{
    esp_err_t result = esp_now_del_peer(peerMac);
    if (settings.debugEspNow == true)
    {
        if (result != ESP_OK)
            systemPrintf("ERROR: Failed to remove ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x, result: %s\r\n",
                         peerMac[0], peerMac[1],
                         peerMac[2], peerMac[3],
                         peerMac[4], peerMac[5],
                         esp_err_to_name(result));
        else
            systemPrintf("Removed ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                         peerMac[0], peerMac[1],
                         peerMac[2], peerMac[3],
                         peerMac[4], peerMac[5]);
    }
    return result;
}

//*********************************************************************
// Update the state of the ESP Now state machine
//
//      +---------------------+
//      |     ESPNOW_OFF      |
//      +---------------------+
//          |             |
//          |             | No pairs listed
//          |             V
//          |  +---------------------+
//          |  | ESPNOW_BROADCASTING |
//          |  +---------------------+
//          |             |
//          |             |
//          |             V
//          |  +--------------------+
//          |  |   ESPNOW_PAIRING   |
//          |  +--------------------+
//          |             |
//          |             |
//          |             V
//          |  +---------------------+
//          |  | ESPNOW_MAC_RECEIVED |
//          |  +---------------------+
//          |             |
//          |             |
//          |             V
//          |  +--------------------+
//          '->|   ESPNOW_PAIRED    |
//             +--------------------+
//
// Send RTCM in either ESPNOW_BROADCASTING or ESPNOW_PAIRED state.
// Receive RTCM in ESPNOW_BROADCASTING, ESPNOW_MAC_RECEIVED and
// ESPNOW_PAIRED states.
//*********************************************************************

//*********************************************************************
// Regularly call during pairing to see if we've received a Pairing message
bool r4aEspNowProcessRxPairedMessage()
{
    if (r4aEspNowState != ESPNOW_MAC_RECEIVED)
        return false;

    // Remove broadcast peer
    r4aEspNowRemovePeer(r4aEspNowBroadcastAddr);

    // Is the received MAC already paired?
    if (esp_now_is_peer_exist(r4aEspNowReceivedMAC) == true)
    {
        // Yes, already paired
        if (settings.debugEspNow == true)
            systemPrintln("Peer already exists");
    }
    else
    {
        // No, add new peer to system
        r4aEspNowAddPeer(r4aEspNowReceivedMAC);

        // Record this MAC to peer list
        memcpy(settings.espnowPeers[settings.espnowPeerCount], r4aEspNowReceivedMAC, 6);
        settings.espnowPeerCount++;
        settings.espnowPeerCount %= ESPNOW_MAX_PEERS;
    }

    // Send message directly to the received MAC (not unicast), then exit
    r4aEspNowSendPairMessage(r4aEspNowReceivedMAC);

    // Enable radio. User may have arrived here from the setup menu rather than serial menu.
    settings.enableEspNow = true;

    // Record enableEspNow and espnowPeerCount to NVM
    recordSystemSettings();
    r4aEspNowSetState(ESPNOW_PAIRED);
    return (true);
}

//*********************************************************************
// Update the state of the ESP-NOW subsystem
void r4aEspNowSetState(ESPNOWState newState)
{
    const char * name[] =
    {
        "ESPNOW_OFF",
        "ESPNOW_BROADCASTING",
        "ESPNOW_PAIRING",
        "ESPNOW_MAC_RECEIVED",
        "ESPNOW_PAIRED",
    };
    const int nameCount = sizeof(name) / sizeof(name[0]);
    const char * newName;
    char nLine[80];
    const char * oldName;
    char oLine[80];

    if (settings.debugEspNow == true)
    {
        // Get the old state name
        if (r4aEspNowState < ESPNOW_MAX)
            oldName = name[espNowState];
        else
        {
            sprintf(oLine, "Unknown state: %d", r4aEspNowState);
            oldName = &oLine[0];
        }

        // Get the new state name
        if (newState < ESPNOW_MAX)
            newName = name[newState];
        else
        {
            sprintf(nLine, "Unknown state: %d", newState);
            newName = &nLine[0];
        }

        // Display the state change
        if (newState == r4aEspNowState)
            systemPrintf("ESP-NOW: *%s\r\n", newName);
        else
            systemPrintf("ESP-NOW: %s --> %s\r\n", oldName, newName);
    }
    r4aEspNowState = newState;
}

//*********************************************************************
// Create special pair packet to a given MAC
esp_err_t r4aEspNowSendPairMessage(const uint8_t *sendToMac)
{
    // Assemble message to send
    ESP_NOW_PAIR_MESSAGE pairMessage;

    // Get unit MAC address
    memcpy(pairMessage.macAddress, wifiMACAddress, 6);
    pairMessage.encrypt = false;
    pairMessage.channel = 0;

    pairMessage.crc = 0; // Calculate CRC
    for (int x = 0; x < 6; x++)
        pairMessage.crc += wifiMACAddress[x];

    return (esp_now_send(sendToMac, (uint8_t *)&pairMessage, sizeof(pairMessage))); // Send packet to given MAC
}

//*********************************************************************
// Start ESP-NOW layer
bool r4aEspNowStart()
{
    int index;
    bool started;
    esp_err_t status;

    do
    {
        started = false;

        // Initialize the ESP-NOW layer
        if (settings.debugEspNow)
            systemPrintf("Calling esp_now_init\r\n");
        status = esp_now_init();
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to initialize ESP-NOW, status: %d\r\n", status);
            break;
        }

        // Set the receive packet routine address
        if (settings.debugEspNow)
            systemPrintf("Calling esp_now_register_recv_cb\r\n");
        status = esp_now_register_recv_cb(espNowOnDataReceived);
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to set ESP_NOW RX callback, status: %d\r\n", status);
            break;
        }

        // Check for peers listed in settings
        if (settings.espnowPeerCount == 0)
        {
            // No peers, enter broadcast mode
            // Determine if the broadcast peer already exists
            if (esp_now_is_peer_exist(r4aEspNowBroadcastAddr) == true)
            {
                if (settings.debugEspNow == true)
                    systemPrintln("Broadcast peer already exists");
            }
            else
            {
                // Add the broadcast peer
                esp_err_t result = espNowAddPeer(r4aEspNowBroadcastAddr);
                if (settings.debugEspNow == true)
                {
                    if (result != ESP_OK)
                        systemPrintln("Failed to add broadcast peer");
                    else
                        systemPrintln("Broadcast peer added");
                }
            }
            r4aEspNowSetState(ESPNOW_BROADCASTING);
        }
        else
        {
            // If we have peers, move to paired state
            r4aEspNowSetState(ESPNOW_PAIRED);

            if (settings.debugEspNow == true)
                systemPrintf("Adding %d espnow peers\r\n", settings.espnowPeerCount);

            // Loop through peers listed in settings
            for (index = 0; index < ESPNOW_MAX_PEERS; index++)
            {
                // Determine if peer exists
                if (esp_now_is_peer_exist(settings.espnowPeers[index]))
                {
                    if (settings.debugEspNow == true)
                        systemPrintf("ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x, status: already exists\r\n",
                                     settings.espnowPeers[index][0],
                                     settings.espnowPeers[index][1],
                                     settings.espnowPeers[index][2],
                                     settings.espnowPeers[index][3],
                                     settings.espnowPeers[index][4],
                                     settings.espnowPeers[index][5]);
                }
                else
                {
                    // Add peer if necessary
                    status = r4aEspNowAddPeer(&settings.espnowPeers[index][0]);
                    if (status == ESP_OK)
                    {
                        if (settings.debugEspNow == true)
                            systemPrintf("Added ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                                         settings.espnowPeers[index][0],
                                         settings.espnowPeers[index][1],
                                         settings.espnowPeers[index][2],
                                         settings.espnowPeers[index][3],
                                         settings.espnowPeers[index][4],
                                         settings.espnowPeers[index][5]);
                    }
                    else
                    {
                        if (settings.debugEspNow == true)
                            systemPrintf("ERROR: Failed to add ESP-NOW peer %02x:%02x:%02x:%02x:%02x:%02x, status: %s\r\n",
                                         settings.espnowPeers[index][0],
                                         settings.espnowPeers[index][1],
                                         settings.espnowPeers[index][2],
                                         settings.espnowPeers[index][3],
                                         settings.espnowPeers[index][4],
                                         settings.espnowPeers[index][5],
                                         esp_err_to_name(status));
                        break;
                    }
                }
            }

            // Determine if an error occurred
            if (index < ESPNOW_MAX_PEERS)
                break;
        }

        // ESP-NOW has started successfully
        if (settings.debugEspNow)
            systemPrintf("ESP-NOW online\r\n");

        started = true;
    } while (0);

    return started;
}

//*********************************************************************
// A blocking function that is used to pair two devices
// either through the serial menu or AP config
void r4aEspNowStaticPairing()
{
    systemPrintln("Begin ESP NOW Pairing");

    // Start ESP-Now if needed, put ESP-Now into broadcast state
    r4aEspNowBeginPairing();

    // Begin sending our MAC every 250ms until a remote device sends us there info
    randomSeed(millis());

    systemPrintln("Begin pairing. Place other unit in pairing mode. Press any key to exit.");
    clearBuffer();

    bool exitPair = false;
    while (exitPair == false)
    {
        if (systemAvailable())
        {
            systemPrintln("User pressed button. Pairing canceled.");
            break;
        }

        int timeout = 1000 + random(0, 100); // Delay 1000 to 1100ms
        for (int x = 0; x < timeout; x++)
        {
            delay(1);

            if (r4aEspNowProcessRxPairedMessage() == true) // Check if we've received a pairing message
            {
                systemPrintln("Pairing compete");
                exitPair = true;
                break;
            }
        }

        r4aEspNowSendPairMessage(); // Send unit's MAC address over broadcast, no ack, no encryption

        systemPrintln("Scanning for other radio...");
    }
}

//*********************************************************************
// Stop ESP-NOW layer
bool r4aEspNowStop()
{
    uint8_t mode;
    esp_now_peer_num_t peerCount;
    uint8_t primaryChannel;
    uint8_t protocols;
    wifi_second_chan_t secondaryChannel;
    bool stopped;
    esp_err_t status;

    do
    {
        stopped = false;

        //   3. esp_now_unregister_recv_cb()
        if (settings.debugEspNow)
            systemPrintf("Calling esp_now_unregister_recv_cb\r\n");
        status = esp_now_unregister_recv_cb();
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to clear ESP_NOW RX callback, status: %d\r\n", status);
            break;
        }
        if (settings.debugEspNow)
            systemPrintf("ESP-NOW: RX callback removed\r\n");

        if (settings.debugEspNow)
            systemPrintf("ESP-NOW offline\r\n");

        //   4. Remove all peers by calling espNowRemovePeer
        if (settings.debugEspNow)
            systemPrintf("Calling esp_now_is_peer_exist\r\n");
        if (esp_now_is_peer_exist(r4aEspNowBroadcastAddr))
        {
            if (settings.debugEspNow)
                systemPrintf("Calling espNowRemovePeer\r\n");
            status = espNowRemovePeer(r4aEspNowBroadcastAddr);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to delete broadcast peer, status: %d\r\n", status);
                break;
            }
            if (settings.debugEspNow)
                systemPrintf("ESP-NOW removed broadcast peer\r\n");
        }

        // Walk the unicast peers
        while (1)
        {
            esp_now_peer_info_t peerInfo;

            // Get the next unicast peer
            if (settings.debugEspNow)
                systemPrintf("Calling esp_now_fetch_peer\r\n");
            status = esp_now_fetch_peer(true, &peerInfo);
            if (status != ESP_OK)
                break;

            // Remove the unicast peer
            if (settings.debugEspNow)
                systemPrintf("Calling espNowRemovePeer\r\n");
            status = r4aEspNowRemovePeer(peerInfo.peer_addr);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to delete peer %02x:%02x:%02x:%02x:%02x:%02x, status: %d\r\n",
                             peerInfo.peer_addr[0], peerInfo.peer_addr[1],
                             peerInfo.peer_addr[2], peerInfo.peer_addr[3],
                             peerInfo.peer_addr[4], peerInfo.peer_addr[5],
                             status);
                break;
            }
            if (settings.debugEspNow)
                systemPrintf("ESP-NOW removed peer %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                             peerInfo.peer_addr[0], peerInfo.peer_addr[1],
                             peerInfo.peer_addr[2], peerInfo.peer_addr[3],
                             peerInfo.peer_addr[4], peerInfo.peer_addr[5]);
        }
        if (status != ESP_ERR_ESPNOW_NOT_FOUND)
        {
            systemPrintf("ERROR: Failed to get peer info for deletion, status: %d\r\n", status);
            break;
        }

        // Get the number of peers
        if (settings.debugEspNow)
        {
            systemPrintf("Calling esp_now_get_peer_num\r\n");
            status = esp_now_get_peer_num(&peerCount);
            if (status != ESP_OK)
            {
                systemPrintf("ERROR: Failed to get the peer count, status: %d\r\n", status);
                break;
            }
            systemPrintf("peerCount: %d\r\n", peerCount);
        }

        // Stop the ESP-NOW state machine
        r4aEspNowSetState(ESPNOW_OFF);

        //   9. Turn off ESP-NOW. call esp_now_deinit
        if (settings.debugEspNow)
            systemPrintf("Calling esp_now_deinit\r\n");
        status = esp_now_deinit();
        if (status != ESP_OK)
        {
            systemPrintf("ERROR: Failed to deinit ESP-NOW, status: %d\r\n", status);
            break;
        }

        //  11. Restart WiFi if necessary

        // ESP-NOW has stopped successfully
        if (settings.debugEspNow)
            systemPrintf("ESP-NOW stopped\r\n");
        stopped = true;
    } while (0);

    // Return the stopped status
    return stopped;
}

//*********************************************************************
// Called from main loop
// Control incoming/outgoing RTCM data from internal ESP NOW radio
// Use the ESP32 to directly transmit/receive RTCM over 2.4GHz (no WiFi needed)
void r4aEspNowUpdate()
{
    if (settings.enableEspNow == true)
    {
        if (r4aEspNowState == ESPNOW_PAIRED || r4aEspNowState == ESPNOW_BROADCASTING)
        {
            // If it's been longer than a few ms since we last added a byte to the buffer
            // then we've reached the end of the RTCM stream. Send partial buffer.
            if (r4aEspNowOutgoingSpot > 0 && (millis() - r4aEspNowLastAdd) > 50)
            {
                if (r4aEspNowState == ESPNOW_PAIRED)
                    esp_now_send(0, (uint8_t *)&r4aEspNowOutgoing, r4aEspNowOutgoingSpot); // Send partial packet to all peers
                else // if (espNowState == ESPNOW_BROADCASTING)
                {
                    esp_now_send(r4aEspNowBroadcastAddr, (uint8_t *)&r4aEspNowOutgoing,
                                 r4aEspNowOutgoingSpot); // Send packet via broadcast
                }

                if (!inMainMenu)
                {
                    if (settings.debugEspNow == true)
                        systemPrintf("ESPNOW transmitted %d RTCM bytes\r\n", espNowBytesSent + espNowOutgoingSpot);
                }
                r4aEspNowBytesSent = 0;
                r4aEspNowOutgoingSpot = 0; // Reset
            }

            // If we don't receive an ESP NOW packet after some time, set RSSI to very negative
            // This removes the ESPNOW icon from the display when the link goes down
            if (millis() - r4aEspNowLastRssiUpdate > 5000 && r4aEspNowRSSI > -255)
                r4aEspNowRSSI = -255;
        }
    }
}

#endif  // COMPILE_ESPNOW
