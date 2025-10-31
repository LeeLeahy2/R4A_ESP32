/**********************************************************************
  Parameters.h

  Declare the parameters and set default values
**********************************************************************/

#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

//****************************************
// Basic Line Following (BLF)
//****************************************

int16_t blfSpeedSlow;
int16_t blfSpeedMedium;
int16_t blfSpeedFast;

//****************************************
// mDNS
//****************************************

const char * mdnsHostName;

//****************************************
// NTP
//****************************************

bool ntpEnable;

//****************************************
// NVM
//****************************************

const char * parameterFilePath;

//****************************************
// PCA9685
//****************************************

uint32_t pca9685FrequencyHz;

//****************************************
// Robot
//****************************************

bool robotDisplayIdle;
bool robotLineSensorLEDs;
volatile bool robotNtpTime;
uint16_t robotRunTimeSec;
volatile bool robotRunTime;
uint8_t robotStartDelaySec;

//****************************************
// Robot challenge
//****************************************

bool enableBatteryCheck;
uint8_t startIndex;

//****************************************
// Servos
//****************************************

uint8_t servoPanStartDegrees;
uint8_t servoTiltStartDegrees;

//****************************************
// SPI Flash
//****************************************

bool spiFlashServerEnable;
uint16_t r4aSpiFlashServerPort;

//****************************************
// Telnet
//****************************************

bool telnetEnable;

//****************************************
// Web server
//****************************************

bool webServerDebug;
bool webServerEnable;

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
    // Basic Line Following (BLF)
// Required    Type                  Minimum          Maximum   Address             Name            Default Value
    {true,  R4A_ESP32_NVM_PT_INT16,  (uint64_t)-4096,   4096,   &blfSpeedFast,      "blfFast",      4000},
    {true,  R4A_ESP32_NVM_PT_INT16,  (uint64_t)-4096,   4096,   &blfSpeedMedium,    "blfMedium",    3000},
    {true,  R4A_ESP32_NVM_PT_INT16,  (uint64_t)-4096,   4096,   &blfSpeedSlow,      "blfSlow",      1500},

    // Memory parameters
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_INT32,  0, (uint64_t)0xffffffff,   &r4aMallocMaxBytes,         "MallocMax",    (uint64_t)128},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aMallocDebug,            "MallocDebug",  false},

    // NTP
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &ntpEnable,                 "NtpEnable",    true},

    // NVM parameters
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &parameterFilePath,         "ParamFile",    R4A_ESP32_NVM_STRING("/Parameters.txt")},

    // PCA9685
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_UINT16, 24,         1525,          &pca9685FrequencyHz,        "pca9685Hz",    50},

    // Robot
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          255,           &robotStartDelaySec,        "RobotDelay",   R4A_CHALLENGE_SEC_START_DELAY},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &robotDisplayIdle,          "RobotIdle",    true},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &robotLineSensorLEDs,       "RobotLsLEDs",  true},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             (void *)&robotNtpTime,      "RobotNtpTime", true},
    {true,  R4A_ESP32_NVM_PT_UINT16, 0,          65535,         &robotRunTimeSec,           "RobotRtSec",   30},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             (void *)&robotRunTime,      "RobotRunTime", true},

    // Robot challenge
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &enableBatteryCheck,       "EnableBattery", true},
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,      CHALLENGE_MAX - 1, &startIndex,                "StartIndex",   0},

    // Servos
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          180,           &servoPanStartDegrees,      "panDegrees",   SERVO_PAN_START},
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          180,           &servoTiltStartDegrees,     "tiltDegrees",  SERVO_TILT_START},

    // SPI Flash
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &spiFlashServerEnable,      "FlashServer",  false},
    {true,  R4A_ESP32_NVM_PT_UINT16, 0,          65535,         &r4aSpiFlashServerPort,  "FlashServerPort", 6868},

    // Telnet
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &telnetEnable,             "telnetEnable",  true},

    // Time zone
// Required    Type                  Minimum        Maximum    Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-12, 12,        &r4aTimeZoneHours,          "tzHours",      (uint64_t)-10},
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-59, 59,        &r4aTimeZoneMinutes,        "tzMinutes",    0},
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-59, 59,        &r4aTimeZoneSeconds,        "tzSeconds",    0},

    // Web server
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &webServerDebug,            "WebDebug",     false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &webServerEnable,           "WebServer",    false},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWebServerNvmArea,       "WebNvmArea",   R4A_ESP32_NVM_STRING(DOWNLOAD_AREA)},

    // WiFi: Public Access Points (APs)
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aWifiDebug,              "WiFiDebug",    false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aWifiVerbose,            "WiFiVerbose",  false},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiHostName,           "WiFiHostName", R4A_ESP32_NVM_STRING("robot")},

    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiSoftApSsid,         "WiFiApSsid",   R4A_ESP32_NVM_STRING("4WD Car")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aWifiSoftApPassword,     "WiFiApPass",   0},

    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID1,                 "WifiSsid1",    R4A_ESP32_NVM_STRING("IEEE")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword1,             "WifiPass1",    R4A_ESP32_NVM_STRING("IEEE-Robot")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID2,                 "WifiSsid2",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword2,             "WifiPass2",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID3,                 "WifiSsid3",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword3,             "WifiPass3",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiSSID4,                 "WifiSsid4",    R4A_ESP32_NVM_STRING("")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wifiPassword4,             "WifiPass4",    R4A_ESP32_NVM_STRING("")},
};
const int nvmParameterCount = sizeof(nvmParameters) / sizeof(nvmParameters[0]);

#endif  // __PARAMETERS_H__
