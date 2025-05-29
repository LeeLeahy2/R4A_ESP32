/**********************************************************************
  Wifi.cpp

  WiFi layer, supports use by ESP-NOW, soft AP and WiFi station

  Modified from https://raw.githubusercontent.com/sparkfun/SparkFun_RTK_Everywhere_Firmware/refs/heads/main/Firmware/RTK_Everywhere/WiFi.ino
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Constants
//****************************************

#define WIFI_DEFAULT_CHANNEL            1
#define WIFI_IP_ADDRESS_TIMEOUT_MSEC    (15 * 1000)

static const char * r4aWifiAuthorizationName[] =
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
static const int r4aWifiAuthorizationNameEntries =
    sizeof(r4aWifiAuthorizationName) / sizeof(r4aWifiAuthorizationName[0]);

//****************************************
// Constants
//****************************************

// Common operations
#define WIFI_START_EVENT_HANDLER                 1
#define WIFI_START_MDNS                          2

// Radio operations
#define WIFI_AP_SET_MODE                         4
#define WIFI_EN_SET_MODE                         8
#define WIFI_STA_SET_MODE               0x00000010
#define WIFI_AP_SET_PROTOCOLS           0x00000020
#define WIFI_EN_SET_PROTOCOLS           0x00000040
#define WIFI_STA_SET_PROTOCOLS          0x00000080
#define WIFI_STA_START_SCAN             0x00000100
#define WIFI_STA_SELECT_REMOTE_AP       0x00000200
#define WIFI_AP_SELECT_CHANNEL          0x00000400
#define WIFI_EN_SELECT_CHANNEL          0x00000800
#define WIFI_STA_SELECT_CHANNEL         0x00001000

// Soft AP
#define WIFI_AP_SET_SSID_PASSWORD       0x00002000
#define WIFI_AP_SET_IP_ADDR             0x00004000
#define WIFI_AP_SET_HOST_NAME           0x00008000
#define WIFI_AP_START_MDNS              0x00010000
#define WIFI_AP_START_DNS_SERVER        0x00020000
#define WIFI_AP_ONLINE                  0x00040000

// WiFi station
#define WIFI_STA_SET_HOST_NAME          0x00080000
#define WIFI_STA_DISABLE_AUTO_RECONNECT 0x00100000
#define WIFI_STA_CONNECT_TO_REMOTE_AP   0x00200000
#define WIFI_STA_START_MDNS             0x00400000
#define WIFI_STA_ONLINE                 0x00800000

// ESP-NOW
#define WIFI_EN_SET_CHANNEL             0x01000000
#define WIFI_EN_SET_PROMISCUOUS_MODE    0x02000000
#define WIFI_EN_START_ESP_NOW           0x04000000
#define WIFI_EN_ESP_NOW_ONLINE          0x08000000

// WIFI_MAX_START must be the last value in the define list
#define WIFI_MAX_START                  0x10000000

const char * const r4aWifiStartNames[] =
{
    "WIFI_START_EVENT_HANDLER",
    "WIFI_START_MDNS",

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
    "WIFI_AP_START_MDNS",
    "WIFI_AP_START_DNS_SERVER",
    "WIFI_AP_ONLINE",

    "WIFI_STA_SET_HOST_NAME",
    "WIFI_STA_DISABLE_AUTO_RECONNECT",
    "WIFI_STA_CONNECT_TO_REMOTE_AP",
    "WIFI_STA_START_MDNS",
    "WIFI_STA_ONLINE",

    "WIFI_EN_SET_CHANNEL",
    "WIFI_EN_SET_PROMISCUOUS_MODE",
    "WIFI_EN_START_ESP_NOW",
    "WIFI_EN_ESP_NOW_ONLINE",
};
const int r4aWifiStartNamesEntries = sizeof(r4aWifiStartNames) / sizeof(r4aWifiStartNames[0]);

#define WIFI_START_ESP_NOW          (WIFI_START_EVENT_HANDLER           \
                                     | WIFI_EN_SET_MODE                 \
                                     | WIFI_EN_SET_PROTOCOLS            \
                                     | WIFI_EN_SELECT_CHANNEL           \
                                     | WIFI_EN_SET_CHANNEL              \
                                     | WIFI_EN_SET_PROMISCUOUS_MODE     \
                                     | WIFI_EN_START_ESP_NOW            \
                                     | WIFI_EN_ESP_NOW_ONLINE)

#define WIFI_START_SOFT_AP          (WIFI_START_EVENT_HANDLER       \
                                     | WIFI_AP_SET_MODE             \
                                     | WIFI_AP_SET_PROTOCOLS        \
                                     | WIFI_AP_SELECT_CHANNEL       \
                                     | WIFI_AP_SET_SSID_PASSWORD    \
                                     | WIFI_AP_SET_IP_ADDR          \
                                     | WIFI_AP_SET_HOST_NAME        \
                                     | WIFI_AP_START_MDNS           \
                                     | WIFI_AP_START_DNS_SERVER     \
                                     | WIFI_AP_ONLINE)

#define WIFI_START_STATION          (WIFI_START_EVENT_HANDLER           \
                                     | WIFI_STA_SET_MODE                \
                                     | WIFI_STA_SET_PROTOCOLS           \
                                     | WIFI_STA_START_SCAN              \
                                     | WIFI_STA_SELECT_CHANNEL          \
                                     | WIFI_STA_SELECT_REMOTE_AP        \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_STA_START_MDNS              \
                                     | WIFI_STA_ONLINE)

#define WIFI_STA_RECONNECT          (WIFI_STA_START_SCAN                \
                                     | WIFI_STA_SELECT_CHANNEL          \
                                     | WIFI_STA_SELECT_REMOTE_AP        \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_AP_START_MDNS               \
                                     | WIFI_STA_ONLINE)

#define WIFI_SELECT_CHANNEL         (WIFI_AP_SELECT_CHANNEL     \
                                     | WIFI_EN_SELECT_CHANNEL   \
                                     | WIFI_STA_SELECT_CHANNEL)

#define WIFI_STA_NO_REMOTE_AP       (WIFI_STA_SELECT_CHANNEL            \
                                     | WIFI_STA_SET_HOST_NAME           \
                                     | WIFI_STA_DISABLE_AUTO_RECONNECT  \
                                     | WIFI_STA_CONNECT_TO_REMOTE_AP    \
                                     | WIFI_AP_START_MDNS               \
                                     | WIFI_STA_ONLINE)

#define WIFI_STA_FAILED_SCAN        (WIFI_STA_START_SCAN          \
                                     | WIFI_STA_SELECT_REMOTE_AP  \
                                     | WIFI_AP_START_MDNS         \
                                     | WIFI_STA_NO_REMOTE_AP)

#define WIFI_MAX_TIMEOUT    (15 * 60 * 1000)    // Timeout in milliseconds
#define WIFI_MIN_TIMEOUT    (15 * 1000)         // Timeout in milliseconds

//****************************************
// Data structures
//****************************************

// Structure containing WiFi private data
typedef struct _R4A_WIFI
{
    R4A_WIFI_CHANNEL_t _apChannel; // Channel required for soft AP, zero (0) use _channel
    int16_t _apCount;           // The number or remote APs detected in the WiFi network
    IPAddress _apDnsAddress;    // DNS IP address to use while translating names into IP addresses
    IPAddress _apFirstDhcpAddress;  // First IP address to use for DHCP
    IPAddress _apGatewayAddress;// IP address of the gateway to the larger network (internet?)
    IPAddress _apIpAddress;     // IP address of the soft AP
    uint8_t _apMacAddress[6];   // MAC address of the soft AP
    IPAddress _apSubnetMask;    // Subnet mask for soft AP
    R4A_WIFI_CHANNEL_t _espNowChannel; // Channel required for ESPNow, zero (0) use _channel
    volatile bool _scanRunning; // Scan running
    int _staAuthType;           // Authorization type for the remote AP
    bool _staConnected;         // True when station is connected
    bool _staEnabled;           // True when at least one SSID is present
    bool _staHasIp;             // True when station has IP address
    IPAddress _staIpAddress;    // IP address of the station
    uint8_t _staIpType;         // 4 or 6 when IP address is assigned
    volatile uint8_t _staMacAddress[6];  // MAC address of the station
    const char * _staRemoteApSsid;       // SSID of remote AP
    const char * _staRemoteApPassword;   // Password of remote AP
    volatile R4A_WIFI_ACTION_t _started; // Components that are started and running
    R4A_WIFI_CHANNEL_t _stationChannel;  // Channel required for station, zero (0) use _channel
    uint32_t _timer;            // Reconnection timer
    bool _usingDefaultChannel;  // Using default WiFi channel
} R4A_WIFI;

//****************************************
// Locals
//****************************************

// DNS server for Captive Portal
static DNSServer r4aWifiDnsServer;

// Start timeout
static uint32_t r4aWifiStartTimeout;

static int r4aWifiFailedConnectionAttempts = 0; // Count the number of connection attempts between restarts
static bool r4aWifiReconnectRequest; // Set true to request WiFi reconnection

static R4A_WIFI r4aWiFi;
static network_event_handle_t r4aWifiEventHandle;

//****************************************
// Globals - For other module direct access
//******************h**********************

R4A_WIFI_CHANNEL_t r4aWifiChannel; // Current WiFi channel number
bool r4aWifiDebug;                 // Set true to display debug output
bool r4aWifiEspNowOnline;          // ESP-Now started successfully
bool r4aWifiEspNowRunning;         // False: stopped, True: starting, running, stopping
uint32_t r4aWifiReconnectionTimer; // Delay before reconnection, timer running when non-zero
bool r4aWifiRestartRequested;      // Restart WiFi if user changes anything
bool r4aWifiSoftApOnline;          // WiFi soft AP started successfully
bool r4aWifiSoftApRunning;         // False: stopped, True: starting, running, stopping
bool r4aWifiStationOnline;         // WiFi station started successfully
bool r4aWifiStationRunning;        // False: stopped, True: starting, running, stopping
bool r4aWifiVerbose;               // True causes more debug output to be displayed

//****************************************
// Forward routine declarations
//******************h**********************

const char * r4aWifiPrintStatus(wl_status_t wifiStatus);
void r4aWifiResetThrottleTimeout();
void r4aWifiResetTimeout();
void r4aWifiSoftApEventHandler(arduino_event_id_t event, arduino_event_info_t info);
bool r4aWiFiStationEnabled();
void r4aWifiStationEventHandler(arduino_event_id_t event, arduino_event_info_t info);
void r4aWifiStationLostIp();
bool r4aWifiStopStart(R4A_WIFI_ACTION_t stopping, R4A_WIFI_ACTION_t starting);

//*********************************************************************
// Perform the WiFi initialization
void r4aWifiBegin()
{
    // Initialize the globals
    r4aWifiChannel = 0;
    r4aWifiEspNowOnline = false;
    r4aWifiEspNowRunning = false;
    r4aWifiFailedConnectionAttempts = 0;
    r4aWifiReconnectionTimer = 0;
    r4aWifiRestartRequested = false;
    r4aWifiSoftApOnline = false;
    r4aWifiSoftApRunning = false;
    r4aWifiStationOnline = false;
    r4aWifiStationRunning = false;

    // Initialize the data structure members
    r4aWiFi._apChannel = 0;
    r4aWiFi._apCount = 0;
    r4aWiFi._apDnsAddress = IPAddress((uint32_t)0);
    r4aWiFi._apFirstDhcpAddress = IPAddress("192.168.4.32");
    r4aWiFi._apGatewayAddress = (uint32_t)0;
    r4aWiFi._apIpAddress = IPAddress("192.168.4.1");
    memset((void *)r4aWiFi._apMacAddress, 0, sizeof(r4aWiFi._apMacAddress));
    r4aWiFi._apSubnetMask = IPAddress("255.255.255.0");
    r4aWiFi._espNowChannel = 0;
    r4aWiFi._scanRunning = false;
    r4aWiFi._staEnabled = r4aWiFiStationEnabled();
    r4aWiFi._staIpAddress = IPAddress((uint32_t)0);
    r4aWiFi._staIpType = 0;
    memset((void *)r4aWiFi._staMacAddress, 0, sizeof(r4aWiFi._staMacAddress));
    r4aWiFi._staRemoteApSsid = nullptr;
    r4aWiFi._staRemoteApPassword = nullptr;
    r4aWiFi._started = false;
    r4aWiFi._stationChannel = 0;
    r4aWiFi._usingDefaultChannel = true;

    // Prepare to start WiFi immediately
    r4aWifiResetThrottleTimeout();
    r4aWifiResetTimeout();
}

//*********************************************************************
// Clear some of the started components
// Inputs:
//   components: Bitmask of components to clear
// Outputs:
//   Returns the bitmask of started components
R4A_WIFI_ACTION_t r4aWifiClearStarted(R4A_WIFI_ACTION_t components)
{
    r4aWiFi._started = r4aWiFi._started & ~components;
    return r4aWiFi._started;
}

//*********************************************************************
// Attempts a connection to all provided SSIDs
// Inputs:
//   timeout: Number of milliseconds to wait for the connection
//   startAP: Set true to start AP mode, false does not change soft AP
//             status
// Outputs:
//   Returns true if successful and false upon timeout, no matching
//   SSID or other failure
bool r4aWifiConnect(unsigned long timeout, bool startAP)
{
    bool started;

    // Display warning
    log_w("WiFi: Not using timeout parameter for connect!\r\n");

    // Enable WiFi station if necessary
    started = false;
    if (r4aWifiStationRunning == false)
        started = r4aWifiStationOn( __FILE__, __LINE__);
    else if (startAP && !r4aWifiSoftApRunning)
        started = r4aWifiSoftApOn( __FILE__, __LINE__);

    // Determine the WiFi station status
    if (started)
    {
        wl_status_t wifiStatus = WiFi.STA.status();
        started = (wifiStatus == WL_CONNECTED);
        if (wifiStatus == WL_DISCONNECTED)
            Serial.print("No friendly WiFi networks detected.\r\n");
        else if (wifiStatus != WL_CONNECTED)
            Serial.printf("WiFi failed to connect: error #%d - %s\r\n",
                         wifiStatus, r4aWifiPrintStatus(wifiStatus));
    }
    return started;
}

//*********************************************************************
// Display components begin started or stopped
// Inputs:
//   text: Text describing the component list
//   components: A bit mask of the components
void r4aWifiDisplayComponents(const char * text, R4A_WIFI_ACTION_t components)
{
    R4A_WIFI_ACTION_t mask;

    Serial.printf("%s: 0x%08lx\r\n", text, components);
    for (int index = r4aWifiStartNamesEntries - 1; index >= 0; index--)
    {
        mask = 1 << index;
        if (components & mask)
            Serial.printf("    0x%08lx: %s\r\n", mask, r4aWifiStartNames[index]);
    }
}

//*********************************************************************
// Display the WiFi state
void r4aWifiDisplayState()
{
    Serial.printf("WiFi: %s\r\n", r4aWifiStationOnline ? "Online" : "Offline");
    Serial.printf("    MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                  r4aWiFi._staMacAddress[0], r4aWiFi._staMacAddress[1],
                  r4aWiFi._staMacAddress[2], r4aWiFi._staMacAddress[3],
                  r4aWiFi._staMacAddress[4], r4aWiFi._staMacAddress[5]);
    if (r4aWifiStationOnline)
    {
        // Get the DNS addresses
        IPAddress dns1 = WiFi.STA.dnsIP(0);
        IPAddress dns2 = WiFi.STA.dnsIP(1);
        IPAddress dns3 = WiFi.STA.dnsIP(2);

        // Get the WiFi status
        wl_status_t wifiStatus = WiFi.status();

        const char *wifiStatusString = r4aWifiPrintStatus(wifiStatus);

        // Display the WiFi state
        Serial.printf("    SSID: %s\r\n", WiFi.STA.SSID().c_str());
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
// Enable or disable the WiFi modes
// Inputs:
//   enableESPNow: Enable ESP-NOW mode
//   enableSoftAP: Enable soft AP mode
//   enableStataion: Enable station mode
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if the modes were successfully configured
bool r4aWifiEnable(bool enableESPNow,
                   bool enableSoftAP,
                   bool enableStation,
                   const char * fileName,
                   int lineNumber)
{
    R4A_WIFI_ACTION_t starting;
    bool status;
    R4A_WIFI_ACTION_t stopping;

    // Turn on WiFi debugging if necessary
    if (r4aWifiVerbose)
        r4aWifiDebug = true;

    // Determine the next actions
    starting = 0;
    stopping = 0;

    // Display the parameters
    if (r4aWifiDebug && r4aWifiVerbose)
    {
        Serial.printf("WiFi: wifiEnable called from %s line %d\r\n", fileName, lineNumber);
        Serial.printf("enableESPNow: %s\r\n", enableESPNow ? "true" : "false");
        Serial.printf("enableSoftAP: %s\r\n", enableSoftAP ? "true" : "false");
        Serial.printf("enableStation: %s\r\n", enableStation ? "true" : "false");
    }

#ifdef  COMPILE_ESPNOW
    // Update the ESP-NOW state
    if (enableESPNow)
    {
        starting |= WIFI_START_ESP_NOW;
        r4aWifiEspNowRunning = true;
    }
    else
    {
        stopping |= WIFI_START_ESP_NOW;
        r4aWifiEspNowRunning = false;
    }
#endif  // COMPILE_ESPNOW

    // Update the soft AP state
    if (enableSoftAP)
    {
        // Verify that the SSID is set
        if (r4aWifiSoftApSsid && strlen(r4aWifiSoftApSsid)
            && ((r4aWifiSoftApPassword == nullptr) || (strlen(r4aWifiSoftApPassword) >= 8)))
        {
            starting |= WIFI_START_SOFT_AP;
            r4aWifiSoftApRunning = true;
        }
        else
        {
            if (r4aWifiSoftApPassword == nullptr)
                Serial.printf("ERROR: AP SSID or password is missing\r\n");
            else
                Serial.printf("ERROR: AP password must be >= 8 characters\r\n");
        }
    }
    else
    {
        stopping |= WIFI_START_SOFT_AP;
        r4aWifiSoftApRunning = false;
    }

    // Update the station state
    if (enableStation && r4aWiFi._staEnabled)
    {
        // Start the WiFi station
        starting |= WIFI_START_STATION;
        r4aWifiStationRunning = true;
    }
    else
    {
        // Stop the WiFi station
        stopping |= WIFI_START_STATION;
        r4aWifiStationRunning = false;
    }

    // Stop and start the WiFi components
    status = r4aWifiStopStart(stopping, starting);
    if (r4aWifiDebug && r4aWifiVerbose)
        Serial.printf("WiFi: r4aWifiEnable returning %s\r\n", status ? "true" : "false");
    return status;
}

//*********************************************************************
// Stop ESP-NOW
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiEspNowOff(const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (r4aWifiDebug)
        Serial.printf("wifiEspNowOff called in %s at line %ld\r\n",
                      fileName, lineNumber);

    if (r4aWifiEspNowRunning)
        return r4aWifiEnable(false, r4aWifiSoftApRunning, r4aWifiStationRunning, __FILE__, __LINE__);
    return true;
}

//*********************************************************************
// Start ESP-NOW
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiEspNowOn(const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (r4aWifiDebug)
        Serial.printf("wifiEspNowOn called in %s at line %ld\r\n",
                      fileName, lineNumber);

    if (r4aWifiEspNowRunning == false)
        return r4aWifiEnable(true, r4aWifiSoftApRunning, r4aWifiStationRunning, __FILE__, __LINE__);
    return true;
}

//*********************************************************************
// Set the ESP-NOW channel
// Inputs:
//   channel: New ESP-NOW channel number
void r4aWifiEspNowSetChannel(R4A_WIFI_CHANNEL_t channel)
{
    r4aWiFi._espNowChannel = channel;
}

//*********************************************************************
// Handle the WiFi event
// Inputs:
//   event: Arduino ESP32 event number found on
//          https://github.com/espressif/arduino-esp32
//          in libraries/Network/src/NetworkEvents.h
//   info: Additional data about the event
void r4aWifiEventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    if (r4aWifiDebug)
        Serial.printf("event: %d (%s)\r\n", event, NetworkEvents::eventName(event));

    // Handle the event
    switch (event)
    {
	default:
		break;

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
        r4aWifiStationEventHandler(event, info);
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
        r4aWifiStationEventHandler(event, info);
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
        r4aWifiSoftApEventHandler(event, info);
        break;
    }
}

//*********************************************************************
// Return the start timeout in milliseconds
uint32_t r4aWifiGetStartTimeout()
{
    return r4aWifiStartTimeout;
}

//*********************************************************************
bool r4aWifiMdnsStart(bool softAp)
{
    bool started;
    IPAddress * ipAddress;

    do
    {
        started = true;
        if (r4aWifiHostName)
        {
            // Verify that a host name exists
            started = (strlen(r4aWifiHostName) != 0);
            if (!started)
            {
                Serial.printf("ERROR: No mDNS host name specified!\r\n");
                break;
            }

            // Start mDNS
            if (r4aWifiDebug)
                Serial.printf("Starting mDNS on %s\r\n", softAp ? "soft AP" : "WiFi station");
            started = MDNS.begin(r4aWifiHostName);
            if (!started)
            {
                Serial.printf("ERROR: Failed to start mDNS for %s!\r\n",
                              softAp ? "soft AP" : "WiFi station");
                break;
            }
            if (r4aWifiDebug)
            {
                ipAddress = softAp ? &r4aWiFi._apIpAddress : &r4aWiFi._staIpAddress;
                Serial.printf("mDNS started on %s as %s.local (%s)\r\n",
                              softAp ? "soft AP" : "WiFi station",
                              r4aWifiHostName,
                              ipAddress->toString().c_str());
            }
            r4aWiFi._started = r4aWiFi._started | WIFI_START_MDNS;
        }
    } while (0);
    return started;
}

//*********************************************************************
void r4aWifiMdnsStop()
{
    if (r4aWifiDebug)
        Serial.printf("WiFi: Stopping mDNS\r\n");
    MDNS.end();
    r4aWiFi._started = r4aWiFi._started & ~WIFI_START_MDNS;
}

//*********************************************************************
// Counts the number of entered SSIDs
int r4aWifiNetworkCount()
{
    // Count SSIDs
    int networkCount = 0;
    for (int x = 0; x < r4aWifiSsidPasswordEntries; x++)
    {
        if (strlen(*r4aWifiSsidPassword[x].ssid) > 0)
            networkCount++;
    }
    return networkCount;
}

//*********************************************************************
// Given a status, return the associated state or error
const char * r4aWifiPrintStatus(wl_status_t wifiStatus)
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
// Reset the last WiFi start attempt
// Useful when WiFi settings have changed
void r4aWifiResetThrottleTimeout()
{
    r4aWifiReconnectionTimer = millis() - WIFI_MAX_TIMEOUT;
}

//*********************************************************************
// Set WiFi timeout back to zero
// Useful if other things (such as a successful ethernet connection) need
// to reset wifi timeout
void r4aWifiResetTimeout()
{
    r4aWifiStartTimeout = 0;
    if (r4aWifiDebug == true)
        Serial.println("WiFi: Start timeout reset to zero");
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
bool r4aWifiSetWiFiMode(uint8_t setMode, uint8_t xorMode)
{
    uint8_t mode;
    uint8_t newMode;
    bool started;

    started = false;
    do
    {
        // Get the current mode
        mode = (uint8_t)WiFi.getMode();
        if (r4aWifiDebug && r4aWifiVerbose)
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
            if (r4aWifiDebug)
                r4aEsp32HeapDisplay();
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
        if (r4aWifiDebug && r4aWifiVerbose)
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
bool r4aWifiSetWiFiProtocols(wifi_interface_t interface,
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
        if (r4aWifiDebug && r4aWifiVerbose)
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
        if (r4aWifiDebug && r4aWifiVerbose)
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
//   firstDhcpAddress: First IP address to use in the DHCP range, set to
//          IPAddress((uint32_t)0) to use ipAddress + 1
//   dnsAddress: IP address to use for DNS lookup (translate name to IP
//          address), none = IPAddress((uint32_t)0)
//   gatewayAddress: IP address of the gateway to a larger network (internet?),
//          none = IPAddress((uint32_t)0)
// Outputs:
//   Returns true if the soft AP was successfully configured.
bool r4aWifiSoftApConfiguration(IPAddress ipAddress,
                             IPAddress subnetMask,
                             IPAddress firstDhcpAddress,
                             IPAddress dnsAddress,
                             IPAddress gatewayAddress)
{
    bool success;

    r4aWiFi._apIpAddress = ipAddress;
    r4aWiFi._apSubnetMask = subnetMask;
    r4aWiFi._apFirstDhcpAddress = firstDhcpAddress;
    r4aWiFi._apDnsAddress = dnsAddress;
    r4aWiFi._apGatewayAddress = gatewayAddress;

    // Restart the soft AP if necessary
    success = true;
    if (r4aWifiSoftApOnline)
    {
        success = r4aWifiEnable(r4aWifiEspNowRunning, false, r4aWifiStationRunning, __FILE__, __LINE__);
        if (success)
            success = r4aWifiEnable(r4aWifiEspNowRunning, true, r4aWifiStationRunning, __FILE__, __LINE__);
    }
    return success;
}

//*********************************************************************
// Display the soft AP configuration
// Inputs:
//   display: Address of a Print object
void r4aWifiSoftApConfigurationDisplay(Print * display)
{
    display->printf("Soft AP configuration:\r\n");
    display->printf("    %s: IP Address\r\n", r4aWiFi._apIpAddress.toString().c_str());
    display->printf("    %s: Subnet mask\r\n", r4aWiFi._apSubnetMask.toString().c_str());
    if ((uint32_t)r4aWiFi._apFirstDhcpAddress)
        display->printf("    %s: First DHCP address\r\n", r4aWiFi._apFirstDhcpAddress.toString().c_str());
    if ((uint32_t)r4aWiFi._apDnsAddress)
        display->printf("    %s: DNS address\r\n", r4aWiFi._apDnsAddress.toString().c_str());
    if ((uint32_t)r4aWiFi._apGatewayAddress)
        display->printf("    %s: Gateway address\r\n", r4aWiFi._apGatewayAddress.toString().c_str());
}

//*********************************************************************
// Handle the soft AP events
void r4aWifiSoftApEventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    // Handle the event
    switch (event)
    {
	default:
		break;

    case ARDUINO_EVENT_WIFI_AP_STOP:
        // Mark the soft AP as offline
        if (r4aWifiDebug && r4aWifiSoftApOnline)
            Serial.printf("AP: Offline\r\n");
        r4aWiFi._started = r4aWiFi._started & ~WIFI_AP_ONLINE;
        if (r4aWifiDebug && r4aWifiVerbose)
            Serial.printf("_started: 0x%08lx\r\n", r4aWiFi._started);
        break;
    }
}

//*********************************************************************
// Get the soft AP IP address
// Returns the soft IP address
IPAddress r4aWifiSoftApIpAddress()
{
    if (r4aWifiSoftApOnline)
        return r4aWiFi._apIpAddress;
    return IPAddress((uint32_t)0);
}

//*********************************************************************
// Turn off WiFi soft AP mode
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns the status of WiFi soft AP stop
bool r4aWifiSoftApOff(const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (r4aWifiDebug)
        Serial.printf("wifiSoftApOff called in %s at line %ld\r\n",
                      fileName, lineNumber);

    return r4aWifiEnable(r4aWifiEspNowRunning, false, r4aWifiStationRunning, __FILE__, __LINE__);
}

//*********************************************************************
// Turn on WiFi soft AP mode
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns the status of WiFi soft AP start
bool r4aWifiSoftApOn(const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (r4aWifiDebug)
        Serial.printf("wifiSoftApOn called in %s at line %ld\r\n",
                      fileName, lineNumber);

    return r4aWifiEnable(r4aWifiEspNowRunning, true, r4aWifiStationRunning, __FILE__, __LINE__);
}

//*********************************************************************
// Set the soft AP host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiSoftApSetHostName(const char * hostName)
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
        if (r4aWifiDebug)
            Serial.printf("WiFI setting AP host name\r\n");
        nameSet = WiFi.AP.setHostname(hostName);
        if (!nameSet)
        {
            Serial.printf("ERROR: Failed to set the Wifi AP host name!\r\n");
            break;
        }
        if (r4aWifiDebug)
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
bool r4aWifiSoftApSetIpAddress(const char * ipAddress,
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
    if (r4aWifiDebug)
        r4aWifiSoftApConfigurationDisplay(&Serial);

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
// Set the soft AP SSID and password
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiSoftApSetSsidPassword(const char * ssid,
                                  const char * password,
                                  R4A_WIFI_CHANNEL_t channel)
{
    bool created;

    // Set the WiFi soft AP SSID and password
    if (r4aWifiDebug)
        Serial.printf("WiFi AP: Attempting to set AP SSID and password\r\n");
    if (r4aWifiDebug && r4aWifiVerbose)
    {
        r4aEsp32DisplayCharPointer("ssid", ssid);
        r4aEsp32DisplayCharPointer("password", password);
        Serial.printf("r4aWifiChannel: %d\r\n", channel);
    }

    // Attempt to set the soft AP SSID, password and channel
    created = WiFi.AP.create(ssid, password, channel);
    if (!created)
        Serial.printf("ERROR: Failed to set soft AP SSID and Password!\r\n");
    else if (r4aWifiDebug)
        Serial.printf("WiFi AP: SSID: %s%s%s\r\n", ssid,
                     password ? ", Password: " : "", password ? password : "");
    return created;
}

//*********************************************************************
// Connect the station to a remote AP
// Return true if the connection was successful and false upon failure.
bool r4aWifiStationConnectAP()
{
    bool connected;

    do
    {
        // Connect to the remote AP
        if (r4aWifiDebug)
            Serial.printf("WiFi connecting to %s on channel %d with %s authorization\r\n",
                         r4aWiFi._staRemoteApSsid,
                         r4aWifiChannel,
                         (r4aWiFi._staAuthType < WIFI_AUTH_MAX) ? r4aWifiAuthorizationName[r4aWiFi._staAuthType] : "Unknown");
        connected = (WiFi.STA.connect(r4aWiFi._staRemoteApSsid, r4aWiFi._staRemoteApPassword, r4aWifiChannel));
        if (!connected)
        {
            if (r4aWifiDebug)
                Serial.printf("WIFI failed to connect to SSID %s with password %s\r\n",
                             r4aWiFi._staRemoteApSsid, r4aWiFi._staRemoteApPassword);
            break;
        }
        if (r4aWifiDebug)
            Serial.printf("WiFi station connected to %s on channel %d with %s authorization\r\n",
                         r4aWiFi._staRemoteApSsid,
                         r4aWifiChannel,
                         (r4aWiFi._staAuthType < WIFI_AUTH_MAX) ? r4aWifiAuthorizationName[r4aWiFi._staAuthType] : "Unknown");

        // Don't delay the next WiFi start request
        r4aWifiResetTimeout();
    } while (0);
    return connected;
}

//*********************************************************************
// Disconnect the station from an AP
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiStationDisconnect()
{
    bool disconnected;

    do
    {
        // Determine if station is connected to a remote AP
        disconnected = !r4aWiFi._staConnected;
        if (disconnected)
        {
            if (r4aWifiDebug)
                Serial.printf("Station already disconnected from remote AP\r\n");
            break;
        }

        // Disconnect from the remote AP
        if (r4aWifiDebug)
            Serial.printf("WiFI disconnecting station from remote AP\r\n");
        disconnected = WiFi.STA.disconnect();
        if (!disconnected)
        {
            Serial.printf("ERROR: Failed to disconnect WiFi from the remote AP!\r\n");
            break;
        }
        if (r4aWifiDebug)
            Serial.printf("WiFi disconnected from the remote AP\r\n");
    } while (0);
    return disconnected;
}

//*********************************************************************
// Determine if WiFi station is enabled
bool r4aWiFiStationEnabled()
{
    int authIndex;
    bool enabled;

    // Verify that at least one SSID is set
    for (authIndex = 0; authIndex < r4aWifiSsidPasswordEntries; authIndex++)
        if (r4aWifiSsidPassword[authIndex].ssid
            && *r4aWifiSsidPassword[authIndex].ssid
            && strlen(*r4aWifiSsidPassword[authIndex].ssid))
        {
            break;
        }
    enabled = (authIndex < r4aWifiSsidPasswordEntries);
    if (!enabled)
        Serial.printf("ERROR: No valid SSID in settings\r\n");
    return enabled;
}

//*********************************************************************
// Handle the WiFi station events
void r4aWifiStationEventHandler(arduino_event_id_t event, arduino_event_info_t info)
{
    IPAddress ipAddress;
    char ssid[sizeof(info.wifi_sta_connected.ssid) + 1];

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
	default:
		break;

    case ARDUINO_EVENT_WIFI_STA_START:
        WiFi.STA.macAddress((uint8_t *)r4aWiFi._staMacAddress);
        if (r4aWifiDebug)
            Serial.printf("WiFi Event: Station start: MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                         r4aWiFi._staMacAddress[0], r4aWiFi._staMacAddress[1],
                         r4aWiFi._staMacAddress[2], r4aWiFi._staMacAddress[3],
                         r4aWiFi._staMacAddress[4], r4aWiFi._staMacAddress[5]);

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_STA_STOP:
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_SCAN_DONE:
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        // The WiFi station is no longer connected to the remote AP
        if (r4aWifiDebug && r4aWiFi._staConnected)
            Serial.printf("WiFi: Station disconnected from %s\r\n",
                         r4aWiFi._staRemoteApSsid);
        r4aWiFi._staConnected = false;

        // Fall through
        //      |
        //      |
        //      V

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        // Mark the WiFi station offline
        if (r4aWiFi._started & WIFI_STA_ONLINE)
            Serial.printf("WiFi: Station offline!\r\n");
        r4aWiFi._started = r4aWiFi._started & ~WIFI_STA_ONLINE;

        // Start the reconnection timer
        if ((event == ARDUINO_EVENT_WIFI_STA_LOST_IP)
            || (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED))
            r4aWifiStationLostIp();

        // Notify user of loss of IP address
        r4aWiFi._staHasIp = false;
        r4aWiFi._staIpAddress = IPAddress((uint32_t)0);
        r4aWiFi._staIpType = 0;
        break;

    //------------------------------
    // Bring the WiFi station back online
    //------------------------------

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        r4aWiFi._staConnected = true;
        if (r4aWifiDebug)
        {
            memcpy(ssid, info.wifi_sta_connected.ssid, info.wifi_sta_connected.ssid_len);
            ssid[info.wifi_sta_connected.ssid_len] = 0;
            Serial.printf("WiFi: STA connected to %s\r\n", ssid);
        }
        break;

        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        r4aWiFi._staIpAddress = WiFi.STA.localIP();
        r4aWiFi._staIpType = (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) ? '4' : '6';

        // Display the IP address
        if (r4aWifiDebug)
            Serial.printf("WiFi: Got IPv%c address %s\r\n",
                         r4aWiFi._staIpType, r4aWiFi._staIpAddress.toString().c_str());
        break;
    }   // End of switch
}

//*********************************************************************
// Set the station's host name
// Inputs:
//   hostName: Zero terminated host name character string
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiStationHostName(const char * hostName)
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
        if (r4aWifiDebug)
            Serial.printf("WiFI setting station host name\r\n");
        nameSet = WiFi.STA.setHostname(hostName);
        if (!nameSet)
        {
            Serial.printf("ERROR: Failed to set the Wifi station host name!\r\n");
            break;
        }
        if (r4aWifiDebug)
            Serial.printf("WiFi station hostname: %s\r\n", hostName);
    } while (0);
    return nameSet;
}

//*********************************************************************
// Get the WiFi station IP address
// Returns the IP address of the WiFi station
IPAddress r4aWifiStationIpAddress()
{
    if (r4aWiFi._staHasIp)
        return r4aWiFi._staIpAddress;
    return IPAddress((uint32_t)0);
}

//*********************************************************************
// Lost the IP address for the WiFi station
void r4aWifiStationLostIp()
{
    if (r4aWifiDebug && r4aWiFi._staHasIp)
        Serial.printf("WiFi station lost IPv%c address %s\r\n",
                     r4aWiFi._staIpType, r4aWiFi._staIpAddress.toString().c_str());
    r4aWiFi._staHasIp = false;
    r4aWifiReconnectRequest = true;
}

//*********************************************************************
// Stop the WiFi station
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiStationOff(const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (r4aWifiDebug)
        Serial.printf("wifiStationOff called in %s at line %ld\r\n",
                      fileName, lineNumber);

    return r4aWifiEnable(r4aWifiEspNowRunning, r4aWifiSoftApRunning, false, __FILE__, __LINE__);
}

//*********************************************************************
// Start the WiFi station
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiStationOn(const char * fileName, uint32_t lineNumber)
{
    // Display the call
    if (r4aWifiDebug)
        Serial.printf("wifiStationOn called in %s at line %ld\r\n",
                      fileName, lineNumber);

    return r4aWifiEnable(r4aWifiEspNowRunning, r4aWifiSoftApRunning, true, __FILE__, __LINE__);
}

//*********************************************************************
// Handle WiFi station reconnection requests
bool r4aWifiStationReconnectionRequest()
{
    bool connected;
    int minutes;
    int seconds;

    // Restart delay
    connected = false;
    if ((millis() - r4aWifiReconnectionTimer) < r4aWifiStartTimeout)
        return connected;
    r4aWifiReconnectionTimer = millis();

    // Check for a reconnection request
    if (r4aWifiReconnectRequest)
    {
        r4aWifiReconnectRequest = false;
        if (r4aWifiDebug)
            Serial.printf("WiFi: Attempting WiFi restart\r\n");
        r4aWifiClearStarted(WIFI_STA_RECONNECT);
    }

    // Attempt to start WiFi station
    if (r4aWifiStationOn(__FILE__, __LINE__))
    {
        // Successfully connected to a remote AP
        connected = true;
        if (r4aWifiDebug)
            Serial.printf("WiFi: WiFi station successfully started\r\n");
        r4aWifiFailedConnectionAttempts = 0;
    }
    else
    {
        // Failed to connect to a remote AP
        if (r4aWifiDebug)
            Serial.printf("WiFi: WiFi station failed to start!\r\n");

        // Account for this connection attempt
        r4aWifiFailedConnectionAttempts++;

        // Increase the timeout
        r4aWifiStartTimeout <<= 1;
        if (!r4aWifiStartTimeout)
            r4aWifiStartTimeout = WIFI_MIN_TIMEOUT;
        else if (r4aWifiStartTimeout > WIFI_MAX_TIMEOUT)
            r4aWifiStartTimeout = WIFI_MAX_TIMEOUT;

        // Display the delay
        seconds = r4aWifiStartTimeout / R4A_MILLISECONDS_IN_A_SECOND;
        minutes = seconds / R4A_SECONDS_IN_A_MINUTE;
        seconds -= minutes * R4A_SECONDS_IN_A_MINUTE;
        if (r4aWifiDebug)
            Serial.printf("WiFi: Delaying %2d:%02d before restarting WiFi\r\n", minutes, seconds);
    }
    if (r4aWifiSoftApSsid)
        r4aWifiSoftApOn(__FILE__, __LINE__);
    return connected;
}

//*********************************************************************
// Scan the WiFi network for remote APs
// Inputs:
//   channel: Channel number for the scan, zero (0) scan all channels
// Outputs:
//   Returns the number of access points
int16_t r4aWifiStationScanForAPs(R4A_WIFI_CHANNEL_t channel)
{
    int16_t apCount;

    do
    {
        // Determine if the WiFi scan is already running
        apCount = 0;
        if (r4aWiFi._scanRunning)
        {
            if (r4aWifiDebug)
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
        if (r4aWifiDebug && r4aWifiVerbose)
            Serial.printf("apCount: %d\r\n", apCount);
        if (apCount < 0)
        {
            Serial.printf("ERROR: WiFi scan failed, status: %d!\r\n", apCount);
            break;
        }
        r4aWiFi._apCount = apCount;
        if (r4aWifiDebug)
            Serial.printf("WiFi scan complete, found %d remote APs\r\n", r4aWiFi._apCount);
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
R4A_WIFI_CHANNEL_t r4aWifiStationSelectAP(uint8_t apCount, bool list)
{
    int ap;
    R4A_WIFI_CHANNEL_t apChannel;
    bool apFound;
    int authIndex;
    R4A_WIFI_CHANNEL_t channel;
    const char * ssid;
    String ssidString;
    int type;

    // Verify that an AP was found
    if (apCount == 0)
        return 0;

    // Print the header
    //                                    1                 1         2         3
    //             1234   1234   123456789012345   12345678901234567890123456789012
    if (r4aWifiDebug || list)
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
            for (authIndex = 0; authIndex < r4aWifiSsidPasswordEntries; authIndex++)
            {
                // Determine if this authorization matches the AP's SSID
                if (strlen(*r4aWifiSsidPassword[authIndex].ssid)
                    && (strcmp(ssid, *r4aWifiSsidPassword[authIndex].ssid) == 0)
                    && ((type == WIFI_AUTH_OPEN)
                        || (strlen(*r4aWifiSsidPassword[authIndex].password))))
                {
                    if (r4aWifiDebug)
                        Serial.printf("WiFi: Found remote AP: %s\r\n", ssid);

                    // A match was found, save it and stop looking
                    r4aWiFi._staRemoteApSsid = *r4aWifiSsidPassword[authIndex].ssid;
                    r4aWiFi._staRemoteApPassword = *r4aWifiSsidPassword[authIndex].password;
                    apChannel = channel;
                    r4aWiFi._staAuthType = type;
                    apFound = true;
                    break;
                }
            }

            // Check for done
            if (apFound && (!(r4aWifiDebug | list)))
                break;
        }

        // Display the list of APs
        if (r4aWifiDebug || list)
            Serial.printf("%4ld   %4d   %s   %s\r\n",
                         WiFi.RSSI(ap),
                         channel,
                         (type < WIFI_AUTH_MAX) ? r4aWifiAuthorizationName[type] : "Unknown",
                         ssid);
    }

    // Return the channel number
    return apChannel;
}

//*********************************************************************
// Get the SSID of the remote AP
const char * r4aWifiStationSsid()
{
    if (r4aWiFi._staHasIp)
        return r4aWiFi._staRemoteApSsid;
    else
        return "";
}

//*********************************************************************
// Stop WiFi and release all resources
void r4aWifiStopAll()
{
    // Stop the Wifi layer
    r4aWifiEnable(false, false, false, __FILE__, __LINE__);

    // Display the heap state
    if (r4aWifiDebug)
        r4aEsp32HeapDisplay();
}

//*********************************************************************
// Stop and start WiFi components
// Inputs:
//   stopping: WiFi components that need to be stopped
//   starting: WiFi components that neet to be started
// Outputs:
//   Returns true if the modes were successfully configured
bool r4aWifiStopStart(R4A_WIFI_ACTION_t stopping, R4A_WIFI_ACTION_t starting)
{
    const R4A_WIFI_ACTION_t allOnline = WIFI_AP_ONLINE | WIFI_EN_ESP_NOW_ONLINE | WIFI_STA_ONLINE;
    R4A_WIFI_CHANNEL_t channel;
    bool defaultChannel;
    R4A_WIFI_ACTION_t expected;
    R4A_WIFI_ACTION_t mask;
    R4A_WIFI_ACTION_t notStarted;
    R4A_WIFI_ACTION_t restarting;
    bool restartWiFiStation;
    R4A_WIFI_ACTION_t startingNow;
    R4A_WIFI_ACTION_t stillRunning;

    // Determine the next actions
    notStarted = 0;
    restartWiFiStation = false;

    // Display the parameters
    if (r4aWifiDebug && r4aWifiVerbose)
    {
        Serial.printf("WiFi: wifiStopStart called\r\n");
        Serial.printf("stopping: 0x%08lx\r\n", stopping);
        Serial.printf("starting: 0x%08lx\r\n", starting);
        r4aEsp32HeapDisplay();
    }

    //****************************************
    // Select the channel
    //
    // The priority order for the channel is:
    //      1. Active channel (not using default channel)
    //      2. r4aWiFi._stationChannel
    //      3. Remote AP channel determined by scan
    //      4. r4aWiFi._espNowChannel
    //      5. r4aWiFi._apChannel
    //      6. Channel 1
    //****************************************

    // Determine if there is an active channel
    defaultChannel = r4aWiFi._usingDefaultChannel;
    r4aWiFi._usingDefaultChannel = false;
    if ((allOnline & r4aWiFi._started & ~stopping) && r4aWifiChannel && !defaultChannel)
    {
        // Continue to use the active channel
        channel = r4aWifiChannel;
        if (r4aWifiDebug && r4aWifiVerbose)
            Serial.printf("channel: %d, active channel\r\n", channel);
    }

    // Use the station channel if specified
    else if (r4aWiFi._stationChannel && (starting & WIFI_STA_ONLINE))
    {
        channel = r4aWiFi._stationChannel;
        if (r4aWifiDebug && r4aWifiVerbose)
            Serial.printf("channel: %d, WiFi station channel\r\n", channel);
    }

    // Determine if a scan for remote APs is needed
    else if (starting & WIFI_STA_START_SCAN)
    {
        channel = 0;
        if (r4aWifiDebug && r4aWifiVerbose)
            Serial.printf("channel: Determine by remote AP scan\r\n");

        // Restart ESP-NOW if necessary
        if (r4aWifiEspNowRunning)
            stopping |= WIFI_START_ESP_NOW;

        // Restart soft AP if necessary
        if (r4aWifiSoftApRunning)
            stopping |= WIFI_START_SOFT_AP;
    }

    // Determine if the ESP-NOW channel was specified
    else if (r4aWiFi._espNowChannel & ((starting | r4aWiFi._started) & WIFI_EN_ESP_NOW_ONLINE))
    {
        channel = r4aWiFi._espNowChannel;
        if (r4aWifiDebug && r4aWifiVerbose)
            Serial.printf("channel: %d, ESP-NOW channel\r\n", channel);
    }

    // Determine if the AP channel was specified
    else if (r4aWiFi._apChannel && ((starting | r4aWiFi._started) & WIFI_AP_ONLINE))
    {
        channel = r4aWiFi._apChannel;
        if (r4aWifiDebug && r4aWifiVerbose)
            Serial.printf("channel: %d, soft AP channel\r\n", channel);
    }

    // No channel specified and scan not being done, use the default channel
    else
    {
        channel = WIFI_DEFAULT_CHANNEL;
        r4aWiFi._usingDefaultChannel = true;
        if (r4aWifiDebug && r4aWifiVerbose)
            Serial.printf("channel: %d, default channel\r\n", channel);
    }

    //****************************************
    // Perform some optimizations
    //****************************************

    // Only stop the started components
    stopping &= r4aWiFi._started;

    // Determine the components that are being started
    expected = starting & allOnline;

    // Determine if mDNS is being stopped or if mDNS needs to restart
    // due to a interface change
    mask = WIFI_AP_START_MDNS | WIFI_STA_START_MDNS;
    if ((mask & stopping)|| (r4aWiFi._started & WIFI_START_MDNS))
        stopping |= WIFI_START_MDNS;

    // Start mDNS if necessary
    if (mask & starting)
        starting |= WIFI_START_MDNS;

    // Determine which components are being restarted
    restarting = r4aWiFi._started & stopping & starting;

    // Only start or stop the WiFi event handler
    if (restarting & WIFI_START_EVENT_HANDLER)
    {
        restarting &= ~WIFI_START_EVENT_HANDLER;
        stopping &= ~WIFI_START_EVENT_HANDLER;
        starting &= ~WIFI_START_EVENT_HANDLER;
    }

    // Display the values
    if (r4aWifiDebug && r4aWifiVerbose)
    {
        Serial.printf("0x%08lx: _started\r\n", r4aWiFi._started);
        Serial.printf("0x%08lx: stopping\r\n", stopping);
        Serial.printf("0x%08lx: starting\r\n", starting);
        Serial.printf("0x%08lx: restarting\r\n", restarting);
        Serial.printf("0x%08lx: expected\r\n", expected);
    }

    // Don't start components that are already running and are not being
    // stopped
    starting &= ~(r4aWiFi._started & ~stopping);

    // Display the starting and stopping
    if (r4aWifiDebug)
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

        if (r4aWifiDebug && r4aWifiVerbose && stopping)
            r4aWifiDisplayComponents("Stopping", stopping);

        //****************************************
        // Stop the ESP-NOW components
        //****************************************

#ifdef  COMPILE_ESPNOW
        // Mark the ESP-NOW offline
        if (stopping & WIFI_EN_ESP_NOW_ONLINE)
        {
            r4aWifiEspNowOnline = false;
            if (r4aWifiDebug)
                Serial.printf("WiFi: ESP-NOW offline!\r\n");
            r4aWiFi._started = r4aWiFi._started & ~WIFI_EN_ESP_NOW_ONLINE;
        }

        // Stop the ESP-NOW layer
        if (stopping & WIFI_EN_START_ESP_NOW)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("WiFi: Stopping ESP-NOW\r\n");
            if (!r4aWiFiEspNowStop())
            {
                Serial.printf("ERROR: Failed to stop ESP-NOW!\r\n");
                stillRunning = r4aWiFi._started;
                break;
            }
            r4aWiFi._started = r4aWiFi._started & ~WIFI_EN_START_ESP_NOW;
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("WiFi: ESP-NOW stopped\r\n");
        }

        // Stop promiscuous mode
        if (stopping & WIFI_EN_SET_PROMISCUOUS_MODE)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Calling esp_wifi_set_promiscuous\r\n");
            status = esp_wifi_set_promiscuous(false);
            if (status != ESP_OK)
            {
                Serial.printf("ERROR: Failed to stop WiFi promiscuous mode, status: %d\r\n", status);
                stillRunning = r4aWiFi._started;
                break;
            }
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("WiFi: Promiscuous mode stopped\r\n");
            r4aWiFi._started = r4aWiFi._started & ~WIFI_EN_SET_PROMISCUOUS_MODE;
        }

        // Handle WiFi set channel
        if (stopping & WIFI_EN_SET_CHANNEL)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_EN_SET_CHANNEL;

        // Stop the long range radio protocols
        if (stopping & WIFI_EN_SET_PROTOCOLS)
        {
            stillRunning = r4aWiFi._started;
            if (!r4aWifiSetWiFiProtocols(WIFI_IF_STA, true, false))
                break;
            r4aWiFi._started = r4aWiFi._started & ~WIFI_EN_SET_PROTOCOLS;
        }
#endif  // COMPILE_ESPNOW

        //****************************************
        // Stop the WiFi station components
        //****************************************

        // Mark the WiFi station offline
        if (stopping & WIFI_STA_ONLINE)
        {
            r4aWifiStationOnline = false;
            if (r4aWiFi._started & WIFI_STA_ONLINE)
                Serial.printf("WiFi: Station offline!\r\n");
            r4aWiFi._started = r4aWiFi._started & ~WIFI_STA_ONLINE;
        }

        // Stop mDNS if necessary
        if (stopping & WIFI_START_MDNS)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("WiFi: Stopping mDNS\r\n");
            r4aWifiMdnsStop();
            r4aWiFi._started = r4aWiFi._started & ~(WIFI_AP_START_MDNS
                                                    | WIFI_STA_START_MDNS);
        }

        // Disconnect from the remote AP
        if (stopping & WIFI_STA_CONNECT_TO_REMOTE_AP)
        {
            stillRunning = r4aWiFi._started;
            if (!r4aWifiStationDisconnect())
                break;
            r4aWiFi._started = r4aWiFi._started & ~WIFI_STA_CONNECT_TO_REMOTE_AP;
        }

        // Handle auto reconnect
        if (stopping & WIFI_STA_DISABLE_AUTO_RECONNECT)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_STA_DISABLE_AUTO_RECONNECT;

        // Handle WiFi station host name
        if (stopping & WIFI_STA_SET_HOST_NAME)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_STA_SET_HOST_NAME;

        // Handle WiFi select channel
        if (stopping & WIFI_SELECT_CHANNEL)
            r4aWiFi._started = r4aWiFi._started & ~(stopping & WIFI_SELECT_CHANNEL);

        // Handle WiFi station select remote AP
        if (stopping & WIFI_STA_SELECT_REMOTE_AP)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_STA_SELECT_REMOTE_AP;

        // Handle WiFi station scan
        if (stopping & WIFI_STA_START_SCAN)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_STA_START_SCAN;

        // Stop the WiFi station radio protocols
        if (stopping & WIFI_STA_SET_PROTOCOLS)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_STA_SET_PROTOCOLS;

        // Stop station mode
        if (stopping & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE))
        {
            // Determine which bits to clear
            mask = ~(stopping & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE));

            // Stop WiFi station if users are gone
            if (!(r4aWiFi._started & mask & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE)))
            {
                stillRunning = r4aWiFi._started;
                if (!r4aWifiSetWiFiMode(WIFI_MODE_STA, WIFI_MODE_STA))
                    break;
            }

            // Remove this WiFi station user
            r4aWiFi._started = r4aWiFi._started & mask;
        }

        //****************************************
        // Stop the soft AP components
        //****************************************

        // Stop soft AP mode
        // Mark the soft AP offline
        if (stopping & WIFI_AP_ONLINE)
        {
            if (r4aWifiSoftApOnline)
                Serial.printf("WiFi: Soft AP offline!\r\n");
            r4aWiFi._started = r4aWiFi._started & ~WIFI_AP_ONLINE;
            r4aWifiSoftApOnline = false;
        }

        // Stop the DNS server
        if (stopping & r4aWiFi._started & WIFI_AP_START_DNS_SERVER)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Calling dnsServer.stop for soft AP\r\n");
            r4aWifiDnsServer.stop();
            r4aWiFi._started = r4aWiFi._started & ~WIFI_AP_START_DNS_SERVER;
        }

        // Handle the soft AP host name
        if (stopping & WIFI_AP_SET_HOST_NAME)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_AP_SET_HOST_NAME;

        // Stop soft AP mode
        stillRunning = r4aWiFi._started;
        if (stopping & WIFI_AP_SET_MODE)
        {
            if (!r4aWifiSetWiFiMode(WIFI_MODE_AP, WIFI_MODE_AP))
                break;
            r4aWiFi._started = r4aWiFi._started & ~WIFI_AP_SET_MODE;
        }

        // Disable the radio protocols for soft AP
        if (stopping & WIFI_AP_SET_PROTOCOLS)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_AP_SET_PROTOCOLS;

        // Stop using the soft AP IP address
        if (stopping & WIFI_AP_SET_IP_ADDR)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_AP_SET_IP_ADDR;

        // Stop use of SSID and password
        if (stopping & WIFI_AP_SET_SSID_PASSWORD)
            r4aWiFi._started = r4aWiFi._started & ~WIFI_AP_SET_SSID_PASSWORD;

        stillRunning = r4aWiFi._started;

        //****************************************
        // Stop the common components
        //****************************************

        // Stop the event handler
        if (stopping & WIFI_START_EVENT_HANDLER)
        {
            if (r4aWifiDebug)
                Serial.printf("Stopping the WiFi event handler\r\n");
            Network.removeEvent(r4aWifiEventHandle);
            r4aWiFi._started = r4aWiFi._started & ~WIFI_START_EVENT_HANDLER;
        }

        //****************************************
        // Channel reset
        //****************************************

        // Reset the channel if all components are stopped
        if ((r4aWifiSoftApOnline == false) && (r4aWifiStationOnline == false))
        {
            r4aWifiChannel = 0;
            r4aWiFi._usingDefaultChannel = true;
        }

        //****************************************
        // Delay to allow mDNS to shutdown and restart properly
        //****************************************

        delay(100);

        //****************************************
        // Display the items already started and being started
        //****************************************

        if (r4aWifiDebug && r4aWifiVerbose && r4aWiFi._started)
            r4aWifiDisplayComponents("Started", r4aWiFi._started);

        if (r4aWifiDebug && r4aWifiVerbose && startingNow)
            r4aWifiDisplayComponents("Starting", startingNow);

        //****************************************
        // Start the common components
        //****************************************

        // Start the WiFi event handler
        if (starting & WIFI_START_EVENT_HANDLER)
        {
            if (!(r4aWiFi._started & WIFI_START_EVENT_HANDLER))
            {
                if (r4aWifiDebug && r4aWifiVerbose)
                    Serial.printf("Starting the WiFi event handler\r\n");

                // Establish the WiFi event handler
                r4aWifiEventHandle = Network.onEvent(r4aWifiEventHandler);
            }
            r4aWiFi._started = r4aWiFi._started | WIFI_START_EVENT_HANDLER;
        }

        //****************************************
        // Start the radio operations
        //****************************************

        // Start the soft AP mode
        if (starting & WIFI_AP_SET_MODE)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Setting soft AP mode\r\n");

            if (!r4aWifiSetWiFiMode(WIFI_MODE_AP, 0))
                break;
            r4aWiFi._started = r4aWiFi._started | WIFI_AP_SET_MODE;
        }

        // Start the station mode
        if (starting & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE))
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Setting WiFi station mode\r\n");

            if (!r4aWifiSetWiFiMode(WIFI_MODE_STA, 0))
                break;
            r4aWiFi._started = r4aWiFi._started | (starting & (WIFI_EN_SET_MODE | WIFI_STA_SET_MODE));
        }

        // Start the soft AP protocols
        if (starting & WIFI_AP_SET_PROTOCOLS)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Enabling WiFi protocols for soft AP\r\n");

            if (!r4aWifiSetWiFiProtocols(WIFI_IF_AP, true, false))
                break;
            r4aWiFi._started = r4aWiFi._started | WIFI_AP_SET_PROTOCOLS;
        }

        // Start the WiFi station radio protocols
        if (starting & (WIFI_EN_SET_PROTOCOLS | WIFI_STA_SET_PROTOCOLS))
        {
            bool lrEnable = (starting & WIFI_EN_SET_PROTOCOLS) ? true : false;
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Enabling WiFi%s for WiFi station\r\n",
                              lrEnable ? " & ESP-NOW" : "");
            if (!r4aWifiSetWiFiProtocols(WIFI_IF_STA, true, lrEnable))
                break;
            r4aWiFi._started = r4aWiFi._started | (starting & (WIFI_EN_SET_PROTOCOLS | WIFI_STA_SET_PROTOCOLS));
        }

        // Start the WiFi scan for remote APs
        if (starting & WIFI_STA_START_SCAN)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("channel: %d\r\n", channel);
            r4aWiFi._started = r4aWiFi._started | WIFI_STA_START_SCAN;

            // Determine if WiFi scan failed, stop WiFi station startup
            if (r4aWifiStationScanForAPs(channel) < 0)
            {
                starting &= ~WIFI_STA_FAILED_SCAN;
                notStarted |= WIFI_STA_FAILED_SCAN;
            }
        }

        // Select an AP from the list
        if (starting & WIFI_STA_SELECT_REMOTE_AP)
        {
            channel = r4aWifiStationSelectAP(r4aWiFi._apCount, false);
            r4aWiFi._started = r4aWiFi._started | WIFI_STA_SELECT_REMOTE_AP;
            if (channel == 0)
            {
                if (r4aWifiChannel)
                    Serial.printf("WiFi STA: No matching remote AP found on channel %d!\r\n", r4aWifiChannel);
                else
                    Serial.printf("WiFi STA: No matching remote AP found!\r\n");

                // Stop bringing up WiFi station
                starting &= ~WIFI_STA_NO_REMOTE_AP;
                notStarted |= WIFI_STA_FAILED_SCAN;
            }
        }

        // Finish the channel selection
        if (starting & WIFI_SELECT_CHANNEL)
        {
            r4aWiFi._started = r4aWiFi._started | (starting & WIFI_SELECT_CHANNEL);
            if (channel & (starting & WIFI_STA_START_SCAN))
            {
                if (r4aWifiDebug && r4aWifiVerbose)
                    Serial.printf("Channel: %d, determined by remote AP scan\r\n",
                                 channel);
            }

            // Use the default channel if necessary
            if (!channel)
                channel = WIFI_DEFAULT_CHANNEL;
            else
                r4aWiFi._usingDefaultChannel = false;
            r4aWifiChannel = channel;

            // Display the selected channel
            if (r4aWifiDebug)
                Serial.printf("Channel: %d selected\r\n", r4aWifiChannel);
        }

        //****************************************
        // Start the soft AP components
        //****************************************

        // Set the soft AP SSID and password
        if (starting & WIFI_AP_SET_SSID_PASSWORD)
        {
            if (!r4aWifiSoftApSetSsidPassword(r4aWifiSoftApSsid, r4aWifiSoftApPassword, r4aWifiChannel))
                break;
            r4aWiFi._started = r4aWiFi._started | WIFI_AP_SET_SSID_PASSWORD;
        }

        // Set the soft AP subnet mask, IP, gateway, DNS, and first DHCP addresses
        if (starting & WIFI_AP_SET_IP_ADDR)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Setting WiFi soft AP IP address and subnet mask\r\n");

            if (!r4aWifiSoftApSetIpAddress(r4aWiFi._apIpAddress.toString().c_str(),
                                           r4aWiFi._apSubnetMask.toString().c_str(),
                                           r4aWiFi._apGatewayAddress.toString().c_str(),
                                           r4aWiFi._apDnsAddress.toString().c_str(),
                                           r4aWiFi._apFirstDhcpAddress.toString().c_str()))
            {
                break;
            }
            r4aWiFi._started = r4aWiFi._started | WIFI_AP_SET_IP_ADDR;
        }

        // Get the soft AP MAC address
        WiFi.AP.macAddress(r4aWiFi._apMacAddress);

        // Set the soft AP host name
        if (starting & WIFI_AP_SET_HOST_NAME)
        {
            if (r4aWifiHostName)
            {
                // Display the host name
                if (r4aWifiDebug && r4aWifiVerbose)
                    Serial.printf("Host name: %s\r\n", r4aWifiHostName);

                // Set the host name
                if (!r4aWifiSoftApSetHostName(r4aWifiHostName))
                    break;
            }
            r4aWiFi._started = r4aWiFi._started | WIFI_AP_SET_HOST_NAME;
        }

        // Start the DNS server
        if (starting & WIFI_AP_START_DNS_SERVER)
        {
            if (r4aWifiDebug)
                Serial.printf("Starting DNS on soft AP\r\n");
            if (r4aWifiDnsServer.start(53, "*", WiFi.softAPIP()) == false)
            {
                Serial.printf("ERROR: Failed to start DNS Server for soft AP\r\n");
                break;
            }
            if (r4aWifiDebug)
                Serial.printf("DNS Server started for soft AP\r\n");
            r4aWiFi._started = r4aWiFi._started | WIFI_AP_START_DNS_SERVER;
        }

        // Mark the soft AP as online
        if (starting & WIFI_AP_ONLINE)
        {
            r4aWiFi._started = r4aWiFi._started | WIFI_AP_ONLINE;
            r4aWifiSoftApOnline = true;

            // Display the soft AP status
            Serial.printf("WiFi: Soft AP online, SSID: %s (%s)%s%s\r\n",
                         r4aWifiSoftApSsid,
                         r4aWiFi._apIpAddress.toString().c_str(),
                         r4aWifiSoftApPassword ? ", Password: " : "",
                         r4aWifiSoftApPassword ? r4aWifiSoftApPassword : "");
        }

        //****************************************
        // Start the WiFi station components
        //****************************************

        restartWiFiStation = true;

        // Set the host name
        if (starting & WIFI_STA_SET_HOST_NAME)
        {
            if (r4aWifiHostName)
            {
                // Display the host name
                if (r4aWifiDebug && r4aWifiVerbose)
                    Serial.printf("Host name: %s\r\n", r4aWifiHostName);

                // Set the host name
                if (!r4aWifiStationHostName(r4aWifiHostName))
                    break;
            }
            r4aWiFi._started = r4aWiFi._started | WIFI_STA_SET_HOST_NAME;
        }

        // Disable auto reconnect
        if (starting & WIFI_STA_DISABLE_AUTO_RECONNECT)
        {
            if (!WiFi.setAutoReconnect(false))
            {
                Serial.printf("ERROR: Failed to disable auto-reconnect!\r\n");
                break;
            }
            r4aWiFi._started = r4aWiFi._started | WIFI_STA_DISABLE_AUTO_RECONNECT;
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("WiFi auto-reconnect disabled\r\n");
        }

        // Connect to the remote AP
        if (starting & WIFI_STA_CONNECT_TO_REMOTE_AP)
        {
            IPAddress ipAddress;
            uint32_t timer;

            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Connecting to the remote AP\r\n");

            if (!r4aWifiStationConnectAP())
                break;
            r4aWiFi._started = r4aWiFi._started | WIFI_STA_CONNECT_TO_REMOTE_AP;

            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Waiting for an IP address\r\n");

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

            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Waiting for a MAC address\r\n");

            // Wait for the station MAC address to be set
            while (!r4aWiFi._staMacAddress[0])
                delay(1);

            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("    MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                              r4aWiFi._staMacAddress[0], r4aWiFi._staMacAddress[1],
                              r4aWiFi._staMacAddress[2], r4aWiFi._staMacAddress[3],
                              r4aWiFi._staMacAddress[4], r4aWiFi._staMacAddress[5]);

            // Save the IP address
            r4aWiFi._staHasIp = true;
            r4aWiFi._staIpType = (r4aWiFi._staIpAddress.type() == IPv4) ? '4' : '6';
        }

        // Mark the station online
        if (starting & WIFI_STA_ONLINE)
        {
            restartWiFiStation = false;
            r4aWiFi._started = r4aWiFi._started | WIFI_STA_ONLINE;
            Serial.printf("WiFi: Station online (%s: %s)\r\n",
                         r4aWiFi._staRemoteApSsid, r4aWiFi._staIpAddress.toString().c_str());
            r4aWifiStationOnline = true;
        }

        //****************************************
        // Start the ESP-NOW components
        //****************************************

#ifdef  COMPILE_ESPNOW
        // Select the ESP-NOW channel
        if (starting & WIFI_EN_SET_CHANNEL)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Calling esp_wifi_get_channel\r\n");
            status = esp_wifi_get_channel(&primaryChannel, &secondaryChannel);
            if (status != ESP_OK)
            {
                Serial.printf("ERROR: Failed to get the WiFi channels, status: %d\r\n", status);
                break;
            }
            if (r4aWifiDebug && r4aWifiVerbose)
            {
                Serial.printf("primaryChannel: %d\r\n", primaryChannel);
                Serial.printf("secondaryChannel: %d (%s)\r\n", secondaryChannel,
                             (secondaryChannel == WIFI_SECOND_CHAN_NONE) ? "None"
                             : ((secondaryChannel == WIFI_SECOND_CHAN_ABOVE) ? "Above"
                             : "Below"));
            }

            // Set the ESP-NOW channel
            if (primaryChannel != r4aWifiChannel)
            {
                if (r4aWifiDebug && r4aWifiVerbose)
                    Serial.printf("Calling esp_wifi_set_channel\r\n");
                status = esp_wifi_set_channel(primaryChannel, secondaryChannel);
                if (status != ESP_OK)
                {
                    Serial.printf("ERROR: Failed to set WiFi primary channel to %d, status: %d\r\n", primaryChannel, status);
                    break;
                }
                if (r4aWifiDebug)
                    Serial.printf("WiFi: Set channel %d\r\n", primaryChannel);
            }
            r4aWiFi._started = r4aWiFi._started | WIFI_EN_SET_CHANNEL;
        }

        // Set promiscuous mode
        if (starting & WIFI_EN_SET_PROMISCUOUS_MODE)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Calling esp_wifi_set_promiscuous\r\n");
            status = esp_wifi_set_promiscuous(true);
            if (status != ESP_OK)
            {
                Serial.printf("ERROR: Failed to set WiFi promiscuous mode, status: %d\r\n", status);
                break;
            }
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("WiFi: Enabled promiscuous mode\r\n");
            r4aWiFi._started = r4aWiFi._started | WIFI_EN_SET_PROMISCUOUS_MODE;
        }

        // Start ESP-NOW
        if (starting & WIFI_EN_START_ESP_NOW)
        {
            if (r4aWifiDebug && r4aWifiVerbose)
                Serial.printf("Calling espNowStart\r\n");
            if (!r4aWifiEspNowStart())
            {
                Serial.printf("ERROR: Failed to start ESP-NOW\r\n");
                break;
            }
            if (r4aWifiDebug)
                Serial.printf("WiFi: ESP-NOW started\r\n");
            r4aWiFi._started = r4aWiFi._started | WIFI_EN_START_ESP_NOW;
        }

        // Mark ESP-NOW online
        if (starting & WIFI_EN_ESP_NOW_ONLINE)
        {
            // Wait for the station MAC address to be set
            while (!r4aWiFi._staMacAddress[0])
                delay(1);
            r4aWifiEspNowOnline = true;

            // Display the ESP-NOW MAC address
            r4aWiFi._started = r4aWiFi._started | WIFI_EN_ESP_NOW_ONLINE;
            Serial.printf("WiFi: ESP-NOW online (%02x:%02x:%02x:%02x:%02x:%02x, channel: %d)\r\n",
                         r4aWiFi._staMacAddress[0], r4aWiFi._staMacAddress[1],
                         r4aWiFi._staMacAddress[2], r4aWiFi._staMacAddress[3],
                         r4aWiFi._staMacAddress[4], r4aWiFi._staMacAddress[5],
                         r4aWifiChannel);
        }
#endif  // COMPILE_ESPNOW

    } while (0);

    //****************************************
    // Start mDNS if necessary
    //****************************************
    if (starting & WIFI_START_MDNS)
    {
        mask = starting & (WIFI_AP_START_MDNS | WIFI_STA_START_MDNS);

        bool startForStation = r4aWiFi._staHasIp && (starting & WIFI_STA_START_MDNS);

        // Attempt to start mDNS
        bool mdnsStarted = false;
        if (startForStation && r4aWifiMdnsStart(false))
            mdnsStarted = true;
        else if (r4aWifiSoftApOnline)
            mdnsStarted = r4aWifiMdnsStart(false);
        if (mdnsStarted)
            r4aWiFi._started = r4aWiFi._started | mask;
    }

    //****************************************
    // Display the items that were not stopped
    //****************************************
    if (r4aWifiDebug && r4aWifiVerbose)
    {
        Serial.printf("0x%08lx: stopping\r\n", stopping);
        Serial.printf("0x%08lx: stillRunning\r\n", stillRunning);
    }

    // Determine which components were not stopped
    stopping &= stillRunning;
    if (r4aWifiDebug && stopping)
        r4aWifiDisplayComponents("ERROR: Items NOT stopped", stopping);

    //****************************************
    // Display the items that were not started
    //****************************************

    if (r4aWifiDebug && r4aWifiVerbose)
    {
        Serial.printf("0x%08lx: startingNow\r\n", startingNow);
        Serial.printf("0x%08lx: _started\r\n", r4aWiFi._started);
    }
    startingNow &= ~r4aWiFi._started;
    if (r4aWifiDebug &&  startingNow)
        r4aWifiDisplayComponents("ERROR: Items NOT started", startingNow);

    //****************************************
    // Display the items that were not started
    //****************************************

    if (r4aWifiDebug && r4aWifiVerbose)
    {
        Serial.printf("0x%08lx: startingNow\r\n", startingNow);
        Serial.printf("0x%08lx: _started\r\n", r4aWiFi._started);
    }

    // Clear the items that were not started
    r4aWiFi._started = r4aWiFi._started & ~notStarted;

    if (r4aWifiDebug && r4aWifiVerbose && r4aWiFi._started)
        r4aWifiDisplayComponents("Started items", r4aWiFi._started);

    // Restart WiFi if necessary
    if (restartWiFiStation)
        r4aWifiReconnectRequest = true;

    // Return the enable status
    bool enabled = ((r4aWiFi._started & allOnline) == expected);
    if (!enabled)
        Serial.printf("ERROR: wifiStopStart failed!\r\n");
    if (r4aWifiDebug && r4aWifiVerbose)
    {
        r4aEsp32HeapDisplay();
        Serial.printf("WiFi: wifiStopStart returning; %s\r\n", enabled ? "true" : "false");
    }
    return enabled;
}

//*********************************************************************
// Test the WiFi modes
// Inputs:
//   testDurationMsec: Milliseconds to run each test
void r4aWifiTest(uint32_t testDurationMsec)
{
    uint32_t currentMsec;
    bool disconnectFirst;
    static uint32_t lastScanMsec = - (1000 * 1000);
    int rand;

    // Delay the mode change until after the WiFi scan completes
    currentMsec = millis();
    if (r4aWiFi._scanRunning)
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
        r4aWifiEnable(false, false, false, __FILE__, __LINE__);
        break;

    case 1:
        Serial.printf("--------------------  %d: STA Start  -------------------\r\n", rand);
        r4aWifiEnable(false, false, true, __FILE__, __LINE__);
        break;

    case 2:
        Serial.printf("--------------------  %d: STA Disconnect  --------------\r\n", rand);
        r4aWifiStationDisconnect();
        break;

    case 4:
        Serial.printf("--------------------  %d: Soft AP Start  -------------------\r\n", rand);
        r4aWifiEnable(false, true, false, __FILE__, __LINE__);
        break;

    case 5:
        Serial.printf("--------------------  %d: Soft AP & STA Start  --------------------\r\n", rand);
        r4aWifiEnable(false, true, true, __FILE__, __LINE__);
        break;

    case 6:
        Serial.printf("--------------------  %d: Soft AP Start, STA Disconnect  -------------------\r\n", rand);
        if (disconnectFirst)
            r4aWifiStationDisconnect();
        r4aWifiEnable(false, true, false, __FILE__, __LINE__);
        if (!disconnectFirst)
            r4aWifiStationDisconnect();
        break;

    case 8:
        Serial.printf("--------------------  %d: ESP-NOW Start  --------------------\r\n", rand);
        r4aWifiEnable(true, false, false, __FILE__, __LINE__);
        break;

    case 9:
        Serial.printf("--------------------  %d: ESP-NOW & STA Start  -------------------\r\n", rand);
        r4aWifiEnable(true, false, true, __FILE__, __LINE__);
        break;

    case 0xa:
        Serial.printf("--------------------  %d: ESP-NOW Start, STA Disconnect  --------------\r\n", rand);
        if (disconnectFirst)
            r4aWifiStationDisconnect();
        r4aWifiEnable(true, false, false, __FILE__, __LINE__);
        if (!disconnectFirst)
            r4aWifiStationDisconnect();
        break;

    case 0xc:
        Serial.printf("--------------------  %d: ESP-NOW & Soft AP Start  -------------------\r\n", rand);
        r4aWifiEnable(true, true, false, __FILE__, __LINE__);
        break;

    case 0xd:
        Serial.printf("--------------------  %d: ESP-NOW, Soft AP & STA Start  --------------------\r\n", rand);
        r4aWifiEnable(true, true, true, __FILE__, __LINE__);
        break;

    case 0xe:
        Serial.printf("--------------------  %d: ESP-NOW & Soft AP Start, STA Disconnect  -------------------\r\n", rand);
        if (disconnectFirst)
            r4aWifiStationDisconnect();
        r4aWifiEnable(true, true, false, __FILE__, __LINE__);
        if (!disconnectFirst)
            r4aWifiStationDisconnect();
        break;
    }
}

//*********************************************************************
// Update the WiFi state, called from loop
void r4aWifiUpdate()
{
    // Try to bring WiFi online
    if (r4aWiFi._staEnabled && !r4aWifiStationOnline)
        r4aWifiStationReconnectionRequest();
}

//*********************************************************************
// Verify the WiFi tables
void r4aWifiVerifyTables()
{
    // Verify the authorization name table
    if (WIFI_AUTH_MAX != r4aWifiAuthorizationNameEntries)
    {
        Serial.printf("ERROR: Fix wifiAuthorizationName list to match wifi_auth_mode_t in esp_wifi_types.h!\r\n");
        r4aReportFatalError("Fix wifiAuthorizationName list to match wifi_auth_mode_t in esp_wifi_types.h!");
    }

    // Verify the start name table
    if (WIFI_MAX_START != (1 << r4aWifiStartNamesEntries))
    {
        Serial.printf("ERROR: Fix wifiStartNames list to match list of defines!\r\n");
        r4aReportFatalError("Fix wifiStartNames list to match list of defines!!");
    }
}
