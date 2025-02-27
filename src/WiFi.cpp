/**********************************************************************
  Wifi.cpp

  Robots-For-All (R4A)
  Handle the WiFi events
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Constants
//****************************************

#define R4A_WIFI_RECONNECTION_DELAY     500 // milliseconds

static const char * r4aEsp32WiFiAuthorizationName[] =
{
    "Open",
    "WEP",
    "WPA_PSK",
    "WPA2_PSK",
    "WPA_WPA2_PSK",
    "Enterprise",
    "WPA3_PSK",
    "WPA2_WPA3_PSK",
    "WAPI_PSK",
    "OWE",
    "WPA3_ENT_192",
    "WPA3_EXT_PSK",
    "WPA3_EXT_PSK_MIXED_MODE",
    "DPP",
};
static const int r4aEsp32WiFiAuthorizationNameEntries =
    sizeof(r4aEsp32WiFiAuthorizationName) / sizeof(r4aEsp32WiFiAuthorizationName[0]);

static R4A_WIFI * r4aWiFi;

//*********************************************************************
// Constructor
// Inputs:
//   display: Print object to display WiFi startup summary
//   debug: Print object to display WiFi debugging messages
R4A_WIFI::R4A_WIFI(Print * display,
                   Print * debug)
    : _apCount{0}, _apFound{false}, _apPassword{""}, _apSSID{""},
      _authType{0}, _debug{debug}, _display{display}, _hostName{nullptr},
      _mdnsAvailable{false}, _stationConnected{false},  _stationHasIp{false},
      _wifiChannel{0}, _wifiScanRunning{false}, _wifiTimer{0}
{
}

//*********************************************************************
// Handle the WiFi event
void r4aEsp32WifiEvent(arduino_event_id_t event, arduino_event_info_t info)
{
    r4aWiFi->eventHandler(event, info);
}

//*********************************************************************
// Setup WiFi
void R4A_WIFI::begin(const char * hostName)
{
    // Verify the tables
    log_v("Calling verifyTables");
    verifyTables();

    // Save the object
    r4aWiFi = this;

    // Update the host name
    _hostName = hostName;

    // Start the WiFi station
    log_v("Calling stationStart");
    stationStart();
}

//*********************************************************************
// Get the active channel
//   Returns the channel number or zero (0) if not connected
uint8_t R4A_WIFI::channelGet()
{
    return _wifiChannel;
}

//*********************************************************************
// Handle the WiFi event
void R4A_WIFI::eventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    R4A_WIFI_CHANNEL_t channel;
    Print * debug;
    Print * display;
    static IPAddress localIP;
    static char localIpType;
    wifi_mode_t mode;

    // Always output display items
    debug = (Print *)_debug;
    display = (Print *)_display;
    if (!display)
        display = debug;

    // Display the event
    log_v("Event: %d", event);
    if (debug)
        debug->printf("event: %d\r\n", event);

    // Notify the upper layers that WiFi is no longer available
    if ((event >= ARDUINO_EVENT_WIFI_SCAN_DONE)
        && (event <= ARDUINO_EVENT_WIFI_STA_LOST_IP))
    {
        switch (event)
        {
        default:
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            break;

        case ARDUINO_EVENT_WIFI_SCAN_DONE:
        case ARDUINO_EVENT_WIFI_STA_START:
        case ARDUINO_EVENT_WIFI_STA_STOP:
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            _mdnsAvailable = false;
            _stationConnected = false;
            _wifiChannel = 0;
            _stationHasIp = false;
            break;

        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            _stationHasIp = false;
            break;
        }
    }

    // Handle the event
    switch (event)
    {
    default:
        break;

    //------------------------------
    // Controller events
    //------------------------------

    case ARDUINO_EVENT_WIFI_OFF:
        if (debug)
            debug->printf("WiFi Event: Off\r\n");
        break;

    case ARDUINO_EVENT_WIFI_READY:
        if (debug)
            debug->printf("WiFi Event: Ready\r\n");
        break;

    //------------------------------
    // WiFi Station State Machine
    //
    //   .--------+<----------+<-----------+<-------------+<----------+<----------+<------------.
    //   v        |           |            |              |           |           |             |
    // STOP --> READY --> STA_START --> SCAN_DONE --> CONNECTED --> GOT_IP --> LOST_IP --> DISCONNECTED
    //            ^                                       ^           ^           |             |
    //            |                                       |           '-----------'             |
    // OFF -------'                                       '-------------------------------------'
    //
    // Handle the WiFi station events
    //------------------------------

    case ARDUINO_EVENT_WIFI_STA_START:
        if (debug)
            debug->printf("WiFi Event: Station start\r\n");
        break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        _stationConnected = true;
        if (debug)
            debug->printf("WiFi Event: Station connected\r\n");
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        _stationHasIp = true;

        // Display the IP address
        if (display)
        {
            localIP = WiFi.STA.localIP();
            localIpType = (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) ? '4' : '6';
            display->printf("WiFi Event: Got IPv%c address %s\r\n",
                            localIpType, localIP.toString().c_str());
        }

        // Start mDNS
        mDNSStart();
        break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        if (display)
            display->printf("WiFi Event: Lost IPv%c address %s\r\n",
                            localIpType, localIP.toString().c_str());
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    // Argument structure for WIFI_EVENT_STA_DISCONNECTED event
    //  typedef struct {
    //      uint8_t ssid[32];         // SSID of disconnected AP
    //      uint8_t ssid_len;         // SSID length of disconnected AP
    //      uint8_t bssid[6];         // BSSID of disconnected AP
    //      uint8_t reason;           // reason of disconnection
    //      int8_t  rssi;             // rssi of disconnection
    //  } wifi_event_sta_disconnected_t;
        if (display)
            display->printf("WiFi Event: Station disconnected from %s\r\n",
                            info.wifi_sta_disconnected.ssid);

        // Start the reconnection timer
        _wifiTimer = millis();
        break;

    case ARDUINO_EVENT_WIFI_STA_STOP:
        if (debug)
            debug->printf("WiFi Event: Station stop\r\n");

        // Stop the reconnection timer
        _wifiTimer = 0;
        break;

    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        if (debug)
            debug->printf("WiFi Event: Station authorization change\r\n");
        break;

    //----------------------------------------
    // Scan events
    //----------------------------------------

    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        // Argument structure for WIFI_EVENT_SCAN_DONE event
        //  typedef struct
        //  {
        //      uint32_t status;    // status of scanning APs: 0 — success, 1 - failure
        //      uint8_t  number;    // number of scan results
        //      uint8_t  scan_id;   // scan sequence number, used for block scan
        //  } wifi_event_sta_scan_done_t;
        _wifiScanRunning = false;
        do {
            if (info.wifi_scan_done.status != 0)
            {
                if (debug)
                    debug->printf("WiFi Event: Scan done - failed!\r\n");
                break;
            }
            if (debug)
                debug->printf("WiFi Event: Scan done\r\n");

            // Select an AP from the list
            _apCount = info.wifi_scan_done.number;
            log_v("Calling stationSelectAP");
            channel = stationSelectAP(true, nullptr);
            if (channel == 0)
            {
                if (debug)
                    debug->printf("No compatible remote AP found!\r\n");
                break;
            }
            _wifiChannel = channel;

            // Start WiFi station mode
            mode = WiFi.getMode();
            if (WiFi.mode((wifi_mode_t)(mode | WIFI_MODE_STA)) == false)
            {
                Serial.printf("ERROR: Failed to enable WiFi station mode!\r\n");
                break;
            }
            if (debug)
                debug->printf("WiFi station mode enabled\r\n");

            // Set the host name
            if (!stationHostName(_hostName))
                break;

            // Disable auto reconnect
            if (!WiFi.setAutoReconnect(false))
            {
                Serial.printf("ERROR: Failed to disable auto-reconnect!\r\n");
                break;
            }
            if (debug)
                debug->printf("WiFi auto-reconnect disabled\r\n");

            // Connect to the remote AP
            stationConnectAP();
        } while (0);
        break;

    //------------------------------
    // WiFi soft AP (access point) events
    //------------------------------

    case ARDUINO_EVENT_WIFI_AP_START:
        if (debug)
            debug->printf("WiFi Event: AP start\r\n");
        break;

    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        if (debug)
            debug->printf("WiFi Event: AP station connected\r\n");
        break;

    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        if (debug)
            debug->printf("WiFi Event: AP got IP6 address\r\n");
        break;

    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        if (debug)
            debug->printf("WiFi Event: AP probe request received\r\n");
        break;

    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        if (debug)
            debug->printf("WiFi Event: AP station IP assigned\r\n");
        break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        if (debug)
            debug->printf("WiFi Event: AP station disconnected\r\n");
        break;

    case ARDUINO_EVENT_WIFI_AP_STOP:
        if (debug)
            debug->printf("WiFi Event: AP stop\r\n");
        break;

    case ARDUINO_EVENT_WIFI_FTM_REPORT:
        if (debug)
            debug->printf("WiFi Event: FTM report\r\n");
        break;
    }
}

//*********************************************************************
// Start the WiFi event handler
void R4A_WIFI::eventHandlerStart()
{
    Print * debug;

    debug = (Print *)_debug;
    if (debug)
        debug->printf("Starting the WiFi event handler\r\n");

    // Establish the WiFi event handler
    Network.onEvent(r4aEsp32WifiEvent);
}

//*********************************************************************
// Stop the WiFi event handler
void R4A_WIFI::eventHandlerStop()
{
    Print * debug;

    debug = (Print *)_debug;
    if (debug)
        debug->printf("Stopping the WiFi event handler\r\n");

    Network.removeEvent(r4aEsp32WifiEvent);
}

//*********************************************************************
// Get the mDNS host name
const char * R4A_WIFI::hostNameGet()
{
    return _hostName;
}

//*********************************************************************
// List the remote APs
// Inputs:
//   display: Address of a Print object used to list the APs
void R4A_WIFI::listAPs(Print * display)
{
    stationSelectAP(false, display);
}

//*********************************************************************
// Determine if mDNS is available
// Outputs:
//   True when the mDNS address translation is available
bool R4A_WIFI::mdnsAvailable()
{
    return _mdnsAvailable;
}

//*********************************************************************
// Start the multicast domain name server
// Outputs:
//   Returns true if successful and false upon failure
bool R4A_WIFI::mDNSStart()
{
    Print * debug;
    Print * display;
    bool started;

    do
    {
        // Always output display items
        debug = (Print *)_debug;
        display = (Print *)_display;
        if (!display)
            display = debug;

        // Verify that a host name exists
        started = (_hostName != nullptr) && (strlen(_hostName) != 0);
        if (!started)
        {
            Serial.printf("ERROR: No mDNS host name specified!\r\n");
            break;
        }

        // Start mDNS
        if (debug)
            debug->printf("Starting mDNS\r\n");
        started = MDNS.begin(_hostName);
        if (!started)
        {
            Serial.printf("ERROR: Failed to start mDNS!\r\n");
            break;
        }
        _mdnsAvailable = started;
        if (display)
            display->printf("mDNS started as %s.local\r\n", _hostName);
    } while (0);
    return started;
}

//*********************************************************************
// Stop the multicast domain name server
void R4A_WIFI::mDNSStop()
{
    Print * debug;

    debug = (Print *)_debug;
    if (debug)
        debug->printf("Stopping mDNS\r\n");
    _mdnsAvailable = false;
    MDNS.end();
    if (debug)
        debug->printf("mDNS stopped\r\n");
}

//*********************************************************************
// Get the connected SSID
//   Returns the address of a zero terminated string of characters or nullptr
const char * R4A_WIFI::ssidGet()
{
    return _apSSID;
}

//*********************************************************************
// Connect the station to a remote AP
bool R4A_WIFI::stationConnectAP()
{
    bool connected;
    Print * debug;
    Print * display;

    do
    {
        // Determine if a remote AP is available
        debug = (Print *)_debug;
        display = (Print *)_display;
        if (!display)
            display = debug;
        connected = _apFound;
        if (!connected)
        {
            Serial.printf("ERROR: No remote AP found!\r\n");
            break;
        }

        // Connect to the remote AP
        if (debug)
            debug->printf("WiFi connecting to %s on channel %d with %s authorization\r\n",
                          _apSSID,
                          _wifiChannel,
                          (_authType < WIFI_AUTH_MAX) ? r4aEsp32WiFiAuthorizationName[_authType] : "Unknown");
        connected = (WiFi.STA.connect(_apSSID, _apPassword, _wifiChannel));
        if (!connected)
        {
            if (debug)
                debug->printf("WIFI failed to connect to SSID %s with password %s\r\n", _apSSID, _apPassword);
            break;
        }
        if (display)
            display->printf("WiFi station connected to %s on channel %d with %s authorization\r\n",
                            _apSSID,
                            _wifiChannel,
                            (_authType < WIFI_AUTH_MAX) ? r4aEsp32WiFiAuthorizationName[_authType] : "Unknown");
    } while (0);
    return connected;
}

//*********************************************************************
// Determine if the stataion is connected to a remote access point
// Outputs:
//   True if station is connected to remote AP
bool R4A_WIFI::stationConnected()
{
    return _stationConnected;
}

//*********************************************************************
// Disconnect the station from an AP
// Outputs:
//   Returns true if successful and false upon failure
bool R4A_WIFI::stationDisconnect()
{
    Print * debug;
    bool disconnected;

    do
    {
        // Determine if station is connected to a remote AP
        debug = (Print *)_debug;
        disconnected = !_stationConnected;
        if (disconnected)
        {
            if (debug)
                debug->printf("Station already disconnected from remote AP\r\n");
            break;
        }

        // Disconnect from the remote AP
        if (debug)
            debug->printf("WiFI disconnection station\r\n");
        disconnected = WiFi.STA.disconnect();
        if (!disconnected)
        {
            Serial.printf("ERROR: Failed to disconnect WiFi from the AP!\r\n");
            break;
        }
        if (debug)
            debug->printf("WiFi disconnected from the AP\r\n");
    } while (0);
    return disconnected;
}

//*********************************************************************
// Determine if the station has an IP address
// Outputs:
//    True if the station has an IP address
bool R4A_WIFI::stationHasIp()
{
    return _stationHasIp;
}

//*********************************************************************
// Set the station's host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool R4A_WIFI::stationHostName(const char * hostName)
{
    Print * debug;
    bool nameSet;

    do
    {
        // Verify that a host name was specified
        debug = (Print *)_debug;
        nameSet =  (hostName != nullptr) && (strlen(hostName) != 0);
        if (!nameSet)
        {
            Serial.printf("ERROR: No host name specified!\r\n");
            break;
        }

        // Set the host name
        if (debug)
            debug->printf("WiFI setting station host name\r\n");
        nameSet = WiFi.STA.setHostname(hostName);
        if (!nameSet)
        {
            Serial.printf("ERROR: Failed to set the Wifi station host name!\r\n");
            break;
        }
        if (debug)
            debug->printf("WiFi station hostname: %s\r\n", hostName);
    } while (0);
    return nameSet;
}

//*********************************************************************
// Start the WiFi scan
// Inputs:
//   channel: Channel number for the scan, zero (0) scan all channels
// Outputs:
//   Returns true if successful and false upon failure
bool R4A_WIFI::stationScanStart(R4A_WIFI_CHANNEL_t channel)
{
    Print * debug;
    int16_t status;

    do
    {
        // Determine if the WiFi scan is already running
        debug = (Print *)_debug;
        if (_wifiScanRunning)
        {
            if (debug)
                debug->printf("WiFi scan already running");
            break;
        }

        // Determine if scanning a single channel or all channels
        if (debug)
        {
            if (channel)
                debug->printf("WiFI starting AP scan on channel %d\r\n", channel);
            else
                debug->printf("WiFI starting AP scan\r\n");
        }

        // Start the WiFi scan
        log_v("Calling WiFi.scanNetworks");
        status = WiFi.scanNetworks(true,        // async
                                   false,       // show_hidden
                                   false,       // passive
                                   300,         // max_ms_per_chan
                                   channel,     // channel number
                                   nullptr,     // ssid *
                                   nullptr);    // bssid *
        log_v("WiFi.scanNetworks returned %d", status);
        _wifiScanRunning = (status == WIFI_SCAN_RUNNING);
        if (!_wifiScanRunning)
        {
            Serial.printf("ERROR: WiFi scan failed to start!\r\n");
            break;
        }
        if (debug)
        {
            if (channel)
                debug->printf("WiFi scan started on channel %d\r\n", channel);
            else
                debug->printf("WiFi scan started\r\n");
        }
    } while (0);
    return _wifiScanRunning;
}

//*********************************************************************
// Select the AP and channel to use for WiFi station
// Inputs:
//   connect: Set to true for connection
//   list: Address of a Print object used to list the APs or nullptr
// Outputs:
//   Returns the channel number of the AP
R4A_WIFI_CHANNEL_t R4A_WIFI::stationSelectAP(bool connect, Print * list)
{
    int ap;
    R4A_WIFI_CHANNEL_t apChannel;
    int authIndex;
    R4A_WIFI_CHANNEL_t channel;
    Print * debug;
    const char * ssid;
    String ssidString;
    int type;

    // Determine the value to use for listing
    debug = (Print *)_debug;
    if (!list)
        list = debug;

    // Verify that an AP was found
    if (_apCount == 0)
        return 0;

    // Print the header
    //                                    1                 1         2         3
    //             1234   1234   123456789012345   12345678901234567890123456789012
    if (list)
    {
        list->printf(" dBm   Chan   Authorization     SSID\r\n");
        list->printf("----   ----   ---------------   --------------------------------\r\n");
    }

    // Walk the list of APs that were found during the scan
    _apFound = false;
    apChannel = 0;
    for (ap = 0; ap < _apCount; ap++)
    {
        // The APs are listed in decending signal strength order
        // Check for a requested AP
        ssidString = WiFi.SSID(ap);
        ssid = ssidString.c_str();
        type = WiFi.encryptionType(ap);
        channel = WiFi.channel(ap);
        if (connect && (!_apFound))
        {
            for (authIndex = 0; authIndex < r4aWifiSsidPasswordEntries; authIndex++)
            {
                // Determine if this authorization matches the AP's SSID
                if (*r4aWifiSsidPassword[authIndex].ssid
                    && strlen(*r4aWifiSsidPassword[authIndex].ssid)
                    && (strcmp(ssid, *r4aWifiSsidPassword[authIndex].ssid) == 0)
                    && ((type == WIFI_AUTH_OPEN)
                        || (*r4aWifiSsidPassword[authIndex].password
                            && (strlen(*r4aWifiSsidPassword[authIndex].password)))))
                {
                    if (debug)
                        debug->printf("WiFi: Found remote AP: %s\r\n", ssid);

                    // A match was found, save it and stop looking
                    _apSSID = *r4aWifiSsidPassword[authIndex].ssid;
                    _apPassword = *r4aWifiSsidPassword[authIndex].password;
                    apChannel = channel;
                    _authType = type;
                    _apFound = true;
                    break;
                }
            }

            // Check for done
            if (_apFound && (!list))
                break;
        }

        // Display the list of APs
        if (list)
            list->printf("%4ld   %4d   %s   %s\r\n",
                         WiFi.RSSI(ap),
                         channel,
                         (type < WIFI_AUTH_MAX) ? r4aEsp32WiFiAuthorizationName[type] : "Unknown",
                         ssid);
    }

    // Return the channel number
    return apChannel;
}

//*********************************************************************
// Start the Wifi station
bool R4A_WIFI::stationStart()
{
    int authIndex;
    Print * debug;
    wifi_mode_t mode;
    bool started;
    esp_err_t status;

    do
    {
        // Verify that at least one WiFi access point is in the list
        debug = (Print *)_debug;
        started = (r4aWifiSsidPasswordEntries != 0);
        if (!started)
        {
            Serial.printf("ERROR: No entries in r4aWifiSsidPasswordEntries\r\n");
            break;
        }

        // Verify that at least one SSID is set
        log_v("Checking for configured WiFi SSID");
        for (authIndex = 0; authIndex < r4aWifiSsidPasswordEntries; authIndex++)
            if (*r4aWifiSsidPassword[authIndex].ssid
                && (strlen(*r4aWifiSsidPassword[authIndex].ssid)))
            {
                break;
            }
        if (authIndex >= r4aWifiSsidPasswordEntries)
        {
            Serial.printf("ERROR: No valid SSID in r4aWifiSsidPassword\r\n");
            break;
        }
        log_v("Configured WiFi SSID found");

        // Determine if WiFi is running
        mode = WiFi.getMode();
        started = ((mode & WIFI_MODE_STA) != 0);
        if (started)
        {
            if (debug)
                debug->printf("WiFi: Station already started\r\n");
            if (_stationConnected)
                break;
        }
        else
        {
            // Start the WiFi event handler
            log_v("Starting WiFi event handler");
            eventHandlerStart();

            // Attempt to enable WiFi station
            if (debug)
                debug->printf("WiFi: Starting station mode\r\n");
            else
            {
                log_v("WiFi: Starting station mode");
            }
            started = WiFi.mode((wifi_mode_t)(mode | WIFI_MODE_STA));
            log_v("WiFi station mode %sstarted", started ? "" : "NOT ");
            if (!started)
            {
                Serial.printf("ERROR: Failed to stop WiFi station mode!\r\n");
                break;
            }
            if (debug)
                debug->printf("WiFi: Started station mode\r\n");

            // Set the protocols
            if (debug)
                debug->printf("WiFi: Selecting WiFi frequencies and protocols\r\n");
            log_v("Calling esp_wifi_set_protocol");
            status = esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B
                                                        | WIFI_PROTOCOL_11G
                                                        | WIFI_PROTOCOL_11N);
            log_v("esp_wifi_set_protocol returned %s", esp_err_to_name(status));
            started = (status == ESP_OK);
            if (!started)
            {
                Serial.printf("ERROR: Failed to set WiFi frequencies and protocols!\r\n");
                break;
            }
            if (debug)
                debug->printf("WiFi: WiFi frequencies and protocols set successfully\r\n");
        }

        // Start the WiFi scan
        log_v("Calling stationScanStart");
        started = stationScanStart(_wifiChannel);
    } while (0);
    return started;
}

//*********************************************************************
// Stop the WiFi station
bool R4A_WIFI::stationStop()
{
    Print * debug;
    wifi_mode_t mode;
    bool stopped;

    do {
        // Determine if WiFi station is already stopped
        debug = (Print *)_debug;
        if (debug)
            debug->printf("WiFI stopping station mode\r\n");
        mode = WiFi.getMode();
        stopped = ((mode & WIFI_MODE_STA) == 0);
        if (stopped)
        {
            if (debug)
                debug->printf("WiFi STA already stopped\r\n");
            break;
        }

        // Stop mDNS
        mDNSStop();

        // Disconnect from the remote AP
        stopped = stationDisconnect();
        if (!stopped)
            break;

        // Stop WiFi station
        stopped = WiFi.mode((wifi_mode_t)(mode & ~WIFI_MODE_STA));
        if (!stopped)
        {
            Serial.printf("ERROR: Failed to stop WiFi STA!\r\n");
            break;
        }
        if (debug)
            debug->printf("WiFI STA: Started --> Stopped\r\n");

        // Remove the WiFi event handler
        eventHandlerStop();
    } while (0);
    return stopped;
}

//*********************************************************************
// Test the WiFi modes, call from loop
void R4A_WIFI::test()
{
    uint32_t currentMsec;
    static uint32_t lastScanMsec = - (180 * 1000);
    int rand;

    // Delay the mode change until after the WiFi scan completes
    currentMsec = millis();
    if (_wifiScanRunning)
        lastScanMsec = currentMsec;

    // Check if it time for a mode change
    else if ((currentMsec - lastScanMsec) >= (8 * 1000))
    {
        lastScanMsec = currentMsec;

        // Get a random number
        rand = random() & 3;

        // Determine the next actions
        switch (rand)
        {
        default:
            lastScanMsec = 0;
            break;

        case 0:
            Serial.printf("--------------------  %d: Stop  --------------------\r\n", rand);
            stationStop();
            break;

        case 1:
            Serial.printf("--------------------  %d: Start  -------------------\r\n", rand);
            stationStart();
            break;

        case 2:
            Serial.printf("--------------------  %d: Disconnect  --------------\r\n", rand);
            stationDisconnect();
            break;
        }
    }
}

//*********************************************************************
// Check for reconnection after disconnect, call from loop
void R4A_WIFI::update()
{
    // Check for reconnection request
    if (_wifiTimer)
    {
        if ((millis() - _wifiTimer) >= R4A_WIFI_RECONNECTION_DELAY)
        {
            _wifiTimer = 0;

            // Start the WiFi scan
            stationScanStart(_wifiChannel);
        }
    }
}

//*********************************************************************
// Verify the WiFi tables
void R4A_WIFI::verifyTables()
{
    Print * debug;
    if (WIFI_AUTH_MAX != r4aEsp32WiFiAuthorizationNameEntries)
    {
        debug = (Print *)_debug;
        if (!debug)
            debug = &Serial;
        ((Print *)debug)->printf("ERROR: Fix authorizationNameName list to match wifi_auth_mode_t in esp_wifi_types.h!\r\n");
        r4aReportFatalError("Fix authorizationNameName list to match wifi_auth_mode_t");
    }
}
