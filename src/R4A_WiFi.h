/**********************************************************************
  R4A_WiFi.h

  Robots-For-All (R4A)
  Definitions and declarations for WiFi support

  Modified from https://raw.githubusercontent.com/sparkfun/SparkFun_RTK_Everywhere_Firmware/refs/heads/main/Firmware/RTK_Everywhere/settings.h
**********************************************************************/

#ifndef __R4A_WIFI_H__
#define __R4A_WIFI_H__

#include "R4A_ESP32.h"

#include <WiFi.h>               // Built-in
#include <WiFiClient.h>         // Built-in
#include <WiFiMulti.h>          // Built-in, multiple WiFi AP support
#include <WiFiServer.h>         // Built-in
#include <DNSServer.h>          // Built-in

//****************************************
// Types and data structures
//****************************************

typedef uint8_t R4A_WIFI_CHANNEL_t;
typedef uint32_t R4A_WIFI_ACTION_t;

// Entry in the SSID and password table r4aSsidPassword
typedef struct _R4A_SSID_PASSWORD
{
    const char ** ssid;     // ID of access point
    const char ** password; // Password for the access point
} R4A_SSID_PASSWORD;

//****************************************
// Common WiFi support
//****************************************

// WiFi Globals - For other module direct access
extern R4A_WIFI_CHANNEL_t r4aWifiChannel; // Current WiFi channel number
extern bool r4aWifiDebug;                 // Set true to display debug output
extern const char * r4aWifiHostName;      // Host name for use by mDNS
extern bool r4aWifiVerbose;               // True causes more debug output to be displayed

// Perform the WiFi initialization
void r4aWifiBegin();

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
                   int lineNumber);

// Test the WiFi modes
// Inputs:
//   testDurationMsec: Milliseconds to run each test
void r4aWifiTest(uint32_t testDurationMsec);

// Update the WiFi state, called from loop
void r4aWifiUpdate();

// Verify the WiFi tables
void r4aWifiVerifyTables();

//****************************************
// ESP-NOW support
//****************************************

extern bool r4aWifiEspNowOnline;          // ESP-Now started successfully
extern bool r4aWifiEspNowRunning;         // False: stopped, True: starting, running, stopping

// Stop ESP-NOW
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiEspNowOff(const char * fileName, uint32_t lineNumber);

// Start ESP-NOW
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiEspNowOn(const char * fileName, uint32_t lineNumber);

// Set the ESP-NOW channel
// Inputs:
//   channel: New ESP-NOW channel number
void r4aWifiEspNowSetChannel(R4A_WIFI_CHANNEL_t channel);

//****************************************
// Soft AP support
//****************************************

extern bool r4aWifiSoftApOnline;          // WiFi soft AP started successfully
extern bool r4aWifiSoftApRunning;         // False: stopped, True: starting, running, stopping

extern const char * r4aWifiSoftApSsid;    // SSID of the soft AP
extern const char * r4aWifiSoftApPassword;// Password of the soft AP

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
                                IPAddress gateway);

// Display the soft AP configuration
// Inputs:
//   display: Address of a Print object
void r4aWifiSoftApConfigurationDisplay(Print * display);

// Turn off WiFi soft AP mode
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns the status of WiFi soft AP stop
bool r4aWifiSoftApOff(const char * fileName, uint32_t lineNumber);

// Turn on WiFi soft AP mode
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns the status of WiFi soft AP start
bool r4aWifiSoftApOn(const char * fileName, uint32_t lineNumber);

//****************************************
// Station support
//****************************************

extern uint32_t r4aWifiReconnectionTimer; // Delay before reconnection, timer running when non-zero
extern bool r4aWifiRestartRequested;      // Restart WiFi if user changes anything
extern bool r4aWifiStationOnline;         // WiFi station started successfully
extern bool r4aWifiStationRunning;        // False: stopped, True: starting, running, stopping

// List of known access points (APs)
extern const R4A_SSID_PASSWORD r4aWifiSsidPassword[];
extern const int r4aWifiSsidPasswordEntries;

// Get the WiFi station IP address
// Outputs:
//   Returns the IP address of the WiFi station
IPAddress r4aWifiStationIpAddress();

// Stop the WiFi station
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiStationOff(const char * fileName, uint32_t lineNumber);

// Start the WiFi station
// Inputs:
//   fileName: Name of file calling the enable routine
//   lineNumber: Line number in the file calling the enable routine
// Outputs:
//   Returns true if successful and false upon failure
bool r4aWifiStationOn(const char * fileName, uint32_t lineNumber);

// Get the SSID of the remote AP
// Outputs:
//   Returns the zero terminated SSID string of the remote AP
const char * r4aWifiStationSsid();

#endif  // __R4A_WIFI_H__
