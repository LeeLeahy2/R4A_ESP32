/**********************************************************************
  Parameters.h

  Declare the parameters and set default values
**********************************************************************/

#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

//****************************************
// NVM
//****************************************

const char * parameterFilePath;

//****************************************
// WiFi Access Points
//****************************************

// Public access point SSID and password
const char * wifiSSID;
const char * wifiPassword;
const char * wifiSSID2;
const char * wifiPassword2;
const char * wifiSSID3;
const char * wifiPassword3;
const char * wifiSSID4;
const char * wifiPassword4;

//****************************************
// NVM Parameters
//****************************************

const R4A_ESP32_NVM_PARAMETER nvmParameters[] =
{
    // NVM parameters
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &parameterFilePath,         "ParamFile",    R4A_ESP32_NVM_STRING("/Parameters.txt")},

    // Time zone
// Required    Type                  Minimum        Maximum    Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-12, 12,        &r4aTimeZoneHours,          "tzHours",      (uint64_t)-10},
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-59, 59,        &r4aTimeZoneMinutes,        "tzMinutes",    0},
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-59, 59,        &r4aTimeZoneSeconds,        "tzSeconds",    0},

    // WiFi: Public Access Points (APs)
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aWifiDebug,              "WiFiDebug",    false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aWifiVerbose,            "WiFiVerbose",  false},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiHostName,           "WiFiHostName", R4A_ESP32_NVM_STRING("robot")},

    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiSoftApSsid,         "WiFiApSsid",   R4A_ESP32_NVM_STRING("4WD Car")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiSoftApPassword,     "WiFiApPass",   0},

    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID,                  "WifiSsid",     R4A_ESP32_NVM_STRING("IEEE")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword,              "WifiPass",     R4A_ESP32_NVM_STRING("Robot-Dev")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID2,                 "WifiSsid2",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword2,             "WifiPass2",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID3,                 "WifiSsid3",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword3,             "WifiPass3",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID4,                 "WifiSsid4",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword4,             "WifiPass4",    R4A_ESP32_NVM_STRING("")},
};
const int nvmParameterCount = sizeof(nvmParameters) / sizeof(nvmParameters[0]);

#endif  // __PARAMETERS_H__
