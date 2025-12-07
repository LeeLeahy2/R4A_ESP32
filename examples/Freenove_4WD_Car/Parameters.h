/**********************************************************************
  Parameters.h

  Declare the parameters and set default values
**********************************************************************/

#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

//****************************************
// Advanced Line Following
//****************************************

int16_t alfSpeedForward;        // Speed going forward

//****************************************
// Basic Line Following (BLF)
//****************************************

int16_t blfSpeedSlow;
int16_t blfSpeedMedium;
int16_t blfSpeedFast;

//****************************************
// Camera Line Following (CLF)
//****************************************

uint8_t clfPanStartDegrees;
uint8_t clfTiltStartDegrees;

//****************************************
// NTP
//****************************************

bool ntpEnable;

//****************************************
// NVM
//****************************************

const char * parameterFilePath;

//****************************************
// OV2640 camera
//****************************************

bool ov2640Enable;

//****************************************
// Robot
//****************************************

bool robotDisplayIdle;
bool robotLineSensorLEDs;
volatile bool robotNtpTime;
volatile bool robotRunTime;
uint16_t robotRunTimeSec;
uint8_t robotStartDelaySec;
bool robotUseWS2812;

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
// Telnet
//****************************************

bool telnetEnable;

//****************************************
// Waypoints
//****************************************

const char * wpLogFileName;

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
    // Advanced Line Following
// Required    Type                  Minimum         Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_INT16,  0,              4096,          &alfSpeedForward,           "AlfForward",   800},

    // Basic Line Following (BLF)
// Required    Type                  Minimum          Maximum   Address             Name            Default Value
    {true,  R4A_ESP32_NVM_PT_INT16,  (uint64_t)-4096,   4096,   &blfSpeedFast,      "blfFast",      4000},
    {true,  R4A_ESP32_NVM_PT_INT16,  (uint64_t)-4096,   4096,   &blfSpeedMedium,    "blfMedium",    3000},
    {true,  R4A_ESP32_NVM_PT_INT16,  (uint64_t)-4096,   4096,   &blfSpeedSlow,      "blfSlow",      1500},

#ifdef  USE_OV2640
    // Camera Line Following (CLF)
// Required    Type                  Minimum     Maximum        Address                         Name            Default Value
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          180,           &clfPanStartDegrees,            "clfPanDeg",    10},
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          180,           &servoTiltStartDegrees,         "clfTiltDeg",   90},
#endif  // USE_OV2640

#ifdef  USE_ZED_F9P
    // GPS (GNSS)
// Required    Type                  Minimum     Maximum        Address                         Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplayAltitude,      "gnssAltitude", false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplayAltitudeStdDev,"gnssAltStdDev",false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplayFixType,       "gpsDispFix",   false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplayHorizAcc,      "gnssHorizAcc", false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplayHorizAccStdDev,"gnssHaStdDev", false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplayLatitude,      "gnssLatLtude", false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,    (void *)&r4aZedF9pDisplayLatitudeStdDev, "gnssLatStdDev",false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplayLongitude,     "gnssLongitude",false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,   (void *)&r4aZedF9pDisplayLongitudeStdDev, "gnssLongStdDev",false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplaySiv,           "gnssSIV",      false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,     (void *)&r4aZedF9pDisplayTime,          "gnssTime",     false},

    {true,  R4A_ESP32_NVM_PT_UINT32, 0,          (600 * 1000),  &r4aZedF9pLocationDisplayMsec,  "GnssDispMsec", 1000},
    {true,  R4A_ESP32_NVM_PT_UINT32, 0,          (60 * 1000),   &r4aZedF9pPollMsec,             "GnssPollMsec", 100},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aZedF9pUnitsFeetInches,      "GnssUseFeet",  true},
#endif  // USE_ZED_F9P

    // Memory parameters
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_INT32,  0, (uint64_t)0xffffffff,   &r4aMallocMaxBytes,         "MallocMax",    (uint64_t)128},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &r4aMallocDebug,            "MallocDebug",  false},

    // NTP
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &ntpEnable,                 "NtpEnable",    true},

#ifdef  USE_NTRIP
    // NTRIP parameters
// Required    Type                  Minimum     Maximum    Address                             Name            Default Value
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,         &r4aNtripClientCasterHost,          "NtripHost",   R4A_ESP32_NVM_STRING("rtk2go.com")},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,         &r4aNtripClientCasterMountPoint,    "NtripMP",     R4A_ESP32_NVM_STRING("MH2-79")},
    {true,  R4A_ESP32_NVM_PT_UINT16, 0,          0xffff,    &r4aNtripClientCasterPort,          "NtripPort",   2101},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,         &r4aNtripClientCasterUser,          "NtripUser",   R4A_ESP32_NVM_STRING("lpleahyjr@gmail.com")},
    {false, R4A_ESP32_NVM_PT_P_CHAR, 0,          0,         &r4aNtripClientCasterUserPW,        "NtripUserPw", R4A_ESP32_NVM_STRING("")},

    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,         &r4aNtripClientCompany,             "Company",      R4A_ESP32_NVM_STRING("Freenove")},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,         &r4aNtripClientProduct,             "Product",      R4A_ESP32_NVM_STRING("4WD_Robot")},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,         &r4aNtripClientProductVersion,      "ProdVersion",  R4A_ESP32_NVM_STRING("v1.0")},

    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,         (void *)&r4aNtripClientEnable,      "NtripEnable",  false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,         (void *)&r4aNtripClientDebugRtcm,   "NtripRtcm",    false},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,         (void *)&r4aNtripClientDebugState,  "NtripState",   false},

    {true,  R4A_ESP32_NVM_PT_UINT32, 0,          0xffffffff, &r4aNtripClientReceiveTimeout,     "NtripRxTmo",   (60 * 1000)},
    {true,  R4A_ESP32_NVM_PT_UINT32, 0,          0xffffffff, &r4aNtripClientResponseDone,       "NtripRspDone", 1000},
    {true,  R4A_ESP32_NVM_PT_UINT32, 0,          0xffffffff, &r4aNtripClientResponseTimeout,    "NtripRspTmo",  (10 * 1000)},
#endif  // USE_NTRIP

    // NVM parameters
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &parameterFilePath,         "ParamFile",    R4A_ESP32_NVM_STRING("/Parameters.txt")},

#ifdef  USE_OV2640
    // OV2640 camera
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &ov2640Enable,              "Camera",       false},
#endif  // USE_OV2640

    // Robot
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          255,           &robotStartDelaySec,        "RobotDelay",   R4A_CHALLENGE_SEC_START_DELAY},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &robotDisplayIdle,          "RobotIdle",    true},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &robotLineSensorLEDs,       "RobotLsLEDs",  true},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             (void *)&robotNtpTime,      "RobotNtpTime", true},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             (void *)&robotRunTime,      "RobotRunTime", true},
    {true,  R4A_ESP32_NVM_PT_UINT16, 0,          65535,         &robotRunTimeSec,           "RobotRtSec",   30},
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &robotUseWS2812,            "RobotWS2812",  true},

    // Robot challenge
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &enableBatteryCheck,       "EnableBattery", true},
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,      CHALLENGE_MAX - 1, &startIndex,                "StartIndex",   0},

    // Servos
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          180,           &servoPanStartDegrees,      "panDegrees",   SERVO_PAN_START},
    {true,  R4A_ESP32_NVM_PT_UINT8,  0,          180,           &servoTiltStartDegrees,     "tiltDegrees",  SERVO_TILT_START},

    // Telnet
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_BOOL,   0,          1,             &telnetEnable,              "telnetEnable", true},

    // Time zone
// Required    Type                  Minimum        Maximum    Address                      Name            Default Value
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-12, 12,        &r4aTimeZoneHours,           "tzHours",      (uint64_t)-10},
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-59, 59,        &r4aTimeZoneMinutes,         "tzMinutes",    0},
    {true,  R4A_ESP32_NVM_PT_INT8,   (uint64_t)-59, 59,        &r4aTimeZoneSeconds,         "tzSeconds",    0},

    // Waypoints
// Required    Type                  Minimum     Maximum        Address                     Name            Default Value
    {true,  R4A_ESP32_NVM_PT_UINT32, 0,          0xffffffff,    &r4aEsp32WpPointsToAverage, "wpPtCnt",      GNSS_POINTS_TO_AVERAGE},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &r4aEsp32WpFileName,        "wpFile",       R4A_ESP32_NVM_STRING("Waypoints.txt")},
    {true,  R4A_ESP32_NVM_PT_P_CHAR, 0,          0,             &wpLogFileName,             "wpLogFile",    R4A_ESP32_NVM_STRING("Waypoints.log")},

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
