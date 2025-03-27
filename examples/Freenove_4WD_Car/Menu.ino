/**********************************************************************
  Menu.ino

  Menu routines
**********************************************************************/

#ifdef  USE_ZED_F9P

//*********************************************************************
// Display the computed point
// Inputs:
//   parameter: Callback parameter passed to computeWayPoint
//   comment: Text to display at the start of the line
//   latitude: Latitude in degrees
//   longitude: Longitude in degrees
//   altitude: Altitude in meters
//   horizontalAccuracy: Accuracy in meters
//   satellitesInView: The number of satellites feeding the GNSS receiver
//   latitudeStdDev: Latitude standard deviation in degrees
//   longitudeStdDev: Longitude standard deviation in degrees
//   altitudeStdDev: Altitude standard deviation in meters
//   horizontalAccuracyStdDev: Horizontal accuracy standard deviation in meters
//   display: Device used for output, passed to computeWayPoint
void gnssDisplayPoint(intptr_t parameter,
                      const char * comment,
                      double latitude,
                      double latitudeStdDev,
                      double longitude,
                      double longitudeStdDev,
                      double altitude,
                      double altitudeStdDev,
                      double horizontalAccuracy,
                      double horizontalAccuracyStdDev,
                      uint8_t satellitesInView,
                      Print * display)
{
    zedf9p.displayLocation(comment,
                           latitude,
                           0,
                           longitude,
                           0,
                           altitude,
                           0,
                           horizontalAccuracy,
                           0,
                           satellitesInView,
                           true,    // unitsFeetInches,
                           false,   // displayTime,
                           true,    // displaySiv,
                           true,    // displayLatitude,
                           false,   // displayLatStdDev,
                           true,    // displayLongitude,
                           false,   // displayLongStdDev,
                           true,    // displayHorizAcc,
                           false,   // displayHorizAccStdDev,
                           false,   // displayAltitude,
                           false,   // displayAltitudeStdDev,
                           false,   // displayFixType,
                           display);
}

//*********************************************************************
// Compute point and display point
void gnssMenuComputePoint(const R4A_MENU_ENTRY * menuEntry,
                          const char * command,
                          Print * display)
{
    zedf9p.computePoint(gnssDisplayPoint,
                        0,
                        GNSS_POINTS_TO_AVERAGE,
                        "Location",
                        display);
}

//*********************************************************************
// Display the location
void gnssMenuDisplayLocation(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display)
{
    zedf9p.displayLocation("Location: ",
                           true,    // unitsFeetInches,
                           false,   // displayTime,
                           true,    // displaySiv,
                           true,    // displayLatitude
                           true,    // displayLongitude
                           true,    // displayHorizAcc,
                           false,   // displayAltitude,
                           false,   // displayFixType,
                           display);
}
#endif  // USE_ZED_F9P

//*********************************************************************
// Display data before the main menu header
void mainMenuPre(Print * display)
{
    if (r4aWifiSsidPasswordEntries)
    {
        // Display the WIFI status
        const char * hostName = r4aWifiHostName;
        if (hostName)
            display->printf("%s (%s): %s channel %d\r\n",
                            hostName,
                            WiFi.localIP().toString().c_str(),
                            r4aWifiStationSsid(),
                            r4aWifiChannel);
        else
            display->printf("%s: %s channel %d\r\n",
                            WiFi.localIP().toString().c_str(),
                            r4aWifiStationSsid(),
                            r4aWifiChannel);

        // Display the current time
        r4aNtpDisplayDateTime(display);
    }
    DISPLAY_BATTERY_VOLTAGE(display);
}

//*********************************************************************
// Toggle the backup lights
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void ledMenuBackup(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display)
{
    car.backupLightsToggle();
}

//*********************************************************************
// Toggle the brake lights
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void ledMenuBrake(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display)
{
    car.brakeLightsToggle();
}

//*********************************************************************
// Toggle the headlights
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void ledMenuHeadlights(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display)
{
    car.headlightsToggle();
}

//*********************************************************************
// Turn off all the LEDs
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void ledMenuOff(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display)
{
    car.backupLightsOff();
    car.brakeLightsOff();
    car.headlightsOff();
    car.ledsTurnOff();
    r4aLEDsOff();
}

//*********************************************************************
// Turn left indicator
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void ledMenuTurnLeft(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display)
{
    car.ledsTurnLeft();
}


//*********************************************************************
// Stop the turn signal blinking
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void ledMenuTurnOff(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display)
{
    car.ledsTurnOff();
}

//*********************************************************************
// Turn right indicator
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void ledMenuTurnRight(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display)
{
    car.ledsTurnRight();
}

//*********************************************************************
// Stop the robot
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void robotMenuStop(const R4A_MENU_ENTRY * menuEntry,
                   const char * command,
                   Print * display)
{
    r4aRobotStop(&robot, millis(), display);
}

//*********************************************************************
// Restart the WiFi station
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void wifiMenuRestart(const R4A_MENU_ENTRY * menuEntry,
                     const char * command,
                     Print * display)
{
    r4aWifiRestartRequested = true;
}

//*********************************************************************
// Toggle WiFi boolean
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void wifiToggleBool(const R4A_MENU_ENTRY * menuEntry,
                    const char * command,
                    Print * display)
{
    r4aMenuBoolToggle(menuEntry, command, display);
    r4aEsp32NvmMenuParameterFileWrite(menuEntry, command, display);
}

//*********************************************************************
// Display data before the waypoint menu header
void wpMenuPre(Print * display)
{
#ifdef  USE_ZED_F9P
    zedf9p.displayLocation(nullptr, // comment
                           zedf9p._latitude,
                           0.,      // latitudeStdDev
                           zedf9p._longitude,
                           0.,      // longitudeStdDev
                           zedf9p._altitude,
                           0.,      // altitudeStdDev
                           zedf9p._horizontalAccuracy,
                           0.,      // horizontalAccuracyStdDev
                           zedf9p._satellitesInView,
                           true,    // unitsFeetInches
                           false,   // displayTime
                           true,    // displaySiv
                           true,    // displayLatitude
                           false,   // displayLatStdDev
                           true,    // displayLongitude
                           false,   // displayLongStdDev
                           true,    // displayHorizAcc
                           false,   // displayHorizAccStdDev
                           false,   // displayAltitude
                           false,   // displayAltitudeStdDev
                           false,   // displayFixType
                           display);
#endif  // USE_ZED_F9P
}

//*********************************************************************

enum MENU_TABLE_INDEX
{
    MTI_DEBUG = R4A_MENU_MAIN + 1,
#ifdef  USE_ZED_F9P
    MTI_GNSS,
#endif  // USE_ZED_F9P
    MTI_I2C,
    MTI_LED,
    MTI_MOTOR,
#ifdef  USE_NTRIP
    MTI_NTRIP,
#endif  // USE_NTRIP
    MTI_NVM,
    MTI_SERVO,
    MTI_TELNET,
    MTI_WAY_POINT,
};

// Debug menu
const R4A_MENU_ENTRY debugMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine align   HelpText
    {"h",       r4aEsp32MenuDisplayHeap,    0,              nullptr,    0,      "Display the heap"},
    {"i",       nullptr,                    MTI_I2C,        nullptr,    0,      "I2C menu"},
    {"l",       nullptr,                    MTI_LED,        nullptr,    0,      "LED menu"},
    {"m",       nullptr,                    MTI_MOTOR,      nullptr,    0,      "Motor menu"},
    {"p",    r4aEsp32MenuDisplayPartitions, 0,              nullptr,    0,      "Display the partitions"},
    {"s",       nullptr,                    MTI_SERVO,      nullptr,    0,      "Servo menu"},
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"},
};
#define DEBUG_MENU_ENTRIES      sizeof(debugMenuTable) / sizeof(debugMenuTable[0])

#ifdef  USE_ZED_F9P
// GNSS menu
const R4A_MENU_ENTRY gnssMenuTable[] =
{
    // Command  menuRoutine         menuParam   HelpRoutine     align   HelpText
    {"l",  gnssMenuDisplayLocation, 0,          nullptr,        0,      "Display location"},
    {"p",     gnssMenuComputePoint, 0,          nullptr,        0,      "Compute point and display point"},
    {"x",       nullptr,         R4A_MENU_MAIN, nullptr,        0,      "Return to the main menu"},
};
#define GNSS_MENU_ENTRIES       sizeof(gnssMenuTable) / sizeof(gnssMenuTable[0])
#endif  // USE_ZED_F9P

// LED menu
const R4A_MENU_ENTRY ledMenuTable[] =
{
    // Command  menuRoutine         menuParam               HelpRoutine         align   HelpText
    {"b",       ledMenuBackup,      0,                      nullptr,            0,      "Toggle backup lights"},
    {"c3",      r4aLEDMenuColor3,   (intptr_t)"ll rrggbb",  r4aMenuHelpSuffix,  9,      "Specify the LED ll color rrggbb (RGB in hex)"},
    {"c4",      r4aLEDMenuColor4,  (intptr_t)"ll wwrrggbb", r4aMenuHelpSuffix,  11,     "Specify the LED ll color cccccccc (RGBW in hex)"},
    {"d",       r4aLEDMenuDisplay,  0,                      nullptr,            0,      "Display the LED status"},
    {"h",       ledMenuHeadlights,  0,                      nullptr,            0,      "Toggle headlights"},
    {"i",      r4aLEDMenuIntensity, (intptr_t)"iii",        r4aMenuHelpSuffix,  4,      "Specify the LED intensity iii (0 - 255)"},
    {"l",       ledMenuTurnLeft,    0,                      nullptr,            0,      "Turn left"},
    {"o",       ledMenuTurnOff,     0,                      nullptr,            0,      "Stop the turn signal blinking"},
    {"off",     ledMenuOff,         0,                      nullptr,            0,      "Turn off all the LEDs"},
    {"r",       ledMenuTurnRight,   0,                      nullptr,            0,      "Turn right"},
    {"s",       ledMenuBrake,       0,                      nullptr,            0,      "Toggle brake lights"},
    {"x",       nullptr,            R4A_MENU_MAIN,          nullptr,            0,      "Return to the main menu"},
};
#define LED_MENU_ENTRIES      sizeof(ledMenuTable) / sizeof(ledMenuTable[0])

// Telnet menu
const R4A_MENU_ENTRY telnetMenuTable[] =
{
    // Command  menuRoutine     menuParam       HelpRoutine align   HelpText
    {"c",       listClients,    0,              nullptr,    0,      "Client list"},
    {"s",       serverInfo,     0,              nullptr,    0,      "Server information"},
    {"x",       nullptr,        R4A_MENU_MAIN,  nullptr,    0,      "Exit the menu system"},
};
#define TELNET_MENU_ENTRIES  sizeof(telnetMenuTable) / sizeof(telnetMenuTable[0])

// Way Point menu
#ifdef  USE_ZED_F9P
const R4A_MENU_ENTRY wayPointMenuTable[] =
{
    // Command  menuRoutine                 menuParam               HelpRoutine         align   HelpText
    {"a",       r4aEsp32WpMenuAddPoint,     (intptr_t)"comment",    r4aMenuHelpSuffix,  7,      "Add a point to the file"},
    {"cat",     r4aEsp32NvmMenuFileCat,     (intptr_t)"ffff",       r4aMenuHelpSuffix,  4,      "Display the contents of file ffff"},
    {"cp",      r4aEsp32NvmMenuFileCopy,    (intptr_t)"src dest",   r4aMenuHelpSuffix,  8,      "Copy src file to dest file"},
    {"dp",      r4aEsp32WpMenuDisplayPoint, 0,                      nullptr,            0,      "Display the next waypoint"},
    {"dump",    r4aEsp32NvmMenuFileDump,    (intptr_t)"ffff",       r4aMenuHelpSuffix,  4,      "Dump the contents of file ffff"},
    {"f",       r4aEsp32WpMenuFileName,     (intptr_t)"nnnn",       r4aMenuHelpSuffix,  4,      "Set waypoint file name"},
    {"ls",      r4aEsp32NvmMenuFileList,    0,                      nullptr,            0,      "List the NVM directory"},
    {"mv",      r4aEsp32NvmMenuFileMove,    (intptr_t)"src dest",   r4aMenuHelpSuffix,  8,      "Rename a file"},
    {"p",       r4aEsp32WpMenuPrintFile,    0,                      nullptr,            0,      "Print the waypoint file contents"},
    {"rm",      r4aEsp32NvmMenuFileRemove,  (intptr_t)"ffff",       r4aMenuHelpSuffix,  4,      "Remove file ffff"},
    {"wget",    r4aEsp32NvmMenuHttpFileGet, (intptr_t)"url",        r4aMenuHelpSuffix,  3,      "Get a file from a web server"},
    {"x",       nullptr,                    R4A_MENU_MAIN,          nullptr,            0,      "Exit the menu system"},
};
#define WAYPOINT_MENU_ENTRIES  sizeof(wayPointMenuTable) / sizeof(wayPointMenuTable[0])
#endif  // USE_ZED_F9P

// Main menu
const R4A_MENU_ENTRY mainMenuTable[] =
{
    // Command  menuRoutine         menuParam       HelpRoutine align   HelpText
    {"alf",     menuAlfStart,       0,              nullptr,    0,      "Advanced line following"},
    {"blf",     menuBlfStart,       0,              nullptr,    0,      "Basic line following"},
    {"blt",     menuBltStart,       0,              nullptr,    0,      "Basic light tracking"},
#ifdef  USE_OV2640
    {"c", r4aMenuBoolToggle, (intptr_t)&ov2640Enable, r4aMenuBoolHelp, 0, "Toggle OV2640 camera"},
    {"clf",     menuClfStart,       0,              nullptr,    0,      "Camera line following"},
#endif  // USE_OV2640
    {"d",       nullptr,            MTI_DEBUG,      nullptr,    0,      "Enter the debug menu"},
#ifdef  USE_ZED_F9P
    {"g",       nullptr,            MTI_GNSS,       nullptr,    0,      "Enter the GNSS menu"},
#endif  // USE_ZED_F9P
    {"i",  r4aMenuBoolToggle, (intptr_t)&ignoreBatteryCheck, r4aMenuBoolHelp, 0, "Ignore the battery check"},
#ifdef  USE_NTRIP
    {"NTRIP",   nullptr,            MTI_NTRIP,      nullptr,    0,      "Enter the NTRIP menu"},
#endif  // USE_NTRIP
    {"nvm",     nullptr,            MTI_NVM,        nullptr,    0,      "Enter the NVM menu"},
    {"r",  r4aEsp32MenuSystemReset, 0,              nullptr,    0,      "System reset"},
    {"s",       robotMenuStop,      0,              nullptr,    0,      "Stop the robot"},
    {"t",       nullptr,            MTI_TELNET,     nullptr,    0,      "Enter the telnet menu"},
    {"w", r4aMenuBoolToggle, (intptr_t)&webServerEnable, r4aMenuBoolHelp, 0, "Toggle web server"},
    {"wd",    wifiToggleBool, (intptr_t)&r4aWifiDebug, r4aMenuBoolHelp, 0, "Toggle WiFi debugging"},
    {"wp",      nullptr,            MTI_WAY_POINT,  nullptr,    0,      "Enter the waypoint menu"},
#ifdef  USE_ZED_F9P
    {"wpf",     wpfStart,           0,              nullptr,    0,      "Waypoint following"},
#endif  // USE_ZED_F9P
    {"wr",      wifiMenuRestart,    0,              nullptr,    0,      "Restart WiFi"},
    {"wv",  wifiToggleBool, (intptr_t)&r4aWifiVerbose, r4aMenuBoolHelp, 0, "Toggle WiFi verbose output"},
    {"x",       nullptr,            R4A_MENU_NONE,  nullptr,    0,      "Exit the menu system"},
};
#define MAIN_MENU_ENTRIES       sizeof(mainMenuTable) / sizeof(mainMenuTable[0])

const R4A_MENU_TABLE menuTable[] =
{
    // menuName         preMenu routine firstEntry          entryCount
    {"Main Menu",       mainMenuPre,    mainMenuTable,      MAIN_MENU_ENTRIES},
    {"Debug Menu",      nullptr,        debugMenuTable,     DEBUG_MENU_ENTRIES},
#ifdef  USE_ZED_F9P
    {"GNSS Menu",       nullptr,        gnssMenuTable,      GNSS_MENU_ENTRIES},
#endif  // USE_ZED_F9P
    {"I2C Menu",        nullptr,        r4aI2cMenuTable,    R4A_I2C_MENU_ENTRIES},
    {"LED Menu",        nullptr,        ledMenuTable,       LED_MENU_ENTRIES},
//    {"LED Menu",        nullptr,        r4aLEDMenuTable,    R4A_LED_MENU_ENTRIES},
    {"Motor Menu",      nullptr,  r4aPca9685MotorMenuTable, R4A_PCA9685_MOTOR_MENU_ENTRIES},
#ifdef  USE_NTRIP
    {"NTRIP Menu",      nullptr,  r4aNtripClientMenuTable,  R4A_NTRIP_CLIENT_MENU_ENTRIES},
#endif  // USE_NTRIP
    {"NVM Menu",        nullptr,      r4aEsp32NvmMenuTable, R4A_ESP32_NVM_MENU_ENTRIES},
    {"Servo Menu",      nullptr,  r4aPca9685ServoMenuTable, R4A_PCA9685_SERVO_MENU_ENTRIES},
    {"Telnet Menu",     nullptr,        telnetMenuTable,    TELNET_MENU_ENTRIES},
#ifdef  USE_ZED_F9P
    {"Waypoint Menu",   wpMenuPre,      wayPointMenuTable,  WAYPOINT_MENU_ENTRIES},
#endif  // USE_ZED_F9P
};
const int menuTableEntries = sizeof(menuTable) / sizeof(menuTable[0]);
