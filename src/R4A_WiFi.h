/**********************************************************************
  R4A_WiFi.h

  Robots-For-All (R4A_WIFI * wifi, R4A)
  Definitions and declarations for WiFi support

  Modified from https://raw.githubusercontent.com/sparkfun/SparkFun_RTK_Everywhere_Firmware/refs/heads/main/Firmware/RTK_Everywhere/settings.h
**********************************************************************/

#ifndef __R4A_WIFI_H__
#define __R4A_WIFI_H__

#include "R4A_ESP32.h"

// Handle the WiFi event
// Inputs:
//   event: Arduino ESP32 event number found on
//      https://github.com/espressif/arduino-esp32
//      in libraries/Network/src/NetworkEvents.h
//   info: Additional data about the event
void wifiEventHandler(R4A_WIFI * wifi, arduino_event_id_t event, arduino_event_info_t info);

typedef uint8_t WIFI_CHANNEL_t;
typedef uint32_t WIFI_ACTION_t;

// Structure containing WiFi private data
typedef struct _R4A_WIFI
{
WIFI_CHANNEL_t _apChannel;  // Channel required for soft AP, zero (R4A_WIFI * wifi, 0) use wifiChannel
int16_t _apCount;           // The number or remote APs detected in the WiFi network
IPAddress _apDnsAddress;    // DNS IP address to use while translating names into IP addresses
IPAddress _apFirstDhcpAddress;  // First IP address to use for DHCP
IPAddress _apGatewayAddress;// IP address of the gateway to the larger network (R4A_WIFI * wifi, internet?)
IPAddress _apIpAddress;     // IP address of the soft AP
uint8_t _apMacAddress[6];   // MAC address of the soft AP
IPAddress _apSubnetMask;    // Subnet mask for soft AP
WIFI_CHANNEL_t _espNowChannel;  // Channel required for ESPNow, zero (R4A_WIFI * wifi, 0) use wifiChannel
volatile bool _scanRunning; // Scan running
int _staAuthType;           // Authorization type for the remote AP
bool _staConnected;         // True when station is connected
bool _staHasIp;             // True when station has IP address
IPAddress _staIpAddress;    // IP address of the station
uint8_t _staIpType;         // 4 or 6 when IP address is assigned
volatile uint8_t _staMacAddress[6]; // MAC address of the station
const char * _staRemoteApSsid;      // SSID of remote AP
const char * _staRemoteApPassword;  // Password of remote AP
volatile WIFI_ACTION_t _started;    // Components that are started and running
WIFI_CHANNEL_t _stationChannel; // Channel required for station, zero (R4A_WIFI * wifi, 0) use wifiChannel
uint32_t _timer;            // Reconnection timer
bool _usingDefaultChannel;  // Using default WiFi channel
bool _verbose;              // True causes more debug output to be displayed
} R4A_WIFI;

// Perform the WiFi initialization
// Inputs:
//   verbose: Set to true to display additional WiFi debug data
wifiBegin(R4A_WIFI * wifi, bool debug = false, bool verbose = false);

// Clear some of the started components
// Inputs:
//   components: Bitmask of components to clear
// Outputs:
//   Returns the bitmask of started components
WIFI_ACTION_t clearStarted(R4A_WIFI * wifi, WIFI_ACTION_t components);

// Attempts a connection to all provided SSIDs
// Inputs:
//    timeout: Number of milliseconds to wait for the connection
//    startAP: Set true to start AP mode, false does not change soft AP
//             status
// Outputs:
//    Returns true if successful and false upon timeout, no matching
//    SSID or other failure
bool connect(R4A_WIFI * wifi, unsigned long timeout,
             bool startAP);

// Enable or disable the WiFi modes
// Inputs:
//   enableESPNow: Enable ESP-NOW mode
//   enableSoftAP: Enable soft AP mode
//   enableStataion: Enable station mode
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if the modes were successfully configured
bool enable(R4A_WIFI * wifi, bool enableESPNow,
            bool enableSoftAP,
            bool enableStation,
            const char * fileName,
            int lineNumber);

// Get the ESP-NOW status
// Outputs:
//   Returns true when ESP-NOW is online and ready for use
bool espNowOnline(R4A_WIFI * wifi);

// Set the ESP-NOW channel
// Inputs:
//   channel: New ESP-NOW channel number
void espNowSetChannel(R4A_WIFI * wifi, WIFI_CHANNEL_t channel);

// Handle the WiFi event
// Inputs:
//   event: Arduino ESP32 event number found on
//          https://github.com/espressif/arduino-esp32
//          in libraries/Network/src/NetworkEvents.h
//   info: Additional data about the event
void eventHandler(R4A_WIFI * wifi, arduino_event_id_t event, arduino_event_info_t info);

// Get the current WiFi channel
// Outputs:
//   Returns the current WiFi channel number
WIFI_CHANNEL_t getChannel(R4A_WIFI * wifi);

// Configure the soft AP
// Inputs:
//   ipAddress: IP address of the soft AP
//   subnetMask: Subnet mask for the soft AP network
//   firstDhcpAddress: First IP address to use in the DHCP range
//   dnsAddress: IP address to use for DNS lookup (R4A_WIFI * wifi, translate name to IP address)
//   gatewayAddress: IP address of the gateway to a larger network (R4A_WIFI * wifi, internet?)
// Outputs:
//   Returns true if the soft AP was successfully configured.
bool softApConfiguration(R4A_WIFI * wifi, IPAddress ipAddress,
                         IPAddress subnetMask,
                         IPAddress firstDhcpAddress,
                         IPAddress dnsAddress,
                         IPAddress gateway);

// Display the soft AP configuration
// Inputs:
//   display: Address of a Print object
void softApConfigurationDisplay(R4A_WIFI * wifi, Print * display);

// Get the soft AP IP address
// Returns the soft IP address
IPAddress softApIpAddress(R4A_WIFI * wifi);

// Get the soft AP status
// Outputs:
//   Returns true when the soft AP is ready for use
bool softApOnline(R4A_WIFI * wifi);

// Attempt to start the soft AP mode
// Inputs:
//    forceAP: Set to true to force AP to start, false will only start
//             soft AP if settings.wifiConfigOverAP is true
// Outputs:
//    Returns true if the soft AP was started successfully and false
//    otherwise
bool startAp(R4A_WIFI * wifi, bool forceAP);

// Get the WiFi station IP address
// Returns the IP address of the WiFi station
IPAddress stationIpAddress(R4A_WIFI * wifi);

// Get the station status
// Outputs:
//   Returns true when the WiFi station is online and ready for use
bool stationOnline(R4A_WIFI * wifi);

// Get the SSID of the remote AP
const char * stationSsid(R4A_WIFI * wifi);

// Stop and start WiFi components
// Inputs:
//   stopping: WiFi components that need to be stopped
//   starting: WiFi components that neet to be started
// Outputs:
//   Returns true if the modes were successfully configured
bool stopStart(R4A_WIFI * wifi, WIFI_ACTION_t stopping, WIFI_ACTION_t starting);

// Test the WiFi modes
// Inputs:
//   testDurationMsec: Milliseconds to run each test
void test(R4A_WIFI * wifi, uint32_t testDurationMsec);

// Enable or disable verbose debug output
// Inputs:
//   enable: Set true to enable verbose debug output
// Outputs:
//   Return the previous enable value
bool verbose(R4A_WIFI * wifi, bool enable);

// Verify the WiFi tables
void verifyTables(R4A_WIFI * wifi);

#endif  // __R4A_WIFI_H__
