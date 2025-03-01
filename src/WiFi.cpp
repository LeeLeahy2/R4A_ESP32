/**********************************************************************
  Wifi.ino

  WiFi layer, supports use by ESP-NOW, soft AP and WiFi station
**********************************************************************/

#ifdef COMPILE_WIFI

//****************************************
// Constants
//****************************************

#define WIFI_DEFAULT_CHANNEL            1
#define WIFI_IP_ADDRESS_TIMEOUT_MSEC    (15 * 1000)

static const char * wifiAuthorizationName[] =
{
    "Open",
    "WEP",
    "WPA_PSK",
    "WPA2_PSK",
    "WPA_WPA2_PSK",
    "WPA2_Enterprise",
    "WPA3_PSK",
    "WPA2_WPA3_PSK",
    "WAPI_PSK",
    "OWE",
    "WPA3_ENT_192",
};
static const int wifiAuthorizationNameEntries =
    sizeof(wifiAuthorizationName) / sizeof(wifiAuthorizationName[0]);

const char * arduinoEventName[] =
{
    "ARDUINO_EVENT_NONE",
    "ARDUINO_EVENT_ETH_START",
    "ARDUINO_EVENT_ETH_STOP",
    "ARDUINO_EVENT_ETH_CONNECTED",
    "ARDUINO_EVENT_ETH_DISCONNECTED",
    "ARDUINO_EVENT_ETH_GOT_IP",
    "ARDUINO_EVENT_ETH_LOST_IP",
    "ARDUINO_EVENT_ETH_GOT_IP6",
    "ARDUINO_EVENT_WIFI_OFF",
    "ARDUINO_EVENT_WIFI_READY",
    "ARDUINO_EVENT_WIFI_SCAN_DONE",
    "ARDUINO_EVENT_WIFI_STA_START",
    "ARDUINO_EVENT_WIFI_STA_STOP",
    "ARDUINO_EVENT_WIFI_STA_CONNECTED",
    "ARDUINO_EVENT_WIFI_STA_DISCONNECTED",
    "ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE",
    "ARDUINO_EVENT_WIFI_STA_GOT_IP",
    "ARDUINO_EVENT_WIFI_STA_GOT_IP6",
    "ARDUINO_EVENT_WIFI_STA_LOST_IP",
    "ARDUINO_EVENT_WIFI_AP_START",
    "ARDUINO_EVENT_WIFI_AP_STOP",
    "ARDUINO_EVENT_WIFI_AP_STACONNECTED",
    "ARDUINO_EVENT_WIFI_AP_STADISCONNECTED",
    "ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED",
    "ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED",
    "ARDUINO_EVENT_WIFI_AP_GOT_IP6",
    "ARDUINO_EVENT_WIFI_FTM_REPORT",
    "ARDUINO_EVENT_WPS_ER_SUCCESS",
    "ARDUINO_EVENT_WPS_ER_FAILED",
    "ARDUINO_EVENT_WPS_ER_TIMEOUT",
    "ARDUINO_EVENT_WPS_ER_PIN",
    "ARDUINO_EVENT_WPS_ER_PBC_OVERLAP",
    "ARDUINO_EVENT_SC_SCAN_DONE",
    "ARDUINO_EVENT_SC_FOUND_CHANNEL",
    "ARDUINO_EVENT_SC_GOT_SSID_PSWD",
    "ARDUINO_EVENT_SC_SEND_ACK_DONE",
    "ARDUINO_EVENT_PROV_INIT",
    "ARDUINO_EVENT_PROV_DEINIT",
    "ARDUINO_EVENT_PROV_START",
    "ARDUINO_EVENT_PROV_END",
    "ARDUINO_EVENT_PROV_CRED_RECV",
    "ARDUINO_EVENT_PROV_CRED_FAIL",
    "ARDUINO_EVENT_PROV_CRED_SUCCESS",
    "ARDUINO_EVENT_PPP_START",
    "ARDUINO_EVENT_PPP_STOP",
    "ARDUINO_EVENT_PPP_CONNECTED",
    "ARDUINO_EVENT_PPP_DISCONNECTED",
    "ARDUINO_EVENT_PPP_GOT_IP",
    "ARDUINO_EVENT_PPP_LOST_IP",
    "ARDUINO_EVENT_PPP_GOT_IP6",
};
const int arduinoEventNameEntries = sizeof(arduinoEventName) / sizeof(arduinoEventName[0]);

//****************************************
// Constants
//****************************************

// Radio operations
#define WIFI_AP_SET_MODE                         1
#define WIFI_EN_SET_MODE                         2
#define WIFI_STA_SET_MODE                        4
#define WIFI_AP_SET_PROTOCOLS                    8
#define WIFI_EN_SET_PROTOCOLS           0x00000010
#define WIFI_STA_SET_PROTOCOLS          0x00000020
#define WIFI_STA_START_SCAN             0x00000040
#define WIFI_STA_SELECT_REMOTE_AP       0x00000080
#define WIFI_AP_SELECT_CHANNEL          0x00000100
#define WIFI_EN_SELECT_CHANNEL          0x00000200
#define WIFI_STA_SELECT_CHANNEL         0x00000400

// Soft AP
#define WIFI_AP_SET_SSID_PASSWORD       0x00000800
#define WIFI_AP_SET_IP_ADDR             0x00001000
#define WIFI_AP_SET_HOST_NAME           0x00002000
#define WIFI_AP_START_DNS_SERVER        0x00004000
#define WIFI_AP_ONLINE                  0x00008000

// WiFi station
#define WIFI_STA_SET_HOST_NAME          0x00010000
#define WIFI_STA_DISABLE_AUTO_RECONNECT 0x00020000
#define WIFI_STA_CONNECT_TO_REMOTE_AP   0x00040000
#define WIFI_STA_ONLINE                 0x00080000

// ESP-NOW
#define WIFI_EN_SET_CHANNEL             0x00100000
#define WIFI_EN_SET_PROMISCUOUS_MODE    0x00200000
#define WIFI_EN_PROMISCUOUS_RX_CALLBACK 0x00400000
#define WIFI_EN_START_ESP_NOW           0x00800000
#define WIFI_EN_ESP_NOW_ONLINE          0x01000000

// WIFI_MAX_START must be the last value in the define list
#define WIFI_MAX_START                  0x02000000

const char * const wifiStartNames[] =
{
    "WIFI_AP_SET_MODE",
    "WIFI_EN_SET_MODE",
    "WIFI_STA_SET_MODE",
    "WIFI_AP_SET_PROTOCOLS",
    "WIFI_EN_SET_PROTOCOLS",
    "WIFI_STA_SET_PROTOCOLS",
    "WIFI_STA_START_SCAN",
    "WIFI_STA_SELECT_REMOTE_AP",
    "WIFI_AP_SELECT_CHANNEL",
    "WIFI_EN_SELECT_CHANNEL",
    "WIFI_STA_SELECT_CHANNEL",

    "WIFI_AP_SET_SSID_PASSWORD",
    "WIFI_AP_SET_IP_ADDR",
    "WIFI_AP_SET_HOST_NAME",
    "WIFI_AP_START_DNS_SERVER",
    "WIFI_AP_ONLINE",

    "WIFI_STA_SET_HOST_NAME",
    "WIFI_STA_DISABLE_AUTO_RECONNECT",
    "WIFI_STA_CONNECT_TO_REMOTE_AP",
    "WIFI_STA_ONLINE",

    "WIFI_EN_SET_CHANNEL",
    "WIFI_EN_SET_PROMISCUOUS_MODE",
    "WIFI_EN_PROMISCUOUS_RX_CALLBACK",
    "WIFI_EN_START_ESP_NOW",
    "WIFI_EN_ESP_NOW_ONLINE",
};
const int wifiStartNamesEntries = sizeof(wifiStartNames) / sizeof(wifiStartNames[0]);

#define WIFI_START_ESP_NOW          (WIFI_EN_SET_MODE                   \
                                     | WIFI_EN_SET_PROTOCOLS            \
                                     | WIFI_EN_SELECT_CHANNEL           \
                                     | WIFI_EN_SET_CHANNEL              \
                                     | WIFI_EN_PROMISCUOUS_RX_CALLBACK  \
                                     | WIFI_EN_SET_PROMISCUOUS_MODE     \
                                     | WIFI_EN_START_ESP_NOW            \
                                     | WIFI_EN_ESP_NOW_ONLINE)

#define WIFI_START_SOFT_AP          (WIFI_AP_SET_MODE               \
                                     | WIFI_AP_SET_PROTOCOLS        \
                                     | WIFI_AP_SELECT_CHANNEL       \
                                     | WIFI_AP_SET_SSID_PASSWORD    \
                                     | WIFI_AP_SET_IP_ADDR          \
                                     | WIFI_AP_SET_HOST_NAME        \
                                     | WIFI_AP_START_DNS_SERVER     \
                                     | WIFI_AP_ONLINE)

#define WIFI_START_STATION          (WIFI_STA_SET_MODE                  \
                                     | WIFI_STA_SET_PROTOCOLS           \
                                     | WIFI_STA_START_SCAN              \
                                     | WIFI_STA_SELECT_CHANNEL          \
                                     | WIFI_STA_SELECT_REMOTE_AP        \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_STA_ONLINE)

#define WIFI_STA_RECONNECT          (WIFI_STA_START_SCAN                \
                                     | WIFI_STA_SELECT_CHANNEL          \
                                     | WIFI_STA_SELECT_REMOTE_AP        \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_STA_ONLINE)

#define WIFI_SELECT_CHANNEL         (WIFI_AP_SELECT_CHANNEL     \
                                     | WIFI_EN_SELECT_CHANNEL   \
                                     | WIFI_STA_SELECT_CHANNEL)

#define WIFI_STA_NO_REMOTE_AP       (WIFI_STA_SELECT_CHANNEL            \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_STA_ONLINE)

#define WIFI_STA_FAILED_SCAN        (WIFI_STA_START_SCAN          \
                                     | WIFI_STA_SELECT_REMOTE_AP  \
                                     | WIFI_STA_NO_REMOTE_AP)

#define WIFI_MAX_TIMEOUT    (15 * 60 * 1000)    // Timeout in milliseconds
#define WIFI_MIN_TIMEOUT    (15 * 1000)         // Timeout in milliseconds

//****************************************
// Locals
//****************************************

// DNS server for Captive Portal
static DNSServer dnsServer;

// Start timeout
static uint32_t wifiStartTimeout;

static int wifiFailedConnectionAttempts = 0; // Count the number of connection attempts between restarts
static bool wifiReconnectRequest; // Set true to request WiFi reconnection

//*********************************************************************
// Set WiFi credentials
// Enable TCP connections
void menuWiFi()
{
    while (1)
    {
        networkDisplayInterface(NETWORK_WIFI_STATION);

        Serial.println();
        Serial.println("Menu: WiFi Networks");

        for (int x = 0; x < MAX_WIFI_NETWORKS; x++)
        {
            Serial.printf("%d) SSID %d: %s\r\n", (x * 2) + 1, x + 1, *r4aWifiSsidPassword[x].ssid);
            Serial.printf("%d) Password %d: %s\r\n", (x * 2) + 2, x + 1, *r4aWifiSsidPassword[x].password);
        }

        Serial.print("a) Configure device via WiFi Access Point or connect to WiFi: ");
        Serial.printf("%s\r\n", settings.wifiConfigOverAP ? "AP" : "WiFi");

        Serial.print("c) Captive Portal: ");
        Serial.printf("%s\r\n", settings.enableCaptivePortal ? "Enabled" : "Disabled");

        Serial.println("x) Exit");

        byte incoming = getUserInputCharacterNumber();

        if (incoming >= 1 && incoming <= MAX_WIFI_NETWORKS * 2)
        {
            int arraySlot = ((incoming - 1) / 2); // Adjust incoming to array starting at 0

            if (incoming % 2 == 1)
            {
                Serial.printf("Enter SSID network %d: ", arraySlot + 1);
                getUserInputString(*r4aWifiSsidPassword[arraySlot].ssid,
                                   sizeof(*r4aWifiSsidPassword[arraySlot].ssid));
            }
            else
            {
                Serial.printf("Enter Password for %s: ", *r4aWifiSsidPassword[arraySlot].ssid);
                getUserInputString(*r4aWifiSsidPassword[arraySlot].password,
                                   sizeof(*r4aWifiSsidPassword[arraySlot].password));
            }

            // If we are modifying the SSID table, force restart of WiFi
            wifiRestartRequested = true;
            wifiFailedConnectionAttempts = 0;
        }
        else if (incoming == 'a')
        {
            settings.wifiConfigOverAP ^= 1;
            wifiRestartRequested = true;
        }
        else if (incoming == 'c')
        {
            settings.enableCaptivePortal ^= 1;
        }
        else if (incoming == 'x')
            break;
        else if (incoming == INPUT_RESPONSE_GETCHARACTERNUMBER_EMPTY)
            break;
        else if (incoming == INPUT_RESPONSE_GETCHARACTERNUMBER_TIMEOUT)
            break;
        else
            printUnknown(incoming);
    }

    // Erase passwords from empty SSID entries
    for (int x = 0; x < MAX_WIFI_NETWORKS; x++)
    {
        if (strlen(*r4aWifiSsidPassword[x].ssid) == 0)
            strcpy(*r4aWifiSsidPassword[x].password, "");
    }

    clearBuffer(); // Empty buffer of any newline chars
}

//*********************************************************************
// Display the WiFi state
void wifiDisplayState()
{
    Serial.printf("WiFi: %s\r\n", networkInterfaceHasInternet(NETWORK_WIFI_STATION) ? "Online" : "Offline");
    Serial.printf("    MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", wifiMACAddress[0], wifiMACAddress[1],
                 wifiMACAddress[2], wifiMACAddress[3], wifiMACAddress[4], wifiMACAddress[5]);
    if (networkInterfaceHasInternet(NETWORK_WIFI_STATION))
    {
        // Get the DNS addresses
        IPAddress dns1 = WiFi.STA.dnsIP(0);
        IPAddress dns2 = WiFi.STA.dnsIP(1);
        IPAddress dns3 = WiFi.STA.dnsIP(2);

        // Get the WiFi status
        wl_status_t wifiStatus = WiFi.status();

        const char *wifiStatusString = wifiPrintState(wifiStatus);

        // Display the WiFi state
        Serial.printf("    SSID: %s\r\n", WiFi.STA.SSID());
        Serial.printf("    IP Address: %s\r\n", WiFi.STA.localIP().toString().c_str());
        Serial.printf("    Subnet Mask: %s\r\n", WiFi.STA.subnetMask().toString().c_str());
        Serial.printf("    Gateway Address: %s\r\n", WiFi.STA.gatewayIP().toString().c_str());
        if ((uint32_t)dns3)
            Serial.printf("    DNS Address: %s, %s, %s\r\n", dns1.toString().c_str(), dns2.toString().c_str(),
                         dns3.toString().c_str());
        else if ((uint32_t)dns3)
            Serial.printf("    DNS Address: %s, %s\r\n", dns1.toString().c_str(), dns2.toString().c_str());
        else
            Serial.printf("    DNS Address: %s\r\n", dns1.toString().c_str());
        Serial.printf("    WiFi Strength: %d dBm\r\n", WiFi.RSSI());
        Serial.printf("    WiFi Status: %d (%s)\r\n", wifiStatus, wifiStatusString);
    }
}

//*********************************************************************
// Start or stop ESP-NOW
// Inputs:
//   on: Set to true to start ESP-NOW and false to stop ESP-NOW
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool wifiEspNowOn(bool on, const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (settings.debugEspNow || wifi->_debug)
        Serial.printf("wifiEspNow(%s) called in %s at line %d\r\n",
                     on ? "true" : "false", fileName, lineNumber);

    // Don't turn on ESP-NOW when it is disabled
    if (settings.enableEspNow == false)
        on = false;
    if (((on == false) && wifiEspNowRunning)
        || ((settings.enableEspNow == true) && !wifiEspNowRunning))
        return wifi.enable(on, wifiSoftApRunning, wifiStationRunning, wifi->__FILE__, __LINE__);
    return settings.enableEspNow;
}

//*********************************************************************
// Return the start timeout in milliseconds
uint32_t wifiGetStartTimeout()
{
    return wifiStartTimeout;
}

//*********************************************************************
// Counts the number of entered SSIDs
int wifiNetworkCount()
{
    // Count SSIDs
    int networkCount = 0;
    for (int x = 0; x < MAX_WIFI_NETWORKS; x++)
    {
        if (strlen(*r4aWifiSsidPassword[x].ssid) > 0)
            networkCount++;
    }
    return networkCount;
}

//*********************************************************************
// Given a status, return the associated state or error
const char *wifiPrintState(wl_status_t wifiStatus)
{
    switch (wifiStatus)
    {
    case WL_NO_SHIELD:       // 255
        return ("WL_NO_SHIELD");
    case WL_STOPPED:         // 254
        return ("WL_STOPPED");
    case WL_IDLE_STATUS:     // 0
        return ("WL_IDLE_STATUS");
    case WL_NO_SSID_AVAIL:   // 1
        return ("WL_NO_SSID_AVAIL");
    case WL_SCAN_COMPLETED:  // 2
        return ("WL_SCAN_COMPLETED");
    case WL_CONNECTED:       // 3
        return ("WL_CONNECTED");
    case WL_CONNECT_FAILED:  // 4
        return ("WL_CONNECT_FAILED");
    case WL_CONNECTION_LOST: // 5
        return ("WL_CONNECTION_LOST");
    case WL_DISCONNECTED:    // 6
        return ("WL_DISCONNECTED");
    }
    return ("WiFi Status Unknown");
}

//*********************************************************************
// Callback for all WiFi RX Packets
// Get RSSI of all incoming management packets: https://esp32.com/viewtopic.php?t=13889
void wifiPromiscuousRxHandler(void *buf, wifi_promiscuous_pkt_type_t type)
{
    const wifi_promiscuous_pkt_t *ppkt; // Defined in esp_wifi_types_native.h

    // All espnow traffic uses action frames which are a subtype of the
    // mgmnt frames so filter out everything else.
    if (type != WIFI_PKT_MGMT)
        return;

    ppkt = (wifi_promiscuous_pkt_t *)buf;
    packetRSSI = ppkt->rx_ctrl.rssi;
}

//*********************************************************************
// Reset the last WiFi start attempt
// Useful when WiFi settings have changed
void wifiResetThrottleTimeout()
{
    wifiReconnectionTimer = millis() - WIFI_MAX_TIMEOUT;
}

//*********************************************************************
// Set WiFi timeout back to zero
// Useful if other things (such as a successful ethernet connection) need
// to reset wifi timeout
void wifiResetTimeout()
{
    wifiStartTimeout = 0;
    if (_debug == true)
        Serial.println("WiFi: Start timeout reset to zero");
}

//*********************************************************************
// Turn on and off WiFi soft AP mode
// Inputs:
//   on: True to start WiFi soft AP mode, false to stop WiFi soft AP mode
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns the status of WiFi soft AP start or stop
bool wifiSoftApOn(bool on, const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (_debug)
        Serial.printf("wifiSoftApOn(%s) called in %s at line %d\r\n",
                     on ? "true" : "false", fileName, lineNumber);

    return wifi.enable(wifiEspNowRunning, on, wifiStationRunning, wifi->__FILE__, __LINE__);
}
    // Check for network shutdown
    else if (networkConsumerCount == 0)
    {
        // Stop the connection attempts
        wifiResetThrottleTimeout();
        wifiResetTimeout();
        networkSequenceStopPolling(NETWORK_WIFI_STATION, debug, true);
    }
}

//*********************************************************************
// Start or stop the WiFi station
// Inputs:
//   on: True to start WiFi station mode, false to stop WiFi station mode
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool wifiStationOn(bool on, const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (_debug)
        Serial.printf("wifiStationOn(%s) called in %s at line %d\r\n",
                     on ? "true" : "false", fileName, lineNumber);

    return wifi.enable(wifiEspNowRunning, wifiSoftApRunning, on, wifi->__FILE__, __LINE__);
}

//*********************************************************************
// Handle WiFi station reconnection requests
bool wifiStationReconnectionRequest()
{
    bool connected;
    int minutes;
    int seconds;

    // Restart delay
    connected = false;
    if ((millis() - wifiReconnectionTimer) < wifiStartTimeout)
        return connected;
    wifiReconnectionTimer = millis();

    // Check for a reconnection request
    if (wifiReconnectRequest)
    {
        wifiReconnectRequest = false;
        if (_debug)
            Serial.printf("WiFi: Attempting WiFi restart\r\n");
        wifi.clearStarted(WIFI_STA_RECONNECT);
    }

    // Attempt to start WiFi station
    if (wifiStationOn(true, wifi->__FILE__, __LINE__))
    {
        // Successfully connected to a remote AP
        connected = true;
        if (_debug)
            Serial.printf("WiFi: WiFi station successfully started\r\n");
        networkSequenceNextEntry(NETWORK_WIFI_STATION, settings.debugNetworkLayer);
        wifiFailedConnectionAttempts = 0;
    }
    else
    {
        // Failed to connect to a remote AP
        if (_debug)
            Serial.printf("WiFi: WiFi station failed to start!\r\n");

        // Account for this connection attempt
        wifiFailedConnectionAttempts++;

        // Start the next network interface if necessary
        if (wifiFailedConnectionAttempts >= 2)
            networkStartNextInterface(NETWORK_WIFI_STATION);

        // Increase the timeout
        wifiStartTimeout <<= 1;
        if (!wifiStartTimeout)
            wifiStartTimeout = WIFI_MIN_TIMEOUT;
        else if (wifiStartTimeout > WIFI_MAX_TIMEOUT)
            wifiStartTimeout = WIFI_MAX_TIMEOUT;

        // Display the delay
        seconds = wifiStartTimeout / MILLISECONDS_IN_A_SECOND;
        minutes = seconds / SECONDS_IN_A_MINUTE;
        seconds -= minutes * SECONDS_IN_A_MINUTE;
        if (_debug)
            Serial.printf("WiFi: Delaying %2d:%02d before restarting WiFi\r\n", minutes, seconds);
    }
    return connected;
}
    wifi.enable(wifiEspNowRunning, wifiSoftApRunning, false, wifi->__FILE__, __LINE__);

    networkSequenceNextEntry(NETWORK_WIFI_STATION, settings.debugNetworkLayer);
}

//*********************************************************************
// Stop WiFi and release all resources
void wifiStopAll()
{
    // Stop the web server
    stopWebServer();

    // Stop the Wifi layer
    wifi.enable(false, false, false, wifi->__FILE__, __LINE__);

    // Take the network offline
    networkInterfaceEventInternetLost(NETWORK_WIFI_STATION);

    // Display the heap state
    reportHeapNow(_debug);
}

//*********************************************************************
// Constructor
// Inputs:
//   verbose: Set to true to display additional WiFi debug data
wifiBegin(R4A_WIFI * wifi, bool debug, bool verbose)
    : wifi->_apChannel{0}, _apCount{0}, _apDnsAddress{IPAddress((uint32_t)0)},
      wifi->_apFirstDhcpAddress{IPAddress("192.168.4.32")},
      wifi->_apGatewayAddress{(uint32_t)0},
      wifi->_apIpAddress{IPAddress("192.168.4.1")},
      wifi->_apMacAddress{0, 0, 0, 0, 0, 0},
      wifi->_apSubnetMask{IPAddress("255.255.255.0")},
      wifi->_espNowChannel{0},
      wifi->_scanRunning{false},
      wifi->_staIpAddress{IPAddress((uint32_t)0)}, _staIpType{0},
      wifi->_staMacAddress{0, 0, 0, 0, 0, 0},
      wifi->_staRemoteApSsid{nullptr}, _staRemoteApPassword{nullptr},
      wifi->_started{false}, _stationChannel{0},
      wifi->_usingDefaultChannel{true}, _verbose{verbose}
{
    wifiChannel = 0;
    wifiEspNowOnline = false;
    wifiEspNowRunning = false;
    wifiFailedConnectionAttempts = 0;
    wifiReconnectionTimer = 0;
    wifiRestartRequested = false;
    wifiSoftApOnline = false;
    wifiSoftApRunning = false;
    wifiStationOnline = false;
    wifiStationRunning = false;

    // Prepare to start WiFi immediately
    wifiResetThrottleTimeout();
    wifiResetTimeout();
}

//*********************************************************************
// Clear some of the started components
// Inputs:
//   components: Bitmask of components to clear
// Outputs:
//   Returns the bitmask of started components
WIFI_ACTION_t wifiClearStarted(R4A_WIFI * wifi, WIFI_ACTION_t components)
{
    wifi->_started = _started & ~components;
    return wifi->_started;
}

//*********************************************************************
// Attempts a connection to all provided SSIDs
bool wifiConnect(R4A_WIFI * wifi, unsigned long timeout,
                       bool startAP)
{
    bool started;

    // Display warning
    log_w("WiFi: Not using timeout parameter for connect!\r\n");

    // Enable WiFi station if necessary
    if (wifiStationRunning == false)
    {
        displayWiFiConnect();
        started = enable(wifiEspNowRunning, wifiSoftApRunning, true, wifi->__FILE__, __LINE__);
    }
    else if (startAP && !wifiSoftApRunning)
        started = enable(wifiEspNowRunning, true, wifiStationRunning, wifi->__FILE__, __LINE__);

    // Determine the WiFi station status
    if (started)
    {
        wl_status_t wifiStatus = WiFi.STA.status();
        started = (wifiStatus == WL_CONNECTED);
        if (wifiStatus == WL_DISCONNECTED)
            Serial.print("No friendly WiFi networks detected.\r\n");
        else if (wifiStatus != WL_CONNECTED)
            Serial.printf("WiFi failed to connect: error #%d - %s\r\n",
                         wifiStatus, wifiPrintState(wifiStatus));
    }
    return started;
}

//*********************************************************************
// Display components begin started or stopped
// Inputs:
//   text: Text describing the component list
//   components: A bit mask of the components
void wifiDisplayComponents(R4A_WIFI * wifi, const char * text, WIFI_ACTION_t components)
{
    WIFI_ACTION_t mask;

    Serial.printf("%s: 0x%08x\r\n", text, components);
    for (int index = wifiStartNamesEntries - 1; index >= 0; index--)
    {
        mask = 1 << index;
        if (components & mask)
            Serial.printf("    0x%08lx: %s\r\n", mask, wifiStartNames[index]);
    }
}

//*********************************************************************
// Enable or disable the WiFi modes
// Inputs:
//   enableESPNow: Enable ESP-NOW mode
//   enableSoftAP: Enable soft AP mode
//   enableStataion: Enable station mode
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if the modes were successfully configured
bool wifiEnable(R4A_WIFI * wifi, bool enableESPNow,
                      bool enableSoftAP,
                      bool enableStation,
                      const char * fileName,
                      int lineNumber)
{
    int authIndex;
    WIFI_ACTION_t starting;
    bool status;
    WIFI_ACTION_t stopping;

    // Turn on WiFi debugging if necessary
    if (_verbose)
    {
        settings.debugEspNow = true;
        wifi->_debug = true;
    }

    // Determine the next actions
    starting = 0;
    stopping = 0;

    // Display the parameters
    if (_debug && wifi->_verbose)
    {
        Serial.printf(R4A_WIFI * wifi, "WiFi: wifiEnable called from %s line %d\r\n", fileName, lineNumber);
        Serial.printf("enableESPNow: %s\r\n", enableESPNow ? "true" : "false");
        Serial.printf("enableSoftAP: %s\r\n", enableSoftAP ? "true" : "false");
        Serial.printf("enableStation: %s\r\n", enableStation ? "true" : "false");
    }

    // Update the ESP-NOW state
    if (enableESPNow)
    {
        starting |= WIFI_START_ESP_NOW;
        wifiEspNowRunning = true;
    }
    else
    {
        stopping |= WIFI_START_ESP_NOW;
        wifiEspNowRunning = false;
    }

    // Update the soft AP state
    if (enableSoftAP)
    {
        // Verify that the SSID is set
        if (wifiSoftApSsid && strlen(wifiSoftApSsid))
        {
            starting |= WIFI_START_SOFT_AP;
            wifiSoftApRunning = true;
        }
        else
            Serial.printf("ERROR: AP SSID or password is missing\r\n");
    }
    else
    {
        stopping |= WIFI_START_SOFT_AP;
        wifiSoftApRunning = false;
    }

    // Update the station state
    if (enableStation)
    {
        // Verify that at least one SSID is set
        for (authIndex = 0; authIndex < MAX_WIFI_NETWORKS; authIndex++)
            if (strlen(*r4aWifiSsidPassword[authIndex].ssid))
                break;
        if (authIndex >= MAX_WIFI_NETWORKS)
        {
            Serial.printf("ERROR: No valid SSID in settings\r\n");
            displayNoSSIDs(2000);
        }
        else
        {
            // Start the WiFi station
            starting |= WIFI_START_STATION;
            wifiStationRunning = true;
        }
    }
    else
    {
        // Stop the WiFi station
        stopping |= WIFI_START_STATION;
        wifiStationRunning = false;
    }

    // Stop and start the WiFi components
    status = stopStart(stopping, starting);
    if (_debug && wifi->_verbose)
        Serial.printf(R4A_WIFI * wifi, "WiFi: wifiEnable returning %s\r\n", status ? "true" : "false");
    return status;
}

//*********************************************************************
// Get the ESP-NOW status
// Outputs:
//   Returns true when ESP-NOW is online and ready for use
bool wifiEspNowOnline(R4A_WIFI * wifi)
{
    return (_started & WIFI_EN_ESP_NOW_ONLINE) ? true : false;
}

//*********************************************************************
// Set the ESP-NOW channel
// Inputs:
//   channel: New ESP-NOW channel number
void wifiEspNowSetChannel(R4A_WIFI * wifi, WIFI_CHANNEL_t channel)
{
    wifi->_espNowChannel = channel;
}

//*********************************************************************
// Handle the WiFi event
void wifiEventHandler(R4A_WIFI * wifi, arduino_event_id_t event, arduino_event_info_t info)
{
    bool success;

    if (_debug)
        Serial.printf("event: %d (%s)\r\n", event, arduinoEventName[event]);

    // Handle the event
    switch (event)
    {

    //------------------------------
    // Controller events
    //------------------------------

    case ARDUINO_EVENT_WIFI_OFF:
    case ARDUINO_EVENT_WIFI_READY:
        break;

    //----------------------------------------
    // Scan events
    //----------------------------------------

    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        stationEventHandler(event, info);
        break;

    //------------------------------
    // Station events
    //------------------------------
    case ARDUINO_EVENT_WIFI_STA_START:
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    case ARDUINO_EVENT_WIFI_STA_STOP:
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        stationEventHandler(event, info);
        break;

    //----------------------------------------
    // Soft AP events
    //----------------------------------------

    case ARDUINO_EVENT_WIFI_AP_START:
    case ARDUINO_EVENT_WIFI_AP_STOP:
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        softApEventHandler(event, info);
        break;
    }
}

//*********************************************************************
// Get the current WiFi channel
// Outputs:
//   Returns the current WiFi channel number
WIFI_CHANNEL_t wifiGetChannel(R4A_WIFI * wifi)
{
    return wifiChannel;
}

//*********************************************************************
// Set the WiFi mode
// Inputs:
//   setMode: Modes to set
//   xorMode: Modes to toggle
//
// Math: result = (mode | setMode) ^ xorMode
//
//                              setMode
//                      0                   1
//  xorMode 0       No change           Set bit
//          1       Toggle bit          Clear bit
//
// Outputs:
//   Returns true if successful and false upon failure
bool wifiSetWiFiMode(R4A_WIFI * wifi, uint8_t setMode, uint8_t xorMode)
{
    uint8_t mode;
    uint8_t newMode;
    bool started;

    started = false;
    do
    {
        // Get the current mode
        mode = (uint8_t)WiFi.getMode();
        if (_debug && wifi->_verbose)
            Serial.printf("Current WiFi mode: 0x%08x (%s)\r\n",
                         mode,
                         ((mode == 0) ? "WiFi off"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP"
                         : "STA"))));

        // Determine the new mode
        newMode = (mode | setMode) ^ xorMode;
        started = (newMode == mode);
        if (started)
            break;

        // Set the new mode
        started = WiFi.mode((wifi_mode_t)newMode);
        if (!started)
        {
            reportHeapNow(true);
            Serial.printf("Current WiFi mode: 0x%08x (%s)\r\n",
                         mode,
                         ((mode == 0) ? "WiFi off"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA"
                         : ((mode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP"
                         : "STA"))));
            Serial.printf("ERROR: Failed to set %d (%s)!\r\n",
                         newMode,
                         ((newMode == 0) ? "WiFi off"
                         : ((newMode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA mode"
                         : ((newMode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP mode"
                         : "STA mode"))));
            break;
        }
        if (_debug && wifi->_verbose)
            Serial.printf("Set WiFi: %d (%s)\r\n",
                         newMode,
                         ((newMode == 0) ? "Off"
                         : ((newMode & (WIFI_MODE_AP | WIFI_MODE_STA)) == (WIFI_MODE_AP | WIFI_MODE_STA) ? "Soft AP + STA mode"
                         : ((newMode & (WIFI_MODE_AP | WIFI_MODE_STA)) == WIFI_MODE_AP ? "Soft AP mode"
                         : "STA mode"))));
    } while (0);

    // Return the final status
    return started;
}

//*********************************************************************
// Set the WiFi radio protocols
// Inputs:
//   interface: Interface on which to set the protocols
//   enableWiFiProtocols: When true, enable the WiFi protocols
//   enableLongRangeProtocol: When true, enable the long range protocol
// Outputs:
//   Returns true if successful and false upon failure
bool wifiSetWiFiProtocols(R4A_WIFI * wifi, wifi_interface_t interface,
                                bool enableWiFiProtocols,
                                bool enableLongRangeProtocol)
{
    uint8_t newProtocols;
    uint8_t oldProtocols;
    bool started;
    esp_err_t status;

    started = false;
    do
    {
        // Get the current protocols
        status = esp_wifi_get_protocol(interface, &oldProtocols);
        started = (status == ESP_OK);
        if (!started)
        {
            Serial.printf("ERROR: Failed to get the WiFi %s radio protocols!\r\n",
                         (interface == WIFI_IF_AP) ? "soft AP" : "station");
            break;
        }
        if (_debug && wifi->_verbose)
            Serial.printf("Current WiFi protocols (%d%s%s%s%s%s)\r\n",
                         oldProtocols,
                         oldProtocols & WIFI_PROTOCOL_11AX ? ", 11AX" : "",
                         oldProtocols & WIFI_PROTOCOL_11B ? ", 11B" : "",
                         oldProtocols & WIFI_PROTOCOL_11G ? ", 11G" : "",
                         oldProtocols & WIFI_PROTOCOL_11N ? ", 11N" : "",
                         oldProtocols & WIFI_PROTOCOL_LR ? ", LR" : "");

        // Determine which protocols to enable
        newProtocols = oldProtocols;
        if (enableLongRangeProtocol || enableWiFiProtocols)
        {
            // Enable the WiFi protocols
            newProtocols |= WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N;

            // Enable the ESP-NOW long range protocol
            if (enableLongRangeProtocol)
                newProtocols |= WIFI_PROTOCOL_LR;
            else
                newProtocols &= ~WIFI_PROTOCOL_LR;
        }

        // Disable the protocols
        else
            newProtocols = 0;

        // Display the new protocols
        if (_debug && wifi->_verbose)
            Serial.printf("Setting WiFi protocols (%d%s%s%s%s%s)\r\n",
                         newProtocols,
                         newProtocols & WIFI_PROTOCOL_11AX ? ", 11AX" : "",
                         newProtocols & WIFI_PROTOCOL_11B ? ", 11B" : "",
                         newProtocols & WIFI_PROTOCOL_11G ? ", 11G" : "",
                         newProtocols & WIFI_PROTOCOL_11N ? ", 11N" : "",
                         newProtocols & WIFI_PROTOCOL_LR ? ", LR" : "");

        // Set the new protocols
        started = true;
        if (newProtocols != oldProtocols)
        {
            status = esp_wifi_set_protocol(interface, newProtocols);
            started = (status == ESP_OK);
        }
        if (!started)
        {
            Serial.printf("Current WiFi protocols (%d%s%s%s%s%s)\r\n",
                         oldProtocols,
                         oldProtocols & WIFI_PROTOCOL_11AX ? ", 11AX" : "",
                         oldProtocols & WIFI_PROTOCOL_11B ? ", 11B" : "",
                         oldProtocols & WIFI_PROTOCOL_11G ? ", 11G" : "",
                         oldProtocols & WIFI_PROTOCOL_11N ? ", 11N" : "",
                         oldProtocols & WIFI_PROTOCOL_LR ? ", LR" : "");
            Serial.printf("Setting WiFi protocols (%d%s%s%s%s%s)\r\n",
                         newProtocols,
                         newProtocols & WIFI_PROTOCOL_11AX ? ", 11AX" : "",
                         newProtocols & WIFI_PROTOCOL_11B ? ", 11B" : "",
                         newProtocols & WIFI_PROTOCOL_11G ? ", 11G" : "",
                         newProtocols & WIFI_PROTOCOL_11N ? ", 11N" : "",
                         newProtocols & WIFI_PROTOCOL_LR ? ", LR" : "");
            Serial.printf("ERROR: Failed to set the WiFi %s radio protocols!\r\n",
                         (interface == WIFI_IF_AP) ? "soft AP" : "station");
            break;
        }
    } while (0);

    // Return the final status
    return started;
}

//*********************************************************************
// Configure the soft AP
// Inputs:
//   ipAddress: IP address of the soft AP
//   subnetMask: Subnet mask for the soft AP network
//   firstDhcpAddress: First IP address to use in the DHCP range
//   dnsAddress: IP address to use for DNS lookup (translate name to IP address)
//   gatewayAddress: IP address of the gateway to a larger network (internet?)
// Outputs:
//   Returns true if the soft AP was successfully configured.
bool wifiSoftApConfiguration(R4A_WIFI * wifi, IPAddress ipAddress,
                                   IPAddress subnetMask,
                                   IPAddress firstDhcpAddress,
                                   IPAddress dnsAddress,
                                   IPAddress gatewayAddress)
{
    bool success;

    wifi->_apIpAddress = ipAddress;
    wifi->_apSubnetMask = subnetMask;
    wifi->_apFirstDhcpAddress = firstDhcpAddress;
    wifi->_apDnsAddress = dnsAddress;
    wifi->_apGatewayAddress = gatewayAddress;

    // Restart the soft AP if necessary
    success = true;
    if (softApOnline())
    {
        success = enable(false, false, wifiStationRunning, wifi->__FILE__, __LINE__);
        if (success)
            success = enable(false, true, wifiStationRunning, wifi->__FILE__, __LINE__);
    }
    return success;
}

//*********************************************************************
// Display the soft AP configuration
// Inputs:
//   display: Address of a Print object
void wifiSoftApConfigurationDisplay(R4A_WIFI * wifi, Print * display)
{
    display->printf("Soft AP configuration:\r\n");
    display->printf("    %s: IP Address\r\n", wifi->_apIpAddress.toString().c_str());
    display->printf("    %s: Subnet mask\r\n", wifi->_apSubnetMask.toString().c_str());
    if ((uint32_t)_apFirstDhcpAddress)
        display->printf("    %s: First DHCP address\r\n", wifi->_apFirstDhcpAddress.toString().c_str());
    if ((uint32_t)_apDnsAddress)
        display->printf("    %s: DNS address\r\n", wifi->_apDnsAddress.toString().c_str());
    if ((uint32_t)_apGatewayAddress)
        display->printf("    %s: Gateway address\r\n", wifi->_apGatewayAddress.toString().c_str());
}

//*********************************************************************
// Handle the soft AP events
void wifiSoftApEventHandler(R4A_WIFI * wifi, arduino_event_id_t event, arduino_event_info_t info)
{
    // Handle the event
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_AP_STOP:
        // Mark the soft AP as offline
        if (_debug && softApOnline())
            Serial.printf("AP: Offline\r\n");
        wifi->_started = _started & ~WIFI_AP_ONLINE;
        if (_debug && wifi->_verbose)
            Serial.printf("_started: 0x%08x\r\n", wifi->_started);
        break;
    }
}

//*********************************************************************
// Set the soft AP host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool wifiSoftApSetHostName(R4A_WIFI * wifi, const char * hostName)
{
    bool nameSet;

    do
    {
        // Verify that a host name was specified
        nameSet =  (hostName != nullptr) && (strlen(hostName) != 0);
        if (!nameSet)
        {
            Serial.printf("ERROR: No host name specified!\r\n");
            break;
        }

        // Set the host name
        if (_debug)
            Serial.printf("WiFI setting AP host name\r\n");
        nameSet = WiFi.AP.setHostname(hostName);
        if (!nameSet)
        {
            Serial.printf("ERROR: Failed to set the Wifi AP host name!\r\n");
            break;
        }
        if (_debug)
            Serial.printf("WiFi AP hostname: %s\r\n", hostName);
    } while (0);
    return nameSet;
}

//*********************************************************************
// Set the soft AP configuration
// Inputs:
//   ipAddress: IP address of the server, nullptr or empty string causes
//              default 192.168.4.1 to be used
//   subnetMask: Subnet mask for local network segment, nullptr or empty
//              string causes default 0.0.0.0 to be used, unless ipAddress
//              is not specified, in which case 255.255.255.0 is used
//   gatewayAddress: Gateway to internet IP address, nullptr or empty string
//            causes default 0.0.0.0 to be used (no access to internet)
//   dnsAddress: Domain name server (name to IP address translation) IP address,
//              nullptr or empty string causes 0.0.0.0 to be used (only
//              mDNS name translation, if started)
//   dhcpStartAddress: Start of DHCP IP address assignments for the local
//              network segment, nullptr or empty string causes default
//              0.0.0.0 to be used (disable DHCP server)  unless ipAddress
//              was not specified in which case 192.168.4.2
// Outputs:
//   Returns true if successful and false upon failure
bool wifiSoftApSetIpAddress(R4A_WIFI * wifi, const char * ipAddress,
                                  const char * subnetMask,
                                  const char * gatewayAddress,
                                  const char * dnsAddress,
                                  const char * dhcpFirstAddress)
{
    bool configured;
    uint32_t uDhcpFirstAddress;
    uint32_t uDnsAddress;
    uint32_t uGatewayAddress;
    uint32_t uIpAddress;
    uint32_t uSubnetMask;

    // Convert the IP address
    if ((!ipAddress) || (strlen(ipAddress) == 0))
        uIpAddress = 0;
    else
        uIpAddress = (uint32_t)IPAddress(ipAddress);

    // Convert the subnet mask
    if ((!subnetMask) || (strlen(subnetMask) == 0))
    {
        if (uIpAddress == 0)
            uSubnetMask = IPAddress("255.255.255.0");
        else
            uSubnetMask = 0;
    }
    else
        uSubnetMask = (uint32_t)IPAddress(subnetMask);

    // Convert the gateway address
    if ((!gatewayAddress) || (strlen(gatewayAddress) == 0))
        uGatewayAddress = 0;
    else
        uGatewayAddress = (uint32_t)IPAddress(gatewayAddress);

    // Convert the first DHCP address
    if ((!dhcpFirstAddress) || (strlen(dhcpFirstAddress) == 0))
    {
        if (uIpAddress == 0)
            uDhcpFirstAddress = IPAddress("192.168.4.32");
        else
            uDhcpFirstAddress = uIpAddress + 0x1f000000;
    }
    else
        uDhcpFirstAddress = (uint32_t)IPAddress(dhcpFirstAddress);

    // Convert the DNS address
    if ((!dnsAddress) || (strlen(dnsAddress) == 0))
        uDnsAddress = 0;
    else
        uDnsAddress = (uint32_t)IPAddress(dnsAddress);

    // Use the default IP address if not specified
    if (uIpAddress == 0)
        uIpAddress = IPAddress("192.168.4.1");

    // Display the soft AP configuration
    if (_debug)
        softApConfigurationDisplay(&Serial);

    // Configure the soft AP
    configured = WiFi.AP.config(IPAddress(uIpAddress),
                                IPAddress(uGatewayAddress),
                                IPAddress(uSubnetMask),
                                IPAddress(uDhcpFirstAddress),
                                IPAddress(uDnsAddress));
    if (!configured)
        Serial.printf("ERROR: Failed to configure the soft AP with IP addresses!\r\n");
    return configured;
}

//*********************************************************************
// Get the soft AP IP address
// Returns the soft IP address
IPAddress wifiSoftApIpAddress(R4A_WIFI * wifi)
{
    if (softApOnline())
        return wifi->_apIpAddress;
    return IPAddress((uint32_t)0);
}

//*********************************************************************
// Get the soft AP status
bool wifiSoftApOnline(R4A_WIFI * wifi)
{
    return (_started & WIFI_AP_ONLINE) ? true : false;
}

//*********************************************************************
// Set the soft AP SSID and password
// Outputs:
//   Returns true if successful and false upon failure
bool wifiSoftApSetSsidPassword(R4A_WIFI * wifi, const char * ssid, const char * password)
{
    bool created;

    // Set the WiFi soft AP SSID and password
    if (_debug)
        Serial.printf("WiFi AP: Attempting to set AP SSID and password\r\n");
    created = WiFi.AP.create(ssid, password);
    if (!created)
        Serial.printf("ERROR: Failed to set soft AP SSID and Password!\r\n");
    else if (_debug)
        Serial.printf("WiFi AP: SSID: %s%s%s\r\n", ssid,
                     password ? ", Password: " : "", password ? password : "");
    return created;
}

//*********************************************************************
// Attempt to start the soft AP mode
// Inputs:
//    forceAP: Set to true to force AP to start, false will only start
//             soft AP if settings.wifiConfigOverAP is true
// Outputs:
//    Returns true if the soft AP was started successfully and false
//    otherwise
bool wifiStartAp(R4A_WIFI * wifi, bool forceAP)
{
    return enable(wifiEspNowRunning, forceAP | settings.wifiConfigOverAP, wifiStationRunning, wifi->__FILE__, __LINE__);
}

//*********************************************************************
// Connect the station to a remote AP
// Return true if the connection was successful and false upon failure.
bool wifiStationConnectAP(R4A_WIFI * wifi)
{
    bool connected;

    do
    {
        // Connect to the remote AP
        if (_debug)
            Serial.printf("WiFi connecting to %s on channel %d with %s authorization\r\n",
                         wifi->_staRemoteApSsid,
                         wifiChannel,
                         (_staAuthType < WIFI_AUTH_MAX) ? wifiAuthorizationName[_staAuthType] : "Unknown");
        connected = (WiFi.STA.connect(_staRemoteApSsid, wifi->_staRemoteApPassword, wifiChannel));
        if (!connected)
        {
            if (_debug)
                Serial.printf("WIFI failed to connect to SSID %s with password %s\r\n",
                             wifi->_staRemoteApSsid, _staRemoteApPassword);
            break;
        }
        if (_debug)
            Serial.printf("WiFi station connected to %s on channel %d with %s authorization\r\n",
                         wifi->_staRemoteApSsid,
                         wifiChannel,
                         (_staAuthType < WIFI_AUTH_MAX) ? wifiAuthorizationName[_staAuthType] : "Unknown");

        // Don't delay the next WiFi start request
        wifiResetTimeout();
    } while (0);
    return connected;
}

//*********************************************************************
// Disconnect the station from an AP
// Outputs:
//   Returns true if successful and false upon failure
bool wifiStationDisconnect(R4A_WIFI * wifi)
{
    bool disconnected;

    do
    {
        // Determine if station is connected to a remote AP
        disconnected = !_staConnected;
        if (disconnected)
        {
            if (_debug)
                Serial.printf("Station already disconnected from remote AP\r\n");
            break;
        }

        // Disconnect from the remote AP
        if (_debug)
            Serial.printf("WiFI disconnecting station from remote AP\r\n");
        disconnected = WiFi.STA.disconnect();
        if (!disconnected)
        {
            Serial.printf("ERROR: Failed to disconnect WiFi from the remote AP!\r\n");
            break;
        }
        if (_debug)
            Serial.printf("WiFi disconnected from the remote AP\r\n");
    } while (0);
    return disconnected;
}

//*********************************************************************
// Handle the WiFi station events
void wifiStationEventHandler(R4A_WIFI * wifi, arduino_event_id_t event, arduino_event_info_t info)
{
    WIFI_CHANNEL_t channel;
    IPAddress ipAddress;
    char ssid[sizeof(info.wifi_sta_connected.ssid) + 1];
    bool success;
    int type;

    //------------------------------
    // WiFi Status Values:
    //     WL_CONNECTED: assigned when connected to a WiFi network
    //     WL_CONNECTION_LOST: assigned when the connection is lost
    //     WL_CONNECT_FAILED: assigned when the connection fails for all the attempts
    //     WL_DISCONNECTED: assigned when disconnected from a network
    //     WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and
    //                     remains active until the number of attempts expires (resulting in
    //                     WL_CONNECT_FAILED) or a connection is established (resulting in
    //                     WL_CONNECTED)
    //     WL_NO_SHIELD: assigned when no WiFi shield is present
    //     WL_NO_SSID_AVAIL: assigned when no SSID are available
    //     WL_SCAN_COMPLETED: assigned when the scan networks is completed
    //
    // WiFi Station State Machine
    //
    //   .--------+<----------+<-----------+<-------------+<----------+<----------+<------------.
    //   v        |           |            |              |           |           |             |
    // STOP --> READY --> STA_START --> SCAN_DONE --> CONNECTED --> GOT_IP --> LOST_IP --> DISCONNECTED
    //            ^                                       ^           ^           |             |
    //            |                                       |           '-----------'             |
    // OFF -------'                                       '-------------------------------------'
    //
    // Notify the upper layers that WiFi is no longer available
    //------------------------------

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_START:
        WiFi.STA.macAddress((uint8_t *)_staMacAddress);
        if (_debug)
            Serial.printf("WiFi Event: Station start: MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                         wifi->_staMacAddress[0], _staMacAddress[1], _staMacAddress[2],
                         wifi->_staMacAddress[3], _staMacAddress[4], _staMacAddress[5]);

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_STA_STOP:
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        // Start the reconnection timer
        if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
        {
            networkInterfaceEventInternetLost(NETWORK_WIFI_STATION);
            wifiReconnectRequest = true;
        }

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_SCAN_DONE:
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        // The WiFi station is no longer connected to the remote AP
        if (_debug && wifi->_staConnected)
            Serial.printf("WiFi: Station disconnected from %s\r\n",
                         wifi->_staRemoteApSsid);
        wifi->_staConnected = false;

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        if (event == ARDUINO_EVENT_WIFI_STA_LOST_IP)
        {
            networkInterfaceEventInternetLost(NETWORK_WIFI_STATION);
            wifiReconnectRequest = true;
        }

        // Mark the WiFi station offline
        if (_started & WIFI_STA_ONLINE)
            Serial.printf("WiFi: Station offline!\r\n");
        wifi->_started = _started & ~WIFI_STA_ONLINE;

        // Notify user of loss of IP address
        if (_debug && wifi->_staHasIp)
            Serial.printf("WiFi station lost IPv%c address %s\r\n",
                         wifi->_staIpType, _staIpAddress.toString().c_str());
        wifi->_staHasIp = false;
        wifi->_staIpAddress = IPAddress((uint32_t)0);
        wifi->_staIpType = 0;
        break;

    //------------------------------
    // Bring the WiFi station back online
    //------------------------------

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        wifi->_staConnected = true;
        if (_debug)
        {
            memcpy(ssid, info.wifi_sta_connected.ssid, info.wifi_sta_connected.ssid_len);
            ssid[info.wifi_sta_connected.ssid_len] = 0;
            Serial.printf("WiFi: STA connected to %s\r\n", ssid);
        }
        break;

        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        wifi->_staIpAddress = WiFi.STA.localIP();
        type = (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) ? '4' : '6';

        // Display the IP address
        if (_debug)
            Serial.printf("WiFi: Got IPv%c address %s\r\n",
                         type, wifi->_staIpAddress.toString().c_str());
        networkInterfaceEventInternetAvailable(NETWORK_WIFI_STATION);
        break;
    }   // End of switch
}

//*********************************************************************
// Set the station's host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool wifiStationHostName(R4A_WIFI * wifi, const char * hostName)
{
    bool nameSet;

    do
    {
        // Verify that a host name was specified
        nameSet =  (hostName != nullptr) && (strlen(hostName) != 0);
        if (!nameSet)
        {
            Serial.printf("ERROR: No host name specified!\r\n");
            break;
        }

        // Set the host name
        if (_debug)
            Serial.printf("WiFI setting station host name\r\n");
        nameSet = WiFi.STA.setHostname(hostName);
        if (!nameSet)
        {
            Serial.printf("ERROR: Failed to set the Wifi station host name!\r\n");
            break;
        }
        if (_debug)
            Serial.printf("WiFi station hostname: %s\r\n", hostName);
    } while (0);
    return nameSet;
}

//*********************************************************************
// Get the WiFi station IP address
// Returns the IP address of the WiFi station
IPAddress wifiStationIpAddress(R4A_WIFI * wifi)
{
    if (stationOnline())
        return wifi->_staIpAddress;
    return IPAddress((uint32_t)0);
}

//*********************************************************************
// Get the station status
bool wifiStationOnline(R4A_WIFI * wifi)
{
    return (_started & WIFI_STA_ONLINE) ? true : false;
}

//*********************************************************************
// Scan the WiFi network for remote APs
// Inputs:
//   channel: Channel number for the scan, zero (0) scan all channels
// Outputs:
//   Returns the number of access points
int16_t wifiStationScanForAPs(R4A_WIFI * wifi, WIFI_CHANNEL_t channel)
{
    int16_t apCount;
    int16_t status;

    do
    {
        // Determine if the WiFi scan is already running
        if (_scanRunning)
        {
            if (_debug)
                Serial.printf("WiFi scan already running");
            break;
        }

        // Determine if scanning a single channel or all channels
        if (channel)
            Serial.printf("WiFi scanning on channel %d\r\n", channel);
        else
            Serial.printf("WiFi scanning for access points\r\n");

        // Start the WiFi scan
        apCount = WiFi.scanNetworks(false,      // async
                                    false,      // show_hidden
                                    false,      // passive
                                    300,        // max_ms_per_chan
                                    channel,    // channel number
                                    nullptr,    // ssid *
                                    nullptr);   // bssid *
        if (_debug && wifi->_verbose)
            Serial.printf("apCount: %d\r\n", apCount);
        if (apCount < 0)
        {
            Serial.printf("ERROR: WiFi scan failed, status: %d!\r\n", apCount);
            break;
        }
        wifi->_apCount = apCount;
        if (_debug)
            Serial.printf("WiFi scan complete, found %d remote APs\r\n", wifi->_apCount);
    } while (0);
    return apCount;
}

//*********************************************************************
// Select the AP and channel to use for WiFi station
// Inputs:
//   apCount: Number to APs detected by the WiFi scan
//   list: Determine if the APs should be listed
// Outputs:
//   Returns the channel number of the AP
WIFI_CHANNEL_t wifiStationSelectAP(R4A_WIFI * wifi, uint8_t apCount, bool list)
{
    int ap;
    WIFI_CHANNEL_t apChannel;
    bool apFound;
    int authIndex;
    WIFI_CHANNEL_t channel;
    const char * ssid;
    String ssidString;
    int type;

    // Verify that an AP was found
    if (apCount == 0)
        return 0;

    // Print the header
    //                                    1                 1         2         3
    //             1234   1234   123456789012345   12345678901234567890123456789012
    if (_debug || list)
    {
        Serial.printf(" dBm   Chan   Authorization     SSID\r\n");
        Serial.printf("----   ----   ---------------   --------------------------------\r\n");
    }

    // Walk the list of APs that were found during the scan
    apFound = false;
    apChannel = 0;
    for (ap = 0; ap < apCount; ap++)
    {
        // The APs are listed in decending signal strength order
        // Check for a requested AP
        ssidString = WiFi.SSID(ap);
        ssid = ssidString.c_str();
        type = WiFi.encryptionType(ap);
        channel = WiFi.channel(ap);
        if (!apFound)
        {
            for (authIndex = 0; authIndex < MAX_WIFI_NETWORKS; authIndex++)
            {
                // Determine if this authorization matches the AP's SSID
                if (strlen(*r4aWifiSsidPassword[authIndex].ssid)
                    && (strcmp(ssid, *r4aWifiSsidPassword[authIndex].ssid) == 0)
                    && ((type == WIFI_AUTH_OPEN)
                        || (strlen(*r4aWifiSsidPassword[authIndex].password))))
                {
                    if (_debug)
                        Serial.printf("WiFi: Found remote AP: %s\r\n", ssid);

                    // A match was found, save it and stop looking
                    wifi->_staRemoteApSsid = *r4aWifiSsidPassword[authIndex].ssid;
                    wifi->_staRemoteApPassword = *r4aWifiSsidPassword[authIndex].password;
                    apChannel = channel;
                    wifi->_staAuthType = type;
                    apFound = true;
                    break;
                }
            }

            // Check for done
            if (apFound && (!(_debug | list)))
                break;
        }

        // Display the list of APs
        if (_debug || list)
            Serial.printf("%4d   %4d   %s   %s\r\n",
                         WiFi.RSSI(ap),
                         channel,
                         (type < WIFI_AUTH_MAX) ? wifiAuthorizationName[type] : "Unknown",
                         ssid);
    }

    // Return the channel number
    return apChannel;
}

//*********************************************************************
// Get the SSID of the remote AP
const char * wifiStationSsid(R4A_WIFI * wifi)
{
    if (stationOnline())
        return wifi->_staRemoteApSsid;
    else
        return "";
}

//*********************************************************************
// Stop and start WiFi components
// Inputs:
//   stopping: WiFi components that need to be stopped
//   starting: WiFi components that neet to be started
// Outputs:
//   Returns true if the modes were successfully configured
bool wifiStopStart(R4A_WIFI * wifi, WIFI_ACTION_t stopping, WIFI_ACTION_t starting)
{
    const WIFI_ACTION_t allOnline = WIFI_AP_ONLINE | WIFI_EN_ESP_NOW_ONLINE | WIFI_STA_ONLINE;
    int authIndex;
    WIFI_CHANNEL_t channel;
    bool defaultChannel;
    WIFI_ACTION_t delta;
    WIFI_ACTION_t expected;
    WIFI_ACTION_t mask;
    WIFI_ACTION_t notStarted;
    uint8_t primaryChannel;
    WIFI_ACTION_t restarting;
    bool restartWiFiStation;
    wifi_second_chan_t secondaryChannel;
    WIFI_ACTION_t startingNow;
    esp_err_t status;
    WIFI_ACTION_t stillRunning;

    // Determine the next actions
    notStarted = 0;
    restartWiFiStation = false;

    // Display the parameters
    if (_debug && wifi->_verbose)
    {
        Serial.printf(R4A_WIFI * wifi, "WiFi: wifiStopStart called\r\n");
        Serial.printf("stopping: 0x%08x\r\n", stopping);
        Serial.printf("starting: 0x%08x\r\n", starting);
        reportHeapNow(true);
    }

    //****************************************
    // Select the channel
    //
    // The priority order for the channel is:
    //      1. Active channel (not using default channel)
    //      2. wifi->_stationChannel
    //      3. Remote AP channel determined by scan
    //      4. wifi->_espNowChannel
    //      5. wifi->_apChannel
    //      6. Channel 1
    //****************************************

    // Determine if there is an active channel
    defaultChannel = wifi->_usingDefaultChannel;
    wifi->_usingDefaultChannel = false;
    if ((allOnline & wifi->_started & ~stopping) && wifiChannel && !defaultChannel)
    {
        // Continue to use the active channel
        channel = wifiChannel;
        if (_debug && wifi->_verbose)
            Serial.printf("channel: %d, active channel\r\n", channel);
    }

    // Use the station channel if specified
    else if (_stationChannel && (starting & WIFI_STA_ONLINE))
    {
        channel = wifi->_stationChannel;
        if (_debug && wifi->_verbose)
            Serial.printf("channel: %d, WiFi station channel\r\n", channel);
    }

    // Determine if a scan for remote APs is needed
    else if (starting & WIFI_STA_START_SCAN)
    {
        channel = 0;
        if (_debug && wifi->_verbose)
            Serial.printf("channel: Determine by remote AP scan\r\n");

        // Restart ESP-NOW if necessary
        if (wifiEspNowRunning)
            stopping |= WIFI_START_ESP_NOW;

        // Restart soft AP if necessary
        if (wifiSoftApRunning)
            stopping |= WIFI_START_SOFT_AP;
    }

    // Determine if the ESP-NOW channel was specified
    else if (_espNowChannel & ((starting | wifi->_started) & WIFI_EN_ESP_NOW_ONLINE))
    {
        channel = wifi->_espNowChannel;
        if (_debug && wifi->_verbose)
            Serial.printf("channel: %d, ESP-NOW channel\r\n", channel);
    }

    // Determine if the AP channel was specified
    else if (_apChannel && ((starting | wifi->_started) & WIFI_AP_ONLINE))
    {
        channel = wifi->_apChannel;
        if (_debug && wifi->_verbose)
            Serial.printf("channel: %d, soft AP channel\r\n", channel);
    }

    // No channel specified and scan not being done, use the default channel
    else
    {
        channel = WIFI_DEFAULT_CHANNEL;
        wifi->_usingDefaultChannel = true;
        if (_debug && wifi->_verbose)
            Serial.printf("channel: %d, default channel\r\n", channel);
    }

    //****************************************
    // Determine if DNS needs to start
    //****************************************

    if (starting & WIFI_AP_START_DNS_SERVER)
    {
        // Only start the DNS server when the captive portal is enabled
        if (!settings.enableCaptivePortal)
            starting &= ~WIFI_AP_START_DNS_SERVER;
    }

    //****************************************
    // Perform some optimizations
    //****************************************

    // Only stop the started components
    stopping &= wifi->_started;

    // Determine the components that are being started
    expected = starting & allOnline;

    // Determine which components are being restarted
    restarting = wifi->_started & stopping & starting;
    if (_debug && wifi->_verbose)
    {
        Serial.printf("0x%08x: wifi->_started\r\n", _started);
        Serial.printf("0x%08x: stopping\r\n", stopping);
        Serial.printf("0x%08x: starting\r\n", starting);
        Serial.printf("0x%08x: restarting\r\n", restarting);
        Serial.printf("0x%08x: expected\r\n", expected);
    }

    // Don't start components that are already running and are not being
    // stopped
    starting &= ~(_started & ~stopping);

    // Display the starting and stopping
    if (_debug)
    {
        bool startEspNow = starting & WIFI_EN_ESP_NOW_ONLINE;
        bool startSoftAP = starting & WIFI_AP_ONLINE;
        bool startStation = starting & WIFI_STA_ONLINE;
        bool start = startEspNow || startSoftAP || startStation;
        bool stopEspNow = stopping & WIFI_EN_ESP_NOW_ONLINE;
        bool stopSoftAP = stopping & WIFI_AP_ONLINE;
        bool stopStation = stopping & WIFI_STA_ONLINE;
        bool stop = stopEspNow || stopSoftAP || stopStation;

        if (start || stop)
            Serial.printf("WiFi:%s%s%s%s%s%s%s%s%s%s%s%s%s%s\r\n",
                         start ? " Starting (" : "",
                         startEspNow ? "ESP-NOW" : "",
                         (startEspNow && (startSoftAP || startStation)) ? ", " : "",
                         startSoftAP ? "Soft AP" : "",
                         (startSoftAP && startStation) ? ", " : "",
                         startStation ? "Station" : "",
                         start ? ")" : "",
                         stop ? " Stopping (" : "",
                         stopEspNow ? "ESP-NOW" : "",
                         (stopEspNow && (stopSoftAP || stopStation)) ? ", " : "",
                         stopSoftAP ? "Soft AP" : "",
                         (stopSoftAP && stopStation) ? ", " : "",
                         stopStation ? "Station" : "",
                         stop ? ")" : "");
    }

    //****************************************
    // Determine which components should end up online
    //****************************************

    // Stop the components
    startingNow = starting;
    do
    {
        //****************************************
        // Display the items being stopped
        //****************************************

        if (_debug && wifi->_verbose && stopping)
            displayComponents("Stopping", stopping);

        //****************************************
        // Stop the ESP-NOW components
        //****************************************

        // Mark the ESP-NOW offline
        if (stopping & WIFI_EN_ESP_NOW_ONLINE)
        {
            if (_debug)
                Serial.printf("WiFi: ESP-NOW offline!\r\n");
            wifi->_started = _started & ~WIFI_EN_ESP_NOW_ONLINE;
        }

        // Stop the ESP-NOW layer
        if (stopping & WIFI_EN_START_ESP_NOW)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("WiFi: Stopping ESP-NOW\r\n");
            if (!espNowStop())
            {
                Serial.printf("ERROR: Failed to stop ESP-NOW!\r\n");
                break;
            }
            wifi->_started = _started & ~WIFI_EN_START_ESP_NOW;
            if (_debug && wifi->_verbose)
                Serial.printf("WiFi: ESP-NOW stopped\r\n");
        }

        // Stop the promiscuous RX callback handler
        if (stopping & WIFI_EN_PROMISCUOUS_RX_CALLBACK)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("Calling esp_wifi_set_promiscuous_rx_cb\r\n");
            status = esp_wifi_set_promiscuous_rx_cb(nullptr);
            if (status != ESP_OK)
            {
                Serial.printf("ERROR: Failed to stop WiFi promiscuous RX callback\r\n");
                break;
            }
            if (_debug)
                Serial.printf("WiFi: Stopped WiFi promiscuous RX callback\r\n");
            wifi->_started = _started & ~WIFI_EN_PROMISCUOUS_RX_CALLBACK;
        }

        // Stop promiscuous mode
        if (stopping & WIFI_EN_SET_PROMISCUOUS_MODE)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("Calling esp_wifi_set_promiscuous\r\n");
            status = esp_wifi_set_promiscuous(false);
            if (status != ESP_OK)
            {
                Serial.printf("ERROR: Failed to stop WiFi promiscuous mode, status: %d\r\n", status);
                break;
            }
            if (_debug && wifi->_verbose)
                Serial.printf("WiFi: Promiscuous mode stopped\r\n");
            wifi->_started = _started & ~WIFI_EN_SET_PROMISCUOUS_MODE;
        }

        // Handle WiFi set channel
        if (stopping & WIFI_EN_SET_CHANNEL)
            wifi->_started = _started & ~WIFI_EN_SET_CHANNEL;

        // Stop the long range radio protocols
        if (stopping & WIFI_EN_SET_PROTOCOLS)
        {
            if (!setWiFiProtocols(WIFI_IF_STA, true, false))
                break;
            wifi->_started = _started & ~WIFI_EN_SET_PROTOCOLS;
        }

        //****************************************
        // Stop the WiFi station components
        //****************************************

        // Mark the WiFi station offline
        if (stopping & WIFI_STA_ONLINE)
        {
            if (_started & WIFI_STA_ONLINE)
                Serial.printf("WiFi: Station offline!\r\n");
            wifi->_started = _started & ~WIFI_STA_ONLINE;
        }

        // Disconnect from the remote AP
        if (stopping & WIFI_STA_CONNECT_TO_REMOTE_AP)
        {
            if (!stationDisconnect())
                break;
            wifi->_started = _started & ~WIFI_STA_CONNECT_TO_REMOTE_AP;
        }

        // Handle auto reconnect
        if (stopping & WIFI_STA_DISABLE_AUTO_RECONNECT)
            wifi->_started = _started & ~WIFI_STA_DISABLE_AUTO_RECONNECT;

        // Handle WiFi station host name
        if (stopping & WIFI_STA_SET_HOST_NAME)
            wifi->_started = _started & ~WIFI_STA_SET_HOST_NAME;

        // Handle WiFi select channel
        if (stopping & WIFI_SELECT_CHANNEL)
            wifi->_started = _started & ~(stopping & WIFI_SELECT_CHANNEL);

        // Handle WiFi station select remote AP
        if (stopping & WIFI_STA_SELECT_REMOTE_AP)
            wifi->_started = _started & ~WIFI_STA_SELECT_REMOTE_AP;

        // Handle WiFi station scan
        if (stopping & WIFI_STA_START_SCAN)
            wifi->_started = _started & ~WIFI_STA_START_SCAN;

        // Stop the WiFi station radio protocols
        if (stopping & WIFI_STA_SET_PROTOCOLS)
            wifi->_started = _started & ~WIFI_STA_SET_PROTOCOLS;

        // Stop station mode
        if (stopping & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE))
        {
            // Determine which bits to clear
            mask = ~(stopping & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE));

            // Stop WiFi station if users are gone
            if (!(_started & mask & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE)))
            {
                if (!setWiFiMode(WIFI_MODE_STA, WIFI_MODE_STA))
                    break;
            }

            // Remove this WiFi station user
            wifi->_started = _started & mask;
        }

        //****************************************
        // Stop the soft AP components
        //****************************************

        // Stop soft AP mode
        // Mark the soft AP offline
        if (stopping & WIFI_AP_ONLINE)
        {
            if (softApOnline())
                Serial.printf("WiFi: Soft AP offline!\r\n");
            wifi->_started = _started & ~WIFI_AP_ONLINE;
        }

        // Stop the DNS server
        if (stopping & wifi->_started & WIFI_AP_START_DNS_SERVER)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("Calling dnsServer.stop for soft AP\r\n");
            dnsServer.stop();
            wifi->_started = _started & ~WIFI_AP_START_DNS_SERVER;
        }

        // Handle the soft AP host name
        if (stopping & WIFI_AP_SET_HOST_NAME)
            wifi->_started = _started & ~WIFI_AP_SET_HOST_NAME;

        // Stop soft AP mode
        if (stopping & WIFI_AP_SET_MODE)
        {
            if (!setWiFiMode(WIFI_MODE_AP, WIFI_MODE_AP))
                break;
            wifi->_started = _started & ~WIFI_AP_SET_MODE;
        }

        // Disable the radio protocols for soft AP
        if (stopping & WIFI_AP_SET_PROTOCOLS)
            wifi->_started = _started & ~WIFI_AP_SET_PROTOCOLS;

        // Stop using the soft AP IP address
        if (stopping & WIFI_AP_SET_IP_ADDR)
            wifi->_started = _started & ~WIFI_AP_SET_IP_ADDR;

        // Stop use of SSID and password
        if (stopping & WIFI_AP_SET_SSID_PASSWORD)
            wifi->_started = _started & ~WIFI_AP_SET_SSID_PASSWORD;

        stillRunning = wifi->_started;

        //****************************************
        // Channel reset
        //****************************************

        // Reset the channel if all components are stopped
        if ((softApOnline() == false) && (stationOnline() == false))
        {
            wifiChannel = 0;
            wifi->_usingDefaultChannel = true;
        }

        //****************************************
        // Delay to allow mDNS to shutdown and restart properly
        //****************************************

        delay(100);

        //****************************************
        // Display the items already started and being started
        //****************************************

        if (_debug && wifi->_verbose && _started)
            displayComponents("Started", wifi->_started);

        if (_debug && wifi->_verbose && startingNow)
            displayComponents("Starting", startingNow);

        //****************************************
        // Start the radio operations
        //****************************************

        // Start the soft AP mode
        if (starting & WIFI_AP_SET_MODE)
        {
            if (!setWiFiMode(WIFI_MODE_AP, 0))
                break;
            wifi->_started = _started | WIFI_AP_SET_MODE;
        }

        // Start the station mode
        if (starting & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE))
        {
            if (!setWiFiMode(WIFI_MODE_STA, 0))
                break;
            wifi->_started = _started | (starting & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE));
        }

        // Start the soft AP protocols
        if (starting & WIFI_AP_SET_PROTOCOLS)
        {
            if (!setWiFiProtocols(WIFI_IF_AP, true, false))
                break;
            wifi->_started = _started | WIFI_AP_SET_PROTOCOLS;
        }

        // Start the WiFi station radio protocols
        if (starting & (WIFI_EN_SET_PROTOCOLS | WIFI_STA_SET_PROTOCOLS))
        {
            bool lrEnable = (starting & WIFI_EN_SET_PROTOCOLS) ? true : false;
            if (!setWiFiProtocols(WIFI_IF_STA, true, lrEnable))
                break;
            wifi->_started = _started | (starting & (WIFI_EN_SET_PROTOCOLS | WIFI_STA_SET_PROTOCOLS));
        }

        // Start the WiFi scan for remote APs
        if (starting & WIFI_STA_START_SCAN)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("channel: %d\r\n", channel);
            wifi->_started = _started | WIFI_STA_START_SCAN;

            displayWiFiConnect();

            // Determine if WiFi scan failed, stop WiFi station startup
            if (wifi.stationScanForAPs(channel) < 0)
            {
                starting &= ~WIFI_STA_FAILED_SCAN;
                notStarted |= WIFI_STA_FAILED_SCAN;
            }
        }

        // Select an AP from the list
        if (starting & WIFI_STA_SELECT_REMOTE_AP)
        {
            channel = stationSelectAP(_apCount, false);
            wifi->_started = _started | WIFI_STA_SELECT_REMOTE_AP;
            if (channel == 0)
            {
                if (wifiChannel)
                    Serial.printf("WiFi STA: No matching remote AP found on channel %d!\r\n", wifiChannel);
                else
                    Serial.printf("WiFi STA: No matching remote AP found!\r\n");

                displayNoWiFi(2000);

                // Stop bringing up WiFi station
                starting &= ~WIFI_STA_NO_REMOTE_AP;
                notStarted |= WIFI_STA_FAILED_SCAN;
            }
        }

        // Finish the channel selection
        if (starting & WIFI_SELECT_CHANNEL)
        {
            wifi->_started = _started | starting & WIFI_SELECT_CHANNEL;
            if (channel & (starting & WIFI_STA_START_SCAN))
            {
                if (_debug && wifi->_verbose)
                    Serial.printf("Channel: %d, determined by remote AP scan\r\n",
                                 channel);
            }

            // Use the default channel if necessary
            if (!channel)
                channel = WIFI_DEFAULT_CHANNEL;
            wifiChannel = channel;

            // Display the selected channel
            if (_debug)
                Serial.printf("Channel: %d selected\r\n", wifiChannel);
        }

        //****************************************
        // Start the soft AP components
        //****************************************

        // Set the soft AP SSID and password
        if (starting & WIFI_AP_SET_SSID_PASSWORD)
        {
            if (!softApSetSsidPassword(wifiSoftApSsid, wifiSoftApPassword))
                break;
            wifi->_started = _started | WIFI_AP_SET_SSID_PASSWORD;
        }

        // Set the soft AP subnet mask, IP, gateway, DNS, and first DHCP addresses
        if (starting & WIFI_AP_SET_IP_ADDR)
        {
            if (!softApSetIpAddress(_apIpAddress.toString().c_str(),
                                    wifi->_apSubnetMask.toString().c_str(),
                                    wifi->_apGatewayAddress.toString().c_str(),
                                    wifi->_apDnsAddress.toString().c_str(),
                                    wifi->_apFirstDhcpAddress.toString().c_str()))
            {
                break;
            }
            wifi->_started = _started | WIFI_AP_SET_IP_ADDR;
        }

        // Get the soft AP MAC address
        WiFi.AP.macAddress(_apMacAddress);

        // Set the soft AP host name
        if (starting & WIFI_AP_SET_HOST_NAME)
        {
            const char * hostName = &settings.mdnsHostName[0];

            // Display the host name
            if (_debug && wifi->_verbose)
                Serial.printf("Host name: %s\r\n", hostName);

            // Set the host name
            if (!softApSetHostName(hostName))
                break;
            wifi->_started = _started | WIFI_AP_SET_HOST_NAME;
        }

        // Start the DNS server
        if (starting & WIFI_AP_START_DNS_SERVER)
        {
            if (_debug)
                Serial.printf("Starting DNS on soft AP\r\n");
            if (dnsServer.start(53, "*", WiFi.softAPIP()) == false)
            {
                Serial.printf("ERROR: Failed to start DNS Server for soft AP\r\n");
                break;
            }
            if (_debug)
                Serial.printf("DNS Server started for soft AP\r\n");
            wifi->_started = _started | WIFI_AP_START_DNS_SERVER;
        }

        // Mark the soft AP as online
        if (starting & WIFI_AP_ONLINE)
        {
            wifi->_started = _started | WIFI_AP_ONLINE;

            // Display the soft AP status
            Serial.printf("WiFi: Soft AP online, SSID: %s (%s)%s%s\r\n",
                         wifiSoftApSsid,
                         wifi->_apIpAddress.toString().c_str(),
                         wifiSoftApPassword ? ", Password: " : "",
                         wifiSoftApPassword ? wifiSoftApPassword : "");
        }

        //****************************************
        // Start the WiFi station components
        //****************************************

        restartWiFiStation = true;

        // Set the host name
        if (starting & WIFI_STA_SET_HOST_NAME)
        {
            const char * hostName = &settings.mdnsHostName[0];

            // Display the host name
            if (_debug && wifi->_verbose)
                Serial.printf("Host name: %s\r\n", hostName);

            // Set the host name
            if (!stationHostName(hostName))
                break;
            wifi->_started = _started | WIFI_STA_SET_HOST_NAME;
        }

        // Disable auto reconnect
        if (starting & WIFI_STA_DISABLE_AUTO_RECONNECT)
        {
            if (!WiFi.setAutoReconnect(false))
            {
                Serial.printf("ERROR: Failed to disable auto-reconnect!\r\n");
                break;
            }
            wifi->_started = _started | WIFI_STA_DISABLE_AUTO_RECONNECT;
            if (_debug)
                Serial.printf("WiFi auto-reconnect disabled\r\n");
        }

        // Connect to the remote AP
        if (starting & WIFI_STA_CONNECT_TO_REMOTE_AP)
        {
            IPAddress ipAddress;
            uint32_t timer;

            if (!stationConnectAP())
                break;
            wifi->_started = _started | WIFI_STA_CONNECT_TO_REMOTE_AP;

            // Wait for an IP address
            ipAddress = WiFi.STA.localIP();
            timer = millis();
            while (!(uint32_t)ipAddress)
            {
                if ((millis() - timer) >= WIFI_IP_ADDRESS_TIMEOUT_MSEC)
                    break;
                delay(10);
                ipAddress = WiFi.STA.localIP();
            }
            if ((millis() - timer) >= WIFI_IP_ADDRESS_TIMEOUT_MSEC)
            {
                Serial.printf("ERROR: Failed to get WiFi station IP address!\r\n");
                break;
            }

            // Wait for the station MAC address to be set
            while (!_staMacAddress[0])
                delay(1);

            // Save the IP address
            wifi->_staHasIp = true;
            wifi->_staIpType = (_staIpAddress.type() == IPv4) ? '4' : '6';
        }

        // Mark the station online
        if (starting & WIFI_STA_ONLINE)
        {
            restartWiFiStation = false;
            wifi->_started = _started | WIFI_STA_ONLINE;
            Serial.printf("WiFi: Station online (%s: %s)\r\n",
                         wifi->_staRemoteApSsid, _staIpAddress.toString().c_str());
        }

        //****************************************
        // Start the ESP-NOW components
        //****************************************

        // Select the ESP-NOW channel
        if (starting & WIFI_EN_SET_CHANNEL)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("Calling esp_wifi_get_channel\r\n");
            status = esp_wifi_get_channel(&primaryChannel, &secondaryChannel);
            if (status != ESP_OK)
            {
                Serial.printf("ERROR: Failed to get the WiFi channels, status: %d\r\n", status);
                break;
            }
            if (_debug && wifi->_verbose)
            {
                Serial.printf("primaryChannel: %d\r\n", primaryChannel);
                Serial.printf("secondaryChannel: %d (%s)\r\n", secondaryChannel,
                             (secondaryChannel == WIFI_SECOND_CHAN_NONE) ? "None"
                             : ((secondaryChannel == WIFI_SECOND_CHAN_ABOVE) ? "Above"
                             : "Below"));
            }

            // Set the ESP-NOW channel
            if (primaryChannel != wifiChannel)
            {
                if (_debug && wifi->_verbose)
                    Serial.printf("Calling esp_wifi_set_channel\r\n");
                status = esp_wifi_set_channel(primaryChannel, secondaryChannel);
                if (status != ESP_OK)
                {
                    Serial.printf("ERROR: Failed to set WiFi primary channel to %d, status: %d\r\n", primaryChannel, status);
                    break;
                }
                if (_debug)
                    Serial.printf("WiFi: Set channel %d\r\n", primaryChannel);
            }
            wifi->_started = _started | WIFI_EN_SET_CHANNEL;
        }

        // Set promiscuous mode
        if (starting & WIFI_EN_SET_PROMISCUOUS_MODE)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("Calling esp_wifi_set_promiscuous\r\n");
            status = esp_wifi_set_promiscuous(true);
            if (status != ESP_OK)
            {
                Serial.printf("ERROR: Failed to set WiFi promiscuous mode, status: %d\r\n", status);
                break;
            }
            if (_debug && wifi->_verbose)
                Serial.printf("WiFi: Enabled promiscuous mode\r\n");
            wifi->_started = _started | WIFI_EN_SET_PROMISCUOUS_MODE;
        }

        // Set promiscuous receive callback to get RSSI of action frames
        if (starting & WIFI_EN_PROMISCUOUS_RX_CALLBACK)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("Calling esp_wifi_set_promiscuous_rx_cb\r\n");
            status = esp_wifi_set_promiscuous_rx_cb(wifiPromiscuousRxHandler);
            if (status != ESP_OK)
            {
                Serial.printf("ERROR: Failed to set the WiFi promiscuous RX callback, status: %d\r\n", status);
                break;
            }
            if (_debug && wifi->_verbose)
                Serial.printf("WiFi: Promiscuous RX callback established\r\n");
            wifi->_started = _started | WIFI_EN_PROMISCUOUS_RX_CALLBACK;
        }

        // Start ESP-NOW
        if (starting & WIFI_EN_START_ESP_NOW)
        {
            if (_debug && wifi->_verbose)
                Serial.printf("Calling espNowStart\r\n");
            if (!espNowStart())
            {
                Serial.printf("ERROR: Failed to start ESP-NOW\r\n");
                break;
            }
            if (_debug)
                Serial.printf("WiFi: ESP-NOW started\r\n");
            wifi->_started = _started | WIFI_EN_START_ESP_NOW;
        }

        // Mark ESP-NOW online
        if (starting & WIFI_EN_ESP_NOW_ONLINE)
        {
            // Wait for the station MAC address to be set
            while (!_staMacAddress[0])
                delay(1);

            // Display the ESP-NOW MAC address
            wifi->_started = _started | WIFI_EN_ESP_NOW_ONLINE;
            Serial.printf("WiFi: ESP-NOW online (%02x:%02x:%02x:%02x:%02x:%02x, channel: %d)\r\n",
                         wifi->_staMacAddress[0], _staMacAddress[1], _staMacAddress[2],
                         wifi->_staMacAddress[3], _staMacAddress[4], _staMacAddress[5],
                         wifiChannel);
        }
    } while (0);

    //****************************************
    // Display the items that were not stopped
    //****************************************
    if (_debug && wifi->_verbose)
    {
        Serial.printf("0x%08x: stopping\r\n", stopping);
        Serial.printf("0x%08x: stillRunning\r\n", stillRunning);
    }

    // Determine which components were not stopped
    stopping &= stillRunning;
    if (_debug && stopping)
        displayComponents("ERROR: Items NOT stopped", stopping);

    //****************************************
    // Display the items that were not started
    //****************************************

    if (_debug && wifi->_verbose && _verbose)
    {
        Serial.printf("0x%08x: startingNow\r\n", startingNow);
        Serial.printf("0x%08x: wifi->_started\r\n", _started);
    }
    startingNow &= ~_started;
    if (_debug &&  startingNow)
        displayComponents("ERROR: Items NOT started", startingNow);

    //****************************************
    // Display the items that were not started
    //****************************************

    if (_debug && wifi->_verbose)
    {
        Serial.printf("0x%08x: startingNow\r\n", startingNow);
        Serial.printf("0x%08x: wifi->_started\r\n", _started);
    }

    // Clear the items that were not started
    wifi->_started = _started & ~notStarted;

    if (_debug && wifi->_verbose && _started)
        displayComponents("Started items", wifi->_started);

    // Restart WiFi if necessary
    if (restartWiFiStation)
        wifiReconnectRequest = true;

    // Set the online flags
    wifiEspNowOnline = espNowOnline();
    wifiSoftApOnline = softApOnline();
    wifiStationOnline = stationOnline();

    // Return the enable status
    bool enabled = ((_started & allOnline) == expected);
    if (!enabled)
        Serial.printf(R4A_WIFI * wifi, "ERROR: wifiStopStart failed!\r\n");
    if (_debug && wifi->_verbose)
    {
        reportHeapNow(true);
        Serial.printf(R4A_WIFI * wifi, "WiFi: wifiStopStart returning; %s\r\n", enabled ? "true" : "false");
    }
    return enabled;
}

//*********************************************************************
// Test the WiFi modes
void wifiTest(R4A_WIFI * wifi, uint32_t testDurationMsec)
{
    uint32_t currentMsec;
    bool disconnectFirst;
    static uint32_t lastScanMsec = - (1000 * 1000);
    int rand;

    // Delay the mode change until after the WiFi scan completes
    currentMsec = millis();
    if (_scanRunning)
        lastScanMsec = currentMsec;

    // Check if it time for a mode change
    else if ((currentMsec - lastScanMsec) < testDurationMsec)
        return;

    // Perform the test
    lastScanMsec = currentMsec;

    // Get a random number
    rand = random() & 0x1f;
    disconnectFirst = (rand & 0x10) ? true : false;

    // Determine the next actions
    switch (rand)
    {
    default:
        lastScanMsec = currentMsec - (1000 * 1000);
        break;

    case 0:
        Serial.printf("--------------------  %d: All Stop  --------------------\r\n", rand);
        enable(false, false, false, wifi->__FILE__, __LINE__);
        break;

    case 1:
        Serial.printf("--------------------  %d: STA Start  -------------------\r\n", rand);
        enable(false, false, true, wifi->__FILE__, __LINE__);
        break;

    case 2:
        Serial.printf("--------------------  %d: STA Disconnect  --------------\r\n", rand);
        wifi.stationDisconnect();
        break;

    case 4:
        Serial.printf("--------------------  %d: Soft AP Start  -------------------\r\n", rand);
        enable(false, true, false, wifi->__FILE__, __LINE__);
        break;

    case 5:
        Serial.printf("--------------------  %d: Soft AP & STA Start  --------------------\r\n", rand);
        enable(false, true, true, wifi->__FILE__, __LINE__);
        break;

    case 6:
        Serial.printf("--------------------  %d: Soft AP Start, STA Disconnect  -------------------\r\n", rand);
        if (disconnectFirst)
            wifi.stationDisconnect();
        enable(false, true, false, wifi->__FILE__, __LINE__);
        if (!disconnectFirst)
            wifi.stationDisconnect();
        break;

    case 8:
        Serial.printf("--------------------  %d: ESP-NOW Start  --------------------\r\n", rand);
        enable(true, false, false, wifi->__FILE__, __LINE__);
        break;

    case 9:
        Serial.printf("--------------------  %d: ESP-NOW & STA Start  -------------------\r\n", rand);
        enable(true, false, true, wifi->__FILE__, __LINE__);
        break;

    case 0xa:
        Serial.printf("--------------------  %d: ESP-NOW Start, STA Disconnect  --------------\r\n", rand);
        if (disconnectFirst)
            wifi.stationDisconnect();
        enable(true, false, false, wifi->__FILE__, __LINE__);
        if (!disconnectFirst)
            wifi.stationDisconnect();
        break;

    case 0xc:
        Serial.printf("--------------------  %d: ESP-NOW & Soft AP Start  -------------------\r\n", rand);
        enable(true, true, false, wifi->__FILE__, __LINE__);
        break;

    case 0xd:
        Serial.printf("--------------------  %d: ESP-NOW, Soft AP & STA Start  --------------------\r\n", rand);
        enable(true, true, true, wifi->__FILE__, __LINE__);
        break;

    case 0xe:
        Serial.printf("--------------------  %d: ESP-NOW & Soft AP Start, STA Disconnect  -------------------\r\n", rand);
        if (disconnectFirst)
            wifi.stationDisconnect();
        enable(true, true, false, wifi->__FILE__, __LINE__);
        if (!disconnectFirst)
            wifi.stationDisconnect();
        break;
    }
}

//*********************************************************************
// Enable or disable verbose debug output
// Inputs:
//   enable: Set true to enable verbose debug output
// Outputs:
//   Return the previous enable value
bool wifiVerbose(R4A_WIFI * wifi, bool enable)
{
    bool oldVerbose;

    oldVerbose = wifi->_verbose;
    wifi->_verbose = enable;
    return oldVerbose;
}

//*********************************************************************
// Verify the WiFi tables
void wifiVerifyTables(R4A_WIFI * wifi)
{
    // Verify the authorization name table
    if (WIFI_AUTH_MAX != wifiAuthorizationNameEntries)
    {
        Serial.printf("ERROR: Fix wifiAuthorizationName list to match wifi_auth_mode_t in esp_wifi_types.h!\r\n");
        reportFatalError("Fix wifiAuthorizationName list to match wifi_auth_mode_t in esp_wifi_types.h!");
    }

    // Verify the Arduino event name table
    if (ARDUINO_EVENT_MAX != arduinoEventNameEntries)
    {
        Serial.printf("ERROR: Fix arduinoEventName list to match arduino_event_id_t in NetworkEvents.h!\r\n");
        reportFatalError("Fix arduinoEventName list to match arduino_event_id_t in NetworkEvents.h!");
    }

    // Verify the start name table
    if (WIFI_MAX_START != (1 << wifiStartNamesEntries))
    {
        Serial.printf("ERROR: Fix wifiStartNames list to match list of defines!\r\n");
        reportFatalError("Fix wifiStartNames list to match list of defines!!");
    }
}

#endif  // COMPILE_WIFI
