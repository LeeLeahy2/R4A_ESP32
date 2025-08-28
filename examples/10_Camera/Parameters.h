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
// Servos
//****************************************

uint8_t servoPanStartDegrees;
uint8_t servoTiltStartDegrees;

//****************************************
// Web server
//****************************************

bool webServerDebug;

//****************************************
// WiFi Access Points
//****************************************

// Public access point SSID and password
const char * wifiSSID1;
const char * wifiPassword1;
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

    // Servos
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          180,           &servoPanStartDegrees,      "PanDegrees",   SERVO_PAN_START},
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          180,           &servoTiltStartDegrees,     "TiltDegrees",  SERVO_TILT_START},

    // Web server
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &webServerDebug,            "WebDebug",     false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aWebServerEnable,        "WebServer",    false},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWebServerNvmArea,       "WebNvmArea",   R4A_ESP32_NVM_STRING(DOWNLOAD_AREA)},

    // WiFi: Public Access Points (APs)
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aWifiDebug,              "WiFiDebug",    false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aWifiVerbose,            "WiFiVerbose",  false},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiHostName,           "WiFiHostName", R4A_ESP32_NVM_STRING("robot")},

    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiSoftApSsid,         "WiFiApSsid",   R4A_ESP32_NVM_STRING("4WD Car")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiSoftApPassword,     "WiFiApPass",   0},

    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID1,                 "WifiSsid1",    R4A_ESP32_NVM_STRING("IEEE")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword1,             "WifiPass1",    R4A_ESP32_NVM_STRING("Robot-Dev")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID2,                 "WifiSsid2",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword2,             "WifiPass2",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID3,                 "WifiSsid3",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword3,             "WifiPass3",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID4,                 "WifiSsid4",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword4,             "WifiPass4",    R4A_ESP32_NVM_STRING("")},
};
const int nvmParameterCount = sizeof(nvmParameters) / sizeof(nvmParameters[0]);

#endif  // __PARAMETERS_H__
